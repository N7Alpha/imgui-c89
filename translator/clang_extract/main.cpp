/*
 * Semantic extractor for the Dear ImGui C89 translator.
 *
 * This is deliberately a LibTooling consumer rather than an AST-dump parser.
 * Its JSON output is our versioned boundary; Clang's internal node addresses
 * and diagnostic serialization are not allowed to leak into that contract.
 */

#include <memory>
#include <string>
#include <system_error>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/RecordLayout.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Version.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Index/USRGeneration.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::tooling;

namespace {

llvm::cl::OptionCategory ToolCategory("imgui-c89 extractor options");
llvm::cl::opt<std::string> OutputPath("output",
                                      llvm::cl::desc("IR JSON output path"),
                                      llvm::cl::Required,
                                      llvm::cl::cat(ToolCategory));
llvm::cl::opt<std::string> SourceRoot(
    "source-root",
    llvm::cl::desc(
        "Include declarations whose expansion file is below this root"),
    llvm::cl::init(""), llvm::cl::cat(ToolCategory));
llvm::cl::opt<bool>
    TraceDeclarations("trace-declarations",
                      llvm::cl::desc("Print declarations before extraction"),
                      llvm::cl::init(false), llvm::cl::cat(ToolCategory));
llvm::cl::opt<bool> DeclarationsOnly(
    "declarations-only",
    llvm::cl::desc(
        "Omit function bodies and constructor initializer expressions"),
    llvm::cl::init(false), llvm::cl::cat(ToolCategory));

static std::string declaration_id(const Decl *decl) {
  /* Calls in an out-of-line definition refer to the header declaration.
   * Always key declarations by their canonical AST declaration so the
   * definition and every call site share one stable identity. */
  const Decl *canonical = decl->getCanonicalDecl();
  llvm::SmallString<256> usr;
  std::string result;
  if (!clang::index::generateUSRForDecl(canonical, usr))
    result = std::string(usr.str());
  else if (const NamedDecl *named = dyn_cast<NamedDecl>(canonical))
    result = named->getQualifiedNameAsString();
  else
    result = canonical->getDeclKindName();

  /* Clang USRs intentionally omit some lexical scopes for declarations local
   * to a function (notably repeated local enums/records with the same name).
   * That is sufficient for indexing but not for source translation.  Add a
   * stable source coordinate for every function-local declaration so enum
   * constants and local class members retain their actual identity. */
  const DeclContext *context = canonical->getDeclContext();
  bool function_local = false;
  while (context) {
    if (isa<FunctionDecl>(context)) {
      function_local = true;
      break;
    }
    context = context->getParent();
  }
  if (function_local) {
    const SourceManager &source = canonical->getASTContext().getSourceManager();
    PresumedLoc location =
        source.getPresumedLoc(source.getExpansionLoc(canonical->getLocation()));
    if (location.isValid()) {
      result += "@local:";
      result += location.getFilename();
      result += ":" + std::to_string(location.getLine());
      result += ":" + std::to_string(location.getColumn());
    }
  }
  return result;
}

static std::string declaration_namespace(const Decl *decl) {
  std::string result;
  const DeclContext *context = decl->getDeclContext();
  while (context) {
    if (const auto *namespace_decl = dyn_cast<NamespaceDecl>(context)) {
      if (!namespace_decl->isAnonymousNamespace()) {
        const std::string name = namespace_decl->getNameAsString();
        result = result.empty() ? name : name + "::" + result;
      }
    }
    context = context->getParent();
  }
  return result;
}

static bool location_in_scope(const SourceManager &source, SourceLocation loc) {
  SourceLocation location = source.getExpansionLoc(loc);
  if (!location.isValid())
    return false;
  if (SourceRoot.empty())
    return source.isWrittenInMainFile(location);
  llvm::SmallString<256> root(SourceRoot);
  llvm::SmallString<256> file(source.getFilename(location));
  llvm::sys::path::remove_dots(root, true);
  llvm::sys::path::remove_dots(file, true);
  llvm::sys::path::native(root);
  llvm::sys::path::native(file);
  if (!llvm::sys::path::is_separator(root.back()))
    root += llvm::sys::path::get_separator();
  return llvm::StringRef(file).starts_with(root);
}

class MacroCapture : public PPCallbacks {
public:
  MacroCapture(Preprocessor &preprocessor, llvm::json::Array &macros)
      : preprocessor_(preprocessor), source_(preprocessor.getSourceManager()),
        macros_(macros) {}

  void MacroDefined(const Token &name_token,
                    const MacroDirective *directive) override {
    const MacroInfo *info = directive->getMacroInfo();
    if (!info || !location_in_scope(source_, info->getDefinitionLoc()))
      return;
    llvm::json::Object item;
    llvm::json::Array parameters;
    llvm::json::Array replacement;
    const IdentifierInfo *identifier = name_token.getIdentifierInfo();
    item["name"] = identifier ? identifier->getName().str() : "";
    item["function_like"] = info->isFunctionLike();
    item["variadic"] = info->isVariadic();
    PresumedLoc presumed = source_.getPresumedLoc(info->getDefinitionLoc());
    if (presumed.isValid()) {
      llvm::json::Object where;
      where["file"] = std::string(presumed.getFilename());
      where["line"] = static_cast<int64_t>(presumed.getLine());
      where["column"] = static_cast<int64_t>(presumed.getColumn());
      item["location"] = std::move(where);
    }
    if (info->isFunctionLike()) {
      for (const IdentifierInfo *parameter : info->params())
        parameters.push_back(parameter ? parameter->getName().str() : "");
    }
    for (const Token &token : info->tokens())
      replacement.push_back(
          Lexer::getSpelling(token, source_, preprocessor_.getLangOpts()));
    item["parameters"] = std::move(parameters);
    item["replacement_tokens"] = std::move(replacement);
    macros_.push_back(std::move(item));
  }

private:
  Preprocessor &preprocessor_;
  SourceManager &source_;
  llvm::json::Array &macros_;
};

class IrWriter {
public:
  explicit IrWriter(ASTContext &context)
      : context_(context), source_(context.getSourceManager()),
        policy_(context.getLangOpts()) {
    policy_.SuppressScope = false;
    policy_.SuppressTagKeyword = true;
    policy_.Bool = true;
  }

  llvm::json::Object location(SourceLocation loc) const {
    llvm::json::Object out;
    PresumedLoc presumed = source_.getPresumedLoc(loc);
    if (presumed.isInvalid())
      return out;
    out["file"] = std::string(presumed.getFilename());
    out["line"] = static_cast<int64_t>(presumed.getLine());
    out["column"] = static_cast<int64_t>(presumed.getColumn());
    return out;
  }

  std::string type_name(QualType type) const {
    return type.getAsString(policy_);
  }

  llvm::json::Object expression(const Expr *expr) {
    llvm::json::Object out;
    if (!expr) {
      out["kind"] = "NullExpr";
      return out;
    }
    out["kind"] = expr->getStmtClassName();
    out["type"] = type_name(expr->getType());
    out["value_category"] = expr->isLValue()   ? "lvalue"
                            : expr->isXValue() ? "xvalue"
                                               : "prvalue";
    out["location"] = location(expr->getExprLoc());

    if (const IntegerLiteral *node = dyn_cast<IntegerLiteral>(expr)) {
      llvm::SmallString<64> value;
      node->getValue().toString(value, 10,
                                node->getType()->isSignedIntegerType());
      out["value"] = std::string(value);
    } else if (const FloatingLiteral *node = dyn_cast<FloatingLiteral>(expr)) {
      llvm::SmallString<64> value;
      node->getValue().toString(value);
      out["value"] = std::string(value);
    } else if (const CharacterLiteral *node =
                   dyn_cast<CharacterLiteral>(expr)) {
      out["value"] = static_cast<int64_t>(node->getValue());
    } else if (const CXXBoolLiteralExpr *node =
                   dyn_cast<CXXBoolLiteralExpr>(expr)) {
      out["value"] = node->getValue();
    } else if (const StringLiteral *node = dyn_cast<StringLiteral>(expr)) {
      out["value"] = node->getBytes().str();
    } else if (isa<CXXNullPtrLiteralExpr>(expr) || isa<GNUNullExpr>(expr)) {
      out["value"] = nullptr;
    } else if (const DeclRefExpr *node = dyn_cast<DeclRefExpr>(expr)) {
      out["decl"] = declaration_id(node->getDecl());
      out["name"] = node->getDecl()->getNameAsString();
    } else if (const CXXThisExpr *node = dyn_cast<CXXThisExpr>(expr)) {
      out["implicit"] = node->isImplicit();
    } else if (const MemberExpr *node = dyn_cast<MemberExpr>(expr)) {
      out["member"] = declaration_id(node->getMemberDecl());
      out["name"] = node->getMemberNameInfo().getAsString();
      out["arrow"] = node->isArrow();
      out["base"] = expression(node->getBase());
    } else if (const BinaryOperator *node = dyn_cast<BinaryOperator>(expr)) {
      out["opcode"] = node->getOpcodeStr().str();
      out["lhs"] = expression(node->getLHS());
      out["rhs"] = expression(node->getRHS());
    } else if (const UnaryOperator *node = dyn_cast<UnaryOperator>(expr)) {
      out["opcode"] = UnaryOperator::getOpcodeStr(node->getOpcode()).str();
      out["postfix"] = node->isPostfix();
      out["operand"] = expression(node->getSubExpr());
    } else if (const ConditionalOperator *node =
                   dyn_cast<ConditionalOperator>(expr)) {
      out["condition"] = expression(node->getCond());
      out["true"] = expression(node->getTrueExpr());
      out["false"] = expression(node->getFalseExpr());
    } else if (const ArraySubscriptExpr *node =
                   dyn_cast<ArraySubscriptExpr>(expr)) {
      out["base"] = expression(node->getBase());
      out["index"] = expression(node->getIdx());
    } else if (const ImplicitCastExpr *node =
                   dyn_cast<ImplicitCastExpr>(expr)) {
      out["cast_kind"] = node->getCastKindName();
      out["operand"] = expression(node->getSubExpr());
    } else if (const ExplicitCastExpr *node =
                   dyn_cast<ExplicitCastExpr>(expr)) {
      out["cast_kind"] = node->getCastKindName();
      out["operand"] = expression(node->getSubExpr());
    } else if (const ParenExpr *node = dyn_cast<ParenExpr>(expr)) {
      out["operand"] = expression(node->getSubExpr());
    } else if (const MaterializeTemporaryExpr *node =
                   dyn_cast<MaterializeTemporaryExpr>(expr)) {
      out["operand"] = expression(node->getSubExpr());
    } else if (const ExprWithCleanups *node =
                   dyn_cast<ExprWithCleanups>(expr)) {
      out["operand"] = expression(node->getSubExpr());
    } else if (const CXXDefaultArgExpr *node =
                   dyn_cast<CXXDefaultArgExpr>(expr)) {
      out["operand"] = expression(node->getExpr());
    } else if (const CXXDefaultInitExpr *node =
                   dyn_cast<CXXDefaultInitExpr>(expr)) {
      out["operand"] = expression(node->getExpr());
    } else if (const ConstantExpr *node = dyn_cast<ConstantExpr>(expr)) {
      out["operand"] = expression(node->getSubExpr());
    } else if (const CXXBindTemporaryExpr *node =
                   dyn_cast<CXXBindTemporaryExpr>(expr)) {
      out["operand"] = expression(node->getSubExpr());
    } else if (isa<ImplicitValueInitExpr>(expr)) {
      out["zero_initialize"] = true;
    } else if (const VAArgExpr *node = dyn_cast<VAArgExpr>(expr)) {
      out["operand"] = expression(node->getSubExpr());
    } else if (const OffsetOfExpr *node = dyn_cast<OffsetOfExpr>(expr)) {
      llvm::json::Array components;
      out["record_type"] = type_name(node->getTypeSourceInfo()->getType());
      for (unsigned index = 0; index < node->getNumComponents(); ++index) {
        const OffsetOfNode &component = node->getComponent(index);
        llvm::json::Object item;
        if (component.getKind() == OffsetOfNode::Field) {
          item["kind"] = "field";
          item["name"] = component.getField()->getNameAsString();
        } else if (component.getKind() == OffsetOfNode::Array) {
          item["kind"] = "array";
          item["index"] = expression(
              node->getIndexExpr(component.getArrayExprIndex()));
        } else {
          item["kind"] = "unsupported";
        }
        components.push_back(std::move(item));
      }
      out["components"] = std::move(components);
    } else if (const LambdaExpr *node = dyn_cast<LambdaExpr>(expr)) {
      out["capture_count"] = static_cast<int64_t>(node->capture_size());
      out["call_operator"] = function(node->getCallOperator());
    } else if (const CXXMemberCallExpr *node =
                   dyn_cast<CXXMemberCallExpr>(expr)) {
      llvm::json::Array arguments;
      for (const Expr *argument : node->arguments())
        arguments.push_back(expression(argument));
      if (const CXXMethodDecl *callee = node->getMethodDecl()) {
        out["callee"] = declaration_id(callee);
        out["callee_name"] = callee->getNameAsString();
        out["callee_qualified_name"] = callee->getQualifiedNameAsString();
        out["callee_project"] = location_in_scope(source_, callee->getLocation());
      }
      out["object"] = expression(node->getImplicitObjectArgument());
      out["arguments"] = std::move(arguments);
    } else if (const CallExpr *node = dyn_cast<CallExpr>(expr)) {
      llvm::json::Array arguments;
      for (const Expr *argument : node->arguments())
        arguments.push_back(expression(argument));
      if (const FunctionDecl *callee = node->getDirectCallee()) {
        out["callee"] = declaration_id(callee);
        out["callee_name"] = callee->getNameAsString();
        out["callee_qualified_name"] = callee->getQualifiedNameAsString();
        out["callee_project"] = location_in_scope(source_, callee->getLocation());
      } else {
        out["callee_expression"] = expression(node->getCallee());
      }
      out["arguments"] = std::move(arguments);
    } else if (const CXXConstructExpr *node =
                   dyn_cast<CXXConstructExpr>(expr)) {
      llvm::json::Array arguments;
      for (const Expr *argument : node->arguments())
        arguments.push_back(expression(argument));
      out["constructor"] = declaration_id(node->getConstructor());
      out["trivial"] = node->getConstructor()->isTrivial();
      out["arguments"] = std::move(arguments);
    } else if (const CXXPseudoDestructorExpr *node =
                   dyn_cast<CXXPseudoDestructorExpr>(expr)) {
      out["base"] = expression(node->getBase());
      out["destroyed_type"] = type_name(node->getDestroyedType());
    } else if (const CXXNewExpr *node = dyn_cast<CXXNewExpr>(expr)) {
      llvm::json::Array placement;
      for (const Expr *argument : node->placement_arguments())
        placement.push_back(expression(argument));
      out["allocated_type"] = type_name(node->getAllocatedType());
      out["array"] = node->isArray();
      out["placement_arguments"] = std::move(placement);
      if (node->hasInitializer())
        out["initializer"] = expression(node->getInitializer());
    } else if (const InitListExpr *node = dyn_cast<InitListExpr>(expr)) {
      llvm::json::Array values;
      for (const Expr *value : node->inits())
        values.push_back(expression(value));
      out["values"] = std::move(values);
      // Preserve the semantic field order even for aggregates declared by an
      // external dependency. The emitter normally resolves project records
      // by identity; external C records are deliberately not copied into the
      // project IR, but their field names still let C89 lowering replace a
      // late aggregate initialization with ordinary member assignments.
      if (const RecordType *record_type = node->getType()->getAs<RecordType>()) {
        llvm::json::Array fields;
        for (const FieldDecl *field : record_type->getDecl()->fields())
          fields.push_back(field->getNameAsString());
        out["field_names"] = std::move(fields);
      }
    } else if (const UnaryExprOrTypeTraitExpr *node =
                   dyn_cast<UnaryExprOrTypeTraitExpr>(expr)) {
      out["operator"] = node->getKind() == UETT_SizeOf ? "sizeof"
                        : node->getKind() == UETT_AlignOf
                            ? "alignof"
                            : node->getStmtClassName();
      if (node->isArgumentType())
        out["argument_type"] = type_name(node->getArgumentType());
      else
        out["argument"] = expression(node->getArgumentExpr());
    } else {
      llvm::json::Array children;
      for (const Stmt *child : expr->children()) {
        if (const Expr *child_expr = dyn_cast_or_null<Expr>(child))
          children.push_back(expression(child_expr));
      }
      out["children"] = std::move(children);
      out["unsupported"] = true;
    }
    return out;
  }

  llvm::json::Object statement(const Stmt *stmt) {
    llvm::json::Object out;
    if (!stmt) {
      out["kind"] = "NullStmt";
      return out;
    }
    if (const Expr *expr = dyn_cast<Expr>(stmt))
      return expression(expr);

    out["kind"] = stmt->getStmtClassName();
    out["location"] = location(stmt->getBeginLoc());
    if (const CompoundStmt *node = dyn_cast<CompoundStmt>(stmt)) {
      llvm::json::Array statements;
      for (const Stmt *child : node->body())
        statements.push_back(statement(child));
      out["statements"] = std::move(statements);
    } else if (const ReturnStmt *node = dyn_cast<ReturnStmt>(stmt)) {
      if (node->getRetValue())
        out["value"] = expression(node->getRetValue());
    } else if (const DeclStmt *node = dyn_cast<DeclStmt>(stmt)) {
      llvm::json::Array declarations;
      for (const Decl *decl : node->decls()) {
        const VarDecl *var = dyn_cast<VarDecl>(decl);
        if (!var)
          continue;
        declarations.push_back(variable(var));
      }
      out["declarations"] = std::move(declarations);
    } else if (const IfStmt *node = dyn_cast<IfStmt>(stmt)) {
      if (const VarDecl *var = node->getConditionVariable()) {
        llvm::json::Object declaration;
        llvm::json::Array declarations;
        declaration["kind"] = "DeclStmt";
        declarations.push_back(variable(var));
        declaration["declarations"] = std::move(declarations);
        out["condition_variable"] = std::move(declaration);
      }
      out["condition"] = expression(node->getCond());
      out["then"] = statement(node->getThen());
      if (node->getElse())
        out["else"] = statement(node->getElse());
    } else if (const ForStmt *node = dyn_cast<ForStmt>(stmt)) {
      if (node->getInit())
        out["initializer"] = statement(node->getInit());
      if (node->getCond())
        out["condition"] = expression(node->getCond());
      if (node->getInc())
        out["increment"] = expression(node->getInc());
      out["body"] = statement(node->getBody());
    } else if (const WhileStmt *node = dyn_cast<WhileStmt>(stmt)) {
      if (const VarDecl *var = node->getConditionVariable()) {
        llvm::json::Object declaration;
        llvm::json::Array declarations;
        declaration["kind"] = "DeclStmt";
        declarations.push_back(variable(var));
        declaration["declarations"] = std::move(declarations);
        out["condition_variable"] = std::move(declaration);
      }
      out["condition"] = expression(node->getCond());
      out["body"] = statement(node->getBody());
    } else if (const DoStmt *node = dyn_cast<DoStmt>(stmt)) {
      out["condition"] = expression(node->getCond());
      out["body"] = statement(node->getBody());
    } else if (const CXXForRangeStmt *node =
                   dyn_cast<CXXForRangeStmt>(stmt)) {
      if (node->getInit())
        out["initializer"] = statement(node->getInit());
      out["range"] = statement(node->getRangeStmt());
      out["begin"] = statement(node->getBeginStmt());
      out["end"] = statement(node->getEndStmt());
      out["condition"] = expression(node->getCond());
      out["increment"] = expression(node->getInc());
      out["loop_variable"] = statement(node->getLoopVarStmt());
      out["body"] = statement(node->getBody());
    } else if (const SwitchStmt *node = dyn_cast<SwitchStmt>(stmt)) {
      out["condition"] = expression(node->getCond());
      out["body"] = statement(node->getBody());
    } else if (const CaseStmt *node = dyn_cast<CaseStmt>(stmt)) {
      out["value"] = expression(node->getLHS());
      out["body"] = statement(node->getSubStmt());
    } else if (const DefaultStmt *node = dyn_cast<DefaultStmt>(stmt)) {
      out["body"] = statement(node->getSubStmt());
    } else if (const GotoStmt *node = dyn_cast<GotoStmt>(stmt)) {
      out["label"] = node->getLabel()->getNameAsString();
    } else if (const LabelStmt *node = dyn_cast<LabelStmt>(stmt)) {
      out["label"] = node->getName();
      out["body"] = statement(node->getSubStmt());
    } else if (isa<BreakStmt>(stmt) || isa<ContinueStmt>(stmt) ||
               isa<NullStmt>(stmt)) {
      /* The kind and location fully describe these statements. */
    } else {
      llvm::json::Array children;
      for (const Stmt *child : stmt->children())
        children.push_back(statement(child));
      out["children"] = std::move(children);
      out["unsupported"] = true;
    }
    return out;
  }

  llvm::json::Object variable(const VarDecl *var) {
    llvm::json::Object item;
    item["id"] = declaration_id(var);
    item["name"] = var->getNameAsString();
    item["type"] = type_name(var->getType());
    item["canonical_type"] = type_name(var->getType().getCanonicalType());
    item["unqualified_type"] =
        type_name(var->getType().getLocalUnqualifiedType());
    item["top_level_const"] = var->getType().isConstQualified();
    item["static_local"] = var->isStaticLocal();
    if (var->hasInit())
      item["initializer"] = expression(var->getInit());
    return item;
  }

  llvm::json::Object function(const FunctionDecl *decl) {
    llvm::json::Object out;
    llvm::json::Array parameters;
    out["id"] = declaration_id(decl);
    out["name"] = decl->getNameAsString();
    out["qualified_name"] = decl->getQualifiedNameAsString();
    out["namespace"] = declaration_namespace(decl);
    out["return_type"] = type_name(decl->getReturnType());
    out["variadic"] = decl->isVariadic();
    out["definition"] = decl->isThisDeclarationADefinition();
    out["dependent"] = decl->isDependentContext();
    out["location"] = location(decl->getLocation());
    for (const ParmVarDecl *parameter : decl->parameters()) {
      llvm::json::Object item;
      item["id"] = declaration_id(parameter);
      item["name"] = parameter->getNameAsString();
      item["type"] = type_name(parameter->getType());
      item["canonical_type"] =
          type_name(parameter->getType().getCanonicalType());
      if (parameter->hasDefaultArg())
        item["default"] = expression(parameter->getDefaultArg());
      parameters.push_back(std::move(item));
    }
    out["parameters"] = std::move(parameters);

    if (const CXXMethodDecl *method = dyn_cast<CXXMethodDecl>(decl)) {
      out["method"] = true;
      out["parent"] = declaration_id(method->getParent());
      out["const"] = method->isConst();
      out["static"] = method->isStatic();
    }
    if (isa<CXXConversionDecl>(decl))
      out["conversion"] = true;
    if (const CXXConstructorDecl *ctor = dyn_cast<CXXConstructorDecl>(decl)) {
      llvm::json::Array initializers;
      out["constructor"] = true;
      if (decl->isThisDeclarationADefinition() && !DeclarationsOnly)
        for (const CXXCtorInitializer *init : ctor->inits()) {
          llvm::json::Object item;
          if (init->isMemberInitializer()) {
            item["target"] = declaration_id(init->getMember());
            item["name"] = init->getMember()->getNameAsString();
          } else if (init->isBaseInitializer()) {
            item["name"] = type_name(init->getTypeSourceInfo()->getType());
            item["base"] = true;
          }
          item["value"] = expression(init->getInit());
          initializers.push_back(std::move(item));
        }
      out["initializers"] = std::move(initializers);
    }
    if (isa<CXXDestructorDecl>(decl))
      out["destructor"] = true;
    if (decl->isThisDeclarationADefinition() && !DeclarationsOnly)
      out["body"] = statement(decl->getBody());
    return out;
  }

  llvm::json::Object record(const CXXRecordDecl *decl) {
    llvm::json::Object out;
    llvm::json::Array fields;
    llvm::json::Array methods;
    llvm::json::Array bases;
    out["id"] = declaration_id(decl);
    out["name"] = decl->getNameAsString();
    if (isa<ClassTemplateSpecializationDecl>(decl) || decl->getName().empty()) {
      std::string printed = type_name(context_.getTypeDeclType(
          ElaboratedTypeKeyword::None, std::nullopt, decl));
      if (decl->getName().empty())
        if (const auto *parent = dyn_cast<NamedDecl>(decl->getDeclContext()))
          if (!parent->getQualifiedNameAsString().empty())
            printed = parent->getQualifiedNameAsString() + "::" + printed;
      out["qualified_name"] = printed;
    }
    else
      out["qualified_name"] = decl->getQualifiedNameAsString();
    out["namespace"] = declaration_namespace(decl);
    out["definition"] = decl->isThisDeclarationADefinition();
    out["dependent"] = decl->isDependentContext();
    out["union"] = decl->isUnion();
    out["location"] = location(decl->getLocation());
    if (decl->isThisDeclarationADefinition()) {
      const bool concrete = !decl->isDependentContext() &&
                            decl->getDescribedClassTemplate() == nullptr;
      const ASTRecordLayout *layout =
          concrete ? &context_.getASTRecordLayout(decl) : nullptr;
      if (layout) {
        out["size_bits"] =
            static_cast<int64_t>(layout->getSize().getQuantity() * 8);
        out["align_bits"] =
            static_cast<int64_t>(layout->getAlignment().getQuantity() * 8);
      }
      unsigned index = 0;
      for (const CXXBaseSpecifier &base : decl->bases()) {
        llvm::json::Object item;
        item["type"] = type_name(base.getType());
        item["virtual"] = base.isVirtual();
        if (const CXXRecordDecl *base_decl = base.getType()->getAsCXXRecordDecl())
          item["record"] = declaration_id(base_decl);
        bases.push_back(std::move(item));
      }
      for (const FieldDecl *field : decl->fields()) {
        llvm::json::Object item;
        item["id"] = declaration_id(field);
        item["name"] = field->getNameAsString();
        item["type"] = type_name(field->getType());
        QualType dependency_type = field->getType();
        if (const ArrayType *array = context_.getAsArrayType(dependency_type))
          dependency_type = array->getElementType();
        if (const CXXRecordDecl *dependency =
                dependency_type->getAsCXXRecordDecl())
          item["record_dependency"] = declaration_id(dependency);
        if (field->isBitField()) {
          item["bit_width"] =
              static_cast<int64_t>(field->getBitWidthValue());
        }
        if (field->hasInClassInitializer())
          item["default"] = expression(field->getInClassInitializer());
        if (layout)
          item["offset_bits"] =
              static_cast<int64_t>(layout->getFieldOffset(index));
        ++index;
        fields.push_back(std::move(item));
      }
      for (const CXXMethodDecl *method : decl->methods())
        methods.push_back(declaration_id(method));
    }
    out["fields"] = std::move(fields);
    out["bases"] = std::move(bases);
    out["methods"] = std::move(methods);
    return out;
  }

private:
  ASTContext &context_;
  SourceManager &source_;
  PrintingPolicy policy_;
};

class ExtractVisitor : public RecursiveASTVisitor<ExtractVisitor> {
public:
  explicit ExtractVisitor(ASTContext &context)
      : context_(context), source_(context.getSourceManager()),
        writer_(context) {}

  bool shouldVisitTemplateInstantiations() const { return true; }

  bool VisitCXXRecordDecl(CXXRecordDecl *decl) {
    if (interestingRecord(decl) && !decl->isInjectedClassName() &&
        (!decl->getPreviousDecl() || decl->isThisDeclarationADefinition())) {
      if (TraceDeclarations)
        llvm::errs() << "record " << decl->getQualifiedNameAsString() << "\n";
      records_.push_back(writer_.record(decl));
    }
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *decl) {
    if (interestingFunction(decl) &&
        (!decl->getPreviousDecl() || decl->isThisDeclarationADefinition())) {
      if (TraceDeclarations)
        llvm::errs() << "function " << decl->getQualifiedNameAsString() << "\n";
      functions_.push_back(writer_.function(decl));
    }
    return true;
  }

  bool VisitEnumDecl(EnumDecl *decl) {
    if (!interesting(decl) || !decl->isThisDeclarationADefinition())
      return true;
    llvm::json::Object out;
    llvm::json::Array constants;
    out["id"] = declaration_id(decl);
    out["name"] = decl->getNameAsString();
    out["qualified_name"] = decl->getQualifiedNameAsString();
    out["namespace"] = declaration_namespace(decl);
    out["location"] = writer_.location(decl->getLocation());
    for (const EnumConstantDecl *constant : decl->enumerators()) {
      llvm::SmallString<64> value;
      constant->getInitVal().toString(value, 10);
      llvm::json::Object item;
      item["id"] = declaration_id(constant);
      item["name"] = constant->getNameAsString();
      item["value"] = std::string(value);
      constants.push_back(std::move(item));
    }
    out["constants"] = std::move(constants);
    enums_.push_back(std::move(out));
    return true;
  }

  bool VisitTypedefNameDecl(TypedefNameDecl *decl) {
    if (!interesting(decl))
      return true;
    llvm::json::Object out;
    out["id"] = declaration_id(decl);
    out["name"] = decl->getNameAsString();
    out["qualified_name"] = decl->getQualifiedNameAsString();
    out["underlying_type"] = writer_.type_name(decl->getUnderlyingType());
    out["canonical_underlying_type"] =
        writer_.type_name(decl->getUnderlyingType().getCanonicalType());
    out["dependent"] = decl->getDeclContext()->isDependentContext();
    out["location"] = writer_.location(decl->getLocation());
    typedefs_.push_back(std::move(out));
    return true;
  }

  bool VisitVarDecl(VarDecl *decl) {
    if (!interesting(decl) || !decl->hasGlobalStorage() ||
        !decl->getLexicalDeclContext()->isFileContext())
      return true;
    llvm::json::Object out;
    out["id"] = declaration_id(decl);
    out["name"] = decl->getNameAsString();
    out["qualified_name"] = decl->getQualifiedNameAsString();
    out["type"] = writer_.type_name(decl->getType());
    out["definition"] =
        decl->isThisDeclarationADefinition() != VarDecl::DeclarationOnly;
    out["static"] = decl->getStorageClass() == SC_Static;
    out["external_linkage"] = decl->getFormalLinkage() == Linkage::External;
    out["location"] = writer_.location(decl->getLocation());
    if (decl->hasInit())
      out["initializer"] = writer_.expression(decl->getInit());
    globals_.push_back(std::move(out));
    return true;
  }

  llvm::json::Object finish() {
    llvm::json::Object root;
    root["schema_version"] = int64_t(1);
    root["clang_version"] = getClangFullVersion();
    root["records"] = std::move(records_);
    root["enums"] = std::move(enums_);
    root["functions"] = std::move(functions_);
    root["typedefs"] = std::move(typedefs_);
    root["globals"] = std::move(globals_);
    return root;
  }

private:
  bool interesting(const Decl *decl) const {
    return !decl->isImplicit() &&
           location_in_scope(source_, decl->getLocation());
  }

  bool interestingRecord(const CXXRecordDecl *decl) const {
    if (!location_in_scope(source_, decl->getLocation()))
      return false;
    if (!decl->isImplicit())
      return true;
    return isa<ClassTemplateSpecializationDecl>(decl);
  }

  bool interestingFunction(const FunctionDecl *decl) const {
    if (!location_in_scope(source_, decl->getLocation()))
      return false;
    if (!decl->isImplicit())
      return true;
    if (!decl->doesThisDeclarationHaveABody())
      return false;
    if (decl->getTemplateSpecializationKind() != TSK_Undeclared)
      return true;
    if (const auto *method = dyn_cast<CXXMethodDecl>(decl))
      return isa<ClassTemplateSpecializationDecl>(method->getParent());
    return false;
  }

  ASTContext &context_;
  SourceManager &source_;
  IrWriter writer_;
  llvm::json::Array records_;
  llvm::json::Array enums_;
  llvm::json::Array functions_;
  llvm::json::Array typedefs_;
  llvm::json::Array globals_;
};

class ExtractConsumer : public ASTConsumer {
public:
  explicit ExtractConsumer(ASTContext &context) : visitor_(context) {}

  void HandleTranslationUnit(ASTContext &context) override {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
    root_ = visitor_.finish();
  }

  llvm::json::Object takeRoot() { return std::move(root_); }

private:
  ExtractVisitor visitor_;
  llvm::json::Object root_;
};

class ExtractAction : public ASTFrontendAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &compiler,
                                                 llvm::StringRef) override {
    compiler.getPreprocessor().addPPCallbacks(
        std::make_unique<MacroCapture>(compiler.getPreprocessor(), macros_));
    consumer_ = new ExtractConsumer(compiler.getASTContext());
    return std::unique_ptr<ASTConsumer>(consumer_);
  }

  void EndSourceFileAction() override {
    llvm::json::Value value(consumer_->takeRoot());
    value.getAsObject()->insert({"macros", std::move(macros_)});
    std::error_code error;
    llvm::raw_fd_ostream output(OutputPath, error, llvm::sys::fs::OF_Text);
    if (error) {
      llvm::errs() << "cannot open " << OutputPath << ": " << error.message()
                   << "\n";
      return;
    }
    output << llvm::formatv("{0:2}\n", value);
  }

private:
  ExtractConsumer *consumer_ = nullptr;
  llvm::json::Array macros_;
};

} // namespace

int main(int argc, const char **argv) {
  auto parser = CommonOptionsParser::create(argc, argv, ToolCategory);
  if (!parser) {
    llvm::errs() << parser.takeError();
    return 2;
  }
  ClangTool tool(parser->getCompilations(), parser->getSourcePathList());
  return tool.run(newFrontendActionFactory<ExtractAction>().get());
}

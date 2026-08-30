/* Generated private declarations for translated C units. */
#ifndef IMGUI_C89_INTERNAL_H
#define IMGUI_C89_INTERNAL_H
#include "imgui_c89.h"

void imgui_c89_debugtrap(void);
const char *imgui_c89_compressed_string_at(const unsigned char *data, const unsigned char *rules, unsigned int rule_count, int index, char *buffer);
void imgui_c89_assert_rtn(const char *function_name, const char *file_name, int line_number, const char *message);

void *imgui_c89_vector_reserve(void *context, void *data, int size, int *capacity, int new_capacity, size_t element_size, int discard);
void *imgui_c89_vector_resize(void *context, void *data, int *size, int *capacity, int new_size, size_t element_size);
void *imgui_c89_vector_resize_fill(void *context, void *data, int *size, int *capacity, int new_size, size_t element_size, const void *value);
void *imgui_c89_vector_push_back(void *context, void *data, int *size, int *capacity, size_t element_size, const void *value);
void *imgui_c89_vector_erase(void *data, int *size, size_t element_size, const void *first, const void *last);
void *imgui_c89_vector_erase_unsorted(void *data, int *size, size_t element_size, const void *item);
void *imgui_c89_vector_insert(void *context, void **data, int *size, int *capacity, size_t element_size, const void *item, const void *value);

void *imgui_c89_vector_at(void *data, int size, int index, size_t element_size, int line);
void *imgui_c89_vector_back(void *data, int size, size_t element_size, int line);
void imgui_c89_vector_pop(int *size);
int imgui_c89_vector_index(const void *data, int size, const void *item, size_t element_size);

void imgui_c89_vector_destroy(void *context, void *data);
void imgui_c89_vector_clear(void *context, void **data, int *size, int *capacity);

int imgui_c89_vector_grow_capacity(int capacity, int size);

void *imgui_c89_chunk_alloc(void *context, void *stream, size_t size);
void *imgui_c89_chunk_begin(void *stream);
int imgui_c89_chunk_size(const void *chunk);
void imgui_c89_chunk_clear(void *context, void *stream);
int imgui_c89_chunk_empty(const void *stream);
void *imgui_c89_chunk_end(void *stream);
void *imgui_c89_chunk_next(void *stream, const void *chunk);
int imgui_c89_chunk_offset(const void *stream, const void *chunk);
void *imgui_c89_chunk_ptr(void *stream, int offset);
int imgui_c89_chunk_size_bytes(const void *stream);
void imgui_c89_chunk_swap(void *stream, void *other);

void *imgui_c89_pool_add_slot(void *context, void **data, int *size, int *capacity, int *free_index, int *alive_count, size_t element_size);
void *imgui_c89_pool_at(void *data, int index, size_t element_size);
int imgui_c89_pool_contains(const void *data, int size, const void *item, size_t element_size);
int imgui_c89_pool_index(const void *data, int size, const void *item, size_t element_size);

typedef struct Func Func;
typedef struct Funcs Funcs;
typedef struct ImBitArray_155_512 ImBitArray_155_512;
typedef struct ImBitArray_155 ImBitArray_155;
typedef struct ImBitVector ImBitVector;
typedef struct ImChunkStream_ImGuiTableSettings ImChunkStream_ImGuiTableSettings;
typedef struct ImChunkStream_ImGuiWindowSettings ImChunkStream_ImGuiWindowSettings;
typedef struct ImDrawDataBuilder ImDrawDataBuilder;
typedef struct imgui_c89_anon_imgui_draw_5418_5 imgui_c89_anon_imgui_draw_5418_5;
typedef struct ImFontAtlasPostProcessData ImFontAtlasPostProcessData;
typedef struct ImFontAtlasRectEntry ImFontAtlasRectEntry;
typedef struct ImFontStackData ImFontStackData;
typedef struct ImGuiBoxSelectState ImGuiBoxSelectState;
typedef struct ImGuiColorMod ImGuiColorMod;
typedef struct ImGuiComboPreviewData ImGuiComboPreviewData;
typedef struct ImGuiContextHook ImGuiContextHook;
typedef struct ImGuiDataTypeInfo ImGuiDataTypeInfo;
typedef struct ImGuiDataTypeStorage ImGuiDataTypeStorage;
typedef struct ImGuiDeactivatedItemData ImGuiDeactivatedItemData;
typedef struct ImGuiDebugAllocEntry ImGuiDebugAllocEntry;
typedef struct ImGuiDebugAllocInfo ImGuiDebugAllocInfo;
typedef struct ImGuiDebugItemPathQuery ImGuiDebugItemPathQuery;
typedef struct ImGuiErrorRecoveryState ImGuiErrorRecoveryState;
typedef struct ImGuiFocusScopeData ImGuiFocusScopeData;
typedef struct ImGuiGroupData ImGuiGroupData;
typedef struct ImGuiIDStackTool ImGuiIDStackTool;
typedef struct ImGuiInputEvent ImGuiInputEvent;
typedef union imgui_c89_anon_imgui_internal_1585_5 imgui_c89_anon_imgui_internal_1585_5;
typedef struct ImGuiInputEventAppFocused ImGuiInputEventAppFocused;
typedef struct ImGuiInputEventKey ImGuiInputEventKey;
typedef struct ImGuiInputEventMouseButton ImGuiInputEventMouseButton;
typedef struct ImGuiInputEventMousePos ImGuiInputEventMousePos;
typedef struct ImGuiInputEventMouseWheel ImGuiInputEventMouseWheel;
typedef struct ImGuiInputEventText ImGuiInputEventText;
typedef struct ImGuiInputTextDeactivatedState ImGuiInputTextDeactivatedState;
typedef struct ImGuiInputTextState ImGuiInputTextState;
typedef struct ImGuiKeyOwnerData ImGuiKeyOwnerData;
typedef struct ImGuiKeyRoutingData ImGuiKeyRoutingData;
typedef struct ImGuiKeyRoutingTable ImGuiKeyRoutingTable;
typedef struct ImGuiLastItemData ImGuiLastItemData;
typedef struct ImGuiListClipperData ImGuiListClipperData;
typedef struct ImGuiListClipperRange ImGuiListClipperRange;
typedef struct ImGuiLocEntry ImGuiLocEntry;
typedef struct ImGuiMenuColumns ImGuiMenuColumns;
typedef struct ImGuiMetricsConfig ImGuiMetricsConfig;
typedef struct ImGuiMultiSelectState ImGuiMultiSelectState;
typedef struct ImGuiMultiSelectTempData ImGuiMultiSelectTempData;
typedef struct ImGuiNavItemData ImGuiNavItemData;
typedef struct ImGuiNextItemData ImGuiNextItemData;
typedef struct ImGuiNextWindowData ImGuiNextWindowData;
typedef struct ImGuiOldColumnData ImGuiOldColumnData;
typedef struct ImGuiOldColumns ImGuiOldColumns;
typedef struct ImGuiPackedDate ImGuiPackedDate;
typedef struct ImGuiPlotArrayGetterData ImGuiPlotArrayGetterData;
typedef struct ImGuiPopupData ImGuiPopupData;
typedef struct ImGuiPtrOrIndex ImGuiPtrOrIndex;
typedef struct ImGuiResizeBorderDef ImGuiResizeBorderDef;
typedef struct ImGuiResizeGripDef ImGuiResizeGripDef;
typedef struct ImGuiSettingsCleanupArgs ImGuiSettingsCleanupArgs;
typedef struct ImGuiSettingsHandler ImGuiSettingsHandler;
typedef struct ImGuiShrinkWidthItem ImGuiShrinkWidthItem;
typedef struct ImGuiStackLevelInfo ImGuiStackLevelInfo;
typedef struct ImGuiStyleMod ImGuiStyleMod;
typedef union imgui_c89_anon_imgui_internal_955_5 imgui_c89_anon_imgui_internal_955_5;
typedef struct ImGuiStyleVarInfo ImGuiStyleVarInfo;
typedef struct ImGuiTabBar ImGuiTabBar;
typedef struct ImGuiTabBarSection ImGuiTabBarSection;
typedef struct ImGuiTabItem ImGuiTabItem;
typedef struct ImGuiTable ImGuiTable;
typedef struct ImGuiTableCellData ImGuiTableCellData;
typedef struct ImGuiTableColumn ImGuiTableColumn;
typedef struct ImGuiTableColumnSettings ImGuiTableColumnSettings;
typedef struct ImGuiTableFixDisplayOrderColumnData ImGuiTableFixDisplayOrderColumnData;
typedef struct ImGuiTableHeaderData ImGuiTableHeaderData;
typedef struct ImGuiTableInstanceData ImGuiTableInstanceData;
typedef struct ImGuiTableReconcileColumnData ImGuiTableReconcileColumnData;
typedef struct ImGuiTableSettings ImGuiTableSettings;
typedef struct ImGuiTableTempData ImGuiTableTempData;
typedef struct ImGuiTextIndex ImGuiTextIndex;
typedef struct ImGuiTreeNodeStackData ImGuiTreeNodeStackData;
typedef struct ImGuiTypingSelectRequest ImGuiTypingSelectRequest;
typedef struct ImGuiTypingSelectState ImGuiTypingSelectState;
typedef struct ImGuiViewportP ImGuiViewportP;
typedef struct ImGuiWindow ImGuiWindow;
typedef struct ImGuiWindowSettings ImGuiWindowSettings;
typedef struct ImGuiWindowStackData ImGuiWindowStackData;
typedef struct ImGuiWindowTempData ImGuiWindowTempData;
typedef struct ImGui_ImplStbTrueType_FontSrcData ImGui_ImplStbTrueType_FontSrcData;
typedef struct ImPool_ImGuiMultiSelectState ImPool_ImGuiMultiSelectState;
typedef struct ImPool_ImGuiTabBar ImPool_ImGuiTabBar;
typedef struct ImGuiTablePool ImGuiTablePool;
typedef struct ImRect ImRect;
typedef struct ImStableVector_ImFontBaked_32 ImStableVector_ImFontBaked_32;
typedef struct ImStb_STB_TexteditState ImStb_STB_TexteditState;
typedef struct imgui_c89_anon_imstb_textedit_553_9 imgui_c89_anon_imstb_textedit_553_9;
typedef struct imgui_c89_anon_imstb_textedit_389_9 imgui_c89_anon_imstb_textedit_389_9;
typedef struct imgui_c89_anon_imstb_textedit_324_9 imgui_c89_anon_imstb_textedit_324_9;
typedef struct imgui_c89_anon_imstb_textedit_333_9 imgui_c89_anon_imstb_textedit_333_9;
typedef struct ImTriangulator ImTriangulator;
typedef struct ImTriangulatorNode ImTriangulatorNode;
typedef struct ImTriangulatorNodeSpan ImTriangulatorNodeSpan;
typedef struct ImVec1 ImVec1;
typedef struct ImVec2i ImVec2i;
typedef struct ImVec2ih ImVec2ih;
typedef struct ImVector_ImFontAtlas_ptr ImVector_ImFontAtlas_ptr;
typedef struct ImVector_ImFontAtlasRectEntry ImVector_ImFontAtlasRectEntry;
typedef struct ImVector_ImFontBaked_ptr ImVector_ImFontBaked_ptr;
typedef struct ImVector_ImFontStackData ImVector_ImFontStackData;
typedef struct ImVector_ImGuiColorMod ImVector_ImGuiColorMod;
typedef struct ImVector_ImGuiContextHook ImVector_ImGuiContextHook;
typedef struct ImVector_ImGuiFocusScopeData ImVector_ImGuiFocusScopeData;
typedef struct ImVector_ImGuiGroupData ImVector_ImGuiGroupData;
typedef struct ImVector_ImGuiInputEvent ImVector_ImGuiInputEvent;
typedef struct ImVector_ImGuiKeyRoutingData ImVector_ImGuiKeyRoutingData;
typedef struct ImVector_ImGuiListClipperData ImVector_ImGuiListClipperData;
typedef struct ImVector_ImGuiListClipperRange ImVector_ImGuiListClipperRange;
typedef struct ImVector_ImGuiMultiSelectState ImVector_ImGuiMultiSelectState;
typedef struct ImVector_ImGuiMultiSelectTempData ImVector_ImGuiMultiSelectTempData;
typedef struct ImVector_ImGuiOldColumnData ImVector_ImGuiOldColumnData;
typedef struct ImVector_ImGuiOldColumns ImVector_ImGuiOldColumns;
typedef struct ImVector_ImGuiPopupData ImVector_ImGuiPopupData;
typedef struct ImVector_ImGuiPtrOrIndex ImVector_ImGuiPtrOrIndex;
typedef struct ImVector_ImGuiSettingsHandler ImVector_ImGuiSettingsHandler;
typedef struct ImVector_ImGuiShrinkWidthItem ImVector_ImGuiShrinkWidthItem;
typedef struct ImVector_ImGuiStackLevelInfo ImVector_ImGuiStackLevelInfo;
typedef struct ImVector_ImGuiStyleMod ImVector_ImGuiStyleMod;
typedef struct ImVector_ImGuiTabBar ImVector_ImGuiTabBar;
typedef struct ImVector_ImGuiTabItem ImVector_ImGuiTabItem;
typedef struct ImVector_ImGuiTreeNodeStackData ImVector_ImGuiTreeNodeStackData;
typedef struct ImVector_ImGuiViewportP_ptr ImVector_ImGuiViewportP_ptr;
typedef struct ImVector_ImGuiWindow_ptr ImVector_ImGuiWindow_ptr;
typedef struct ImVector_ImGuiWindowStackData ImVector_ImGuiWindowStackData;
typedef struct ImVector_int ImVector_int;
typedef struct ImVector_stbrp_node ImVector_stbrp_node;
typedef struct KeyLayoutData KeyLayoutData;
typedef struct MergeGroup MergeGroup;
typedef struct ScopedHighlightOlderThan ScopedHighlightOlderThan;
typedef struct imgui_c89_anon_imstb_rectpack_341_9 imgui_c89_anon_imstb_rectpack_341_9;
typedef struct stbrp_context stbrp_context;
typedef struct stbrp_context_opaque stbrp_context_opaque;
typedef struct stbrp_node stbrp_node;
typedef struct stbrp_rect stbrp_rect;
typedef struct stbtt_active_edge stbtt_active_edge;
typedef struct imgui_c89_anon_imstb_truetype_929_9 imgui_c89_anon_imstb_truetype_929_9;
typedef struct imgui_c89_anon_imstb_truetype_518_9 imgui_c89_anon_imstb_truetype_518_9;
typedef struct imgui_c89_anon_imstb_truetype_1902_9 imgui_c89_anon_imstb_truetype_1902_9;
typedef struct stbtt_edge stbtt_edge;
typedef struct stbtt_hheap stbtt_hheap;
typedef struct stbtt_hheap_chunk stbtt_hheap_chunk;
typedef struct imgui_c89_anon_imstb_truetype_3493_9 imgui_c89_anon_imstb_truetype_3493_9;
typedef struct imgui_c89_anon_imstb_truetype_548_9 imgui_c89_anon_imstb_truetype_548_9;
typedef struct imgui_c89_anon_imstb_truetype_532_9 imgui_c89_anon_imstb_truetype_532_9;
typedef struct stbtt_fontinfo stbtt_fontinfo;
typedef struct stbtt_kerningentry stbtt_kerningentry;
typedef struct stbtt_pack_context stbtt_pack_context;
typedef struct imgui_c89_anon_imstb_truetype_624_9 imgui_c89_anon_imstb_truetype_624_9;
typedef struct imgui_c89_anon_imstb_truetype_580_9 imgui_c89_anon_imstb_truetype_580_9;
typedef struct imgui_c89_anon_imstb_truetype_840_12 imgui_c89_anon_imstb_truetype_840_12;

typedef int ImGuiLayoutType;
typedef int ImGuiActivateFlags;
typedef int ImGuiDebugLogFlags;
typedef int ImGuiFocusRequestFlags;
typedef int ImGuiItemStatusFlags;
typedef int ImGuiOldColumnFlags;
typedef int ImGuiLogFlags;
typedef int ImGuiNavRenderCursorFlags;
typedef int ImGuiNavMoveFlags;
typedef int ImGuiNextItemDataFlags;
typedef int ImGuiNextWindowDataFlags;
typedef int ImGuiScrollFlags;
typedef int ImGuiSeparatorFlags;
typedef int ImGuiTextFlags;
typedef int ImGuiTooltipFlags;
typedef int ImGuiTypingSelectFlags;
typedef int ImGuiWindowBgClickFlags;
typedef int ImGuiWindowRefreshFlags;
typedef ImS16 ImGuiTableColumnIdx;
typedef ImU16 ImGuiTableDrawChannelIdx;
typedef FILE * ImFileHandle;
typedef ImU32 * ImBitArrayPtr;
typedef int ImPoolIdx;
typedef ImBitArray_155_512 ImBitArrayForNamedKeys;
typedef ImS16 ImGuiKeyRoutingIndex;
typedef void (*ImGuiErrorCallback)(ImGuiContext *, void *, const char *);
typedef void (*ImGuiContextHookCallback)(ImGuiContext *, ImGuiContextHook *);
typedef void (*ImGuiDemoMarkerCallback)(const char *, int, const char *);
typedef ImFontLoader ImFontBuilderIO;
typedef stbrp_node stbrp_node_im;
typedef int stbrp_coord;
typedef struct imgui_c89_anon_imstb_rectpack_341_9 stbrp_findresult;
typedef struct imgui_c89_anon_imstb_textedit_324_9 ImStb_StbUndoRecord;
typedef struct imgui_c89_anon_imstb_textedit_333_9 ImStb_StbUndoState;
typedef ImStb_STB_TexteditState ImStbTexteditState;
typedef struct imgui_c89_anon_imstb_textedit_389_9 ImStb_StbTexteditRow;
typedef struct imgui_c89_anon_imstb_textedit_553_9 ImStb_StbFindState;
typedef unsigned char stbtt_uint8;
typedef signed char stbtt_int8;
typedef unsigned short stbtt_uint16;
typedef short stbtt_int16;
typedef unsigned int stbtt_uint32;
typedef int stbtt_int32;
typedef char stbtt_check_size32[1];
typedef char stbtt_check_size16[1];
typedef struct imgui_c89_anon_imstb_truetype_518_9 stbtt_buf;
typedef struct imgui_c89_anon_imstb_truetype_532_9 stbtt_bakedchar;
typedef struct imgui_c89_anon_imstb_truetype_548_9 stbtt_aligned_quad;
typedef struct imgui_c89_anon_imstb_truetype_580_9 stbtt_packedchar;
typedef struct imgui_c89_anon_imstb_truetype_624_9 stbtt_pack_range;
typedef struct imgui_c89_anon_imstb_truetype_840_12 stbtt_vertex;
typedef struct imgui_c89_anon_imstb_truetype_929_9 stbtt_bitmap;
typedef int stbtt_test_oversample_pow2[1];
typedef struct imgui_c89_anon_imstb_truetype_1902_9 stbtt_csctx;
typedef struct imgui_c89_anon_imstb_truetype_3493_9 stbtt_point;

typedef int ImGuiAxis;
typedef int ImGuiButtonFlagsPrivate;
typedef int ImGuiComboFlagsPrivate;
typedef int ImGuiContextHookType;
typedef int ImGuiDataTypePrivate;
typedef int ImGuiHoveredFlagsPrivate;
typedef int ImGuiInputEventType;
typedef int ImGuiInputFlagsPrivate;
typedef int ImGuiInputSource;
typedef int ImGuiInputTextFlagsPrivate;
typedef int ImGuiItemFlagsPrivate;
typedef int ImGuiLocKey;
typedef int ImGuiNavLayer;
typedef int ImGuiPlotType;
typedef int ImGuiPopupPositionPolicy;
typedef int ImGuiSelectableFlagsPrivate;
typedef int ImGuiSliderFlagsPrivate;
typedef int ImGuiTabBarFlagsPrivate;
typedef int ImGuiTabItemFlagsPrivate;
typedef int ImGuiTreeNodeFlagsPrivate;
typedef int ImTriangulatorNodeType;
typedef int ImWcharClass;

/* (unnamed enum at /Users/johnk/.codex/.chatgpt-projects/g-p-6a6fdfd120a48191a13b5591a2528fa9/build/upstream/f1cc2ae15e53a861a874c3034aae6798fde194ab/imgui.cpp:16844:5) */
enum {
    WRT_OuterRect = 0,
    WRT_OuterRectClipped = 1,
    WRT_InnerRect = 2,
    WRT_InnerClipRect = 3,
    WRT_WorkRect = 4,
    WRT_Content = 5,
    WRT_ContentIdeal = 6,
    WRT_ContentRegionRect = 7,
    WRT_Count = 8
};

/* (unnamed enum at /Users/johnk/.codex/.chatgpt-projects/g-p-6a6fdfd120a48191a13b5591a2528fa9/build/upstream/f1cc2ae15e53a861a874c3034aae6798fde194ab/imgui.cpp:16846:5) */
enum {
    TRT_OuterRect = 0,
    TRT_InnerRect = 1,
    TRT_WorkRect = 2,
    TRT_HostClipRect = 3,
    TRT_InnerClipRect = 4,
    TRT_BackgroundClipRect = 5,
    TRT_ColumnsRect = 6,
    TRT_ColumnsWorkRect = 7,
    TRT_ColumnsClipRect = 8,
    TRT_ColumnsContentHeadersUsed = 9,
    TRT_ColumnsContentHeadersIdeal = 10,
    TRT_ColumnsContentFrozen = 11,
    TRT_ColumnsContentUnfrozen = 12,
    TRT_Count = 13
};

/* (unnamed enum at /Users/johnk/.codex/.chatgpt-projects/g-p-6a6fdfd120a48191a13b5591a2528fa9/build/upstream/f1cc2ae15e53a861a874c3034aae6798fde194ab/imstb_rectpack.h:166:1) */
enum {
    STBRP_HEURISTIC_Skyline_default = 0,
    STBRP_HEURISTIC_Skyline_BL_sortHeight = 0,
    STBRP_HEURISTIC_Skyline_BF_sortHeight = 1
};

/* (unnamed enum at /Users/johnk/.codex/.chatgpt-projects/g-p-6a6fdfd120a48191a13b5591a2528fa9/build/upstream/f1cc2ae15e53a861a874c3034aae6798fde194ab/imstb_rectpack.h:228:1) */
enum {
    STBRP__INIT_skyline = 1
};

/* (unnamed enum at /Users/johnk/.codex/.chatgpt-projects/g-p-6a6fdfd120a48191a13b5591a2528fa9/build/upstream/f1cc2ae15e53a861a874c3034aae6798fde194ab/imstb_truetype.h:1049:1) */
enum {
    STBTT_PLATFORM_ID_UNICODE = 0,
    STBTT_PLATFORM_ID_MAC = 1,
    STBTT_PLATFORM_ID_ISO = 2,
    STBTT_PLATFORM_ID_MICROSOFT = 3
};

/* (unnamed enum at /Users/johnk/.codex/.chatgpt-projects/g-p-6a6fdfd120a48191a13b5591a2528fa9/build/upstream/f1cc2ae15e53a861a874c3034aae6798fde194ab/imstb_truetype.h:1056:1) */
enum {
    STBTT_UNICODE_EID_UNICODE_1_0 = 0,
    STBTT_UNICODE_EID_UNICODE_1_1 = 1,
    STBTT_UNICODE_EID_ISO_10646 = 2,
    STBTT_UNICODE_EID_UNICODE_2_0_BMP = 3,
    STBTT_UNICODE_EID_UNICODE_2_0_FULL = 4
};

/* (unnamed enum at /Users/johnk/.codex/.chatgpt-projects/g-p-6a6fdfd120a48191a13b5591a2528fa9/build/upstream/f1cc2ae15e53a861a874c3034aae6798fde194ab/imstb_truetype.h:1064:1) */
enum {
    STBTT_MS_EID_SYMBOL = 0,
    STBTT_MS_EID_UNICODE_BMP = 1,
    STBTT_MS_EID_SHIFTJIS = 2,
    STBTT_MS_EID_UNICODE_FULL = 10
};

/* (unnamed enum at /Users/johnk/.codex/.chatgpt-projects/g-p-6a6fdfd120a48191a13b5591a2528fa9/build/upstream/f1cc2ae15e53a861a874c3034aae6798fde194ab/imstb_truetype.h:1071:1) */
enum {
    STBTT_MAC_EID_ROMAN = 0,
    STBTT_MAC_EID_ARABIC = 4,
    STBTT_MAC_EID_JAPANESE = 1,
    STBTT_MAC_EID_HEBREW = 5,
    STBTT_MAC_EID_CHINESE_TRAD = 2,
    STBTT_MAC_EID_GREEK = 6,
    STBTT_MAC_EID_KOREAN = 3,
    STBTT_MAC_EID_RUSSIAN = 7
};

/* (unnamed enum at /Users/johnk/.codex/.chatgpt-projects/g-p-6a6fdfd120a48191a13b5591a2528fa9/build/upstream/f1cc2ae15e53a861a874c3034aae6798fde194ab/imstb_truetype.h:1078:1) */
enum {
    STBTT_MS_LANG_ENGLISH = 1033,
    STBTT_MS_LANG_ITALIAN = 1040,
    STBTT_MS_LANG_CHINESE = 2052,
    STBTT_MS_LANG_JAPANESE = 1041,
    STBTT_MS_LANG_DUTCH = 1043,
    STBTT_MS_LANG_KOREAN = 1042,
    STBTT_MS_LANG_FRENCH = 1036,
    STBTT_MS_LANG_RUSSIAN = 1049,
    STBTT_MS_LANG_GERMAN = 1031,
    STBTT_MS_LANG_SPANISH = 1033,
    STBTT_MS_LANG_HEBREW = 1037,
    STBTT_MS_LANG_SWEDISH = 1053
};

/* (unnamed enum at /Users/johnk/.codex/.chatgpt-projects/g-p-6a6fdfd120a48191a13b5591a2528fa9/build/upstream/f1cc2ae15e53a861a874c3034aae6798fde194ab/imstb_truetype.h:1088:1) */
enum {
    STBTT_MAC_LANG_ENGLISH = 0,
    STBTT_MAC_LANG_JAPANESE = 11,
    STBTT_MAC_LANG_ARABIC = 12,
    STBTT_MAC_LANG_KOREAN = 23,
    STBTT_MAC_LANG_DUTCH = 4,
    STBTT_MAC_LANG_RUSSIAN = 32,
    STBTT_MAC_LANG_FRENCH = 1,
    STBTT_MAC_LANG_SPANISH = 6,
    STBTT_MAC_LANG_GERMAN = 2,
    STBTT_MAC_LANG_SWEDISH = 5,
    STBTT_MAC_LANG_HEBREW = 10,
    STBTT_MAC_LANG_CHINESE_SIMPLIFIED = 33,
    STBTT_MAC_LANG_ITALIAN = 3,
    STBTT_MAC_LANG_CHINESE_TRAD = 19
};

/* (unnamed enum at /Users/johnk/.codex/.chatgpt-projects/g-p-6a6fdfd120a48191a13b5591a2528fa9/build/upstream/f1cc2ae15e53a861a874c3034aae6798fde194ab/imstb_truetype.h:829:4) */
enum {
    STBTT_vmove = 1,
    STBTT_vline = 2,
    STBTT_vcurve = 3,
    STBTT_vcubic = 4
};

/* ImDrawTextFlags_ */
enum {
    ImDrawTextFlags_CpuFineClip = 1,
    ImDrawTextFlags_None = 0,
    ImDrawTextFlags_StopOnNewLine = 4,
    ImDrawTextFlags_WrapKeepBlanks = 2
};

/* ImGuiActivateFlags_ */
enum {
    ImGuiActivateFlags_FromFocusApi = 32,
    ImGuiActivateFlags_FromShortcut = 16,
    ImGuiActivateFlags_FromTabbing = 8,
    ImGuiActivateFlags_None = 0,
    ImGuiActivateFlags_PreferInput = 1,
    ImGuiActivateFlags_PreferTweak = 2,
    ImGuiActivateFlags_TryToPreserveState = 4
};

/* ImGuiAxis */
enum {
    ImGuiAxis_None = -1,
    ImGuiAxis_X = 0,
    ImGuiAxis_Y = 1
};

/* ImGuiButtonFlagsPrivate_ */
enum {
    ImGuiButtonFlags_AlignTextBaseLine = 32768,
    ImGuiButtonFlags_FlattenChildren = 2048,
    ImGuiButtonFlags_NoFocus = 4194304,
    ImGuiButtonFlags_NoHoldingActiveId = 131072,
    ImGuiButtonFlags_NoHoveredOnFocus = 524288,
    ImGuiButtonFlags_NoKeyModsAllowed = 65536,
    ImGuiButtonFlags_NoNavFocus = 262144,
    ImGuiButtonFlags_NoSetKeyOwner = 1048576,
    ImGuiButtonFlags_NoTestKeyOwner = 2097152,
    ImGuiButtonFlags_PressedOnClick = 16,
    ImGuiButtonFlags_PressedOnClickRelease = 32,
    ImGuiButtonFlags_PressedOnClickReleaseAnywhere = 64,
    ImGuiButtonFlags_PressedOnDefault_ = 32,
    ImGuiButtonFlags_PressedOnDoubleClick = 256,
    ImGuiButtonFlags_PressedOnDragDropHold = 512,
    ImGuiButtonFlags_PressedOnMask_ = 1008,
    ImGuiButtonFlags_PressedOnRelease = 128
};

/* ImGuiComboFlagsPrivate_ */
enum {
    ImGuiComboFlags_CustomPreview = 1048576
};

/* ImGuiContextHookType */
enum {
    ImGuiContextHookType_EndFramePost = 3,
    ImGuiContextHookType_EndFramePre = 2,
    ImGuiContextHookType_NewFramePost = 1,
    ImGuiContextHookType_NewFramePre = 0,
    ImGuiContextHookType_PendingRemoval_ = 7,
    ImGuiContextHookType_RenderPost = 5,
    ImGuiContextHookType_RenderPre = 4,
    ImGuiContextHookType_Shutdown = 6
};

/* ImGuiDataTypePrivate_ */
enum {
    ImGuiDataType_ID = 13,
    ImGuiDataType_Pointer = 12
};

/* ImGuiDebugLogFlags_ */
enum {
    ImGuiDebugLogFlags_EventActiveId = 2,
    ImGuiDebugLogFlags_EventClipper = 32,
    ImGuiDebugLogFlags_EventDocking = 1024,
    ImGuiDebugLogFlags_EventError = 1,
    ImGuiDebugLogFlags_EventFocus = 4,
    ImGuiDebugLogFlags_EventFont = 256,
    ImGuiDebugLogFlags_EventIO = 128,
    ImGuiDebugLogFlags_EventInputRouting = 512,
    ImGuiDebugLogFlags_EventMask_ = 8191,
    ImGuiDebugLogFlags_EventNav = 16,
    ImGuiDebugLogFlags_EventPopup = 8,
    ImGuiDebugLogFlags_EventSelection = 64,
    ImGuiDebugLogFlags_EventTable = 4096,
    ImGuiDebugLogFlags_EventViewport = 2048,
    ImGuiDebugLogFlags_None = 0,
    ImGuiDebugLogFlags_OutputToDebugger = 2097152,
    ImGuiDebugLogFlags_OutputToTTY = 1048576,
    ImGuiDebugLogFlags_OutputToTestEngine = 4194304
};

/* ImGuiFocusRequestFlags_ */
enum {
    ImGuiFocusRequestFlags_None = 0,
    ImGuiFocusRequestFlags_RestoreFocusedChild = 1,
    ImGuiFocusRequestFlags_UnlessBelowModal = 2
};

/* ImGuiHoveredFlagsPrivate_ */
enum {
    ImGuiHoveredFlags_AllowedMaskForIsItemHovered = 262048,
    ImGuiHoveredFlags_AllowedMaskForIsWindowHovered = 12463,
    ImGuiHoveredFlags_DelayMask_ = 245760
};

/* ImGuiInputEventType */
enum {
    ImGuiInputEventType_COUNT = 7,
    ImGuiInputEventType_Focus = 6,
    ImGuiInputEventType_Key = 4,
    ImGuiInputEventType_MouseButton = 3,
    ImGuiInputEventType_MousePos = 1,
    ImGuiInputEventType_MouseWheel = 2,
    ImGuiInputEventType_None = 0,
    ImGuiInputEventType_Text = 5
};

/* ImGuiInputFlagsPrivate_ */
enum {
    ImGuiInputFlags_CondActive = 8388608,
    ImGuiInputFlags_CondDefault_ = 12582912,
    ImGuiInputFlags_CondHovered = 4194304,
    ImGuiInputFlags_CondMask_ = 12582912,
    ImGuiInputFlags_LockThisFrame = 1048576,
    ImGuiInputFlags_LockUntilRelease = 2097152,
    ImGuiInputFlags_RepeatMask_ = 255,
    ImGuiInputFlags_RepeatRateDefault = 2,
    ImGuiInputFlags_RepeatRateMask_ = 14,
    ImGuiInputFlags_RepeatRateNavMove = 4,
    ImGuiInputFlags_RepeatRateNavTweak = 8,
    ImGuiInputFlags_RepeatUntilKeyModsChange = 32,
    ImGuiInputFlags_RepeatUntilKeyModsChangeFromNone = 64,
    ImGuiInputFlags_RepeatUntilMask_ = 240,
    ImGuiInputFlags_RepeatUntilOtherKeyPress = 128,
    ImGuiInputFlags_RepeatUntilRelease = 16,
    ImGuiInputFlags_RouteOptionsMask_ = 245760,
    ImGuiInputFlags_RouteTypeMask_ = 15360,
    ImGuiInputFlags_SupportedByIsKeyPressed = 255,
    ImGuiInputFlags_SupportedByIsMouseClicked = 1,
    ImGuiInputFlags_SupportedBySetItemKeyOwner = 15728640,
    ImGuiInputFlags_SupportedBySetKeyOwner = 3145728,
    ImGuiInputFlags_SupportedBySetNextItemShortcut = 523519,
    ImGuiInputFlags_SupportedByShortcut = 261375
};

/* ImGuiInputSource */
enum {
    ImGuiInputSource_COUNT = 4,
    ImGuiInputSource_Gamepad = 3,
    ImGuiInputSource_Keyboard = 2,
    ImGuiInputSource_Mouse = 1,
    ImGuiInputSource_None = 0
};

/* ImGuiInputTextFlagsPrivate_ */
enum {
    ImGuiInputTextFlags_LocalizeDecimalPoint = 268435456,
    ImGuiInputTextFlags_Multiline = 67108864,
    ImGuiInputTextFlags_TempInput = 134217728
};

/* ImGuiItemFlagsPrivate_ */
enum {
    ImGuiItemFlags_AllowOverlap = 16384,
    ImGuiItemFlags_Default_ = 144,
    ImGuiItemFlags_HasSelectionUserData = 2097152,
    ImGuiItemFlags_Inputable = 1048576,
    ImGuiItemFlags_IsMultiSelect = 4194304,
    ImGuiItemFlags_MixedValue = 4096,
    ImGuiItemFlags_NoFocus = 131072,
    ImGuiItemFlags_NoMarkEdited = 65536,
    ImGuiItemFlags_NoNavDisableMouseHover = 32768,
    ImGuiItemFlags_NoWindowHoverableCheck = 8192,
    ImGuiItemFlags_ReadOnly = 2048
};

/* ImGuiItemStatusFlags_ */
enum {
    ImGuiItemStatusFlags_Deactivated = 64,
    ImGuiItemStatusFlags_Edited = 4,
    ImGuiItemStatusFlags_EditedInternal = 2048,
    ImGuiItemStatusFlags_HasClipRect = 512,
    ImGuiItemStatusFlags_HasDeactivated = 32,
    ImGuiItemStatusFlags_HasDisplayRect = 2,
    ImGuiItemStatusFlags_HasShortcut = 1024,
    ImGuiItemStatusFlags_HoveredRect = 1,
    ImGuiItemStatusFlags_HoveredWindow = 128,
    ImGuiItemStatusFlags_None = 0,
    ImGuiItemStatusFlags_ToggledOpen = 16,
    ImGuiItemStatusFlags_ToggledSelection = 8,
    ImGuiItemStatusFlags_Visible = 256
};

/* ImGuiLayoutType_ */
enum {
    ImGuiLayoutType_Horizontal = 0,
    ImGuiLayoutType_Vertical = 1
};

/* ImGuiLocKey */
enum {
    ImGuiLocKey_COUNT = 12,
    ImGuiLocKey_CopyLink = 11,
    ImGuiLocKey_OpenLink_s = 10,
    ImGuiLocKey_TableReset = 4,
    ImGuiLocKey_TableResetOrder = 5,
    ImGuiLocKey_TableResetVisibility = 6,
    ImGuiLocKey_TableSizeAllDefault = 3,
    ImGuiLocKey_TableSizeAllFit = 2,
    ImGuiLocKey_TableSizeOne = 1,
    ImGuiLocKey_VersionStr = 0,
    ImGuiLocKey_WindowingMainMenuBar = 7,
    ImGuiLocKey_WindowingPopup = 8,
    ImGuiLocKey_WindowingUntitled = 9
};

/* ImGuiLogFlags_ */
enum {
    ImGuiLogFlags_None = 0,
    ImGuiLogFlags_OutputBuffer = 4,
    ImGuiLogFlags_OutputClipboard = 8,
    ImGuiLogFlags_OutputFile = 2,
    ImGuiLogFlags_OutputMask_ = 15,
    ImGuiLogFlags_OutputTTY = 1
};

/* ImGuiNavLayer */
enum {
    ImGuiNavLayer_COUNT = 2,
    ImGuiNavLayer_Main = 0,
    ImGuiNavLayer_Menu = 1
};

/* ImGuiNavMoveFlags_ */
enum {
    ImGuiNavMoveFlags_Activate = 4096,
    ImGuiNavMoveFlags_AllowCurrentNavId = 16,
    ImGuiNavMoveFlags_AlsoScoreVisibleSet = 32,
    ImGuiNavMoveFlags_DebugNoResult = 256,
    ImGuiNavMoveFlags_FocusApi = 512,
    ImGuiNavMoveFlags_Forwarded = 128,
    ImGuiNavMoveFlags_IsPageMove = 2048,
    ImGuiNavMoveFlags_IsTabbing = 1024,
    ImGuiNavMoveFlags_LoopX = 1,
    ImGuiNavMoveFlags_LoopY = 2,
    ImGuiNavMoveFlags_NoClearActiveId = 32768,
    ImGuiNavMoveFlags_NoSelect = 8192,
    ImGuiNavMoveFlags_NoSetNavCursorVisible = 16384,
    ImGuiNavMoveFlags_None = 0,
    ImGuiNavMoveFlags_ScrollToEdgeY = 64,
    ImGuiNavMoveFlags_WrapMask_ = 15,
    ImGuiNavMoveFlags_WrapX = 4,
    ImGuiNavMoveFlags_WrapY = 8
};

/* ImGuiNavRenderCursorFlags_ */
enum {
    ImGuiNavHighlightFlags_AlwaysDraw = 4,
    ImGuiNavHighlightFlags_Compact = 2,
    ImGuiNavHighlightFlags_NoRounding = 8,
    ImGuiNavHighlightFlags_None = 0,
    ImGuiNavRenderCursorFlags_AlwaysDraw = 4,
    ImGuiNavRenderCursorFlags_Compact = 2,
    ImGuiNavRenderCursorFlags_NoRounding = 8,
    ImGuiNavRenderCursorFlags_None = 0
};

/* ImGuiNextItemDataFlags_ */
enum {
    ImGuiNextItemDataFlags_HasColorMarker = 32,
    ImGuiNextItemDataFlags_HasOpen = 2,
    ImGuiNextItemDataFlags_HasRefVal = 8,
    ImGuiNextItemDataFlags_HasShortcut = 4,
    ImGuiNextItemDataFlags_HasStorageID = 16,
    ImGuiNextItemDataFlags_HasWidth = 1,
    ImGuiNextItemDataFlags_None = 0
};

/* ImGuiNextWindowDataFlags_ */
enum {
    ImGuiNextWindowDataFlags_HasBgAlpha = 64,
    ImGuiNextWindowDataFlags_HasChildFlags = 512,
    ImGuiNextWindowDataFlags_HasCollapsed = 8,
    ImGuiNextWindowDataFlags_HasContentSize = 4,
    ImGuiNextWindowDataFlags_HasFocus = 32,
    ImGuiNextWindowDataFlags_HasPos = 1,
    ImGuiNextWindowDataFlags_HasRefreshPolicy = 1024,
    ImGuiNextWindowDataFlags_HasScroll = 128,
    ImGuiNextWindowDataFlags_HasSize = 2,
    ImGuiNextWindowDataFlags_HasSizeConstraint = 16,
    ImGuiNextWindowDataFlags_HasWindowFlags = 256,
    ImGuiNextWindowDataFlags_None = 0
};

/* ImGuiOldColumnFlags_ */
enum {
    ImGuiOldColumnFlags_GrowParentContentsSize = 16,
    ImGuiOldColumnFlags_NoBorder = 1,
    ImGuiOldColumnFlags_NoForceWithinWindow = 8,
    ImGuiOldColumnFlags_NoPreserveWidths = 4,
    ImGuiOldColumnFlags_NoResize = 2,
    ImGuiOldColumnFlags_None = 0
};

/* ImGuiPlotType */
enum {
    ImGuiPlotType_Histogram = 1,
    ImGuiPlotType_Lines = 0
};

/* ImGuiPopupPositionPolicy */
enum {
    ImGuiPopupPositionPolicy_ComboBox = 1,
    ImGuiPopupPositionPolicy_Default = 0,
    ImGuiPopupPositionPolicy_Tooltip = 2
};

/* ImGuiScrollFlags_ */
enum {
    ImGuiScrollFlags_AlwaysCenterX = 16,
    ImGuiScrollFlags_AlwaysCenterY = 32,
    ImGuiScrollFlags_KeepVisibleCenterX = 4,
    ImGuiScrollFlags_KeepVisibleCenterY = 8,
    ImGuiScrollFlags_KeepVisibleEdgeX = 1,
    ImGuiScrollFlags_KeepVisibleEdgeY = 2,
    ImGuiScrollFlags_MaskX_ = 21,
    ImGuiScrollFlags_MaskY_ = 42,
    ImGuiScrollFlags_NoScrollParent = 64,
    ImGuiScrollFlags_None = 0
};

/* ImGuiSelectableFlagsPrivate_ */
enum {
    ImGuiSelectableFlags_NoHoldingActiveID = 1048576,
    ImGuiSelectableFlags_NoPadWithHalfSpacing = 67108864,
    ImGuiSelectableFlags_NoSetKeyOwner = 134217728,
    ImGuiSelectableFlags_SelectOnClick = 4194304,
    ImGuiSelectableFlags_SelectOnRelease = 8388608,
    ImGuiSelectableFlags_SetNavIdOnHover = 33554432,
    ImGuiSelectableFlags_SpanAvailWidth = 16777216
};

/* ImGuiSeparatorFlags_ */
enum {
    ImGuiSeparatorFlags_Horizontal = 1,
    ImGuiSeparatorFlags_None = 0,
    ImGuiSeparatorFlags_SpanAllColumns = 4,
    ImGuiSeparatorFlags_Vertical = 2
};

/* ImGuiSliderFlagsPrivate_ */
enum {
    ImGuiSliderFlags_ReadOnly = 2097152,
    ImGuiSliderFlags_Vertical = 1048576
};

/* ImGuiTabBarFlagsPrivate_ */
enum {
    ImGuiTabBarFlags_DockNode = 1048576,
    ImGuiTabBarFlags_IsFocused = 2097152,
    ImGuiTabBarFlags_SaveSettings = 4194304
};

/* ImGuiTabItemFlagsPrivate_ */
enum {
    ImGuiTabItemFlags_Button = 2097152,
    ImGuiTabItemFlags_Invisible = 4194304,
    ImGuiTabItemFlags_NoCloseButton = 1048576,
    ImGuiTabItemFlags_SectionMask_ = 192
};

/* ImGuiTextFlags_ */
enum {
    ImGuiTextFlags_NoWidthForLargeClippedText = 1,
    ImGuiTextFlags_None = 0
};

/* ImGuiTooltipFlags_ */
enum {
    ImGuiTooltipFlags_None = 0,
    ImGuiTooltipFlags_OverridePrevious = 2
};

/* ImGuiTreeNodeFlagsPrivate_ */
enum {
    ImGuiTreeNodeFlags_ClipLabelForTrailingButton = 268435456,
    ImGuiTreeNodeFlags_DrawLinesMask_ = 1835008,
    ImGuiTreeNodeFlags_NoNavFocus = 134217728,
    ImGuiTreeNodeFlags_OpenOnMask_ = 192,
    ImGuiTreeNodeFlags_UpsideDownArrow = 536870912
};

/* ImGuiTypingSelectFlags_ */
enum {
    ImGuiTypingSelectFlags_AllowBackspace = 1,
    ImGuiTypingSelectFlags_AllowSingleCharMode = 2,
    ImGuiTypingSelectFlags_None = 0
};

/* ImGuiWindowBgClickFlags_ */
enum {
    ImGuiWindowBgClickFlags_Move = 1,
    ImGuiWindowBgClickFlags_None = 0
};

/* ImGuiWindowRefreshFlags_ */
enum {
    ImGuiWindowRefreshFlags_None = 0,
    ImGuiWindowRefreshFlags_RefreshOnFocus = 4,
    ImGuiWindowRefreshFlags_RefreshOnHover = 2,
    ImGuiWindowRefreshFlags_TryToAvoidRefresh = 1
};

/* ImTriangulatorNodeType */
enum {
    ImTriangulatorNodeType_Convex = 0,
    ImTriangulatorNodeType_Ear = 1,
    ImTriangulatorNodeType_Reflex = 2
};

/* ImWcharClass */
enum {
    ImWcharClass_Blank = 0,
    ImWcharClass_Other = 2,
    ImWcharClass_Punct = 1
};

struct Func {
    unsigned char imgui_c89_empty;
};

struct Funcs {
    unsigned char imgui_c89_empty;
};

struct ImBitArray_155_512 {
    ImU32 Data[5];
};

struct ImBitArray_155 {
    ImU32 Data[5];
};

struct imgui_c89_anon_imgui_draw_5418_5 {
    ImGuiID FontId;
    float BakedSize;
    float RasterizerDensity;
};

struct ImFontAtlasPostProcessData {
    ImFontAtlas * FontAtlas;
    ImFont * Font;
    ImFontConfig * FontSrc;
    ImFontBaked * FontBaked;
    ImFontGlyph * Glyph;
    void * Pixels;
    ImTextureFormat Format;
    int Pitch;
    int Width;
    int Height;
};

struct ImFontAtlasRectEntry {
    IMGUI_C89_EXTENSION int TargetIndex : 20;
    IMGUI_C89_EXTENSION unsigned int Generation : 10;
    IMGUI_C89_EXTENSION unsigned int IsUsed : 1;
};

struct ImFontLoader {
    const char * Name;
    unsigned char (*LoaderInit)(ImFontAtlas *);
    void (*LoaderShutdown)(ImFontAtlas *);
    unsigned char (*FontSrcInit)(ImFontAtlas *, ImFontConfig *);
    void (*FontSrcDestroy)(ImFontAtlas *, ImFontConfig *);
    unsigned char (*FontSrcContainsGlyph)(ImFontAtlas *, ImFontConfig *, ImWchar);
    unsigned char (*FontBakedInit)(ImFontAtlas *, ImFontConfig *, ImFontBaked *, void *);
    void (*FontBakedDestroy)(ImFontAtlas *, ImFontConfig *, ImFontBaked *, void *);
    unsigned char (*FontBakedLoadGlyph)(ImFontAtlas *, ImFontConfig *, ImFontBaked *, void *, ImWchar, ImFontGlyph *, float *);
    size_t FontBakedSrcLoaderDataSize;
};

struct ImFontStackData {
    ImFont * Font;
    float FontSizeBeforeScaling;
    float FontSizeAfterScaling;
};

struct ImGuiContextHook {
    ImGuiID HookId;
    ImGuiContextHookType Type;
    ImGuiID Owner;
    ImGuiContextHookCallback Callback;
    void * UserData;
};

struct ImGuiDataTypeInfo {
    size_t Size;
    const char * Name;
    const char * PrintFmt;
    const char * ScanFmt;
};

struct ImGuiDataTypeStorage {
    ImU8 Data[8];
};

struct ImGuiDeactivatedItemData {
    ImGuiID ID;
    int ElapseFrame;
    unsigned char HasBeenEditedBefore;
    unsigned char IsAlive;
};

struct ImGuiDebugAllocEntry {
    int FrameCount;
    ImS16 AllocCount;
    ImS16 FreeCount;
};

struct ImGuiDebugAllocInfo {
    int TotalAllocCount;
    int TotalFreeCount;
    ImS16 LastEntriesIdx;
    ImGuiDebugAllocEntry LastEntriesBuf[6];
};

struct ImGuiErrorRecoveryState {
    short SizeOfWindowStack;
    short SizeOfIDStack;
    short SizeOfTreeStack;
    short SizeOfColorStack;
    short SizeOfStyleVarStack;
    short SizeOfFontStack;
    short SizeOfFocusScopeStack;
    short SizeOfGroupStack;
    short SizeOfItemFlagsStack;
    short SizeOfBeginPopupStack;
    short SizeOfDisabledStack;
};

struct ImGuiFocusScopeData {
    ImGuiID ID;
    ImGuiID WindowID;
};

struct ImGuiIDStackTool {
    unsigned char OptHexEncodeNonAsciiChars;
    unsigned char OptCopyToClipboardOnCtrlC;
    int LastActiveFrame;
    float CopyToClipboardLastTime;
};

struct ImGuiInputEventAppFocused {
    unsigned char Focused;
};

struct ImGuiInputEventKey {
    ImGuiKey Key;
    unsigned char Down;
    float AnalogValue;
};

struct ImGuiInputEventMouseButton {
    int Button;
    unsigned char Down;
    ImGuiMouseSource MouseSource;
};

struct ImGuiInputEventMousePos {
    float PosX;
    float PosY;
    ImGuiMouseSource MouseSource;
};

struct ImGuiInputEventMouseWheel {
    float WheelX;
    float WheelY;
    ImGuiMouseSource MouseSource;
};

struct ImGuiInputEventText {
    unsigned int Char;
};

union imgui_c89_anon_imgui_internal_1585_5 {
    ImGuiInputEventMousePos MousePos;
    ImGuiInputEventMouseWheel MouseWheel;
    ImGuiInputEventMouseButton MouseButton;
    ImGuiInputEventKey Key;
    ImGuiInputEventText Text;
    ImGuiInputEventAppFocused AppFocused;
};

struct ImGuiInputEvent {
    ImGuiInputEventType Type;
    ImGuiInputSource Source;
    ImU32 EventId;
    imgui_c89_anon_imgui_internal_1585_5 imgui_c89_unnamed_e911c36c;
    unsigned char AddedByTestEngine;
};

struct ImGuiKeyOwnerData {
    ImGuiID OwnerCurr;
    ImGuiID OwnerNext;
    unsigned char LockThisFrame;
    unsigned char LockUntilRelease;
};

struct ImGuiKeyRoutingData {
    ImGuiKeyRoutingIndex NextEntryIndex;
    ImU16 Mods;
    ImU16 RoutingCurrScore;
    ImU16 RoutingNextScore;
    ImGuiID RoutingCurr;
    ImGuiID RoutingNext;
};

struct ImGuiListClipperRange {
    int Min;
    int Max;
    unsigned char PosToIndexConvert;
    ImS8 PosToIndexOffsetMin;
    ImS8 PosToIndexOffsetMax;
};

struct ImGuiLocEntry {
    ImGuiLocKey Key;
    const char * Text;
};

struct ImGuiMenuColumns {
    ImU32 TotalWidth;
    ImU32 NextTotalWidth;
    ImU16 Spacing;
    ImU16 OffsetIcon;
    ImU16 OffsetLabel;
    ImU16 OffsetShortcut;
    ImU16 OffsetMark;
    ImU16 Widths[4];
};

struct ImGuiMetricsConfig {
    unsigned char ShowDebugLog;
    unsigned char ShowIDStackTool;
    unsigned char ShowWindowsRects;
    unsigned char ShowWindowsBeginOrder;
    unsigned char ShowTablesRects;
    unsigned char ShowDrawCmdMesh;
    unsigned char ShowDrawCmdBoundingBoxes;
    unsigned char ShowTextEncodingViewer;
    unsigned char ShowTextureUsedRect;
    int ShowWindowsRectsType;
    int ShowTablesRectsType;
    int HighlightMonitorIdx;
    ImGuiID HighlightViewportID;
    int SettingsDiscardMonths;
    unsigned char SettingsHighlightOldEntries;
    unsigned char ShowFontPreview;
};

struct ImGuiMultiSelectState {
    ImGuiWindow * Window;
    ImGuiID ID;
    int LastFrameActive;
    int LastSelectionSize;
    ImS8 RangeSelected;
    ImS8 NavIdSelected;
    ImGuiSelectionUserData RangeSrcItem;
    ImGuiSelectionUserData NavIdItem;
};

struct ImGuiNextItemData {
    ImGuiNextItemDataFlags HasFlags;
    ImGuiItemFlags ItemFlagsSet;
    ImGuiID FocusScopeId;
    ImGuiSelectionUserData SelectionUserData;
    float Width;
    ImGuiKeyChord Shortcut;
    ImGuiInputFlags ShortcutFlags;
    unsigned char OpenVal;
    ImU8 OpenCond;
    ImGuiDataTypeStorage RefVal;
    ImGuiID StorageId;
    ImU32 ColorMarker;
};

struct ImGuiPackedDate {
    IMGUI_C89_EXTENSION ImU16 Year : 7;
    IMGUI_C89_EXTENSION ImU16 Month : 4;
    IMGUI_C89_EXTENSION ImU16 Day : 5;
};

struct ImGuiPlotArrayGetterData {
    const float * Values;
    int Stride;
};

struct ImGuiPtrOrIndex {
    void * Ptr;
    int Index;
};

struct ImGuiSettingsCleanupArgs {
    ImGuiID TypeHashFilter;
    int DiscardOlderThanMonths;
    unsigned char DiscardWhenMissingDate;
    unsigned char DiscardAll;
    unsigned char SetCurrentSessionDateToAll;
    unsigned char SetCurrentSessionDateWhenMissingDate;
    int _DiscardOlderThanDate;
};

struct ImGuiSettingsHandler {
    const char * TypeName;
    ImGuiID TypeHash;
    void (*ClearAllFn)(ImGuiContext *, ImGuiSettingsHandler *);
    void (*ReadInitFn)(ImGuiContext *, ImGuiSettingsHandler *);
    void *(*ReadOpenFn)(ImGuiContext *, ImGuiSettingsHandler *, const char *);
    void (*ReadLineFn)(ImGuiContext *, ImGuiSettingsHandler *, void *, const char *);
    void (*ApplyAllFn)(ImGuiContext *, ImGuiSettingsHandler *);
    void (*WriteAllFn)(ImGuiContext *, ImGuiSettingsHandler *, ImGuiTextBuffer *);
    void (*CleanupFn)(ImGuiContext *, ImGuiSettingsHandler *, ImGuiSettingsCleanupArgs *);
    void * UserData;
};

struct ImGuiShrinkWidthItem {
    int Index;
    float Width;
    float InitialWidth;
};

struct ImGuiStackLevelInfo {
    ImGuiID ID;
    ImS8 QueryFrameCount;
    unsigned char QuerySuccess;
    ImS8 DataType;
    int DescOffset;
};

union imgui_c89_anon_imgui_internal_955_5 {
    int BackupInt[2];
    float BackupFloat[2];
};

struct ImGuiStyleMod {
    ImGuiStyleVar VarIdx;
    imgui_c89_anon_imgui_internal_955_5 imgui_c89_unnamed_1f2e9324;
};

struct ImGuiStyleVarInfo {
    IMGUI_C89_EXTENSION ImU32 Count : 8;
    IMGUI_C89_EXTENSION ImGuiDataType DataType : 8;
    IMGUI_C89_EXTENSION ImU32 Offset : 16;
};

struct ImGuiTabBarSection {
    int TabCount;
    float Width;
    float WidthAfterShrinkMinWidth;
    float Spacing;
};

struct ImGuiTabItem {
    ImGuiID ID;
    ImGuiTabItemFlags Flags;
    int LastFrameVisible;
    int LastFrameSelected;
    float Offset;
    float Width;
    float ContentWidth;
    float RequestedWidth;
    ImS32 NameOffset;
    ImS16 BeginOrder;
    ImS16 IndexDuringLayout;
    unsigned char WantClose;
};

struct ImGuiTableCellData {
    ImU32 BgColor;
    ImGuiTableColumnIdx Column;
};

struct ImGuiTableColumnSettings {
    float WidthOrWeight;
    ImGuiID ID;
    ImGuiTableColumnIdx Index;
    ImGuiTableColumnIdx DisplayOrder;
    ImGuiTableColumnIdx SortOrder;
    IMGUI_C89_EXTENSION ImU8 SortDirection : 2;
    IMGUI_C89_EXTENSION ImS8 IsEnabled : 2;
    IMGUI_C89_EXTENSION ImU8 IsStretch : 1;
    IMGUI_C89_EXTENSION unsigned char IsLoaded : 1;
};

struct ImGuiTableFixDisplayOrderColumnData {
    ImGuiTableColumnIdx Idx;
    ImGuiTable * Table;
};

struct ImGuiTableHeaderData {
    ImGuiTableColumnIdx Index;
    ImU32 TextColor;
    ImU32 BgColor0;
    ImU32 BgColor1;
};

struct ImGuiTableInstanceData {
    ImGuiID TableInstanceID;
    float LastOuterHeight;
    float LastTopHeadersRowHeight;
    float LastFrozenHeight;
    int HoveredRowLast;
    int HoveredRowNext;
};

struct ImGuiTableSettings {
    ImGuiID ID;
    ImGuiTableFlags SaveFlags;
    float RefScale;
    ImGuiTableColumnIdx ColumnsCount;
    ImGuiTableColumnIdx ColumnsCountMax;
    ImGuiPackedDate LastUsedDate;
    IMGUI_C89_EXTENSION unsigned char WantApply : 1;
};

struct ImGuiTypingSelectRequest {
    ImGuiTypingSelectFlags Flags;
    int SearchBufferLen;
    const char * SearchBuffer;
    unsigned char SelectRequest;
    unsigned char SingleCharMode;
    ImS8 SingleCharSize;
};

struct ImGuiTypingSelectState {
    ImGuiTypingSelectRequest Request;
    char SearchBuffer[64];
    ImGuiID FocusScope;
    int LastRequestFrame;
    float LastRequestTime;
    unsigned char SingleCharModeLock;
};

struct imgui_c89_anon_imstb_textedit_553_9 {
    float x;
    float y;
    float height;
    int first_char;
    int length;
    int prev_first;
};

struct imgui_c89_anon_imstb_textedit_389_9 {
    float x0;
    float x1;
    float baseline_y_delta;
    float ymin;
    float ymax;
    int num_chars;
};

struct imgui_c89_anon_imstb_textedit_324_9 {
    int where;
    int insert_length;
    int delete_length;
    int char_storage;
};

struct imgui_c89_anon_imstb_textedit_333_9 {
    imgui_c89_anon_imstb_textedit_324_9 undo_rec[99];
    char undo_char[999];
    short undo_point;
    short redo_point;
    int undo_char_point;
    int redo_char_point;
};

struct ImStb_STB_TexteditState {
    int cursor;
    int select_start;
    int select_end;
    unsigned char insert_mode;
    int row_count_per_page;
    unsigned char cursor_at_end_of_line;
    unsigned char initialized;
    unsigned char has_preferred_x;
    unsigned char single_line;
    unsigned char padding1;
    unsigned char padding2;
    unsigned char padding3;
    float preferred_x;
    imgui_c89_anon_imstb_textedit_333_9 undostate;
};

struct ImTriangulatorNodeSpan {
    ImTriangulatorNode ** Data;
    int Size;
};

struct ImTriangulator {
    int _TrianglesLeft;
    ImTriangulatorNode * _Nodes;
    ImTriangulatorNodeSpan _Ears;
    ImTriangulatorNodeSpan _Reflexes;
};

struct ImVec1 {
    float x;
};

struct ImGuiGroupData {
    ImGuiID WindowID;
    ImVec2 BackupCursorPos;
    ImVec2 BackupCursorMaxPos;
    ImVec2 BackupCursorPosPrevLine;
    ImVec1 BackupIndent;
    ImVec1 BackupGroupOffset;
    ImVec2 BackupCurrLineSize;
    float BackupCurrLineTextBaseOffset;
    ImGuiID BackupActiveIdIsAlive;
    unsigned char BackupAnyIdHasBeenEditedThisFrame;
    unsigned char BackupDeactivatedIdIsAlive;
    unsigned char BackupHoveredIdIsAlive;
    unsigned char BackupIsSameLine;
    unsigned char EmitItem;
};

struct ImGuiPopupData {
    ImGuiID PopupId;
    ImGuiWindow * Window;
    ImGuiWindow * RestoreNavWindow;
    int ParentNavLayer;
    int OpenFrameCount;
    ImGuiID OpenParentId;
    ImVec2 OpenPopupPos;
    ImVec2 OpenMousePos;
};

struct ImGuiResizeBorderDef {
    ImVec2 InnerDir;
    ImVec2 SegmentN1;
    ImVec2 SegmentN2;
    float OuterAngle;
};

struct ImGuiResizeGripDef {
    ImVec2 CornerPosN;
    ImVec2 InnerDir;
    int AngleMin12;
    int AngleMax12;
};

struct ImRect {
    ImVec2 Min;
    ImVec2 Max;
};

struct ImGuiBoxSelectState {
    ImGuiID ID;
    unsigned char IsActive;
    unsigned char IsStarting;
    unsigned char IsStartedFromVoid;
    unsigned char IsStartedSetNavIdOnce;
    unsigned char RequestClear;
    IMGUI_C89_EXTENSION ImGuiKeyChord KeyMods : 16;
    ImVec2 StartPosRel;
    ImVec2 EndPosRel;
    ImVec2 ScrollAccum;
    ImGuiWindow * Window;
    unsigned char UnclipMode;
    ImRect UnclipRect;
    ImRect UnclipRects[2];
    ImRect BoxSelectRectPrev;
    ImRect BoxSelectRectCurr;
};

struct ImGuiComboPreviewData {
    ImRect PreviewRect;
    ImVec2 BackupCursorPos;
    ImVec2 BackupCursorMaxPos;
    ImVec2 BackupCursorPosPrevLine;
    float BackupPrevLineTextBaseOffset;
    ImGuiLayoutType BackupLayout;
};

struct ImGuiLastItemData {
    ImGuiID ID;
    ImGuiItemFlags ItemFlags;
    ImGuiItemStatusFlags StatusFlags;
    ImRect Rect;
    ImRect NavRect;
    ImRect DisplayRect;
    ImRect ClipRect;
    ImGuiKeyChord Shortcut;
};

struct ImGuiNavItemData {
    ImGuiWindow * Window;
    ImGuiID ID;
    ImGuiID FocusScopeId;
    ImRect RectRel;
    ImGuiItemFlags ItemFlags;
    float DistBox;
    float DistCenter;
    float DistAxial;
    ImGuiSelectionUserData SelectionUserData;
};

struct ImGuiNextWindowData {
    ImGuiNextWindowDataFlags HasFlags;
    ImGuiCond PosCond;
    ImGuiCond SizeCond;
    ImGuiCond CollapsedCond;
    ImVec2 PosVal;
    ImVec2 PosPivotVal;
    ImVec2 SizeVal;
    ImVec2 ContentSizeVal;
    ImVec2 ScrollVal;
    ImGuiWindowFlags WindowFlags;
    ImGuiChildFlags ChildFlags;
    unsigned char CollapsedVal;
    ImRect SizeConstraintRect;
    ImGuiSizeCallback SizeCallback;
    void * SizeCallbackUserData;
    float BgAlphaVal;
    ImVec2 MenuBarOffsetMinVal;
    ImGuiWindowRefreshFlags RefreshFlagsVal;
};

struct ImGuiOldColumnData {
    float OffsetNorm;
    float OffsetNormBeforeResize;
    ImGuiOldColumnFlags Flags;
    ImRect ClipRect;
};

struct ImGuiTableColumn {
    ImGuiTableColumnFlags Flags;
    float WidthGiven;
    float MinX;
    float MaxX;
    float WidthRequest;
    float WidthAuto;
    float WidthMax;
    float StretchWeight;
    float InitStretchWeightOrWidth;
    ImRect ClipRect;
    ImGuiID ID;
    ImGuiID UserData;
    float WorkMinX;
    float WorkMaxX;
    float ItemWidth;
    float ContentMaxXFrozen;
    float ContentMaxXUnfrozen;
    float ContentMaxXHeadersUsed;
    float ContentMaxXHeadersIdeal;
    ImS16 NameOffset;
    ImGuiTableColumnIdx DisplayOrder;
    ImGuiTableColumnIdx IndexWithinEnabledSet;
    ImGuiTableColumnIdx PrevEnabledColumn;
    ImGuiTableColumnIdx NextEnabledColumn;
    ImGuiTableColumnIdx SortOrder;
    ImGuiTableDrawChannelIdx DrawChannelCurrent;
    ImGuiTableDrawChannelIdx DrawChannelFrozen;
    ImGuiTableDrawChannelIdx DrawChannelUnfrozen;
    unsigned char IsEnabled;
    unsigned char IsUserEnabled;
    unsigned char IsUserEnabledNextFrame;
    unsigned char IsVisibleX;
    unsigned char IsVisibleY;
    unsigned char IsRequestOutput;
    unsigned char IsSkipItems;
    IMGUI_C89_EXTENSION unsigned char IsPreserveWidthAuto : 1;
    IMGUI_C89_EXTENSION unsigned char IsJustCreated : 1;
    IMGUI_C89_EXTENSION unsigned char IsLoadedSettings : 1;
    IMGUI_C89_EXTENSION unsigned char IsNeedReconcileSrc : 1;
    IMGUI_C89_EXTENSION unsigned char IsNeedReconcileDst : 1;
    ImS8 NavLayerCurrent;
    IMGUI_C89_EXTENSION ImU8 AutoFitQueue : 4;
    IMGUI_C89_EXTENSION ImU8 CannotSkipItemsQueue : 4;
    IMGUI_C89_EXTENSION ImU8 SortDirection : 2;
    IMGUI_C89_EXTENSION ImU8 SortDirectionsAvailCount : 2;
    IMGUI_C89_EXTENSION ImU8 SortDirectionsAvailMask : 4;
    ImU8 SortDirectionsAvailList;
};

struct ImGuiTableReconcileColumnData {
    ImGuiID ID;
    ImS16 NameOffset;
    ImGuiTableColumnFlags Flags;
    float InitWidthOrWeight;
    ImGuiID UserData;
    ImGuiTableColumnIdx ColumnNewIdx;
    ImGuiTableColumnIdx ColumnOldIdx;
    ImGuiTableColumn ColumnOldData;
};

struct ImGuiTreeNodeStackData {
    ImGuiID ID;
    ImGuiTreeNodeFlags TreeFlags;
    ImGuiItemFlags ItemFlags;
    ImRect NavRect;
    float DrawLinesX1;
    float DrawLinesToNodesY2;
    ImGuiTableColumnIdx DrawLinesTableColumn;
};

struct ImGuiWindowStackData {
    ImGuiWindow * Window;
    ImGuiLastItemData ParentLastItemDataBackup;
    ImGuiErrorRecoveryState StackSizesInBegin;
    unsigned char DisabledOverrideReenable;
    float DisabledOverrideReenableAlphaBackup;
};

struct ImTriangulatorNode {
    ImTriangulatorNodeType Type;
    int Index;
    ImVec2 Pos;
    ImTriangulatorNode * Next;
    ImTriangulatorNode * Prev;
};

struct ImVec2i {
    int x;
    int y;
};

struct ImVec2ih {
    short x;
    short y;
};

struct ImGuiWindowSettings {
    ImGuiID ID;
    ImVec2ih Pos;
    ImVec2ih Size;
    ImGuiPackedDate LastUsedDate;
    IMGUI_C89_EXTENSION unsigned char Collapsed : 1;
    IMGUI_C89_EXTENSION unsigned char IsChild : 1;
    IMGUI_C89_EXTENSION unsigned char WantApply : 1;
    IMGUI_C89_EXTENSION unsigned char WantDelete : 1;
};

struct ImGuiColorMod {
    ImGuiCol Col;
    ImVec4 BackupValue;
};

struct ImGuiTableTempData {
    ImGuiID WindowID;
    int TableIndex;
    float LastTimeActive;
    float AngledHeadersExtraWidth;
    int AngledHeadersRequestsSize;
    int AngledHeadersRequestsCapacity;
    ImGuiTableHeaderData * AngledHeadersRequests;
    int ReconcileColumnsRequestsSize;
    int ReconcileColumnsRequestsCapacity;
    ImGuiTableReconcileColumnData * ReconcileColumnsRequests;
    void * OldColumnsRawData;
    ImGuiTableColumn * OldColumnsData;
    ImGuiTableColumn * OldColumnsDataEnd;
    ImVec2 UserOuterSize;
    ImDrawListSplitter DrawSplitter;
    ImRect HostBackupWorkRect;
    ImRect HostBackupParentWorkRect;
    ImVec2 HostBackupPrevLineSize;
    ImVec2 HostBackupCurrLineSize;
    ImVec2 HostBackupCursorMaxPos;
    ImVec1 HostBackupColumnsOffset;
    float HostBackupItemWidth;
    int HostBackupItemWidthStackSize;
};

struct ImDrawDataBuilder {
    ImVector_ImDrawList_ptr * Layers[2];
    ImVector_ImDrawList_ptr LayerData1;
};

struct ImGuiViewportP {
    ImGuiID ID;
    ImGuiViewportFlags Flags;
    ImVec2 Pos;
    ImVec2 Size;
    ImVec2 FramebufferScale;
    ImVec2 WorkPos;
    ImVec2 WorkSize;
    void * PlatformHandle;
    void * PlatformHandleRaw;
    float BgFgDrawListsLastTimeActive[2];
    ImDrawList * BgFgDrawLists[2];
    ImDrawData DrawDataP;
    ImDrawDataBuilder DrawDataBuilder;
    ImVec2 WorkInsetMin;
    ImVec2 WorkInsetMax;
    ImVec2 BuildWorkInsetMin;
    ImVec2 BuildWorkInsetMax;
};

struct ImVector_ImFontAtlas_ptr {
    int Size;
    int Capacity;
    ImFontAtlas ** Data;
};

struct ImVector_ImFontAtlasRectEntry {
    int Size;
    int Capacity;
    ImFontAtlasRectEntry * Data;
};

struct ImVector_ImFontBaked_ptr {
    int Size;
    int Capacity;
    ImFontBaked ** Data;
};

struct ImStableVector_ImFontBaked_32 {
    int Size;
    int Capacity;
    ImVector_ImFontBaked_ptr Blocks;
};

struct ImVector_ImFontStackData {
    int Size;
    int Capacity;
    ImFontStackData * Data;
};

struct ImVector_ImGuiColorMod {
    int Size;
    int Capacity;
    ImGuiColorMod * Data;
};

struct ImVector_ImGuiContextHook {
    int Size;
    int Capacity;
    ImGuiContextHook * Data;
};

struct ImVector_ImGuiFocusScopeData {
    int Size;
    int Capacity;
    ImGuiFocusScopeData * Data;
};

struct ImVector_ImGuiGroupData {
    int Size;
    int Capacity;
    ImGuiGroupData * Data;
};

struct ImVector_ImGuiInputEvent {
    int Size;
    int Capacity;
    ImGuiInputEvent * Data;
};

struct ImVector_ImGuiKeyRoutingData {
    int Size;
    int Capacity;
    ImGuiKeyRoutingData * Data;
};

struct ImGuiKeyRoutingTable {
    ImGuiKeyRoutingIndex Index[155];
    ImVector_ImGuiKeyRoutingData Entries;
    ImVector_ImGuiKeyRoutingData EntriesNext;
};

struct ImVector_ImGuiListClipperData {
    int Size;
    int Capacity;
    ImGuiListClipperData * Data;
};

struct ImVector_ImGuiListClipperRange {
    int Size;
    int Capacity;
    ImGuiListClipperRange * Data;
};

struct ImGuiListClipperData {
    ImGuiListClipper * ListClipper;
    float LossynessOffset;
    int StepNo;
    int ItemsFrozen;
    ImVector_ImGuiListClipperRange Ranges;
};

struct ImVector_ImGuiMultiSelectState {
    int Size;
    int Capacity;
    ImGuiMultiSelectState * Data;
};

struct ImVector_ImGuiMultiSelectTempData {
    int Size;
    int Capacity;
    ImGuiMultiSelectTempData * Data;
};

struct ImVector_ImGuiOldColumnData {
    int Size;
    int Capacity;
    ImGuiOldColumnData * Data;
};

struct ImGuiOldColumns {
    ImGuiID ID;
    ImGuiOldColumnFlags Flags;
    unsigned char IsFirstFrame;
    unsigned char IsBeingResized;
    int Current;
    int Count;
    float OffMinX;
    float OffMaxX;
    float LineMinY;
    float LineMaxY;
    float HostCursorPosY;
    float HostCursorMaxPosX;
    ImRect HostInitialClipRect;
    ImRect HostBackupClipRect;
    ImRect HostBackupParentWorkRect;
    ImVector_ImGuiOldColumnData Columns;
    ImDrawListSplitter Splitter;
};

struct ImVector_ImGuiOldColumns {
    int Size;
    int Capacity;
    ImGuiOldColumns * Data;
};

struct ImVector_ImGuiPopupData {
    int Size;
    int Capacity;
    ImGuiPopupData * Data;
};

struct ImVector_ImGuiPtrOrIndex {
    int Size;
    int Capacity;
    ImGuiPtrOrIndex * Data;
};

struct ImGuiMultiSelectTempData {
    ImGuiMultiSelectIO IO;
    ImGuiMultiSelectState * Storage;
    ImGuiID FocusScopeId;
    ImGuiMultiSelectFlags Flags;
    ImVec2 ScopeRectMin;
    ImVec2 BackupCursorMaxPos;
    ImGuiID BoxSelectId;
    ImGuiKeyChord KeyMods;
    ImS8 LoopRequestSetAll;
    unsigned char IsEndIO;
    unsigned char IsFocused;
    unsigned char IsKeyboardSetRange;
    unsigned char NavIdPassedBy;
    unsigned char RangeSrcPassedBy;
    unsigned char RangeDstPassedBy;
    unsigned char IsSoleOrUnknownSelectionSize;
};

struct ImVector_ImGuiSettingsHandler {
    int Size;
    int Capacity;
    ImGuiSettingsHandler * Data;
};

struct ImVector_ImGuiShrinkWidthItem {
    int Size;
    int Capacity;
    ImGuiShrinkWidthItem * Data;
};

struct ImVector_ImGuiStackLevelInfo {
    int Size;
    int Capacity;
    ImGuiStackLevelInfo * Data;
};

struct ImPool_ImGuiMultiSelectState {
    ImVector_ImGuiMultiSelectState Buf;
    ImGuiStorage Map;
    ImPoolIdx FreeIdx;
    ImPoolIdx AliveCount;
};

struct ImGuiTablePool {
    int Size;
    int Capacity;
    ImGuiTable * Data;
    ImGuiStorage Map;
    ImPoolIdx FreeIdx;
    ImPoolIdx AliveCount;
};

struct ImVector_ImGuiStyleMod {
    int Size;
    int Capacity;
    ImGuiStyleMod * Data;
};

struct ImVector_ImGuiTabBar {
    int Size;
    int Capacity;
    ImGuiTabBar * Data;
};

struct ImPool_ImGuiTabBar {
    ImVector_ImGuiTabBar Buf;
    ImGuiStorage Map;
    ImPoolIdx FreeIdx;
    ImPoolIdx AliveCount;
};

struct ImVector_ImGuiTabItem {
    int Size;
    int Capacity;
    ImGuiTabItem * Data;
};

struct ImVector_ImGuiTreeNodeStackData {
    int Size;
    int Capacity;
    ImGuiTreeNodeStackData * Data;
};

struct ImVector_ImGuiViewportP_ptr {
    int Size;
    int Capacity;
    ImGuiViewportP ** Data;
};

struct ImVector_ImGuiWindow_ptr {
    int Size;
    int Capacity;
    ImGuiWindow ** Data;
};

struct ImVector_ImGuiWindowStackData {
    int Size;
    int Capacity;
    ImGuiWindowStackData * Data;
};

struct ImDrawListSharedData {
    ImVec2 TexUvWhitePixel;
    const ImVec4 * TexUvLines;
    ImFontAtlas * FontAtlas;
    ImFont * Font;
    float FontSize;
    float FontScale;
    float CurveTessellationTol;
    float CircleTessellationMaxError;
    float InitialFringeScale;
    ImDrawListFlags InitialFlags;
    ImVec4 ClipRectFullscreen;
    ImVector_ImVec2 TempBuffer;
    ImVector_ImDrawList_ptr DrawLists;
    ImGuiContext * Context;
    ImVec2 ArcFastVtx[48];
    float ArcFastRadiusCutoff;
    ImU8 CircleSegmentCounts[64];
};

struct ImChunkStream_ImGuiTableSettings {
    ImVector_char Buf;
};

struct ImChunkStream_ImGuiWindowSettings {
    ImVector_char Buf;
};

struct ImGuiInputTextDeactivatedState {
    ImGuiID ID;
    int ElapseFrame;
    ImVector_char TextA;
};

struct ImGuiInputTextState {
    ImGuiContext * Ctx;
    ImStb_STB_TexteditState * Stb;
    ImGuiInputTextFlags Flags;
    ImGuiID ID;
    int TextLen;
    const char * TextSrc;
    ImVector_char TextA;
    ImVector_char TextToRevertTo;
    ImVector_char CallbackTextBackup;
    int BufCapacity;
    ImVec2 Scroll;
    int LineCount;
    float WrapWidth;
    float CursorAnim;
    unsigned char CursorFollow;
    unsigned char CursorCenterY;
    unsigned char SelectedAllMouseLock;
    unsigned char EditedBefore;
    unsigned char EditedThisFrame;
    unsigned char WantReloadUserBuf;
    ImS8 LastMoveDirectionLR;
    int ReloadSelectionStart;
    int ReloadSelectionEnd;
};

struct ImGuiDebugItemPathQuery {
    ImGuiID MainID;
    unsigned char Active;
    unsigned char Complete;
    ImS8 Step;
    ImVector_ImGuiStackLevelInfo Results;
    ImGuiTextBuffer ResultsDescBuf;
    ImGuiTextBuffer ResultPathBuf;
};

struct ImGuiTabBar {
    ImGuiWindow * Window;
    ImVector_ImGuiTabItem Tabs;
    ImGuiTabBarFlags Flags;
    ImGuiID ID;
    ImGuiID SelectedTabId;
    ImGuiID NextSelectedTabId;
    ImGuiID NextScrollToTabId;
    ImGuiID VisibleTabId;
    int CurrFrameVisible;
    int PrevFrameVisible;
    ImRect BarRect;
    float BarRectPrevWidth;
    float CurrTabsContentsHeight;
    float PrevTabsContentsHeight;
    float WidthAllTabs;
    float WidthAllTabsIdeal;
    float ScrollingAnim;
    float ScrollingTarget;
    float ScrollingTargetDistToVisibility;
    float ScrollingSpeed;
    float ScrollingRectMinX;
    float ScrollingRectMaxX;
    float SeparatorMinX;
    float SeparatorMaxX;
    ImGuiID ReorderRequestTabId;
    ImS16 ReorderRequestOffset;
    ImS8 BeginCount;
    unsigned char WantLayout;
    unsigned char VisibleTabWasSubmitted;
    unsigned char TabsAddedNew;
    unsigned char ScrollButtonEnabled;
    ImS16 TabsActiveCount;
    ImS16 LastTabItemIdx;
    float ItemSpacingY;
    ImVec2 FramePadding;
    ImVec2 BackupCursorPos;
    ImGuiTextBuffer TabsNames;
};

struct ImGuiTable {
    ImGuiID ID;
    ImGuiTableFlags Flags;
    void * RawData;
    ImGuiTableTempData * TempData;
    ImGuiTableColumn * Columns;
    ImGuiTableColumn * ColumnsEnd;
    ImGuiTableColumnIdx * DisplayOrderToIndex;
    ImGuiTableColumnIdx * DisplayOrderToIndexEnd;
    ImGuiTableCellData * RowCellData;
    ImGuiTableCellData * RowCellDataEnd;
    ImBitArrayPtr EnabledMaskByDisplayOrder;
    ImBitArrayPtr EnabledMaskByIndex;
    ImBitArrayPtr VisibleMaskByIndex;
    ImGuiTableFlags SettingsLoadedFlags;
    int SettingsOffset;
    int LastFrameActive;
    int ColumnsCount;
    int CurrentRow;
    int CurrentColumn;
    ImS16 InstanceCurrent;
    ImS16 InstanceInteracted;
    float RowPosY1;
    float RowPosY2;
    float RowMinHeight;
    float RowCellPaddingY;
    float RowTextBaseline;
    float RowIndentOffsetX;
    IMGUI_C89_EXTENSION ImGuiTableRowFlags RowFlags : 16;
    IMGUI_C89_EXTENSION ImGuiTableRowFlags LastRowFlags : 16;
    int RowBgColorCounter;
    ImU32 RowBgColor[2];
    ImU32 BorderColorStrong;
    ImU32 BorderColorLight;
    float BorderX1;
    float BorderX2;
    float HostIndentX;
    float MinColumnWidth;
    float OuterPaddingX;
    float CellPaddingX;
    float CellSpacingX1;
    float CellSpacingX2;
    float InnerWidth;
    float ColumnsGivenWidth;
    float ColumnsAutoFitWidth;
    float ColumnsStretchSumWeights;
    float ResizedColumnNextWidth;
    float ResizeLockMinContentsX2;
    float RefScale;
    float AngledHeadersHeight;
    float AngledHeadersSlope;
    ImRect OuterRect;
    ImRect InnerRect;
    ImRect WorkRect;
    ImRect InnerClipRect;
    ImRect BgClipRect;
    ImRect Bg0ClipRectForDrawCmd;
    ImRect Bg2ClipRectForDrawCmd;
    ImRect HostClipRect;
    ImRect HostBackupInnerClipRect;
    ImGuiWindow * OuterWindow;
    ImGuiWindow * InnerWindow;
    ImGuiTextBuffer ColumnsNames;
    ImDrawListSplitter * DrawSplitter;
    ImGuiTableInstanceData InstanceDataFirst;
    int InstanceDataExtraSize;
    int InstanceDataExtraCapacity;
    ImGuiTableInstanceData * InstanceDataExtra;
    ImGuiTableColumnSortSpecs SortSpecsSingle;
    /* Preserve upstream ImVector aggregate alignment. */
    int SortSpecsMultiPadding;
    int SortSpecsMultiSize;
    int SortSpecsMultiCapacity;
    ImGuiTableColumnSortSpecs * SortSpecsMulti;
    ImGuiTableSortSpecs SortSpecs;
    ImGuiTableColumnIdx SortSpecsCount;
    ImGuiTableColumnIdx ColumnsEnabledCount;
    ImGuiTableColumnIdx ColumnsEnabledFixedCount;
    ImGuiTableColumnIdx DeclColumnsCount;
    ImGuiTableColumnIdx AngledHeadersCount;
    ImGuiTableColumnIdx HoveredColumnBody;
    ImGuiTableColumnIdx HoveredColumnBorder;
    ImGuiTableColumnIdx HighlightColumnHeader;
    ImGuiTableColumnIdx AutoFitSingleColumn;
    ImGuiTableColumnIdx ResizedColumn;
    ImGuiTableColumnIdx LastResizedColumn;
    ImGuiTableColumnIdx HeldHeaderColumn;
    ImGuiTableColumnIdx LastHeldHeaderColumn;
    ImGuiTableColumnIdx ReorderColumn;
    ImGuiTableColumnIdx ReorderColumnDstOrder;
    ImGuiTableColumnIdx LeftMostEnabledColumn;
    ImGuiTableColumnIdx RightMostEnabledColumn;
    ImGuiTableColumnIdx LeftMostStretchedColumn;
    ImGuiTableColumnIdx RightMostStretchedColumn;
    ImGuiTableColumnIdx ContextPopupColumn;
    ImGuiTableColumnIdx FreezeRowsRequest;
    ImGuiTableColumnIdx FreezeRowsCount;
    ImGuiTableColumnIdx FreezeColumnsRequest;
    ImGuiTableColumnIdx FreezeColumnsCount;
    ImGuiTableColumnIdx RowCellDataCurrent;
    ImGuiTableDrawChannelIdx DummyDrawChannel;
    ImGuiTableDrawChannelIdx Bg2DrawChannelCurrent;
    ImGuiTableDrawChannelIdx Bg2DrawChannelUnfrozen;
    ImS8 NavLayer;
    unsigned char IsLayoutLocked;
    unsigned char IsInsideRow;
    unsigned char IsInitializing;
    unsigned char IsReconcileMode;
    unsigned char IsSortSpecsDirty;
    unsigned char IsUsingHeaders;
    unsigned char IsContextPopupOpen;
    unsigned char DisableDefaultContextMenu;
    unsigned char IsSettingsRequestLoad;
    unsigned char IsSettingsDirty;
    unsigned char IsDefaultDisplayOrder;
    unsigned char IsDefaultVisibility;
    unsigned char IsResetAllRequest;
    unsigned char IsResetDisplayOrderRequest;
    unsigned char IsResetVisibilityRequest;
    unsigned char IsUnfrozenRows;
    unsigned char IsDefaultSizingPolicy;
    unsigned char IsActiveIdAliveBeforeTable;
    unsigned char IsActiveIdInTable;
    unsigned char HasScrollbarYCurr;
    unsigned char HasScrollbarYPrev;
    unsigned char MemoryCompacted;
    unsigned char HostSkipItems;
};

struct ImGuiWindowTempData {
    ImVec2 CursorPos;
    ImVec2 CursorPosPrevLine;
    ImVec2 CursorStartPos;
    ImVec2 CursorMaxPos;
    ImVec2 IdealMaxPos;
    ImVec2 CurrLineSize;
    ImVec2 PrevLineSize;
    float CurrLineTextBaseOffset;
    float PrevLineTextBaseOffset;
    unsigned char IsSameLine;
    unsigned char IsSetPos;
    ImVec1 Indent;
    ImVec1 ColumnsOffset;
    ImVec1 GroupOffset;
    ImVec2 CursorStartPosLossyness;
    ImGuiNavLayer NavLayerCurrent;
    short NavLayersActiveMask;
    short NavLayersActiveMaskNext;
    unsigned char NavIsScrollPushableX;
    unsigned char NavHideHighlightOneFrame;
    unsigned char NavWindowHasScrollY;
    unsigned char MenuBarAppending;
    ImVec2 MenuBarOffset;
    ImGuiMenuColumns MenuColumns;
    int TreeDepth;
    ImU32 TreeHasStackDataDepthMask;
    ImU32 TreeRecordsClippedNodesY2Mask;
    ImVector_ImGuiWindow_ptr ChildWindows;
    ImGuiStorage * StateStorage;
    ImGuiOldColumns * CurrentColumns;
    int CurrentTableIdx;
    ImGuiLayoutType LayoutType;
    ImGuiLayoutType ParentLayoutType;
    ImU32 ModalDimBgColor;
    ImGuiItemStatusFlags WindowItemStatusFlags;
    ImGuiItemStatusFlags ChildItemStatusFlags;
    float ItemWidth;
    float ItemWidthDefault;
    float TextWrapPos;
    ImVector_float ItemWidthStack;
    ImVector_float TextWrapPosStack;
};

struct ImVector_int {
    int Size;
    int Capacity;
    int * Data;
};

struct ImGuiTextIndex {
    ImVector_int Offsets;
    int EndOffset;
};

struct ImVector_stbrp_node {
    int Size;
    int Capacity;
    stbrp_node * Data;
};

struct ImBitVector {
    ImVector_unsigned_int Storage;
};

struct ImGuiContext {
    unsigned char Initialized;
    unsigned char WithinFrameScope;
    unsigned char WithinFrameScopeWithImplicitWindow;
    unsigned char TestEngineHookItems;
    int FrameCount;
    int FrameCountEnded;
    int FrameCountRendered;
    double Time;
    char ContextName[16];
    ImGuiIO IO;
    ImGuiPlatformIO PlatformIO;
    ImGuiStyle Style;
    ImVector_ImFontAtlas_ptr FontAtlases;
    ImFont * Font;
    ImFontBaked * FontBaked;
    float FontSize;
    float FontSizeBase;
    float FontBakedScale;
    float FontRasterizerDensity;
    float CurrentDpiScale;
    ImDrawListSharedData DrawListSharedData;
    ImGuiID WithinEndChildID;
    ImGuiID WithinEndPopupID;
    void * TestEngine;
    ImVector_ImGuiInputEvent InputEventsQueue;
    ImVector_ImGuiInputEvent InputEventsTrail;
    ImGuiMouseSource InputEventsNextMouseSource;
    ImU32 InputEventsNextEventId;
    ImVector_ImGuiWindow_ptr Windows;
    ImVector_ImGuiWindow_ptr WindowsFocusOrder;
    ImVector_ImGuiWindow_ptr WindowsTempSortBuffer;
    ImVector_ImGuiWindowStackData CurrentWindowStack;
    ImGuiStorage WindowsById;
    int WindowsActiveCount;
    float WindowsBorderHoverPadding;
    ImGuiID DebugBreakInWindow;
    ImGuiWindow * CurrentWindow;
    ImGuiWindow * HoveredWindow;
    ImGuiWindow * HoveredWindowUnderMovingWindow;
    ImGuiWindow * HoveredWindowBeforeClear;
    ImGuiWindow * MovingWindow;
    ImGuiWindow * WheelingWindow;
    ImVec2 WheelingWindowRefMousePos;
    int WheelingWindowStartFrame;
    int WheelingWindowScrolledFrame;
    float WheelingWindowReleaseTimer;
    ImVec2 WheelingWindowWheelRemainder;
    ImVec2 WheelingAxisAvg;
    ImGuiID DebugDrawIdConflictsId;
    ImGuiID DebugHookIdInfoId;
    ImGuiID HoveredId;
    ImGuiID HoveredIdPreviousFrame;
    int HoveredIdPreviousFrameItemCount;
    float HoveredIdTimer;
    float HoveredIdNotActiveTimer;
    unsigned char HoveredIdAllowOverlap;
    unsigned char HoveredIdIsDisabled;
    unsigned char ItemUnclipByLog;
    unsigned char AnyIdHasBeenEditedThisFrame;
    ImGuiID ActiveId;
    ImGuiID ActiveIdIsAlive;
    float ActiveIdTimer;
    unsigned char ActiveIdIsJustActivated;
    unsigned char ActiveIdWasSelected;
    unsigned char ActiveIdWasSoleSelected;
    unsigned char ActiveIdAllowOverlap;
    unsigned char ActiveIdNoClearOnFocusLoss;
    unsigned char ActiveIdHasBeenPressedBefore;
    unsigned char ActiveIdHasBeenEditedBefore;
    unsigned char ActiveIdHasBeenEditedThisFrame;
    unsigned char ActiveIdFromShortcut;
    ImS8 ActiveIdMouseButton;
    ImGuiID ActiveIdDisabledId;
    ImVec2 ActiveIdClickOffset;
    ImGuiInputSource ActiveIdSource;
    ImGuiWindow * ActiveIdWindow;
    ImGuiID ActiveIdPreviousFrame;
    ImGuiDeactivatedItemData DeactivatedItemData;
    ImGuiDataTypeStorage ActiveIdValueOnActivation;
    ImGuiID LastActiveId;
    float LastActiveIdTimer;
    unsigned char LastActiveIdWasSelected;
    unsigned char LastActiveIdWasSoleSelected;
    double LastKeyModsChangeTime;
    double LastKeyModsChangeFromNoneTime;
    double LastKeyboardKeyPressTime;
    ImBitArray_155_512 KeysMayBeCharInput;
    ImGuiKeyOwnerData KeysOwnerData[155];
    ImGuiKeyRoutingTable KeysRoutingTable;
    ImU32 ActiveIdUsingNavDirMask;
    unsigned char ActiveIdUsingAllKeyboardKeys;
    ImGuiKeyChord DebugBreakInShortcutRouting;
    ImGuiID CurrentFocusScopeId;
    ImGuiItemFlags CurrentItemFlags;
    ImGuiID DebugLocateId;
    ImGuiNextItemData NextItemData;
    ImGuiLastItemData LastItemData;
    ImGuiNextWindowData NextWindowData;
    unsigned char DebugShowGroupRects;
    unsigned char GcCompactAll;
    ImGuiCol DebugFlashStyleColorIdx;
    ImVector_ImGuiColorMod ColorStack;
    ImVector_ImGuiStyleMod StyleVarStack;
    ImVector_ImFontStackData FontStack;
    ImVector_ImGuiFocusScopeData FocusScopeStack;
    ImVector_int ItemFlagsStack;
    ImVector_ImGuiGroupData GroupStack;
    ImVector_ImGuiPopupData OpenPopupStack;
    ImVector_ImGuiPopupData BeginPopupStack;
    ImVector_ImGuiTreeNodeStackData TreeNodeStack;
    ImVector_ImGuiViewportP_ptr Viewports;
    unsigned char NavCursorVisible;
    unsigned char NavHighlightItemUnderNav;
    unsigned char NavMousePosDirty;
    unsigned char NavIdIsAlive;
    ImGuiID NavId;
    ImGuiWindow * NavWindow;
    ImGuiID NavFocusScopeId;
    ImGuiNavLayer NavLayer;
    ImGuiItemFlags NavIdItemFlags;
    ImGuiID NavActivateId;
    ImGuiID NavActivateDownId;
    ImGuiID NavActivatePressedId;
    ImGuiActivateFlags NavActivateFlags;
    ImVector_ImGuiFocusScopeData NavFocusRoute;
    ImGuiID NavHighlightActivatedId;
    float NavHighlightActivatedTimer;
    ImGuiID NavOpenContextMenuItemId;
    ImGuiID NavOpenContextMenuWindowId;
    ImGuiID NavNextActivateId;
    ImGuiActivateFlags NavNextActivateFlags;
    ImGuiInputSource NavInputSource;
    ImGuiSelectionUserData NavLastValidSelectionUserData;
    ImS8 NavCursorHideFrames;
    unsigned char NavAnyRequest;
    unsigned char NavInitRequest;
    unsigned char NavInitRequestFromMove;
    ImGuiNavItemData NavInitResult;
    unsigned char NavMoveSubmitted;
    unsigned char NavMoveScoringItems;
    unsigned char NavMoveForwardToNextFrame;
    ImGuiNavMoveFlags NavMoveFlags;
    ImGuiScrollFlags NavMoveScrollFlags;
    ImGuiKeyChord NavMoveKeyMods;
    ImGuiDir NavMoveDir;
    ImGuiDir NavMoveDirForDebug;
    ImGuiDir NavMoveClipDir;
    ImRect NavScoringRect;
    ImRect NavScoringNoClipRect;
    int NavScoringDebugCount;
    int NavTabbingDir;
    int NavTabbingCounter;
    ImGuiNavItemData NavMoveResultLocal;
    ImGuiNavItemData NavMoveResultLocalVisible;
    ImGuiNavItemData NavMoveResultOther;
    ImGuiNavItemData NavTabbingResultFirst;
    ImGuiID NavJustMovedFromFocusScopeId;
    ImGuiID NavJustMovedToId;
    ImGuiID NavJustMovedToFocusScopeId;
    ImGuiKeyChord NavJustMovedToKeyMods;
    unsigned char NavJustMovedToIsTabbing;
    unsigned char NavJustMovedToHasSelectionData;
    unsigned char ConfigNavEnableTabbing;
    unsigned char ConfigNavWindowingWithGamepad;
    ImGuiKeyChord ConfigNavWindowingKeyNext;
    ImGuiKeyChord ConfigNavWindowingKeyPrev;
    ImGuiWindow * NavWindowingTarget;
    ImGuiWindow * NavWindowingTargetAnim;
    ImGuiWindow * NavWindowingListWindow;
    float NavWindowingTimer;
    float NavWindowingHighlightAlpha;
    ImGuiInputSource NavWindowingInputSource;
    unsigned char NavWindowingToggleLayer;
    ImGuiKey NavWindowingToggleKey;
    ImVec2 NavWindowingAccumDeltaPos;
    ImVec2 NavWindowingAccumDeltaSize;
    float DimBgRatio;
    unsigned char DragDropActive;
    unsigned char DragDropWithinSource;
    unsigned char DragDropWithinTarget;
    ImGuiDragDropFlags DragDropSourceFlags;
    int DragDropSourceFrameCount;
    int DragDropMouseButton;
    ImGuiPayload DragDropPayload;
    ImRect DragDropTargetRect;
    ImRect DragDropTargetClipRect;
    ImGuiID DragDropTargetId;
    ImGuiID DragDropTargetFullViewport;
    ImGuiDragDropFlags DragDropAcceptFlagsCurr;
    ImGuiDragDropFlags DragDropAcceptFlagsPrev;
    float DragDropAcceptIdCurrRectSurface;
    ImGuiID DragDropAcceptIdCurr;
    ImGuiID DragDropAcceptIdPrev;
    int DragDropAcceptFrameCount;
    ImGuiID DragDropHoldJustPressedId;
    ImVector_unsigned_char DragDropPayloadBufHeap;
    unsigned char DragDropPayloadBufLocal[16];
    int ClipperTempDataStacked;
    ImVector_ImGuiListClipperData ClipperTempData;
    ImGuiTable * CurrentTable;
    ImGuiID DebugBreakInTable;
    int TablesTempDataStacked;
    int TablesTempDataSize;
    int TablesTempDataCapacity;
    ImGuiTableTempData * TablesTempData;
    ImGuiTablePool Tables;
    ImVector_float TablesLastTimeActive;
    ImVector_ImDrawChannel DrawChannelsTempMergeBuffer;
    ImGuiTabBar * CurrentTabBar;
    ImPool_ImGuiTabBar TabBars;
    ImVector_ImGuiPtrOrIndex CurrentTabBarStack;
    ImVector_ImGuiShrinkWidthItem ShrinkWidthBuffer;
    ImGuiBoxSelectState BoxSelectState;
    ImGuiMultiSelectTempData * CurrentMultiSelect;
    int MultiSelectTempDataStacked;
    ImVector_ImGuiMultiSelectTempData MultiSelectTempData;
    ImPool_ImGuiMultiSelectState MultiSelectStorage;
    ImGuiID HoverItemDelayId;
    ImGuiID HoverItemDelayIdPreviousFrame;
    float HoverItemDelayTimer;
    float HoverItemDelayClearTimer;
    ImGuiID HoverItemUnlockedStationaryId;
    ImGuiID HoverWindowUnlockedStationaryId;
    ImGuiMouseCursor MouseCursor;
    float MouseStationaryTimer;
    ImVec2 MouseLastValidPos;
    ImGuiInputTextState InputTextState;
    ImGuiTextIndex InputTextLineIndex;
    ImGuiInputTextDeactivatedState InputTextDeactivatedState;
    ImFontBaked InputTextPasswordFontBackupBaked;
    ImFontFlags InputTextPasswordFontBackupFlags;
    ImGuiID InputTextReactivateId;
    ImGuiID TempInputId;
    ImGuiDataTypeStorage DataTypeZeroValue;
    int BeginMenuDepth;
    int BeginComboDepth;
    ImGuiID ColorEditCurrentID;
    ImGuiID ColorEditSavedID;
    float ColorEditSavedHue;
    float ColorEditSavedSat;
    ImU32 ColorEditSavedColor;
    ImVec4 ColorPickerRef;
    ImGuiComboPreviewData ComboPreviewData;
    ImRect WindowResizeBorderExpectedRect;
    unsigned char WindowResizeRelativeMode;
    short ScrollbarSeekMode;
    float ScrollbarClickDeltaToGrabCenter;
    float SliderGrabClickOffset;
    float SliderCurrentAccum;
    unsigned char SliderCurrentAccumDirty;
    unsigned char DragCurrentAccumDirty;
    float DragCurrentAccum;
    float DragSpeedDefaultRatio;
    float DisabledAlphaBackup;
    short DisabledStackSize;
    short TooltipOverrideCount;
    ImGuiWindow * TooltipPreviousWindow;
    ImVector_char ClipboardHandlerData;
    ImVector_unsigned_int MenusIdSubmittedThisFrame;
    ImGuiTypingSelectState TypingSelectState;
    ImGuiPlatformImeData PlatformImeData;
    ImGuiPlatformImeData PlatformImeDataPrev;
    ImVector_ImTextureData_ptr UserTextures;
    ImGuiPackedDate SessionDate;
    unsigned char SettingsLoaded;
    float SettingsDirtyTimer;
    ImGuiTextBuffer SettingsIniData;
    ImVector_ImGuiSettingsHandler SettingsHandlers;
    ImChunkStream_ImGuiWindowSettings SettingsWindows;
    ImChunkStream_ImGuiTableSettings SettingsTables;
    ImVector_ImGuiContextHook Hooks;
    ImGuiID HookIdNext;
    ImGuiDemoMarkerCallback DemoMarkerCallback;
    const char * LocalizationTable[12];
    unsigned char LogEnabled;
    unsigned char LogLineFirstItem;
    ImGuiLogFlags LogFlags;
    ImGuiWindow * LogWindow;
    ImFileHandle LogFile;
    ImGuiTextBuffer LogBuffer;
    const char * LogNextPrefix;
    const char * LogNextSuffix;
    float LogLinePosY;
    int LogDepthRef;
    int LogDepthToExpand;
    int LogDepthToExpandDefault;
    ImGuiErrorCallback ErrorCallback;
    void * ErrorCallbackUserData;
    ImVec2 ErrorTooltipLockedPos;
    unsigned char ErrorFirst;
    int ErrorCountCurrentFrame;
    ImGuiErrorRecoveryState StackSizesInNewFrame;
    ImGuiErrorRecoveryState * StackSizesInBeginForCurrentWindow;
    int DebugDrawIdConflictsCount;
    ImGuiDebugLogFlags DebugLogFlags;
    ImGuiTextBuffer DebugLogBuf;
    ImGuiTextIndex DebugLogIndex;
    int DebugLogSkippedErrors;
    ImGuiDebugLogFlags DebugLogAutoDisableFlags;
    ImU8 DebugLogAutoDisableFrames;
    ImU8 DebugLocateFrames;
    unsigned char DebugBreakInLocateId;
    ImGuiKeyChord DebugBreakKeyChord;
    ImS8 DebugBeginReturnValueCullDepth;
    unsigned char DebugItemPickerActive;
    ImU8 DebugItemPickerMouseButton;
    ImGuiID DebugItemPickerBreakId;
    float DebugFlashStyleColorTime;
    ImVec4 DebugFlashStyleColorBackup;
    ImGuiMetricsConfig DebugMetricsConfig;
    ImGuiDebugItemPathQuery DebugItemPathQuery;
    ImGuiIDStackTool DebugIDStackTool;
    ImGuiDebugAllocInfo DebugAllocInfo;
    float FramerateSecPerFrame[60];
    int FramerateSecPerFrameIdx;
    int FramerateSecPerFrameCount;
    float FramerateSecPerFrameAccum;
    int WantCaptureMouseNextFrame;
    int WantCaptureKeyboardNextFrame;
    int WantTextInputNextFrame;
    ImVector_char TempBuffer;
    char TempKeychordName[64];
};

struct ImGuiWindow {
    ImGuiContext * Ctx;
    char * Name;
    ImGuiID ID;
    ImGuiWindowFlags Flags;
    ImGuiChildFlags ChildFlags;
    ImGuiViewportP * Viewport;
    ImVec2 Pos;
    ImVec2 Size;
    ImVec2 SizeFull;
    ImVec2 ContentSize;
    ImVec2 ContentSizeIdeal;
    ImVec2 ContentSizeExplicit;
    ImVec2 WindowPadding;
    float WindowRounding;
    float WindowBorderSize;
    float TitleBarHeight;
    float MenuBarHeight;
    float DecoOuterSizeX1;
    float DecoOuterSizeY1;
    float DecoOuterSizeX2;
    float DecoOuterSizeY2;
    float DecoInnerSizeX1;
    float DecoInnerSizeY1;
    int NameBufLen;
    ImGuiID MoveId;
    ImGuiID ChildId;
    ImGuiID PopupId;
    ImVec2 Scroll;
    ImVec2 ScrollMax;
    ImVec2 ScrollTarget;
    ImVec2 ScrollTargetCenterRatio;
    ImVec2 ScrollTargetEdgeSnapDist;
    ImVec2 ScrollbarSizes;
    unsigned char ScrollbarX;
    unsigned char ScrollbarY;
    unsigned char ScrollbarXStabilizeEnabled;
    ImU8 ScrollbarXStabilizeToggledHistory;
    unsigned char Active;
    unsigned char WasActive;
    unsigned char WriteAccessed;
    unsigned char Collapsed;
    unsigned char WantCollapseToggle;
    unsigned char SkipItems;
    unsigned char SkipRefresh;
    unsigned char Appearing;
    unsigned char Hidden;
    unsigned char IsFallbackWindow;
    unsigned char IsExplicitChild;
    unsigned char HasCloseButton;
    signed char ResizeBorderHovered;
    signed char ResizeBorderHeld;
    short BeginCount;
    short BeginCountPreviousFrame;
    short BeginOrderWithinParent;
    short BeginOrderWithinContext;
    short FocusOrder;
    ImGuiDir AutoPosLastDirection;
    ImS8 AutoFitFramesX;
    ImS8 AutoFitFramesY;
    unsigned char AutoFitOnlyGrows;
    ImS8 HiddenFramesCanSkipItems;
    ImS8 HiddenFramesCannotSkipItems;
    ImS8 HiddenFramesForRenderOnly;
    ImS8 DisableInputsFrames;
    IMGUI_C89_EXTENSION ImGuiWindowBgClickFlags BgClickFlags : 8;
    IMGUI_C89_EXTENSION ImGuiCond SetWindowPosAllowFlags : 8;
    IMGUI_C89_EXTENSION ImGuiCond SetWindowSizeAllowFlags : 8;
    IMGUI_C89_EXTENSION ImGuiCond SetWindowCollapsedAllowFlags : 8;
    ImVec2 SetWindowPosVal;
    ImVec2 SetWindowPosPivot;
    ImVector_unsigned_int IDStack;
    ImGuiWindowTempData DC;
    ImRect OuterRectClipped;
    ImRect InnerRect;
    ImRect InnerClipRect;
    ImRect WorkRect;
    ImRect ParentWorkRect;
    ImRect ClipRect;
    ImRect ContentRegionRect;
    ImVec2ih HitTestHoleSize;
    ImVec2ih HitTestHoleOffset;
    int LastFrameActive;
    float LastTimeActive;
    ImGuiStorage StateStorage;
    ImVector_ImGuiOldColumns ColumnsStorage;
    float FontWindowScale;
    float FontWindowScaleParents;
    float FontRefSize;
    int SettingsOffset;
    ImDrawList * DrawList;
    ImDrawList DrawListInst;
    ImGuiWindow * ParentWindow;
    ImGuiWindow * ParentWindowInBeginStack;
    ImGuiWindow * RootWindow;
    ImGuiWindow * RootWindowPopupTree;
    ImGuiWindow * RootWindowForTitleBarHighlight;
    ImGuiWindow * RootWindowForNav;
    ImGuiWindow * ParentWindowForFocusRoute;
    ImGuiWindow * NavLastChildNavWindow;
    ImGuiID NavLastIds[2];
    ImRect NavRectRel[2];
    ImVec2 NavPreferredScoringPosRel[2];
    ImGuiID NavRootFocusScopeId;
    int MemoryDrawListIdxCapacity;
    int MemoryDrawListVtxCapacity;
    unsigned char MemoryCompacted;
};

struct KeyLayoutData {
    int Row;
    int Col;
    const char * Label;
    ImGuiKey Key;
};

struct MergeGroup {
    ImRect ClipRect;
    int ChannelsCount;
    ImBitArrayPtr ChannelsMask;
};

struct ScopedHighlightOlderThan {
    unsigned char Highlight;
};

struct imgui_c89_anon_imstb_rectpack_341_9 {
    int x;
    int y;
    stbrp_node ** prev_link;
};

struct stbrp_context_opaque {
    char data[80];
};

struct ImFontAtlasBuilder {
    stbrp_context_opaque PackContext;
    ImVector_stbrp_node PackNodes;
    ImVector_ImTextureRect Rects;
    ImVector_ImFontAtlasRectEntry RectsIndex;
    ImVector_unsigned_char TempBuffer;
    int RectsIndexFreeListStart;
    int RectsPackedCount;
    int RectsPackedSurface;
    int RectsDiscardedCount;
    int RectsDiscardedSurface;
    int FrameCount;
    ImVec2i MaxRectSize;
    ImVec2i MaxRectBounds;
    unsigned char LockDisableResize;
    unsigned char PreloadedAllGlyphsRanges;
    ImStableVector_ImFontBaked_32 BakedPool;
    ImGuiStorage BakedMap;
    int BakedDiscardedCount;
    ImFontAtlasRectId PackIdMouseCursors;
    ImFontAtlasRectId PackIdLinesTexData;
};

struct stbrp_node {
    stbrp_coord x;
    stbrp_coord y;
    stbrp_node * next;
};

struct stbrp_context {
    int width;
    int height;
    int align;
    int init_mode;
    int heuristic;
    int num_nodes;
    stbrp_node * active_head;
    stbrp_node * free_head;
    stbrp_node extra[2];
};

struct stbrp_rect {
    int id;
    stbrp_coord w;
    stbrp_coord h;
    stbrp_coord x;
    stbrp_coord y;
    int was_packed;
};

struct stbtt_active_edge {
    struct stbtt_active_edge * next;
    float fx;
    float fdx;
    float fdy;
    float direction;
    float sy;
    float ey;
};

struct imgui_c89_anon_imstb_truetype_929_9 {
    int w;
    int h;
    int stride;
    unsigned char * pixels;
};

struct imgui_c89_anon_imstb_truetype_518_9 {
    unsigned char * data;
    int cursor;
    int size;
};

struct imgui_c89_anon_imstb_truetype_1902_9 {
    int bounds;
    int started;
    float first_x;
    float first_y;
    float x;
    float y;
    stbtt_int32 min_x;
    stbtt_int32 max_x;
    stbtt_int32 min_y;
    stbtt_int32 max_y;
    imgui_c89_anon_imstb_truetype_840_12 * pvertices;
    int num_vertices;
};

struct stbtt_edge {
    float x0;
    float y0;
    float x1;
    float y1;
    int invert;
};

struct stbtt_hheap {
    struct stbtt_hheap_chunk * head;
    void * first_free;
    int num_remaining_in_head_chunk;
};

struct stbtt_hheap_chunk {
    struct stbtt_hheap_chunk * next;
};

struct imgui_c89_anon_imstb_truetype_3493_9 {
    float x;
    float y;
};

struct imgui_c89_anon_imstb_truetype_548_9 {
    float x0;
    float y0;
    float s0;
    float t0;
    float x1;
    float y1;
    float s1;
    float t1;
};

struct imgui_c89_anon_imstb_truetype_532_9 {
    unsigned short x0;
    unsigned short y0;
    unsigned short x1;
    unsigned short y1;
    float xoff;
    float yoff;
    float xadvance;
};

struct stbtt_fontinfo {
    void * userdata;
    unsigned char * data;
    int fontstart;
    int numGlyphs;
    int loca;
    int head;
    int glyf;
    int hhea;
    int hmtx;
    int kern;
    int gpos;
    int svg;
    int index_map;
    int indexToLocFormat;
    imgui_c89_anon_imstb_truetype_518_9 cff;
    imgui_c89_anon_imstb_truetype_518_9 charstrings;
    imgui_c89_anon_imstb_truetype_518_9 gsubrs;
    imgui_c89_anon_imstb_truetype_518_9 subrs;
    imgui_c89_anon_imstb_truetype_518_9 fontdicts;
    imgui_c89_anon_imstb_truetype_518_9 fdselect;
};

struct ImGui_ImplStbTrueType_FontSrcData {
    stbtt_fontinfo FontInfo;
    float ScaleFactor;
};

struct stbtt_kerningentry {
    int glyph1;
    int glyph2;
    int advance;
};

struct stbtt_pack_context {
    void * user_allocator_context;
    void * pack_info;
    int width;
    int height;
    int stride_in_bytes;
    int padding;
    int skip_missing;
    unsigned int h_oversample;
    unsigned int v_oversample;
    unsigned char * pixels;
    void * nodes;
};

struct imgui_c89_anon_imstb_truetype_624_9 {
    float font_size;
    int first_unicode_codepoint_in_range;
    int * array_of_unicode_codepoints;
    int num_chars;
    imgui_c89_anon_imstb_truetype_580_9 * chardata_for_range;
    unsigned char h_oversample;
    unsigned char v_oversample;
};

struct imgui_c89_anon_imstb_truetype_580_9 {
    unsigned short x0;
    unsigned short y0;
    unsigned short x1;
    unsigned short y1;
    float xoff;
    float yoff;
    float xadvance;
    float xoff2;
    float yoff2;
};

struct imgui_c89_anon_imstb_truetype_840_12 {
    short x;
    short y;
    short cx;
    short cy;
    short cx1;
    short cy1;
    unsigned char type;
    unsigned char padding;
};

int ImAbs__e30e4b582c(int x);
double ImAbs__b997a4f302(double x);
float ImAbs__34413c361e(float x);
ImU32 imgui__im_alpha_blend_colors(ImU32 col_a, ImU32 col_b);
ImVec2 imgui__im_bezier_cubic_calc(const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, float t);
ImVec2 imgui__im_bezier_cubic_closest_point(const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, const ImVec2 * p, int num_segments);
ImVec2 imgui__im_bezier_cubic_closest_point_casteljau(const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, const ImVec2 * p, float tess_tol);
ImVec2 imgui__im_bezier_quadratic_calc(const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, float t);
void ImBitArrayClearAllBits__0e7c4d256e(ImU32 * arr, int bitcount);
size_t ImBitArrayGetStorageSizeInBytes__ac1b7b5bd0(int bitcount);
void ImBitArraySetBit__e371039a31(ImU32 * arr, int n);
unsigned char ImCharIsBlankA__64174024a3(char c);
unsigned char ImCharIsBlankW__848c59161d(unsigned int c);
ImVec2 ImClamp__f0577074b0(const ImVec2 * v, const ImVec2 * mn, const ImVec2 * mx);
ImVec2 ImClamp__8dce10afbc(ImVec2 v, ImVec2 mn, ImVec2 mx);
int ImClamp__2e3747f496(int v, int mn, int mx);
float ImClamp__36c731a202(float v, float mn, float mx);
unsigned char imgui__im_file_close(ImFileHandle f);
ImU64 imgui__im_file_get_size(ImFileHandle f);
void * imgui__im_file_load_to_memory(ImGuiContext *imgui_c89_ctx, const char * filename, const char * mode, size_t * out_file_size, int padding_bytes);
ImFileHandle imgui__im_file_open(const char * filename, const char * mode);
ImU64 imgui__im_file_read(void * data, ImU64 sz, ImU64 count, ImFileHandle f);
ImU64 imgui__im_file_write(const void * data, ImU64 sz, ImU64 count, ImFileHandle f);
float ImFloor__5652c8955b(float f);
void imgui__im_font_atlas_add_draw_list_shared_data(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImDrawListSharedData * data);
ImFontBaked * imgui__im_font_atlas_baked_add(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFont * font, float font_size, float font_rasterizer_density, ImGuiID baked_id);
ImFontGlyph * imgui__im_font_atlas_baked_add_font_glyph(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFontBaked * baked, ImFontConfig * src, const ImFontGlyph * in_glyph);
void imgui__im_font_atlas_baked_add_font_glyph_advanced_x(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFontBaked * baked, ImFontConfig * src, ImWchar codepoint, float advance_x);
void imgui__im_font_atlas_baked_discard(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFont * font, ImFontBaked * baked);
void imgui__im_font_atlas_baked_discard_font_glyph(ImFontAtlas * atlas, ImFont * font, ImFontBaked * baked, ImFontGlyph * glyph);
ImFontBaked * imgui__im_font_atlas_baked_get_closest_match(ImFontAtlas * atlas, ImFont * font, float font_size, float font_rasterizer_density);
ImGuiID imgui__im_font_atlas_baked_get_id(ImGuiID font_id, float baked_size, float rasterizer_density);
ImFontBaked * imgui__im_font_atlas_baked_get_or_add(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFont * font, float font_size, float font_rasterizer_density);
void imgui__im_font_atlas_baked_set_font_glyph_bitmap(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFontBaked * baked, ImFontConfig * src, ImFontGlyph * glyph, ImTextureRect * r, const unsigned char * src_pixels, ImTextureFormat src_fmt, int src_pitch);
void imgui__im_font_atlas_build_clear(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
void imgui__im_font_atlas_build_destroy(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
void imgui__im_font_atlas_build_discard_bakes(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, int unused_frames);
void imgui__im_font_atlas_build_get_oversample_factors(ImFontConfig * src, ImFontBaked * baked, int * out_oversample_h, int * out_oversample_v);
void imgui__im_font_atlas_build_init(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
void imgui__im_font_atlas_build_legacy_preload_all_glyph_ranges(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
void imgui__im_font_atlas_build_main(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
void imgui__im_font_atlas_build_notify_set_font(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFont * old_font, ImFont * new_font);
void imgui__im_font_atlas_build_render_bitmap_from_string(ImFontAtlas * atlas, int x, int y, int w, int h, const char * in_str, char in_marker_char);
void imgui__im_font_atlas_build_setup_font_loader(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, const ImFontLoader * font_loader);
void imgui__im_font_atlas_build_setup_font_special_glyphs(ImFontAtlas * atlas, ImFont * font, ImFontConfig * src);
void imgui__im_font_atlas_build_update_pointers(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
void imgui__im_font_atlas_debug_log_texture_requests(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
void imgui__im_font_atlas_font_destroy_output(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFont * font);
void imgui__im_font_atlas_font_destroy_source_data(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFontConfig * src);
void imgui__im_font_atlas_font_discard_bakes(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFont * font, int unused_frames);
unsigned char imgui__im_font_atlas_font_init_output(ImFontAtlas * atlas, ImFont * font);
void imgui__im_font_atlas_font_rebuild_output(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFont * font);
void imgui__im_font_atlas_font_source_add_to_font(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFont * font, ImFontConfig * src);
unsigned char imgui__im_font_atlas_font_source_init(ImFontAtlas * atlas, ImFontConfig * src);
const ImFontLoader * imgui__im_font_atlas_get_font_loader_for_stb_truetype(void);
unsigned char imgui__im_font_atlas_get_mouse_cursor_tex_data(ImFontAtlas * atlas, ImGuiMouseCursor cursor_type, ImVec2 * out_offset, ImVec2 * out_size, ImVec2 * out_uv_border, ImVec2 * out_uv_fill);
ImFontAtlasRectId imgui__im_font_atlas_pack_add_rect(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, int w, int h, ImFontAtlasRectEntry * overwrite_entry);
void imgui__im_font_atlas_pack_discard_rect(ImFontAtlas * atlas, ImFontAtlasRectId id);
ImTextureRect * imgui__im_font_atlas_pack_get_rect(ImFontAtlas * atlas, ImFontAtlasRectId id);
ImTextureRect * imgui__im_font_atlas_pack_get_rect_safe(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImFontAtlasRectId id);
void imgui__im_font_atlas_pack_init(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
unsigned int ImFontAtlasRectId_GetGeneration__0410b48430(ImFontAtlasRectId id);
int ImFontAtlasRectId_GetIndex__dda34df082(ImFontAtlasRectId id);
ImFontAtlasRectId ImFontAtlasRectId_Make__76050ffe54(int index_idx, int gen_idx);
void imgui__im_font_atlas_remove_draw_list_shared_data(ImFontAtlas * atlas, ImDrawListSharedData * data);
ImTextureData * imgui__im_font_atlas_texture_add(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, int w, int h);
void imgui__im_font_atlas_texture_block_convert(const unsigned char * src_pixels, ImTextureFormat src_fmt, int src_pitch, unsigned char * dst_pixels, ImTextureFormat dst_fmt, int dst_pitch, int w, int h);
void imgui__im_font_atlas_texture_block_copy(ImTextureData * src_tex, int src_x, int src_y, ImTextureData * dst_tex, int dst_x, int dst_y, int w, int h);
void imgui__im_font_atlas_texture_block_fill(ImTextureData * dst_tex, int dst_x, int dst_y, int w, int h, ImU32 col);
void imgui__im_font_atlas_texture_block_post_process(ImFontAtlasPostProcessData * data);
void imgui__im_font_atlas_texture_block_post_process_multiply(ImFontAtlasPostProcessData * data, float multiply_factor);
void imgui__im_font_atlas_texture_block_queue_upload(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImTextureData * tex, int x, int y, int w, int h);
void imgui__im_font_atlas_texture_compact(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
ImVec2i imgui__im_font_atlas_texture_get_size_estimate(ImFontAtlas * atlas);
void imgui__im_font_atlas_texture_grow(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, int old_tex_w, int old_tex_h);
void imgui__im_font_atlas_texture_make_space(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
void imgui__im_font_atlas_texture_repack(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, int w, int h);
void imgui__im_font_atlas_update_draw_lists_shared_data(ImFontAtlas * atlas);
void imgui__im_font_atlas_update_draw_lists_textures(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, ImTextureRef old_tex, ImTextureRef new_tex);
void imgui__im_font_atlas_update_new_frame(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas, int frame_count, unsigned char renderer_has_textures);
ImVec2 imgui__im_font_calc_text_size_ex(ImGuiContext *imgui_c89_ctx, ImFont * font, float size, float max_width, float wrap_width, const char * text_begin, const char * text_end_display, const char * text_end, const char ** out_remaining, ImVec2 * out_offset, ImDrawTextFlags flags);
const char * imgui__im_font_calc_word_wrap_position_ex(ImGuiContext *imgui_c89_ctx, ImFont * font, float size, const char * text, const char * text_end, float wrap_width, ImDrawTextFlags flags);
int imgui__im_format_string(char * buf, size_t buf_size, const char * fmt, ...);
void imgui__im_format_string_to_temp_buffer(ImGuiContext *imgui_c89_ctx, const char ** out_buf, const char ** out_buf_end, const char * fmt, ...);
void imgui__im_format_string_to_temp_buffer_v(ImGuiContext *imgui_c89_ctx, const char ** out_buf, const char ** out_buf_end, const char * fmt, va_list args);
int imgui__im_format_string_v(char * buf, size_t buf_size, const char * fmt, va_list args);
ImGuiID imgui__im_hash_data(const void * data_p, size_t data_size, ImGuiID seed);
const char * imgui__im_hash_skip_uncontributing_prefix(const char * label);
ImGuiID imgui__im_hash_str(const char * data_p, size_t data_size, ImGuiID seed);
unsigned char ImIsPowerOfTwo__988b9b7202(int v);
float ImLengthSqr__a1d968fcc5(const ImVec2 * lhs);
ImVec2 ImLerp__5412da1300(const ImVec2 * a, const ImVec2 * b, const ImVec2 * t);
ImVec2 ImLerp__622596b98b(const ImVec2 * a, const ImVec2 * b, float t);
ImVec4 ImLerp__4129e9c45f(const ImVec4 * a, const ImVec4 * b, float t);
ImVec2 ImLerp__dd89496925(ImVec2 a, ImVec2 b, float t);
ImVec4 ImLerp__e915113587(ImVec4 a, ImVec4 b, float t);
int ImLerp__e5f6555a3d(int a, int b, float t);
float ImLerp__95dd708a82(float a, float b, float t);
ImVec2 imgui__im_line_closest_point(const ImVec2 * a, const ImVec2 * b, const ImVec2 * p);
float ImLinearRemapClamp__4b5b197d8b(float s0, float s1, float d0, float d1, float x);
float ImLinearSweep__8883b7bc03(float current, float target, float speed);
double ImLog__be4df5b999(double x);
float ImLog__8ae734901e(float x);
ImGuiStoragePair * imgui__im_lower_bound(ImGuiStoragePair * in_begin, ImGuiStoragePair * in_end, ImGuiID key);
ImVec2 ImMax__6e9fc4176a(const ImVec2 * lhs, const ImVec2 * rhs);
ImVec2 ImMax__19ddae5de5(ImVec2 lhs, ImVec2 rhs);
int ImMax__55d6006f6c(int lhs, int rhs);
float ImMax__3c7b1bb7d1(float lhs, float rhs);
void * imgui__im_memdup(ImGuiContext *imgui_c89_ctx, const void * src, size_t size);
ImVec2 ImMin__51efb64ad1(const ImVec2 * lhs, const ImVec2 * rhs);
ImVec2 ImMin__8f9edb5b68(ImVec2 lhs, ImVec2 rhs);
int ImMin__16ca668bb0(int lhs, int rhs);
float ImMin__f04263da73(float lhs, float rhs);
unsigned int ImMin__e0a9b02c47(unsigned int lhs, unsigned int rhs);
unsigned long ImMin__ba5a9f750f(unsigned long lhs, unsigned long rhs);
const char * imgui__im_parse_format_find_end(const char * fmt);
const char * imgui__im_parse_format_find_start(const char * fmt);
int imgui__im_parse_format_precision(const char * fmt, int default_precision);
void imgui__im_parse_format_sanitize_for_printing(const char * fmt_in, char * fmt_out, size_t fmt_out_size);
const char * imgui__im_parse_format_sanitize_for_scanning(const char * fmt_in, char * fmt_out, size_t fmt_out_size);
const char * imgui__im_parse_format_trim_decorations(const char * fmt, char * buf, size_t buf_size);
double ImPow__0fceb59920(double x, double y);
float ImPow__e621e1814f(float x, float y);
void ImQsort__c5a9d8474e(void * base, size_t count, size_t size_of_element, int (*compare_func)(const void *, const void *));
ImVec2 ImRotate__cc154588bb(const ImVec2 * v, float cos_a, float sin_a);
float ImRsqrt__97e21295fc(float x);
float ImSaturate__5281e00f2e(float f);
float ImSign__f9f863f978(float x);
const char * imgui__im_str_skip_blank(const char * str);
void imgui__im_str_trim_blanks(char * buf);
const char * imgui__im_strbol(const char * buf_mid_line, const char * buf_begin);
const char * imgui__im_strchr_range(const char * str, const char * str_end, char c);
char * imgui__im_strdup(ImGuiContext *imgui_c89_ctx, const char * str);
char * imgui__im_strdupcpy(ImGuiContext *imgui_c89_ctx, char * dst, size_t * p_dst_size, const char * src);
const char * imgui__im_streol_range(const char * str, const char * str_end);
int imgui__im_stricmp(const char * str1, const char * str2);
const char * imgui__im_stristr(const char * haystack, const char * haystack_end, const char * needle, const char * needle_end);
int imgui__im_strlen_w(const ImWchar * str);
void imgui__im_strncpy(char * dst, const char * src, size_t count);
int imgui__im_strnicmp(const char * str1, const char * str2, size_t count);
void ImSwap__3e5ab29a0d(float * a, float * b);
const char * imgui__im_text_calc_word_wrap_next_line_start(const char * text, const char * text_end, ImDrawTextFlags flags);
int imgui__im_text_char_from_utf8(unsigned int * out_char, const char * in_text, const char * in_text_end);
int imgui__im_text_char_to_utf8(char * out_buf, unsigned int c);
void imgui__im_text_classifier_clear(ImU32 * bits, unsigned int codepoint_min, unsigned int codepoint_end, ImWcharClass char_class);
void imgui__im_text_classifier_set_char_class(ImU32 * bits, unsigned int codepoint_min, unsigned int codepoint_end, ImWcharClass char_class, unsigned int c);
void imgui__im_text_classifier_set_char_class_from_str(ImU32 * bits, unsigned int codepoint_min, unsigned int codepoint_end, ImWcharClass char_class, const char * s);
int imgui__im_text_count_chars_from_utf8(const char * in_text, const char * in_text_end);
int imgui__im_text_count_lines(const char * in_text, const char * in_text_end);
int imgui__im_text_count_utf8_bytes_from_char(const char * in_text, const char * in_text_end);
int imgui__im_text_count_utf8_bytes_from_str(const ImWchar * in_text, const ImWchar * in_text_end);
const char * imgui__im_text_find_previous_utf8_codepoint(const char * in_text_start, const char * in_p);
const char * imgui__im_text_find_valid_utf8_codepoint_end(const char * in_text_start, const char * in_text_end, const char * in_p);
void imgui__im_text_init_classifiers(void);
int imgui__im_text_str_from_utf8(ImWchar * buf, int buf_size, const char * in_text, const char * in_text_end, const char ** in_text_remaining);
int imgui__im_text_str_to_utf8(char * out_buf, int out_buf_size, const ImWchar * in_text, const ImWchar * in_text_end);
int imgui__im_texture_data_get_format_bytes_per_pixel(ImTextureFormat format);
const char * imgui__im_texture_data_get_format_name(ImTextureFormat format);
const char * imgui__im_texture_data_get_status_name(ImTextureStatus status);
void imgui__im_texture_data_queue_upload(ImGuiContext *imgui_c89_ctx, ImTextureData * tex, int x, int y, int w, int h);
unsigned char imgui__im_texture_data_update_new_frame(ImGuiContext *imgui_c89_ctx, ImTextureData * tex);
char ImToUpper__176018ea7f(char c);
void imgui__im_triangle_barycentric_coords(const ImVec2 * a, const ImVec2 * b, const ImVec2 * c, const ImVec2 * p, float * out_u, float * out_v, float * out_w);
ImVec2 imgui__im_triangle_closest_point(const ImVec2 * a, const ImVec2 * b, const ImVec2 * c, const ImVec2 * p);
unsigned char imgui__im_triangle_contains_point(const ImVec2 * a, const ImVec2 * b, const ImVec2 * c, const ImVec2 * p);
unsigned char ImTriangleIsClockwise__b1071f1010(const ImVec2 * a, const ImVec2 * b, const ImVec2 * c);
ImVec2 ImTrunc__735393dfb3(const ImVec2 * v);
float ImTrunc__ae7a4018f8(float f);
int ImUpperPowerOfTwo__034825575f(int v);
unsigned char operator____65519c27cd(const ImTextureRef * lhs, const ImTextureRef * rhs);
ImVec2 operator___c29d694b8f(const ImVec2 * lhs, const ImVec2 * rhs);
ImVec2 operator___fef625b53c(const ImVec2 * lhs, const float rhs);
ImVec2 operator___29d06b7915(const ImVec2 * lhs, const ImVec2 * rhs);
ImVec2 * operator____6e144c17f2(ImVec2 * lhs, const ImVec2 * rhs);
ImVec2 operator___cb7271567d(const ImVec2 * lhs, const ImVec2 * rhs);
unsigned char operator____fe19e57b70(const ImTextureRef * lhs, const ImTextureRef * rhs);
void imgui__activate_item_by_id(ImGuiContext *imgui_c89_ctx, ImGuiID id);
ImGuiID imgui__add_context_hook(ImGuiContext *imgui_c89_ctx, ImGuiContext * ctx, const ImGuiContextHook * hook);
void imgui__add_draw_list_to_draw_data_ex(ImGuiContext *imgui_c89_ctx, ImDrawData * draw_data, ImVector_ImDrawList_ptr * out_list, ImDrawList * draw_list);
void imgui__add_settings_handler(ImGuiContext *imgui_c89_ctx, const ImGuiSettingsHandler * handler);
unsigned char imgui__arrow_button_ex(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiDir dir, ImVec2 size, ImGuiButtonFlags flags);
unsigned char imgui__begin_box_select(ImGuiContext *imgui_c89_ctx, const ImRect * scope_rect, ImGuiWindow * window, ImGuiID box_select_id, ImGuiMultiSelectFlags ms_flags);
unsigned char imgui__begin_child_ex(ImGuiContext *imgui_c89_ctx, const char * name, ImGuiID id, const ImVec2 * size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);
void imgui__begin_columns(ImGuiContext *imgui_c89_ctx, const char * str_id, int columns_count, ImGuiOldColumnFlags flags);
unsigned char imgui__begin_combo_popup(ImGuiContext *imgui_c89_ctx, ImGuiID popup_id, const ImRect * bb, ImGuiComboFlags flags);
unsigned char imgui__begin_combo_preview(ImGuiContext *imgui_c89_ctx);
void imgui__begin_disabled_override_reenable(ImGuiContext *imgui_c89_ctx);
unsigned char imgui__begin_drag_drop_target_custom(ImGuiContext *imgui_c89_ctx, const ImRect * bb, ImGuiID id);
unsigned char imgui__begin_drag_drop_target_viewport(ImGuiContext *imgui_c89_ctx, ImGuiViewport * viewport, const ImRect * p_bb);
unsigned char imgui__begin_error_tooltip(ImGuiContext *imgui_c89_ctx);
unsigned char imgui__begin_menu_ex(ImGuiContext *imgui_c89_ctx, const char * label, const char * icon, unsigned char enabled);
unsigned char imgui__begin_popup_ex(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImGuiWindowFlags extra_window_flags);
unsigned char imgui__begin_popup_menu_ex(ImGuiContext *imgui_c89_ctx, ImGuiID id, const char * label, ImGuiWindowFlags extra_window_flags);
unsigned char imgui__begin_tab_bar_ex(ImGuiContext *imgui_c89_ctx, ImGuiTabBar * tab_bar, const ImRect * tab_bar_bb, ImGuiTabBarFlags flags);
unsigned char imgui__begin_table_ex(ImGuiContext *imgui_c89_ctx, const char * name, ImGuiID id, int columns_count, ImGuiTableFlags flags, const ImVec2 * outer_size, float inner_width);
unsigned char imgui__begin_tooltip_ex(ImGuiContext *imgui_c89_ctx, ImGuiTooltipFlags tooltip_flags, ImGuiWindowFlags extra_window_flags);
unsigned char imgui__begin_tooltip_hidden(ImGuiContext *imgui_c89_ctx);
unsigned char imgui__begin_viewport_side_bar(ImGuiContext *imgui_c89_ctx, const char * name, ImGuiViewport * viewport_p, ImGuiDir dir, float axis_size, ImGuiWindowFlags window_flags);
void imgui__bring_window_to_display_back(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
void imgui__bring_window_to_display_behind(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, ImGuiWindow * behind_window);
void imgui__bring_window_to_display_front(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
void imgui__bring_window_to_focus_front(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
void imgui__bullet_text(ImGuiContext *imgui_c89_ctx, const char * fmt, ...);
unsigned char imgui__button_behavior(ImGuiContext *imgui_c89_ctx, const ImRect * bb, ImGuiID id, unsigned char * out_hovered, unsigned char * out_held, ImGuiButtonFlags flags);
unsigned char imgui__button_ex(ImGuiContext *imgui_c89_ctx, const char * label, const ImVec2 * size_arg, ImGuiButtonFlags flags);
void imgui__calc_clip_rect_visible_items_y(const ImRect * clip_rect, const ImVec2 * pos, float items_height, int * out_visible_start, int * out_visible_end);
ImVec2 imgui__calc_item_size(ImGuiContext *imgui_c89_ctx, ImVec2 size, float default_w, float default_h);
ImDrawFlags imgui__calc_rounding_flags_for_rect_in_rect(const ImRect * r_in, const ImRect * r_outer, float threshold);
int imgui__calc_typematic_repeat_amount(float t0, float t1, float repeat_delay, float repeat_rate);
ImVec2 imgui__calc_window_next_auto_fit_size(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
float imgui__calc_wrap_width_for_pos(ImGuiContext *imgui_c89_ctx, const ImVec2 * pos, float wrap_pos_x);
void imgui__call_context_hooks(ImGuiContext * ctx, ImGuiContextHookType hook_type);
unsigned char imgui__checkbox_flags_string_s64_pointer_s64(ImGuiContext *imgui_c89_ctx, const char * label, ImS64 * flags, ImS64 flags_value);
unsigned char imgui__checkbox_flags_string_u64_pointer_u64(ImGuiContext *imgui_c89_ctx, const char * label, ImU64 * flags, ImU64 flags_value);
void imgui__cleanup_ini_settings(ImGuiContext *imgui_c89_ctx, ImGuiSettingsCleanupArgs * args);
void imgui__clear_active_id(ImGuiContext *imgui_c89_ctx);
void imgui__clear_drag_drop(ImGuiContext *imgui_c89_ctx);
void imgui__clear_ini_settings(ImGuiContext *imgui_c89_ctx);
void imgui__clear_window_settings(ImGuiContext *imgui_c89_ctx, const char * name);
unsigned char imgui__close_button(ImGuiContext *imgui_c89_ctx, ImGuiID id, const ImVec2 * pos);
void imgui__close_popup_to_level(ImGuiContext *imgui_c89_ctx, int remaining, unsigned char restore_focus_to_window_under_popup);
void imgui__close_popups_except_modals(ImGuiContext *imgui_c89_ctx);
void imgui__close_popups_over_window(ImGuiContext *imgui_c89_ctx, ImGuiWindow * ref_window, unsigned char restore_focus_to_window_under_popup);
unsigned char imgui__collapse_button(ImGuiContext *imgui_c89_ctx, ImGuiID id, const ImVec2 * pos);
void imgui__color_edit_options_popup(ImGuiContext *imgui_c89_ctx, const float * col, ImGuiColorEditFlags flags);
void imgui__color_picker_options_popup(ImGuiContext *imgui_c89_ctx, const float * ref_col, ImGuiColorEditFlags flags);
void imgui__color_tooltip(ImGuiContext *imgui_c89_ctx, const char * text, const float * col, ImGuiColorEditFlags flags);
ImGuiWindowSettings * imgui__create_new_window_settings(ImGuiContext *imgui_c89_ctx, const char * name);
unsigned char imgui__data_type_apply_from_text(const char * buf, ImGuiDataType data_type, void * p_data, const char * format, void * p_data_when_empty);
void imgui__data_type_apply_op(ImGuiDataType data_type, int op, void * output, const void * arg1, const void * arg2);
unsigned char imgui__data_type_clamp(ImGuiDataType data_type, void * p_data, const void * p_min, const void * p_max);
int imgui__data_type_compare(ImGuiDataType data_type, const void * arg_1, const void * arg_2);
int imgui__data_type_format_string(char * buf, int buf_size, ImGuiDataType data_type, const void * p_data, const char * format);
const ImGuiDataTypeInfo * imgui__data_type_get_info(ImGuiDataType data_type);
unsigned char imgui__data_type_is_zero(ImGuiContext *imgui_c89_ctx, ImGuiDataType data_type, const void * p_data);
void imgui__debug_alloc_hook(ImGuiDebugAllocInfo * info, int frame_count, void * ptr, size_t size);
unsigned char imgui__debug_break_button(ImGuiContext *imgui_c89_ctx, const char * label, const char * description_of_location);
void imgui__debug_break_button_tooltip(ImGuiContext *imgui_c89_ctx, unsigned char keyboard_only, const char * description_of_location);
void imgui__debug_break_clear_data(ImGuiContext *imgui_c89_ctx);
void imgui__debug_draw_cursor_pos(ImGuiContext *imgui_c89_ctx, ImU32 col);
void imgui__debug_draw_item_rect(ImGuiContext *imgui_c89_ctx, ImU32 col);
void imgui__debug_draw_line_extents(ImGuiContext *imgui_c89_ctx, ImU32 col);
void imgui__debug_hook_id_info(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImGuiDataType data_type, const void * data_id, const void * data_id_end);
void imgui__debug_locate_item(ImGuiContext *imgui_c89_ctx, ImGuiID target_id);
void imgui__debug_locate_item_on_hover(ImGuiContext *imgui_c89_ctx, ImGuiID target_id);
void imgui__debug_locate_item_resolve_with_last_item(ImGuiContext *imgui_c89_ctx);
void imgui__debug_log(ImGuiContext *imgui_c89_ctx, const char * fmt, ...);
void imgui__debug_node_columns(ImGuiContext *imgui_c89_ctx, ImGuiOldColumns * columns);
void imgui__debug_node_draw_cmd_show_mesh_and_bounding_box(ImGuiContext *imgui_c89_ctx, ImDrawList * out_draw_list, const ImDrawList * draw_list, const ImDrawCmd * draw_cmd, unsigned char show_mesh, unsigned char show_aabb);
void imgui__debug_node_draw_list(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, ImGuiViewportP * viewport, const ImDrawList * draw_list, const char * label);
void imgui__debug_node_font(ImGuiContext *imgui_c89_ctx, ImFont * font);
void imgui__debug_node_font_glyph(ImGuiContext *imgui_c89_ctx, ImFont * font, const ImFontGlyph * glyph);
void imgui__debug_node_font_glyphs_for_src_mask(ImGuiContext *imgui_c89_ctx, ImFont * font, ImFontBaked * baked, int src_mask);
void imgui__debug_node_input_text_state(ImGuiContext *imgui_c89_ctx, ImGuiInputTextState * state);
void imgui__debug_node_multi_select_state(ImGuiContext *imgui_c89_ctx, ImGuiMultiSelectState * storage);
void imgui__debug_node_storage(ImGuiContext *imgui_c89_ctx, ImGuiStorage * storage, const char * label);
void imgui__debug_node_tab_bar(ImGuiContext *imgui_c89_ctx, ImGuiTabBar * tab_bar, const char * label);
void imgui__debug_node_table(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__debug_node_table_settings(ImGuiContext *imgui_c89_ctx, ImGuiTableSettings * settings, ImGuiTable * table);
void imgui__debug_node_texture(ImGuiContext *imgui_c89_ctx, ImTextureData * tex, int int_id, const ImFontAtlasRect * highlight_rect);
void imgui__debug_node_typing_select_state(ImGuiContext *imgui_c89_ctx, ImGuiTypingSelectState * data);
void imgui__debug_node_viewport(ImGuiContext *imgui_c89_ctx, ImGuiViewportP * viewport);
void imgui__debug_node_window(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, const char * label);
void imgui__debug_node_window_settings(ImGuiContext *imgui_c89_ctx, ImGuiWindowSettings * settings);
void imgui__debug_node_windows_list(ImGuiContext *imgui_c89_ctx, ImVector_ImGuiWindow_ptr * windows, const char * label);
void imgui__debug_node_windows_list_by_begin_stack_parent(ImGuiContext *imgui_c89_ctx, ImGuiWindow ** windows, int windows_size, ImGuiWindow * parent_in_begin_stack);
void imgui__debug_render_keyboard_preview(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list);
void imgui__debug_render_viewport_thumbnail(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, ImGuiViewportP * viewport, const ImRect * bb);
void imgui__debug_text_unformatted_with_locate_item(ImGuiContext *imgui_c89_ctx, const char * line_begin, const char * line_end);
ImU64 imgui__debug_texture_id_to_u64(ImTextureID tex_id);
void imgui__demo_marker(ImGuiContext *imgui_c89_ctx, const char * file, int line, const char * section);
unsigned char imgui__drag_behavior(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImGuiDataType data_type, void * p_v, float v_speed, const void * p_min, const void * p_max, const char * format, ImGuiSliderFlags flags);
void imgui__end_box_select(ImGuiContext *imgui_c89_ctx, const ImRect * scope_rect, ImGuiMultiSelectFlags ms_flags);
void imgui__end_columns(ImGuiContext *imgui_c89_ctx);
void imgui__end_combo_preview(ImGuiContext *imgui_c89_ctx);
void imgui__end_disabled_override_reenable(ImGuiContext *imgui_c89_ctx);
void imgui__end_error_tooltip(ImGuiContext *imgui_c89_ctx);
void imgui__error_check_end_frame_finalize_error_tooltip(ImGuiContext *imgui_c89_ctx);
void imgui__error_check_using_set_cursor_pos_to_extend_parent_boundaries(ImGuiContext *imgui_c89_ctx);
unsigned char imgui__error_log(ImGuiContext *imgui_c89_ctx, const char * msg);
void imgui__error_recovery_store_state(ImGuiContext *imgui_c89_ctx, ImGuiErrorRecoveryState * state_out);
void imgui__error_recovery_try_to_recover_state(ImGuiContext *imgui_c89_ctx, const ImGuiErrorRecoveryState * state_in);
void imgui__error_recovery_try_to_recover_window_state(ImGuiContext *imgui_c89_ctx, const ImGuiErrorRecoveryState * state_in);
void imgui__extend_hit_box_when_near_viewport_edge(ImGuiWindow * window, ImRect * bb, float threshold, ImGuiAxis axis);
ImVec2 imgui__find_best_window_pos_for_popup(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
ImVec2 imgui__find_best_window_pos_for_popup_ex(const ImVec2 * ref_pos, const ImVec2 * size, ImGuiDir * last_dir, const ImRect * r_outer, const ImRect * r_avoid, ImGuiPopupPositionPolicy policy);
ImGuiWindow * imgui__find_blocking_modal(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
ImGuiWindow * imgui__find_bottom_most_visible_window_within_begin_stack(ImGuiContext *imgui_c89_ctx, ImGuiWindow * parent_window);
ImGuiWindow * imgui__find_front_most_visible_child_window(ImGuiWindow * window);
void imgui__find_hovered_window_ex(ImGuiContext *imgui_c89_ctx, const ImVec2 * pos, unsigned char find_first_and_in_any_viewport, ImGuiWindow ** out_hovered_window, ImGuiWindow ** out_hovered_window_under_moving_window);
ImGuiOldColumns * imgui__find_or_create_columns(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, ImGuiID id);
const char * imgui__find_rendered_text_end(const char * text, const char * text_end);
ImGuiSettingsHandler * imgui__find_settings_handler(ImGuiContext *imgui_c89_ctx, const char * type_name);
ImGuiWindow * imgui__find_window_by_id(ImGuiContext *imgui_c89_ctx, ImGuiID id);
ImGuiWindow * imgui__find_window_by_name(ImGuiContext *imgui_c89_ctx, const char * name);
int imgui__find_window_display_index(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
ImGuiWindowSettings * imgui__find_window_settings_by_id(ImGuiContext *imgui_c89_ctx, ImGuiID id);
ImGuiWindowSettings * imgui__find_window_settings_by_window(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
ImGuiKeyChord imgui__fixup_key_chord(ImGuiKeyChord key_chord);
void imgui__focus_item(ImGuiContext *imgui_c89_ctx);
void imgui__focus_top_most_window_under_one(ImGuiContext *imgui_c89_ctx, ImGuiWindow * under_this_window, ImGuiWindow * ignore_window, ImGuiViewport * filter_viewport, ImGuiFocusRequestFlags flags);
void imgui__focus_window(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, ImGuiFocusRequestFlags flags);
void imgui__gc_awake_transient_window_buffers(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
void imgui__gc_compact_transient_misc_buffers(ImGuiContext *imgui_c89_ctx);
void imgui__gc_compact_transient_window_buffers(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
ImDrawList * imgui__get_background_draw_list(ImGuiContext *imgui_c89_ctx, ImGuiViewport * viewport);
ImGuiBoxSelectState * ImGui_GetBoxSelectState__9b73c23d2f(ImGuiContext *imgui_c89_ctx, ImGuiID id);
float imgui__get_column_norm_from_offset(const ImGuiOldColumns * columns, float offset);
float imgui__get_column_offset_from_norm(const ImGuiOldColumns * columns, float offset_norm);
ImGuiID imgui__get_columns_id(ImGuiContext *imgui_c89_ctx, const char * str_id, int columns_count);
ImGuiWindow * ImGui_GetCurrentWindow__f542a96313(ImGuiContext *imgui_c89_ctx);
ImFont * imgui__get_default_font(ImGuiContext *imgui_c89_ctx);
ImDrawList * imgui__get_foreground_draw_list(ImGuiContext *imgui_c89_ctx, ImGuiViewport * viewport);
ImDrawList * ImGui_GetForegroundDrawList__167528742e(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
ImGuiID imgui__get_hovered_id(ImGuiContext *imgui_c89_ctx);
ImGuiID imgui__get_id_with_seed_string_string_id(ImGuiContext *imgui_c89_ctx, const char * str, const char * str_end, ImGuiID seed);
ImGuiID imgui__get_id_with_seed_int_id(ImGuiContext *imgui_c89_ctx, int n, ImGuiID seed);
ImGuiIO * imgui__get_io(ImGuiContext * ctx);
ImGuiInputTextState * ImGui_GetInputTextState__1cf253e65e(ImGuiContext *imgui_c89_ctx, ImGuiID id);
const char * imgui__get_key_chord_name(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord);
ImGuiKeyData * ImGui_GetKeyData__35ffe023d5(ImGuiContext *imgui_c89_ctx, ImGuiKey key);
ImGuiKeyData * imgui__get_key_data(ImGuiContext * ctx, ImGuiKey key);
ImVec2 imgui__get_key_magnitude2d(ImGuiContext *imgui_c89_ctx, ImGuiKey key_left, ImGuiKey key_right, ImGuiKey key_up, ImGuiKey key_down);
ImGuiID imgui__get_key_owner(ImGuiContext *imgui_c89_ctx, ImGuiKey key);
ImGuiMouseButton imgui__get_mouse_button_from_popup_flags(ImGuiPopupFlags flags);
float imgui__get_nav_tweak_pressed_amount(ImGuiContext *imgui_c89_ctx, ImGuiAxis axis);
ImGuiPlatformIO * imgui__get_platform_io(ImGuiContext * ctx);
ImRect imgui__get_popup_allowed_extent_rect(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
float ImGui_GetRoundedFontSize__822dc3c5f9(float size);
ImGuiKeyRoutingData * imgui__get_shortcut_routing_data(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord);
const ImGuiStyleVarInfo * imgui__get_style_var_info(ImGuiStyleVar idx);
ImGuiWindow * imgui__get_top_most_and_visible_popup_modal(ImGuiContext *imgui_c89_ctx);
ImGuiWindow * imgui__get_top_most_popup_modal(ImGuiContext *imgui_c89_ctx);
void imgui__get_typematic_repeat_rate(ImGuiContext *imgui_c89_ctx, ImGuiInputFlags flags, float * repeat_delay, float * repeat_rate);
ImGuiTypingSelectRequest * imgui__get_typing_select_request(ImGuiContext *imgui_c89_ctx, ImGuiTypingSelectFlags flags);
ImGuiID imgui__get_window_resize_border_id(ImGuiWindow * window, ImGuiDir dir);
ImGuiID imgui__get_window_resize_corner_id(ImGuiWindow * window, int n);
ImGuiID imgui__get_window_scrollbar_id(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, ImGuiAxis axis);
ImRect imgui__get_window_scrollbar_rect(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, ImGuiAxis axis);
unsigned char imgui__image_button_ex(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImTextureRef tex_ref, const ImVec2 * image_size, const ImVec2 * uv0, const ImVec2 * uv1, const ImVec4 * bg_col, const ImVec4 * tint_col, ImGuiButtonFlags flags);
void imgui__initialize(ImGuiContext *imgui_c89_ctx);
void imgui__input_text_deactivate_hook(ImGuiContext *imgui_c89_ctx, ImGuiID id);
unsigned char imgui__input_text_ex(ImGuiContext *imgui_c89_ctx, const char * label, const char * hint, char * buf, int buf_size, const ImVec2 * size_arg, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void * callback_user_data);
unsigned char imgui__is_clipped_ex(ImGuiContext *imgui_c89_ctx, const ImRect * bb, ImGuiID id);
unsigned char imgui__is_drag_drop_active(ImGuiContext *imgui_c89_ctx);
unsigned char imgui__is_drag_drop_payload_being_accepted(ImGuiContext *imgui_c89_ctx);
unsigned char imgui__is_in_nav_focus_route(ImGuiContext *imgui_c89_ctx, ImGuiID focus_scope_id);
unsigned char imgui__is_key_chord_pressed(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id);
unsigned char imgui__is_key_down(ImGuiContext *imgui_c89_ctx, ImGuiKey key, ImGuiID owner_id);
unsigned char imgui__is_key_pressed(ImGuiContext *imgui_c89_ctx, ImGuiKey key, ImGuiInputFlags flags, ImGuiID owner_id);
unsigned char imgui__is_key_released(ImGuiContext *imgui_c89_ctx, ImGuiKey key, ImGuiID owner_id);
unsigned char imgui__is_mouse_clicked(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, ImGuiInputFlags flags, ImGuiID owner_id);
unsigned char imgui__is_mouse_double_clicked(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, ImGuiID owner_id);
unsigned char imgui__is_mouse_down(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, ImGuiID owner_id);
unsigned char imgui__is_mouse_drag_past_threshold(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, float lock_threshold);
unsigned char imgui__is_mouse_released(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, ImGuiID owner_id);
unsigned char imgui__is_popup_open(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImGuiPopupFlags popup_flags);
unsigned char imgui__is_popup_open_request_for_item(ImGuiContext *imgui_c89_ctx, ImGuiPopupFlags popup_flags, ImGuiID id);
unsigned char imgui__is_popup_open_request_for_window(ImGuiContext *imgui_c89_ctx, ImGuiPopupFlags popup_flags);
unsigned char imgui__is_window_above(ImGuiContext *imgui_c89_ctx, ImGuiWindow * potential_above, ImGuiWindow * potential_below);
unsigned char imgui__is_window_child_of(ImGuiWindow * window, ImGuiWindow * potential_parent, unsigned char popup_hierarchy);
unsigned char imgui__is_window_content_hoverable(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, ImGuiHoveredFlags flags);
unsigned char imgui__is_window_in_begin_stack(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
unsigned char imgui__is_window_nav_focusable(ImGuiWindow * window);
unsigned char imgui__is_window_within_begin_stack_of(ImGuiWindow * window, ImGuiWindow * potential_parent);
unsigned char imgui__item_add(ImGuiContext *imgui_c89_ctx, const ImRect * bb, ImGuiID id, const ImRect * nav_bb_arg, ImGuiItemFlags extra_flags);
unsigned char imgui__item_hoverable(ImGuiContext *imgui_c89_ctx, const ImRect * bb, ImGuiID id, ImGuiItemFlags item_flags);
void ImGui_ItemSize__8033f34120(ImGuiContext *imgui_c89_ctx, const ImRect * bb, float text_baseline_y);
void imgui__item_size(ImGuiContext *imgui_c89_ctx, const ImVec2 * size, float text_baseline_y);
void imgui__keep_alive_id(ImGuiContext *imgui_c89_ctx, ImGuiID id);
void imgui__label_text(ImGuiContext *imgui_c89_ctx, const char * label, const char * fmt, ...);
const char * ImGui_LocalizeGetMsg__d57d7d5e4e(ImGuiContext *imgui_c89_ctx, ImGuiLocKey key);
void imgui__localize_register_entries(ImGuiContext *imgui_c89_ctx, const ImGuiLocEntry * entries, int count);
void imgui__log_begin(ImGuiContext *imgui_c89_ctx, ImGuiLogFlags flags, int auto_open_depth);
void imgui__log_rendered_text(ImGuiContext *imgui_c89_ctx, const ImVec2 * ref_pos, const char * text, const char * text_end);
void imgui__log_set_next_text_decoration(ImGuiContext *imgui_c89_ctx, const char * prefix, const char * suffix);
void imgui__log_text(ImGuiContext *imgui_c89_ctx, const char * fmt, ...);
void imgui__log_to_buffer(ImGuiContext *imgui_c89_ctx, int auto_open_depth);
void imgui__mark_ini_settings_dirty_void(ImGuiContext *imgui_c89_ctx);
void imgui__mark_ini_settings_dirty_window_pointer(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
void imgui__mark_item_edited(ImGuiContext *imgui_c89_ctx, ImGuiID id);
unsigned char imgui__menu_item_ex(ImGuiContext *imgui_c89_ctx, const char * label, const char * icon, const char * shortcut, unsigned char selected, unsigned char enabled);
ImGuiKey ImGui_MouseButtonToKey__2b6640ccb6(ImGuiMouseButton button);
void imgui__multi_select_add_set_all(ImGuiContext *imgui_c89_ctx, ImGuiMultiSelectTempData * ms, unsigned char selected);
void imgui__multi_select_add_set_range(ImGuiContext *imgui_c89_ctx, ImGuiMultiSelectTempData * ms, unsigned char selected, int range_dir, ImGuiSelectionUserData first_item, ImGuiSelectionUserData last_item);
void imgui__multi_select_item_footer(ImGuiContext *imgui_c89_ctx, ImGuiID id, unsigned char * p_selected, unsigned char * p_pressed, ImGuiMultiSelectFlags extra_flags);
void imgui__multi_select_item_header(ImGuiContext *imgui_c89_ctx, ImGuiID id, unsigned char * p_selected, ImGuiButtonFlags * p_button_flags);
void imgui__nav_clear_preferred_pos_for_axis(ImGuiContext *imgui_c89_ctx, ImGuiAxis axis);
void imgui__nav_highlight_activated(ImGuiContext *imgui_c89_ctx, ImGuiID id);
void imgui__nav_init_request_apply_result(ImGuiContext *imgui_c89_ctx);
void imgui__nav_init_window(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, unsigned char force_reinit);
void imgui__nav_move_request_apply_result(ImGuiContext *imgui_c89_ctx);
unsigned char imgui__nav_move_request_but_no_result_yet(ImGuiContext *imgui_c89_ctx);
void imgui__nav_move_request_cancel(ImGuiContext *imgui_c89_ctx);
void imgui__nav_move_request_forward(ImGuiContext *imgui_c89_ctx, ImGuiDir move_dir, ImGuiDir clip_dir, ImGuiNavMoveFlags move_flags, ImGuiScrollFlags scroll_flags);
void imgui__nav_move_request_resolve_with_last_item(ImGuiContext *imgui_c89_ctx, ImGuiNavItemData * result);
void imgui__nav_move_request_resolve_with_past_tree_node(ImGuiContext *imgui_c89_ctx, ImGuiNavItemData * result, const ImGuiTreeNodeStackData * tree_node_data);
void imgui__nav_move_request_submit(ImGuiContext *imgui_c89_ctx, ImGuiDir move_dir, ImGuiDir clip_dir, ImGuiNavMoveFlags move_flags, ImGuiScrollFlags scroll_flags);
void imgui__nav_move_request_try_wrapping(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, ImGuiNavMoveFlags wrap_flags);
void imgui__nav_update_current_window_is_scroll_pushable_x(ImGuiContext *imgui_c89_ctx);
unsigned char imgui__open_popup_ex(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImGuiPopupFlags popup_flags);
int imgui__plot_ex(ImGuiContext *imgui_c89_ctx, ImGuiPlotType plot_type, const char * label, float (*values_getter)(void *, int), void * data, int values_count, int values_offset, const char * overlay_text, float scale_min, float scale_max, const ImVec2 * size_arg);
void imgui__pop_columns_background(ImGuiContext *imgui_c89_ctx);
void imgui__pop_focus_scope(ImGuiContext *imgui_c89_ctx);
void imgui__pop_password_font(ImGuiContext *imgui_c89_ctx);
void imgui__push_column_clip_rect(ImGuiContext *imgui_c89_ctx, int column_index);
void imgui__push_columns_background(ImGuiContext *imgui_c89_ctx);
void imgui__push_focus_scope(ImGuiContext *imgui_c89_ctx, ImGuiID id);
void imgui__push_multi_items_widths(ImGuiContext *imgui_c89_ctx, int components, float w_full);
void imgui__push_override_id(ImGuiContext *imgui_c89_ctx, ImGuiID id);
void imgui__push_password_font(ImGuiContext *imgui_c89_ctx);
void imgui__register_font_atlas(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
void imgui__register_user_texture(ImGuiContext *imgui_c89_ctx, ImTextureData * tex);
void imgui__remove_context_hook(ImGuiContext * ctx, ImGuiID hook_id);
void imgui__remove_settings_handler(ImGuiContext *imgui_c89_ctx, const char * type_name);
void imgui__render_arrow(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, ImVec2 pos, ImU32 col, ImGuiDir dir, float scale);
void imgui__render_arrow_pointing_at(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col);
void imgui__render_bullet(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, ImVec2 pos, ImU32 col);
void imgui__render_check_mark(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, ImVec2 pos, ImU32 col, float sz);
void imgui__render_color_component_marker(ImGuiContext *imgui_c89_ctx, const ImRect * bb, ImU32 col, float rounding);
void imgui__render_color_rect_with_alpha_checkerboard(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, ImVec2 p_min, ImVec2 p_max, ImU32 col, float grid_step, ImVec2 grid_off, float rounding, ImDrawFlags flags);
void imgui__render_drag_drop_target_rect_ex(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, const ImRect * bb, float rounding);
void imgui__render_drag_drop_target_rect_for_item(ImGuiContext *imgui_c89_ctx, const ImRect * bb);
void imgui__render_frame(ImGuiContext *imgui_c89_ctx, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, unsigned char borders, float rounding);
void imgui__render_frame_border(ImGuiContext *imgui_c89_ctx, ImVec2 p_min, ImVec2 p_max, float rounding);
void imgui__render_mouse_cursor(ImGuiContext *imgui_c89_ctx, ImVec2 base_pos, float base_scale, ImGuiMouseCursor mouse_cursor, ImU32 col_fill, ImU32 col_border, ImU32 col_shadow);
void imgui__render_nav_cursor(ImGuiContext *imgui_c89_ctx, const ImRect * bb, ImGuiID id, ImGuiNavRenderCursorFlags flags, float rounding);
void imgui__render_rect_filled_in_range_h(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, const ImRect * rect, ImU32 col, float fill_x0, float fill_x1, float rounding);
void imgui__render_rect_filled_with_hole(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, const ImRect * outer, const ImRect * inner, ImU32 col, float rounding);
void imgui__render_text(ImGuiContext *imgui_c89_ctx, ImVec2 pos, const char * text, const char * text_end, unsigned char hide_text_after_hash);
void imgui__render_text_clipped(ImGuiContext *imgui_c89_ctx, const ImVec2 * pos_min, const ImVec2 * pos_max, const char * text, const char * text_end, const ImVec2 * text_size_if_known, const ImVec2 * align, const ImRect * clip_rect);
void imgui__render_text_clipped_ex(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, const ImVec2 * pos_min, const ImVec2 * pos_max, const char * text, const char * text_display_end, const ImVec2 * text_size_if_known, const ImVec2 * align, const ImRect * clip_rect);
void imgui__render_text_ellipsis(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, const ImVec2 * pos_min, const ImVec2 * pos_max, float ellipsis_max_x, const char * text, const char * text_end_full, const ImVec2 * text_size_if_known);
void imgui__render_text_wrapped(ImGuiContext *imgui_c89_ctx, ImVec2 pos, const char * text, const char * text_end, float wrap_width);
void imgui__scale_windows_in_viewport(ImGuiContext *imgui_c89_ctx, ImGuiViewportP * viewport, float scale);
void imgui__scroll_to_item(ImGuiContext *imgui_c89_ctx, ImGuiScrollFlags flags);
void imgui__scroll_to_rect(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, const ImRect * item_rect, ImGuiScrollFlags flags);
ImVec2 imgui__scroll_to_rect_ex(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, const ImRect * item_rect, ImGuiScrollFlags flags);
void imgui__scrollbar(ImGuiContext *imgui_c89_ctx, ImGuiAxis axis);
unsigned char imgui__scrollbar_ex(ImGuiContext *imgui_c89_ctx, const ImRect * bb_frame, ImGuiID id, ImGuiAxis axis, ImS64 * p_scroll_v, ImS64 size_visible_v, ImS64 size_contents_v, ImDrawFlags draw_rounding_flags);
void imgui__separator_ex(ImGuiContext *imgui_c89_ctx, ImGuiSeparatorFlags flags, float thickness);
void imgui__separator_text_ex(ImGuiContext *imgui_c89_ctx, ImGuiID id, const char * label, const char * label_end, float extra_w);
void imgui__set_active_id(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImGuiWindow * window);
void imgui__set_active_id_using_all_keyboard_keys(ImGuiContext *imgui_c89_ctx);
void imgui__set_context_name(ImGuiContext * ctx, const char * name);
void imgui__set_current_font(ImGuiContext *imgui_c89_ctx, ImFont * font, float font_size_before_scaling, float font_size_after_scaling);
void imgui__set_focus_id(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImGuiWindow * window);
void imgui__set_font_rasterizer_density(ImGuiContext *imgui_c89_ctx, float rasterizer_density);
void imgui__set_hovered_id(ImGuiContext *imgui_c89_ctx, ImGuiID id);
unsigned char imgui__set_item_key_owner(ImGuiContext *imgui_c89_ctx, ImGuiKey key, ImGuiInputFlags flags);
void imgui__set_item_tooltip(ImGuiContext *imgui_c89_ctx, const char * fmt, ...);
void imgui__set_key_owner(ImGuiContext *imgui_c89_ctx, ImGuiKey key, ImGuiID owner_id, ImGuiInputFlags flags);
void imgui__set_key_owners_for_key_chord(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord, ImGuiID owner_id, ImGuiInputFlags flags);
void imgui__set_last_item_data(ImGuiContext *imgui_c89_ctx, ImGuiID item_id, ImGuiItemFlags item_flags, ImGuiItemStatusFlags status_flags, const ImRect * item_rect);
void imgui__set_nav_cursor_visible_after_move(ImGuiContext *imgui_c89_ctx);
void imgui__set_nav_focus_scope(ImGuiContext *imgui_c89_ctx, ImGuiID focus_scope_id);
void imgui__set_nav_id(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImGuiNavLayer nav_layer, ImGuiID focus_scope_id, const ImRect * rect_rel);
void imgui__set_nav_window(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
void ImGui_SetNextItemColorMarker__7d8a80241b(ImGuiContext *imgui_c89_ctx, ImU32 col);
void imgui__set_next_item_ref_val(ImGuiContext *imgui_c89_ctx, ImGuiDataType data_type, void * p_data);
void imgui__set_next_window_refresh_policy(ImGuiContext *imgui_c89_ctx, ImGuiWindowRefreshFlags flags);
void imgui__set_scroll_from_pos_x(ImGuiWindow * window, float local_x, float center_x_ratio);
void imgui__set_scroll_from_pos_y(ImGuiWindow * window, float local_y, float center_y_ratio);
void imgui__set_scroll_x(ImGuiWindow * window, float scroll_x);
void imgui__set_scroll_y(ImGuiWindow * window, float scroll_y);
unsigned char imgui__set_shortcut_routing(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id);
void imgui__set_tooltip(ImGuiContext *imgui_c89_ctx, const char * fmt, ...);
void imgui__set_window_clip_rect_before_set_channel(ImGuiWindow * window, const ImRect * clip_rect);
void imgui__set_window_collapsed(ImGuiWindow * window, unsigned char collapsed, ImGuiCond cond);
void imgui__set_window_hidden_and_skip_items_for_current_frame(ImGuiWindow * window);
void imgui__set_window_hit_test_hole(ImGuiWindow * window, const ImVec2 * pos, const ImVec2 * size);
void imgui__set_window_pos(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, const ImVec2 * pos, ImGuiCond cond);
void imgui__set_window_size(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window, const ImVec2 * size, ImGuiCond cond);
void imgui__set_window_viewport(ImGuiWindow * window, ImGuiViewportP * viewport);
void imgui__shade_verts_linear_color_gradient_keep_alpha(ImDrawList * draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1);
void imgui__shade_verts_linear_uv(ImDrawList * draw_list, int vert_start_idx, int vert_end_idx, const ImVec2 * a, const ImVec2 * b, const ImVec2 * uv_a, const ImVec2 * uv_b, unsigned char clamp);
void imgui__shade_verts_transform_pos(ImDrawList * draw_list, int vert_start_idx, int vert_end_idx, const ImVec2 * pivot_in, float cos_a, float sin_a, const ImVec2 * pivot_out);
unsigned char imgui__shortcut(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id);
void ImGui_ShowAboutWindow__9333fde1e2(unsigned char * p_open);
void ImGui_ShowDemoWindow__f1609d4301(unsigned char * p_open);
void imgui__show_font_atlas(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
void ImGui_ShowStyleEditor__04d10cac9b(ImGuiStyle * ref);
unsigned char ImGui_ShowStyleSelector__e526939305(const char * label);
void ImGui_ShowUserGuide__dab54210eb(void);
void imgui__shrink_widths(ImGuiShrinkWidthItem * items, int count, float width_excess, float width_min);
void imgui__shutdown(ImGuiContext *imgui_c89_ctx);
unsigned char imgui__slider_behavior(ImGuiContext *imgui_c89_ctx, const ImRect * bb, ImGuiID id, ImGuiDataType data_type, void * p_v, const void * p_min, const void * p_max, const char * format, ImGuiSliderFlags flags, ImRect * out_grab_bb);
unsigned char imgui__splitter_behavior(ImGuiContext *imgui_c89_ctx, const ImRect * bb, ImGuiID id, ImGuiAxis axis, float * size1, float * size2, float min_size1, float min_size2, float hover_extend, float hover_visibility_delay, ImU32 bg_col);
void imgui__start_mouse_moving_window(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
void imgui__stop_mouse_moving_window(ImGuiContext *imgui_c89_ctx);
void imgui__tab_bar_close_tab(ImGuiTabBar * tab_bar, ImGuiTabItem * tab);
ImGuiTabBar * imgui__tab_bar_find_by_id(ImGuiContext *imgui_c89_ctx, ImGuiID id);
ImGuiTabItem * imgui__tab_bar_find_tab_by_id(ImGuiTabBar * tab_bar, ImGuiID tab_id);
ImGuiTabItem * imgui__tab_bar_find_tab_by_order(ImGuiTabBar * tab_bar, int order);
ImGuiTabItem * imgui__tab_bar_get_current_tab(ImGuiTabBar * tab_bar);
const char * imgui__tab_bar_get_tab_name(ImGuiTabBar * tab_bar, ImGuiTabItem * tab);
int ImGui_TabBarGetTabOrder__3b7fa0d04c(ImGuiTabBar * tab_bar, ImGuiTabItem * tab);
unsigned char imgui__tab_bar_process_reorder(ImGuiContext *imgui_c89_ctx, ImGuiTabBar * tab_bar);
void imgui__tab_bar_queue_focus_tab_bar_pointer_tab_item_pointer(ImGuiTabBar * tab_bar, ImGuiTabItem * tab);
void imgui__tab_bar_queue_focus_tab_bar_pointer_string(ImGuiContext *imgui_c89_ctx, ImGuiTabBar * tab_bar, const char * tab_name);
void imgui__tab_bar_queue_reorder(ImGuiTabBar * tab_bar, ImGuiTabItem * tab, int offset);
void imgui__tab_bar_queue_reorder_from_mouse_pos(ImGuiContext *imgui_c89_ctx, ImGuiTabBar * tab_bar, ImGuiTabItem * src_tab, ImVec2 mouse_pos);
void imgui__tab_bar_remove(ImGuiContext *imgui_c89_ctx, ImGuiTabBar * tab_bar);
void imgui__tab_bar_remove_tab(ImGuiTabBar * tab_bar, ImGuiID tab_id);
void imgui__tab_item_background(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, const ImRect * bb, ImGuiTabItemFlags flags, ImU32 col);
ImVec2 imgui__tab_item_calc_size_window_pointer(ImGuiWindow * arg_0);
ImVec2 imgui__tab_item_calc_size_string_bool(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char has_close_button_or_unsaved_marker);
unsigned char imgui__tab_item_ex(ImGuiContext *imgui_c89_ctx, ImGuiTabBar * tab_bar, const char * label, unsigned char * p_open, ImGuiTabItemFlags flags, ImGuiWindow * docked_window);
void imgui__tab_item_label_and_close_button(ImGuiContext *imgui_c89_ctx, ImDrawList * draw_list, const ImRect * bb, ImGuiTabItemFlags flags, ImVec2 frame_padding, const char * label, ImGuiID tab_id, ImGuiID close_button_id, unsigned char is_contents_visible, unsigned char * out_just_closed, unsigned char * out_text_clipped);
void imgui__tab_item_spacing(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiTabItemFlags flags, float width);
void imgui__table_angled_headers_row_ex(ImGuiContext *imgui_c89_ctx, ImGuiID row_id, float angle, float max_label_width, const ImGuiTableHeaderData * data, int data_count);
void imgui__table_apply_external_unclip_rect(ImGuiTable * table, ImRect * rect);
void imgui__table_apply_queued_requests(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_begin_cell(ImGuiContext *imgui_c89_ctx, ImGuiTable * table, int column_n);
unsigned char imgui__table_begin_context_menu_popup(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_begin_init_memory(ImGuiContext *imgui_c89_ctx, ImGuiTable * table, int columns_count);
void imgui__table_begin_row(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
float imgui__table_calc_max_column_width(const ImGuiTable * table, int column_n);
void imgui__table_draw_borders(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_draw_default_context_menu(ImGuiContext *imgui_c89_ctx, ImGuiTable * table, ImGuiTableFlags flags_for_section_to_display);
void imgui__table_end_cell(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_end_row(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
ImGuiTable * imgui__table_find_by_id(ImGuiContext *imgui_c89_ctx, ImGuiID id);
void imgui__table_fix_column_sort_direction(ImGuiTable * table, ImGuiTableColumn * column);
void imgui__table_fix_display_order(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_gc_compact_settings(ImGuiContext *imgui_c89_ctx);
void imgui__table_gc_compact_transient_buffers_table_pointer(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_gc_compact_transient_buffers_table_temp_data_pointer(ImGuiContext *imgui_c89_ctx, ImGuiTableTempData * temp_data);
ImGuiTableSettings * imgui__table_get_bound_settings(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
ImRect imgui__table_get_cell_bg_rect(const ImGuiTable * table, int column_n);
const char * imgui__table_get_column_name(const ImGuiTable * table, int column_n);
ImGuiSortDirection imgui__table_get_column_next_sort_direction(ImGuiTableColumn * column);
ImGuiID imgui__table_get_column_resize_id(ImGuiTable * table, int column_n, int instance_no);
float imgui__table_get_column_width_auto(ImGuiTable * table, ImGuiTableColumn * column);
float imgui__table_get_header_angled_max_label_width(ImGuiContext *imgui_c89_ctx);
float imgui__table_get_header_row_height(ImGuiContext *imgui_c89_ctx);
int imgui__table_get_hovered_row(ImGuiContext *imgui_c89_ctx);
ImGuiTableInstanceData * ImGui_TableGetInstanceData__27767b51c6(ImGuiTable * table, int instance_no);
void imgui__table_init_column_defaults(ImGuiTable * table, ImGuiTableColumn * column, ImGuiTableColumnFlags init_mask);
void imgui__table_load_settings(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_load_settings_for_column(ImGuiTableColumn * column, ImGuiTableColumnSettings * column_settings, ImGuiTableFlags load_flags);
void imgui__table_load_settings_for_columns(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_merge_draw_channels(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_open_context_menu(ImGuiContext *imgui_c89_ctx, int column_n);
void imgui__table_pop_background_channel(ImGuiContext *imgui_c89_ctx);
void imgui__table_pop_column_channel(ImGuiContext *imgui_c89_ctx);
void imgui__table_push_background_channel(ImGuiContext *imgui_c89_ctx);
void imgui__table_push_column_channel(ImGuiContext *imgui_c89_ctx, int column_n);
void imgui__table_queue_set_column_display_order(ImGuiTable * table, int column_n, int dst_order);
void imgui__table_reconcile_columns(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_remove(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_reset_settings(ImGuiTable * table);
void imgui__table_save_settings(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_set_column_display_order(ImGuiTable * table, int column_n, int dst_order);
void imgui__table_set_column_sort_direction(ImGuiContext *imgui_c89_ctx, int column_n, ImGuiSortDirection sort_direction, unsigned char append_to_sort_specs);
void imgui__table_set_column_width(ImGuiContext *imgui_c89_ctx, int column_n, float width);
void imgui__table_set_column_width_auto_all(ImGuiTable * table);
void imgui__table_set_column_width_auto_single(ImGuiTable * table, int column_n);
void imgui__table_settings_add_settings_handler(ImGuiContext *imgui_c89_ctx);
ImGuiTableSettings * imgui__table_settings_create(ImGuiContext *imgui_c89_ctx, ImGuiID id, int columns_count);
ImGuiTableSettings * imgui__table_settings_find_by_id(ImGuiContext *imgui_c89_ctx, ImGuiID id);
void imgui__table_setup_draw_channels(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_sort_specs_build(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_sort_specs_sanitize(ImGuiTable * table);
void imgui__table_update_borders(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__table_update_columns_weight_from_width(ImGuiTable * table);
void imgui__table_update_layout(ImGuiContext *imgui_c89_ctx, ImGuiTable * table);
void imgui__teleport_mouse_pos(ImGuiContext *imgui_c89_ctx, const ImVec2 * pos);
unsigned char ImGui_TempInputIsActive__3488146d16(ImGuiContext *imgui_c89_ctx, ImGuiID id);
unsigned char imgui__temp_input_scalar(ImGuiContext *imgui_c89_ctx, const ImRect * bb, ImGuiID id, const char * label, ImGuiDataType data_type, void * p_data, const char * format, const void * p_clamp_min, const void * p_clamp_max);
unsigned char imgui__temp_input_text(ImGuiContext *imgui_c89_ctx, const ImRect * bb, ImGuiID id, const char * label, char * buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void * user_data);
unsigned char imgui__test_key_owner(ImGuiContext *imgui_c89_ctx, ImGuiKey key, ImGuiID owner_id);
unsigned char imgui__test_shortcut_routing(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord, ImGuiID owner_id);
void imgui__text(ImGuiContext *imgui_c89_ctx, const char * fmt, ...);
void imgui__text_aligned(ImGuiContext *imgui_c89_ctx, float align_x, float size_x, const char * fmt, ...);
void imgui__text_aligned_v(ImGuiContext *imgui_c89_ctx, float align_x, float size_x, const char * fmt, va_list args);
void imgui__text_colored(ImGuiContext *imgui_c89_ctx, const ImVec4 * col, const char * fmt, ...);
void imgui__text_disabled(ImGuiContext *imgui_c89_ctx, const char * fmt, ...);
void imgui__text_ex(ImGuiContext *imgui_c89_ctx, const char * text, const char * text_end, ImGuiTextFlags flags);
void imgui__text_wrapped(ImGuiContext *imgui_c89_ctx, const char * fmt, ...);
unsigned char imgui__tree_node_string_string_varargs(ImGuiContext *imgui_c89_ctx, const char * str_id, const char * fmt, ...);
unsigned char imgui__tree_node_const_pointer_string_varargs(ImGuiContext *imgui_c89_ctx, const void * ptr_id, const char * fmt, ...);
unsigned char imgui__tree_node_behavior(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImGuiTreeNodeFlags flags, const char * label, const char * label_end);
void imgui__tree_node_draw_line_to_child_node(ImGuiContext *imgui_c89_ctx, const ImVec2 * target_pos);
void imgui__tree_node_draw_line_to_tree_pop(ImGuiContext *imgui_c89_ctx, const ImGuiTreeNodeStackData * data);
unsigned char imgui__tree_node_ex_string_tree_node_flags_string_varargs(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiTreeNodeFlags flags, const char * fmt, ...);
unsigned char imgui__tree_node_ex_const_pointer_tree_node_flags_string_varargs(ImGuiContext *imgui_c89_ctx, const void * ptr_id, ImGuiTreeNodeFlags flags, const char * fmt, ...);
void imgui__tree_node_set_open(ImGuiContext *imgui_c89_ctx, ImGuiID storage_id, unsigned char is_open);
unsigned char imgui__tree_node_update_next_open(ImGuiContext *imgui_c89_ctx, ImGuiID storage_id, ImGuiTreeNodeFlags flags);
void imgui__tree_push_override_id(ImGuiContext *imgui_c89_ctx, ImGuiID id);
int imgui__typing_select_find_best_leading_match(ImGuiTypingSelectRequest * req, int items_count, const char *(*get_item_name_func)(void *, int), void * user_data);
int imgui__typing_select_find_match(ImGuiContext *imgui_c89_ctx, ImGuiTypingSelectRequest * req, int items_count, const char *(*get_item_name_func)(void *, int), void * user_data, int nav_item_idx);
int imgui__typing_select_find_next_single_char_match(ImGuiTypingSelectRequest * req, int items_count, const char *(*get_item_name_func)(void *, int), void * user_data, int nav_item_idx);
void imgui__unregister_font_atlas(ImGuiContext *imgui_c89_ctx, ImFontAtlas * atlas);
void imgui__unregister_user_texture(ImGuiContext *imgui_c89_ctx, ImTextureData * tex);
void imgui__update_current_font_size(ImGuiContext *imgui_c89_ctx, float restore_font_size_after_scaling);
void imgui__update_hovered_window_and_capture_flags(ImGuiContext *imgui_c89_ctx, const ImVec2 * mouse_pos);
void imgui__update_input_events(ImGuiContext *imgui_c89_ctx, unsigned char trickle_fast_inputs);
void imgui__update_mouse_moving_window_end_frame(ImGuiContext *imgui_c89_ctx);
void imgui__update_mouse_moving_window_new_frame(ImGuiContext *imgui_c89_ctx);
void imgui__update_window_parent_and_root_links(ImGuiWindow * window, ImGuiWindowFlags flags, ImGuiWindow * parent_window);
void imgui__update_window_skip_refresh(ImGuiContext *imgui_c89_ctx, ImGuiWindow * window);
ImVec2 ImGui_WindowPosAbsToRel__d80e1f0105(ImGuiWindow * window, const ImVec2 * p);
ImVec2 ImGui_WindowPosRelToAbs__fb3fdede90(ImGuiWindow * window, const ImVec2 * p);
ImRect ImGui_WindowRectAbsToRel__7725b098d7(ImGuiWindow * window, const ImRect * r);
void ImBitArray_155___512__ClearBit__1523841161(ImBitArray_155_512 *self, int n);
void ImBitArray_155___512__SetAllBits__ba00f6832e(ImBitArray_155_512 *self);
void ImBitArray_155___512__SetBitRange__c4edaf94bc(ImBitArray_155_512 *self, int n, int n2);
unsigned char ImBitArray_155___512__operator____d4392115f0(ImBitArray_155_512 *self, int n);
void ImBitArray_155__ClearBit__8146509f72(ImBitArray_155 *self, int n);
void ImBitArray_155__SetAllBits__d01861344e(ImBitArray_155 *self);
void ImBitArray_155__SetBitRange__5917dda214(ImBitArray_155 *self, int n, int n2);
unsigned char ImBitArray_155__operator____032ad77886(ImBitArray_155 *self, int n);
ImGuiTableSettings * ImChunkStream_ImGuiTableSettings__begin__f2068e0963(ImChunkStream_ImGuiTableSettings *self);
unsigned char ImChunkStream_ImGuiTableSettings__empty__bef6a3583b(ImChunkStream_ImGuiTableSettings *self);
ImGuiTableSettings * ImChunkStream_ImGuiTableSettings__end__d7e9073b0d(ImChunkStream_ImGuiTableSettings *self);
ImGuiTableSettings * ImChunkStream_ImGuiTableSettings__next_chunk__b4a289c3c3(ImChunkStream_ImGuiTableSettings *self, ImGuiTableSettings * p);
void ImChunkStream_ImGuiWindowSettings__swap__59fda3196d(ImChunkStream_ImGuiWindowSettings *self, ImChunkStream_ImGuiWindowSettings * rhs);
void ImDrawCmd_ImDrawCmd__7f0b9f10b7(ImDrawCmd *self);
void imgui__im_draw_list_init(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImDrawListSharedData * shared_data);
void imgui__im_draw_list_destroy(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui__im_draw_list_shared_data_init(ImDrawListSharedData *self);
void imgui__im_draw_list_shared_data_set_circle_tessellation_max_error(ImDrawListSharedData *self, float max_error);
void imgui__im_draw_list_shared_data_destroy(ImGuiContext *imgui_c89_ctx, ImDrawListSharedData *self);
void ImDrawListSplitter_ImDrawListSplitter__d0ba8fa7b0(ImDrawListSplitter *self);
void ImDrawListSplitter_dtor_ImDrawListSplitter__838f52a7ff(ImGuiContext *imgui_c89_ctx, ImDrawListSplitter *self);
void imgui__im_font_init(ImFont *self);
void imgui__im_font_destroy(ImGuiContext *imgui_c89_ctx, ImFont *self);
void imgui__im_font_atlas_init(ImFontAtlas *self);
void imgui__im_font_atlas_destroy(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
void ImFontAtlasBuilder_ImFontAtlasBuilder__dc97765d12(ImFontAtlasBuilder *self);
void ImFontAtlasRect_ImFontAtlasRect__a33e8f2147(ImFontAtlasRect *self);
void imgui__im_font_baked_init(ImFontBaked *self);
void imgui__im_font_config_init(ImFontConfig *self);
void ImFontGlyph_ImFontGlyph__28375ed6b5(ImFontGlyph *self);
void ImFontLoader_ImFontLoader__ccf02b9543(ImFontLoader *self);
void imgui__context_init(ImGuiContext *imgui_c89_ctx, ImGuiContext *self, ImFontAtlas * shared_font_atlas);
void imgui__context_destroy(ImGuiContext *imgui_c89_ctx, ImGuiContext *self);
void imgui__io_init(ImGuiIO *self);
void imgui__input_text_callback_data_init(ImGuiInputTextCallbackData *self);
void imgui__input_text_state_clear_selection(ImGuiInputTextState *self);
void imgui__input_text_state_cursor_anim_reset(ImGuiInputTextState *self);
void imgui__input_text_state_cursor_clamp(ImGuiInputTextState *self);
int imgui__input_text_state_get_cursor_pos(ImGuiInputTextState *self);
float imgui__input_text_state_get_preferred_offset_x(ImGuiInputTextState *self);
int imgui__input_text_state_get_selection_end(ImGuiInputTextState *self);
int imgui__input_text_state_get_selection_start(ImGuiInputTextState *self);
unsigned char imgui__input_text_state_has_selection(ImGuiInputTextState *self);
void imgui__input_text_state_init(ImGuiContext *imgui_c89_ctx, ImGuiInputTextState *self);
void imgui__input_text_state_on_char_pressed(ImGuiContext *imgui_c89_ctx, ImGuiInputTextState *self, unsigned int c);
void imgui__input_text_state_on_key_pressed(ImGuiContext *imgui_c89_ctx, ImGuiInputTextState *self, int key);
void imgui__input_text_state_reload_user_buf_and_keep_selection(ImGuiInputTextState *self);
void imgui__input_text_state_reload_user_buf_and_move_to_end(ImGuiInputTextState *self);
void imgui__input_text_state_reload_user_buf_and_select_all(ImGuiInputTextState *self);
void imgui__input_text_state_select_all(ImGuiInputTextState *self);
void imgui__input_text_state_set_selection(ImGuiInputTextState *self, int start, int end);
void imgui__input_text_state_destroy(ImGuiContext *imgui_c89_ctx, ImGuiInputTextState *self);
void ImGuiLastItemData_ImGuiLastItemData__2eec235511(ImGuiLastItemData *self);
void imgui__list_clipper_init(ImGuiListClipper *self);
void imgui__list_clipper_destroy(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self);
void imgui__menu_columns_calc_next_total_width(ImGuiMenuColumns *self, unsigned char update_offsets);
float imgui__menu_columns_decl_columns(ImGuiMenuColumns *self, float w_icon, float w_label, float w_shortcut, float w_mark);
void imgui__menu_columns_update(ImGuiMenuColumns *self, float spacing, unsigned char window_reappearing);
void ImGuiMultiSelectState_ImGuiMultiSelectState__03efebd571(ImGuiMultiSelectState *self);
void ImGuiMultiSelectTempData_Clear__40cc00d8bb(ImGuiContext *imgui_c89_ctx, ImGuiMultiSelectTempData *self);
void ImGuiMultiSelectTempData_ImGuiMultiSelectTempData__1b2162e50a(ImGuiContext *imgui_c89_ctx, ImGuiMultiSelectTempData *self);
void ImGuiNextItemData_ClearFlags__3d6601f626(ImGuiNextItemData *self);
void ImGuiNextWindowData_ClearFlags__5c145439ca(ImGuiNextWindowData *self);
unsigned char ImGuiPackedDate_IsValid__ee10de0281(ImGuiPackedDate *self);
int ImGuiPackedDate_Unpack__6d04c4f8e1(ImGuiPackedDate *self);
void imgui__platform_io_init(ImGuiPlatformIO *self);
void ImGuiPtrOrIndex_ImGuiPtrOrIndex__a12bce3150(ImGuiPtrOrIndex *self, void * ptr);
void ImGuiPtrOrIndex_ImGuiPtrOrIndex__b69285f83f(ImGuiPtrOrIndex *self, int index);
void imgui__selection_basic_storage_init(ImGuiSelectionBasicStorage *self);
void imgui__selection_external_storage_init(ImGuiSelectionExternalStorage *self);
void ImGuiStoragePair_ImGuiStoragePair__a5d3a535fc(ImGuiStoragePair *self, ImGuiID _key, int _val);
void imgui__style_init(ImGuiContext *imgui_c89_ctx, ImGuiStyle *self);
void imgui__tab_bar_init(ImGuiTabBar *self);
void ImGuiTabItem_ImGuiTabItem__9e73f47f28(ImGuiTabItem *self);
void imgui__table_fini(ImGuiContext *imgui_c89_ctx, ImGuiTable *self);
void ImGuiTextBuffer_ImGuiTextBuffer__cf299f8f93(ImGuiTextBuffer *self);
void imgui__text_buffer_appendf(ImGuiContext *imgui_c89_ctx, ImGuiTextBuffer *self, const char * fmt, ...);
void imgui__text_filter_init(ImGuiContext *imgui_c89_ctx, ImGuiTextFilter *self, const char * default_filter);
void imgui__text_index_append(ImGuiContext *imgui_c89_ctx, ImGuiTextIndex *self, const char * base, int old_size, int new_size);
const char * ImGuiTextIndex_get_line_begin__5b804528d7(ImGuiTextIndex *self, const char * base, int n);
const char * ImGuiTextIndex_get_line_end__433c94708a(ImGuiTextIndex *self, const char * base, int n);
void ImGuiTypingSelectState_Clear__a5970bc428(ImGuiTypingSelectState *self);
ImRect ImGuiViewportP_GetBuildWorkRect__43e7d5c2ca(ImGuiViewportP *self);
ImRect ImGuiViewportP_GetMainRect__fb7e78d91b(ImGuiViewportP *self);
ImGuiID imgui__window_get_id_string_string(ImGuiContext *imgui_c89_ctx, ImGuiWindow *self, const char * str, const char * str_end);
ImGuiID imgui__window_get_id_const_pointer(ImGuiContext *imgui_c89_ctx, ImGuiWindow *self, const void * ptr);
ImGuiID imgui__window_get_id_int(ImGuiContext *imgui_c89_ctx, ImGuiWindow *self, int n);
ImGuiID imgui__window_get_id_from_pos(ImGuiWindow *self, const ImVec2 * p_abs);
ImGuiID imgui__window_get_id_from_rectangle(ImGuiWindow *self, const ImRect * r_abs);
void imgui__window_init(ImGuiContext *imgui_c89_ctx, ImGuiWindow *self, ImGuiContext * ctx, const char * name);
ImRect ImGuiWindow_MenuBarRect__0336d8f646(ImGuiWindow *self);
ImRect ImGuiWindow_Rect__460e84dccd(ImGuiWindow *self);
void imgui__window_destroy(ImGuiContext *imgui_c89_ctx, ImGuiWindow *self);
unsigned char ImPool_ImGuiMultiSelectState__Contains__c268a2f74a(ImPool_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * p);
int ImPool_ImGuiMultiSelectState__GetBufSize__01b09428b6(ImPool_ImGuiMultiSelectState *self);
ImPoolIdx ImPool_ImGuiMultiSelectState__GetIndex__af2b3e1f52(ImPool_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * p);
void ImPool_ImGuiMultiSelectState__Remove__51e73082ee(ImPool_ImGuiMultiSelectState *self, ImGuiID key, const ImGuiMultiSelectState * p);
void ImPool_ImGuiMultiSelectState__Remove__37645ae633(ImPool_ImGuiMultiSelectState *self, ImGuiID key, ImPoolIdx idx);
void ImPool_ImGuiMultiSelectState__Reserve__c8805f032b(ImPool_ImGuiMultiSelectState *self, int capacity);
int ImPool_ImGuiTabBar__GetBufSize__710d35ebcf(ImPool_ImGuiTabBar *self);
ImGuiTabBar * ImPool_ImGuiTabBar__GetByIndex__80660b82f1(ImPool_ImGuiTabBar *self, ImPoolIdx n);
void ImPool_ImGuiTabBar__Reserve__1eb47979d2(ImPool_ImGuiTabBar *self, int capacity);
unsigned char imgui__table_pool_contains(ImGuiTablePool *self, const ImGuiTable * p);
int imgui__table_pool_size(ImGuiTablePool *self);
ImGuiTable * imgui__table_pool_at(ImGuiTablePool *self, ImPoolIdx n);
int imgui__table_pool_map_size(ImGuiTablePool *self);
void imgui__table_pool_reserve(ImGuiTablePool *self, int capacity);
ImGuiTable * imgui__table_pool_map_at(ImGuiTablePool *self, ImPoolIdx n);
void ImRect_Add__3d3cf3b5dc(ImRect *self, const ImRect * r);
void ImRect_AddX__d602d7db92(ImRect *self, float x);
void ImRect_AddY__4a8e999125(ImRect *self, float y);
const ImVec4 * ImRect_AsVec4__821165f0d0(ImRect *self);
void ImRect_ClipWith__be335bbfd9(ImRect *self, const ImRect * r);
void ImRect_ClipWithFull__ea61dd1d9a(ImRect *self, const ImRect * r);
unsigned char ImRect_Contains__60a2869552(ImRect *self, const ImVec2 * p);
void ImRect_Expand__cf4e5ff370(ImRect *self, const ImVec2 * amount);
void ImRect_Expand__d4c31dde12(ImRect *self, const float amount);
float ImRect_GetArea__d99e9ca7ee(ImRect *self);
ImVec2 ImRect_GetBL__431ad3739c(ImRect *self);
ImVec2 ImRect_GetBR__8f4f795066(ImRect *self);
ImVec2 ImRect_GetCenter__a01005e528(ImRect *self);
float ImRect_GetHeight__bcfe92168a(ImRect *self);
ImVec2 ImRect_GetSize__c91c92deb7(ImRect *self);
ImVec2 ImRect_GetTL__7329ec7894(ImRect *self);
ImVec2 ImRect_GetTR__e804f7073a(ImRect *self);
float ImRect_GetWidth__7eeda790fc(ImRect *self);
void ImRect_ImRect__1bc503c88f(ImRect *self);
void ImRect_ImRect__b1b06c34ac(ImRect *self, const ImVec2 * min, const ImVec2 * max);
void ImRect_ImRect__9d29cc465f(ImRect *self, float x1, float y1, float x2, float y2);
unsigned char ImRect_IsInverted__46b5b9bd7e(ImRect *self);
unsigned char ImRect_Overlaps__02ba25d225(ImRect *self, const ImRect * r);
void ImRect_Translate__44699a5176(ImRect *self, const ImVec2 * d);
void ImStableVector_ImFontBaked__32__clear__9a61fd13f4(ImStableVector_ImFontBaked_32 *self);
ImFontBaked * ImStableVector_ImFontBaked__32__operator____abbc4372e3(ImStableVector_ImFontBaked_32 *self, int i);
const ImFontBaked * ImStableVector_ImFontBaked__32__operator____e34aee4682(ImStableVector_ImFontBaked_32 *self, int i);
void ImStableVector_ImFontBaked__32__resize__58384c8c35(ImStableVector_ImFontBaked_32 *self, int new_size);
void ImTextureData_ImTextureData__491ad6a35b(ImTextureData *self);
void ImTextureData_dtor_ImTextureData__b906dc34ab(ImGuiContext *imgui_c89_ctx, ImTextureData *self);
void ImTextureRef_ImTextureRef__b4471d7198(ImTextureRef *self);
void ImVec2_ImVec2__22b45734f5(ImVec2 *self);
void ImVec2_ImVec2__2b588b6e68(ImVec2 *self, float _x, float _y);
float * ImVec2_operator____051d42fa54(ImVec2 *self, size_t idx);
float ImVec2_operator____dddfb65966(ImVec2 *self, size_t idx);
void ImVec2i_ImVec2i__9d63c0311f(ImVec2i *self, int _x, int _y);
void ImVec4_ImVec4__711f95b97f(ImVec4 *self);
void ImVec4_ImVec4__2b63b1272a(ImVec4 *self, float _x, float _y, float _z, float _w);
void ImVector_ImDrawChannel__ImVector__ea880e62ea(ImVector_ImDrawChannel *self, const ImVector_ImDrawChannel * src);
ImDrawChannel * ImVector_ImDrawChannel__back__0f46c3e6a4(ImVector_ImDrawChannel *self);
const ImDrawChannel * ImVector_ImDrawChannel__back__f5c863be13(ImVector_ImDrawChannel *self);
ImDrawChannel * ImVector_ImDrawChannel__begin__277322a902(ImVector_ImDrawChannel *self);
const ImDrawChannel * ImVector_ImDrawChannel__begin__cb7a0c770e(ImVector_ImDrawChannel *self);
int ImVector_ImDrawChannel__capacity__03046050ef(ImVector_ImDrawChannel *self);
void ImVector_ImDrawChannel__clear__de3ff3e70a(ImGuiContext *imgui_c89_ctx, ImVector_ImDrawChannel *self);
void ImVector_ImDrawChannel__clear_delete__fcbf7a3608(ImVector_ImDrawChannel *self);
void ImVector_ImDrawChannel__clear_destruct__7bd92dd24b(ImVector_ImDrawChannel *self);
unsigned char ImVector_ImDrawChannel__contains__4c407a0d8c(ImVector_ImDrawChannel *self, const ImDrawChannel * v);
unsigned char ImVector_ImDrawChannel__empty__c67853b676(ImVector_ImDrawChannel *self);
ImDrawChannel * ImVector_ImDrawChannel__end__c4c4339b99(ImVector_ImDrawChannel *self);
const ImDrawChannel * ImVector_ImDrawChannel__end__ee2b13d3c0(ImVector_ImDrawChannel *self);
ImDrawChannel * ImVector_ImDrawChannel__erase__fc7d92891f(ImVector_ImDrawChannel *self, const ImDrawChannel * it);
ImDrawChannel * ImVector_ImDrawChannel__erase__d564fa15b9(ImVector_ImDrawChannel *self, const ImDrawChannel * it, const ImDrawChannel * it_last);
ImDrawChannel * ImVector_ImDrawChannel__erase_unsorted__90b450064d(ImVector_ImDrawChannel *self, const ImDrawChannel * it);
ImDrawChannel * ImVector_ImDrawChannel__find__720ad93984(ImVector_ImDrawChannel *self, const ImDrawChannel * v);
const ImDrawChannel * ImVector_ImDrawChannel__find__6fbb28e1ba(ImVector_ImDrawChannel *self, const ImDrawChannel * v);
unsigned char ImVector_ImDrawChannel__find_erase__b8e446e977(ImVector_ImDrawChannel *self, const ImDrawChannel * v);
unsigned char ImVector_ImDrawChannel__find_erase_unsorted__df547cc904(ImVector_ImDrawChannel *self, const ImDrawChannel * v);
int ImVector_ImDrawChannel__find_index__9fc75a4e0f(ImVector_ImDrawChannel *self, const ImDrawChannel * v);
ImDrawChannel * ImVector_ImDrawChannel__front__d4670397e5(ImVector_ImDrawChannel *self);
const ImDrawChannel * ImVector_ImDrawChannel__front__c1c396fdb8(ImVector_ImDrawChannel *self);
int ImVector_ImDrawChannel__index_from_ptr__a9e1833666(ImVector_ImDrawChannel *self, const ImDrawChannel * it);
ImDrawChannel * ImVector_ImDrawChannel__insert__46281a89bb(ImVector_ImDrawChannel *self, const ImDrawChannel * it, const ImDrawChannel * v);
int ImVector_ImDrawChannel__max_size__4a9e394ade(ImVector_ImDrawChannel *self);
ImVector_ImDrawChannel * ImVector_ImDrawChannel__operator___1af36d5334(ImVector_ImDrawChannel *self, const ImVector_ImDrawChannel * src);
const ImDrawChannel * ImVector_ImDrawChannel__operator____bff4d5d45e(ImVector_ImDrawChannel *self, int i);
void ImVector_ImDrawChannel__pop_back__8d2d2f46a0(ImVector_ImDrawChannel *self);
void ImVector_ImDrawChannel__push_back__adc6c3610f(ImVector_ImDrawChannel *self, const ImDrawChannel * v);
void ImVector_ImDrawChannel__push_front__f631c44019(ImVector_ImDrawChannel *self, const ImDrawChannel * v);
void ImVector_ImDrawChannel__reserve_discard__03d72bfed7(ImVector_ImDrawChannel *self, int new_capacity);
void ImVector_ImDrawChannel__resize__b5c8399512(ImGuiContext *imgui_c89_ctx, ImVector_ImDrawChannel *self, int new_size);
void ImVector_ImDrawChannel__resize__28e6b48db9(ImVector_ImDrawChannel *self, int new_size, const ImDrawChannel * v);
void ImVector_ImDrawChannel__shrink__20ec3bf4fe(ImVector_ImDrawChannel *self, int new_size);
int ImVector_ImDrawChannel__size__c9525d4dce(ImVector_ImDrawChannel *self);
int ImVector_ImDrawChannel__size_in_bytes__0a2c008974(ImVector_ImDrawChannel *self);
void ImVector_ImDrawChannel__swap__e557f36844(ImVector_ImDrawChannel *self, ImVector_ImDrawChannel * rhs);
void ImVector_ImDrawCmd__ImVector__d64e59cc2f(ImVector_ImDrawCmd *self, const ImVector_ImDrawCmd * src);
int ImVector_ImDrawCmd___grow_capacity__6be98bd1f5(ImVector_ImDrawCmd *self, int sz);
int ImVector_ImDrawCmd__capacity__ddd7424317(ImVector_ImDrawCmd *self);
void ImVector_ImDrawCmd__clear_delete__9cc9b5297c(ImVector_ImDrawCmd *self);
void ImVector_ImDrawCmd__clear_destruct__0d7ad3753b(ImVector_ImDrawCmd *self);
unsigned char ImVector_ImDrawCmd__contains__edaf2fdd86(ImVector_ImDrawCmd *self, const ImDrawCmd * v);
unsigned char ImVector_ImDrawCmd__empty__c81aa69ad3(ImVector_ImDrawCmd *self);
const ImDrawCmd * ImVector_ImDrawCmd__end__6c841a1858(ImVector_ImDrawCmd *self);
ImDrawCmd * ImVector_ImDrawCmd__erase__e2923ea122(ImVector_ImDrawCmd *self, const ImDrawCmd * it, const ImDrawCmd * it_last);
ImDrawCmd * ImVector_ImDrawCmd__erase_unsorted__ca3aaff778(ImVector_ImDrawCmd *self, const ImDrawCmd * it);
ImDrawCmd * ImVector_ImDrawCmd__find__9dbcfef702(ImVector_ImDrawCmd *self, const ImDrawCmd * v);
const ImDrawCmd * ImVector_ImDrawCmd__find__d2eb9071fb(ImVector_ImDrawCmd *self, const ImDrawCmd * v);
unsigned char ImVector_ImDrawCmd__find_erase__35ce945db0(ImVector_ImDrawCmd *self, const ImDrawCmd * v);
unsigned char ImVector_ImDrawCmd__find_erase_unsorted__0929f43a5c(ImVector_ImDrawCmd *self, const ImDrawCmd * v);
int ImVector_ImDrawCmd__find_index__04a96bb469(ImVector_ImDrawCmd *self, const ImDrawCmd * v);
ImDrawCmd * ImVector_ImDrawCmd__front__3fe5253689(ImVector_ImDrawCmd *self);
const ImDrawCmd * ImVector_ImDrawCmd__front__5428d18cc7(ImVector_ImDrawCmd *self);
int ImVector_ImDrawCmd__index_from_ptr__5ee242405a(ImVector_ImDrawCmd *self, const ImDrawCmd * it);
int ImVector_ImDrawCmd__max_size__2d4ebc9968(ImVector_ImDrawCmd *self);
ImDrawCmd * ImVector_ImDrawCmd__operator____4e49f65cd2(ImVector_ImDrawCmd *self, int i);
const ImDrawCmd * ImVector_ImDrawCmd__operator____5b15da2ddf(ImVector_ImDrawCmd *self, int i);
void ImVector_ImDrawCmd__pop_back__4a4254c60d(ImVector_ImDrawCmd *self);
void ImVector_ImDrawCmd__push_back__158c3ecb3d(ImGuiContext *imgui_c89_ctx, ImVector_ImDrawCmd *self, const ImDrawCmd * v);
void ImVector_ImDrawCmd__reserve__a89a2ead10(ImGuiContext *imgui_c89_ctx, ImVector_ImDrawCmd *self, int new_capacity);
void ImVector_ImDrawCmd__reserve_discard__f3e02fc6db(ImVector_ImDrawCmd *self, int new_capacity);
void ImVector_ImDrawCmd__resize__89432f1cb4(ImVector_ImDrawCmd *self, int new_size, const ImDrawCmd * v);
void ImVector_ImDrawCmd__shrink__44c1a25c89(ImVector_ImDrawCmd *self, int new_size);
int ImVector_ImDrawCmd__size__cd6bc2ba11(ImVector_ImDrawCmd *self);
int ImVector_ImDrawCmd__size_in_bytes__11a504b668(ImVector_ImDrawCmd *self);
void ImVector_ImDrawCmd__swap__ba9dbbb444(ImVector_ImDrawCmd *self, ImVector_ImDrawCmd * rhs);
void ImVector_ImDrawVert__ImVector__60c5e41940(ImVector_ImDrawVert *self, const ImVector_ImDrawVert * src);
ImDrawVert * ImVector_ImDrawVert__back__2d66c5e0e8(ImVector_ImDrawVert *self);
const ImDrawVert * ImVector_ImDrawVert__back__ac54a330a0(ImVector_ImDrawVert *self);
ImDrawVert * ImVector_ImDrawVert__begin__0f915dc2ce(ImVector_ImDrawVert *self);
const ImDrawVert * ImVector_ImDrawVert__begin__4d8598aec8(ImVector_ImDrawVert *self);
int ImVector_ImDrawVert__capacity__39df08b9fe(ImVector_ImDrawVert *self);
void ImVector_ImDrawVert__clear_delete__e4f076621d(ImVector_ImDrawVert *self);
void ImVector_ImDrawVert__clear_destruct__c23bfec0fa(ImVector_ImDrawVert *self);
unsigned char ImVector_ImDrawVert__contains__2ac3631820(ImVector_ImDrawVert *self, const ImDrawVert * v);
unsigned char ImVector_ImDrawVert__empty__10895b6b6b(ImVector_ImDrawVert *self);
ImDrawVert * ImVector_ImDrawVert__end__966485184f(ImVector_ImDrawVert *self);
const ImDrawVert * ImVector_ImDrawVert__end__647086e061(ImVector_ImDrawVert *self);
ImDrawVert * ImVector_ImDrawVert__erase__852cca82c3(ImVector_ImDrawVert *self, const ImDrawVert * it);
ImDrawVert * ImVector_ImDrawVert__erase__40dfdf140f(ImVector_ImDrawVert *self, const ImDrawVert * it, const ImDrawVert * it_last);
ImDrawVert * ImVector_ImDrawVert__erase_unsorted__9fe2e70160(ImVector_ImDrawVert *self, const ImDrawVert * it);
ImDrawVert * ImVector_ImDrawVert__find__01c4b55b7c(ImVector_ImDrawVert *self, const ImDrawVert * v);
const ImDrawVert * ImVector_ImDrawVert__find__1aae0985af(ImVector_ImDrawVert *self, const ImDrawVert * v);
unsigned char ImVector_ImDrawVert__find_erase__ae1c10bd8c(ImVector_ImDrawVert *self, const ImDrawVert * v);
unsigned char ImVector_ImDrawVert__find_erase_unsorted__a8512fa7e7(ImVector_ImDrawVert *self, const ImDrawVert * v);
int ImVector_ImDrawVert__find_index__bc92786287(ImVector_ImDrawVert *self, const ImDrawVert * v);
ImDrawVert * ImVector_ImDrawVert__front__35a5bd8f5d(ImVector_ImDrawVert *self);
const ImDrawVert * ImVector_ImDrawVert__front__b67b091fce(ImVector_ImDrawVert *self);
int ImVector_ImDrawVert__index_from_ptr__eecb565463(ImVector_ImDrawVert *self, const ImDrawVert * it);
ImDrawVert * ImVector_ImDrawVert__insert__a703e79a5d(ImVector_ImDrawVert *self, const ImDrawVert * it, const ImDrawVert * v);
int ImVector_ImDrawVert__max_size__c54764746f(ImVector_ImDrawVert *self);
const ImDrawVert * ImVector_ImDrawVert__operator____ba3bd716fb(ImVector_ImDrawVert *self, int i);
void ImVector_ImDrawVert__pop_back__2b7bbc05b5(ImVector_ImDrawVert *self);
void ImVector_ImDrawVert__push_back__ae4a55be66(ImVector_ImDrawVert *self, const ImDrawVert * v);
void ImVector_ImDrawVert__push_front__1afd35168f(ImVector_ImDrawVert *self, const ImDrawVert * v);
void ImVector_ImDrawVert__reserve__2cccf7446f(ImGuiContext *imgui_c89_ctx, ImVector_ImDrawVert *self, int new_capacity);
void ImVector_ImDrawVert__reserve_discard__f735a29cb9(ImVector_ImDrawVert *self, int new_capacity);
void ImVector_ImDrawVert__resize__31b86d0f2b(ImVector_ImDrawVert *self, int new_size, const ImDrawVert * v);
int ImVector_ImDrawVert__size__fbeeb5badb(ImVector_ImDrawVert *self);
int ImVector_ImDrawVert__size_in_bytes__c62562d8e9(ImVector_ImDrawVert *self);
ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__back__4f70040b84(ImVector_ImFontAtlasRectEntry *self);
const ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__back__e960e5f358(ImVector_ImFontAtlasRectEntry *self);
const ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__begin__f75d4ea596(ImVector_ImFontAtlasRectEntry *self);
int ImVector_ImFontAtlasRectEntry__capacity__0dbe9aeac3(ImVector_ImFontAtlasRectEntry *self);
void ImVector_ImFontAtlasRectEntry__clear_delete__ac950f2f12(ImVector_ImFontAtlasRectEntry *self);
void ImVector_ImFontAtlasRectEntry__clear_destruct__72ece7cf15(ImVector_ImFontAtlasRectEntry *self);
unsigned char ImVector_ImFontAtlasRectEntry__contains__a240ff5677(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * v);
unsigned char ImVector_ImFontAtlasRectEntry__empty__3dba663eb9(ImVector_ImFontAtlasRectEntry *self);
const ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__end__17a891e605(ImVector_ImFontAtlasRectEntry *self);
ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__erase__1b064d85ed(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * it);
ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__erase__52d23ca3a5(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * it, const ImFontAtlasRectEntry * it_last);
ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__erase_unsorted__a51d889de8(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * it);
ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__find__8255a8ffd7(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * v);
const ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__find__253f73db23(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * v);
unsigned char ImVector_ImFontAtlasRectEntry__find_erase__ce575a88a2(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * v);
unsigned char ImVector_ImFontAtlasRectEntry__find_erase_unsorted__9265461e04(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * v);
int ImVector_ImFontAtlasRectEntry__find_index__3785b98a01(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * v);
ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__front__07246b67a7(ImVector_ImFontAtlasRectEntry *self);
const ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__front__bc5d00d770(ImVector_ImFontAtlasRectEntry *self);
ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__insert__655161cf6a(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * it, const ImFontAtlasRectEntry * v);
int ImVector_ImFontAtlasRectEntry__max_size__2eb464f308(ImVector_ImFontAtlasRectEntry *self);
const ImFontAtlasRectEntry * ImVector_ImFontAtlasRectEntry__operator____17126a7637(ImVector_ImFontAtlasRectEntry *self, int i);
void ImVector_ImFontAtlasRectEntry__pop_back__537f7b7341(ImVector_ImFontAtlasRectEntry *self);
void ImVector_ImFontAtlasRectEntry__push_back__63d114e16f(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * v);
void ImVector_ImFontAtlasRectEntry__push_front__4d52f4a596(ImVector_ImFontAtlasRectEntry *self, const ImFontAtlasRectEntry * v);
void ImVector_ImFontAtlasRectEntry__reserve_discard__1f89061356(ImVector_ImFontAtlasRectEntry *self, int new_capacity);
void ImVector_ImFontAtlasRectEntry__resize__6d6266590d(ImVector_ImFontAtlasRectEntry *self, int new_size, const ImFontAtlasRectEntry * v);
void ImVector_ImFontAtlasRectEntry__shrink__bda90aba66(ImVector_ImFontAtlasRectEntry *self, int new_size);
int ImVector_ImFontAtlasRectEntry__size__0b387e42fd(ImVector_ImFontAtlasRectEntry *self);
int ImVector_ImFontAtlasRectEntry__size_in_bytes__924e95675f(ImVector_ImFontAtlasRectEntry *self);
void ImVector_ImFontAtlasRectEntry__swap__cb98209dea(ImVector_ImFontAtlasRectEntry *self, ImVector_ImFontAtlasRectEntry * rhs);
void ImVector_ImFontConfig__ImVector__dc54a05cd2(ImVector_ImFontConfig *self, const ImVector_ImFontConfig * src);
const ImFontConfig * ImVector_ImFontConfig__back__fd82dd0398(ImVector_ImFontConfig *self);
const ImFontConfig * ImVector_ImFontConfig__begin__ac27c0e44d(ImVector_ImFontConfig *self);
int ImVector_ImFontConfig__capacity__ad0fc67fdc(ImVector_ImFontConfig *self);
void ImVector_ImFontConfig__clear_delete__8f553ae4a9(ImVector_ImFontConfig *self);
void ImVector_ImFontConfig__clear_destruct__eea7d28307(ImVector_ImFontConfig *self);
unsigned char ImVector_ImFontConfig__contains__502268b27c(ImVector_ImFontConfig *self, const ImFontConfig * v);
unsigned char ImVector_ImFontConfig__empty__40068deee2(ImVector_ImFontConfig *self);
const ImFontConfig * ImVector_ImFontConfig__end__de27cbeceb(ImVector_ImFontConfig *self);
ImFontConfig * ImVector_ImFontConfig__erase__2d2cefed92(ImVector_ImFontConfig *self, const ImFontConfig * it, const ImFontConfig * it_last);
ImFontConfig * ImVector_ImFontConfig__erase_unsorted__999fe6c4e1(ImVector_ImFontConfig *self, const ImFontConfig * it);
ImFontConfig * ImVector_ImFontConfig__find__437e7d89c0(ImVector_ImFontConfig *self, const ImFontConfig * v);
const ImFontConfig * ImVector_ImFontConfig__find__25cd41a846(ImVector_ImFontConfig *self, const ImFontConfig * v);
unsigned char ImVector_ImFontConfig__find_erase__265d48eeb9(ImVector_ImFontConfig *self, const ImFontConfig * v);
unsigned char ImVector_ImFontConfig__find_erase_unsorted__659e2f8474(ImVector_ImFontConfig *self, const ImFontConfig * v);
int ImVector_ImFontConfig__find_index__55693f4294(ImVector_ImFontConfig *self, const ImFontConfig * v);
ImFontConfig * ImVector_ImFontConfig__front__b6d9df88ef(ImVector_ImFontConfig *self);
const ImFontConfig * ImVector_ImFontConfig__front__26732c392d(ImVector_ImFontConfig *self);
int ImVector_ImFontConfig__index_from_ptr__6d6b0ef4d9(ImVector_ImFontConfig *self, const ImFontConfig * it);
ImFontConfig * ImVector_ImFontConfig__insert__56e5d46af3(ImVector_ImFontConfig *self, const ImFontConfig * it, const ImFontConfig * v);
int ImVector_ImFontConfig__max_size__29af00e9a0(ImVector_ImFontConfig *self);
ImVector_ImFontConfig * ImVector_ImFontConfig__operator___dd73987a3b(ImVector_ImFontConfig *self, const ImVector_ImFontConfig * src);
const ImFontConfig * ImVector_ImFontConfig__operator____488ee57081(ImVector_ImFontConfig *self, int i);
void ImVector_ImFontConfig__push_front__07684f7bcc(ImVector_ImFontConfig *self, const ImFontConfig * v);
void ImVector_ImFontConfig__reserve_discard__85c729d25c(ImVector_ImFontConfig *self, int new_capacity);
void ImVector_ImFontConfig__resize__600bc56123(ImVector_ImFontConfig *self, int new_size);
void ImVector_ImFontConfig__resize__6937e6f78c(ImVector_ImFontConfig *self, int new_size, const ImFontConfig * v);
void ImVector_ImFontConfig__shrink__a2104f6c27(ImVector_ImFontConfig *self, int new_size);
int ImVector_ImFontConfig__size__7f29efc3c0(ImVector_ImFontConfig *self);
int ImVector_ImFontConfig__size_in_bytes__838329928c(ImVector_ImFontConfig *self);
void ImVector_ImFontConfig__swap__a2be0d6f8e(ImVector_ImFontConfig *self, ImVector_ImFontConfig * rhs);
void ImVector_ImFontGlyph__ImVector__1dad1637c8(ImVector_ImFontGlyph *self, const ImVector_ImFontGlyph * src);
ImFontGlyph * ImVector_ImFontGlyph__back__7baf940d82(ImVector_ImFontGlyph *self);
const ImFontGlyph * ImVector_ImFontGlyph__back__cd865e3df4(ImVector_ImFontGlyph *self);
const ImFontGlyph * ImVector_ImFontGlyph__begin__b9a2e05a4c(ImVector_ImFontGlyph *self);
int ImVector_ImFontGlyph__capacity__8251dbcd53(ImVector_ImFontGlyph *self);
void ImVector_ImFontGlyph__clear_delete__0334df05a6(ImVector_ImFontGlyph *self);
void ImVector_ImFontGlyph__clear_destruct__2d4b69bf53(ImVector_ImFontGlyph *self);
unsigned char ImVector_ImFontGlyph__contains__92902d08be(ImVector_ImFontGlyph *self, const ImFontGlyph * v);
unsigned char ImVector_ImFontGlyph__empty__7ecacb9bed(ImVector_ImFontGlyph *self);
const ImFontGlyph * ImVector_ImFontGlyph__end__ed2582194c(ImVector_ImFontGlyph *self);
ImFontGlyph * ImVector_ImFontGlyph__erase__046839634d(ImVector_ImFontGlyph *self, const ImFontGlyph * it);
ImFontGlyph * ImVector_ImFontGlyph__erase__3971aa986c(ImVector_ImFontGlyph *self, const ImFontGlyph * it, const ImFontGlyph * it_last);
ImFontGlyph * ImVector_ImFontGlyph__erase_unsorted__c6230f570d(ImVector_ImFontGlyph *self, const ImFontGlyph * it);
ImFontGlyph * ImVector_ImFontGlyph__find__50fcd878a5(ImVector_ImFontGlyph *self, const ImFontGlyph * v);
const ImFontGlyph * ImVector_ImFontGlyph__find__a8783142c0(ImVector_ImFontGlyph *self, const ImFontGlyph * v);
unsigned char ImVector_ImFontGlyph__find_erase__164417148a(ImVector_ImFontGlyph *self, const ImFontGlyph * v);
unsigned char ImVector_ImFontGlyph__find_erase_unsorted__80c6d749b0(ImVector_ImFontGlyph *self, const ImFontGlyph * v);
int ImVector_ImFontGlyph__find_index__8b78bb6733(ImVector_ImFontGlyph *self, const ImFontGlyph * v);
ImFontGlyph * ImVector_ImFontGlyph__front__59d3c5a074(ImVector_ImFontGlyph *self);
const ImFontGlyph * ImVector_ImFontGlyph__front__05abe79aa4(ImVector_ImFontGlyph *self);
int ImVector_ImFontGlyph__index_from_ptr__c833655680(ImVector_ImFontGlyph *self, const ImFontGlyph * it);
ImFontGlyph * ImVector_ImFontGlyph__insert__c401b1d5e2(ImVector_ImFontGlyph *self, const ImFontGlyph * it, const ImFontGlyph * v);
int ImVector_ImFontGlyph__max_size__349fc0d0b2(ImVector_ImFontGlyph *self);
ImVector_ImFontGlyph * ImVector_ImFontGlyph__operator___5c4dfdfe9c(ImVector_ImFontGlyph *self, const ImVector_ImFontGlyph * src);
const ImFontGlyph * ImVector_ImFontGlyph__operator____3638bb617e(ImVector_ImFontGlyph *self, int i);
void ImVector_ImFontGlyph__pop_back__63386d116e(ImVector_ImFontGlyph *self);
void ImVector_ImFontGlyph__push_front__c3dc0dc3b5(ImVector_ImFontGlyph *self, const ImFontGlyph * v);
void ImVector_ImFontGlyph__reserve_discard__bac6413be5(ImVector_ImFontGlyph *self, int new_capacity);
void ImVector_ImFontGlyph__resize__8019ca7724(ImVector_ImFontGlyph *self, int new_size);
void ImVector_ImFontGlyph__resize__be2f380ca5(ImVector_ImFontGlyph *self, int new_size, const ImFontGlyph * v);
void ImVector_ImFontGlyph__shrink__c312eda856(ImVector_ImFontGlyph *self, int new_size);
int ImVector_ImFontGlyph__size__283eb23a59(ImVector_ImFontGlyph *self);
int ImVector_ImFontGlyph__size_in_bytes__237114e16a(ImVector_ImFontGlyph *self);
void ImVector_ImFontGlyph__swap__2408d80d0c(ImVector_ImFontGlyph *self, ImVector_ImFontGlyph * rhs);
void ImVector_ImFontStackData__ImVector__8034c3a591(ImVector_ImFontStackData *self, const ImVector_ImFontStackData * src);
const ImFontStackData * ImVector_ImFontStackData__back__ddc49eb540(ImVector_ImFontStackData *self);
const ImFontStackData * ImVector_ImFontStackData__begin__ef392dcc23(ImVector_ImFontStackData *self);
int ImVector_ImFontStackData__capacity__9680bee922(ImVector_ImFontStackData *self);
void ImVector_ImFontStackData__clear_delete__b81c6e2f2a(ImVector_ImFontStackData *self);
void ImVector_ImFontStackData__clear_destruct__b733a92eb7(ImVector_ImFontStackData *self);
unsigned char ImVector_ImFontStackData__contains__7e1d7bf149(ImVector_ImFontStackData *self, const ImFontStackData * v);
unsigned char ImVector_ImFontStackData__empty__e13baf7653(ImVector_ImFontStackData *self);
const ImFontStackData * ImVector_ImFontStackData__end__aaa18aa99b(ImVector_ImFontStackData *self);
ImFontStackData * ImVector_ImFontStackData__erase__7cecfa38e9(ImVector_ImFontStackData *self, const ImFontStackData * it);
ImFontStackData * ImVector_ImFontStackData__erase__7f5b9c4a8c(ImVector_ImFontStackData *self, const ImFontStackData * it, const ImFontStackData * it_last);
ImFontStackData * ImVector_ImFontStackData__erase_unsorted__464bd60a0a(ImVector_ImFontStackData *self, const ImFontStackData * it);
ImFontStackData * ImVector_ImFontStackData__find__4f6338c21d(ImVector_ImFontStackData *self, const ImFontStackData * v);
const ImFontStackData * ImVector_ImFontStackData__find__68fc328791(ImVector_ImFontStackData *self, const ImFontStackData * v);
unsigned char ImVector_ImFontStackData__find_erase__7d6341ea02(ImVector_ImFontStackData *self, const ImFontStackData * v);
unsigned char ImVector_ImFontStackData__find_erase_unsorted__d0175a042d(ImVector_ImFontStackData *self, const ImFontStackData * v);
int ImVector_ImFontStackData__find_index__164cec9420(ImVector_ImFontStackData *self, const ImFontStackData * v);
ImFontStackData * ImVector_ImFontStackData__front__5a312ad547(ImVector_ImFontStackData *self);
const ImFontStackData * ImVector_ImFontStackData__front__9df9f25e61(ImVector_ImFontStackData *self);
int ImVector_ImFontStackData__index_from_ptr__74e70d9b71(ImVector_ImFontStackData *self, const ImFontStackData * it);
ImFontStackData * ImVector_ImFontStackData__insert__2a908e9c0a(ImVector_ImFontStackData *self, const ImFontStackData * it, const ImFontStackData * v);
int ImVector_ImFontStackData__max_size__4f420a8b34(ImVector_ImFontStackData *self);
ImVector_ImFontStackData * ImVector_ImFontStackData__operator___99c31d5c5a(ImVector_ImFontStackData *self, const ImVector_ImFontStackData * src);
ImFontStackData * ImVector_ImFontStackData__operator____3f8633995e(ImVector_ImFontStackData *self, int i);
const ImFontStackData * ImVector_ImFontStackData__operator____0aa9a08951(ImVector_ImFontStackData *self, int i);
void ImVector_ImFontStackData__push_front__67e90354e0(ImVector_ImFontStackData *self, const ImFontStackData * v);
void ImVector_ImFontStackData__reserve_discard__aa3cf6ef3b(ImVector_ImFontStackData *self, int new_capacity);
void ImVector_ImFontStackData__resize__15a2759f50(ImVector_ImFontStackData *self, int new_size);
void ImVector_ImFontStackData__resize__0f6d7fed40(ImVector_ImFontStackData *self, int new_size, const ImFontStackData * v);
void ImVector_ImFontStackData__shrink__72e275193e(ImVector_ImFontStackData *self, int new_size);
int ImVector_ImFontStackData__size__398868172d(ImVector_ImFontStackData *self);
int ImVector_ImFontStackData__size_in_bytes__3a4ed9ff5c(ImVector_ImFontStackData *self);
void ImVector_ImFontStackData__swap__71303064d5(ImVector_ImFontStackData *self, ImVector_ImFontStackData * rhs);
void ImVector_ImGuiColorMod__ImVector__92333614b9(ImVector_ImGuiColorMod *self, const ImVector_ImGuiColorMod * src);
const ImGuiColorMod * ImVector_ImGuiColorMod__back__5ccfd0eca6(ImVector_ImGuiColorMod *self);
ImGuiColorMod * ImVector_ImGuiColorMod__begin__85df11620e(ImVector_ImGuiColorMod *self);
const ImGuiColorMod * ImVector_ImGuiColorMod__begin__3f022400ed(ImVector_ImGuiColorMod *self);
int ImVector_ImGuiColorMod__capacity__1a4665827d(ImVector_ImGuiColorMod *self);
void ImVector_ImGuiColorMod__clear_delete__0ba202767c(ImVector_ImGuiColorMod *self);
void ImVector_ImGuiColorMod__clear_destruct__909d449abf(ImVector_ImGuiColorMod *self);
unsigned char ImVector_ImGuiColorMod__contains__9c86822430(ImVector_ImGuiColorMod *self, const ImGuiColorMod * v);
unsigned char ImVector_ImGuiColorMod__empty__872acd7a61(ImVector_ImGuiColorMod *self);
ImGuiColorMod * ImVector_ImGuiColorMod__end__8261fede6e(ImVector_ImGuiColorMod *self);
const ImGuiColorMod * ImVector_ImGuiColorMod__end__0e78a2d832(ImVector_ImGuiColorMod *self);
ImGuiColorMod * ImVector_ImGuiColorMod__erase__40f6fd5ea8(ImVector_ImGuiColorMod *self, const ImGuiColorMod * it);
ImGuiColorMod * ImVector_ImGuiColorMod__erase__fcf664fb55(ImVector_ImGuiColorMod *self, const ImGuiColorMod * it, const ImGuiColorMod * it_last);
ImGuiColorMod * ImVector_ImGuiColorMod__erase_unsorted__1ccfd56cde(ImVector_ImGuiColorMod *self, const ImGuiColorMod * it);
ImGuiColorMod * ImVector_ImGuiColorMod__find__59f201b892(ImVector_ImGuiColorMod *self, const ImGuiColorMod * v);
const ImGuiColorMod * ImVector_ImGuiColorMod__find__9487ff4b8c(ImVector_ImGuiColorMod *self, const ImGuiColorMod * v);
unsigned char ImVector_ImGuiColorMod__find_erase__453bc1d9bf(ImVector_ImGuiColorMod *self, const ImGuiColorMod * v);
unsigned char ImVector_ImGuiColorMod__find_erase_unsorted__b146542efd(ImVector_ImGuiColorMod *self, const ImGuiColorMod * v);
int ImVector_ImGuiColorMod__find_index__2474e5676e(ImVector_ImGuiColorMod *self, const ImGuiColorMod * v);
ImGuiColorMod * ImVector_ImGuiColorMod__front__8716d0cf84(ImVector_ImGuiColorMod *self);
const ImGuiColorMod * ImVector_ImGuiColorMod__front__41c4100c4b(ImVector_ImGuiColorMod *self);
int ImVector_ImGuiColorMod__index_from_ptr__f04735d5e5(ImVector_ImGuiColorMod *self, const ImGuiColorMod * it);
ImGuiColorMod * ImVector_ImGuiColorMod__insert__8c0d96804e(ImVector_ImGuiColorMod *self, const ImGuiColorMod * it, const ImGuiColorMod * v);
int ImVector_ImGuiColorMod__max_size__04246baba5(ImVector_ImGuiColorMod *self);
ImVector_ImGuiColorMod * ImVector_ImGuiColorMod__operator___18697d40c3(ImVector_ImGuiColorMod *self, const ImVector_ImGuiColorMod * src);
ImGuiColorMod * ImVector_ImGuiColorMod__operator____40defa015f(ImVector_ImGuiColorMod *self, int i);
const ImGuiColorMod * ImVector_ImGuiColorMod__operator____f40b790889(ImVector_ImGuiColorMod *self, int i);
void ImVector_ImGuiColorMod__push_front__73c4d60c1c(ImVector_ImGuiColorMod *self, const ImGuiColorMod * v);
void ImVector_ImGuiColorMod__reserve_discard__59e9dc64c5(ImVector_ImGuiColorMod *self, int new_capacity);
void ImVector_ImGuiColorMod__resize__f612027044(ImVector_ImGuiColorMod *self, int new_size);
void ImVector_ImGuiColorMod__resize__b076880a79(ImVector_ImGuiColorMod *self, int new_size, const ImGuiColorMod * v);
void ImVector_ImGuiColorMod__shrink__8908f1db44(ImVector_ImGuiColorMod *self, int new_size);
int ImVector_ImGuiColorMod__size__5b017e1300(ImVector_ImGuiColorMod *self);
int ImVector_ImGuiColorMod__size_in_bytes__9cc4f34b89(ImVector_ImGuiColorMod *self);
void ImVector_ImGuiColorMod__swap__cfe3b673e5(ImVector_ImGuiColorMod *self, ImVector_ImGuiColorMod * rhs);
void ImVector_ImGuiContextHook__ImVector__fe2ffa4d50(ImVector_ImGuiContextHook *self, const ImVector_ImGuiContextHook * src);
const ImGuiContextHook * ImVector_ImGuiContextHook__back__5b4e242cd4(ImVector_ImGuiContextHook *self);
const ImGuiContextHook * ImVector_ImGuiContextHook__begin__639716fb23(ImVector_ImGuiContextHook *self);
int ImVector_ImGuiContextHook__capacity__5863816971(ImVector_ImGuiContextHook *self);
void ImVector_ImGuiContextHook__clear__1579707a8b(ImVector_ImGuiContextHook *self);
void ImVector_ImGuiContextHook__clear_delete__524926b5ca(ImVector_ImGuiContextHook *self);
void ImVector_ImGuiContextHook__clear_destruct__c84b2956d4(ImVector_ImGuiContextHook *self);
unsigned char ImVector_ImGuiContextHook__contains__406648622c(ImVector_ImGuiContextHook *self, const ImGuiContextHook * v);
unsigned char ImVector_ImGuiContextHook__empty__fe5e019e7a(ImVector_ImGuiContextHook *self);
const ImGuiContextHook * ImVector_ImGuiContextHook__end__6747d6b9b6(ImVector_ImGuiContextHook *self);
ImGuiContextHook * ImVector_ImGuiContextHook__erase__522ba8ba76(ImVector_ImGuiContextHook *self, const ImGuiContextHook * it, const ImGuiContextHook * it_last);
ImGuiContextHook * ImVector_ImGuiContextHook__erase_unsorted__de7ddf63a0(ImVector_ImGuiContextHook *self, const ImGuiContextHook * it);
ImGuiContextHook * ImVector_ImGuiContextHook__find__0a22c6f8d6(ImVector_ImGuiContextHook *self, const ImGuiContextHook * v);
const ImGuiContextHook * ImVector_ImGuiContextHook__find__1fc813d1c9(ImVector_ImGuiContextHook *self, const ImGuiContextHook * v);
unsigned char ImVector_ImGuiContextHook__find_erase__a58bcac0d2(ImVector_ImGuiContextHook *self, const ImGuiContextHook * v);
unsigned char ImVector_ImGuiContextHook__find_erase_unsorted__31804617b3(ImVector_ImGuiContextHook *self, const ImGuiContextHook * v);
int ImVector_ImGuiContextHook__find_index__f9aa1f87b0(ImVector_ImGuiContextHook *self, const ImGuiContextHook * v);
ImGuiContextHook * ImVector_ImGuiContextHook__front__4cb8caa2e1(ImVector_ImGuiContextHook *self);
const ImGuiContextHook * ImVector_ImGuiContextHook__front__50a5c92bad(ImVector_ImGuiContextHook *self);
int ImVector_ImGuiContextHook__index_from_ptr__39b804c207(ImVector_ImGuiContextHook *self, const ImGuiContextHook * it);
ImGuiContextHook * ImVector_ImGuiContextHook__insert__3f6644a797(ImVector_ImGuiContextHook *self, const ImGuiContextHook * it, const ImGuiContextHook * v);
int ImVector_ImGuiContextHook__max_size__ea7d33fc27(ImVector_ImGuiContextHook *self);
ImVector_ImGuiContextHook * ImVector_ImGuiContextHook__operator___33809debe9(ImVector_ImGuiContextHook *self, const ImVector_ImGuiContextHook * src);
const ImGuiContextHook * ImVector_ImGuiContextHook__operator____6a5eba27ac(ImVector_ImGuiContextHook *self, int i);
void ImVector_ImGuiContextHook__pop_back__340393a5ac(ImVector_ImGuiContextHook *self);
void ImVector_ImGuiContextHook__push_front__f3d0d0538d(ImVector_ImGuiContextHook *self, const ImGuiContextHook * v);
void ImVector_ImGuiContextHook__reserve_discard__21c6362035(ImVector_ImGuiContextHook *self, int new_capacity);
void ImVector_ImGuiContextHook__resize__cf47a24506(ImVector_ImGuiContextHook *self, int new_size);
void ImVector_ImGuiContextHook__resize__7cbecb4d68(ImVector_ImGuiContextHook *self, int new_size, const ImGuiContextHook * v);
void ImVector_ImGuiContextHook__shrink__9545f7edf1(ImVector_ImGuiContextHook *self, int new_size);
int ImVector_ImGuiContextHook__size__01a9f36ee2(ImVector_ImGuiContextHook *self);
int ImVector_ImGuiContextHook__size_in_bytes__14156be49c(ImVector_ImGuiContextHook *self);
void ImVector_ImGuiContextHook__swap__fcdc95317d(ImVector_ImGuiContextHook *self, ImVector_ImGuiContextHook * rhs);
void ImVector_ImGuiFocusScopeData__ImVector__e5611acf9a(ImVector_ImGuiFocusScopeData *self, const ImVector_ImGuiFocusScopeData * src);
const ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__back__0e027ba322(ImVector_ImGuiFocusScopeData *self);
const ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__begin__f79b48e799(ImVector_ImGuiFocusScopeData *self);
int ImVector_ImGuiFocusScopeData__capacity__19a464482f(ImVector_ImGuiFocusScopeData *self);
void ImVector_ImGuiFocusScopeData__clear__f224b1ae17(ImVector_ImGuiFocusScopeData *self);
void ImVector_ImGuiFocusScopeData__clear_delete__8eefa25e24(ImVector_ImGuiFocusScopeData *self);
void ImVector_ImGuiFocusScopeData__clear_destruct__2f2df5dfa8(ImVector_ImGuiFocusScopeData *self);
unsigned char ImVector_ImGuiFocusScopeData__contains__9e14071d71(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * v);
unsigned char ImVector_ImGuiFocusScopeData__empty__480c37d346(ImVector_ImGuiFocusScopeData *self);
const ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__end__3667c34049(ImVector_ImGuiFocusScopeData *self);
ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__erase__7ea5d96921(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * it);
ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__erase__c56a803421(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * it, const ImGuiFocusScopeData * it_last);
ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__erase_unsorted__c484082791(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * it);
ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__find__bd23d83d99(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * v);
const ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__find__8c11ef7220(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * v);
unsigned char ImVector_ImGuiFocusScopeData__find_erase__66e5efdd79(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * v);
unsigned char ImVector_ImGuiFocusScopeData__find_erase_unsorted__ee7ff69138(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * v);
int ImVector_ImGuiFocusScopeData__find_index__7c6cd40869(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * v);
ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__front__73ee5d1c3b(ImVector_ImGuiFocusScopeData *self);
const ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__front__d360a3d9ca(ImVector_ImGuiFocusScopeData *self);
int ImVector_ImGuiFocusScopeData__index_from_ptr__7e86128bfc(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * it);
ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__insert__9ffbd3fe71(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * it, const ImGuiFocusScopeData * v);
int ImVector_ImGuiFocusScopeData__max_size__0588f8bbe6(ImVector_ImGuiFocusScopeData *self);
ImVector_ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__operator___f474f66d94(ImVector_ImGuiFocusScopeData *self, const ImVector_ImGuiFocusScopeData * src);
const ImGuiFocusScopeData * ImVector_ImGuiFocusScopeData__operator____24b3be45e3(ImVector_ImGuiFocusScopeData *self, int i);
void ImVector_ImGuiFocusScopeData__push_front__d57e010d5b(ImVector_ImGuiFocusScopeData *self, const ImGuiFocusScopeData * v);
void ImVector_ImGuiFocusScopeData__reserve_discard__7f28e875fa(ImVector_ImGuiFocusScopeData *self, int new_capacity);
void ImVector_ImGuiFocusScopeData__resize__37494f82ca(ImVector_ImGuiFocusScopeData *self, int new_size, const ImGuiFocusScopeData * v);
void ImVector_ImGuiFocusScopeData__shrink__31afbe8f20(ImVector_ImGuiFocusScopeData *self, int new_size);
int ImVector_ImGuiFocusScopeData__size__ac921b8f21(ImVector_ImGuiFocusScopeData *self);
int ImVector_ImGuiFocusScopeData__size_in_bytes__5acd7dcb42(ImVector_ImGuiFocusScopeData *self);
void ImVector_ImGuiFocusScopeData__swap__5b502f3d89(ImVector_ImGuiFocusScopeData *self, ImVector_ImGuiFocusScopeData * rhs);
void ImVector_ImGuiGroupData__ImVector__474e7b04c9(ImVector_ImGuiGroupData *self, const ImVector_ImGuiGroupData * src);
ImGuiGroupData * ImVector_ImGuiGroupData__back__465d5ab118(ImVector_ImGuiGroupData *self);
const ImGuiGroupData * ImVector_ImGuiGroupData__back__af40232e8d(ImVector_ImGuiGroupData *self);
ImGuiGroupData * ImVector_ImGuiGroupData__begin__9b39d4e8ee(ImVector_ImGuiGroupData *self);
const ImGuiGroupData * ImVector_ImGuiGroupData__begin__90acb36726(ImVector_ImGuiGroupData *self);
int ImVector_ImGuiGroupData__capacity__634ff7b67c(ImVector_ImGuiGroupData *self);
void ImVector_ImGuiGroupData__clear_delete__f866df2b50(ImVector_ImGuiGroupData *self);
void ImVector_ImGuiGroupData__clear_destruct__efb6e19023(ImVector_ImGuiGroupData *self);
unsigned char ImVector_ImGuiGroupData__contains__5ac62bd7a3(ImVector_ImGuiGroupData *self, const ImGuiGroupData * v);
unsigned char ImVector_ImGuiGroupData__empty__99625fc7e4(ImVector_ImGuiGroupData *self);
ImGuiGroupData * ImVector_ImGuiGroupData__end__64884fe3f6(ImVector_ImGuiGroupData *self);
const ImGuiGroupData * ImVector_ImGuiGroupData__end__629785bab0(ImVector_ImGuiGroupData *self);
ImGuiGroupData * ImVector_ImGuiGroupData__erase__9d42c0061d(ImVector_ImGuiGroupData *self, const ImGuiGroupData * it);
ImGuiGroupData * ImVector_ImGuiGroupData__erase__24031a5078(ImVector_ImGuiGroupData *self, const ImGuiGroupData * it, const ImGuiGroupData * it_last);
ImGuiGroupData * ImVector_ImGuiGroupData__erase_unsorted__cb84a49aeb(ImVector_ImGuiGroupData *self, const ImGuiGroupData * it);
ImGuiGroupData * ImVector_ImGuiGroupData__find__32644a56f5(ImVector_ImGuiGroupData *self, const ImGuiGroupData * v);
const ImGuiGroupData * ImVector_ImGuiGroupData__find__2e0b1e4dca(ImVector_ImGuiGroupData *self, const ImGuiGroupData * v);
unsigned char ImVector_ImGuiGroupData__find_erase__4e4113ce4b(ImVector_ImGuiGroupData *self, const ImGuiGroupData * v);
unsigned char ImVector_ImGuiGroupData__find_erase_unsorted__02a966dbeb(ImVector_ImGuiGroupData *self, const ImGuiGroupData * v);
int ImVector_ImGuiGroupData__find_index__df69b958e3(ImVector_ImGuiGroupData *self, const ImGuiGroupData * v);
ImGuiGroupData * ImVector_ImGuiGroupData__front__a2841e24e2(ImVector_ImGuiGroupData *self);
const ImGuiGroupData * ImVector_ImGuiGroupData__front__c8e0866bb5(ImVector_ImGuiGroupData *self);
int ImVector_ImGuiGroupData__index_from_ptr__eebafb10fb(ImVector_ImGuiGroupData *self, const ImGuiGroupData * it);
ImGuiGroupData * ImVector_ImGuiGroupData__insert__72e6288cbe(ImVector_ImGuiGroupData *self, const ImGuiGroupData * it, const ImGuiGroupData * v);
int ImVector_ImGuiGroupData__max_size__bc705bf161(ImVector_ImGuiGroupData *self);
ImVector_ImGuiGroupData * ImVector_ImGuiGroupData__operator___ae4f2cefa6(ImVector_ImGuiGroupData *self, const ImVector_ImGuiGroupData * src);
ImGuiGroupData * ImVector_ImGuiGroupData__operator____d6119f77f4(ImVector_ImGuiGroupData *self, int i);
const ImGuiGroupData * ImVector_ImGuiGroupData__operator____a3eac24dbb(ImVector_ImGuiGroupData *self, int i);
void ImVector_ImGuiGroupData__push_back__7bf59d1ee1(ImVector_ImGuiGroupData *self, const ImGuiGroupData * v);
void ImVector_ImGuiGroupData__push_front__efc25e2cb3(ImVector_ImGuiGroupData *self, const ImGuiGroupData * v);
void ImVector_ImGuiGroupData__reserve_discard__db30d0e110(ImVector_ImGuiGroupData *self, int new_capacity);
void ImVector_ImGuiGroupData__resize__91bc934cc2(ImVector_ImGuiGroupData *self, int new_size, const ImGuiGroupData * v);
void ImVector_ImGuiGroupData__shrink__513576a4a4(ImVector_ImGuiGroupData *self, int new_size);
int ImVector_ImGuiGroupData__size__118e9b45f1(ImVector_ImGuiGroupData *self);
int ImVector_ImGuiGroupData__size_in_bytes__9055d6b5b6(ImVector_ImGuiGroupData *self);
void ImVector_ImGuiGroupData__swap__0f96477b4c(ImVector_ImGuiGroupData *self, ImVector_ImGuiGroupData * rhs);
void ImVector_ImGuiInputEvent__ImVector__b3f50198e6(ImVector_ImGuiInputEvent *self, const ImVector_ImGuiInputEvent * src);
ImGuiInputEvent * ImVector_ImGuiInputEvent__back__45289f855e(ImVector_ImGuiInputEvent *self);
const ImGuiInputEvent * ImVector_ImGuiInputEvent__back__9f19115112(ImVector_ImGuiInputEvent *self);
ImGuiInputEvent * ImVector_ImGuiInputEvent__begin__7a66fe4034(ImVector_ImGuiInputEvent *self);
const ImGuiInputEvent * ImVector_ImGuiInputEvent__begin__90e8ac022f(ImVector_ImGuiInputEvent *self);
int ImVector_ImGuiInputEvent__capacity__85128f9c17(ImVector_ImGuiInputEvent *self);
void ImVector_ImGuiInputEvent__clear_delete__c38ef25b1f(ImVector_ImGuiInputEvent *self);
void ImVector_ImGuiInputEvent__clear_destruct__e00e8064e4(ImVector_ImGuiInputEvent *self);
unsigned char ImVector_ImGuiInputEvent__contains__cae9ca82ff(ImVector_ImGuiInputEvent *self, const ImGuiInputEvent * v);
unsigned char ImVector_ImGuiInputEvent__empty__a2785a20aa(ImVector_ImGuiInputEvent *self);
ImGuiInputEvent * ImVector_ImGuiInputEvent__end__8c04f9de37(ImVector_ImGuiInputEvent *self);
const ImGuiInputEvent * ImVector_ImGuiInputEvent__end__e0cbdd2230(ImVector_ImGuiInputEvent *self);
ImGuiInputEvent * ImVector_ImGuiInputEvent__erase__ba1a935605(ImVector_ImGuiInputEvent *self, const ImGuiInputEvent * it);
ImGuiInputEvent * ImVector_ImGuiInputEvent__erase_unsorted__398d9263ee(ImVector_ImGuiInputEvent *self, const ImGuiInputEvent * it);
ImGuiInputEvent * ImVector_ImGuiInputEvent__find__9a6f769122(ImVector_ImGuiInputEvent *self, const ImGuiInputEvent * v);
const ImGuiInputEvent * ImVector_ImGuiInputEvent__find__310c62b1fd(ImVector_ImGuiInputEvent *self, const ImGuiInputEvent * v);
unsigned char ImVector_ImGuiInputEvent__find_erase__8edb2a54fe(ImVector_ImGuiInputEvent *self, const ImGuiInputEvent * v);
unsigned char ImVector_ImGuiInputEvent__find_erase_unsorted__a1e12f37fe(ImVector_ImGuiInputEvent *self, const ImGuiInputEvent * v);
int ImVector_ImGuiInputEvent__find_index__90884aaee5(ImVector_ImGuiInputEvent *self, const ImGuiInputEvent * v);
ImGuiInputEvent * ImVector_ImGuiInputEvent__front__abaf6da22f(ImVector_ImGuiInputEvent *self);
const ImGuiInputEvent * ImVector_ImGuiInputEvent__front__b8b345543b(ImVector_ImGuiInputEvent *self);
int ImVector_ImGuiInputEvent__index_from_ptr__aa401dbab1(ImVector_ImGuiInputEvent *self, const ImGuiInputEvent * it);
ImGuiInputEvent * ImVector_ImGuiInputEvent__insert__f0264b5ec5(ImVector_ImGuiInputEvent *self, const ImGuiInputEvent * it, const ImGuiInputEvent * v);
int ImVector_ImGuiInputEvent__max_size__8b31a528c7(ImVector_ImGuiInputEvent *self);
ImVector_ImGuiInputEvent * ImVector_ImGuiInputEvent__operator___c9a9d5a8ab(ImVector_ImGuiInputEvent *self, const ImVector_ImGuiInputEvent * src);
const ImGuiInputEvent * ImVector_ImGuiInputEvent__operator____2a06019540(ImVector_ImGuiInputEvent *self, int i);
void ImVector_ImGuiInputEvent__pop_back__d6aedd9fc1(ImVector_ImGuiInputEvent *self);
void ImVector_ImGuiInputEvent__push_front__f635534759(ImVector_ImGuiInputEvent *self, const ImGuiInputEvent * v);
void ImVector_ImGuiInputEvent__reserve_discard__3dc946b397(ImVector_ImGuiInputEvent *self, int new_capacity);
void ImVector_ImGuiInputEvent__resize__90d9669c3a(ImVector_ImGuiInputEvent *self, int new_size, const ImGuiInputEvent * v);
void ImVector_ImGuiInputEvent__shrink__d156449189(ImVector_ImGuiInputEvent *self, int new_size);
int ImVector_ImGuiInputEvent__size__c21c3752b2(ImVector_ImGuiInputEvent *self);
int ImVector_ImGuiInputEvent__size_in_bytes__7d164b2fa5(ImVector_ImGuiInputEvent *self);
void ImVector_ImGuiInputEvent__swap__dacc7646c6(ImVector_ImGuiInputEvent *self, ImVector_ImGuiInputEvent * rhs);
void ImVector_ImGuiKeyRoutingData__ImVector__f3c99a43eb(ImVector_ImGuiKeyRoutingData *self, const ImVector_ImGuiKeyRoutingData * src);
ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__back__f2268e3cbe(ImVector_ImGuiKeyRoutingData *self);
const ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__back__67f48d2a87(ImVector_ImGuiKeyRoutingData *self);
ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__begin__355be3c8dc(ImVector_ImGuiKeyRoutingData *self);
const ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__begin__e54289d4ea(ImVector_ImGuiKeyRoutingData *self);
int ImVector_ImGuiKeyRoutingData__capacity__f83e3b0f25(ImVector_ImGuiKeyRoutingData *self);
void ImVector_ImGuiKeyRoutingData__clear_delete__cfb5022877(ImVector_ImGuiKeyRoutingData *self);
void ImVector_ImGuiKeyRoutingData__clear_destruct__b0f3622bfb(ImVector_ImGuiKeyRoutingData *self);
unsigned char ImVector_ImGuiKeyRoutingData__contains__132b1341a2(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * v);
unsigned char ImVector_ImGuiKeyRoutingData__empty__a02a27c158(ImVector_ImGuiKeyRoutingData *self);
ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__end__1d788581ca(ImVector_ImGuiKeyRoutingData *self);
const ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__end__329849b750(ImVector_ImGuiKeyRoutingData *self);
ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__erase__5912600ebc(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * it);
ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__erase__953ccdfe73(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * it, const ImGuiKeyRoutingData * it_last);
ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__erase_unsorted__ea4ffcb317(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * it);
ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__find__ea49a70180(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * v);
const ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__find__340f99b819(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * v);
unsigned char ImVector_ImGuiKeyRoutingData__find_erase__05d485aea4(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * v);
unsigned char ImVector_ImGuiKeyRoutingData__find_erase_unsorted__06ad9f1921(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * v);
int ImVector_ImGuiKeyRoutingData__find_index__f87f0148bd(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * v);
ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__front__03c62e6565(ImVector_ImGuiKeyRoutingData *self);
const ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__front__428c9259f3(ImVector_ImGuiKeyRoutingData *self);
int ImVector_ImGuiKeyRoutingData__index_from_ptr__bd5844f60c(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * it);
ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__insert__8d9e840d14(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * it, const ImGuiKeyRoutingData * v);
int ImVector_ImGuiKeyRoutingData__max_size__f9a2b5173a(ImVector_ImGuiKeyRoutingData *self);
ImVector_ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__operator___27667fbbd0(ImVector_ImGuiKeyRoutingData *self, const ImVector_ImGuiKeyRoutingData * src);
const ImGuiKeyRoutingData * ImVector_ImGuiKeyRoutingData__operator____a8c3bf6c69(ImVector_ImGuiKeyRoutingData *self, int i);
void ImVector_ImGuiKeyRoutingData__pop_back__4531ffa82b(ImVector_ImGuiKeyRoutingData *self);
void ImVector_ImGuiKeyRoutingData__push_front__4ebf294314(ImVector_ImGuiKeyRoutingData *self, const ImGuiKeyRoutingData * v);
void ImVector_ImGuiKeyRoutingData__reserve_discard__51b7003dd3(ImVector_ImGuiKeyRoutingData *self, int new_capacity);
void ImVector_ImGuiKeyRoutingData__resize__8650d28ade(ImVector_ImGuiKeyRoutingData *self, int new_size, const ImGuiKeyRoutingData * v);
void ImVector_ImGuiKeyRoutingData__shrink__50233afa99(ImVector_ImGuiKeyRoutingData *self, int new_size);
int ImVector_ImGuiKeyRoutingData__size__78e7bcf681(ImVector_ImGuiKeyRoutingData *self);
int ImVector_ImGuiKeyRoutingData__size_in_bytes__cfb5260d90(ImVector_ImGuiKeyRoutingData *self);
void ImVector_ImGuiListClipperData__ImVector__da2c4e2261(ImVector_ImGuiListClipperData *self, const ImVector_ImGuiListClipperData * src);
ImGuiListClipperData * ImVector_ImGuiListClipperData__back__1a5de14b52(ImVector_ImGuiListClipperData *self);
const ImGuiListClipperData * ImVector_ImGuiListClipperData__back__45118ead31(ImVector_ImGuiListClipperData *self);
ImGuiListClipperData * ImVector_ImGuiListClipperData__begin__cb4c0dc98a(ImVector_ImGuiListClipperData *self);
const ImGuiListClipperData * ImVector_ImGuiListClipperData__begin__cf3eb51542(ImVector_ImGuiListClipperData *self);
int ImVector_ImGuiListClipperData__capacity__3f26f4981b(ImVector_ImGuiListClipperData *self);
void ImVector_ImGuiListClipperData__clear_delete__c67200a8a2(ImVector_ImGuiListClipperData *self);
unsigned char ImVector_ImGuiListClipperData__contains__78bb3018a0(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * v);
unsigned char ImVector_ImGuiListClipperData__empty__5c931c019a(ImVector_ImGuiListClipperData *self);
ImGuiListClipperData * ImVector_ImGuiListClipperData__end__1eb47061cf(ImVector_ImGuiListClipperData *self);
const ImGuiListClipperData * ImVector_ImGuiListClipperData__end__cdda12772a(ImVector_ImGuiListClipperData *self);
ImGuiListClipperData * ImVector_ImGuiListClipperData__erase__702e2dc6f9(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * it);
ImGuiListClipperData * ImVector_ImGuiListClipperData__erase__c342c40098(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * it, const ImGuiListClipperData * it_last);
ImGuiListClipperData * ImVector_ImGuiListClipperData__erase_unsorted__7d42883559(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * it);
ImGuiListClipperData * ImVector_ImGuiListClipperData__find__df6502a6ed(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * v);
const ImGuiListClipperData * ImVector_ImGuiListClipperData__find__af2c034a38(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * v);
unsigned char ImVector_ImGuiListClipperData__find_erase__a5376c590f(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * v);
unsigned char ImVector_ImGuiListClipperData__find_erase_unsorted__b023e6bf7f(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * v);
int ImVector_ImGuiListClipperData__find_index__d5a553ab26(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * v);
ImGuiListClipperData * ImVector_ImGuiListClipperData__front__6becd9abda(ImVector_ImGuiListClipperData *self);
const ImGuiListClipperData * ImVector_ImGuiListClipperData__front__3ded1d414c(ImVector_ImGuiListClipperData *self);
int ImVector_ImGuiListClipperData__index_from_ptr__f6e117e552(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * it);
ImGuiListClipperData * ImVector_ImGuiListClipperData__insert__88f8b10e62(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * it, const ImGuiListClipperData * v);
int ImVector_ImGuiListClipperData__max_size__e2784a6018(ImVector_ImGuiListClipperData *self);
ImVector_ImGuiListClipperData * ImVector_ImGuiListClipperData__operator___4ae107edca(ImVector_ImGuiListClipperData *self, const ImVector_ImGuiListClipperData * src);
const ImGuiListClipperData * ImVector_ImGuiListClipperData__operator____105301264f(ImVector_ImGuiListClipperData *self, int i);
void ImVector_ImGuiListClipperData__pop_back__b65a4c089d(ImVector_ImGuiListClipperData *self);
void ImVector_ImGuiListClipperData__push_back__24cdeb683a(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * v);
void ImVector_ImGuiListClipperData__push_front__0230858911(ImVector_ImGuiListClipperData *self, const ImGuiListClipperData * v);
void ImVector_ImGuiListClipperData__reserve_discard__aad6ca23f8(ImVector_ImGuiListClipperData *self, int new_capacity);
void ImVector_ImGuiListClipperData__resize__954d27432f(ImVector_ImGuiListClipperData *self, int new_size);
void ImVector_ImGuiListClipperData__shrink__779f5a283a(ImVector_ImGuiListClipperData *self, int new_size);
int ImVector_ImGuiListClipperData__size__353f0075ca(ImVector_ImGuiListClipperData *self);
int ImVector_ImGuiListClipperData__size_in_bytes__92030a407b(ImVector_ImGuiListClipperData *self);
void ImVector_ImGuiListClipperData__swap__94d0834ced(ImVector_ImGuiListClipperData *self, ImVector_ImGuiListClipperData * rhs);
void ImVector_ImGuiListClipperRange__ImVector__af6d88fffa(ImVector_ImGuiListClipperRange *self, const ImVector_ImGuiListClipperRange * src);
ImGuiListClipperRange * ImVector_ImGuiListClipperRange__back__bb4a3d58c2(ImVector_ImGuiListClipperRange *self);
const ImGuiListClipperRange * ImVector_ImGuiListClipperRange__back__ad694657d5(ImVector_ImGuiListClipperRange *self);
const ImGuiListClipperRange * ImVector_ImGuiListClipperRange__begin__d36d409ce2(ImVector_ImGuiListClipperRange *self);
int ImVector_ImGuiListClipperRange__capacity__1284824cb1(ImVector_ImGuiListClipperRange *self);
void ImVector_ImGuiListClipperRange__clear__676b33488c(ImVector_ImGuiListClipperRange *self);
void ImVector_ImGuiListClipperRange__clear_delete__516fc4efec(ImVector_ImGuiListClipperRange *self);
void ImVector_ImGuiListClipperRange__clear_destruct__6697632e18(ImVector_ImGuiListClipperRange *self);
unsigned char ImVector_ImGuiListClipperRange__contains__749665641a(ImVector_ImGuiListClipperRange *self, const ImGuiListClipperRange * v);
unsigned char ImVector_ImGuiListClipperRange__empty__96b457ae54(ImVector_ImGuiListClipperRange *self);
const ImGuiListClipperRange * ImVector_ImGuiListClipperRange__end__39799ae0fd(ImVector_ImGuiListClipperRange *self);
ImGuiListClipperRange * ImVector_ImGuiListClipperRange__erase__a928582df5(ImVector_ImGuiListClipperRange *self, const ImGuiListClipperRange * it, const ImGuiListClipperRange * it_last);
ImGuiListClipperRange * ImVector_ImGuiListClipperRange__erase_unsorted__2c411dc67c(ImVector_ImGuiListClipperRange *self, const ImGuiListClipperRange * it);
ImGuiListClipperRange * ImVector_ImGuiListClipperRange__find__7e14769bd3(ImVector_ImGuiListClipperRange *self, const ImGuiListClipperRange * v);
const ImGuiListClipperRange * ImVector_ImGuiListClipperRange__find__9717533615(ImVector_ImGuiListClipperRange *self, const ImGuiListClipperRange * v);
unsigned char ImVector_ImGuiListClipperRange__find_erase__104aaa8690(ImVector_ImGuiListClipperRange *self, const ImGuiListClipperRange * v);
unsigned char ImVector_ImGuiListClipperRange__find_erase_unsorted__933b41d66c(ImVector_ImGuiListClipperRange *self, const ImGuiListClipperRange * v);
int ImVector_ImGuiListClipperRange__find_index__c376ae6f0f(ImVector_ImGuiListClipperRange *self, const ImGuiListClipperRange * v);
ImGuiListClipperRange * ImVector_ImGuiListClipperRange__front__b466ebd9ba(ImVector_ImGuiListClipperRange *self);
const ImGuiListClipperRange * ImVector_ImGuiListClipperRange__front__ffb7494923(ImVector_ImGuiListClipperRange *self);
int ImVector_ImGuiListClipperRange__index_from_ptr__9dd7c8346f(ImVector_ImGuiListClipperRange *self, const ImGuiListClipperRange * it);
int ImVector_ImGuiListClipperRange__max_size__59d9f9b089(ImVector_ImGuiListClipperRange *self);
ImVector_ImGuiListClipperRange * ImVector_ImGuiListClipperRange__operator___a20b182c5d(ImVector_ImGuiListClipperRange *self, const ImVector_ImGuiListClipperRange * src);
const ImGuiListClipperRange * ImVector_ImGuiListClipperRange__operator____02a729b32b(ImVector_ImGuiListClipperRange *self, int i);
void ImVector_ImGuiListClipperRange__pop_back__4cde3c7f29(ImVector_ImGuiListClipperRange *self);
void ImVector_ImGuiListClipperRange__reserve_discard__b235d72131(ImVector_ImGuiListClipperRange *self, int new_capacity);
void ImVector_ImGuiListClipperRange__resize__45e10ea69d(ImVector_ImGuiListClipperRange *self, int new_size, const ImGuiListClipperRange * v);
void ImVector_ImGuiListClipperRange__shrink__d4c30a68d9(ImVector_ImGuiListClipperRange *self, int new_size);
int ImVector_ImGuiListClipperRange__size__ee057d0fb3(ImVector_ImGuiListClipperRange *self);
int ImVector_ImGuiListClipperRange__size_in_bytes__826bea0ab7(ImVector_ImGuiListClipperRange *self);
void ImVector_ImGuiListClipperRange__swap__9181fa119b(ImVector_ImGuiListClipperRange *self, ImVector_ImGuiListClipperRange * rhs);
void ImVector_ImGuiMultiSelectState__ImVector__79493e50c9(ImVector_ImGuiMultiSelectState *self, const ImVector_ImGuiMultiSelectState * src);
ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__back__09383676f0(ImVector_ImGuiMultiSelectState *self);
const ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__back__1cf308d450(ImVector_ImGuiMultiSelectState *self);
ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__begin__57cbda4455(ImVector_ImGuiMultiSelectState *self);
const ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__begin__7fda80fac6(ImVector_ImGuiMultiSelectState *self);
int ImVector_ImGuiMultiSelectState__capacity__077465c950(ImVector_ImGuiMultiSelectState *self);
void ImVector_ImGuiMultiSelectState__clear_delete__c206d94426(ImVector_ImGuiMultiSelectState *self);
void ImVector_ImGuiMultiSelectState__clear_destruct__98277c8203(ImVector_ImGuiMultiSelectState *self);
unsigned char ImVector_ImGuiMultiSelectState__contains__45aff4e086(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * v);
unsigned char ImVector_ImGuiMultiSelectState__empty__106a5bf90b(ImVector_ImGuiMultiSelectState *self);
ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__end__c5eabc5bb9(ImVector_ImGuiMultiSelectState *self);
const ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__end__d8cf0d3233(ImVector_ImGuiMultiSelectState *self);
ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__erase__92832dd32f(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * it);
ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__erase__1152d58d80(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * it, const ImGuiMultiSelectState * it_last);
ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__erase_unsorted__675aadaf11(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * it);
ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__find__a838aceaa3(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * v);
const ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__find__b3649d412c(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * v);
unsigned char ImVector_ImGuiMultiSelectState__find_erase__f142760f26(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * v);
unsigned char ImVector_ImGuiMultiSelectState__find_erase_unsorted__1816a68f07(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * v);
int ImVector_ImGuiMultiSelectState__find_index__c922c9e5d0(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * v);
ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__front__400e1caf44(ImVector_ImGuiMultiSelectState *self);
const ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__front__e7fafa6e1e(ImVector_ImGuiMultiSelectState *self);
int ImVector_ImGuiMultiSelectState__index_from_ptr__715a960674(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * it);
ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__insert__218b6c70aa(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * it, const ImGuiMultiSelectState * v);
int ImVector_ImGuiMultiSelectState__max_size__5eae170a14(ImVector_ImGuiMultiSelectState *self);
ImVector_ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__operator___7543c0983b(ImVector_ImGuiMultiSelectState *self, const ImVector_ImGuiMultiSelectState * src);
ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__operator____244968aff0(ImVector_ImGuiMultiSelectState *self, int i);
const ImGuiMultiSelectState * ImVector_ImGuiMultiSelectState__operator____b585544eac(ImVector_ImGuiMultiSelectState *self, int i);
void ImVector_ImGuiMultiSelectState__pop_back__d2bf799b8b(ImVector_ImGuiMultiSelectState *self);
void ImVector_ImGuiMultiSelectState__push_back__f911985b14(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * v);
void ImVector_ImGuiMultiSelectState__push_front__2fef719668(ImVector_ImGuiMultiSelectState *self, const ImGuiMultiSelectState * v);
void ImVector_ImGuiMultiSelectState__reserve_discard__e32af07a37(ImVector_ImGuiMultiSelectState *self, int new_capacity);
void ImVector_ImGuiMultiSelectState__resize__86f73b97c8(ImVector_ImGuiMultiSelectState *self, int new_size, const ImGuiMultiSelectState * v);
void ImVector_ImGuiMultiSelectState__shrink__2d20bd215e(ImVector_ImGuiMultiSelectState *self, int new_size);
int ImVector_ImGuiMultiSelectState__size__7e0a5b7b9c(ImVector_ImGuiMultiSelectState *self);
int ImVector_ImGuiMultiSelectState__size_in_bytes__8a22c60687(ImVector_ImGuiMultiSelectState *self);
void ImVector_ImGuiMultiSelectState__swap__77e10e8119(ImVector_ImGuiMultiSelectState *self, ImVector_ImGuiMultiSelectState * rhs);
void ImVector_ImGuiMultiSelectTempData__ImVector__1ad27df1b4(ImVector_ImGuiMultiSelectTempData *self, const ImVector_ImGuiMultiSelectTempData * src);
ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__back__9c3afe2b54(ImVector_ImGuiMultiSelectTempData *self);
const ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__back__191837198e(ImVector_ImGuiMultiSelectTempData *self);
ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__begin__c79a4cd747(ImVector_ImGuiMultiSelectTempData *self);
const ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__begin__a3476ffe18(ImVector_ImGuiMultiSelectTempData *self);
int ImVector_ImGuiMultiSelectTempData__capacity__26d819e63a(ImVector_ImGuiMultiSelectTempData *self);
void ImVector_ImGuiMultiSelectTempData__clear_delete__db974fee40(ImVector_ImGuiMultiSelectTempData *self);
unsigned char ImVector_ImGuiMultiSelectTempData__contains__cb62fab4f6(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * v);
unsigned char ImVector_ImGuiMultiSelectTempData__empty__c2c500b4bf(ImVector_ImGuiMultiSelectTempData *self);
ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__end__99b6cf5ede(ImVector_ImGuiMultiSelectTempData *self);
const ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__end__d2342890b1(ImVector_ImGuiMultiSelectTempData *self);
ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__erase__1957235fc5(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * it);
ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__erase__e6aef9623b(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * it, const ImGuiMultiSelectTempData * it_last);
ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__erase_unsorted__20b297c6ed(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * it);
ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__find__16eb687694(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * v);
const ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__find__4acf436539(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * v);
unsigned char ImVector_ImGuiMultiSelectTempData__find_erase__1cf7d56055(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * v);
unsigned char ImVector_ImGuiMultiSelectTempData__find_erase_unsorted__452392e995(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * v);
int ImVector_ImGuiMultiSelectTempData__find_index__d82e3083ca(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * v);
ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__front__d471a37bfd(ImVector_ImGuiMultiSelectTempData *self);
const ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__front__99b2b117a7(ImVector_ImGuiMultiSelectTempData *self);
int ImVector_ImGuiMultiSelectTempData__index_from_ptr__dfc486b984(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * it);
ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__insert__7e53b70874(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * it, const ImGuiMultiSelectTempData * v);
int ImVector_ImGuiMultiSelectTempData__max_size__60678c231a(ImVector_ImGuiMultiSelectTempData *self);
ImVector_ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__operator___2847253a34(ImVector_ImGuiMultiSelectTempData *self, const ImVector_ImGuiMultiSelectTempData * src);
const ImGuiMultiSelectTempData * ImVector_ImGuiMultiSelectTempData__operator____9b01fe7282(ImVector_ImGuiMultiSelectTempData *self, int i);
void ImVector_ImGuiMultiSelectTempData__pop_back__2052e13b30(ImVector_ImGuiMultiSelectTempData *self);
void ImVector_ImGuiMultiSelectTempData__push_back__83d6b2f84c(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * v);
void ImVector_ImGuiMultiSelectTempData__push_front__150cd8dd21(ImVector_ImGuiMultiSelectTempData *self, const ImGuiMultiSelectTempData * v);
void ImVector_ImGuiMultiSelectTempData__reserve_discard__9c4b607699(ImVector_ImGuiMultiSelectTempData *self, int new_capacity);
void ImVector_ImGuiMultiSelectTempData__resize__9844c3c7ba(ImVector_ImGuiMultiSelectTempData *self, int new_size);
void ImVector_ImGuiMultiSelectTempData__shrink__aa33482148(ImVector_ImGuiMultiSelectTempData *self, int new_size);
int ImVector_ImGuiMultiSelectTempData__size__2ad91c4c6d(ImVector_ImGuiMultiSelectTempData *self);
int ImVector_ImGuiMultiSelectTempData__size_in_bytes__4e2ce4c0fa(ImVector_ImGuiMultiSelectTempData *self);
void ImVector_ImGuiMultiSelectTempData__swap__32b75c828b(ImVector_ImGuiMultiSelectTempData *self, ImVector_ImGuiMultiSelectTempData * rhs);
void ImVector_ImGuiOldColumnData__ImVector__a9a8235b54(ImVector_ImGuiOldColumnData *self, const ImVector_ImGuiOldColumnData * src);
ImGuiOldColumnData * ImVector_ImGuiOldColumnData__back__1e0eadafb0(ImVector_ImGuiOldColumnData *self);
const ImGuiOldColumnData * ImVector_ImGuiOldColumnData__back__142caa7f4f(ImVector_ImGuiOldColumnData *self);
const ImGuiOldColumnData * ImVector_ImGuiOldColumnData__begin__978b952397(ImVector_ImGuiOldColumnData *self);
int ImVector_ImGuiOldColumnData__capacity__c0fdb13666(ImVector_ImGuiOldColumnData *self);
void ImVector_ImGuiOldColumnData__clear__ea6f079e4d(ImVector_ImGuiOldColumnData *self);
void ImVector_ImGuiOldColumnData__clear_delete__9c38e1255e(ImVector_ImGuiOldColumnData *self);
void ImVector_ImGuiOldColumnData__clear_destruct__eee1933bbe(ImVector_ImGuiOldColumnData *self);
unsigned char ImVector_ImGuiOldColumnData__contains__87ec18f887(ImVector_ImGuiOldColumnData *self, const ImGuiOldColumnData * v);
unsigned char ImVector_ImGuiOldColumnData__empty__b92233cd89(ImVector_ImGuiOldColumnData *self);
const ImGuiOldColumnData * ImVector_ImGuiOldColumnData__end__9df296cd63(ImVector_ImGuiOldColumnData *self);
ImGuiOldColumnData * ImVector_ImGuiOldColumnData__erase__faf5d4ddbf(ImVector_ImGuiOldColumnData *self, const ImGuiOldColumnData * it);
ImGuiOldColumnData * ImVector_ImGuiOldColumnData__erase__45505dd20a(ImVector_ImGuiOldColumnData *self, const ImGuiOldColumnData * it, const ImGuiOldColumnData * it_last);
ImGuiOldColumnData * ImVector_ImGuiOldColumnData__erase_unsorted__aac2651fb4(ImVector_ImGuiOldColumnData *self, const ImGuiOldColumnData * it);
ImGuiOldColumnData * ImVector_ImGuiOldColumnData__find__409031f93c(ImVector_ImGuiOldColumnData *self, const ImGuiOldColumnData * v);
const ImGuiOldColumnData * ImVector_ImGuiOldColumnData__find__2556814bb9(ImVector_ImGuiOldColumnData *self, const ImGuiOldColumnData * v);
unsigned char ImVector_ImGuiOldColumnData__find_erase__deff2dbf20(ImVector_ImGuiOldColumnData *self, const ImGuiOldColumnData * v);
unsigned char ImVector_ImGuiOldColumnData__find_erase_unsorted__236b64b3d3(ImVector_ImGuiOldColumnData *self, const ImGuiOldColumnData * v);
int ImVector_ImGuiOldColumnData__find_index__1da71e76f0(ImVector_ImGuiOldColumnData *self, const ImGuiOldColumnData * v);
ImGuiOldColumnData * ImVector_ImGuiOldColumnData__front__77c1c0a7a4(ImVector_ImGuiOldColumnData *self);
const ImGuiOldColumnData * ImVector_ImGuiOldColumnData__front__64ccfb4e9c(ImVector_ImGuiOldColumnData *self);
ImGuiOldColumnData * ImVector_ImGuiOldColumnData__insert__d876236984(ImVector_ImGuiOldColumnData *self, const ImGuiOldColumnData * it, const ImGuiOldColumnData * v);
int ImVector_ImGuiOldColumnData__max_size__324d6602f8(ImVector_ImGuiOldColumnData *self);
ImVector_ImGuiOldColumnData * ImVector_ImGuiOldColumnData__operator___a30f049dd1(ImVector_ImGuiOldColumnData *self, const ImVector_ImGuiOldColumnData * src);
const ImGuiOldColumnData * ImVector_ImGuiOldColumnData__operator____70beb62045(ImVector_ImGuiOldColumnData *self, int i);
void ImVector_ImGuiOldColumnData__pop_back__d172154096(ImVector_ImGuiOldColumnData *self);
void ImVector_ImGuiOldColumnData__push_front__67809d4c79(ImVector_ImGuiOldColumnData *self, const ImGuiOldColumnData * v);
void ImVector_ImGuiOldColumnData__reserve_discard__b6cb18d2cf(ImVector_ImGuiOldColumnData *self, int new_capacity);
void ImVector_ImGuiOldColumnData__resize__8553568938(ImVector_ImGuiOldColumnData *self, int new_size, const ImGuiOldColumnData * v);
void ImVector_ImGuiOldColumnData__shrink__723a33b017(ImVector_ImGuiOldColumnData *self, int new_size);
int ImVector_ImGuiOldColumnData__size__27b08c6a10(ImVector_ImGuiOldColumnData *self);
int ImVector_ImGuiOldColumnData__size_in_bytes__d4eee2d838(ImVector_ImGuiOldColumnData *self);
void ImVector_ImGuiOldColumnData__swap__5849a86e0f(ImVector_ImGuiOldColumnData *self, ImVector_ImGuiOldColumnData * rhs);
void ImVector_ImGuiOldColumns__ImVector__43af9a794e(ImVector_ImGuiOldColumns *self, const ImVector_ImGuiOldColumns * src);
const ImGuiOldColumns * ImVector_ImGuiOldColumns__back__9f03868681(ImVector_ImGuiOldColumns *self);
const ImGuiOldColumns * ImVector_ImGuiOldColumns__begin__12ab993050(ImVector_ImGuiOldColumns *self);
int ImVector_ImGuiOldColumns__capacity__286c76400b(ImVector_ImGuiOldColumns *self);
void ImVector_ImGuiOldColumns__clear_delete__194bff5219(ImVector_ImGuiOldColumns *self);
unsigned char ImVector_ImGuiOldColumns__contains__c36c167de4(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * v);
unsigned char ImVector_ImGuiOldColumns__empty__8902d5e8b1(ImVector_ImGuiOldColumns *self);
const ImGuiOldColumns * ImVector_ImGuiOldColumns__end__3adf25687e(ImVector_ImGuiOldColumns *self);
ImGuiOldColumns * ImVector_ImGuiOldColumns__erase__e10b483f89(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * it);
ImGuiOldColumns * ImVector_ImGuiOldColumns__erase__97b3158c99(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * it, const ImGuiOldColumns * it_last);
ImGuiOldColumns * ImVector_ImGuiOldColumns__erase_unsorted__faf309ebbd(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * it);
ImGuiOldColumns * ImVector_ImGuiOldColumns__find__ec392c70c0(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * v);
const ImGuiOldColumns * ImVector_ImGuiOldColumns__find__f609875a76(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * v);
unsigned char ImVector_ImGuiOldColumns__find_erase__25e4749ee5(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * v);
unsigned char ImVector_ImGuiOldColumns__find_erase_unsorted__d60ed6cd91(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * v);
int ImVector_ImGuiOldColumns__find_index__0c69091234(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * v);
ImGuiOldColumns * ImVector_ImGuiOldColumns__front__ae1ec91c1d(ImVector_ImGuiOldColumns *self);
const ImGuiOldColumns * ImVector_ImGuiOldColumns__front__54036637d6(ImVector_ImGuiOldColumns *self);
int ImVector_ImGuiOldColumns__index_from_ptr__7ea37ec7d9(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * it);
ImGuiOldColumns * ImVector_ImGuiOldColumns__insert__b7d909f992(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * it, const ImGuiOldColumns * v);
int ImVector_ImGuiOldColumns__max_size__642e4d8bc3(ImVector_ImGuiOldColumns *self);
ImVector_ImGuiOldColumns * ImVector_ImGuiOldColumns__operator___13e6dbd151(ImVector_ImGuiOldColumns *self, const ImVector_ImGuiOldColumns * src);
const ImGuiOldColumns * ImVector_ImGuiOldColumns__operator____2f70f3e039(ImVector_ImGuiOldColumns *self, int i);
void ImVector_ImGuiOldColumns__pop_back__abe4c02001(ImVector_ImGuiOldColumns *self);
void ImVector_ImGuiOldColumns__push_front__ef3e981594(ImVector_ImGuiOldColumns *self, const ImGuiOldColumns * v);
void ImVector_ImGuiOldColumns__reserve_discard__058a52b8ed(ImVector_ImGuiOldColumns *self, int new_capacity);
void ImVector_ImGuiOldColumns__resize__236bfc9559(ImVector_ImGuiOldColumns *self, int new_size);
void ImVector_ImGuiOldColumns__resize__09ecb99408(ImVector_ImGuiOldColumns *self, int new_size, const ImGuiOldColumns * v);
void ImVector_ImGuiOldColumns__shrink__5cefcb4cb5(ImVector_ImGuiOldColumns *self, int new_size);
int ImVector_ImGuiOldColumns__size__c4f552ebc3(ImVector_ImGuiOldColumns *self);
int ImVector_ImGuiOldColumns__size_in_bytes__d07c3a5f54(ImVector_ImGuiOldColumns *self);
void ImVector_ImGuiOldColumns__swap__e95b11702d(ImVector_ImGuiOldColumns *self, ImVector_ImGuiOldColumns * rhs);
void ImVector_ImGuiPopupData__ImVector__a151081516(ImVector_ImGuiPopupData *self, const ImVector_ImGuiPopupData * src);
const ImGuiPopupData * ImVector_ImGuiPopupData__back__e009f03708(ImVector_ImGuiPopupData *self);
const ImGuiPopupData * ImVector_ImGuiPopupData__begin__098f850226(ImVector_ImGuiPopupData *self);
int ImVector_ImGuiPopupData__capacity__2fba69b21d(ImVector_ImGuiPopupData *self);
void ImVector_ImGuiPopupData__clear_delete__3d2c510e8c(ImVector_ImGuiPopupData *self);
void ImVector_ImGuiPopupData__clear_destruct__661613e618(ImVector_ImGuiPopupData *self);
unsigned char ImVector_ImGuiPopupData__contains__6605181c5b(ImVector_ImGuiPopupData *self, const ImGuiPopupData * v);
unsigned char ImVector_ImGuiPopupData__empty__b2bebd7453(ImVector_ImGuiPopupData *self);
const ImGuiPopupData * ImVector_ImGuiPopupData__end__b5eaab7807(ImVector_ImGuiPopupData *self);
ImGuiPopupData * ImVector_ImGuiPopupData__erase__be99d49bb7(ImVector_ImGuiPopupData *self, const ImGuiPopupData * it);
ImGuiPopupData * ImVector_ImGuiPopupData__erase__f5ed5bf229(ImVector_ImGuiPopupData *self, const ImGuiPopupData * it, const ImGuiPopupData * it_last);
ImGuiPopupData * ImVector_ImGuiPopupData__erase_unsorted__757641c22d(ImVector_ImGuiPopupData *self, const ImGuiPopupData * it);
ImGuiPopupData * ImVector_ImGuiPopupData__find__ce7ce88628(ImVector_ImGuiPopupData *self, const ImGuiPopupData * v);
const ImGuiPopupData * ImVector_ImGuiPopupData__find__ed3e90ab4d(ImVector_ImGuiPopupData *self, const ImGuiPopupData * v);
unsigned char ImVector_ImGuiPopupData__find_erase__6270eb6f68(ImVector_ImGuiPopupData *self, const ImGuiPopupData * v);
unsigned char ImVector_ImGuiPopupData__find_erase_unsorted__e5d0ffd612(ImVector_ImGuiPopupData *self, const ImGuiPopupData * v);
int ImVector_ImGuiPopupData__find_index__5f5051e02a(ImVector_ImGuiPopupData *self, const ImGuiPopupData * v);
ImGuiPopupData * ImVector_ImGuiPopupData__front__30350029ff(ImVector_ImGuiPopupData *self);
const ImGuiPopupData * ImVector_ImGuiPopupData__front__e8f63826d9(ImVector_ImGuiPopupData *self);
int ImVector_ImGuiPopupData__index_from_ptr__fc37b15549(ImVector_ImGuiPopupData *self, const ImGuiPopupData * it);
ImGuiPopupData * ImVector_ImGuiPopupData__insert__5761c5a110(ImVector_ImGuiPopupData *self, const ImGuiPopupData * it, const ImGuiPopupData * v);
int ImVector_ImGuiPopupData__max_size__8a13d43b9a(ImVector_ImGuiPopupData *self);
ImVector_ImGuiPopupData * ImVector_ImGuiPopupData__operator___6ecfcd9a45(ImVector_ImGuiPopupData *self, const ImVector_ImGuiPopupData * src);
ImGuiPopupData * ImVector_ImGuiPopupData__operator____ffd965e503(ImVector_ImGuiPopupData *self, int i);
const ImGuiPopupData * ImVector_ImGuiPopupData__operator____dc0407489e(ImVector_ImGuiPopupData *self, int i);
void ImVector_ImGuiPopupData__push_front__4f91b76182(ImVector_ImGuiPopupData *self, const ImGuiPopupData * v);
void ImVector_ImGuiPopupData__reserve_discard__68b28b134e(ImVector_ImGuiPopupData *self, int new_capacity);
void ImVector_ImGuiPopupData__resize__56bb3074e2(ImVector_ImGuiPopupData *self, int new_size, const ImGuiPopupData * v);
void ImVector_ImGuiPopupData__shrink__1412a53bac(ImVector_ImGuiPopupData *self, int new_size);
int ImVector_ImGuiPopupData__size__be37184fe1(ImVector_ImGuiPopupData *self);
int ImVector_ImGuiPopupData__size_in_bytes__d7d0a4ff9e(ImVector_ImGuiPopupData *self);
void ImVector_ImGuiPopupData__swap__e321d8c2a1(ImVector_ImGuiPopupData *self, ImVector_ImGuiPopupData * rhs);
void ImVector_ImGuiPtrOrIndex__ImVector__2c2a1bedb4(ImVector_ImGuiPtrOrIndex *self, const ImVector_ImGuiPtrOrIndex * src);
const ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__back__3de1e23ad4(ImVector_ImGuiPtrOrIndex *self);
ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__begin__bb9e896611(ImVector_ImGuiPtrOrIndex *self);
const ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__begin__2d4ac7e913(ImVector_ImGuiPtrOrIndex *self);
int ImVector_ImGuiPtrOrIndex__capacity__07c490329b(ImVector_ImGuiPtrOrIndex *self);
void ImVector_ImGuiPtrOrIndex__clear_delete__2356866607(ImVector_ImGuiPtrOrIndex *self);
void ImVector_ImGuiPtrOrIndex__clear_destruct__177cfb345f(ImVector_ImGuiPtrOrIndex *self);
unsigned char ImVector_ImGuiPtrOrIndex__contains__a921ca59a7(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * v);
ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__end__4ea42fdbb0(ImVector_ImGuiPtrOrIndex *self);
const ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__end__6362d5a77d(ImVector_ImGuiPtrOrIndex *self);
ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__erase__e304f7ce9d(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * it);
ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__erase__562ea362e3(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * it, const ImGuiPtrOrIndex * it_last);
ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__erase_unsorted__c8e974aa57(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * it);
ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__find__27ef783469(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * v);
const ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__find__57736235b8(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * v);
unsigned char ImVector_ImGuiPtrOrIndex__find_erase__459c10a1cb(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * v);
unsigned char ImVector_ImGuiPtrOrIndex__find_erase_unsorted__5ec6abd462(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * v);
int ImVector_ImGuiPtrOrIndex__find_index__fa800166a6(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * v);
ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__front__adb6afa4fc(ImVector_ImGuiPtrOrIndex *self);
const ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__front__c28e84448c(ImVector_ImGuiPtrOrIndex *self);
int ImVector_ImGuiPtrOrIndex__index_from_ptr__af2614db1f(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * it);
ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__insert__f978415144(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * it, const ImGuiPtrOrIndex * v);
int ImVector_ImGuiPtrOrIndex__max_size__741f23b260(ImVector_ImGuiPtrOrIndex *self);
ImVector_ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__operator___3a58ca55ae(ImVector_ImGuiPtrOrIndex *self, const ImVector_ImGuiPtrOrIndex * src);
ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__operator____752f10372e(ImVector_ImGuiPtrOrIndex *self, int i);
const ImGuiPtrOrIndex * ImVector_ImGuiPtrOrIndex__operator____7111417356(ImVector_ImGuiPtrOrIndex *self, int i);
void ImVector_ImGuiPtrOrIndex__push_front__07583f508b(ImVector_ImGuiPtrOrIndex *self, const ImGuiPtrOrIndex * v);
void ImVector_ImGuiPtrOrIndex__reserve_discard__85a332bf53(ImVector_ImGuiPtrOrIndex *self, int new_capacity);
void ImVector_ImGuiPtrOrIndex__resize__6aabd477f9(ImVector_ImGuiPtrOrIndex *self, int new_size);
void ImVector_ImGuiPtrOrIndex__resize__d8f5e62b5b(ImVector_ImGuiPtrOrIndex *self, int new_size, const ImGuiPtrOrIndex * v);
void ImVector_ImGuiPtrOrIndex__shrink__3823b92084(ImVector_ImGuiPtrOrIndex *self, int new_size);
int ImVector_ImGuiPtrOrIndex__size__6cdd0a810c(ImVector_ImGuiPtrOrIndex *self);
int ImVector_ImGuiPtrOrIndex__size_in_bytes__96ed1e1934(ImVector_ImGuiPtrOrIndex *self);
void ImVector_ImGuiPtrOrIndex__swap__2b8c69389c(ImVector_ImGuiPtrOrIndex *self, ImVector_ImGuiPtrOrIndex * rhs);
void ImVector_ImGuiSelectionRequest__ImVector__c87105c6f0(ImVector_ImGuiSelectionRequest *self, const ImVector_ImGuiSelectionRequest * src);
int ImVector_ImGuiSelectionRequest___grow_capacity__e03477f01a(ImVector_ImGuiSelectionRequest *self, int sz);
ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__back__5306f4e6ac(ImVector_ImGuiSelectionRequest *self);
const ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__back__9c32a63c22(ImVector_ImGuiSelectionRequest *self);
int ImVector_ImGuiSelectionRequest__capacity__2333a557e4(ImVector_ImGuiSelectionRequest *self);
void ImVector_ImGuiSelectionRequest__clear__b3f36f2f52(ImVector_ImGuiSelectionRequest *self);
void ImVector_ImGuiSelectionRequest__clear_delete__6ad891757f(ImVector_ImGuiSelectionRequest *self);
void ImVector_ImGuiSelectionRequest__clear_destruct__bf1ba3a0bf(ImVector_ImGuiSelectionRequest *self);
unsigned char ImVector_ImGuiSelectionRequest__contains__f95447fe63(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * v);
unsigned char ImVector_ImGuiSelectionRequest__empty__b467ced24a(ImVector_ImGuiSelectionRequest *self);
ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__erase__14f7dfb4fa(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * it);
ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__erase__01722cfa3f(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * it, const ImGuiSelectionRequest * it_last);
ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__erase_unsorted__0677df0d6b(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * it);
ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__find__728e47e243(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * v);
const ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__find__88d76b7347(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * v);
unsigned char ImVector_ImGuiSelectionRequest__find_erase__07adb4ed49(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * v);
unsigned char ImVector_ImGuiSelectionRequest__find_erase_unsorted__6178677e80(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * v);
int ImVector_ImGuiSelectionRequest__find_index__77124acdbe(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * v);
ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__front__53cdc76ef4(ImVector_ImGuiSelectionRequest *self);
const ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__front__799577a988(ImVector_ImGuiSelectionRequest *self);
int ImVector_ImGuiSelectionRequest__index_from_ptr__48feadaf0b(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * it);
ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__insert__2e66a6547b(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * it, const ImGuiSelectionRequest * v);
int ImVector_ImGuiSelectionRequest__max_size__8a5bc86f7c(ImVector_ImGuiSelectionRequest *self);
ImVector_ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__operator___59ad1dfb00(ImVector_ImGuiSelectionRequest *self, const ImVector_ImGuiSelectionRequest * src);
ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__operator____a0e6b47fae(ImVector_ImGuiSelectionRequest *self, int i);
const ImGuiSelectionRequest * ImVector_ImGuiSelectionRequest__operator____e3db4eb4a5(ImVector_ImGuiSelectionRequest *self, int i);
void ImVector_ImGuiSelectionRequest__pop_back__ee3d257bc2(ImVector_ImGuiSelectionRequest *self);
void ImVector_ImGuiSelectionRequest__push_front__5d6f4e8f2b(ImVector_ImGuiSelectionRequest *self, const ImGuiSelectionRequest * v);
void ImVector_ImGuiSelectionRequest__reserve__b423f68834(ImGuiContext *imgui_c89_ctx, ImVector_ImGuiSelectionRequest *self, int new_capacity);
void ImVector_ImGuiSelectionRequest__reserve_discard__35479c61f1(ImVector_ImGuiSelectionRequest *self, int new_capacity);
void ImVector_ImGuiSelectionRequest__resize__9234735512(ImGuiContext *imgui_c89_ctx, ImVector_ImGuiSelectionRequest *self, int new_size);
void ImVector_ImGuiSelectionRequest__resize__e1ddb1656d(ImVector_ImGuiSelectionRequest *self, int new_size, const ImGuiSelectionRequest * v);
void ImVector_ImGuiSelectionRequest__shrink__06b8a6d056(ImVector_ImGuiSelectionRequest *self, int new_size);
int ImVector_ImGuiSelectionRequest__size__af2ab3cdbd(ImVector_ImGuiSelectionRequest *self);
int ImVector_ImGuiSelectionRequest__size_in_bytes__ff2ba70ce1(ImVector_ImGuiSelectionRequest *self);
void ImVector_ImGuiSelectionRequest__swap__2bd452a176(ImVector_ImGuiSelectionRequest *self, ImVector_ImGuiSelectionRequest * rhs);
void ImVector_ImGuiSettingsHandler__ImVector__568488bd63(ImVector_ImGuiSettingsHandler *self, const ImVector_ImGuiSettingsHandler * src);
ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__back__98a721be15(ImVector_ImGuiSettingsHandler *self);
const ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__back__b44b09f7a2(ImVector_ImGuiSettingsHandler *self);
const ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__begin__43d62731c0(ImVector_ImGuiSettingsHandler *self);
int ImVector_ImGuiSettingsHandler__capacity__74814c6ed7(ImVector_ImGuiSettingsHandler *self);
void ImVector_ImGuiSettingsHandler__clear_delete__214ca1b316(ImVector_ImGuiSettingsHandler *self);
void ImVector_ImGuiSettingsHandler__clear_destruct__4bc1ac485d(ImVector_ImGuiSettingsHandler *self);
unsigned char ImVector_ImGuiSettingsHandler__contains__5da712a556(ImVector_ImGuiSettingsHandler *self, const ImGuiSettingsHandler * v);
unsigned char ImVector_ImGuiSettingsHandler__empty__46a676cd07(ImVector_ImGuiSettingsHandler *self);
const ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__end__0aeef1b228(ImVector_ImGuiSettingsHandler *self);
ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__erase__4b89648073(ImVector_ImGuiSettingsHandler *self, const ImGuiSettingsHandler * it, const ImGuiSettingsHandler * it_last);
ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__erase_unsorted__502da3d2fe(ImVector_ImGuiSettingsHandler *self, const ImGuiSettingsHandler * it);
ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__find__01d646af26(ImVector_ImGuiSettingsHandler *self, const ImGuiSettingsHandler * v);
const ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__find__38cee579d0(ImVector_ImGuiSettingsHandler *self, const ImGuiSettingsHandler * v);
unsigned char ImVector_ImGuiSettingsHandler__find_erase__773e16e252(ImVector_ImGuiSettingsHandler *self, const ImGuiSettingsHandler * v);
unsigned char ImVector_ImGuiSettingsHandler__find_erase_unsorted__bc9e93ec20(ImVector_ImGuiSettingsHandler *self, const ImGuiSettingsHandler * v);
int ImVector_ImGuiSettingsHandler__find_index__8e9f1399a6(ImVector_ImGuiSettingsHandler *self, const ImGuiSettingsHandler * v);
ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__front__5db6d0b7a8(ImVector_ImGuiSettingsHandler *self);
const ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__front__a7abf73af5(ImVector_ImGuiSettingsHandler *self);
int ImVector_ImGuiSettingsHandler__index_from_ptr__cb37bb577a(ImVector_ImGuiSettingsHandler *self, const ImGuiSettingsHandler * it);
ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__insert__4fd1578c98(ImVector_ImGuiSettingsHandler *self, const ImGuiSettingsHandler * it, const ImGuiSettingsHandler * v);
int ImVector_ImGuiSettingsHandler__max_size__4148f0c368(ImVector_ImGuiSettingsHandler *self);
ImVector_ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__operator___504b46cc6b(ImVector_ImGuiSettingsHandler *self, const ImVector_ImGuiSettingsHandler * src);
ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__operator____17a8ce7ea5(ImVector_ImGuiSettingsHandler *self, int i);
const ImGuiSettingsHandler * ImVector_ImGuiSettingsHandler__operator____8c26e28b72(ImVector_ImGuiSettingsHandler *self, int i);
void ImVector_ImGuiSettingsHandler__pop_back__6cf8d63973(ImVector_ImGuiSettingsHandler *self);
void ImVector_ImGuiSettingsHandler__push_front__c273c0b45a(ImVector_ImGuiSettingsHandler *self, const ImGuiSettingsHandler * v);
void ImVector_ImGuiSettingsHandler__reserve_discard__2b7a7c6e50(ImVector_ImGuiSettingsHandler *self, int new_capacity);
void ImVector_ImGuiSettingsHandler__resize__cf3454b6a3(ImVector_ImGuiSettingsHandler *self, int new_size);
void ImVector_ImGuiSettingsHandler__resize__a5b77aa6e1(ImVector_ImGuiSettingsHandler *self, int new_size, const ImGuiSettingsHandler * v);
void ImVector_ImGuiSettingsHandler__shrink__cc4dac93e4(ImVector_ImGuiSettingsHandler *self, int new_size);
int ImVector_ImGuiSettingsHandler__size__af512aa96a(ImVector_ImGuiSettingsHandler *self);
int ImVector_ImGuiSettingsHandler__size_in_bytes__f0b1786a83(ImVector_ImGuiSettingsHandler *self);
void ImVector_ImGuiSettingsHandler__swap__7d60b55933(ImVector_ImGuiSettingsHandler *self, ImVector_ImGuiSettingsHandler * rhs);
void ImVector_ImGuiShrinkWidthItem__ImVector__96fb75550e(ImVector_ImGuiShrinkWidthItem *self, const ImVector_ImGuiShrinkWidthItem * src);
ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__back__b0e50a4385(ImVector_ImGuiShrinkWidthItem *self);
const ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__back__3ccc348b6f(ImVector_ImGuiShrinkWidthItem *self);
ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__begin__3d33613aec(ImVector_ImGuiShrinkWidthItem *self);
const ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__begin__56f55d9e3c(ImVector_ImGuiShrinkWidthItem *self);
int ImVector_ImGuiShrinkWidthItem__capacity__cee34917ac(ImVector_ImGuiShrinkWidthItem *self);
void ImVector_ImGuiShrinkWidthItem__clear_delete__d0c99d7978(ImVector_ImGuiShrinkWidthItem *self);
void ImVector_ImGuiShrinkWidthItem__clear_destruct__91d0524912(ImVector_ImGuiShrinkWidthItem *self);
unsigned char ImVector_ImGuiShrinkWidthItem__contains__dcd7d5f052(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * v);
unsigned char ImVector_ImGuiShrinkWidthItem__empty__752fb69d88(ImVector_ImGuiShrinkWidthItem *self);
ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__end__3020c25ddc(ImVector_ImGuiShrinkWidthItem *self);
const ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__end__2463518a04(ImVector_ImGuiShrinkWidthItem *self);
ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__erase__d47e3fc615(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * it);
ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__erase__5c838b4d4d(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * it, const ImGuiShrinkWidthItem * it_last);
ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__erase_unsorted__b1ed696f50(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * it);
ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__find__db898ef957(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * v);
const ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__find__44b87e5b03(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * v);
unsigned char ImVector_ImGuiShrinkWidthItem__find_erase__891fe2e9c3(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * v);
unsigned char ImVector_ImGuiShrinkWidthItem__find_erase_unsorted__9d975824a4(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * v);
int ImVector_ImGuiShrinkWidthItem__find_index__003c27a13d(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * v);
ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__front__b9cf3c45aa(ImVector_ImGuiShrinkWidthItem *self);
const ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__front__a514b33242(ImVector_ImGuiShrinkWidthItem *self);
int ImVector_ImGuiShrinkWidthItem__index_from_ptr__a20c794429(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * it);
ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__insert__c292f35084(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * it, const ImGuiShrinkWidthItem * v);
int ImVector_ImGuiShrinkWidthItem__max_size__040c5b4e8a(ImVector_ImGuiShrinkWidthItem *self);
ImVector_ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__operator___af6d4993da(ImVector_ImGuiShrinkWidthItem *self, const ImVector_ImGuiShrinkWidthItem * src);
const ImGuiShrinkWidthItem * ImVector_ImGuiShrinkWidthItem__operator____c5ee629212(ImVector_ImGuiShrinkWidthItem *self, int i);
void ImVector_ImGuiShrinkWidthItem__pop_back__ccf5d914d2(ImVector_ImGuiShrinkWidthItem *self);
void ImVector_ImGuiShrinkWidthItem__push_back__25760d219b(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * v);
void ImVector_ImGuiShrinkWidthItem__push_front__f1d4ea0b8a(ImVector_ImGuiShrinkWidthItem *self, const ImGuiShrinkWidthItem * v);
void ImVector_ImGuiShrinkWidthItem__reserve_discard__223eeb004f(ImVector_ImGuiShrinkWidthItem *self, int new_capacity);
void ImVector_ImGuiShrinkWidthItem__resize__077704f787(ImVector_ImGuiShrinkWidthItem *self, int new_size, const ImGuiShrinkWidthItem * v);
void ImVector_ImGuiShrinkWidthItem__shrink__0ccba25a4f(ImVector_ImGuiShrinkWidthItem *self, int new_size);
int ImVector_ImGuiShrinkWidthItem__size__f57a713996(ImVector_ImGuiShrinkWidthItem *self);
int ImVector_ImGuiShrinkWidthItem__size_in_bytes__53a18a577b(ImVector_ImGuiShrinkWidthItem *self);
void ImVector_ImGuiShrinkWidthItem__swap__0afd1213b5(ImVector_ImGuiShrinkWidthItem *self, ImVector_ImGuiShrinkWidthItem * rhs);
void ImVector_ImGuiStackLevelInfo__ImVector__453bbdc518(ImVector_ImGuiStackLevelInfo *self, const ImVector_ImGuiStackLevelInfo * src);
ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__back__a6a2280c01(ImVector_ImGuiStackLevelInfo *self);
const ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__back__27d89dbea4(ImVector_ImGuiStackLevelInfo *self);
ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__begin__1bdd0433a6(ImVector_ImGuiStackLevelInfo *self);
const ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__begin__7bcc78104e(ImVector_ImGuiStackLevelInfo *self);
int ImVector_ImGuiStackLevelInfo__capacity__3c6ee6e9d6(ImVector_ImGuiStackLevelInfo *self);
void ImVector_ImGuiStackLevelInfo__clear__3ab053fd8b(ImVector_ImGuiStackLevelInfo *self);
void ImVector_ImGuiStackLevelInfo__clear_delete__d2142b385c(ImVector_ImGuiStackLevelInfo *self);
void ImVector_ImGuiStackLevelInfo__clear_destruct__3462bffb0a(ImVector_ImGuiStackLevelInfo *self);
unsigned char ImVector_ImGuiStackLevelInfo__contains__6c1b233e96(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * v);
unsigned char ImVector_ImGuiStackLevelInfo__empty__565d2e3fca(ImVector_ImGuiStackLevelInfo *self);
ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__end__9ea9680e89(ImVector_ImGuiStackLevelInfo *self);
const ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__end__5012fc7362(ImVector_ImGuiStackLevelInfo *self);
ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__erase__9b58b09bea(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * it);
ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__erase__96ea486a7f(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * it, const ImGuiStackLevelInfo * it_last);
ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__erase_unsorted__29c75440f9(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * it);
ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__find__eaa1889ac1(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * v);
const ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__find__e06c11a20d(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * v);
unsigned char ImVector_ImGuiStackLevelInfo__find_erase__761eff807a(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * v);
unsigned char ImVector_ImGuiStackLevelInfo__find_erase_unsorted__fe34a956c3(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * v);
int ImVector_ImGuiStackLevelInfo__find_index__a7d54fb670(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * v);
ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__front__cb943a4970(ImVector_ImGuiStackLevelInfo *self);
const ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__front__5980c8b246(ImVector_ImGuiStackLevelInfo *self);
int ImVector_ImGuiStackLevelInfo__index_from_ptr__caa14a0fe3(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * it);
ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__insert__078cf00b24(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * it, const ImGuiStackLevelInfo * v);
int ImVector_ImGuiStackLevelInfo__max_size__bb58fc0263(ImVector_ImGuiStackLevelInfo *self);
ImVector_ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__operator___7d8a1da0e3(ImVector_ImGuiStackLevelInfo *self, const ImVector_ImGuiStackLevelInfo * src);
const ImGuiStackLevelInfo * ImVector_ImGuiStackLevelInfo__operator____e13789d5fb(ImVector_ImGuiStackLevelInfo *self, int i);
void ImVector_ImGuiStackLevelInfo__pop_back__4f6eaa2075(ImVector_ImGuiStackLevelInfo *self);
void ImVector_ImGuiStackLevelInfo__push_back__e503dd300b(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * v);
void ImVector_ImGuiStackLevelInfo__push_front__adcf5978d1(ImVector_ImGuiStackLevelInfo *self, const ImGuiStackLevelInfo * v);
void ImVector_ImGuiStackLevelInfo__reserve_discard__02666b918f(ImVector_ImGuiStackLevelInfo *self, int new_capacity);
void ImVector_ImGuiStackLevelInfo__shrink__ac425a820b(ImVector_ImGuiStackLevelInfo *self, int new_size);
int ImVector_ImGuiStackLevelInfo__size__82bea95f97(ImVector_ImGuiStackLevelInfo *self);
int ImVector_ImGuiStackLevelInfo__size_in_bytes__9b6a6e244a(ImVector_ImGuiStackLevelInfo *self);
void ImVector_ImGuiStackLevelInfo__swap__6cb3511d45(ImVector_ImGuiStackLevelInfo *self, ImVector_ImGuiStackLevelInfo * rhs);
void ImVector_ImGuiStoragePair__ImVector__a22baf84e2(ImVector_ImGuiStoragePair *self);
void ImVector_ImGuiStoragePair__ImVector__19fd419963(ImVector_ImGuiStoragePair *self, const ImVector_ImGuiStoragePair * src);
int ImVector_ImGuiStoragePair___grow_capacity__751c538886(ImVector_ImGuiStoragePair *self, int sz);
ImGuiStoragePair * ImVector_ImGuiStoragePair__back__c52e5d388d(ImVector_ImGuiStoragePair *self);
const ImGuiStoragePair * ImVector_ImGuiStoragePair__back__70323ac536(ImVector_ImGuiStoragePair *self);
const ImGuiStoragePair * ImVector_ImGuiStoragePair__begin__6d42c5ad64(ImVector_ImGuiStoragePair *self);
int ImVector_ImGuiStoragePair__capacity__a369aa69a0(ImVector_ImGuiStoragePair *self);
void ImVector_ImGuiStoragePair__clear_delete__b45f84a205(ImVector_ImGuiStoragePair *self);
void ImVector_ImGuiStoragePair__clear_destruct__021de82a64(ImVector_ImGuiStoragePair *self);
unsigned char ImVector_ImGuiStoragePair__contains__c690af0ada(ImVector_ImGuiStoragePair *self, const ImGuiStoragePair * v);
unsigned char ImVector_ImGuiStoragePair__empty__917f8290ce(ImVector_ImGuiStoragePair *self);
const ImGuiStoragePair * ImVector_ImGuiStoragePair__end__f141fef7e7(ImVector_ImGuiStoragePair *self);
ImGuiStoragePair * ImVector_ImGuiStoragePair__erase__52f91eb4ce(ImVector_ImGuiStoragePair *self, const ImGuiStoragePair * it);
ImGuiStoragePair * ImVector_ImGuiStoragePair__erase__e0f44217db(ImVector_ImGuiStoragePair *self, const ImGuiStoragePair * it, const ImGuiStoragePair * it_last);
ImGuiStoragePair * ImVector_ImGuiStoragePair__erase_unsorted__596dd4eb75(ImVector_ImGuiStoragePair *self, const ImGuiStoragePair * it);
ImGuiStoragePair * ImVector_ImGuiStoragePair__find__5c84cdca52(ImVector_ImGuiStoragePair *self, const ImGuiStoragePair * v);
const ImGuiStoragePair * ImVector_ImGuiStoragePair__find__3e28e88e73(ImVector_ImGuiStoragePair *self, const ImGuiStoragePair * v);
unsigned char ImVector_ImGuiStoragePair__find_erase__9095a7d34c(ImVector_ImGuiStoragePair *self, const ImGuiStoragePair * v);
unsigned char ImVector_ImGuiStoragePair__find_erase_unsorted__85ce9636c7(ImVector_ImGuiStoragePair *self, const ImGuiStoragePair * v);
int ImVector_ImGuiStoragePair__find_index__25b0afada8(ImVector_ImGuiStoragePair *self, const ImGuiStoragePair * v);
ImGuiStoragePair * ImVector_ImGuiStoragePair__front__de2b316f02(ImVector_ImGuiStoragePair *self);
const ImGuiStoragePair * ImVector_ImGuiStoragePair__front__8209845eaf(ImVector_ImGuiStoragePair *self);
int ImVector_ImGuiStoragePair__index_from_ptr__feae9e9a8a(ImVector_ImGuiStoragePair *self, const ImGuiStoragePair * it);
int ImVector_ImGuiStoragePair__max_size__bd8dfe065d(ImVector_ImGuiStoragePair *self);
ImVector_ImGuiStoragePair * ImVector_ImGuiStoragePair__operator___5878fdc549(ImVector_ImGuiStoragePair *self, const ImVector_ImGuiStoragePair * src);
const ImGuiStoragePair * ImVector_ImGuiStoragePair__operator____2fbc4f1b09(ImVector_ImGuiStoragePair *self, int i);
void ImVector_ImGuiStoragePair__pop_back__62fc501683(ImVector_ImGuiStoragePair *self);
void ImVector_ImGuiStoragePair__push_front__b041aedf11(ImVector_ImGuiStoragePair *self, const ImGuiStoragePair * v);
void ImVector_ImGuiStoragePair__reserve__176c6adf5b(ImGuiContext *imgui_c89_ctx, ImVector_ImGuiStoragePair *self, int new_capacity);
void ImVector_ImGuiStoragePair__reserve_discard__7f32753be7(ImVector_ImGuiStoragePair *self, int new_capacity);
void ImVector_ImGuiStoragePair__resize__abec9792fd(ImVector_ImGuiStoragePair *self, int new_size, const ImGuiStoragePair * v);
void ImVector_ImGuiStoragePair__shrink__a192643b54(ImVector_ImGuiStoragePair *self, int new_size);
int ImVector_ImGuiStoragePair__size__5f4a5d0290(ImVector_ImGuiStoragePair *self);
void ImVector_ImGuiStoragePair__dtor_ImVector__b2dac07a8e(ImGuiContext *imgui_c89_ctx, ImVector_ImGuiStoragePair *self);
void ImVector_ImGuiStyleMod__ImVector__be500944df(ImVector_ImGuiStyleMod *self, const ImVector_ImGuiStyleMod * src);
const ImGuiStyleMod * ImVector_ImGuiStyleMod__back__fd2081c844(ImVector_ImGuiStyleMod *self);
ImGuiStyleMod * ImVector_ImGuiStyleMod__begin__03cd1adb71(ImVector_ImGuiStyleMod *self);
const ImGuiStyleMod * ImVector_ImGuiStyleMod__begin__6075d934bf(ImVector_ImGuiStyleMod *self);
int ImVector_ImGuiStyleMod__capacity__d354e793d4(ImVector_ImGuiStyleMod *self);
void ImVector_ImGuiStyleMod__clear_delete__fb84f3eb66(ImVector_ImGuiStyleMod *self);
void ImVector_ImGuiStyleMod__clear_destruct__05b16f15f9(ImVector_ImGuiStyleMod *self);
unsigned char ImVector_ImGuiStyleMod__contains__6e8fc665c5(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * v);
unsigned char ImVector_ImGuiStyleMod__empty__c3345d3bbf(ImVector_ImGuiStyleMod *self);
ImGuiStyleMod * ImVector_ImGuiStyleMod__end__fb4e4f0409(ImVector_ImGuiStyleMod *self);
const ImGuiStyleMod * ImVector_ImGuiStyleMod__end__ca42a73de1(ImVector_ImGuiStyleMod *self);
ImGuiStyleMod * ImVector_ImGuiStyleMod__erase__46e2395306(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * it);
ImGuiStyleMod * ImVector_ImGuiStyleMod__erase__1975cdd3a4(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * it, const ImGuiStyleMod * it_last);
ImGuiStyleMod * ImVector_ImGuiStyleMod__erase_unsorted__8c2f623a10(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * it);
ImGuiStyleMod * ImVector_ImGuiStyleMod__find__c04ef8d70b(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * v);
const ImGuiStyleMod * ImVector_ImGuiStyleMod__find__3d7f0c27da(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * v);
unsigned char ImVector_ImGuiStyleMod__find_erase__17dc1eaf57(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * v);
unsigned char ImVector_ImGuiStyleMod__find_erase_unsorted__c30446a1e8(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * v);
int ImVector_ImGuiStyleMod__find_index__6dc004b449(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * v);
ImGuiStyleMod * ImVector_ImGuiStyleMod__front__7411afe184(ImVector_ImGuiStyleMod *self);
const ImGuiStyleMod * ImVector_ImGuiStyleMod__front__0bbf2c41f8(ImVector_ImGuiStyleMod *self);
int ImVector_ImGuiStyleMod__index_from_ptr__ed9fa1a189(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * it);
ImGuiStyleMod * ImVector_ImGuiStyleMod__insert__655ca01f34(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * it, const ImGuiStyleMod * v);
int ImVector_ImGuiStyleMod__max_size__4a1dc6c55d(ImVector_ImGuiStyleMod *self);
ImVector_ImGuiStyleMod * ImVector_ImGuiStyleMod__operator___9523a025c1(ImVector_ImGuiStyleMod *self, const ImVector_ImGuiStyleMod * src);
ImGuiStyleMod * ImVector_ImGuiStyleMod__operator____a6d1dc696c(ImVector_ImGuiStyleMod *self, int i);
const ImGuiStyleMod * ImVector_ImGuiStyleMod__operator____001771f82e(ImVector_ImGuiStyleMod *self, int i);
void ImVector_ImGuiStyleMod__push_front__5510dc3940(ImVector_ImGuiStyleMod *self, const ImGuiStyleMod * v);
void ImVector_ImGuiStyleMod__reserve_discard__211a7175f9(ImVector_ImGuiStyleMod *self, int new_capacity);
void ImVector_ImGuiStyleMod__resize__4cc66bebdc(ImVector_ImGuiStyleMod *self, int new_size);
void ImVector_ImGuiStyleMod__resize__139b922047(ImVector_ImGuiStyleMod *self, int new_size, const ImGuiStyleMod * v);
void ImVector_ImGuiStyleMod__shrink__106e8b02b7(ImVector_ImGuiStyleMod *self, int new_size);
int ImVector_ImGuiStyleMod__size__d38cbc0c83(ImVector_ImGuiStyleMod *self);
int ImVector_ImGuiStyleMod__size_in_bytes__9f9f1fb4ed(ImVector_ImGuiStyleMod *self);
void ImVector_ImGuiStyleMod__swap__443c122eaf(ImVector_ImGuiStyleMod *self, ImVector_ImGuiStyleMod * rhs);
void ImVector_ImGuiTabBar__ImVector__737b93da71(ImVector_ImGuiTabBar *self, const ImVector_ImGuiTabBar * src);
ImGuiTabBar * ImVector_ImGuiTabBar__back__e26c810631(ImVector_ImGuiTabBar *self);
const ImGuiTabBar * ImVector_ImGuiTabBar__back__1214484213(ImVector_ImGuiTabBar *self);
ImGuiTabBar * ImVector_ImGuiTabBar__begin__5afa89123f(ImVector_ImGuiTabBar *self);
const ImGuiTabBar * ImVector_ImGuiTabBar__begin__9134917af4(ImVector_ImGuiTabBar *self);
int ImVector_ImGuiTabBar__capacity__d9f998095b(ImVector_ImGuiTabBar *self);
void ImVector_ImGuiTabBar__clear_delete__0530671de0(ImVector_ImGuiTabBar *self);
void ImVector_ImGuiTabBar__clear_destruct__7ff8424489(ImVector_ImGuiTabBar *self);
unsigned char ImVector_ImGuiTabBar__contains__d71d49a545(ImVector_ImGuiTabBar *self, const ImGuiTabBar * v);
unsigned char ImVector_ImGuiTabBar__empty__834c9a976c(ImVector_ImGuiTabBar *self);
ImGuiTabBar * ImVector_ImGuiTabBar__end__01ab143e35(ImVector_ImGuiTabBar *self);
const ImGuiTabBar * ImVector_ImGuiTabBar__end__bc0a2f470a(ImVector_ImGuiTabBar *self);
ImGuiTabBar * ImVector_ImGuiTabBar__erase__068b1fb2df(ImVector_ImGuiTabBar *self, const ImGuiTabBar * it);
ImGuiTabBar * ImVector_ImGuiTabBar__erase__c2b1382a77(ImVector_ImGuiTabBar *self, const ImGuiTabBar * it, const ImGuiTabBar * it_last);
ImGuiTabBar * ImVector_ImGuiTabBar__erase_unsorted__44cb4f3d8d(ImVector_ImGuiTabBar *self, const ImGuiTabBar * it);
ImGuiTabBar * ImVector_ImGuiTabBar__find__000aefee6e(ImVector_ImGuiTabBar *self, const ImGuiTabBar * v);
const ImGuiTabBar * ImVector_ImGuiTabBar__find__04a3f927b4(ImVector_ImGuiTabBar *self, const ImGuiTabBar * v);
unsigned char ImVector_ImGuiTabBar__find_erase__74fe688fa3(ImVector_ImGuiTabBar *self, const ImGuiTabBar * v);
unsigned char ImVector_ImGuiTabBar__find_erase_unsorted__0693b8a11e(ImVector_ImGuiTabBar *self, const ImGuiTabBar * v);
int ImVector_ImGuiTabBar__find_index__bf957e31d3(ImVector_ImGuiTabBar *self, const ImGuiTabBar * v);
ImGuiTabBar * ImVector_ImGuiTabBar__front__93c934a213(ImVector_ImGuiTabBar *self);
const ImGuiTabBar * ImVector_ImGuiTabBar__front__f49235a3bb(ImVector_ImGuiTabBar *self);
int ImVector_ImGuiTabBar__index_from_ptr__389c71d2d1(ImVector_ImGuiTabBar *self, const ImGuiTabBar * it);
ImGuiTabBar * ImVector_ImGuiTabBar__insert__9e4f7f36d5(ImVector_ImGuiTabBar *self, const ImGuiTabBar * it, const ImGuiTabBar * v);
int ImVector_ImGuiTabBar__max_size__3df4826418(ImVector_ImGuiTabBar *self);
ImVector_ImGuiTabBar * ImVector_ImGuiTabBar__operator___191f8af4a4(ImVector_ImGuiTabBar *self, const ImVector_ImGuiTabBar * src);
ImGuiTabBar * ImVector_ImGuiTabBar__operator____b60c2ccbe8(ImVector_ImGuiTabBar *self, int i);
const ImGuiTabBar * ImVector_ImGuiTabBar__operator____e3206b771a(ImVector_ImGuiTabBar *self, int i);
void ImVector_ImGuiTabBar__pop_back__42d5522e4a(ImVector_ImGuiTabBar *self);
void ImVector_ImGuiTabBar__push_back__9f46466a1a(ImVector_ImGuiTabBar *self, const ImGuiTabBar * v);
void ImVector_ImGuiTabBar__push_front__2f32dd128f(ImVector_ImGuiTabBar *self, const ImGuiTabBar * v);
void ImVector_ImGuiTabBar__reserve_discard__3648434211(ImVector_ImGuiTabBar *self, int new_capacity);
void ImVector_ImGuiTabBar__resize__f359092ef2(ImVector_ImGuiTabBar *self, int new_size, const ImGuiTabBar * v);
void ImVector_ImGuiTabBar__shrink__24e86967ba(ImVector_ImGuiTabBar *self, int new_size);
int ImVector_ImGuiTabBar__size__912e558d02(ImVector_ImGuiTabBar *self);
int ImVector_ImGuiTabBar__size_in_bytes__75ea1d0a1c(ImVector_ImGuiTabBar *self);
void ImVector_ImGuiTabBar__swap__13779ed3a1(ImVector_ImGuiTabBar *self, ImVector_ImGuiTabBar * rhs);
void ImVector_ImGuiTabItem__ImVector__24768b0bfb(ImVector_ImGuiTabItem *self, const ImVector_ImGuiTabItem * src);
const ImGuiTabItem * ImVector_ImGuiTabItem__back__df91c48d56(ImVector_ImGuiTabItem *self);
ImGuiTabItem * ImVector_ImGuiTabItem__begin__7c2911b299(ImVector_ImGuiTabItem *self);
const ImGuiTabItem * ImVector_ImGuiTabItem__begin__b519f641f6(ImVector_ImGuiTabItem *self);
int ImVector_ImGuiTabItem__capacity__354e68789e(ImVector_ImGuiTabItem *self);
void ImVector_ImGuiTabItem__clear__392ff41f86(ImVector_ImGuiTabItem *self);
void ImVector_ImGuiTabItem__clear_delete__b70b7d9bc7(ImVector_ImGuiTabItem *self);
void ImVector_ImGuiTabItem__clear_destruct__7c3c1e0c9f(ImVector_ImGuiTabItem *self);
unsigned char ImVector_ImGuiTabItem__contains__0e9122a2e9(ImVector_ImGuiTabItem *self, const ImGuiTabItem * v);
unsigned char ImVector_ImGuiTabItem__empty__e761a5e7d3(ImVector_ImGuiTabItem *self);
ImGuiTabItem * ImVector_ImGuiTabItem__end__a126631ec0(ImVector_ImGuiTabItem *self);
const ImGuiTabItem * ImVector_ImGuiTabItem__end__4ef021df65(ImVector_ImGuiTabItem *self);
ImGuiTabItem * ImVector_ImGuiTabItem__erase__bb9ab998de(ImVector_ImGuiTabItem *self, const ImGuiTabItem * it, const ImGuiTabItem * it_last);
ImGuiTabItem * ImVector_ImGuiTabItem__erase_unsorted__167f13c654(ImVector_ImGuiTabItem *self, const ImGuiTabItem * it);
ImGuiTabItem * ImVector_ImGuiTabItem__find__8fc6c24530(ImVector_ImGuiTabItem *self, const ImGuiTabItem * v);
const ImGuiTabItem * ImVector_ImGuiTabItem__find__e22217ec97(ImVector_ImGuiTabItem *self, const ImGuiTabItem * v);
unsigned char ImVector_ImGuiTabItem__find_erase__ed72f28198(ImVector_ImGuiTabItem *self, const ImGuiTabItem * v);
unsigned char ImVector_ImGuiTabItem__find_erase_unsorted__cf3bf67832(ImVector_ImGuiTabItem *self, const ImGuiTabItem * v);
int ImVector_ImGuiTabItem__find_index__84238a3e03(ImVector_ImGuiTabItem *self, const ImGuiTabItem * v);
ImGuiTabItem * ImVector_ImGuiTabItem__front__6b2b81fc90(ImVector_ImGuiTabItem *self);
const ImGuiTabItem * ImVector_ImGuiTabItem__front__72ed1c10a4(ImVector_ImGuiTabItem *self);
int ImVector_ImGuiTabItem__index_from_ptr__c89320e32a(ImVector_ImGuiTabItem *self, const ImGuiTabItem * it);
ImGuiTabItem * ImVector_ImGuiTabItem__insert__1699205f90(ImVector_ImGuiTabItem *self, const ImGuiTabItem * it, const ImGuiTabItem * v);
int ImVector_ImGuiTabItem__max_size__957c50ba49(ImVector_ImGuiTabItem *self);
ImVector_ImGuiTabItem * ImVector_ImGuiTabItem__operator___1152f57c5e(ImVector_ImGuiTabItem *self, const ImVector_ImGuiTabItem * src);
ImGuiTabItem * ImVector_ImGuiTabItem__operator____f111b122c1(ImVector_ImGuiTabItem *self, int i);
const ImGuiTabItem * ImVector_ImGuiTabItem__operator____249e58d90e(ImVector_ImGuiTabItem *self, int i);
void ImVector_ImGuiTabItem__pop_back__a15ef80b81(ImVector_ImGuiTabItem *self);
void ImVector_ImGuiTabItem__push_front__8ff1895c60(ImVector_ImGuiTabItem *self, const ImGuiTabItem * v);
void ImVector_ImGuiTabItem__reserve_discard__e05ef3b7aa(ImVector_ImGuiTabItem *self, int new_capacity);
void ImVector_ImGuiTabItem__resize__f1306a45c8(ImVector_ImGuiTabItem *self, int new_size, const ImGuiTabItem * v);
void ImVector_ImGuiTabItem__shrink__047e49494f(ImVector_ImGuiTabItem *self, int new_size);
int ImVector_ImGuiTabItem__size__651d2c3e0e(ImVector_ImGuiTabItem *self);
int ImVector_ImGuiTabItem__size_in_bytes__c667eb8c6a(ImVector_ImGuiTabItem *self);
void ImVector_ImGuiTabItem__swap__4288b65862(ImVector_ImGuiTabItem *self, ImVector_ImGuiTabItem * rhs);
void ImVector_ImGuiTextFilter_ImGuiTextRange__ImVector__3263a4fdb8(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImVector_ImGuiTextFilter_ImGuiTextRange * src);
ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__back__b840303c62(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
const ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__back__98d1f7ff24(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
int ImVector_ImGuiTextFilter_ImGuiTextRange__capacity__651405e475(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
void ImVector_ImGuiTextFilter_ImGuiTextRange__clear__2220f8268f(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
void ImVector_ImGuiTextFilter_ImGuiTextRange__clear_delete__9da43ac5ca(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
void ImVector_ImGuiTextFilter_ImGuiTextRange__clear_destruct__5aa9a84144(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
unsigned char ImVector_ImGuiTextFilter_ImGuiTextRange__contains__d186c51987(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * v);
ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__erase__b654e43d1a(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * it);
ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__erase__b7640a8b34(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * it, const ImGuiTextFilter_ImGuiTextRange * it_last);
ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__erase_unsorted__3b2882a2fe(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * it);
ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__find__08aefbb05a(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * v);
const ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__find__ca4ddc3c58(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * v);
unsigned char ImVector_ImGuiTextFilter_ImGuiTextRange__find_erase__d7329adc42(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * v);
unsigned char ImVector_ImGuiTextFilter_ImGuiTextRange__find_erase_unsorted__44dedf9e30(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * v);
int ImVector_ImGuiTextFilter_ImGuiTextRange__find_index__5f1ce10633(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * v);
ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__front__1c6ac50fbf(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
const ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__front__86f0b52953(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
int ImVector_ImGuiTextFilter_ImGuiTextRange__index_from_ptr__957c40ae6d(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * it);
ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__insert__4609537a9c(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * it, const ImGuiTextFilter_ImGuiTextRange * v);
int ImVector_ImGuiTextFilter_ImGuiTextRange__max_size__b46a8a0510(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
ImVector_ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__operator___e3ac197b36(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImVector_ImGuiTextFilter_ImGuiTextRange * src);
ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__operator____f18cba3076(ImVector_ImGuiTextFilter_ImGuiTextRange *self, int i);
const ImGuiTextFilter_ImGuiTextRange * ImVector_ImGuiTextFilter_ImGuiTextRange__operator____64e4aef001(ImVector_ImGuiTextFilter_ImGuiTextRange *self, int i);
void ImVector_ImGuiTextFilter_ImGuiTextRange__pop_back__6470e1d001(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
void ImVector_ImGuiTextFilter_ImGuiTextRange__push_front__105dc8c96e(ImVector_ImGuiTextFilter_ImGuiTextRange *self, const ImGuiTextFilter_ImGuiTextRange * v);
void ImVector_ImGuiTextFilter_ImGuiTextRange__reserve_discard__695443a993(ImVector_ImGuiTextFilter_ImGuiTextRange *self, int new_capacity);
void ImVector_ImGuiTextFilter_ImGuiTextRange__resize__97d692f221(ImVector_ImGuiTextFilter_ImGuiTextRange *self, int new_size, const ImGuiTextFilter_ImGuiTextRange * v);
void ImVector_ImGuiTextFilter_ImGuiTextRange__shrink__e66282e675(ImVector_ImGuiTextFilter_ImGuiTextRange *self, int new_size);
int ImVector_ImGuiTextFilter_ImGuiTextRange__size__a7859e4e27(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
int ImVector_ImGuiTextFilter_ImGuiTextRange__size_in_bytes__e284698367(ImVector_ImGuiTextFilter_ImGuiTextRange *self);
void ImVector_ImGuiTextFilter_ImGuiTextRange__swap__8da55ba1bc(ImVector_ImGuiTextFilter_ImGuiTextRange *self, ImVector_ImGuiTextFilter_ImGuiTextRange * rhs);
void ImVector_ImGuiTreeNodeStackData__ImVector__041e4f3085(ImVector_ImGuiTreeNodeStackData *self, const ImVector_ImGuiTreeNodeStackData * src);
ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__back__7608aa0b90(ImVector_ImGuiTreeNodeStackData *self);
const ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__back__d1a198ab10(ImVector_ImGuiTreeNodeStackData *self);
ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__begin__287c633a7a(ImVector_ImGuiTreeNodeStackData *self);
const ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__begin__8f2e6a37f7(ImVector_ImGuiTreeNodeStackData *self);
int ImVector_ImGuiTreeNodeStackData__capacity__2ba2cbaf44(ImVector_ImGuiTreeNodeStackData *self);
void ImVector_ImGuiTreeNodeStackData__clear_delete__270048e5a2(ImVector_ImGuiTreeNodeStackData *self);
void ImVector_ImGuiTreeNodeStackData__clear_destruct__f025a71597(ImVector_ImGuiTreeNodeStackData *self);
unsigned char ImVector_ImGuiTreeNodeStackData__contains__548d411546(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * v);
unsigned char ImVector_ImGuiTreeNodeStackData__empty__743e10ad91(ImVector_ImGuiTreeNodeStackData *self);
ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__end__57d9330005(ImVector_ImGuiTreeNodeStackData *self);
const ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__end__ffdc80320b(ImVector_ImGuiTreeNodeStackData *self);
ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__erase__17ea7910eb(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * it);
ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__erase__4411f1204a(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * it, const ImGuiTreeNodeStackData * it_last);
ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__erase_unsorted__8122c0a34d(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * it);
ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__find__313d2c6f02(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * v);
const ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__find__e759302c79(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * v);
unsigned char ImVector_ImGuiTreeNodeStackData__find_erase__05a455b52e(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * v);
unsigned char ImVector_ImGuiTreeNodeStackData__find_erase_unsorted__ebc0410819(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * v);
int ImVector_ImGuiTreeNodeStackData__find_index__ef49c569ed(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * v);
ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__front__b74d81962e(ImVector_ImGuiTreeNodeStackData *self);
const ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__front__c73fc62b85(ImVector_ImGuiTreeNodeStackData *self);
int ImVector_ImGuiTreeNodeStackData__index_from_ptr__43d643569e(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * it);
ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__insert__b170e0eb4d(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * it, const ImGuiTreeNodeStackData * v);
int ImVector_ImGuiTreeNodeStackData__max_size__b5ec0d783a(ImVector_ImGuiTreeNodeStackData *self);
ImVector_ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__operator___b6f61adee5(ImVector_ImGuiTreeNodeStackData *self, const ImVector_ImGuiTreeNodeStackData * src);
ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__operator____b9ec747124(ImVector_ImGuiTreeNodeStackData *self, int i);
const ImGuiTreeNodeStackData * ImVector_ImGuiTreeNodeStackData__operator____c57af471a8(ImVector_ImGuiTreeNodeStackData *self, int i);
void ImVector_ImGuiTreeNodeStackData__push_back__684133395c(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * v);
void ImVector_ImGuiTreeNodeStackData__push_front__924e1182c9(ImVector_ImGuiTreeNodeStackData *self, const ImGuiTreeNodeStackData * v);
void ImVector_ImGuiTreeNodeStackData__reserve_discard__3ee0ec127c(ImVector_ImGuiTreeNodeStackData *self, int new_capacity);
void ImVector_ImGuiTreeNodeStackData__resize__41f4274ee7(ImVector_ImGuiTreeNodeStackData *self, int new_size, const ImGuiTreeNodeStackData * v);
void ImVector_ImGuiTreeNodeStackData__shrink__8218205c03(ImVector_ImGuiTreeNodeStackData *self, int new_size);
int ImVector_ImGuiTreeNodeStackData__size__e1f07d12db(ImVector_ImGuiTreeNodeStackData *self);
int ImVector_ImGuiTreeNodeStackData__size_in_bytes__30e8b3efb4(ImVector_ImGuiTreeNodeStackData *self);
void ImVector_ImGuiTreeNodeStackData__swap__0de3eac9a7(ImVector_ImGuiTreeNodeStackData *self, ImVector_ImGuiTreeNodeStackData * rhs);
void ImVector_ImGuiWindowStackData__ImVector__d8a5c89c97(ImVector_ImGuiWindowStackData *self, const ImVector_ImGuiWindowStackData * src);
const ImGuiWindowStackData * ImVector_ImGuiWindowStackData__back__51202f1336(ImVector_ImGuiWindowStackData *self);
ImGuiWindowStackData * ImVector_ImGuiWindowStackData__begin__b7f860e14c(ImVector_ImGuiWindowStackData *self);
const ImGuiWindowStackData * ImVector_ImGuiWindowStackData__begin__6a8b6fed8c(ImVector_ImGuiWindowStackData *self);
int ImVector_ImGuiWindowStackData__capacity__40d3fa1444(ImVector_ImGuiWindowStackData *self);
void ImVector_ImGuiWindowStackData__clear_delete__900e89d9d7(ImVector_ImGuiWindowStackData *self);
void ImVector_ImGuiWindowStackData__clear_destruct__87d36b50f9(ImVector_ImGuiWindowStackData *self);
unsigned char ImVector_ImGuiWindowStackData__contains__22a61e915b(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * v);
ImGuiWindowStackData * ImVector_ImGuiWindowStackData__end__dbcff1fa87(ImVector_ImGuiWindowStackData *self);
const ImGuiWindowStackData * ImVector_ImGuiWindowStackData__end__6e48dcf113(ImVector_ImGuiWindowStackData *self);
ImGuiWindowStackData * ImVector_ImGuiWindowStackData__erase__0a30169f93(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * it);
ImGuiWindowStackData * ImVector_ImGuiWindowStackData__erase__556335427a(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * it, const ImGuiWindowStackData * it_last);
ImGuiWindowStackData * ImVector_ImGuiWindowStackData__erase_unsorted__5936d8b591(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * it);
ImGuiWindowStackData * ImVector_ImGuiWindowStackData__find__7844fe6fcf(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * v);
const ImGuiWindowStackData * ImVector_ImGuiWindowStackData__find__38ed596d64(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * v);
unsigned char ImVector_ImGuiWindowStackData__find_erase__9226af87dc(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * v);
unsigned char ImVector_ImGuiWindowStackData__find_erase_unsorted__4197631f4f(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * v);
int ImVector_ImGuiWindowStackData__find_index__20d77c778e(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * v);
ImGuiWindowStackData * ImVector_ImGuiWindowStackData__front__90d7769da7(ImVector_ImGuiWindowStackData *self);
const ImGuiWindowStackData * ImVector_ImGuiWindowStackData__front__4125e17839(ImVector_ImGuiWindowStackData *self);
int ImVector_ImGuiWindowStackData__index_from_ptr__39b03b5552(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * it);
ImGuiWindowStackData * ImVector_ImGuiWindowStackData__insert__9fd649a9b7(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * it, const ImGuiWindowStackData * v);
int ImVector_ImGuiWindowStackData__max_size__b046a67dec(ImVector_ImGuiWindowStackData *self);
ImVector_ImGuiWindowStackData * ImVector_ImGuiWindowStackData__operator___424eaa9035(ImVector_ImGuiWindowStackData *self, const ImVector_ImGuiWindowStackData * src);
const ImGuiWindowStackData * ImVector_ImGuiWindowStackData__operator____dfe2159e8d(ImVector_ImGuiWindowStackData *self, int i);
void ImVector_ImGuiWindowStackData__push_back__827b58928b(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * v);
void ImVector_ImGuiWindowStackData__push_front__2ba71f2e81(ImVector_ImGuiWindowStackData *self, const ImGuiWindowStackData * v);
void ImVector_ImGuiWindowStackData__reserve_discard__7df4091e76(ImVector_ImGuiWindowStackData *self, int new_capacity);
void ImVector_ImGuiWindowStackData__resize__112fa06ecc(ImVector_ImGuiWindowStackData *self, int new_size, const ImGuiWindowStackData * v);
void ImVector_ImGuiWindowStackData__shrink__4ba3b8afa7(ImVector_ImGuiWindowStackData *self, int new_size);
int ImVector_ImGuiWindowStackData__size__50f39d89ae(ImVector_ImGuiWindowStackData *self);
int ImVector_ImGuiWindowStackData__size_in_bytes__d90fb82703(ImVector_ImGuiWindowStackData *self);
void ImVector_ImGuiWindowStackData__swap__7539634d48(ImVector_ImGuiWindowStackData *self, ImVector_ImGuiWindowStackData * rhs);
void ImVector_ImTextureRect__ImVector__fc74ae4626(ImVector_ImTextureRect *self, const ImVector_ImTextureRect * src);
ImTextureRect * ImVector_ImTextureRect__back__4282a37f5a(ImVector_ImTextureRect *self);
const ImTextureRect * ImVector_ImTextureRect__back__52f14d059e(ImVector_ImTextureRect *self);
const ImTextureRect * ImVector_ImTextureRect__begin__6a65bc95b0(ImVector_ImTextureRect *self);
int ImVector_ImTextureRect__capacity__1ef0eb8a8c(ImVector_ImTextureRect *self);
void ImVector_ImTextureRect__clear__e007c127af(ImVector_ImTextureRect *self);
void ImVector_ImTextureRect__clear_delete__63a90d732c(ImVector_ImTextureRect *self);
void ImVector_ImTextureRect__clear_destruct__878604dc47(ImVector_ImTextureRect *self);
unsigned char ImVector_ImTextureRect__contains__a2b4378dd9(ImVector_ImTextureRect *self, const ImTextureRect * v);
unsigned char ImVector_ImTextureRect__empty__08bb0dab12(ImVector_ImTextureRect *self);
const ImTextureRect * ImVector_ImTextureRect__end__85b1ebeeb5(ImVector_ImTextureRect *self);
ImTextureRect * ImVector_ImTextureRect__erase__73a31b83cc(ImVector_ImTextureRect *self, const ImTextureRect * it);
ImTextureRect * ImVector_ImTextureRect__erase__2ca7f6a105(ImVector_ImTextureRect *self, const ImTextureRect * it, const ImTextureRect * it_last);
ImTextureRect * ImVector_ImTextureRect__erase_unsorted__db1942eeea(ImVector_ImTextureRect *self, const ImTextureRect * it);
ImTextureRect * ImVector_ImTextureRect__find__14fbcb7e43(ImVector_ImTextureRect *self, const ImTextureRect * v);
const ImTextureRect * ImVector_ImTextureRect__find__760305dd17(ImVector_ImTextureRect *self, const ImTextureRect * v);
unsigned char ImVector_ImTextureRect__find_erase__b2b6b080ca(ImVector_ImTextureRect *self, const ImTextureRect * v);
unsigned char ImVector_ImTextureRect__find_erase_unsorted__15fdf9b430(ImVector_ImTextureRect *self, const ImTextureRect * v);
int ImVector_ImTextureRect__find_index__1b02edc07b(ImVector_ImTextureRect *self, const ImTextureRect * v);
ImTextureRect * ImVector_ImTextureRect__front__cc48c04744(ImVector_ImTextureRect *self);
const ImTextureRect * ImVector_ImTextureRect__front__befe166553(ImVector_ImTextureRect *self);
int ImVector_ImTextureRect__index_from_ptr__793ec66bda(ImVector_ImTextureRect *self, const ImTextureRect * it);
ImTextureRect * ImVector_ImTextureRect__insert__43fbe5627f(ImVector_ImTextureRect *self, const ImTextureRect * it, const ImTextureRect * v);
int ImVector_ImTextureRect__max_size__7585011213(ImVector_ImTextureRect *self);
ImVector_ImTextureRect * ImVector_ImTextureRect__operator___6825210199(ImVector_ImTextureRect *self, const ImVector_ImTextureRect * src);
const ImTextureRect * ImVector_ImTextureRect__operator____a5bed51879(ImVector_ImTextureRect *self, int i);
void ImVector_ImTextureRect__pop_back__d4de9925e0(ImVector_ImTextureRect *self);
void ImVector_ImTextureRect__push_front__82f06e841d(ImVector_ImTextureRect *self, const ImTextureRect * v);
void ImVector_ImTextureRect__reserve_discard__4b44124e4b(ImVector_ImTextureRect *self, int new_capacity);
void ImVector_ImTextureRect__resize__b212715d10(ImVector_ImTextureRect *self, int new_size, const ImTextureRect * v);
void ImVector_ImTextureRect__shrink__ee198f42b2(ImVector_ImTextureRect *self, int new_size);
int ImVector_ImTextureRect__size__6240660544(ImVector_ImTextureRect *self);
int ImVector_ImTextureRect__size_in_bytes__3df3680762(ImVector_ImTextureRect *self);
void ImVector_ImTextureRef__ImVector__4f91b30f71(ImVector_ImTextureRef *self, const ImVector_ImTextureRef * src);
const ImTextureRef * ImVector_ImTextureRef__back__bb637632c7(ImVector_ImTextureRef *self);
const ImTextureRef * ImVector_ImTextureRef__begin__106606e0e3(ImVector_ImTextureRef *self);
int ImVector_ImTextureRef__capacity__556e872b61(ImVector_ImTextureRef *self);
void ImVector_ImTextureRef__clear_delete__5015aefe20(ImVector_ImTextureRef *self);
void ImVector_ImTextureRef__clear_destruct__f41d6e192d(ImVector_ImTextureRef *self);
unsigned char ImVector_ImTextureRef__contains__4553494bbb(ImVector_ImTextureRef *self, const ImTextureRef * v);
unsigned char ImVector_ImTextureRef__empty__5b7d290a39(ImVector_ImTextureRef *self);
const ImTextureRef * ImVector_ImTextureRef__end__f103e66cbe(ImVector_ImTextureRef *self);
ImTextureRef * ImVector_ImTextureRef__erase__fb7341e295(ImVector_ImTextureRef *self, const ImTextureRef * it);
ImTextureRef * ImVector_ImTextureRef__erase__23723b235f(ImVector_ImTextureRef *self, const ImTextureRef * it, const ImTextureRef * it_last);
ImTextureRef * ImVector_ImTextureRef__erase_unsorted__83ce8af6bc(ImVector_ImTextureRef *self, const ImTextureRef * it);
ImTextureRef * ImVector_ImTextureRef__find__19577f41ea(ImVector_ImTextureRef *self, const ImTextureRef * v);
const ImTextureRef * ImVector_ImTextureRef__find__c3dc165587(ImVector_ImTextureRef *self, const ImTextureRef * v);
unsigned char ImVector_ImTextureRef__find_erase__7917123ef3(ImVector_ImTextureRef *self, const ImTextureRef * v);
unsigned char ImVector_ImTextureRef__find_erase_unsorted__4e5f723149(ImVector_ImTextureRef *self, const ImTextureRef * v);
int ImVector_ImTextureRef__find_index__add7a76f4a(ImVector_ImTextureRef *self, const ImTextureRef * v);
ImTextureRef * ImVector_ImTextureRef__front__f4fba22fed(ImVector_ImTextureRef *self);
const ImTextureRef * ImVector_ImTextureRef__front__d681a93666(ImVector_ImTextureRef *self);
int ImVector_ImTextureRef__index_from_ptr__d59ddb4a06(ImVector_ImTextureRef *self, const ImTextureRef * it);
ImTextureRef * ImVector_ImTextureRef__insert__3d357efccb(ImVector_ImTextureRef *self, const ImTextureRef * it, const ImTextureRef * v);
int ImVector_ImTextureRef__max_size__e13b6d0fc9(ImVector_ImTextureRef *self);
ImVector_ImTextureRef * ImVector_ImTextureRef__operator___2b0d2266de(ImVector_ImTextureRef *self, const ImVector_ImTextureRef * src);
ImTextureRef * ImVector_ImTextureRef__operator____b00eeb2eae(ImVector_ImTextureRef *self, int i);
const ImTextureRef * ImVector_ImTextureRef__operator____4ca8d75c4c(ImVector_ImTextureRef *self, int i);
void ImVector_ImTextureRef__push_front__9b070a3719(ImVector_ImTextureRef *self, const ImTextureRef * v);
void ImVector_ImTextureRef__reserve_discard__d7f9a809a3(ImVector_ImTextureRef *self, int new_capacity);
void ImVector_ImTextureRef__resize__1ad9353733(ImVector_ImTextureRef *self, int new_size, const ImTextureRef * v);
void ImVector_ImTextureRef__shrink__179860d044(ImVector_ImTextureRef *self, int new_size);
int ImVector_ImTextureRef__size__409656739b(ImVector_ImTextureRef *self);
int ImVector_ImTextureRef__size_in_bytes__42d0a36cf6(ImVector_ImTextureRef *self);
void ImVector_ImTextureRef__swap__cfea5ea316(ImVector_ImTextureRef *self, ImVector_ImTextureRef * rhs);
void ImVector_ImVec2__ImVector__b6c99dbefa(ImVector_ImVec2 *self, const ImVector_ImVec2 * src);
int ImVector_ImVec2___grow_capacity__17b9135fe9(ImVector_ImVec2 *self, int sz);
const ImVec2 * ImVector_ImVec2__back__18d5daef32(ImVector_ImVec2 *self);
ImVec2 * ImVector_ImVec2__begin__14be0ae753(ImVector_ImVec2 *self);
const ImVec2 * ImVector_ImVec2__begin__3a5111b1ce(ImVector_ImVec2 *self);
int ImVector_ImVec2__capacity__66b0ff9ae8(ImVector_ImVec2 *self);
void ImVector_ImVec2__clear__70cd3a0dee(ImGuiContext *imgui_c89_ctx, ImVector_ImVec2 *self);
void ImVector_ImVec2__clear_delete__327ae73505(ImVector_ImVec2 *self);
void ImVector_ImVec2__clear_destruct__cb087bf08e(ImVector_ImVec2 *self);
unsigned char ImVector_ImVec2__contains__e4289a42e7(ImVector_ImVec2 *self, const ImVec2 * v);
unsigned char ImVector_ImVec2__empty__4678a9aa9c(ImVector_ImVec2 *self);
ImVec2 * ImVector_ImVec2__end__3b1a0a953a(ImVector_ImVec2 *self);
const ImVec2 * ImVector_ImVec2__end__ae8fbdd9e3(ImVector_ImVec2 *self);
ImVec2 * ImVector_ImVec2__erase__54d1b2eb26(ImVector_ImVec2 *self, const ImVec2 * it);
ImVec2 * ImVector_ImVec2__erase__73a307af8f(ImVector_ImVec2 *self, const ImVec2 * it, const ImVec2 * it_last);
ImVec2 * ImVector_ImVec2__erase_unsorted__70f56768eb(ImVector_ImVec2 *self, const ImVec2 * it);
ImVec2 * ImVector_ImVec2__find__7863c8887f(ImVector_ImVec2 *self, const ImVec2 * v);
const ImVec2 * ImVector_ImVec2__find__c9ccef86d8(ImVector_ImVec2 *self, const ImVec2 * v);
unsigned char ImVector_ImVec2__find_erase__2e95e854b5(ImVector_ImVec2 *self, const ImVec2 * v);
unsigned char ImVector_ImVec2__find_erase_unsorted__eca0fd8fcd(ImVector_ImVec2 *self, const ImVec2 * v);
int ImVector_ImVec2__find_index__ac765fb871(ImVector_ImVec2 *self, const ImVec2 * v);
ImVec2 * ImVector_ImVec2__front__25c98a49a0(ImVector_ImVec2 *self);
const ImVec2 * ImVector_ImVec2__front__37a8b2d5fb(ImVector_ImVec2 *self);
int ImVector_ImVec2__index_from_ptr__b955051df7(ImVector_ImVec2 *self, const ImVec2 * it);
ImVec2 * ImVector_ImVec2__insert__014493983d(ImVector_ImVec2 *self, const ImVec2 * it, const ImVec2 * v);
int ImVector_ImVec2__max_size__457d7bb967(ImVector_ImVec2 *self);
ImVector_ImVec2 * ImVector_ImVec2__operator___d012c4a7be(ImVector_ImVec2 *self, const ImVector_ImVec2 * src);
ImVec2 * ImVector_ImVec2__operator____d1fa3450fe(ImVector_ImVec2 *self, int i);
const ImVec2 * ImVector_ImVec2__operator____203e129f7b(ImVector_ImVec2 *self, int i);
void ImVector_ImVec2__pop_back__ee60548364(ImVector_ImVec2 *self);
void ImVector_ImVec2__push_back__e0b23d7d67(ImGuiContext *imgui_c89_ctx, ImVector_ImVec2 *self, const ImVec2 * v);
void ImVector_ImVec2__push_front__b2ceff852a(ImVector_ImVec2 *self, const ImVec2 * v);
void ImVector_ImVec2__reserve__052b8f19cc(ImGuiContext *imgui_c89_ctx, ImVector_ImVec2 *self, int new_capacity);
void ImVector_ImVec2__resize__fa950d3844(ImVector_ImVec2 *self, int new_size, const ImVec2 * v);
void ImVector_ImVec2__shrink__f2f44d71cf(ImVector_ImVec2 *self, int new_size);
int ImVector_ImVec2__size__3b2fb8cdd2(ImVector_ImVec2 *self);
int ImVector_ImVec2__size_in_bytes__2ba2bb9db6(ImVector_ImVec2 *self);
void ImVector_ImVec2__swap__84daf85f3a(ImVector_ImVec2 *self, ImVector_ImVec2 * rhs);
void ImVector_ImVec4__ImVector__2f923a788c(ImVector_ImVec4 *self, const ImVector_ImVec4 * src);
ImVec4 * ImVector_ImVec4__begin__72419e3b76(ImVector_ImVec4 *self);
const ImVec4 * ImVector_ImVec4__begin__e500a8baa5(ImVector_ImVec4 *self);
int ImVector_ImVec4__capacity__8cb419a075(ImVector_ImVec4 *self);
void ImVector_ImVec4__clear_delete__238239c41d(ImVector_ImVec4 *self);
void ImVector_ImVec4__clear_destruct__bb47091a26(ImVector_ImVec4 *self);
unsigned char ImVector_ImVec4__contains__ccefd920e6(ImVector_ImVec4 *self, const ImVec4 * v);
unsigned char ImVector_ImVec4__empty__bcb9224644(ImVector_ImVec4 *self);
ImVec4 * ImVector_ImVec4__end__a4fe51434e(ImVector_ImVec4 *self);
const ImVec4 * ImVector_ImVec4__end__a681b71bf8(ImVector_ImVec4 *self);
ImVec4 * ImVector_ImVec4__erase__c542a15a55(ImVector_ImVec4 *self, const ImVec4 * it);
ImVec4 * ImVector_ImVec4__erase__93bd9b2855(ImVector_ImVec4 *self, const ImVec4 * it, const ImVec4 * it_last);
ImVec4 * ImVector_ImVec4__erase_unsorted__377666ae69(ImVector_ImVec4 *self, const ImVec4 * it);
ImVec4 * ImVector_ImVec4__find__733425f01f(ImVector_ImVec4 *self, const ImVec4 * v);
const ImVec4 * ImVector_ImVec4__find__4149c2a430(ImVector_ImVec4 *self, const ImVec4 * v);
unsigned char ImVector_ImVec4__find_erase__7d9065b789(ImVector_ImVec4 *self, const ImVec4 * v);
unsigned char ImVector_ImVec4__find_erase_unsorted__16f99488c6(ImVector_ImVec4 *self, const ImVec4 * v);
int ImVector_ImVec4__find_index__005af0f3d9(ImVector_ImVec4 *self, const ImVec4 * v);
ImVec4 * ImVector_ImVec4__front__2e518e840f(ImVector_ImVec4 *self);
const ImVec4 * ImVector_ImVec4__front__e8fc4e4da5(ImVector_ImVec4 *self);
int ImVector_ImVec4__index_from_ptr__2416000542(ImVector_ImVec4 *self, const ImVec4 * it);
ImVec4 * ImVector_ImVec4__insert__11531f6034(ImVector_ImVec4 *self, const ImVec4 * it, const ImVec4 * v);
int ImVector_ImVec4__max_size__f5461646b1(ImVector_ImVec4 *self);
ImVector_ImVec4 * ImVector_ImVec4__operator___9c786d1828(ImVector_ImVec4 *self, const ImVector_ImVec4 * src);
ImVec4 * ImVector_ImVec4__operator____6856c2a008(ImVector_ImVec4 *self, int i);
const ImVec4 * ImVector_ImVec4__operator____8629f8a6b2(ImVector_ImVec4 *self, int i);
void ImVector_ImVec4__push_front__3eedbcfb0c(ImVector_ImVec4 *self, const ImVec4 * v);
void ImVector_ImVec4__reserve_discard__7eb6b1bbbf(ImVector_ImVec4 *self, int new_capacity);
void ImVector_ImVec4__resize__2e261abcd4(ImVector_ImVec4 *self, int new_size, const ImVec4 * v);
void ImVector_ImVec4__shrink__2cb9d16136(ImVector_ImVec4 *self, int new_size);
int ImVector_ImVec4__size__6309f8800e(ImVector_ImVec4 *self);
int ImVector_ImVec4__size_in_bytes__06aa2728a4(ImVector_ImVec4 *self);
void ImVector_ImVec4__swap__2d89ad2e0e(ImVector_ImVec4 *self, ImVector_ImVec4 * rhs);
void ImVector_stbrp_node__ImVector__eff70fcb02(ImVector_stbrp_node *self, const ImVector_stbrp_node * src);
stbrp_node * ImVector_stbrp_node__back__7d7af07bbc(ImVector_stbrp_node *self);
const stbrp_node * ImVector_stbrp_node__back__7791e9cd4b(ImVector_stbrp_node *self);
stbrp_node * ImVector_stbrp_node__begin__e561e13769(ImVector_stbrp_node *self);
const stbrp_node * ImVector_stbrp_node__begin__38053fae8c(ImVector_stbrp_node *self);
int ImVector_stbrp_node__capacity__313e6f1611(ImVector_stbrp_node *self);
void ImVector_stbrp_node__clear__44a9060093(ImVector_stbrp_node *self);
void ImVector_stbrp_node__clear_delete__9040a16083(ImVector_stbrp_node *self);
void ImVector_stbrp_node__clear_destruct__cb77033e59(ImVector_stbrp_node *self);
unsigned char ImVector_stbrp_node__contains__44505ec2d6(ImVector_stbrp_node *self, const stbrp_node * v);
unsigned char ImVector_stbrp_node__empty__a32f821915(ImVector_stbrp_node *self);
stbrp_node * ImVector_stbrp_node__end__e8ae40529e(ImVector_stbrp_node *self);
const stbrp_node * ImVector_stbrp_node__end__5a40f01473(ImVector_stbrp_node *self);
stbrp_node * ImVector_stbrp_node__erase__5126d2fcd9(ImVector_stbrp_node *self, const stbrp_node * it);
stbrp_node * ImVector_stbrp_node__erase__10085471ab(ImVector_stbrp_node *self, const stbrp_node * it, const stbrp_node * it_last);
stbrp_node * ImVector_stbrp_node__erase_unsorted__f346a41a98(ImVector_stbrp_node *self, const stbrp_node * it);
stbrp_node * ImVector_stbrp_node__find__8aa7fd5bd9(ImVector_stbrp_node *self, const stbrp_node * v);
const stbrp_node * ImVector_stbrp_node__find__345e031f05(ImVector_stbrp_node *self, const stbrp_node * v);
unsigned char ImVector_stbrp_node__find_erase__bd1363c676(ImVector_stbrp_node *self, const stbrp_node * v);
unsigned char ImVector_stbrp_node__find_erase_unsorted__274f334275(ImVector_stbrp_node *self, const stbrp_node * v);
int ImVector_stbrp_node__find_index__f9c292aef1(ImVector_stbrp_node *self, const stbrp_node * v);
stbrp_node * ImVector_stbrp_node__front__049c09bcfd(ImVector_stbrp_node *self);
const stbrp_node * ImVector_stbrp_node__front__2bb0cc1ecd(ImVector_stbrp_node *self);
int ImVector_stbrp_node__index_from_ptr__ef744e0c11(ImVector_stbrp_node *self, const stbrp_node * it);
stbrp_node * ImVector_stbrp_node__insert__c6dce776ed(ImVector_stbrp_node *self, const stbrp_node * it, const stbrp_node * v);
int ImVector_stbrp_node__max_size__7522360cf1(ImVector_stbrp_node *self);
ImVector_stbrp_node * ImVector_stbrp_node__operator___85a01e637d(ImVector_stbrp_node *self, const ImVector_stbrp_node * src);
stbrp_node * ImVector_stbrp_node__operator____d8623efe2b(ImVector_stbrp_node *self, int i);
const stbrp_node * ImVector_stbrp_node__operator____52ab02f150(ImVector_stbrp_node *self, int i);
void ImVector_stbrp_node__pop_back__e56a46ef0f(ImVector_stbrp_node *self);
void ImVector_stbrp_node__push_back__bce2c82709(ImVector_stbrp_node *self, const stbrp_node * v);
void ImVector_stbrp_node__push_front__5c9bb66675(ImVector_stbrp_node *self, const stbrp_node * v);
void ImVector_stbrp_node__reserve_discard__490e2d0f8c(ImVector_stbrp_node *self, int new_capacity);
void ImVector_stbrp_node__resize__42234e9170(ImVector_stbrp_node *self, int new_size, const stbrp_node * v);
void ImVector_stbrp_node__shrink__7bbe50bab5(ImVector_stbrp_node *self, int new_size);
int ImVector_stbrp_node__size__a34493cdd7(ImVector_stbrp_node *self);
int ImVector_stbrp_node__size_in_bytes__da627bad6c(ImVector_stbrp_node *self);
void ImVector_stbrp_node__swap__e76f6b3d62(ImVector_stbrp_node *self, ImVector_stbrp_node * rhs);
void ImVector_ImDrawList____ImVector__40035d1030(ImVector_ImDrawList_ptr *self);
void ImVector_ImDrawList____ImVector__c7875b66c9(ImVector_ImDrawList_ptr *self, const ImVector_ImDrawList_ptr * src);
int ImVector_ImDrawList_____grow_capacity__440e116c08(ImVector_ImDrawList_ptr *self, int sz);
ImDrawList * * ImVector_ImDrawList____back__3722ae1c59(ImVector_ImDrawList_ptr *self);
ImDrawList *const * ImVector_ImDrawList____back__0ff564eaf0(ImVector_ImDrawList_ptr *self);
ImDrawList ** ImVector_ImDrawList____begin__f18a70ccac(ImVector_ImDrawList_ptr *self);
ImDrawList *const * ImVector_ImDrawList____begin__feb7ba7408(ImVector_ImDrawList_ptr *self);
int ImVector_ImDrawList____capacity__38ef5992d5(ImVector_ImDrawList_ptr *self);
void ImVector_ImDrawList____clear__3b766ce698(ImVector_ImDrawList_ptr *self);
void ImVector_ImDrawList____clear_delete__4ec24b486f(ImVector_ImDrawList_ptr *self);
void ImVector_ImDrawList____clear_destruct__8dc1f9461d(ImVector_ImDrawList_ptr *self);
unsigned char ImVector_ImDrawList____contains__42f71b93f5(ImVector_ImDrawList_ptr *self, ImDrawList *const * v);
ImDrawList ** ImVector_ImDrawList____end__151818158d(ImVector_ImDrawList_ptr *self);
ImDrawList *const * ImVector_ImDrawList____end__3f5aae21cd(ImVector_ImDrawList_ptr *self);
ImDrawList ** ImVector_ImDrawList____erase__968c331fa4(ImVector_ImDrawList_ptr *self, ImDrawList *const * it);
ImDrawList ** ImVector_ImDrawList____erase__b6e6acb9c2(ImVector_ImDrawList_ptr *self, ImDrawList *const * it, ImDrawList *const * it_last);
ImDrawList *const * ImVector_ImDrawList____find__eec51fb74d(ImVector_ImDrawList_ptr *self, ImDrawList *const * v);
unsigned char ImVector_ImDrawList____find_erase__d677b1be1d(ImVector_ImDrawList_ptr *self, ImDrawList *const * v);
int ImVector_ImDrawList____find_index__454440daf7(ImVector_ImDrawList_ptr *self, ImDrawList *const * v);
ImDrawList * * ImVector_ImDrawList____front__5afa6d447d(ImVector_ImDrawList_ptr *self);
ImDrawList *const * ImVector_ImDrawList____front__a790abbab8(ImVector_ImDrawList_ptr *self);
int ImVector_ImDrawList____index_from_ptr__3affcec502(ImVector_ImDrawList_ptr *self, ImDrawList *const * it);
ImDrawList ** ImVector_ImDrawList____insert__ec6675b04a(ImVector_ImDrawList_ptr *self, ImDrawList *const * it, ImDrawList *const * v);
int ImVector_ImDrawList____max_size__e6541c5e4c(ImVector_ImDrawList_ptr *self);
ImVector_ImDrawList_ptr * ImVector_ImDrawList____operator___f5e8f9b766(ImVector_ImDrawList_ptr *self, const ImVector_ImDrawList_ptr * src);
ImDrawList * * ImVector_ImDrawList____operator____35a77fc1fb(ImVector_ImDrawList_ptr *self, int i);
ImDrawList *const * ImVector_ImDrawList____operator____6a5c4a4e8e(ImVector_ImDrawList_ptr *self, int i);
void ImVector_ImDrawList____pop_back__e9b2e5a5e7(ImVector_ImDrawList_ptr *self);
void ImVector_ImDrawList____push_front__b7d2475248(ImVector_ImDrawList_ptr *self, ImDrawList *const * v);
void ImVector_ImDrawList____reserve__5c1d9153a3(ImGuiContext *imgui_c89_ctx, ImVector_ImDrawList_ptr *self, int new_capacity);
void ImVector_ImDrawList____reserve_discard__3c54344d87(ImVector_ImDrawList_ptr *self, int new_capacity);
void ImVector_ImDrawList____resize__7ecac77b13(ImVector_ImDrawList_ptr *self, int new_size, ImDrawList *const * v);
void ImVector_ImDrawList____shrink__b4ac09c2af(ImVector_ImDrawList_ptr *self, int new_size);
int ImVector_ImDrawList____size__12e037c923(ImVector_ImDrawList_ptr *self);
int ImVector_ImDrawList____size_in_bytes__f2e43845ab(ImVector_ImDrawList_ptr *self);
void ImVector_ImDrawList____swap__cab5f0abfd(ImVector_ImDrawList_ptr *self, ImVector_ImDrawList_ptr * rhs);
void ImVector_ImDrawList____dtor_ImVector__6807174401(ImGuiContext *imgui_c89_ctx, ImVector_ImDrawList_ptr *self);
void ImVector_ImDrawListSharedData____ImVector__b5c0ff41f1(ImVector_ImDrawListSharedData_ptr *self, const ImVector_ImDrawListSharedData_ptr * src);
ImDrawListSharedData * * ImVector_ImDrawListSharedData____back__2713766e46(ImVector_ImDrawListSharedData_ptr *self);
ImDrawListSharedData *const * ImVector_ImDrawListSharedData____back__8c51d38372(ImVector_ImDrawListSharedData_ptr *self);
ImDrawListSharedData *const * ImVector_ImDrawListSharedData____begin__423d519575(ImVector_ImDrawListSharedData_ptr *self);
int ImVector_ImDrawListSharedData____capacity__ff5faaa70e(ImVector_ImDrawListSharedData_ptr *self);
void ImVector_ImDrawListSharedData____clear__4cb5b61689(ImVector_ImDrawListSharedData_ptr *self);
void ImVector_ImDrawListSharedData____clear_delete__1f798e73b8(ImVector_ImDrawListSharedData_ptr *self);
void ImVector_ImDrawListSharedData____clear_destruct__48dce52801(ImVector_ImDrawListSharedData_ptr *self);
unsigned char ImVector_ImDrawListSharedData____empty__90ba0a6a56(ImVector_ImDrawListSharedData_ptr *self);
ImDrawListSharedData *const * ImVector_ImDrawListSharedData____end__c9bccfd36a(ImVector_ImDrawListSharedData_ptr *self);
ImDrawListSharedData ** ImVector_ImDrawListSharedData____erase__7b1c53e95b(ImVector_ImDrawListSharedData_ptr *self, ImDrawListSharedData *const * it, ImDrawListSharedData *const * it_last);
ImDrawListSharedData ** ImVector_ImDrawListSharedData____erase_unsorted__5b49fb347b(ImVector_ImDrawListSharedData_ptr *self, ImDrawListSharedData *const * it);
ImDrawListSharedData *const * ImVector_ImDrawListSharedData____find__5d291f611d(ImVector_ImDrawListSharedData_ptr *self, ImDrawListSharedData *const * v);
unsigned char ImVector_ImDrawListSharedData____find_erase_unsorted__b77ad2efe1(ImVector_ImDrawListSharedData_ptr *self, ImDrawListSharedData *const * v);
int ImVector_ImDrawListSharedData____find_index__a79ac564e2(ImVector_ImDrawListSharedData_ptr *self, ImDrawListSharedData *const * v);
ImDrawListSharedData * * ImVector_ImDrawListSharedData____front__31fafe002a(ImVector_ImDrawListSharedData_ptr *self);
ImDrawListSharedData *const * ImVector_ImDrawListSharedData____front__41c5b107f8(ImVector_ImDrawListSharedData_ptr *self);
int ImVector_ImDrawListSharedData____index_from_ptr__b24a43b669(ImVector_ImDrawListSharedData_ptr *self, ImDrawListSharedData *const * it);
ImDrawListSharedData ** ImVector_ImDrawListSharedData____insert__be355253fd(ImVector_ImDrawListSharedData_ptr *self, ImDrawListSharedData *const * it, ImDrawListSharedData *const * v);
int ImVector_ImDrawListSharedData____max_size__1441ea1910(ImVector_ImDrawListSharedData_ptr *self);
ImVector_ImDrawListSharedData_ptr * ImVector_ImDrawListSharedData____operator___b968595764(ImVector_ImDrawListSharedData_ptr *self, const ImVector_ImDrawListSharedData_ptr * src);
ImDrawListSharedData * * ImVector_ImDrawListSharedData____operator____aab904fff9(ImVector_ImDrawListSharedData_ptr *self, int i);
ImDrawListSharedData *const * ImVector_ImDrawListSharedData____operator____2f7c2378ea(ImVector_ImDrawListSharedData_ptr *self, int i);
void ImVector_ImDrawListSharedData____pop_back__bf5fda4ac4(ImVector_ImDrawListSharedData_ptr *self);
void ImVector_ImDrawListSharedData____push_front__d43804afcc(ImVector_ImDrawListSharedData_ptr *self, ImDrawListSharedData *const * v);
void ImVector_ImDrawListSharedData____reserve_discard__89853b24c0(ImVector_ImDrawListSharedData_ptr *self, int new_capacity);
void ImVector_ImDrawListSharedData____resize__ce3fc05281(ImVector_ImDrawListSharedData_ptr *self, int new_size);
void ImVector_ImDrawListSharedData____resize__43a3e39cbc(ImVector_ImDrawListSharedData_ptr *self, int new_size, ImDrawListSharedData *const * v);
void ImVector_ImDrawListSharedData____shrink__180bd4a0a3(ImVector_ImDrawListSharedData_ptr *self, int new_size);
int ImVector_ImDrawListSharedData____size__af12fd9cf2(ImVector_ImDrawListSharedData_ptr *self);
int ImVector_ImDrawListSharedData____size_in_bytes__a0f8ebe477(ImVector_ImDrawListSharedData_ptr *self);
void ImVector_ImDrawListSharedData____swap__fc169da68a(ImVector_ImDrawListSharedData_ptr *self, ImVector_ImDrawListSharedData_ptr * rhs);
void ImVector_ImFont____ImVector__6d26295678(ImVector_ImFont_ptr *self, const ImVector_ImFont_ptr * src);
ImFont *const * ImVector_ImFont____back__c07ef4fd88(ImVector_ImFont_ptr *self);
ImFont ** ImVector_ImFont____begin__24205a3db7(ImVector_ImFont_ptr *self);
ImFont *const * ImVector_ImFont____begin__c577a76169(ImVector_ImFont_ptr *self);
int ImVector_ImFont____capacity__be95ef80fe(ImVector_ImFont_ptr *self);
void ImVector_ImFont____clear_destruct__d3a8fb6437(ImVector_ImFont_ptr *self);
unsigned char ImVector_ImFont____contains__0743a06e92(ImVector_ImFont_ptr *self, ImFont *const * v);
ImFont ** ImVector_ImFont____end__43d2d7f52c(ImVector_ImFont_ptr *self);
ImFont *const * ImVector_ImFont____end__728cdcdf0d(ImVector_ImFont_ptr *self);
ImFont ** ImVector_ImFont____erase__0526f9c065(ImVector_ImFont_ptr *self, ImFont *const * it, ImFont *const * it_last);
ImFont ** ImVector_ImFont____erase_unsorted__afb3ee66f2(ImVector_ImFont_ptr *self, ImFont *const * it);
ImFont *const * ImVector_ImFont____find__356cbfddb8(ImVector_ImFont_ptr *self, ImFont *const * v);
unsigned char ImVector_ImFont____find_erase_unsorted__dabb9149ea(ImVector_ImFont_ptr *self, ImFont *const * v);
int ImVector_ImFont____find_index__84fd8d8971(ImVector_ImFont_ptr *self, ImFont *const * v);
ImFont * * ImVector_ImFont____front__b6f617efe0(ImVector_ImFont_ptr *self);
ImFont *const * ImVector_ImFont____front__9b8897657d(ImVector_ImFont_ptr *self);
int ImVector_ImFont____index_from_ptr__d0e2b6c963(ImVector_ImFont_ptr *self, ImFont *const * it);
ImFont ** ImVector_ImFont____insert__0f2a7776e9(ImVector_ImFont_ptr *self, ImFont *const * it, ImFont *const * v);
int ImVector_ImFont____max_size__06472b44ac(ImVector_ImFont_ptr *self);
ImVector_ImFont_ptr * ImVector_ImFont____operator___1d26cab799(ImVector_ImFont_ptr *self, const ImVector_ImFont_ptr * src);
ImFont * * ImVector_ImFont____operator____0cce6192b3(ImVector_ImFont_ptr *self, int i);
ImFont *const * ImVector_ImFont____operator____392d365733(ImVector_ImFont_ptr *self, int i);
void ImVector_ImFont____push_front__ebe166cdce(ImVector_ImFont_ptr *self, ImFont *const * v);
void ImVector_ImFont____reserve_discard__6ff07747da(ImVector_ImFont_ptr *self, int new_capacity);
void ImVector_ImFont____resize__eda1db7838(ImVector_ImFont_ptr *self, int new_size);
void ImVector_ImFont____resize__e9bc91f897(ImVector_ImFont_ptr *self, int new_size, ImFont *const * v);
void ImVector_ImFont____shrink__b9a3f0ff34(ImVector_ImFont_ptr *self, int new_size);
int ImVector_ImFont____size__2953f4c5d8(ImVector_ImFont_ptr *self);
int ImVector_ImFont____size_in_bytes__5fcbc85f1b(ImVector_ImFont_ptr *self);
void ImVector_ImFont____swap__08c86b9848(ImVector_ImFont_ptr *self, ImVector_ImFont_ptr * rhs);
void ImVector_ImFontAtlas____ImVector__e3f4df322a(ImVector_ImFontAtlas_ptr *self, const ImVector_ImFontAtlas_ptr * src);
ImFontAtlas * * ImVector_ImFontAtlas____back__c84d4e7678(ImVector_ImFontAtlas_ptr *self);
ImFontAtlas *const * ImVector_ImFontAtlas____back__cc0362deba(ImVector_ImFontAtlas_ptr *self);
ImFontAtlas *const * ImVector_ImFontAtlas____begin__f9b9028c2a(ImVector_ImFontAtlas_ptr *self);
int ImVector_ImFontAtlas____capacity__e7815d860a(ImVector_ImFontAtlas_ptr *self);
void ImVector_ImFontAtlas____clear__8c6ecd59b9(ImVector_ImFontAtlas_ptr *self);
void ImVector_ImFontAtlas____clear_delete__65caf6fb00(ImVector_ImFontAtlas_ptr *self);
void ImVector_ImFontAtlas____clear_destruct__b6c0bfafbf(ImVector_ImFontAtlas_ptr *self);
unsigned char ImVector_ImFontAtlas____contains__c765df12cc(ImVector_ImFontAtlas_ptr *self, ImFontAtlas *const * v);
unsigned char ImVector_ImFontAtlas____empty__1ab99d3e25(ImVector_ImFontAtlas_ptr *self);
ImFontAtlas *const * ImVector_ImFontAtlas____end__f0eb6b3666(ImVector_ImFontAtlas_ptr *self);
ImFontAtlas ** ImVector_ImFontAtlas____erase__3f396ce511(ImVector_ImFontAtlas_ptr *self, ImFontAtlas *const * it, ImFontAtlas *const * it_last);
ImFontAtlas ** ImVector_ImFontAtlas____erase_unsorted__7d302bba60(ImVector_ImFontAtlas_ptr *self, ImFontAtlas *const * it);
ImFontAtlas *const * ImVector_ImFontAtlas____find__e6491bef4e(ImVector_ImFontAtlas_ptr *self, ImFontAtlas *const * v);
unsigned char ImVector_ImFontAtlas____find_erase_unsorted__454f5203a3(ImVector_ImFontAtlas_ptr *self, ImFontAtlas *const * v);
int ImVector_ImFontAtlas____find_index__46c34b6e84(ImVector_ImFontAtlas_ptr *self, ImFontAtlas *const * v);
ImFontAtlas * * ImVector_ImFontAtlas____front__9582e8726e(ImVector_ImFontAtlas_ptr *self);
ImFontAtlas *const * ImVector_ImFontAtlas____front__40405a08ee(ImVector_ImFontAtlas_ptr *self);
int ImVector_ImFontAtlas____index_from_ptr__789a4d076d(ImVector_ImFontAtlas_ptr *self, ImFontAtlas *const * it);
ImFontAtlas ** ImVector_ImFontAtlas____insert__a589fd374a(ImVector_ImFontAtlas_ptr *self, ImFontAtlas *const * it, ImFontAtlas *const * v);
int ImVector_ImFontAtlas____max_size__f232e1e53c(ImVector_ImFontAtlas_ptr *self);
ImVector_ImFontAtlas_ptr * ImVector_ImFontAtlas____operator___3da33856eb(ImVector_ImFontAtlas_ptr *self, const ImVector_ImFontAtlas_ptr * src);
ImFontAtlas * * ImVector_ImFontAtlas____operator____1b7a079ccc(ImVector_ImFontAtlas_ptr *self, int i);
ImFontAtlas *const * ImVector_ImFontAtlas____operator____7bc4557d3d(ImVector_ImFontAtlas_ptr *self, int i);
void ImVector_ImFontAtlas____pop_back__842f494331(ImVector_ImFontAtlas_ptr *self);
void ImVector_ImFontAtlas____push_front__94d11a6447(ImVector_ImFontAtlas_ptr *self, ImFontAtlas *const * v);
void ImVector_ImFontAtlas____reserve_discard__e174985e6c(ImVector_ImFontAtlas_ptr *self, int new_capacity);
void ImVector_ImFontAtlas____resize__b2e59f5da6(ImVector_ImFontAtlas_ptr *self, int new_size);
void ImVector_ImFontAtlas____resize__8b25163c39(ImVector_ImFontAtlas_ptr *self, int new_size, ImFontAtlas *const * v);
void ImVector_ImFontAtlas____shrink__0b1894e706(ImVector_ImFontAtlas_ptr *self, int new_size);
int ImVector_ImFontAtlas____size__1f45078780(ImVector_ImFontAtlas_ptr *self);
int ImVector_ImFontAtlas____size_in_bytes__d2c829dfec(ImVector_ImFontAtlas_ptr *self);
void ImVector_ImFontAtlas____swap__0234f58a3a(ImVector_ImFontAtlas_ptr *self, ImVector_ImFontAtlas_ptr * rhs);
void ImVector_ImFontBaked____ImVector__bb8447e05d(ImVector_ImFontBaked_ptr *self, const ImVector_ImFontBaked_ptr * src);
ImFontBaked * * ImVector_ImFontBaked____back__33d7a4507e(ImVector_ImFontBaked_ptr *self);
ImFontBaked *const * ImVector_ImFontBaked____back__1153071475(ImVector_ImFontBaked_ptr *self);
ImFontBaked *const * ImVector_ImFontBaked____begin__f5df246369(ImVector_ImFontBaked_ptr *self);
int ImVector_ImFontBaked____capacity__b8c6c52f0b(ImVector_ImFontBaked_ptr *self);
void ImVector_ImFontBaked____clear__800993242a(ImVector_ImFontBaked_ptr *self);
void ImVector_ImFontBaked____clear_delete__486dd20829(ImVector_ImFontBaked_ptr *self);
void ImVector_ImFontBaked____clear_destruct__26a2ba3240(ImVector_ImFontBaked_ptr *self);
unsigned char ImVector_ImFontBaked____contains__7a29da6948(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * v);
unsigned char ImVector_ImFontBaked____empty__6969392fa5(ImVector_ImFontBaked_ptr *self);
ImFontBaked *const * ImVector_ImFontBaked____end__de8a041af1(ImVector_ImFontBaked_ptr *self);
ImFontBaked ** ImVector_ImFontBaked____erase__5020d4565a(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * it);
ImFontBaked ** ImVector_ImFontBaked____erase__23067cb17b(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * it, ImFontBaked *const * it_last);
ImFontBaked ** ImVector_ImFontBaked____erase_unsorted__379798e3e3(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * it);
ImFontBaked ** ImVector_ImFontBaked____find__6539e5a241(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * v);
ImFontBaked *const * ImVector_ImFontBaked____find__7e048ab8e6(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * v);
unsigned char ImVector_ImFontBaked____find_erase__b622c3f54c(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * v);
unsigned char ImVector_ImFontBaked____find_erase_unsorted__a5a53b8762(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * v);
int ImVector_ImFontBaked____find_index__c455e75408(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * v);
ImFontBaked * * ImVector_ImFontBaked____front__881f5d5420(ImVector_ImFontBaked_ptr *self);
ImFontBaked *const * ImVector_ImFontBaked____front__aef0c9e05c(ImVector_ImFontBaked_ptr *self);
int ImVector_ImFontBaked____index_from_ptr__739f4db292(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * it);
ImFontBaked ** ImVector_ImFontBaked____insert__e23118e55b(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * it, ImFontBaked *const * v);
int ImVector_ImFontBaked____max_size__a26f975e29(ImVector_ImFontBaked_ptr *self);
ImVector_ImFontBaked_ptr * ImVector_ImFontBaked____operator___48eaeca830(ImVector_ImFontBaked_ptr *self, const ImVector_ImFontBaked_ptr * src);
ImFontBaked * * ImVector_ImFontBaked____operator____551016c6b0(ImVector_ImFontBaked_ptr *self, int i);
ImFontBaked *const * ImVector_ImFontBaked____operator____3ec56104fd(ImVector_ImFontBaked_ptr *self, int i);
void ImVector_ImFontBaked____pop_back__8b63e0a066(ImVector_ImFontBaked_ptr *self);
void ImVector_ImFontBaked____push_back__8e28addead(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * v);
void ImVector_ImFontBaked____push_front__68471d35bf(ImVector_ImFontBaked_ptr *self, ImFontBaked *const * v);
void ImVector_ImFontBaked____reserve_discard__596cf646f7(ImVector_ImFontBaked_ptr *self, int new_capacity);
void ImVector_ImFontBaked____resize__e59cb59783(ImVector_ImFontBaked_ptr *self, int new_size, ImFontBaked *const * v);
void ImVector_ImFontBaked____shrink__6609b9ca6d(ImVector_ImFontBaked_ptr *self, int new_size);
int ImVector_ImFontBaked____size__d3ea4c93d9(ImVector_ImFontBaked_ptr *self);
int ImVector_ImFontBaked____size_in_bytes__888d6f3273(ImVector_ImFontBaked_ptr *self);
void ImVector_ImFontBaked____swap__b2627bb6d2(ImVector_ImFontBaked_ptr *self, ImVector_ImFontBaked_ptr * rhs);
void ImVector_ImFontConfig____ImVector__00f7a65b59(ImVector_ImFontConfig_ptr *self, const ImVector_ImFontConfig_ptr * src);
ImFontConfig * * ImVector_ImFontConfig____back__fd5ef6cec6(ImVector_ImFontConfig_ptr *self);
ImFontConfig *const * ImVector_ImFontConfig____back__bf3151250c(ImVector_ImFontConfig_ptr *self);
ImFontConfig *const * ImVector_ImFontConfig____begin__077d67a460(ImVector_ImFontConfig_ptr *self);
int ImVector_ImFontConfig____capacity__4056f77af9(ImVector_ImFontConfig_ptr *self);
void ImVector_ImFontConfig____clear_delete__bc0654c050(ImVector_ImFontConfig_ptr *self);
void ImVector_ImFontConfig____clear_destruct__f7cc427304(ImVector_ImFontConfig_ptr *self);
unsigned char ImVector_ImFontConfig____empty__cbca9fce65(ImVector_ImFontConfig_ptr *self);
ImFontConfig *const * ImVector_ImFontConfig____end__10a23ac6fa(ImVector_ImFontConfig_ptr *self);
ImFontConfig ** ImVector_ImFontConfig____erase__f5c7d79d1d(ImVector_ImFontConfig_ptr *self, ImFontConfig *const * it);
ImFontConfig ** ImVector_ImFontConfig____erase__7cf21a61b1(ImVector_ImFontConfig_ptr *self, ImFontConfig *const * it, ImFontConfig *const * it_last);
ImFontConfig ** ImVector_ImFontConfig____erase_unsorted__b2f0432bd4(ImVector_ImFontConfig_ptr *self, ImFontConfig *const * it);
ImFontConfig ** ImVector_ImFontConfig____find__7e3fe02aa9(ImVector_ImFontConfig_ptr *self, ImFontConfig *const * v);
ImFontConfig *const * ImVector_ImFontConfig____find__851f33a68a(ImVector_ImFontConfig_ptr *self, ImFontConfig *const * v);
unsigned char ImVector_ImFontConfig____find_erase__d09eddb9e6(ImVector_ImFontConfig_ptr *self, ImFontConfig *const * v);
unsigned char ImVector_ImFontConfig____find_erase_unsorted__44901515a7(ImVector_ImFontConfig_ptr *self, ImFontConfig *const * v);
int ImVector_ImFontConfig____find_index__75b16f98fa(ImVector_ImFontConfig_ptr *self, ImFontConfig *const * v);
ImFontConfig * * ImVector_ImFontConfig____front__a46504a510(ImVector_ImFontConfig_ptr *self);
ImFontConfig *const * ImVector_ImFontConfig____front__6f8c957b19(ImVector_ImFontConfig_ptr *self);
int ImVector_ImFontConfig____index_from_ptr__bc81b85e2e(ImVector_ImFontConfig_ptr *self, ImFontConfig *const * it);
ImFontConfig ** ImVector_ImFontConfig____insert__3e223d9ff5(ImVector_ImFontConfig_ptr *self, ImFontConfig *const * it, ImFontConfig *const * v);
int ImVector_ImFontConfig____max_size__e2f29bf3cc(ImVector_ImFontConfig_ptr *self);
ImVector_ImFontConfig_ptr * ImVector_ImFontConfig____operator___cbf1729b1e(ImVector_ImFontConfig_ptr *self, const ImVector_ImFontConfig_ptr * src);
ImFontConfig * * ImVector_ImFontConfig____operator____f864e92bd4(ImVector_ImFontConfig_ptr *self, int i);
void ImVector_ImFontConfig____push_front__086606559f(ImVector_ImFontConfig_ptr *self, ImFontConfig *const * v);
void ImVector_ImFontConfig____reserve_discard__2b965afb84(ImVector_ImFontConfig_ptr *self, int new_capacity);
void ImVector_ImFontConfig____resize__97a589e95b(ImVector_ImFontConfig_ptr *self, int new_size, ImFontConfig *const * v);
void ImVector_ImFontConfig____shrink__51a4537b97(ImVector_ImFontConfig_ptr *self, int new_size);
int ImVector_ImFontConfig____size__2ec8b901a3(ImVector_ImFontConfig_ptr *self);
int ImVector_ImFontConfig____size_in_bytes__16a6fcad93(ImVector_ImFontConfig_ptr *self);
void ImVector_ImFontConfig____swap__815aa632a3(ImVector_ImFontConfig_ptr *self, ImVector_ImFontConfig_ptr * rhs);
void ImVector_ImGuiViewportP____ImVector__c15eb9a4ed(ImVector_ImGuiViewportP_ptr *self, const ImVector_ImGuiViewportP_ptr * src);
ImGuiViewportP * * ImVector_ImGuiViewportP____back__443d272ffd(ImVector_ImGuiViewportP_ptr *self);
ImGuiViewportP *const * ImVector_ImGuiViewportP____back__2cdd219c1b(ImVector_ImGuiViewportP_ptr *self);
ImGuiViewportP *const * ImVector_ImGuiViewportP____begin__9ba458aa3d(ImVector_ImGuiViewportP_ptr *self);
int ImVector_ImGuiViewportP____capacity__01d4b3f8cc(ImVector_ImGuiViewportP_ptr *self);
void ImVector_ImGuiViewportP____clear_destruct__90f929905b(ImVector_ImGuiViewportP_ptr *self);
unsigned char ImVector_ImGuiViewportP____contains__297773e125(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * v);
unsigned char ImVector_ImGuiViewportP____empty__8d433c7874(ImVector_ImGuiViewportP_ptr *self);
ImGuiViewportP *const * ImVector_ImGuiViewportP____end__9d5a819b96(ImVector_ImGuiViewportP_ptr *self);
ImGuiViewportP ** ImVector_ImGuiViewportP____erase__ee04975cb7(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * it);
ImGuiViewportP ** ImVector_ImGuiViewportP____erase__c496c9bcaf(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * it, ImGuiViewportP *const * it_last);
ImGuiViewportP ** ImVector_ImGuiViewportP____erase_unsorted__d90ba12818(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * it);
ImGuiViewportP ** ImVector_ImGuiViewportP____find__b25ab1f64e(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * v);
ImGuiViewportP *const * ImVector_ImGuiViewportP____find__425e10781c(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * v);
unsigned char ImVector_ImGuiViewportP____find_erase__6deb1e62bc(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * v);
unsigned char ImVector_ImGuiViewportP____find_erase_unsorted__ce4d50cd89(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * v);
int ImVector_ImGuiViewportP____find_index__3490ddd7c5(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * v);
ImGuiViewportP * * ImVector_ImGuiViewportP____front__d7a2116b7e(ImVector_ImGuiViewportP_ptr *self);
ImGuiViewportP *const * ImVector_ImGuiViewportP____front__b05d22803c(ImVector_ImGuiViewportP_ptr *self);
int ImVector_ImGuiViewportP____index_from_ptr__a10576875a(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * it);
ImGuiViewportP ** ImVector_ImGuiViewportP____insert__8c71b4b3aa(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * it, ImGuiViewportP *const * v);
int ImVector_ImGuiViewportP____max_size__75ec50d10a(ImVector_ImGuiViewportP_ptr *self);
ImVector_ImGuiViewportP_ptr * ImVector_ImGuiViewportP____operator___d8cbe39ce6(ImVector_ImGuiViewportP_ptr *self, const ImVector_ImGuiViewportP_ptr * src);
ImGuiViewportP *const * ImVector_ImGuiViewportP____operator____22e8607eba(ImVector_ImGuiViewportP_ptr *self, int i);
void ImVector_ImGuiViewportP____pop_back__c675dd4404(ImVector_ImGuiViewportP_ptr *self);
void ImVector_ImGuiViewportP____push_front__f7ff4216ba(ImVector_ImGuiViewportP_ptr *self, ImGuiViewportP *const * v);
void ImVector_ImGuiViewportP____reserve_discard__cd9f58417d(ImVector_ImGuiViewportP_ptr *self, int new_capacity);
void ImVector_ImGuiViewportP____resize__2e4d7beec1(ImVector_ImGuiViewportP_ptr *self, int new_size);
void ImVector_ImGuiViewportP____resize__e2fb87882d(ImVector_ImGuiViewportP_ptr *self, int new_size, ImGuiViewportP *const * v);
void ImVector_ImGuiViewportP____shrink__69505d284f(ImVector_ImGuiViewportP_ptr *self, int new_size);
int ImVector_ImGuiViewportP____size__d04950f012(ImVector_ImGuiViewportP_ptr *self);
int ImVector_ImGuiViewportP____size_in_bytes__d009bbfaac(ImVector_ImGuiViewportP_ptr *self);
void ImVector_ImGuiViewportP____swap__0608708e99(ImVector_ImGuiViewportP_ptr *self, ImVector_ImGuiViewportP_ptr * rhs);
void ImVector_ImGuiWindow____ImVector__c0ee416cea(ImVector_ImGuiWindow_ptr *self, const ImVector_ImGuiWindow_ptr * src);
ImGuiWindow *const * ImVector_ImGuiWindow____back__0172f90bcb(ImVector_ImGuiWindow_ptr *self);
ImGuiWindow *const * ImVector_ImGuiWindow____begin__50ea862f3d(ImVector_ImGuiWindow_ptr *self);
int ImVector_ImGuiWindow____capacity__02d122a0a2(ImVector_ImGuiWindow_ptr *self);
void ImVector_ImGuiWindow____clear_destruct__22a6032e18(ImVector_ImGuiWindow_ptr *self);
unsigned char ImVector_ImGuiWindow____empty__da2663637a(ImVector_ImGuiWindow_ptr *self);
ImGuiWindow *const * ImVector_ImGuiWindow____end__97569de3a9(ImVector_ImGuiWindow_ptr *self);
ImGuiWindow ** ImVector_ImGuiWindow____erase__08a9a949fe(ImVector_ImGuiWindow_ptr *self, ImGuiWindow *const * it, ImGuiWindow *const * it_last);
ImGuiWindow ** ImVector_ImGuiWindow____erase_unsorted__6d6b7f316f(ImVector_ImGuiWindow_ptr *self, ImGuiWindow *const * it);
ImGuiWindow *const * ImVector_ImGuiWindow____find__b46d739eed(ImVector_ImGuiWindow_ptr *self, ImGuiWindow *const * v);
unsigned char ImVector_ImGuiWindow____find_erase__ebceaede26(ImVector_ImGuiWindow_ptr *self, ImGuiWindow *const * v);
unsigned char ImVector_ImGuiWindow____find_erase_unsorted__d3b80efb4a(ImVector_ImGuiWindow_ptr *self, ImGuiWindow *const * v);
int ImVector_ImGuiWindow____find_index__efc5df4683(ImVector_ImGuiWindow_ptr *self, ImGuiWindow *const * v);
ImGuiWindow * * ImVector_ImGuiWindow____front__60165db653(ImVector_ImGuiWindow_ptr *self);
ImGuiWindow *const * ImVector_ImGuiWindow____front__34b7f9919f(ImVector_ImGuiWindow_ptr *self);
int ImVector_ImGuiWindow____max_size__d5ac54c352(ImVector_ImGuiWindow_ptr *self);
ImVector_ImGuiWindow_ptr * ImVector_ImGuiWindow____operator___3d592217ae(ImVector_ImGuiWindow_ptr *self, const ImVector_ImGuiWindow_ptr * src);
ImGuiWindow *const * ImVector_ImGuiWindow____operator____1b32902ebe(ImVector_ImGuiWindow_ptr *self, int i);
void ImVector_ImGuiWindow____pop_back__82dc549bd8(ImVector_ImGuiWindow_ptr *self);
void ImVector_ImGuiWindow____reserve_discard__ac032e8d2a(ImVector_ImGuiWindow_ptr *self, int new_capacity);
void ImVector_ImGuiWindow____resize__707024d081(ImVector_ImGuiWindow_ptr *self, int new_size, ImGuiWindow *const * v);
void ImVector_ImGuiWindow____shrink__593fe498ac(ImVector_ImGuiWindow_ptr *self, int new_size);
int ImVector_ImGuiWindow____size__2d7afe939b(ImVector_ImGuiWindow_ptr *self);
int ImVector_ImGuiWindow____size_in_bytes__98afd45119(ImVector_ImGuiWindow_ptr *self);
void ImVector_ImTextureData____ImVector__ad4479a99e(ImVector_ImTextureData_ptr *self);
void ImVector_ImTextureData____ImVector__d2c6906057(ImVector_ImTextureData_ptr *self, const ImVector_ImTextureData_ptr * src);
ImTextureData * * ImVector_ImTextureData____back__2deba8eb32(ImVector_ImTextureData_ptr *self);
ImTextureData *const * ImVector_ImTextureData____back__735970ceeb(ImVector_ImTextureData_ptr *self);
ImTextureData ** ImVector_ImTextureData____begin__5f590fe7fe(ImVector_ImTextureData_ptr *self);
ImTextureData *const * ImVector_ImTextureData____begin__6b9e054c88(ImVector_ImTextureData_ptr *self);
int ImVector_ImTextureData____capacity__e9cacd69ba(ImVector_ImTextureData_ptr *self);
void ImVector_ImTextureData____clear_destruct__78721a0cb2(ImVector_ImTextureData_ptr *self);
unsigned char ImVector_ImTextureData____contains__aa0cb5ba70(ImVector_ImTextureData_ptr *self, ImTextureData *const * v);
unsigned char ImVector_ImTextureData____empty__b7ecd70fea(ImVector_ImTextureData_ptr *self);
ImTextureData ** ImVector_ImTextureData____end__3efe30f1db(ImVector_ImTextureData_ptr *self);
ImTextureData *const * ImVector_ImTextureData____end__b80c05188b(ImVector_ImTextureData_ptr *self);
ImTextureData ** ImVector_ImTextureData____erase__c5406a2ec3(ImVector_ImTextureData_ptr *self, ImTextureData *const * it, ImTextureData *const * it_last);
ImTextureData ** ImVector_ImTextureData____erase_unsorted__a6adc96b22(ImVector_ImTextureData_ptr *self, ImTextureData *const * it);
ImTextureData *const * ImVector_ImTextureData____find__a53494c743(ImVector_ImTextureData_ptr *self, ImTextureData *const * v);
unsigned char ImVector_ImTextureData____find_erase_unsorted__af125a2176(ImVector_ImTextureData_ptr *self, ImTextureData *const * v);
int ImVector_ImTextureData____find_index__8705e57941(ImVector_ImTextureData_ptr *self, ImTextureData *const * v);
ImTextureData * * ImVector_ImTextureData____front__57f8d09259(ImVector_ImTextureData_ptr *self);
ImTextureData *const * ImVector_ImTextureData____front__47dad0b552(ImVector_ImTextureData_ptr *self);
int ImVector_ImTextureData____index_from_ptr__372282ad3e(ImVector_ImTextureData_ptr *self, ImTextureData *const * it);
ImTextureData ** ImVector_ImTextureData____insert__8cbcab82c9(ImVector_ImTextureData_ptr *self, ImTextureData *const * it, ImTextureData *const * v);
int ImVector_ImTextureData____max_size__f4b2c2cf87(ImVector_ImTextureData_ptr *self);
ImVector_ImTextureData_ptr * ImVector_ImTextureData____operator___c2cbc35f46(ImVector_ImTextureData_ptr *self, const ImVector_ImTextureData_ptr * src);
ImTextureData *const * ImVector_ImTextureData____operator____2b867fb92b(ImVector_ImTextureData_ptr *self, int i);
void ImVector_ImTextureData____pop_back__9d3fad9f63(ImVector_ImTextureData_ptr *self);
void ImVector_ImTextureData____push_back__dff6905d74(ImGuiContext *imgui_c89_ctx, ImVector_ImTextureData_ptr *self, ImTextureData *const * v);
void ImVector_ImTextureData____push_front__aa529feeda(ImVector_ImTextureData_ptr *self, ImTextureData *const * v);
void ImVector_ImTextureData____reserve_discard__56fb2875cb(ImVector_ImTextureData_ptr *self, int new_capacity);
void ImVector_ImTextureData____resize__89f2b5bc3d(ImVector_ImTextureData_ptr *self, int new_size, ImTextureData *const * v);
void ImVector_ImTextureData____shrink__5b90b78d31(ImVector_ImTextureData_ptr *self, int new_size);
int ImVector_ImTextureData____size__18e4379f33(ImVector_ImTextureData_ptr *self);
int ImVector_ImTextureData____size_in_bytes__bc345d61c3(ImVector_ImTextureData_ptr *self);
void ImVector_ImTextureData____swap__d26e89e105(ImVector_ImTextureData_ptr *self, ImVector_ImTextureData_ptr * rhs);
void ImVector_ImTextureData____dtor_ImVector__a5e3735b22(ImGuiContext *imgui_c89_ctx, ImVector_ImTextureData_ptr *self);
void ImVector_char__ImVector__5f659794af(ImVector_char *self);
void ImVector_char__ImVector__1705ba97ea(ImVector_char *self, const ImVector_char * src);
char * ImVector_char__back__3f3488ab69(ImVector_char *self);
const char * ImVector_char__begin__7f3e675ba6(ImVector_char *self);
int ImVector_char__capacity__e866f0aa82(ImVector_char *self);
void ImVector_char__clear__8c1dcc5c15(ImGuiContext *imgui_c89_ctx, ImVector_char *self);
void ImVector_char__clear_delete__c6122f3c81(ImVector_char *self);
void ImVector_char__clear_destruct__1a44e04721(ImVector_char *self);
unsigned char ImVector_char__contains__0fe29997eb(ImVector_char *self, const char * v);
char * ImVector_char__end__b7b7b665d5(ImVector_char *self);
const char * ImVector_char__end__5689c745d3(ImVector_char *self);
char * ImVector_char__erase__e3b76c5079(ImVector_char *self, const char * it);
char * ImVector_char__erase__eabb0b0504(ImVector_char *self, const char * it, const char * it_last);
char * ImVector_char__erase_unsorted__35871d439f(ImVector_char *self, const char * it);
char * ImVector_char__find__543fa547ea(ImVector_char *self, const char * v);
const char * ImVector_char__find__c21020baea(ImVector_char *self, const char * v);
unsigned char ImVector_char__find_erase__5921fa8fed(ImVector_char *self, const char * v);
unsigned char ImVector_char__find_erase_unsorted__e05630070d(ImVector_char *self, const char * v);
int ImVector_char__find_index__4cbc2e8204(ImVector_char *self, const char * v);
char * ImVector_char__front__7eff4f2cc6(ImVector_char *self);
int ImVector_char__index_from_ptr__6a8f91d03e(ImVector_char *self, const char * it);
char * ImVector_char__insert__bd194adb47(ImVector_char *self, const char * it, const char * v);
int ImVector_char__max_size__41e7c831d1(ImVector_char *self);
ImVector_char * ImVector_char__operator___3b3c33de1a(ImVector_char *self, const ImVector_char * src);
char * ImVector_char__operator____80421f14cc(ImVector_char *self, int i);
void ImVector_char__pop_back__5e9d3268b5(ImVector_char *self);
void ImVector_char__push_back__6223be1825(ImGuiContext *imgui_c89_ctx, ImVector_char *self, const char * v);
void ImVector_char__push_front__e838794784(ImVector_char *self, const char * v);
void ImVector_char__reserve__a13dbd8026(ImGuiContext *imgui_c89_ctx, ImVector_char *self, int new_capacity);
void ImVector_char__reserve_discard__453d3b035f(ImVector_char *self, int new_capacity);
void ImVector_char__resize__4f5f852684(ImGuiContext *imgui_c89_ctx, ImVector_char *self, int new_size);
void ImVector_char__shrink__7c67138e91(ImVector_char *self, int new_size);
int ImVector_char__size__2dd3e07713(ImVector_char *self);
int ImVector_char__size_in_bytes__934a00d511(ImVector_char *self);
void ImVector_char__dtor_ImVector__0fbf250d7d(ImGuiContext *imgui_c89_ctx, ImVector_char *self);
void ImVector_int__ImVector__07a8c889dc(ImVector_int *self, const ImVector_int * src);
const int * ImVector_int__back__4bdcb6a322(ImVector_int *self);
const int * ImVector_int__begin__221781fa0a(ImVector_int *self);
int ImVector_int__capacity__a458602ce3(ImVector_int *self);
void ImVector_int__clear_delete__629d9feefc(ImVector_int *self);
void ImVector_int__clear_destruct__e4e1729d94(ImVector_int *self);
unsigned char ImVector_int__contains__5afe72f1d1(ImVector_int *self, const int * v);
unsigned char ImVector_int__empty__9f8fa380b7(ImVector_int *self);
const int * ImVector_int__end__523e004d49(ImVector_int *self);
int * ImVector_int__erase__215c8921a4(ImVector_int *self, const int * it);
int * ImVector_int__erase__a9bdac69d6(ImVector_int *self, const int * it, const int * it_last);
int * ImVector_int__erase_unsorted__17c27d24c8(ImVector_int *self, const int * it);
int * ImVector_int__find__32190a0a20(ImVector_int *self, const int * v);
const int * ImVector_int__find__0ea1e995fa(ImVector_int *self, const int * v);
unsigned char ImVector_int__find_erase__4255db7fba(ImVector_int *self, const int * v);
unsigned char ImVector_int__find_erase_unsorted__2a709800d4(ImVector_int *self, const int * v);
int ImVector_int__find_index__307889f390(ImVector_int *self, const int * v);
int * ImVector_int__front__b8cc99459b(ImVector_int *self);
const int * ImVector_int__front__9eb17c61f5(ImVector_int *self);
int * ImVector_int__insert__df6cdf12be(ImVector_int *self, const int * it, const int * v);
int ImVector_int__max_size__6c852f8236(ImVector_int *self);
ImVector_int * ImVector_int__operator___fe0de7b4e5(ImVector_int *self, const ImVector_int * src);
const int * ImVector_int__operator____7dbb786810(ImVector_int *self, int i);
void ImVector_int__push_back__c45e525747(ImGuiContext *imgui_c89_ctx, ImVector_int *self, const int * v);
void ImVector_int__push_front__6cbcbab12a(ImVector_int *self, const int * v);
void ImVector_int__reserve_discard__51f12267b2(ImVector_int *self, int new_capacity);
void ImVector_int__resize__77e2f5dbcd(ImGuiContext *imgui_c89_ctx, ImVector_int *self, int new_size);
void ImVector_int__resize__d5b076ab00(ImVector_int *self, int new_size, const int * v);
void ImVector_int__shrink__c5a81dbc3c(ImVector_int *self, int new_size);
int ImVector_int__size__d816cd7596(ImVector_int *self);
int ImVector_int__size_in_bytes__f80d5486d6(ImVector_int *self);
void ImVector_int__swap__2e49b4d4e3(ImVector_int *self, ImVector_int * rhs);
void ImVector_unsigned_char__ImVector__dd9c86aa77(ImVector_unsigned_char *self);
void ImVector_unsigned_char__ImVector__22fa741b2c(ImVector_unsigned_char *self, const ImVector_unsigned_char * src);
unsigned char * ImVector_unsigned_char__back__480c2b83ca(ImVector_unsigned_char *self);
const unsigned char * ImVector_unsigned_char__back__ec628cdcfd(ImVector_unsigned_char *self);
unsigned char * ImVector_unsigned_char__begin__0879ce061d(ImVector_unsigned_char *self);
const unsigned char * ImVector_unsigned_char__begin__f127729dbb(ImVector_unsigned_char *self);
int ImVector_unsigned_char__capacity__2e10ccbdf4(ImVector_unsigned_char *self);
void ImVector_unsigned_char__clear__4b7f445ce6(ImGuiContext *imgui_c89_ctx, ImVector_unsigned_char *self);
void ImVector_unsigned_char__clear_delete__23a333322d(ImVector_unsigned_char *self);
void ImVector_unsigned_char__clear_destruct__f240401dad(ImVector_unsigned_char *self);
unsigned char ImVector_unsigned_char__contains__b5bf8482ec(ImVector_unsigned_char *self, const unsigned char * v);
unsigned char ImVector_unsigned_char__empty__ec7482b785(ImVector_unsigned_char *self);
unsigned char * ImVector_unsigned_char__end__154c56cfc2(ImVector_unsigned_char *self);
const unsigned char * ImVector_unsigned_char__end__4761eaf252(ImVector_unsigned_char *self);
unsigned char * ImVector_unsigned_char__erase__21099d604f(ImVector_unsigned_char *self, const unsigned char * it);
unsigned char * ImVector_unsigned_char__erase__2910c8490c(ImVector_unsigned_char *self, const unsigned char * it, const unsigned char * it_last);
unsigned char * ImVector_unsigned_char__erase_unsorted__aa04f1fc5c(ImVector_unsigned_char *self, const unsigned char * it);
unsigned char * ImVector_unsigned_char__find__fffff1269a(ImVector_unsigned_char *self, const unsigned char * v);
const unsigned char * ImVector_unsigned_char__find__b4bb5de16f(ImVector_unsigned_char *self, const unsigned char * v);
unsigned char ImVector_unsigned_char__find_erase__a7572eeaee(ImVector_unsigned_char *self, const unsigned char * v);
unsigned char ImVector_unsigned_char__find_erase_unsorted__90473da428(ImVector_unsigned_char *self, const unsigned char * v);
int ImVector_unsigned_char__find_index__c56165eb8a(ImVector_unsigned_char *self, const unsigned char * v);
unsigned char * ImVector_unsigned_char__front__c4b4a02844(ImVector_unsigned_char *self);
const unsigned char * ImVector_unsigned_char__front__00c1393822(ImVector_unsigned_char *self);
int ImVector_unsigned_char__index_from_ptr__c9e7160942(ImVector_unsigned_char *self, const unsigned char * it);
unsigned char * ImVector_unsigned_char__insert__de522602c3(ImVector_unsigned_char *self, const unsigned char * it, const unsigned char * v);
int ImVector_unsigned_char__max_size__62aa784016(ImVector_unsigned_char *self);
ImVector_unsigned_char * ImVector_unsigned_char__operator___d263dc8178(ImVector_unsigned_char *self, const ImVector_unsigned_char * src);
unsigned char * ImVector_unsigned_char__operator____87c64dde59(ImVector_unsigned_char *self, int i);
const unsigned char * ImVector_unsigned_char__operator____a41b3dfd74(ImVector_unsigned_char *self, int i);
void ImVector_unsigned_char__pop_back__34fd9b6a74(ImVector_unsigned_char *self);
void ImVector_unsigned_char__push_back__e4735b3318(ImVector_unsigned_char *self, const unsigned char * v);
void ImVector_unsigned_char__push_front__0fb24bbc1a(ImVector_unsigned_char *self, const unsigned char * v);
void ImVector_unsigned_char__reserve_discard__9f109191e5(ImVector_unsigned_char *self, int new_capacity);
void ImVector_unsigned_char__resize__c4c804ad65(ImGuiContext *imgui_c89_ctx, ImVector_unsigned_char *self, int new_size);
void ImVector_unsigned_char__resize__ed9106829a(ImVector_unsigned_char *self, int new_size, const unsigned char * v);
void ImVector_unsigned_char__shrink__e112f8a258(ImVector_unsigned_char *self, int new_size);
int ImVector_unsigned_char__size__8c7fe1970d(ImVector_unsigned_char *self);
int ImVector_unsigned_char__size_in_bytes__b548a61f6f(ImVector_unsigned_char *self);
void ImVector_unsigned_char__swap__bf6cecb2dd(ImVector_unsigned_char *self, ImVector_unsigned_char * rhs);
void ImVector_unsigned_char__dtor_ImVector__f47bcae332(ImGuiContext *imgui_c89_ctx, ImVector_unsigned_char *self);
void ImVector_float__ImVector__1f8a67dfe8(ImVector_float *self);
void ImVector_float__ImVector__31e67496e7(ImVector_float *self, const ImVector_float * src);
int ImVector_float___grow_capacity__9cb4f4d34b(ImVector_float *self, int sz);
const float * ImVector_float__back__e79b233ff6(ImVector_float *self);
float * ImVector_float__begin__945b5663d3(ImVector_float *self);
const float * ImVector_float__begin__527ff50a1a(ImVector_float *self);
int ImVector_float__capacity__043eec0cb5(ImVector_float *self);
void ImVector_float__clear__da202e242b(ImGuiContext *imgui_c89_ctx, ImVector_float *self);
void ImVector_float__clear_delete__93c99795e6(ImVector_float *self);
void ImVector_float__clear_destruct__269dbe3a76(ImVector_float *self);
unsigned char ImVector_float__contains__81f58e903b(ImVector_float *self, const float * v);
unsigned char ImVector_float__empty__ed1d7c4ec1(ImVector_float *self);
float * ImVector_float__end__b067a49d63(ImVector_float *self);
const float * ImVector_float__end__da7489b6b4(ImVector_float *self);
float * ImVector_float__erase__7d7d277e64(ImVector_float *self, const float * it);
float * ImVector_float__erase__15fc7ca3d1(ImVector_float *self, const float * it, const float * it_last);
float * ImVector_float__erase_unsorted__f21bdbc8d5(ImVector_float *self, const float * it);
float * ImVector_float__find__cec7839fdc(ImVector_float *self, const float * v);
const float * ImVector_float__find__a2c1e4bfbf(ImVector_float *self, const float * v);
unsigned char ImVector_float__find_erase__ba2cc81171(ImVector_float *self, const float * v);
unsigned char ImVector_float__find_erase_unsorted__9aabfff095(ImVector_float *self, const float * v);
int ImVector_float__find_index__3e639e3997(ImVector_float *self, const float * v);
float * ImVector_float__front__b2377c57f7(ImVector_float *self);
const float * ImVector_float__front__065c7e68d1(ImVector_float *self);
int ImVector_float__index_from_ptr__8028ec2aef(ImVector_float *self, const float * it);
float * ImVector_float__insert__c6b5f51a84(ImVector_float *self, const float * it, const float * v);
int ImVector_float__max_size__716589b35b(ImVector_float *self);
ImVector_float * ImVector_float__operator___71520c8058(ImVector_float *self, const ImVector_float * src);
float * ImVector_float__operator____739fa74fbe(ImVector_float *self, int i);
const float * ImVector_float__operator____870377b320(ImVector_float *self, int i);
void ImVector_float__push_front__bc84ef0a11(ImVector_float *self, const float * v);
void ImVector_float__reserve__478f9a71cf(ImGuiContext *imgui_c89_ctx, ImVector_float *self, int new_capacity);
void ImVector_float__reserve_discard__1ea875d6c7(ImVector_float *self, int new_capacity);
void ImVector_float__shrink__10e202c0ae(ImVector_float *self, int new_size);
int ImVector_float__size__cfb3dce7c2(ImVector_float *self);
int ImVector_float__size_in_bytes__bcad9d9bf8(ImVector_float *self);
void ImVector_unsigned_int__ImVector__94d0c9f871(ImVector_unsigned_int *self, const ImVector_unsigned_int * src);
unsigned int * ImVector_unsigned_int__back__2320fb1e8e(ImVector_unsigned_int *self);
const unsigned int * ImVector_unsigned_int__back__705558b093(ImVector_unsigned_int *self);
unsigned int * ImVector_unsigned_int__begin__d02a60095f(ImVector_unsigned_int *self);
const unsigned int * ImVector_unsigned_int__begin__16762edad0(ImVector_unsigned_int *self);
int ImVector_unsigned_int__capacity__56b7b7069e(ImVector_unsigned_int *self);
void ImVector_unsigned_int__clear_delete__f7f3b10511(ImVector_unsigned_int *self);
void ImVector_unsigned_int__clear_destruct__d9d98db80f(ImVector_unsigned_int *self);
unsigned char ImVector_unsigned_int__empty__3324fed6fa(ImVector_unsigned_int *self);
unsigned int * ImVector_unsigned_int__end__6adf2cd56c(ImVector_unsigned_int *self);
const unsigned int * ImVector_unsigned_int__end__06de522790(ImVector_unsigned_int *self);
unsigned int * ImVector_unsigned_int__erase__9e32c0ef24(ImVector_unsigned_int *self, const unsigned int * it);
unsigned int * ImVector_unsigned_int__erase__f5dd055dba(ImVector_unsigned_int *self, const unsigned int * it, const unsigned int * it_last);
unsigned int * ImVector_unsigned_int__erase_unsorted__307740819d(ImVector_unsigned_int *self, const unsigned int * it);
unsigned int * ImVector_unsigned_int__find__09d8b8090b(ImVector_unsigned_int *self, const unsigned int * v);
const unsigned int * ImVector_unsigned_int__find__1b22994057(ImVector_unsigned_int *self, const unsigned int * v);
unsigned char ImVector_unsigned_int__find_erase__99c64c8ba6(ImVector_unsigned_int *self, const unsigned int * v);
unsigned char ImVector_unsigned_int__find_erase_unsorted__592d93e57a(ImVector_unsigned_int *self, const unsigned int * v);
int ImVector_unsigned_int__find_index__f0365342df(ImVector_unsigned_int *self, const unsigned int * v);
unsigned int * ImVector_unsigned_int__front__868d6ee1ac(ImVector_unsigned_int *self);
const unsigned int * ImVector_unsigned_int__front__f41f9f273f(ImVector_unsigned_int *self);
int ImVector_unsigned_int__index_from_ptr__f74540f0d7(ImVector_unsigned_int *self, const unsigned int * it);
unsigned int * ImVector_unsigned_int__insert__f398383b64(ImVector_unsigned_int *self, const unsigned int * it, const unsigned int * v);
int ImVector_unsigned_int__max_size__d2f66630c8(ImVector_unsigned_int *self);
ImVector_unsigned_int * ImVector_unsigned_int__operator___22ee17f9d1(ImVector_unsigned_int *self, const ImVector_unsigned_int * src);
void ImVector_unsigned_int__push_back__c86a2609cc(ImGuiContext *imgui_c89_ctx, ImVector_unsigned_int *self, const unsigned int * v);
void ImVector_unsigned_int__push_front__023ff841a5(ImVector_unsigned_int *self, const unsigned int * v);
void ImVector_unsigned_int__reserve_discard__b7b6e24be3(ImVector_unsigned_int *self, int new_capacity);
void ImVector_unsigned_int__resize__b084819988(ImVector_unsigned_int *self, int new_size, const unsigned int * v);
void ImVector_unsigned_int__shrink__ac56cc0dc8(ImVector_unsigned_int *self, int new_size);
int ImVector_unsigned_int__size__085e251047(ImVector_unsigned_int *self);
int ImVector_unsigned_int__size_in_bytes__a814a83d8e(ImVector_unsigned_int *self);
void ImVector_unsigned_int__swap__fa996d3e87(ImVector_unsigned_int *self, ImVector_unsigned_int * rhs);
void ImVector_unsigned_short__ImVector__cdfb5f8d17(ImVector_unsigned_short *self);
void ImVector_unsigned_short__ImVector__d7fbf88da1(ImVector_unsigned_short *self, const ImVector_unsigned_short * src);
int ImVector_unsigned_short___grow_capacity__46e61881e0(ImVector_unsigned_short *self, int sz);
unsigned short * ImVector_unsigned_short__back__c948fc4d64(ImVector_unsigned_short *self);
const unsigned short * ImVector_unsigned_short__back__6e6448f151(ImVector_unsigned_short *self);
const unsigned short * ImVector_unsigned_short__begin__875535c162(ImVector_unsigned_short *self);
int ImVector_unsigned_short__capacity__388bc028ea(ImVector_unsigned_short *self);
void ImVector_unsigned_short__clear_delete__2f178842ad(ImVector_unsigned_short *self);
void ImVector_unsigned_short__clear_destruct__a068f20813(ImVector_unsigned_short *self);
unsigned char ImVector_unsigned_short__contains__7dba1e1954(ImVector_unsigned_short *self, const unsigned short * v);
const unsigned short * ImVector_unsigned_short__end__607ce415a3(ImVector_unsigned_short *self);
unsigned short * ImVector_unsigned_short__erase__62aa4331db(ImVector_unsigned_short *self, const unsigned short * it);
unsigned short * ImVector_unsigned_short__erase__b06afb218d(ImVector_unsigned_short *self, const unsigned short * it, const unsigned short * it_last);
unsigned short * ImVector_unsigned_short__erase_unsorted__54a78abdd9(ImVector_unsigned_short *self, const unsigned short * it);
unsigned short * ImVector_unsigned_short__find__68f1719214(ImVector_unsigned_short *self, const unsigned short * v);
const unsigned short * ImVector_unsigned_short__find__05d9108e95(ImVector_unsigned_short *self, const unsigned short * v);
unsigned char ImVector_unsigned_short__find_erase__6fd030ab20(ImVector_unsigned_short *self, const unsigned short * v);
unsigned char ImVector_unsigned_short__find_erase_unsorted__ec1adc7a07(ImVector_unsigned_short *self, const unsigned short * v);
int ImVector_unsigned_short__find_index__24500f08dc(ImVector_unsigned_short *self, const unsigned short * v);
unsigned short * ImVector_unsigned_short__front__effc617d2d(ImVector_unsigned_short *self);
const unsigned short * ImVector_unsigned_short__front__ef2e9e6851(ImVector_unsigned_short *self);
int ImVector_unsigned_short__index_from_ptr__b3009c9588(ImVector_unsigned_short *self, const unsigned short * it);
unsigned short * ImVector_unsigned_short__insert__0a57806631(ImVector_unsigned_short *self, const unsigned short * it, const unsigned short * v);
int ImVector_unsigned_short__max_size__772e821520(ImVector_unsigned_short *self);
unsigned short * ImVector_unsigned_short__operator____6927bb710f(ImVector_unsigned_short *self, int i);
const unsigned short * ImVector_unsigned_short__operator____db4d4f987f(ImVector_unsigned_short *self, int i);
void ImVector_unsigned_short__pop_back__78a1021945(ImVector_unsigned_short *self);
void ImVector_unsigned_short__push_back__bc7e736faa(ImGuiContext *imgui_c89_ctx, ImVector_unsigned_short *self, const unsigned short * v);
void ImVector_unsigned_short__push_front__ac0afae107(ImVector_unsigned_short *self, const unsigned short * v);
void ImVector_unsigned_short__reserve__ceba390e44(ImGuiContext *imgui_c89_ctx, ImVector_unsigned_short *self, int new_capacity);
void ImVector_unsigned_short__reserve_discard__1fcb1d799a(ImVector_unsigned_short *self, int new_capacity);
void ImVector_unsigned_short__resize__b08fe8284a(ImGuiContext *imgui_c89_ctx, ImVector_unsigned_short *self, int new_size);
int ImVector_unsigned_short__size__1891779e6f(ImVector_unsigned_short *self);
int ImVector_unsigned_short__size_in_bytes__8e1ed19b80(ImVector_unsigned_short *self);
void ImVector_unsigned_short__dtor_ImVector__17f975c613(ImGuiContext *imgui_c89_ctx, ImVector_unsigned_short *self);
void FreeWrapper__3f4bef908d(void * ptr, void * user_data);
void * MallocWrapper__0d59de03a2(size_t size, void * user_data);
extern ImGuiContext * GImGui;
extern char ImGuiTextBuffer_EmptyString__0b55b222e6[1];
extern const ImGuiID ImGui_IMGUI_VIEWPORT_DEFAULT_ID__0e4fa71ad9;
extern const int FONT_ATLAS_DEFAULT_TEX_DATA_H__50aaa226c3;
extern const int FONT_ATLAS_DEFAULT_TEX_DATA_W__4d8431d9a1;
#endif

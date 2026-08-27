/* Generated optional C89 convenience helpers. Do not edit. */
#include "imgui_c89_api.h"
#include "imgui_c89_internal.h"

imgui_scope imgui_begin_scope(ImGuiContext *ctx, const char * name, unsigned char * p_open, ImGuiWindowFlags flags)
{
    if (ctx == 0) return IMGUI_SCOPE_ERROR;
    if (imgui_begin(ctx, name, p_open, flags)) return IMGUI_SCOPE_ACTIVE;
    imgui_end(ctx);
    return IMGUI_SCOPE_INACTIVE;
}

imgui_scope imgui_begin_child_id_scope(ImGuiContext *ctx, ImGuiID id, const ImVec2 * size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    if (ctx == 0) return IMGUI_SCOPE_ERROR;
    if (imgui_begin_child_id(ctx, id, size_arg, child_flags, window_flags)) return IMGUI_SCOPE_ACTIVE;
    imgui_end_child(ctx);
    return IMGUI_SCOPE_INACTIVE;
}

imgui_scope imgui_begin_child_string_scope(ImGuiContext *ctx, const char * str_id, const ImVec2 * size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    if (ctx == 0) return IMGUI_SCOPE_ERROR;
    if (imgui_begin_child_string(ctx, str_id, size_arg, child_flags, window_flags)) return IMGUI_SCOPE_ACTIVE;
    imgui_end_child(ctx);
    return IMGUI_SCOPE_INACTIVE;
}

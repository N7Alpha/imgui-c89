/* Generated Dear ImGui C89 API. Do not edit. */
#ifndef IMGUI_C89_API_H
#define IMGUI_C89_API_H
#include "imgui_c89.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum imgui_scope {
    IMGUI_SCOPE_ERROR = -1,
    IMGUI_SCOPE_INACTIVE = 0,
    IMGUI_SCOPE_ACTIVE = 1
} imgui_scope;

/* Plain imgui_* functions are exact implementation entry points
 * shared by C and the C++ facade. These optional helpers normalize
 * Dear ImGui's exceptional Begin/End rule by closing an inactive
 * Begin or BeginChild before returning. */
/* Variadic C++ entry points are represented by their va_list (V) siblings. */
imgui_scope imgui_begin_scope(ImGuiContext *ctx, const char * name, unsigned char * p_open, ImGuiWindowFlags flags);
imgui_scope imgui_begin_child_id_scope(ImGuiContext *ctx, ImGuiID id, const ImVec2 * size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);
imgui_scope imgui_begin_child_string_scope(ImGuiContext *ctx, const char * str_id, const ImVec2 * size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);

#ifdef __cplusplus
}
#endif
#endif

#ifndef IMGUI_C89_PLATFORM_H
#define IMGUI_C89_PLATFORM_H

/* Platform-backend contract. Graphics integration is in imgui_c89_render.h. */

#include "imgui_c89.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef imgui_flags imgui_platform_capabilities;

enum {
    IMGUI_PLATFORM_CAP_NONE = 0,
    IMGUI_PLATFORM_CAP_CLIPBOARD = 1UL << 0,
    IMGUI_PLATFORM_CAP_MOUSE_CURSOR = 1UL << 1,
    IMGUI_PLATFORM_CAP_SET_MOUSE_POSITION = 1UL << 2,
    IMGUI_PLATFORM_CAP_IME = 1UL << 3,
    IMGUI_PLATFORM_CAP_OPEN_URL = 1UL << 4,
    IMGUI_PLATFORM_CAP_GAMEPAD = 1UL << 5,
    IMGUI_PLATFORM_CAP_MULTI_VIEWPORT = 1UL << 6
};

typedef enum imgui_mouse_cursor {
    IMGUI_MOUSE_CURSOR_NONE = -1,
    IMGUI_MOUSE_CURSOR_ARROW = 0,
    IMGUI_MOUSE_CURSOR_TEXT_INPUT = 1,
    IMGUI_MOUSE_CURSOR_RESIZE_ALL = 2,
    IMGUI_MOUSE_CURSOR_RESIZE_VERTICAL = 3,
    IMGUI_MOUSE_CURSOR_RESIZE_HORIZONTAL = 4,
    IMGUI_MOUSE_CURSOR_RESIZE_DIAGONAL_NE_SW = 5,
    IMGUI_MOUSE_CURSOR_RESIZE_DIAGONAL_NW_SE = 6,
    IMGUI_MOUSE_CURSOR_HAND = 7,
    IMGUI_MOUSE_CURSOR_NOT_ALLOWED = 8
} imgui_mouse_cursor;

typedef const char *(*imgui_clipboard_get_fn)(void *user_data);
typedef void (*imgui_clipboard_set_fn)(const char *text, void *user_data);
typedef imgui_result (*imgui_open_url_fn)(const char *url, void *user_data);
typedef imgui_result (*imgui_platform_viewport_create_fn)(
    const imgui_viewport_desc *viewport, void *user_data);
typedef imgui_result (*imgui_platform_viewport_update_fn)(
    const imgui_viewport_desc *viewport, void *user_data);
typedef void (*imgui_platform_viewport_destroy_fn)(
    imgui_id viewport_id, void *user_data);

typedef struct imgui_platform_desc {
    size_t struct_size;
    imgui_platform_capabilities capabilities;
    const char *backend_name;
    imgui_clipboard_get_fn clipboard_get;
    imgui_clipboard_set_fn clipboard_set;
    imgui_open_url_fn open_url;
    imgui_platform_viewport_create_fn viewport_create;
    imgui_platform_viewport_update_fn viewport_update;
    imgui_platform_viewport_destroy_fn viewport_destroy;
    void *user_data;
} imgui_platform_desc;

typedef struct imgui_ime_request {
    imgui_bool visible;
    imgui_bool wants_text_input;
    imgui_vec2 input_position;
    float input_line_height;
    imgui_id viewport_id;
} imgui_ime_request;

typedef struct imgui_platform_output {
    imgui_mouse_cursor mouse_cursor;
    imgui_bool set_mouse_position;
    imgui_vec2 mouse_position;
    imgui_ime_request ime;
} imgui_platform_output;

IMGUI_API void imgui_platform_desc_init(imgui_platform_desc *desc);
IMGUI_API imgui_result imgui_platform_configure(
    imgui_context *ctx,
    const imgui_platform_desc *desc);
IMGUI_API imgui_platform_capabilities imgui_platform_get_capabilities(
    const imgui_context *ctx);
IMGUI_API const imgui_platform_output *imgui_get_platform_output(
    const imgui_context *ctx);
IMGUI_API const char *imgui_clipboard_get(imgui_context *ctx);
IMGUI_API imgui_result imgui_clipboard_set(imgui_context *ctx,
                                           const char *text);

#ifdef __cplusplus
}
#endif

#endif

#ifndef IMGUI_C89_TRACE_H
#define IMGUI_C89_TRACE_H

#include "imgui_c89.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct imgui_trace imgui_trace;

typedef enum imgui_trace_event_type {
    IMGUI_TRACE_MOUSE_POSITION = 0,
    IMGUI_TRACE_MOUSE_BUTTON = 1,
    IMGUI_TRACE_MOUSE_WHEEL = 2,
    IMGUI_TRACE_KEY = 3,
    IMGUI_TRACE_KEY_ANALOG = 4,
    IMGUI_TRACE_TEXT_UTF8 = 5,
    IMGUI_TRACE_CODEPOINT = 6,
    IMGUI_TRACE_FOCUS = 7,
    IMGUI_TRACE_FRAME = 8
} imgui_trace_event_type;

typedef struct imgui_trace_event {
    size_t struct_size;
    imgui_trace_event_type type;
    float x;
    float y;
    float analog_value;
    double time;
    int code;
    imgui_bool down;
    unsigned long codepoint;
    size_t text_length;
    char text[256];
} imgui_trace_event;

typedef imgui_result (*imgui_trace_frame_callback)(
    imgui_context *ctx, float delta_time, double time, void *user_data);

IMGUI_API void imgui_trace_event_init(imgui_trace_event *event,
                                      imgui_trace_event_type type);
IMGUI_API imgui_result imgui_trace_create(
    const imgui_allocator *allocator, imgui_trace **out_trace);
IMGUI_API void imgui_trace_destroy(imgui_trace *trace);
IMGUI_API void imgui_trace_clear(imgui_trace *trace);
IMGUI_API size_t imgui_trace_get_count(const imgui_trace *trace);
IMGUI_API const imgui_trace_event *imgui_trace_get_event(
    const imgui_trace *trace, size_t index);
IMGUI_API imgui_result imgui_trace_append_event(
    imgui_trace *trace, const imgui_trace_event *event);
IMGUI_API imgui_result imgui_trace_append_mouse_position(
    imgui_trace *trace, float x, float y);
IMGUI_API imgui_result imgui_trace_append_mouse_button(
    imgui_trace *trace, imgui_mouse_button button, imgui_bool down);
IMGUI_API imgui_result imgui_trace_append_mouse_wheel(
    imgui_trace *trace, float horizontal, float vertical);
IMGUI_API imgui_result imgui_trace_append_key(
    imgui_trace *trace, imgui_key key, imgui_bool down);
IMGUI_API imgui_result imgui_trace_append_key_analog(
    imgui_trace *trace, imgui_key key, imgui_bool down, float value);
IMGUI_API imgui_result imgui_trace_append_text(
    imgui_trace *trace, const char *text);
IMGUI_API imgui_result imgui_trace_append_codepoint(
    imgui_trace *trace, unsigned long codepoint);
IMGUI_API imgui_result imgui_trace_append_focus(
    imgui_trace *trace, imgui_bool focused);
IMGUI_API imgui_result imgui_trace_append_frame(
    imgui_trace *trace, float delta_time, double time);
IMGUI_API imgui_result imgui_trace_apply_event(
    imgui_context *ctx, const imgui_trace_event *event);
IMGUI_API imgui_result imgui_trace_replay(
    imgui_context *ctx, const imgui_trace *trace,
    size_t first_event, size_t event_count);
IMGUI_API imgui_result imgui_trace_replay_frames(
    imgui_context *ctx, const imgui_trace *trace,
    size_t first_event, size_t event_count,
    imgui_trace_frame_callback callback, void *user_data);
IMGUI_API imgui_result imgui_trace_save(
    const imgui_trace *trace, char *buffer, size_t capacity,
    size_t *required);
IMGUI_API imgui_result imgui_trace_load(
    imgui_trace *trace, const char *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif

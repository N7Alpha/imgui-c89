#include "imgui_c89.h"
#include "imgui_c89_font.h"
#include "imgui_c89_platform.h"
#include "imgui_c89_render.h"
#include "imgui_c89_software.h"
#include "imgui_c89_trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int failures;
static int reported_errors;
static int edit_events;
static int always_events;
static int filter_events;
static int completion_events;
static int history_events;
static size_t log_bytes;
static char clipboard_text[4096];
static char logged_text[128];
static unsigned char software_pixels[256 * 256 * 4];
static int software_texture_resolves;
static int software_command_callbacks;
static int software_copy_callbacks;
static int software_clear_callbacks;
static int software_update_callbacks;
static int platform_viewport_creates;
static int platform_viewport_updates;
static int platform_viewport_destroys;
static int trace_frame_callbacks;
static float trace_last_delta;
static double trace_last_time;
static imgui_texture *software_source_texture;
static imgui_texture *software_destination_texture;
static unsigned char software_source_pixels[16 * 16 * 4];
static unsigned char software_destination_pixels[16 * 16 * 4];

static imgui_bool software_texture_resolver(const imgui_texture *texture,
                                            imgui_software_texture_view *view,
                                            void *user_data)
{
    static unsigned char fallback_pixel[4] = {255, 0, 0, 255};
    (void)user_data;
    if (view == NULL) return IMGUI_FALSE;
    ++software_texture_resolves;
    if (texture == software_source_texture) {
        view->pixels = software_source_pixels;
        view->mutable_pixels = software_source_pixels;
    } else if (texture == software_destination_texture) {
        view->pixels = software_destination_pixels;
        view->mutable_pixels = software_destination_pixels;
    } else {
        view->pixels = fallback_pixel;
        view->mutable_pixels = fallback_pixel;
    }
    view->width = 16;
    view->height = 16;
    view->stride = 16U * 4U;
    view->format = IMGUI_TEXTURE_FORMAT_RGBA8;
    return IMGUI_TRUE;
}

static void software_command_callback(const imgui_render_command *command,
                                      void *user_data)
{
    (void)user_data;
    ++software_command_callbacks;
    if (command == NULL) return;
    if (command->type == IMGUI_RENDER_COMMAND_TEXTURE_COPY) {
        ++software_copy_callbacks;
    } else if (command->type == IMGUI_RENDER_COMMAND_TEXTURE_CLEAR) {
        ++software_clear_callbacks;
    } else if (command->type == IMGUI_RENDER_COMMAND_TEXTURE_UPDATE) {
        ++software_update_callbacks;
    }
}

static imgui_result platform_viewport_create(
    const imgui_viewport_desc *viewport, void *user_data)
{
    (void)user_data;
    if (viewport == NULL || viewport->viewport_id == 0) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ++platform_viewport_creates;
    return IMGUI_RESULT_OK;
}

static imgui_result platform_viewport_update(
    const imgui_viewport_desc *viewport, void *user_data)
{
    (void)user_data;
    if (viewport == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    ++platform_viewport_updates;
    return IMGUI_RESULT_OK;
}

static void platform_viewport_destroy(imgui_id viewport_id, void *user_data)
{
    (void)viewport_id;
    (void)user_data;
    ++platform_viewport_destroys;
}

static imgui_result trace_frame_callback(imgui_context *ctx,
                                         float delta_time,
                                         double time,
                                         void *user_data)
{
    (void)ctx;
    (void)user_data;
    ++trace_frame_callbacks;
    trace_last_delta = delta_time;
    trace_last_time = time;
    return IMGUI_RESULT_OK;
}

static const char *localize_callback(const char *key, size_t length,
                                     void *user_data)
{
    (void)length;
    (void)user_data;
    if (strcmp(key, "hello") == 0) return "Bonjour";
    return NULL;
}

static void error_counter(imgui_error_code code,
                          const char *message,
                          void *user_data)
{
    (void)message;
    (void)user_data;
    if (code != IMGUI_ERROR_NONE) ++reported_errors;
}

static int edit_callback(imgui_text_event *event)
{
    if (event != NULL && event->type == IMGUI_TEXT_EVENT_EDIT) {
        ++edit_events;
    } else if (event != NULL && event->type == IMGUI_TEXT_EVENT_ALWAYS) {
        ++always_events;
    } else if (event != NULL &&
               event->type == IMGUI_TEXT_EVENT_FILTER_CHARACTER) {
        ++filter_events;
        if (event->input_codepoint == (unsigned long)'!') {
            event->input_codepoint = 0;
        }
    } else if (event != NULL &&
               event->type == IMGUI_TEXT_EVENT_COMPLETION) {
        ++completion_events;
    } else if (event != NULL && event->type == IMGUI_TEXT_EVENT_HISTORY) {
        ++history_events;
    }
    return 0;
}

static const char *clipboard_get(void *user_data)
{
    (void)user_data;
    return clipboard_text;
}

static void clipboard_set(const char *text, void *user_data)
{
    size_t length;
    (void)user_data;
    length = strlen(text);
    if (length >= sizeof(clipboard_text)) length = sizeof(clipboard_text) - 1;
    memcpy(clipboard_text, text, length);
    clipboard_text[length] = '\0';
}

static imgui_result long_text_reserve(void *user_data,
                                      size_t required_capacity,
                                      char **data,
                                      size_t *capacity)
{
    char *storage;
    (void)user_data;
    if (data == NULL || capacity == NULL || required_capacity == 0) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    storage = (char *)realloc(*data, required_capacity);
    if (storage == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    *data = storage;
    *capacity = required_capacity;
    return IMGUI_RESULT_OK;
}

static void log_callback(const char *text, size_t length, void *user_data)
{
    size_t copy_length;
    (void)user_data;
    copy_length = length;
    if (copy_length >= sizeof(logged_text)) {
        copy_length = sizeof(logged_text) - 1;
    }
    memcpy(logged_text, text, copy_length);
    logged_text[copy_length] = '\0';
    log_bytes += length;
}

static void check(imgui_bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures += 1;
    }
}

/* Most of this behavioral fixture predates the pinned Dear ImGui fallback
   geometry and intentionally exercises interaction at the legacy (8,8)
   origin. Keep those coordinates explicit so the suite can also test the
   production default (60,60; 400x400) independently. */
static imgui_scope smoke_legacy_window_begin(imgui_context *ctx,
                                             const char *title)
{
    imgui_window_desc desc;
    imgui_window_desc_init(&desc, title);
    desc.has_position = IMGUI_TRUE;
    desc.position = imgui_make_vec2(8.0f, 8.0f);
    desc.has_size = IMGUI_TRUE;
    desc.size = imgui_make_vec2(624.0f, 464.0f);
    return imgui_window_begin_ex(ctx, &desc);
}

#define imgui_window_begin(ctx, title) \
    smoke_legacy_window_begin((ctx), (title))

static void font_smoke(imgui_context *ctx,
                       imgui_font_atlas **out_atlas,
                       imgui_font **out_font,
                       imgui_texture **out_texture)
{
    static const char *font_paths[] = {
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        NULL
    };
    imgui_font_atlas *atlas;
    imgui_font *font;
    imgui_font_config config;
    imgui_font_atlas_pixels pixels;
    imgui_texture_desc texture_desc;
    imgui_texture *texture;
    const imgui_font_glyph *glyph;
    imgui_vec2 measured;
    FILE *file;
    unsigned char *bytes;
    long file_size;
    size_t read_size;
    int path_index;
    const unsigned char invalid_font[4] = {0, 1, 2, 3};

    if (out_atlas != NULL) *out_atlas = NULL;
    if (out_font != NULL) *out_font = NULL;
    if (out_texture != NULL) *out_texture = NULL;

    check(imgui_font_atlas_create(ctx, &atlas) == IMGUI_RESULT_OK,
          "font atlas creation");
    if (atlas == NULL) return;
    check(imgui_font_atlas_add_ttf(atlas, invalid_font, sizeof(invalid_font),
                                   NULL, &font) == IMGUI_RESULT_CORRUPT_DATA,
          "invalid font rejected");
    file = NULL;
    path_index = 0;
    while (font_paths[path_index] != NULL && file == NULL) {
        file = fopen(font_paths[path_index], "rb");
        ++path_index;
    }
    if (file != NULL) {
        fseek(file, 0L, SEEK_END);
        file_size = ftell(file);
        fseek(file, 0L, SEEK_SET);
        if (file_size > 0L && file_size < 4L * 1024L * 1024L) {
            bytes = (unsigned char *)malloc((size_t)file_size);
            if (bytes != NULL) {
                read_size = fread(bytes, 1, (size_t)file_size, file);
                check(read_size == (size_t)file_size, "font file read");
                imgui_font_config_init(&config);
                config.first_codepoint = 32UL;
                config.last_codepoint = 126UL;
                config.atlas_width = 256;
                config.pixel_height = 18.0f;
                config.last_codepoint = 0x110000UL;
                check(imgui_font_atlas_add_ttf(
                          atlas, bytes, read_size, &config, &font) ==
                          IMGUI_RESULT_INVALID_ARGUMENT,
                      "font codepoint range rejects values outside Unicode");
                config.last_codepoint = 126UL;
                config.pixel_height = (float)strtod("nan", NULL);
                check(imgui_font_atlas_add_ttf(
                          atlas, bytes, read_size, &config, &font) ==
                          IMGUI_RESULT_INVALID_ARGUMENT,
                      "non-finite font height rejected");
                config.pixel_height = 18.0f;
                config.font_index = 99;
                check(imgui_font_atlas_add_ttf(
                          atlas, bytes, read_size, &config, &font) ==
                          IMGUI_RESULT_CORRUPT_DATA,
                      "invalid TrueType collection face rejected");
                config.font_index = 0;
                config.struct_size = offsetof(imgui_font_config, font_index);
                check(imgui_font_atlas_add_ttf(
                          atlas, bytes, read_size, &config, &font) ==
                          IMGUI_RESULT_OK,
                      "legacy font config accepted");
                config.struct_size = sizeof(config);
                config.last_codepoint = 600UL;
                check(imgui_font_atlas_add_ttf(
                          atlas, bytes, read_size, &config, &font) ==
                          IMGUI_RESULT_OK &&
                          imgui_font_find_glyph(font, 600UL) != NULL,
                      "TrueType font add supports dynamic glyph ranges");
                check(imgui_font_add_codepoint_range(font, 700UL, 710UL) ==
                          IMGUI_RESULT_OK &&
                          imgui_font_find_glyph(font, 705UL) != NULL,
                      "existing font extends Unicode coverage");
                check(imgui_font_atlas_build(atlas) == IMGUI_RESULT_OK,
                      "font atlas build");
                check(imgui_font_atlas_build(atlas) == IMGUI_RESULT_OK,
                      "font atlas build cache");
                check(imgui_font_atlas_get_pixels(atlas, &pixels) ==
                          IMGUI_RESULT_OK && pixels.width == 256 &&
                          pixels.bytes_per_pixel == 1,
                      "font atlas pixels");
                imgui_texture_desc_init(&texture_desc);
                texture_desc.format = IMGUI_TEXTURE_FORMAT_ALPHA8;
                texture_desc.width = pixels.width;
                texture_desc.height = pixels.height;
                texture = NULL;
                check(imgui_texture_create(ctx, &texture_desc, NULL, 0,
                                            &texture) == IMGUI_RESULT_OK,
                      "font atlas texture creation");
                if (texture != NULL) {
                    check(imgui_font_atlas_upload(ctx, atlas, texture) ==
                              IMGUI_RESULT_OK, "font atlas texture upload");
                }
                glyph = imgui_font_find_glyph(font, (unsigned long)'A');
                check(glyph != NULL && glyph->width > 0 && glyph->height > 0,
                      "font glyph lookup");
                measured = imgui_font_measure_text(font, "A\nB", NULL, 0.0f);
                check(measured.x > 0.0f && measured.y >= 36.0f,
                      "font text measurement");
                free(bytes);
                if (out_atlas != NULL) *out_atlas = atlas;
                if (out_font != NULL) *out_font = font;
                if (out_texture != NULL) *out_texture = texture;
                if (out_atlas != NULL && *out_atlas != NULL) {
                    fclose(file);
                    return;
                }
            }
        }
        fclose(file);
    }
    imgui_font_atlas_destroy(atlas);
}

int main(void)
{
    imgui_config config;
    imgui_frame_desc frame;
    imgui_log_desc log_desc;
    imgui_localization_desc localization_desc;
    imgui_metrics metrics;
    imgui_style style;
    imgui_software_target software_target;
    imgui_software_diff software_diff;
    imgui_software_packet_diff packet_diff;
    unsigned char software_pixels_copy[256 * 256 * 4];
    imgui_trace *trace;
    imgui_trace *loaded_trace;
    char trace_text[4096];
    size_t trace_required;
    imgui_renderer_desc renderer;
    imgui_platform_desc platform;
    imgui_viewport_desc viewport_desc;
    imgui_context *ctx;
    imgui_context *focus_ctx;
    imgui_context *close_ctx;
    imgui_context *detached_ctx;
    imgui_context *resource_ctx;
    imgui_context *settings_ctx;
    imgui_texture *resource_texture;
    imgui_renderer_desc capability_renderer;
    imgui_font_atlas *font_atlas;
    imgui_font_atlas *detached_atlas;
    imgui_font *font;
    imgui_texture *font_texture;
    const imgui_render_packet *packet;
    imgui_render_packet *cloned_packet = NULL;
    imgui_render_packet *malformed_packet_clone = NULL;
    imgui_render_packet malformed_packet;
    imgui_viewport_packet malformed_viewport;
    imgui_render_list malformed_list;
    imgui_render_command malformed_command;
    imgui_resource_operation malformed_resource;
    imgui_render_packet resource_packet_left;
    imgui_render_packet resource_packet_right;
    imgui_resource_operation partial_upload_left;
    imgui_resource_operation partial_upload_right;
    unsigned char partial_upload_a[8];
    unsigned char partial_upload_b[8];
    imgui_scope scope;
    imgui_window_desc title_desc;
    imgui_window_desc close_desc;
    imgui_bool close_open;
    char long_title[256];
    imgui_vec2 move_cursor;
    imgui_vec2 saved_cursor;
    imgui_bool demo_open;
    imgui_texture_desc texture_desc;
    imgui_texture *source_texture;
    imgui_texture *destination_texture;
    imgui_texture *owned_texture;
    imgui_texture *capability_texture;
    float color_rgba[4];
    float color_rgb[3];
    imgui_texture_copy_command copy;
    imgui_texture_clear_command clear;
    imgui_texture_update_command ordered_update;
    imgui_texture_update_command capability_update;
    imgui_texture_desc capability_texture_desc;
    const char custom_payload[] = "payload";
    const unsigned char texture_pixels[16] = {0};
    const unsigned char update_pixels[4] = {0x7a, 0x6b, 0x5c, 0x4d};
    const unsigned char capability_pixels[4] = {0, 0, 0, 0};
    imgui_id first_id;
    imgui_id nested_id;
    imgui_id crc_id;
    imgui_id dock_window_id;
    imgui_id dock_node_id;
    imgui_id dock_child_left;
    imgui_id dock_child_right;
    imgui_bool found_secondary_viewport;
    float split_left_x;
    float split_right_x;
    int command_index;
    int sort_column;
    int sort_direction;
    const imgui_table_sort_specs *sort_specs;
    const char *stable_first;
    const char *stable_second;
    int path_index;
    int navigation_index;
    imgui_bool large_tree_ok;
    imgui_result path_result;
    imgui_u32 title_font_indices;
    imgui_frame_desc resource_frame;
    imgui_texture_desc resource_desc;
    imgui_renderer_desc resource_renderer;
    const unsigned char resource_pixel[4] = {1, 2, 3, 4};
    int pixel_index;
    imgui_bool found_copy;
    imgui_bool found_custom;
    imgui_bool found_font_draw;
    imgui_bool found_font_update;
    imgui_bool found_image_button_uv;
    imgui_bool font_geometry_clipped;
    imgui_bool found_window_a;
    imgui_bool found_window_b;
    imgui_bool found_style_color;
    imgui_bool tab_open;
    imgui_bool pressed;
    imgui_bool checkbox_value;
    int custom_index;
    int channel_middle_index;
    int channel_high_index;
    char fixed_text[32];
    char long_text[4096];
    imgui_text_buffer managed_text;
    char settings_text[8192];
    char settings_before_bad_load[8192];
    char wide_table_settings[8192];
    char wide_table_piece[32];
    char wide_window_settings[65536];
    char wide_window_piece[128];
    char settings_output[131072];
    imgui_id selected_ids[2];
    imgui_multi_select_storage selection;
    imgui_id selection_first_id;
    imgui_id selection_second_id;
    imgui_bool selection_value;
    imgui_bool metrics_open;
    int drag_value;
    int drag_type_index;
    int scope_index;
    int scope_error_count;
    int id_index;
    int id_error_count;
    int dock_index;
    int window_index;
    int window_error_count;
    char long_drag_type[300];
    const imgui_drag_payload *drag_payload_view;
    imgui_id drag_source_id;
    imgui_id drag_target_id;
    imgui_id capture_id;
    imgui_window_desc geometry_desc;
    imgui_window_desc auto_resize_desc;
    imgui_window_desc smoke_desc;
    imgui_window_desc scroll_desc;
    imgui_window_desc hscroll_desc;
    imgui_window_desc no_mouse_scroll_desc;
    imgui_window_desc no_focus_desc;
    imgui_frame_desc focus_frame;
    imgui_vec2 geometry_cursor;
    imgui_vec2 text_size;
    imgui_input_text_desc text_desc;
    size_t settings_required;
    float slider_value;
    float logarithmic_value;
    float signed_logarithmic_value;
    float drag_unclamped_value;
    float vertical_slider_value;
    float vector_values[3];
    int vector_integer_values[3];
    int drag_integer_value;
    float input_float_value;
    float invalid_widget_value;
    int slider_integer_value;
    int numeric_value;
    imgui_vec2 cursor_before_indent;
    imgui_vec2 cursor_after_indent;
    imgui_vec2 vertical_window_size;
    imgui_vec2 child_cursor_before;
    imgui_vec2 child_cursor_after;
    imgui_rect custom_rect;
    imgui_vec2 polyline[3];
    float plot_values[3];
    imgui_u32 invisible_base_vertices;
    imgui_u32 invisible_base_indices;
    imgui_u32 invisible_base_commands;

    imgui_config_init(&config);
    config.error_callback = error_counter;
    config.struct_size = offsetof(imgui_config, flags);
    ctx = imgui_context_create(&config);
    config.struct_size = sizeof(config);
    check(ctx != NULL, "context creation");
    if (ctx == NULL) {
        return 1;
    }
    imgui_window_desc_init(&smoke_desc, "Smoke");
    smoke_desc.has_position = IMGUI_TRUE;
    smoke_desc.position = imgui_make_vec2(8.0f, 8.0f);
    smoke_desc.has_size = IMGUI_TRUE;
    smoke_desc.size = imgui_make_vec2(624.0f, 464.0f);
    focus_ctx = imgui_context_create(&config);
    check(focus_ctx != NULL, "focus-flag context creation");
    settings_ctx = NULL;

    trace = NULL;
    loaded_trace = NULL;
    check(imgui_trace_create(NULL, &trace) == IMGUI_RESULT_OK,
          "trace creation");
    if (trace != NULL) {
        check(imgui_trace_append_mouse_position(trace, 20.0f, 50.0f) ==
                  IMGUI_RESULT_OK, "trace mouse event");
        check(imgui_trace_append_key(trace, IMGUI_KEY_A, IMGUI_TRUE) ==
                  IMGUI_RESULT_OK, "trace key event");
        check(imgui_trace_append_text(trace, "trace text") ==
                  IMGUI_RESULT_OK, "trace text event");
        check(imgui_trace_append_codepoint(trace, 0x1f642UL) ==
                  IMGUI_RESULT_OK, "trace codepoint event");
        check(imgui_trace_append_frame(trace, 1.0f / 60.0f, 2.5) ==
                  IMGUI_RESULT_OK, "trace frame marker");
        check(imgui_trace_save(trace, NULL, 0, &trace_required) ==
                  IMGUI_RESULT_INVALID_ARGUMENT && trace_required > 0,
              "trace size query");
        check(imgui_trace_save(trace, trace_text, sizeof(trace_text),
                               &trace_required) == IMGUI_RESULT_OK &&
                  strstr(trace_text, "IMGUI_C89_TRACE 1\n") != NULL,
              "trace save");
        check(imgui_trace_create(NULL, &loaded_trace) == IMGUI_RESULT_OK,
              "loaded trace creation");
        check(imgui_trace_load(loaded_trace, trace_text,
                               strlen(trace_text)) == IMGUI_RESULT_OK &&
                  imgui_trace_get_count(loaded_trace) ==
                  imgui_trace_get_count(trace), "trace round trip");
        check(imgui_trace_apply_event(ctx,
                  imgui_trace_get_event(loaded_trace, 0)) == IMGUI_RESULT_OK,
              "trace event replay");
        check(imgui_trace_replay(ctx, loaded_trace, 0, 1) == IMGUI_RESULT_OK,
              "trace range replay");
        trace_frame_callbacks = 0;
        check(imgui_trace_replay_frames(ctx, loaded_trace,
                                        imgui_trace_get_count(loaded_trace) - 1,
                                        1,
                                        trace_frame_callback, NULL) ==
                  IMGUI_RESULT_OK && trace_frame_callbacks == 1 &&
                  trace_last_delta > 0.0f && trace_last_time == 2.5,
              "trace frame replay callback");
    }

    imgui_renderer_desc_init(&renderer);
    renderer.capabilities = IMGUI_RENDERER_CAP_VERTEX_OFFSET |
                            IMGUI_RENDERER_CAP_TEXTURES |
                            IMGUI_RENDERER_CAP_CUSTOM_COMMANDS |
                            IMGUI_RENDERER_CAP_ORDERED_TEXTURE_COPY |
                            IMGUI_RENDERER_CAP_TEXTURE_CLEAR |
                            IMGUI_RENDERER_CAP_TEXTURE_UPDATE;
    renderer.struct_size = offsetof(imgui_renderer_desc, backend_name);
    check(imgui_renderer_configure(ctx, &renderer) == IMGUI_RESULT_OK,
          "legacy renderer descriptor configuration");
    check(imgui_renderer_configure(focus_ctx, &renderer) == IMGUI_RESULT_OK,
          "focus renderer descriptor configuration");
    imgui_style_init(&style);
    style.struct_size = offsetof(imgui_style, color_button);
    check(imgui_style_set(ctx, &style) == IMGUI_RESULT_OK,
          "legacy style descriptor configuration");
    check(imgui_style_get(ctx)->color_plot_lines ==
              imgui_style_get(ctx)->color_frame_active &&
              imgui_style_get(ctx)->color_plot_histogram ==
              imgui_style_get(ctx)->color_frame_active,
          "legacy style defaults appended plot colors");
    style.struct_size = sizeof(style);
    imgui_style_init(&style);
    style.color_button_hovered = 0xff123456UL;
    style.color_button_active = 0xff123456UL;
    style.color_plot_lines = 0xff12ab34UL;
    style.color_plot_histogram = 0xffab3412UL;
    check(imgui_style_set(ctx, &style) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_button_hovered == 0xff123456UL &&
              imgui_style_get(ctx)->color_plot_lines == 0xff12ab34UL &&
              imgui_style_get(ctx)->color_plot_histogram == 0xffab3412UL,
          "style configuration");
    imgui_style_init(&style);
    style.scrollbar_size = 17.0f;
    style.scrollbar_grab_min_size = 23.0f;
    style.child_rounding = 4.0f;
    style.window_border_size = 0.0f;
    style.child_border_size = 2.0f;
    style.frame_border_size = 1.5f;
    check(imgui_style_set(focus_ctx, &style) == IMGUI_RESULT_OK &&
              imgui_style_get(focus_ctx)->scrollbar_size == 17.0f &&
              imgui_style_get(focus_ctx)->scrollbar_grab_min_size == 23.0f &&
              imgui_style_get(focus_ctx)->child_rounding == 4.0f &&
              imgui_style_get(focus_ctx)->window_border_size == 0.0f &&
              imgui_style_get(focus_ctx)->child_border_size == 2.0f &&
              imgui_style_get(focus_ctx)->frame_border_size == 1.5f,
          "style-configurable scrollbar geometry");
    imgui_style_init(&style);
    check(imgui_style_set(focus_ctx, &style) == IMGUI_RESULT_OK,
          "restore default scrollbar geometry");
    font_smoke(ctx, &font_atlas, &font, &font_texture);
    if (font != NULL && font_texture != NULL) {
        imgui_renderer_desc_init(&capability_renderer);
        capability_renderer.capabilities = IMGUI_RENDERER_CAP_NONE;
        check(imgui_renderer_configure(ctx, &capability_renderer) ==
                  IMGUI_RESULT_OK &&
              imgui_font_bind(ctx, font, font_texture) ==
                  IMGUI_RESULT_UNSUPPORTED,
              "font binding requires texture capability");
        check(imgui_renderer_configure(ctx, &renderer) == IMGUI_RESULT_OK,
              "font renderer capability restore");
    }
    imgui_localization_desc_init(&localization_desc);
    localization_desc.callback = localize_callback;
    check(imgui_localization_configure(ctx, &localization_desc) ==
              IMGUI_RESULT_OK && strcmp(imgui_localize(ctx, "hello"),
                                        "Bonjour") == 0,
          "localization configuration");

    imgui_texture_desc_init(&texture_desc);
    texture_desc.width = 2;
    texture_desc.height = 2;
    check(imgui_texture_create(ctx, &texture_desc, texture_pixels, 8,
                               &owned_texture) == IMGUI_RESULT_OK,
          "owned texture creation");

    imgui_platform_desc_init(&platform);
    platform.capabilities = IMGUI_PLATFORM_CAP_MOUSE_CURSOR |
                            IMGUI_PLATFORM_CAP_CLIPBOARD;
    platform.clipboard_get = clipboard_get;
    platform.clipboard_set = clipboard_set;
    platform.viewport_create = platform_viewport_create;
    platform.viewport_update = platform_viewport_update;
    platform.viewport_destroy = platform_viewport_destroy;
    platform.struct_size = offsetof(imgui_platform_desc, user_data);
    check(imgui_platform_configure(ctx, &platform) == IMGUI_RESULT_OK,
          "legacy platform descriptor configuration");
    check(imgui_clipboard_set(ctx, "clipboard test") == IMGUI_RESULT_OK,
          "clipboard set");
    check(strcmp(imgui_clipboard_get(ctx), "clipboard test") == 0,
          "clipboard get");

    check(imgui_input_add_mouse_position(ctx, 20.0f, 70.0f) ==
              IMGUI_RESULT_OK,
          "mouse input");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "mouse button input");
    check(imgui_input_add_key(ctx, IMGUI_KEY_A, IMGUI_FALSE) ==
              IMGUI_RESULT_OK,
          "key input");
    check(imgui_input_add_codepoint(ctx, 0x1f642UL) == IMGUI_RESULT_OK,
          "UTF-8 codepoint input");
    check(imgui_input_add_text_utf8(ctx, "bad\xc0\xaf") ==
              IMGUI_RESULT_INVALID_ARGUMENT,
          "invalid UTF-8 rejection");
    check(reported_errors > 0, "error callback receives invalid input");

    imgui_frame_desc_init(&frame);
    frame.delta_time = 0.25f;
    frame.time = 12.5;
    imgui_frame_desc_init(&focus_frame);
    focus_frame.display_size = imgui_make_vec2(640.0f, 480.0f);
    focus_frame.time = 1.0;
    imgui_window_desc_init(&no_focus_desc, "NoFocusOnAppearing");
    no_focus_desc.flags = IMGUI_WINDOW_NO_FOCUS_ON_APPEARING;
    no_focus_desc.has_position = IMGUI_TRUE;
    no_focus_desc.position = imgui_make_vec2(8.0f, 8.0f);
    no_focus_desc.has_size = IMGUI_TRUE;
    no_focus_desc.size = imgui_make_vec2(400.0f, 300.0f);
    if (focus_ctx != NULL) {
        check(imgui_frame_begin(focus_ctx, &focus_frame) == IMGUI_RESULT_OK,
              "no-focus first frame begin");
        check(imgui_set_next_window_size_constraints(
                  focus_ctx, imgui_make_vec2(200.0f, 120.0f),
                  imgui_make_vec2(250.0f, 180.0f)) == IMGUI_RESULT_OK,
              "next-window size constraints setter");
        check(imgui_set_next_window_position_ex(
                  focus_ctx, imgui_make_vec2(320.0f, 240.0f),
                  imgui_make_vec2(0.5f, 0.5f)) == IMGUI_RESULT_OK,
              "next-window position pivot setter");
        check(imgui_set_next_window_scroll(
                  focus_ctx, imgui_make_vec2(3.0f, 4.0f)) == IMGUI_RESULT_OK,
              "next-window scroll setter");
        scope = imgui_window_begin_ex(focus_ctx, &no_focus_desc);
        check(scope == IMGUI_SCOPE_ACTIVE &&
                  imgui_get_window_size(focus_ctx).x <= 250.0f &&
                  imgui_get_window_size(focus_ctx).y <= 180.0f &&
                  imgui_get_scroll_x(focus_ctx) == 3.0f &&
                  imgui_get_scroll_y(focus_ctx) == 4.0f &&
                  imgui_get_window_position(focus_ctx).x > 190.0f &&
                  imgui_get_window_position(focus_ctx).x < 200.0f &&
                  !imgui_is_window_focused(focus_ctx, 0),
              "no-focus-on-appearing suppresses automatic focus");
        imgui_window_end(focus_ctx);
        check(imgui_frame_end(focus_ctx) == IMGUI_RESULT_OK,
              "no-focus first frame end");
        (void)imgui_render(focus_ctx);
        check(imgui_input_add_mouse_position(focus_ctx, 16.0f, 16.0f) ==
                  IMGUI_RESULT_OK &&
                  imgui_input_add_mouse_button(focus_ctx,
                      IMGUI_MOUSE_BUTTON_LEFT, IMGUI_TRUE) == IMGUI_RESULT_OK,
              "no-focus click input");
        focus_frame.time = 2.0;
        check(imgui_frame_begin(focus_ctx, &focus_frame) == IMGUI_RESULT_OK,
              "no-focus click frame begin");
        check(imgui_set_next_window_background_alpha(focus_ctx, 0.25f) ==
                  IMGUI_RESULT_OK,
              "next-window background alpha setter");
        scope = imgui_window_begin_ex(focus_ctx, &no_focus_desc);
        check(scope == IMGUI_SCOPE_ACTIVE &&
                  imgui_is_window_focused(focus_ctx, 0),
              "no-focus window accepts explicit click focus");
        imgui_window_end(focus_ctx);
        check(imgui_frame_end(focus_ctx) == IMGUI_RESULT_OK,
              "no-focus click frame end");
        (void)imgui_render(focus_ctx);
        focus_frame.time = 2.5;
        check(imgui_frame_begin(focus_ctx, &focus_frame) == IMGUI_RESULT_OK &&
                  imgui_set_next_window_collapsed(focus_ctx, IMGUI_TRUE) ==
                  IMGUI_RESULT_OK,
              "next-window collapsed setup");
        scope = imgui_window_begin_ex(focus_ctx, &no_focus_desc);
        check(scope == IMGUI_SCOPE_INACTIVE &&
                  imgui_is_window_collapsed(focus_ctx) == IMGUI_TRUE,
              "next-window collapsed applies before begin");
        imgui_window_end(focus_ctx);
        check(imgui_frame_end(focus_ctx) == IMGUI_RESULT_OK,
              "next-window collapsed frame end");
        packet = imgui_render(focus_ctx);
        check(packet != NULL && packet->viewport_count == 1 &&
                  packet->viewports[0].list_count > 0,
              "collapsed window retains title-bar render data");
        focus_frame.time = 3.0;
        check(imgui_frame_begin(focus_ctx, &focus_frame) == IMGUI_RESULT_OK &&
                  imgui_set_next_window_collapsed(focus_ctx, IMGUI_FALSE) ==
                  IMGUI_RESULT_OK &&
                  imgui_set_next_window_focus(focus_ctx) == IMGUI_RESULT_OK,
              "next-window restore and focus setup");
        scope = imgui_window_begin_ex(focus_ctx, &no_focus_desc);
        check(scope == IMGUI_SCOPE_ACTIVE &&
                  imgui_is_window_collapsed(focus_ctx) == IMGUI_FALSE &&
                  imgui_is_window_focused(focus_ctx, 0) == IMGUI_TRUE,
              "next-window restore and focus apply");
        imgui_window_end(focus_ctx);
        check(imgui_frame_end(focus_ctx) == IMGUI_RESULT_OK,
              "next-window restore frame end");
        (void)imgui_render(focus_ctx);
        close_ctx = imgui_context_create(NULL);
        close_open = IMGUI_TRUE;
        imgui_frame_desc_init(&focus_frame);
        focus_frame.display_size = imgui_make_vec2(640.0f, 480.0f);
        imgui_window_desc_init(&close_desc, "Closeable");
        close_desc.open = &close_open;
        close_desc.has_position = IMGUI_TRUE;
        close_desc.position = imgui_make_vec2(60.0f, 60.0f);
        close_desc.has_size = IMGUI_TRUE;
        close_desc.size = imgui_make_vec2(240.0f, 160.0f);
        if (close_ctx != NULL) {
            check(imgui_frame_begin(close_ctx, &focus_frame) == IMGUI_RESULT_OK,
                  "close button first frame begin");
            scope = imgui_window_begin_ex(close_ctx, &close_desc);
            check(scope == IMGUI_SCOPE_ACTIVE, "close button first window");
            imgui_window_end(close_ctx);
            check(imgui_frame_end(close_ctx) == IMGUI_RESULT_OK,
                  "close button first frame end");
            (void)imgui_render(close_ctx);
            check(imgui_input_add_mouse_position(close_ctx, 292.0f, 70.0f) ==
                      IMGUI_RESULT_OK &&
                      imgui_input_add_mouse_button(close_ctx,
                          IMGUI_MOUSE_BUTTON_LEFT, IMGUI_TRUE) ==
                      IMGUI_RESULT_OK, "close button click input");
            focus_frame.time = 2.0;
            check(imgui_frame_begin(close_ctx, &focus_frame) == IMGUI_RESULT_OK,
                  "close button click frame begin");
            scope = imgui_window_begin_ex(close_ctx, &close_desc);
            check(scope == IMGUI_SCOPE_ACTIVE && close_open == IMGUI_TRUE,
                  "close button waits for release");
            imgui_window_end(close_ctx);
            check(imgui_frame_end(close_ctx) == IMGUI_RESULT_OK,
                  "close button click frame end");
            (void)imgui_render(close_ctx);
            check(imgui_input_add_mouse_button(close_ctx,
                                               IMGUI_MOUSE_BUTTON_LEFT,
                                               IMGUI_FALSE) == IMGUI_RESULT_OK,
                  "close button release input");
            focus_frame.time = 3.0;
            check(imgui_frame_begin(close_ctx, &focus_frame) == IMGUI_RESULT_OK,
                  "close button release frame begin");
            scope = imgui_window_begin_ex(close_ctx, &close_desc);
            check(scope == IMGUI_SCOPE_INACTIVE &&
                      close_open == IMGUI_FALSE,
                  "title bar close mutates open flag");
            imgui_window_end(close_ctx);
            check(imgui_frame_end(close_ctx) == IMGUI_RESULT_OK,
                  "close button release frame end");
            (void)imgui_render(close_ctx);
            imgui_context_destroy(close_ctx);
            close_ctx = NULL;
        } else {
            check(IMGUI_FALSE, "close button context creation");
        }
        imgui_renderer_desc_init(&capability_renderer);
        capability_renderer.capabilities = IMGUI_RENDERER_CAP_NONE;
        check(imgui_renderer_configure(focus_ctx, &capability_renderer) ==
                  IMGUI_RESULT_OK, "limited renderer configuration");
        imgui_texture_desc_init(&capability_texture_desc);
        capability_texture_desc.width = 1;
        capability_texture_desc.height = 1;
        check(imgui_texture_register_external(
                  focus_ctx, (imgui_texture_id)&capability_pixels,
                  &capability_texture_desc, &capability_texture) ==
                  IMGUI_RESULT_OK, "limited renderer texture registration");
        memset(&capability_update, 0, sizeof(capability_update));
        capability_update.texture = capability_texture;
        capability_update.region.width = 1;
        capability_update.region.height = 1;
        capability_update.format = IMGUI_TEXTURE_FORMAT_RGBA8;
        capability_update.pixels = capability_pixels;
        capability_update.pitch = 4U;
        focus_frame.time = 3.0;
        check(imgui_frame_begin(focus_ctx, &focus_frame) == IMGUI_RESULT_OK,
              "limited renderer frame begin");
        scope = imgui_window_begin_ex(focus_ctx, &no_focus_desc);
        custom_rect.x1 = 0.0f;
        custom_rect.y1 = 0.0f;
        custom_rect.x2 = 1.0f;
        custom_rect.y2 = 1.0f;
        check(scope == IMGUI_SCOPE_ACTIVE &&
                  imgui_draw_list_add_texture_update(
                      focus_ctx, imgui_get_window_draw_list(focus_ctx),
                      &capability_update) == IMGUI_RESULT_UNSUPPORTED &&
                  imgui_draw_list_add_image(
                      focus_ctx, imgui_get_window_draw_list(focus_ctx),
                      capability_texture, custom_rect,
                      imgui_make_vec2(0.0f, 0.0f),
                      imgui_make_vec2(1.0f, 1.0f), 0xffffffffUL) ==
                  IMGUI_RESULT_UNSUPPORTED &&
                  imgui_draw_list_add_rect(
                      focus_ctx, imgui_get_window_draw_list(focus_ctx),
                      custom_rect, 0xffffffffUL, capability_texture) ==
                  IMGUI_RESULT_UNSUPPORTED &&
                  imgui_draw_list_add_rect_rounded(
                      focus_ctx, imgui_get_window_draw_list(focus_ctx),
                      custom_rect, 1.0f, 0xffffffffUL, 2,
                      capability_texture) == IMGUI_RESULT_UNSUPPORTED,
              "texture draw capabilities are enforced");
        check(imgui_draw_list_add_line(
                  focus_ctx, imgui_get_window_draw_list(focus_ctx),
                  imgui_make_vec2(0.0f, 0.0f),
                  imgui_make_vec2(2.0f, 2.0f), 0xffffffffUL, 1.0f) ==
                  IMGUI_RESULT_OK,
              "limited renderer accepts geometry before capability check");
        memset(&copy, 0, sizeof(copy));
        copy.source = capability_texture;
        copy.destination = capability_texture;
        copy.source_region.width = 1;
        copy.source_region.height = 1;
        memset(&clear, 0, sizeof(clear));
        clear.texture = capability_texture;
        clear.region.width = 1;
        clear.region.height = 1;
        check(imgui_draw_list_add_texture_copy(
                  focus_ctx, imgui_get_window_draw_list(focus_ctx), &copy) ==
                  IMGUI_RESULT_UNSUPPORTED &&
              imgui_draw_list_add_texture_clear(
                  focus_ctx, imgui_get_window_draw_list(focus_ctx), &clear) ==
                  IMGUI_RESULT_UNSUPPORTED,
              "ordered texture commands require base texture capability");
        imgui_window_end(focus_ctx);
        check(imgui_frame_end(focus_ctx) == IMGUI_RESULT_OK,
              "limited renderer frame end");
        check(imgui_render(focus_ctx) == NULL,
              "limited renderer rejects unsupported vertex offsets");
    }
    imgui_multi_select_storage_init(&selection, selected_ids, 2);
    frame.display_size = imgui_make_vec2(1280.0f, 720.0f);
    frame.time = 10.0;
    frame.struct_size = offsetof(imgui_frame_desc, time);
    /* Dear ImGui suppresses rendering for one frame when a window first
       appears. Warm the primary fixture so subsequent packet assertions are
       comparing a steady-state frame. */
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "appearing-window warmup release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "appearing-window warmup begin");
    check(imgui_get_mouse_position(ctx).x == 20.0f &&
          imgui_get_mouse_position(ctx).y == 70.0f &&
          imgui_get_delta_time(ctx) == frame.delta_time &&
          imgui_get_frame_count(ctx) == 1U,
          "frame and mouse state queries");
    check(imgui_get_text_line_height(ctx) > 0.0f &&
          imgui_get_frame_height(ctx) > imgui_get_text_line_height(ctx) &&
          imgui_get_frame_height_with_spacing(ctx) >
              imgui_get_frame_height(ctx),
          "font and frame metric queries");
    scope = imgui_window_begin_ex(ctx, &smoke_desc);
    check(scope != IMGUI_SCOPE_ERROR, "appearing-window warmup window");
    {
        imgui_vec2 cursor_screen = imgui_get_cursor_screen_position(ctx);
        imgui_vec2 cursor_local = imgui_get_cursor_position(ctx);
        imgui_vec2 cursor_start = imgui_get_cursor_start_position(ctx);
        imgui_set_cursor_position(ctx, cursor_local);
        check(cursor_start.x >= 0.0f && cursor_start.y >= 0.0f &&
              imgui_get_cursor_position_x(ctx) == cursor_local.x &&
              imgui_get_cursor_position_y(ctx) == cursor_local.y &&
              imgui_get_cursor_screen_position(ctx).x == cursor_screen.x &&
              imgui_get_cursor_screen_position(ctx).y == cursor_screen.y,
              "local cursor position round-trip");
    }
    check(imgui_calc_item_width(ctx) > 0.0f,
          "current item width query");
    check(imgui_is_mouse_hovering_rect(
              ctx, imgui_make_vec2(0.0f, 0.0f),
              imgui_make_vec2(100.0f, 100.0f), IMGUI_FALSE) &&
          imgui_is_rect_visible(ctx, imgui_make_vec2(0.0f, 0.0f),
                                imgui_make_vec2(100.0f, 100.0f)),
          "independent rectangle geometry queries");
    check(imgui_get_window_content_region_size(ctx).x > 0.0f &&
              imgui_get_window_content_region_size(ctx).y > 0.0f &&
              imgui_get_window_content_region_max(ctx).x >
              imgui_get_window_content_region_min(ctx).x,
          "window content-region queries");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "appearing-window warmup end");
    packet = imgui_render(ctx);
    check(packet != NULL && packet->viewport_count == 1 &&
              packet->viewports[0].list_count == 0 &&
              packet->resource_operation_count >= 2U,
          "appearing-window draw suppression");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "appearing-window click restore");
    check(imgui_input_add_key(ctx, IMGUI_KEY_F12, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "shortcut key input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "frame begin");
    check(imgui_input_add_mouse_position(ctx, 40.0f, 70.0f) ==
              IMGUI_RESULT_OK &&
          imgui_is_mouse_dragging(ctx, IMGUI_MOUSE_BUTTON_LEFT, -1.0f) &&
          imgui_get_mouse_drag_delta(ctx, IMGUI_MOUSE_BUTTON_LEFT, -1.0f).x >=
              19.9f,
          "mouse drag threshold and delta");
    imgui_reset_mouse_drag_delta(ctx, IMGUI_MOUSE_BUTTON_LEFT);
    check(imgui_get_mouse_drag_delta(ctx, IMGUI_MOUSE_BUTTON_LEFT, -1.0f).x ==
              0.0f,
          "mouse drag delta reset");
    check(imgui_input_add_mouse_position(ctx, 20.0f, 70.0f) ==
              IMGUI_RESULT_OK, "restore mouse position");
    check(imgui_get_id_string(ctx, "abc") == (imgui_id)0x352441c2UL,
          "root CRC32 ID hash");
    check(imgui_is_key_down(ctx, IMGUI_KEY_F12) == IMGUI_TRUE,
          "key down query");
    check(imgui_get_key_analog(ctx, IMGUI_KEY_F12) == 1.0f,
          "analog key query");
    check(imgui_get_mouse_clicked_count(ctx, IMGUI_MOUSE_BUTTON_LEFT) == 1,
          "mouse click count query");
    check(imgui_is_key_pressed(ctx, IMGUI_KEY_F12, IMGUI_FALSE) == IMGUI_TRUE,
          "key pressed query");
    check(imgui_get_key_pressed_amount(ctx, IMGUI_KEY_F12, -1.0f, -1.0f) ==
              1,
          "key pressed amount query");
    check(imgui_is_shortcut_pressed(ctx, IMGUI_KEY_F12, IMGUI_KEY_MOD_NONE,
                                    IMGUI_FALSE) == IMGUI_TRUE,
          "unmodified shortcut query");
    imgui_log_desc_init(&log_desc);
    log_desc.callback = log_callback;
    logged_text[0] = '\0';
    log_bytes = 0;
    check(imgui_log_begin(ctx, &log_desc) == IMGUI_RESULT_OK,
          "logging begin");
    imgui_text_unformatted(ctx, "log sample", NULL);
    check(imgui_log_is_active(ctx), "logging active query");
    check(imgui_log_end(ctx) == IMGUI_RESULT_OK && log_bytes == 10 &&
              strcmp(logged_text, "log sample") == 0,
          "logging captures text");
    if (font != NULL && font_texture != NULL) {
        check(imgui_font_bind(ctx, font, font_texture) == IMGUI_RESULT_OK,
              "font bind");
    }
    check((frame.struct_size == offsetof(imgui_frame_desc, time) &&
           imgui_get_frame_output(ctx)->next_wake_time > 0.0 &&
           imgui_get_frame_output(ctx)->next_wake_time <= 1.0 / 60.0) ||
          (frame.struct_size != offsetof(imgui_frame_desc, time) &&
           imgui_get_frame_output(ctx)->next_wake_time > frame.time &&
           imgui_get_frame_output(ctx)->next_wake_time <=
           frame.time + 1.0 / 60.0),
          "held input wake deadline");
    text_size = imgui_calc_text_size("A\xf0\x9f\x99\x82" "B", NULL, 0.0f);
    check(text_size.x == 24.0f && text_size.y == 16.0f,
          "UTF-8 text measurement");
    text_size = imgui_calc_text_size("A\nB", NULL, 0.0f);
    check(text_size.x == 8.0f && text_size.y == 32.0f,
          "multiline text measurement");
    text_size = imgui_calc_text_size("12345", NULL, 24.0f);
    check(text_size.x == 24.0f && text_size.y == 32.0f,
          "wrapped text measurement");

    scope = imgui_window_begin_ex(ctx, &smoke_desc);
    check(scope != IMGUI_SCOPE_ERROR, "window begin");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_TEXT,
                                 0xff00ff00UL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_text == 0xff00ff00UL,
          "scoped style color push");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_BUTTON,
                                 0xff2020ffUL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_button == 0xff2020ffUL,
          "nested scoped style color push");
    check(imgui_style_pop_color(ctx) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_button != 0xff2020ffUL &&
              imgui_style_get(ctx)->color_text == 0xff00ff00UL,
          "nested scoped style color pop");
    check(imgui_style_pop_color(ctx) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_text != 0xff00ff00UL,
          "outer scoped style color pop");
    check(imgui_style_push_var_float(
              ctx, IMGUI_STYLE_VAR_CHILD_ROUNDING, 3.0f) ==
              IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->child_rounding == 3.0f &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->child_rounding != 3.0f,
          "scoped child geometry style variable");
    check(imgui_style_push_var_float(
              ctx, IMGUI_STYLE_VAR_FRAME_BORDER_SIZE, 1.0f) ==
              IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->frame_border_size == 1.0f &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "scoped frame border style variable");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_MODAL_DIM,
                                 0x40000000UL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_modal_dim == 0x40000000UL &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "modal dim style color push");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_CHILD_BACKGROUND,
                                 0x40112233UL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_child_background == 0x40112233UL &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "child background style color push");
    check(imgui_style_push_color(
              ctx, IMGUI_STYLE_COLOR_WINDOW_TITLE_BACKGROUND_COLLAPSED,
              0xff102030UL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_window_title_background_collapsed ==
              0xff102030UL && imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "collapsed title style color push");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_POPUP_BACKGROUND,
                                 0xff203040UL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_popup_background == 0xff203040UL &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "popup background style color push");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_RESIZE_GRIP,
                                 0x804080ffUL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_resize_grip == 0x804080ffUL &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "resize grip style color push");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_TAB_ACTIVE,
                                 0xff304050UL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_tab_active == 0xff304050UL &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "tab style color push");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_DRAG_DROP_TARGET,
                                 0xffd09030UL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_drag_drop_target == 0xffd09030UL &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "drag target style color push");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_NAV_HIGHLIGHT,
                                 0xff40a0ffUL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_nav_highlight == 0xff40a0ffUL &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "navigation highlight style color push");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_MENU_BAR_BACKGROUND,
                                 0xff304020UL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_menu_bar_background == 0xff304020UL &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "menu bar style color push");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_MENU_ITEM_ACTIVE,
                                 0xff506030UL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_menu_item_active == 0xff506030UL &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "menu item style color push");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_POPUP_BORDER,
                                 0xff405060UL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_popup_border == 0xff405060UL &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "popup border style color push");
    check(imgui_style_push_color(ctx, IMGUI_STYLE_COLOR_TEXT_LINK,
                                 0xff102030UL) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_text_link == 0xff102030UL &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "text link style color push");
    check(imgui_style_push_var_float(ctx, IMGUI_STYLE_VAR_FRAME_ROUNDING,
                                     3.0f) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->frame_rounding == 3.0f &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "scoped style float push");
    check(imgui_style_push_var_vec2(ctx, IMGUI_STYLE_VAR_WINDOW_PADDING,
                                    imgui_make_vec2(5.0f, 6.0f)) ==
              IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->window_padding.x == 5.0f &&
              imgui_style_get(ctx)->window_padding.y == 6.0f &&
              imgui_style_pop_color(ctx) == IMGUI_RESULT_OK,
          "scoped style vector push");
    if (font_atlas != NULL && font_texture != NULL) {
        check(imgui_font_atlas_upload(ctx, font_atlas, font_texture) ==
                  IMGUI_RESULT_OK,
              "ordered in-frame font atlas upload");
    }
    move_cursor = imgui_get_window_position(ctx);
    saved_cursor = imgui_get_cursor_screen_position(ctx);
    check(imgui_get_window_size(ctx).x > 0.0f &&
              imgui_get_window_size(ctx).y > 0.0f,
          "current window geometry query");
    imgui_set_window_position(ctx, move_cursor);
    imgui_set_window_size(ctx, imgui_get_window_size(ctx));
    imgui_set_window_position(ctx, imgui_make_vec2(move_cursor.x + 4.0f,
                                                   move_cursor.y + 3.0f));
    check(imgui_get_cursor_screen_position(ctx).x == saved_cursor.x + 4.0f &&
              imgui_get_cursor_screen_position(ctx).y == saved_cursor.y + 3.0f,
          "window position translates cursor");
    imgui_set_window_position(ctx, move_cursor);
    if (font != NULL && font_texture != NULL) {
        imgui_text_unformatted(ctx, "Font A", NULL);
        imgui_text_wrapped(ctx, "Wrapped font text", 24.0f);
        check(imgui_draw_list_add_text(
                  ctx, imgui_get_window_draw_list(ctx),
                  imgui_make_vec2(16.0f, 48.0f), "Draw list text",
                  0xffffffffUL) == IMGUI_RESULT_OK,
              "public draw-list text");
        check(imgui_draw_list_add_text(
                  ctx, imgui_get_window_draw_list(ctx),
                  imgui_make_vec2(4.0f, 48.0f), "A",
                  0xffffffffUL) == IMGUI_RESULT_OK,
              "clipped draw-list text");
        check(imgui_font_unbind(ctx) == IMGUI_RESULT_OK, "font unbind");
    }
    imgui_text(ctx, "C89 port");
    imgui_set_keyboard_focus_here(ctx, 0);
    check(imgui_input_add_mouse_position(ctx, 20.0f, 65.0f) ==
              IMGUI_RESULT_OK, "button fixture position");
    pressed = imgui_button(ctx, "Button");
    check(pressed == IMGUI_FALSE, "button waits for release");
    check(imgui_is_item_focused(ctx) == IMGUI_TRUE,
          "explicit keyboard focus request");
    check(imgui_is_item_clicked(ctx, IMGUI_MOUSE_BUTTON_LEFT),
          "last item press query");
    check(imgui_is_any_item_hovered(ctx) == IMGUI_TRUE,
          "any-item hover query");
    check(imgui_is_any_item_focused(ctx) == IMGUI_TRUE,
          "any-item focus query");
    imgui_begin_disabled(ctx);
    check(imgui_is_disabled(ctx) == IMGUI_TRUE,
          "disabled region begins");
    check(imgui_input_add_mouse_position(ctx, 20.0f, 93.0f) ==
              IMGUI_RESULT_OK, "disabled hover input position");
    check(imgui_button(ctx, "Disabled button") == IMGUI_FALSE,
          "disabled button does not activate");
    check(imgui_is_item_hovered(ctx, IMGUI_HOVERED_NONE) == IMGUI_FALSE,
          "disabled item is not hovered by default");
    check(imgui_is_item_hovered(ctx, IMGUI_HOVERED_ALLOW_WHEN_DISABLED |
                                     IMGUI_HOVERED_RECT_ONLY |
                                     IMGUI_HOVERED_ALLOW_WHEN_BLOCKED_BY_ACTIVE_ITEM) ==
              IMGUI_TRUE,
          "disabled item hover can be queried explicitly");
    check(imgui_is_item_focused(ctx) == IMGUI_FALSE,
          "disabled button does not retain keyboard focus");
    checkbox_value = IMGUI_FALSE;
    check(imgui_selectable_ex(ctx, "Disabled selectable", &checkbox_value,
                              IMGUI_SELECTABLE_DISABLED,
                              imgui_make_vec2(120.0f, 22.0f)) == IMGUI_FALSE &&
              checkbox_value == IMGUI_FALSE,
          "disabled selectable does not activate");
    check(imgui_selectable_ex(ctx, "Double-click selectable", &checkbox_value,
                              IMGUI_SELECTABLE_ALLOW_DOUBLE_CLICK,
                              imgui_make_vec2(120.0f, 22.0f)) == IMGUI_FALSE,
          "double-click selectable idle state");
    imgui_end_disabled(ctx);
    check(imgui_is_disabled(ctx) == IMGUI_FALSE,
          "disabled region ends");
    imgui_text_colored(ctx, 0xff20c080UL, "Colored text %d", 7);
    imgui_text_disabled(ctx, "Disabled text");
    check(imgui_text_link(ctx, "Documentation") == IMGUI_FALSE,
          "text link idle interaction");
    check(imgui_item_tooltip_begin(ctx) == IMGUI_SCOPE_ERROR,
          "item tooltip idle gate");
    check(imgui_arrow_button(ctx, 0x4152524fUL, IMGUI_ARROW_RIGHT) ==
              IMGUI_FALSE && imgui_get_item_rect(ctx).x2 >
              imgui_get_item_rect(ctx).x1,
          "arrow button geometry and idle state");
    check(imgui_get_item_id(ctx) != 0 &&
              imgui_get_item_rect_min(ctx).x == imgui_get_item_rect(ctx).x1 &&
              imgui_get_item_rect_min(ctx).y == imgui_get_item_rect(ctx).y1 &&
              imgui_get_item_rect_max(ctx).x == imgui_get_item_rect(ctx).x2 &&
              imgui_get_item_rect_max(ctx).y == imgui_get_item_rect(ctx).y2 &&
              imgui_get_item_rect_size(ctx).x > 0.0f &&
              imgui_get_item_rect_size(ctx).y > 0.0f,
          "item identity and rectangle queries");
    checkbox_value = IMGUI_FALSE;
    check(imgui_checkbox(ctx, "Checkbox", &checkbox_value) == IMGUI_FALSE &&
              checkbox_value == IMGUI_FALSE,
          "checkbox square control layout");
    check(imgui_radio_button(ctx, "Radio off", IMGUI_FALSE) == IMGUI_FALSE,
          "inactive radio does not activate");
    check(imgui_radio_button(ctx, "Radio on", IMGUI_TRUE) == IMGUI_FALSE,
          "active radio does not activate without click");
    imgui_text_localized(ctx, "hello");
    imgui_set_next_item_open(ctx, IMGUI_TRUE);
    scope = imgui_tree_node_begin(ctx, "Node", 0);
    check(scope == IMGUI_SCOPE_ACTIVE, "set-next-item-open tree behavior");
    check(imgui_is_item_toggled_open(ctx) == IMGUI_FALSE,
          "tree toggle query idle state");
    imgui_tree_node_end(ctx);
    (void)imgui_collapsing_header(ctx, "Header", 0);
    saved_cursor = imgui_get_cursor_screen_position(ctx);
    scope = imgui_tree_node_begin(ctx, "Open by default",
                                  IMGUI_TREE_DEFAULT_OPEN);
    check(scope == IMGUI_SCOPE_ACTIVE, "tree default-open flag");
    cursor_before_indent = imgui_get_cursor_screen_position(ctx);
    imgui_text_unformatted(ctx, "indented child", NULL);
    check(imgui_get_item_rect(ctx).x1 > saved_cursor.x,
          "open tree indents child content");
    first_id = imgui_get_id_string(ctx, "tree child control");
    scope = imgui_tree_node_begin(ctx, "Nested tree",
                                  IMGUI_TREE_DEFAULT_OPEN);
    check(scope == IMGUI_SCOPE_ACTIVE, "nested tree scope active");
    nested_id = imgui_get_id_string(ctx, "tree child control");
    check(nested_id != first_id, "tree nodes create ID scopes");
    imgui_tree_node_end(ctx);
    saved_cursor = imgui_get_cursor_screen_position(ctx);
    move_cursor = saved_cursor;
    first_id = imgui_get_id_string(ctx, "window child control");
    imgui_window_desc_init(&title_desc, "Nested state window");
    title_desc.flags = IMGUI_WINDOW_NO_MOUSE_INPUTS;
    scope = imgui_window_begin_ex(ctx, &title_desc);
    check(scope != IMGUI_SCOPE_ERROR, "nested window inside tree");
    nested_id = imgui_get_id_string(ctx, "window child control");
    check(nested_id != first_id, "windows create ID scopes");
    imgui_window_end(ctx);
    check(imgui_get_id_string(ctx, "window child control") == first_id,
          "window end restores ID scope");
    check(imgui_get_cursor_screen_position(ctx).x == saved_cursor.x,
          "nested window restores parent indentation");
    imgui_tree_node_end(ctx);
    check(imgui_get_cursor_screen_position(ctx).x == move_cursor.x -
              imgui_style_get(ctx)->indent_spacing,
          "tree end restores indentation");
    scope = imgui_tree_node_begin(ctx, "Leaf node", IMGUI_TREE_LEAF);
    check(scope == IMGUI_SCOPE_INACTIVE, "tree leaf flag");
    imgui_tree_node_end(ctx);
    imgui_set_cursor_screen_position(ctx, imgui_make_vec2(16.0f, 16.0f));
    check(imgui_child_begin(ctx, 16U,
                            imgui_make_vec2((float)strtod("nan", NULL),
                                            40.0f), 0) == IMGUI_SCOPE_ERROR,
          "non-finite child size rejected");
    check(imgui_input_add_mouse_position(ctx, 20.0f, 40.0f) ==
              IMGUI_RESULT_OK, "child scroll mouse position");
    check(imgui_input_add_mouse_wheel(ctx, 0.0f, 1.0f) == IMGUI_RESULT_OK,
          "child scroll wheel");
    child_cursor_before = imgui_get_cursor_screen_position(ctx);
    first_id = imgui_get_id_string(ctx, "child control");
    scope = imgui_child_begin(ctx, 17U, imgui_make_vec2(100.0f, 80.0f), 0);
    check(scope == IMGUI_SCOPE_ACTIVE, "child active scope");
    nested_id = imgui_get_id_string(ctx, "child control");
    check(nested_id != first_id, "child windows create ID scopes");
    check(imgui_get_cursor_screen_position(ctx).y < child_cursor_before.y,
          "child wheel scrolls content");
    check(imgui_get_scroll_y(ctx) > 0.0f &&
              imgui_get_scroll_max_y(ctx) >= 0.0f &&
              imgui_set_scroll_y(ctx, 12.0f) == IMGUI_RESULT_OK &&
              imgui_get_scroll_y(ctx) == 12.0f,
          "child scroll query and setter");
    imgui_child_end(ctx);
    check(imgui_get_id_string(ctx, "child control") == first_id,
          "child end restores ID scope");
    child_cursor_after = imgui_get_cursor_screen_position(ctx);
    check(child_cursor_after.x == child_cursor_before.x &&
              child_cursor_after.y > child_cursor_before.y,
          "child advances parent cursor");
    imgui_set_cursor_screen_position(ctx, imgui_make_vec2(16.0f, 16.0f));
    move_cursor = imgui_get_content_region_available(ctx);
    scope = imgui_child_begin(ctx, 20U,
                              imgui_make_vec2(0.0f, -20.0f), 0);
    check(scope == IMGUI_SCOPE_ACTIVE &&
              imgui_get_window_size(ctx).x >= move_cursor.x - 0.1f &&
              imgui_get_window_size(ctx).y >= move_cursor.y - 20.1f,
          "child zero and negative sizes use available content region");
    imgui_child_end(ctx);
    imgui_set_cursor_screen_position(ctx, imgui_make_vec2(16.0f, 16.0f));
    check(imgui_input_add_mouse_position(ctx, 20.0f, 38.0f) ==
              IMGUI_RESULT_OK, "clipping mouse position");
    check(imgui_input_add_mouse_wheel(ctx, 0.0f, 10.0f) == IMGUI_RESULT_OK,
          "clipping mouse wheel");
    scope = imgui_child_begin(ctx, 18U, imgui_make_vec2(30.0f, 4.0f), 0);
    check(scope == IMGUI_SCOPE_ACTIVE, "clipped child active scope");
    imgui_button(ctx, "Clipped");
    check(!imgui_is_item_hovered(ctx, 0),
          "items outside child clip are not hovered");
    check(!imgui_is_item_visible(ctx),
          "items outside child clip are not visible");
    imgui_child_end(ctx);
    check(imgui_is_item_visible(ctx),
          "child surface becomes the last visible item after child scope end");
    imgui_set_cursor_screen_position(ctx, imgui_make_vec2(16.0f, 16.0f));
    check(imgui_input_add_mouse_position(ctx, 20.0f, 20.0f) ==
              IMGUI_RESULT_OK, "horizontal child mouse position");
    scope = imgui_child_begin(ctx, 19U, imgui_make_vec2(80.0f, 50.0f),
                              IMGUI_WINDOW_HORIZONTAL_SCROLLBAR);
    check(scope == IMGUI_SCOPE_ACTIVE, "horizontal child first scope");
    imgui_text(ctx, "A long child line for horizontal scrolling");
    imgui_child_end(ctx);
    check(imgui_input_add_mouse_wheel(ctx, 1.0f, 0.0f) == IMGUI_RESULT_OK,
          "horizontal child wheel");
    imgui_set_cursor_screen_position(ctx, imgui_make_vec2(16.0f, 16.0f));
    scope = imgui_child_begin(ctx, 19U, imgui_make_vec2(80.0f, 50.0f),
                              IMGUI_WINDOW_HORIZONTAL_SCROLLBAR);
    check(scope == IMGUI_SCOPE_ACTIVE && imgui_get_scroll_max_x(ctx) > 0.0f &&
              imgui_get_scroll_x(ctx) > 0.0f &&
              imgui_set_scroll_x(ctx, 0.0f) == IMGUI_RESULT_OK &&
              imgui_get_scroll_x(ctx) == 0.0f,
          "horizontal child scroll query and setter");
    imgui_text(ctx, "A long child line for horizontal scrolling");
    imgui_child_end(ctx);
    imgui_set_cursor_screen_position(ctx, imgui_make_vec2(16.0f, 16.0f));
    scope = imgui_child_begin(ctx, 20U, imgui_make_vec2(90.0f, 40.0f),
                              IMGUI_CHILD_BORDER | IMGUI_CHILD_FRAME_STYLE);
    check(scope == IMGUI_SCOPE_ACTIVE, "bordered child active scope");
    imgui_text(ctx, "Bordered child");
    imgui_child_end(ctx);
    child_cursor_before = imgui_get_cursor_screen_position(ctx);
    scope = imgui_child_begin(
        ctx, 21U, imgui_make_vec2(20.0f, 20.0f),
        IMGUI_CHILD_AUTO_RESIZE_X | IMGUI_CHILD_AUTO_RESIZE_Y |
        IMGUI_CHILD_ALWAYS_USE_WINDOW_PADDING);
    check(scope == IMGUI_SCOPE_ACTIVE, "auto-resize child active scope");
    imgui_text(ctx, "Auto-resized child content");
    imgui_child_end(ctx);
    child_cursor_after = imgui_get_cursor_screen_position(ctx);
    check(child_cursor_after.x == child_cursor_before.x &&
              child_cursor_after.y - child_cursor_before.y > 20.0f,
          "auto-resize child expands parent layout");
    scope = imgui_menu_bar_begin(ctx);
    check(scope == IMGUI_SCOPE_ACTIVE, "menu bar active scope");
    imgui_menu_bar_end(ctx);
    scope = imgui_tooltip_begin(ctx);
    check(scope == IMGUI_SCOPE_ACTIVE, "tooltip active scope");
    imgui_text(ctx, "Tooltip");
    imgui_tooltip_end(ctx);
    imgui_set_tooltip(ctx, "Formatted tooltip %d", 3);
    imgui_set_item_tooltip(ctx, "Hovered item tooltip");
    cursor_before_indent = imgui_get_cursor_screen_position(ctx);
    imgui_group_begin(ctx);
    imgui_button(ctx, "Grouped");
    imgui_same_line(ctx);
    imgui_button(ctx, "Item");
    imgui_group_end(ctx);
    cursor_after_indent = imgui_get_cursor_screen_position(ctx);
    check(cursor_after_indent.x == cursor_before_indent.x &&
              cursor_after_indent.y > cursor_before_indent.y,
          "group restores parent horizontal layout");
    scope_error_count = reported_errors;
    for (scope_index = 0; scope_index < 200; ++scope_index) {
        imgui_group_begin(ctx);
    }
    for (scope_index = 0; scope_index < 200; ++scope_index) {
        imgui_group_end(ctx);
    }
    check(reported_errors == scope_error_count,
          "deep scope nesting is allocator-backed");
    cursor_before_indent = imgui_get_cursor_screen_position(ctx);
    imgui_indent(ctx, 10.0f);
    cursor_after_indent = imgui_get_cursor_screen_position(ctx);
    check(cursor_after_indent.x > cursor_before_indent.x,
          "indent advances cursor");
    imgui_unindent(ctx, 10.0f);
    imgui_separator(ctx);
    scope = imgui_tab_bar_begin(ctx, "Tabs", 0);
    check(scope == IMGUI_SCOPE_ACTIVE, "tab bar active scope");
    first_id = imgui_get_id_string(ctx, "tab child control");
    scope = imgui_tab_item_begin(ctx, "First", NULL, 0);
    check(scope == IMGUI_SCOPE_ACTIVE, "tab item active scope");
    nested_id = imgui_get_id_string(ctx, "tab child control");
    check(nested_id != first_id, "tab items create ID scopes");
    imgui_tab_item_end(ctx);
    scope = imgui_tab_item_begin(ctx, "Disabled tab", NULL,
                                 IMGUI_TAB_ITEM_DISABLED);
    check(scope == IMGUI_SCOPE_INACTIVE, "disabled tab is inactive");
    imgui_tab_item_end(ctx);
    tab_open = IMGUI_TRUE;
    scope = imgui_tab_item_begin(ctx, "Closable unsaved", &tab_open,
                                 IMGUI_TAB_ITEM_UNSAVED_DOCUMENT);
    check(scope != IMGUI_SCOPE_ERROR && tab_open,
          "closable unsaved tab available");
    imgui_tab_item_end(ctx);
    check(imgui_get_id_string(ctx, "tab child control") == first_id,
          "tab item end restores ID scope");
    imgui_tab_bar_end(ctx);
    first_id = imgui_get_id_string(ctx, "list child control");
    cursor_before_indent = imgui_get_cursor_screen_position(ctx);
    scope = imgui_list_box_begin(ctx, "List", imgui_make_vec2(120.0f, 60.0f));
    check(scope == IMGUI_SCOPE_ACTIVE, "list box active scope");
    nested_id = imgui_get_id_string(ctx, "list child control");
    check(nested_id != first_id, "list boxes create ID scopes");
    imgui_list_box_end(ctx);
    check(imgui_get_id_string(ctx, "list child control") == first_id,
          "list box end restores ID scope");
    cursor_after_indent = imgui_get_cursor_screen_position(ctx);
    check(cursor_after_indent.x == cursor_before_indent.x &&
              cursor_after_indent.y > cursor_before_indent.y,
          "list box restores parent layout");
    first_id = imgui_get_id_string(ctx, "table child control");
    cursor_before_indent = imgui_get_cursor_screen_position(ctx);
    scope = imgui_table_begin(ctx, "Data", 2,
                              IMGUI_TABLE_BORDERS | IMGUI_TABLE_ROW_BACKGROUND);
    check(scope == IMGUI_SCOPE_ACTIVE, "table active scope");
    check(imgui_table_setup_column(ctx, 0, "Name", IMGUI_TABLE_NONE) ==
              IMGUI_RESULT_OK &&
              imgui_table_setup_column(ctx, 1, "Value", IMGUI_TABLE_NONE) ==
              IMGUI_RESULT_OK,
          "table column setup");
    check(imgui_table_setup_scroll_freeze(ctx, 1, 1) == IMGUI_RESULT_OK,
          "table scroll freeze setup");
    check(imgui_table_get_column_count(ctx) == 2 &&
              imgui_table_get_column_index(ctx) == 0 &&
              imgui_table_get_row_index(ctx) == 0 &&
              strcmp(imgui_table_get_column_name(ctx, 0), "Name") == 0 &&
              imgui_table_get_column_width(ctx, 0) > 0.0f &&
              imgui_table_get_column_flags(ctx, 0) == IMGUI_TABLE_NONE,
          "table column metadata queries");
    imgui_table_headers_row(ctx);
    check(imgui_input_add_mouse_position(
              ctx, cursor_before_indent.x + 5.0f,
              cursor_before_indent.y + 5.0f) == IMGUI_RESULT_OK &&
              imgui_table_get_hovered_column(ctx) == 0,
          "table hovered column query");
    sort_column = -1;
    sort_direction = 0;
    sort_specs = imgui_table_get_sort_specs(ctx);
    check(!imgui_table_get_sort(ctx, &sort_column, &sort_direction) &&
              sort_column == -1 && sort_direction == 0 &&
              sort_specs != NULL && sort_specs->count == 0 &&
              sort_specs->specs != NULL,
          "table sort starts inactive");
    nested_id = imgui_get_id_string(ctx, "table child control");
    check(nested_id != first_id, "tables create ID scopes");
    imgui_table_next_column(ctx);
    imgui_table_set_column_width(ctx, 80.0f);
    imgui_text(ctx, "A");
    imgui_table_next_column(ctx);
    imgui_text(ctx, "B");
    imgui_table_next_row(ctx);
    imgui_table_set_column_index(ctx, 1);
    imgui_text(ctx, "C");
    imgui_table_set_column_index(ctx, 0);
    imgui_text(ctx, "D");
    imgui_table_end(ctx);
    check(imgui_get_id_string(ctx, "table child control") == first_id,
          "table end restores ID scope");
    cursor_after_indent = imgui_get_cursor_screen_position(ctx);
    check(cursor_after_indent.x == cursor_before_indent.x &&
              cursor_after_indent.y > cursor_before_indent.y,
          "table consumes parent layout");

    first_id = imgui_get_id_string(ctx, "same");
    crc_id = imgui_get_id_string(ctx, "abc");
    check(crc_id != (imgui_id)0, "window-scoped ID hash");
    check(imgui_get_id_string(ctx, "First###Stable") ==
              imgui_get_id_string(ctx, "Second###Stable"),
          "stable ### label ID");
    stable_first = "First###Stable";
    stable_second = "Second###Stable";
    check(imgui_get_id_range(ctx, stable_first,
                             stable_first + strlen(stable_first)) ==
              imgui_get_id_range(ctx, stable_second,
                                 stable_second + strlen(stable_second)),
          "stable ### range ID");
    check(imgui_get_id_string(ctx, "First##one") !=
              imgui_get_id_string(ctx, "First##two"),
          "hidden ## label IDs remain distinct");
    imgui_push_id_integer(ctx, 7);
    nested_id = imgui_get_id_string(ctx, "same");
    check(first_id != nested_id, "ID stack changes identity");
    imgui_pop_id(ctx);
    id_error_count = reported_errors;
    for (id_index = 0; id_index < 200; ++id_index) {
        imgui_push_id_integer(ctx, id_index);
    }
    for (id_index = 0; id_index < 200; ++id_index) {
        imgui_pop_id(ctx);
    }
    check(reported_errors == id_error_count,
          "deep ID nesting is allocator-backed");

    imgui_texture_desc_init(&texture_desc);
    texture_desc.width = 16;
    texture_desc.height = 16;
    texture_desc.struct_size = offsetof(imgui_texture_desc, debug_name);
    check(imgui_texture_register_external(ctx, (imgui_texture_id)ctx,
                                          &texture_desc,
                                          &source_texture) == IMGUI_RESULT_OK,
          "source texture registration");
    texture_desc.struct_size = sizeof(texture_desc);
    check(imgui_texture_register_external(ctx, (imgui_texture_id)&frame,
                                          &texture_desc,
                                          &destination_texture) ==
              IMGUI_RESULT_OK,
          "destination texture registration");
    software_source_texture = source_texture;
    software_destination_texture = destination_texture;
    memset(software_source_pixels, 0, sizeof(software_source_pixels));
    memset(software_destination_pixels, 0,
           sizeof(software_destination_pixels));
    software_source_pixels[0] = 17;
    software_source_pixels[1] = 34;
    software_source_pixels[2] = 51;
    software_source_pixels[3] = 255;
    imgui_image(ctx, source_texture, imgui_make_vec2(16.0f, 16.0f));
    check(imgui_texture_destroy(ctx, source_texture) ==
              IMGUI_RESULT_INVALID_STATE,
          "texture destroy rejects same-frame draw dependency");
    custom_rect.x1 = 40.0f;
    custom_rect.y1 = 300.0f;
    custom_rect.x2 = 56.0f;
    custom_rect.y2 = 316.0f;
    check(imgui_draw_list_add_image(
              ctx, imgui_get_window_draw_list(ctx), source_texture,
              custom_rect, imgui_make_vec2(0.0f, 0.0f),
              imgui_make_vec2(1.0f, 1.0f), 0xffffffffUL) == IMGUI_RESULT_OK,
          "public draw-list image");
    check(imgui_draw_list_add_image_quad(
              ctx, imgui_get_window_draw_list(ctx), source_texture,
              imgui_make_vec2(80.0f, 300.0f),
              imgui_make_vec2(100.0f, 300.0f),
              imgui_make_vec2(104.0f, 320.0f),
              imgui_make_vec2(76.0f, 320.0f),
              imgui_make_vec2(0.0f, 0.0f), imgui_make_vec2(1.0f, 0.0f),
              imgui_make_vec2(1.0f, 1.0f), imgui_make_vec2(0.0f, 1.0f),
              0xffffffffUL) == IMGUI_RESULT_OK,
          "public draw-list image quad");
    check(imgui_image_button(ctx, imgui_get_id_string(ctx, "ImageButton"),
                             destination_texture,
                             imgui_make_vec2(16.0f, 16.0f)) ==
              IMGUI_FALSE, "image button draw");
    imgui_progress_bar(ctx, 0.5f);
    color_rgba[0] = 0.2f;
    color_rgba[1] = 0.4f;
    color_rgba[2] = 0.6f;
    color_rgba[3] = 0.37f;
    color_rgba[0] = (float)strtod("nan", NULL);
    check(imgui_color_edit_rgba(ctx, "Invalid color", color_rgba,
                                IMGUI_COLOR_EDIT_NONE) == IMGUI_FALSE,
          "non-finite color rejected");
    color_rgba[0] = 0.2f;
    check(imgui_color_rgba_to_u32(color_rgba) == 0x5e996633UL,
          "public RGBA color packing");
    imgui_color_u32_to_rgba(0x5e996633UL, color_rgba);
    check(fabs((double)color_rgba[0] - 0.2) < 0.01 &&
              fabs((double)color_rgba[1] - 0.4) < 0.01 &&
              fabs((double)color_rgba[2] - 0.6) < 0.01 &&
              fabs((double)color_rgba[3] - 0.37) < 0.01,
          "public RGBA color unpacking");
    {
        float rgb[3];
        float hue;
        float saturation;
        float value;
        rgb[0] = 1.0f; rgb[1] = 0.0f; rgb[2] = 0.0f;
        imgui_color_rgb_to_hsv(rgb, &hue, &saturation, &value);
        check(fabs((double)hue) < 0.001 &&
                  fabs((double)saturation - 1.0) < 0.001 &&
                  fabs((double)value - 1.0) < 0.001,
              "public RGB to HSV conversion");
        imgui_color_hsv_to_rgb(1.0f / 3.0f, 1.0f, 1.0f, rgb);
        check(fabs((double)rgb[0]) < 0.001 &&
                  fabs((double)rgb[1] - 1.0) < 0.001 &&
                  fabs((double)rgb[2]) < 0.001,
              "public HSV to RGB conversion");
    }
    color_rgba[3] = 0.37f;
          check(imgui_color_edit_rgba(ctx, "Tint", color_rgba,
                                IMGUI_COLOR_EDIT_NO_INPUTS |
                                IMGUI_COLOR_EDIT_NO_ALPHA) == IMGUI_FALSE &&
              color_rgba[3] == 0.37f,
          "color editor no-alpha state");
    check(imgui_color_edit_rgba(ctx, "Picker", color_rgba,
                                IMGUI_COLOR_EDIT_PICKER |
                                IMGUI_COLOR_EDIT_NO_INPUTS) == IMGUI_FALSE,
          "color picker no-input state");
    color_rgb[0] = 0.15f;
    color_rgb[1] = 0.35f;
    color_rgb[2] = 0.55f;
    check(imgui_color_edit_rgb(ctx, "RGB editor", color_rgb,
                               IMGUI_COLOR_EDIT_NO_INPUTS) == IMGUI_FALSE &&
              color_rgb[0] >= 0.0f && color_rgb[2] <= 1.0f,
          "RGB editor wrapper");
    check(imgui_color_picker_rgba(ctx, "RGBA picker", color_rgba,
                                  IMGUI_COLOR_EDIT_NO_INPUTS) == IMGUI_FALSE,
          "RGBA picker wrapper");
    check(imgui_color_picker_rgb(ctx, "RGB picker", color_rgb,
                                 IMGUI_COLOR_EDIT_NO_INPUTS) == IMGUI_FALSE,
          "RGB picker wrapper");
    check(imgui_color_button(ctx, (imgui_id)0x54494e54UL, color_rgba,
                             imgui_make_vec2(18.0f, 18.0f)) == IMGUI_FALSE,
          "color button no-click state");
    custom_rect.x1 = 16.0f;
    custom_rect.y1 = 300.0f;
    custom_rect.x2 = 32.0f;
    custom_rect.y2 = 316.0f;
    custom_rect.x1 = (float)strtod("nan", NULL);
    check(imgui_draw_list_add_rect(
              ctx, imgui_get_window_draw_list(ctx), custom_rect,
              0xff00ffffUL, NULL) == IMGUI_RESULT_INVALID_ARGUMENT,
          "non-finite draw rectangle rejected");
    custom_rect.x1 = 16.0f;
    check(imgui_draw_list_add_rect(
              ctx, imgui_get_window_draw_list(ctx), custom_rect,
              0xff00ffffUL, NULL) == IMGUI_RESULT_OK,
          "public draw-list rectangle");
    check(imgui_draw_list_add_line(
              ctx, imgui_get_window_draw_list(ctx),
              imgui_make_vec2(16.0f, 300.0f),
              imgui_make_vec2(32.0f, 316.0f), 0xffffffffUL,
              (float)strtod("nan", NULL)) == IMGUI_RESULT_INVALID_ARGUMENT,
          "non-finite draw thickness rejected");
    custom_rect.x1 = 40.0f;
    custom_rect.y1 = 300.0f;
    custom_rect.x2 = 72.0f;
    custom_rect.y2 = 324.0f;
    check(imgui_draw_list_add_rect_rounded(
              ctx, imgui_get_window_draw_list(ctx), custom_rect, 5.0f,
              0xff40c0ffUL, 3, NULL) == IMGUI_RESULT_OK,
          "public draw-list rounded rectangle");
    check(imgui_draw_list_add_line(
              ctx, imgui_get_window_draw_list(ctx),
              imgui_make_vec2(60.0f, 300.0f),
              imgui_make_vec2(100.0f, 320.0f), 0xffffffffUL, 2.0f) ==
              IMGUI_RESULT_OK, "public draw-list line");
    check(imgui_draw_list_add_triangle_filled(
              ctx, imgui_get_window_draw_list(ctx),
              imgui_make_vec2(190.0f, 300.0f),
              imgui_make_vec2(210.0f, 320.0f),
              imgui_make_vec2(170.0f, 320.0f), 0xff40ffffUL) ==
              IMGUI_RESULT_OK &&
              imgui_draw_list_add_triangle(
                  ctx, imgui_get_window_draw_list(ctx),
                  imgui_make_vec2(230.0f, 300.0f),
                  imgui_make_vec2(250.0f, 320.0f),
                  imgui_make_vec2(210.0f, 320.0f), 0xffffffffUL, 1.0f) ==
              IMGUI_RESULT_OK,
          "public draw-list triangle primitives");
    check(imgui_draw_list_add_quad_filled(
              ctx, imgui_get_window_draw_list(ctx),
              imgui_make_vec2(260.0f, 300.0f),
              imgui_make_vec2(290.0f, 300.0f),
              imgui_make_vec2(290.0f, 320.0f),
              imgui_make_vec2(260.0f, 320.0f), 0xff80c040UL) ==
              IMGUI_RESULT_OK &&
              imgui_draw_list_add_quad(
                  ctx, imgui_get_window_draw_list(ctx),
                  imgui_make_vec2(300.0f, 300.0f),
                  imgui_make_vec2(330.0f, 300.0f),
                  imgui_make_vec2(330.0f, 320.0f),
                  imgui_make_vec2(300.0f, 320.0f), 0xffffffffUL, 1.0f) ==
              IMGUI_RESULT_OK,
          "public draw-list quad primitives");
    check(imgui_draw_list_add_circle(
              ctx, imgui_get_window_draw_list(ctx),
              imgui_make_vec2(130.0f, 308.0f), 8.0f, 0xffff00ffUL, 16) ==
              IMGUI_RESULT_OK, "public draw-list circle");
    polyline[0] = imgui_make_vec2(150.0f, 300.0f);
    polyline[1] = imgui_make_vec2(165.0f, 320.0f);
    polyline[2] = imgui_make_vec2(180.0f, 300.0f);
    check(imgui_draw_list_add_polyline(
              ctx, imgui_get_window_draw_list(ctx), polyline, 3,
              0xff00ffffUL, 1.5f, IMGUI_FALSE) == IMGUI_RESULT_OK,
          "public draw-list polyline");
    check(imgui_draw_list_path_begin(ctx, imgui_get_window_draw_list(ctx)) ==
              IMGUI_RESULT_OK, "public draw-list path begin");
    check(imgui_draw_list_path_line_to(
              ctx, imgui_get_window_draw_list(ctx),
              imgui_make_vec2(190.0f, 300.0f)) == IMGUI_RESULT_OK,
          "public draw-list path first point");
    check(imgui_draw_list_path_line_to(
              ctx, imgui_get_window_draw_list(ctx),
              imgui_make_vec2(220.0f, 300.0f)) == IMGUI_RESULT_OK,
          "public draw-list path second point");
    check(imgui_draw_list_path_line_to(
              ctx, imgui_get_window_draw_list(ctx),
              imgui_make_vec2(205.0f, 325.0f)) == IMGUI_RESULT_OK,
          "public draw-list path third point");
    check(imgui_draw_list_path_fill(ctx, imgui_get_window_draw_list(ctx),
                                    0xff40a0ffUL, NULL) == IMGUI_RESULT_OK,
          "public draw-list path fill");
    check(imgui_draw_list_path_begin(ctx, imgui_get_window_draw_list(ctx)) ==
              IMGUI_RESULT_OK, "public draw-list arc path begin");
    check(imgui_draw_list_path_arc_to(
              ctx, imgui_get_window_draw_list(ctx),
              imgui_make_vec2(250.0f, 312.0f), 12.0f, 0.0f, 3.1415926f,
              6) == IMGUI_RESULT_OK, "public draw-list path arc");
    check(imgui_draw_list_path_stroke(ctx, imgui_get_window_draw_list(ctx),
                                      0xffffc040UL, 2.0f,
                                      IMGUI_FALSE) == IMGUI_RESULT_OK,
          "public draw-list path stroke");
    path_result = imgui_draw_list_path_begin(
        ctx, imgui_get_window_draw_list(ctx));
    for (path_index = 0; path_index < 600 && path_result == IMGUI_RESULT_OK;
         ++path_index) {
        path_result = imgui_draw_list_path_line_to(
            ctx, imgui_get_window_draw_list(ctx),
            imgui_make_vec2(280.0f + (float)(path_index % 100),
                            300.0f + (float)(path_index / 100)));
    }
    if (path_result == IMGUI_RESULT_OK) {
        path_result = imgui_draw_list_path_stroke(
            ctx, imgui_get_window_draw_list(ctx), 0xff80ff80UL, 1.0f,
            IMGUI_FALSE);
    }
    check(path_result == IMGUI_RESULT_OK,
          "dynamic draw-list path beyond initial capacity");
    plot_values[0] = 0.0f;
    plot_values[1] = 1.0f;
    plot_values[2] = 0.5f;
    imgui_plot_lines(ctx, "Plot", plot_values, 3,
                     imgui_make_vec2(120.0f, 40.0f), 0.0f, 1.0f);
    imgui_plot_histogram(ctx, "Histogram", plot_values, 3,
                         imgui_make_vec2(120.0f, 40.0f), 0.0f, 1.0f);
    invalid_widget_value = (float)strtod("nan", NULL);
    check(imgui_slider_float(ctx, "Invalid slider", &invalid_widget_value,
                             0.0f, 1.0f) == IMGUI_FALSE,
          "non-finite slider value rejected");
    imgui_progress_bar_ex(ctx, invalid_widget_value,
                          imgui_make_vec2(120.0f, 20.0f), NULL);
    plot_values[1] = invalid_widget_value;
    imgui_plot_lines(ctx, "Invalid plot", plot_values, 3,
                     imgui_make_vec2(120.0f, 40.0f), 0.0f, 1.0f);
    imgui_plot_histogram(ctx, "Invalid histogram", plot_values, 3,
                         imgui_make_vec2(120.0f, 40.0f), 0.0f, 1.0f);
    plot_values[1] = 1.0f;
    memset(&copy, 0, sizeof(copy));
    copy.source = source_texture;
    copy.destination = destination_texture;
    copy.source_region.width = 4;
    copy.source_region.height = 4;
    check(imgui_draw_list_add_texture_copy(
              ctx, imgui_get_window_draw_list(ctx), &copy) == IMGUI_RESULT_OK,
          "ordered texture copy");
    copy.source_region.x = 14;
    check(imgui_draw_list_add_texture_copy(
              ctx, imgui_get_window_draw_list(ctx), &copy) ==
              IMGUI_RESULT_INVALID_ARGUMENT,
          "out-of-bounds texture copy rejected");
    copy.source_region.x = INT_MAX;
    check(imgui_draw_list_add_texture_copy(
              ctx, imgui_get_window_draw_list(ctx), &copy) ==
              IMGUI_RESULT_INVALID_ARGUMENT,
          "overflowing texture copy rejected");
    copy.source_region.x = 0;
    check(imgui_texture_update(ctx, source_texture, INT_MAX, 0, 1, 1,
                               texture_pixels, 4U) ==
              IMGUI_RESULT_INVALID_ARGUMENT,
          "overflowing texture update rejected");
    check(imgui_draw_list_add_custom_command(
              ctx, imgui_get_window_draw_list(ctx), 42U, custom_payload,
              sizeof(custom_payload)) == IMGUI_RESULT_OK,
          "copied custom command");
    copy.destination = source_texture;
    copy.source_region.width = 2;
    copy.source_region.height = 1;
    copy.destination_x = 1;
    copy.destination_y = 0;
    check(imgui_draw_list_add_texture_copy(
              ctx, imgui_get_window_draw_list(ctx), &copy) == IMGUI_RESULT_OK,
          "overlapping texture copy");
    copy.destination = destination_texture;
    copy.source_region.width = 4;
    copy.source_region.height = 4;
    copy.destination_x = 0;
    copy.destination_y = 0;
    memset(&clear, 0, sizeof(clear));
    clear.texture = destination_texture;
    clear.region.x = 8;
    clear.region.y = 8;
    clear.region.width = 2;
    clear.region.height = 2;
    clear.color = 0x11223344UL;
    check(imgui_draw_list_add_texture_clear(
              ctx, imgui_get_window_draw_list(ctx), &clear) ==
              IMGUI_RESULT_OK, "ordered texture clear");
    memset(&ordered_update, 0, sizeof(ordered_update));
    ordered_update.texture = destination_texture;
    ordered_update.region.x = 4;
    ordered_update.region.y = 4;
    ordered_update.region.width = 1;
    ordered_update.region.height = 1;
    ordered_update.format = IMGUI_TEXTURE_FORMAT_RGBA8;
    ordered_update.pixels = update_pixels;
    ordered_update.pitch = 4U;
    check(imgui_draw_list_add_texture_update(
              ctx, imgui_get_window_draw_list(ctx), &ordered_update) ==
              IMGUI_RESULT_OK, "ordered texture update");
    check(imgui_texture_destroy(ctx, destination_texture) ==
              IMGUI_RESULT_INVALID_STATE,
          "texture destroy rejects ordered update dependency");
    check(imgui_draw_list_add_sampler(
              ctx, imgui_get_window_draw_list(ctx),
              (imgui_sampler)99, 0U) == IMGUI_RESULT_INVALID_ARGUMENT,
          "invalid sampler rejected");
    check(imgui_draw_list_add_sampler(
              ctx, imgui_get_window_draw_list(ctx), IMGUI_SAMPLER_NEAREST,
              0U) == IMGUI_RESULT_OK,
          "nearest sampler command");
    check(imgui_draw_list_split(ctx, imgui_get_window_draw_list(ctx), 3) ==
              IMGUI_RESULT_OK, "draw-list channel split");
    check(imgui_draw_list_set_channel(ctx, imgui_get_window_draw_list(ctx), 2) ==
              IMGUI_RESULT_OK, "draw-list channel select high");
    check(imgui_draw_list_add_custom_command(
              ctx, imgui_get_window_draw_list(ctx), 100U,
              custom_payload, sizeof(custom_payload)) == IMGUI_RESULT_OK,
          "draw-list high channel command");
    check(imgui_draw_list_set_channel(ctx, imgui_get_window_draw_list(ctx), 1) ==
              IMGUI_RESULT_OK, "draw-list channel select middle");
    check(imgui_draw_list_add_reset_state(
              ctx, imgui_get_window_draw_list(ctx)) == IMGUI_RESULT_OK,
          "draw-list middle channel command");
    check(imgui_draw_list_set_channel(ctx, imgui_get_window_draw_list(ctx), 3) ==
              IMGUI_RESULT_INVALID_ARGUMENT,
          "draw-list invalid channel rejected");
    check(imgui_draw_list_merge(ctx, imgui_get_window_draw_list(ctx)) ==
              IMGUI_RESULT_OK, "draw-list channel merge");

    scope = imgui_combo_begin(ctx, "Mode", "One");
    check(scope != IMGUI_SCOPE_ERROR, "inactive combo begin");
    imgui_combo_end(ctx);
    scope = imgui_combo_begin_ex(ctx, "Compact mode", "Two",
                                 IMGUI_COMBO_NO_ARROW_BUTTON |
                                 IMGUI_COMBO_NO_PREVIEW);
    check(scope != IMGUI_SCOPE_ERROR, "combo flag variants");
    imgui_combo_end(ctx);
    first_id = imgui_get_id_string(ctx, "popup child control");
    imgui_popup_open(ctx, "TestPopup", 0);
    check(imgui_get_mouse_position_on_opening_current_popup(ctx).x ==
              imgui_get_mouse_position(ctx).x &&
          imgui_get_mouse_position_on_opening_current_popup(ctx).y ==
              imgui_get_mouse_position(ctx).y,
          "popup opening mouse position query");
    scope = imgui_popup_begin(ctx, "TestPopup", IMGUI_WINDOW_NONE);
    check(scope == IMGUI_SCOPE_ACTIVE, "popup active scope");
    nested_id = imgui_get_id_string(ctx, "popup child control");
    check(nested_id != first_id, "popups create ID scopes");
    imgui_popup_open(ctx, "NestedPopup", IMGUI_POPUP_NONE);
    scope = imgui_popup_begin(ctx, "NestedPopup", IMGUI_WINDOW_NONE);
    check(scope == IMGUI_SCOPE_ACTIVE, "nested popup active scope");
    imgui_popup_end(ctx);
    imgui_text(ctx, "Parent popup restored");
    imgui_text(ctx, "Popup");
    check(imgui_menu_item_shortcut(ctx, "Shortcut", "F12",
                                   IMGUI_KEY_F12, IMGUI_KEY_MOD_NONE,
                                   IMGUI_TRUE) == IMGUI_TRUE,
          "keyboard menu shortcut activation");
    check(imgui_menu_item(ctx, "Disabled menu item", NULL,
                          IMGUI_FALSE) == IMGUI_FALSE,
          "disabled menu item is non-interactive");
    scope = imgui_menu_begin(ctx, "Disabled menu", IMGUI_FALSE);
    check(scope == IMGUI_SCOPE_INACTIVE,
          "disabled menu header is non-interactive");
    imgui_menu_end(ctx);
    imgui_popup_end(ctx);
    check(imgui_get_id_string(ctx, "popup child control") == first_id,
          "popup end restores ID scope");
    imgui_window_end(ctx);

    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "frame end");
    frame.struct_size = sizeof(frame);
    packet = imgui_render(ctx);

        check(packet != NULL, "render packet");
    if (packet != NULL) {
        check(packet->protocol_version == 1, "render protocol version");
        check(packet->frame_index == 2, "render frame index");
        check(packet->resource_operation_count == 0,
              "resource prelude is not replayed");
        check(imgui_is_window_hovered(ctx, 0), "window hover query");
        check(imgui_is_window_focused(ctx, 0), "window focus query");
        check(imgui_is_window_hovered(ctx, IMGUI_HOVERED_ANY_WINDOW),
              "any-window hover query");
        check(imgui_is_window_focused(ctx, IMGUI_FOCUSED_ANY_WINDOW),
              "any-window focus query");
        check(imgui_get_frame_output(ctx)->want_capture_mouse,
              "mouse capture output");
        check(imgui_get_frame_output(ctx)->want_capture_keyboard,
              "keyboard capture output");
        check(packet->viewport_count == 1, "viewport packet");
        found_copy = IMGUI_FALSE;
        found_custom = IMGUI_FALSE;
        found_font_draw = IMGUI_FALSE;
        found_font_update = IMGUI_FALSE;
        found_image_button_uv = IMGUI_FALSE;
        font_geometry_clipped = IMGUI_TRUE;
        found_style_color = IMGUI_FALSE;
        custom_index = -1;
        channel_middle_index = -1;
        channel_high_index = -1;
        for (command_index = 0;
             command_index < (int)packet->viewports[0].lists[0].command_count;
             ++command_index) {
            if (packet->viewports[0].lists[0].commands[command_index].type ==
                IMGUI_RENDER_COMMAND_DRAW_INDEXED) {
                check(packet->viewports[0].lists[0].commands[command_index]
                          .data.draw_indexed.clip_rect.x1 >= 0.0f,
                      "draw clip left bound");
                if (font_texture != NULL &&
                    packet->viewports[0].lists[0].commands[command_index]
                        .data.draw_indexed.texture == font_texture) {
                    found_font_draw = IMGUI_TRUE;
                    for (path_index = 0; path_index < 4; ++path_index) {
                        imgui_u32 vertex_index =
                            packet->viewports[0].lists[0]
                                .commands[command_index]
                                .data.draw_indexed.vertex_offset +
                            (imgui_u32)path_index;
                        if (vertex_index >= packet->viewports[0].lists[0]
                                .vertex_count ||
                            packet->viewports[0].lists[0].vertices[vertex_index]
                                .position.x < packet->viewports[0].lists[0]
                                .commands[command_index].data.draw_indexed
                                .clip_rect.x1 ||
                            packet->viewports[0].lists[0].vertices[vertex_index]
                                .position.y < packet->viewports[0].lists[0]
                                .commands[command_index].data.draw_indexed
                                .clip_rect.y1) {
                            font_geometry_clipped = IMGUI_FALSE;
                        }
                    }
                }
                if (packet->viewports[0].lists[0]
                        .vertices[packet->viewports[0].lists[0]
                            .commands[command_index]
                            .data.draw_indexed.vertex_offset].color ==
                    0xff123456UL) {
                    found_style_color = IMGUI_TRUE;
                }
                if (packet->viewports[0].lists[0].commands[command_index]
                        .data.draw_indexed.texture == destination_texture) {
                    imgui_u32 image_vertex = packet->viewports[0].lists[0]
                        .commands[command_index].data.draw_indexed.vertex_offset;
                    if (image_vertex + 3U < packet->viewports[0].lists[0]
                            .vertex_count &&
                        packet->viewports[0].lists[0].vertices[image_vertex]
                            .uv.x == 0.0f &&
                        packet->viewports[0].lists[0].vertices[image_vertex + 1U]
                            .uv.x == 1.0f &&
                        packet->viewports[0].lists[0].vertices[image_vertex + 2U]
                            .uv.y == 1.0f) {
                        found_image_button_uv = IMGUI_TRUE;
                    }
                }
            }
            if (packet->viewports[0].lists[0].commands[command_index].type ==
                IMGUI_RENDER_COMMAND_TEXTURE_COPY) found_copy = IMGUI_TRUE;
            if (packet->viewports[0].lists[0].commands[command_index].type ==
                IMGUI_RENDER_COMMAND_TEXTURE_UPDATE &&
                packet->viewports[0].lists[0].commands[command_index]
                    .data.texture_update.texture == font_texture) {
                found_font_update = IMGUI_TRUE;
            }
            if (packet->viewports[0].lists[0].commands[command_index].type ==
                IMGUI_RENDER_COMMAND_CUSTOM) {
                found_custom = IMGUI_TRUE;
                custom_index = command_index;
                if (packet->viewports[0].lists[0].commands[command_index]
                        .data.custom.command_id == 100U) {
                    channel_high_index = command_index;
                }
            }
            if (packet->viewports[0].lists[0].commands[command_index].type ==
                IMGUI_RENDER_COMMAND_RESET_STATE) {
                channel_middle_index = command_index;
            }
        }
        check(packet->viewports[0].lists[0].command_count >= 3,
              "ordered command packet");
        check(found_copy,
              "texture copy command type");
        check(found_image_button_uv,
              "image button uses full texture UVs");
        check(found_style_color, "style button color in render packet");
        if (font != NULL && font_texture != NULL) {
            check(found_font_draw, "font text draw command type");
            check(found_font_update, "ordered font atlas update command type");
            check(font_geometry_clipped, "font glyph geometry is clipped");
        }
        check(found_custom,
              "custom command type");
        check(channel_middle_index >= 0 && channel_high_index >
                  channel_middle_index,
              "merged channel ordering");
        if (custom_index >= 0) {
            check(packet->viewports[0].lists[0].commands[custom_index]
                      .data.custom.payload != custom_payload,
                  "custom payload copied");
        }
        cloned_packet = NULL;
        check(imgui_render_packet_clone(packet, &config.allocator,
                                        &cloned_packet) == IMGUI_RESULT_OK,
              "render packet clone");
        check(cloned_packet != NULL && cloned_packet->owned,
              "owned render packet clone");
        memset(&resource_packet_left, 0, sizeof(resource_packet_left));
        memset(&resource_packet_right, 0, sizeof(resource_packet_right));
        memset(&partial_upload_left, 0, sizeof(partial_upload_left));
        memset(&partial_upload_right, 0, sizeof(partial_upload_right));
        memset(partial_upload_a, 0x11, sizeof(partial_upload_a));
        memset(partial_upload_b, 0x22, sizeof(partial_upload_b));
        memcpy(partial_upload_b, partial_upload_a, 4U);
        partial_upload_left.type = IMGUI_RESOURCE_UPLOAD_TEXTURE;
        partial_upload_left.texture = owned_texture;
        partial_upload_left.region.width = 1;
        partial_upload_left.region.height = 1;
        partial_upload_left.format = IMGUI_TEXTURE_FORMAT_RGBA8;
        partial_upload_left.texture_width = 2;
        partial_upload_left.texture_height = 2;
        partial_upload_left.pixels = partial_upload_a;
        partial_upload_left.pitch = 4U;
        partial_upload_right = partial_upload_left;
        partial_upload_right.pixels = partial_upload_b;
        resource_packet_left.struct_size = sizeof(resource_packet_left);
        resource_packet_left.protocol_version = 1;
        resource_packet_left.resource_operations = &partial_upload_left;
        resource_packet_left.resource_operation_count = 1;
        resource_packet_right = resource_packet_left;
        resource_packet_right.resource_operations = &partial_upload_right;
        imgui_software_packet_diff_init(&packet_diff);
        check(imgui_software_compare_packets(&resource_packet_left,
                                             &resource_packet_right,
                                             &packet_diff) == IMGUI_RESULT_OK &&
                  packet_diff.differing_resources == 0,
              "partial upload diff uses region height");
        malformed_packet = *packet;
        malformed_viewport = packet->viewports[0];
        malformed_viewport.lists = NULL;
        malformed_viewport.list_count = 1;
        malformed_packet.viewports = &malformed_viewport;
        malformed_packet.viewport_count = 1;
        check(imgui_render_packet_clone(&malformed_packet,
                                        &config.allocator,
                                        &malformed_packet_clone) ==
                  IMGUI_RESULT_INVALID_ARGUMENT,
              "render packet clone rejects missing nested lists");
        if (packet->resource_operation_count != 0) {
            malformed_packet = *packet;
            malformed_resource = packet->resource_operations[0];
            malformed_resource.pixels = texture_pixels;
            malformed_resource.pitch = (size_t)-1;
            malformed_packet.resource_operations = &malformed_resource;
            malformed_packet.resource_operation_count = 1;
            check(imgui_render_packet_clone(&malformed_packet,
                                            &config.allocator,
                                            &malformed_packet_clone) ==
                      IMGUI_RESULT_INVALID_ARGUMENT,
                  "render packet clone rejects overflowing resource payload");
        }
        imgui_software_packet_diff_init(&packet_diff);
        check(imgui_software_compare_packets(packet, packet, &packet_diff) ==
                  IMGUI_RESULT_OK && packet_diff.differing_viewports == 0 &&
                  packet_diff.differing_resources == 0 &&
                  packet_diff.differing_lists == 0 &&
                  packet_diff.differing_vertices == 0 &&
                  packet_diff.differing_indices == 0 &&
                  packet_diff.differing_commands == 0,
              "structural packet diff identical packet");
        malformed_packet = *packet;
        malformed_viewport = packet->viewports[0];
        malformed_viewport.lists = NULL;
        malformed_viewport.list_count = 1;
        malformed_packet.viewports = &malformed_viewport;
        malformed_packet.viewport_count = 1;
        check(imgui_software_compare_packets(&malformed_packet, packet,
                                             &packet_diff) ==
                  IMGUI_RESULT_INVALID_ARGUMENT,
              "structural packet diff rejects missing nested lists");
        malformed_packet = *packet;
        malformed_viewport = packet->viewports[0];
        malformed_list = packet->viewports[0].lists[0];
        malformed_command = malformed_list.commands[0];
        malformed_command.type = (imgui_render_command_type)99;
        malformed_list.commands = &malformed_command;
        malformed_list.command_count = 1;
        malformed_viewport.lists = &malformed_list;
        malformed_viewport.list_count = 1;
        malformed_packet.viewports = &malformed_viewport;
        malformed_packet.viewport_count = 1;
        check(imgui_software_compare_packets(&malformed_packet, packet,
                                             &packet_diff) ==
                  IMGUI_RESULT_INVALID_ARGUMENT,
              "structural packet diff rejects unknown command");
        packet_diff.struct_size = offsetof(imgui_software_packet_diff,
                                           differing_commands);
        check(imgui_software_compare_packets(packet, packet, &packet_diff) ==
                  IMGUI_RESULT_OK && packet_diff.differing_vertices == 0,
              "legacy packet diff descriptor accepted");
        imgui_software_packet_diff_init(&packet_diff);
        check(imgui_software_compare_packets(packet, cloned_packet,
                                              &packet_diff) ==
                  IMGUI_RESULT_OK && packet_diff.differing_resources == 0 &&
                  packet_diff.differing_commands == 0,
              "structural packet diff clone normalization");
        imgui_software_target_init(&software_target);
        software_target.pixels = software_pixels;
        software_target.width = 256;
        software_target.height = 256;
        software_target.stride = 256U * 4U;
        software_target.clear_color = 0x00000000UL;
        software_target.texture_resolver = software_texture_resolver;
        software_target.texture_user_data = NULL;
        software_target.command_callback = software_command_callback;
        software_target.command_user_data = NULL;
        software_texture_resolves = 0;
        software_command_callbacks = 0;
        software_copy_callbacks = 0;
        software_clear_callbacks = 0;
        software_update_callbacks = 0;
        memset(software_pixels, 0, sizeof(software_pixels));
        check(imgui_software_render_packet(packet, &software_target) ==
                  IMGUI_RESULT_OK, "software render packet");
        pixel_index = 0;
        while (pixel_index < (int)sizeof(software_pixels) &&
               software_pixels[pixel_index] == 0) ++pixel_index;
        check(pixel_index < (int)sizeof(software_pixels),
              "software renderer produced pixels");
        check(software_texture_resolves > 0,
              "software renderer resolved image texture");
        check(software_destination_pixels[0] == 17 &&
                  software_destination_pixels[1] == 34 &&
                  software_destination_pixels[2] == 51 &&
                  software_destination_pixels[3] == 255,
              "software renderer executed texture copy");
        check(software_source_pixels[4] == 17 &&
                  software_source_pixels[5] == 34 &&
                  software_source_pixels[6] == 51 &&
                  software_source_pixels[7] == 255,
              "software renderer preserves overlapping texture copy");
        check(software_destination_pixels[(8 * 16 + 8) * 4 + 0] == 0x44 &&
                  software_destination_pixels[(8 * 16 + 8) * 4 + 1] == 0x33 &&
                  software_destination_pixels[(8 * 16 + 8) * 4 + 2] == 0x22 &&
                  software_destination_pixels[(8 * 16 + 8) * 4 + 3] == 0x11,
              "software renderer executed texture clear");
        check(software_destination_pixels[(4 * 16 + 4) * 4 + 0] == 0x7a &&
                  software_destination_pixels[(4 * 16 + 4) * 4 + 1] == 0x6b &&
                  software_destination_pixels[(4 * 16 + 4) * 4 + 2] == 0x5c &&
                  software_destination_pixels[(4 * 16 + 4) * 4 + 3] == 0x4d,
              "software renderer executed ordered texture update");
        check(software_command_callbacks > 0,
              "software renderer surfaced non-draw commands");
        check(software_copy_callbacks > 0 && software_clear_callbacks > 0 &&
                  software_update_callbacks > 0,
              "software renderer callback surfaced ordered texture commands");
        software_target.struct_size = offsetof(imgui_software_target,
                                               command_callback);
        software_command_callbacks = 0;
        check(imgui_software_render_packet(packet, &software_target) ==
                  IMGUI_RESULT_OK,
              "legacy software target render");
        check(software_command_callbacks == 0,
              "legacy software target descriptor accepted");
        memcpy(software_pixels_copy, software_pixels, sizeof(software_pixels));
        imgui_software_diff_init(&software_diff);
        check(imgui_software_compare_rgba8(software_pixels, 256U * 4U,
                                           software_pixels_copy, 256U * 4U,
                                           256, 256, 0U, &software_diff) ==
                  IMGUI_RESULT_OK && software_diff.differing_pixels == 0,
              "software renderer identical image diff");
        software_diff.struct_size = offsetof(imgui_software_diff,
                                             max_channel_error);
        check(imgui_software_compare_rgba8(software_pixels, 256U * 4U,
                                           software_pixels_copy, 256U * 4U,
                                           256, 256, 0U, &software_diff) ==
                  IMGUI_RESULT_OK && software_diff.differing_pixels == 0,
              "legacy image diff descriptor accepted");
        software_pixels_copy[0] = (unsigned char)(software_pixels_copy[0] ^ 1U);
        check(imgui_software_compare_rgba8(software_pixels, 256U * 4U,
                                           software_pixels_copy, 256U * 4U,
                                           256, 256, 0U, &software_diff) ==
                  IMGUI_RESULT_OK && software_diff.differing_pixels == 1,
              "software renderer image diff detects pixel");
        memset(&malformed_packet, 0, sizeof(malformed_packet));
        malformed_packet.struct_size = sizeof(malformed_packet);
        malformed_packet.viewport_count = 1;
        check(imgui_software_render_packet(&malformed_packet,
                                           &software_target) ==
                  IMGUI_RESULT_INVALID_ARGUMENT,
              "software renderer rejects missing viewport array");
        memset(&malformed_viewport, 0, sizeof(malformed_viewport));
        malformed_viewport.list_count = 1;
        malformed_packet.viewports = &malformed_viewport;
        check(imgui_software_render_packet(&malformed_packet,
                                           &software_target) ==
              IMGUI_RESULT_CORRUPT_DATA,
              "software renderer rejects missing render lists");
        memset(&malformed_list, 0, sizeof(malformed_list));
        memset(&malformed_command, 0, sizeof(malformed_command));
        malformed_command.type = IMGUI_RENDER_COMMAND_TEXTURE_CLEAR;
        malformed_list.commands = &malformed_command;
        malformed_list.command_count = 1;
        malformed_viewport.lists = &malformed_list;
        check(imgui_software_render_packet(&malformed_packet,
                                           &software_target) ==
              IMGUI_RESULT_CORRUPT_DATA,
              "software renderer rejects malformed texture clear");
        malformed_command.data.texture_clear.texture = source_texture;
        malformed_command.data.texture_clear.region.x = INT_MAX;
        malformed_command.data.texture_clear.region.y = 0;
        malformed_command.data.texture_clear.region.width = 1;
        malformed_command.data.texture_clear.region.height = 1;
        check(imgui_software_render_packet(&malformed_packet,
                                           &software_target) ==
              IMGUI_RESULT_CORRUPT_DATA,
              "software renderer rejects overflowing texture clear");
        malformed_command.type = (imgui_render_command_type)99;
        check(imgui_software_render_packet(&malformed_packet,
                                           &software_target) ==
              IMGUI_RESULT_CORRUPT_DATA,
              "software renderer rejects unknown command type");
    }

    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "button release input");
    check(imgui_input_add_key(ctx, IMGUI_KEY_F12, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "shortcut key release input");
    check(imgui_input_add_mouse_position(ctx, 20.0f, 65.0f) ==
              IMGUI_RESULT_OK, "button release position");
    frame.time = 11.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "button release frame begin");
    check(imgui_is_key_released(ctx, IMGUI_KEY_F12) == IMGUI_TRUE,
          "key release query");
    check(imgui_is_mouse_released(ctx, IMGUI_MOUSE_BUTTON_LEFT) ==
              IMGUI_TRUE && imgui_is_mouse_down(ctx,
              IMGUI_MOUSE_BUTTON_LEFT) == IMGUI_FALSE &&
              imgui_is_any_mouse_down(ctx) == IMGUI_FALSE,
          "mouse edge queries");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "button release window");
    imgui_text(ctx, "C89 port");
    check(imgui_button(ctx, "Button") == IMGUI_TRUE,
          "button activates on release");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "button release frame end");
    (void)imgui_render(ctx);

    imgui_window_desc_init(&scroll_desc, "Scrollable");
    scroll_desc.has_position = IMGUI_TRUE;
    scroll_desc.position = imgui_make_vec2(300.0f, 300.0f);
    scroll_desc.has_size = IMGUI_TRUE;
    scroll_desc.size = imgui_make_vec2(140.0f, 90.0f);
    frame.time = 12.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window scroll first frame begin");
    scope = imgui_window_begin_ex(ctx, &scroll_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "window scroll first window");
    imgui_text(ctx, "line 1");
    imgui_text(ctx, "line 2");
    imgui_text(ctx, "line 3");
    imgui_text(ctx, "line 4");
    imgui_text(ctx, "line 5");
    imgui_text(ctx, "line 6");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window scroll first frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 320.0f, 320.0f) ==
              IMGUI_RESULT_OK &&
              imgui_input_add_mouse_wheel(ctx, 0.0f, 1.0f) ==
              IMGUI_RESULT_OK, "window scroll wheel input");
    frame.time = 13.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window scroll second frame begin");
    scope = imgui_window_begin_ex(ctx, &scroll_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "window scroll second window");
    check(imgui_get_scroll_max_y(ctx) > 0.0f, "window scroll has range");
    check(imgui_get_scroll_y(ctx) > 0.0f, "window scroll consumes wheel");
    imgui_text(ctx, "line 1");
    imgui_text(ctx, "line 2");
    imgui_text(ctx, "line 3");
    imgui_text(ctx, "line 4");
    imgui_text(ctx, "line 5");
    imgui_text(ctx, "line 6");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window scroll second frame end");
    (void)imgui_render(ctx);

    check(imgui_input_add_mouse_position(ctx, 430.0f, 350.0f) ==
              IMGUI_RESULT_OK &&
              imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                            IMGUI_TRUE) == IMGUI_RESULT_OK,
          "window scrollbar grab input");
    frame.time = 14.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window scrollbar grab frame begin");
    scope = imgui_window_begin_ex(ctx, &scroll_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "window scrollbar grab window");
    imgui_text(ctx, "line 1");
    imgui_text(ctx, "line 2");
    imgui_text(ctx, "line 3");
    imgui_text(ctx, "line 4");
    imgui_text(ctx, "line 5");
    imgui_text(ctx, "line 6");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window scrollbar grab frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 430.0f, 380.0f) ==
              IMGUI_RESULT_OK, "window scrollbar drag input");
    frame.time = 15.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window scrollbar drag frame begin");
    scope = imgui_window_begin_ex(ctx, &scroll_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "window scrollbar drag window");
    check(imgui_get_scroll_y(ctx) > 20.0f,
          "window scrollbar drag advances scroll");
    imgui_text(ctx, "line 1");
    imgui_text(ctx, "line 2");
    imgui_text(ctx, "line 3");
    imgui_text(ctx, "line 4");
    imgui_text(ctx, "line 5");
    imgui_text(ctx, "line 6");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window scrollbar drag frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "window scrollbar release input");

    imgui_window_desc_init(&hscroll_desc, "Horizontal scroll");
    hscroll_desc.has_position = IMGUI_TRUE;
    hscroll_desc.position = imgui_make_vec2(470.0f, 300.0f);
    hscroll_desc.has_size = IMGUI_TRUE;
    hscroll_desc.size = imgui_make_vec2(120.0f, 70.0f);
    hscroll_desc.flags = IMGUI_WINDOW_HORIZONTAL_SCROLLBAR;
    frame.time = 16.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "horizontal scroll first frame begin");
    scope = imgui_window_begin_ex(ctx, &hscroll_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "horizontal scroll first window");
    imgui_text(ctx, "This is a deliberately long horizontal scrolling line");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "horizontal scroll first frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 500.0f, 320.0f) ==
              IMGUI_RESULT_OK &&
              imgui_input_add_mouse_wheel(ctx, 1.0f, 0.0f) ==
              IMGUI_RESULT_OK, "horizontal scroll wheel input");
    frame.time = 17.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "horizontal scroll second frame begin");
    scope = imgui_window_begin_ex(ctx, &hscroll_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "horizontal scroll second window");
    check(imgui_get_scroll_max_x(ctx) > 0.0f &&
              imgui_get_scroll_x(ctx) > 0.0f &&
              imgui_set_scroll_x(ctx, 0.0f) == IMGUI_RESULT_OK &&
              imgui_get_scroll_x(ctx) == 0.0f,
          "horizontal scroll query and setter");
    imgui_text(ctx, "This is a deliberately long horizontal scrolling line");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "horizontal scroll second frame end");
    (void)imgui_render(ctx);

    imgui_window_desc_init(&no_mouse_scroll_desc, "No mouse scroll");
    no_mouse_scroll_desc.has_position = IMGUI_TRUE;
    no_mouse_scroll_desc.position = imgui_make_vec2(300.0f, 100.0f);
    no_mouse_scroll_desc.has_size = IMGUI_TRUE;
    no_mouse_scroll_desc.size = imgui_make_vec2(140.0f, 90.0f);
    no_mouse_scroll_desc.flags = IMGUI_WINDOW_NO_SCROLL_WITH_MOUSE;
    frame.time = 18.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "no-mouse-scroll first frame begin");
    scope = imgui_window_begin_ex(ctx, &no_mouse_scroll_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "no-mouse-scroll first window");
    imgui_text(ctx, "line 1");
    imgui_text(ctx, "line 2");
    imgui_text(ctx, "line 3");
    imgui_text(ctx, "line 4");
    imgui_text(ctx, "line 5");
    imgui_text(ctx, "line 6");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "no-mouse-scroll first frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 320.0f, 120.0f) ==
              IMGUI_RESULT_OK &&
              imgui_input_add_mouse_wheel(ctx, 0.0f, 1.0f) ==
              IMGUI_RESULT_OK, "no-mouse-scroll wheel input");
    frame.time = 19.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "no-mouse-scroll second frame begin");
    scope = imgui_window_begin_ex(ctx, &no_mouse_scroll_desc);
    check(scope == IMGUI_SCOPE_ACTIVE && imgui_get_scroll_max_y(ctx) > 0.0f &&
              imgui_get_scroll_y(ctx) == 0.0f,
          "no-mouse-scroll blocks wheel movement");
    imgui_text(ctx, "line 1");
    imgui_text(ctx, "line 2");
    imgui_text(ctx, "line 3");
    imgui_text(ctx, "line 4");
    imgui_text(ctx, "line 5");
    imgui_text(ctx, "line 6");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "no-mouse-scroll second frame end");
    (void)imgui_render(ctx);

    imgui_window_desc_init(&geometry_desc, "Window A");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(20.0f, 30.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(120.0f, 90.0f);
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multi-window appearing warmup begin");
    geometry_desc.position.x = (float)strtod("nan", NULL);
    check(imgui_window_begin_ex(ctx, &geometry_desc) == IMGUI_SCOPE_ERROR,
          "non-finite window descriptor rejected");
    geometry_desc.position.x = 20.0f;
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "multi-window appearing warmup A");
    imgui_window_end(ctx);
    imgui_window_desc_init(&geometry_desc, "Window B");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(240.0f, 50.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(140.0f, 100.0f);
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "multi-window appearing warmup B");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multi-window appearing warmup end");
    (void)imgui_render(ctx);
    imgui_window_desc_init(&geometry_desc, "Window A");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(20.0f, 30.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(120.0f, 90.0f);
    check(imgui_input_add_mouse_position(ctx, 40.0f, 60.0f) ==
              IMGUI_RESULT_OK, "multi-window capture position");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multi-window frame begin");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "first independent window");
    imgui_text(ctx, "A");
    imgui_window_end(ctx);
    imgui_window_desc_init(&geometry_desc, "Window B");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(240.0f, 50.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(140.0f, 100.0f);
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "second independent window");
    imgui_text(ctx, "B");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multi-window frame end");
    packet = imgui_render(ctx);
    found_window_a = IMGUI_FALSE;
    found_window_b = IMGUI_FALSE;
    if (packet != NULL) {
        for (command_index = 0;
             command_index < (int)packet->viewports[0].lists[0].command_count;
             ++command_index) {
            if (packet->viewports[0].lists[0].commands[command_index].type ==
                IMGUI_RENDER_COMMAND_DRAW_INDEXED) {
                if (packet->viewports[0].lists[0].commands[command_index]
                            .data.draw_indexed.vertex_offset <
                        packet->viewports[0].lists[0].vertex_count &&
                    packet->viewports[0].lists[0].vertices[
                        packet->viewports[0].lists[0].commands[command_index]
                            .data.draw_indexed.vertex_offset].position.x ==
                        20.0f) {
                    found_window_a = IMGUI_TRUE;
                }
                if (packet->viewports[0].lists[0].commands[command_index]
                            .data.draw_indexed.vertex_offset <
                        packet->viewports[0].lists[0].vertex_count &&
                    packet->viewports[0].lists[0].vertices[
                        packet->viewports[0].lists[0].commands[command_index]
                            .data.draw_indexed.vertex_offset].position.x ==
                        240.0f) {
                    found_window_b = IMGUI_TRUE;
                }
            }
        }
    }
    check(found_window_a && found_window_b,
          "independent window geometry in packet");
    check(imgui_get_frame_output(ctx)->want_capture_mouse == IMGUI_TRUE,
          "mouse capture aggregates earlier window hover");
    imgui_metrics_init(&metrics);
    check(imgui_get_metrics(ctx, &metrics) == IMGUI_RESULT_OK &&
              metrics.window_count >= 3 && metrics.open_window_count >= 3 &&
              metrics.command_count == packet->viewports[0].lists[0]
                  .command_count && metrics.vertex_count ==
                  packet->viewports[0].lists[0].vertex_count,
          "metrics match multi-window packet");
    metrics.struct_size = offsetof(imgui_metrics, texture_count);
    check(imgui_get_metrics(ctx, &metrics) == IMGUI_RESULT_OK &&
              metrics.frame_index != 0,
          "legacy metrics descriptor accepted");

    dock_window_id = imgui_get_id_string(ctx, "Window B");
    imgui_viewport_desc_init(&viewport_desc, 77);
    viewport_desc.position = imgui_make_vec2(640.0f, 20.0f);
    viewport_desc.size = imgui_make_vec2(320.0f, 240.0f);
    viewport_desc.struct_size = offsetof(imgui_viewport_desc, framebuffer_scale);
    check(imgui_viewport_configure(ctx, &viewport_desc) == IMGUI_RESULT_OK,
          "legacy viewport descriptor accepted");
    viewport_desc.struct_size = sizeof(viewport_desc);
    check(platform_viewport_creates == 1,
          "platform creates secondary viewport");
    viewport_desc.position.x += 8.0f;
    check(imgui_viewport_configure(ctx, &viewport_desc) == IMGUI_RESULT_OK &&
              platform_viewport_updates == 1,
          "platform updates secondary viewport");
    check(imgui_platform_configure(ctx, &platform) == IMGUI_RESULT_OK &&
              platform_viewport_destroys == 1 &&
              platform_viewport_creates == 2,
          "platform reconfiguration migrates secondary viewport ownership");
    check(imgui_window_set_viewport(ctx, dock_window_id, 77) ==
              IMGUI_RESULT_OK && imgui_window_get_viewport(ctx, dock_window_id) ==
              77, "assign window to secondary viewport");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multi-viewport frame begin");
    imgui_window_desc_init(&geometry_desc, "Window A");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(20.0f, 30.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(120.0f, 90.0f);
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "main viewport window");
    imgui_text(ctx, "main");
    imgui_window_end(ctx);
    imgui_window_desc_init(&geometry_desc, "Window B");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(40.0f, 40.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(140.0f, 100.0f);
    check(imgui_set_next_window_viewport(ctx, 77) == IMGUI_RESULT_OK,
          "next-window viewport setter");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "secondary viewport window");
    imgui_text(ctx, "secondary");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multi-viewport frame end");
    packet = imgui_render(ctx);
    found_secondary_viewport = IMGUI_FALSE;
    found_window_a = IMGUI_FALSE;
    if (packet != NULL && packet->viewport_count == 2) {
        imgui_u32 viewport_index;
        for (viewport_index = 0; viewport_index < packet->viewport_count;
             ++viewport_index) {
            if (packet->viewports[viewport_index].viewport_id == 0 &&
                packet->viewports[viewport_index].display_size.x ==
                    frame.display_size.x &&
                packet->viewports[viewport_index].display_size.y ==
                    frame.display_size.y) {
                found_window_a = IMGUI_TRUE;
            }
            if (packet->viewports[viewport_index].viewport_id == 77 &&
                packet->viewports[viewport_index].list_count == 1 &&
                packet->viewports[viewport_index].display_size.x == 320.0f) {
                found_secondary_viewport = IMGUI_TRUE;
            }
        }
    }
    check(found_window_a && found_secondary_viewport,
          "primary and secondary viewport packet extraction");
    dock_node_id = imgui_get_id_string(ctx, "MainDock");
    dock_child_left = imgui_get_id_string(ctx, "MainDock.Left");
    dock_child_right = imgui_get_id_string(ctx, "MainDock.Right");
    check(imgui_window_set_dock(ctx, dock_window_id, dock_child_right) ==
              IMGUI_RESULT_OK && imgui_window_get_dock(ctx, dock_window_id) ==
              dock_child_right, "dock window assignment");
    check(imgui_window_set_dock(ctx, imgui_get_id_string(ctx, "Window A"),
                                dock_child_left) == IMGUI_RESULT_OK,
          "second dock window assignment");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "dock space frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope == IMGUI_SCOPE_ACTIVE, "dock space host window");
    check(imgui_dock_space(ctx, dock_node_id,
                           imgui_make_vec2(200.0f, 100.0f), IMGUI_DOCK_NONE) ==
              IMGUI_RESULT_OK, "dock space creation");
    check(imgui_dock_space_split(ctx, dock_node_id,
                                 IMGUI_DOCK_SPLIT_LEFT, 0.4f,
                                 dock_child_left) == IMGUI_RESULT_OK &&
              imgui_dock_space_split(ctx, dock_node_id,
                                     IMGUI_DOCK_SPLIT_LEFT, 0.4f,
                                     dock_child_right) == IMGUI_RESULT_OK,
          "dock split creation");
    check(imgui_dock_tab_bar(ctx, dock_child_right) == IMGUI_RESULT_OK,
          "dock tab bar generation");
    check(imgui_dock_get_active_window(ctx, dock_child_right) ==
              dock_window_id, "dock tab active query");
    imgui_window_end(ctx);
    imgui_window_desc_init(&geometry_desc, "Window B");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE && imgui_window_get_dock(
              ctx, dock_window_id) == dock_child_right, "docked window begin");
    split_right_x = imgui_get_cursor_screen_position(ctx).x;
    imgui_window_end(ctx);
    imgui_window_desc_init(&geometry_desc, "Window A");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "second docked window begin");
    split_left_x = imgui_get_cursor_screen_position(ctx).x;
    imgui_window_end(ctx);
    check(split_right_x > split_left_x, "dock split geometry");
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "dock space frame end");
    (void)imgui_render(ctx);

    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "window focus release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window focus release frame begin");
    imgui_window_desc_init(&geometry_desc, "Window A");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(20.0f, 30.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(120.0f, 90.0f);
    imgui_window_begin_ex(ctx, &geometry_desc);
    imgui_window_end(ctx);
    imgui_window_desc_init(&geometry_desc, "Window B");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(240.0f, 50.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(140.0f, 100.0f);
    imgui_window_begin_ex(ctx, &geometry_desc);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window focus release frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 140.0f, 60.0f) ==
              IMGUI_RESULT_OK, "window focus target position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "window focus target click");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window focus target frame begin");
    imgui_window_desc_init(&geometry_desc, "Window A");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(20.0f, 30.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(120.0f, 90.0f);
    imgui_window_begin_ex(ctx, &geometry_desc);
    imgui_window_end(ctx);
    imgui_window_desc_init(&geometry_desc, "Window B");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(240.0f, 50.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(140.0f, 100.0f);
    imgui_window_begin_ex(ctx, &geometry_desc);
    check(imgui_is_window_focused(ctx, 0),
          "window focus follows click");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window focus target frame end");
    (void)imgui_render(ctx);
    imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT, IMGUI_FALSE);

    check(imgui_settings_save(ctx, NULL, 0, &settings_required) ==
              IMGUI_RESULT_INVALID_ARGUMENT && settings_required > 0,
          "settings size query");
    check(imgui_settings_save(ctx, settings_text, sizeof(settings_text),
                              &settings_required) == IMGUI_RESULT_OK,
          "settings save");
    check(strstr(settings_text, "DOCK ") != NULL,
          "settings saves dock assignment");
    check(strstr(settings_text, "WINDOW ") != NULL,
          "settings saves window geometry");
    check(strstr(settings_text, "VIEWPORT 77 ") != NULL,
          "settings saves viewport descriptor");
    check(strstr(settings_text, "SPLIT ") != NULL,
          "settings saves dock split");
    check(strstr(settings_text, "ACTIVE ") != NULL,
          "settings saves active dock tab");
    check(strstr(settings_text, "IMGUI_C89_SETTINGS 1\n") != NULL,
          "settings header");
    check(strstr(settings_text, "TREE ") != NULL,
          "settings tree record");
    check(strstr(settings_text, "CHILD ") != NULL,
          "settings child scroll record");
    check(strstr(settings_text, "TABLE ") != NULL,
          "settings table width record");
    check(imgui_settings_save(ctx, settings_before_bad_load,
                              sizeof(settings_before_bad_load), NULL) ==
              IMGUI_RESULT_OK, "settings baseline save");
    check(imgui_settings_load(ctx, "IMGUI_C89_SETTINGS 1\nTREE 1 2\n",
                              strlen("IMGUI_C89_SETTINGS 1\nTREE 1 2\n")) ==
              IMGUI_RESULT_CORRUPT_DATA, "invalid settings rejected");
    check(imgui_settings_save(ctx, settings_text, sizeof(settings_text),
                              NULL) == IMGUI_RESULT_OK &&
              strcmp(settings_text, settings_before_bad_load) == 0,
          "invalid settings load is transactional");
    check(imgui_settings_load(
              ctx,
              "IMGUI_C89_SETTINGS 1\nWINDOW 123 0 0 nan 40 40 0\n",
              strlen("IMGUI_C89_SETTINGS 1\nWINDOW 123 0 0 nan 40 40 0\n")) ==
              IMGUI_RESULT_CORRUPT_DATA,
          "non-finite settings geometry rejected");
    check(imgui_settings_save(ctx, settings_text, sizeof(settings_text),
                              NULL) == IMGUI_RESULT_OK &&
              strcmp(settings_text, settings_before_bad_load) == 0,
          "non-finite settings load is transactional");
    check(imgui_settings_load(ctx,
                              "IMGUI_C89_SETTINGS 1\nTREE 123 1\nCHILD 321 4.5 8.0\nTABLE 1234 2 33 44 SORT 2 1 -1 0 1\nTAB 456\nDOCK 999 888\nSPLIT 777 1 0.3 778 779\n",
                              strlen("IMGUI_C89_SETTINGS 1\nTREE 123 1\nCHILD 321 4.5 8.0\nTABLE 1234 2 33 44 SORT 2 1 -1 0 1\nTAB 456\nDOCK 999 888\nSPLIT 777 1 0.3 778 779\n")) ==
              IMGUI_RESULT_OK, "settings load");
    check(imgui_settings_save(ctx, settings_text, sizeof(settings_text),
                              NULL) == IMGUI_RESULT_OK &&
              strstr(settings_text, "TREE 123 1\n") != NULL &&
              strstr(settings_text, "CHILD 321 4.5 8 ") != NULL &&
              strstr(settings_text, "TABLE 1234 2 33 44 SORT") != NULL &&
              strstr(settings_text, "SORT 2 1 -1 0 1\n") != NULL &&
              strstr(settings_text, "TAB 456\n") != NULL &&
              strstr(settings_text, "DOCK 999 888\n") != NULL &&
              strstr(settings_text, "SPLIT 777 1 ") != NULL &&
              strstr(settings_text, " 778 779\n") != NULL,
          "settings round trip state");

    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK &&
              imgui_settings_load(ctx, "IMGUI_C89_SETTINGS 1\n",
                                  strlen("IMGUI_C89_SETTINGS 1\n")) ==
              IMGUI_RESULT_INVALID_STATE,
          "settings load rejects active frame mutation");
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "settings lifecycle guard frame end");
    (void)imgui_render(ctx);

    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "multi-select release input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multi-select idle frame begin");
    scope = imgui_window_begin(ctx, "Selection");
    check(scope != IMGUI_SCOPE_ERROR, "multi-select idle window");
    check(imgui_multi_select_begin(ctx, &selection,
                                   IMGUI_MULTI_SELECT_NONE) ==
              IMGUI_RESULT_OK, "multi-select begin");
    selection_first_id = imgui_get_id_string(ctx, "Choice A");
    selection_value = imgui_multi_select_contains(ctx, selection_first_id);
    (void)imgui_selectable_ex(ctx, "Choice A", &selection_value, 0,
                              imgui_make_vec2(220.0f, 22.0f));
    check(imgui_multi_select_item(ctx, selection_first_id) == IMGUI_FALSE,
          "multi-select idle item");
    check(imgui_multi_select_end(ctx) == IMGUI_RESULT_OK,
          "multi-select end");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multi-select idle frame end");
    (void)imgui_render(ctx);
    check(selection.count == 0, "multi-select idle preserves storage");

    check(imgui_input_add_mouse_position(ctx, 20.0f, 50.0f) ==
              IMGUI_RESULT_OK, "multi-select first position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "multi-select first click");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multi-select first frame begin");
    scope = imgui_window_begin(ctx, "Selection");
    check(scope != IMGUI_SCOPE_ERROR, "multi-select first window");
    check(imgui_multi_select_begin(ctx, &selection,
                                   IMGUI_MULTI_SELECT_NONE) ==
              IMGUI_RESULT_OK, "multi-select first begin");
    selection_first_id = imgui_get_id_string(ctx, "Choice A");
    selection_value = imgui_multi_select_contains(ctx, selection_first_id);
    (void)imgui_selectable_ex(ctx, "Choice A", &selection_value, 0,
                              imgui_make_vec2(220.0f, 22.0f));
    check(imgui_multi_select_item(ctx, selection_first_id) == IMGUI_FALSE,
          "multi-select waits for release");
    selection_second_id = imgui_get_id_string(ctx, "Choice B");
    selection_value = imgui_multi_select_contains(ctx, selection_second_id);
    (void)imgui_selectable_ex(ctx, "Choice B", &selection_value, 0,
                              imgui_make_vec2(220.0f, 22.0f));
    check(imgui_multi_select_end(ctx) == IMGUI_RESULT_OK,
          "multi-select first end");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multi-select first frame end");
    (void)imgui_render(ctx);
    check(selection.count == 0, "multi-select press does not select");

    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "multi-select toggle release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multi-select release frame begin");
    scope = imgui_window_begin(ctx, "Selection");
    check(scope != IMGUI_SCOPE_ERROR, "multi-select release window");
    check(imgui_multi_select_begin(ctx, &selection,
                                   IMGUI_MULTI_SELECT_NONE) == IMGUI_RESULT_OK,
          "multi-select release begin");
    selection_value = imgui_multi_select_contains(ctx, selection_first_id);
    (void)imgui_selectable_ex(ctx, "Choice A", &selection_value, 0,
                              imgui_make_vec2(220.0f, 22.0f));
    check(imgui_multi_select_item(ctx, selection_first_id) == IMGUI_TRUE,
          "multi-select activates on release");
    check(imgui_is_item_toggled_selection(ctx) == IMGUI_TRUE,
          "multi-select toggled-selection query");
    check(imgui_multi_select_end(ctx) == IMGUI_RESULT_OK,
          "multi-select release end");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multi-select release frame end");
    (void)imgui_render(ctx);
    check(selection.count == 1 && selected_ids[0] == selection_first_id,
          "multi-select single-click selection");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "multi-select ctrl press");
    check(imgui_input_add_mouse_position(ctx, 20.0f, 72.0f) ==
              IMGUI_RESULT_OK, "multi-select second position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "multi-select second click");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multi-select toggle frame begin");
    scope = imgui_window_begin(ctx, "Selection");
    check(scope != IMGUI_SCOPE_ERROR, "multi-select toggle window");
    check(imgui_multi_select_begin(ctx, &selection,
                                   IMGUI_MULTI_SELECT_NONE) ==
              IMGUI_RESULT_OK, "multi-select toggle begin");
    selection_value = imgui_multi_select_contains(ctx, selection_first_id);
    (void)imgui_selectable_ex(ctx, "Choice A", &selection_value, 0,
                              imgui_make_vec2(220.0f, 22.0f));
    (void)imgui_multi_select_item(ctx, selection_first_id);
    selection_value = imgui_multi_select_contains(ctx, selection_second_id);
    (void)imgui_selectable_ex(ctx, "Choice B", &selection_value, 0,
                              imgui_make_vec2(220.0f, 22.0f));
    check(imgui_is_item_clicked(ctx, IMGUI_MOUSE_BUTTON_LEFT),
          "multi-select ctrl press query");
    check(imgui_multi_select_item(ctx, selection_second_id) == IMGUI_FALSE,
          "multi-select ctrl waits for release");
    check(imgui_multi_select_end(ctx) == IMGUI_RESULT_OK,
          "multi-select toggle end");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multi-select toggle frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "multi-select final release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multi-select ctrl release frame begin");
    scope = imgui_window_begin(ctx, "Selection");
    check(scope != IMGUI_SCOPE_ERROR, "multi-select ctrl release window");
    check(imgui_multi_select_begin(ctx, &selection,
                                   IMGUI_MULTI_SELECT_NONE) == IMGUI_RESULT_OK,
          "multi-select ctrl release begin");
    selection_value = imgui_multi_select_contains(ctx, selection_first_id);
    (void)imgui_selectable_ex(ctx, "Choice A", &selection_value, 0,
                              imgui_make_vec2(220.0f, 22.0f));
    (void)imgui_multi_select_item(ctx, selection_first_id);
    selection_value = imgui_multi_select_contains(ctx, selection_second_id);
    (void)imgui_selectable_ex(ctx, "Choice B", &selection_value, 0,
                              imgui_make_vec2(220.0f, 22.0f));
    check(imgui_multi_select_item(ctx, selection_second_id) == IMGUI_TRUE,
          "multi-select ctrl activates on release");
    check(imgui_multi_select_end(ctx) == IMGUI_RESULT_OK,
          "multi-select ctrl release end");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multi-select ctrl release frame end");
    (void)imgui_render(ctx);
    check(selection.count == 2, "multi-select ctrl adds item");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "multi-select ctrl release");

    drag_value = 1234;
    for (drag_type_index = 0; drag_type_index < 299; ++drag_type_index) {
        long_drag_type[drag_type_index] =
            (char)('A' + (drag_type_index % 26));
    }
    long_drag_type[299] = '\0';
    drag_source_id = imgui_get_id_integer(ctx, 7001);
    drag_target_id = imgui_get_id_integer(ctx, 7002);
    check(imgui_input_add_mouse_position(ctx, 20.0f, 50.0f) ==
              IMGUI_RESULT_OK, "drag source position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "drag source press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "drag/drop frame begin");
    scope = imgui_window_begin(ctx, "Drag");
    check(scope != IMGUI_SCOPE_ERROR, "drag/drop window");
    (void)imgui_invisible_button(ctx, drag_source_id,
                                 imgui_make_vec2(60.0f, 22.0f), 0);
    scope = imgui_drag_drop_source_begin(ctx, drag_source_id, 0);
    check(scope == IMGUI_SCOPE_INACTIVE, "drag source waits for threshold");
    imgui_drag_drop_source_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "drag threshold frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 30.0f, 50.0f) ==
              IMGUI_RESULT_OK, "drag threshold movement");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "drag active frame begin");
    scope = imgui_window_begin(ctx, "Drag");
    check(scope != IMGUI_SCOPE_ERROR, "drag active window");
    (void)imgui_invisible_button(ctx, drag_source_id,
                                 imgui_make_vec2(60.0f, 22.0f), 0);
    scope = imgui_drag_drop_source_begin(ctx, drag_source_id, 0);
    check(scope == IMGUI_SCOPE_ACTIVE, "drag source active after threshold");
    check(imgui_drag_drop_set_payload(ctx, long_drag_type, &drag_value,
                                      sizeof(drag_value)) == IMGUI_RESULT_OK,
          "long drag payload set");
    imgui_drag_drop_source_end(ctx);
    imgui_set_cursor_screen_position(ctx, imgui_make_vec2(16.0f, 36.0f));
    (void)imgui_invisible_button(ctx, drag_target_id,
                                 imgui_make_vec2(60.0f, 22.0f), 0);
    scope = imgui_drag_drop_target_begin(ctx);
    check(scope == IMGUI_SCOPE_ACTIVE, "drag target active");
    check(imgui_drag_drop_target_accept(ctx, "WRONG", 0) == NULL,
          "drag type filter");
    drag_payload_view = imgui_drag_drop_target_accept(ctx, long_drag_type, 0);
    check(drag_payload_view != NULL && drag_payload_view->preview &&
              !drag_payload_view->delivery,
          "drag delivery waits for release");
    drag_payload_view = imgui_drag_drop_target_accept(
        ctx, long_drag_type, IMGUI_DRAG_DROP_TARGET_ACCEPT_BEFORE_DELIVERY);
    check(drag_payload_view != NULL && drag_payload_view->preview &&
              drag_payload_view->delivery && drag_payload_view->data_size ==
              sizeof(drag_value) &&
              *((const int *)drag_payload_view->data) == 1234,
          "drag payload accepted");
    imgui_drag_drop_target_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "drag/drop frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "drag release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "drag release delivery frame begin");
    scope = imgui_window_begin(ctx, "Drag");
    check(scope != IMGUI_SCOPE_ERROR, "drag release delivery window");
    imgui_set_cursor_screen_position(ctx, imgui_make_vec2(16.0f, 36.0f));
    (void)imgui_invisible_button(ctx, drag_target_id,
                                 imgui_make_vec2(60.0f, 22.0f), 0);
    scope = imgui_drag_drop_target_begin(ctx);
    drag_payload_view = imgui_drag_drop_target_accept(ctx, long_drag_type, 0);
    check(scope == IMGUI_SCOPE_ACTIVE && drag_payload_view != NULL &&
              drag_payload_view->delivery,
          "drag payload delivers after release");
    imgui_drag_drop_target_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "drag release delivery frame end");
    (void)imgui_render(ctx);

    /* External sources do not depend on a submitted item or an item ID. */
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "external drag press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "external drag frame begin");
    scope = imgui_window_begin(ctx, "Drag");
    check(scope != IMGUI_SCOPE_ERROR, "external drag window");
    scope = imgui_drag_drop_source_begin(
        ctx, 0, IMGUI_DRAG_DROP_SOURCE_EXTERN |
        IMGUI_DRAG_DROP_SOURCE_NO_PREVIEW);
    check(scope == IMGUI_SCOPE_ACTIVE, "external drag source active");
    check(imgui_drag_drop_set_payload(ctx, "EXTERNAL", &drag_value,
                                      sizeof(drag_value)) == IMGUI_RESULT_OK,
          "external drag payload set");
    imgui_drag_drop_source_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "external drag frame end");
    (void)imgui_render(ctx);
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "external drag target frame begin");
    scope = imgui_window_begin(ctx, "Drag");
    check(scope != IMGUI_SCOPE_ERROR, "external drag target window");
    (void)imgui_invisible_button(ctx, drag_target_id,
                                 imgui_make_vec2(60.0f, 22.0f), 0);
    scope = imgui_drag_drop_target_begin(ctx);
    drag_payload_view = imgui_drag_drop_target_accept(
        ctx, "EXTERNAL", IMGUI_DRAG_DROP_TARGET_ACCEPT_BEFORE_DELIVERY);
    check(scope == IMGUI_SCOPE_ACTIVE && drag_payload_view != NULL &&
              drag_payload_view->source_id != 0 &&
              *((const int *)drag_payload_view->data) == 1234,
          "external drag payload accepted");
    imgui_drag_drop_target_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "external drag target frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "external drag release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "external drag release settle begin");
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "external drag release settle end");
    (void)imgui_render(ctx);
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "external drag payload retirement begin");
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "external drag payload retirement end");
    (void)imgui_render(ctx);

    /* Items without an ID can opt in to a manufactured, position-based ID. */
    check(imgui_input_add_mouse_position(ctx, 20.0f, 50.0f) ==
              IMGUI_RESULT_OK, "null-id drag position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "null-id drag press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "null-id drag pending frame begin");
    scope = imgui_window_begin(ctx, "Drag");
    check(scope != IMGUI_SCOPE_ERROR, "null-id drag window");
    imgui_text_unformatted(ctx, "Null source", NULL);
    scope = imgui_drag_drop_source_begin(
        ctx, 0, IMGUI_DRAG_DROP_SOURCE_ALLOW_NULL_ID);
    check(scope == IMGUI_SCOPE_INACTIVE,
          "null-id drag waits for threshold");
    imgui_drag_drop_source_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "null-id drag pending frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 30.0f, 50.0f) ==
              IMGUI_RESULT_OK, "null-id drag movement");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "null-id drag active frame begin");
    scope = imgui_window_begin(ctx, "Drag");
    check(scope != IMGUI_SCOPE_ERROR, "null-id drag active window");
    imgui_text_unformatted(ctx, "Null source", NULL);
    scope = imgui_drag_drop_source_begin(
        ctx, 0, IMGUI_DRAG_DROP_SOURCE_ALLOW_NULL_ID);
    check(scope == IMGUI_SCOPE_ACTIVE, "null-id drag source active");
    check(imgui_drag_drop_set_payload(ctx, "NULL_ID", &drag_value,
                                      sizeof(drag_value)) == IMGUI_RESULT_OK,
          "null-id drag payload set");
    imgui_drag_drop_source_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "null-id drag active frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "null-id drag release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "null-id drag release settle begin");
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "null-id drag release settle end");
    (void)imgui_render(ctx);

    /* PayloadAutoExpire keeps one grace frame, then clears a source that is
       no longer submitted even while the mouse remains held. */
    check(imgui_input_add_mouse_position(ctx, 30.0f, 50.0f) ==
              IMGUI_RESULT_OK, "auto-expire drag position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "auto-expire drag press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "auto-expire source frame begin");
    scope = imgui_window_begin(ctx, "Drag");
    check(scope != IMGUI_SCOPE_ERROR, "auto-expire source window");
    scope = imgui_drag_drop_source_begin(
        ctx, 0, IMGUI_DRAG_DROP_SOURCE_EXTERN |
        IMGUI_DRAG_DROP_SOURCE_PAYLOAD_AUTO_EXPIRE);
    check(scope == IMGUI_SCOPE_ACTIVE, "auto-expire source active");
    check(imgui_drag_drop_set_payload(ctx, "EXPIRE", &drag_value,
                                      sizeof(drag_value)) == IMGUI_RESULT_OK,
          "auto-expire payload set");
    imgui_drag_drop_source_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "auto-expire source frame end");
    (void)imgui_render(ctx);
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "auto-expire grace frame begin");
    scope = imgui_window_begin(ctx, "Drag");
    check(scope != IMGUI_SCOPE_ERROR, "auto-expire grace window");
    (void)imgui_invisible_button(ctx, drag_target_id,
                                 imgui_make_vec2(60.0f, 22.0f), 0);
    scope = imgui_drag_drop_target_begin(ctx);
    drag_payload_view = imgui_drag_drop_target_accept(
        ctx, "EXPIRE", IMGUI_DRAG_DROP_TARGET_ACCEPT_BEFORE_DELIVERY);
    check(scope == IMGUI_SCOPE_ACTIVE && drag_payload_view != NULL,
          "auto-expire grace payload remains");
    imgui_drag_drop_target_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "auto-expire grace frame end");
    (void)imgui_render(ctx);
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "auto-expire cleared frame begin");
    scope = imgui_window_begin(ctx, "Drag");
    check(scope != IMGUI_SCOPE_ERROR, "auto-expire cleared window");
    (void)imgui_invisible_button(ctx, drag_target_id,
                                 imgui_make_vec2(60.0f, 22.0f), 0);
    scope = imgui_drag_drop_target_begin(ctx);
    check(scope == IMGUI_SCOPE_INACTIVE,
          "auto-expire clears missing source");
    imgui_drag_drop_target_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "auto-expire cleared frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "auto-expire drag release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "auto-expire release settle begin");
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "auto-expire release settle end");
    (void)imgui_render(ctx);

    capture_id = imgui_get_id_integer(ctx, 8001);
    check(imgui_input_add_mouse_position(ctx, 20.0f, 50.0f) ==
              IMGUI_RESULT_OK, "capture initial position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "capture initial press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "capture initial frame begin");
    scope = imgui_window_begin(ctx, "Capture");
    check(scope != IMGUI_SCOPE_ERROR, "capture initial window");
    (void)imgui_invisible_button(ctx, capture_id,
                                 imgui_make_vec2(80.0f, 22.0f), 0);
    check(imgui_is_item_active(ctx), "capture initial item active");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "capture initial frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 400.0f, 400.0f) ==
              IMGUI_RESULT_OK, "capture outside position");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "capture outside frame begin");
    scope = imgui_window_begin(ctx, "Capture");
    check(scope != IMGUI_SCOPE_ERROR, "capture outside window");
    (void)imgui_invisible_button(ctx, capture_id,
                                 imgui_make_vec2(80.0f, 22.0f), 0);
    check(imgui_is_item_active(ctx), "capture remains active outside item");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "capture outside frame end");
    check(imgui_get_frame_output(ctx)->want_capture_mouse,
          "capture output is available at frame end");
    (void)imgui_render(ctx);
    check(imgui_get_frame_output(ctx)->want_capture_mouse,
          "capture output remains active outside item");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "capture release input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "capture release frame begin");
    scope = imgui_window_begin(ctx, "Capture");
    check(scope != IMGUI_SCOPE_ERROR, "capture release window");
    (void)imgui_invisible_button(ctx, capture_id,
                                 imgui_make_vec2(80.0f, 22.0f), 0);
    check(!imgui_is_item_active(ctx) && imgui_is_item_deactivated(ctx),
          "capture deactivates on release");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "capture release frame end");
    (void)imgui_render(ctx);

    imgui_window_desc_init(&geometry_desc, "Geometry");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(100.0f, 120.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(300.0f, 200.0f);
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "explicit window geometry frame begin");
    check(imgui_get_frame_output(ctx)->next_wake_time > frame.time,
          "idle wake deadline");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "explicit window geometry active");
    geometry_cursor = imgui_get_cursor_screen_position(ctx);
    check(geometry_cursor.x == 108.0f && geometry_cursor.y == 148.0f,
          "explicit window geometry cursor");
    check(imgui_get_content_region_available(ctx).x == 292.0f &&
              imgui_get_content_region_available(ctx).y == 172.0f,
          "explicit window geometry content region");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "explicit window geometry frame end");
    (void)imgui_render(ctx);

    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "keyboard activation focus release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "keyboard activation release frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "keyboard activation release window");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "keyboard activation release frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 20.0f, 65.0f) ==
              IMGUI_RESULT_OK, "keyboard activation focus position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "keyboard activation focus press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "keyboard activation focus frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "keyboard activation focus window");
    imgui_text(ctx, "C89 port");
    (void)imgui_button(ctx, "Button");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "keyboard activation focus frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "popup outside clear prior click");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "popup outside open frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "popup outside open window");
    imgui_popup_open(ctx, "OutsidePopup", 0);
    scope = imgui_popup_begin(ctx, "OutsidePopup", IMGUI_WINDOW_NONE);
    check(scope == IMGUI_SCOPE_ACTIVE, "popup outside open active");
    imgui_text(ctx, "outside test");
    imgui_popup_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "popup outside open frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 620.0f, 460.0f) ==
              IMGUI_RESULT_OK, "popup outside click position");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "popup outside move frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "popup outside move window");
    imgui_popup_open(ctx, "OutsidePopup", 0);
    scope = imgui_popup_begin(ctx, "OutsidePopup", IMGUI_WINDOW_NONE);
    check(scope == IMGUI_SCOPE_ACTIVE, "popup remains after pointer move");
    imgui_popup_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "popup outside move frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "popup outside click press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "popup outside dismiss frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "popup outside dismiss window");
    scope = imgui_popup_begin(ctx, "OutsidePopup", IMGUI_WINDOW_NONE);
    check(scope == IMGUI_SCOPE_INACTIVE, "popup outside dismissed");
    imgui_popup_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "popup outside dismiss frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "popup outside click release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_ESCAPE, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "popup escape input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "popup escape frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "popup escape window");
    scope = imgui_popup_begin(ctx, "TestPopup", IMGUI_WINDOW_NONE);
    check(scope == IMGUI_SCOPE_INACTIVE, "popup dismissed by escape");
    imgui_popup_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "popup escape frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ESCAPE, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "popup escape release");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "keyboard activation mouse release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "keyboard activation key press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "keyboard activation frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "keyboard activation window");
    imgui_text(ctx, "C89 port");
    check(imgui_button(ctx, "Button") == IMGUI_TRUE,
          "keyboard activates focused button");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "keyboard activation frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "keyboard activation key release");

    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_DPAD_RIGHT, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "spatial navigation key press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "spatial navigation frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "spatial navigation window");
    (void)imgui_button(ctx, "Button");
    imgui_same_line(ctx);
    (void)imgui_button(ctx, "Second");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "spatial navigation frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_DPAD_RIGHT, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "spatial navigation key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_FACE_DOWN, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "spatial activation key press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "spatial activation frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "spatial activation window");
    (void)imgui_button(ctx, "Button");
    imgui_same_line(ctx);
    check(imgui_button(ctx, "Second") == IMGUI_TRUE,
          "spatial navigation activates adjacent item");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "spatial activation frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_FACE_DOWN, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "spatial activation key release");
    check(imgui_input_add_mouse_position(ctx, 20.0f, 45.0f) ==
              IMGUI_RESULT_OK, "navigation focus restore position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "navigation focus restore press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "navigation focus restore frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "navigation focus restore window");
    (void)imgui_button(ctx, "Button");
    imgui_same_line(ctx);
    (void)imgui_button(ctx, "Second");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "navigation focus restore frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "navigation focus restore release");

    check(imgui_input_add_key(ctx, IMGUI_KEY_TAB, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "tab navigation key press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "tab navigation frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "tab navigation window");
    (void)imgui_button(ctx, "Button");
    (void)imgui_button(ctx, "Second");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "tab navigation frame end");
    (void)imgui_render(ctx);
    check(imgui_get_frame_output(ctx)->navigation_active,
          "tab navigation active output");
    check(imgui_input_add_key(ctx, IMGUI_KEY_TAB, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "tab navigation key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "tab activation key press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "tab activation frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "tab activation window");
    (void)imgui_button(ctx, "Button");
    check(imgui_button(ctx, "Second") == IMGUI_TRUE,
          "tab focuses next button");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "tab activation frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "tab activation key release");

    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_DPAD_DOWN, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "gamepad navigation key press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "gamepad navigation frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "gamepad navigation window");
    (void)imgui_button(ctx, "Button");
    (void)imgui_button(ctx, "Second");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "gamepad navigation frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_DPAD_DOWN, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "gamepad navigation key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_FACE_DOWN, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "gamepad activation key press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "gamepad activation frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "gamepad activation window");
    check(imgui_button(ctx, "Button") == IMGUI_TRUE,
          "gamepad activates wrapped button");
    (void)imgui_button(ctx, "Second");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "gamepad activation frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_FACE_DOWN, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "gamepad activation key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_DPAD_UP, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "reverse gamepad navigation key press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "reverse gamepad navigation frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "reverse gamepad navigation window");
    (void)imgui_button(ctx, "Button");
    (void)imgui_button(ctx, "Second");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "reverse gamepad navigation frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_DPAD_UP, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "reverse gamepad navigation key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_FACE_DOWN, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "reverse gamepad activation key press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "reverse gamepad activation frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "reverse gamepad activation window");
    (void)imgui_button(ctx, "Button");
    check(imgui_button(ctx, "Second") == IMGUI_TRUE,
          "reverse navigation activates wrapped item");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "reverse gamepad activation frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_GAMEPAD_FACE_DOWN, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "reverse gamepad activation key release");

    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_SHIFT, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "shift-tab modifier press");
    check(imgui_input_add_key(ctx, IMGUI_KEY_TAB, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "shift-tab key press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "shift-tab navigation frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "shift-tab navigation window");
    (void)imgui_button(ctx, "Button");
    (void)imgui_button(ctx, "Second");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "shift-tab navigation frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_TAB, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "shift-tab key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_SHIFT, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "shift-tab modifier release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "shift-tab activation key press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "shift-tab activation frame begin");
    scope = imgui_window_begin(ctx, "Smoke");
    check(scope != IMGUI_SCOPE_ERROR, "shift-tab activation window");
    check(imgui_button(ctx, "Button") == IMGUI_TRUE,
          "shift-tab focuses previous button");
    (void)imgui_button(ctx, "Second");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "shift-tab activation frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "shift-tab activation key release");

    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "large navigation frame begin");
    scope = imgui_window_begin(ctx, "ManyNav");
    check(scope != IMGUI_SCOPE_ERROR, "large navigation window");
    for (navigation_index = 0; navigation_index < 300;
         ++navigation_index) {
        (void)imgui_button(ctx, "NavItem");
    }
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "large navigation frame end");
    (void)imgui_render(ctx);
    imgui_metrics_init(&metrics);
    check(imgui_get_metrics(ctx, &metrics) == IMGUI_RESULT_OK &&
              metrics.navigation_item_count >= 300,
          "navigation storage grows beyond legacy capacity");

    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "large tree frame begin");
    scope = imgui_window_begin(ctx, "ManyTrees");
    check(scope != IMGUI_SCOPE_ERROR, "large tree window");
    large_tree_ok = IMGUI_TRUE;
    for (navigation_index = 0; navigation_index < 150;
         ++navigation_index) {
        scope = imgui_tree_node_begin_with_id(
            ctx, (imgui_id)(1000 + navigation_index), "Tree", 0);
        if (scope == IMGUI_SCOPE_ERROR) large_tree_ok = IMGUI_FALSE;
        if (scope != IMGUI_SCOPE_ERROR) imgui_tree_node_end(ctx);
    }
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK && large_tree_ok,
          "tree storage grows beyond legacy capacity");
    (void)imgui_render(ctx);

    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "large child frame begin");
    scope = imgui_window_begin(ctx, "ManyChildren");
    check(scope != IMGUI_SCOPE_ERROR, "large child window");
    large_tree_ok = IMGUI_TRUE;
    for (navigation_index = 0; navigation_index < 100;
         ++navigation_index) {
        scope = imgui_child_begin(ctx, (imgui_id)(2000 + navigation_index),
                                  imgui_make_vec2(80.0f, 20.0f), 0);
        if (scope == IMGUI_SCOPE_ERROR) {
            large_tree_ok = IMGUI_FALSE;
        } else {
            imgui_child_end(ctx);
        }
    }
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK && large_tree_ok,
          "child storage grows beyond legacy capacity");
    (void)imgui_render(ctx);

    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "large table frame begin");
    scope = imgui_window_begin(ctx, "ManyColumns");
    check(scope != IMGUI_SCOPE_ERROR, "large table window");
    scope = imgui_table_begin(ctx, "LargeTable", 100, IMGUI_TABLE_NONE);
    large_tree_ok = scope == IMGUI_SCOPE_ACTIVE ? IMGUI_TRUE : IMGUI_FALSE;
    if (large_tree_ok) {
        for (navigation_index = 0; navigation_index < 100;
             ++navigation_index) {
            imgui_table_next_column(ctx);
            imgui_text_unformatted(ctx, "cell", NULL);
        }
        imgui_table_end(ctx);
    }
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK && large_tree_ok,
          "table storage grows beyond legacy capacity");
    (void)imgui_render(ctx);
    check(imgui_settings_save(ctx, settings_text, sizeof(settings_text), NULL) ==
              IMGUI_RESULT_OK && strstr(settings_text, "TABLE ") != NULL,
          "large table settings serialize");
    {
        imgui_result large_settings_result;
        large_settings_result = imgui_settings_load(
            ctx, settings_text, strlen(settings_text));
        check(large_settings_result == IMGUI_RESULT_OK,
              "large table settings round trip");
    }
    strcpy(wide_table_settings, "IMGUI_C89_SETTINGS 1\nTABLE 4242 300");
    for (navigation_index = 0; navigation_index < 300; ++navigation_index) {
        snprintf(wide_table_piece, sizeof(wide_table_piece), " %d",
                 navigation_index + 1);
        strcat(wide_table_settings, wide_table_piece);
    }
    strcat(wide_table_settings, "\n");
    check(imgui_settings_load(ctx, wide_table_settings,
                              strlen(wide_table_settings)) ==
              IMGUI_RESULT_OK,
          "settings loads table beyond legacy column capacity");
    check(imgui_settings_save(ctx, settings_text, sizeof(settings_text),
                              NULL) == IMGUI_RESULT_OK &&
              strstr(settings_text, "TABLE 4242 300 ") != NULL,
          "settings preserves dynamically sized table widths");

    settings_ctx = imgui_context_create(&config);
    check(settings_ctx != NULL, "wide window settings context creation");
    if (settings_ctx != NULL) {
        strcpy(wide_window_settings, "IMGUI_C89_SETTINGS 1\n");
        for (navigation_index = 0; navigation_index < 300;
             ++navigation_index) {
            snprintf(wide_window_piece, sizeof(wide_window_piece),
                     "WINDOW %d 10 20 100 80 100 80 0 0 0 0 0 0\n",
                     navigation_index + 10000);
            strcat(wide_window_settings, wide_window_piece);
        }
        check(imgui_settings_load(settings_ctx, wide_window_settings,
                                  strlen(wide_window_settings)) ==
                  IMGUI_RESULT_OK,
              "settings loads windows beyond legacy staging capacity");
        check(imgui_settings_save(settings_ctx, wide_window_settings,
                                   sizeof(wide_window_settings), NULL) ==
                  IMGUI_RESULT_OK &&
                  strstr(wide_window_settings, "WINDOW 10299 ") != NULL,
              "settings preserves dynamically sized window records");
        strcpy(wide_window_settings, "IMGUI_C89_SETTINGS 1\n");
        for (navigation_index = 0; navigation_index < 700;
             ++navigation_index) {
            snprintf(wide_window_piece, sizeof(wide_window_piece),
                     "CHILD %d 1 4 0 0\n", navigation_index + 50000);
            strcat(wide_window_settings, wide_window_piece);
        }
        check(imgui_settings_load(settings_ctx, wide_window_settings,
                                  strlen(wide_window_settings)) ==
                  IMGUI_RESULT_OK,
              "settings loads children beyond legacy staging capacity");
        check(imgui_settings_save(settings_ctx, settings_output,
                                   sizeof(settings_output), NULL) ==
                  IMGUI_RESULT_OK &&
                  strstr(settings_output, "CHILD 50699 ") != NULL,
              "settings preserves dynamically sized child records");
        strcpy(wide_window_settings, "IMGUI_C89_SETTINGS 1\n");
        for (navigation_index = 0; navigation_index < 3000;
             ++navigation_index) {
            snprintf(wide_window_piece, sizeof(wide_window_piece),
                     "TREE %d 1\n", navigation_index + 50000);
            strcat(wide_window_settings, wide_window_piece);
        }
        check(imgui_settings_load(settings_ctx, wide_window_settings,
                                  strlen(wide_window_settings)) ==
                  IMGUI_RESULT_OK,
              "settings loads trees beyond legacy staging capacity");
        check(imgui_settings_save(settings_ctx, settings_output,
                                   sizeof(settings_output), NULL) ==
                  IMGUI_RESULT_OK &&
                  strstr(settings_output, "TREE 52999 1\n") != NULL,
              "settings preserves dynamically sized tree records");
        strcpy(wide_window_settings, "IMGUI_C89_SETTINGS 1\n");
        for (navigation_index = 0; navigation_index < 100;
             ++navigation_index) {
            snprintf(wide_window_piece, sizeof(wide_window_piece),
                     "DOCK %d %d\n", navigation_index + 10000,
                     navigation_index + 60000);
            strcat(wide_window_settings, wide_window_piece);
        }
        for (navigation_index = 0; navigation_index < 100;
             ++navigation_index) {
            snprintf(wide_window_piece, sizeof(wide_window_piece),
                     "SPLIT %d 1 0.5 %d %d\n", navigation_index + 60000,
                     navigation_index + 70000, navigation_index + 80000);
            strcat(wide_window_settings, wide_window_piece);
        }
        for (navigation_index = 0; navigation_index < 100;
             ++navigation_index) {
            snprintf(wide_window_piece, sizeof(wide_window_piece),
                     "ACTIVE %d %d\n", navigation_index + 60000,
                     navigation_index + 10000);
            strcat(wide_window_settings, wide_window_piece);
        }
        check(imgui_settings_load(settings_ctx, wide_window_settings,
                                  strlen(wide_window_settings)) ==
                  IMGUI_RESULT_OK,
              "settings loads dock records beyond legacy staging capacity");
        check(imgui_settings_save(settings_ctx, settings_output,
                                   sizeof(settings_output), NULL) ==
                  IMGUI_RESULT_OK &&
                  strstr(settings_output, "DOCK 10099 60099\n") != NULL,
              "settings preserves dynamically sized dock records");
        strcpy(wide_window_settings, "IMGUI_C89_SETTINGS 1\n");
        for (navigation_index = 0; navigation_index < 16;
             ++navigation_index) {
            snprintf(wide_window_piece, sizeof(wide_window_piece),
                     "VIEWPORT %d 0 0 640 480 1 1\n",
                     navigation_index + 90000);
            strcat(wide_window_settings, wide_window_piece);
        }
        check(imgui_settings_load(settings_ctx, wide_window_settings,
                                  strlen(wide_window_settings)) ==
                  IMGUI_RESULT_OK,
              "settings loads expanded viewport descriptors");
        check(imgui_settings_save(settings_ctx, settings_output,
                                   sizeof(settings_output), NULL) ==
                  IMGUI_RESULT_OK &&
                  strstr(settings_output, "VIEWPORT 90015 ") != NULL,
              "settings preserves expanded viewport descriptors");
    }

    fixed_text[0] = '\0';
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "mouse release input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text release frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text release window");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "text release frame end");
    (void)imgui_render(ctx);

    imgui_window_desc_init(&geometry_desc, "Cursor probe");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(10.0f, 10.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(120.0f, 90.0f);
    check(imgui_input_add_mouse_position(ctx, 125.0f, 95.0f) ==
              IMGUI_RESULT_OK, "cursor resize position");
    frame.time += 1.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "cursor resize frame begin");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "cursor resize window");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "cursor resize frame end");
    (void)imgui_render(ctx);
    check(imgui_get_platform_output(ctx)->mouse_cursor ==
              IMGUI_MOUSE_CURSOR_RESIZE_DIAGONAL_NW_SE,
          "resize cursor survives window end");

    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "text click input");
    check(imgui_input_add_mouse_position(ctx, 20.0f, 50.0f) ==
              IMGUI_RESULT_OK, "text click position");
    check(imgui_input_add_text_utf8(ctx, "abc") == IMGUI_RESULT_OK,
          "text event input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text window");
    imgui_input_text_desc_init(&text_desc, "Name");
    text_desc.event_callback = edit_callback;
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_TRUE,
          "fixed text edit");
    check(strcmp(fixed_text, "\xf0\x9f\x99\x82" "abc") == 0,
          "fixed text contents");
    check(edit_events == 1, "text edit callback");
    check(always_events > 0 && filter_events >= 4,
          "text always/filter callbacks");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "text frame end");
    (void)imgui_render(ctx);
    check(imgui_get_platform_output(ctx)->mouse_cursor ==
              IMGUI_MOUSE_CURSOR_TEXT_INPUT, "text cursor platform output");
    check(imgui_get_platform_output(ctx)->ime.wants_text_input,
          "IME platform output");

    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text select-all modifier");
    check(imgui_input_add_key(ctx, IMGUI_KEY_A, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text select-all key reset");
    check(imgui_input_add_key(ctx, IMGUI_KEY_A, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text select-all key");
    check(imgui_input_add_text_utf8(ctx, "Z") == IMGUI_RESULT_OK,
          "text selection replacement input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text selection frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text selection window");
    imgui_input_text_desc_init(&text_desc, "Name");
    check(imgui_input_text_fixed(ctx, "Name", fixed_text,
                                 sizeof(fixed_text)) == IMGUI_TRUE,
          "text selection replacement");
    check(strcmp(fixed_text, "Z") == 0, "text selection replaced contents");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "text selection frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_A, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text select-all key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text select-all modifier release");
    check(imgui_input_add_text_utf8(ctx, "Y") == IMGUI_RESULT_OK,
          "text undo edit input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text undo edit frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text undo edit window");
    check(imgui_input_text_fixed(ctx, "Name", fixed_text,
                                 sizeof(fixed_text)) == IMGUI_TRUE,
          "text undo edit");
    check(strcmp(fixed_text, "ZY") == 0, "text undo edit contents");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "text undo edit frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text undo modifier");
    check(imgui_input_add_key(ctx, IMGUI_KEY_Z, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text undo key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text undo frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text undo window");
    check(imgui_input_text_fixed(ctx, "Name", fixed_text,
                                 sizeof(fixed_text)) == IMGUI_TRUE,
          "text undo");
    check(strcmp(fixed_text, "Z") == 0, "text undo restored contents");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "text undo frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_Z, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text undo key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_Y, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text redo key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text redo frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text redo window");
    check(imgui_input_text_fixed(ctx, "Name", fixed_text,
                                 sizeof(fixed_text)) == IMGUI_TRUE,
          "text redo");
    check(strcmp(fixed_text, "ZY") == 0, "text redo restored contents");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "text redo frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_Y, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text redo key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text undo modifier release");

    for (path_index = 0; path_index < 3000; ++path_index) {
        long_text[path_index] = 'A';
    }
    long_text[3000] = '\0';
    memset(&managed_text, 0, sizeof(managed_text));
    managed_text.struct_size = sizeof(managed_text);
    managed_text.data = long_text;
    managed_text.length = 3000;
    managed_text.capacity = sizeof(long_text);
    managed_text.reserve = long_text_reserve;
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "long text focus key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "long text focus frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "long text focus window");
    imgui_set_keyboard_focus_here(ctx, 0);
    imgui_input_text_desc_init(&text_desc, "Name");
    (void)imgui_input_text_buffer_ex(ctx, &text_desc, &managed_text);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "long text focus frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "long text focus key release");
    check(imgui_input_add_text_utf8(ctx, "Z") == IMGUI_RESULT_OK,
          "long text edit input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "long text edit frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "long text edit window");
    check(imgui_input_text_buffer_ex(ctx, &text_desc, &managed_text) ==
              IMGUI_TRUE && managed_text.length == 3001,
          "long text edit beyond legacy history limit");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "long text edit frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "long text undo modifier");
    check(imgui_input_add_key(ctx, IMGUI_KEY_Z, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "long text undo key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "long text undo frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "long text undo window");
    check(imgui_input_text_buffer_ex(ctx, &text_desc, &managed_text) ==
              IMGUI_TRUE && managed_text.length == 3000 &&
              long_text[2999] == 'A',
          "long text undo restores dynamic history");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "long text undo frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_Z, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "long text undo key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "long text undo modifier release");
    if (font_texture != NULL) {
        text_desc.flags = IMGUI_INPUT_TEXT_PASSWORD;
        check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
              "long password frame begin");
        scope = imgui_window_begin(ctx, "Text");
        check(scope != IMGUI_SCOPE_ERROR, "long password window");
        (void)imgui_input_text_buffer_ex(ctx, &text_desc, &managed_text);
        imgui_window_end(ctx);
        check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
              "long password frame end");
        packet = imgui_render(ctx);
        title_font_indices = 0;
        if (packet != NULL && packet->viewport_count > 0) {
            for (command_index = 0;
                 command_index < (int)packet->viewports[0].lists[0].command_count;
                 ++command_index) {
                if (packet->viewports[0].lists[0].commands[command_index].type ==
                        IMGUI_RENDER_COMMAND_DRAW_INDEXED &&
                    packet->viewports[0].lists[0].commands[command_index]
                        .data.draw_indexed.texture == font_texture) {
                    title_font_indices += packet->viewports[0].lists[0]
                        .commands[command_index].data.draw_indexed.index_count;
                }
            }
        }
        check(title_font_indices == 18000U,
              "long password renders without truncation");
    }

    text_desc.flags = IMGUI_INPUT_TEXT_NONE;
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "long clipboard modifier");
    check(imgui_input_add_key(ctx, IMGUI_KEY_A, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "long clipboard select key");
    check(imgui_input_add_key(ctx, IMGUI_KEY_C, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "long clipboard copy key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "long clipboard frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "long clipboard window");
    (void)imgui_input_text_buffer_ex(ctx, &text_desc, &managed_text);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "long clipboard frame end");
    (void)imgui_render(ctx);
    check(strlen(clipboard_text) == 3000 && clipboard_text[2999] == 'A',
          "long clipboard selection is not truncated");
    check(imgui_input_add_key(ctx, IMGUI_KEY_A, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "long clipboard select release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_C, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "long clipboard copy release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "long clipboard modifier release");

    for (path_index = 0; path_index < 3000; ++path_index) {
        long_text[path_index] = 'a';
    }
    long_text[3000] = '\0';
    managed_text.length = 3000;
    check(imgui_input_add_text_utf8(ctx, long_text) == IMGUI_RESULT_OK,
          "long filtered input");
    text_desc.flags = IMGUI_INPUT_TEXT_UPPERCASE;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "long filtered frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "long filtered window");
    check(imgui_input_text_buffer_ex(ctx, &text_desc, &managed_text) ==
              IMGUI_TRUE && managed_text.length == 3000 &&
              long_text[0] == 'A' && long_text[2999] == 'A',
          "long filtered input is not truncated");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "long filtered frame end");
    (void)imgui_render(ctx);

    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text clipboard modifier");
    check(imgui_input_add_key(ctx, IMGUI_KEY_A, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text clipboard select-all");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text clipboard select frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text clipboard select window");
    check(imgui_input_text_fixed(ctx, "Name", fixed_text,
                                 sizeof(fixed_text)) == IMGUI_FALSE,
          "text clipboard select has no edit");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "text clipboard select frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_A, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text clipboard select-all release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_C, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text clipboard copy key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text clipboard copy frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text clipboard copy window");
    (void)imgui_input_text_fixed(ctx, "Name", fixed_text,
                                 sizeof(fixed_text));
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "text clipboard copy frame end");
    (void)imgui_render(ctx);
    check(strcmp(clipboard_text, "ZY") == 0, "text clipboard copied selection");
    check(imgui_input_add_key(ctx, IMGUI_KEY_C, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text clipboard copy key release");
    strcpy(clipboard_text, "Q");
    check(imgui_input_add_key(ctx, IMGUI_KEY_V, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text clipboard paste key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text clipboard paste frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text clipboard paste window");
    check(imgui_input_text_fixed(ctx, "Name", fixed_text,
                                 sizeof(fixed_text)) == IMGUI_TRUE,
          "text clipboard paste edit");
    check(strcmp(fixed_text, "Q") == 0, "text clipboard pasted selection");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "text clipboard paste frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_V, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text clipboard paste key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text clipboard modifier release");

    check(imgui_input_add_key(ctx, IMGUI_KEY_RIGHT_CTRL, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text right-control modifier");
    check(imgui_input_add_key(ctx, IMGUI_KEY_A, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text right-control select-all");
    check(imgui_input_add_text_utf8(ctx, "R") == IMGUI_RESULT_OK,
          "text right-control replacement input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text right-control frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text right-control window");
    check(imgui_input_text_fixed(ctx, "Name", fixed_text,
                                 sizeof(fixed_text)) == IMGUI_TRUE &&
              strcmp(fixed_text, "R") == 0,
          "right-control select-all replacement");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "text right-control frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_A, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text right-control select-all release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_RIGHT_CTRL, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text right-control modifier release");

    strcpy(fixed_text, "one\ntwo");
    check(imgui_input_add_key(ctx, IMGUI_KEY_END, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "multiline text end key");
    check(imgui_input_add_key(ctx, IMGUI_KEY_UP_ARROW, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "multiline text up key");
    check(imgui_input_add_text_utf8(ctx, "X") == IMGUI_RESULT_OK,
          "multiline vertical insertion");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multiline vertical frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "multiline vertical window");
    imgui_input_text_desc_init(&text_desc, "Name");
    text_desc.multiline = IMGUI_TRUE;
    (void)imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text));
    check(strcmp(fixed_text, "oneX\ntwo") == 0,
          "multiline up preserves UTF-8 column");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multiline vertical frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_END, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "multiline text end key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_UP_ARROW, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "multiline text up key release");

    check(imgui_input_add_key(ctx, IMGUI_KEY_ESCAPE, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text escape key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text escape frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text escape window");
    imgui_input_text_desc_init(&text_desc, "Name");
    text_desc.flags = IMGUI_INPUT_TEXT_ESCAPE_CLEARS;
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_TRUE,
          "text escape clears");
    check(fixed_text[0] == '\0', "text escape cleared contents");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "text escape frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ESCAPE, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text escape key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "text enter key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text enter frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text enter window");
    imgui_input_text_desc_init(&text_desc, "Name");
    text_desc.flags = IMGUI_INPUT_TEXT_ENTER_RETURNS_TRUE;
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_TRUE,
          "text enter returns true");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "text enter frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "text enter key release");
    check(imgui_input_add_text_utf8(ctx, "a b") == IMGUI_RESULT_OK,
          "text filter input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "text filter frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "text filter window");
    imgui_input_text_desc_init(&text_desc, "Name");
    text_desc.flags = IMGUI_INPUT_TEXT_UPPERCASE |
                      IMGUI_INPUT_TEXT_NO_BLANK;
    text_desc.struct_size = offsetof(imgui_input_text_desc, event_callback);
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_TRUE,
          "text filters characters");
    text_desc.struct_size = sizeof(text_desc);
    check(strcmp(fixed_text, "AB") == 0, "text filter result");
    text_desc.multiline = IMGUI_TRUE;
    text_desc.flags = IMGUI_INPUT_TEXT_NONE;
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_FALSE &&
              imgui_get_item_rect(ctx).y2 - imgui_get_item_rect(ctx).y1 ==
                  100.0f,
          "multiline text default geometry");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "text filter frame end");
    (void)imgui_render(ctx);

    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multiline focus frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "multiline focus window");
    imgui_set_keyboard_focus_here(ctx, 0);
    imgui_input_text_desc_init(&text_desc, "Name");
    text_desc.multiline = IMGUI_TRUE;
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_FALSE,
          "multiline focus request");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multiline focus frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "multiline enter key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multiline enter frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "multiline enter window");
    imgui_input_text_desc_init(&text_desc, "Name");
    text_desc.multiline = IMGUI_TRUE;
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_TRUE,
          "multiline enter inserts newline");
    check(strcmp(fixed_text, "AB\n") == 0,
          "multiline enter result");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multiline enter frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "multiline enter key release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multiline enter release settle begin");
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multiline enter release settle end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_TRUE) ==
              IMGUI_RESULT_OK &&
              imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "multiline ctrl-enter key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multiline ctrl-enter frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "multiline ctrl-enter window");
    imgui_input_text_desc_init(&text_desc, "Name");
    text_desc.multiline = IMGUI_TRUE;
    text_desc.flags = IMGUI_INPUT_TEXT_ENTER_RETURNS_TRUE;
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_TRUE &&
              strcmp(fixed_text, "AB\n") == 0,
          "multiline ctrl-enter validates without newline");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multiline ctrl-enter frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_FALSE) ==
              IMGUI_RESULT_OK &&
              imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "multiline ctrl-enter key release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multiline ctrl-enter release settle begin");
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multiline ctrl-enter release settle end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_TRUE) ==
              IMGUI_RESULT_OK &&
              imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "multiline ctrl-enter newline key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "multiline ctrl-enter newline frame begin");
    scope = imgui_window_begin(ctx, "Text");
    check(scope != IMGUI_SCOPE_ERROR, "multiline ctrl-enter newline window");
    imgui_input_text_desc_init(&text_desc, "Name");
    text_desc.multiline = IMGUI_TRUE;
    text_desc.flags = IMGUI_INPUT_TEXT_CTRL_ENTER_FOR_NEW_LINE;
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_TRUE &&
              strcmp(fixed_text, "AB\n\n") == 0,
          "multiline ctrl-enter inserts newline");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "multiline ctrl-enter newline frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_ENTER, IMGUI_FALSE) ==
              IMGUI_RESULT_OK &&
              imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "multiline ctrl-enter newline release");

    slider_value = 0.0f;
    slider_integer_value = -2;
    drag_integer_value = 3;
    check(imgui_input_add_mouse_position(ctx, 200.0f, 50.0f) ==
              IMGUI_RESULT_OK, "slider mouse position");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "slider frame begin");
    scope = imgui_window_begin(ctx, "Slider");
    check(scope != IMGUI_SCOPE_ERROR, "slider window");
    check(imgui_slider_float(ctx, "Amount", &slider_value, 0.0f, 1.0f),
          "slider edit");
    check(slider_value > 0.7f, "slider value clamped from mouse");
    (void)imgui_slider_integer_ex(ctx, "Whole Amount", &slider_integer_value,
                                  -5, 5, "%04.0f", IMGUI_SLIDER_ALWAYS_CLAMP);
    check(slider_integer_value >= -5 && slider_integer_value <= 5,
          "integer slider remains in range");
    (void)imgui_drag_integer_ex(ctx, "Dragged Whole Amount",
                                &drag_integer_value, 0.25f, -5, 5,
                                NULL, IMGUI_SLIDER_ALWAYS_CLAMP);
    check(drag_integer_value >= -5 && drag_integer_value <= 5,
          "integer drag remains in range");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "slider frame end");
    (void)imgui_render(ctx);

    vertical_slider_value = 0.0f;
    slider_integer_value = 0;
    check(imgui_input_add_mouse_position(ctx, 20.0f, 70.0f) ==
              IMGUI_RESULT_OK, "vertical slider mouse position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "vertical slider mouse down");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "vertical slider frame begin");
    check(imgui_set_next_window_position(ctx, imgui_make_vec2(10.0f, 10.0f)) ==
              IMGUI_RESULT_OK &&
              imgui_set_next_window_size(ctx, imgui_make_vec2(180.0f, 150.0f)) ==
              IMGUI_RESULT_OK,
          "next window geometry setters");
    scope = imgui_window_begin(ctx, "Vertical sliders");
    check(scope != IMGUI_SCOPE_ERROR, "vertical slider window");
    check(imgui_get_window_position(ctx).x == 10.0f &&
              imgui_get_window_position(ctx).y == 10.0f &&
              imgui_get_window_size(ctx).x == 180.0f &&
              imgui_get_window_size(ctx).y == 150.0f,
          "next window geometry applied once");
    check(imgui_is_window_appearing(ctx) == IMGUI_TRUE &&
              imgui_is_window_collapsed(ctx) == IMGUI_FALSE,
          "window appearing and collapsed queries");
    check(imgui_set_window_focus(ctx) == IMGUI_RESULT_OK &&
              imgui_is_window_focused(ctx, IMGUI_FOCUSED_NONE) == IMGUI_TRUE,
          "window focus setter");
    vertical_window_size = imgui_get_window_size(ctx);
    check(imgui_set_window_collapsed(ctx, IMGUI_TRUE) == IMGUI_RESULT_OK &&
              imgui_is_window_collapsed(ctx) == IMGUI_TRUE &&
              imgui_get_window_size(ctx).y == 20.0f,
          "window collapse setter");
    check(imgui_set_window_collapsed(ctx, IMGUI_FALSE) == IMGUI_RESULT_OK &&
              imgui_is_window_collapsed(ctx) == IMGUI_FALSE &&
              imgui_get_window_size(ctx).y == vertical_window_size.y,
          "window collapse restore");
    check(imgui_vslider_float(ctx, "Vertical", imgui_make_vec2(28.0f, 90.0f),
                              &vertical_slider_value, 0.0f, 1.0f),
          "vertical slider mouse edit");
    check(vertical_slider_value > 0.5f && vertical_slider_value <= 1.0f,
          "vertical slider maps y axis");
    (void)imgui_vslider_integer_ex(ctx, "Vertical integer",
                                   imgui_make_vec2(28.0f, 90.0f),
                                   &slider_integer_value, -5, 5,
                                   "%.0f", IMGUI_SLIDER_ALWAYS_CLAMP);
    check(slider_integer_value >= -5 && slider_integer_value <= 5,
          "vertical integer slider remains in range");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "vertical slider frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "vertical slider mouse up");

    /* Scalar flag semantics are isolated in their own frame so their extra
       widgets cannot perturb the input-coordinate fixtures below. */
    logarithmic_value = 1.0f;
    drag_unclamped_value = 2.0f;
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "scalar flags mouse down");
    check(imgui_input_add_mouse_position(ctx, 200.0f, 50.0f) ==
              IMGUI_RESULT_OK, "logarithmic slider mouse position");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "scalar flags frame begin");
    scope = imgui_window_begin(ctx, "Scalar flags");
    check(scope != IMGUI_SCOPE_ERROR, "scalar flags window");
    check(imgui_slider_float_ex(ctx, "Log Amount", &logarithmic_value,
                                1.0f, 1000.0f, NULL,
                                IMGUI_SLIDER_LOGARITHMIC),
          "logarithmic slider edit");
    check(logarithmic_value > 100.0f,
          "logarithmic slider maps fraction exponentially");
    signed_logarithmic_value = -100.0f;
    check(imgui_input_add_mouse_position(
              ctx, 200.0f, imgui_get_item_rect(ctx).y2 + 11.0f) ==
              IMGUI_RESULT_OK, "signed logarithmic slider position");
    check(imgui_slider_float_ex(ctx, "Signed Log Amount",
                                &signed_logarithmic_value,
                                -100.0f, 100.0f, NULL,
                                IMGUI_SLIDER_LOGARITHMIC),
          "signed logarithmic slider edit");
    check(signed_logarithmic_value > 0.0f,
          "signed logarithmic slider crosses zero");
    check(imgui_drag_float_ex(ctx, "Unclamped Drag", &drag_unclamped_value,
                              0.1f, 0.0f, 1.0f, NULL,
                              IMGUI_SLIDER_NONE) == IMGUI_FALSE &&
              drag_unclamped_value == 2.0f,
          "drag preserves out-of-range value without always-clamp");
    check(imgui_drag_float_ex(ctx, "Clamped Drag", &drag_unclamped_value,
                              0.1f, 0.0f, 1.0f, NULL,
                              IMGUI_SLIDER_ALWAYS_CLAMP) == IMGUI_TRUE &&
              drag_unclamped_value == 1.0f,
          "drag always-clamp flag clamps value");
    vector_values[0] = 0.1f;
    vector_values[1] = 0.2f;
    vector_values[2] = 0.3f;
    (void)imgui_slider_float_n(ctx, "Vector slider", vector_values, 3,
                               0.0f, 1.0f, "%.2f",
                               IMGUI_SLIDER_ALWAYS_CLAMP);
    (void)imgui_drag_float_n(ctx, "Vector drag", vector_values, 3,
                             0.1f, 0.0f, 1.0f, "%.2f",
                             IMGUI_SLIDER_ALWAYS_CLAMP);
    (void)imgui_input_float_n(ctx, "Vector input", vector_values, 3,
                              "%.2f");
    check(vector_values[0] >= 0.0f && vector_values[0] <= 1.0f &&
              vector_values[1] >= 0.0f && vector_values[1] <= 1.0f &&
              vector_values[2] >= 0.0f && vector_values[2] <= 1.0f,
          "vector scalar controls preserve component ranges");
    vector_integer_values[0] = -2;
    vector_integer_values[1] = 0;
    vector_integer_values[2] = 3;
    (void)imgui_slider_integer_n(ctx, "Vector integer slider",
                                 vector_integer_values, 3, -5, 5,
                                 "%.0f", IMGUI_SLIDER_ALWAYS_CLAMP);
    (void)imgui_drag_integer_n(ctx, "Vector integer drag",
                               vector_integer_values, 3, 0.1f, -5, 5,
                               NULL, IMGUI_SLIDER_ALWAYS_CLAMP);
    (void)imgui_input_integer_n(ctx, "Vector integer input",
                                vector_integer_values, 3, "%d");
    check(vector_integer_values[0] >= -5 && vector_integer_values[0] <= 5 &&
              vector_integer_values[1] >= -5 && vector_integer_values[1] <= 5 &&
              vector_integer_values[2] >= -5 && vector_integer_values[2] <= 5,
          "integer vector controls preserve component ranges");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "scalar flags frame end");
    (void)imgui_render(ctx);

    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "tree no-push frame begin");
    scope = imgui_window_begin(ctx, "Tree flags");
    check(scope != IMGUI_SCOPE_ERROR, "tree no-push window");
    cursor_before_indent = imgui_get_cursor_screen_position(ctx);
    first_id = imgui_get_id_string(ctx, "tree no-push child");
    scope = imgui_tree_node_begin(ctx, "No push",
                                  IMGUI_TREE_DEFAULT_OPEN |
                                  IMGUI_TREE_NO_TREE_PUSH_ON_OPEN);
    check(scope == IMGUI_SCOPE_ACTIVE, "tree no-push scope active");
    nested_id = imgui_get_id_string(ctx, "tree no-push child");
    check(nested_id == first_id,
          "tree no-push flag preserves ID scope");
    check(imgui_get_cursor_screen_position(ctx).x == cursor_before_indent.x,
          "tree no-push flag preserves indentation");
    imgui_tree_node_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "tree no-push frame end");
    (void)imgui_render(ctx);

    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "popup flags frame begin");
    scope = imgui_window_begin(ctx, "Popup flags");
    check(scope != IMGUI_SCOPE_ERROR, "popup flags window");
    imgui_popup_open(ctx, "ExistingPopup", IMGUI_POPUP_NONE);
    check(imgui_popup_is_open(ctx, "ExistingPopup") == IMGUI_TRUE,
          "popup open query");
    scope = imgui_popup_begin(ctx, "ExistingPopup", IMGUI_WINDOW_NONE);
    check(scope == IMGUI_SCOPE_ACTIVE, "existing popup active");
    imgui_popup_end(ctx);
    imgui_popup_open(ctx, "SuppressedPopup",
                     IMGUI_POPUP_NO_OPEN_OVER_EXISTING);
    scope = imgui_popup_begin(ctx, "SuppressedPopup", IMGUI_WINDOW_NONE);
    check(scope == IMGUI_SCOPE_INACTIVE,
          "popup does not open over existing popup");
    imgui_popup_end(ctx);
    imgui_popup_close_current(ctx);
    check(imgui_input_add_mouse_position(ctx, 20.0f, 50.0f) ==
              IMGUI_RESULT_OK, "popup item suppression position");
    (void)imgui_button(ctx, "Popup anchor item");
    imgui_popup_open(ctx, "ItemSuppressedPopup",
                     IMGUI_POPUP_NO_OPEN_OVER_ITEMS);
    scope = imgui_popup_begin(ctx, "ItemSuppressedPopup", IMGUI_WINDOW_NONE);
    check(scope == IMGUI_SCOPE_INACTIVE,
          "popup does not open over hovered item");
    imgui_popup_end(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "popup flags frame end");
    (void)imgui_render(ctx);

    check(imgui_input_add_mouse_position(ctx, 40.0f, 50.0f) ==
              IMGUI_RESULT_OK, "unanchored popup position");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "unanchored popup frame begin");
    scope = imgui_window_begin(ctx, "Unanchored popup host");
    check(scope != IMGUI_SCOPE_ERROR, "unanchored popup host window");
    imgui_popup_open(ctx, "UnanchoredPopup", IMGUI_POPUP_NONE);
    scope = imgui_popup_begin(ctx, "UnanchoredPopup", IMGUI_WINDOW_NONE);
    check(scope == IMGUI_SCOPE_ACTIVE, "unanchored popup active");
    check(imgui_get_cursor_screen_position(ctx).x == 48.0f &&
              imgui_get_cursor_screen_position(ctx).y == 58.0f,
          "unanchored popup uses mouse fallback");
    imgui_popup_end(ctx);
    imgui_popup_close_current(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "unanchored popup frame end");
    (void)imgui_render(ctx);

    /* Modal popup keeps ownership on outside clicks and contributes a dim
       layer behind its surface until explicitly closed. */
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "modal popup frame begin");
    scope = imgui_window_begin(ctx, "Modal popup host");
    check(scope != IMGUI_SCOPE_ERROR, "modal popup host window");
    imgui_popup_open(ctx, "ModalPopup", IMGUI_POPUP_NONE);
    scope = imgui_popup_begin(ctx, "ModalPopup", IMGUI_WINDOW_MODAL);
    check(scope == IMGUI_SCOPE_ACTIVE, "modal popup active");
    imgui_popup_end(ctx);
    check(imgui_button(ctx, "Modal background item") == IMGUI_FALSE,
          "modal popup blocks background item");
    check(imgui_is_window_hovered(ctx, IMGUI_HOVERED_ANY_WINDOW) ==
              IMGUI_FALSE, "modal popup owns aggregate hover");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "modal popup frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 250.0f, 250.0f) ==
              IMGUI_RESULT_OK, "modal popup outside position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "modal popup outside click");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "modal popup retained frame begin");
    scope = imgui_window_begin(ctx, "Modal popup host");
    check(scope != IMGUI_SCOPE_ERROR, "modal popup retained host window");
    scope = imgui_popup_begin(ctx, "ModalPopup", IMGUI_WINDOW_MODAL);
    check(scope == IMGUI_SCOPE_ACTIVE,
          "modal popup survives outside click");
    imgui_popup_end(ctx);
    imgui_popup_close_current(ctx);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "modal popup retained frame end");
    (void)imgui_render(ctx);

    numeric_value = 0;
    input_float_value = 1.25f;
    check(imgui_input_add_mouse_position(ctx, 20.0f, 50.0f) ==
              IMGUI_RESULT_OK, "numeric reset position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "numeric mouse release");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "numeric release frame begin");
    scope = imgui_window_begin(ctx, "Numeric");
    check(scope != IMGUI_SCOPE_ERROR, "numeric release window");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "numeric release frame end");
    (void)imgui_render(ctx);

    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "numeric click input");
    check(imgui_input_add_text_utf8(ctx, "42") == IMGUI_RESULT_OK,
          "numeric text input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "numeric frame begin");
    scope = imgui_window_begin(ctx, "Numeric");
    check(scope != IMGUI_SCOPE_ERROR, "numeric window");
    check(imgui_input_integer(ctx, "Count", &numeric_value),
          "numeric edit");
    check(numeric_value == 42, "numeric value parsed");
    (void)imgui_input_integer_ex(ctx, "Hex-like count", &numeric_value,
                                 "%04d");
    (void)imgui_input_float_ex(ctx, "Ratio", &input_float_value, "%.2f");
    invalid_widget_value = (float)strtod("nan", NULL);
    check(imgui_input_float_ex(ctx, "Invalid ratio", &invalid_widget_value,
                               "%.2f") == IMGUI_FALSE,
          "non-finite numeric input rejected");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "numeric frame end");
    (void)imgui_render(ctx);

    strcpy(fixed_text, "old");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "auto-select release input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "auto-select release frame begin");
    scope = imgui_window_begin(ctx, "Auto");
    check(scope != IMGUI_SCOPE_ERROR, "auto-select release window");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "auto-select release frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 20.0f, 50.0f) ==
              IMGUI_RESULT_OK, "auto-select click position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "auto-select click input");
    check(imgui_input_add_text_utf8(ctx, "new") == IMGUI_RESULT_OK,
          "auto-select replacement input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "auto-select frame begin");
    scope = imgui_window_begin(ctx, "Auto");
    check(scope != IMGUI_SCOPE_ERROR, "auto-select window");
    imgui_input_text_desc_init(&text_desc, "AutoName");
    text_desc.flags = IMGUI_INPUT_TEXT_AUTO_SELECT_ALL;
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_TRUE,
          "auto-select edit");
    check(strcmp(fixed_text, "new") == 0, "auto-select replaced contents");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "auto-select frame end");
    (void)imgui_render(ctx);

    strcpy(fixed_text, "abc");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_SHIFT, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "shift selection modifier");
    check(imgui_input_add_key(ctx, IMGUI_KEY_HOME, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "shift selection home key");
    check(imgui_input_add_text_utf8(ctx, "X") == IMGUI_RESULT_OK,
          "shift selection replacement input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "shift selection frame begin");
    scope = imgui_window_begin(ctx, "Auto");
    check(scope != IMGUI_SCOPE_ERROR, "shift selection window");
    imgui_input_text_desc_init(&text_desc, "AutoName");
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_TRUE,
          "shift selection edit");
    check(strcmp(fixed_text, "X") == 0,
          "shift home replaced selected range");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "shift selection frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_HOME, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "shift selection home release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_SHIFT, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "shift selection modifier release");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "shift selection mouse release");

    strcpy(fixed_text, "one two three");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "word navigation modifier");
    check(imgui_input_add_key(ctx, IMGUI_KEY_END, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "word navigation end key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "word navigation end frame begin");
    scope = imgui_window_begin(ctx, "Auto");
    check(scope != IMGUI_SCOPE_ERROR, "word navigation end window");
    imgui_input_text_desc_init(&text_desc, "AutoName");
    (void)imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text));
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "word navigation end frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_END, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "word navigation end release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_BACKSPACE, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "word deletion key");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "word deletion frame begin");
    scope = imgui_window_begin(ctx, "Auto");
    check(scope != IMGUI_SCOPE_ERROR, "word deletion window");
    imgui_input_text_desc_init(&text_desc, "AutoName");
    check(imgui_input_text_fixed_ex(ctx, &text_desc, fixed_text,
                                    sizeof(fixed_text)) == IMGUI_TRUE,
          "word deletion edit");
    check(strcmp(fixed_text, "one two ") == 0,
          "ctrl-backspace removed previous word");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "word deletion frame end");
    (void)imgui_render(ctx);
    check(imgui_input_add_key(ctx, IMGUI_KEY_BACKSPACE, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "word deletion key release");
    check(imgui_input_add_key(ctx, IMGUI_KEY_LEFT_CTRL, IMGUI_FALSE) ==
              IMGUI_RESULT_OK, "word navigation modifier release");

    check(imgui_input_add_key(ctx, IMGUI_KEY_A, IMGUI_TRUE) ==
              IMGUI_RESULT_OK, "focus-loss held key");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "focus-loss held mouse");
    check(imgui_input_add_focus(ctx, IMGUI_FALSE) == IMGUI_RESULT_OK,
          "focus loss input");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "focus-loss frame begin");
    scope = imgui_window_begin(ctx, "Focus");
    check(scope != IMGUI_SCOPE_ERROR, "focus-loss window");
    (void)imgui_button(ctx, "Not Hovered");
    check(!imgui_is_item_hovered(ctx, 0),
          "focus loss suppresses item hover");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "focus-loss frame end");
    (void)imgui_render(ctx);
    check(!imgui_get_frame_output(ctx)->want_capture_mouse &&
              !imgui_get_frame_output(ctx)->want_capture_keyboard &&
              !imgui_get_frame_output(ctx)->want_text_input,
          "focus loss clears capture outputs");
    check(imgui_input_add_focus(ctx, IMGUI_TRUE) == IMGUI_RESULT_OK,
          "focus regain input");

    /* Window title-bar movement is persistent and respects explicit geometry
       as the drag origin. */
    imgui_window_desc_init(&geometry_desc, "MoveWindow");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(30.0f, 30.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(120.0f, 90.0f);
    check(imgui_input_add_mouse_position(ctx, 40.0f, 35.0f) ==
              IMGUI_RESULT_OK, "window move press position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "window move press");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window move press frame");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "window move press begin");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "window move press end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 240.0f, 240.0f) ==
              IMGUI_RESULT_OK, "window move drag position");
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window move drag frame");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    move_cursor = imgui_get_cursor_screen_position(ctx);
    check(scope == IMGUI_SCOPE_ACTIVE && move_cursor.x > 57.0f &&
              move_cursor.y > 57.0f, "window moved by title bar drag");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK, "window move drag end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "window move release");
    frame.time = 20.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window move release frame");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "window move release begin");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window move release frame end");
    (void)imgui_render(ctx);
    check(imgui_window_get_dock(ctx, imgui_get_id_string(ctx, "MoveWindow")) == 0,
          "window move remains undocked");
    imgui_window_desc_init(&auto_resize_desc, "AutoResize");
    auto_resize_desc.flags = IMGUI_WINDOW_ALWAYS_AUTO_RESIZE;
    frame.time = 20.5;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "auto-resize first frame");
    scope = imgui_window_begin_ex(ctx, &auto_resize_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "auto-resize first begin");
    imgui_text(ctx, "small");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "auto-resize first end");
    (void)imgui_render(ctx);
    frame.time = 20.6;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "auto-resize second frame");
    check(imgui_set_next_window_content_size(
              ctx, imgui_make_vec2(220.0f, 100.0f)) == IMGUI_RESULT_OK,
          "next-window content-size setter");
    scope = imgui_window_begin_ex(ctx, &auto_resize_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "auto-resize second begin");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "auto-resize second end");
    (void)imgui_render(ctx);
    frame.time = 20.7;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "auto-resize content-size result frame");
    scope = imgui_window_begin_ex(ctx, &auto_resize_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "auto-resize content-size result begin");
    move_cursor = imgui_get_window_size(ctx);
    check(move_cursor.x >= 220.0f && move_cursor.x < frame.display_size.x &&
              move_cursor.y >= 100.0f && move_cursor.y < frame.display_size.y,
          "next-window content size affects auto-resize");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "auto-resize content-size result end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 40.0f, 35.0f) ==
              IMGUI_RESULT_OK, "window collapse first position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "window collapse first click");
    frame.time = 21.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window collapse first frame");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "window collapse first begin");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window collapse first end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "window collapse first release");
    frame.time = 21.05;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window collapse release frame");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "window collapse release begin");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window collapse release end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "window collapse second click");
    frame.time = 21.10;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window collapse second frame");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_INACTIVE, "window collapsed on double click");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window collapse second end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "window collapse second release");

    imgui_window_desc_init(&geometry_desc, "DropWindow");
    geometry_desc.has_position = IMGUI_TRUE;
    geometry_desc.position = imgui_make_vec2(300.0f, 200.0f);
    geometry_desc.has_size = IMGUI_TRUE;
    geometry_desc.size = imgui_make_vec2(100.0f, 80.0f);
    frame.time = 30.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window dock drag setup frame");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "window dock drag setup");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window dock drag setup end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 310.0f, 205.0f) ==
              IMGUI_RESULT_OK, "window dock drag press position");
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_TRUE) == IMGUI_RESULT_OK,
          "window dock drag press");
    frame.time = 31.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window dock drag press frame");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "window dock drag press begin");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window dock drag press end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_position(ctx, 40.0f, 55.0f) ==
              IMGUI_RESULT_OK, "window dock drag target position");
    frame.time = 31.1;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window dock drag target frame");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_ACTIVE, "window dock drag target begin");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window dock drag target end");
    (void)imgui_render(ctx);
    check(imgui_input_add_mouse_button(ctx, IMGUI_MOUSE_BUTTON_LEFT,
                                       IMGUI_FALSE) == IMGUI_RESULT_OK,
          "window dock drag release");
    frame.time = 31.2;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "window dock drag release frame");
    scope = imgui_window_begin_ex(ctx, &geometry_desc);
    check(scope == IMGUI_SCOPE_INACTIVE || scope == IMGUI_SCOPE_ACTIVE,
          "window dock drag release begin");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "window dock drag release end");
    (void)imgui_render(ctx);
    check(imgui_window_get_dock(ctx, imgui_get_id_string(ctx, "DropWindow")) != 0,
          "window docked by title drag");

    window_error_count = reported_errors;
    frame.time = 31.25;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "many windows frame begin");
    for (window_index = 0; window_index < 40; ++window_index) {
        imgui_push_id_integer(ctx, window_index);
        scope = imgui_window_begin(ctx, "Many Window");
        check(scope == IMGUI_SCOPE_ACTIVE, "many windows begin");
        imgui_window_end(ctx);
        imgui_pop_id(ctx);
    }
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "many windows frame end");
    (void)imgui_render(ctx);
    check(reported_errors == window_error_count,
          "window storage grows beyond the former fixed limit");

    frame.time = 31.3;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "deep dock frame begin");
    dock_node_id = 9000U;
    scope = imgui_window_begin(ctx, "Deep Dock");
    check(scope == IMGUI_SCOPE_ACTIVE, "deep dock window begin");
    check(imgui_dock_space(ctx, dock_node_id,
                           imgui_make_vec2(300.0f, 200.0f), 0) ==
              IMGUI_RESULT_OK, "deep dock root");
    for (dock_index = 0; dock_index < 24; ++dock_index) {
        check(imgui_dock_space_split(ctx, dock_node_id,
                                     IMGUI_DOCK_SPLIT_RIGHT, 0.5f,
                                     dock_node_id + (imgui_id)dock_index + 1U) ==
                  IMGUI_RESULT_OK, "deep dock split");
        dock_node_id += (imgui_id)dock_index + 1U;
    }
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "deep dock frame end");
    (void)imgui_render(ctx);

    metrics_open = IMGUI_TRUE;
    demo_open = IMGUI_TRUE;
    frame.time = 32.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "metrics frame begin");
    imgui_show_metrics_window(ctx, &metrics_open);
    imgui_show_demo_window(ctx, &demo_open);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "metrics frame end");
    (void)imgui_render(ctx);
    check(metrics_open == IMGUI_TRUE, "metrics window remains open");
    check(demo_open == IMGUI_TRUE, "demo window remains open");

    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "unbalanced ID frame begin");
    imgui_push_id_integer(ctx, 99);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_INVALID_STATE,
          "unbalanced ID stack rejected");
    (void)imgui_render(ctx);

    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "unfinished path frame begin");
    check(imgui_draw_list_path_begin(ctx, imgui_get_window_draw_list(ctx)) ==
              IMGUI_RESULT_OK, "unfinished path begin");
    check(imgui_frame_end(ctx) == IMGUI_RESULT_INVALID_STATE,
          "unfinished path rejected at frame end");
    (void)imgui_render(ctx);

    if (font != NULL && font_texture != NULL) {
        check(imgui_font_bind(ctx, font, font_texture) == IMGUI_RESULT_OK,
              "title label font bind");
        for (path_index = 0; path_index < 200; ++path_index) {
            long_title[path_index] = 'T';
        }
        strcpy(long_title + 200, "##hidden_id");
        imgui_window_desc_init(&title_desc, long_title);
        check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
              "hidden title frame begin");
        scope = imgui_window_begin_ex(ctx, &title_desc);
        check(scope == IMGUI_SCOPE_ACTIVE, "hidden title window begin");
        imgui_window_end(ctx);
        check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
              "hidden title frame end");
        packet = imgui_render(ctx);
        title_font_indices = 0;
        if (packet != NULL && packet->viewport_count > 0) {
            for (command_index = 0;
                 command_index < (int)packet->viewports[0].lists[0].command_count;
                 ++command_index) {
                if (packet->viewports[0].lists[0].commands[command_index].type ==
                        IMGUI_RENDER_COMMAND_DRAW_INDEXED &&
                    packet->viewports[0].lists[0].commands[command_index]
                        .data.draw_indexed.texture == font_texture) {
                    title_font_indices += packet->viewports[0].lists[0]
                        .commands[command_index].data.draw_indexed.index_count;
                }
            }
        }
        check(title_font_indices == 1200U,
              "long hidden title is not truncated or exposed");
        check(imgui_font_unbind(ctx) == IMGUI_RESULT_OK,
              "title label font unbind");
    }

    imgui_texture_destroy(ctx, source_texture);
    imgui_texture_destroy(ctx, destination_texture);
    if (font_texture != NULL) {
        check(imgui_texture_destroy(ctx, font_texture) == IMGUI_RESULT_OK,
              "font texture destroy");
    }
    if (font_atlas != NULL) imgui_font_atlas_destroy(font_atlas);
    imgui_trace_destroy(loaded_trace);
    imgui_trace_destroy(trace);

    /* Resource operations are consumed once. A later external upload queued
       between renders must survive into the next packet. */
    resource_ctx = imgui_context_create(NULL);
    resource_texture = NULL;
    if (resource_ctx != NULL) {
        imgui_renderer_desc_init(&resource_renderer);
        resource_renderer.capabilities =
            IMGUI_RENDERER_CAP_VERTEX_OFFSET |
            IMGUI_RENDERER_CAP_TEXTURES |
            IMGUI_RENDERER_CAP_TEXTURE_UPDATE;
        check(imgui_renderer_configure(resource_ctx, &resource_renderer) ==
                  IMGUI_RESULT_OK, "resource lifecycle renderer setup");
        imgui_texture_desc_init(&resource_desc);
        resource_desc.width = 1;
        resource_desc.height = 1;
        check(imgui_texture_create(resource_ctx, &resource_desc,
                                   resource_pixel, 4U,
                                   &resource_texture) == IMGUI_RESULT_OK,
              "resource lifecycle texture creation");
        imgui_frame_desc_init(&resource_frame);
        resource_frame.display_size = imgui_make_vec2(320.0f, 240.0f);
        resource_frame.time = 1.0;
        check(imgui_frame_begin(resource_ctx, &resource_frame) ==
                  IMGUI_RESULT_OK, "resource lifecycle first frame begin");
        scope = imgui_window_begin(resource_ctx, "Resource lifecycle");
        check(scope == IMGUI_SCOPE_ACTIVE,
              "resource lifecycle first window");
        imgui_window_end(resource_ctx);
        check(imgui_frame_end(resource_ctx) == IMGUI_RESULT_OK,
              "resource lifecycle first frame end");
        packet = imgui_render(resource_ctx);
        check(packet != NULL && packet->resource_operation_count >= 2U,
              "resource lifecycle first packet contains upload");
        resource_frame.time = 2.0;
        check(imgui_frame_begin(resource_ctx, &resource_frame) ==
                  IMGUI_RESULT_OK, "resource lifecycle second frame begin");
        scope = imgui_window_begin(resource_ctx, "Resource lifecycle");
        check(scope == IMGUI_SCOPE_ACTIVE,
              "resource lifecycle second window");
        imgui_window_end(resource_ctx);
        check(imgui_frame_end(resource_ctx) == IMGUI_RESULT_OK,
              "resource lifecycle second frame end");
        packet = imgui_render(resource_ctx);
        check(packet != NULL && packet->resource_operation_count == 0U,
              "resource lifecycle retires consumed operations");
        check(imgui_texture_update(resource_ctx, resource_texture, 0, 0,
                                   1, 1, resource_pixel, 4U) ==
                  IMGUI_RESULT_OK,
              "resource lifecycle late upload");
        resource_frame.time = 3.0;
        check(imgui_frame_begin(resource_ctx, &resource_frame) ==
                  IMGUI_RESULT_OK, "resource lifecycle third frame begin");
        scope = imgui_window_begin(resource_ctx, "Resource lifecycle");
        check(scope == IMGUI_SCOPE_ACTIVE,
              "resource lifecycle third window");
        imgui_window_end(resource_ctx);
        check(imgui_frame_end(resource_ctx) == IMGUI_RESULT_OK,
              "resource lifecycle third frame end");
        packet = imgui_render(resource_ctx);
        check(packet != NULL && packet->resource_operation_count == 1U,
              "resource lifecycle preserves late upload");
        imgui_texture_destroy(resource_ctx, resource_texture);
        imgui_context_destroy(resource_ctx);
    } else {
        check(IMGUI_FALSE, "resource lifecycle context creation");
    }
    check(imgui_viewport_destroy(ctx, 77) == IMGUI_RESULT_OK &&
              platform_viewport_destroys == 2 &&
              imgui_window_get_viewport(ctx, dock_window_id) == 0,
          "secondary viewport destruction reassigns windows");
    frame.time += 1.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "layout primitive frame begin");
    scope = imgui_window_begin(ctx, "Layout primitives");
    check(scope == IMGUI_SCOPE_ACTIVE, "layout primitive window");
    style = *imgui_style_get(ctx);
    style.color_button = 0xff123456UL;
    check(imgui_style_push(ctx, &style) == IMGUI_RESULT_OK,
          "scoped style push");
    (void)imgui_button(ctx, "Scoped style button");
    check(imgui_style_pop(ctx) == IMGUI_RESULT_OK &&
              imgui_style_get(ctx)->color_button != 0xff123456UL,
          "scoped style pop restores style");
    imgui_set_next_item_width(ctx, 96.0f);
    (void)imgui_button(ctx, "Width override");
    check(imgui_get_item_rect(ctx).x2 - imgui_get_item_rect(ctx).x1 ==
              96.0f, "next item width applies once");
    saved_cursor = imgui_get_cursor_screen_position(ctx);
    imgui_dummy(ctx, imgui_make_vec2(12.0f, 7.0f));
    move_cursor = imgui_get_cursor_screen_position(ctx);
    check(move_cursor.y > saved_cursor.y,
          "dummy advances layout");
    imgui_separator_text(ctx, "Section");
    imgui_bullet(ctx);
    imgui_bullet_text(ctx, "Bullet %d", 7);
    check(imgui_get_cursor_screen_position(ctx).y > move_cursor.y,
          "bullet text advances layout");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "layout primitive frame end");
    (void)imgui_render(ctx);
    /* InvisibleButton must retain layout and interaction semantics without
       adding a visible rectangle to the render packet.  Compare a steady
       state window with and without the item. */
    frame.time += 1.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "invisible button warmup frame begin");
    scope = imgui_window_begin(ctx, "Invisible probe");
    check(scope == IMGUI_SCOPE_ACTIVE, "invisible button warmup window");
    (void)imgui_invisible_button(ctx, imgui_get_id_integer(ctx, 9901),
                                 imgui_make_vec2(80.0f, 22.0f), 0);
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "invisible button warmup frame end");
    (void)imgui_render(ctx);
    frame.time += 1.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "invisible button baseline frame begin");
    scope = imgui_window_begin(ctx, "Invisible probe");
    check(scope == IMGUI_SCOPE_ACTIVE, "invisible button baseline window");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "invisible button baseline frame end");
    packet = imgui_render(ctx);
    check(packet != NULL && packet->viewport_count > 0U &&
              packet->viewports[0].list_count > 0U,
          "invisible button baseline packet");
    if (packet != NULL && packet->viewport_count > 0U &&
        packet->viewports[0].list_count > 0U) {
        invisible_base_vertices = packet->viewports[0].lists[0].vertex_count;
        invisible_base_indices = packet->viewports[0].lists[0].index_count;
        invisible_base_commands = packet->viewports[0].lists[0].command_count;
    } else {
        invisible_base_vertices = 0;
        invisible_base_indices = 0;
        invisible_base_commands = 0;
    }
    frame.time += 1.0;
    check(imgui_frame_begin(ctx, &frame) == IMGUI_RESULT_OK,
          "invisible button render frame begin");
    scope = imgui_window_begin(ctx, "Invisible probe");
    check(scope == IMGUI_SCOPE_ACTIVE, "invisible button render window");
    (void)imgui_invisible_button(ctx, imgui_get_id_integer(ctx, 9901),
                                 imgui_make_vec2(80.0f, 22.0f), 0);
    check(imgui_get_item_rect(ctx).x2 > imgui_get_item_rect(ctx).x1,
          "invisible button reserves hit region");
    imgui_window_end(ctx);
    check(imgui_frame_end(ctx) == IMGUI_RESULT_OK,
          "invisible button render frame end");
    packet = imgui_render(ctx);
    check(packet != NULL && packet->viewport_count > 0U &&
              packet->viewports[0].list_count > 0U &&
              packet->viewports[0].lists[0].vertex_count == invisible_base_vertices &&
              packet->viewports[0].lists[0].index_count == invisible_base_indices &&
              packet->viewports[0].lists[0].command_count == invisible_base_commands,
          "invisible button emits no widget geometry");
    imgui_context_destroy(ctx);
    imgui_context_destroy(focus_ctx);
    imgui_context_destroy(settings_ctx);
    detached_ctx = imgui_context_create(NULL);
    detached_atlas = NULL;
    if (detached_ctx != NULL &&
        imgui_font_atlas_create(detached_ctx, &detached_atlas) ==
            IMGUI_RESULT_OK) {
        imgui_context_destroy(detached_ctx);
        imgui_font_atlas_destroy(detached_atlas);
    } else {
        if (detached_ctx != NULL) imgui_context_destroy(detached_ctx);
        check(IMGUI_FALSE, "detached font atlas setup");
    }
    imgui_render_packet_destroy(cloned_packet, &config.allocator);
    if (failures != 0) {
        return 1;
    }
    printf("imgui_c89 port smoke test passed\n");
    return 0;
}

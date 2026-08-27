#ifndef IMGUI_C89_SOFTWARE_H
#define IMGUI_C89_SOFTWARE_H

#include "imgui_c89_render.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct imgui_software_texture_view {
    const unsigned char *pixels;
    unsigned char *mutable_pixels;
    int width;
    int height;
    size_t stride;
    imgui_texture_format format;
} imgui_software_texture_view;

typedef imgui_bool (*imgui_software_texture_resolver)(
    const imgui_texture *texture,
    imgui_software_texture_view *view,
    void *user_data);

/* Called before each ordered non-draw command, including texture copies,
   clears, updates, sampler changes, resets, and custom commands. */
typedef void (*imgui_software_command_callback)(
    const imgui_render_command *command, void *user_data);

typedef struct imgui_software_diff {
    size_t struct_size;
    size_t differing_pixels;
    unsigned long total_error;
    unsigned int max_channel_error;
} imgui_software_diff;

typedef struct imgui_software_packet_diff {
    size_t struct_size;
    size_t differing_resources;
    size_t differing_viewports;
    size_t differing_lists;
    size_t differing_vertices;
    size_t differing_indices;
    size_t differing_commands;
} imgui_software_packet_diff;

typedef struct imgui_software_target {
    size_t struct_size;
    unsigned char *pixels;
    int width;
    int height;
    size_t stride;
    imgui_u32 clear_color;
    imgui_software_texture_resolver texture_resolver;
    void *texture_user_data;
    imgui_software_command_callback command_callback;
    void *command_user_data;
} imgui_software_target;

IMGUI_API void imgui_software_target_init(imgui_software_target *target);
IMGUI_API void imgui_software_diff_init(imgui_software_diff *diff);
IMGUI_API void imgui_software_packet_diff_init(
    imgui_software_packet_diff *diff);
IMGUI_API imgui_result imgui_software_compare_packets(
    const imgui_render_packet *left,
    const imgui_render_packet *right,
    imgui_software_packet_diff *diff);
IMGUI_API imgui_result imgui_software_compare_rgba8(
    const unsigned char *left, size_t left_stride,
    const unsigned char *right, size_t right_stride,
    int width, int height, unsigned int tolerance,
    imgui_software_diff *diff);
IMGUI_API imgui_result imgui_software_render_packet(
    const imgui_render_packet *packet, imgui_software_target *target);

#ifdef __cplusplus
}
#endif

#endif

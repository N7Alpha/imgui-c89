#ifndef IMGUI_C89_RENDER_H
#define IMGUI_C89_RENDER_H

/* Immutable renderer-facing protocol and ordered custom-command API. */

#include "imgui_c89.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef imgui_flags imgui_renderer_capabilities;

enum {
    IMGUI_RENDERER_CAP_NONE = 0,
    IMGUI_RENDERER_CAP_VERTEX_OFFSET = 1UL << 0,
    IMGUI_RENDERER_CAP_TEXTURES = 1UL << 1,
    IMGUI_RENDERER_CAP_ORDERED_TEXTURE_COPY = 1UL << 2,
    IMGUI_RENDERER_CAP_CUSTOM_COMMANDS = 1UL << 3,
    IMGUI_RENDERER_CAP_MULTI_VIEWPORT = 1UL << 4,
    IMGUI_RENDERER_CAP_PACKET_SERIALIZATION = 1UL << 5,
    IMGUI_RENDERER_CAP_TEXTURE_CLEAR = 1UL << 6,
    IMGUI_RENDERER_CAP_TEXTURE_UPDATE = 1UL << 7
};

typedef enum imgui_sampler {
    IMGUI_SAMPLER_LINEAR = 0,
    IMGUI_SAMPLER_NEAREST = 1,
    IMGUI_SAMPLER_CUSTOM = 2
} imgui_sampler;

typedef enum imgui_resource_operation_type {
    IMGUI_RESOURCE_CREATE_TEXTURE = 0,
    IMGUI_RESOURCE_UPLOAD_TEXTURE = 1,
    IMGUI_RESOURCE_UPDATE_TEXTURE = 2,
    IMGUI_RESOURCE_DESTROY_TEXTURE = 3
} imgui_resource_operation_type;

typedef enum imgui_render_command_type {
    IMGUI_RENDER_COMMAND_DRAW_INDEXED = 0,
    IMGUI_RENDER_COMMAND_TEXTURE_COPY = 1,
    IMGUI_RENDER_COMMAND_RESET_STATE = 2,
    IMGUI_RENDER_COMMAND_SET_SAMPLER = 3,
    IMGUI_RENDER_COMMAND_CUSTOM = 4,
    IMGUI_RENDER_COMMAND_TEXTURE_CLEAR = 5,
    IMGUI_RENDER_COMMAND_TEXTURE_UPDATE = 6
} imgui_render_command_type;

typedef struct imgui_renderer_desc {
    size_t struct_size;
    imgui_renderer_capabilities capabilities;
    int maximum_texture_width;
    int maximum_texture_height;
    const char *backend_name;
} imgui_renderer_desc;

typedef struct imgui_texture_region {
    int x;
    int y;
    int width;
    int height;
} imgui_texture_region;

typedef struct imgui_texture_copy_command {
    imgui_texture *source;
    imgui_texture *destination;
    imgui_texture_region source_region;
    int destination_x;
    int destination_y;
    imgui_flags flags;
} imgui_texture_copy_command;

typedef struct imgui_texture_clear_command {
    imgui_texture *texture;
    imgui_texture_region region;
    imgui_u32 color;
    imgui_flags flags;
} imgui_texture_clear_command;

typedef struct imgui_texture_update_command {
    imgui_texture *texture;
    imgui_texture_region region;
    imgui_texture_format format;
    const void *pixels;
    size_t pitch;
} imgui_texture_update_command;

typedef struct imgui_resource_operation {
    imgui_resource_operation_type type;
    imgui_texture *texture;
    imgui_texture_region region;
    imgui_texture_format format;
    int texture_width;
    int texture_height;
    const void *pixels;
    size_t pitch;
} imgui_resource_operation;

typedef struct imgui_render_vertex {
    imgui_vec2 position;
    imgui_vec2 uv;
    imgui_u32 color;
} imgui_render_vertex;

#if defined(IMGUI_RENDER_INDEX_32)
typedef imgui_u32 imgui_render_index;
#else
typedef imgui_u16 imgui_render_index;
#endif

typedef struct imgui_draw_indexed_command {
    imgui_rect clip_rect;
    imgui_texture *texture;
    imgui_u32 index_offset;
    imgui_u32 index_count;
    imgui_u32 vertex_offset;
} imgui_draw_indexed_command;

typedef struct imgui_sampler_command {
    imgui_sampler sampler;
    imgui_u32 custom_sampler_id;
} imgui_sampler_command;

typedef struct imgui_custom_command {
    imgui_u32 command_id;
    const void *payload;
    size_t payload_size;
} imgui_custom_command;

typedef union imgui_render_command_data {
    imgui_draw_indexed_command draw_indexed;
    imgui_texture_copy_command texture_copy;
    imgui_texture_clear_command texture_clear;
    imgui_texture_update_command texture_update;
    imgui_sampler_command set_sampler;
    imgui_custom_command custom;
} imgui_render_command_data;

typedef struct imgui_render_command {
    imgui_render_command_type type;
    imgui_render_command_data data;
} imgui_render_command;

typedef struct imgui_render_list {
    const imgui_render_vertex *vertices;
    imgui_u32 vertex_count;
    const imgui_render_index *indices;
    imgui_u32 index_count;
    const imgui_render_command *commands;
    imgui_u32 command_count;
} imgui_render_list;

typedef struct imgui_viewport_packet {
    imgui_id viewport_id;
    imgui_vec2 display_position;
    imgui_vec2 display_size;
    imgui_vec2 framebuffer_scale;
    const imgui_render_list *lists;
    imgui_u32 list_count;
} imgui_viewport_packet;

struct imgui_render_packet {
    size_t struct_size;
    imgui_u32 protocol_version;
    imgui_u32 frame_index;
    const imgui_resource_operation *resource_operations;
    imgui_u32 resource_operation_count;
    const imgui_viewport_packet *viewports;
    imgui_u32 viewport_count;
    imgui_bool owned;
    void *private_data;
};

/* Renderer configuration is copied into the context. */
IMGUI_API void imgui_renderer_desc_init(imgui_renderer_desc *desc);
IMGUI_API imgui_result imgui_renderer_configure(
    imgui_context *ctx,
    const imgui_renderer_desc *desc);
IMGUI_API imgui_renderer_capabilities imgui_renderer_get_capabilities(
    const imgui_context *ctx);
IMGUI_API imgui_bool imgui_renderer_has_capability(
    const imgui_context *ctx,
    imgui_renderer_capabilities capability);

/* Basic geometry and ordered operations inserted into the current draw list. */
IMGUI_API imgui_result imgui_draw_list_add_rect(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    imgui_rect rect,
    imgui_u32 color,
    imgui_texture *texture);
IMGUI_API imgui_result imgui_draw_list_add_rect_rounded(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    imgui_rect rect,
    float radius,
    imgui_u32 color,
    int corner_segments,
    imgui_texture *texture);
IMGUI_API imgui_result imgui_draw_list_add_text(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    imgui_vec2 position,
    const char *text,
    imgui_u32 color);
IMGUI_API imgui_result imgui_draw_list_add_image(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    imgui_texture *texture,
    imgui_rect rect,
    imgui_vec2 uv_min,
    imgui_vec2 uv_max,
    imgui_u32 color);
IMGUI_API imgui_result imgui_draw_list_add_image_quad(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_texture *texture,
    imgui_vec2 a, imgui_vec2 b, imgui_vec2 c, imgui_vec2 d,
    imgui_vec2 uv_a, imgui_vec2 uv_b, imgui_vec2 uv_c, imgui_vec2 uv_d,
    imgui_u32 color);
IMGUI_API imgui_result imgui_draw_list_add_line(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    imgui_vec2 start,
    imgui_vec2 end,
    imgui_u32 color,
    float thickness);
IMGUI_API imgui_result imgui_draw_list_add_triangle_filled(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_vec2 a, imgui_vec2 b, imgui_vec2 c, imgui_u32 color);
IMGUI_API imgui_result imgui_draw_list_add_triangle(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_vec2 a, imgui_vec2 b, imgui_vec2 c,
    imgui_u32 color, float thickness);
IMGUI_API imgui_result imgui_draw_list_add_quad_filled(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_vec2 a, imgui_vec2 b, imgui_vec2 c, imgui_vec2 d,
    imgui_u32 color);
IMGUI_API imgui_result imgui_draw_list_add_quad(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_vec2 a, imgui_vec2 b, imgui_vec2 c, imgui_vec2 d,
    imgui_u32 color, float thickness);
IMGUI_API imgui_result imgui_draw_list_add_circle(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    imgui_vec2 center,
    float radius,
    imgui_u32 color,
    int segments);
IMGUI_API imgui_result imgui_draw_list_add_polyline(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    const imgui_vec2 *points,
    size_t point_count,
    imgui_u32 color,
    float thickness,
    imgui_bool closed);

/* Bounded path builder for translated custom geometry.  begin() resets the
   path; fill() or stroke() emits it and ends the path. */
IMGUI_API imgui_result imgui_draw_list_path_begin(
    imgui_context *ctx, imgui_draw_list *draw_list);
IMGUI_API imgui_result imgui_draw_list_path_line_to(
    imgui_context *ctx, imgui_draw_list *draw_list, imgui_vec2 point);
IMGUI_API imgui_result imgui_draw_list_path_arc_to(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_vec2 center, float radius, float angle_min, float angle_max,
    int segments);
IMGUI_API imgui_result imgui_draw_list_path_fill(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_u32 color, imgui_texture *texture);
IMGUI_API imgui_result imgui_draw_list_path_stroke(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_u32 color, float thickness, imgui_bool closed);
IMGUI_API imgui_result imgui_draw_list_add_texture_copy(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    const imgui_texture_copy_command *command);
IMGUI_API imgui_result imgui_draw_list_add_texture_clear(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    const imgui_texture_clear_command *command);
IMGUI_API imgui_result imgui_draw_list_add_texture_update(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    const imgui_texture_update_command *command);
IMGUI_API imgui_result imgui_draw_list_add_reset_state(
    imgui_context *ctx,
    imgui_draw_list *draw_list);
IMGUI_API imgui_result imgui_draw_list_add_sampler(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    imgui_sampler sampler,
    imgui_u32 custom_sampler_id);
IMGUI_API imgui_result imgui_draw_list_add_custom_command(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    imgui_u32 command_id,
    const void *payload,
    size_t payload_size);

/* Split a draw list into ordered channels.  Commands remain in channel order
   after merge; vertex and index storage is shared by all channels. */
IMGUI_API imgui_result imgui_draw_list_split(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    int channel_count);
IMGUI_API imgui_result imgui_draw_list_set_channel(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    int channel_index);
IMGUI_API imgui_result imgui_draw_list_merge(
    imgui_context *ctx,
    imgui_draw_list *draw_list);

/* A clone owns all arrays and copied payloads and may outlive its context. */
IMGUI_API imgui_result imgui_render_packet_clone(
    const imgui_render_packet *source,
    const imgui_allocator *allocator,
    imgui_render_packet **out_packet);
IMGUI_API void imgui_render_packet_destroy(
    imgui_render_packet *packet,
    const imgui_allocator *allocator);

#ifdef __cplusplus
}
#endif

#endif

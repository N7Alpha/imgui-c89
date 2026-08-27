#include "imgui_c89.h"
#include "imgui_c89_font.h"
#include "imgui_c89_trace.h"
#include "imgui_c89_render.h"
#include "imgui_c89_software.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct trace_runner_user_data {
    int dump;
    imgui_font_atlas *font_atlas;
    imgui_texture *font_texture;
} trace_runner_user_data;

static imgui_u32 trace_hash_bytes(imgui_u32 hash, const void *data,
                                  size_t length);

static imgui_bool trace_texture_resolver(const imgui_texture *texture,
                                         imgui_software_texture_view *view,
                                         void *user_data)
{
    trace_runner_user_data *data = (trace_runner_user_data *)user_data;
    imgui_font_atlas_pixels pixels;
    if (data == NULL || texture != data->font_texture ||
        imgui_font_atlas_get_pixels(data->font_atlas, &pixels) !=
        IMGUI_RESULT_OK) return IMGUI_FALSE;
    view->pixels = pixels.pixels;
    view->mutable_pixels = NULL;
    view->width = pixels.width;
    view->height = pixels.height;
    view->stride = (size_t)pixels.width;
    view->format = IMGUI_TEXTURE_FORMAT_ALPHA8;
    return IMGUI_TRUE;
}

static imgui_bool trace_flat_texture_resolver(const imgui_texture *texture,
                                              imgui_software_texture_view *view,
                                              void *user_data)
{
    static const unsigned char white = 255;
    (void)texture;
    (void)user_data;
    if (view == NULL) return IMGUI_FALSE;
    view->pixels = &white;
    view->mutable_pixels = NULL;
    view->width = 1;
    view->height = 1;
    view->stride = 1;
    view->format = IMGUI_TEXTURE_FORMAT_ALPHA8;
    return IMGUI_TRUE;
}

static imgui_u32 trace_framebuffer_hash(const unsigned char *pixels,
                                        size_t length)
{
    return trace_hash_bytes((imgui_u32)2166136261UL, pixels, length);
}

static imgui_u32 trace_hash_bytes(imgui_u32 hash, const void *data,
                                  size_t length)
{
    const unsigned char *bytes;
    size_t index;
    bytes = (const unsigned char *)data;
    for (index = 0; index < length; ++index) {
        hash ^= (imgui_u32)bytes[index];
        hash *= (imgui_u32)16777619UL;
    }
    return hash;
}

static imgui_u32 trace_hash_vertex(imgui_u32 hash,
                                   const imgui_render_vertex *vertex)
{
    hash = trace_hash_bytes(hash, &vertex->position,
                            sizeof(vertex->position));
    return trace_hash_bytes(hash, &vertex->color, sizeof(vertex->color));
}

static imgui_result trace_frame(imgui_context *ctx,
                                float delta_time,
                                double time,
                                void *user_data)
{
    imgui_frame_desc frame;
    imgui_scope scope;
    imgui_vec2 window_size;
    const imgui_render_packet *packet;
    imgui_u32 command_count;
    imgui_u32 vertex_count;
    imgui_u32 index_count;
    int dump;
    imgui_u32 geometry_hash;
    dump = user_data != NULL ?
        ((trace_runner_user_data *)user_data)->dump : 0;
    imgui_frame_desc_init(&frame);
    frame.display_size = imgui_make_vec2(640.0f, 480.0f);
    frame.delta_time = delta_time;
    frame.time = time;
    if (imgui_frame_begin(ctx, &frame) != IMGUI_RESULT_OK) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    scope = imgui_window_begin(ctx, "Trace runner");
    if (scope == IMGUI_SCOPE_ERROR) return IMGUI_RESULT_INVALID_STATE;
    imgui_text(ctx, "Deterministic C89 trace frame");
    window_size = imgui_get_window_size(ctx);
    imgui_window_end(ctx);
    if (imgui_frame_end(ctx) != IMGUI_RESULT_OK) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    packet = imgui_render(ctx);
    if (packet == NULL) return IMGUI_RESULT_INVALID_STATE;
    {
        unsigned char *framebuffer;
        imgui_software_target target;
        size_t framebuffer_size = (size_t)640U * 480U * 4U;
        framebuffer = (unsigned char *)calloc(1, framebuffer_size);
        if (framebuffer == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
        imgui_software_target_init(&target);
        target.pixels = framebuffer;
        target.width = 640;
        target.height = 480;
        target.stride = (size_t)640U * 4U;
        target.texture_resolver = trace_texture_resolver;
        target.texture_user_data = user_data;
        if (imgui_software_render_packet(packet, &target) !=
            IMGUI_RESULT_OK) {
            free(framebuffer);
            return IMGUI_RESULT_INVALID_STATE;
        }
        if (dump) {
            printf("pixel_hash=%lu\n",
                   (unsigned long)trace_framebuffer_hash(framebuffer,
                                                         framebuffer_size));
        }
        if (dump) {
            target.texture_resolver = trace_flat_texture_resolver;
            memset(framebuffer, 0, framebuffer_size);
            if (imgui_software_render_packet(packet, &target) !=
                IMGUI_RESULT_OK) {
                free(framebuffer);
                return IMGUI_RESULT_INVALID_STATE;
            }
            printf("flat_pixel_hash=%lu\n",
                   (unsigned long)trace_framebuffer_hash(framebuffer,
                                                         framebuffer_size));
        }
        free(framebuffer);
    }
    command_count = 0;
    vertex_count = 0;
    index_count = 0;
    if (packet->viewport_count != 0 && packet->viewports != NULL &&
        packet->viewports[0].list_count != 0 &&
        packet->viewports[0].lists != NULL) {
        command_count = packet->viewports[0].lists[0].command_count;
        vertex_count = packet->viewports[0].lists[0].vertex_count;
        index_count = packet->viewports[0].lists[0].index_count;
    }
    printf("frame=%lu time=%.17g viewports=%lu commands=%lu vertices=%lu "
           "indices=%lu window=%.9g,%.9g\n",
           (unsigned long)packet->frame_index, time,
           (unsigned long)packet->viewport_count,
           (unsigned long)command_count, (unsigned long)vertex_count,
           (unsigned long)index_count, (double)window_size.x,
           (double)window_size.y);
    if (dump && packet->viewport_count != 0 && packet->viewports != NULL &&
        packet->viewports[0].list_count != 0 &&
        packet->viewports[0].lists != NULL) {
        const imgui_render_list *list = &packet->viewports[0].lists[0];
        imgui_u32 index;
        geometry_hash = (imgui_u32)2166136261UL;
        for (index = 0; index < list->command_count; ++index) {
            const imgui_render_command *command = &list->commands[index];
            if (command->type == IMGUI_RENDER_COMMAND_DRAW_INDEXED) {
                imgui_u32 draw_index;
                geometry_hash = trace_hash_bytes(
                    geometry_hash, &command->data.draw_indexed.clip_rect,
                    sizeof(command->data.draw_indexed.clip_rect));
                for (draw_index = 0;
                     draw_index < command->data.draw_indexed.index_count;
                     ++draw_index) {
                    imgui_u32 index_offset =
                        command->data.draw_indexed.index_offset + draw_index;
                    if (index_offset < list->index_count) {
                        imgui_u32 vertex_offset =
                            command->data.draw_indexed.vertex_offset +
                            (imgui_u32)list->indices[index_offset];
                        if (vertex_offset < list->vertex_count) {
                            geometry_hash = trace_hash_vertex(
                                geometry_hash,
                                &list->vertices[vertex_offset]);
                        }
                    }
                }
                printf("draw type=0 clip=%.9g,%.9g,%.9g,%.9g elem=%lu "
                       "idx=%lu vtx=%lu\n",
                       (double)command->data.draw_indexed.clip_rect.x1,
                       (double)command->data.draw_indexed.clip_rect.y1,
                       (double)command->data.draw_indexed.clip_rect.x2,
                       (double)command->data.draw_indexed.clip_rect.y2,
                       (unsigned long)command->data.draw_indexed.index_count,
                       (unsigned long)command->data.draw_indexed.index_offset,
                       (unsigned long)command->data.draw_indexed.vertex_offset);
                if (command->data.draw_indexed.index_count != 0U &&
                    command->data.draw_indexed.index_offset <
                        list->index_count) {
                    imgui_u32 first_index =
                        command->data.draw_indexed.index_offset;
                    imgui_u32 last_index = first_index +
                        command->data.draw_indexed.index_count - 1U;
                    if (last_index < list->index_count) {
                        imgui_render_index first_vertex_index =
                            list->indices[first_index];
                        imgui_render_index last_vertex_index =
                            list->indices[last_index];
                        imgui_u32 first_vertex_offset =
                            command->data.draw_indexed.vertex_offset +
                            (imgui_u32)first_vertex_index;
                        imgui_u32 last_vertex_offset =
                            command->data.draw_indexed.vertex_offset +
                            (imgui_u32)last_vertex_index;
                        if (first_vertex_offset < list->vertex_count &&
                            last_vertex_offset < list->vertex_count) {
                            const imgui_render_vertex *first_vertex =
                                &list->vertices[first_vertex_offset];
                            const imgui_render_vertex *last_vertex =
                                &list->vertices[last_vertex_offset];
                        printf("range first=%.9g,%.9g last=%.9g,%.9g\n",
                               (double)first_vertex->position.x,
                               (double)first_vertex->position.y,
                               (double)last_vertex->position.x,
                               (double)last_vertex->position.y);
                        }
                    }
                }
            } else {
                printf("command type=%d\n", (int)command->type);
            }
        }
        printf("geometry_hash=%lu\n", (unsigned long)geometry_hash);
    }
    return IMGUI_RESULT_OK;
}

static imgui_result bind_reference_font(imgui_context *ctx,
                                        imgui_font_atlas **out_atlas,
                                        imgui_texture **out_texture)
{
    FILE *file;
    long file_size;
    unsigned char *bytes;
    size_t read_size;
    imgui_font_atlas *atlas;
    imgui_font *font;
    imgui_font_config font_config;
    imgui_font_atlas_pixels pixels;
    imgui_texture_desc texture_desc;
    imgui_texture *texture;
    imgui_renderer_desc renderer;
    imgui_result result;
    if (out_atlas != NULL) *out_atlas = NULL;
    if (out_texture != NULL) *out_texture = NULL;
    file = fopen("third_party/ProggyClean.ttf", "rb");
    if (file == NULL) return IMGUI_RESULT_UNSUPPORTED;
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return IMGUI_RESULT_CORRUPT_DATA;
    }
    file_size = ftell(file);
    if (file_size <= 0L || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return IMGUI_RESULT_CORRUPT_DATA;
    }
    bytes = (unsigned char *)malloc((size_t)file_size);
    if (bytes == NULL) {
        fclose(file);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    read_size = fread(bytes, 1, (size_t)file_size, file);
    fclose(file);
    if (read_size != (size_t)file_size) {
        free(bytes);
        return IMGUI_RESULT_CORRUPT_DATA;
    }
    imgui_renderer_desc_init(&renderer);
    renderer.capabilities = IMGUI_RENDERER_CAP_VERTEX_OFFSET |
                            IMGUI_RENDERER_CAP_TEXTURES;
    result = imgui_renderer_configure(ctx, &renderer);
    if (result != IMGUI_RESULT_OK) {
        free(bytes);
        return result;
    }
    result = imgui_font_atlas_create(ctx, &atlas);
    if (result == IMGUI_RESULT_OK) {
        imgui_font_config_init(&font_config);
        font_config.pixel_height = 13.0f;
        font_config.first_codepoint = 32UL;
        font_config.last_codepoint = 126UL;
        font_config.atlas_width = 256;
        result = imgui_font_atlas_add_ttf(atlas, bytes, read_size,
                                          &font_config, &font);
    }
    free(bytes);
    if (result == IMGUI_RESULT_OK) result = imgui_font_atlas_build(atlas);
    if (result == IMGUI_RESULT_OK) {
        result = imgui_font_atlas_get_pixels(atlas, &pixels);
    }
    texture = NULL;
    if (result == IMGUI_RESULT_OK) {
        imgui_texture_desc_init(&texture_desc);
        texture_desc.format = IMGUI_TEXTURE_FORMAT_ALPHA8;
        texture_desc.width = pixels.width;
        texture_desc.height = pixels.height;
        result = imgui_texture_create(ctx, &texture_desc, NULL, 0, &texture);
    }
    if (result == IMGUI_RESULT_OK) {
        result = imgui_font_atlas_upload(ctx, atlas, texture);
    }
    if (result == IMGUI_RESULT_OK) result = imgui_font_bind(ctx, font, texture);
    if (result != IMGUI_RESULT_OK) {
        if (texture != NULL) (void)imgui_texture_destroy(ctx, texture);
        if (atlas != NULL) imgui_font_atlas_destroy(atlas);
        return result;
    }
    if (out_atlas != NULL) *out_atlas = atlas;
    if (out_texture != NULL) *out_texture = texture;
    return IMGUI_RESULT_OK;
}

int main(int argc, char **argv)
{
    FILE *file;
    long file_length;
    char *data;
    size_t length;
    imgui_config config;
    imgui_context *ctx;
    imgui_trace *trace;
    imgui_result result;
    imgui_font_atlas *font_atlas;
    imgui_texture *font_texture;
    int dump;
    trace_runner_user_data user_data;
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "usage: %s trace.txt [--dump]\n", argv[0]);
        return 2;
    }
    dump = argc == 3 && strcmp(argv[2], "--dump") == 0;
    if (argc == 3 && !dump) return 2;
    file = fopen(argv[1], "rb");
    if (file == NULL) {
        fprintf(stderr, "cannot open trace: %s\n", argv[1]);
        return 2;
    }
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return 2;
    }
    file_length = ftell(file);
    if (file_length < 0L || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return 2;
    }
    length = (size_t)file_length;
    data = (char *)malloc(length == 0 ? 1 : length);
    if (data == NULL || (length != 0 &&
                         fread(data, 1, length, file) != length)) {
        free(data);
        fclose(file);
        return 2;
    }
    fclose(file);
    imgui_config_init(&config);
    ctx = imgui_context_create(&config);
    font_atlas = NULL;
    font_texture = NULL;
    if (ctx != NULL) {
        (void)bind_reference_font(ctx, &font_atlas, &font_texture);
    }
    user_data.dump = dump;
    user_data.font_atlas = font_atlas;
    user_data.font_texture = font_texture;
    trace = NULL;
    if (ctx == NULL || imgui_trace_create(&config.allocator, &trace) !=
        IMGUI_RESULT_OK || trace == NULL) {
        imgui_context_destroy(ctx);
        free(data);
        return 2;
    }
    result = imgui_trace_load(trace, data, length);
    if (result == IMGUI_RESULT_OK) {
        result = imgui_trace_replay_frames(ctx, trace, 0,
                                           imgui_trace_get_count(trace),
                                           trace_frame, &user_data);
    }
    if (result != IMGUI_RESULT_OK) {
        fprintf(stderr, "trace replay failed: %d\n", (int)result);
    }
    imgui_trace_destroy(trace);
    if (font_atlas != NULL) imgui_font_atlas_destroy(font_atlas);
    imgui_context_destroy(ctx);
    free(data);
    return result == IMGUI_RESULT_OK ? 0 : 1;
}

#include "../include/imgui_c89_software.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static imgui_bool imgui_software_target_normalize(
    const imgui_software_target *source,
    imgui_software_target *destination)
{
    size_t copy_size;
    if (source == NULL || destination == NULL ||
        source->struct_size < sizeof(source->struct_size)) {
        return IMGUI_FALSE;
    }
    imgui_software_target_init(destination);
    copy_size = source->struct_size < sizeof(*destination) ?
        source->struct_size : sizeof(*destination);
    memcpy(destination, source, copy_size);
    destination->struct_size = sizeof(*destination);
    return IMGUI_TRUE;
}

static unsigned char imgui_software_channel(imgui_u32 color, int shift)
{
    return (unsigned char)((color >> shift) & 0xffU);
}

static float imgui_software_edge(float ax, float ay, float bx, float by,
                                 float px, float py)
{
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

void imgui_software_target_init(imgui_software_target *target)
{
    if (target == NULL) return;
    memset(target, 0, sizeof(*target));
    target->struct_size = sizeof(*target);
    target->clear_color = 0xff000000UL;
}

void imgui_software_diff_init(imgui_software_diff *diff)
{
    if (diff == NULL) return;
    memset(diff, 0, sizeof(*diff));
    diff->struct_size = sizeof(*diff);
}

void imgui_software_packet_diff_init(imgui_software_packet_diff *diff)
{
    if (diff == NULL) return;
    memset(diff, 0, sizeof(*diff));
    diff->struct_size = sizeof(*diff);
}

static imgui_bool imgui_software_same_vertex(const imgui_render_vertex *left,
                                             const imgui_render_vertex *right)
{
    return left->position.x == right->position.x &&
           left->position.y == right->position.y &&
           left->uv.x == right->uv.x && left->uv.y == right->uv.y &&
           left->color == right->color;
}

static imgui_bool imgui_software_command_known(
    imgui_render_command_type type)
{
    return type == IMGUI_RENDER_COMMAND_DRAW_INDEXED ||
           type == IMGUI_RENDER_COMMAND_TEXTURE_COPY ||
           type == IMGUI_RENDER_COMMAND_TEXTURE_CLEAR ||
           type == IMGUI_RENDER_COMMAND_TEXTURE_UPDATE ||
           type == IMGUI_RENDER_COMMAND_SET_SAMPLER ||
           type == IMGUI_RENDER_COMMAND_RESET_STATE ||
           type == IMGUI_RENDER_COMMAND_CUSTOM;
}

static imgui_bool imgui_software_same_command(
    const imgui_render_command *left, const imgui_render_command *right)
{
    if (!imgui_software_command_known(left->type) ||
        !imgui_software_command_known(right->type)) return IMGUI_FALSE;
    if (left->type != right->type) return IMGUI_FALSE;
    if (left->type == IMGUI_RENDER_COMMAND_DRAW_INDEXED) {
        return imgui_texture_equivalent(left->data.draw_indexed.texture,
                                        right->data.draw_indexed.texture) &&
            left->data.draw_indexed.index_offset == right->data.draw_indexed.index_offset &&
            left->data.draw_indexed.index_count == right->data.draw_indexed.index_count &&
            left->data.draw_indexed.vertex_offset == right->data.draw_indexed.vertex_offset &&
            left->data.draw_indexed.clip_rect.x1 == right->data.draw_indexed.clip_rect.x1 &&
            left->data.draw_indexed.clip_rect.y1 == right->data.draw_indexed.clip_rect.y1 &&
            left->data.draw_indexed.clip_rect.x2 == right->data.draw_indexed.clip_rect.x2 &&
            left->data.draw_indexed.clip_rect.y2 == right->data.draw_indexed.clip_rect.y2;
    }
    if (left->type == IMGUI_RENDER_COMMAND_TEXTURE_COPY) {
        return imgui_texture_equivalent(left->data.texture_copy.source,
                                        right->data.texture_copy.source) &&
            imgui_texture_equivalent(left->data.texture_copy.destination,
                                     right->data.texture_copy.destination) &&
            left->data.texture_copy.source_region.x == right->data.texture_copy.source_region.x &&
            left->data.texture_copy.source_region.y == right->data.texture_copy.source_region.y &&
            left->data.texture_copy.source_region.width == right->data.texture_copy.source_region.width &&
            left->data.texture_copy.source_region.height == right->data.texture_copy.source_region.height &&
            left->data.texture_copy.destination_x == right->data.texture_copy.destination_x &&
            left->data.texture_copy.destination_y == right->data.texture_copy.destination_y &&
            left->data.texture_copy.flags == right->data.texture_copy.flags;
    }
    if (left->type == IMGUI_RENDER_COMMAND_TEXTURE_CLEAR) {
        return imgui_texture_equivalent(left->data.texture_clear.texture,
                                        right->data.texture_clear.texture) &&
            left->data.texture_clear.region.x ==
                right->data.texture_clear.region.x &&
            left->data.texture_clear.region.y ==
                right->data.texture_clear.region.y &&
            left->data.texture_clear.region.width ==
                right->data.texture_clear.region.width &&
            left->data.texture_clear.region.height ==
                right->data.texture_clear.region.height &&
            left->data.texture_clear.color ==
                right->data.texture_clear.color &&
            left->data.texture_clear.flags ==
                right->data.texture_clear.flags;
    }
    if (left->type == IMGUI_RENDER_COMMAND_TEXTURE_UPDATE) {
        if (!imgui_texture_equivalent(left->data.texture_update.texture,
                                      right->data.texture_update.texture) ||
            left->data.texture_update.region.x !=
                right->data.texture_update.region.x ||
            left->data.texture_update.region.y !=
                right->data.texture_update.region.y ||
            left->data.texture_update.region.width !=
                right->data.texture_update.region.width ||
            left->data.texture_update.region.height !=
                right->data.texture_update.region.height ||
            left->data.texture_update.format !=
                right->data.texture_update.format ||
            left->data.texture_update.pitch !=
                right->data.texture_update.pitch) {
            return IMGUI_FALSE;
        }
        if (left->data.texture_update.pixels == NULL ||
            right->data.texture_update.pixels == NULL) {
            return left->data.texture_update.pixels ==
                   right->data.texture_update.pixels;
        }
        if (left->data.texture_update.pitch == 0 ||
            left->data.texture_update.region.height < 0 ||
            (size_t)left->data.texture_update.region.height >
                (size_t)-1 / left->data.texture_update.pitch) {
            return IMGUI_FALSE;
        }
        return memcmp(left->data.texture_update.pixels,
                      right->data.texture_update.pixels,
                      left->data.texture_update.pitch *
                      (size_t)left->data.texture_update.region.height) == 0;
    }
    if (left->type == IMGUI_RENDER_COMMAND_SET_SAMPLER) {
        return left->data.set_sampler.sampler == right->data.set_sampler.sampler &&
            left->data.set_sampler.custom_sampler_id ==
            right->data.set_sampler.custom_sampler_id;
    }
    if (left->type == IMGUI_RENDER_COMMAND_CUSTOM) {
        if (left->data.custom.command_id != right->data.custom.command_id ||
            left->data.custom.payload_size != right->data.custom.payload_size) {
            return IMGUI_FALSE;
        }
        if (left->data.custom.payload_size == 0) return IMGUI_TRUE;
        if (left->data.custom.payload == NULL ||
            right->data.custom.payload == NULL) return IMGUI_FALSE;
        return memcmp(left->data.custom.payload, right->data.custom.payload,
                      left->data.custom.payload_size) == 0;
    }
    return IMGUI_TRUE;
}

static imgui_bool imgui_software_same_resource(
    const imgui_resource_operation *left,
    const imgui_resource_operation *right)
{
    size_t byte_count;
    if (left->type != right->type ||
        !imgui_texture_equivalent(left->texture, right->texture) ||
        left->region.x != right->region.x ||
        left->region.y != right->region.y ||
        left->region.width != right->region.width ||
        left->region.height != right->region.height ||
        left->format != right->format ||
        left->texture_width != right->texture_width ||
        left->texture_height != right->texture_height ||
        left->pitch != right->pitch) {
        return IMGUI_FALSE;
    }
    if (left->pixels == NULL || right->pixels == NULL) {
        return left->pixels == right->pixels;
    }
    byte_count = left->pitch;
    if (byte_count == 0) return IMGUI_TRUE;
    if (left->type == IMGUI_RESOURCE_CREATE_TEXTURE) {
        if (left->texture_height < 0 ||
            (size_t)left->texture_height > (size_t)-1 / byte_count) {
            return IMGUI_FALSE;
        }
        byte_count *= (size_t)left->texture_height;
    } else {
        if (left->region.height < 0 ||
            (size_t)left->region.height > (size_t)-1 / byte_count) {
            return IMGUI_FALSE;
        }
        byte_count *= (size_t)left->region.height;
    }
    return memcmp(left->pixels, right->pixels, byte_count) == 0;
}

imgui_result imgui_software_compare_packets(
    const imgui_render_packet *left, const imgui_render_packet *right,
    imgui_software_packet_diff *diff)
{
    imgui_software_packet_diff local_diff;
    imgui_software_packet_diff *destination_diff;
    size_t copy_size;
    imgui_u32 viewport_index;
    imgui_u32 list_index;
    imgui_u32 item_index;
    const imgui_viewport_packet *left_viewport;
    const imgui_viewport_packet *right_viewport;
    const imgui_render_list *left_list;
    const imgui_render_list *right_list;
    imgui_u32 validation_index;
    imgui_u32 validation_list_index;
    imgui_u32 validation_command_index;
    if (left == NULL || right == NULL || diff == NULL ||
        diff->struct_size < sizeof(diff->struct_size) ||
        left->struct_size < sizeof(*left) || right->struct_size < sizeof(*right) ||
        (left->viewport_count != 0 && left->viewports == NULL) ||
        (right->viewport_count != 0 && right->viewports == NULL) ||
        (left->resource_operation_count != 0 &&
         left->resource_operations == NULL) ||
        (right->resource_operation_count != 0 &&
         right->resource_operations == NULL)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    for (validation_index = 0;
         validation_index < left->viewport_count;
         ++validation_index) {
        if (left->viewports[validation_index].list_count != 0 &&
            left->viewports[validation_index].lists == NULL) {
            return IMGUI_RESULT_INVALID_ARGUMENT;
        }
        for (validation_list_index = 0;
             validation_list_index <
                 left->viewports[validation_index].list_count;
             ++validation_list_index) {
            left_list = &left->viewports[validation_index]
                .lists[validation_list_index];
            if ((left_list->vertex_count != 0 && left_list->vertices == NULL) ||
                (left_list->index_count != 0 && left_list->indices == NULL) ||
                (left_list->command_count != 0 && left_list->commands == NULL)) {
                return IMGUI_RESULT_INVALID_ARGUMENT;
            }
            for (validation_command_index = 0;
                 validation_command_index < left_list->command_count;
                 ++validation_command_index) {
                if (!imgui_software_command_known(
                        left_list->commands[validation_command_index].type)) {
                    return IMGUI_RESULT_INVALID_ARGUMENT;
                }
            }
        }
    }
    for (validation_index = 0;
         validation_index < right->viewport_count;
         ++validation_index) {
        if (right->viewports[validation_index].list_count != 0 &&
            right->viewports[validation_index].lists == NULL) {
            return IMGUI_RESULT_INVALID_ARGUMENT;
        }
        for (validation_list_index = 0;
             validation_list_index <
                 right->viewports[validation_index].list_count;
             ++validation_list_index) {
            right_list = &right->viewports[validation_index]
                .lists[validation_list_index];
            if ((right_list->vertex_count != 0 && right_list->vertices == NULL) ||
                (right_list->index_count != 0 && right_list->indices == NULL) ||
                (right_list->command_count != 0 && right_list->commands == NULL)) {
                return IMGUI_RESULT_INVALID_ARGUMENT;
            }
            for (validation_command_index = 0;
                 validation_command_index < right_list->command_count;
                 ++validation_command_index) {
                if (!imgui_software_command_known(
                        right_list->commands[validation_command_index].type)) {
                    return IMGUI_RESULT_INVALID_ARGUMENT;
                }
            }
        }
    }
    destination_diff = diff;
    imgui_software_packet_diff_init(&local_diff);
    diff = &local_diff;
    diff->differing_viewports = left->viewport_count > right->viewport_count ?
        left->viewport_count - right->viewport_count :
        right->viewport_count - left->viewport_count;
    diff->differing_lists = 0;
    diff->differing_resources = left->resource_operation_count >
        right->resource_operation_count ? left->resource_operation_count -
        right->resource_operation_count : right->resource_operation_count -
        left->resource_operation_count;
    diff->differing_vertices = 0;
    diff->differing_indices = 0;
    diff->differing_commands = 0;
    for (item_index = 0; item_index < left->resource_operation_count &&
         item_index < right->resource_operation_count; ++item_index) {
        if (!imgui_software_same_resource(&left->resource_operations[item_index],
                                          &right->resource_operations[item_index])) {
            ++diff->differing_resources;
        }
    }
    for (viewport_index = 0; viewport_index < left->viewport_count &&
         viewport_index < right->viewport_count; ++viewport_index) {
        left_viewport = &left->viewports[viewport_index];
        right_viewport = &right->viewports[viewport_index];
        if (left_viewport->viewport_id != right_viewport->viewport_id ||
            left_viewport->display_position.x != right_viewport->display_position.x ||
            left_viewport->display_position.y != right_viewport->display_position.y ||
            left_viewport->display_size.x != right_viewport->display_size.x ||
            left_viewport->display_size.y != right_viewport->display_size.y ||
            left_viewport->framebuffer_scale.x != right_viewport->framebuffer_scale.x ||
            left_viewport->framebuffer_scale.y != right_viewport->framebuffer_scale.y ||
            left_viewport->list_count != right_viewport->list_count) {
            ++diff->differing_viewports;
        }
        diff->differing_lists += left_viewport->list_count >
            right_viewport->list_count ? left_viewport->list_count -
            right_viewport->list_count : right_viewport->list_count -
            left_viewport->list_count;
        for (list_index = 0; list_index < left_viewport->list_count &&
             list_index < right_viewport->list_count; ++list_index) {
            left_list = &left_viewport->lists[list_index];
            right_list = &right_viewport->lists[list_index];
            diff->differing_vertices += left_list->vertex_count >
                right_list->vertex_count ? left_list->vertex_count -
                right_list->vertex_count : right_list->vertex_count -
                left_list->vertex_count;
            diff->differing_indices += left_list->index_count >
                right_list->index_count ? left_list->index_count -
                right_list->index_count : right_list->index_count -
                left_list->index_count;
            diff->differing_commands += left_list->command_count >
                right_list->command_count ? left_list->command_count -
                right_list->command_count : right_list->command_count -
                left_list->command_count;
            for (item_index = 0; item_index < left_list->vertex_count &&
                 item_index < right_list->vertex_count; ++item_index) {
                if (!imgui_software_same_vertex(&left_list->vertices[item_index],
                                                &right_list->vertices[item_index])) {
                    ++diff->differing_vertices;
                }
            }
            for (item_index = 0; item_index < left_list->index_count &&
                 item_index < right_list->index_count; ++item_index) {
                if (left_list->indices[item_index] != right_list->indices[item_index]) {
                    ++diff->differing_indices;
                }
            }
            for (item_index = 0; item_index < left_list->command_count &&
                 item_index < right_list->command_count; ++item_index) {
                if (!imgui_software_same_command(&left_list->commands[item_index],
                                                 &right_list->commands[item_index])) {
                    ++diff->differing_commands;
                }
            }
        }
    }
    copy_size = destination_diff->struct_size < sizeof(local_diff) ?
        destination_diff->struct_size : sizeof(local_diff);
    memcpy(destination_diff, &local_diff, copy_size);
    return IMGUI_RESULT_OK;
}

imgui_result imgui_software_compare_rgba8(
    const unsigned char *left, size_t left_stride,
    const unsigned char *right, size_t right_stride,
    int width, int height, unsigned int tolerance,
    imgui_software_diff *diff)
{
    imgui_software_diff local_diff;
    imgui_software_diff *destination_diff;
    size_t copy_size;
    int x;
    int y;
    int channel;
    unsigned int difference;
    unsigned int maximum;
    const unsigned char *left_pixel;
    const unsigned char *right_pixel;
    if (left == NULL || right == NULL || diff == NULL ||
        diff->struct_size < sizeof(diff->struct_size) || width <= 0 || height <= 0 ||
        left_stride < (size_t)width * 4U ||
        right_stride < (size_t)width * 4U) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    destination_diff = diff;
    imgui_software_diff_init(&local_diff);
    diff = &local_diff;
    diff->differing_pixels = 0;
    diff->total_error = 0;
    diff->max_channel_error = 0;
    for (y = 0; y < height; ++y) {
        left_pixel = left + (size_t)y * left_stride;
        right_pixel = right + (size_t)y * right_stride;
        for (x = 0; x < width; ++x) {
            maximum = 0;
            difference = 0;
            for (channel = 0; channel < 4; ++channel) {
                if (left_pixel[channel] >= right_pixel[channel]) {
                    maximum = left_pixel[channel] - right_pixel[channel];
                } else {
                    maximum = right_pixel[channel] - left_pixel[channel];
                }
                difference += maximum;
                if (maximum > diff->max_channel_error) {
                    diff->max_channel_error = maximum;
                }
            }
            if (difference > tolerance) ++diff->differing_pixels;
            diff->total_error += (unsigned long)difference;
            left_pixel += 4;
            right_pixel += 4;
        }
    }
    copy_size = destination_diff->struct_size < sizeof(local_diff) ?
        destination_diff->struct_size : sizeof(local_diff);
    memcpy(destination_diff, &local_diff, copy_size);
    return IMGUI_RESULT_OK;
}

static void imgui_software_clear(imgui_software_target *target)
{
    int x;
    int y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    unsigned char *row;
    red = imgui_software_channel(target->clear_color, 0);
    green = imgui_software_channel(target->clear_color, 8);
    blue = imgui_software_channel(target->clear_color, 16);
    alpha = imgui_software_channel(target->clear_color, 24);
    for (y = 0; y < target->height; ++y) {
        row = target->pixels + (size_t)y * target->stride;
        for (x = 0; x < target->width; ++x) {
            row[x * 4 + 0] = red;
            row[x * 4 + 1] = green;
            row[x * 4 + 2] = blue;
            row[x * 4 + 3] = alpha;
        }
    }
}

static void imgui_software_blend(unsigned char *pixel, imgui_u32 color)
{
    unsigned int source_alpha;
    unsigned int inverse_alpha;
    unsigned int source_red;
    unsigned int source_green;
    unsigned int source_blue;
    source_alpha = imgui_software_channel(color, 24);
    inverse_alpha = 255U - source_alpha;
    source_red = imgui_software_channel(color, 0);
    source_green = imgui_software_channel(color, 8);
    source_blue = imgui_software_channel(color, 16);
    pixel[0] = (unsigned char)((source_red * source_alpha +
                                pixel[0] * inverse_alpha) / 255U);
    pixel[1] = (unsigned char)((source_green * source_alpha +
                                pixel[1] * inverse_alpha) / 255U);
    pixel[2] = (unsigned char)((source_blue * source_alpha +
                                pixel[2] * inverse_alpha) / 255U);
    pixel[3] = (unsigned char)(source_alpha +
                               (pixel[3] * inverse_alpha) / 255U);
}

static void imgui_software_read_texel(const imgui_software_texture_view *view,
                                       int x, int y, unsigned char *sample)
{
    int bytes_per_pixel;
    const unsigned char *source;
    if (sample == NULL) return;
    sample[0] = 255; sample[1] = 255; sample[2] = 255; sample[3] = 255;
    if (view == NULL || view->pixels == NULL || view->width <= 0 ||
        view->height <= 0) return;
    if (x < 0) x = 0;
    if (x >= view->width) x = view->width - 1;
    if (y < 0) y = 0;
    if (y >= view->height) y = view->height - 1;
    bytes_per_pixel = view->format == IMGUI_TEXTURE_FORMAT_ALPHA8 ? 1 : 4;
    source = view->pixels + (size_t)y * view->stride +
             (size_t)x * (size_t)bytes_per_pixel;
    if (bytes_per_pixel == 1) {
        sample[3] = source[0];
    } else {
        sample[0] = source[0]; sample[1] = source[1];
        sample[2] = source[2]; sample[3] = source[3];
    }
}

static void imgui_software_sample(const imgui_software_texture_view *view,
                                  float u, float v, imgui_sampler sampler,
                                  unsigned char *sample)
{
    int x;
    int y;
    int x0;
    int y0;
    int x1;
    int y1;
    int channel;
    float fx;
    float fy;
    float tx;
    float ty;
    float top;
    float bottom;
    unsigned char top_left[4];
    unsigned char top_right[4];
    unsigned char bottom_left[4];
    unsigned char bottom_right[4];
    if (sample == NULL) return;
    sample[0] = 255; sample[1] = 255; sample[2] = 255; sample[3] = 255;
    if (view == NULL || view->pixels == NULL || view->width <= 0 ||
        view->height <= 0) return;
    if (u < 0.0f) u = 0.0f;
    if (u > 1.0f) u = 1.0f;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    /* Normalized GPU coordinates address texel centers: u*width - 0.5.
       Mapping to width-1 biases interior bilinear samples toward the
       neighboring texel and produces visible differences on scaled images. */
    fx = u * (float)view->width - 0.5f;
    fy = v * (float)view->height - 0.5f;
    if (sampler != IMGUI_SAMPLER_LINEAR) {
        x = (int)(fx + 0.5f);
        y = (int)(fy + 0.5f);
        imgui_software_read_texel(view, x, y, sample);
        return;
    }
    if (fx < 0.0f) fx = 0.0f;
    if (fy < 0.0f) fy = 0.0f;
    x0 = (int)fx;
    y0 = (int)fy;
    x1 = x0 + 1 < view->width ? x0 + 1 : x0;
    y1 = y0 + 1 < view->height ? y0 + 1 : y0;
    tx = fx - (float)x0;
    ty = fy - (float)y0;
    imgui_software_read_texel(view, x0, y0, top_left);
    imgui_software_read_texel(view, x1, y0, top_right);
    imgui_software_read_texel(view, x0, y1, bottom_left);
    imgui_software_read_texel(view, x1, y1, bottom_right);
    for (channel = 0; channel < 4; ++channel) {
        top = (float)top_left[channel] +
              ((float)top_right[channel] - (float)top_left[channel]) * tx;
        bottom = (float)bottom_left[channel] +
                 ((float)bottom_right[channel] - (float)bottom_left[channel]) * tx;
        sample[channel] = (unsigned char)(top + (bottom - top) * ty + 0.5f);
    }
}

static imgui_bool imgui_software_copy_texture(
    imgui_software_target *target, const imgui_texture_copy_command *copy)
{
    imgui_software_texture_view source_view;
    imgui_software_texture_view destination_view;
    int x;
    int y;
    int source_x;
    int source_y;
    int destination_x;
    int destination_y;
    int source_bpp;
    int destination_bpp;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    const unsigned char *source_pixel;
    unsigned char *destination_pixel;
    unsigned char *temporary_pixels;
    size_t pixel_count;
    size_t temporary_size;
    size_t temporary_offset;
    int width;
    int height;
    if (target == NULL || copy == NULL || target->texture_resolver == NULL ||
        copy->source == NULL || copy->destination == NULL) return IMGUI_FALSE;
    memset(&source_view, 0, sizeof(source_view));
    memset(&destination_view, 0, sizeof(destination_view));
    if (!target->texture_resolver(copy->source, &source_view,
                                  target->texture_user_data) ||
        !target->texture_resolver(copy->destination, &destination_view,
                                  target->texture_user_data) ||
        destination_view.mutable_pixels == NULL ||
        source_view.pixels == NULL || source_view.width <= 0 ||
        source_view.height <= 0 || destination_view.width <= 0 ||
        destination_view.height <= 0) return IMGUI_FALSE;
    source_bpp = source_view.format == IMGUI_TEXTURE_FORMAT_ALPHA8 ? 1 : 4;
    destination_bpp = destination_view.format == IMGUI_TEXTURE_FORMAT_ALPHA8 ? 1 : 4;
    width = copy->source_region.width;
    height = copy->source_region.height;
    if (width <= 0 || height <= 0 || copy->source_region.x < 0 ||
        copy->source_region.y < 0 || copy->destination_x < 0 ||
        copy->destination_y < 0 ||
        copy->source_region.x > source_view.width ||
        copy->source_region.y > source_view.height ||
        copy->destination_x > destination_view.width ||
        copy->destination_y > destination_view.height ||
        width > source_view.width - copy->source_region.x ||
        height > source_view.height - copy->source_region.y ||
        width > destination_view.width - copy->destination_x ||
        height > destination_view.height - copy->destination_y ||
        (size_t)height > (size_t)-1 / (size_t)width) return IMGUI_FALSE;
    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count > (size_t)-1 / 4U) return IMGUI_FALSE;
    temporary_size = pixel_count * 4U;
    temporary_pixels = (unsigned char *)malloc(temporary_size);
    if (temporary_pixels == NULL) return IMGUI_FALSE;
    memset(temporary_pixels, 0, temporary_size);
    for (y = 0; y < height; ++y) {
        source_y = copy->source_region.y + y;
        if (source_y < 0 || source_y >= source_view.height) {
            continue;
        }
        for (x = 0; x < width; ++x) {
            source_x = copy->source_region.x + x;
            if (source_x < 0 || source_x >= source_view.width) {
                continue;
            }
            source_pixel = source_view.pixels + (size_t)source_y *
                source_view.stride + (size_t)source_x * (size_t)source_bpp;
            if (source_bpp == 1) {
                red = 255; green = 255; blue = 255; alpha = source_pixel[0];
            } else {
                red = source_pixel[0]; green = source_pixel[1];
                blue = source_pixel[2]; alpha = source_pixel[3];
            }
            temporary_offset = ((size_t)y * (size_t)width + (size_t)x) * 4U;
            temporary_pixels[temporary_offset + 0] = red;
            temporary_pixels[temporary_offset + 1] = green;
            temporary_pixels[temporary_offset + 2] = blue;
            temporary_pixels[temporary_offset + 3] = alpha;
        }
    }
    for (y = 0; y < height; ++y) {
        destination_y = copy->destination_y + y;
        if (destination_y < 0 || destination_y >= destination_view.height) {
            continue;
        }
        for (x = 0; x < width; ++x) {
            destination_x = copy->destination_x + x;
            if (destination_x < 0 || destination_x >= destination_view.width) {
                continue;
            }
            temporary_offset = ((size_t)y * (size_t)width + (size_t)x) * 4U;
            destination_pixel = destination_view.mutable_pixels +
                (size_t)destination_y * destination_view.stride +
                (size_t)destination_x * (size_t)destination_bpp;
            if (destination_bpp == 1) {
                destination_pixel[0] = temporary_pixels[temporary_offset + 3];
            } else {
                destination_pixel[0] = temporary_pixels[temporary_offset + 0];
                destination_pixel[1] = temporary_pixels[temporary_offset + 1];
                destination_pixel[2] = temporary_pixels[temporary_offset + 2];
                destination_pixel[3] = temporary_pixels[temporary_offset + 3];
            }
        }
    }
    free(temporary_pixels);
    return IMGUI_TRUE;
}

static imgui_bool imgui_software_clear_texture(
    imgui_software_target *target,
    const imgui_texture_clear_command *clear)
{
    imgui_software_texture_view view;
    int x;
    int y;
    int bytes_per_pixel;
    unsigned char *pixel;
    unsigned char alpha;
    if (target == NULL || clear == NULL || target->texture_resolver == NULL ||
        clear->texture == NULL) return IMGUI_FALSE;
    memset(&view, 0, sizeof(view));
    if (!target->texture_resolver(clear->texture, &view,
                                  target->texture_user_data) ||
        view.mutable_pixels == NULL || view.width <= 0 || view.height <= 0) {
        return IMGUI_FALSE;
    }
    if (clear->region.x < 0 || clear->region.y < 0 ||
        clear->region.width <= 0 || clear->region.height <= 0 ||
        clear->region.x > view.width || clear->region.y > view.height ||
        clear->region.width > view.width - clear->region.x ||
        clear->region.height > view.height - clear->region.y) {
        return IMGUI_FALSE;
    }
    bytes_per_pixel = view.format == IMGUI_TEXTURE_FORMAT_ALPHA8 ? 1 : 4;
    alpha = imgui_software_channel(clear->color, 24);
    for (y = clear->region.y;
         y < clear->region.y + clear->region.height; ++y) {
        if (y < 0 || y >= view.height) continue;
        for (x = clear->region.x;
             x < clear->region.x + clear->region.width; ++x) {
            if (x < 0 || x >= view.width) continue;
            pixel = view.mutable_pixels + (size_t)y * view.stride +
                    (size_t)x * (size_t)bytes_per_pixel;
            if (bytes_per_pixel == 1) {
                pixel[0] = alpha;
            } else {
                pixel[0] = imgui_software_channel(clear->color, 0);
                pixel[1] = imgui_software_channel(clear->color, 8);
                pixel[2] = imgui_software_channel(clear->color, 16);
                pixel[3] = alpha;
            }
        }
    }
    return IMGUI_TRUE;
}

static imgui_bool imgui_software_update_texture(
    imgui_software_target *target,
    const imgui_texture_update_command *update)
{
    imgui_software_texture_view view;
    size_t row_bytes;
    int bytes_per_pixel;
    int row;
    const unsigned char *source;
    unsigned char *destination;
    if (target == NULL || update == NULL || update->texture == NULL ||
        update->pixels == NULL || target->texture_resolver == NULL) {
        return IMGUI_FALSE;
    }
    memset(&view, 0, sizeof(view));
    if (!target->texture_resolver(update->texture, &view,
                                  target->texture_user_data) ||
        view.mutable_pixels == NULL || view.width <= 0 || view.height <= 0) {
        return IMGUI_FALSE;
    }
    if (update->region.x < 0 || update->region.y < 0 ||
        update->region.width <= 0 || update->region.height <= 0 ||
        update->region.x > view.width || update->region.y > view.height ||
        update->region.width > view.width - update->region.x ||
        update->region.height > view.height - update->region.y ||
        update->format != view.format) {
        return IMGUI_FALSE;
    }
    bytes_per_pixel = view.format == IMGUI_TEXTURE_FORMAT_ALPHA8 ? 1 : 4;
    row_bytes = (size_t)update->region.width *
                (size_t)bytes_per_pixel;
    if (update->pitch == 0 || update->pitch < row_bytes ||
        (size_t)update->region.height > (size_t)-1 / update->pitch) {
        return IMGUI_FALSE;
    }
    for (row = 0; row < update->region.height; ++row) {
        source = (const unsigned char *)update->pixels +
                 (size_t)row * update->pitch;
        destination = view.mutable_pixels +
            (size_t)(update->region.y + row) * view.stride +
            (size_t)update->region.x * (size_t)bytes_per_pixel;
        memcpy(destination, source, row_bytes);
    }
    return IMGUI_TRUE;
}

static void imgui_software_triangle(imgui_software_target *target,
                                    const imgui_render_vertex *a,
                                    const imgui_render_vertex *b,
                                    const imgui_render_vertex *c,
                                    imgui_rect clip,
                                    const imgui_software_texture_view *texture,
                                    imgui_sampler sampler)
{
    float area;
    float minimum_x;
    float maximum_x;
    float minimum_y;
    float maximum_y;
    int x_start;
    int x_end;
    int y_start;
    int y_end;
    int x;
    int y;
    float px;
    float py;
    float wa;
    float wb;
    float wc;
    imgui_u32 color;
    unsigned int red;
    unsigned int green;
    unsigned int blue;
    unsigned int alpha;
    unsigned char *pixel;
    unsigned char texel[4];
    float u;
    float v;
    area = imgui_software_edge(a->position.x, a->position.y,
                               b->position.x, b->position.y,
                               c->position.x, c->position.y);
    if (area == 0.0f) return;
    minimum_x = a->position.x;
    if (b->position.x < minimum_x) minimum_x = b->position.x;
    if (c->position.x < minimum_x) minimum_x = c->position.x;
    maximum_x = a->position.x;
    if (b->position.x > maximum_x) maximum_x = b->position.x;
    if (c->position.x > maximum_x) maximum_x = c->position.x;
    minimum_y = a->position.y;
    if (b->position.y < minimum_y) minimum_y = b->position.y;
    if (c->position.y < minimum_y) minimum_y = c->position.y;
    maximum_y = a->position.y;
    if (b->position.y > maximum_y) maximum_y = b->position.y;
    if (c->position.y > maximum_y) maximum_y = c->position.y;
    if (minimum_x < clip.x1) minimum_x = clip.x1;
    if (minimum_y < clip.y1) minimum_y = clip.y1;
    if (maximum_x > clip.x2) maximum_x = clip.x2;
    if (maximum_y > clip.y2) maximum_y = clip.y2;
    x_start = (int)floor((double)minimum_x);
    x_end = (int)ceil((double)maximum_x);
    y_start = (int)floor((double)minimum_y);
    y_end = (int)ceil((double)maximum_y);
    if (x_start < 0) x_start = 0;
    if (y_start < 0) y_start = 0;
    if (x_end > target->width) x_end = target->width;
    if (y_end > target->height) y_end = target->height;
    for (y = y_start; y < y_end; ++y) {
        for (x = x_start; x < x_end; ++x) {
            px = (float)x + 0.5f;
            py = (float)y + 0.5f;
            wa = imgui_software_edge(b->position.x, b->position.y,
                                     c->position.x, c->position.y, px, py);
            wb = imgui_software_edge(c->position.x, c->position.y,
                                     a->position.x, a->position.y, px, py);
            wc = imgui_software_edge(a->position.x, a->position.y,
                                     b->position.x, b->position.y, px, py);
            if ((wa >= 0.0f && wb >= 0.0f && wc >= 0.0f) ||
                (wa <= 0.0f && wb <= 0.0f && wc <= 0.0f)) {
                red = (unsigned int)(imgui_software_channel(a->color, 0) *
                    wa / area + imgui_software_channel(b->color, 0) *
                    wb / area + imgui_software_channel(c->color, 0) *
                    wc / area);
                green = (unsigned int)(imgui_software_channel(a->color, 8) *
                    wa / area + imgui_software_channel(b->color, 8) *
                    wb / area + imgui_software_channel(c->color, 8) *
                    wc / area);
                blue = (unsigned int)(imgui_software_channel(a->color, 16) *
                    wa / area + imgui_software_channel(b->color, 16) *
                    wb / area + imgui_software_channel(c->color, 16) *
                    wc / area);
                alpha = (unsigned int)(imgui_software_channel(a->color, 24) *
                    wa / area + imgui_software_channel(b->color, 24) *
                    wb / area + imgui_software_channel(c->color, 24) *
                    wc / area);
                u = a->uv.x * wa / area + b->uv.x * wb / area + c->uv.x * wc / area;
                v = a->uv.y * wa / area + b->uv.y * wb / area + c->uv.y * wc / area;
                imgui_software_sample(texture, u, v, sampler, texel);
                red = red * texel[0] / 255U;
                green = green * texel[1] / 255U;
                blue = blue * texel[2] / 255U;
                alpha = alpha * texel[3] / 255U;
                color = (imgui_u32)(red | (green << 8) |
                                    (blue << 16) | (alpha << 24));
                pixel = target->pixels + (size_t)y * target->stride +
                        (size_t)x * 4U;
                imgui_software_blend(pixel, color);
            }
        }
    }
}

imgui_result imgui_software_render_packet(const imgui_render_packet *packet,
                                          imgui_software_target *target)
{
    imgui_software_target local_target;
    imgui_u32 viewport_index;
    imgui_u32 list_index;
    imgui_u32 command_index;
    imgui_u32 index;
    const imgui_render_list *list;
    const imgui_render_command *command;
    imgui_render_index ia;
    imgui_render_index ib;
    imgui_render_index ic;
    imgui_rect clip;
    imgui_software_texture_view texture_view;
    const imgui_software_texture_view *texture;
    imgui_sampler sampler;
    if (packet == NULL || target == NULL || packet->struct_size < sizeof(*packet) ||
        !imgui_software_target_normalize(target, &local_target) ||
        local_target.pixels == NULL || local_target.width <= 0 ||
        local_target.height <= 0 ||
        local_target.stride < (size_t)local_target.width * 4U ||
        (packet->viewport_count != 0 && packet->viewports == NULL)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    target = &local_target;
    imgui_software_clear(target);
    for (viewport_index = 0; viewport_index < packet->viewport_count;
         ++viewport_index) {
        if (packet->viewports[viewport_index].list_count != 0 &&
            packet->viewports[viewport_index].lists == NULL) {
            return IMGUI_RESULT_CORRUPT_DATA;
        }
        for (list_index = 0; list_index <
             packet->viewports[viewport_index].list_count; ++list_index) {
            list = &packet->viewports[viewport_index].lists[list_index];
            sampler = IMGUI_SAMPLER_LINEAR;
            if ((list->vertex_count != 0 && list->vertices == NULL) ||
                (list->index_count != 0 && list->indices == NULL) ||
                (list->command_count != 0 && list->commands == NULL)) {
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            for (command_index = 0; command_index < list->command_count;
                 ++command_index) {
                command = &list->commands[command_index];
                if (command->type > IMGUI_RENDER_COMMAND_TEXTURE_UPDATE) {
                    return IMGUI_RESULT_CORRUPT_DATA;
                }
                if ((command->type == IMGUI_RENDER_COMMAND_CUSTOM ||
                     command->type == IMGUI_RENDER_COMMAND_RESET_STATE ||
                     command->type == IMGUI_RENDER_COMMAND_SET_SAMPLER ||
                     command->type == IMGUI_RENDER_COMMAND_TEXTURE_COPY ||
                     command->type == IMGUI_RENDER_COMMAND_TEXTURE_CLEAR ||
                     command->type == IMGUI_RENDER_COMMAND_TEXTURE_UPDATE) &&
                    target->command_callback != NULL) {
                    target->command_callback(command,
                                             target->command_user_data);
                }
                if (command->type == IMGUI_RENDER_COMMAND_TEXTURE_COPY) {
                    if (command->data.texture_copy.source == NULL ||
                        command->data.texture_copy.destination == NULL ||
                        command->data.texture_copy.source_region.width <= 0 ||
                        command->data.texture_copy.source_region.height <= 0) {
                        return IMGUI_RESULT_CORRUPT_DATA;
                    }
                    if (!imgui_software_copy_texture(
                            target, &command->data.texture_copy)) {
                        return IMGUI_RESULT_CORRUPT_DATA;
                    }
                    continue;
                }
                if (command->type == IMGUI_RENDER_COMMAND_TEXTURE_CLEAR) {
                    if (command->data.texture_clear.texture == NULL ||
                        command->data.texture_clear.region.width <= 0 ||
                        command->data.texture_clear.region.height <= 0) {
                        return IMGUI_RESULT_CORRUPT_DATA;
                    }
                    if (!imgui_software_clear_texture(
                            target, &command->data.texture_clear)) {
                        return IMGUI_RESULT_CORRUPT_DATA;
                    }
                    continue;
                }
                if (command->type == IMGUI_RENDER_COMMAND_TEXTURE_UPDATE) {
                    if (command->data.texture_update.texture == NULL ||
                        command->data.texture_update.pixels == NULL ||
                        command->data.texture_update.region.width <= 0 ||
                        command->data.texture_update.region.height <= 0) {
                        return IMGUI_RESULT_CORRUPT_DATA;
                    }
                    if (!imgui_software_update_texture(
                            target, &command->data.texture_update)) {
                        return IMGUI_RESULT_CORRUPT_DATA;
                    }
                    continue;
                }
                if (command->type == IMGUI_RENDER_COMMAND_SET_SAMPLER) {
                    if (command->data.set_sampler.sampler <
                            IMGUI_SAMPLER_LINEAR ||
                        command->data.set_sampler.sampler >
                            IMGUI_SAMPLER_CUSTOM) {
                        return IMGUI_RESULT_CORRUPT_DATA;
                    }
                    sampler = command->data.set_sampler.sampler;
                    continue;
                }
                if (command->type == IMGUI_RENDER_COMMAND_RESET_STATE) {
                    sampler = IMGUI_SAMPLER_LINEAR;
                    continue;
                }
                if (command->type == IMGUI_RENDER_COMMAND_CUSTOM &&
                    command->data.custom.payload_size != 0 &&
                    command->data.custom.payload == NULL) {
                    return IMGUI_RESULT_CORRUPT_DATA;
                }
                if (command->type != IMGUI_RENDER_COMMAND_DRAW_INDEXED) {
                    continue;
                }
                if (command->data.draw_indexed.index_count < 3U) continue;
                if (command->data.draw_indexed.index_offset >
                    list->index_count ||
                    command->data.draw_indexed.index_count >
                    list->index_count -
                    command->data.draw_indexed.index_offset) {
                    return IMGUI_RESULT_CORRUPT_DATA;
                }
                clip = command->data.draw_indexed.clip_rect;
                texture = NULL;
                if (target->texture_resolver != NULL &&
                    command->data.draw_indexed.texture != NULL) {
                    memset(&texture_view, 0, sizeof(texture_view));
                    if (target->texture_resolver(command->data.draw_indexed.texture,
                                                 &texture_view,
                                                 target->texture_user_data)) {
                        texture = &texture_view;
                    }
                }
                for (index = 0; index + 2U <
                     command->data.draw_indexed.index_count; index += 3U) {
                    ia = list->indices[command->data.draw_indexed.index_offset +
                                       index];
                    ib = list->indices[command->data.draw_indexed.index_offset +
                                       index + 1U];
                    ic = list->indices[command->data.draw_indexed.index_offset +
                                       index + 2U];
                    if ((imgui_u32)ia > 0xffffffffUL -
                            command->data.draw_indexed.vertex_offset ||
                        (imgui_u32)ib > 0xffffffffUL -
                            command->data.draw_indexed.vertex_offset ||
                        (imgui_u32)ic > 0xffffffffUL -
                            command->data.draw_indexed.vertex_offset ||
                        (imgui_u32)ia + command->data.draw_indexed.vertex_offset >=
                            list->vertex_count ||
                        (imgui_u32)ib + command->data.draw_indexed.vertex_offset >=
                            list->vertex_count ||
                        (imgui_u32)ic + command->data.draw_indexed.vertex_offset >=
                            list->vertex_count) {
                        return IMGUI_RESULT_CORRUPT_DATA;
                    }
                    imgui_software_triangle(target,
                        &list->vertices[(imgui_u32)ia +
                                        command->data.draw_indexed.vertex_offset],
                        &list->vertices[(imgui_u32)ib +
                                        command->data.draw_indexed.vertex_offset],
                        &list->vertices[(imgui_u32)ic +
                                        command->data.draw_indexed.vertex_offset],
                        clip, texture, sampler);
                }
            }
        }
    }
    return IMGUI_RESULT_OK;
}

#include "imgui_c89_internal.h"

#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct imgui_trace {
    imgui_allocator allocator;
    imgui_trace_event *events;
    size_t count;
    size_t capacity;
};

static imgui_bool imgui_item_register(imgui_context *ctx,
                                       imgui_id id,
                                       imgui_vec2 size);

static imgui_bool imgui_float_is_finite(float value)
{
    return value == value && value <= FLT_MAX && value >= -FLT_MAX;
}

static imgui_u32 imgui_color_override_alpha(imgui_u32 color, float alpha)
{
    int alpha_byte;
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    alpha_byte = (int)(alpha * 255.0f + 0.5f);
    return (color & 0x00ffffffUL) | ((imgui_u32)alpha_byte << 24);
}

static imgui_bool imgui_double_is_finite(double value)
{
    return value == value && value <= DBL_MAX && value >= -DBL_MAX;
}

static imgui_bool imgui_vec2_is_finite(imgui_vec2 value)
{
    return imgui_float_is_finite(value.x) &&
           imgui_float_is_finite(value.y);
}

static imgui_bool imgui_rect_is_finite(imgui_rect value)
{
    return imgui_float_is_finite(value.x1) &&
           imgui_float_is_finite(value.y1) &&
           imgui_float_is_finite(value.x2) &&
           imgui_float_is_finite(value.y2);
}

static int imgui_popup_stack_find(const imgui_context *ctx, imgui_id id)
{
    int index;
    if (ctx == NULL) return -1;
    for (index = 0; index < ctx->popup_stack_count; ++index) {
        if (ctx->popup_stack_ids[index] == id) return index;
    }
    return -1;
}

static void imgui_popup_stack_sync_top(imgui_context *ctx)
{
    if (ctx != NULL && ctx->popup_stack_count > 0) {
        ctx->popup_id = ctx->popup_stack_ids[
            ctx->popup_stack_count - 1];
        ctx->popup_rect = ctx->popup_stack_rects[
            ctx->popup_stack_count - 1];
        ctx->popup_rect_valid = ctx->popup_stack_rect_valid[
            ctx->popup_stack_count - 1];
    }
}

static void imgui_popup_stack_clear(imgui_context *ctx)
{
    if (ctx == NULL) return;
    ctx->popup_stack_count = 0;
    ctx->popup_modal = IMGUI_FALSE;
    ctx->popup_id = 0;
    ctx->current_popup_id = 0;
    ctx->popup_rect_valid = IMGUI_FALSE;
}

static imgui_result imgui_style_stack_reserve(imgui_context *ctx,
                                              int required)
{
    int capacity;
    size_t bytes;
    imgui_style *styles;
    if (ctx == NULL || required < 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (required <= ctx->style_stack_capacity) return IMGUI_RESULT_OK;
    capacity = ctx->style_stack_capacity > 0 ? ctx->style_stack_capacity : 8;
    while (capacity < required) {
        if (capacity > 1073741823) {
            capacity = required;
            break;
        }
        capacity *= 2;
    }
    if ((size_t)capacity > (size_t)-1 / sizeof(*styles)) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    bytes = (size_t)capacity * sizeof(*styles);
    styles = (imgui_style *)imgui_internal_allocate(&ctx->allocator, bytes);
    if (styles == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    if (ctx->style_stack_count > 0) {
        memcpy(styles, ctx->style_stack,
               (size_t)ctx->style_stack_count * sizeof(*styles));
    }
    imgui_internal_release(&ctx->allocator, ctx->style_stack);
    ctx->style_stack = styles;
    ctx->style_stack_capacity = capacity;
    return IMGUI_RESULT_OK;
}

static imgui_result imgui_window_reserve(imgui_context *ctx, int required)
{
    int capacity;
    size_t bytes;
    imgui_internal_window *windows;
    if (ctx == NULL || required < 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (required <= ctx->window_capacity) return IMGUI_RESULT_OK;
    capacity = ctx->window_capacity;
    if (capacity < IMGUI_INTERNAL_WINDOW_CAPACITY) {
        capacity = IMGUI_INTERNAL_WINDOW_CAPACITY;
    }
    while (capacity < required) {
        if (capacity > 1073741823) {
            capacity = required;
            break;
        }
        capacity *= 2;
    }
    if ((size_t)capacity > (size_t)-1 / sizeof(*windows)) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    bytes = (size_t)capacity * sizeof(*windows);
    windows = (imgui_internal_window *)imgui_internal_allocate(
        &ctx->allocator, bytes);
    if (windows == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    if (ctx->window_count > 0) {
        memcpy(windows, ctx->windows,
               (size_t)ctx->window_count * sizeof(*windows));
    }
    imgui_internal_release(&ctx->allocator, ctx->windows);
    ctx->windows = windows;
    ctx->window_capacity = capacity;
    return IMGUI_RESULT_OK;
}

static imgui_bool imgui_window_is_mouse_topmost(
    const imgui_context *ctx, int window_index)
{
    const imgui_internal_window *window;
    int index;
    if (ctx == NULL || window_index < 0 ||
        window_index >= ctx->window_count) return IMGUI_FALSE;
    window = &ctx->windows[window_index];
    for (index = 0; index < ctx->window_count; ++index) {
        const imgui_internal_window *other = &ctx->windows[index];
        if (index == window_index || !other->open || other->collapsed ||
            other->hidden_this_frame ||
            (other->flags & IMGUI_WINDOW_NO_MOUSE_INPUTS) != 0 ||
            other->z_order <= window->z_order) continue;
        if (ctx->input.mouse_x >= other->position.x &&
            ctx->input.mouse_x < other->position.x + other->size.x &&
            ctx->input.mouse_y >= other->position.y &&
            ctx->input.mouse_y < other->position.y + other->size.y) {
            return IMGUI_FALSE;
        }
    }
    return IMGUI_TRUE;
}

static imgui_result imgui_viewport_lists_reserve(imgui_context *ctx,
                                                  int required)
{
    int capacity;
    int viewport_index;
    imgui_render_list *rows[IMGUI_INTERNAL_VIEWPORT_CAPACITY];
    size_t bytes;
    if (ctx == NULL || required < 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (required <= ctx->viewport_list_capacity) return IMGUI_RESULT_OK;
    capacity = ctx->viewport_list_capacity;
    if (capacity < IMGUI_INTERNAL_WINDOW_CAPACITY) {
        capacity = IMGUI_INTERNAL_WINDOW_CAPACITY;
    }
    while (capacity < required) {
        if (capacity > 1073741823) {
            capacity = required;
            break;
        }
        capacity *= 2;
    }
    if ((size_t)capacity > (size_t)-1 / sizeof(*rows[0])) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    bytes = (size_t)capacity * sizeof(*rows[0]);
    for (viewport_index = 0;
         viewport_index < IMGUI_INTERNAL_VIEWPORT_CAPACITY;
         ++viewport_index) {
        rows[viewport_index] = (imgui_render_list *)imgui_internal_allocate(
            &ctx->allocator, bytes);
        if (rows[viewport_index] == NULL) {
            int cleanup_index;
            for (cleanup_index = 0; cleanup_index < viewport_index;
                 ++cleanup_index) {
                imgui_internal_release(&ctx->allocator, rows[cleanup_index]);
            }
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        if (ctx->viewport_lists[viewport_index] != NULL) {
            memcpy(rows[viewport_index], ctx->viewport_lists[viewport_index],
                   (size_t)ctx->viewport_list_capacity *
                   sizeof(*rows[viewport_index]));
        }
    }
    for (viewport_index = 0;
         viewport_index < IMGUI_INTERNAL_VIEWPORT_CAPACITY;
         ++viewport_index) {
        imgui_internal_release(&ctx->allocator,
                                ctx->viewport_lists[viewport_index]);
        ctx->viewport_lists[viewport_index] = rows[viewport_index];
    }
    ctx->viewport_list_capacity = capacity;
    return IMGUI_RESULT_OK;
}

static int imgui_window_find_or_create(imgui_context *ctx,
                                       const imgui_window_desc *desc);
static int imgui_dock_find_or_create(imgui_context *ctx, imgui_id dock_id);
static void imgui_dock_apply_children(imgui_context *ctx, int parent_index);
static void imgui_window_try_dock(imgui_context *ctx,
                                  imgui_internal_window *window);
static imgui_result imgui_reorder_window_commands(imgui_context *ctx);
static imgui_result imgui_remove_hidden_window_commands(imgui_context *ctx);
static int imgui_viewport_find(const imgui_context *ctx, imgui_id viewport_id);
static void imgui_build_viewport_packets(imgui_context *ctx);
static int imgui_child_find_or_create(imgui_context *ctx, imgui_id id);
static int imgui_tree_state_index(imgui_context *ctx, imgui_id id);
static int imgui_table_width_state_find(const imgui_context *ctx,
                                        imgui_id id);
static imgui_result imgui_table_width_state_reserve(imgui_context *ctx,
                                                    int required);
static void imgui_table_finalize_row_background(imgui_context *ctx,
                                                float row_height);
static imgui_u32 imgui_texture_next_identity(imgui_context *ctx);
static imgui_bool imgui_texture_desc_normalize(
    const imgui_texture_desc *source,
    imgui_texture_desc *destination);
static imgui_bool imgui_renderer_desc_normalize(
    const imgui_renderer_desc *source,
    imgui_renderer_desc *destination);
static imgui_bool imgui_platform_desc_normalize(
    const imgui_platform_desc *source,
    imgui_platform_desc *destination);
static imgui_bool imgui_input_text_desc_normalize(
    const imgui_input_text_desc *source,
    imgui_input_text_desc *destination);
static imgui_bool imgui_config_normalize(
    const imgui_config *source,
    imgui_config *destination);
static imgui_bool imgui_viewport_desc_normalize(
    const imgui_viewport_desc *source,
    imgui_viewport_desc *destination);
static void imgui_text_draw_font(imgui_context *ctx, const char *begin,
                                 const char *end, imgui_vec2 origin);
static imgui_vec2 imgui_label_size(const imgui_context *ctx,
                                   const char *label);
static float imgui_item_label_y(const imgui_context *ctx,
                                imgui_rect rect);
static imgui_result imgui_add_frame_surface(imgui_context *ctx,
                                            imgui_rect rect,
                                            imgui_u32 color);

static imgui_id imgui_hash_bytes(const void *data, size_t length,
                                 imgui_id seed)
{
    const unsigned char *bytes;
    size_t index;
    int bit;
    imgui_u32 crc;
    bytes = (const unsigned char *)data;
    crc = ~seed;
    for (index = 0; index < length; ++index) {
        crc ^= (imgui_u32)bytes[index];
        for (bit = 0; bit < 8; ++bit) {
            crc = (crc & 1U) != 0 ?
                  (crc >> 1) ^ (imgui_u32)0xedb88320UL : crc >> 1;
        }
    }
    return ~crc;
}

static imgui_id imgui_hash_id_range(const char *begin, const char *end,
                                    imgui_id seed)
{
    const char *cursor;
    if (begin == NULL || end == NULL || end < begin) return 0;
    cursor = begin;
    while (cursor + 2 < end) {
        if (cursor[0] == '#' && cursor[1] == '#' && cursor[2] == '#') {
            return imgui_hash_bytes(cursor + 3, (size_t)(end - cursor - 3),
                                    seed);
        }
        ++cursor;
    }
    return imgui_hash_bytes(begin, (size_t)(end - begin), seed);
}

static imgui_id imgui_id_seed(const imgui_context *ctx)
{
    if (ctx == NULL || ctx->id_depth <= 0) {
        return 0;
    }
    return ctx->id_stack[ctx->id_depth - 1];
}

static imgui_result imgui_id_stack_reserve(imgui_context *ctx, int required)
{
    int capacity;
    size_t bytes;
    imgui_id *stack;
    if (ctx == NULL || required < 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (required <= ctx->id_capacity) return IMGUI_RESULT_OK;
    capacity = ctx->id_capacity;
    if (capacity < IMGUI_INTERNAL_ID_CAPACITY) {
        capacity = IMGUI_INTERNAL_ID_CAPACITY;
    }
    while (capacity < required) {
        if (capacity > 1073741823) {
            capacity = required;
            break;
        }
        capacity *= 2;
    }
    if ((size_t)capacity > (size_t)-1 / sizeof(*stack)) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    bytes = (size_t)capacity * sizeof(*stack);
    stack = (imgui_id *)imgui_internal_allocate(&ctx->allocator, bytes);
    if (stack == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    if (ctx->id_depth > 0) {
        memcpy(stack, ctx->id_stack,
               (size_t)ctx->id_depth * sizeof(*stack));
    }
    imgui_internal_release(&ctx->allocator, ctx->id_stack);
    ctx->id_stack = stack;
    ctx->id_capacity = capacity;
    return IMGUI_RESULT_OK;
}

static void imgui_id_stack_push(imgui_context *ctx, imgui_id value)
{
    if (!imgui_internal_require_building(ctx, "push ID outside frame")) {
        return;
    }
    if (imgui_id_stack_reserve(ctx, ctx->id_depth + 1) != IMGUI_RESULT_OK) {
        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                              "ID stack allocation failed");
        return;
    }
    ctx->id_stack[ctx->id_depth++] = value;
}

static const char *imgui_label_visible_end(const char *label)
{
    const char *hidden;
    if (label == NULL) return NULL;
    hidden = strstr(label, "##");
    return hidden != NULL ? hidden : label + strlen(label);
}

static imgui_bool imgui_utf8_valid(const char *text, size_t length)
{
    size_t i;
    unsigned char c;
    unsigned char c1;
    unsigned char c2;
    unsigned char c3;
    if (text == NULL) {
        return IMGUI_FALSE;
    }
    i = 0;
    while (i < length) {
        c = (unsigned char)text[i++];
        if (c <= 0x7fU) {
            continue;
        }
        if (c >= 0xc2U && c <= 0xdfU) {
            if (i >= length) return IMGUI_FALSE;
            c1 = (unsigned char)text[i++];
            if (c1 < 0x80U || c1 > 0xbfU) return IMGUI_FALSE;
            continue;
        }
        if (c >= 0xe0U && c <= 0xefU) {
            if (i + 1 >= length) return IMGUI_FALSE;
            c1 = (unsigned char)text[i++];
            c2 = (unsigned char)text[i++];
            if (c1 < 0x80U || c1 > 0xbfU ||
                c2 < 0x80U || c2 > 0xbfU ||
                (c == 0xe0U && c1 < 0xa0U) ||
                (c == 0xedU && c1 >= 0xa0U)) return IMGUI_FALSE;
            continue;
        }
        if (c >= 0xf0U && c <= 0xf4U) {
            if (i + 2 >= length) return IMGUI_FALSE;
            c1 = (unsigned char)text[i++];
            c2 = (unsigned char)text[i++];
            c3 = (unsigned char)text[i++];
            if (c1 < 0x80U || c1 > 0xbfU ||
                c2 < 0x80U || c2 > 0xbfU ||
                c3 < 0x80U || c3 > 0xbfU ||
                (c == 0xf0U && c1 < 0x90U) ||
                (c == 0xf4U && c1 > 0x8fU)) return IMGUI_FALSE;
            continue;
        }
        return IMGUI_FALSE;
    }
    return IMGUI_TRUE;
}

static void imgui_render_commands_clear(imgui_context *ctx)
{
    imgui_u32 index;
    int channel;
    if (ctx == NULL) {
        return;
    }
    if (ctx->draw_channels_active) {
        if (ctx->draw_channel_current >= 0 &&
            ctx->draw_channel_current < ctx->draw_channel_count) {
            ctx->draw_channel_commands[ctx->draw_channel_current] =
                ctx->commands;
            ctx->draw_channel_command_counts[ctx->draw_channel_current] =
                ctx->command_count;
            ctx->draw_channel_command_capacities[ctx->draw_channel_current] =
                ctx->command_capacity;
            ctx->draw_channel_payloads[ctx->draw_channel_current] =
                ctx->command_payloads;
        }
        for (channel = 0; channel < ctx->draw_channel_count; ++channel) {
            for (index = 0;
                 index < ctx->draw_channel_command_counts[channel]; ++index) {
                if (ctx->draw_channel_payloads[channel] != NULL &&
                    ctx->draw_channel_payloads[channel][index] != NULL) {
                    imgui_internal_release(
                        &ctx->allocator,
                        ctx->draw_channel_payloads[channel][index]);
                }
            }
            imgui_internal_release(&ctx->allocator,
                                   ctx->draw_channel_payloads[channel]);
            imgui_internal_release(&ctx->allocator,
                                   ctx->draw_channel_commands[channel]);
            ctx->draw_channel_payloads[channel] = NULL;
            ctx->draw_channel_commands[channel] = NULL;
            ctx->draw_channel_command_counts[channel] = 0;
            ctx->draw_channel_command_capacities[channel] = 0;
        }
        ctx->commands = NULL;
        ctx->command_payloads = NULL;
        ctx->command_count = 0;
        ctx->command_capacity = 0;
        ctx->draw_channels_active = IMGUI_FALSE;
        ctx->draw_channel_count = 0;
        ctx->draw_channel_current = -1;
        return;
    }
    for (index = 0; index < ctx->command_count; ++index) {
        if (ctx->command_payloads != NULL &&
            ctx->command_payloads[index] != NULL) {
            imgui_internal_release(&ctx->allocator,
                                   ctx->command_payloads[index]);
        }
    }
    imgui_internal_release(&ctx->allocator, ctx->command_payloads);
    imgui_internal_release(&ctx->allocator, ctx->commands);
    ctx->command_payloads = NULL;
    ctx->commands = NULL;
    ctx->command_count = 0;
    ctx->command_capacity = 0;
}

static void imgui_resource_operations_clear(imgui_context *ctx)
{
    imgui_u32 index;
    if (ctx == NULL) {
        return;
    }
    for (index = 0; index < ctx->resource_operation_count; ++index) {
        imgui_internal_release(&ctx->allocator,
                               ctx->resource_payloads[index]);
    }
    imgui_internal_release(&ctx->allocator, ctx->resource_payloads);
    imgui_internal_release(&ctx->allocator, ctx->resource_operations);
    ctx->resource_payloads = NULL;
    ctx->resource_operations = NULL;
    ctx->resource_operation_count = 0;
    ctx->resource_operation_capacity = 0;
    ctx->resource_operation_consumed_count = 0;
}

/* Retire only operations already handed to a renderer. Operations queued
 * after the last render (for example an external texture upload) remain
 * pending for the next packet. */
static void imgui_resource_operations_retire_consumed(imgui_context *ctx)
{
    imgui_u32 consumed;
    imgui_u32 remaining;
    if (ctx == NULL || ctx->resource_operation_consumed_count == 0) return;
    consumed = ctx->resource_operation_consumed_count;
    if (consumed > ctx->resource_operation_count) {
        consumed = ctx->resource_operation_count;
    }
    for (remaining = 0; remaining < consumed; ++remaining) {
        imgui_internal_release(&ctx->allocator,
                               ctx->resource_payloads[remaining]);
    }
    remaining = ctx->resource_operation_count - consumed;
    if (remaining != 0) {
        memmove(ctx->resource_operations,
                ctx->resource_operations + consumed,
                (size_t)remaining * sizeof(*ctx->resource_operations));
        memmove(ctx->resource_payloads,
                ctx->resource_payloads + consumed,
                (size_t)remaining * sizeof(*ctx->resource_payloads));
    }
    ctx->resource_operation_count = remaining;
    ctx->resource_operation_consumed_count = 0;
}

static void imgui_drag_payload_clear(imgui_context *ctx)
{
    if (ctx == NULL) return;
    imgui_internal_release(&ctx->allocator, ctx->drag_payload_data);
    imgui_internal_release(&ctx->allocator, ctx->drag_payload_type);
    ctx->drag_payload_data = NULL;
    ctx->drag_payload_capacity = 0;
    ctx->drag_payload_type = NULL;
    ctx->drag_payload_type_capacity = 0;
    ctx->drag_payload_active = IMGUI_FALSE;
    ctx->drag_source_scope_active = IMGUI_FALSE;
    ctx->drag_target_scope_active = IMGUI_FALSE;
    ctx->drag_payload_source_id = 0;
    ctx->drag_payload_source_frame = 0;
    memset(&ctx->drag_payload, 0, sizeof(ctx->drag_payload));
}

static imgui_result imgui_drag_payload_store_type(imgui_context *ctx,
                                                   const char *type,
                                                   size_t length)
{
    size_t required;
    size_t capacity;
    char *copy;
    if (ctx == NULL || type == NULL || length == (size_t)-1) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (length > (size_t)-1 - 1) return IMGUI_RESULT_OUT_OF_MEMORY;
    required = length + 1;
    if (required <= ctx->drag_payload_type_capacity) {
        memcpy(ctx->drag_payload_type, type, required);
        return IMGUI_RESULT_OK;
    }
    capacity = ctx->drag_payload_type_capacity;
    if (capacity < 64) capacity = 64;
    while (capacity < required) {
        if (capacity > (size_t)-1 / 2) {
            capacity = required;
            break;
        }
        capacity *= 2;
    }
    copy = (char *)imgui_internal_allocate(&ctx->allocator, capacity);
    if (copy == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    memcpy(copy, type, required);
    imgui_internal_release(&ctx->allocator, ctx->drag_payload_type);
    ctx->drag_payload_type = copy;
    ctx->drag_payload_type_capacity = capacity;
    return IMGUI_RESULT_OK;
}

static void imgui_drag_payload_begin_frame(imgui_context *ctx)
{
    if (ctx == NULL) return;
    if (ctx->drag_payload_active &&
        !ctx->input.mouse_down[IMGUI_MOUSE_BUTTON_LEFT] &&
        !ctx->input.mouse_down_previous[IMGUI_MOUSE_BUTTON_LEFT]) {
        imgui_drag_payload_clear(ctx);
        return;
    }
    if (ctx->drag_payload_active &&
        (ctx->drag_source_flags &
         IMGUI_DRAG_DROP_SOURCE_PAYLOAD_AUTO_EXPIRE) != 0 &&
        ctx->drag_payload_source_frame + 1U < ctx->frame_index) {
        imgui_drag_payload_clear(ctx);
        return;
    }
    ctx->drag_source_scope_active = IMGUI_FALSE;
    ctx->drag_target_scope_active = IMGUI_FALSE;
    if (ctx->drag_payload_active) {
        ctx->drag_payload.preview = IMGUI_FALSE;
        ctx->drag_payload.delivery = IMGUI_FALSE;
    }
}

static imgui_result imgui_resource_operation_append(
    imgui_context *ctx,
    const imgui_resource_operation *operation,
    const void *pixels,
    size_t pixel_bytes)
{
    imgui_u32 capacity;
    imgui_resource_operation *operations;
    void **payloads;
    void *copy;
    if (ctx->resource_operation_count == ctx->resource_operation_capacity) {
        capacity = ctx->resource_operation_capacity == 0 ? 4U :
                   ctx->resource_operation_capacity;
        if (capacity > 0xffffffffUL / 2UL) {
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        if (ctx->resource_operation_capacity != 0) capacity *= 2U;
        if ((size_t)capacity > (size_t)-1 / sizeof(*operations) ||
            (size_t)capacity > (size_t)-1 / sizeof(*payloads)) {
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        operations = (imgui_resource_operation *)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*operations));
        payloads = (void **)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*payloads));
        if (operations == NULL || payloads == NULL) {
            imgui_internal_release(&ctx->allocator, operations);
            imgui_internal_release(&ctx->allocator, payloads);
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        if (ctx->resource_operation_count != 0) {
            memcpy(operations, ctx->resource_operations,
                   (size_t)ctx->resource_operation_count * sizeof(*operations));
            memcpy(payloads, ctx->resource_payloads,
                   (size_t)ctx->resource_operation_count * sizeof(*payloads));
        }
        imgui_internal_release(&ctx->allocator, ctx->resource_operations);
        imgui_internal_release(&ctx->allocator, ctx->resource_payloads);
        ctx->resource_operations = operations;
        ctx->resource_payloads = payloads;
        ctx->resource_operation_capacity = capacity;
    }
    copy = NULL;
    if (pixel_bytes != 0) {
        copy = imgui_internal_allocate(&ctx->allocator, pixel_bytes);
        if (copy == NULL) {
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        memcpy(copy, pixels, pixel_bytes);
    }
    ctx->resource_operations[ctx->resource_operation_count] = *operation;
    ctx->resource_operations[ctx->resource_operation_count].pixels = copy;
    ctx->resource_payloads[ctx->resource_operation_count] = copy;
    ctx->resource_operation_count += 1;
    return IMGUI_RESULT_OK;
}

static imgui_result imgui_render_command_append(
    imgui_context *ctx,
    const imgui_render_command *command,
    void *payload)
{
    imgui_u32 new_capacity;
    imgui_render_command *new_commands;
    void **new_payloads;
    imgui_render_command *previous;
    imgui_u32 index;
    imgui_u32 vertex_delta;
    if (ctx != NULL && command != NULL && payload == NULL &&
        ctx->command_count != 0U &&
        command->type == IMGUI_RENDER_COMMAND_DRAW_INDEXED) {
        previous = &ctx->commands[ctx->command_count - 1U];
        if (previous->type == IMGUI_RENDER_COMMAND_DRAW_INDEXED &&
            previous->data.draw_indexed.texture != NULL &&
            previous->data.draw_indexed.texture ==
                command->data.draw_indexed.texture &&
            previous->data.draw_indexed.clip_rect.x1 ==
                command->data.draw_indexed.clip_rect.x1 &&
            previous->data.draw_indexed.clip_rect.y1 ==
                command->data.draw_indexed.clip_rect.y1 &&
            previous->data.draw_indexed.clip_rect.x2 ==
                command->data.draw_indexed.clip_rect.x2 &&
            previous->data.draw_indexed.clip_rect.y2 ==
                command->data.draw_indexed.clip_rect.y2 &&
            previous->data.draw_indexed.index_offset +
                previous->data.draw_indexed.index_count ==
                command->data.draw_indexed.index_offset &&
            previous->data.draw_indexed.vertex_offset <
                command->data.draw_indexed.vertex_offset) {
            /* Each primitive is initially emitted with indices relative to
               its own vertex block. Once adjacent commands are merged, shift
               the new block into the first command's index space. */
            vertex_delta = command->data.draw_indexed.vertex_offset -
                           previous->data.draw_indexed.vertex_offset;
            for (index = command->data.draw_indexed.index_offset;
                 index < command->data.draw_indexed.index_offset +
                         command->data.draw_indexed.index_count; ++index) {
                if ((imgui_u32)ctx->indices[index] >
#if defined(IMGUI_RENDER_INDEX_32)
                    0xffffffffUL - vertex_delta) {
#else
                    0xffffUL - vertex_delta) {
#endif
                    return IMGUI_RESULT_OUT_OF_MEMORY;
                }
                ctx->indices[index] = (imgui_render_index)
                    ((imgui_u32)ctx->indices[index] + vertex_delta);
            }
            previous->data.draw_indexed.index_count +=
                command->data.draw_indexed.index_count;
            return IMGUI_RESULT_OK;
        }
    }
    if (ctx->command_count == ctx->command_capacity) {
        new_capacity = ctx->command_capacity == 0 ? 8U :
                       ctx->command_capacity;
        if (new_capacity > 0xffffffffUL / 2UL) {
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        if (ctx->command_capacity != 0) new_capacity *= 2U;
        if ((size_t)new_capacity > (size_t)-1 / sizeof(*new_commands) ||
            (size_t)new_capacity > (size_t)-1 / sizeof(*new_payloads)) {
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        new_commands = (imgui_render_command *)imgui_internal_allocate(
            &ctx->allocator, (size_t)new_capacity * sizeof(*new_commands));
        new_payloads = (void **)imgui_internal_allocate(
            &ctx->allocator, (size_t)new_capacity * sizeof(*new_payloads));
        if (new_commands == NULL || new_payloads == NULL) {
            imgui_internal_release(&ctx->allocator, new_commands);
            imgui_internal_release(&ctx->allocator, new_payloads);
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        if (ctx->command_count != 0) {
            memcpy(new_commands, ctx->commands,
                   (size_t)ctx->command_count * sizeof(*new_commands));
            memcpy(new_payloads, ctx->command_payloads,
                   (size_t)ctx->command_count * sizeof(*new_payloads));
        }
        imgui_internal_release(&ctx->allocator, ctx->commands);
        imgui_internal_release(&ctx->allocator, ctx->command_payloads);
        ctx->commands = new_commands;
        ctx->command_payloads = new_payloads;
        ctx->command_capacity = new_capacity;
    }
    ctx->commands[ctx->command_count] = *command;
    ctx->command_payloads[ctx->command_count] = payload;
    ctx->command_count += 1;
    return IMGUI_RESULT_OK;
}

static void imgui_merge_last_text_command(imgui_context *ctx)
{
    imgui_render_command *previous;
    imgui_render_command *current;
    imgui_u32 index;
    imgui_u32 vertex_delta;
    if (ctx == NULL || ctx->command_count < 2U) return;
    previous = &ctx->commands[ctx->command_count - 2U];
    current = &ctx->commands[ctx->command_count - 1U];
    if (previous->type != IMGUI_RENDER_COMMAND_DRAW_INDEXED ||
        current->type != IMGUI_RENDER_COMMAND_DRAW_INDEXED ||
        previous->data.draw_indexed.texture == NULL ||
        previous->data.draw_indexed.texture !=
            current->data.draw_indexed.texture ||
        previous->data.draw_indexed.clip_rect.x1 !=
            current->data.draw_indexed.clip_rect.x1 ||
        previous->data.draw_indexed.clip_rect.y1 !=
            current->data.draw_indexed.clip_rect.y1 ||
        previous->data.draw_indexed.clip_rect.x2 !=
            current->data.draw_indexed.clip_rect.x2 ||
        previous->data.draw_indexed.clip_rect.y2 !=
            current->data.draw_indexed.clip_rect.y2 ||
        previous->data.draw_indexed.index_offset +
            previous->data.draw_indexed.index_count !=
            current->data.draw_indexed.index_offset ||
        previous->data.draw_indexed.vertex_offset >=
            current->data.draw_indexed.vertex_offset) return;
    vertex_delta = current->data.draw_indexed.vertex_offset -
                   previous->data.draw_indexed.vertex_offset;
    for (index = current->data.draw_indexed.index_offset;
         index < current->data.draw_indexed.index_offset +
                 current->data.draw_indexed.index_count; ++index) {
        if ((imgui_u32)ctx->indices[index] >
#if defined(IMGUI_RENDER_INDEX_32)
            0xffffffffUL - vertex_delta) {
#else
            0xffffUL - vertex_delta) {
#endif
            return;
        }
        ctx->indices[index] = (imgui_render_index)
            ((imgui_u32)ctx->indices[index] + vertex_delta);
    }
    previous->data.draw_indexed.index_count +=
        current->data.draw_indexed.index_count;
    ctx->command_payloads[ctx->command_count - 2U] = NULL;
    --ctx->command_count;
}

static void imgui_draw_channels_save_current(imgui_context *ctx)
{
    int channel;
    if (ctx == NULL || !ctx->draw_channels_active) return;
    channel = ctx->draw_channel_current;
    if (channel < 0 || channel >= ctx->draw_channel_count) return;
    ctx->draw_channel_commands[channel] = ctx->commands;
    ctx->draw_channel_payloads[channel] = ctx->command_payloads;
    ctx->draw_channel_command_counts[channel] = ctx->command_count;
    ctx->draw_channel_command_capacities[channel] = ctx->command_capacity;
}

static void imgui_draw_channels_load(imgui_context *ctx, int channel)
{
    if (ctx == NULL || !ctx->draw_channels_active || channel < 0 ||
        channel >= ctx->draw_channel_count) return;
    ctx->draw_channel_current = channel;
    ctx->commands = ctx->draw_channel_commands[channel];
    ctx->command_payloads = ctx->draw_channel_payloads[channel];
    ctx->command_count = ctx->draw_channel_command_counts[channel];
    ctx->command_capacity = ctx->draw_channel_command_capacities[channel];
}

static void imgui_mesh_clear(imgui_context *ctx)
{
    if (ctx == NULL) {
        return;
    }
    imgui_internal_release(&ctx->allocator, ctx->vertices);
    imgui_internal_release(&ctx->allocator, ctx->indices);
    ctx->vertices = NULL;
    ctx->indices = NULL;
    ctx->vertex_count = 0;
    ctx->index_count = 0;
    ctx->vertex_capacity = 0;
    ctx->index_capacity = 0;
}

static imgui_result imgui_mesh_reserve(imgui_context *ctx,
                                       imgui_u32 vertex_count,
                                       imgui_u32 index_count)
{
    imgui_u32 vertex_capacity;
    imgui_u32 index_capacity;
    imgui_render_vertex *vertices;
    imgui_render_index *indices;
#if !defined(IMGUI_RENDER_INDEX_32)
    if (vertex_count > 65535U - ctx->vertex_count) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
#endif
    if (vertex_count > 0xffffffffUL - ctx->vertex_count) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    if (ctx->vertex_count + vertex_count > ctx->vertex_capacity) {
        vertex_capacity = ctx->vertex_capacity == 0 ? 64U : ctx->vertex_capacity;
        while (vertex_capacity < ctx->vertex_count + vertex_count) {
            if (vertex_capacity > 0xffffffffUL / 2UL) {
                vertex_capacity = ctx->vertex_count + vertex_count;
                break;
            }
            vertex_capacity *= 2U;
        }
        if ((size_t)vertex_capacity > (size_t)-1 / sizeof(*vertices)) {
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        vertices = (imgui_render_vertex *)imgui_internal_allocate(
            &ctx->allocator, (size_t)vertex_capacity * sizeof(*vertices));
        if (vertices == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
        if (ctx->vertex_count != 0) {
            memcpy(vertices, ctx->vertices,
                   (size_t)ctx->vertex_count * sizeof(*vertices));
        }
        imgui_internal_release(&ctx->allocator, ctx->vertices);
        ctx->vertices = vertices;
        ctx->vertex_capacity = vertex_capacity;
    }
    if (index_count > 0xffffffffUL - ctx->index_count) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    if (ctx->index_count + index_count > ctx->index_capacity) {
        index_capacity = ctx->index_capacity == 0 ? 96U : ctx->index_capacity;
        while (index_capacity < ctx->index_count + index_count) {
            if (index_capacity > 0xffffffffUL / 2UL) {
                index_capacity = ctx->index_count + index_count;
                break;
            }
            index_capacity *= 2U;
        }
        if ((size_t)index_capacity > (size_t)-1 / sizeof(*indices)) {
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        indices = (imgui_render_index *)imgui_internal_allocate(
            &ctx->allocator, (size_t)index_capacity * sizeof(*indices));
        if (indices == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
        if (ctx->index_count != 0) {
            memcpy(indices, ctx->indices,
                   (size_t)ctx->index_count * sizeof(*indices));
        }
        imgui_internal_release(&ctx->allocator, ctx->indices);
        ctx->indices = indices;
        ctx->index_capacity = index_capacity;
    }
    return IMGUI_RESULT_OK;
}

static imgui_result imgui_mesh_add_rect(imgui_context *ctx,
                                        imgui_rect rect,
                                        imgui_u32 color)
{
    imgui_render_command command;
    imgui_u32 vertex_offset;
    imgui_u32 index_offset;
    if (imgui_mesh_reserve(ctx, 4U, 6U) != IMGUI_RESULT_OK) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    vertex_offset = ctx->vertex_count;
    index_offset = ctx->index_count;
    ctx->vertices[vertex_offset + 0].position = imgui_make_vec2(rect.x1, rect.y1);
    ctx->vertices[vertex_offset + 1].position = imgui_make_vec2(rect.x2, rect.y1);
    ctx->vertices[vertex_offset + 2].position = imgui_make_vec2(rect.x2, rect.y2);
    ctx->vertices[vertex_offset + 3].position = imgui_make_vec2(rect.x1, rect.y2);
    ctx->vertices[vertex_offset + 0].uv = imgui_make_vec2(0.0f, 0.0f);
    ctx->vertices[vertex_offset + 1].uv = imgui_make_vec2(1.0f, 0.0f);
    ctx->vertices[vertex_offset + 2].uv = imgui_make_vec2(1.0f, 1.0f);
    ctx->vertices[vertex_offset + 3].uv = imgui_make_vec2(0.0f, 1.0f);
    ctx->vertices[vertex_offset + 0].color = color;
    ctx->vertices[vertex_offset + 1].color = color;
    ctx->vertices[vertex_offset + 2].color = color;
    ctx->vertices[vertex_offset + 3].color = color;
    ctx->indices[index_offset + 0] = (imgui_render_index)0U;
    ctx->indices[index_offset + 1] = (imgui_render_index)1U;
    ctx->indices[index_offset + 2] = (imgui_render_index)2U;
    ctx->indices[index_offset + 3] = (imgui_render_index)0U;
    ctx->indices[index_offset + 4] = (imgui_render_index)2U;
    ctx->indices[index_offset + 5] = (imgui_render_index)3U;
    ctx->vertex_count += 4U;
    ctx->index_count += 6U;
    memset(&command, 0, sizeof(command));
    command.type = IMGUI_RENDER_COMMAND_DRAW_INDEXED;
    /* Geometry remains unclipped; the renderer applies the current draw-list
       clip rectangle. This matches ImDrawList's command semantics and keeps
       partially visible primitives from changing their vertex coordinates. */
    command.data.draw_indexed.clip_rect = ctx->clip_rect;
    command.data.draw_indexed.index_offset = index_offset;
    command.data.draw_indexed.index_count = 6U;
    command.data.draw_indexed.vertex_offset = vertex_offset;
    {
        imgui_result result = imgui_render_command_append(ctx, &command, NULL);
        if (result != IMGUI_RESULT_OK) {
            ctx->vertex_count = vertex_offset;
            ctx->index_count = index_offset;
        }
        return result;
    }
}

static imgui_result imgui_mesh_add_triangle(imgui_context *ctx,
                                            imgui_vec2 a,
                                            imgui_vec2 b,
                                            imgui_vec2 c,
                                            imgui_u32 color)
{
    imgui_render_command command;
    imgui_u32 vertex_offset;
    imgui_u32 index_offset;
    if (ctx == NULL || imgui_mesh_reserve(ctx, 3U, 3U) != IMGUI_RESULT_OK) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    vertex_offset = ctx->vertex_count;
    index_offset = ctx->index_count;
    ctx->vertices[vertex_offset + 0U].position = a;
    ctx->vertices[vertex_offset + 1U].position = b;
    ctx->vertices[vertex_offset + 2U].position = c;
    ctx->vertices[vertex_offset + 0U].uv = imgui_make_vec2(0.0f, 0.0f);
    ctx->vertices[vertex_offset + 1U].uv = imgui_make_vec2(0.0f, 0.0f);
    ctx->vertices[vertex_offset + 2U].uv = imgui_make_vec2(0.0f, 0.0f);
    ctx->vertices[vertex_offset + 0U].color = color;
    ctx->vertices[vertex_offset + 1U].color = color;
    ctx->vertices[vertex_offset + 2U].color = color;
    ctx->indices[index_offset + 0U] = 0U;
    ctx->indices[index_offset + 1U] = 1U;
    ctx->indices[index_offset + 2U] = 2U;
    ctx->vertex_count += 3U;
    ctx->index_count += 3U;
    memset(&command, 0, sizeof(command));
    command.type = IMGUI_RENDER_COMMAND_DRAW_INDEXED;
    command.data.draw_indexed.clip_rect = ctx->clip_rect;
    command.data.draw_indexed.index_offset = index_offset;
    command.data.draw_indexed.index_count = 3U;
    command.data.draw_indexed.vertex_offset = vertex_offset;
    {
        imgui_result result = imgui_render_command_append(ctx, &command, NULL);
        if (result != IMGUI_RESULT_OK) {
            ctx->vertex_count = vertex_offset;
            ctx->index_count = index_offset;
        }
        return result;
    }
}

static imgui_result imgui_mesh_add_window_menu_icon(imgui_context *ctx,
                                                    float x,
                                                    float y,
                                                    imgui_u32 color)
{
    static const float points[6][2] = {
        {11.5f, 12.3999786f}, {11.5f, 14.4000244f},
        {7.8628159f, 6.0999985f}, {6.1307907f, 5.0999985f},
        {15.1371841f, 6.0999985f}, {16.8692093f, 5.0999985f}
    };
    static const unsigned char triangles[21] = {
        0, 2, 4, 0, 4, 5, 5, 1, 0, 2, 0, 1,
        1, 3, 2, 4, 2, 3, 3, 5, 4
    };
    imgui_render_command command;
    imgui_u32 vertex_offset;
    imgui_u32 index_offset;
    int point_index;
    int index;
    if (ctx == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (imgui_mesh_reserve(ctx, 6U, 21U) != IMGUI_RESULT_OK) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    vertex_offset = ctx->vertex_count;
    index_offset = ctx->index_count;
    for (point_index = 0; point_index < 6; ++point_index) {
        ctx->vertices[vertex_offset + (imgui_u32)point_index].position =
            imgui_make_vec2(x + points[point_index][0],
                            y + points[point_index][1]);
        ctx->vertices[vertex_offset + (imgui_u32)point_index].uv =
            imgui_make_vec2(0.0f, 0.0f);
        ctx->vertices[vertex_offset + (imgui_u32)point_index].color =
            (point_index & 1) != 0 ? color & 0x00ffffffUL : color;
    }
    for (index = 0; index < 21; ++index) {
        ctx->indices[index_offset + (imgui_u32)index] =
            (imgui_render_index)triangles[index];
    }
    ctx->vertex_count += 6U;
    ctx->index_count += 21U;
    memset(&command, 0, sizeof(command));
    command.type = IMGUI_RENDER_COMMAND_DRAW_INDEXED;
    command.data.draw_indexed.clip_rect = ctx->clip_rect;
    command.data.draw_indexed.index_offset = index_offset;
    command.data.draw_indexed.index_count = 21U;
    command.data.draw_indexed.vertex_offset = vertex_offset;
    {
        imgui_result result = imgui_render_command_append(ctx, &command, NULL);
        if (result != IMGUI_RESULT_OK) {
            ctx->vertex_count = vertex_offset;
            ctx->index_count = index_offset;
        }
        return result;
    }
}

static imgui_result imgui_mesh_add_window_border(imgui_context *ctx,
                                                 imgui_rect rect,
                                                 imgui_u32 color)
{
    static const unsigned char triangles[24] = {
        2, 0, 1, 3, 1, 2, 4, 2, 3, 5, 3, 4,
        6, 4, 5, 7, 5, 6, 0, 6, 7, 1, 7, 0
    };
    imgui_vec2 points[8];
    imgui_render_command command;
    imgui_u32 vertex_offset;
    imgui_u32 index_offset;
    float border_size;
    int point_index;
    int index;
    if (ctx == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    border_size = ctx->style.window_border_size;
    if (border_size <= 0.0f) return IMGUI_RESULT_OK;
    points[0] = imgui_make_vec2(rect.x1 - border_size,
                                rect.y1 - border_size);
    points[1] = imgui_make_vec2(rect.x1 + border_size + 1.0f,
                                rect.y1 + border_size + 1.0f);
    points[2] = imgui_make_vec2(rect.x2 + border_size,
                                rect.y1 - border_size);
    points[3] = imgui_make_vec2(rect.x2 - border_size - 1.0f,
                                rect.y1 + border_size + 1.0f);
    points[4] = imgui_make_vec2(rect.x2 + border_size,
                                rect.y2 + border_size);
    points[5] = imgui_make_vec2(rect.x2 - border_size - 1.0f,
                                rect.y2 - border_size - 1.0f);
    points[6] = imgui_make_vec2(rect.x1 - border_size,
                                rect.y2 + border_size);
    points[7] = imgui_make_vec2(rect.x1 + border_size + 1.0f,
                                rect.y2 - border_size - 1.0f);
    if (imgui_mesh_reserve(ctx, 8U, 24U) != IMGUI_RESULT_OK) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    vertex_offset = ctx->vertex_count;
    index_offset = ctx->index_count;
    for (point_index = 0; point_index < 8; ++point_index) {
        ctx->vertices[vertex_offset + (imgui_u32)point_index].position =
            points[point_index];
        ctx->vertices[vertex_offset + (imgui_u32)point_index].uv =
            imgui_make_vec2(0.0f, 0.0f);
        ctx->vertices[vertex_offset + (imgui_u32)point_index].color = color;
    }
    for (index = 0; index < 24; ++index) {
        ctx->indices[index_offset + (imgui_u32)index] =
            (imgui_render_index)triangles[index];
    }
    ctx->vertex_count += 8U;
    ctx->index_count += 24U;
    memset(&command, 0, sizeof(command));
    command.type = IMGUI_RENDER_COMMAND_DRAW_INDEXED;
    command.data.draw_indexed.clip_rect = ctx->clip_rect;
    command.data.draw_indexed.index_offset = index_offset;
    command.data.draw_indexed.index_count = 24U;
    command.data.draw_indexed.vertex_offset = vertex_offset;
    {
        imgui_result result = imgui_render_command_append(ctx, &command, NULL);
        if (result != IMGUI_RESULT_OK) {
            ctx->vertex_count = vertex_offset;
            ctx->index_count = index_offset;
        }
        return result;
    }
}

static void imgui_mesh_add_font_glyph(imgui_context *ctx,
                                      const imgui_font_glyph *glyph,
                                      float x,
                                      float y,
                                      imgui_u32 color)
{
    imgui_rect rect;
    imgui_rect clipped;
    float tx1;
    float ty1;
    float tx2;
    float ty2;
    imgui_u32 vertex_offset;
    if (ctx == NULL || glyph == NULL || ctx->font_texture == NULL ||
        glyph->width <= 0 || glyph->height <= 0) return;
    rect.x1 = x + (float)glyph->offset_x;
    rect.y1 = y + (float)glyph->offset_y;
    rect.x2 = rect.x1 + (float)glyph->width;
    rect.y2 = rect.y1 + (float)glyph->height;
    clipped = rect;
    if (clipped.x1 < ctx->clip_rect.x1) clipped.x1 = ctx->clip_rect.x1;
    if (clipped.y1 < ctx->clip_rect.y1) clipped.y1 = ctx->clip_rect.y1;
    if (clipped.x2 > ctx->clip_rect.x2) clipped.x2 = ctx->clip_rect.x2;
    if (clipped.y2 > ctx->clip_rect.y2) clipped.y2 = ctx->clip_rect.y2;
    if (clipped.x1 >= clipped.x2 || clipped.y1 >= clipped.y2) {
        return;
    }
    tx1 = (clipped.x1 - rect.x1) / (rect.x2 - rect.x1);
    ty1 = (clipped.y1 - rect.y1) / (rect.y2 - rect.y1);
    tx2 = (clipped.x2 - rect.x1) / (rect.x2 - rect.x1);
    ty2 = (clipped.y2 - rect.y1) / (rect.y2 - rect.y1);
    if (imgui_mesh_add_rect(ctx, clipped, color) != IMGUI_RESULT_OK) {
        return;
    }
    vertex_offset = ctx->vertex_count - 4U;
    ctx->vertices[vertex_offset + 0].uv =
        imgui_make_vec2(glyph->uv.x1 +
                        (glyph->uv.x2 - glyph->uv.x1) * tx1,
                        glyph->uv.y1 +
                        (glyph->uv.y2 - glyph->uv.y1) * ty1);
    ctx->vertices[vertex_offset + 1].uv =
        imgui_make_vec2(glyph->uv.x1 +
                        (glyph->uv.x2 - glyph->uv.x1) * tx2,
                        glyph->uv.y1 +
                        (glyph->uv.y2 - glyph->uv.y1) * ty1);
    ctx->vertices[vertex_offset + 2].uv =
        imgui_make_vec2(glyph->uv.x1 +
                        (glyph->uv.x2 - glyph->uv.x1) * tx2,
                        glyph->uv.y1 +
                        (glyph->uv.y2 - glyph->uv.y1) * ty2);
    ctx->vertices[vertex_offset + 3].uv =
        imgui_make_vec2(glyph->uv.x1 +
                        (glyph->uv.x2 - glyph->uv.x1) * tx1,
                        glyph->uv.y1 +
                        (glyph->uv.y2 - glyph->uv.y1) * ty2);
    ctx->commands[ctx->command_count - 1].data.draw_indexed.texture =
        ctx->font_texture;
    imgui_merge_last_text_command(ctx);
}

static void *imgui_default_allocate(size_t size, void *user_data)
{
    (void)user_data;
    return malloc(size);
}

static void imgui_default_release(void *memory, void *user_data)
{
    (void)user_data;
    free(memory);
}

static void imgui_default_error(imgui_error_code code,
                                const char *message,
                                void *user_data)
{
    (void)user_data;
    fprintf(stderr, "imgui_c89 error %d: %s\n", (int)code, message);
}

void *imgui_internal_allocate(const imgui_allocator *allocator, size_t size)
{
    if (allocator == NULL || allocator->allocate == NULL) {
        return malloc(size);
    }
    return allocator->allocate(size, allocator->user_data);
}

void imgui_internal_release(const imgui_allocator *allocator, void *memory)
{
    if (memory == NULL) {
        return;
    }
    if (allocator == NULL || allocator->release == NULL) {
        free(memory);
        return;
    }
    allocator->release(memory, allocator->user_data);
}

void imgui_internal_report(imgui_context *ctx,
                           imgui_error_code code,
                           const char *message)
{
    if (ctx != NULL && ctx->error_callback != NULL) {
        ctx->error_callback(code, message, ctx->error_user_data);
    } else {
        imgui_default_error(code, message, NULL);
    }
}

imgui_bool imgui_internal_require_building(imgui_context *ctx,
                                           const char *operation)
{
    if (ctx == NULL) {
        imgui_internal_report(NULL, IMGUI_ERROR_INVALID_ARGUMENT, operation);
        return IMGUI_FALSE;
    }
    if (ctx->frame_state != IMGUI_INTERNAL_FRAME_BUILDING) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE, operation);
        return IMGUI_FALSE;
    }
    return IMGUI_TRUE;
}

static imgui_result imgui_scope_reserve(imgui_context *ctx, int required)
{
    int capacity;
    imgui_internal_scope_kind *scopes;
    imgui_bool *scope_active;
    imgui_vec2 *saved_cursor;
    imgui_vec2 *saved_size;
    imgui_rect *saved_clip;
    imgui_internal_window_scope_state *saved_window;
    int *saved_child_index;
    float *saved_child_scroll;
    float *saved_child_scroll_x;
    imgui_flags *saved_child_flags;
    imgui_bool *tree_pushed;
    size_t count;
    if (ctx == NULL || required < 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (required <= ctx->scope_capacity) return IMGUI_RESULT_OK;
    capacity = ctx->scope_capacity;
    if (capacity < IMGUI_INTERNAL_SCOPE_CAPACITY) {
        capacity = IMGUI_INTERNAL_SCOPE_CAPACITY;
    }
    while (capacity < required) {
        if (capacity > 1073741823) {
            capacity = required;
            break;
        }
        capacity *= 2;
    }
    count = (size_t)capacity;
    if (count > (size_t)-1 / sizeof(*scopes) ||
        count > (size_t)-1 / sizeof(*scope_active) ||
        count > (size_t)-1 / sizeof(*saved_cursor) ||
        count > (size_t)-1 / sizeof(*saved_size) ||
        count > (size_t)-1 / sizeof(*saved_clip) ||
        count > (size_t)-1 / sizeof(*saved_window) ||
        count > (size_t)-1 / sizeof(*saved_child_index) ||
        count > (size_t)-1 / sizeof(*saved_child_scroll) ||
        count > (size_t)-1 / sizeof(*saved_child_scroll_x) ||
        count > (size_t)-1 / sizeof(*saved_child_flags) ||
        count > (size_t)-1 / sizeof(*tree_pushed)) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    scopes = (imgui_internal_scope_kind *)imgui_internal_allocate(
        &ctx->allocator, count * sizeof(*scopes));
    scope_active = (imgui_bool *)imgui_internal_allocate(
        &ctx->allocator, count * sizeof(*scope_active));
    saved_cursor = (imgui_vec2 *)imgui_internal_allocate(
        &ctx->allocator, count * sizeof(*saved_cursor));
    saved_size = (imgui_vec2 *)imgui_internal_allocate(
        &ctx->allocator, count * sizeof(*saved_size));
    saved_clip = (imgui_rect *)imgui_internal_allocate(
        &ctx->allocator, count * sizeof(*saved_clip));
    saved_window = (imgui_internal_window_scope_state *)
        imgui_internal_allocate(&ctx->allocator, count * sizeof(*saved_window));
    saved_child_index = (int *)imgui_internal_allocate(
        &ctx->allocator, count * sizeof(*saved_child_index));
    saved_child_scroll = (float *)imgui_internal_allocate(
        &ctx->allocator, count * sizeof(*saved_child_scroll));
    saved_child_scroll_x = (float *)imgui_internal_allocate(
        &ctx->allocator, count * sizeof(*saved_child_scroll_x));
    saved_child_flags = (imgui_flags *)imgui_internal_allocate(
        &ctx->allocator, count * sizeof(*saved_child_flags));
    tree_pushed = (imgui_bool *)imgui_internal_allocate(
        &ctx->allocator, count * sizeof(*tree_pushed));
    if (scopes == NULL || scope_active == NULL || saved_cursor == NULL || saved_size == NULL ||
        saved_clip == NULL || saved_window == NULL ||
        saved_child_index == NULL ||
        saved_child_scroll == NULL || saved_child_scroll_x == NULL ||
        saved_child_flags == NULL || tree_pushed == NULL) {
        imgui_internal_release(&ctx->allocator, scopes);
        imgui_internal_release(&ctx->allocator, scope_active);
        imgui_internal_release(&ctx->allocator, saved_cursor);
        imgui_internal_release(&ctx->allocator, saved_size);
        imgui_internal_release(&ctx->allocator, saved_clip);
        imgui_internal_release(&ctx->allocator, saved_window);
        imgui_internal_release(&ctx->allocator, saved_child_index);
        imgui_internal_release(&ctx->allocator, saved_child_scroll);
        imgui_internal_release(&ctx->allocator, saved_child_scroll_x);
        imgui_internal_release(&ctx->allocator, saved_child_flags);
        imgui_internal_release(&ctx->allocator, tree_pushed);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    if (ctx->scope_depth > 0) {
        count = (size_t)ctx->scope_depth;
        memcpy(scopes, ctx->scopes, count * sizeof(*scopes));
        memcpy(scope_active, ctx->scope_active,
               count * sizeof(*scope_active));
        memcpy(saved_cursor, ctx->scope_saved_cursor,
               count * sizeof(*saved_cursor));
        memcpy(saved_size, ctx->scope_saved_size, count * sizeof(*saved_size));
        memcpy(saved_clip, ctx->scope_saved_clip, count * sizeof(*saved_clip));
        memcpy(saved_window, ctx->scope_saved_window,
               count * sizeof(*saved_window));
        memcpy(saved_child_index, ctx->scope_saved_child_index,
               count * sizeof(*saved_child_index));
        memcpy(saved_child_scroll, ctx->scope_saved_child_scroll,
               count * sizeof(*saved_child_scroll));
        memcpy(saved_child_scroll_x, ctx->scope_saved_child_scroll_x,
               count * sizeof(*saved_child_scroll_x));
        memcpy(saved_child_flags, ctx->scope_saved_child_flags,
               count * sizeof(*saved_child_flags));
        memcpy(tree_pushed, ctx->scope_tree_pushed,
               count * sizeof(*tree_pushed));
    }
    imgui_internal_release(&ctx->allocator, ctx->scopes);
    imgui_internal_release(&ctx->allocator, ctx->scope_active);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_cursor);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_size);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_clip);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_window);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_child_index);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_child_scroll);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_child_scroll_x);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_child_flags);
    imgui_internal_release(&ctx->allocator, ctx->scope_tree_pushed);
    ctx->scopes = scopes;
    ctx->scope_active = scope_active;
    ctx->scope_saved_cursor = saved_cursor;
    ctx->scope_saved_size = saved_size;
    ctx->scope_saved_clip = saved_clip;
    ctx->scope_saved_window = saved_window;
    ctx->scope_saved_child_index = saved_child_index;
    ctx->scope_saved_child_scroll = saved_child_scroll;
    ctx->scope_saved_child_scroll_x = saved_child_scroll_x;
    ctx->scope_saved_child_flags = saved_child_flags;
    ctx->scope_tree_pushed = tree_pushed;
    ctx->scope_capacity = capacity;
    return IMGUI_RESULT_OK;
}

static void imgui_draw_frame_border(imgui_context *ctx, imgui_rect rect)
{
    float width;
    if (ctx == NULL) return;
    width = ctx->style.frame_border_size;
    if (width <= 0.0f) return;
    (void)imgui_draw_list_add_line(
        ctx, &ctx->default_draw_list,
        imgui_make_vec2(rect.x1, rect.y1),
        imgui_make_vec2(rect.x2, rect.y1),
        ctx->style.color_window_border, width);
    (void)imgui_draw_list_add_line(
        ctx, &ctx->default_draw_list,
        imgui_make_vec2(rect.x2, rect.y1),
        imgui_make_vec2(rect.x2, rect.y2),
        ctx->style.color_window_border, width);
    (void)imgui_draw_list_add_line(
        ctx, &ctx->default_draw_list,
        imgui_make_vec2(rect.x2, rect.y2),
        imgui_make_vec2(rect.x1, rect.y2),
        ctx->style.color_window_border, width);
    (void)imgui_draw_list_add_line(
        ctx, &ctx->default_draw_list,
        imgui_make_vec2(rect.x1, rect.y2),
        imgui_make_vec2(rect.x1, rect.y1),
        ctx->style.color_window_border, width);
}

imgui_scope imgui_internal_scope_begin(imgui_context *ctx,
                                       imgui_internal_scope_kind kind,
                                       imgui_bool active)
{
    if (!imgui_internal_require_building(ctx, "scope begin outside frame")) {
        return IMGUI_SCOPE_ERROR;
    }
    if (imgui_scope_reserve(ctx, ctx->scope_depth + 1) != IMGUI_RESULT_OK) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_OUT_OF_MEMORY,
                              "scope stack allocation failed");
        return IMGUI_SCOPE_ERROR;
    }
    ctx->scope_saved_cursor[ctx->scope_depth] = ctx->cursor;
    ctx->scope_saved_clip[ctx->scope_depth] = ctx->clip_rect;
    ctx->scopes[ctx->scope_depth] = kind;
    ctx->scope_active[ctx->scope_depth] = active;
    ctx->scope_tree_pushed[ctx->scope_depth] = IMGUI_TRUE;
    ctx->scope_depth += 1;
    return active ? IMGUI_SCOPE_ACTIVE : IMGUI_SCOPE_INACTIVE;
}

void imgui_internal_scope_end(imgui_context *ctx,
                              imgui_internal_scope_kind expected)
{
    imgui_bool active;
    if (!imgui_internal_require_building(ctx, "scope end outside frame")) {
        return;
    }
    if (ctx->scope_depth <= 0) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_SCOPE_MISMATCH,
                              "scope end without matching begin");
        return;
    }
    if (ctx->scopes[ctx->scope_depth - 1] != expected) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_SCOPE_MISMATCH,
                              "scope end kind does not match begin");
        return;
    }
    active = ctx->scope_active[ctx->scope_depth - 1];
    ctx->scope_depth -= 1;
    if (expected == IMGUI_INTERNAL_SCOPE_CHILD ||
        expected == IMGUI_INTERNAL_SCOPE_TOOLTIP ||
        expected == IMGUI_INTERNAL_SCOPE_POPUP ||
        expected == IMGUI_INTERNAL_SCOPE_COMBO ||
        expected == IMGUI_INTERNAL_SCOPE_LIST_BOX ||
        expected == IMGUI_INTERNAL_SCOPE_MENU ||
        expected == IMGUI_INTERNAL_SCOPE_MENU_BAR) {
        ctx->cursor = ctx->scope_saved_cursor[ctx->scope_depth];
        ctx->clip_rect = ctx->scope_saved_clip[ctx->scope_depth];
        if (expected == IMGUI_INTERNAL_SCOPE_CHILD ||
            expected == IMGUI_INTERNAL_SCOPE_LIST_BOX) {
            ctx->cursor.y += ctx->scope_saved_size[ctx->scope_depth].y +
                             ctx->item_spacing;
            if (expected == IMGUI_INTERNAL_SCOPE_CHILD && active &&
                ctx->id_depth > 0) {
                ctx->id_depth -= 1;
            }
        }
        if ((expected == IMGUI_INTERNAL_SCOPE_POPUP ||
             expected == IMGUI_INTERNAL_SCOPE_COMBO ||
             expected == IMGUI_INTERNAL_SCOPE_LIST_BOX ||
             expected == IMGUI_INTERNAL_SCOPE_MENU) && active &&
            ctx->id_depth > 0) {
            ctx->id_depth -= 1;
        }
    } else if (expected == IMGUI_INTERNAL_SCOPE_GROUP) {
        /* A group owns horizontal layout, but its contents still consume
         * vertical space in the parent layout. */
        ctx->cursor.x = ctx->scope_saved_cursor[ctx->scope_depth].x;
        if (ctx->cursor.y < ctx->scope_saved_cursor[ctx->scope_depth].y) {
            ctx->cursor.y = ctx->scope_saved_cursor[ctx->scope_depth].y;
        }
    } else if (expected == IMGUI_INTERNAL_SCOPE_TREE && active) {
        if (ctx->scope_tree_pushed[ctx->scope_depth] && ctx->id_depth > 0) {
            ctx->id_depth -= 1;
        }
        if (ctx->scope_tree_pushed[ctx->scope_depth] &&
            ctx->indent_width >= ctx->style.indent_spacing) {
            ctx->indent_width -= ctx->style.indent_spacing;
            ctx->cursor.x -= ctx->style.indent_spacing;
        } else if (ctx->scope_tree_pushed[ctx->scope_depth]) {
            ctx->indent_width = 0.0f;
        }
    } else if ((expected == IMGUI_INTERNAL_SCOPE_TAB_ITEM ||
                expected == IMGUI_INTERNAL_SCOPE_MENU ||
                expected == IMGUI_INTERNAL_SCOPE_COMBO ||
                expected == IMGUI_INTERNAL_SCOPE_LIST_BOX ||
                expected == IMGUI_INTERNAL_SCOPE_TABLE) && active) {
        if (ctx->id_depth > 0) {
            ctx->id_depth -= 1;
        }
    } else if (expected == IMGUI_INTERNAL_SCOPE_WINDOW) {
        if (ctx->id_depth > 0) {
            ctx->id_depth -= 1;
        }
    }
}

void imgui_config_init(imgui_config *config)
{
    if (config == NULL) {
        return;
    }
    memset(config, 0, sizeof(*config));
    config->struct_size = sizeof(*config);
    config->allocator.allocate = imgui_default_allocate;
    config->allocator.release = imgui_default_release;
    config->error_callback = imgui_default_error;
    config->double_click_time = 0.30f;
    config->double_click_max_distance = 6.0f;
    config->mouse_drag_threshold = 6.0f;
    config->key_repeat_delay = 0.275f;
    config->key_repeat_rate = 0.050f;
}

void imgui_frame_desc_init(imgui_frame_desc *desc)
{
    if (desc == NULL) {
        return;
    }
    memset(desc, 0, sizeof(*desc));
    desc->struct_size = sizeof(*desc);
    desc->framebuffer_scale.x = 1.0f;
    desc->framebuffer_scale.y = 1.0f;
    desc->delta_time = 1.0f / 60.0f;
}

void imgui_viewport_desc_init(imgui_viewport_desc *desc,
                              imgui_id viewport_id)
{
    if (desc == NULL) return;
    memset(desc, 0, sizeof(*desc));
    desc->struct_size = sizeof(*desc);
    desc->viewport_id = viewport_id;
    desc->framebuffer_scale = imgui_make_vec2(1.0f, 1.0f);
}

void imgui_metrics_init(imgui_metrics *metrics)
{
    if (metrics == NULL) return;
    memset(metrics, 0, sizeof(*metrics));
    metrics->struct_size = sizeof(*metrics);
}

void imgui_style_init(imgui_style *style)
{
    if (style == NULL) return;
    memset(style, 0, sizeof(*style));
    style->struct_size = sizeof(*style);
    style->window_padding = imgui_make_vec2(8.0f, 8.0f);
    style->frame_padding = imgui_make_vec2(8.0f, 4.0f);
    style->item_spacing = 4.0f;
    style->indent_spacing = 21.0f;
    style->window_rounding = 0.0f;
    style->frame_rounding = 0.0f;
    style->color_text = 0xffffffffUL;
    style->color_text_disabled = 0xffa0a0a0UL;
    style->color_window_background = 0xf00f0f0fUL;
    style->color_header = 0xff606060UL;
    style->color_header_hovered = 0xff909090UL;
    style->color_header_active = 0xff909090UL;
    style->color_separator = 0xff505050UL;
    style->color_check_mark = 0xffffffffUL;
    style->color_button = 0xff808080UL;
    style->color_button_hovered = 0xffb0b0b0UL;
    style->color_button_active = 0xff909090UL;
    style->color_frame = 0xff505050UL;
    style->color_frame_hovered = 0xff808080UL;
    style->color_frame_active = 0xff909090UL;
    style->color_window_title_background = 0xff7a4a29UL;
    style->color_window_title_background_active =
        style->color_window_title_background;
    style->color_window_title_background_collapsed =
        style->color_window_title_background;
    style->color_popup_background = style->color_window_background;
    style->color_window_title_text = style->color_text;
    style->color_window_border = 0x80806e6eUL;
    style->color_scrollbar_background = 0xff202020UL;
    style->color_scrollbar_grab = 0xff606060UL;
    style->color_scrollbar_grab_hovered = 0xff808080UL;
    style->color_scrollbar_grab_active = 0xffa0a0a0UL;
    style->color_table_row_even = 0xff282e34UL;
    style->color_table_row_odd = 0xff303840UL;
    style->color_table_border = style->color_window_border;
    style->color_text_selection = 0x804080ffUL;
    style->color_plot_lines = style->color_frame_active;
    style->color_plot_histogram = style->color_frame_active;
    style->color_modal_dim = 0x80000000UL;
    style->color_child_background = style->color_window_background;
    style->color_tab = style->color_header;
    style->color_tab_hovered = style->color_header_hovered;
    style->color_tab_active = style->color_header_active;
    style->color_tab_active_hovered = style->color_header_hovered;
    style->color_drag_drop_target = 0xffe0a040UL;
    style->color_nav_highlight = 0x80ffffffUL;
    style->color_menu_bar_background = style->color_window_title_background;
    style->color_menu_item = style->color_header;
    style->color_menu_item_hovered = style->color_header_hovered;
    style->color_menu_item_active = style->color_header_active;
    style->color_popup_border = style->color_window_border;
    style->color_text_link = 0xff65a5e8UL;
    style->color_text_link_hovered = 0xff8fc5f5UL;
    style->color_text_link_active = 0xffb8dcffUL;
    style->scrollbar_size = 13.0f;
    style->scrollbar_grab_min_size = 20.0f;
    style->child_rounding = 0.0f;
    style->window_border_size = 1.0f;
    style->child_border_size = 1.0f;
    style->frame_border_size = 0.0f;
}

void imgui_window_desc_init(imgui_window_desc *desc, const char *title)
{
    if (desc == NULL) {
        return;
    }
    memset(desc, 0, sizeof(*desc));
    desc->struct_size = sizeof(*desc);
    desc->title = title;
}

void imgui_text_buffer_init(imgui_text_buffer *buffer)
{
    if (buffer == NULL) {
        return;
    }
    memset(buffer, 0, sizeof(*buffer));
    buffer->struct_size = sizeof(*buffer);
}

void imgui_input_text_desc_init(imgui_input_text_desc *desc,
                                const char *label)
{
    if (desc == NULL) {
        return;
    }
    memset(desc, 0, sizeof(*desc));
    desc->struct_size = sizeof(*desc);
    desc->label = label;
}

void imgui_multi_select_storage_init(imgui_multi_select_storage *storage,
                                     imgui_id *ids,
                                     size_t capacity)
{
    if (storage == NULL) return;
    memset(storage, 0, sizeof(*storage));
    storage->struct_size = sizeof(*storage);
    storage->ids = ids;
    storage->capacity = capacity;
}

void imgui_texture_desc_init(imgui_texture_desc *desc)
{
    if (desc == NULL) {
        return;
    }
    memset(desc, 0, sizeof(*desc));
    desc->struct_size = sizeof(*desc);
}

void imgui_renderer_desc_init(imgui_renderer_desc *desc)
{
    if (desc == NULL) {
        return;
    }
    memset(desc, 0, sizeof(*desc));
    desc->struct_size = sizeof(*desc);
}

void imgui_platform_desc_init(imgui_platform_desc *desc)
{
    if (desc == NULL) {
        return;
    }
    memset(desc, 0, sizeof(*desc));
    desc->struct_size = sizeof(*desc);
}

imgui_vec2 imgui_make_vec2(float x, float y)
{
    imgui_vec2 value;
    value.x = x;
    value.y = y;
    return value;
}

imgui_vec4 imgui_make_vec4(float x, float y, float z, float w)
{
    imgui_vec4 value;
    value.x = x;
    value.y = y;
    value.z = z;
    value.w = w;
    return value;
}

const char *imgui_get_version(void)
{
    return IMGUI_VERSION_STRING;
}

static imgui_bool imgui_config_normalize(const imgui_config *source,
                                         imgui_config *destination)
{
    size_t copy_size;
    if (source == NULL || destination == NULL ||
        source->struct_size < sizeof(source->struct_size)) {
        return IMGUI_FALSE;
    }
    imgui_config_init(destination);
    copy_size = source->struct_size < sizeof(*destination) ?
        source->struct_size : sizeof(*destination);
    memcpy(destination, source, copy_size);
    destination->struct_size = sizeof(*destination);
    return IMGUI_TRUE;
}

static imgui_bool imgui_viewport_desc_normalize(
    const imgui_viewport_desc *source,
    imgui_viewport_desc *destination)
{
    size_t copy_size;
    if (source == NULL || destination == NULL ||
        source->struct_size < sizeof(source->struct_size)) {
        return IMGUI_FALSE;
    }
    imgui_viewport_desc_init(destination, 0);
    copy_size = source->struct_size < sizeof(*destination) ?
        source->struct_size : sizeof(*destination);
    memcpy(destination, source, copy_size);
    destination->struct_size = sizeof(*destination);
    return imgui_float_is_finite(destination->position.x) &&
           imgui_float_is_finite(destination->position.y) &&
           imgui_float_is_finite(destination->size.x) &&
           imgui_float_is_finite(destination->size.y) &&
           imgui_float_is_finite(destination->framebuffer_scale.x) &&
           imgui_float_is_finite(destination->framebuffer_scale.y);
}

imgui_context *imgui_context_create(const imgui_config *config)
{
    imgui_config defaults;
    imgui_config normalized;
    const imgui_config *selected;
    imgui_context *ctx;

    imgui_config_init(&defaults);
    if (config != NULL) {
        if (!imgui_config_normalize(config, &normalized)) {
            imgui_internal_report(NULL,
                                  IMGUI_ERROR_INVALID_ARGUMENT,
                                  "imgui_config has invalid struct_size");
            return NULL;
        }
        selected = &normalized;
    } else {
        selected = &defaults;
    }
    if (selected->struct_size < sizeof(imgui_config)) {
        imgui_internal_report(NULL,
                              IMGUI_ERROR_INVALID_ARGUMENT,
                              "imgui_config has invalid struct_size");
        return NULL;
    }
    if (!imgui_float_is_finite(selected->double_click_time) ||
        !imgui_float_is_finite(selected->double_click_max_distance) ||
        !imgui_float_is_finite(selected->mouse_drag_threshold) ||
        !imgui_float_is_finite(selected->key_repeat_delay) ||
        !imgui_float_is_finite(selected->key_repeat_rate)) {
        imgui_internal_report(NULL, IMGUI_ERROR_INVALID_ARGUMENT,
                              "imgui_config contains non-finite timing values");
        return NULL;
    }
    if ((selected->allocator.allocate == NULL) !=
        (selected->allocator.release == NULL)) {
        imgui_internal_report(NULL,
                              IMGUI_ERROR_INVALID_ARGUMENT,
                              "allocator functions must be supplied as a pair");
        return NULL;
    }
    ctx = (imgui_context *)imgui_internal_allocate(&selected->allocator,
                                                   sizeof(*ctx));
    if (ctx == NULL) {
        imgui_internal_report(NULL,
                              IMGUI_ERROR_OUT_OF_MEMORY,
                              "context allocation failed");
        return NULL;
    }
    memset(ctx, 0, sizeof(*ctx));
    ctx->allocator = selected->allocator;
    if (ctx->allocator.allocate == NULL) {
        ctx->allocator.allocate = imgui_default_allocate;
        ctx->allocator.release = imgui_default_release;
    }
    ctx->error_callback = selected->error_callback != NULL
        ? selected->error_callback
        : imgui_default_error;
    ctx->error_user_data = selected->error_user_data;
    ctx->key_repeat_delay = selected->key_repeat_delay;
    ctx->key_repeat_rate = selected->key_repeat_rate;
    ctx->double_click_time = selected->double_click_time;
    ctx->double_click_max_distance = selected->double_click_max_distance;
    ctx->mouse_drag_threshold = selected->mouse_drag_threshold;
    ctx->frame_state = IMGUI_INTERNAL_FRAME_IDLE;
    ctx->current_window_index = -1;
    ctx->last_window_index = -1;
    ctx->child_current_index = -1;
    ctx->focus_request = -1;
    imgui_viewport_desc_init(&ctx->viewport_configs[0].desc, 0);
    ctx->viewport_configs[0].configured = IMGUI_TRUE;
    ctx->viewport_count = 1;
    imgui_style_init(&ctx->style);
    ctx->input.focused = IMGUI_TRUE;
    ctx->demo_enabled = IMGUI_TRUE;
    ctx->demo_integer = 50;
    ctx->demo_float = 0.5f;
    ctx->demo_color[0] = 0.35f;
    ctx->demo_color[1] = 0.55f;
    ctx->demo_color[2] = 0.85f;
    ctx->demo_color[3] = 1.0f;
    ctx->default_draw_list.owner = ctx;
    ctx->packet.struct_size = sizeof(ctx->packet);
    ctx->packet.protocol_version = 1;
    return ctx;
}

void imgui_context_destroy(imgui_context *ctx)
{
    imgui_allocator allocator;
    imgui_texture *texture;
    imgui_texture *next_texture;
    if (ctx == NULL) {
        return;
    }
    if (ctx->frame_state == IMGUI_INTERNAL_FRAME_BUILDING) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_INVALID_STATE,
                              "destroying context during an open frame");
    }
    if (ctx->scope_depth != 0) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_UNCLOSED_SCOPE,
                              "destroying context with open scopes");
    }
    if (ctx->id_depth != 0) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_INVALID_STATE,
                              "destroying context with an open ID stack");
    }
    imgui_font_atlas_detach_context(ctx);
    imgui_render_commands_clear(ctx);
    imgui_mesh_clear(ctx);
    imgui_resource_operations_clear(ctx);
    imgui_drag_payload_clear(ctx);
    imgui_internal_release(&ctx->allocator, ctx->pending_text);
    imgui_internal_release(&ctx->allocator, ctx->path_points);
    imgui_internal_release(&ctx->allocator, ctx->text_undo_buffer);
    imgui_internal_release(&ctx->allocator, ctx->text_redo_buffer);
    imgui_internal_release(&ctx->allocator, ctx->navigation_item_ids);
    imgui_internal_release(&ctx->allocator, ctx->navigation_item_rects);
    imgui_internal_release(&ctx->allocator, ctx->style_stack);
    imgui_internal_release(&ctx->allocator, ctx->tree_ids);
    imgui_internal_release(&ctx->allocator, ctx->tree_open);
    imgui_internal_release(&ctx->allocator, ctx->tree_initialized);
    imgui_internal_release(&ctx->allocator, ctx->child_ids);
    imgui_internal_release(&ctx->allocator, ctx->child_scrolls);
    imgui_internal_release(&ctx->allocator, ctx->child_scroll_maxs);
    imgui_internal_release(&ctx->allocator, ctx->child_scroll_xs);
    imgui_internal_release(&ctx->allocator, ctx->child_scroll_max_xs);
    imgui_internal_release(&ctx->allocator, ctx->child_auto_widths);
    imgui_internal_release(&ctx->allocator, ctx->child_auto_heights);
    imgui_internal_release(&ctx->allocator, ctx->table_column_widths);
    imgui_internal_release(&ctx->allocator, ctx->table_column_labels);
    imgui_internal_release(&ctx->allocator, ctx->table_column_flags);
    imgui_internal_release(&ctx->allocator, ctx->table_sort_specs);
    imgui_internal_release(&ctx->allocator, ctx->multi_select_frame_ids);
    {
        int table_width_index;
        for (table_width_index = 0;
             table_width_index < ctx->table_width_state_count;
             ++table_width_index) {
            imgui_internal_release(
                &ctx->allocator,
                ctx->table_width_states[table_width_index].widths);
            imgui_internal_release(
                &ctx->allocator,
                ctx->table_width_states[table_width_index].sort_specs);
        }
    }
    imgui_internal_release(&ctx->allocator, ctx->table_width_states);
    imgui_internal_release(&ctx->allocator, ctx->numeric_buffer);
    imgui_internal_release(&ctx->allocator, ctx->id_stack);
    imgui_internal_release(&ctx->allocator, ctx->dock_nodes);
    {
        int viewport_index;
        for (viewport_index = 0;
             viewport_index < IMGUI_INTERNAL_VIEWPORT_CAPACITY;
             ++viewport_index) {
            imgui_internal_release(&ctx->allocator,
                                    ctx->viewport_lists[viewport_index]);
        }
    }
    imgui_internal_release(&ctx->allocator, ctx->scopes);
    imgui_internal_release(&ctx->allocator, ctx->scope_active);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_cursor);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_size);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_clip);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_window);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_child_index);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_child_scroll);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_child_scroll_x);
    imgui_internal_release(&ctx->allocator, ctx->scope_saved_child_flags);
    texture = ctx->textures;
    while (texture != NULL) {
        next_texture = texture->next;
        imgui_internal_release(&ctx->allocator, texture);
        texture = next_texture;
    }
    {
        int window_index;
        for (window_index = 0; window_index < ctx->window_count;
             ++window_index) {
            imgui_internal_release(&ctx->allocator,
                                   ctx->windows[window_index].title);
        }
    }
    imgui_internal_release(&ctx->allocator, ctx->windows);
    if (ctx->platform.viewport_destroy != NULL) {
        int viewport_index;
        for (viewport_index = 1; viewport_index < ctx->viewport_count;
             ++viewport_index) {
            if (ctx->viewport_configs[viewport_index].configured &&
                ctx->viewport_configs[viewport_index].platform_created) {
                ctx->platform.viewport_destroy(
                    ctx->viewport_configs[viewport_index].desc.viewport_id,
                    ctx->platform.user_data);
            }
        }
    }
    allocator = ctx->allocator;
    memset(ctx, 0, sizeof(*ctx));
    imgui_internal_release(&allocator, ctx);
}

static void imgui_update_next_wake_time(imgui_context *ctx)
{
    double wake_time;
    double candidate;
    float delay;
    int key;
    int mouse_button;
    if (ctx == NULL) return;
    wake_time = ctx->frame_desc.time + 3600.0;
    for (key = 0; key < IMGUI_KEY_COUNT; ++key) {
        if (!ctx->input.keys_down[key]) continue;
        if (ctx->input.key_down_duration[key] < ctx->key_repeat_delay) {
            delay = ctx->key_repeat_delay -
                    ctx->input.key_down_duration[key];
        } else {
            delay = ctx->key_repeat_rate > 0.0f ?
                    ctx->key_repeat_rate : 0.050f;
        }
        if (delay < 0.0f) delay = 0.0f;
        candidate = ctx->frame_desc.time + (double)delay;
        if (candidate < wake_time) wake_time = candidate;
    }
    for (mouse_button = 0; mouse_button < IMGUI_MOUSE_BUTTON_COUNT;
         ++mouse_button) {
        if (ctx->input.mouse_down[mouse_button]) {
            candidate = ctx->frame_desc.time + 1.0 / 60.0;
            if (candidate < wake_time) wake_time = candidate;
            break;
        }
    }
    if (ctx->popup_open) {
        candidate = ctx->frame_desc.time + 1.0 / 60.0;
        if (candidate < wake_time) wake_time = candidate;
    }
    ctx->frame_output.next_wake_time = wake_time;
}

static void imgui_update_frame_output_capture(imgui_context *ctx)
{
    if (ctx == NULL) return;
    ctx->frame_output.want_capture_mouse = ctx->input.focused &&
        (ctx->frame_any_window_hovered || ctx->active_item_valid ||
         ctx->popup_open);
    ctx->frame_output.want_capture_keyboard = ctx->input.focused &&
        (ctx->frame_any_window_focused || ctx->active_item_valid ||
         ctx->text_input_active);
    ctx->frame_output.navigation_active = ctx->focused_item_valid &&
                                          ctx->input.focused;
    ctx->frame_output.navigation_visible = ctx->focused_item_valid &&
                                           ctx->input.focused;
    ctx->frame_output.want_text_input = ctx->input.focused &&
                                        ctx->text_input_active;
}

static imgui_bool imgui_frame_desc_normalize(
    const imgui_frame_desc *source,
    imgui_frame_desc *destination)
{
    size_t copy_size;
    if (source == NULL || destination == NULL ||
        source->struct_size < sizeof(source->struct_size)) {
        return IMGUI_FALSE;
    }
    imgui_frame_desc_init(destination);
    copy_size = source->struct_size < sizeof(*destination) ?
        source->struct_size : sizeof(*destination);
    memcpy(destination, source, copy_size);
    destination->struct_size = sizeof(*destination);
    return imgui_float_is_finite(destination->display_size.x) &&
           imgui_float_is_finite(destination->display_size.y) &&
           imgui_float_is_finite(destination->framebuffer_scale.x) &&
           imgui_float_is_finite(destination->framebuffer_scale.y) &&
           imgui_float_is_finite(destination->delta_time) &&
           imgui_double_is_finite(destination->time) &&
           destination->delta_time > 0.0f;
}

static imgui_bool imgui_window_desc_normalize(
    const imgui_window_desc *source,
    imgui_window_desc *destination)
{
    size_t copy_size;
    if (destination == NULL) return IMGUI_FALSE;
    imgui_window_desc_init(destination, NULL);
    if (source == NULL) return IMGUI_TRUE;
    if (source->struct_size < sizeof(source->struct_size)) {
        return IMGUI_FALSE;
    }
    copy_size = source->struct_size < sizeof(*destination) ?
                source->struct_size : sizeof(*destination);
    memcpy(destination, source, copy_size);
    destination->struct_size = sizeof(*destination);
    if (destination->has_position &&
        (!imgui_float_is_finite(destination->position.x) ||
         !imgui_float_is_finite(destination->position.y))) {
        return IMGUI_FALSE;
    }
    if (destination->has_size &&
        (!imgui_float_is_finite(destination->size.x) ||
         !imgui_float_is_finite(destination->size.y))) {
        return IMGUI_FALSE;
    }
    return IMGUI_TRUE;
}

static imgui_bool imgui_input_text_desc_normalize(
    const imgui_input_text_desc *source,
    imgui_input_text_desc *destination)
{
    size_t copy_size;
    if (source == NULL || destination == NULL ||
        source->struct_size < sizeof(source->struct_size)) {
        return IMGUI_FALSE;
    }
    imgui_input_text_desc_init(destination, NULL);
    copy_size = source->struct_size < sizeof(*destination) ?
        source->struct_size : sizeof(*destination);
    memcpy(destination, source, copy_size);
    destination->struct_size = sizeof(*destination);
    return imgui_float_is_finite(destination->size.x) &&
           imgui_float_is_finite(destination->size.y);
}

imgui_result imgui_frame_begin(imgui_context *ctx,
                               const imgui_frame_desc *desc)
{
    imgui_frame_desc local_desc;
    int mouse_button;
    int key;
    float old_duration;
    float click_dx;
    float click_dy;
    double click_elapsed;
    int old_tick;
    int new_tick;
    imgui_window_desc fallback_desc;
    int fallback_index;
    if (ctx == NULL || desc == NULL) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_INVALID_ARGUMENT,
                              "frame begin requires context and descriptor");
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (!imgui_frame_desc_normalize(desc, &local_desc)) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_INVALID_ARGUMENT,
                              "frame descriptor is invalid");
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    imgui_resource_operations_retire_consumed(ctx);
    if (ctx->frame_state == IMGUI_INTERNAL_FRAME_BUILDING) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_INVALID_STATE,
                              "frame is already building");
        return IMGUI_RESULT_INVALID_STATE;
    }
    /* Dear ImGui creates an implicit fallback window at NewFrame(). It is
       intentionally inert unless an otherwise-invalid call targets it, but
       it participates in window metrics, settings, and ID ordering. */
    imgui_window_desc_init(&fallback_desc, "Debug##Default");
    fallback_index = imgui_window_find_or_create(ctx, &fallback_desc);
    if (fallback_index < 0) return IMGUI_RESULT_OUT_OF_MEMORY;
    ctx->windows[fallback_index].appearing = IMGUI_FALSE;
    ctx->windows[fallback_index].auto_fit_pending = IMGUI_FALSE;
    ctx->frame_desc = local_desc;
    imgui_render_commands_clear(ctx);
    imgui_mesh_clear(ctx);
    ctx->frame_state = IMGUI_INTERNAL_FRAME_BUILDING;
    ctx->frame_index += 1;
    ctx->item_clicked_this_frame = IMGUI_FALSE;
    /* Item queries and popup anchoring are frame-local. Retaining the last
       item's rectangle across frames incorrectly anchors an unanchored
       OpenPopup() to stale geometry instead of the current mouse position. */
    ctx->last_item_id = 0;
    ctx->last_item_hovered = IMGUI_FALSE;
    ctx->last_item_clicked = IMGUI_FALSE;
    ctx->last_item_active = IMGUI_FALSE;
    ctx->last_item_disabled = IMGUI_FALSE;
    ctx->last_item_activated = IMGUI_FALSE;
    ctx->last_item_deactivated = IMGUI_FALSE;
    ctx->last_item_deactivated_after_edit = IMGUI_FALSE;
    ctx->last_item_edited = IMGUI_FALSE;
    ctx->last_item_toggled_open = IMGUI_FALSE;
    ctx->last_item_toggled_selection = IMGUI_FALSE;
    ctx->any_item_hovered = IMGUI_FALSE;
    ctx->any_item_active = IMGUI_FALSE;
    ctx->any_item_focused = IMGUI_FALSE;
    ctx->next_item_width_valid = IMGUI_FALSE;
    ctx->next_item_shortcut_valid = IMGUI_FALSE;
    ctx->next_item_open_valid = IMGUI_FALSE;
    ctx->next_window_position_valid = IMGUI_FALSE;
    ctx->next_window_position_pivot_valid = IMGUI_FALSE;
    ctx->next_window_viewport_valid = IMGUI_FALSE;
    ctx->next_window_size_valid = IMGUI_FALSE;
    ctx->next_window_content_size_valid = IMGUI_FALSE;
    ctx->next_window_size_constraints_valid = IMGUI_FALSE;
    ctx->next_window_scroll_valid = IMGUI_FALSE;
    ctx->window_size_constraints_valid = IMGUI_FALSE;
    ctx->next_window_background_alpha_valid = IMGUI_FALSE;
    ctx->next_window_collapsed_valid = IMGUI_FALSE;
    ctx->next_window_focus_valid = IMGUI_FALSE;
    ctx->popup_render_valid = IMGUI_FALSE;
    ctx->popup_render_start = 0;
    ctx->popup_render_end = 0;
    ctx->combo_render_valid = IMGUI_FALSE;
    ctx->combo_render_tracking = IMGUI_FALSE;
    ctx->combo_render_start = 0;
    ctx->combo_render_end = 0;
    ctx->tooltip_render_valid = IMGUI_FALSE;
    ctx->tooltip_render_tracking = IMGUI_FALSE;
    ctx->tooltip_render_start = 0;
    ctx->tooltip_render_end = 0;
    ctx->tooltip_saved_window_flags = IMGUI_WINDOW_NONE;
    ctx->tooltip_saved_window_active = IMGUI_FALSE;
    ctx->list_box_child_index = -1;
    ctx->list_box_scroll_y = 0.0f;
    ctx->list_box_scroll_max_y = 0.0f;
    ctx->list_box_saved_content_max_x = 0.0f;
    ctx->list_box_saved_content_max_y = 0.0f;
    memset(&ctx->last_item_rect, 0, sizeof(ctx->last_item_rect));
    memset(&ctx->last_item_clip_rect, 0, sizeof(ctx->last_item_clip_rect));
    imgui_drag_payload_begin_frame(ctx);
    for (mouse_button = 0; mouse_button < IMGUI_MOUSE_BUTTON_COUNT;
         ++mouse_button) {
        ctx->mouse_clicked[mouse_button] =
            ctx->input.mouse_down[mouse_button] &&
            !ctx->input.mouse_down_previous[mouse_button];
        ctx->mouse_released[mouse_button] =
            !ctx->input.mouse_down[mouse_button] &&
            ctx->input.mouse_down_previous[mouse_button];
        ctx->mouse_double_clicked[mouse_button] = IMGUI_FALSE;
        if (ctx->mouse_clicked[mouse_button]) {
            ctx->mouse_down_duration[mouse_button] = 0.0f;
            ctx->mouse_down_start[mouse_button] = imgui_make_vec2(
                ctx->input.mouse_x, ctx->input.mouse_y);
            click_dx = ctx->input.mouse_x -
                ctx->mouse_last_click_position[mouse_button].x;
            click_dy = ctx->input.mouse_y -
                ctx->mouse_last_click_position[mouse_button].y;
            click_elapsed = ctx->frame_desc.time -
                ctx->mouse_last_click_time[mouse_button];
            if (ctx->mouse_last_click_valid[mouse_button] &&
                click_elapsed >= 0.0 &&
                click_elapsed <= (double)ctx->double_click_time &&
                click_dx * click_dx + click_dy * click_dy <=
                ctx->double_click_max_distance *
                ctx->double_click_max_distance) {
                ctx->mouse_double_clicked[mouse_button] = IMGUI_TRUE;
                if (ctx->mouse_clicked_count[mouse_button] < 2) {
                    ctx->mouse_clicked_count[mouse_button] = 2;
                } else {
                    ++ctx->mouse_clicked_count[mouse_button];
                }
            } else {
                ctx->mouse_clicked_count[mouse_button] = 1;
            }
            ctx->mouse_last_click_time[mouse_button] = ctx->frame_desc.time;
            ctx->mouse_last_click_position[mouse_button] = imgui_make_vec2(
                ctx->input.mouse_x, ctx->input.mouse_y);
            ctx->mouse_last_click_valid[mouse_button] = IMGUI_TRUE;
        } else if (ctx->input.mouse_down[mouse_button]) {
            if (ctx->mouse_down_duration[mouse_button] < 0.0f) {
                ctx->mouse_down_duration[mouse_button] = 0.0f;
                ctx->mouse_down_start[mouse_button] = imgui_make_vec2(
                    ctx->input.mouse_x, ctx->input.mouse_y);
            } else {
                ctx->mouse_down_duration[mouse_button] +=
                    ctx->frame_desc.delta_time;
            }
        } else {
            ctx->mouse_down_duration[mouse_button] = -1.0f;
        }
        ctx->input.mouse_down_previous[mouse_button] =
            ctx->input.mouse_down[mouse_button];
    }
    for (key = 0; key < IMGUI_KEY_COUNT; ++key) {
        ctx->input.keys_pressed[key] = ctx->input.keys_down[key] &&
                                      !ctx->input.keys_down_previous[key];
        ctx->input.keys_released[key] = !ctx->input.keys_down[key] &&
                                       ctx->input.keys_down_previous[key];
        ctx->input.keys_repeated[key] = ctx->input.keys_pressed[key];
        old_duration = ctx->input.key_down_duration[key];
        if (ctx->input.keys_down[key]) {
            ctx->input.key_down_duration[key] += desc->delta_time;
            if (!ctx->input.keys_pressed[key] &&
                ctx->key_repeat_rate > 0.0f &&
                ctx->input.key_down_duration[key] >= ctx->key_repeat_delay) {
                old_tick = old_duration >= ctx->key_repeat_delay ?
                    (int)((old_duration - ctx->key_repeat_delay) /
                          ctx->key_repeat_rate) : -1;
                new_tick = (int)((ctx->input.key_down_duration[key] -
                                  ctx->key_repeat_delay) /
                                 ctx->key_repeat_rate);
                ctx->input.keys_repeated[key] = new_tick > old_tick ?
                                                IMGUI_TRUE : IMGUI_FALSE;
            }
        } else {
            ctx->input.key_down_duration[key] = 0.0f;
        }
        ctx->input.keys_down_previous[key] = ctx->input.keys_down[key];
    }
    if (ctx->input.keys_pressed[IMGUI_KEY_ESCAPE]) {
        ctx->popup_open = IMGUI_FALSE;
        imgui_popup_stack_clear(ctx);
    }
    if (ctx->popup_open && !ctx->popup_modal && ctx->popup_rect_valid &&
        ctx->popup_mouse_button >= 0 &&
        ctx->popup_mouse_button < IMGUI_MOUSE_BUTTON_COUNT &&
        ctx->mouse_clicked[ctx->popup_mouse_button] &&
        (ctx->input.mouse_x < ctx->popup_rect.x1 ||
         ctx->input.mouse_x >= ctx->popup_rect.x2 ||
         ctx->input.mouse_y < ctx->popup_rect.y1 ||
         ctx->input.mouse_y >= ctx->popup_rect.y2)) {
        ctx->popup_open = IMGUI_FALSE;
        imgui_popup_stack_clear(ctx);
    }
    ctx->scope_depth = 0;
    ctx->current_window_index = -1;
    ctx->window_active = IMGUI_FALSE;
    ctx->window_focused = IMGUI_FALSE;
    ctx->frame_any_window_hovered = IMGUI_FALSE;
    ctx->frame_any_window_focused = IMGUI_FALSE;
    ctx->id_depth = 0;
    ctx->focus_request = -1;
    ctx->previous_active_item_id = ctx->active_item_id;
    ctx->previous_active_item_valid = ctx->active_item_valid;
    ctx->previous_active_item_was_edited = ctx->active_item_was_edited;
    if (ctx->previous_active_item_valid &&
        ctx->input.mouse_down[IMGUI_MOUSE_BUTTON_LEFT]) {
        ctx->active_item_id = ctx->previous_active_item_id;
        ctx->active_item_valid = IMGUI_TRUE;
        ctx->active_item_was_edited =
            ctx->previous_active_item_was_edited;
    } else {
        ctx->active_item_id = 0;
        ctx->active_item_valid = IMGUI_FALSE;
        ctx->active_item_was_edited = IMGUI_FALSE;
    }
    ctx->navigation_tab_pending =
        ctx->input.keys_pressed[IMGUI_KEY_TAB] ||
        ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_DOWN] ||
        ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_RIGHT] ||
        ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_UP] ||
        ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_LEFT];
    ctx->navigation_reverse_pending =
        (ctx->input.keys_pressed[IMGUI_KEY_TAB] &&
         (ctx->input.keys_down[IMGUI_KEY_LEFT_SHIFT] ||
          ctx->input.keys_down[IMGUI_KEY_RIGHT_SHIFT])) ||
        ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_UP] ||
        ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_LEFT];
    ctx->navigation_spatial_direction = 0;
    if (ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_RIGHT]) {
        ctx->navigation_spatial_direction = 1;
    } else if (ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_LEFT]) {
        ctx->navigation_spatial_direction = 2;
    } else if (ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_DOWN]) {
        ctx->navigation_spatial_direction = 3;
    } else if (ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_UP]) {
        ctx->navigation_spatial_direction = 4;
    }
    ctx->navigation_tab_seen_focus = IMGUI_FALSE;
    ctx->navigation_first_id = 0;
    ctx->navigation_first_valid = IMGUI_FALSE;
    ctx->navigation_candidate_id = 0;
    ctx->navigation_candidate_valid = IMGUI_FALSE;
    ctx->navigation_last_id = 0;
    ctx->navigation_last_valid = IMGUI_FALSE;
    ctx->navigation_item_count = 0;
    ctx->navigation_focused_rect_valid = IMGUI_FALSE;
    ctx->navigation_focused_item_seen = IMGUI_FALSE;
    ctx->last_item_edited = IMGUI_FALSE;
    ctx->last_item_deactivated_after_edit = IMGUI_FALSE;
    ctx->packet.resource_operations = ctx->resource_operations;
    ctx->packet.resource_operation_count = ctx->resource_operation_count;
    ctx->packet.viewports = NULL;
    ctx->packet.viewport_count = 0;
    ctx->packet.frame_index = ctx->frame_index;
    ctx->render_list.vertices = NULL;
    ctx->render_list.vertex_count = 0;
    ctx->render_list.indices = NULL;
    ctx->render_list.index_count = 0;
    ctx->render_list.commands = NULL;
    ctx->render_list.command_count = 0;
    ctx->viewport.viewport_id = 0;
    ctx->viewport.display_position = imgui_make_vec2(0.0f, 0.0f);
    ctx->viewport.display_size = desc->display_size;
    ctx->viewport.framebuffer_scale = desc->framebuffer_scale;
    ctx->viewport.lists = &ctx->render_list;
    ctx->viewport.list_count = 1;
    ctx->packet.viewports = &ctx->viewport;
    ctx->packet.viewport_count = 1;
    ctx->viewport_configs[0].desc.position = imgui_make_vec2(0.0f, 0.0f);
    ctx->viewport_configs[0].desc.size = desc->display_size;
    ctx->viewport_configs[0].desc.framebuffer_scale = desc->framebuffer_scale;
    ctx->frame_output.frame_index = ctx->frame_index;
    ctx->frame_output.want_capture_mouse = IMGUI_FALSE;
    ctx->frame_output.want_capture_keyboard = IMGUI_FALSE;
    ctx->frame_output.want_text_input = IMGUI_FALSE;
    ctx->frame_output.navigation_active = IMGUI_FALSE;
    ctx->frame_output.navigation_visible = IMGUI_FALSE;
    imgui_update_next_wake_time(ctx);
    return IMGUI_RESULT_OK;
}

static imgui_result imgui_navigation_reserve(imgui_context *ctx,
                                             int additional)
{
    size_t needed;
    size_t id_bytes;
    size_t rect_bytes;
    int capacity;
    imgui_id *ids;
    imgui_rect *rects;
    if (ctx == NULL || additional < 0 ||
        ctx->navigation_item_count > INT_MAX - additional) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    needed = (size_t)ctx->navigation_item_count + (size_t)additional;
    if (needed <= (size_t)ctx->navigation_item_capacity) {
        return IMGUI_RESULT_OK;
    }
    capacity = ctx->navigation_item_capacity > 0 ?
               ctx->navigation_item_capacity : IMGUI_INTERNAL_NAV_ITEM_CAPACITY;
    while ((size_t)capacity < needed) {
        if (capacity > INT_MAX / 2) {
            capacity = INT_MAX;
            break;
        }
        capacity *= 2;
    }
    if ((size_t)capacity > (size_t)-1 / sizeof(*ids) ||
        (size_t)capacity > (size_t)-1 / sizeof(*rects)) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    id_bytes = (size_t)capacity * sizeof(*ids);
    rect_bytes = (size_t)capacity * sizeof(*rects);
    ids = (imgui_id *)imgui_internal_allocate(&ctx->allocator, id_bytes);
    rects = (imgui_rect *)imgui_internal_allocate(&ctx->allocator, rect_bytes);
    if (ids == NULL || rects == NULL) {
        imgui_internal_release(&ctx->allocator, ids);
        imgui_internal_release(&ctx->allocator, rects);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    if (ctx->navigation_item_count != 0) {
        memcpy(ids, ctx->navigation_item_ids,
               (size_t)ctx->navigation_item_count * sizeof(*ids));
        memcpy(rects, ctx->navigation_item_rects,
               (size_t)ctx->navigation_item_count * sizeof(*rects));
    }
    imgui_internal_release(&ctx->allocator, ctx->navigation_item_ids);
    imgui_internal_release(&ctx->allocator, ctx->navigation_item_rects);
    ctx->navigation_item_ids = ids;
    ctx->navigation_item_rects = rects;
    ctx->navigation_item_capacity = capacity;
    return IMGUI_RESULT_OK;
}

static imgui_bool imgui_navigation_choose_spatial(imgui_context *ctx,
                                                   imgui_id *out_id)
{
    int index;
    int direction;
    imgui_rect rect;
    float focus_x;
    float focus_y;
    float center_x;
    float center_y;
    float primary;
    float secondary;
    float score;
    float best_score;
    imgui_bool found;
    if (ctx == NULL || out_id == NULL ||
        !ctx->navigation_focused_rect_valid ||
        ctx->navigation_spatial_direction == 0) {
        return IMGUI_FALSE;
    }
    direction = ctx->navigation_spatial_direction;
    focus_x = (ctx->navigation_focused_rect.x1 +
               ctx->navigation_focused_rect.x2) * 0.5f;
    focus_y = (ctx->navigation_focused_rect.y1 +
               ctx->navigation_focused_rect.y2) * 0.5f;
    found = IMGUI_FALSE;
    best_score = 0.0f;
    for (index = 0; index < ctx->navigation_item_count; ++index) {
        if (ctx->navigation_item_ids[index] == ctx->focused_item_id) {
            continue;
        }
        rect = ctx->navigation_item_rects[index];
        center_x = (rect.x1 + rect.x2) * 0.5f;
        center_y = (rect.y1 + rect.y2) * 0.5f;
        if (direction == 1) {
            primary = center_x - focus_x;
            secondary = center_y - focus_y;
        } else if (direction == 2) {
            primary = focus_x - center_x;
            secondary = center_y - focus_y;
        } else if (direction == 3) {
            primary = center_y - focus_y;
            secondary = center_x - focus_x;
        } else {
            primary = focus_y - center_y;
            secondary = center_x - focus_x;
        }
        if (primary <= 0.0f) continue;
        if (secondary < 0.0f) secondary = -secondary;
        score = primary * primary * 4.0f + secondary * secondary;
        if (!found || score < best_score) {
            found = IMGUI_TRUE;
            best_score = score;
            *out_id = ctx->navigation_item_ids[index];
        }
    }
    return found;
}

imgui_result imgui_frame_end(imgui_context *ctx)
{
    imgui_id spatial_id;
    if (!imgui_internal_require_building(ctx, "frame end outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    /* Wheel input is queued with the rest of the backend events before
       frame_begin and remains available throughout this frame.  Retire it
       only after the frame has consumed it, including malformed-frame paths. */
    ctx->input.mouse_wheel_x = 0.0f;
    ctx->input.mouse_wheel_y = 0.0f;
    if (ctx->scope_depth != 0) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_UNCLOSED_SCOPE,
                              "frame ended with open scopes");
        ctx->scope_depth = 0;
        ctx->frame_state = IMGUI_INTERNAL_FRAME_ENDED;
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->id_depth != 0) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_INVALID_STATE,
                              "frame ended with an unbalanced ID stack");
        ctx->id_depth = 0;
        ctx->frame_state = IMGUI_INTERNAL_FRAME_ENDED;
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->disabled_depth != 0) {
        imgui_internal_report(ctx, IMGUI_ERROR_SCOPE_MISMATCH,
                              "frame ended with active disabled region");
        ctx->disabled_depth = 0;
        ctx->frame_state = IMGUI_INTERNAL_FRAME_ENDED;
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->style_stack_count != 0) {
        imgui_internal_report(ctx, IMGUI_ERROR_SCOPE_MISMATCH,
                              "frame ended with active style stack");
        while (ctx->style_stack_count > 0) {
            --ctx->style_stack_count;
            ctx->style = ctx->style_stack[ctx->style_stack_count];
        }
        ctx->frame_state = IMGUI_INTERNAL_FRAME_ENDED;
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->path_active) {
        imgui_internal_report(ctx, IMGUI_ERROR_SCOPE_MISMATCH,
                              "frame ended with an unfinished draw path");
        ctx->path_active = IMGUI_FALSE;
        ctx->path_count = 0;
        ctx->frame_state = IMGUI_INTERNAL_FRAME_ENDED;
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->draw_channels_active) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_INVALID_STATE,
                              "frame ended with unmerged draw channels");
        ctx->frame_state = IMGUI_INTERNAL_FRAME_ENDED;
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->multi_select_active) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_INVALID_STATE,
                              "frame ended with active multi-select scope");
        ctx->multi_select_active = IMGUI_FALSE;
        ctx->multi_select_storage = NULL;
        ctx->frame_state = IMGUI_INTERNAL_FRAME_ENDED;
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->log_active) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                              "frame ended with active log");
        ctx->log_active = IMGUI_FALSE;
        memset(&ctx->log_desc, 0, sizeof(ctx->log_desc));
        ctx->frame_state = IMGUI_INTERNAL_FRAME_ENDED;
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT] &&
        !ctx->item_clicked_this_frame && ctx->text_input_active) {
        ctx->text_input_active = IMGUI_FALSE;
        ctx->active_text_id = 0;
        ctx->text_edit_id = 0;
        ctx->text_selection_start_byte = 0;
        ctx->text_selection_end_byte = 0;
        ctx->text_selection_anchor_byte = 0;
    }
    if (ctx->last_item_edited && ctx->last_item_id != 0 &&
        ctx->active_item_valid && ctx->last_item_id == ctx->active_item_id) {
        ctx->active_item_was_edited = IMGUI_TRUE;
    }
    if (imgui_reorder_window_commands(ctx) != IMGUI_RESULT_OK) {
        ctx->frame_state = IMGUI_INTERNAL_FRAME_ENDED;
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    if (imgui_remove_hidden_window_commands(ctx) != IMGUI_RESULT_OK) {
        ctx->frame_state = IMGUI_INTERNAL_FRAME_ENDED;
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    if (ctx->navigation_tab_pending) {
        if (imgui_navigation_choose_spatial(ctx, &spatial_id)) {
            ctx->navigation_candidate_id = spatial_id;
            ctx->navigation_candidate_valid = IMGUI_TRUE;
        }
        if (ctx->navigation_candidate_valid) {
            ctx->focused_item_id = ctx->navigation_candidate_id;
            ctx->focused_item_valid = IMGUI_TRUE;
        } else if (ctx->navigation_reverse_pending &&
                   ctx->navigation_last_valid) {
            ctx->focused_item_id = ctx->navigation_last_id;
            ctx->focused_item_valid = IMGUI_TRUE;
        } else if (ctx->navigation_first_valid) {
            ctx->focused_item_id = ctx->navigation_first_id;
            ctx->focused_item_valid = IMGUI_TRUE;
        }
    }
    if (ctx->navigation_tab_pending && ctx->navigation_focused_rect_valid &&
        ((ctx->style.color_nav_highlight >> 24) & 0xffU) != 0U) {
        imgui_rect focus_rect = ctx->navigation_focused_rect;
        focus_rect.x1 -= 1.0f;
        focus_rect.y1 -= 1.0f;
        focus_rect.x2 += 1.0f;
        focus_rect.y2 += 1.0f;
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(focus_rect.x1, focus_rect.y1),
            imgui_make_vec2(focus_rect.x2, focus_rect.y1),
            ctx->style.color_nav_highlight, 1.0f);
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(focus_rect.x2, focus_rect.y1),
            imgui_make_vec2(focus_rect.x2, focus_rect.y2),
            ctx->style.color_nav_highlight, 1.0f);
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(focus_rect.x2, focus_rect.y2),
            imgui_make_vec2(focus_rect.x1, focus_rect.y2),
            ctx->style.color_nav_highlight, 1.0f);
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(focus_rect.x1, focus_rect.y2),
            imgui_make_vec2(focus_rect.x1, focus_rect.y1),
            ctx->style.color_nav_highlight, 1.0f);
    }
    /* IO-style frame output is observable immediately after frame_end; do
       not make callers render a packet merely to learn input capture state. */
    imgui_update_frame_output_capture(ctx);
    ctx->frame_state = IMGUI_INTERNAL_FRAME_ENDED;
    return IMGUI_RESULT_OK;
}

static imgui_result imgui_settings_append_dynamic(
    imgui_context *ctx,
    char **buffer,
    size_t *capacity,
    size_t *length,
    const char *text)
{
    size_t text_length;
    size_t needed;
    size_t new_capacity;
    char *new_buffer;
    if (ctx == NULL || buffer == NULL || capacity == NULL ||
        length == NULL || text == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    text_length = strlen(text);
    if (text_length > (size_t)-1 - *length - 1U) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    needed = *length + text_length + 1U;
    if (needed > *capacity) {
        new_capacity = *capacity == 0 ? 1024U : *capacity;
        while (new_capacity < needed) {
            if (new_capacity > (size_t)-1 / 2U) {
                new_capacity = needed;
                break;
            }
            new_capacity *= 2U;
        }
        new_buffer = (char *)imgui_internal_allocate(
            &ctx->allocator, new_capacity);
        if (new_buffer == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
        if (*length != 0 && *buffer != NULL) {
            memcpy(new_buffer, *buffer, *length);
        }
        imgui_internal_release(&ctx->allocator, *buffer);
        *buffer = new_buffer;
        *capacity = new_capacity;
    }
    memcpy(*buffer + *length, text, text_length);
    *length += text_length;
    (*buffer)[*length] = '\0';
    return IMGUI_RESULT_OK;
}

static int imgui_settings_read_line(const char *data,
                                    size_t length,
                                    size_t *offset,
                                    char *line,
                                    size_t line_capacity)
{
    size_t start;
    size_t line_length;
    if (data == NULL || offset == NULL || line == NULL ||
        line_capacity == 0 || *offset >= length) {
        return 0;
    }
    start = *offset;
    while (*offset < length && data[*offset] != '\n') ++*offset;
    line_length = *offset - start;
    if (line_length >= line_capacity) return -1;
    memcpy(line, data + start, line_length);
    line[line_length] = '\0';
    if (line_length != 0 && line[line_length - 1] == '\r') {
        line[line_length - 1] = '\0';
    }
    if (*offset < length && data[*offset] == '\n') ++*offset;
    return 1;
}

imgui_result imgui_settings_save(imgui_context *ctx,
                                  char *buffer,
                                  size_t capacity,
                                  size_t *required)
{
    char *generated;
    size_t generated_capacity;
    char line[2048];
    size_t length;
    int index;
    if (ctx == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    generated = NULL;
    generated_capacity = 0;
    length = 0;
    if (imgui_settings_append_dynamic(
            ctx, &generated, &generated_capacity, &length,
            "IMGUI_C89_SETTINGS 1\n") != IMGUI_RESULT_OK) {
        goto settings_save_oom;
    }
    for (index = 0; index < ctx->tree_count; ++index) {
        sprintf(line, "TREE %lu %d\n",
                (unsigned long)ctx->tree_ids[index],
                ctx->tree_open[index] ? 1 : 0);
        if (imgui_settings_append_dynamic(
                ctx, &generated, &generated_capacity, &length, line) !=
            IMGUI_RESULT_OK) {
            goto settings_save_oom;
        }
    }
    for (index = 0; index < ctx->child_count; ++index) {
        sprintf(line, "CHILD %lu %.9g %.9g %.9g %.9g\n",
                (unsigned long)ctx->child_ids[index],
                (double)ctx->child_scrolls[index],
                (double)ctx->child_scroll_maxs[index],
                (double)ctx->child_scroll_xs[index],
                (double)ctx->child_scroll_max_xs[index]);
        if (imgui_settings_append_dynamic(
                ctx, &generated, &generated_capacity, &length, line) !=
            IMGUI_RESULT_OK) {
            goto settings_save_oom;
        }
    }
    for (index = 0; index < ctx->table_width_state_count; ++index) {
        int column;
        sprintf(line, "TABLE %lu %d",
                (unsigned long)ctx->table_width_states[index].id,
                ctx->table_width_states[index].columns);
        if (imgui_settings_append_dynamic(
                ctx, &generated, &generated_capacity, &length, line) !=
            IMGUI_RESULT_OK) {
            goto settings_save_oom;
        }
        for (column = 0; column < ctx->table_width_states[index].columns;
             ++column) {
            sprintf(line, " %.9g",
                    (double)ctx->table_width_states[index].widths[column]);
            if (imgui_settings_append_dynamic(
                    ctx, &generated, &generated_capacity, &length, line) !=
                IMGUI_RESULT_OK) {
                goto settings_save_oom;
            }
        }
        if (ctx->table_width_states[index].sort_spec_count > 0) {
            sprintf(line, " SORT %d",
                    ctx->table_width_states[index].sort_spec_count);
            if (imgui_settings_append_dynamic(
                    ctx, &generated, &generated_capacity, &length, line) !=
                IMGUI_RESULT_OK) {
                goto settings_save_oom;
            }
            for (column = 0;
                 column < ctx->table_width_states[index].sort_spec_count;
                 ++column) {
                sprintf(line, " %d %d",
                        ctx->table_width_states[index].sort_specs[column].column_index,
                        ctx->table_width_states[index].sort_specs[column].direction);
                if (imgui_settings_append_dynamic(
                        ctx, &generated, &generated_capacity, &length, line) !=
                    IMGUI_RESULT_OK) {
                    goto settings_save_oom;
                }
            }
        }
        sprintf(line, "\n");
        if (imgui_settings_append_dynamic(
                ctx, &generated, &generated_capacity, &length, line) !=
            IMGUI_RESULT_OK) {
            goto settings_save_oom;
        }
    }
    sprintf(line, "TAB %lu\n", (unsigned long)ctx->tab_active_id);
    if (imgui_settings_append_dynamic(
            ctx, &generated, &generated_capacity, &length, line) !=
        IMGUI_RESULT_OK) {
        goto settings_save_oom;
    }
    for (index = 1; index < ctx->viewport_count; ++index) {
        if (!ctx->viewport_configs[index].configured) continue;
        sprintf(line, "VIEWPORT %lu %.9g %.9g %.9g %.9g %.9g %.9g\n",
                (unsigned long)ctx->viewport_configs[index].desc.viewport_id,
                (double)ctx->viewport_configs[index].desc.position.x,
                (double)ctx->viewport_configs[index].desc.position.y,
                (double)ctx->viewport_configs[index].desc.size.x,
                (double)ctx->viewport_configs[index].desc.size.y,
                (double)ctx->viewport_configs[index].desc.framebuffer_scale.x,
                (double)ctx->viewport_configs[index].desc.framebuffer_scale.y);
        if (imgui_settings_append_dynamic(
                ctx, &generated, &generated_capacity, &length, line) !=
            IMGUI_RESULT_OK) goto settings_save_oom;
    }
    for (index = 0; index < ctx->window_count; ++index) {
        if ((ctx->windows[index].flags & IMGUI_WINDOW_NO_SAVED_SETTINGS) != 0) {
            continue;
        }
        sprintf(line, "WINDOW %lu %.9g %.9g %.9g %.9g %.9g %.9g %d %lu %.9g %.9g %.9g %.9g\n",
                (unsigned long)ctx->windows[index].id,
                (double)ctx->windows[index].position.x,
                (double)ctx->windows[index].position.y,
                (double)ctx->windows[index].size.x,
                (double)ctx->windows[index].size.y,
                (double)(ctx->windows[index].collapsed ?
                    ctx->windows[index].expanded_size.x :
                    ctx->windows[index].size.x),
                (double)(ctx->windows[index].collapsed ?
                    ctx->windows[index].expanded_size.y :
                    ctx->windows[index].size.y),
                ctx->windows[index].collapsed ? 1 : 0,
                (unsigned long)ctx->windows[index].viewport_id,
                (double)ctx->windows[index].scroll_y,
                (double)ctx->windows[index].scroll_max_y,
                (double)ctx->windows[index].scroll_x,
                (double)ctx->windows[index].scroll_max_x);
        if (imgui_settings_append_dynamic(
                ctx, &generated, &generated_capacity, &length, line) !=
            IMGUI_RESULT_OK) {
            goto settings_save_oom;
        }
        if (ctx->windows[index].dock_id != 0) {
            sprintf(line, "DOCK %lu %lu\n",
                    (unsigned long)ctx->windows[index].id,
                    (unsigned long)ctx->windows[index].dock_id);
            if (imgui_settings_append_dynamic(
                    ctx, &generated, &generated_capacity, &length, line) !=
                IMGUI_RESULT_OK) {
                goto settings_save_oom;
            }
        }
    }
    for (index = 0; index < ctx->dock_node_count; ++index) {
        if (ctx->dock_nodes[index].split_direction != IMGUI_DOCK_SPLIT_NONE &&
            ctx->dock_nodes[index].child_a != 0 &&
            ctx->dock_nodes[index].child_b != 0) {
            sprintf(line, "SPLIT %lu %d %.9g %lu %lu\n",
                    (unsigned long)ctx->dock_nodes[index].id,
                    (int)ctx->dock_nodes[index].split_direction,
                    (double)ctx->dock_nodes[index].split_ratio,
                    (unsigned long)ctx->dock_nodes[index].child_a,
                    (unsigned long)ctx->dock_nodes[index].child_b);
                if (imgui_settings_append_dynamic(
                        ctx, &generated, &generated_capacity, &length, line) !=
                    IMGUI_RESULT_OK) goto settings_save_oom;
        }
        if (ctx->dock_nodes[index].active_window_id != 0) {
            sprintf(line, "ACTIVE %lu %lu\n",
                    (unsigned long)ctx->dock_nodes[index].id,
                    (unsigned long)ctx->dock_nodes[index].active_window_id);
            if (imgui_settings_append_dynamic(
                    ctx, &generated, &generated_capacity, &length, line) !=
                IMGUI_RESULT_OK) goto settings_save_oom;
        }
    }
    if (required != NULL) *required = length + 1;
    if (buffer == NULL || capacity < length + 1) {
        if (buffer != NULL && capacity != 0) buffer[0] = '\0';
        imgui_internal_release(&ctx->allocator, generated);
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    memcpy(buffer, generated, length + 1);
    imgui_internal_release(&ctx->allocator, generated);
    return IMGUI_RESULT_OK;

settings_save_oom:
    imgui_internal_release(&ctx->allocator, generated);
    return IMGUI_RESULT_OUT_OF_MEMORY;
}

static imgui_result imgui_settings_load_internal(imgui_context *ctx,
                                                  const char *data,
                                                  size_t length)
{
    /* Settings records are intentionally variable length (table widths can
       exceed the historical fixed-column limit). */
    char line[8192];
    imgui_id loaded_tab;
    imgui_viewport_desc loaded_viewports[IMGUI_INTERNAL_VIEWPORT_CAPACITY];
    size_t offset;
    int line_result;
    int viewport_count;
    int index;
    unsigned long parsed_id;
    unsigned long parsed_dock_id;
    unsigned long parsed_child_b;
    unsigned long parsed_viewport_id;
    float parsed_ratio;
    float parsed_x;
    float parsed_y;
    float parsed_width;
    float parsed_height;
    float parsed_expanded_width;
    float parsed_expanded_height;
    float parsed_scroll;
    float parsed_scroll_max;
    float parsed_scroll_x;
    float parsed_scroll_max_x;
    float parsed_table_width;
    int parsed_open;
    int window_scan_count;
    int child_scan_count;
    int parsed_columns;
    int parsed_sort_count;
    int parsed_sort_column;
    int parsed_sort_direction;
    int consumed;
    char *table_cursor;
    float *parsed_table_widths;
    imgui_table_sort_spec *parsed_sort_specs;
    char extra;
    if (ctx == NULL || data == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (ctx->frame_state == IMGUI_INTERNAL_FRAME_BUILDING) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                              "settings load during an active frame");
        return IMGUI_RESULT_INVALID_STATE;
    }
    offset = 0;
    line_result = imgui_settings_read_line(data, length, &offset, line,
                                           sizeof(line));
    if (line_result <= 0 || strcmp(line, "IMGUI_C89_SETTINGS 1") != 0) {
        imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                              "settings header is invalid");
        return IMGUI_RESULT_CORRUPT_DATA;
    }
    loaded_tab = 0;
    ++ctx->settings_load_generation;
    if (ctx->settings_load_generation == 0UL) {
        ++ctx->settings_load_generation;
    }
    viewport_count = 0;
    while (offset < length) {
        line_result = imgui_settings_read_line(data, length, &offset, line,
                                               sizeof(line));
        if (line_result <= 0) {
            imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                  "settings line is too long");
            return IMGUI_RESULT_CORRUPT_DATA;
        }
        parsed_id = 0UL;
        parsed_open = 0;
        extra = '\0';
        if (sscanf(line, "TREE %lu %d %c", &parsed_id, &parsed_open,
                   &extra) == 2) {
            if (parsed_id > 0xffffffffUL ||
                (parsed_open != 0 && parsed_open != 1)) {
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings tree record is invalid");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            {
                int tree_index;
                tree_index = imgui_tree_state_index(
                    ctx, (imgui_id)parsed_id);
                if (tree_index < 0) return IMGUI_RESULT_OUT_OF_MEMORY;
                ctx->tree_open[tree_index] = parsed_open ?
                    IMGUI_TRUE : IMGUI_FALSE;
                ctx->tree_initialized[tree_index] = IMGUI_TRUE;
            }
        } else if ((child_scan_count = sscanf(
                       line, "CHILD %lu %f %f %f %f %c", &parsed_id,
                       &parsed_scroll, &parsed_scroll_max, &parsed_scroll_x,
                       &parsed_scroll_max_x, &extra)) >= 3) {
            if (child_scan_count == 3) {
                parsed_scroll_x = 0.0f;
                parsed_scroll_max_x = 0.0f;
            } else if (child_scan_count != 5) {
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings child record is invalid");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            if (parsed_id > 0xffffffffUL ||
                !imgui_float_is_finite(parsed_scroll) ||
                !imgui_float_is_finite(parsed_scroll_max) ||
                !imgui_float_is_finite(parsed_scroll_x) ||
                !imgui_float_is_finite(parsed_scroll_max_x) ||
                parsed_scroll < 0.0f || parsed_scroll_max < 0.0f ||
                parsed_scroll_max_x < 0.0f ||
                parsed_scroll_x > parsed_scroll_max_x) {
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings child record is invalid");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            {
                int child_index;
                child_index = imgui_child_find_or_create(
                    ctx, (imgui_id)parsed_id);
                if (child_index < 0) return IMGUI_RESULT_OUT_OF_MEMORY;
                ctx->child_scrolls[child_index] = parsed_scroll;
                ctx->child_scroll_maxs[child_index] = parsed_scroll_max;
                ctx->child_scroll_xs[child_index] = parsed_scroll_x;
                ctx->child_scroll_max_xs[child_index] = parsed_scroll_max_x;
            }
        } else if (sscanf(line, "TABLE %lu %d %n", &parsed_id,
                          &parsed_columns, &consumed) == 2) {
            if (parsed_id == 0UL || parsed_id > 0xffffffffUL ||
                parsed_columns <= 0) {
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings table record is invalid");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            parsed_table_widths = (float *)imgui_internal_allocate(
                &ctx->allocator, (size_t)parsed_columns *
                sizeof(*parsed_table_widths));
            if (parsed_table_widths == NULL) {
                return IMGUI_RESULT_OUT_OF_MEMORY;
            }
            table_cursor = line + consumed;
            parsed_sort_specs = NULL;
            parsed_sort_count = 0;
            for (index = 0; index < parsed_columns; ++index) {
                while (*table_cursor == ' ' || *table_cursor == '\t') {
                    ++table_cursor;
                }
                if (sscanf(table_cursor, "%f%n", &parsed_table_width,
                           &consumed) != 1 ||
                    !imgui_float_is_finite(parsed_table_width) ||
                    parsed_table_width <= 0.0f) {
                    imgui_internal_release(&ctx->allocator,
                                           parsed_table_widths);
                    imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                          "settings table width is invalid");
                    return IMGUI_RESULT_CORRUPT_DATA;
                }
                parsed_table_widths[index] = parsed_table_width;
                table_cursor += consumed;
            }
            while (*table_cursor == ' ' || *table_cursor == '\t') {
                ++table_cursor;
            }
            if (*table_cursor != '\0') {
                if (sscanf(table_cursor, "SORT %d %n", &parsed_sort_count,
                           &consumed) != 1 || parsed_sort_count <= 0 ||
                    parsed_sort_count > parsed_columns) {
                    imgui_internal_release(&ctx->allocator,
                                           parsed_table_widths);
                    imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                          "settings table sort record is invalid");
                    return IMGUI_RESULT_CORRUPT_DATA;
                }
                parsed_sort_specs = (imgui_table_sort_spec *)
                    imgui_internal_allocate(
                        &ctx->allocator,
                        (size_t)parsed_sort_count *
                        sizeof(*parsed_sort_specs));
                if (parsed_sort_specs == NULL) {
                    imgui_internal_release(&ctx->allocator,
                                           parsed_table_widths);
                    return IMGUI_RESULT_OUT_OF_MEMORY;
                }
                table_cursor += consumed;
                for (index = 0; index < parsed_sort_count; ++index) {
                    if (sscanf(table_cursor, "%d %d%n", &parsed_sort_column,
                               &parsed_sort_direction, &consumed) != 2 ||
                        parsed_sort_column < 0 ||
                        parsed_sort_column >= parsed_columns ||
                        (parsed_sort_direction != 1 &&
                         parsed_sort_direction != -1)) {
                        imgui_internal_release(&ctx->allocator,
                                               parsed_sort_specs);
                        imgui_internal_release(&ctx->allocator,
                                               parsed_table_widths);
                        imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                              "settings table sort spec is invalid");
                        return IMGUI_RESULT_CORRUPT_DATA;
                    }
                    parsed_sort_specs[index].column_index = parsed_sort_column;
                    parsed_sort_specs[index].sort_order = index;
                    parsed_sort_specs[index].direction = parsed_sort_direction;
                    parsed_sort_specs[index].column_user_id = 0;
                    table_cursor += consumed;
                    while (*table_cursor == ' ' || *table_cursor == '\t') {
                        ++table_cursor;
                    }
                }
            }
            if (*table_cursor != '\0') {
                imgui_internal_release(&ctx->allocator, parsed_sort_specs);
                imgui_internal_release(&ctx->allocator, parsed_table_widths);
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings table record has trailing data");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            {
                int table_width_index;
                table_width_index = imgui_table_width_state_find(
                    ctx, (imgui_id)parsed_id);
                if (table_width_index < 0) {
                    if (imgui_table_width_state_reserve(
                            ctx, ctx->table_width_state_count + 1) !=
                        IMGUI_RESULT_OK) {
                        imgui_internal_release(&ctx->allocator,
                                               parsed_table_widths);
                        return IMGUI_RESULT_OUT_OF_MEMORY;
                    }
                    table_width_index = ctx->table_width_state_count++;
                    ctx->table_width_states[table_width_index].id =
                        (imgui_id)parsed_id;
                    ctx->table_width_states[table_width_index].columns = 0;
                    ctx->table_width_states[table_width_index].widths = NULL;
                    ctx->table_width_states[table_width_index].sort_column = -1;
                    ctx->table_width_states[table_width_index].sort_direction = 0;
                    ctx->table_width_states[table_width_index].sort_specs = NULL;
                    ctx->table_width_states[table_width_index].sort_spec_count = 0;
                }
                imgui_internal_release(
                    &ctx->allocator,
                    ctx->table_width_states[table_width_index].widths);
                imgui_internal_release(
                    &ctx->allocator,
                    ctx->table_width_states[table_width_index].sort_specs);
                ctx->table_width_states[table_width_index].widths =
                    parsed_table_widths;
                ctx->table_width_states[table_width_index].columns =
                    parsed_columns;
                ctx->table_width_states[table_width_index].sort_specs =
                    parsed_sort_specs;
                ctx->table_width_states[table_width_index].sort_spec_count =
                    parsed_sort_count;
                ctx->table_width_states[table_width_index].sort_column =
                    parsed_sort_count > 0 ? parsed_sort_specs[0].column_index : -1;
                ctx->table_width_states[table_width_index].sort_direction =
                    parsed_sort_count > 0 ? parsed_sort_specs[0].direction : 0;
            }
            parsed_table_widths = NULL;
            parsed_sort_specs = NULL;
        } else if ((window_scan_count = sscanf(line,
                          "WINDOW %lu %f %f %f %f %f %f %d %lu %f %f %f %f %c", &parsed_id,
                          &parsed_x, &parsed_y, &parsed_width,
                          &parsed_height, &parsed_expanded_width,
                          &parsed_expanded_height, &parsed_open,
                          &parsed_viewport_id, &parsed_scroll,
                          &parsed_scroll_max, &parsed_scroll_x,
                          &parsed_scroll_max_x, &extra)) >= 8) {
            if (window_scan_count < 9) {
                parsed_viewport_id = 0UL;
                parsed_scroll = 0.0f;
                parsed_scroll_max = 0.0f;
                parsed_scroll_x = 0.0f;
                parsed_scroll_max_x = 0.0f;
            } else if (window_scan_count == 9) {
                parsed_scroll = 0.0f;
                parsed_scroll_max = 0.0f;
                parsed_scroll_x = 0.0f;
                parsed_scroll_max_x = 0.0f;
            } else if (window_scan_count != 13) {
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings window record is invalid");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            if (parsed_id == 0UL || parsed_id > 0xffffffffUL ||
                !imgui_float_is_finite(parsed_x) ||
                !imgui_float_is_finite(parsed_y) ||
                !imgui_float_is_finite(parsed_width) ||
                !imgui_float_is_finite(parsed_height) ||
                !imgui_float_is_finite(parsed_expanded_width) ||
                !imgui_float_is_finite(parsed_expanded_height) ||
                !imgui_float_is_finite(parsed_scroll) ||
                !imgui_float_is_finite(parsed_scroll_max) ||
                !imgui_float_is_finite(parsed_scroll_x) ||
                !imgui_float_is_finite(parsed_scroll_max_x) ||
                parsed_width <= 0.0f || parsed_height <= 0.0f ||
                parsed_expanded_width <= 0.0f || parsed_expanded_height <= 0.0f ||
                (parsed_open != 0 && parsed_open != 1) ||
                parsed_scroll < 0.0f || parsed_scroll_max < 0.0f ||
                parsed_scroll > parsed_scroll_max || parsed_scroll_x < 0.0f ||
                parsed_scroll_max_x < 0.0f ||
                parsed_scroll_x > parsed_scroll_max_x) {
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings window record is invalid");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            {
                imgui_window_desc window_desc;
                imgui_internal_window *window;
                int window_index;
                imgui_window_desc_init(&window_desc, NULL);
                window_desc.use_explicit_id = IMGUI_TRUE;
                window_desc.id = (imgui_id)parsed_id;
                window_index = imgui_window_find_or_create(ctx, &window_desc);
                if (window_index < 0) return IMGUI_RESULT_OUT_OF_MEMORY;
                window = &ctx->windows[window_index];
                if (window->settings_load_generation ==
                    ctx->settings_load_generation) {
                    imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                          "settings window record is duplicated");
                    return IMGUI_RESULT_CORRUPT_DATA;
                }
                window->settings_load_generation =
                    ctx->settings_load_generation;
                if ((window->flags & IMGUI_WINDOW_NO_SAVED_SETTINGS) == 0) {
                    window->position = imgui_make_vec2(parsed_x, parsed_y);
                    window->size = imgui_make_vec2(parsed_width, parsed_height);
                    window->expanded_size = imgui_make_vec2(
                        parsed_expanded_width, parsed_expanded_height);
                    window->scroll_y = parsed_scroll;
                    window->scroll_max_y = parsed_scroll_max;
                    window->scroll_x = parsed_scroll_x;
                    window->scroll_max_x = parsed_scroll_max_x;
                    window->collapsed = parsed_open ? IMGUI_TRUE : IMGUI_FALSE;
                    if (window->collapsed) window->size.y = 20.0f;
                    window->viewport_id = parsed_viewport_id != 0UL ?
                        (imgui_id)parsed_viewport_id : 0;
                }
            }
        } else if (sscanf(line, "VIEWPORT %lu %f %f %f %f %f %f %c",
                          &parsed_viewport_id, &parsed_x, &parsed_y,
                          &parsed_width, &parsed_height, &parsed_expanded_width,
                          &parsed_expanded_height, &extra) == 7) {
            if (parsed_viewport_id == 0UL || parsed_viewport_id > 0xffffffffUL ||
                !imgui_float_is_finite(parsed_x) ||
                !imgui_float_is_finite(parsed_y) ||
                !imgui_float_is_finite(parsed_width) ||
                !imgui_float_is_finite(parsed_height) ||
                !imgui_float_is_finite(parsed_expanded_width) ||
                !imgui_float_is_finite(parsed_expanded_height) ||
                parsed_width <= 0.0f || parsed_height <= 0.0f ||
                viewport_count >= IMGUI_INTERNAL_VIEWPORT_CAPACITY) {
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings viewport record is invalid");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            for (index = 0; index < viewport_count; ++index) {
                if (loaded_viewports[index].viewport_id ==
                    (imgui_id)parsed_viewport_id) {
                    imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                          "settings viewport record is duplicated");
                    return IMGUI_RESULT_CORRUPT_DATA;
                }
            }
            imgui_viewport_desc_init(&loaded_viewports[viewport_count],
                                     (imgui_id)parsed_viewport_id);
            loaded_viewports[viewport_count].position =
                imgui_make_vec2(parsed_x, parsed_y);
            loaded_viewports[viewport_count].size =
                imgui_make_vec2(parsed_width, parsed_height);
            loaded_viewports[viewport_count].framebuffer_scale =
                imgui_make_vec2(parsed_expanded_width, parsed_expanded_height);
            ++viewport_count;
        } else if (sscanf(line, "SPLIT %lu %d %f %lu %lu %c", &parsed_id,
                          &parsed_open, &parsed_ratio, &parsed_dock_id,
                          &parsed_child_b, &extra) == 5) {
            if (parsed_id == 0UL || parsed_dock_id == 0UL ||
                parsed_child_b == 0UL ||
                parsed_id > 0xffffffffUL || parsed_dock_id > 0xffffffffUL ||
                parsed_child_b > 0xffffffffUL ||
                parsed_open < (int)IMGUI_DOCK_SPLIT_LEFT ||
                parsed_open > (int)IMGUI_DOCK_SPLIT_DOWN ||
                !imgui_float_is_finite(parsed_ratio) ||
                parsed_ratio <= 0.0f || parsed_ratio >= 1.0f) {
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings split record is invalid");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            {
                int node_index;
                int child_index;
                node_index = imgui_dock_find_or_create(
                    ctx, (imgui_id)parsed_id);
                child_index = imgui_dock_find_or_create(
                    ctx, (imgui_id)parsed_dock_id);
                if (node_index < 0 || child_index < 0) {
                    return IMGUI_RESULT_OUT_OF_MEMORY;
                }
                if (imgui_dock_find_or_create(
                        ctx, (imgui_id)parsed_child_b) < 0) {
                    return IMGUI_RESULT_OUT_OF_MEMORY;
                }
                ctx->dock_nodes[node_index].split_direction =
                    (imgui_dock_split_direction)parsed_open;
                ctx->dock_nodes[node_index].split_ratio = parsed_ratio;
                ctx->dock_nodes[node_index].child_a =
                    (imgui_id)parsed_dock_id;
                ctx->dock_nodes[node_index].child_b =
                    (imgui_id)parsed_child_b;
                imgui_dock_apply_children(ctx, node_index);
            }
        } else if (sscanf(line, "DOCK %lu %lu %c", &parsed_id,
                          &parsed_dock_id, &extra) == 2) {
            if (parsed_id == 0UL || parsed_dock_id == 0UL ||
                parsed_id > 0xffffffffUL || parsed_dock_id > 0xffffffffUL) {
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings dock record is invalid");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            if (imgui_window_set_dock(ctx, (imgui_id)parsed_id,
                                      (imgui_id)parsed_dock_id) !=
                IMGUI_RESULT_OK) {
                return IMGUI_RESULT_OUT_OF_MEMORY;
            }
        } else if (sscanf(line, "ACTIVE %lu %lu %c", &parsed_id,
                          &parsed_dock_id, &extra) == 2) {
            if (parsed_id == 0UL || parsed_dock_id == 0UL ||
                parsed_id > 0xffffffffUL || parsed_dock_id > 0xffffffffUL) {
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings active dock record is invalid");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            if (imgui_dock_activate(ctx, (imgui_id)parsed_id,
                                    (imgui_id)parsed_dock_id) !=
                IMGUI_RESULT_OK) {
                return IMGUI_RESULT_CORRUPT_DATA;
            }
        } else if (sscanf(line, "TAB %lu %c", &parsed_id, &extra) == 1) {
            if (parsed_id > 0xffffffffUL) {
                imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                      "settings tab record is invalid");
                return IMGUI_RESULT_CORRUPT_DATA;
            }
            loaded_tab = (imgui_id)parsed_id;
        } else {
            imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                  "settings record is unknown");
            return IMGUI_RESULT_CORRUPT_DATA;
        }
    }
    ctx->tab_active_id = loaded_tab;
    for (index = 0; index < viewport_count; ++index) {
        if (imgui_viewport_configure(ctx, &loaded_viewports[index]) !=
            IMGUI_RESULT_OK) {
            return IMGUI_RESULT_CORRUPT_DATA;
        }
    }
    for (index = 0; index < ctx->window_count; ++index) {
        if (ctx->windows[index].viewport_id != 0 &&
            imgui_viewport_find(ctx, ctx->windows[index].viewport_id) < 0) {
            ctx->windows[index].viewport_id = 0;
        }
    }
    return IMGUI_RESULT_OK;
}

imgui_result imgui_settings_load(imgui_context *ctx,
                                  const char *data,
                                  size_t length)
{
    char *snapshot;
    size_t snapshot_required;
    imgui_result result;
    imgui_result restore_result;
    if (ctx == NULL || data == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    /* The parser below validates each record, but it intentionally stages
       into bounded local arrays and commits records incrementally. Preserve
       the complete pre-load state so a malformed or out-of-memory load is
       atomic from the caller's perspective. */
    snapshot_required = 0;
    result = imgui_settings_save(ctx, NULL, 0, &snapshot_required);
    if (result != IMGUI_RESULT_INVALID_ARGUMENT || snapshot_required == 0) {
        return result == IMGUI_RESULT_OK ? IMGUI_RESULT_OUT_OF_MEMORY : result;
    }
    snapshot = (char *)imgui_internal_allocate(&ctx->allocator,
                                               snapshot_required);
    if (snapshot == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    result = imgui_settings_save(ctx, snapshot, snapshot_required,
                                  &snapshot_required);
    if (result != IMGUI_RESULT_OK) {
        imgui_internal_release(&ctx->allocator, snapshot);
        return result;
    }
    result = imgui_settings_load_internal(ctx, data, length);
    if (result != IMGUI_RESULT_OK) {
        restore_result = imgui_settings_load_internal(
            ctx, snapshot, snapshot_required - 1U);
        if (restore_result != IMGUI_RESULT_OK) {
            imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                                  "settings rollback failed");
        }
    }
    imgui_internal_release(&ctx->allocator, snapshot);
    return result;
}

const imgui_render_packet *imgui_render(imgui_context *ctx)
{
    imgui_u32 command_index;
    const imgui_internal_window *cursor_window;
    if (ctx == NULL) {
        imgui_internal_report(NULL,
                              IMGUI_ERROR_INVALID_ARGUMENT,
                              "render requires a context");
        return NULL;
    }
    if (ctx->frame_state == IMGUI_INTERNAL_FRAME_BUILDING) {
        if (imgui_frame_end(ctx) != IMGUI_RESULT_OK) {
            return NULL;
        }
    }
    if (ctx->frame_state != IMGUI_INTERNAL_FRAME_ENDED &&
        ctx->frame_state != IMGUI_INTERNAL_FRAME_RENDERED) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_INVALID_STATE,
                              "render requires an ended frame");
        return NULL;
    }
    if (!imgui_renderer_has_capability(ctx,
                                       IMGUI_RENDERER_CAP_VERTEX_OFFSET)) {
        for (command_index = 0; command_index < ctx->command_count;
             ++command_index) {
            if (ctx->commands[command_index].type ==
                    IMGUI_RENDER_COMMAND_DRAW_INDEXED &&
                ctx->commands[command_index].data.draw_indexed.vertex_offset !=
                    0U) {
                imgui_internal_report(ctx, IMGUI_ERROR_UNSUPPORTED,
                                      "renderer lacks vertex-offset capability");
                return NULL;
            }
        }
    }
    ctx->packet.resource_operations = ctx->resource_operations;
    ctx->packet.resource_operation_count = ctx->resource_operation_count;
    ctx->render_list.vertices = ctx->vertices;
    ctx->render_list.vertex_count = ctx->vertex_count;
    ctx->render_list.indices = ctx->indices;
    ctx->render_list.index_count = ctx->index_count;
    ctx->render_list.commands = ctx->commands;
    ctx->render_list.command_count = ctx->command_count;
    imgui_build_viewport_packets(ctx);
    imgui_update_frame_output_capture(ctx);
    cursor_window = (ctx->last_window_index >= 0 &&
                     ctx->last_window_index < ctx->window_count) ?
                    &ctx->windows[ctx->last_window_index] : NULL;
    ctx->platform_output.mouse_cursor = ctx->text_input_active ?
        IMGUI_MOUSE_CURSOR_TEXT_INPUT : IMGUI_MOUSE_CURSOR_ARROW;
    if (!ctx->text_input_active && cursor_window != NULL &&
        (cursor_window->flags & IMGUI_WINDOW_NO_MOUSE_INPUTS) == 0) {
        if ((cursor_window->flags & IMGUI_WINDOW_NO_RESIZE) == 0 &&
            ctx->input.mouse_x >= cursor_window->position.x +
                cursor_window->size.x - 14.0f &&
            ctx->input.mouse_x < cursor_window->position.x +
                cursor_window->size.x &&
            ctx->input.mouse_y >= cursor_window->position.y +
                cursor_window->size.y - 14.0f &&
            ctx->input.mouse_y < cursor_window->position.y +
                cursor_window->size.y) {
            ctx->platform_output.mouse_cursor =
                IMGUI_MOUSE_CURSOR_RESIZE_DIAGONAL_NW_SE;
        } else if ((cursor_window->flags & IMGUI_WINDOW_NO_MOVE) == 0 &&
                   (cursor_window->flags & IMGUI_WINDOW_NO_TITLE_BAR) == 0 &&
                   ctx->input.mouse_y >= cursor_window->position.y &&
                   ctx->input.mouse_y < cursor_window->position.y + 20.0f &&
                   ctx->input.mouse_x >= cursor_window->position.x &&
                   ctx->input.mouse_x < cursor_window->position.x +
                       cursor_window->size.x) {
            ctx->platform_output.mouse_cursor = IMGUI_MOUSE_CURSOR_RESIZE_ALL;
        }
    }
    ctx->platform_output.set_mouse_position = IMGUI_FALSE;
    ctx->platform_output.ime.visible = ctx->text_input_active;
    ctx->platform_output.ime.wants_text_input = ctx->text_input_active;
    ctx->platform_output.ime.input_position = imgui_make_vec2(
        ctx->last_item_rect.x1, ctx->last_item_rect.y2);
    ctx->platform_output.ime.input_line_height =
        ctx->last_item_rect.y2 - ctx->last_item_rect.y1;
    ctx->platform_output.ime.viewport_id =
        (ctx->last_window_index >= 0 &&
         ctx->last_window_index < ctx->window_count) ?
        ctx->windows[ctx->last_window_index].viewport_id : 0;
    ctx->resource_operation_consumed_count =
        ctx->resource_operation_count;
    ctx->frame_state = IMGUI_INTERNAL_FRAME_RENDERED;
    return &ctx->packet;
}

const imgui_frame_output *imgui_get_frame_output(const imgui_context *ctx)
{
    return ctx != NULL ? &ctx->frame_output : NULL;
}

imgui_result imgui_get_metrics(const imgui_context *ctx,
                               imgui_metrics *metrics)
{
    imgui_metrics local_metrics;
    size_t copy_size;
    const imgui_texture *texture;
    imgui_u32 open_count;
    imgui_u32 texture_count;
    int index;
    if (ctx == NULL || metrics == NULL ||
        metrics->struct_size < sizeof(metrics->struct_size)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    imgui_metrics_init(&local_metrics);
    open_count = 0;
    for (index = 0; index < ctx->window_count; ++index) {
        if (ctx->windows[index].open) ++open_count;
    }
    texture_count = 0;
    texture = ctx->textures;
    while (texture != NULL) {
        if (texture->alive) ++texture_count;
        texture = texture->next;
    }
    local_metrics.frame_index = ctx->frame_index;
    local_metrics.window_count = (imgui_u32)ctx->window_count;
    local_metrics.open_window_count = open_count;
    local_metrics.texture_count = texture_count;
    local_metrics.vertex_count = ctx->vertex_count;
    local_metrics.index_count = ctx->index_count;
    local_metrics.command_count = ctx->command_count;
    local_metrics.resource_operation_count = ctx->resource_operation_count;
    local_metrics.navigation_item_count = ctx->navigation_item_count;
    local_metrics.active_item_id = ctx->active_item_valid ?
                              ctx->active_item_id : 0;
    local_metrics.focused_item_id = ctx->focused_item_valid ?
                               ctx->focused_item_id : 0;
    copy_size = metrics->struct_size < sizeof(local_metrics) ?
        metrics->struct_size : sizeof(local_metrics);
    memcpy(metrics, &local_metrics, copy_size);
    metrics->struct_size = local_metrics.struct_size;
    return IMGUI_RESULT_OK;
}

void imgui_show_metrics_window(imgui_context *ctx, imgui_bool *open)
{
    imgui_scope scope;
    imgui_window_desc desc;
    imgui_metrics metrics;
    if (ctx == NULL || open == NULL || !*open ||
        !imgui_internal_require_building(ctx,
                                         "metrics window outside frame")) {
        return;
    }
    imgui_window_desc_init(&desc, "Dear ImGui C89 Metrics");
    desc.open = open;
    desc.flags = IMGUI_WINDOW_ALWAYS_AUTO_RESIZE;
    scope = imgui_window_begin_ex(ctx, &desc);
    if (scope == IMGUI_SCOPE_ERROR) return;
    if (scope == IMGUI_SCOPE_ACTIVE &&
        imgui_get_metrics(ctx, &metrics) == IMGUI_RESULT_OK) {
        imgui_text(ctx, "Frame: %lu", (unsigned long)metrics.frame_index);
        imgui_text(ctx, "Windows: %lu open / %lu total",
                   (unsigned long)metrics.open_window_count,
                   (unsigned long)metrics.window_count);
        imgui_text(ctx, "Textures: %lu", (unsigned long)metrics.texture_count);
        imgui_text(ctx, "Geometry: %lu vertices, %lu indices",
                   (unsigned long)metrics.vertex_count,
                   (unsigned long)metrics.index_count);
        imgui_text(ctx, "Render commands: %lu",
                   (unsigned long)metrics.command_count);
        imgui_text(ctx, "Resource operations: %lu",
                   (unsigned long)metrics.resource_operation_count);
        imgui_text(ctx, "Navigation items: %d", metrics.navigation_item_count);
        imgui_text(ctx, "Active item: 0x%08lx",
                   (unsigned long)metrics.active_item_id);
        imgui_text(ctx, "Focused item: 0x%08lx",
                   (unsigned long)metrics.focused_item_id);
    }
    imgui_window_end(ctx);
}

void imgui_show_demo_window(imgui_context *ctx, imgui_bool *open)
{
    static const float plot_values[8] = {
        0.10f, 0.35f, 0.22f, 0.65f, 0.48f, 0.78f, 0.56f, 0.92f
    };
    imgui_scope scope;
    imgui_scope child_scope;
    imgui_scope tree_scope;
    imgui_scope table_scope;
    imgui_window_desc desc;
    imgui_id child_id;
    if (ctx == NULL || open == NULL || !*open ||
        !imgui_internal_require_building(ctx,
                                         "demo window outside frame")) {
        return;
    }
    imgui_window_desc_init(&desc, "Dear ImGui C89 Demo");
    desc.open = open;
    desc.flags = IMGUI_WINDOW_ALWAYS_AUTO_RESIZE;
    scope = imgui_window_begin_ex(ctx, &desc);
    if (scope == IMGUI_SCOPE_ERROR) return;
    if (scope == IMGUI_SCOPE_ACTIVE) {
        imgui_text(ctx, "First-class C89 API demonstration");
        imgui_text(ctx, "No overloads, no C++ ABI, deterministic packets.");
        imgui_separator(ctx);
        (void)imgui_checkbox(ctx, "Demo enabled", &ctx->demo_enabled);
        (void)imgui_slider_integer(ctx, "Integer", &ctx->demo_integer,
                                    0, 100);
        (void)imgui_slider_float(ctx, "Float", &ctx->demo_float,
                                 0.0f, 1.0f);
        (void)imgui_color_edit_rgba(ctx, "Color", ctx->demo_color,
                                    IMGUI_COLOR_EDIT_NONE);
        imgui_progress_bar_ex(ctx, ctx->demo_float,
                              imgui_make_vec2(220.0f, 18.0f),
                              "progress");
        imgui_plot_lines(ctx, "Sample plot", plot_values, 8,
                         imgui_make_vec2(220.0f, 60.0f), 0.0f, 1.0f);
        tree_scope = imgui_tree_node_begin(ctx, "Nested controls",
                                           0);
        if (tree_scope == IMGUI_SCOPE_ACTIVE) {
            imgui_text(ctx, "Tree state is ID-persistent.");
            imgui_same_line(ctx);
            (void)imgui_button(ctx, "Button");
        }
        if (tree_scope != IMGUI_SCOPE_ERROR) imgui_tree_node_end(ctx);
        child_id = imgui_get_id_string(ctx, "DemoChild");
        child_scope = imgui_child_begin(ctx, child_id,
                                        imgui_make_vec2(220.0f, 60.0f), 0);
        if (child_scope != IMGUI_SCOPE_ERROR) {
            imgui_text(ctx, "Child scopes preserve cursor and clipping.");
            imgui_child_end(ctx);
        }
        table_scope = imgui_table_begin(ctx, "DemoTable", 2,
                                        IMGUI_TABLE_NONE);
        if (table_scope == IMGUI_SCOPE_ACTIVE) {
            imgui_text(ctx, "Name");
            imgui_table_next_column(ctx);
            imgui_text(ctx, "Value");
            imgui_table_next_row(ctx);
            imgui_text(ctx, "Integer");
            imgui_table_next_column(ctx);
            imgui_text(ctx, "%d", ctx->demo_integer);
            imgui_table_end(ctx);
        }
    }
    imgui_window_end(ctx);
}

const imgui_style *imgui_style_get(const imgui_context *ctx)
{
    return ctx != NULL ? &ctx->style : NULL;
}

imgui_result imgui_style_set(imgui_context *ctx, const imgui_style *style)
{
    imgui_style local_style;
    size_t copy_size;
    if (ctx == NULL || style == NULL ||
        style->struct_size < sizeof(style->struct_size)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    imgui_style_init(&local_style);
    copy_size = style->struct_size < sizeof(local_style) ?
        style->struct_size : sizeof(local_style);
    memcpy(&local_style, style, copy_size);
    local_style.struct_size = sizeof(local_style);
    if (!imgui_float_is_finite(local_style.window_padding.x) ||
        !imgui_float_is_finite(local_style.window_padding.y) ||
        !imgui_float_is_finite(local_style.frame_padding.x) ||
        !imgui_float_is_finite(local_style.frame_padding.y) ||
        !imgui_float_is_finite(local_style.item_spacing) ||
        !imgui_float_is_finite(local_style.indent_spacing) ||
        !imgui_float_is_finite(local_style.window_rounding) ||
        !imgui_float_is_finite(local_style.frame_rounding) ||
        !imgui_float_is_finite(local_style.scrollbar_size) ||
        !imgui_float_is_finite(local_style.scrollbar_grab_min_size) ||
        !imgui_float_is_finite(local_style.child_rounding) ||
        !imgui_float_is_finite(local_style.window_border_size) ||
        !imgui_float_is_finite(local_style.child_border_size) ||
        !imgui_float_is_finite(local_style.frame_border_size) ||
        local_style.window_padding.x < 0.0f ||
        local_style.window_padding.y < 0.0f ||
        local_style.frame_padding.x < 0.0f ||
        local_style.frame_padding.y < 0.0f ||
        local_style.item_spacing < 0.0f ||
        local_style.indent_spacing < 0.0f ||
        local_style.window_rounding < 0.0f ||
        local_style.frame_rounding < 0.0f ||
        local_style.scrollbar_size <= 0.0f ||
        local_style.scrollbar_grab_min_size <= 0.0f ||
        local_style.child_rounding < 0.0f ||
        local_style.window_border_size < 0.0f ||
        local_style.child_border_size < 0.0f ||
        local_style.frame_border_size < 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->style = local_style;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_style_push(imgui_context *ctx, const imgui_style *style)
{
    imgui_result result;
    if (ctx == NULL || style == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (!imgui_internal_require_building(ctx,
                                         "style push outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    result = imgui_style_stack_reserve(ctx, ctx->style_stack_count + 1);
    if (result != IMGUI_RESULT_OK) return result;
    ctx->style_stack[ctx->style_stack_count] = ctx->style;
    result = imgui_style_set(ctx, style);
    if (result != IMGUI_RESULT_OK) return result;
    ctx->style_stack_count += 1;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_style_pop(imgui_context *ctx)
{
    if (ctx == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (!imgui_internal_require_building(ctx,
                                         "style pop outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->style_stack_count <= 0) return IMGUI_RESULT_INVALID_STATE;
    --ctx->style_stack_count;
    ctx->style = ctx->style_stack[ctx->style_stack_count];
    return IMGUI_RESULT_OK;
}

static imgui_bool imgui_style_set_color(imgui_style *style,
                                        imgui_style_color color,
                                        imgui_u32 value)
{
    if (style == NULL) return IMGUI_FALSE;
    switch (color) {
    case IMGUI_STYLE_COLOR_TEXT: style->color_text = value; break;
    case IMGUI_STYLE_COLOR_TEXT_DISABLED: style->color_text_disabled = value; break;
    case IMGUI_STYLE_COLOR_WINDOW_BACKGROUND: style->color_window_background = value; break;
    case IMGUI_STYLE_COLOR_HEADER: style->color_header = value; break;
    case IMGUI_STYLE_COLOR_HEADER_HOVERED: style->color_header_hovered = value; break;
    case IMGUI_STYLE_COLOR_HEADER_ACTIVE: style->color_header_active = value; break;
    case IMGUI_STYLE_COLOR_SEPARATOR: style->color_separator = value; break;
    case IMGUI_STYLE_COLOR_CHECK_MARK: style->color_check_mark = value; break;
    case IMGUI_STYLE_COLOR_BUTTON: style->color_button = value; break;
    case IMGUI_STYLE_COLOR_BUTTON_HOVERED: style->color_button_hovered = value; break;
    case IMGUI_STYLE_COLOR_BUTTON_ACTIVE: style->color_button_active = value; break;
    case IMGUI_STYLE_COLOR_FRAME: style->color_frame = value; break;
    case IMGUI_STYLE_COLOR_FRAME_HOVERED: style->color_frame_hovered = value; break;
    case IMGUI_STYLE_COLOR_FRAME_ACTIVE: style->color_frame_active = value; break;
    case IMGUI_STYLE_COLOR_WINDOW_TITLE_BACKGROUND: style->color_window_title_background = value; break;
    case IMGUI_STYLE_COLOR_WINDOW_TITLE_BACKGROUND_ACTIVE: style->color_window_title_background_active = value; break;
    case IMGUI_STYLE_COLOR_WINDOW_TITLE_BACKGROUND_COLLAPSED: style->color_window_title_background_collapsed = value; break;
    case IMGUI_STYLE_COLOR_WINDOW_TITLE_TEXT: style->color_window_title_text = value; break;
    case IMGUI_STYLE_COLOR_WINDOW_BORDER: style->color_window_border = value; break;
    case IMGUI_STYLE_COLOR_SCROLLBAR_BACKGROUND: style->color_scrollbar_background = value; break;
    case IMGUI_STYLE_COLOR_SCROLLBAR_GRAB: style->color_scrollbar_grab = value; break;
    case IMGUI_STYLE_COLOR_SCROLLBAR_GRAB_HOVERED: style->color_scrollbar_grab_hovered = value; break;
    case IMGUI_STYLE_COLOR_SCROLLBAR_GRAB_ACTIVE: style->color_scrollbar_grab_active = value; break;
    case IMGUI_STYLE_COLOR_TABLE_ROW_EVEN: style->color_table_row_even = value; break;
    case IMGUI_STYLE_COLOR_TABLE_ROW_ODD: style->color_table_row_odd = value; break;
    case IMGUI_STYLE_COLOR_TABLE_BORDER: style->color_table_border = value; break;
    case IMGUI_STYLE_COLOR_TEXT_SELECTION: style->color_text_selection = value; break;
    case IMGUI_STYLE_COLOR_PLOT_LINES: style->color_plot_lines = value; break;
    case IMGUI_STYLE_COLOR_PLOT_HISTOGRAM: style->color_plot_histogram = value; break;
    case IMGUI_STYLE_COLOR_MODAL_DIM: style->color_modal_dim = value; break;
    case IMGUI_STYLE_COLOR_CHILD_BACKGROUND: style->color_child_background = value; break;
    case IMGUI_STYLE_COLOR_POPUP_BACKGROUND: style->color_popup_background = value; break;
    case IMGUI_STYLE_COLOR_RESIZE_GRIP: style->color_resize_grip = value; break;
    case IMGUI_STYLE_COLOR_RESIZE_GRIP_HOVERED: style->color_resize_grip_hovered = value; break;
    case IMGUI_STYLE_COLOR_RESIZE_GRIP_ACTIVE: style->color_resize_grip_active = value; break;
    case IMGUI_STYLE_COLOR_TAB: style->color_tab = value; break;
    case IMGUI_STYLE_COLOR_TAB_HOVERED: style->color_tab_hovered = value; break;
    case IMGUI_STYLE_COLOR_TAB_ACTIVE: style->color_tab_active = value; break;
    case IMGUI_STYLE_COLOR_TAB_ACTIVE_HOVERED: style->color_tab_active_hovered = value; break;
    case IMGUI_STYLE_COLOR_DRAG_DROP_TARGET: style->color_drag_drop_target = value; break;
    case IMGUI_STYLE_COLOR_NAV_HIGHLIGHT: style->color_nav_highlight = value; break;
    case IMGUI_STYLE_COLOR_MENU_BAR_BACKGROUND: style->color_menu_bar_background = value; break;
    case IMGUI_STYLE_COLOR_MENU_ITEM: style->color_menu_item = value; break;
    case IMGUI_STYLE_COLOR_MENU_ITEM_HOVERED: style->color_menu_item_hovered = value; break;
    case IMGUI_STYLE_COLOR_MENU_ITEM_ACTIVE: style->color_menu_item_active = value; break;
    case IMGUI_STYLE_COLOR_POPUP_BORDER: style->color_popup_border = value; break;
    case IMGUI_STYLE_COLOR_TEXT_LINK: style->color_text_link = value; break;
    case IMGUI_STYLE_COLOR_TEXT_LINK_HOVERED:
        style->color_text_link_hovered = value; break;
    case IMGUI_STYLE_COLOR_TEXT_LINK_ACTIVE:
        style->color_text_link_active = value; break;
    default: return IMGUI_FALSE;
    }
    return IMGUI_TRUE;
}

imgui_result imgui_style_push_color(imgui_context *ctx,
                                    imgui_style_color color,
                                    imgui_u32 value)
{
    imgui_style style;
    imgui_result result;
    if (ctx == NULL || color < IMGUI_STYLE_COLOR_TEXT ||
        color > IMGUI_STYLE_COLOR_TEXT_LINK_ACTIVE) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    style = ctx->style;
    if (!imgui_style_set_color(&style, color, value)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    result = imgui_style_push(ctx, &style);
    return result;
}

imgui_result imgui_style_push_var_float(imgui_context *ctx,
                                        imgui_style_var var,
                                        float value)
{
    imgui_style style;
    if (ctx == NULL || var < IMGUI_STYLE_VAR_ITEM_SPACING ||
        var > IMGUI_STYLE_VAR_FRAME_BORDER_SIZE ||
        !imgui_float_is_finite(value) || value < 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    style = ctx->style;
    switch (var) {
    case IMGUI_STYLE_VAR_ITEM_SPACING: style.item_spacing = value; break;
    case IMGUI_STYLE_VAR_INDENT_SPACING: style.indent_spacing = value; break;
    case IMGUI_STYLE_VAR_WINDOW_ROUNDING: style.window_rounding = value; break;
    case IMGUI_STYLE_VAR_FRAME_ROUNDING: style.frame_rounding = value; break;
    case IMGUI_STYLE_VAR_CHILD_ROUNDING: style.child_rounding = value; break;
    case IMGUI_STYLE_VAR_WINDOW_BORDER_SIZE:
        style.window_border_size = value; break;
    case IMGUI_STYLE_VAR_CHILD_BORDER_SIZE:
        style.child_border_size = value; break;
    case IMGUI_STYLE_VAR_SCROLLBAR_SIZE:
        style.scrollbar_size = value; break;
    case IMGUI_STYLE_VAR_SCROLLBAR_GRAB_MIN_SIZE:
        style.scrollbar_grab_min_size = value; break;
    case IMGUI_STYLE_VAR_FRAME_BORDER_SIZE:
        style.frame_border_size = value; break;
    default: return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    return imgui_style_push(ctx, &style);
}

imgui_result imgui_style_push_var_vec2(imgui_context *ctx,
                                       imgui_style_var var,
                                       imgui_vec2 value)
{
    imgui_style style;
    if (ctx == NULL || (var != IMGUI_STYLE_VAR_WINDOW_PADDING &&
                        var != IMGUI_STYLE_VAR_FRAME_PADDING) ||
        !imgui_vec2_is_finite(value) || value.x < 0.0f || value.y < 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    style = ctx->style;
    if (var == IMGUI_STYLE_VAR_WINDOW_PADDING) style.window_padding = value;
    else style.frame_padding = value;
    return imgui_style_push(ctx, &style);
}

imgui_result imgui_style_pop_color(imgui_context *ctx)
{
    return imgui_style_pop(ctx);
}

void imgui_log_desc_init(imgui_log_desc *desc)
{
    if (desc == NULL) return;
    memset(desc, 0, sizeof(*desc));
    desc->struct_size = sizeof(*desc);
}

void imgui_localization_desc_init(imgui_localization_desc *desc)
{
    if (desc == NULL) return;
    memset(desc, 0, sizeof(*desc));
    desc->struct_size = sizeof(*desc);
}

imgui_result imgui_log_begin(imgui_context *ctx, const imgui_log_desc *desc)
{
    if (ctx == NULL || desc == NULL || desc->struct_size < sizeof(*desc) ||
        desc->callback == NULL || !imgui_internal_require_building(
            ctx, "log begin outside frame")) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (ctx->log_active) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                              "log is already active");
        return IMGUI_RESULT_INVALID_STATE;
    }
    ctx->log_desc = *desc;
    ctx->log_active = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_log_text(imgui_context *ctx, const char *begin,
                            const char *end)
{
    size_t length;
    if (ctx == NULL || begin == NULL || !ctx->log_active ||
        ctx->log_desc.callback == NULL) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (end == NULL) end = begin + strlen(begin);
    if (end < begin) return IMGUI_RESULT_INVALID_ARGUMENT;
    length = (size_t)(end - begin);
    ctx->log_desc.callback(begin, length, ctx->log_desc.user_data);
    return IMGUI_RESULT_OK;
}

imgui_result imgui_log_end(imgui_context *ctx)
{
    if (ctx == NULL || !ctx->log_active) return IMGUI_RESULT_INVALID_STATE;
    ctx->log_active = IMGUI_FALSE;
    memset(&ctx->log_desc, 0, sizeof(ctx->log_desc));
    return IMGUI_RESULT_OK;
}

imgui_bool imgui_log_is_active(const imgui_context *ctx)
{
    return ctx != NULL && ctx->log_active ? IMGUI_TRUE : IMGUI_FALSE;
}

imgui_result imgui_localization_configure(
    imgui_context *ctx, const imgui_localization_desc *desc)
{
    if (ctx == NULL || desc == NULL ||
        desc->struct_size < sizeof(*desc)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->localization = *desc;
    return IMGUI_RESULT_OK;
}

const char *imgui_localize(const imgui_context *ctx, const char *key)
{
    const char *translated;
    if (key == NULL) return "";
    if (ctx == NULL || ctx->localization.callback == NULL) return key;
    translated = ctx->localization.callback(key, strlen(key),
                                             ctx->localization.user_data);
    return translated != NULL ? translated : key;
}

imgui_result imgui_input_add_mouse_position(imgui_context *ctx,
                                            float x,
                                            float y)
{
    if (ctx == NULL || !imgui_float_is_finite(x) ||
        !imgui_float_is_finite(y)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->input.mouse_x = x;
    ctx->input.mouse_y = y;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_input_add_mouse_button(imgui_context *ctx,
                                          imgui_mouse_button button,
                                          imgui_bool down)
{
    if (ctx == NULL || button < 0 || button >= IMGUI_MOUSE_BUTTON_COUNT) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->input.mouse_down[(int)button] = down ? IMGUI_TRUE : IMGUI_FALSE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_input_add_mouse_wheel(imgui_context *ctx,
                                         float horizontal,
                                         float vertical)
{
    if (ctx == NULL || !imgui_float_is_finite(horizontal) ||
        !imgui_float_is_finite(vertical)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->input.mouse_wheel_x += horizontal;
    ctx->input.mouse_wheel_y += vertical;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_input_add_key(imgui_context *ctx,
                                 imgui_key key,
                                 imgui_bool down)
{
    if (ctx == NULL || key <= IMGUI_KEY_NONE || key >= IMGUI_KEY_COUNT) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->input.keys_down[(int)key] = down ? IMGUI_TRUE : IMGUI_FALSE;
    ctx->input.key_analog[(int)key] = down ? 1.0f : 0.0f;
    return IMGUI_RESULT_OK;
}

imgui_bool imgui_is_key_down(const imgui_context *ctx, imgui_key key)
{
    if (ctx == NULL || key <= IMGUI_KEY_NONE || key >= IMGUI_KEY_COUNT) {
        return IMGUI_FALSE;
    }
    return ctx->input.keys_down[(int)key];
}

float imgui_get_key_analog(const imgui_context *ctx, imgui_key key)
{
    if (ctx == NULL || key <= IMGUI_KEY_NONE || key >= IMGUI_KEY_COUNT) {
        return 0.0f;
    }
    return ctx->input.key_analog[(int)key];
}

imgui_bool imgui_is_key_pressed(const imgui_context *ctx,
                                imgui_key key,
                                imgui_bool repeat)
{
    if (ctx == NULL || key <= IMGUI_KEY_NONE || key >= IMGUI_KEY_COUNT) {
        return IMGUI_FALSE;
    }
    if (repeat) return ctx->input.keys_repeated[(int)key];
    return ctx->input.keys_pressed[(int)key];
}

int imgui_get_key_pressed_amount(const imgui_context *ctx,
                                 imgui_key key,
                                 float repeat_delay,
                                 float repeat_rate)
{
    float duration;
    float previous_duration;
    int previous_tick;
    int current_tick;
    if (ctx == NULL || key <= IMGUI_KEY_NONE || key >= IMGUI_KEY_COUNT) {
        return 0;
    }
    if (ctx->input.keys_pressed[(int)key]) return 1;
    if (!ctx->input.keys_down[(int)key]) return 0;
    if (!imgui_float_is_finite(repeat_delay) || repeat_delay < 0.0f) {
        repeat_delay = ctx->key_repeat_delay;
    }
    if (!imgui_float_is_finite(repeat_rate) || repeat_rate <= 0.0f) {
        repeat_rate = ctx->key_repeat_rate;
    }
    if (repeat_rate <= 0.0f) return 0;
    duration = ctx->input.key_down_duration[(int)key];
    previous_duration = duration - ctx->frame_desc.delta_time;
    if (duration < repeat_delay || previous_duration < repeat_delay) {
        previous_tick = -1;
    } else {
        previous_tick = (int)((previous_duration - repeat_delay) /
                              repeat_rate);
    }
    current_tick = duration >= repeat_delay ?
        (int)((duration - repeat_delay) / repeat_rate) : -1;
    return current_tick > previous_tick ? current_tick - previous_tick : 0;
}

imgui_bool imgui_is_key_released(const imgui_context *ctx, imgui_key key)
{
    if (ctx == NULL || key <= IMGUI_KEY_NONE || key >= IMGUI_KEY_COUNT) {
        return IMGUI_FALSE;
    }
    return ctx->input.keys_released[(int)key];
}

imgui_bool imgui_is_mouse_down(const imgui_context *ctx,
                               imgui_mouse_button button)
{
    if (ctx == NULL || button < 0 || button >= IMGUI_MOUSE_BUTTON_COUNT) {
        return IMGUI_FALSE;
    }
    return ctx->input.mouse_down[(int)button];
}

imgui_bool imgui_is_mouse_clicked(const imgui_context *ctx,
                                  imgui_mouse_button button)
{
    if (ctx == NULL || button < 0 || button >= IMGUI_MOUSE_BUTTON_COUNT) {
        return IMGUI_FALSE;
    }
    return ctx->mouse_clicked[(int)button];
}

int imgui_get_mouse_clicked_count(const imgui_context *ctx,
                                  imgui_mouse_button button)
{
    if (ctx == NULL || button < 0 || button >= IMGUI_MOUSE_BUTTON_COUNT) {
        return 0;
    }
    return ctx->mouse_clicked_count[(int)button];
}

imgui_bool imgui_is_mouse_released(const imgui_context *ctx,
                                   imgui_mouse_button button)
{
    if (ctx == NULL || button < 0 || button >= IMGUI_MOUSE_BUTTON_COUNT) {
        return IMGUI_FALSE;
    }
    return ctx->mouse_released[(int)button];
}

imgui_bool imgui_is_mouse_double_clicked(const imgui_context *ctx,
                                         imgui_mouse_button button)
{
    if (ctx == NULL || button < 0 || button >= IMGUI_MOUSE_BUTTON_COUNT) {
        return IMGUI_FALSE;
    }
    return ctx->mouse_double_clicked[(int)button];
}

imgui_bool imgui_is_any_mouse_down(const imgui_context *ctx)
{
    int button;
    if (ctx == NULL) return IMGUI_FALSE;
    for (button = 0; button < IMGUI_MOUSE_BUTTON_COUNT; ++button) {
        if (ctx->input.mouse_down[button]) return IMGUI_TRUE;
    }
    return IMGUI_FALSE;
}

imgui_bool imgui_is_mouse_dragging(const imgui_context *ctx,
                                   imgui_mouse_button button,
                                   float lock_threshold)
{
    float dx;
    float dy;
    if (ctx == NULL || button < 0 || button >= IMGUI_MOUSE_BUTTON_COUNT ||
        !ctx->input.mouse_down[(int)button]) {
        return IMGUI_FALSE;
    }
    if (!imgui_float_is_finite(lock_threshold) || lock_threshold < 0.0f) {
        lock_threshold = ctx->mouse_drag_threshold;
    }
    dx = ctx->input.mouse_x - ctx->mouse_down_start[(int)button].x;
    dy = ctx->input.mouse_y - ctx->mouse_down_start[(int)button].y;
    return dx * dx + dy * dy >= lock_threshold * lock_threshold ?
        IMGUI_TRUE : IMGUI_FALSE;
}

imgui_vec2 imgui_get_mouse_drag_delta(const imgui_context *ctx,
                                      imgui_mouse_button button,
                                      float lock_threshold)
{
    imgui_vec2 delta = imgui_make_vec2(0.0f, 0.0f);
    if (!imgui_is_mouse_dragging(ctx, button, lock_threshold)) return delta;
    delta.x = ctx->input.mouse_x - ctx->mouse_down_start[(int)button].x;
    delta.y = ctx->input.mouse_y - ctx->mouse_down_start[(int)button].y;
    return delta;
}

void imgui_reset_mouse_drag_delta(imgui_context *ctx,
                                  imgui_mouse_button button)
{
    if (ctx == NULL || button < 0 || button >= IMGUI_MOUSE_BUTTON_COUNT) {
        return;
    }
    ctx->mouse_down_start[(int)button] = imgui_make_vec2(
        ctx->input.mouse_x, ctx->input.mouse_y);
}

static imgui_bool imgui_modifier_is_down(const imgui_context *ctx,
                                         imgui_key_modifiers modifiers,
                                         imgui_key_modifiers modifier)
{
    if ((modifiers & modifier) == 0) return IMGUI_FALSE;
    if (modifier == IMGUI_KEY_MOD_CTRL) {
        return (imgui_is_key_down(ctx, IMGUI_KEY_LEFT_CTRL) ||
                imgui_is_key_down(ctx, IMGUI_KEY_RIGHT_CTRL)) ?
            IMGUI_TRUE : IMGUI_FALSE;
    }
    if (modifier == IMGUI_KEY_MOD_SHIFT) {
        return (imgui_is_key_down(ctx, IMGUI_KEY_LEFT_SHIFT) ||
                imgui_is_key_down(ctx, IMGUI_KEY_RIGHT_SHIFT)) ?
            IMGUI_TRUE : IMGUI_FALSE;
    }
    if (modifier == IMGUI_KEY_MOD_ALT) {
        return (imgui_is_key_down(ctx, IMGUI_KEY_LEFT_ALT) ||
                imgui_is_key_down(ctx, IMGUI_KEY_RIGHT_ALT)) ?
            IMGUI_TRUE : IMGUI_FALSE;
    }
    return (imgui_is_key_down(ctx, IMGUI_KEY_LEFT_SUPER) ||
            imgui_is_key_down(ctx, IMGUI_KEY_RIGHT_SUPER)) ?
        IMGUI_TRUE : IMGUI_FALSE;
}

imgui_bool imgui_is_shortcut_pressed(const imgui_context *ctx,
                                     imgui_key key,
                                     imgui_key_modifiers modifiers,
                                     imgui_bool repeat)
{
    imgui_key_modifiers known;
    imgui_key_modifiers modifier;
    if (ctx == NULL || key <= IMGUI_KEY_NONE || key >= IMGUI_KEY_COUNT) {
        return IMGUI_FALSE;
    }
    known = IMGUI_KEY_MOD_CTRL | IMGUI_KEY_MOD_SHIFT |
            IMGUI_KEY_MOD_ALT | IMGUI_KEY_MOD_SUPER;
    if ((modifiers & ~known) != 0) return IMGUI_FALSE;
    for (modifier = IMGUI_KEY_MOD_CTRL; modifier <= IMGUI_KEY_MOD_SUPER;
         modifier <<= 1) {
        if (imgui_modifier_is_down(ctx, modifiers, modifier) !=
                (((modifiers & modifier) != 0) ? IMGUI_TRUE : IMGUI_FALSE)) {
            return IMGUI_FALSE;
        }
    }
    return imgui_is_key_pressed(ctx, key, repeat);
}

imgui_result imgui_input_add_key_analog(imgui_context *ctx,
                                        imgui_key key,
                                        imgui_bool down,
                                        float value)
{
    if (ctx == NULL || key <= IMGUI_KEY_NONE || key >= IMGUI_KEY_COUNT ||
        !imgui_float_is_finite(value) || value < 0.0f || value > 1.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->input.keys_down[(int)key] = down ? IMGUI_TRUE : IMGUI_FALSE;
    ctx->input.key_analog[(int)key] = value;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_input_add_text_utf8(imgui_context *ctx, const char *text)
{
    size_t length;
    size_t needed;
    size_t capacity;
    char *storage;
    if (ctx == NULL || text == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    length = strlen(text);
    if (!imgui_utf8_valid(text, length)) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_ARGUMENT,
                              "text input is not valid UTF-8");
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (length > (size_t)-1 - ctx->pending_text_length - 1U) {
        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                              "text input length overflows storage size");
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    needed = ctx->pending_text_length + length + 1;
    if (needed > ctx->pending_text_capacity) {
        capacity = ctx->pending_text_capacity == 0 ? 32 :
                   ctx->pending_text_capacity;
        while (capacity < needed) {
            if (capacity > (size_t)-1 / 2U) {
                capacity = needed;
                break;
            }
            capacity *= 2;
        }
        storage = (char *)imgui_internal_allocate(&ctx->allocator, capacity);
        if (storage == NULL) {
            imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                                  "text event allocation failed");
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        if (ctx->pending_text_length != 0) {
            memcpy(storage, ctx->pending_text, ctx->pending_text_length);
        }
        imgui_internal_release(&ctx->allocator, ctx->pending_text);
        ctx->pending_text = storage;
        ctx->pending_text_capacity = capacity;
    }
    memcpy(ctx->pending_text + ctx->pending_text_length, text, length);
    ctx->pending_text_length += length;
    ctx->pending_text[ctx->pending_text_length] = '\0';
    return IMGUI_RESULT_OK;
}

imgui_result imgui_input_add_codepoint(imgui_context *ctx,
                                       unsigned long codepoint)
{
    char encoded[5];
    int length;
    if (ctx == NULL || codepoint > 0x10ffffUL ||
        (codepoint >= 0xd800UL && codepoint <= 0xdfffUL)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (codepoint <= 0x7fUL) {
        encoded[0] = (char)codepoint;
        length = 1;
    } else if (codepoint <= 0x7ffUL) {
        encoded[0] = (char)(0xc0UL | (codepoint >> 6));
        encoded[1] = (char)(0x80UL | (codepoint & 0x3fUL));
        length = 2;
    } else if (codepoint <= 0xffffUL) {
        encoded[0] = (char)(0xe0UL | (codepoint >> 12));
        encoded[1] = (char)(0x80UL | ((codepoint >> 6) & 0x3fUL));
        encoded[2] = (char)(0x80UL | (codepoint & 0x3fUL));
        length = 3;
    } else {
        encoded[0] = (char)(0xf0UL | (codepoint >> 18));
        encoded[1] = (char)(0x80UL | ((codepoint >> 12) & 0x3fUL));
        encoded[2] = (char)(0x80UL | ((codepoint >> 6) & 0x3fUL));
        encoded[3] = (char)(0x80UL | (codepoint & 0x3fUL));
        length = 4;
    }
    encoded[length] = '\0';
    return imgui_input_add_text_utf8(ctx, encoded);
}

imgui_result imgui_input_add_focus(imgui_context *ctx, imgui_bool focused)
{
    if (ctx == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->input.focused = focused ? IMGUI_TRUE : IMGUI_FALSE;
    if (!ctx->input.focused) {
        memset(ctx->input.mouse_down, 0, sizeof(ctx->input.mouse_down));
        memset(ctx->input.mouse_down_previous, 0,
               sizeof(ctx->input.mouse_down_previous));
        memset(ctx->input.keys_down, 0, sizeof(ctx->input.keys_down));
        memset(ctx->input.keys_down_previous, 0,
               sizeof(ctx->input.keys_down_previous));
        memset(ctx->input.keys_pressed, 0, sizeof(ctx->input.keys_pressed));
        memset(ctx->input.keys_released, 0,
               sizeof(ctx->input.keys_released));
        memset(ctx->input.keys_repeated, 0,
               sizeof(ctx->input.keys_repeated));
        memset(ctx->input.key_down_duration, 0,
               sizeof(ctx->input.key_down_duration));
        memset(ctx->mouse_clicked, 0, sizeof(ctx->mouse_clicked));
        memset(ctx->mouse_clicked_count, 0,
               sizeof(ctx->mouse_clicked_count));
        memset(ctx->mouse_down_duration, 0,
               sizeof(ctx->mouse_down_duration));
        memset(ctx->mouse_down_start, 0, sizeof(ctx->mouse_down_start));
        ctx->active_item_id = 0;
        ctx->active_item_valid = IMGUI_FALSE;
        ctx->text_input_active = IMGUI_FALSE;
        ctx->active_text_id = 0;
        ctx->text_edit_id = 0;
        imgui_drag_payload_clear(ctx);
    }
    return IMGUI_RESULT_OK;
}

imgui_vec2 imgui_get_mouse_position(const imgui_context *ctx)
{
    imgui_vec2 position = imgui_make_vec2(0.0f, 0.0f);
    if (ctx == NULL) return position;
    position.x = ctx->input.mouse_x;
    position.y = ctx->input.mouse_y;
    return position;
}

imgui_u32 imgui_get_frame_count(const imgui_context *ctx)
{
    return ctx != NULL ? ctx->frame_index : 0U;
}

float imgui_get_delta_time(const imgui_context *ctx)
{
    return ctx != NULL ? ctx->frame_desc.delta_time : 0.0f;
}

double imgui_get_time(const imgui_context *ctx)
{
    return ctx != NULL ? ctx->frame_desc.time : 0.0;
}

static void imgui_push_id_default(imgui_context *ctx)
{
    if (imgui_internal_require_building(ctx, "push ID outside frame")) {
        imgui_id_stack_push(ctx, imgui_id_seed(ctx));
    }
}

void imgui_push_id_string(imgui_context *ctx, const char *id)
{
    imgui_id value;
    if (id == NULL) {
        imgui_push_id_default(ctx);
        return;
    }
    value = imgui_hash_id_range(id, id + strlen(id), imgui_id_seed(ctx));
    imgui_id_stack_push(ctx, value);
}

void imgui_push_id_range(imgui_context *ctx,
                         const char *begin,
                         const char *end)
{
    imgui_id value;
    if (begin == NULL || end == NULL || end < begin) {
        imgui_push_id_default(ctx);
        return;
    }
    value = imgui_hash_id_range(begin, end, imgui_id_seed(ctx));
    imgui_id_stack_push(ctx, value);
}

void imgui_push_id_pointer(imgui_context *ctx, const void *id)
{
    imgui_id value;
    value = imgui_hash_bytes(&id, sizeof(id), imgui_id_seed(ctx));
    imgui_id_stack_push(ctx, value);
}

void imgui_push_id_integer(imgui_context *ctx, int id)
{
    imgui_id value;
    value = imgui_hash_bytes(&id, sizeof(id), imgui_id_seed(ctx));
    imgui_id_stack_push(ctx, value);
}

void imgui_push_id_value(imgui_context *ctx, imgui_id id)
{
    imgui_id value;
    value = imgui_hash_bytes(&id, sizeof(id), imgui_id_seed(ctx));
    imgui_id_stack_push(ctx, value);
}

void imgui_pop_id(imgui_context *ctx)
{
    if (!imgui_internal_require_building(ctx, "pop ID outside frame")) {
        return;
    }
    if (ctx->id_depth <= 0) {
        imgui_internal_report(ctx,
                              IMGUI_ERROR_INVALID_STATE,
                              "ID stack underflow");
        return;
    }
    ctx->id_depth -= 1;
}

imgui_id imgui_get_id_string(imgui_context *ctx, const char *id)
{
    if (ctx == NULL || id == NULL) {
        return 0;
    }
    return imgui_hash_id_range(id, id + strlen(id), imgui_id_seed(ctx));
}

imgui_id imgui_get_id_range(imgui_context *ctx,
                            const char *begin,
                            const char *end)
{
    if (ctx == NULL || begin == NULL || end == NULL || end < begin) {
        return 0;
    }
    return imgui_hash_id_range(begin, end, imgui_id_seed(ctx));
}

imgui_id imgui_get_id_pointer(imgui_context *ctx, const void *id)
{
    if (ctx == NULL || id == NULL) {
        return 0;
    }
    return imgui_hash_bytes(&id, sizeof(id), imgui_id_seed(ctx));
}

imgui_id imgui_get_id_integer(imgui_context *ctx, int id)
{
    if (ctx == NULL) {
        return 0;
    }
    return imgui_hash_bytes(&id, sizeof(id), imgui_id_seed(ctx));
}

static imgui_result imgui_window_store_title(
    imgui_context *ctx, imgui_internal_window *window, const char *title)
{
    size_t length;
    size_t needed;
    size_t capacity;
    char *storage;
    if (ctx == NULL || window == NULL || title == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    length = strlen(title);
    if (length > (size_t)-1 - 1U) return IMGUI_RESULT_OUT_OF_MEMORY;
    needed = length + 1U;
    if (window->title_capacity < needed) {
        capacity = window->title_capacity == 0 ? 64U :
                   window->title_capacity;
        while (capacity < needed) {
            if (capacity > (size_t)-1 / 2U) {
                capacity = needed;
                break;
            }
            capacity *= 2U;
        }
        storage = (char *)imgui_internal_allocate(&ctx->allocator, capacity);
        if (storage == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
        imgui_internal_release(&ctx->allocator, window->title);
        window->title = storage;
        window->title_capacity = capacity;
    }
    memcpy(window->title, title, needed);
    return IMGUI_RESULT_OK;
}

static int imgui_window_find_or_create(imgui_context *ctx,
                                       const imgui_window_desc *desc)
{
    imgui_id id;
    int index;
    imgui_internal_window *window;
    if (ctx == NULL) return -1;
    if (desc != NULL && desc->use_explicit_id) {
        id = desc->id;
    } else {
        id = imgui_get_id_string(ctx, desc != NULL && desc->title != NULL ?
                                 desc->title : "");
    }
    for (index = 0; index < ctx->window_count; ++index) {
        if (ctx->windows[index].id == id) return index;
    }
    if (imgui_window_reserve(ctx, ctx->window_count + 1) != IMGUI_RESULT_OK) {
        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                              "window allocation failed");
        return -1;
    }
    index = ctx->window_count++;
    window = &ctx->windows[index];
    memset(window, 0, sizeof(*window));
    window->id = id;
    window->z_order = ++ctx->next_window_z_order;
    if (desc != NULL && desc->title != NULL &&
        imgui_window_store_title(ctx, window, desc->title) !=
        IMGUI_RESULT_OK) {
        --ctx->window_count;
        return -1;
    }
    /* Dear ImGui's first-use fallback is a stable 400x400 window at (60,60).
       Applications can opt into independent placement through the explicit
       descriptor fields, while saved settings continue to override this
       fallback before the first visible frame. */
    window->position = imgui_make_vec2(60.0f, 60.0f);
    window->size = imgui_make_vec2(400.0f, 400.0f);
    window->initialized = IMGUI_TRUE;
    window->appearing = IMGUI_TRUE;
    window->auto_fit_pending = IMGUI_TRUE;
    window->open = IMGUI_TRUE;
    return index;
}

static imgui_bool imgui_window_scrollbar_geometry(
    const imgui_context *ctx,
    const imgui_internal_window *window,
    imgui_rect *track,
    imgui_rect *grab)
{
    float track_height;
    float content_height;
    float grab_height;
    float grab_range;
    float grab_offset;
    float title_height;
    if (ctx == NULL || window == NULL || track == NULL || grab == NULL ||
        (window->scroll_max_y <= 0.0f &&
         (window->flags & IMGUI_WINDOW_ALWAYS_VERTICAL_SCROLLBAR) == 0) ||
        (window->flags & IMGUI_WINDOW_NO_SCROLLBAR) != 0) {
        return IMGUI_FALSE;
    }
    title_height = (window->flags & IMGUI_WINDOW_NO_TITLE_BAR) != 0 ?
        1.0f : 19.0f;
    track->x1 = window->position.x + window->size.x -
                ctx->style.scrollbar_size;
    track->x2 = window->position.x + window->size.x - 2.0f;
    track->y1 = window->position.y + title_height;
    track->y2 = window->position.y + window->size.y - 1.0f;
    track_height = track->y2 - track->y1;
    if (track_height <= 0.0f) return IMGUI_FALSE;
    content_height = track_height + window->scroll_max_y;
    grab_height = track_height * track_height / content_height;
    if (grab_height < ctx->style.scrollbar_grab_min_size) {
        grab_height = ctx->style.scrollbar_grab_min_size;
    }
    if (grab_height > track_height) grab_height = track_height;
    grab_range = track_height - grab_height;
    grab_offset = window->scroll_max_y > 0.0f ?
        grab_range * window->scroll_y / window->scroll_max_y : 0.0f;
    *grab = *track;
    grab->y1 += grab_offset;
    grab->y2 = grab->y1 + grab_height;
    return IMGUI_TRUE;
}

static imgui_bool imgui_window_hscrollbar_geometry(
    const imgui_context *ctx,
    const imgui_internal_window *window,
    imgui_rect *track,
    imgui_rect *grab)
{
    float track_width;
    float content_width;
    float grab_width;
    float grab_range;
    float grab_offset;
    float title_height;
    if (ctx == NULL || window == NULL || track == NULL || grab == NULL ||
        (window->scroll_max_x <= 0.0f &&
         (window->flags & IMGUI_WINDOW_ALWAYS_HORIZONTAL_SCROLLBAR) == 0) ||
        (window->flags & (IMGUI_WINDOW_HORIZONTAL_SCROLLBAR |
                          IMGUI_WINDOW_ALWAYS_HORIZONTAL_SCROLLBAR)) == 0) {
        return IMGUI_FALSE;
    }
    title_height = (window->flags & IMGUI_WINDOW_NO_TITLE_BAR) != 0 ?
        1.0f : 19.0f;
    track->x1 = window->position.x + 1.0f;
    track->x2 = window->position.x + window->size.x -
                ctx->style.scrollbar_size;
    track->y1 = window->position.y + window->size.y -
                ctx->style.scrollbar_size;
    track->y2 = window->position.y + window->size.y - 2.0f;
    if (track->x2 <= track->x1 || track->y2 <= track->y1 ||
        track->y1 < window->position.y + title_height) {
        return IMGUI_FALSE;
    }
    track_width = track->x2 - track->x1;
    content_width = track_width + window->scroll_max_x;
    grab_width = track_width * track_width / content_width;
    if (grab_width < ctx->style.scrollbar_grab_min_size) {
        grab_width = ctx->style.scrollbar_grab_min_size;
    }
    if (grab_width > track_width) grab_width = track_width;
    grab_range = track_width - grab_width;
    grab_offset = window->scroll_max_x > 0.0f ?
        grab_range * window->scroll_x / window->scroll_max_x : 0.0f;
    *grab = *track;
    grab->x1 += grab_offset;
    grab->x2 = grab->x1 + grab_width;
    return IMGUI_TRUE;
}

static imgui_result imgui_reorder_window_commands(imgui_context *ctx)
{
    imgui_render_command *commands;
    void **payloads;
    unsigned char *covered;
    unsigned char *emitted;
    unsigned char *popup_covered;
    imgui_u32 output;
    imgui_u32 command_index;
    int order;
    int selected;
    int index;
    imgui_u32 selected_start;
    imgui_u32 best_z;
    if (ctx == NULL || (!ctx->window_z_order_dirty &&
                        !ctx->popup_render_valid &&
                        !ctx->combo_render_valid &&
                        !ctx->tooltip_render_valid) ||
        ctx->command_count == 0 || ctx->window_count == 0) {
        return IMGUI_RESULT_OK;
    }
    emitted = (unsigned char *)imgui_internal_allocate(
        &ctx->allocator, (size_t)ctx->window_count);
    commands = (imgui_render_command *)imgui_internal_allocate(
        &ctx->allocator, (size_t)ctx->command_count * sizeof(*commands));
    payloads = (void **)imgui_internal_allocate(
        &ctx->allocator, (size_t)ctx->command_count * sizeof(*payloads));
    covered = (unsigned char *)imgui_internal_allocate(
        &ctx->allocator, (size_t)ctx->command_count);
    popup_covered = (unsigned char *)imgui_internal_allocate(
        &ctx->allocator, (size_t)ctx->command_count);
    if (emitted == NULL || commands == NULL || payloads == NULL ||
        covered == NULL || popup_covered == NULL) {
        imgui_internal_release(&ctx->allocator, emitted);
        imgui_internal_release(&ctx->allocator, commands);
        imgui_internal_release(&ctx->allocator, payloads);
        imgui_internal_release(&ctx->allocator, covered);
        imgui_internal_release(&ctx->allocator, popup_covered);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    memset(covered, 0, (size_t)ctx->command_count);
    memset(popup_covered, 0, (size_t)ctx->command_count);
    memset(emitted, 0, (size_t)ctx->window_count);
    output = 0;
    if (ctx->popup_render_valid && ctx->popup_render_start <=
        ctx->popup_render_end && ctx->popup_render_end <= ctx->command_count) {
        for (command_index = ctx->popup_render_start;
             command_index < ctx->popup_render_end; ++command_index) {
            popup_covered[command_index] = 1;
        }
    }
    if (ctx->combo_render_valid && ctx->combo_render_start <=
        ctx->combo_render_end && ctx->combo_render_end <= ctx->command_count) {
        for (command_index = ctx->combo_render_start;
             command_index < ctx->combo_render_end; ++command_index) {
            popup_covered[command_index] = 1;
        }
    }
    if (ctx->tooltip_render_valid && ctx->tooltip_render_start <=
        ctx->tooltip_render_end &&
        ctx->tooltip_render_end <= ctx->command_count) {
        for (command_index = ctx->tooltip_render_start;
             command_index < ctx->tooltip_render_end; ++command_index) {
            popup_covered[command_index] = 1;
        }
    }
    for (order = 0; order < ctx->window_count; ++order) {
        selected = -1;
        best_z = 0xffffffffUL;
        for (index = 0; index < ctx->window_count; ++index) {
            if (!emitted[index] && ctx->windows[index].z_order < best_z) {
                selected = index;
                best_z = ctx->windows[index].z_order;
            }
        }
        if (selected < 0) break;
        emitted[selected] = 1;
        selected_start = output;
        if (ctx->windows[selected].command_start > ctx->command_count ||
            ctx->windows[selected].command_end > ctx->command_count ||
            ctx->windows[selected].command_end <
            ctx->windows[selected].command_start) continue;
        for (command_index = ctx->windows[selected].command_start;
             command_index < ctx->windows[selected].command_end;
             ++command_index) {
            if (covered[command_index] == 0 &&
                popup_covered[command_index] == 0) {
                commands[output] = ctx->commands[command_index];
                payloads[output] = ctx->command_payloads != NULL ?
                    ctx->command_payloads[command_index] : NULL;
                covered[command_index] = 1;
                ++output;
            }
        }
        ctx->windows[selected].command_start = selected_start;
        ctx->windows[selected].command_end = output;
    }
    /* Append every overlay command in its original order.  This keeps
       popup/combo/tooltip nesting and interleaving deterministic while still
       placing all overlay surfaces above ordinary window ranges. */
    for (command_index = 0; command_index < ctx->command_count;
         ++command_index) {
        if (popup_covered[command_index] != 0 &&
            covered[command_index] == 0) {
            commands[output] = ctx->commands[command_index];
            payloads[output] = ctx->command_payloads != NULL ?
                ctx->command_payloads[command_index] : NULL;
            covered[command_index] = 1;
            ++output;
        }
    }
    for (command_index = 0; command_index < ctx->command_count;
         ++command_index) {
        if (covered[command_index] == 0) {
            commands[output] = ctx->commands[command_index];
            payloads[output] = ctx->command_payloads != NULL ?
                ctx->command_payloads[command_index] : NULL;
            ++output;
        }
    }
    memcpy(ctx->commands, commands, (size_t)ctx->command_count *
           sizeof(*ctx->commands));
    if (ctx->command_payloads != NULL) {
        memcpy(ctx->command_payloads, payloads, (size_t)ctx->command_count *
               sizeof(*ctx->command_payloads));
    }
    imgui_internal_release(&ctx->allocator, commands);
    imgui_internal_release(&ctx->allocator, payloads);
    imgui_internal_release(&ctx->allocator, covered);
    imgui_internal_release(&ctx->allocator, popup_covered);
    imgui_internal_release(&ctx->allocator, emitted);
    ctx->window_z_order_dirty = IMGUI_FALSE;
    ctx->popup_render_valid = IMGUI_FALSE;
    ctx->combo_render_valid = IMGUI_FALSE;
    ctx->combo_render_tracking = IMGUI_FALSE;
    ctx->tooltip_render_valid = IMGUI_FALSE;
    ctx->tooltip_render_tracking = IMGUI_FALSE;
    return IMGUI_RESULT_OK;
}

/* Dear ImGui keeps an appearing window's item state alive but does not submit
   its draw list until the following frame. Compact those command ranges after
   z-ordering so the immutable packet observes the same lifecycle without
   invalidating vertex/index storage or resource payload ownership. */
static imgui_result imgui_remove_hidden_window_commands(imgui_context *ctx)
{
    unsigned char *hidden;
    imgui_render_command *commands;
    void **payloads;
    imgui_u32 index;
    imgui_u32 output;
    int window_index;
    imgui_bool any_hidden;
    if (ctx == NULL || ctx->command_count == 0 || ctx->window_count == 0) {
        return IMGUI_RESULT_OK;
    }
    any_hidden = IMGUI_FALSE;
    for (window_index = 0; window_index < ctx->window_count; ++window_index) {
        if (ctx->windows[window_index].hidden_this_frame) {
            any_hidden = IMGUI_TRUE;
            break;
        }
    }
    if (!any_hidden) return IMGUI_RESULT_OK;
    hidden = (unsigned char *)imgui_internal_allocate(
        &ctx->allocator, (size_t)ctx->command_count);
    commands = (imgui_render_command *)imgui_internal_allocate(
        &ctx->allocator, (size_t)ctx->command_count * sizeof(*commands));
    payloads = (void **)imgui_internal_allocate(
        &ctx->allocator, (size_t)ctx->command_count * sizeof(*payloads));
    if (hidden == NULL || commands == NULL || payloads == NULL) {
        imgui_internal_release(&ctx->allocator, hidden);
        imgui_internal_release(&ctx->allocator, commands);
        imgui_internal_release(&ctx->allocator, payloads);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    memset(hidden, 0, (size_t)ctx->command_count);
    for (window_index = 0; window_index < ctx->window_count; ++window_index) {
        imgui_internal_window *window = &ctx->windows[window_index];
        if (!window->hidden_this_frame) continue;
        if (window->command_start > ctx->command_count ||
            window->command_end > ctx->command_count ||
            window->command_end < window->command_start) continue;
        for (index = window->command_start; index < window->command_end;
             ++index) hidden[index] = 1;
    }
    output = 0;
    for (index = 0; index < ctx->command_count; ++index) {
        if (hidden[index] != 0) continue;
        commands[output] = ctx->commands[index];
        payloads[output] = ctx->command_payloads != NULL ?
            ctx->command_payloads[index] : NULL;
        ++output;
    }
    for (window_index = 0; window_index < ctx->window_count; ++window_index) {
        imgui_internal_window *window = &ctx->windows[window_index];
        imgui_u32 start = 0;
        imgui_u32 end = 0;
        for (index = 0; index < window->command_start &&
             index < ctx->command_count; ++index) {
            if (hidden[index] == 0) ++start;
        }
        for (index = 0; index < window->command_end &&
             index < ctx->command_count; ++index) {
            if (hidden[index] == 0) ++end;
        }
        window->command_start = start;
        window->command_end = end;
        window->hidden_this_frame = IMGUI_FALSE;
    }
    memcpy(ctx->commands, commands, (size_t)output * sizeof(*commands));
    if (ctx->command_payloads != NULL) {
        memcpy(ctx->command_payloads, payloads,
               (size_t)output * sizeof(*payloads));
    }
    ctx->command_count = output;
    imgui_internal_release(&ctx->allocator, hidden);
    imgui_internal_release(&ctx->allocator, commands);
    imgui_internal_release(&ctx->allocator, payloads);
    return IMGUI_RESULT_OK;
}

static void imgui_build_viewport_packets(imgui_context *ctx)
{
    int viewport_index;
    int window_index;
    int order;
    int selected;
    int list_index;
    imgui_bool has_external_window;
    unsigned char *emitted;
    imgui_u32 best_z;
    imgui_internal_viewport *config;
    imgui_viewport_packet *packet;
    imgui_render_list *list;
    imgui_internal_window *window;
    if (ctx == NULL) return;
    if (imgui_viewport_lists_reserve(ctx, ctx->window_count) !=
            IMGUI_RESULT_OK) {
        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                              "viewport list allocation failed");
        return;
    }
    emitted = (unsigned char *)imgui_internal_allocate(
        &ctx->allocator, (size_t)ctx->window_count);
    if (emitted == NULL && ctx->window_count != 0) {
        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                              "viewport z-order storage allocation failed");
        return;
    }
    if (emitted != NULL) {
        memset(emitted, 0, (size_t)ctx->window_count);
    }
    has_external_window = IMGUI_FALSE;
    for (window_index = 0; window_index < ctx->window_count;
         ++window_index) {
        if (ctx->windows[window_index].viewport_id != 0) {
            has_external_window = IMGUI_TRUE;
            break;
        }
    }
    if (!has_external_window) {
        ctx->render_list.vertices = ctx->vertices;
        ctx->render_list.vertex_count = ctx->command_count != 0U ?
            ctx->vertex_count : 0U;
        ctx->render_list.indices = ctx->indices;
        ctx->render_list.index_count = ctx->command_count != 0U ?
            ctx->index_count : 0U;
        ctx->render_list.commands = ctx->commands;
        ctx->render_list.command_count = ctx->command_count;
        ctx->viewport.viewport_id = 0;
        ctx->viewport.display_position = imgui_make_vec2(0.0f, 0.0f);
        ctx->viewport.display_size = ctx->frame_desc.display_size;
        ctx->viewport.framebuffer_scale = ctx->frame_desc.framebuffer_scale;
        ctx->viewport.lists = &ctx->render_list;
        ctx->viewport.list_count = ctx->command_count != 0U ? 1U : 0U;
        ctx->packet.viewports = &ctx->viewport;
        ctx->packet.viewport_count = 1;
        imgui_internal_release(&ctx->allocator, emitted);
        return;
    }
    for (viewport_index = 0; viewport_index < ctx->viewport_count;
         ++viewport_index) {
        config = &ctx->viewport_configs[viewport_index];
        packet = &ctx->viewport_packets[viewport_index];
        packet->viewport_id = config->desc.viewport_id;
        if (viewport_index == 0) {
            packet->display_position = imgui_make_vec2(0.0f, 0.0f);
            packet->display_size = ctx->frame_desc.display_size;
            packet->framebuffer_scale = ctx->frame_desc.framebuffer_scale;
        } else {
            packet->display_position = config->desc.position;
            packet->display_size = config->desc.size;
            packet->framebuffer_scale = config->desc.framebuffer_scale;
        }
        packet->lists = ctx->viewport_lists[viewport_index];
        packet->list_count = 0;
    }
    /* Extract lists in the same back-to-front z-order used when command
       ranges were reordered. Creation order is not a valid substitute after
       a window is focused or brought to the front. */
    for (order = 0; order < ctx->window_count; ++order) {
        selected = -1;
        best_z = 0xffffffffUL;
        for (window_index = 0; window_index < ctx->window_count;
             ++window_index) {
            if (emitted[window_index] == 0 &&
                ctx->windows[window_index].z_order < best_z) {
                selected = window_index;
                best_z = ctx->windows[window_index].z_order;
            }
        }
        if (selected < 0) break;
        emitted[selected] = 1;
        window = &ctx->windows[selected];
        if (window->command_end <= window->command_start ||
            window->command_start > ctx->command_count ||
            window->command_end > ctx->command_count) continue;
        viewport_index = imgui_viewport_find(ctx, window->viewport_id);
        if (viewport_index < 0) viewport_index = 0;
        packet = &ctx->viewport_packets[viewport_index];
        if (packet->list_count >= (imgui_u32)ctx->viewport_list_capacity) {
            continue;
        }
        list_index = (int)packet->list_count++;
        list = &ctx->viewport_lists[viewport_index][list_index];
        list->vertices = ctx->vertices;
        list->vertex_count = ctx->command_count != 0U ? ctx->vertex_count : 0U;
        list->indices = ctx->indices;
        list->index_count = ctx->command_count != 0U ? ctx->index_count : 0U;
        list->commands = ctx->commands + window->command_start;
        list->command_count = window->command_end - window->command_start;
    }
    imgui_internal_release(&ctx->allocator, emitted);
    ctx->packet.viewports = ctx->viewport_packets;
    ctx->packet.viewport_count = (imgui_u32)ctx->viewport_count;
}

static imgui_result imgui_dock_reserve(imgui_context *ctx, int required)
{
    int capacity;
    size_t bytes;
    imgui_internal_dock_node *nodes;
    if (ctx == NULL || required < 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (required <= ctx->dock_node_capacity) return IMGUI_RESULT_OK;
    capacity = ctx->dock_node_capacity;
    if (capacity < IMGUI_INTERNAL_DOCK_CAPACITY) {
        capacity = IMGUI_INTERNAL_DOCK_CAPACITY;
    }
    while (capacity < required) {
        if (capacity > 1073741823) {
            capacity = required;
            break;
        }
        capacity *= 2;
    }
    if ((size_t)capacity > (size_t)-1 / sizeof(*nodes)) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    bytes = (size_t)capacity * sizeof(*nodes);
    nodes = (imgui_internal_dock_node *)imgui_internal_allocate(
        &ctx->allocator, bytes);
    if (nodes == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    if (ctx->dock_node_count > 0) {
        memcpy(nodes, ctx->dock_nodes,
               (size_t)ctx->dock_node_count * sizeof(*nodes));
    }
    imgui_internal_release(&ctx->allocator, ctx->dock_nodes);
    ctx->dock_nodes = nodes;
    ctx->dock_node_capacity = capacity;
    return IMGUI_RESULT_OK;
}

static int imgui_dock_find_or_create(imgui_context *ctx, imgui_id dock_id)
{
    int index;
    if (ctx == NULL || dock_id == 0) return -1;
    for (index = 0; index < ctx->dock_node_count; ++index) {
        if (ctx->dock_nodes[index].id == dock_id) return index;
    }
    if (imgui_dock_reserve(ctx, ctx->dock_node_count + 1) !=
            IMGUI_RESULT_OK) {
        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                              "dock node allocation failed");
        return -1;
    }
    index = ctx->dock_node_count++;
    memset(&ctx->dock_nodes[index], 0, sizeof(ctx->dock_nodes[index]));
    ctx->dock_nodes[index].id = dock_id;
    return index;
}

static void imgui_dock_apply_children(imgui_context *ctx, int parent_index)
{
    imgui_internal_dock_node *parent;
    imgui_internal_dock_node *first;
    imgui_internal_dock_node *second;
    int first_index;
    int second_index;
    float split;
    if (ctx == NULL || parent_index < 0 ||
        parent_index >= ctx->dock_node_count) return;
    parent = &ctx->dock_nodes[parent_index];
    if (parent->child_a == 0 || parent->child_b == 0 ||
        parent->split_direction == IMGUI_DOCK_SPLIT_NONE) return;
    first_index = imgui_dock_find_or_create(ctx, parent->child_a);
    second_index = imgui_dock_find_or_create(ctx, parent->child_b);
    if (first_index < 0 || second_index < 0) return;
    first = &ctx->dock_nodes[first_index];
    second = &ctx->dock_nodes[second_index];
    split = parent->split_ratio;
    if (parent->split_direction == IMGUI_DOCK_SPLIT_LEFT ||
        parent->split_direction == IMGUI_DOCK_SPLIT_RIGHT) {
        float width = parent->size.x * split;
        first->position = parent->position;
        first->size = imgui_make_vec2(width, parent->size.y);
        second->position = imgui_make_vec2(parent->position.x + width,
                                           parent->position.y);
        second->size = imgui_make_vec2(parent->size.x - width,
                                       parent->size.y);
    } else {
        float height = parent->size.y * split;
        first->position = parent->position;
        first->size = imgui_make_vec2(parent->size.x, height);
        second->position = imgui_make_vec2(parent->position.x,
                                           parent->position.y + height);
        second->size = imgui_make_vec2(parent->size.x,
                                       parent->size.y - height);
    }
    if (parent->split_direction == IMGUI_DOCK_SPLIT_RIGHT ||
        parent->split_direction == IMGUI_DOCK_SPLIT_DOWN) {
        imgui_vec2 position = first->position;
        imgui_vec2 size = first->size;
        first->position = second->position;
        first->size = second->size;
        second->position = position;
        second->size = size;
    }
    first->initialized = IMGUI_TRUE;
    second->initialized = IMGUI_TRUE;
}

static void imgui_window_try_dock(imgui_context *ctx,
                                  imgui_internal_window *window)
{
    int index;
    int best_index;
    float area;
    float best_area;
    imgui_internal_dock_node *node;
    if (ctx == NULL || window == NULL || window->dock_id != 0) return;
    best_index = -1;
    best_area = 0.0f;
    for (index = 0; index < ctx->dock_node_count; ++index) {
        node = &ctx->dock_nodes[index];
        if (!node->initialized || node->size.x <= 0.0f ||
            node->size.y <= 0.0f ||
            ctx->input.mouse_x < node->position.x ||
            ctx->input.mouse_x >= node->position.x + node->size.x ||
            ctx->input.mouse_y < node->position.y ||
            ctx->input.mouse_y >= node->position.y + node->size.y) {
            continue;
        }
        area = node->size.x * node->size.y;
        if (best_index < 0 || area < best_area) {
            best_index = index;
            best_area = area;
        }
    }
    if (best_index >= 0) {
        window->dock_id = ctx->dock_nodes[best_index].id;
        ctx->dock_nodes[best_index].active_window_id = window->id;
    }
}

imgui_result imgui_dock_space_split(imgui_context *ctx, imgui_id parent_id,
                                    imgui_dock_split_direction direction,
                                    float ratio, imgui_id child_id)
{
    int parent_index;
    int child_index;
    if (ctx == NULL || parent_id == 0 || child_id == 0 ||
        parent_id == child_id || direction <= IMGUI_DOCK_SPLIT_NONE ||
        direction > IMGUI_DOCK_SPLIT_DOWN || ratio <= 0.0f ||
        ratio >= 1.0f) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (!imgui_internal_require_building(ctx,
                                         "dock split outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    parent_index = imgui_dock_find_or_create(ctx, parent_id);
    child_index = imgui_dock_find_or_create(ctx, child_id);
    if (parent_index < 0 || child_index < 0) return IMGUI_RESULT_OUT_OF_MEMORY;
    ctx->dock_nodes[parent_index].split_direction = direction;
    ctx->dock_nodes[parent_index].split_ratio = ratio;
    if (ctx->dock_nodes[parent_index].child_a == 0) {
        ctx->dock_nodes[parent_index].child_a = child_id;
    } else if (ctx->dock_nodes[parent_index].child_b == 0 &&
               ctx->dock_nodes[parent_index].child_a != child_id) {
        ctx->dock_nodes[parent_index].child_b = child_id;
    } else {
        return IMGUI_RESULT_INVALID_STATE;
    }
    imgui_dock_apply_children(ctx, parent_index);
    return IMGUI_RESULT_OK;
}

imgui_result imgui_dock_space(imgui_context *ctx, imgui_id dock_id,
                              imgui_vec2 size, imgui_dock_flags flags)
{
    int node_index;
    if (ctx == NULL || dock_id == 0 ||
        !imgui_internal_require_building(ctx, "dock space outside frame")) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (size.x <= 0.0f) size.x = ctx->window_size.x -
                                     2.0f * ctx->style.window_padding.x;
    if (size.y <= 0.0f) size.y = 100.0f;
    node_index = imgui_dock_find_or_create(ctx, dock_id);
    if (node_index < 0) return IMGUI_RESULT_OUT_OF_MEMORY;
    ctx->dock_nodes[node_index].position = ctx->cursor;
    ctx->dock_nodes[node_index].size = size;
    ctx->dock_nodes[node_index].flags = flags;
    ctx->dock_nodes[node_index].initialized = IMGUI_TRUE;
    imgui_dock_apply_children(ctx, node_index);
    if (!imgui_item_register(ctx, dock_id, size)) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if ((flags & IMGUI_DOCK_PASSTHRU_CENTRAL) == 0) {
        (void)imgui_mesh_add_rect(ctx, ctx->last_item_rect,
                                  ctx->style.color_frame);
    }
    return IMGUI_RESULT_OK;
}

imgui_result imgui_window_set_dock(imgui_context *ctx, imgui_id window_id,
                                   imgui_id dock_id)
{
    imgui_window_desc desc;
    int window_index;
    if (ctx == NULL || window_id == 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    imgui_window_desc_init(&desc, NULL);
    desc.use_explicit_id = IMGUI_TRUE;
    desc.id = window_id;
    window_index = imgui_window_find_or_create(ctx, &desc);
    if (window_index < 0) return IMGUI_RESULT_OUT_OF_MEMORY;
    ctx->windows[window_index].dock_id = dock_id;
    if (dock_id != 0) {
        int dock_index = imgui_dock_find_or_create(ctx, dock_id);
        if (dock_index < 0) return IMGUI_RESULT_OUT_OF_MEMORY;
        if (ctx->dock_nodes[dock_index].active_window_id == 0) {
            ctx->dock_nodes[dock_index].active_window_id = window_id;
        }
    }
    return IMGUI_RESULT_OK;
}

imgui_id imgui_window_get_dock(const imgui_context *ctx, imgui_id window_id)
{
    int index;
    if (ctx == NULL || window_id == 0) return 0;
    for (index = 0; index < ctx->window_count; ++index) {
        if (ctx->windows[index].id == window_id) {
            return ctx->windows[index].dock_id;
        }
    }
    return 0;
}

imgui_result imgui_dock_activate(imgui_context *ctx, imgui_id dock_id,
                                 imgui_id window_id)
{
    int dock_index;
    int window_index;
    if (ctx == NULL || dock_id == 0 || window_id == 0) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    dock_index = imgui_dock_find_or_create(ctx, dock_id);
    if (dock_index < 0) return IMGUI_RESULT_OUT_OF_MEMORY;
    for (window_index = 0; window_index < ctx->window_count; ++window_index) {
        if (ctx->windows[window_index].id == window_id) break;
    }
    if (window_index >= ctx->window_count ||
        ctx->windows[window_index].dock_id != dock_id) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->dock_nodes[dock_index].active_window_id = window_id;
    return IMGUI_RESULT_OK;
}

imgui_id imgui_dock_get_active_window(const imgui_context *ctx,
                                      imgui_id dock_id)
{
    int dock_index;
    if (ctx == NULL || dock_id == 0) return 0;
    for (dock_index = 0; dock_index < ctx->dock_node_count; ++dock_index) {
        if (ctx->dock_nodes[dock_index].id == dock_id) {
            return ctx->dock_nodes[dock_index].active_window_id;
        }
    }
    return 0;
}

imgui_bool imgui_dock_tab_button(imgui_context *ctx, imgui_id dock_id,
                                 imgui_id window_id, const char *label)
{
    imgui_id id;
    imgui_vec2 size;
    imgui_u32 color;
    if (ctx == NULL || dock_id == 0 || window_id == 0) return IMGUI_FALSE;
    if (imgui_window_get_dock(ctx, window_id) != dock_id) return IMGUI_FALSE;
    id = imgui_hash_bytes(&window_id, sizeof(window_id),
                          imgui_id_seed(ctx));
    size = imgui_label_size(ctx, label != NULL ? label : "");
    if (!imgui_item_register(ctx, id, size)) return IMGUI_FALSE;
    color = imgui_dock_get_active_window(ctx, dock_id) == window_id ?
        ctx->style.color_button_active :
        (ctx->last_item_hovered ? ctx->style.color_button_hovered :
                                  ctx->style.color_button);
    if (ctx->style.frame_rounding > 0.0f) {
        (void)imgui_draw_list_add_rect_rounded(
            ctx, &ctx->default_draw_list, ctx->last_item_rect,
            ctx->style.frame_rounding, color, 4, NULL);
    } else {
        (void)imgui_mesh_add_rect(ctx, ctx->last_item_rect, color);
    }
    imgui_draw_frame_border(ctx, ctx->last_item_rect);
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        const char *hidden = strstr(label, "##");
        imgui_text_draw_font(ctx, label, hidden != NULL ? hidden :
                             label + strlen(label), imgui_make_vec2(
                                 ctx->last_item_rect.x1 +
                                 ctx->style.frame_padding.x,
                                 ctx->last_item_rect.y1 +
                                 ctx->style.frame_padding.y));
    }
    if (ctx->last_item_clicked) {
        (void)imgui_dock_activate(ctx, dock_id, window_id);
    }
    return ctx->last_item_clicked;
}

imgui_result imgui_dock_tab_bar(imgui_context *ctx, imgui_id dock_id)
{
    int index;
    imgui_bool first;
    if (ctx == NULL || dock_id == 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (!imgui_internal_require_building(ctx,
                                         "dock tab bar outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (imgui_dock_find_or_create(ctx, dock_id) < 0) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    first = IMGUI_TRUE;
    for (index = 0; index < ctx->window_count; ++index) {
        if (ctx->windows[index].dock_id == dock_id) {
            if (!first) imgui_same_line(ctx);
            (void)imgui_dock_tab_button(ctx, dock_id,
                                        ctx->windows[index].id,
                                        ctx->windows[index].title);
            first = IMGUI_FALSE;
        }
    }
    return IMGUI_RESULT_OK;
}

imgui_scope imgui_window_begin(imgui_context *ctx, const char *title)
{
    imgui_window_desc desc;
    imgui_window_desc_init(&desc, title);
    return imgui_window_begin_ex(ctx, &desc);
}

imgui_scope imgui_window_begin_ex(imgui_context *ctx,
                                  const imgui_window_desc *desc)
{
    imgui_window_desc normalized_desc;
    imgui_bool active;
    imgui_bool visible;
    imgui_u32 window_background_color;
    imgui_rect window_rect;
    int window_index;
    imgui_internal_window *window;
    imgui_internal_window_scope_state *saved_window;
    window = NULL;
    if (!imgui_window_desc_normalize(desc, &normalized_desc)) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_ARGUMENT,
                              "window descriptor is invalid");
        return IMGUI_SCOPE_ERROR;
    }
    desc = &normalized_desc;
    if (ctx != NULL && imgui_internal_require_building(ctx,
                                                        "window begin outside frame")) {
        if (ctx->scope_depth > 0) {
            if (imgui_scope_reserve(ctx, ctx->scope_depth + 1) !=
                IMGUI_RESULT_OK) {
                imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                                      "window scope storage allocation failed");
                return IMGUI_SCOPE_ERROR;
            }
            saved_window = &ctx->scope_saved_window[ctx->scope_depth];
            saved_window->window_index = ctx->current_window_index;
            saved_window->origin = ctx->window_origin;
            saved_window->size = ctx->window_size;
            saved_window->content_max = imgui_make_vec2(
                ctx->content_max_x, ctx->content_max_y);
            saved_window->cursor = ctx->cursor;
            saved_window->clip = ctx->clip_rect;
            saved_window->flags = ctx->window_flags;
            saved_window->active = ctx->window_active;
            saved_window->focused = ctx->window_focused;
            saved_window->indent_width = ctx->indent_width;
            saved_window->item_spacing = ctx->item_spacing;
            saved_window->line_spacing_override =
                ctx->line_spacing_override;
            saved_window->line_spacing_override_valid =
                ctx->line_spacing_override_valid;
            saved_window->child_current_index = ctx->child_current_index;
            saved_window->child_flags = ctx->child_flags;
            saved_window->window_size_min = ctx->window_size_min;
            saved_window->window_size_max = ctx->window_size_max;
            saved_window->window_size_constraints_valid =
                ctx->window_size_constraints_valid;
            saved_window->scroll_y = ctx->current_window_index >= 0 &&
                ctx->current_window_index < ctx->window_count ?
                ctx->windows[ctx->current_window_index].scroll_y : 0.0f;
            saved_window->scroll_x = ctx->current_window_index >= 0 &&
                ctx->current_window_index < ctx->window_count ?
                ctx->windows[ctx->current_window_index].scroll_x : 0.0f;
        }
        window_index = imgui_window_find_or_create(ctx, desc);
        if (window_index < 0) return IMGUI_SCOPE_ERROR;
        window = &ctx->windows[window_index];
        window->hidden_this_frame = window->appearing;
        window->appearing = IMGUI_FALSE;
        if (desc != NULL && desc->title != NULL &&
            imgui_window_store_title(ctx, window, desc->title) !=
            IMGUI_RESULT_OK) {
            return IMGUI_SCOPE_ERROR;
        }
        if (desc != NULL && desc->struct_size >= sizeof(*desc) &&
            desc->has_position) window->position = desc->position;
        if (desc != NULL && desc->struct_size >= sizeof(*desc) &&
            desc->has_size && desc->size.x > 0.0f && desc->size.y > 0.0f) {
            window->size = desc->size;
            window->auto_fit_pending = IMGUI_FALSE;
        }
        window->flags = desc != NULL ? desc->flags : IMGUI_WINDOW_NONE;
        window->open = desc == NULL || desc->open == NULL || *desc->open;
        if (ctx->next_window_viewport_valid) {
            window->viewport_id = ctx->next_window_viewport_id;
            ctx->next_window_viewport_valid = IMGUI_FALSE;
        }
        if (ctx->next_window_position_valid) {
            window->position = ctx->next_window_position;
        }
        if (ctx->next_window_size_valid &&
            ctx->next_window_size.x > 0.0f &&
            ctx->next_window_size.y > 0.0f) {
            window->size = ctx->next_window_size;
            window->auto_fit_pending = IMGUI_FALSE;
            ctx->next_window_size_valid = IMGUI_FALSE;
        }
        ctx->window_size_constraints_valid =
            ctx->next_window_size_constraints_valid;
        if (ctx->window_size_constraints_valid) {
            float minimum_x = ctx->next_window_size_min.x;
            float minimum_y = ctx->next_window_size_min.y;
            float maximum_x = ctx->next_window_size_max.x;
            float maximum_y = ctx->next_window_size_max.y;
            ctx->window_size_min = ctx->next_window_size_min;
            ctx->window_size_max = ctx->next_window_size_max;
            if (window->size.x < minimum_x) window->size.x = minimum_x;
            if (window->size.y < minimum_y) window->size.y = minimum_y;
            if (maximum_x >= 0.0f && window->size.x > maximum_x) {
                window->size.x = maximum_x;
            }
            if (maximum_y >= 0.0f && window->size.y > maximum_y) {
                window->size.y = maximum_y;
            }
            ctx->next_window_size_constraints_valid = IMGUI_FALSE;
        }
        if (ctx->next_window_scroll_valid) {
            window->scroll_x = ctx->next_window_scroll.x;
            window->scroll_y = ctx->next_window_scroll.y;
            ctx->next_window_scroll_valid = IMGUI_FALSE;
        }
        if (ctx->next_window_position_valid) {
            if (ctx->next_window_position_pivot_valid) {
                window->position.x -= window->size.x *
                    ctx->next_window_position_pivot.x;
                window->position.y -= window->size.y *
                    ctx->next_window_position_pivot.y;
            }
            ctx->next_window_position_valid = IMGUI_FALSE;
            ctx->next_window_position_pivot_valid = IMGUI_FALSE;
        }
        if (ctx->next_window_collapsed_valid) {
            if ((window->flags & IMGUI_WINDOW_NO_COLLAPSE) == 0) {
                if (ctx->next_window_collapsed && !window->collapsed) {
                    window->expanded_size = window->size;
                    window->collapsed = IMGUI_TRUE;
                    window->size.y = 20.0f;
                } else if (!ctx->next_window_collapsed && window->collapsed) {
                    window->collapsed = IMGUI_FALSE;
                    if (window->expanded_size.x > 0.0f &&
                        window->expanded_size.y > 20.0f) {
                        window->size = window->expanded_size;
                    }
                }
            }
            ctx->next_window_collapsed_valid = IMGUI_FALSE;
        }
        if (ctx->next_window_focus_valid) {
            if (window->open && !window->collapsed) {
                ctx->focused_window_id = window->id;
                ctx->focused_window_valid = IMGUI_TRUE;
                window->z_order = ++ctx->next_window_z_order;
                ctx->window_z_order_dirty = IMGUI_TRUE;
            }
            ctx->next_window_focus_valid = IMGUI_FALSE;
        }
        if (window->hidden_this_frame && window->auto_fit_pending &&
            (desc == NULL || desc->struct_size < sizeof(*desc) ||
             !desc->has_size)) {
            /* Dear ImGui exposes the minimum title-bar-sized rectangle while
               suppressing a first-use window's draw data; its measured content
               size becomes visible on the following frame. */
            window->size = imgui_make_vec2(32.0f, 35.0f);
        }
        if (window->dock_id != 0) {
            int dock_index = imgui_dock_find_or_create(ctx, window->dock_id);
            if (dock_index >= 0 && ctx->dock_nodes[dock_index].initialized) {
                window->position = ctx->dock_nodes[dock_index].position;
                window->size = ctx->dock_nodes[dock_index].size;
            }
        }
        /* A caller-owned open flag gets the same title-bar close behavior as
           Dear ImGui's bool* Begin() overload.  Treat the close hit before
           movement/resize handling so a click cannot also start a drag. */
        if (window->open && desc != NULL && desc->open != NULL &&
            (window->flags & IMGUI_WINDOW_NO_TITLE_BAR) == 0 &&
            ctx->mouse_released[IMGUI_MOUSE_BUTTON_LEFT] &&
            ctx->input.mouse_x >= window->position.x + window->size.x - 20.0f &&
            ctx->input.mouse_x < window->position.x + window->size.x &&
            ctx->input.mouse_y >= window->position.y &&
            ctx->input.mouse_y < window->position.y + 20.0f) {
            *desc->open = IMGUI_FALSE;
            window->open = IMGUI_FALSE;
        }
        if (window->dock_id != 0) {
            int active_dock_index;
            active_dock_index = imgui_dock_find_or_create(ctx,
                                                           window->dock_id);
            if (active_dock_index >= 0 &&
                ctx->dock_nodes[active_dock_index].active_window_id == 0) {
                ctx->dock_nodes[active_dock_index].active_window_id = window->id;
            }
        }
        if (window->open && window->dock_id == 0 &&
            (window->flags & IMGUI_WINDOW_NO_COLLAPSE) == 0 &&
            (window->flags & IMGUI_WINDOW_NO_TITLE_BAR) == 0 &&
            ctx->mouse_double_clicked[IMGUI_MOUSE_BUTTON_LEFT] &&
            ctx->input.mouse_x >= window->position.x &&
            ctx->input.mouse_x < window->position.x + window->size.x &&
            ctx->input.mouse_y >= window->position.y &&
            ctx->input.mouse_y < window->position.y + 20.0f) {
            if (window->collapsed) {
                window->collapsed = IMGUI_FALSE;
                window->size = window->expanded_size;
            } else {
                window->expanded_size = window->size;
                window->collapsed = IMGUI_TRUE;
                window->size.y = 20.0f;
            }
        }
        if (window->open && window->dock_id == 0 && ctx->input.focused &&
            (window->flags & IMGUI_WINDOW_NO_MOUSE_INPUTS) == 0) {
            imgui_bool inside_title;
            imgui_bool inside_resize;
            inside_title = (window->flags & IMGUI_WINDOW_NO_TITLE_BAR) == 0 &&
                ctx->input.mouse_x >= window->position.x &&
                ctx->input.mouse_x < window->position.x + window->size.x &&
                ctx->input.mouse_y >= window->position.y &&
                ctx->input.mouse_y < window->position.y + 20.0f;
            inside_resize = ctx->input.mouse_x >= window->position.x +
                window->size.x - 14.0f &&
                ctx->input.mouse_x < window->position.x + window->size.x &&
                ctx->input.mouse_y >= window->position.y + window->size.y -
                14.0f && ctx->input.mouse_y < window->position.y +
                window->size.y;
            if (ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT]) {
                if (inside_resize && (window->flags & IMGUI_WINDOW_NO_RESIZE) == 0) {
                    ctx->moving_window_id = window->id;
                    ctx->moving_window_valid = IMGUI_TRUE;
                    ctx->resizing_window = IMGUI_TRUE;
                    ctx->window_drag_mouse = imgui_make_vec2(
                        ctx->input.mouse_x, ctx->input.mouse_y);
                    ctx->window_drag_position = window->position;
                    ctx->window_drag_size = window->size;
                    ctx->window_drag_moved = IMGUI_FALSE;
                } else if (inside_title && (window->flags & IMGUI_WINDOW_NO_MOVE) == 0) {
                    ctx->moving_window_id = window->id;
                    ctx->moving_window_valid = IMGUI_TRUE;
                    ctx->resizing_window = IMGUI_FALSE;
                    ctx->window_drag_mouse = imgui_make_vec2(
                        ctx->input.mouse_x, ctx->input.mouse_y);
                    ctx->window_drag_position = window->position;
                    ctx->window_drag_size = window->size;
                    ctx->window_drag_moved = IMGUI_FALSE;
                }
            }
            if (ctx->moving_window_valid && ctx->moving_window_id == window->id) {
                if (ctx->input.mouse_down[IMGUI_MOUSE_BUTTON_LEFT]) {
                    float dx = ctx->input.mouse_x - ctx->window_drag_mouse.x;
                    float dy = ctx->input.mouse_y - ctx->window_drag_mouse.y;
                    if (!ctx->resizing_window && !ctx->window_drag_moved) {
                        float threshold = ctx->mouse_drag_threshold > 0.0f ?
                                          ctx->mouse_drag_threshold : 0.0f;
                        if (dx * dx + dy * dy >= threshold * threshold) {
                            ctx->window_drag_moved = IMGUI_TRUE;
                        }
                    }
                    if (ctx->resizing_window) {
                        window->size.x = ctx->window_drag_size.x + dx;
                        window->size.y = ctx->window_drag_size.y + dy;
                        if (window->size.x < 64.0f) window->size.x = 64.0f;
                        if (window->size.y < 32.0f) window->size.y = 32.0f;
                    } else if (ctx->window_drag_moved) {
                        window->position.x = ctx->window_drag_position.x + dx;
                        window->position.y = ctx->window_drag_position.y + dy;
                    }
                } else {
                    if (!ctx->resizing_window && ctx->window_drag_moved) {
                        imgui_window_try_dock(ctx, window);
                    }
                    ctx->moving_window_valid = IMGUI_FALSE;
                    ctx->resizing_window = IMGUI_FALSE;
                    ctx->window_drag_moved = IMGUI_FALSE;
                }
            }
        }
        ctx->current_window_index = window_index;
        ctx->last_window_index = window_index;
        if (ctx->scope_depth == 0 ||
            ctx->scopes[ctx->scope_depth - 1] != IMGUI_INTERNAL_SCOPE_CHILD) {
            ctx->child_current_index = -1;
        }
        if (ctx->scrollbar_drag_active && !ctx->scrollbar_drag_child &&
            ctx->scrollbar_drag_window_id == window->id) {
            if (ctx->input.mouse_down[IMGUI_MOUSE_BUTTON_LEFT]) {
                imgui_rect scrollbar_track;
                imgui_rect scrollbar_grab;
                if ((ctx->scrollbar_drag_horizontal &&
                     imgui_window_hscrollbar_geometry(
                         ctx, window, &scrollbar_track, &scrollbar_grab)) ||
                    (!ctx->scrollbar_drag_horizontal &&
                     imgui_window_scrollbar_geometry(
                         ctx, window, &scrollbar_track, &scrollbar_grab))) {
                    float grab_range = ctx->scrollbar_drag_horizontal ?
                        scrollbar_track.x2 - scrollbar_track.x1 -
                        (scrollbar_grab.x2 - scrollbar_grab.x1) :
                        scrollbar_track.y2 - scrollbar_track.y1 -
                        (scrollbar_grab.y2 - scrollbar_grab.y1);
                    if (grab_range > 0.0f) {
                        float mouse_delta = ctx->scrollbar_drag_horizontal ?
                            ctx->input.mouse_x -
                            ctx->scrollbar_drag_mouse_start :
                            ctx->input.mouse_y -
                            ctx->scrollbar_drag_mouse_start;
                        if (ctx->scrollbar_drag_horizontal) {
                            window->scroll_x =
                                ctx->scrollbar_drag_scroll_start + mouse_delta *
                                window->scroll_max_x / grab_range;
                            if (window->scroll_x < 0.0f) window->scroll_x = 0.0f;
                            if (window->scroll_x > window->scroll_max_x) {
                                window->scroll_x = window->scroll_max_x;
                            }
                        } else {
                            window->scroll_y =
                            ctx->scrollbar_drag_scroll_start +
                                mouse_delta * window->scroll_max_y / grab_range;
                            if (window->scroll_y < 0.0f) window->scroll_y = 0.0f;
                            if (window->scroll_y > window->scroll_max_y) {
                                window->scroll_y = window->scroll_max_y;
                            }
                        }
                    }
                }
                ctx->input.mouse_wheel_x = 0.0f;
                ctx->input.mouse_wheel_y = 0.0f;
            } else {
                ctx->scrollbar_drag_active = IMGUI_FALSE;
                ctx->scrollbar_drag_window_id = 0;
            }
        }
        if (!ctx->scrollbar_drag_active &&
            ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT] &&
            (window->scroll_max_y > 0.0f || window->scroll_max_x > 0.0f)) {
            imgui_rect scrollbar_track;
            imgui_rect scrollbar_grab;
            if (imgui_window_scrollbar_geometry(
                    ctx, window, &scrollbar_track, &scrollbar_grab) &&
                ctx->input.mouse_x >= scrollbar_grab.x1 &&
                ctx->input.mouse_x < scrollbar_grab.x2 &&
                ctx->input.mouse_y >= scrollbar_grab.y1 &&
                ctx->input.mouse_y < scrollbar_grab.y2) {
                ctx->scrollbar_drag_active = IMGUI_TRUE;
                ctx->scrollbar_drag_child = IMGUI_FALSE;
                ctx->scrollbar_drag_window_id = window->id;
                ctx->scrollbar_drag_mouse_start = ctx->input.mouse_y;
                ctx->scrollbar_drag_scroll_start = window->scroll_y;
                ctx->scrollbar_drag_horizontal = IMGUI_FALSE;
            } else if (imgui_window_hscrollbar_geometry(
                           ctx, window, &scrollbar_track, &scrollbar_grab) &&
                       ctx->input.mouse_x >= scrollbar_grab.x1 &&
                       ctx->input.mouse_x < scrollbar_grab.x2 &&
                       ctx->input.mouse_y >= scrollbar_grab.y1 &&
                       ctx->input.mouse_y < scrollbar_grab.y2) {
                ctx->scrollbar_drag_active = IMGUI_TRUE;
                ctx->scrollbar_drag_child = IMGUI_FALSE;
                ctx->scrollbar_drag_window_id = window->id;
                ctx->scrollbar_drag_mouse_start = ctx->input.mouse_x;
                ctx->scrollbar_drag_scroll_start = window->scroll_x;
                ctx->scrollbar_drag_horizontal = IMGUI_TRUE;
            }
        }
        if (window->open && ctx->child_current_index < 0 &&
            (window->flags & IMGUI_WINDOW_NO_SCROLLBAR) == 0 &&
            ctx->input.mouse_x >= window->position.x &&
            ctx->input.mouse_x < window->position.x + window->size.x &&
            ctx->input.mouse_y >= window->position.y &&
            ctx->input.mouse_y < window->position.y + window->size.y &&
            (window->flags & IMGUI_WINDOW_NO_SCROLL_WITH_MOUSE) == 0 &&
            window->scroll_max_y > 0.0f &&
            ctx->input.mouse_wheel_y != 0.0f) {
            window->scroll_y += ctx->input.mouse_wheel_y * 20.0f;
            if (window->scroll_y < 0.0f) window->scroll_y = 0.0f;
            if (window->scroll_y > window->scroll_max_y) {
                window->scroll_y = window->scroll_max_y;
            }
            ctx->input.mouse_wheel_x = 0.0f;
            ctx->input.mouse_wheel_y = 0.0f;
        }
        if (window->open && ctx->child_current_index < 0 &&
            (window->flags & (IMGUI_WINDOW_HORIZONTAL_SCROLLBAR |
                              IMGUI_WINDOW_ALWAYS_HORIZONTAL_SCROLLBAR)) != 0 &&
            (window->flags & IMGUI_WINDOW_NO_SCROLLBAR) == 0 &&
            ctx->input.mouse_x >= window->position.x &&
            ctx->input.mouse_x < window->position.x + window->size.x &&
            ctx->input.mouse_y >= window->position.y &&
            ctx->input.mouse_y < window->position.y + window->size.y &&
            (window->flags & IMGUI_WINDOW_NO_SCROLL_WITH_MOUSE) == 0 &&
            window->scroll_max_x > 0.0f &&
            ctx->input.mouse_wheel_x != 0.0f) {
            window->scroll_x += ctx->input.mouse_wheel_x * 20.0f;
            if (window->scroll_x < 0.0f) window->scroll_x = 0.0f;
            if (window->scroll_x > window->scroll_max_x) {
                window->scroll_x = window->scroll_max_x;
            }
            ctx->input.mouse_wheel_x = 0.0f;
        }
        window->command_start = ctx->command_count;
        ctx->window_origin = window->position;
        ctx->window_size = window->size;
        ctx->clip_rect.x1 = ctx->window_origin.x;
        ctx->clip_rect.y1 = ctx->window_origin.y;
        ctx->clip_rect.x2 = ctx->window_origin.x + ctx->window_size.x;
        ctx->clip_rect.y2 = ctx->window_origin.y + ctx->window_size.y;
        ctx->cursor = imgui_make_vec2(
            ctx->window_origin.x + ctx->style.window_padding.x,
            ctx->window_origin.y + ctx->style.window_padding.y);
        if ((window->flags & IMGUI_WINDOW_NO_TITLE_BAR) == 0) {
            ctx->cursor.y += 20.0f;
        }
        ctx->cursor.x -= window->scroll_x;
        ctx->cursor.y -= window->scroll_y;
        ctx->content_max_x = ctx->cursor.x;
        ctx->content_max_y = ctx->cursor.y;
        if (ctx->next_window_content_size_valid) {
            /* A non-negative component is an explicit desired content
               extent; -1 preserves Dear ImGui's automatic extent behavior.
               Widgets may still grow the extent beyond this floor. */
            if (ctx->next_window_content_size.x >= 0.0f) {
                float requested_x = ctx->cursor.x +
                    ctx->next_window_content_size.x;
                if (requested_x > ctx->content_max_x) {
                    ctx->content_max_x = requested_x;
                }
            }
            if (ctx->next_window_content_size.y >= 0.0f) {
                float requested_y = ctx->cursor.y +
                    ctx->next_window_content_size.y;
                if (requested_y > ctx->content_max_y) {
                    ctx->content_max_y = requested_y;
                }
            }
            ctx->next_window_content_size_valid = IMGUI_FALSE;
        }
        ctx->item_spacing = ctx->style.item_spacing;
        ctx->indent_width = 0.0f;
    }
    active = window != NULL && window->open && !window->collapsed;
    visible = window != NULL && window->open;
    if (active && window->dock_id != 0) {
        int active_dock_index;
        active_dock_index = imgui_dock_find_or_create(ctx, window->dock_id);
        if (active_dock_index >= 0 &&
            ctx->dock_nodes[active_dock_index].active_window_id != 0 &&
            ctx->dock_nodes[active_dock_index].active_window_id != window->id) {
            active = IMGUI_FALSE;
        }
    }
    if (visible && window->dock_id != 0) {
        int visible_dock_index;
        visible_dock_index = imgui_dock_find_or_create(ctx,
                                                        window->dock_id);
        if (visible_dock_index >= 0 &&
            ctx->dock_nodes[visible_dock_index].active_window_id != 0 &&
            ctx->dock_nodes[visible_dock_index].active_window_id !=
            window->id) {
            visible = IMGUI_FALSE;
        }
    }
    if (desc != NULL && desc->open != NULL && !*desc->open) {
        active = IMGUI_FALSE;
        visible = IMGUI_FALSE;
    }
    if (ctx != NULL) {
        window_background_color = ctx->style.color_window_background;
        if (ctx->next_window_background_alpha_valid) {
            window_background_color = imgui_color_override_alpha(
                window_background_color,
                ctx->next_window_background_alpha);
            ctx->next_window_background_alpha_valid = IMGUI_FALSE;
        }
        ctx->window_active = active;
        if (active && !ctx->focused_window_valid &&
            (window->flags & IMGUI_WINDOW_NO_FOCUS_ON_APPEARING) == 0) {
            ctx->focused_window_id = window->id;
            ctx->focused_window_valid = IMGUI_TRUE;
        }
        if (active && ctx->input.mouse_x >= ctx->window_origin.x &&
            ctx->input.mouse_x < ctx->window_origin.x + ctx->window_size.x &&
            ctx->input.mouse_y >= ctx->window_origin.y &&
            ctx->input.mouse_y < ctx->window_origin.y + ctx->window_size.y &&
            ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT] &&
            (window->flags & IMGUI_WINDOW_NO_BRING_TO_FRONT_ON_FOCUS) == 0) {
            ctx->focused_window_id = window->id;
            ctx->focused_window_valid = IMGUI_TRUE;
            window->z_order = ++ctx->next_window_z_order;
            ctx->window_z_order_dirty = IMGUI_TRUE;
        }
        ctx->window_focused = active && ctx->input.focused &&
                             ctx->focused_window_valid &&
                             ctx->focused_window_id == window->id;
        if (active && ctx->input.focused &&
            (window->flags & IMGUI_WINDOW_NO_MOUSE_INPUTS) == 0 &&
            ctx->input.mouse_x >= ctx->window_origin.x &&
            ctx->input.mouse_x < ctx->window_origin.x + ctx->window_size.x &&
            ctx->input.mouse_y >= ctx->window_origin.y &&
            ctx->input.mouse_y < ctx->window_origin.y + ctx->window_size.y) {
            ctx->frame_any_window_hovered = IMGUI_TRUE;
        }
        if (ctx->window_focused) ctx->frame_any_window_focused = IMGUI_TRUE;
        ctx->window_flags = desc != NULL ? desc->flags : IMGUI_WINDOW_NONE;
        if (visible && window->open &&
            (ctx->window_flags & IMGUI_WINDOW_NO_BACKGROUND) == 0) {
            imgui_rect content_clip;
        imgui_u32 decoration_start;
        imgui_u32 decoration_index;
        imgui_u32 title_background_color;
            window_rect.x1 = ctx->window_origin.x;
            window_rect.y1 = ctx->window_origin.y;
            window_rect.x2 = ctx->window_origin.x + ctx->window_size.x;
            window_rect.y2 = ctx->window_origin.y + ctx->window_size.y;
            content_clip = ctx->clip_rect;
            title_background_color = window->collapsed ?
                ctx->style.color_window_title_background_collapsed :
                (ctx->window_focused ?
                 ctx->style.color_window_title_background_active :
                 ctx->style.color_window_title_background);
            ctx->clip_rect.x1 = 0.0f;
            ctx->clip_rect.y1 = 0.0f;
            ctx->clip_rect.x2 = ctx->frame_desc.display_size.x;
            ctx->clip_rect.y2 = ctx->frame_desc.display_size.y;
            decoration_start = ctx->command_count;
            {
                imgui_rect content_rect;
                imgui_rect title_rect;
                content_rect = window_rect;
                title_rect = window_rect;
                title_rect.y2 = title_rect.y1 + 19.0f;
                if ((ctx->window_flags & IMGUI_WINDOW_NO_TITLE_BAR) == 0) {
                    content_rect.y1 += 19.0f;
                }
                if (ctx->style.window_rounding > 0.0f) {
                    (void)imgui_draw_list_add_rect_rounded(
                        ctx, &ctx->default_draw_list, window_rect,
                        ctx->style.window_rounding,
                        window_background_color, 6, NULL);
                    if ((ctx->window_flags & IMGUI_WINDOW_NO_TITLE_BAR) == 0) {
                        (void)imgui_draw_list_add_rect_rounded(
                            ctx, &ctx->default_draw_list, title_rect,
                            ctx->style.window_rounding,
                            title_background_color,
                            6, NULL);
                    }
                    (void)imgui_mesh_add_window_border(
                        ctx, window_rect, ctx->style.color_window_border);
                } else {
                        (void)imgui_mesh_add_rect(
                            ctx, content_rect, window_background_color);
                }
                if (ctx->style.window_rounding == 0.0f) {
                    if ((ctx->window_flags & IMGUI_WINDOW_NO_TITLE_BAR) == 0) {
                        (void)imgui_mesh_add_rect(
                            ctx, title_rect,
                            title_background_color);
                        (void)imgui_mesh_add_window_border(
                            ctx, window_rect, ctx->style.color_window_border);
                    } else {
                        (void)imgui_mesh_add_window_border(
                            ctx, window_rect, ctx->style.color_window_border);
                    }
                }
                if ((ctx->window_flags & IMGUI_WINDOW_NO_TITLE_BAR) == 0) {
                    imgui_vec2 close_a;
                    imgui_vec2 close_b;
                    (void)imgui_mesh_add_window_menu_icon(
                        ctx, ctx->window_origin.x, ctx->window_origin.y,
                        ctx->style.color_window_title_text);
                    if (desc != NULL && desc->open != NULL) {
                        close_a = imgui_make_vec2(
                            ctx->window_origin.x + ctx->window_size.x -
                            14.0f, ctx->window_origin.y + 5.0f);
                        close_b = imgui_make_vec2(
                            ctx->window_origin.x + ctx->window_size.x -
                            6.0f, ctx->window_origin.y + 13.0f);
                        (void)imgui_draw_list_add_line(
                            ctx, &ctx->default_draw_list, close_a, close_b,
                            ctx->style.color_window_title_text, 1.0f);
                        close_a = imgui_make_vec2(
                            ctx->window_origin.x + ctx->window_size.x -
                            6.0f, ctx->window_origin.y + 5.0f);
                        close_b = imgui_make_vec2(
                            ctx->window_origin.x + ctx->window_size.x -
                            14.0f, ctx->window_origin.y + 13.0f);
                        (void)imgui_draw_list_add_line(
                            ctx, &ctx->default_draw_list, close_a, close_b,
                            ctx->style.color_window_title_text, 1.0f);
                    }
                }
            }
            if (ctx->font_texture != NULL) {
                for (decoration_index = decoration_start;
                     decoration_index < ctx->command_count;
                     ++decoration_index) {
                    if (ctx->commands[decoration_index].type ==
                        IMGUI_RENDER_COMMAND_DRAW_INDEXED) {
                        ctx->commands[decoration_index].data.draw_indexed
                            .texture = ctx->font_texture;
                    }
                }
                while (ctx->command_count > decoration_start + 1U) {
                    imgui_u32 old_count = ctx->command_count;
                    imgui_merge_last_text_command(ctx);
                    if (ctx->command_count == old_count) break;
                }
            }
            ctx->clip_rect = content_clip;
        }
        if (visible && window->open && desc != NULL && desc->title != NULL &&
            (ctx->window_flags & IMGUI_WINDOW_NO_TITLE_BAR) == 0 &&
            ctx->font != NULL && ctx->font_texture != NULL) {
            imgui_rect title_clip;
            const char *title_end;
            imgui_vec2 title_size;
            imgui_vec2 marker_center;
            imgui_u32 saved_title_color;
            title_clip = ctx->clip_rect;
            ctx->clip_rect.x1 = 0.0f;
            ctx->clip_rect.y1 = 0.0f;
            ctx->clip_rect.x2 = ctx->frame_desc.display_size.x;
            ctx->clip_rect.y2 = ctx->frame_desc.display_size.y;
            title_end = imgui_label_visible_end(desc->title);
            title_size = imgui_font_measure_text(ctx->font, desc->title,
                                                 title_end, 0.0f);
            saved_title_color = ctx->style.color_text;
            ctx->style.color_text = ctx->style.color_window_title_text;
            imgui_text_draw_font(ctx, desc->title,
                                 title_end,
                                 imgui_make_vec2(ctx->window_origin.x +
                                                 ctx->style.window_padding.x +
                                                 18.0f,
                                                 ctx->window_origin.y + 4.0f));
            ctx->style.color_text = saved_title_color;
            if ((ctx->window_flags & IMGUI_WINDOW_UNSAVED_DOCUMENT) != 0) {
                marker_center = imgui_make_vec2(
                    ctx->window_origin.x + ctx->style.window_padding.x +
                    18.0f + title_size.x + 5.0f,
                    ctx->window_origin.y + 10.0f);
                if (marker_center.x < ctx->window_origin.x +
                    ctx->window_size.x - 22.0f) {
                    (void)imgui_draw_list_add_circle(
                        ctx, &ctx->default_draw_list, marker_center, 2.5f,
                        ctx->style.color_window_title_text, 8);
                }
            }
            ctx->clip_rect = title_clip;
        }
        if (visible && window->open) {
            ctx->clip_rect.x1 = ctx->window_origin.x + 1.0f;
            ctx->clip_rect.y1 = ctx->window_origin.y +
                ((ctx->window_flags & IMGUI_WINDOW_NO_TITLE_BAR) != 0 ?
                 1.0f : 19.0f);
            ctx->clip_rect.x2 = ctx->window_origin.x + ctx->window_size.x - 1.0f;
            ctx->clip_rect.y2 = ctx->window_origin.y + ctx->window_size.y - 1.0f;
        }
    }
    {
        imgui_scope scope;
        scope = imgui_internal_scope_begin(
            ctx, IMGUI_INTERNAL_SCOPE_WINDOW, active);
        if (scope != IMGUI_SCOPE_ERROR) {
            imgui_push_id_value(ctx, window->id);
        }
        return scope;
    }
}

void imgui_window_end(imgui_context *ctx)
{
    imgui_internal_window_scope_state *saved_window;
    if (ctx != NULL && ctx->current_window_index >= 0 &&
        ctx->current_window_index < ctx->window_count) {
        if (!ctx->windows[ctx->current_window_index].collapsed &&
            ((ctx->window_flags & IMGUI_WINDOW_ALWAYS_AUTO_RESIZE) != 0 ||
             ctx->windows[ctx->current_window_index].auto_fit_pending)) {
            float content_width;
            float content_height;
            float minimum_height;
            content_width = ctx->content_max_x - ctx->window_origin.x +
                            ctx->style.window_padding.x;
            content_height = ctx->content_max_y - ctx->window_origin.y +
                             ctx->style.window_padding.y;
            if (ctx->font != NULL &&
                (ctx->window_flags & IMGUI_WINDOW_NO_TITLE_BAR) == 0 &&
                content_height > 0.0f) {
                /* Dear's text item height uses the snapped font ascent and
                   leaves the final half-pixel below the baseline outside
                   the auto-fit extent. */
                content_height -= 1.0f;
            }
            minimum_height = (ctx->window_flags & IMGUI_WINDOW_NO_TITLE_BAR) != 0 ?
                2.0f * ctx->style.window_padding.y :
                20.0f + 2.0f * ctx->style.window_padding.y;
            if (content_width < 64.0f) content_width = 64.0f;
            if (content_height < minimum_height) content_height = minimum_height;
            if (content_height < 32.0f) content_height = 32.0f;
            if (ctx->window_size_constraints_valid) {
                if (content_width < ctx->window_size_min.x) {
                    content_width = ctx->window_size_min.x;
                }
                if (content_height < ctx->window_size_min.y) {
                    content_height = ctx->window_size_min.y;
                }
                if (ctx->window_size_max.x >= 0.0f &&
                    content_width > ctx->window_size_max.x) {
                    content_width = ctx->window_size_max.x;
                }
                if (ctx->window_size_max.y >= 0.0f &&
                    content_height > ctx->window_size_max.y) {
                    content_height = ctx->window_size_max.y;
                }
            }
            ctx->window_size.x = content_width;
            ctx->window_size.y = content_height;
            ctx->windows[ctx->current_window_index].size = ctx->window_size;
            ctx->windows[ctx->current_window_index].auto_fit_pending =
                IMGUI_FALSE;
        }
        if (ctx->child_current_index < 0 &&
            (ctx->window_flags & IMGUI_WINDOW_NO_SCROLLBAR) == 0) {
            float content_bottom;
            float viewport_bottom;
            content_bottom = ctx->content_max_y +
                             ctx->windows[ctx->current_window_index].scroll_y;
            viewport_bottom = ctx->window_origin.y + ctx->window_size.y -
                              ctx->style.window_padding.y;
            /* Avoid manufacturing a scrollbar from the sub-pixel baseline
               remainder left by a font-metric-sized final item. */
            ctx->windows[ctx->current_window_index].scroll_max_y =
                content_bottom > viewport_bottom + 2.0f ?
                content_bottom - viewport_bottom : 0.0f;
            if (ctx->windows[ctx->current_window_index].scroll_y >
                ctx->windows[ctx->current_window_index].scroll_max_y) {
                ctx->windows[ctx->current_window_index].scroll_y =
                    ctx->windows[ctx->current_window_index].scroll_max_y;
            }
            if (ctx->window_active &&
                (ctx->windows[ctx->current_window_index].scroll_max_y > 0.0f ||
                 (ctx->window_flags &
                  IMGUI_WINDOW_ALWAYS_VERTICAL_SCROLLBAR) != 0)) {
                imgui_rect track;
                imgui_rect grab;
                if (imgui_window_scrollbar_geometry(
                        ctx, &ctx->windows[ctx->current_window_index],
                        &track, &grab)) {
                    imgui_bool grab_hovered =
                        ctx->input.mouse_x >= grab.x1 &&
                        ctx->input.mouse_x < grab.x2 &&
                        ctx->input.mouse_y >= grab.y1 &&
                        ctx->input.mouse_y < grab.y2;
                    imgui_u32 grab_color =
                        ctx->scrollbar_drag_active &&
                        ctx->scrollbar_drag_window_id ==
                        ctx->windows[ctx->current_window_index].id ?
                        ctx->style.color_scrollbar_grab_active :
                        (grab_hovered ?
                         ctx->style.color_scrollbar_grab_hovered :
                         ctx->style.color_scrollbar_grab);
                    (void)imgui_mesh_add_rect(
                        ctx, track, ctx->style.color_scrollbar_background);
                    (void)imgui_mesh_add_rect(ctx, grab, grab_color);
                }
            }
        }
        if (ctx->child_current_index < 0 &&
            (ctx->window_flags & (IMGUI_WINDOW_HORIZONTAL_SCROLLBAR |
                                  IMGUI_WINDOW_ALWAYS_HORIZONTAL_SCROLLBAR)) != 0 &&
            (ctx->window_flags & IMGUI_WINDOW_NO_SCROLLBAR) == 0) {
            float content_right;
            float viewport_right;
            content_right = ctx->content_max_x +
                            ctx->windows[ctx->current_window_index].scroll_x;
            viewport_right = ctx->window_origin.x + ctx->window_size.x -
                             ctx->style.window_padding.x;
            ctx->windows[ctx->current_window_index].scroll_max_x =
                content_right > viewport_right + 2.0f ?
                content_right - viewport_right : 0.0f;
            if (ctx->windows[ctx->current_window_index].scroll_x >
                ctx->windows[ctx->current_window_index].scroll_max_x) {
                ctx->windows[ctx->current_window_index].scroll_x =
                    ctx->windows[ctx->current_window_index].scroll_max_x;
            }
            if (ctx->window_active &&
                (ctx->windows[ctx->current_window_index].scroll_max_x > 0.0f ||
                 (ctx->window_flags &
                  IMGUI_WINDOW_ALWAYS_HORIZONTAL_SCROLLBAR) != 0)) {
                imgui_rect track;
                imgui_rect grab;
                if (imgui_window_hscrollbar_geometry(
                        ctx, &ctx->windows[ctx->current_window_index],
                        &track, &grab)) {
                    imgui_bool grab_hovered =
                        ctx->input.mouse_x >= grab.x1 &&
                        ctx->input.mouse_x < grab.x2 &&
                        ctx->input.mouse_y >= grab.y1 &&
                        ctx->input.mouse_y < grab.y2;
                    imgui_u32 grab_color =
                        ctx->scrollbar_drag_active &&
                        ctx->scrollbar_drag_horizontal &&
                        ctx->scrollbar_drag_window_id ==
                        ctx->windows[ctx->current_window_index].id ?
                        ctx->style.color_scrollbar_grab_active :
                        (grab_hovered ?
                         ctx->style.color_scrollbar_grab_hovered :
                         ctx->style.color_scrollbar_grab);
                    (void)imgui_mesh_add_rect(
                        ctx, track, ctx->style.color_scrollbar_background);
                    (void)imgui_mesh_add_rect(ctx, grab, grab_color);
                }
            }
        }
        if (ctx->window_active &&
            !ctx->windows[ctx->current_window_index].collapsed &&
            (ctx->window_flags & IMGUI_WINDOW_NO_RESIZE) == 0) {
            imgui_bool grip_hovered;
            imgui_bool grip_active;
            imgui_u32 grip_color;
            imgui_rect grip_rect;
            grip_rect.x1 = ctx->window_origin.x + ctx->window_size.x - 18.0f;
            grip_rect.y1 = ctx->window_origin.y + ctx->window_size.y - 18.0f;
            grip_rect.x2 = ctx->window_origin.x + ctx->window_size.x;
            grip_rect.y2 = ctx->window_origin.y + ctx->window_size.y;
            grip_hovered = ctx->input.mouse_x >= grip_rect.x1 &&
                           ctx->input.mouse_x < grip_rect.x2 &&
                           ctx->input.mouse_y >= grip_rect.y1 &&
                           ctx->input.mouse_y < grip_rect.y2;
            grip_active = ctx->resizing_window &&
                          ctx->moving_window_valid &&
                          ctx->moving_window_id ==
                          ctx->windows[ctx->current_window_index].id &&
                          ctx->input.mouse_down[IMGUI_MOUSE_BUTTON_LEFT];
            grip_color = grip_active ? ctx->style.color_resize_grip_active :
                         (grip_hovered ? ctx->style.color_resize_grip_hovered :
                                         ctx->style.color_resize_grip);
            if ((grip_color & 0xff000000UL) != 0) {
                int grip_line;
                for (grip_line = 0; grip_line < 3; ++grip_line) {
                    float offset = (float)grip_line * 4.0f;
                    (void)imgui_draw_list_add_line(
                        ctx, &ctx->default_draw_list,
                        imgui_make_vec2(grip_rect.x1 + 5.0f - offset,
                                        grip_rect.y2 - 2.0f),
                        imgui_make_vec2(grip_rect.x2 - 2.0f,
                                        grip_rect.y1 + 5.0f + offset),
                        grip_color, 1.0f);
                }
            }
        }
        ctx->windows[ctx->current_window_index].position = ctx->window_origin;
        ctx->windows[ctx->current_window_index].size = ctx->window_size;
        ctx->windows[ctx->current_window_index].command_end = ctx->command_count;
    }
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_WINDOW);
    if (ctx != NULL && ctx->scope_depth > 0 &&
        ctx->scope_saved_window[ctx->scope_depth].window_index >= 0) {
        saved_window = &ctx->scope_saved_window[ctx->scope_depth];
        ctx->current_window_index = saved_window->window_index;
        ctx->window_origin = saved_window->origin;
        ctx->window_size = saved_window->size;
        ctx->content_max_x = saved_window->content_max.x;
        ctx->content_max_y = saved_window->content_max.y;
        ctx->cursor = saved_window->cursor;
        ctx->clip_rect = saved_window->clip;
        ctx->window_flags = saved_window->flags;
        ctx->window_active = saved_window->active;
        ctx->window_focused = saved_window->focused;
        ctx->indent_width = saved_window->indent_width;
        ctx->item_spacing = saved_window->item_spacing;
        ctx->line_spacing_override = saved_window->line_spacing_override;
        ctx->line_spacing_override_valid =
            saved_window->line_spacing_override_valid;
        ctx->child_current_index = saved_window->child_current_index;
        ctx->child_flags = saved_window->child_flags;
        ctx->window_size_min = saved_window->window_size_min;
        ctx->window_size_max = saved_window->window_size_max;
        ctx->window_size_constraints_valid =
            saved_window->window_size_constraints_valid;
        if (ctx->current_window_index >= 0 &&
            ctx->current_window_index < ctx->window_count) {
            ctx->windows[ctx->current_window_index].scroll_y =
                saved_window->scroll_y;
            ctx->windows[ctx->current_window_index].scroll_x =
                saved_window->scroll_x;
        }
    } else if (ctx != NULL) {
        ctx->current_window_index = -1;
    }
}

static int imgui_viewport_find(const imgui_context *ctx, imgui_id viewport_id)
{
    int index;
    if (ctx == NULL) return -1;
    for (index = 0; index < ctx->viewport_count; ++index) {
        if (ctx->viewport_configs[index].configured &&
            ctx->viewport_configs[index].desc.viewport_id == viewport_id) {
            return index;
        }
    }
    return -1;
}

imgui_result imgui_viewport_configure(imgui_context *ctx,
                                      const imgui_viewport_desc *desc)
{
    imgui_viewport_desc local_desc;
    int index;
    imgui_bool is_new;
    imgui_result callback_result;
    if (ctx == NULL ||
        !imgui_viewport_desc_normalize(desc, &local_desc) ||
        local_desc.viewport_id == 0 || local_desc.size.x <= 0.0f ||
        local_desc.size.y <= 0.0f || local_desc.framebuffer_scale.x <= 0.0f ||
        local_desc.framebuffer_scale.y <= 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    desc = &local_desc;
    index = imgui_viewport_find(ctx, desc->viewport_id);
    is_new = index < 0 ? IMGUI_TRUE : IMGUI_FALSE;
    if (index < 0) {
        if (ctx->viewport_count >= IMGUI_INTERNAL_VIEWPORT_CAPACITY) {
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        index = ctx->viewport_count++;
    }
    if (desc->viewport_id != 0 && is_new &&
        ctx->platform.viewport_create != NULL) {
        callback_result = ctx->platform.viewport_create(
            desc, ctx->platform.user_data);
        if (callback_result != IMGUI_RESULT_OK) {
            --ctx->viewport_count;
            return IMGUI_RESULT_INVALID_STATE;
        }
        ctx->viewport_configs[index].platform_created = IMGUI_TRUE;
    }
    if (desc->viewport_id != 0 && !is_new &&
        ctx->viewport_configs[index].platform_created &&
        ctx->platform.viewport_update != NULL &&
        ctx->platform.viewport_update(desc, ctx->platform.user_data) !=
        IMGUI_RESULT_OK) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    ctx->viewport_configs[index].desc = *desc;
    ctx->viewport_configs[index].configured = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_viewport_destroy(imgui_context *ctx, imgui_id viewport_id)
{
    int index;
    int window_index;
    int move_index;
    imgui_render_list *removed_lists;
    if (ctx == NULL || viewport_id == 0) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (ctx->frame_state == IMGUI_INTERNAL_FRAME_BUILDING) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                              "viewport destroy during an active frame");
        return IMGUI_RESULT_INVALID_STATE;
    }
    index = imgui_viewport_find(ctx, viewport_id);
    if (index <= 0) {
        /* The primary viewport is owned by the context and cannot be
           destroyed through the secondary-viewport lifecycle API. */
        return index == 0 ? IMGUI_RESULT_INVALID_ARGUMENT :
                            IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->viewport_configs[index].platform_created &&
        ctx->platform.viewport_destroy != NULL) {
        ctx->platform.viewport_destroy(viewport_id, ctx->platform.user_data);
    }
    removed_lists = ctx->viewport_lists[index];
    for (window_index = 0; window_index < ctx->window_count; ++window_index) {
        if (ctx->windows[window_index].viewport_id == viewport_id) {
            ctx->windows[window_index].viewport_id = 0;
        }
    }
    /* Keep viewport indices dense so packet extraction and list ownership
       remain stable after a platform window disappears. */
    for (move_index = index; move_index + 1 < ctx->viewport_count;
         ++move_index) {
        ctx->viewport_configs[move_index] =
            ctx->viewport_configs[move_index + 1];
        ctx->viewport_lists[move_index] =
            ctx->viewport_lists[move_index + 1];
    }
    --ctx->viewport_count;
    ctx->viewport_lists[ctx->viewport_count] = NULL;
    imgui_internal_release(&ctx->allocator, removed_lists);
    memset(&ctx->viewport_configs[ctx->viewport_count], 0,
           sizeof(ctx->viewport_configs[ctx->viewport_count]));
    return IMGUI_RESULT_OK;
}

imgui_result imgui_window_set_viewport(imgui_context *ctx,
                                       imgui_id window_id,
                                       imgui_id viewport_id)
{
    int index;
    if (ctx == NULL || window_id == 0 ||
        imgui_viewport_find(ctx, viewport_id) < 0) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    for (index = 0; index < ctx->window_count; ++index) {
        if (ctx->windows[index].id == window_id) {
            ctx->windows[index].viewport_id = viewport_id;
            return IMGUI_RESULT_OK;
        }
    }
    return IMGUI_RESULT_INVALID_ARGUMENT;
}

imgui_id imgui_window_get_viewport(const imgui_context *ctx,
                                   imgui_id window_id)
{
    int index;
    if (ctx == NULL || window_id == 0) return 0;
    for (index = 0; index < ctx->window_count; ++index) {
        if (ctx->windows[index].id == window_id) {
            return ctx->windows[index].viewport_id;
        }
    }
    return 0;
}

static int imgui_child_find_or_create(imgui_context *ctx, imgui_id id)
{
    int index;
    if (ctx == NULL) return -1;
    for (index = 0; index < ctx->child_count; ++index) {
        if (ctx->child_ids[index] == id) return index;
    }
    if (ctx->child_count >= ctx->child_capacity) {
        int capacity;
        imgui_id *ids;
        float *scrolls;
        float *maximums;
        float *scroll_xs;
        float *maximum_xs;
        float *auto_widths;
        float *auto_heights;
        capacity = ctx->child_capacity > 0 ? ctx->child_capacity :
                   IMGUI_INTERNAL_CHILD_CAPACITY;
        if (capacity > INT_MAX / 2) capacity = INT_MAX;
        else capacity *= 2;
        if ((size_t)capacity > (size_t)-1 / sizeof(*ids) ||
            (size_t)capacity > (size_t)-1 / sizeof(*scrolls) ||
            (size_t)capacity > (size_t)-1 / sizeof(*scroll_xs)) return -1;
        ids = (imgui_id *)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*ids));
        scrolls = (float *)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*scrolls));
        maximums = (float *)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*maximums));
        scroll_xs = (float *)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*scroll_xs));
        maximum_xs = (float *)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*maximum_xs));
        auto_widths = (float *)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*auto_widths));
        auto_heights = (float *)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*auto_heights));
        if (ids == NULL || scrolls == NULL || maximums == NULL ||
            scroll_xs == NULL || maximum_xs == NULL ||
            auto_widths == NULL || auto_heights == NULL) {
            imgui_internal_release(&ctx->allocator, ids);
            imgui_internal_release(&ctx->allocator, scrolls);
            imgui_internal_release(&ctx->allocator, maximums);
            imgui_internal_release(&ctx->allocator, scroll_xs);
            imgui_internal_release(&ctx->allocator, maximum_xs);
            imgui_internal_release(&ctx->allocator, auto_widths);
            imgui_internal_release(&ctx->allocator, auto_heights);
            return -1;
        }
        if (ctx->child_count != 0) {
            memcpy(ids, ctx->child_ids,
                   (size_t)ctx->child_count * sizeof(*ids));
            memcpy(scrolls, ctx->child_scrolls,
                   (size_t)ctx->child_count * sizeof(*scrolls));
            memcpy(maximums, ctx->child_scroll_maxs,
                   (size_t)ctx->child_count * sizeof(*maximums));
            memcpy(scroll_xs, ctx->child_scroll_xs,
                   (size_t)ctx->child_count * sizeof(*scroll_xs));
            memcpy(maximum_xs, ctx->child_scroll_max_xs,
                   (size_t)ctx->child_count * sizeof(*maximum_xs));
            memcpy(auto_widths, ctx->child_auto_widths,
                   (size_t)ctx->child_count * sizeof(*auto_widths));
            memcpy(auto_heights, ctx->child_auto_heights,
                   (size_t)ctx->child_count * sizeof(*auto_heights));
        }
        imgui_internal_release(&ctx->allocator, ctx->child_ids);
        imgui_internal_release(&ctx->allocator, ctx->child_scrolls);
        imgui_internal_release(&ctx->allocator, ctx->child_scroll_maxs);
        imgui_internal_release(&ctx->allocator, ctx->child_scroll_xs);
        imgui_internal_release(&ctx->allocator, ctx->child_scroll_max_xs);
        imgui_internal_release(&ctx->allocator, ctx->child_auto_widths);
        imgui_internal_release(&ctx->allocator, ctx->child_auto_heights);
        ctx->child_ids = ids;
        ctx->child_scrolls = scrolls;
        ctx->child_scroll_maxs = maximums;
        ctx->child_scroll_xs = scroll_xs;
        ctx->child_scroll_max_xs = maximum_xs;
        ctx->child_auto_widths = auto_widths;
        ctx->child_auto_heights = auto_heights;
        ctx->child_capacity = capacity;
    }
    index = ctx->child_count++;
    ctx->child_ids[index] = id;
    ctx->child_scrolls[index] = 0.0f;
    ctx->child_scroll_maxs[index] = 0.0f;
    ctx->child_scroll_xs[index] = 0.0f;
    ctx->child_scroll_max_xs[index] = 0.0f;
    ctx->child_auto_widths[index] = 0.0f;
    ctx->child_auto_heights[index] = 0.0f;
    return index;
}

imgui_scope imgui_child_begin(imgui_context *ctx,
                              imgui_id id,
                              imgui_vec2 size,
                              imgui_flags flags)
{
    imgui_scope scope;
    imgui_rect child_rect;
    imgui_rect scrollbar_track;
    imgui_rect scrollbar_grab;
    float track_size;
    float grab_size;
    float grab_range;
    float padding_x;
    float padding_y;
    imgui_vec2 available;
    imgui_bool auto_resize_x;
    imgui_bool auto_resize_y;
    int child_index;
    if (ctx == NULL || !imgui_internal_require_building(
            ctx, "child begin outside frame") ||
        !imgui_float_is_finite(size.x) || !imgui_float_is_finite(size.y)) {
        return IMGUI_SCOPE_ERROR;
    }
    /* Match CalcItemSize(): zero fills the current content region unless
       that axis is auto-sized; a negative component subtracts from the
       available region.  The previous fixed 100-pixel fallback made common
       BeginChild(..., {0, 0}) layouts diverge substantially from Dear ImGui. */
    available = imgui_get_content_region_available(ctx);
    auto_resize_x = (flags & IMGUI_CHILD_AUTO_RESIZE_X) != 0 ?
                    IMGUI_TRUE : IMGUI_FALSE;
    auto_resize_y = (flags & IMGUI_CHILD_AUTO_RESIZE_Y) != 0 ?
                    IMGUI_TRUE : IMGUI_FALSE;
    if (size.x == 0.0f) size.x = auto_resize_x ? 4.0f : available.x;
    else if (size.x < 0.0f) size.x = available.x + size.x;
    if (size.y == 0.0f) size.y = auto_resize_y ? 4.0f : available.y;
    else if (size.y < 0.0f) size.y = available.y + size.y;
    if (size.x < 4.0f) size.x = 4.0f;
    if (size.y < 4.0f) size.y = 4.0f;
    child_index = imgui_child_find_or_create(ctx, id);
    if ((flags & IMGUI_CHILD_FRAME_STYLE) != 0) {
        padding_x = ctx->style.frame_padding.x;
        padding_y = ctx->style.frame_padding.y;
    } else if ((flags & (IMGUI_CHILD_BORDER |
                         IMGUI_CHILD_ALWAYS_USE_WINDOW_PADDING)) != 0) {
        padding_x = ctx->style.window_padding.x;
        padding_y = ctx->style.window_padding.y;
    } else {
        padding_x = 4.0f;
        padding_y = 4.0f;
    }
    if (child_index >= 0 &&
        (flags & IMGUI_CHILD_AUTO_RESIZE_X) != 0 &&
        ctx->child_auto_widths[child_index] > 0.0f) {
        size.x = ctx->child_auto_widths[child_index];
    }
    if (child_index >= 0 &&
        (flags & IMGUI_CHILD_AUTO_RESIZE_Y) != 0 &&
        ctx->child_auto_heights[child_index] > 0.0f) {
        size.y = ctx->child_auto_heights[child_index];
    }
    ctx->child_current_index = child_index;
    ctx->child_scroll_y = child_index >= 0 ?
        ctx->child_scrolls[child_index] : 0.0f;
    ctx->child_scroll_x = child_index >= 0 ?
        ctx->child_scroll_xs[child_index] : 0.0f;
    scope = imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_CHILD,
                                       IMGUI_TRUE);
    if (scope == IMGUI_SCOPE_ERROR) return scope;
    ctx->scope_saved_child_index[ctx->scope_depth - 1] =
        ctx->child_current_index;
    ctx->scope_saved_child_scroll[ctx->scope_depth - 1] =
        ctx->child_scroll_y;
    ctx->scope_saved_child_scroll_x[ctx->scope_depth - 1] =
        ctx->child_scroll_x;
    ctx->scope_saved_child_flags[ctx->scope_depth - 1] = ctx->child_flags;
    ctx->child_flags = flags;
    ctx->scope_saved_size[ctx->scope_depth - 1] = size;
    imgui_push_id_value(ctx, id);
    child_rect.x1 = ctx->cursor.x;
    child_rect.y1 = ctx->cursor.y;
    child_rect.x2 = child_rect.x1 + size.x;
    child_rect.y2 = child_rect.y1 + size.y;
    if ((flags & (IMGUI_CHILD_BORDER | IMGUI_CHILD_FRAME_STYLE)) != 0) {
        imgui_u32 background_color =
            (flags & IMGUI_CHILD_FRAME_STYLE) != 0 ?
            ctx->style.color_frame : ctx->style.color_child_background;
        if (ctx->style.child_rounding > 0.0f) {
            (void)imgui_draw_list_add_rect_rounded(
                ctx, &ctx->default_draw_list, child_rect,
                ctx->style.child_rounding, background_color, 6, NULL);
        } else {
            (void)imgui_mesh_add_rect(ctx, child_rect, background_color);
        }
    }
    if ((flags & IMGUI_CHILD_BORDER) != 0 &&
        ctx->style.child_border_size > 0.0f) {
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(child_rect.x1, child_rect.y1),
            imgui_make_vec2(child_rect.x2, child_rect.y1),
            ctx->style.color_window_border, ctx->style.child_border_size);
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(child_rect.x2, child_rect.y1),
            imgui_make_vec2(child_rect.x2, child_rect.y2),
            ctx->style.color_window_border, ctx->style.child_border_size);
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(child_rect.x2, child_rect.y2),
            imgui_make_vec2(child_rect.x1, child_rect.y2),
            ctx->style.color_window_border, ctx->style.child_border_size);
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(child_rect.x1, child_rect.y2),
            imgui_make_vec2(child_rect.x1, child_rect.y1),
            ctx->style.color_window_border, ctx->style.child_border_size);
    }
    if (ctx->scrollbar_drag_active && ctx->scrollbar_drag_child &&
        ctx->scrollbar_drag_window_id == id) {
        if (ctx->input.mouse_down[IMGUI_MOUSE_BUTTON_LEFT]) {
            if (ctx->scrollbar_drag_horizontal &&
                ctx->child_scroll_max_xs[child_index] > 0.0f) {
                scrollbar_track.x1 = child_rect.x1 + 1.0f;
                scrollbar_track.x2 = child_rect.x2 - 2.0f;
                track_size = scrollbar_track.x2 - scrollbar_track.x1;
                grab_size = track_size * track_size /
                    (track_size + ctx->child_scroll_max_xs[child_index]);
                if (grab_size < ctx->style.scrollbar_grab_min_size) {
                    grab_size = ctx->style.scrollbar_grab_min_size;
                }
                if (grab_size > track_size) grab_size = track_size;
                grab_range = track_size - grab_size;
                if (grab_range > 0.0f) {
                    ctx->child_scroll_x =
                        ctx->scrollbar_drag_scroll_start +
                        (ctx->input.mouse_x -
                         ctx->scrollbar_drag_mouse_start) *
                        ctx->child_scroll_max_xs[child_index] / grab_range;
                }
            } else if (!ctx->scrollbar_drag_horizontal &&
                       ctx->child_scroll_maxs[child_index] > 0.0f) {
                scrollbar_track.y1 = child_rect.y1 + 1.0f;
                scrollbar_track.y2 = child_rect.y2 - 2.0f;
                track_size = scrollbar_track.y2 - scrollbar_track.y1;
                grab_size = track_size * track_size /
                    (track_size + ctx->child_scroll_maxs[child_index]);
                if (grab_size < ctx->style.scrollbar_grab_min_size) {
                    grab_size = ctx->style.scrollbar_grab_min_size;
                }
                if (grab_size > track_size) grab_size = track_size;
                grab_range = track_size - grab_size;
                if (grab_range > 0.0f) {
                    ctx->child_scroll_y =
                        ctx->scrollbar_drag_scroll_start +
                        (ctx->input.mouse_y -
                         ctx->scrollbar_drag_mouse_start) *
                        ctx->child_scroll_maxs[child_index] / grab_range;
                }
            }
            if (ctx->child_scroll_x < 0.0f) ctx->child_scroll_x = 0.0f;
            if (ctx->child_scroll_y < 0.0f) ctx->child_scroll_y = 0.0f;
            if (ctx->child_scroll_x > ctx->child_scroll_max_xs[child_index])
                ctx->child_scroll_x = ctx->child_scroll_max_xs[child_index];
            if (ctx->child_scroll_y > ctx->child_scroll_maxs[child_index])
                ctx->child_scroll_y = ctx->child_scroll_maxs[child_index];
            ctx->child_scroll_xs[child_index] = ctx->child_scroll_x;
            ctx->child_scrolls[child_index] = ctx->child_scroll_y;
            ctx->input.mouse_wheel_x = 0.0f;
            ctx->input.mouse_wheel_y = 0.0f;
        } else {
            ctx->scrollbar_drag_active = IMGUI_FALSE;
            ctx->scrollbar_drag_child = IMGUI_FALSE;
            ctx->scrollbar_drag_window_id = 0;
        }
    }
    if (!ctx->scrollbar_drag_active &&
        ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT] && child_index >= 0) {
        scrollbar_track.x1 = child_rect.x1 + size.x -
            ctx->style.scrollbar_size;
        scrollbar_track.x2 = child_rect.x1 + size.x - 2.0f;
        scrollbar_track.y1 = child_rect.y1 + 1.0f;
        scrollbar_track.y2 = child_rect.y2 - 2.0f;
        track_size = scrollbar_track.y2 - scrollbar_track.y1;
        if (ctx->child_scroll_maxs[child_index] > 0.0f) {
            grab_size = track_size * track_size /
                (track_size + ctx->child_scroll_maxs[child_index]);
            if (grab_size < ctx->style.scrollbar_grab_min_size) {
                grab_size = ctx->style.scrollbar_grab_min_size;
            }
            if (grab_size > track_size) grab_size = track_size;
            grab_range = track_size - grab_size;
            scrollbar_grab = scrollbar_track;
            scrollbar_grab.y1 += grab_range * ctx->child_scroll_y /
                ctx->child_scroll_maxs[child_index];
            scrollbar_grab.y2 = scrollbar_grab.y1 + grab_size;
            if (ctx->input.mouse_x >= scrollbar_grab.x1 &&
                ctx->input.mouse_x < scrollbar_grab.x2 &&
                ctx->input.mouse_y >= scrollbar_grab.y1 &&
                ctx->input.mouse_y < scrollbar_grab.y2) {
                ctx->scrollbar_drag_active = IMGUI_TRUE;
                ctx->scrollbar_drag_child = IMGUI_TRUE;
                ctx->scrollbar_drag_horizontal = IMGUI_FALSE;
                ctx->scrollbar_drag_window_id = id;
                ctx->scrollbar_drag_mouse_start = ctx->input.mouse_y;
                ctx->scrollbar_drag_scroll_start = ctx->child_scroll_y;
            }
        }
        scrollbar_track.x1 = child_rect.x1 + 1.0f;
        scrollbar_track.x2 = child_rect.x2 - 2.0f;
        scrollbar_track.y1 = child_rect.y1 + size.y -
            ctx->style.scrollbar_size;
        scrollbar_track.y2 = child_rect.y1 + size.y - 2.0f;
        track_size = scrollbar_track.x2 - scrollbar_track.x1;
        if (!ctx->scrollbar_drag_active &&
            ((flags & IMGUI_WINDOW_HORIZONTAL_SCROLLBAR) != 0 ||
             (flags & IMGUI_CHILD_HORIZONTAL_SCROLLBAR) != 0) &&
            ctx->child_scroll_max_xs[child_index] > 0.0f) {
            grab_size = track_size * track_size /
                (track_size + ctx->child_scroll_max_xs[child_index]);
            if (grab_size < ctx->style.scrollbar_grab_min_size) {
                grab_size = ctx->style.scrollbar_grab_min_size;
            }
            if (grab_size > track_size) grab_size = track_size;
            grab_range = track_size - grab_size;
            scrollbar_grab = scrollbar_track;
            scrollbar_grab.x1 += grab_range * ctx->child_scroll_x /
                ctx->child_scroll_max_xs[child_index];
            scrollbar_grab.x2 = scrollbar_grab.x1 + grab_size;
            if (ctx->input.mouse_x >= scrollbar_grab.x1 &&
                ctx->input.mouse_x < scrollbar_grab.x2 &&
                ctx->input.mouse_y >= scrollbar_grab.y1 &&
                ctx->input.mouse_y < scrollbar_grab.y2) {
                ctx->scrollbar_drag_active = IMGUI_TRUE;
                ctx->scrollbar_drag_child = IMGUI_TRUE;
                ctx->scrollbar_drag_horizontal = IMGUI_TRUE;
                ctx->scrollbar_drag_window_id = id;
                ctx->scrollbar_drag_mouse_start = ctx->input.mouse_x;
                ctx->scrollbar_drag_scroll_start = ctx->child_scroll_x;
            }
        }
    }
    if (ctx->input.mouse_x >= child_rect.x1 &&
        ctx->input.mouse_x < child_rect.x2 &&
        ctx->input.mouse_y >= child_rect.y1 &&
        ctx->input.mouse_y < child_rect.y2) {
        ctx->child_scroll_y += ctx->input.mouse_wheel_y * 20.0f;
        if (ctx->child_scroll_y < 0.0f) ctx->child_scroll_y = 0.0f;
        if (child_index >= 0) ctx->child_scrolls[child_index] =
            ctx->child_scroll_y;
        if (ctx->input.mouse_wheel_x != 0.0f &&
            ((flags & IMGUI_WINDOW_HORIZONTAL_SCROLLBAR) != 0 ||
             (flags & IMGUI_CHILD_HORIZONTAL_SCROLLBAR) != 0)) {
            ctx->child_scroll_x += ctx->input.mouse_wheel_x * 20.0f;
            if (ctx->child_scroll_x < 0.0f) ctx->child_scroll_x = 0.0f;
            if (child_index >= 0) ctx->child_scroll_xs[child_index] =
                ctx->child_scroll_x;
        }
        ctx->input.mouse_wheel_x = 0.0f;
        ctx->input.mouse_wheel_y = 0.0f;
    } else if (ctx->input.mouse_wheel_x != 0.0f &&
               ((flags & IMGUI_WINDOW_HORIZONTAL_SCROLLBAR) != 0 ||
                (flags & IMGUI_CHILD_HORIZONTAL_SCROLLBAR) != 0)) {
        ctx->child_scroll_x += ctx->input.mouse_wheel_x * 20.0f;
        if (ctx->child_scroll_x < 0.0f) ctx->child_scroll_x = 0.0f;
        if (child_index >= 0) ctx->child_scroll_xs[child_index] =
            ctx->child_scroll_x;
        ctx->input.mouse_wheel_x = 0.0f;
    }
    if (child_rect.x1 < ctx->clip_rect.x1) child_rect.x1 = ctx->clip_rect.x1;
    if (child_rect.y1 < ctx->clip_rect.y1) child_rect.y1 = ctx->clip_rect.y1;
    if (child_rect.x2 > ctx->clip_rect.x2) child_rect.x2 = ctx->clip_rect.x2;
    if (child_rect.y2 > ctx->clip_rect.y2) child_rect.y2 = ctx->clip_rect.y2;
    ctx->clip_rect = child_rect;
    ctx->cursor.x += padding_x - ctx->child_scroll_x;
    ctx->cursor.y += padding_y - ctx->child_scroll_y;
    /* Child content measurement is local. Preserve the parent's accumulated
       extent in the otherwise-unused window-scope slot so nested children do
       not inherit a parent's wide content and auto-size incorrectly. */
    ctx->scope_saved_window[ctx->scope_depth - 1].content_max =
        imgui_make_vec2(ctx->content_max_x, ctx->content_max_y);
    ctx->content_max_x = ctx->cursor.x;
    ctx->content_max_y = ctx->cursor.y;
    return scope;
}

void imgui_child_end(imgui_context *ctx)
{
    int scope_index;
    int saved_child_index;
    float saved_child_scroll;
    float saved_child_scroll_x;
    imgui_flags saved_child_flags;
    float child_start_y;
    float child_height;
    float content_bottom;
    float maximum_scroll;
    float content_right;
    float maximum_scroll_x;
    imgui_rect scrollbar_track;
    imgui_rect scrollbar_grab;
    float track_width;
    float grab_width;
    float grab_range;
    float track_height;
    float grab_height;
    float padding_x;
    float padding_y;
    float desired_width;
    float desired_height;
    imgui_vec2 child_item_cursor;
    imgui_vec2 child_item_size;
    imgui_rect child_item_clip;
    imgui_rect parent_item_clip;
    imgui_id child_item_id;
    imgui_bool child_item_valid;
    child_item_cursor = imgui_make_vec2(0.0f, 0.0f);
    child_item_size = imgui_make_vec2(0.0f, 0.0f);
    memset(&child_item_clip, 0, sizeof(child_item_clip));
    memset(&parent_item_clip, 0, sizeof(parent_item_clip));
    child_item_id = 0;
    child_item_valid = IMGUI_FALSE;
    if (ctx != NULL && ctx->child_current_index >= 0 &&
        ctx->child_current_index < ctx->child_count) {
        scope_index = ctx->scope_depth - 1;
        child_start_y = scope_index >= 0 ?
            ctx->scope_saved_cursor[scope_index].y : ctx->cursor.y;
        child_height = scope_index >= 0 ?
            ctx->scope_saved_size[scope_index].y : 0.0f;
        if ((ctx->child_flags & IMGUI_CHILD_FRAME_STYLE) != 0) {
            padding_x = ctx->style.frame_padding.x;
            padding_y = ctx->style.frame_padding.y;
        } else if ((ctx->child_flags & (IMGUI_CHILD_BORDER |
                                         IMGUI_CHILD_ALWAYS_USE_WINDOW_PADDING)) != 0) {
            padding_x = ctx->style.window_padding.x;
            padding_y = ctx->style.window_padding.y;
        } else {
            padding_x = 4.0f;
            padding_y = 4.0f;
        }
        content_bottom = ctx->cursor.y + ctx->child_scroll_y;
        desired_width = ctx->content_max_x + ctx->child_scroll_x -
                        ctx->scope_saved_cursor[scope_index].x + padding_x;
        desired_height = content_bottom - child_start_y + padding_y;
        if (desired_width < padding_x * 2.0f) desired_width = padding_x * 2.0f;
        if (desired_height < padding_y * 2.0f) desired_height = padding_y * 2.0f;
        if ((ctx->child_flags & IMGUI_CHILD_AUTO_RESIZE_X) != 0 &&
            scope_index >= 0) {
            ctx->scope_saved_size[scope_index].x = desired_width;
            child_height = ctx->scope_saved_size[scope_index].y;
            ctx->child_auto_widths[ctx->child_current_index] = desired_width;
        }
        if ((ctx->child_flags & IMGUI_CHILD_AUTO_RESIZE_Y) != 0 &&
            scope_index >= 0) {
            ctx->scope_saved_size[scope_index].y = desired_height;
            child_height = desired_height;
            ctx->child_auto_heights[ctx->child_current_index] = desired_height;
        }
        maximum_scroll = content_bottom - (child_start_y + child_height - padding_y);
        if (maximum_scroll < 0.0f) maximum_scroll = 0.0f;
        if (ctx->child_scroll_y > maximum_scroll) {
            ctx->child_scroll_y = maximum_scroll;
        }
        ctx->child_scroll_maxs[ctx->child_current_index] = maximum_scroll;
        ctx->child_scrolls[ctx->child_current_index] = ctx->child_scroll_y;
        content_right = ctx->content_max_x + ctx->child_scroll_x;
        maximum_scroll_x = content_right -
            (ctx->scope_saved_cursor[scope_index].x +
             ctx->scope_saved_size[scope_index].x - padding_x);
        if (maximum_scroll_x < 0.0f) maximum_scroll_x = 0.0f;
        if (ctx->child_scroll_x > maximum_scroll_x) {
            ctx->child_scroll_x = maximum_scroll_x;
        }
        ctx->child_scroll_max_xs[ctx->child_current_index] =
            maximum_scroll_x;
        ctx->child_scroll_xs[ctx->child_current_index] = ctx->child_scroll_x;
        {
            imgui_vec2 parent_content_max =
                ctx->scope_saved_window[scope_index].content_max;
            float child_right =
                ctx->scope_saved_cursor[scope_index].x +
                ctx->scope_saved_size[scope_index].x;
            float child_bottom =
                ctx->scope_saved_cursor[scope_index].y +
                ctx->scope_saved_size[scope_index].y;
            ctx->content_max_x = parent_content_max.x > child_right ?
                parent_content_max.x : child_right;
            ctx->content_max_y = parent_content_max.y > child_bottom ?
                parent_content_max.y : child_bottom;
        }
        if (maximum_scroll > 0.0f && scope_index >= 0 &&
            (ctx->child_flags & (IMGUI_WINDOW_NO_SCROLLBAR |
                                 IMGUI_CHILD_NO_SCROLLBAR)) == 0) {
            scrollbar_track.x1 = ctx->scope_saved_cursor[scope_index].x +
                ctx->scope_saved_size[scope_index].x -
                ctx->style.scrollbar_size;
            scrollbar_track.x2 = ctx->scope_saved_cursor[scope_index].x +
                ctx->scope_saved_size[scope_index].x - 2.0f;
            scrollbar_track.y1 = ctx->scope_saved_cursor[scope_index].y + 1.0f;
            scrollbar_track.y2 = ctx->scope_saved_cursor[scope_index].y +
                ctx->scope_saved_size[scope_index].y - 2.0f;
            track_height = scrollbar_track.y2 - scrollbar_track.y1;
            grab_height = track_height * track_height /
                (track_height + maximum_scroll);
            if (grab_height < ctx->style.scrollbar_grab_min_size) {
                grab_height = ctx->style.scrollbar_grab_min_size;
            }
            if (grab_height > track_height) grab_height = track_height;
            grab_range = track_height - grab_height;
            scrollbar_grab = scrollbar_track;
            scrollbar_grab.y1 += grab_range * ctx->child_scroll_y /
                maximum_scroll;
            scrollbar_grab.y2 = scrollbar_grab.y1 + grab_height;
            (void)imgui_mesh_add_rect(
                ctx, scrollbar_track, ctx->style.color_scrollbar_background);
            (void)imgui_mesh_add_rect(
                ctx, scrollbar_grab, ctx->style.color_scrollbar_grab);
        }
        if (maximum_scroll_x > 0.0f && scope_index >= 0 &&
            (ctx->child_flags & (IMGUI_WINDOW_NO_SCROLLBAR |
                                 IMGUI_CHILD_NO_SCROLLBAR)) == 0 &&
            (ctx->child_flags & (IMGUI_WINDOW_HORIZONTAL_SCROLLBAR |
                                 IMGUI_CHILD_HORIZONTAL_SCROLLBAR)) != 0) {
            scrollbar_track.x1 = ctx->scope_saved_cursor[scope_index].x + 1.0f;
            scrollbar_track.x2 = ctx->scope_saved_cursor[scope_index].x +
                ctx->scope_saved_size[scope_index].x - 2.0f;
            scrollbar_track.y1 = ctx->scope_saved_cursor[scope_index].y +
                ctx->scope_saved_size[scope_index].y -
                ctx->style.scrollbar_size;
            scrollbar_track.y2 = ctx->scope_saved_cursor[scope_index].y +
                ctx->scope_saved_size[scope_index].y - 2.0f;
            track_width = scrollbar_track.x2 - scrollbar_track.x1;
            grab_width = track_width * track_width /
                (track_width + maximum_scroll_x);
            if (grab_width < ctx->style.scrollbar_grab_min_size) {
                grab_width = ctx->style.scrollbar_grab_min_size;
            }
            if (grab_width > track_width) grab_width = track_width;
            grab_range = track_width - grab_width;
            scrollbar_grab = scrollbar_track;
            scrollbar_grab.x1 += grab_range * ctx->child_scroll_x /
                maximum_scroll_x;
            scrollbar_grab.x2 = scrollbar_grab.x1 + grab_width;
            (void)imgui_mesh_add_rect(
                ctx, scrollbar_track, ctx->style.color_scrollbar_background);
            (void)imgui_mesh_add_rect(
                ctx, scrollbar_grab, ctx->style.color_scrollbar_grab);
        }
    }
    scope_index = ctx != NULL ? ctx->scope_depth - 1 : -1;
    saved_child_index = scope_index >= 0 ?
        ctx->scope_saved_child_index[scope_index] : -1;
    saved_child_scroll = scope_index >= 0 ?
        ctx->scope_saved_child_scroll[scope_index] : 0.0f;
    saved_child_scroll_x = scope_index >= 0 ?
        ctx->scope_saved_child_scroll_x[scope_index] : 0.0f;
    saved_child_flags = scope_index >= 0 ?
        ctx->scope_saved_child_flags[scope_index] : 0;
    if (scope_index >= 0) {
        child_item_cursor = ctx->scope_saved_cursor[scope_index];
        child_item_size = ctx->scope_saved_size[scope_index];
        /* The child item is clipped by the parent's clip rectangle; the
           current clip may already be the child rectangle intersected with
           it, which would incorrectly report an entirely clipped child as
           visible. */
        child_item_clip = ctx->scope_saved_clip[scope_index];
        if (saved_child_index >= 0 &&
            saved_child_index < ctx->child_count) {
            child_item_id = ctx->child_ids[saved_child_index];
            child_item_valid = child_item_id != 0 ? IMGUI_TRUE : IMGUI_FALSE;
        }
    }
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_CHILD);
    if (ctx != NULL) {
        ctx->child_current_index = saved_child_index;
        ctx->child_scroll_y = saved_child_scroll;
        ctx->child_scroll_x = saved_child_scroll_x;
        ctx->child_flags = saved_child_flags;
        if (child_item_valid) {
            /* BeginChild contributes a parent item just like the C++ API.
               Register it after restoring the parent clip/cursor state so
               hover, focus, activation, and last-item queries describe the
               child surface rather than its final inner widget. */
            parent_item_clip = ctx->clip_rect;
            ctx->cursor = child_item_cursor;
            ctx->clip_rect = child_item_clip;
            (void)imgui_item_register(ctx, child_item_id, child_item_size);
            ctx->clip_rect = parent_item_clip;
        }
    }
}

void imgui_same_line(imgui_context *ctx)
{
    if (imgui_internal_require_building(ctx, "same line outside frame")) {
        ctx->cursor.x = ctx->last_item_rect.x2 + ctx->item_spacing;
        ctx->cursor.y = ctx->last_item_rect.y1;
    }
}

void imgui_same_line_at(imgui_context *ctx, float offset, float spacing)
{
    if (imgui_internal_require_building(ctx, "same line outside frame") &&
        imgui_float_is_finite(offset) && imgui_float_is_finite(spacing)) {
        ctx->cursor.x = ctx->window_origin.x + offset;
        ctx->cursor.y = ctx->last_item_rect.y1;
        if (spacing < 0.0f) {
            ctx->line_spacing_override_valid = IMGUI_FALSE;
        } else {
            ctx->line_spacing_override = spacing;
            ctx->line_spacing_override_valid = IMGUI_TRUE;
        }
    }
}

void imgui_set_next_item_width(imgui_context *ctx, float width)
{
    if (!imgui_internal_require_building(ctx,
                                         "set item width outside frame")) {
        return;
    }
    if (!imgui_float_is_finite(width)) return;
    ctx->next_item_width = width;
    ctx->next_item_width_valid = IMGUI_TRUE;
}

void imgui_set_next_item_open(imgui_context *ctx, imgui_bool open)
{
    if (!imgui_internal_require_building(ctx,
                                         "set next item open outside frame")) {
        return;
    }
    ctx->next_item_open = open ? IMGUI_TRUE : IMGUI_FALSE;
    ctx->next_item_open_valid = IMGUI_TRUE;
}

void imgui_set_next_item_shortcut(imgui_context *ctx,
                                  imgui_key key,
                                  imgui_key_modifiers modifiers)
{
    if (!imgui_internal_require_building(ctx,
                                         "set next item shortcut outside frame") ||
        key <= IMGUI_KEY_NONE || key >= IMGUI_KEY_COUNT) {
        return;
    }
    ctx->next_item_shortcut_key = key;
    ctx->next_item_shortcut_modifiers = modifiers;
    ctx->next_item_shortcut_valid = IMGUI_TRUE;
}

void imgui_new_line(imgui_context *ctx)
{
    if (imgui_internal_require_building(ctx, "new line outside frame")) {
        ctx->cursor.x = ctx->window_origin.x + ctx->style.window_padding.x +
                        ctx->indent_width;
        ctx->cursor.y = ctx->last_item_rect.y2 + ctx->item_spacing;
    }
}

void imgui_spacing(imgui_context *ctx)
{
    if (imgui_internal_require_building(ctx, "spacing outside frame")) {
        ctx->cursor.y += ctx->style.item_spacing;
    }
}

void imgui_separator(imgui_context *ctx)
{
    imgui_rect rect;
    if (!imgui_internal_require_building(ctx, "separator outside frame")) {
        return;
    }
    rect.x1 = ctx->cursor.x;
    rect.x2 = ctx->window_origin.x + ctx->window_size.x -
              ctx->style.window_padding.x;
    rect.y1 = ctx->cursor.y + 2.0f;
    rect.y2 = rect.y1 + 1.0f;
    (void)imgui_mesh_add_rect(ctx, rect, ctx->style.color_separator);
    ctx->cursor.y += 6.0f;
}

void imgui_separator_text(imgui_context *ctx, const char *label)
{
    const char *end;
    imgui_vec2 label_size;
    imgui_vec2 origin;
    imgui_rect rect;
    float label_x;
    float line_y;
    imgui_u32 old_text_color;
    if (!imgui_internal_require_building(ctx,
                                         "separator text outside frame")) {
        return;
    }
    if (label == NULL || *label == '\0') {
        imgui_separator(ctx);
        return;
    }
    end = imgui_label_visible_end(label);
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        label_size = imgui_font_measure_text(ctx->font, label, end, 0.0f);
    } else {
        label_size = imgui_calc_text_size(label, end, 0.0f);
    }
    origin = ctx->cursor;
    rect.x1 = origin.x;
    rect.x2 = ctx->window_origin.x + ctx->window_size.x -
              ctx->style.window_padding.x;
    rect.y1 = origin.y;
    rect.y2 = origin.y + (label_size.y > 16.0f ? label_size.y : 16.0f);
    if (!imgui_item_register(ctx, 0,
                             imgui_make_vec2(rect.x2 - rect.x1,
                                             rect.y2 - rect.y1))) {
        return;
    }
    label_x = rect.x1 + ctx->style.frame_padding.x;
    line_y = rect.y1 + (rect.y2 - rect.y1) * 0.5f;
    if (label_x - 4.0f > rect.x1) {
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(rect.x1, line_y),
            imgui_make_vec2(label_x - 4.0f, line_y),
            ctx->style.color_separator, 1.0f);
    }
    if (label_x + label_size.x + 4.0f < rect.x2) {
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(label_x + label_size.x + 4.0f, line_y),
            imgui_make_vec2(rect.x2, line_y),
            ctx->style.color_separator, 1.0f);
    }
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        old_text_color = ctx->style.color_text;
        if (ctx->disabled_depth > 0) {
            ctx->style.color_text = ctx->style.color_text_disabled;
        }
        imgui_text_draw_font(ctx, label, end,
                             imgui_make_vec2(label_x,
                                             imgui_item_label_y(ctx, rect)));
        ctx->style.color_text = old_text_color;
    }
}

void imgui_indent(imgui_context *ctx, float width)
{
    if (imgui_internal_require_building(ctx, "indent outside frame") &&
        imgui_float_is_finite(width)) {
        if (width <= 0.0f) width = 20.0f;
        ctx->indent_width += width;
        ctx->cursor.x += width;
    }
}

void imgui_unindent(imgui_context *ctx, float width)
{
    if (imgui_internal_require_building(ctx, "unindent outside frame") &&
        imgui_float_is_finite(width)) {
        if (width <= 0.0f) width = 20.0f;
        if (width > ctx->indent_width) width = ctx->indent_width;
        ctx->indent_width -= width;
        ctx->cursor.x -= width;
    }
}

void imgui_group_begin(imgui_context *ctx)
{
    (void)imgui_internal_scope_begin(ctx,
                                     IMGUI_INTERNAL_SCOPE_GROUP,
                                     IMGUI_TRUE);
}

void imgui_group_end(imgui_context *ctx)
{
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_GROUP);
}

void imgui_begin_disabled(imgui_context *ctx)
{
    if (!imgui_internal_require_building(ctx,
                                         "begin disabled outside frame")) {
        return;
    }
    if (ctx->disabled_depth < INT_MAX) ++ctx->disabled_depth;
}

void imgui_end_disabled(imgui_context *ctx)
{
    if (!imgui_internal_require_building(ctx,
                                         "end disabled outside frame")) {
        return;
    }
    if (ctx->disabled_depth <= 0) {
        imgui_internal_report(ctx, IMGUI_ERROR_SCOPE_MISMATCH,
                              "disabled end without matching begin");
        return;
    }
    --ctx->disabled_depth;
}

imgui_bool imgui_is_disabled(const imgui_context *ctx)
{
    return ctx != NULL && ctx->disabled_depth > 0 ?
        IMGUI_TRUE : IMGUI_FALSE;
}

void imgui_set_keyboard_focus_here(imgui_context *ctx, int offset)
{
    int target_index;
    if (!imgui_internal_require_building(ctx,
                                         "set keyboard focus outside frame")) {
        return;
    }
    /* Dear ImGui permits negative offsets to address an item that has
       already been submitted in this frame (SetKeyboardFocusHere(-1) is the
       common way to focus the immediately preceding item).  Treating every
       negative value as zero silently focused the next item instead. */
    if (offset < 0) {
        if (offset < -ctx->navigation_item_count) return;
        target_index = ctx->navigation_item_count + offset;
        if (target_index >= 0 &&
            target_index < ctx->navigation_item_count) {
            ctx->focused_item_id =
                ctx->navigation_item_ids[target_index];
            ctx->focused_item_valid = IMGUI_TRUE;
            if (ctx->current_window_index >= 0 &&
                ctx->current_window_index < ctx->window_count) {
                ctx->navigation_window_id =
                    ctx->windows[ctx->current_window_index].id;
            }
            ctx->navigation_focused_rect =
                ctx->navigation_item_rects[target_index];
            ctx->navigation_focused_rect_valid = IMGUI_TRUE;
        }
        return;
    }
    ctx->focus_request = offset;
}

void imgui_set_item_default_focus(imgui_context *ctx)
{
    if (!imgui_internal_require_building(ctx,
                                         "set default focus outside frame")) {
        return;
    }
    if (ctx->last_item_id != 0 && !ctx->focused_item_valid &&
        !ctx->last_item_disabled) {
        ctx->focused_item_id = ctx->last_item_id;
        ctx->focused_item_valid = IMGUI_TRUE;
        if (ctx->current_window_index >= 0 &&
            ctx->current_window_index < ctx->window_count) {
            ctx->navigation_window_id =
                ctx->windows[ctx->current_window_index].id;
        }
        ctx->navigation_focused_rect = ctx->last_item_rect;
        ctx->navigation_focused_rect_valid = IMGUI_TRUE;
    }
}

imgui_vec2 imgui_get_cursor_screen_position(imgui_context *ctx)
{
    if (ctx == NULL || !imgui_internal_require_building(ctx,
                                                         "get cursor outside frame")) {
        return imgui_make_vec2(0.0f, 0.0f);
    }
    return ctx->cursor;
}

void imgui_set_cursor_screen_position(imgui_context *ctx, imgui_vec2 position)
{
    if (imgui_internal_require_building(ctx, "set cursor outside frame") &&
        imgui_float_is_finite(position.x) &&
        imgui_float_is_finite(position.y)) {
        ctx->cursor = position;
    }
}

imgui_vec2 imgui_get_cursor_position(const imgui_context *ctx)
{
    float scroll_x;
    float scroll_y;
    if (ctx == NULL || ctx->current_window_index < 0) {
        return imgui_make_vec2(0.0f, 0.0f);
    }
    scroll_x = ctx->child_current_index >= 0 ? ctx->child_scroll_x :
        ctx->windows[ctx->current_window_index].scroll_x;
    scroll_y = ctx->child_current_index >= 0 ? ctx->child_scroll_y :
        ctx->windows[ctx->current_window_index].scroll_y;
    return imgui_make_vec2(ctx->cursor.x - ctx->window_origin.x + scroll_x,
                           ctx->cursor.y - ctx->window_origin.y + scroll_y);
}

void imgui_set_cursor_position(imgui_context *ctx, imgui_vec2 position)
{
    float scroll_x;
    float scroll_y;
    if (!imgui_internal_require_building(ctx,
                                         "set cursor position outside frame") ||
        !imgui_vec2_is_finite(position)) {
        return;
    }
    scroll_x = ctx->child_current_index >= 0 ? ctx->child_scroll_x :
        ctx->windows[ctx->current_window_index].scroll_x;
    scroll_y = ctx->child_current_index >= 0 ? ctx->child_scroll_y :
        ctx->windows[ctx->current_window_index].scroll_y;
    ctx->cursor.x = ctx->window_origin.x + position.x - scroll_x;
    ctx->cursor.y = ctx->window_origin.y + position.y - scroll_y;
}

imgui_vec2 imgui_get_cursor_start_position(const imgui_context *ctx)
{
    float title_height;
    if (ctx == NULL || ctx->current_window_index < 0) {
        return imgui_make_vec2(0.0f, 0.0f);
    }
    title_height = ctx->child_current_index >= 0 ||
                   (ctx->window_flags & IMGUI_WINDOW_NO_TITLE_BAR) != 0 ?
                   0.0f : 20.0f;
    return imgui_make_vec2(ctx->style.window_padding.x,
                           title_height + ctx->style.window_padding.y);
}

float imgui_get_cursor_position_x(const imgui_context *ctx)
{
    return imgui_get_cursor_position(ctx).x;
}

float imgui_get_cursor_position_y(const imgui_context *ctx)
{
    return imgui_get_cursor_position(ctx).y;
}

void imgui_set_cursor_position_x(imgui_context *ctx, float x)
{
    imgui_vec2 position;
    position = imgui_get_cursor_position(ctx);
    position.x = x;
    imgui_set_cursor_position(ctx, position);
}

void imgui_set_cursor_position_y(imgui_context *ctx, float y)
{
    imgui_vec2 position;
    position = imgui_get_cursor_position(ctx);
    position.y = y;
    imgui_set_cursor_position(ctx, position);
}

imgui_vec2 imgui_get_window_position(const imgui_context *ctx)
{
    return ctx != NULL && ctx->current_window_index >= 0 ?
        ctx->window_origin : imgui_make_vec2(0.0f, 0.0f);
}

imgui_vec2 imgui_get_window_size(const imgui_context *ctx)
{
    return ctx != NULL && ctx->current_window_index >= 0 ?
        ctx->window_size : imgui_make_vec2(0.0f, 0.0f);
}

static imgui_rect imgui_current_content_region(const imgui_context *ctx)
{
    imgui_rect region;
    imgui_vec2 origin;
    imgui_vec2 size;
    float scroll_x;
    float scroll_y;
    float title_height;
    memset(&region, 0, sizeof(region));
    if (ctx == NULL || ctx->current_window_index < 0 ||
        ctx->current_window_index >= ctx->window_count) {
        return region;
    }
    origin = ctx->window_origin;
    size = ctx->window_size;
    scroll_x = ctx->windows[ctx->current_window_index].scroll_x;
    scroll_y = ctx->windows[ctx->current_window_index].scroll_y;
    if (ctx->child_current_index >= 0 && ctx->scope_depth > 0) {
        origin = ctx->scope_saved_cursor[ctx->scope_depth - 1];
        size = ctx->scope_saved_size[ctx->scope_depth - 1];
        scroll_x = ctx->child_scroll_x;
        scroll_y = ctx->child_scroll_y;
        title_height = 0.0f;
    } else {
        title_height = (ctx->window_flags & IMGUI_WINDOW_NO_TITLE_BAR) != 0 ?
                       0.0f : 20.0f;
    }
    region.x1 = origin.x + ctx->style.window_padding.x - scroll_x;
    region.y1 = origin.y + title_height + ctx->style.window_padding.y -
                scroll_y;
    region.x2 = origin.x + size.x - ctx->style.window_padding.x - scroll_x;
    region.y2 = origin.y + size.y - ctx->style.window_padding.y - scroll_y;
    return region;
}

imgui_vec2 imgui_get_window_content_region_min(const imgui_context *ctx)
{
    imgui_rect region = imgui_current_content_region(ctx);
    return imgui_make_vec2(region.x1, region.y1);
}

imgui_vec2 imgui_get_window_content_region_max(const imgui_context *ctx)
{
    imgui_rect region = imgui_current_content_region(ctx);
    return imgui_make_vec2(region.x2, region.y2);
}

imgui_vec2 imgui_get_window_content_region_size(const imgui_context *ctx)
{
    imgui_rect region = imgui_current_content_region(ctx);
    return imgui_make_vec2(region.x2 - region.x1, region.y2 - region.y1);
}

imgui_result imgui_set_next_window_position(imgui_context *ctx,
                                            imgui_vec2 position)
{
    if (ctx == NULL || !imgui_float_is_finite(position.x) ||
        !imgui_float_is_finite(position.y) ||
        !imgui_internal_require_building(
            ctx, "set next window position outside frame")) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->next_window_position = position;
    ctx->next_window_position_pivot = imgui_make_vec2(0.0f, 0.0f);
    ctx->next_window_position_valid = IMGUI_TRUE;
    ctx->next_window_position_pivot_valid = IMGUI_FALSE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_set_next_window_position_ex(imgui_context *ctx,
                                               imgui_vec2 position,
                                               imgui_vec2 pivot)
{
    if (ctx == NULL || !imgui_vec2_is_finite(position) ||
        !imgui_vec2_is_finite(pivot) || pivot.x < 0.0f || pivot.x > 1.0f ||
        pivot.y < 0.0f || pivot.y > 1.0f ||
        !imgui_internal_require_building(
            ctx, "set next window position pivot outside frame")) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->next_window_position = position;
    ctx->next_window_position_pivot = pivot;
    ctx->next_window_position_valid = IMGUI_TRUE;
    ctx->next_window_position_pivot_valid = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_set_next_window_viewport(imgui_context *ctx,
                                            imgui_id viewport_id)
{
    if (ctx == NULL || viewport_id == 0 ||
        imgui_viewport_find(ctx, viewport_id) < 0 ||
        !imgui_internal_require_building(
            ctx, "set next window viewport outside frame")) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->next_window_viewport_id = viewport_id;
    ctx->next_window_viewport_valid = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_set_next_window_size(imgui_context *ctx, imgui_vec2 size)
{
    if (ctx == NULL || !imgui_vec2_is_finite(size) || size.x <= 0.0f ||
        size.y <= 0.0f ||
        !imgui_internal_require_building(
            ctx, "set next window size outside frame")) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->next_window_size = size;
    ctx->next_window_size_valid = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_set_next_window_content_size(imgui_context *ctx,
                                                imgui_vec2 size)
{
    if (ctx == NULL || !imgui_float_is_finite(size.x) ||
        !imgui_float_is_finite(size.y) || size.x < -1.0f || size.y < -1.0f ||
        !imgui_internal_require_building(
            ctx, "set next window content size outside frame")) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->next_window_content_size = size;
    ctx->next_window_content_size_valid = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_set_next_window_background_alpha(imgui_context *ctx,
                                                    float alpha)
{
    if (ctx == NULL || !imgui_float_is_finite(alpha) || alpha < 0.0f ||
        alpha > 1.0f ||
        !imgui_internal_require_building(
            ctx, "set next window background alpha outside frame")) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->next_window_background_alpha = alpha;
    ctx->next_window_background_alpha_valid = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_set_next_window_size_constraints(imgui_context *ctx,
                                                    imgui_vec2 minimum,
                                                    imgui_vec2 maximum)
{
    if (ctx == NULL || !imgui_vec2_is_finite(minimum) ||
        !imgui_vec2_is_finite(maximum) || minimum.x < 0.0f ||
        minimum.y < 0.0f || maximum.x < -1.0f || maximum.y < -1.0f ||
        (maximum.x >= 0.0f && maximum.x < minimum.x) ||
        (maximum.y >= 0.0f && maximum.y < minimum.y) ||
        !imgui_internal_require_building(
            ctx, "set next window size constraints outside frame")) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->next_window_size_min = minimum;
    ctx->next_window_size_max = maximum;
    ctx->next_window_size_constraints_valid = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_set_next_window_scroll(imgui_context *ctx,
                                           imgui_vec2 scroll)
{
    if (ctx == NULL || !imgui_vec2_is_finite(scroll) || scroll.x < 0.0f ||
        scroll.y < 0.0f || !imgui_internal_require_building(
            ctx, "set next window scroll outside frame")) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->next_window_scroll = scroll;
    ctx->next_window_scroll_valid = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_set_next_window_collapsed(imgui_context *ctx,
                                             imgui_bool collapsed)
{
    if (ctx == NULL ||
        !imgui_internal_require_building(
            ctx, "set next window collapsed outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    ctx->next_window_collapsed = collapsed ? IMGUI_TRUE : IMGUI_FALSE;
    ctx->next_window_collapsed_valid = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_set_next_window_focus(imgui_context *ctx)
{
    if (ctx == NULL ||
        !imgui_internal_require_building(
            ctx, "set next window focus outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    ctx->next_window_focus_valid = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_bool imgui_is_window_collapsed(const imgui_context *ctx)
{
    if (ctx == NULL || ctx->current_window_index < 0 ||
        ctx->current_window_index >= ctx->window_count) return IMGUI_FALSE;
    return ctx->windows[ctx->current_window_index].collapsed;
}

imgui_bool imgui_is_window_appearing(const imgui_context *ctx)
{
    if (ctx == NULL || ctx->current_window_index < 0 ||
        ctx->current_window_index >= ctx->window_count) return IMGUI_FALSE;
    return ctx->windows[ctx->current_window_index].hidden_this_frame;
}

imgui_result imgui_set_window_focus(imgui_context *ctx)
{
    imgui_internal_window *window;
    if (!imgui_internal_require_building(ctx,
                                         "set window focus outside frame") ||
        ctx->current_window_index < 0 ||
        ctx->current_window_index >= ctx->window_count) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    window = &ctx->windows[ctx->current_window_index];
    if (!window->open || window->collapsed) return IMGUI_RESULT_INVALID_STATE;
    ctx->focused_window_id = window->id;
    ctx->focused_window_valid = IMGUI_TRUE;
    window->z_order = ++ctx->next_window_z_order;
    ctx->window_z_order_dirty = IMGUI_TRUE;
    ctx->window_focused = IMGUI_TRUE;
    ctx->frame_any_window_focused = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_set_window_collapsed(imgui_context *ctx,
                                        imgui_bool collapsed)
{
    imgui_internal_window *window;
    if (!imgui_internal_require_building(ctx,
                                         "set window collapsed outside frame") ||
        ctx->current_window_index < 0 ||
        ctx->current_window_index >= ctx->window_count) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    window = &ctx->windows[ctx->current_window_index];
    if ((window->flags & IMGUI_WINDOW_NO_COLLAPSE) != 0) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    collapsed = collapsed ? IMGUI_TRUE : IMGUI_FALSE;
    if (collapsed && !window->collapsed) {
        window->expanded_size = window->size;
        window->collapsed = IMGUI_TRUE;
        window->size.y = 20.0f;
    } else if (!collapsed && window->collapsed) {
        window->collapsed = IMGUI_FALSE;
        if (window->expanded_size.x > 0.0f &&
            window->expanded_size.y > 20.0f) {
            window->size = window->expanded_size;
        }
    }
    ctx->window_size = window->size;
    ctx->window_active = window->open && !window->collapsed;
    ctx->clip_rect.x2 = ctx->window_origin.x + window->size.x;
    ctx->clip_rect.y2 = ctx->window_origin.y + window->size.y;
    return IMGUI_RESULT_OK;
}

void imgui_set_window_position(imgui_context *ctx, imgui_vec2 position)
{
    imgui_internal_window *window;
    imgui_vec2 delta;
    if (!imgui_float_is_finite(position.x) ||
        !imgui_float_is_finite(position.y) ||
        !imgui_internal_require_building(ctx,
                                         "set window position outside frame") ||
        ctx->current_window_index < 0 ||
        ctx->current_window_index >= ctx->window_count) {
        return;
    }
    window = &ctx->windows[ctx->current_window_index];
    delta.x = position.x - ctx->window_origin.x;
    delta.y = position.y - ctx->window_origin.y;
    window->position = position;
    ctx->window_origin = position;
    ctx->cursor.x += delta.x;
    ctx->cursor.y += delta.y;
    ctx->clip_rect.x1 = position.x;
    ctx->clip_rect.y1 = position.y;
    ctx->clip_rect.x2 = position.x + ctx->window_size.x;
    ctx->clip_rect.y2 = position.y + ctx->window_size.y;
}

void imgui_set_window_size(imgui_context *ctx, imgui_vec2 size)
{
    imgui_internal_window *window;
    if (!imgui_float_is_finite(size.x) ||
        !imgui_float_is_finite(size.y) ||
        !imgui_internal_require_building(ctx,
                                         "set window size outside frame") ||
        ctx->current_window_index < 0 ||
        ctx->current_window_index >= ctx->window_count) {
        return;
    }
    if (size.x <= 0.0f) size.x = 1.0f;
    if (size.y <= 0.0f) size.y = 1.0f;
    window = &ctx->windows[ctx->current_window_index];
    window->size = size;
    window->expanded_size = size;
    ctx->window_size = size;
    ctx->clip_rect.x2 = ctx->window_origin.x + size.x;
    ctx->clip_rect.y2 = ctx->window_origin.y + size.y;
}

imgui_vec2 imgui_get_content_region_available(imgui_context *ctx)
{
    imgui_vec2 available;
    if (ctx == NULL || !imgui_internal_require_building(ctx,
                                                         "content region outside frame")) {
        return imgui_make_vec2(0.0f, 0.0f);
    }
    available = imgui_make_vec2(
        ctx->window_origin.x + ctx->window_size.x - ctx->cursor.x,
        ctx->window_origin.y + ctx->window_size.y - ctx->cursor.y);
    if (available.x < 0.0f) available.x = 0.0f;
    if (available.y < 0.0f) available.y = 0.0f;
    return available;
}

float imgui_calc_item_width(const imgui_context *ctx)
{
    float width;
    if (ctx == NULL || ctx->current_window_index < 0) return 0.0f;
    if (ctx->next_item_width_valid) {
        if (ctx->next_item_width > 0.0f) return ctx->next_item_width;
        width = ctx->window_origin.x + ctx->window_size.x -
                ctx->style.window_padding.x - ctx->cursor.x;
    } else {
        width = ctx->window_origin.x + ctx->window_size.x -
                ctx->style.window_padding.x - ctx->cursor.x;
    }
    return width > 0.0f ? width : 0.0f;
}

float imgui_get_text_line_height(const imgui_context *ctx)
{
    if (ctx != NULL && ctx->font != NULL) {
        return imgui_font_get_line_height(ctx->font);
    }
    return 16.0f;
}

float imgui_get_frame_height(const imgui_context *ctx)
{
    if (ctx == NULL) return 0.0f;
    return imgui_get_text_line_height(ctx) +
           2.0f * ctx->style.frame_padding.y;
}

float imgui_get_frame_height_with_spacing(const imgui_context *ctx)
{
    if (ctx == NULL) return 0.0f;
    return imgui_get_frame_height(ctx) + ctx->style.item_spacing;
}

float imgui_get_scroll_y(const imgui_context *ctx)
{
    if (ctx == NULL) return 0.0f;
    if (ctx->child_current_index >= 0) return ctx->child_scroll_y;
    if (ctx->current_window_index >= 0 &&
        ctx->current_window_index < ctx->window_count) {
        return ctx->windows[ctx->current_window_index].scroll_y;
    }
    return 0.0f;
}

float imgui_get_scroll_max_y(const imgui_context *ctx)
{
    if (ctx == NULL) return 0.0f;
    if (ctx->child_current_index >= 0 &&
        ctx->child_current_index < ctx->child_count) {
        return ctx->child_scroll_maxs[ctx->child_current_index];
    }
    if (ctx->current_window_index >= 0 &&
        ctx->current_window_index < ctx->window_count) {
        return ctx->windows[ctx->current_window_index].scroll_max_y;
    }
    return 0.0f;
}

imgui_result imgui_set_scroll_y(imgui_context *ctx, float scroll_y)
{
    if (ctx == NULL || !imgui_float_is_finite(scroll_y) ||
        scroll_y < 0.0f) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (ctx->child_current_index >= 0 &&
        ctx->child_current_index < ctx->child_count) {
        if (ctx->child_scroll_maxs[ctx->child_current_index] > 0.0f &&
            scroll_y > ctx->child_scroll_maxs[ctx->child_current_index]) {
            scroll_y = ctx->child_scroll_maxs[ctx->child_current_index];
        }
        ctx->child_scroll_y = scroll_y;
        ctx->child_scrolls[ctx->child_current_index] = scroll_y;
        return IMGUI_RESULT_OK;
    }
    if (ctx->current_window_index >= 0 &&
        ctx->current_window_index < ctx->window_count) {
        if (scroll_y > ctx->windows[ctx->current_window_index].scroll_max_y) {
            scroll_y = ctx->windows[ctx->current_window_index].scroll_max_y;
        }
        ctx->windows[ctx->current_window_index].scroll_y = scroll_y;
        return IMGUI_RESULT_OK;
    }
    return IMGUI_RESULT_INVALID_STATE;
}

float imgui_get_scroll_x(const imgui_context *ctx)
{
    if (ctx == NULL) return 0.0f;
    if (ctx->child_current_index >= 0) return ctx->child_scroll_x;
    if (ctx->current_window_index >= 0 &&
        ctx->current_window_index < ctx->window_count) {
        return ctx->windows[ctx->current_window_index].scroll_x;
    }
    return 0.0f;
}

float imgui_get_scroll_max_x(const imgui_context *ctx)
{
    if (ctx == NULL) return 0.0f;
    if (ctx->child_current_index >= 0 &&
        ctx->child_current_index < ctx->child_count) {
        return ctx->child_scroll_max_xs[ctx->child_current_index];
    }
    if (ctx->current_window_index >= 0 &&
        ctx->current_window_index < ctx->window_count) {
        return ctx->windows[ctx->current_window_index].scroll_max_x;
    }
    return 0.0f;
}

imgui_result imgui_set_scroll_x(imgui_context *ctx, float scroll_x)
{
    if (ctx == NULL || !imgui_float_is_finite(scroll_x) ||
        scroll_x < 0.0f) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (ctx->child_current_index >= 0 &&
        ctx->child_current_index < ctx->child_count) {
        if (scroll_x > ctx->child_scroll_max_xs[ctx->child_current_index]) {
            scroll_x = ctx->child_scroll_max_xs[ctx->child_current_index];
        }
        ctx->child_scroll_x = scroll_x;
        ctx->child_scroll_xs[ctx->child_current_index] = scroll_x;
        return IMGUI_RESULT_OK;
    }
    if (ctx->current_window_index >= 0 &&
        ctx->current_window_index < ctx->window_count) {
        if (scroll_x > ctx->windows[ctx->current_window_index].scroll_max_x) {
            scroll_x = ctx->windows[ctx->current_window_index].scroll_max_x;
        }
        ctx->windows[ctx->current_window_index].scroll_x = scroll_x;
        return IMGUI_RESULT_OK;
    }
    return IMGUI_RESULT_INVALID_STATE;
}

void imgui_text(imgui_context *ctx, const char *format, ...)
{
    char text[2048];
    va_list arguments;
    if (format == NULL) return;
    va_start(arguments, format);
    (void)vsnprintf(text, sizeof(text), format, arguments);
    text[sizeof(text) - 1U] = '\0';
    va_end(arguments);
    imgui_text_unformatted(ctx, text, NULL);
}

void imgui_text_colored(imgui_context *ctx, imgui_u32 color,
                        const char *format, ...)
{
    char text[2048];
    imgui_u32 old_color;
    va_list arguments;
    if (ctx == NULL || format == NULL) return;
    va_start(arguments, format);
    (void)vsnprintf(text, sizeof(text), format, arguments);
    text[sizeof(text) - 1U] = '\0';
    va_end(arguments);
    old_color = ctx->style.color_text;
    ctx->style.color_text = color;
    imgui_text_unformatted(ctx, text, NULL);
    ctx->style.color_text = old_color;
}

void imgui_text_disabled(imgui_context *ctx, const char *format, ...)
{
    char text[2048];
    imgui_u32 old_color;
    va_list arguments;
    if (ctx == NULL || format == NULL) return;
    va_start(arguments, format);
    (void)vsnprintf(text, sizeof(text), format, arguments);
    text[sizeof(text) - 1U] = '\0';
    va_end(arguments);
    old_color = ctx->style.color_text;
    ctx->style.color_text = ctx->style.color_text_disabled;
    imgui_text_unformatted(ctx, text, NULL);
    ctx->style.color_text = old_color;
}

imgui_bool imgui_text_link(imgui_context *ctx, const char *label)
{
    imgui_vec2 size;
    imgui_u32 old_color;
    imgui_u32 link_color;
    const char *end;
    if (ctx == NULL || label == NULL ||
        !imgui_internal_require_building(ctx,
                                          "text link outside frame")) {
        return IMGUI_FALSE;
    }
    end = imgui_label_visible_end(label);
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        size = imgui_font_measure_text(ctx->font, label, end, 0.0f);
    } else {
        size = imgui_calc_text_size(label, end, 0.0f);
    }
    if (size.y < 16.0f) size.y = 16.0f;
    if (!imgui_item_register(ctx, imgui_get_id_string(ctx, label), size)) {
        return IMGUI_FALSE;
    }
    link_color = ctx->last_item_active ? ctx->style.color_text_link_active :
                 (ctx->last_item_hovered ? ctx->style.color_text_link_hovered :
                                            ctx->style.color_text_link);
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        old_color = ctx->style.color_text;
        ctx->style.color_text = link_color;
        imgui_text_draw_font(ctx, label, end,
                             imgui_make_vec2(ctx->last_item_rect.x1,
                                             imgui_item_label_y(
                                                 ctx, ctx->last_item_rect)));
        ctx->style.color_text = old_color;
    }
    (void)imgui_draw_list_add_line(
        ctx, &ctx->default_draw_list,
        imgui_make_vec2(ctx->last_item_rect.x1, ctx->last_item_rect.y2 - 2.0f),
        imgui_make_vec2(ctx->last_item_rect.x2, ctx->last_item_rect.y2 - 2.0f),
        link_color, 1.0f);
    return ctx->last_item_clicked;
}

static size_t imgui_utf8_measure_step(const char *cursor,
                                      const char *end)
{
    unsigned char first;
    size_t remaining;
    if (cursor == NULL || end == NULL || cursor >= end) return 0;
    first = (unsigned char)*cursor;
    remaining = (size_t)(end - cursor);
    if (first < 0x80U) return 1;
    if (first >= 0xc2U && first <= 0xdfU && remaining >= 2) return 2;
    if (first >= 0xe0U && first <= 0xefU && remaining >= 3) return 3;
    if (first >= 0xf0U && first <= 0xf4U && remaining >= 4) return 4;
    return 1;
}

imgui_vec2 imgui_calc_text_size(const char *begin,
                                const char *end,
                                float wrap_width)
{
    const char *cursor;
    size_t step;
    float line_width;
    float maximum_width;
    int line_count;
    if (begin == NULL) return imgui_make_vec2(0.0f, 16.0f);
    if (end == NULL) end = begin + strlen(begin);
    if (end < begin) return imgui_make_vec2(0.0f, 16.0f);
    cursor = begin;
    line_width = 0.0f;
    maximum_width = 0.0f;
    line_count = 1;
    while (cursor < end) {
        if (*cursor == '\n') {
            if (line_width > maximum_width) maximum_width = line_width;
            line_width = 0.0f;
            ++line_count;
            ++cursor;
            continue;
        }
        if (wrap_width > 0.0f && line_width > 0.0f &&
            (*cursor == ' ' || *cursor == '\t') &&
            line_width + 8.0f > wrap_width) {
            if (line_width > maximum_width) maximum_width = line_width;
            line_width = 0.0f;
            ++line_count;
            ++cursor;
            continue;
        }
        if (wrap_width > 0.0f && line_width > 0.0f &&
            line_width + 8.0f > wrap_width) {
            if (line_width > maximum_width) maximum_width = line_width;
            line_width = 0.0f;
            ++line_count;
        }
        line_width += 8.0f;
        step = imgui_utf8_measure_step(cursor, end);
        cursor += step != 0 ? step : 1;
    }
    if (line_width > maximum_width) maximum_width = line_width;
    return imgui_make_vec2(maximum_width, (float)line_count * 16.0f);
}

static imgui_vec2 imgui_text_measure(const char *begin, const char *end)
{
    return imgui_calc_text_size(begin, end, 0.0f);
}

static unsigned long imgui_text_decode_codepoint(const char *cursor,
                                                 size_t step)
{
    const unsigned char *bytes;
    bytes = (const unsigned char *)cursor;
    if (step == 1) return (unsigned long)bytes[0];
    if (step == 2) {
        return ((unsigned long)(bytes[0] & 0x1fU) << 6) |
               (unsigned long)(bytes[1] & 0x3fU);
    }
    if (step == 3) {
        return ((unsigned long)(bytes[0] & 0x0fU) << 12) |
               ((unsigned long)(bytes[1] & 0x3fU) << 6) |
               (unsigned long)(bytes[2] & 0x3fU);
    }
    return ((unsigned long)(bytes[0] & 0x07U) << 18) |
           ((unsigned long)(bytes[1] & 0x3fU) << 12) |
           ((unsigned long)(bytes[2] & 0x3fU) << 6) |
           (unsigned long)(bytes[3] & 0x3fU);
}

static float imgui_text_word_width(const imgui_context *ctx,
                                   const char *begin, const char *end)
{
    const char *cursor;
    unsigned long codepoint;
    unsigned long previous;
    size_t step;
    float width;
    const imgui_font_glyph *glyph;
    imgui_bool have_previous;
    if (ctx == NULL || ctx->font == NULL || begin == NULL || end == NULL) {
        return 0.0f;
    }
    cursor = begin;
    previous = 0;
    width = 0.0f;
    have_previous = IMGUI_FALSE;
    while (cursor < end && *cursor != ' ' && *cursor != '\t' &&
           *cursor != '\n' && *cursor != '\r') {
        step = imgui_utf8_measure_step(cursor, end);
        if (step == 0) break;
        codepoint = imgui_text_decode_codepoint(cursor, step);
        glyph = imgui_font_find_glyph(ctx->font, codepoint);
        if (glyph == NULL) glyph = imgui_font_find_glyph(ctx->font, '?');
        if (glyph != NULL) {
            if (have_previous) width += imgui_font_get_kerning(
                ctx->font, previous, codepoint);
            width += glyph->advance_x;
            previous = glyph->codepoint;
            have_previous = IMGUI_TRUE;
        }
        cursor += step;
    }
    return width;
}

static void imgui_text_draw_font_wrap(imgui_context *ctx, const char *begin,
                                      const char *end, imgui_vec2 origin,
                                      float wrap_width)
{
    const char *cursor;
    size_t step;
    unsigned long codepoint;
    unsigned long previous_codepoint;
    imgui_bool have_previous;
    const imgui_font_glyph *glyph;
    float x;
    float y;
    float line_height;
    cursor = begin;
    x = origin.x;
    y = origin.y;
    previous_codepoint = 0;
    have_previous = IMGUI_FALSE;
    line_height = imgui_font_get_line_height(ctx->font);
    while (cursor < end) {
        step = imgui_utf8_measure_step(cursor, end);
        if (step == 0) break;
        if (*cursor == '\n') {
            x = origin.x;
            y += line_height;
            have_previous = IMGUI_FALSE;
            cursor += 1;
            continue;
        }
        codepoint = imgui_text_decode_codepoint(cursor, step);
        glyph = imgui_font_find_glyph(ctx->font, codepoint);
        if (glyph == NULL) glyph = imgui_font_find_glyph(ctx->font, '?');
        if (glyph != NULL) {
            if (wrap_width > 0.0f && x > origin.x &&
                codepoint != (unsigned long)' ' &&
                codepoint != (unsigned long)'\t' &&
                codepoint != (unsigned long)'\n' &&
                codepoint != (unsigned long)'\r') {
                float word_width = imgui_text_word_width(ctx, cursor, end);
                if (word_width <= wrap_width &&
                    x + word_width > origin.x + wrap_width) {
                    x = origin.x;
                    y += line_height;
                    have_previous = IMGUI_FALSE;
                }
            }
            if (wrap_width > 0.0f && x > origin.x &&
                (codepoint == (unsigned long)' ' ||
                 codepoint == (unsigned long)'\t') &&
                x + glyph->advance_x > origin.x + wrap_width) {
                x = origin.x;
                y += line_height;
                have_previous = IMGUI_FALSE;
                cursor += step;
                continue;
            }
            if (have_previous) {
                x += imgui_font_get_kerning(ctx->font, previous_codepoint,
                                            codepoint);
            }
            if (wrap_width > 0.0f && x > origin.x &&
                x + glyph->advance_x > origin.x + wrap_width) {
                x = origin.x;
                y += line_height;
                have_previous = IMGUI_FALSE;
            }
            imgui_mesh_add_font_glyph(ctx, glyph, x,
                                      (float)floor((double)(
                                          y + imgui_font_get_ascent(ctx->font))),
                                      ctx->disabled_depth > 0 ?
                                      ctx->style.color_text_disabled :
                                      ctx->style.color_text);
            x += glyph->advance_x;
            previous_codepoint = glyph->codepoint;
            have_previous = IMGUI_TRUE;
        }
        cursor += step;
    }
}

static void imgui_text_draw_font(imgui_context *ctx, const char *begin,
                                 const char *end, imgui_vec2 origin)
{
    imgui_text_draw_font_wrap(ctx, begin, end, origin, 0.0f);
}

void imgui_text_unformatted(imgui_context *ctx,
                            const char *begin,
                            const char *end)
{
    imgui_vec2 size;
    imgui_vec2 origin;
    imgui_u32 old_text_color;
    if (ctx == NULL || begin == NULL ||
        !imgui_internal_require_building(ctx, "text outside frame")) return;
    if (ctx->log_active) (void)imgui_log_text(ctx, begin, end);
    if (end == NULL) end = begin + strlen(begin);
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        size = imgui_font_measure_text(ctx->font, begin, end, 0.0f);
    } else {
        size = imgui_text_measure(begin, end);
    }
    origin = ctx->cursor;
    (void)imgui_item_register(ctx, 0, size);
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        old_text_color = ctx->style.color_text;
        if (ctx->disabled_depth > 0) {
            ctx->style.color_text = ctx->style.color_text_disabled;
        }
        imgui_text_draw_font(ctx, begin, end, origin);
        ctx->style.color_text = old_text_color;
    }
}

void imgui_text_wrapped(imgui_context *ctx, const char *text,
                        float wrap_width)
{
    imgui_vec2 size;
    imgui_vec2 origin;
    const char *end;
    imgui_u32 old_text_color;
    if (ctx == NULL || text == NULL ||
        !imgui_internal_require_building(ctx, "wrapped text outside frame")) {
        return;
    }
    end = text + strlen(text);
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        size = imgui_font_measure_text(ctx->font, text, end, wrap_width);
    } else {
        size = imgui_calc_text_size(text, end, wrap_width);
    }
    origin = ctx->cursor;
    (void)imgui_item_register(ctx, 0, size);
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        old_text_color = ctx->style.color_text;
        if (ctx->disabled_depth > 0) {
            ctx->style.color_text = ctx->style.color_text_disabled;
        }
        imgui_text_draw_font_wrap(ctx, text, end, origin, wrap_width);
        ctx->style.color_text = old_text_color;
    }
}

void imgui_text_localized(imgui_context *ctx, const char *key)
{
    const char *text;
    text = imgui_localize(ctx, key);
    imgui_text_unformatted(ctx, text, NULL);
}

void imgui_dummy(imgui_context *ctx, imgui_vec2 size)
{
    if (ctx == NULL ||
        !imgui_internal_require_building(ctx, "dummy outside frame") ||
        !imgui_float_is_finite(size.x) ||
        !imgui_float_is_finite(size.y)) {
        return;
    }
    if (size.x < 0.0f) size.x = 0.0f;
    if (size.y < 0.0f) size.y = 0.0f;
    (void)imgui_item_register(ctx, 0, size);
}

void imgui_bullet(imgui_context *ctx)
{
    imgui_vec2 origin;
    if (ctx == NULL ||
        !imgui_internal_require_building(ctx, "bullet outside frame")) {
        return;
    }
    origin = ctx->cursor;
    if (!imgui_item_register(ctx, 0, imgui_make_vec2(10.0f, 16.0f))) {
        return;
    }
    (void)imgui_draw_list_add_circle(
        ctx, &ctx->default_draw_list,
        imgui_make_vec2(origin.x + 3.0f, origin.y + 8.0f), 3.0f,
        ctx->disabled_depth > 0 ? ctx->style.color_text_disabled :
                                   ctx->style.color_text, 8);
}

void imgui_bullet_text(imgui_context *ctx, const char *format, ...)
{
    char text[2048];
    imgui_vec2 text_size;
    imgui_vec2 origin;
    const char *end;
    imgui_u32 old_text_color;
    va_list arguments;
    if (ctx == NULL || format == NULL ||
        !imgui_internal_require_building(ctx,
                                          "bullet text outside frame")) {
        return;
    }
    va_start(arguments, format);
    (void)vsnprintf(text, sizeof(text), format, arguments);
    text[sizeof(text) - 1U] = '\0';
    va_end(arguments);
    end = text + strlen(text);
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        text_size = imgui_font_measure_text(ctx->font, text, end, 0.0f);
    } else {
        text_size = imgui_calc_text_size(text, end, 0.0f);
    }
    if (text_size.y < 16.0f) text_size.y = 16.0f;
    origin = ctx->cursor;
    if (!imgui_item_register(ctx, 0,
                             imgui_make_vec2(text_size.x + 10.0f,
                                             text_size.y))) {
        return;
    }
    (void)imgui_draw_list_add_circle(
        ctx, &ctx->default_draw_list,
        imgui_make_vec2(origin.x + 3.0f, origin.y + 8.0f), 3.0f,
        ctx->disabled_depth > 0 ? ctx->style.color_text_disabled :
                                   ctx->style.color_text, 8);
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        old_text_color = ctx->style.color_text;
        if (ctx->disabled_depth > 0) {
            ctx->style.color_text = ctx->style.color_text_disabled;
        }
        imgui_text_draw_font(ctx, text, end,
                             imgui_make_vec2(origin.x + 10.0f,
                                             origin.y));
        ctx->style.color_text = old_text_color;
    }
}

void imgui_label_text(imgui_context *ctx,
                      const char *label,
                      const char *format,
                      ...)
{
    char text[2048];
    va_list arguments;
    if (format == NULL) return;
    va_start(arguments, format);
    (void)vsnprintf(text, sizeof(text), format, arguments);
    text[sizeof(text) - 1U] = '\0';
    va_end(arguments);
    if (label != NULL) {
        imgui_text_unformatted(ctx, label, NULL);
        imgui_same_line(ctx);
    }
    imgui_text_unformatted(ctx, text, NULL);
}

static imgui_bool imgui_item_register(imgui_context *ctx,
                                      imgui_id id,
                                      imgui_vec2 size)
{
    imgui_bool mouse_window_active;
    imgui_bool disabled;
    imgui_bool mouse_clicked;
    imgui_bool mouse_released;
    imgui_bool keyboard_activated;
    imgui_bool shortcut_activated;
    imgui_id current_window_id;
    float item_spacing;
    if (!imgui_internal_require_building(ctx, "widget outside frame")) {
        return IMGUI_FALSE;
    }
    if (ctx->next_item_width_valid) {
        if (ctx->next_item_width <= 0.0f) {
            size.x = ctx->window_origin.x + ctx->window_size.x -
                     ctx->style.window_padding.x - ctx->cursor.x;
        } else {
            size.x = ctx->next_item_width;
        }
        if (size.x < 0.0f) size.x = 0.0f;
        ctx->next_item_width_valid = IMGUI_FALSE;
    }
    if (ctx->last_item_edited && ctx->last_item_id != 0 &&
        ctx->active_item_valid &&
        ctx->last_item_id == ctx->active_item_id) {
        ctx->active_item_was_edited = IMGUI_TRUE;
    }
    ctx->last_item_id = id;
    ctx->last_item_toggled_open = IMGUI_FALSE;
    ctx->last_item_toggled_selection = IMGUI_FALSE;
    ctx->last_item_rect.x1 = ctx->cursor.x;
    ctx->last_item_rect.y1 = ctx->cursor.y;
    ctx->last_item_rect.x2 = ctx->cursor.x + size.x;
    ctx->last_item_rect.y2 = ctx->cursor.y + size.y;
    if (ctx->popup_open && ctx->current_popup_id != 0 &&
        ctx->popup_rect_valid) {
        if (ctx->last_item_rect.x2 > ctx->popup_rect.x2) {
            ctx->popup_rect.x2 = ctx->last_item_rect.x2;
        }
        if (ctx->last_item_rect.y2 > ctx->popup_rect.y2) {
            ctx->popup_rect.y2 = ctx->last_item_rect.y2;
        }
        if (ctx->clip_rect.x2 < ctx->popup_rect.x2) {
            ctx->clip_rect.x2 = ctx->popup_rect.x2;
            if (ctx->clip_rect.x2 > ctx->frame_desc.display_size.x) {
                ctx->clip_rect.x2 = ctx->frame_desc.display_size.x;
            }
        }
        if (ctx->clip_rect.y2 < ctx->popup_rect.y2) {
            ctx->clip_rect.y2 = ctx->popup_rect.y2;
            if (ctx->clip_rect.y2 > ctx->frame_desc.display_size.y) {
                ctx->clip_rect.y2 = ctx->frame_desc.display_size.y;
            }
        }
        if (ctx->popup_stack_count > 0) {
            ctx->popup_stack_rects[ctx->popup_stack_count - 1] =
                ctx->popup_rect;
            ctx->popup_stack_rect_valid[ctx->popup_stack_count - 1] =
                IMGUI_TRUE;
        }
    }
    if (ctx->tooltip_background_active) {
        if (ctx->last_item_rect.x2 + ctx->style.window_padding.x >
            ctx->tooltip_rect.x2) {
            ctx->tooltip_rect.x2 = ctx->last_item_rect.x2 +
                                   ctx->style.window_padding.x;
        }
        if (ctx->last_item_rect.y2 + ctx->style.window_padding.y >
            ctx->tooltip_rect.y2) {
            ctx->tooltip_rect.y2 = ctx->last_item_rect.y2 +
                                   ctx->style.window_padding.y;
        }
        if (ctx->clip_rect.x2 < ctx->tooltip_rect.x2) {
            ctx->clip_rect.x2 = ctx->tooltip_rect.x2;
            if (ctx->clip_rect.x2 > ctx->frame_desc.display_size.x) {
                ctx->clip_rect.x2 = ctx->frame_desc.display_size.x;
            }
        }
        if (ctx->clip_rect.y2 < ctx->tooltip_rect.y2) {
            ctx->clip_rect.y2 = ctx->tooltip_rect.y2;
            if (ctx->clip_rect.y2 > ctx->frame_desc.display_size.y) {
                ctx->clip_rect.y2 = ctx->frame_desc.display_size.y;
            }
        }
    }
    if (ctx->last_item_rect.x2 > ctx->content_max_x) {
        ctx->content_max_x = ctx->last_item_rect.x2;
    }
    if (ctx->last_item_rect.y2 > ctx->content_max_y) {
        ctx->content_max_y = ctx->last_item_rect.y2;
    }
    ctx->last_item_clip_rect = ctx->clip_rect;
    current_window_id = (ctx->current_window_index >= 0 &&
                         ctx->current_window_index < ctx->window_count) ?
                        ctx->windows[ctx->current_window_index].id : 0;
    if (id != 0 && ctx->focused_item_valid &&
        id == ctx->focused_item_id && current_window_id != 0) {
        ctx->navigation_window_id = current_window_id;
        ctx->navigation_focused_item_seen = IMGUI_TRUE;
    }
    disabled = ctx->disabled_depth > 0 ? IMGUI_TRUE : IMGUI_FALSE;
    ctx->last_item_disabled = disabled;
    /* A widget can become disabled while it still owns keyboard focus or a
       mouse drag from the previous frame. Dear ImGui drops that ownership;
       retaining it here would let a later key press activate an unavailable
       item or keep capture stuck until another click. */
    if (disabled && ctx->focused_item_valid &&
        ctx->focused_item_id == id) {
        ctx->focused_item_valid = IMGUI_FALSE;
        ctx->navigation_focused_rect_valid = IMGUI_FALSE;
    }
    if (disabled && ctx->active_item_valid &&
        ctx->active_item_id == id) {
        ctx->active_item_valid = IMGUI_FALSE;
        ctx->active_item_id = 0;
        ctx->active_item_was_edited = IMGUI_FALSE;
    }
    if (!disabled && id != 0 && ctx->focus_request >= 0) {
        if (ctx->focus_request == 0) {
            ctx->focused_item_id = id;
            ctx->focused_item_valid = IMGUI_TRUE;
            ctx->focus_request = -1;
        } else {
            --ctx->focus_request;
        }
    }
    /* Inactive/collapsed windows still allow callers to issue widget calls,
       but their items are not part of Dear ImGui's navigation order.  Keep
       those hidden items from stealing Tab/D-pad focus from the visible
       window. */
    if (id != 0 && !disabled && ctx->window_active &&
        (ctx->window_flags & IMGUI_WINDOW_NO_NAV_INPUTS) == 0 &&
        (ctx->window_flags & IMGUI_WINDOW_NO_NAV_FOCUS) == 0 &&
        (ctx->navigation_window_id == 0 ||
         !ctx->navigation_focused_item_seen ||
         current_window_id == ctx->navigation_window_id)) {
        if (imgui_navigation_reserve(ctx, 1) == IMGUI_RESULT_OK) {
            ctx->navigation_item_ids[ctx->navigation_item_count] = id;
            ctx->navigation_item_rects[ctx->navigation_item_count] =
                ctx->last_item_rect;
            ctx->navigation_item_count += 1;
        } else {
            imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                                  "navigation item storage allocation failed");
        }
        if (ctx->focused_item_valid && id == ctx->focused_item_id) {
            ctx->navigation_focused_rect = ctx->last_item_rect;
            ctx->navigation_focused_rect_valid = IMGUI_TRUE;
        }
        if (!ctx->navigation_first_valid) {
            ctx->navigation_first_id = id;
            ctx->navigation_first_valid = IMGUI_TRUE;
        }
        if (ctx->navigation_tab_pending) {
            if (ctx->navigation_reverse_pending &&
                ctx->focused_item_valid && id == ctx->focused_item_id) {
                if (ctx->navigation_last_valid) {
                    ctx->navigation_candidate_id = ctx->navigation_last_id;
                    ctx->navigation_candidate_valid = IMGUI_TRUE;
                }
            } else if (!ctx->navigation_reverse_pending &&
                       ctx->focused_item_valid && id == ctx->focused_item_id) {
                ctx->navigation_tab_seen_focus = IMGUI_TRUE;
            } else if (ctx->navigation_tab_seen_focus &&
                       !ctx->navigation_candidate_valid) {
                ctx->navigation_candidate_id = id;
                ctx->navigation_candidate_valid = IMGUI_TRUE;
            }
        }
        ctx->navigation_last_id = id;
        ctx->navigation_last_valid = IMGUI_TRUE;
    }
    mouse_window_active = ctx->window_active;
    if (ctx->current_window_index >= 0 &&
        (!ctx->focused_window_valid ||
         ctx->focused_window_id !=
         ctx->windows[ctx->current_window_index].id)) {
        mouse_window_active = imgui_window_is_mouse_topmost(
            ctx, ctx->current_window_index);
    }
    ctx->last_item_hovered = !disabled && mouse_window_active &&
        ctx->input.focused &&
        (ctx->window_flags & IMGUI_WINDOW_NO_MOUSE_INPUTS) == 0 &&
        ctx->input.mouse_x >= ctx->clip_rect.x1 &&
        ctx->input.mouse_x < ctx->clip_rect.x2 &&
        ctx->input.mouse_y >= ctx->clip_rect.y1 &&
        ctx->input.mouse_y < ctx->clip_rect.y2 &&
        ctx->input.mouse_x >= ctx->last_item_rect.x1 &&
        ctx->input.mouse_x < ctx->last_item_rect.x2 &&
        ctx->input.mouse_y >= ctx->last_item_rect.y1 &&
        ctx->input.mouse_y < ctx->last_item_rect.y2;
    if (ctx->last_item_hovered && ctx->popup_open &&
        ctx->current_popup_id == 0) {
        if (ctx->popup_modal) {
            ctx->last_item_hovered = IMGUI_FALSE;
        } else if (ctx->popup_rect_valid &&
                   ctx->input.mouse_x >= ctx->popup_rect.x1 &&
                   ctx->input.mouse_x < ctx->popup_rect.x2 &&
                   ctx->input.mouse_y >= ctx->popup_rect.y1 &&
                   ctx->input.mouse_y < ctx->popup_rect.y2) {
            ctx->last_item_hovered = IMGUI_FALSE;
        }
    }
    mouse_clicked = ctx->last_item_hovered &&
                    ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT];
    if (mouse_clicked && ctx->text_input_active &&
        ctx->active_text_id != 0 && ctx->active_text_id != id) {
        ctx->text_input_active = IMGUI_FALSE;
        ctx->active_text_id = 0;
        ctx->text_edit_id = 0;
        ctx->text_selection_start_byte = 0;
        ctx->text_selection_end_byte = 0;
        ctx->text_selection_anchor_byte = 0;
    }
    mouse_released = ctx->last_item_hovered &&
                     ctx->mouse_released[IMGUI_MOUSE_BUTTON_LEFT] &&
                     ctx->previous_active_item_valid &&
                     ctx->previous_active_item_id == id;
    if (mouse_clicked && id != 0) {
        ctx->focused_item_id = id;
        ctx->focused_item_valid = IMGUI_TRUE;
        if ((ctx->window_flags & IMGUI_WINDOW_NO_NAV_FOCUS) == 0) {
            ctx->navigation_window_id = current_window_id;
        }
    }
    shortcut_activated = ctx->next_item_shortcut_valid &&
        imgui_is_shortcut_pressed(ctx, ctx->next_item_shortcut_key,
                                  ctx->next_item_shortcut_modifiers,
                                  IMGUI_FALSE);
    if (ctx->next_item_shortcut_valid) {
        ctx->next_item_shortcut_valid = IMGUI_FALSE;
    }
    keyboard_activated = !disabled && ctx->focused_item_valid && id != 0 &&
                         ctx->focused_item_id == id &&
                         (ctx->input.keys_pressed[IMGUI_KEY_ENTER] ||
                          ctx->input.keys_pressed[IMGUI_KEY_SPACE] ||
                          ctx->input.keys_pressed[
                              IMGUI_KEY_GAMEPAD_FACE_DOWN]);
    /* Buttons and selectable-style widgets use Dear ImGui's default
       PressedOnClickRelease behavior.  Mouse-down still establishes active
       state for drag widgets, but activation is reported on release while
       hovered. */
    ctx->last_item_clicked = mouse_released || keyboard_activated ||
                             (!disabled && shortcut_activated);
    if (mouse_clicked || ctx->last_item_clicked) {
        ctx->item_clicked_this_frame = IMGUI_TRUE;
    }
    ctx->last_item_active = !disabled &&
        ((ctx->last_item_hovered &&
         ctx->input.mouse_down[IMGUI_MOUSE_BUTTON_LEFT]) ||
        (ctx->active_item_valid && ctx->active_item_id == id &&
         ctx->input.mouse_down[IMGUI_MOUSE_BUTTON_LEFT]));
    ctx->last_item_activated = ctx->last_item_clicked;
    ctx->last_item_deactivated = ctx->previous_active_item_valid &&
        ctx->previous_active_item_id == id && !ctx->last_item_active;
    ctx->last_item_deactivated_after_edit = ctx->last_item_deactivated &&
        ctx->previous_active_item_was_edited;
    ctx->last_item_edited = IMGUI_FALSE;
    /* Once a press owns the active ID, later hovered items must not steal it
       while the pointer is held (Dear ImGui keeps the original owner until
       release, even if the cursor crosses other widgets). */
    if (ctx->last_item_active && id != 0 &&
        (!ctx->active_item_valid || ctx->active_item_id == id)) {
        ctx->active_item_id = id;
        ctx->active_item_valid = IMGUI_TRUE;
    }
    item_spacing = ctx->line_spacing_override_valid ?
                   ctx->line_spacing_override : ctx->item_spacing;
    ctx->cursor.y += size.y + item_spacing;
    ctx->line_spacing_override_valid = IMGUI_FALSE;
    if (ctx->last_item_hovered) ctx->any_item_hovered = IMGUI_TRUE;
    if (ctx->last_item_active) ctx->any_item_active = IMGUI_TRUE;
    if (ctx->focused_item_valid && id != 0 &&
        ctx->focused_item_id == id && !disabled) {
        ctx->any_item_focused = IMGUI_TRUE;
    }
    return IMGUI_TRUE;
}

static imgui_vec2 imgui_label_size(const imgui_context *ctx,
                                   const char *label)
{
    const char *hidden;
    imgui_vec2 measured;
    float padding_x;
    padding_x = ctx != NULL ? ctx->style.frame_padding.x : 8.0f;
    if (label == NULL) return imgui_make_vec2(2.0f * padding_x, 22.0f);
    hidden = label != NULL ? strstr(label, "##") : NULL;
    if (ctx != NULL && ctx->font != NULL) {
        measured = imgui_font_measure_text(ctx->font, label, hidden, 0.0f);
    } else {
        measured = imgui_calc_text_size(label, hidden, 0.0f);
    }
    return imgui_make_vec2(measured.x + 2.0f * padding_x, 22.0f);
}

static float imgui_label_measure_width(const imgui_context *ctx,
                                       const char *label)
{
    const char *hidden;
    imgui_vec2 measured;
    if (label == NULL) return 0.0f;
    hidden = strstr(label, "##");
    if (ctx != NULL && ctx->font != NULL) {
        measured = imgui_font_measure_text(ctx->font, label, hidden, 0.0f);
    } else {
        measured = imgui_calc_text_size(label, hidden, 0.0f);
    }
    return measured.x;
}

static float imgui_item_label_y(const imgui_context *ctx,
                                imgui_rect rect)
{
    float line_height;
    if (ctx != NULL && ctx->font != NULL) {
        line_height = imgui_font_get_line_height(ctx->font);
        if (line_height < rect.y2 - rect.y1) {
            return rect.y1 + ((rect.y2 - rect.y1) - line_height) * 0.5f;
        }
    }
    return rect.y1 + (ctx != NULL ? ctx->style.frame_padding.y : 0.0f);
}

imgui_bool imgui_arrow_button(imgui_context *ctx, imgui_id id,
                              imgui_arrow_direction direction)
{
    imgui_vec2 center;
    imgui_vec2 first;
    imgui_vec2 second;
    imgui_vec2 third;
    imgui_u32 color;
    if (ctx == NULL || direction < IMGUI_ARROW_LEFT ||
        direction > IMGUI_ARROW_DOWN ||
        !imgui_item_register(ctx, id, imgui_make_vec2(22.0f, 22.0f))) {
        return IMGUI_FALSE;
    }
    color = ctx->last_item_active ? ctx->style.color_button_active :
            (ctx->last_item_hovered ? ctx->style.color_button_hovered :
                                      ctx->style.color_button);
    if (ctx->style.frame_rounding > 0.0f) {
        (void)imgui_draw_list_add_rect_rounded(
            ctx, &ctx->default_draw_list, ctx->last_item_rect,
            ctx->style.frame_rounding, color, 4, NULL);
    } else {
        (void)imgui_mesh_add_rect(ctx, ctx->last_item_rect, color);
    }
    imgui_draw_frame_border(ctx, ctx->last_item_rect);
    center = imgui_make_vec2((ctx->last_item_rect.x1 +
                              ctx->last_item_rect.x2) * 0.5f,
                             (ctx->last_item_rect.y1 +
                              ctx->last_item_rect.y2) * 0.5f);
    if (direction == IMGUI_ARROW_LEFT) {
        first = imgui_make_vec2(center.x - 5.0f, center.y);
        second = imgui_make_vec2(center.x + 3.0f, center.y - 5.0f);
        third = imgui_make_vec2(center.x + 3.0f, center.y + 5.0f);
    } else if (direction == IMGUI_ARROW_RIGHT) {
        first = imgui_make_vec2(center.x + 5.0f, center.y);
        second = imgui_make_vec2(center.x - 3.0f, center.y - 5.0f);
        third = imgui_make_vec2(center.x - 3.0f, center.y + 5.0f);
    } else if (direction == IMGUI_ARROW_UP) {
        first = imgui_make_vec2(center.x, center.y - 5.0f);
        second = imgui_make_vec2(center.x - 5.0f, center.y + 3.0f);
        third = imgui_make_vec2(center.x + 5.0f, center.y + 3.0f);
    } else {
        first = imgui_make_vec2(center.x, center.y + 5.0f);
        second = imgui_make_vec2(center.x - 5.0f, center.y - 3.0f);
        third = imgui_make_vec2(center.x + 5.0f, center.y - 3.0f);
    }
    (void)imgui_mesh_add_triangle(ctx, first, second, third,
                                  ctx->style.color_text);
    return ctx->last_item_clicked;
}

imgui_bool imgui_button(imgui_context *ctx, const char *label)
{
    return imgui_button_sized(ctx, label, imgui_label_size(ctx, label));
}

imgui_bool imgui_button_sized(imgui_context *ctx,
                              const char *label,
                              imgui_vec2 size)
{
    imgui_id id;
    imgui_u32 color;
    imgui_vec2 label_size;
    const char *label_end;
    float label_x;
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    if (!imgui_item_register(ctx, id, size)) return IMGUI_FALSE;
    color = ctx->last_item_active ? ctx->style.color_button_active :
            (ctx->last_item_hovered ? ctx->style.color_button_hovered :
                                      ctx->style.color_button);
    if (ctx->style.frame_rounding > 0.0f) {
        (void)imgui_draw_list_add_rect_rounded(
            ctx, &ctx->default_draw_list, ctx->last_item_rect,
            ctx->style.frame_rounding, color, 4, NULL);
    } else {
        (void)imgui_mesh_add_rect(ctx, ctx->last_item_rect, color);
    }
    imgui_draw_frame_border(ctx, ctx->last_item_rect);
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        label_end = imgui_label_visible_end(label);
        label_size = imgui_font_measure_text(ctx->font, label, label_end,
                                             0.0f);
        label_x = ctx->last_item_rect.x1 +
                  (ctx->last_item_rect.x2 - ctx->last_item_rect.x1 -
                   label_size.x) * 0.5f;
        if (label_x < ctx->last_item_rect.x1 + ctx->style.frame_padding.x) {
            label_x = ctx->last_item_rect.x1 + ctx->style.frame_padding.x;
        }
        imgui_text_draw_font(ctx, label, label_end,
                             imgui_make_vec2(label_x,
                                             imgui_item_label_y(
                                                 ctx, ctx->last_item_rect)));
    }
    return ctx->last_item_clicked;
}

imgui_bool imgui_button_with_id(imgui_context *ctx,
                                imgui_id id,
                                const char *label)
{
    imgui_vec2 size;
    imgui_u32 color;
    imgui_vec2 label_size;
    const char *label_end;
    float label_x;
    size = imgui_label_size(ctx, label);
    if (!imgui_item_register(ctx, id, size)) return IMGUI_FALSE;
    color = ctx->last_item_active ? ctx->style.color_button_active :
            (ctx->last_item_hovered ? ctx->style.color_button_hovered :
                                      ctx->style.color_button);
    if (ctx->style.frame_rounding > 0.0f) {
        (void)imgui_draw_list_add_rect_rounded(
            ctx, &ctx->default_draw_list, ctx->last_item_rect,
            ctx->style.frame_rounding, color, 4, NULL);
    } else {
        (void)imgui_mesh_add_rect(ctx, ctx->last_item_rect, color);
    }
    imgui_draw_frame_border(ctx, ctx->last_item_rect);
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        label_end = imgui_label_visible_end(label);
        label_size = imgui_font_measure_text(ctx->font, label, label_end,
                                             0.0f);
        label_x = ctx->last_item_rect.x1 +
                  (ctx->last_item_rect.x2 - ctx->last_item_rect.x1 -
                   label_size.x) * 0.5f;
        if (label_x < ctx->last_item_rect.x1 + ctx->style.frame_padding.x) {
            label_x = ctx->last_item_rect.x1 + ctx->style.frame_padding.x;
        }
        imgui_text_draw_font(ctx, label, label_end,
                             imgui_make_vec2(label_x,
                                             imgui_item_label_y(
                                                 ctx, ctx->last_item_rect)));
    }
    return ctx->last_item_clicked;
}

imgui_bool imgui_button_localized(imgui_context *ctx, const char *key)
{
    const char *label;
    imgui_id id;
    label = imgui_localize(ctx, key);
    id = imgui_get_id_string(ctx, key != NULL ? key : "");
    return imgui_button_with_id(ctx, id, label);
}

imgui_bool imgui_small_button(imgui_context *ctx, const char *label)
{
    return imgui_button_sized(ctx, label, imgui_make_vec2(
        imgui_label_measure_width(ctx, label) + 12.0f, 18.0f));
}

imgui_bool imgui_invisible_button(imgui_context *ctx,
                                  imgui_id id,
                                  imgui_vec2 size,
                                  imgui_flags flags)
{
    (void)flags;
    if (!imgui_item_register(ctx, id, size)) return IMGUI_FALSE;
    /* InvisibleButton is an interaction primitive only.  It must reserve
       layout space and update item state without submitting any visible
       geometry; callers can draw their own hit target through the draw list. */
    return ctx->last_item_clicked;
}

imgui_bool imgui_checkbox(imgui_context *ctx,
                          const char *label,
                          imgui_bool *value)
{
    imgui_id id;
    imgui_vec2 size;
    imgui_rect box;
    if (value == NULL) return IMGUI_FALSE;
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    size = imgui_make_vec2(imgui_label_measure_width(ctx, label) + 30.0f,
                           22.0f);
    if (!imgui_item_register(ctx, id, size)) return IMGUI_FALSE;
    box.x1 = ctx->last_item_rect.x1;
    box.y1 = ctx->last_item_rect.y1 + 2.0f;
    box.x2 = box.x1 + 18.0f;
    box.y2 = box.y1 + 18.0f;
    {
        imgui_u32 box_color = ctx->last_item_active ?
            ctx->style.color_frame_active :
            (ctx->last_item_hovered ? ctx->style.color_frame_hovered :
             (*value ? ctx->style.color_frame_active :
                        ctx->style.color_frame));
        if (ctx->style.frame_rounding > 0.0f) {
            (void)imgui_draw_list_add_rect_rounded(
                ctx, &ctx->default_draw_list, box,
                ctx->style.frame_rounding, box_color, 4, NULL);
        } else {
            (void)imgui_mesh_add_rect(ctx, box, box_color);
        }
        imgui_draw_frame_border(ctx, box);
    }
    if (*value) {
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(box.x1 + 4.0f, box.y1 + 9.0f),
            imgui_make_vec2(box.x1 + 8.0f, box.y1 + 13.0f),
            ctx->style.color_check_mark, 2.0f);
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(box.x1 + 8.0f, box.y1 + 13.0f),
            imgui_make_vec2(box.x1 + 15.0f, box.y1 + 5.0f),
            ctx->style.color_check_mark, 2.0f);
    }
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        const char *hidden = strstr(label, "##");
        imgui_text_draw_font(ctx, label, hidden != NULL ? hidden :
                             label + strlen(label), imgui_make_vec2(
                                 ctx->last_item_rect.x1 + 26.0f,
                                 imgui_item_label_y(ctx,
                                                    ctx->last_item_rect)));
    }
    if (ctx->last_item_clicked) {
        *value = *value ? IMGUI_FALSE : IMGUI_TRUE;
        ctx->last_item_edited = IMGUI_TRUE;
    }
    return ctx->last_item_clicked;
}

imgui_bool imgui_checkbox_flags(imgui_context *ctx,
                                const char *label,
                                imgui_flags *flags,
                                imgui_flags value)
{
    imgui_bool enabled;
    if (flags == NULL) return IMGUI_FALSE;
    enabled = (*flags & value) != 0 ? IMGUI_TRUE : IMGUI_FALSE;
    if (imgui_checkbox(ctx, label, &enabled)) {
        if (enabled) *flags |= value;
        else *flags &= ~value;
        return IMGUI_TRUE;
    }
    return IMGUI_FALSE;
}

imgui_bool imgui_radio_button(imgui_context *ctx,
                              const char *label,
                              imgui_bool active)
{
    imgui_id id;
    imgui_vec2 size;
    imgui_vec2 center;
    imgui_u32 frame_color;
    if (ctx == NULL) return IMGUI_FALSE;
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    size = imgui_label_size(ctx, label);
    /* The control is drawn at x+10 and the label at x+20. Include that
       complete control-to-label gap in the hit rectangle, matching checkbox
       and button label extents. */
    size.x += 4.0f;
    if (!imgui_item_register(ctx, id, size)) return IMGUI_FALSE;
    center = imgui_make_vec2(ctx->last_item_rect.x1 + 10.0f,
                             ctx->last_item_rect.y1 + size.y * 0.5f);
    /* Radio controls are a framed disk with a smaller active indicator.  A
       single filled disk made inactive radios look selected and diverged
       visibly from Dear ImGui. */
    frame_color = ctx->last_item_active ? ctx->style.color_frame_active :
                  (ctx->last_item_hovered ? ctx->style.color_frame_hovered :
                                            ctx->style.color_frame);
    (void)imgui_draw_list_add_circle(
        ctx, &ctx->default_draw_list, center, 7.0f,
        frame_color, 12);
    (void)imgui_draw_list_add_circle(
        ctx, &ctx->default_draw_list, center, 4.5f,
        ctx->style.color_window_background, 12);
    if (active) {
        (void)imgui_draw_list_add_circle(
            ctx, &ctx->default_draw_list, center, 3.0f,
            ctx->style.color_check_mark, 12);
    }
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        const char *hidden = strstr(label, "##");
        imgui_text_draw_font(ctx, label, hidden != NULL ? hidden :
                             label + strlen(label), imgui_make_vec2(
                                 ctx->last_item_rect.x1 + 20.0f,
                                 imgui_item_label_y(ctx,
                                                    ctx->last_item_rect)));
    }
    return ctx->last_item_clicked;
}

imgui_bool imgui_radio_button_value(imgui_context *ctx,
                                    const char *label,
                                    int *value,
                                    int button_value)
{
    imgui_bool active;
    if (value == NULL) return IMGUI_FALSE;
    active = *value == button_value ? IMGUI_TRUE : IMGUI_FALSE;
    if (imgui_radio_button(ctx, label, active)) {
        *value = button_value;
        return IMGUI_TRUE;
    }
    return IMGUI_FALSE;
}

void imgui_progress_bar(imgui_context *ctx, float fraction)
{
    imgui_progress_bar_ex(ctx, fraction, imgui_make_vec2(220.0f, 22.0f),
                          NULL);
}

void imgui_progress_bar_ex(imgui_context *ctx,
                           float fraction,
                           imgui_vec2 size,
                           const char *overlay)
{
    imgui_rect rect;
    imgui_rect fill;
    /* NaN compares false to both bounds and would otherwise turn the fill
       rectangle into NaN vertices.  Widget arguments follow the same
       finite-input contract as frame/style/layout descriptors. */
    if (ctx == NULL || !imgui_float_is_finite(fraction) ||
        !imgui_float_is_finite(size.x) || !imgui_float_is_finite(size.y)) {
        return;
    }
    if (fraction < 0.0f) fraction = 0.0f;
    if (fraction > 1.0f) fraction = 1.0f;
    if (size.x <= 0.0f) size.x = 220.0f;
    if (size.y <= 0.0f) size.y = 22.0f;
    if (!imgui_item_register(ctx, 0, size)) return;
    rect = ctx->last_item_rect;
    (void)imgui_add_frame_surface(ctx, rect, ctx->style.color_frame);
    fill = rect;
    fill.x2 = fill.x1 + (fill.x2 - fill.x1) * fraction;
    if (fill.x2 > fill.x1) {
        (void)imgui_add_frame_surface(ctx, fill,
                                      ctx->style.color_plot_histogram);
    }
    if (ctx->font != NULL && ctx->font_texture != NULL && overlay != NULL) {
        imgui_vec2 overlay_size;
        overlay_size = imgui_font_measure_text(ctx->font, overlay, NULL, 0.0f);
        imgui_text_draw_font(ctx, overlay,
                             overlay + strlen(overlay), imgui_make_vec2(
                                 rect.x1 + (rect.x2 - rect.x1 - overlay_size.x) *
                                 0.5f,
                                 imgui_item_label_y(ctx, rect)));
    }
}

void imgui_plot_lines(imgui_context *ctx,
                      const char *label,
                      const float *values,
                      size_t count,
                      imgui_vec2 size,
                      float minimum,
                      float maximum)
{
    imgui_vec2 previous;
    imgui_vec2 current;
    imgui_id id;
    imgui_rect rect;
    imgui_rect saved_clip;
    float fraction;
    size_t index;
    if (ctx == NULL || values == NULL || count == 0 ||
        !imgui_float_is_finite(size.x) || !imgui_float_is_finite(size.y) ||
        !imgui_float_is_finite(minimum) ||
        !imgui_float_is_finite(maximum)) return;
    for (index = 0; index < count; ++index) {
        if (!imgui_float_is_finite(values[index])) return;
    }
    if (size.x <= 0.0f) size.x = 220.0f;
    if (size.y <= 0.0f) size.y = 80.0f;
    if (maximum <= minimum) {
        minimum = 0.0f;
        maximum = 1.0f;
    }
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    if (!imgui_item_register(ctx, id, size)) return;
    rect = ctx->last_item_rect;
    (void)imgui_add_frame_surface(ctx, rect, ctx->style.color_frame);
    saved_clip = ctx->clip_rect;
    if (rect.x1 > ctx->clip_rect.x1) ctx->clip_rect.x1 = rect.x1;
    if (rect.y1 > ctx->clip_rect.y1) ctx->clip_rect.y1 = rect.y1;
    if (rect.x2 < ctx->clip_rect.x2) ctx->clip_rect.x2 = rect.x2;
    if (rect.y2 < ctx->clip_rect.y2) ctx->clip_rect.y2 = rect.y2;
    previous.x = rect.x1;
    fraction = (values[0] - minimum) / (maximum - minimum);
    if (fraction < 0.0f) fraction = 0.0f;
    if (fraction > 1.0f) fraction = 1.0f;
    previous.y = rect.y2 - fraction * (rect.y2 - rect.y1);
    for (index = 1; index < count; ++index) {
        current.x = rect.x1 + (float)index * (rect.x2 - rect.x1) /
                    (float)(count - 1);
        fraction = (values[index] - minimum) / (maximum - minimum);
        if (fraction < 0.0f) fraction = 0.0f;
        if (fraction > 1.0f) fraction = 1.0f;
        current.y = rect.y2 - fraction * (rect.y2 - rect.y1);
        (void)imgui_draw_list_add_line(ctx, &ctx->default_draw_list,
                                       previous, current,
                                       ctx->style.color_plot_lines,
                                       1.0f);
        previous = current;
    }
    ctx->clip_rect = saved_clip;
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        const char *hidden = strstr(label, "##");
        float label_x = rect.x2 + ctx->style.item_spacing;
        imgui_text_draw_font(ctx, label, hidden != NULL ? hidden :
                             label + strlen(label), imgui_make_vec2(
                                 label_x,
                                 imgui_item_label_y(ctx, rect)));
        if (label_x + imgui_label_measure_width(ctx, label) >
            ctx->content_max_x) {
            ctx->content_max_x = label_x +
                                 imgui_label_measure_width(ctx, label);
        }
    }
}

void imgui_plot_histogram(imgui_context *ctx,
                          const char *label,
                          const float *values,
                          size_t count,
                          imgui_vec2 size,
                          float minimum,
                          float maximum)
{
    imgui_rect rect;
    imgui_rect bar;
    imgui_rect saved_clip;
    imgui_id id;
    float range;
    float fraction;
    float baseline;
    float bar_width;
    size_t index;
    if (ctx == NULL || values == NULL || count == 0 ||
        !imgui_float_is_finite(size.x) || !imgui_float_is_finite(size.y) ||
        !imgui_float_is_finite(minimum) ||
        !imgui_float_is_finite(maximum)) return;
    for (index = 0; index < count; ++index) {
        if (!imgui_float_is_finite(values[index])) return;
    }
    if (size.x <= 0.0f) size.x = 220.0f;
    if (size.y <= 0.0f) size.y = 80.0f;
    if (maximum <= minimum) {
        minimum = 0.0f;
        maximum = 1.0f;
    }
    range = maximum - minimum;
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    if (!imgui_item_register(ctx, id, size)) return;
    rect = ctx->last_item_rect;
    (void)imgui_add_frame_surface(ctx, rect, ctx->style.color_frame);
    saved_clip = ctx->clip_rect;
    if (rect.x1 > ctx->clip_rect.x1) ctx->clip_rect.x1 = rect.x1;
    if (rect.y1 > ctx->clip_rect.y1) ctx->clip_rect.y1 = rect.y1;
    if (rect.x2 < ctx->clip_rect.x2) ctx->clip_rect.x2 = rect.x2;
    if (rect.y2 < ctx->clip_rect.y2) ctx->clip_rect.y2 = rect.y2;
    baseline = rect.y2 - ((0.0f - minimum) / range) *
               (rect.y2 - rect.y1);
    if (baseline < rect.y1) baseline = rect.y1;
    if (baseline > rect.y2) baseline = rect.y2;
    bar_width = (rect.x2 - rect.x1) / (float)count;
    for (index = 0; index < count; ++index) {
        fraction = (values[index] - minimum) / range;
        if (fraction < 0.0f) fraction = 0.0f;
        if (fraction > 1.0f) fraction = 1.0f;
        bar.x1 = rect.x1 + (float)index * bar_width + 0.5f;
        bar.x2 = rect.x1 + (float)(index + 1) * bar_width - 0.5f;
        if (bar.x2 < bar.x1) bar.x2 = bar.x1;
        bar.y1 = rect.y2 - fraction * (rect.y2 - rect.y1);
        bar.y2 = baseline;
        if (bar.y1 > bar.y2) {
            float swap_y = bar.y1;
            bar.y1 = bar.y2;
            bar.y2 = swap_y;
        }
        if (bar.y2 > bar.y1) {
            (void)imgui_mesh_add_rect(ctx, bar,
                                      ctx->style.color_plot_histogram);
        }
    }
    ctx->clip_rect = saved_clip;
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        const char *hidden = strstr(label, "##");
        float label_x = rect.x2 + ctx->style.item_spacing;
        imgui_text_draw_font(ctx, label, hidden != NULL ? hidden :
                             label + strlen(label), imgui_make_vec2(
                                 label_x,
                                 imgui_item_label_y(ctx, rect)));
        if (label_x + imgui_label_measure_width(ctx, label) >
            ctx->content_max_x) {
            ctx->content_max_x = label_x +
                                 imgui_label_measure_width(ctx, label);
        }
    }
}

static imgui_u32 imgui_color_pack_rgba(const float rgba[4])
{
    int red;
    int green;
    int blue;
    int alpha;
    float value;
    value = rgba[0] < 0.0f ? 0.0f : (rgba[0] > 1.0f ? 1.0f : rgba[0]);
    red = (int)(value * 255.0f + 0.5f);
    value = rgba[1] < 0.0f ? 0.0f : (rgba[1] > 1.0f ? 1.0f : rgba[1]);
    green = (int)(value * 255.0f + 0.5f);
    value = rgba[2] < 0.0f ? 0.0f : (rgba[2] > 1.0f ? 1.0f : rgba[2]);
    blue = (int)(value * 255.0f + 0.5f);
    value = rgba[3] < 0.0f ? 0.0f : (rgba[3] > 1.0f ? 1.0f : rgba[3]);
    alpha = (int)(value * 255.0f + 0.5f);
    /* Render colors use Dear ImGui's IM_COL32 layout: red in the low
       byte, then green, blue, and alpha in the high byte. */
    return (imgui_u32)red | ((imgui_u32)green << 8) |
           ((imgui_u32)blue << 16) | ((imgui_u32)alpha << 24);
}

static void imgui_color_rgb_to_hsv_internal(const float rgb[3], float *hue,
                                   float *saturation, float *value)
{
    float maximum;
    float minimum;
    float delta;
    float result_hue;
    maximum = rgb[0] > rgb[1] ? rgb[0] : rgb[1];
    if (rgb[2] > maximum) maximum = rgb[2];
    minimum = rgb[0] < rgb[1] ? rgb[0] : rgb[1];
    if (rgb[2] < minimum) minimum = rgb[2];
    delta = maximum - minimum;
    result_hue = 0.0f;
    if (delta > 0.00001f) {
        if (maximum == rgb[0]) result_hue = (rgb[1] - rgb[2]) / delta;
        else if (maximum == rgb[1]) result_hue = 2.0f +
            (rgb[2] - rgb[0]) / delta;
        else result_hue = 4.0f + (rgb[0] - rgb[1]) / delta;
        result_hue /= 6.0f;
        if (result_hue < 0.0f) result_hue += 1.0f;
    }
    if (hue != NULL) *hue = result_hue;
    if (saturation != NULL) *saturation = maximum > 0.0f ?
        delta / maximum : 0.0f;
    if (value != NULL) *value = maximum;
}

static void imgui_color_hsv_to_rgb_internal(float hue, float saturation, float value,
                                   float rgb[3])
{
    int sector;
    float fraction;
    float p;
    float q;
    float t;
    if (saturation <= 0.0f) {
        rgb[0] = value; rgb[1] = value; rgb[2] = value;
        return;
    }
    if (hue < 0.0f) hue = 0.0f;
    while (hue >= 1.0f) hue -= 1.0f;
    hue *= 6.0f;
    sector = (int)floor((double)hue);
    fraction = hue - (float)sector;
    p = value * (1.0f - saturation);
    q = value * (1.0f - saturation * fraction);
    t = value * (1.0f - saturation * (1.0f - fraction));
    switch (sector) {
    case 0: rgb[0] = value; rgb[1] = t; rgb[2] = p; break;
    case 1: rgb[0] = q; rgb[1] = value; rgb[2] = p; break;
    case 2: rgb[0] = p; rgb[1] = value; rgb[2] = t; break;
    case 3: rgb[0] = p; rgb[1] = q; rgb[2] = value; break;
    case 4: rgb[0] = t; rgb[1] = p; rgb[2] = value; break;
    default: rgb[0] = value; rgb[1] = p; rgb[2] = q; break;
    }
}

imgui_u32 imgui_color_rgba_to_u32(const float rgba[4])
{
    int index;
    if (rgba == NULL) return 0;
    for (index = 0; index < 4; ++index) {
        if (!imgui_float_is_finite(rgba[index])) return 0;
    }
    return imgui_color_pack_rgba(rgba);
}

void imgui_color_u32_to_rgba(imgui_u32 color, float rgba[4])
{
    if (rgba == NULL) return;
    rgba[0] = (float)(color & 0xffU) / 255.0f;
    rgba[1] = (float)((color >> 8) & 0xffU) / 255.0f;
    rgba[2] = (float)((color >> 16) & 0xffU) / 255.0f;
    rgba[3] = (float)((color >> 24) & 0xffU) / 255.0f;
}

void imgui_color_rgb_to_hsv(const float rgb[3], float *hue,
                            float *saturation, float *value)
{
    if (rgb == NULL) return;
    imgui_color_rgb_to_hsv_internal(rgb, hue, saturation, value);
}

void imgui_color_hsv_to_rgb(float hue, float saturation, float value,
                            float rgb[3])
{
    if (rgb == NULL || !imgui_float_is_finite(hue) ||
        !imgui_float_is_finite(saturation) || !imgui_float_is_finite(value)) {
        return;
    }
    imgui_color_hsv_to_rgb_internal(hue, saturation, value, rgb);
}

/* Render the scalar portion of a color editor without changing the item
   identity observed by callers.  The fields are ordinary InputFloat widgets
   so they inherit UTF-8 editing, clipboard, undo, and keyboard behavior. */
static void imgui_color_numeric_fields(imgui_context *ctx,
                                        imgui_id parent_id,
                                        float rgba[4],
                                        int channel_count,
                                        imgui_rect field_rect,
                                        float field_y)
{
    imgui_vec2 saved_cursor;
    imgui_rect primary_rect;
    imgui_rect primary_clip_rect;
    imgui_id primary_id;
    imgui_bool primary_hovered;
    imgui_bool primary_active;
    imgui_bool primary_clicked;
    imgui_bool primary_edited;
    imgui_bool primary_activated;
    imgui_bool primary_deactivated;
    imgui_bool primary_deactivated_after_edit;
    imgui_bool primary_disabled;
    imgui_bool any_hovered;
    imgui_bool any_active;
    imgui_bool any_edited;
    float field_width;
    int spacing_count;
    int index;
    const char *labels[4];
    if (ctx == NULL || rgba == NULL || channel_count <= 0 ||
        channel_count > 4) return;
    saved_cursor = ctx->cursor;
    primary_rect = ctx->last_item_rect;
    primary_clip_rect = ctx->last_item_clip_rect;
    primary_id = ctx->last_item_id;
    primary_hovered = ctx->last_item_hovered;
    primary_active = ctx->last_item_active;
    primary_clicked = ctx->last_item_clicked;
    primary_edited = ctx->last_item_edited;
    primary_activated = ctx->last_item_activated;
    primary_deactivated = ctx->last_item_deactivated;
    primary_deactivated_after_edit = ctx->last_item_deactivated_after_edit;
    primary_disabled = ctx->last_item_disabled;
    labels[0] = "##ColorR";
    labels[1] = "##ColorG";
    labels[2] = "##ColorB";
    labels[3] = "##ColorA";
    spacing_count = channel_count > 1 ? channel_count - 1 : 0;
    field_width = (field_rect.x2 - field_rect.x1 -
                   ctx->style.item_spacing * (float)spacing_count) /
                  (float)channel_count;
    if (field_width < 1.0f) field_width = 1.0f;
    ctx->cursor.x = field_rect.x1;
    ctx->cursor.y = field_y;
    any_hovered = IMGUI_FALSE;
    any_active = IMGUI_FALSE;
    any_edited = IMGUI_FALSE;
    imgui_push_id_value(ctx, parent_id);
    for (index = 0; index < channel_count; ++index) {
        imgui_set_next_item_width(ctx, field_width);
        if (imgui_input_float_ex(ctx, labels[index], &rgba[index], "%.3f")) {
            any_edited = IMGUI_TRUE;
        }
        if (ctx->last_item_hovered) any_hovered = IMGUI_TRUE;
        if (ctx->last_item_active) any_active = IMGUI_TRUE;
        if (index + 1 < channel_count) imgui_same_line(ctx);
    }
    imgui_pop_id(ctx);
    ctx->cursor = saved_cursor;
    ctx->last_item_rect = primary_rect;
    ctx->last_item_clip_rect = primary_clip_rect;
    ctx->last_item_id = primary_id;
    ctx->last_item_hovered = primary_hovered || any_hovered;
    ctx->last_item_active = primary_active || any_active;
    ctx->last_item_clicked = primary_clicked;
    ctx->last_item_edited = primary_edited || any_edited;
    ctx->last_item_activated = primary_activated;
    ctx->last_item_deactivated = primary_deactivated;
    ctx->last_item_deactivated_after_edit = primary_deactivated_after_edit;
    ctx->last_item_disabled = primary_disabled;
}

imgui_bool imgui_color_edit_rgba(imgui_context *ctx,
                                 const char *label,
                                 float rgba[4],
                                 imgui_color_edit_flags flags)
{
    imgui_rect segment;
    imgui_id id;
    imgui_u32 color;
    int channel;
    float fraction;
    float old_rgba[4];
    float preview[4];
    float hsv_hue;
    float hsv_saturation;
    float hsv_value;
    float hsv_rgb[3];
    imgui_rect picker_area;
    imgui_rect hue_area;
    int picker_x;
    int picker_y;
    int picker_rows;
    int picker_columns;
    int index;
    int channel_count;
    imgui_bool changed;
    imgui_id item_id;
    if (ctx == NULL || rgba == NULL) return IMGUI_FALSE;
    for (index = 0; index < 4; ++index) {
        if (!imgui_float_is_finite(rgba[index])) return IMGUI_FALSE;
    }
    channel_count = (flags & IMGUI_COLOR_EDIT_NO_ALPHA) != 0 ? 3 : 4;
    for (index = 0; index < 4; ++index) old_rgba[index] = rgba[index];
    for (index = 0; index < channel_count; ++index) {
        if (rgba[index] < 0.0f) rgba[index] = 0.0f;
        if (rgba[index] > 1.0f) rgba[index] = 1.0f;
    }
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    /* In the regular numeric form the channel fields are the actual
       navigable widgets.  Keep the parent ID for scoping, but avoid adding a
       redundant hidden navigation stop for the color editor itself. */
    item_id = ((flags & IMGUI_COLOR_EDIT_PICKER) == 0 &&
               (flags & IMGUI_COLOR_EDIT_NO_INPUTS) == 0) ? 0 : id;
    if (!imgui_item_register(ctx, item_id,
                             (flags & IMGUI_COLOR_EDIT_PICKER) != 0 ?
                             imgui_make_vec2(220.0f,
                                 (flags & IMGUI_COLOR_EDIT_NO_INPUTS) == 0 ?
                                 184.0f : 154.0f) :
                             imgui_make_vec2(220.0f, 22.0f))) {
        return IMGUI_FALSE;
    }
    if ((flags & IMGUI_COLOR_EDIT_PICKER) != 0) {
        imgui_color_rgb_to_hsv_internal(rgba, &hsv_hue, &hsv_saturation,
                                         &hsv_value);
        picker_area.x1 = ctx->last_item_rect.x1;
        picker_area.y1 = ctx->last_item_rect.y1;
        picker_area.x2 = picker_area.x1 + 180.0f;
        picker_area.y2 = picker_area.y1 + 128.0f;
        hue_area.x1 = picker_area.x2 + 6.0f;
        hue_area.y1 = picker_area.y1;
        hue_area.x2 = hue_area.x1 + 24.0f;
        hue_area.y2 = picker_area.y2;
        /* NO_INPUTS hides the numeric fields; it must not disable the
           picker itself.  Dear ImGui still permits HSV-square and hue-strip
           interaction in picker-only mode. */
        if (ctx->last_item_hovered || ctx->last_item_active) {
            if (ctx->input.mouse_x >= picker_area.x1 &&
                ctx->input.mouse_x < picker_area.x2 &&
                ctx->input.mouse_y >= picker_area.y1 &&
                ctx->input.mouse_y < picker_area.y2) {
                hsv_saturation = (ctx->input.mouse_x - picker_area.x1) /
                    (picker_area.x2 - picker_area.x1);
                hsv_value = 1.0f - (ctx->input.mouse_y - picker_area.y1) /
                    (picker_area.y2 - picker_area.y1);
                imgui_color_hsv_to_rgb_internal(hsv_hue, hsv_saturation,
                                                hsv_value, hsv_rgb);
                rgba[0] = hsv_rgb[0]; rgba[1] = hsv_rgb[1]; rgba[2] = hsv_rgb[2];
                ctx->last_item_edited = IMGUI_TRUE;
            } else if (ctx->input.mouse_x >= hue_area.x1 &&
                       ctx->input.mouse_x < hue_area.x2 &&
                       ctx->input.mouse_y >= hue_area.y1 &&
                       ctx->input.mouse_y < hue_area.y2) {
                hsv_hue = (ctx->input.mouse_y - hue_area.y1) /
                    (hue_area.y2 - hue_area.y1);
                imgui_color_hsv_to_rgb_internal(hsv_hue, hsv_saturation,
                                                hsv_value, hsv_rgb);
                rgba[0] = hsv_rgb[0]; rgba[1] = hsv_rgb[1]; rgba[2] = hsv_rgb[2];
                ctx->last_item_edited = IMGUI_TRUE;
            }
        }
        picker_rows = 8;
        picker_columns = 12;
        for (picker_y = 0; picker_y < picker_rows; ++picker_y) {
            for (picker_x = 0; picker_x < picker_columns; ++picker_x) {
                imgui_color_hsv_to_rgb_internal(hsv_hue,
                    (float)picker_x / (float)(picker_columns - 1),
                    1.0f - (float)picker_y / (float)(picker_rows - 1), hsv_rgb);
                segment.x1 = picker_area.x1 + (picker_area.x2 - picker_area.x1) *
                    (float)picker_x / (float)picker_columns;
                segment.x2 = picker_area.x1 + (picker_area.x2 - picker_area.x1) *
                    (float)(picker_x + 1) / (float)picker_columns;
                segment.y1 = picker_area.y1 + (picker_area.y2 - picker_area.y1) *
                    (float)picker_y / (float)picker_rows;
                segment.y2 = picker_area.y1 + (picker_area.y2 - picker_area.y1) *
                    (float)(picker_y + 1) / (float)picker_rows;
                preview[0] = hsv_rgb[0];
                preview[1] = hsv_rgb[1];
                preview[2] = hsv_rgb[2];
                preview[3] = 1.0f;
                (void)imgui_mesh_add_rect(ctx, segment,
                    imgui_color_pack_rgba(preview));
            }
        }
        picker_rows = 6;
        for (picker_y = 0; picker_y < picker_rows; ++picker_y) {
            imgui_color_hsv_to_rgb_internal((float)picker_y /
                                   (float)picker_rows,
                                   1.0f, 1.0f, hsv_rgb);
            segment.x1 = hue_area.x1;
            segment.x2 = hue_area.x2;
            segment.y1 = hue_area.y1 + (hue_area.y2 - hue_area.y1) *
                (float)picker_y / (float)picker_rows;
            segment.y2 = hue_area.y1 + (hue_area.y2 - hue_area.y1) *
                (float)(picker_y + 1) / (float)picker_rows;
            preview[0] = hsv_rgb[0]; preview[1] = hsv_rgb[1];
            preview[2] = hsv_rgb[2]; preview[3] = 1.0f;
            (void)imgui_mesh_add_rect(ctx, segment,
                                       imgui_color_pack_rgba(preview));
        }
    } else if ((flags & IMGUI_COLOR_EDIT_NO_INPUTS) != 0 &&
        ((ctx->last_item_hovered &&
          ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT]) ||
         ctx->last_item_active)) {
        channel = (int)(((ctx->input.mouse_x - ctx->last_item_rect.x1) /
                         (ctx->last_item_rect.x2 - ctx->last_item_rect.x1)) *
                        (float)channel_count);
        if (channel < 0) channel = 0;
        if (channel >= channel_count) channel = channel_count - 1;
        segment = ctx->last_item_rect;
        fraction = (ctx->input.mouse_x - segment.x1) /
                   (segment.x2 - segment.x1);
        fraction = fraction * (float)channel_count - (float)channel;
        if (fraction < 0.0f) fraction = 0.0f;
        if (fraction > 1.0f) fraction = 1.0f;
        rgba[channel] = fraction;
        ctx->last_item_edited = IMGUI_TRUE;
    } else if ((flags & IMGUI_COLOR_EDIT_NO_INPUTS) == 0) {
        imgui_color_numeric_fields(ctx, id, rgba, channel_count,
                                   ctx->last_item_rect,
                                   ctx->last_item_rect.y1);
    }
    /* In picker mode the HSV grid and hue strip already occupy the item.
       Drawing the channel swatches over the full item rectangle here would
       cover those pixels (and made the picker effectively invisible). Keep
       the compact channel visualization for the normal editor, and reserve
       only the bottom strip for a picker preview. */
    if ((flags & IMGUI_COLOR_EDIT_PICKER) != 0) {
        segment.x1 = ctx->last_item_rect.x1;
        segment.x2 = ctx->last_item_rect.x2;
        segment.y1 = ctx->last_item_rect.y1 + 130.0f;
        segment.y2 = ctx->last_item_rect.y1 + 154.0f;
        preview[0] = rgba[0];
        preview[1] = rgba[1];
        preview[2] = rgba[2];
        preview[3] = (flags & IMGUI_COLOR_EDIT_NO_ALPHA) != 0 ?
            1.0f : rgba[3];
        (void)imgui_mesh_add_rect(ctx, segment,
                                  imgui_color_pack_rgba(preview));
        if ((flags & IMGUI_COLOR_EDIT_NO_INPUTS) == 0) {
            imgui_color_numeric_fields(ctx, id, rgba, channel_count,
                                       ctx->last_item_rect,
                                       ctx->last_item_rect.y1 + 158.0f);
        }
    } else {
        segment = ctx->last_item_rect;
        for (index = 0; index < channel_count; ++index) {
            segment.x1 = ctx->last_item_rect.x1 +
                (ctx->last_item_rect.x2 - ctx->last_item_rect.x1) *
                (float)index / (float)channel_count;
            segment.x2 = ctx->last_item_rect.x1 +
                (ctx->last_item_rect.x2 - ctx->last_item_rect.x1) *
                (float)(index + 1) / (float)channel_count;
            preview[0] = 0.0f;
            preview[1] = 0.0f;
            preview[2] = 0.0f;
            preview[3] = 1.0f;
            if (index < 3) preview[index] = rgba[index];
            else {
                preview[0] = rgba[3];
                preview[1] = rgba[3];
                preview[2] = rgba[3];
            }
            color = imgui_color_pack_rgba(preview);
            (void)imgui_mesh_add_rect(ctx, segment, color);
        }
    }
    changed = IMGUI_FALSE;
    for (index = 0; index < channel_count; ++index) {
        if (old_rgba[index] != rgba[index]) changed = IMGUI_TRUE;
    }
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        const char *hidden;
        float label_x;
        float label_width;
        hidden = strstr(label, "##");
        label_x = ctx->last_item_rect.x2 + ctx->style.item_spacing;
        label_width = imgui_label_measure_width(ctx, label);
        imgui_text_draw_font(ctx, label, hidden != NULL ? hidden :
                             label + strlen(label), imgui_make_vec2(
                                 label_x,
                                 imgui_item_label_y(ctx,
                                                    ctx->last_item_rect)));
        if (label_x + label_width > ctx->content_max_x) {
            ctx->content_max_x = label_x + label_width;
        }
    }
    return changed;
}

imgui_bool imgui_color_edit_rgb(imgui_context *ctx, const char *label,
                                float rgb[3], imgui_color_edit_flags flags)
{
    float rgba[4];
    imgui_bool changed;
    if (rgb == NULL) return IMGUI_FALSE;
    rgba[0] = rgb[0];
    rgba[1] = rgb[1];
    rgba[2] = rgb[2];
    rgba[3] = 1.0f;
    changed = imgui_color_edit_rgba(ctx, label, rgba,
                                    flags | IMGUI_COLOR_EDIT_NO_ALPHA);
    rgb[0] = rgba[0];
    rgb[1] = rgba[1];
    rgb[2] = rgba[2];
    return changed;
}

imgui_bool imgui_color_picker_rgba(imgui_context *ctx, const char *label,
                                   float rgba[4],
                                   imgui_color_edit_flags flags)
{
    return imgui_color_edit_rgba(ctx, label, rgba,
                                 flags | IMGUI_COLOR_EDIT_PICKER);
}

imgui_bool imgui_color_picker_rgb(imgui_context *ctx, const char *label,
                                  float rgb[3],
                                  imgui_color_edit_flags flags)
{
    if (rgb == NULL) return IMGUI_FALSE;
    return imgui_color_edit_rgb(ctx, label, rgb,
                                flags | IMGUI_COLOR_EDIT_PICKER);
}

imgui_bool imgui_color_button(imgui_context *ctx,
                              imgui_id id,
                              const float rgba[4],
                              imgui_vec2 size)
{
    if (ctx == NULL || rgba == NULL) return IMGUI_FALSE;
    if (size.x <= 0.0f) size.x = 22.0f;
    if (size.y <= 0.0f) size.y = 22.0f;
    if (!imgui_item_register(ctx, id, size)) return IMGUI_FALSE;
    if (ctx->style.frame_rounding > 0.0f) {
        (void)imgui_draw_list_add_rect_rounded(
            ctx, &ctx->default_draw_list, ctx->last_item_rect,
            ctx->style.frame_rounding, imgui_color_pack_rgba(rgba),
            4, NULL);
    } else {
        (void)imgui_mesh_add_rect(ctx, ctx->last_item_rect,
                                  imgui_color_pack_rgba(rgba));
    }
    return ctx->last_item_clicked;
}

imgui_result imgui_texture_register_external(imgui_context *ctx,
                                             imgui_texture_id backend_id,
                                             const imgui_texture_desc *desc,
                                             imgui_texture **out_texture)
{
    imgui_texture *texture;
    imgui_texture_desc local_desc;
    if (ctx == NULL || backend_id == NULL || desc == NULL ||
        out_texture == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (!imgui_texture_desc_normalize(desc, &local_desc)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    texture = (imgui_texture *)imgui_internal_allocate(&ctx->allocator,
                                                       sizeof(*texture));
    if (texture == NULL) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    memset(texture, 0, sizeof(*texture));
    texture->backend_id = backend_id;
    texture->identity = imgui_texture_next_identity(ctx);
    texture->desc = local_desc;
    texture->external = IMGUI_TRUE;
    texture->alive = IMGUI_TRUE;
    texture->owner = ctx;
    texture->next = ctx->textures;
    ctx->textures = texture;
    *out_texture = texture;
    return IMGUI_RESULT_OK;
}

static size_t imgui_texture_bytes_per_pixel(imgui_texture_format format)
{
    return format == IMGUI_TEXTURE_FORMAT_ALPHA8 ? 1U : 4U;
}

static imgui_u32 imgui_texture_next_identity(imgui_context *ctx)
{
    if (ctx == NULL) return 0;
    ++ctx->next_texture_identity;
    if (ctx->next_texture_identity == 0) ++ctx->next_texture_identity;
    return ctx->next_texture_identity;
}

static imgui_bool imgui_texture_desc_normalize(
    const imgui_texture_desc *source,
    imgui_texture_desc *destination)
{
    size_t copy_size;
    if (source == NULL || destination == NULL ||
        source->struct_size < sizeof(source->struct_size)) {
        return IMGUI_FALSE;
    }
    imgui_texture_desc_init(destination);
    copy_size = source->struct_size < sizeof(*destination) ?
        source->struct_size : sizeof(*destination);
    memcpy(destination, source, copy_size);
    destination->struct_size = sizeof(*destination);
    return destination->width > 0 && destination->height > 0 &&
           destination->format >= IMGUI_TEXTURE_FORMAT_RGBA8 &&
           destination->format <= IMGUI_TEXTURE_FORMAT_ALPHA8;
}

imgui_result imgui_texture_create(imgui_context *ctx,
                                  const imgui_texture_desc *desc,
                                  const void *pixels,
                                  size_t pitch,
                                  imgui_texture **out_texture)
{
    imgui_texture *texture;
    imgui_resource_operation operation;
    imgui_texture_desc local_desc;
    size_t row_bytes;
    size_t bytes;
    if (out_texture != NULL) {
        *out_texture = NULL;
    }
    if (ctx == NULL || out_texture == NULL ||
        !imgui_texture_desc_normalize(desc, &local_desc)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (!imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    row_bytes = (size_t)local_desc.width *
                imgui_texture_bytes_per_pixel(local_desc.format);
    if ((pixels != NULL && pitch < row_bytes) ||
        (pixels != NULL && (size_t)local_desc.height >
         ((size_t)-1) / pitch)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    bytes = pixels != NULL ? pitch * (size_t)local_desc.height : 0;
    texture = (imgui_texture *)imgui_internal_allocate(&ctx->allocator,
                                                        sizeof(*texture));
    if (texture == NULL) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    memset(texture, 0, sizeof(*texture));
    texture->identity = imgui_texture_next_identity(ctx);
    texture->desc = local_desc;
    texture->owner = ctx;
    texture->alive = IMGUI_TRUE;
    texture->next = ctx->textures;
    ctx->textures = texture;
    memset(&operation, 0, sizeof(operation));
    operation.type = IMGUI_RESOURCE_CREATE_TEXTURE;
    operation.texture = texture;
    operation.format = local_desc.format;
    operation.texture_width = local_desc.width;
    operation.texture_height = local_desc.height;
    operation.pitch = pitch;
    if (imgui_resource_operation_append(ctx, &operation, pixels, bytes) !=
        IMGUI_RESULT_OK) {
        texture->alive = IMGUI_FALSE;
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    if (pixels != NULL) {
        operation.type = IMGUI_RESOURCE_UPLOAD_TEXTURE;
        operation.region.x = 0;
        operation.region.y = 0;
        operation.region.width = local_desc.width;
        operation.region.height = local_desc.height;
        if (imgui_resource_operation_append(ctx, &operation, pixels, bytes) !=
            IMGUI_RESULT_OK) {
            texture->alive = IMGUI_FALSE;
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
    }
    *out_texture = texture;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_texture_update(imgui_context *ctx,
                                  imgui_texture *texture,
                                  int x,
                                  int y,
                                  int width,
                                  int height,
                                  const void *pixels,
                                  size_t pitch)
{
    imgui_resource_operation operation;
    size_t row_bytes;
    size_t bytes;
    if (ctx == NULL || texture == NULL || texture->owner != ctx ||
        !texture->alive || pixels == NULL || width <= 0 || height <= 0 ||
        x < 0 || y < 0 || x > texture->desc.width ||
        y > texture->desc.height || width > texture->desc.width - x ||
        height > texture->desc.height - y) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (!imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    row_bytes = (size_t)width * imgui_texture_bytes_per_pixel(
        texture->desc.format);
    if (pitch < row_bytes || (size_t)height > ((size_t)-1) / pitch) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    bytes = pitch * (size_t)height;
    memset(&operation, 0, sizeof(operation));
    operation.type = IMGUI_RESOURCE_UPDATE_TEXTURE;
    operation.texture = texture;
    operation.region.x = x;
    operation.region.y = y;
    operation.region.width = width;
    operation.region.height = height;
    operation.format = texture->desc.format;
    operation.pitch = pitch;
    if (imgui_resource_operation_append(ctx, &operation, pixels, bytes) !=
        IMGUI_RESULT_OK) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    return IMGUI_RESULT_OK;
}

imgui_result imgui_texture_destroy(imgui_context *ctx, imgui_texture *texture)
{
    imgui_resource_operation operation;
    imgui_u32 command_index;
    if (ctx == NULL || texture == NULL || texture->owner != ctx) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (!texture->alive) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (!imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    for (command_index = 0; command_index < ctx->command_count;
         ++command_index) {
        if (ctx->commands[command_index].type ==
            IMGUI_RENDER_COMMAND_DRAW_INDEXED &&
            ctx->commands[command_index].data.draw_indexed.texture == texture) {
            imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                                  "cannot destroy texture used by this frame");
            return IMGUI_RESULT_INVALID_STATE;
        }
        if (ctx->commands[command_index].type ==
            IMGUI_RENDER_COMMAND_TEXTURE_COPY &&
            (ctx->commands[command_index].data.texture_copy.source == texture ||
             ctx->commands[command_index].data.texture_copy.destination ==
             texture)) {
            imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                                  "cannot destroy texture used by this frame");
            return IMGUI_RESULT_INVALID_STATE;
        }
        if (ctx->commands[command_index].type ==
            IMGUI_RENDER_COMMAND_TEXTURE_CLEAR &&
            ctx->commands[command_index].data.texture_clear.texture ==
            texture) {
            imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                                  "cannot destroy texture used by this frame");
            return IMGUI_RESULT_INVALID_STATE;
        }
        if (ctx->commands[command_index].type ==
            IMGUI_RENDER_COMMAND_TEXTURE_UPDATE &&
            ctx->commands[command_index].data.texture_update.texture ==
            texture) {
            imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                                  "cannot destroy texture used by this frame");
            return IMGUI_RESULT_INVALID_STATE;
        }
    }
    memset(&operation, 0, sizeof(operation));
    operation.type = IMGUI_RESOURCE_DESTROY_TEXTURE;
    operation.texture = texture;
    if (imgui_resource_operation_append(ctx, &operation, NULL, 0) !=
        IMGUI_RESULT_OK) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    if (ctx->font_texture == texture) {
        ctx->font = NULL;
        ctx->font_texture = NULL;
    }
    texture->alive = IMGUI_FALSE;
    return IMGUI_RESULT_OK;
}

imgui_bool imgui_texture_equivalent(const imgui_texture *left,
                                    const imgui_texture *right)
{
    const char *left_name;
    const char *right_name;
    if (left == right) return IMGUI_TRUE;
    if (left == NULL || right == NULL) return IMGUI_FALSE;
    if ((left->identity != 0 && right->identity != 0 &&
         left->identity != right->identity) ||
        left->backend_id != right->backend_id ||
        left->desc.width != right->desc.width ||
        left->desc.height != right->desc.height ||
        left->desc.format != right->desc.format ||
        left->desc.flags != right->desc.flags ||
        left->external != right->external || left->alive != right->alive) {
        return IMGUI_FALSE;
    }
    left_name = left->desc.debug_name;
    right_name = right->desc.debug_name;
    if (left_name == NULL || right_name == NULL) return left_name == right_name;
    return strcmp(left_name, right_name) == 0 ? IMGUI_TRUE : IMGUI_FALSE;
}

void imgui_image(imgui_context *ctx,
                 imgui_texture *texture,
                 imgui_vec2 size)
{
    imgui_image_uv(ctx, texture, size, imgui_make_vec2(0.0f, 0.0f),
                   imgui_make_vec2(1.0f, 1.0f));
}

void imgui_image_uv(imgui_context *ctx,
                    imgui_texture *texture,
                    imgui_vec2 size,
                    imgui_vec2 uv_min,
                    imgui_vec2 uv_max)
{
    imgui_id id;
    imgui_u32 vertex_offset;
    if (ctx == NULL || texture == NULL || texture->owner != ctx ||
        !texture->alive) {
        if (ctx != NULL && ctx->frame_state == IMGUI_INTERNAL_FRAME_BUILDING) {
            imgui_internal_report(ctx, IMGUI_ERROR_INVALID_ARGUMENT,
                                  "image texture is invalid");
        }
        return;
    }
    if (!imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES)) {
        imgui_internal_report(ctx, IMGUI_ERROR_UNSUPPORTED,
                              "image texture capability is unavailable");
        return;
    }
    id = imgui_get_id_pointer(ctx, texture);
    if (size.x <= 0.0f) size.x = (float)texture->desc.width;
    if (size.y <= 0.0f) size.y = (float)texture->desc.height;
    if (!imgui_item_register(ctx, id, size)) return;
    if (imgui_mesh_add_rect(ctx, ctx->last_item_rect, 0xffffffffUL) !=
        IMGUI_RESULT_OK) return;
    vertex_offset = ctx->vertex_count - 4U;
    ctx->vertices[vertex_offset + 0].uv = uv_min;
    ctx->vertices[vertex_offset + 1].uv = imgui_make_vec2(uv_max.x, uv_min.y);
    ctx->vertices[vertex_offset + 2].uv = uv_max;
    ctx->vertices[vertex_offset + 3].uv = imgui_make_vec2(uv_min.x, uv_max.y);
    ctx->commands[ctx->command_count - 1].data.draw_indexed.texture = texture;
}

imgui_bool imgui_image_button(imgui_context *ctx,
                              imgui_id id,
                              imgui_texture *texture,
                              imgui_vec2 size)
{
    imgui_u32 vertex_offset;
    imgui_result result;
    imgui_rect image_rect;
    float padding;
    float padding_y;
    if (ctx == NULL) return IMGUI_FALSE;
    if (texture == NULL || texture->owner != ctx || !texture->alive ||
        !imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES)) {
        return IMGUI_FALSE;
    }
    if (size.x <= 0.0f) size.x = (float)texture->desc.width;
    if (size.y <= 0.0f) size.y = (float)texture->desc.height;
    padding = ctx->style.frame_padding.x;
    if (padding < 0.0f) padding = 0.0f;
    padding_y = ctx->style.frame_padding.y;
    if (padding_y < 0.0f) padding_y = 0.0f;
    if (!imgui_item_register(ctx, id,
                             imgui_make_vec2(size.x + 2.0f * padding,
                                             size.y + 2.0f * padding))) {
        return IMGUI_FALSE;
    }
    (void)imgui_add_frame_surface(
        ctx, ctx->last_item_rect,
        ctx->last_item_active ? ctx->style.color_button_active :
        (ctx->last_item_hovered ? ctx->style.color_button_hovered :
                                  ctx->style.color_button));
    image_rect = ctx->last_item_rect;
    image_rect.x1 += padding;
    image_rect.y1 += padding_y;
    image_rect.x2 -= padding;
    image_rect.y2 -= padding_y;
    result = imgui_mesh_add_rect(ctx, image_rect, 0xffffffffUL);
    if (result != IMGUI_RESULT_OK || ctx->vertex_count < 4U ||
        ctx->command_count == 0) {
        return IMGUI_FALSE;
    }
    vertex_offset = ctx->vertex_count - 4U;
    ctx->vertices[vertex_offset + 0U].uv = imgui_make_vec2(0.0f, 0.0f);
    ctx->vertices[vertex_offset + 1U].uv = imgui_make_vec2(1.0f, 0.0f);
    ctx->vertices[vertex_offset + 2U].uv = imgui_make_vec2(1.0f, 1.0f);
    ctx->vertices[vertex_offset + 3U].uv = imgui_make_vec2(0.0f, 1.0f);
    ctx->commands[ctx->command_count - 1].data.draw_indexed.texture = texture;
    return ctx->last_item_clicked;
}

imgui_scope imgui_combo_begin(imgui_context *ctx,
                              const char *label,
                              const char *preview)
{
    return imgui_combo_begin_ex(ctx, label, preview, 0);
}

imgui_scope imgui_combo_begin_ex(imgui_context *ctx,
                                 const char *label,
                                 const char *preview,
                                 imgui_combo_flags flags)
{
    imgui_id id;
    imgui_vec2 size;
    imgui_bool clicked;
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    size = imgui_make_vec2(220.0f, 22.0f);
    if (!imgui_item_register(ctx, id, size)) {
        return IMGUI_SCOPE_ERROR;
    }
    if (ctx->style.frame_rounding > 0.0f) {
        (void)imgui_draw_list_add_rect_rounded(
            ctx, &ctx->default_draw_list, ctx->last_item_rect,
            ctx->style.frame_rounding,
            ctx->last_item_hovered ? ctx->style.color_frame_hovered :
                                     ctx->style.color_frame,
            4, NULL);
    } else {
        (void)imgui_mesh_add_rect(ctx, ctx->last_item_rect,
                                  ctx->last_item_hovered ?
                                  ctx->style.color_frame_hovered :
                                  ctx->style.color_frame);
    }
    /* Dear ImGui reserves a small right-hand button area for the combo
       indicator. Keep the preview text left-aligned while making the closed
       control visibly discoverable. */
    if ((flags & IMGUI_COMBO_NO_ARROW_BUTTON) == 0) {
        (void)imgui_mesh_add_triangle(
            ctx, imgui_make_vec2(ctx->last_item_rect.x2 - 14.0f,
                                 ctx->last_item_rect.y1 + 8.0f),
            imgui_make_vec2(ctx->last_item_rect.x2 - 5.0f,
                           ctx->last_item_rect.y1 + 8.0f),
            imgui_make_vec2(ctx->last_item_rect.x2 - 9.5f,
                           ctx->last_item_rect.y1 + 14.0f),
            ctx->style.color_text);
    }
    if ((flags & IMGUI_COMBO_NO_PREVIEW) == 0 &&
        ctx->font != NULL && ctx->font_texture != NULL && preview != NULL) {
        imgui_text_draw_font(ctx, preview, preview + strlen(preview),
                             imgui_make_vec2(ctx->last_item_rect.x1 +
                                             ctx->style.frame_padding.x,
                                             imgui_item_label_y(ctx,
                                                                ctx->last_item_rect)));
    }
    clicked = ctx->last_item_clicked;
    if (clicked) {
        if (ctx->popup_open && ctx->popup_id == id) {
            ctx->popup_open = IMGUI_FALSE;
            imgui_popup_stack_clear(ctx);
        } else {
            ctx->popup_id = id;
            ctx->popup_open = IMGUI_TRUE;
            ctx->popup_mouse_button = IMGUI_MOUSE_BUTTON_LEFT;
            ctx->popup_stack_count = 1;
            ctx->popup_stack_ids[0] = id;
            ctx->popup_rect.x1 = ctx->last_item_rect.x1;
            ctx->popup_rect.y1 = ctx->last_item_rect.y2;
            ctx->popup_rect.x2 = ctx->popup_rect.x1 + 220.0f;
            ctx->popup_rect.y2 = ctx->popup_rect.y1 + 200.0f;
            ctx->popup_rect_valid = IMGUI_TRUE;
            ctx->popup_stack_rects[0] = ctx->popup_rect;
            ctx->popup_stack_rect_valid[0] = IMGUI_TRUE;
        }
    }
    if (ctx->popup_open && ctx->popup_id == id) {
        imgui_scope combo_scope;
        imgui_rect combo_clip;
        ctx->current_popup_id = id;
        combo_scope = imgui_internal_scope_begin(
            ctx, IMGUI_INTERNAL_SCOPE_COMBO, IMGUI_TRUE);
        if (combo_scope != IMGUI_SCOPE_ACTIVE) return combo_scope;
        ctx->combo_render_start = ctx->command_count;
        ctx->combo_render_tracking = IMGUI_TRUE;
        imgui_push_id_value(ctx, id);
        combo_clip.x1 = 0.0f;
        combo_clip.y1 = 0.0f;
        combo_clip.x2 = ctx->frame_desc.display_size.x;
        combo_clip.y2 = ctx->frame_desc.display_size.y;
        ctx->clip_rect = combo_clip;
        (void)imgui_mesh_add_rect(ctx, ctx->popup_rect,
                                  ctx->style.color_popup_background);
        ctx->popup_background_vertex_offset = ctx->vertex_count - 4U;
        ctx->popup_background_active = IMGUI_TRUE;
        ctx->clip_rect = ctx->popup_rect;
        if (ctx->clip_rect.x1 < 0.0f) ctx->clip_rect.x1 = 0.0f;
        if (ctx->clip_rect.y1 < 0.0f) ctx->clip_rect.y1 = 0.0f;
        if (ctx->clip_rect.x2 > ctx->frame_desc.display_size.x) {
            ctx->clip_rect.x2 = ctx->frame_desc.display_size.x;
        }
        if (ctx->clip_rect.y2 > ctx->frame_desc.display_size.y) {
            ctx->clip_rect.y2 = ctx->frame_desc.display_size.y;
        }
        ctx->cursor.x = ctx->popup_rect.x1;
        ctx->cursor.y = ctx->popup_rect.y1;
        return combo_scope;
    }
    return imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_COMBO,
                                      IMGUI_FALSE);
}

void imgui_combo_end(imgui_context *ctx)
{
    if (ctx != NULL && ctx->popup_background_active &&
        ctx->popup_background_vertex_offset + 3U < ctx->vertex_count) {
        imgui_u32 offset = ctx->popup_background_vertex_offset;
        imgui_rect rect = ctx->popup_rect;
        ctx->vertices[offset + 0U].position =
            imgui_make_vec2(rect.x1, rect.y1);
        ctx->vertices[offset + 1U].position =
            imgui_make_vec2(rect.x2, rect.y1);
        ctx->vertices[offset + 2U].position =
            imgui_make_vec2(rect.x2, rect.y2);
        ctx->vertices[offset + 3U].position =
            imgui_make_vec2(rect.x1, rect.y2);
        (void)imgui_mesh_add_window_border(
            ctx, rect, ctx->style.color_popup_border);
        ctx->popup_background_active = IMGUI_FALSE;
    }
    if (ctx != NULL && ctx->combo_render_tracking &&
        ctx->combo_render_start <= ctx->command_count) {
        ctx->combo_render_end = ctx->command_count;
        ctx->combo_render_valid = IMGUI_TRUE;
        ctx->combo_render_tracking = IMGUI_FALSE;
    }
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_COMBO);
    if (ctx != NULL) ctx->current_popup_id = 0;
}

imgui_bool imgui_selectable(imgui_context *ctx,
                            const char *label,
                            imgui_bool selected)
{
    imgui_bool clicked;
    imgui_vec2 size;
    size = imgui_make_vec2(220.0f, 22.0f);
    if (!imgui_item_register(ctx, imgui_get_id_string(ctx, label), size)) {
        return IMGUI_FALSE;
    }
    if (ctx->style.frame_rounding > 0.0f) {
        (void)imgui_draw_list_add_rect_rounded(
            ctx, &ctx->default_draw_list, ctx->last_item_rect,
            ctx->style.frame_rounding,
            selected ? ctx->style.color_header_active :
            (ctx->last_item_hovered ? ctx->style.color_header_hovered :
                                      ctx->style.color_header),
            4, NULL);
    } else {
        (void)imgui_mesh_add_rect(ctx, ctx->last_item_rect,
                                  selected ? ctx->style.color_header_active :
                                  (ctx->last_item_hovered ?
                                   ctx->style.color_header_hovered :
                                   ctx->style.color_header));
    }
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        imgui_text_draw_font(ctx, label, imgui_label_visible_end(label),
                             imgui_make_vec2(ctx->last_item_rect.x1 +
                                             ctx->style.frame_padding.x,
                                             imgui_item_label_y(ctx,
                                                                ctx->last_item_rect)));
    }
    clicked = ctx->last_item_clicked;
    return clicked;
}

imgui_bool imgui_selectable_ex(imgui_context *ctx,
                               const char *label,
                               imgui_bool *selected,
                               imgui_selectable_flags flags,
                               imgui_vec2 size)
{
    imgui_bool clicked;
    imgui_bool disabled;
    imgui_u32 color;
    if (ctx == NULL) return IMGUI_FALSE;
    disabled = (flags & IMGUI_SELECTABLE_DISABLED) != 0 ?
               IMGUI_TRUE : IMGUI_FALSE;
    if (disabled) ++ctx->disabled_depth;
    if (size.x <= 0.0f) size.x = 220.0f;
    if (size.y <= 0.0f) size.y = 22.0f;
    if (!imgui_item_register(ctx, imgui_get_id_string(ctx, label), size)) {
        if (disabled) --ctx->disabled_depth;
        return IMGUI_FALSE;
    }
    if (!disabled &&
        (flags & IMGUI_SELECTABLE_ALLOW_DOUBLE_CLICK) != 0 &&
        ctx->last_item_hovered &&
        ctx->mouse_double_clicked[IMGUI_MOUSE_BUTTON_LEFT]) {
        ctx->last_item_clicked = IMGUI_TRUE;
        ctx->last_item_activated = IMGUI_TRUE;
        ctx->item_clicked_this_frame = IMGUI_TRUE;
    }
    color = disabled ? ctx->style.color_frame :
            (selected != NULL && *selected ? ctx->style.color_header_active :
             (ctx->last_item_hovered ? ctx->style.color_header_hovered :
                                       ctx->style.color_header));
    if (ctx->style.frame_rounding > 0.0f) {
        (void)imgui_draw_list_add_rect_rounded(
            ctx, &ctx->default_draw_list, ctx->last_item_rect,
            ctx->style.frame_rounding, color,
            4, NULL);
    } else {
        (void)imgui_mesh_add_rect(ctx, ctx->last_item_rect, color);
    }
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        imgui_text_draw_font(ctx, label, imgui_label_visible_end(label),
                             imgui_make_vec2(ctx->last_item_rect.x1 +
                                             ctx->style.frame_padding.x,
                                             imgui_item_label_y(ctx,
                                             ctx->last_item_rect)));
    }
    if (disabled) --ctx->disabled_depth;
    clicked = ctx->last_item_clicked;
    if (clicked && selected != NULL) {
        *selected = *selected ? IMGUI_FALSE : IMGUI_TRUE;
        ctx->last_item_edited = IMGUI_TRUE;
    }
    return clicked;
}

static int imgui_multi_select_find(const imgui_multi_select_storage *storage,
                                   imgui_id id)
{
    size_t index;
    if (storage == NULL || storage->ids == NULL) return -1;
    for (index = 0; index < storage->count; ++index) {
        if (storage->ids[index] == id) return (int)index;
    }
    return -1;
}

static imgui_bool imgui_multi_select_storage_valid(
    const imgui_multi_select_storage *storage)
{
    return storage != NULL && storage->struct_size >= sizeof(*storage) &&
           storage->count <= storage->capacity &&
           (storage->capacity == 0 || storage->ids != NULL);
}

static imgui_result imgui_multi_select_frame_reserve(
    imgui_context *ctx, int required)
{
    int capacity;
    imgui_id *ids;
    if (ctx == NULL || required < 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (required <= ctx->multi_select_frame_capacity) {
        return IMGUI_RESULT_OK;
    }
    capacity = ctx->multi_select_frame_capacity > 0 ?
        ctx->multi_select_frame_capacity : 32;
    while (capacity < required) {
        if (capacity > INT_MAX / 2) {
            capacity = required;
            break;
        }
        capacity *= 2;
    }
    if ((size_t)capacity > (size_t)-1 / sizeof(*ids)) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    ids = (imgui_id *)imgui_internal_allocate(
        &ctx->allocator, (size_t)capacity * sizeof(*ids));
    if (ids == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    if (ctx->multi_select_frame_count != 0) {
        memcpy(ids, ctx->multi_select_frame_ids,
               (size_t)ctx->multi_select_frame_count * sizeof(*ids));
    }
    imgui_internal_release(&ctx->allocator, ctx->multi_select_frame_ids);
    ctx->multi_select_frame_ids = ids;
    ctx->multi_select_frame_capacity = capacity;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_multi_select_begin(imgui_context *ctx,
                                      imgui_multi_select_storage *storage,
                                      imgui_multi_select_flags flags)
{
    if (ctx == NULL || storage == NULL ||
        !imgui_multi_select_storage_valid(storage)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (!imgui_internal_require_building(ctx,
                                         "multi-select begin outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->multi_select_active ||
        (flags & ~(IMGUI_MULTI_SELECT_TOGGLE_ON_CLICK |
                   IMGUI_MULTI_SELECT_ALLOW_SHIFT_RANGE)) != 0) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->multi_select_anchor_storage != storage) {
        ctx->multi_select_anchor_id = 0;
    }
    ctx->multi_select_anchor_storage = storage;
    ctx->multi_select_storage = storage;
    ctx->multi_select_flags = flags;
    ctx->multi_select_active = IMGUI_TRUE;
    ctx->multi_select_anchor_id = 0;
    ctx->multi_select_frame_count = 0;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_multi_select_end(imgui_context *ctx)
{
    if (ctx == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (!imgui_internal_require_building(ctx,
                                         "multi-select end outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (!ctx->multi_select_active) return IMGUI_RESULT_INVALID_STATE;
    ctx->multi_select_storage = NULL;
    ctx->multi_select_flags = IMGUI_MULTI_SELECT_NONE;
    ctx->multi_select_active = IMGUI_FALSE;
    ctx->multi_select_frame_count = 0;
    return IMGUI_RESULT_OK;
}

imgui_bool imgui_multi_select_contains(const imgui_context *ctx,
                                       imgui_id id)
{
    if (ctx == NULL || !ctx->multi_select_active ||
        !imgui_multi_select_storage_valid(ctx->multi_select_storage) ||
        id == 0) {
        return IMGUI_FALSE;
    }
    return imgui_multi_select_find(ctx->multi_select_storage, id) >= 0 ?
           IMGUI_TRUE : IMGUI_FALSE;
}

imgui_result imgui_multi_select_clear(imgui_context *ctx)
{
    if (ctx == NULL || !ctx->multi_select_active ||
        !imgui_multi_select_storage_valid(ctx->multi_select_storage)) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    ctx->multi_select_storage->count = 0;
    ctx->multi_select_anchor_id = 0;
    return IMGUI_RESULT_OK;
}

imgui_bool imgui_multi_select_item(imgui_context *ctx, imgui_id id)
{
    imgui_multi_select_storage *storage;
    int found;
    imgui_bool toggle;
    imgui_bool shift_down;
    int anchor_index;
    int current_index;
    int range_start;
    int range_end;
    int range_index;
    imgui_bool clicked;
    if (ctx == NULL || id == 0 || !ctx->multi_select_active ||
        !imgui_multi_select_storage_valid(ctx->multi_select_storage) ||
        !imgui_internal_require_building(ctx,
                                         "multi-select item outside frame")) {
        return IMGUI_FALSE;
    }
    if (ctx->last_item_id != id) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                              "multi-select item must follow its widget");
        return IMGUI_FALSE;
    }
    if (imgui_multi_select_frame_reserve(
            ctx, ctx->multi_select_frame_count + 1) != IMGUI_RESULT_OK) {
        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                              "multi-select frame item storage exceeded");
        return IMGUI_FALSE;
    }
    current_index = ctx->multi_select_frame_count;
    ctx->multi_select_frame_ids[current_index] = id;
    ctx->multi_select_frame_count += 1;
    clicked = ctx->last_item_clicked;
    if (!clicked) return IMGUI_FALSE;
    storage = ctx->multi_select_storage;
    found = imgui_multi_select_find(storage, id);
    toggle = (ctx->multi_select_flags & IMGUI_MULTI_SELECT_TOGGLE_ON_CLICK) != 0 ||
             ctx->input.keys_down[IMGUI_KEY_LEFT_CTRL] ||
             ctx->input.keys_down[IMGUI_KEY_RIGHT_CTRL];
    shift_down = (ctx->multi_select_flags & IMGUI_MULTI_SELECT_ALLOW_SHIFT_RANGE) != 0 &&
                 (ctx->input.keys_down[IMGUI_KEY_LEFT_SHIFT] ||
                  ctx->input.keys_down[IMGUI_KEY_RIGHT_SHIFT]);
    if (shift_down && ctx->multi_select_anchor_id != 0) {
        anchor_index = -1;
        for (range_index = 0; range_index < current_index; ++range_index) {
            if (ctx->multi_select_frame_ids[range_index] ==
                ctx->multi_select_anchor_id) {
                anchor_index = range_index;
                break;
            }
        }
        if (anchor_index >= 0) {
            range_start = anchor_index < current_index ? anchor_index : current_index;
            range_end = anchor_index > current_index ? anchor_index : current_index;
            if (!toggle) storage->count = 0;
            for (range_index = range_start; range_index <= range_end;
                 ++range_index) {
                if (imgui_multi_select_find(storage,
                        ctx->multi_select_frame_ids[range_index]) < 0) {
                    if (storage->count >= storage->capacity) {
                        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                                              "multi-select range exceeds storage");
                        return IMGUI_FALSE;
                    }
                    storage->ids[storage->count++] =
                        ctx->multi_select_frame_ids[range_index];
                }
            }
            ctx->last_item_edited = IMGUI_TRUE;
            ctx->last_item_toggled_selection = IMGUI_TRUE;
            return IMGUI_TRUE;
        }
    }
    if (toggle && found >= 0) {
        if ((size_t)found + 1 < storage->count) {
            memmove(storage->ids + found, storage->ids + found + 1,
                    (storage->count - (size_t)found - 1) *
                    sizeof(*storage->ids));
        }
        storage->count -= 1;
        ctx->multi_select_anchor_id = id;
        ctx->last_item_edited = IMGUI_TRUE;
        ctx->last_item_toggled_selection = IMGUI_TRUE;
        return IMGUI_TRUE;
    }
    if (!toggle) storage->count = 0;
    if (imgui_multi_select_find(storage, id) < 0) {
        if (storage->count >= storage->capacity) {
            imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                                  "multi-select storage capacity exceeded");
            return IMGUI_FALSE;
        }
        storage->ids[storage->count++] = id;
    }
    ctx->multi_select_anchor_id = id;
    ctx->last_item_edited = IMGUI_TRUE;
    ctx->last_item_toggled_selection = IMGUI_TRUE;
    return IMGUI_TRUE;
}

imgui_scope imgui_drag_drop_source_begin(imgui_context *ctx,
                                         imgui_id source_id,
                                         imgui_drag_drop_source_flags flags)
{
    imgui_bool active;
    imgui_bool threshold_reached;
    imgui_id effective_source_id;
    float dx;
    float dy;
    float threshold;
    if (ctx == NULL ||
        (source_id == 0 &&
         (flags & (IMGUI_DRAG_DROP_SOURCE_ALLOW_NULL_ID |
                   IMGUI_DRAG_DROP_SOURCE_EXTERN)) == 0) ||
        (flags & ~(IMGUI_DRAG_DROP_SOURCE_NO_PREVIEW |
                   IMGUI_DRAG_DROP_SOURCE_NO_DISABLE_HOVER |
                   IMGUI_DRAG_DROP_SOURCE_NO_HOLD_TO_OPEN_OTHERS |
                   IMGUI_DRAG_DROP_SOURCE_ALLOW_NULL_ID |
                   IMGUI_DRAG_DROP_SOURCE_EXTERN |
                   IMGUI_DRAG_DROP_SOURCE_PAYLOAD_AUTO_EXPIRE |
                   IMGUI_DRAG_DROP_SOURCE_PAYLOAD_NO_CROSS_CONTEXT |
                   IMGUI_DRAG_DROP_SOURCE_PAYLOAD_NO_CROSS_PROCESS)) != 0) {
        return IMGUI_SCOPE_ERROR;
    }
    if ((flags & IMGUI_DRAG_DROP_SOURCE_EXTERN) != 0) {
        effective_source_id = source_id != 0 ? source_id :
            imgui_hash_bytes("#SourceExtern", 13U, imgui_id_seed(ctx));
    } else if (source_id == 0 &&
               (flags & IMGUI_DRAG_DROP_SOURCE_ALLOW_NULL_ID) != 0) {
        effective_source_id = imgui_hash_bytes(&ctx->last_item_rect,
                                               sizeof(ctx->last_item_rect),
                                               imgui_id_seed(ctx));
        if (effective_source_id == 0) effective_source_id = 1U;
    } else {
        effective_source_id = source_id;
    }
    if (!imgui_internal_require_building(ctx,
                                         "drag source begin outside frame")) {
        return IMGUI_SCOPE_ERROR;
    }
    if (ctx->drag_source_scope_active || ctx->drag_target_scope_active) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                              "nested drag/drop scope is not supported");
        return IMGUI_SCOPE_ERROR;
    }
    threshold = ctx->mouse_drag_threshold > 0.0f ?
                ctx->mouse_drag_threshold : 0.0f;
    dx = ctx->input.mouse_x -
         ctx->mouse_last_click_position[IMGUI_MOUSE_BUTTON_LEFT].x;
    dy = ctx->input.mouse_y -
         ctx->mouse_last_click_position[IMGUI_MOUSE_BUTTON_LEFT].y;
    threshold_reached = ctx->drag_payload_active ||
        (dx * dx + dy * dy >= threshold * threshold);
    /* BeginDragDropSource is allowed to be called every frame while the
       source item owns the mouse.  The first held frame only arms the
       source; payload submission becomes active after the configured drag
       threshold, matching Dear ImGui's delayed drag activation. */
    active = (flags & IMGUI_DRAG_DROP_SOURCE_EXTERN) != 0 ?
             IMGUI_TRUE :
             ((ctx->last_item_id == source_id ||
               (source_id == 0 &&
                (flags & IMGUI_DRAG_DROP_SOURCE_ALLOW_NULL_ID) != 0)) &&
              ctx->last_item_active && threshold_reached);
    if (imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_DRAG_SOURCE,
                                   active) == IMGUI_SCOPE_ERROR) {
        return IMGUI_SCOPE_ERROR;
    }
    ctx->drag_source_scope_active = active;
    if (active) {
        ctx->drag_payload_source_id = effective_source_id;
        ctx->drag_source_flags = flags;
        ctx->drag_payload_source_frame = ctx->frame_index;
    }
    return active ? IMGUI_SCOPE_ACTIVE : IMGUI_SCOPE_INACTIVE;
}

imgui_result imgui_drag_drop_set_payload(imgui_context *ctx,
                                         const char *type,
                                         const void *data,
                                         size_t data_size)
{
    size_t type_length;
    void *copy;
    if (ctx == NULL || type == NULL || type[0] == '\0' ||
        (data == NULL && data_size != 0) || !ctx->drag_source_scope_active) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    type_length = strlen(type);
    copy = NULL;
    if (data_size != 0) {
        copy = imgui_internal_allocate(&ctx->allocator, data_size);
        if (copy == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
        memcpy(copy, data, data_size);
    }
    if (imgui_drag_payload_store_type(ctx, type, type_length) !=
            IMGUI_RESULT_OK) {
        imgui_internal_release(&ctx->allocator, copy);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    imgui_internal_release(&ctx->allocator, ctx->drag_payload_data);
    ctx->drag_payload_data = copy;
    ctx->drag_payload_capacity = data_size;
    ctx->drag_payload_active = IMGUI_TRUE;
    ctx->drag_payload.struct_size = sizeof(ctx->drag_payload);
    ctx->drag_payload.type = ctx->drag_payload_type;
    ctx->drag_payload.data = copy;
    ctx->drag_payload.data_size = data_size;
    ctx->drag_payload.source_id = ctx->drag_payload_source_id;
    ctx->drag_payload.preview = IMGUI_FALSE;
    ctx->drag_payload.delivery = IMGUI_FALSE;
    return IMGUI_RESULT_OK;
}

void imgui_drag_drop_source_end(imgui_context *ctx)
{
    if (ctx == NULL) return;
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_DRAG_SOURCE);
    ctx->drag_source_scope_active = IMGUI_FALSE;
}

imgui_scope imgui_drag_drop_target_begin(imgui_context *ctx)
{
    imgui_bool active;
    if (ctx == NULL ||
        !imgui_internal_require_building(ctx,
                                         "drag target begin outside frame")) {
        return IMGUI_SCOPE_ERROR;
    }
    if (ctx->drag_source_scope_active || ctx->drag_target_scope_active) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                              "nested drag/drop scope is not supported");
        return IMGUI_SCOPE_ERROR;
    }
    active = ctx->drag_payload_active && ctx->last_item_hovered;
    if (imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_DRAG_TARGET,
                                   active) == IMGUI_SCOPE_ERROR) {
        return IMGUI_SCOPE_ERROR;
    }
    ctx->drag_target_scope_active = active;
    ctx->drag_target_rect_drawn = IMGUI_FALSE;
    return active ? IMGUI_SCOPE_ACTIVE : IMGUI_SCOPE_INACTIVE;
}

const imgui_drag_payload *imgui_drag_drop_target_accept(
    imgui_context *ctx,
    const char *type,
    imgui_drag_drop_target_flags flags)
{
    if (ctx == NULL || type == NULL || ctx->drag_payload_type == NULL ||
        (flags & ~(IMGUI_DRAG_DROP_TARGET_ACCEPT_BEFORE_DELIVERY |
                   IMGUI_DRAG_DROP_TARGET_NO_DRAW_DEFAULT_RECT |
                   IMGUI_DRAG_DROP_TARGET_NO_PREVIEW_TOOLTIP |
                   IMGUI_DRAG_DROP_TARGET_DRAW_AS_HOVERED)) != 0 ||
        !ctx->drag_target_scope_active || !ctx->drag_payload_active ||
        strcmp(type, ctx->drag_payload_type) != 0) {
        return NULL;
    }
    ctx->drag_payload.preview =
        (ctx->drag_source_flags & IMGUI_DRAG_DROP_SOURCE_NO_PREVIEW) == 0 ?
        IMGUI_TRUE : IMGUI_FALSE;
    ctx->drag_payload.delivery =
        (flags & IMGUI_DRAG_DROP_TARGET_ACCEPT_BEFORE_DELIVERY) != 0 ||
        !ctx->input.mouse_down[IMGUI_MOUSE_BUTTON_LEFT];
    if (!ctx->drag_target_rect_drawn &&
        (flags & IMGUI_DRAG_DROP_TARGET_NO_DRAW_DEFAULT_RECT) == 0) {
        imgui_rect rect = ctx->last_item_rect;
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(rect.x1, rect.y1),
            imgui_make_vec2(rect.x2, rect.y1),
            ctx->style.color_drag_drop_target, 2.0f);
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(rect.x2, rect.y1),
            imgui_make_vec2(rect.x2, rect.y2),
            ctx->style.color_drag_drop_target, 2.0f);
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(rect.x2, rect.y2),
            imgui_make_vec2(rect.x1, rect.y2),
            ctx->style.color_drag_drop_target, 2.0f);
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(rect.x1, rect.y2),
            imgui_make_vec2(rect.x1, rect.y1),
            ctx->style.color_drag_drop_target, 2.0f);
        ctx->drag_target_rect_drawn = IMGUI_TRUE;
    }
    return &ctx->drag_payload;
}

void imgui_drag_drop_target_end(imgui_context *ctx)
{
    if (ctx == NULL) return;
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_DRAG_TARGET);
    ctx->drag_target_scope_active = IMGUI_FALSE;
}

imgui_scope imgui_list_box_begin(imgui_context *ctx,
                                 const char *label,
                                 imgui_vec2 size)
{
    imgui_scope scope;
    imgui_rect rect;
    imgui_id id;
    int child_index;
    if (size.x <= 0.0f) size.x = 220.0f;
    if (size.y <= 0.0f) size.y = 100.0f;
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    if (!imgui_item_register(ctx, id, size)) {
        return IMGUI_SCOPE_ERROR;
    }
    rect = ctx->last_item_rect;
    (void)imgui_add_frame_surface(ctx, rect, ctx->style.color_frame);
    child_index = imgui_child_find_or_create(ctx, id);
    if (child_index < 0) return IMGUI_SCOPE_ERROR;
    scope = imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_LIST_BOX,
                                       IMGUI_TRUE);
    if (scope != IMGUI_SCOPE_ACTIVE) return scope;
    ctx->scope_saved_size[ctx->scope_depth - 1] = size;
    ctx->list_box_child_index = child_index;
    ctx->list_box_rect = rect;
    ctx->list_box_scroll_y = ctx->child_scrolls[child_index];
    ctx->list_box_scroll_max_y = ctx->child_scroll_maxs[child_index];
    ctx->list_box_saved_content_max_x = ctx->content_max_x;
    ctx->list_box_saved_content_max_y = ctx->content_max_y;
    if (ctx->input.mouse_x >= rect.x1 && ctx->input.mouse_x < rect.x2 &&
        ctx->input.mouse_y >= rect.y1 && ctx->input.mouse_y < rect.y2 &&
        ctx->input.mouse_wheel_y != 0.0f) {
        ctx->list_box_scroll_y += ctx->input.mouse_wheel_y * 20.0f;
        if (ctx->list_box_scroll_y < 0.0f) ctx->list_box_scroll_y = 0.0f;
        if (ctx->list_box_scroll_y > ctx->list_box_scroll_max_y) {
            ctx->list_box_scroll_y = ctx->list_box_scroll_max_y;
        }
        ctx->input.mouse_wheel_y = 0.0f;
    }
    imgui_push_id_value(ctx, id);
    ctx->clip_rect = rect;
    if (ctx->clip_rect.x1 < 0.0f) ctx->clip_rect.x1 = 0.0f;
    if (ctx->clip_rect.y1 < 0.0f) ctx->clip_rect.y1 = 0.0f;
    if (ctx->clip_rect.x2 > ctx->frame_desc.display_size.x) {
        ctx->clip_rect.x2 = ctx->frame_desc.display_size.x;
    }
    if (ctx->clip_rect.y2 > ctx->frame_desc.display_size.y) {
        ctx->clip_rect.y2 = ctx->frame_desc.display_size.y;
    }
    ctx->cursor.x = rect.x1 + ctx->style.frame_padding.x;
    ctx->cursor.y = rect.y1 + ctx->style.frame_padding.y -
                    ctx->list_box_scroll_y;
    ctx->content_max_x = ctx->cursor.x;
    ctx->content_max_y = ctx->cursor.y;
    return scope;
}

void imgui_list_box_end(imgui_context *ctx)
{
    if (ctx != NULL && ctx->list_box_child_index >= 0 &&
        ctx->list_box_child_index < ctx->child_count) {
        float content_bottom;
        float maximum;
        content_bottom = ctx->content_max_y + ctx->list_box_scroll_y;
        maximum = content_bottom -
                  (ctx->list_box_rect.y2 - ctx->style.frame_padding.y);
        if (maximum < 0.0f) maximum = 0.0f;
        ctx->list_box_scroll_max_y = maximum;
        if (ctx->list_box_scroll_y > maximum) {
            ctx->list_box_scroll_y = maximum;
        }
        ctx->child_scrolls[ctx->list_box_child_index] =
            ctx->list_box_scroll_y;
        ctx->child_scroll_maxs[ctx->list_box_child_index] = maximum;
        if (maximum > 0.0f) {
            imgui_rect track;
            imgui_rect grab;
            float track_height;
            float grab_height;
            float grab_range;
            track.x1 = ctx->list_box_rect.x2 - ctx->style.scrollbar_size;
            track.x2 = ctx->list_box_rect.x2 - 2.0f;
            track.y1 = ctx->list_box_rect.y1 + 1.0f;
            track.y2 = ctx->list_box_rect.y2 - 2.0f;
            track_height = track.y2 - track.y1;
            grab_height = track_height * track_height /
                          (track_height + maximum);
            if (grab_height < ctx->style.scrollbar_grab_min_size) {
                grab_height = ctx->style.scrollbar_grab_min_size;
            }
            if (grab_height > track_height) grab_height = track_height;
            grab_range = track_height - grab_height;
            grab = track;
            if (grab_range > 0.0f) {
                grab.y1 += grab_range * ctx->list_box_scroll_y / maximum;
            }
            grab.y2 = grab.y1 + grab_height;
            (void)imgui_mesh_add_rect(
                ctx, track, ctx->style.color_scrollbar_background);
            (void)imgui_mesh_add_rect(
                ctx, grab, ctx->style.color_scrollbar_grab);
        }
        ctx->content_max_x = ctx->list_box_saved_content_max_x;
        ctx->content_max_y = ctx->list_box_saved_content_max_y;
    }
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_LIST_BOX);
    if (ctx != NULL) ctx->list_box_child_index = -1;
}

imgui_scope imgui_menu_bar_begin(imgui_context *ctx)
{
    imgui_scope scope;
    imgui_rect bar;
    imgui_bool active;
    if (ctx == NULL || !imgui_internal_require_building(
            ctx, "menu bar begin outside frame")) {
        return IMGUI_SCOPE_ERROR;
    }
    /* The C API treats BeginMenuBar as an explicit scope request.  Callers
       may use it on a window without predeclaring a menu-bar flag; unlike the
       C++ helper this keeps the scope useful for custom window descriptors. */
    active = ctx->window_active ? IMGUI_TRUE : IMGUI_FALSE;
    scope = imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_MENU_BAR,
                                       active);
    if (scope != IMGUI_SCOPE_ACTIVE) return scope;
    bar.x1 = ctx->cursor.x - ctx->style.window_padding.x;
    bar.y1 = ctx->cursor.y;
    bar.x2 = ctx->window_origin.x + ctx->window_size.x -
             ctx->style.window_padding.x;
    bar.y2 = bar.y1 + 22.0f;
    (void)imgui_mesh_add_rect(ctx, bar,
                              ctx->style.color_menu_bar_background);
    if (ctx->clip_rect.x1 < bar.x1) ctx->clip_rect.x1 = bar.x1;
    if (ctx->clip_rect.y1 < bar.y1) ctx->clip_rect.y1 = bar.y1;
    if (ctx->clip_rect.x2 > bar.x2) ctx->clip_rect.x2 = bar.x2;
    if (ctx->clip_rect.y2 > bar.y2) ctx->clip_rect.y2 = bar.y2;
    return scope;
}

void imgui_menu_bar_end(imgui_context *ctx)
{
    int scope_index;
    imgui_vec2 saved_cursor;
    if (ctx != NULL && ctx->scope_depth > 0 &&
        ctx->scopes[ctx->scope_depth - 1] == IMGUI_INTERNAL_SCOPE_MENU_BAR) {
        scope_index = ctx->scope_depth - 1;
        saved_cursor = ctx->scope_saved_cursor[scope_index];
        if (ctx->scope_active[scope_index] &&
            ctx->cursor.y < saved_cursor.y + 22.0f + ctx->item_spacing) {
            ctx->cursor.y = saved_cursor.y + 22.0f + ctx->item_spacing;
        }
    }
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_MENU_BAR);
}

imgui_scope imgui_menu_begin(imgui_context *ctx,
                             const char *label,
                             imgui_bool enabled)
{
    imgui_vec2 size;
    imgui_scope scope;
    imgui_bool active;
    imgui_id id;
    int menu_stack_index;
    (void)enabled;
    size = imgui_label_size(ctx, label);
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    if (!enabled) ++ctx->disabled_depth;
    if (!imgui_item_register(ctx, imgui_get_id_string(ctx, label), size)) {
        if (!enabled) --ctx->disabled_depth;
        return IMGUI_SCOPE_ERROR;
    }
    (void)imgui_add_frame_surface(
        ctx, ctx->last_item_rect,
        !enabled ? ctx->style.color_frame :
        (ctx->last_item_active ? ctx->style.color_menu_item_active :
         (ctx->last_item_hovered ? ctx->style.color_menu_item_hovered :
                                   ctx->style.color_menu_item)));
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        imgui_text_draw_font(ctx, label, imgui_label_visible_end(label),
                             imgui_make_vec2(ctx->last_item_rect.x1 +
                                             ctx->style.frame_padding.x,
                             imgui_item_label_y(ctx,
                                                 ctx->last_item_rect)));
    }
    if (!enabled) --ctx->disabled_depth;
    if (enabled && ctx->popup_open && ctx->last_item_hovered &&
        ctx->popup_id != id && !ctx->last_item_clicked) {
        if (ctx->current_popup_id != 0 && ctx->popup_stack_count > 1) {
            --ctx->popup_stack_count;
        }
        menu_stack_index = imgui_popup_stack_find(ctx, id);
        if (menu_stack_index < 0 &&
            ctx->popup_stack_count < IMGUI_INTERNAL_POPUP_CAPACITY) {
            menu_stack_index = ctx->popup_stack_count++;
        }
        if (menu_stack_index >= 0) {
            ctx->popup_stack_ids[menu_stack_index] = id;
            ctx->popup_stack_modal[menu_stack_index] = IMGUI_FALSE;
        }
        ctx->popup_id = id;
        ctx->popup_open = IMGUI_TRUE;
        if (ctx->current_popup_id != 0) {
            ctx->popup_rect.x1 = ctx->last_item_rect.x2;
            ctx->popup_rect.y1 = ctx->last_item_rect.y1;
        } else {
            ctx->popup_rect.x1 = ctx->last_item_rect.x1;
            ctx->popup_rect.y1 = ctx->last_item_rect.y2;
        }
        ctx->popup_rect.x2 = ctx->popup_rect.x1 + 220.0f;
        ctx->popup_rect.y2 = ctx->popup_rect.y1 + 200.0f;
            ctx->popup_rect_valid = IMGUI_TRUE;
            if (menu_stack_index >= 0) {
                ctx->popup_stack_rects[menu_stack_index] = ctx->popup_rect;
                ctx->popup_stack_rect_valid[menu_stack_index] = IMGUI_TRUE;
            }
    } else if (enabled && ctx->last_item_clicked) {
        if (ctx->popup_open && ctx->popup_id == id) {
            ctx->popup_open = IMGUI_FALSE;
            imgui_popup_stack_clear(ctx);
        } else {
            ctx->popup_id = id;
            ctx->popup_open = IMGUI_TRUE;
            ctx->popup_mouse_button = IMGUI_MOUSE_BUTTON_LEFT;
            ctx->popup_stack_count = 1;
            ctx->popup_stack_ids[0] = id;
            ctx->popup_rect.x1 = ctx->last_item_rect.x1;
            ctx->popup_rect.y1 = ctx->last_item_rect.y2;
            ctx->popup_rect.x2 = ctx->popup_rect.x1 + 220.0f;
            ctx->popup_rect.y2 = ctx->popup_rect.y1 + 200.0f;
            ctx->popup_rect_valid = IMGUI_TRUE;
            ctx->popup_stack_rects[0] = ctx->popup_rect;
            ctx->popup_stack_rect_valid[0] = IMGUI_TRUE;
        }
    }
    active = enabled && ctx->popup_open &&
             imgui_popup_stack_find(ctx, id) >= 0;
    scope = imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_MENU,
                                       active ? IMGUI_TRUE : IMGUI_FALSE);
    if (scope != IMGUI_SCOPE_ACTIVE) return scope;
    imgui_push_id_value(ctx, id);
    ctx->current_popup_id = id;
    ctx->clip_rect.x1 = 0.0f;
    ctx->clip_rect.y1 = 0.0f;
    ctx->clip_rect.x2 = ctx->frame_desc.display_size.x;
    ctx->clip_rect.y2 = ctx->frame_desc.display_size.y;
    (void)imgui_mesh_add_rect(ctx, ctx->popup_rect,
                              ctx->style.color_popup_background);
    ctx->clip_rect = ctx->popup_rect;
    if (ctx->clip_rect.x1 < 0.0f) ctx->clip_rect.x1 = 0.0f;
    if (ctx->clip_rect.y1 < 0.0f) ctx->clip_rect.y1 = 0.0f;
    if (ctx->clip_rect.x2 > ctx->frame_desc.display_size.x) {
        ctx->clip_rect.x2 = ctx->frame_desc.display_size.x;
    }
    if (ctx->clip_rect.y2 > ctx->frame_desc.display_size.y) {
        ctx->clip_rect.y2 = ctx->frame_desc.display_size.y;
    }
    ctx->cursor.x = ctx->popup_rect.x1;
    ctx->cursor.y = ctx->popup_rect.y1;
    return scope;
}

void imgui_menu_end(imgui_context *ctx)
{
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_MENU);
    if (ctx != NULL) {
        if (ctx->popup_stack_count > 1 &&
            ctx->current_popup_id == ctx->popup_id) {
            --ctx->popup_stack_count;
            imgui_popup_stack_sync_top(ctx);
            ctx->current_popup_id = ctx->popup_id;
        } else {
            ctx->current_popup_id = 0;
        }
    }
}

imgui_bool imgui_menu_item(imgui_context *ctx,
                           const char *label,
                           const char *shortcut,
                           imgui_bool enabled)
{
    imgui_id id;
    imgui_vec2 size;
    imgui_vec2 shortcut_size;
    float label_width;
    float shortcut_gap;
    imgui_u32 old_text_color;
    if (ctx == NULL) return IMGUI_FALSE;
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    size = imgui_label_size(ctx, label);
    if (shortcut != NULL) {
        label_width = imgui_label_measure_width(ctx, label);
        if (ctx->font != NULL && ctx->font_texture != NULL) {
            shortcut_size = imgui_font_measure_text(ctx->font, shortcut,
                                                    NULL, 0.0f);
        } else {
            shortcut_size = imgui_calc_text_size(shortcut, NULL, 0.0f);
        }
        shortcut_gap = ctx->style.frame_padding.x * 3.0f;
        if (size.x < label_width + shortcut_size.x + shortcut_gap) {
            size.x = label_width + shortcut_size.x + shortcut_gap;
        }
    }
    if (!enabled) ++ctx->disabled_depth;
    if (!imgui_item_register(ctx, id, size)) {
        if (!enabled) --ctx->disabled_depth;
        return IMGUI_FALSE;
    }
    if (!enabled) --ctx->disabled_depth;
    (void)imgui_add_frame_surface(
        ctx, ctx->last_item_rect,
        enabled ? (ctx->last_item_active ? ctx->style.color_menu_item_active :
                   (ctx->last_item_hovered ? ctx->style.color_menu_item_hovered :
                                             ctx->style.color_menu_item)) :
                  ctx->style.color_frame);
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        old_text_color = ctx->style.color_text;
        if (!enabled) ctx->style.color_text = ctx->style.color_text_disabled;
        if (label != NULL) {
            const char *hidden = strstr(label, "##");
            imgui_text_draw_font(ctx, label, hidden != NULL ? hidden :
                                 label + strlen(label), imgui_make_vec2(
                                     ctx->last_item_rect.x1 +
                                     ctx->style.frame_padding.x,
                                     imgui_item_label_y(ctx,
                                                        ctx->last_item_rect)));
        }
        if (shortcut != NULL) {
            shortcut_size = imgui_font_measure_text(ctx->font, shortcut,
                                                    NULL, 0.0f);
            imgui_text_draw_font(ctx, shortcut,
                                 shortcut + strlen(shortcut), imgui_make_vec2(
                                     ctx->last_item_rect.x2 -
                                     ctx->style.frame_padding.x - shortcut_size.x,
                                     imgui_item_label_y(ctx,
                                                        ctx->last_item_rect)));
        }
        ctx->style.color_text = old_text_color;
    }
    if (enabled && ctx->last_item_clicked) {
        if (ctx->popup_open && ctx->current_popup_id != 0) {
            ctx->popup_open = IMGUI_FALSE;
            imgui_popup_stack_clear(ctx);
        }
        return IMGUI_TRUE;
    }
    return IMGUI_FALSE;
}

imgui_bool imgui_menu_item_shortcut(imgui_context *ctx,
                                    const char *label,
                                    const char *shortcut_text,
                                    imgui_key key,
                                    imgui_key_modifiers modifiers,
                                    imgui_bool enabled)
{
    imgui_bool clicked;
    if (ctx == NULL || !enabled) {
        (void)imgui_menu_item(ctx, label, shortcut_text, enabled);
        return IMGUI_FALSE;
    }
    clicked = imgui_menu_item(ctx, label, shortcut_text, enabled);
    if (clicked) return IMGUI_TRUE;
    if (imgui_is_shortcut_pressed(ctx, key, modifiers, IMGUI_FALSE)) {
        /* Keyboard activation follows the same close-on-activation rule as
           mouse activation, including nested menu hierarchies represented by
           the current popup scope. */
        if (ctx->popup_open && ctx->current_popup_id != 0) {
            ctx->popup_open = IMGUI_FALSE;
            imgui_popup_stack_clear(ctx);
        }
        return IMGUI_TRUE;
    }
    return IMGUI_FALSE;
}

imgui_bool imgui_menu_item_toggle(imgui_context *ctx,
                                  const char *label,
                                  const char *shortcut,
                                  imgui_bool *selected,
                                  imgui_bool enabled)
{
    (void)shortcut;
    if (selected == NULL || !enabled) {
        return imgui_menu_item(ctx, label, shortcut, enabled);
    }
    if (imgui_menu_item(ctx, label, shortcut, enabled)) {
        *selected = *selected ? IMGUI_FALSE : IMGUI_TRUE;
        return IMGUI_TRUE;
    }
    return IMGUI_FALSE;
}

void imgui_popup_open(imgui_context *ctx,
                      const char *id,
                      imgui_popup_flags flags)
{
    imgui_id popup_id;
    if (imgui_internal_require_building(ctx, "popup open outside frame")) {
        if ((flags & IMGUI_POPUP_NO_OPEN_OVER_EXISTING) != 0 &&
            ctx->popup_open) {
            return;
        }
        if ((flags & IMGUI_POPUP_NO_OPEN_OVER_ITEMS) != 0 &&
            ctx->last_item_hovered) {
            return;
        }
        popup_id = imgui_get_id_string(ctx, id != NULL ? id : "");
        if (ctx->popup_open && ctx->current_popup_id != 0 &&
            ctx->popup_id != popup_id) {
            if (ctx->popup_stack_count >= IMGUI_INTERNAL_POPUP_CAPACITY) {
                return;
            }
            ++ctx->popup_stack_count;
            ctx->popup_stack_ids[ctx->popup_stack_count - 1] = popup_id;
            ctx->popup_stack_modal[ctx->popup_stack_count - 1] = IMGUI_FALSE;
            ctx->popup_id = popup_id;
            ctx->popup_opening_mouse_position = imgui_make_vec2(
                ctx->input.mouse_x, ctx->input.mouse_y);
            ctx->popup_rect.x1 = ctx->last_item_id != 0 &&
                ctx->last_item_rect.x2 > ctx->last_item_rect.x1 ?
                ctx->last_item_rect.x2 : ctx->popup_rect.x2;
            ctx->popup_rect.y1 = ctx->last_item_id != 0 &&
                ctx->last_item_rect.y2 > ctx->last_item_rect.y1 ?
                ctx->last_item_rect.y1 : ctx->popup_rect.y1;
            ctx->popup_rect.x2 = ctx->popup_rect.x1 + 220.0f;
            ctx->popup_rect.y2 = ctx->popup_rect.y1 + 200.0f;
            ctx->popup_rect_valid = IMGUI_TRUE;
            ctx->popup_stack_rects[ctx->popup_stack_count - 1] =
                ctx->popup_rect;
            ctx->popup_stack_rect_valid[ctx->popup_stack_count - 1] =
                IMGUI_TRUE;
            return;
        }
        if (ctx->popup_open && ctx->popup_id == popup_id &&
            ctx->popup_rect_valid) {
            /* Re-opening the same popup while it is already visible must not
               move its anchor or invalidate its outside-click bounds. */
            return;
        }
        ctx->popup_id = popup_id;
        ctx->popup_opening_mouse_position = imgui_make_vec2(
            ctx->input.mouse_x, ctx->input.mouse_y);
        ctx->popup_open = IMGUI_TRUE;
        ctx->popup_stack_count = 1;
        ctx->popup_stack_ids[0] = popup_id;
        ctx->popup_stack_modal[0] = IMGUI_FALSE;
        ctx->popup_mouse_button =
            (flags & IMGUI_POPUP_MOUSE_BUTTON_MIDDLE) != 0 ?
            IMGUI_MOUSE_BUTTON_MIDDLE :
            ((flags & IMGUI_POPUP_MOUSE_BUTTON_RIGHT) != 0 ?
             IMGUI_MOUSE_BUTTON_RIGHT : IMGUI_MOUSE_BUTTON_LEFT);
        if (ctx->last_item_id != 0 && ctx->last_item_rect.x2 >
            ctx->last_item_rect.x1 && ctx->last_item_rect.y2 >
            ctx->last_item_rect.y1) {
            ctx->popup_rect.x1 = ctx->last_item_rect.x1;
            ctx->popup_rect.y1 = ctx->last_item_rect.y2;
            ctx->popup_rect.x2 = ctx->popup_rect.x1 + 220.0f;
            ctx->popup_rect.y2 = ctx->popup_rect.y1 + 200.0f;
            ctx->popup_rect_valid = IMGUI_TRUE;
        } else {
            ctx->popup_rect_valid = IMGUI_FALSE;
        }
        if (ctx->popup_rect_valid) {
            ctx->popup_stack_rects[0] = ctx->popup_rect;
        }
        ctx->popup_stack_rect_valid[0] = ctx->popup_rect_valid;
    }
}

imgui_result imgui_popup_open_on_item_click(imgui_context *ctx,
                                            const char *id,
                                            imgui_popup_flags flags)
{
    imgui_mouse_button button;
    if (!imgui_internal_require_building(
            ctx, "popup open on item click outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if ((flags & IMGUI_POPUP_MOUSE_BUTTON_MIDDLE) != 0) {
        button = IMGUI_MOUSE_BUTTON_MIDDLE;
    } else if ((flags & IMGUI_POPUP_MOUSE_BUTTON_RIGHT) != 0) {
        button = IMGUI_MOUSE_BUTTON_RIGHT;
    } else {
        button = IMGUI_MOUSE_BUTTON_LEFT;
    }
    if (ctx->last_item_hovered && ctx->mouse_clicked[button]) {
        imgui_popup_open(ctx, id, flags);
    }
    return IMGUI_RESULT_OK;
}

imgui_bool imgui_popup_is_open(imgui_context *ctx, const char *id)
{
    imgui_id popup_id;
    int index;
    if (ctx == NULL || id == NULL) return IMGUI_FALSE;
    popup_id = imgui_get_id_string(ctx, id);
    if (!ctx->popup_open) return IMGUI_FALSE;
    if (ctx->popup_id == popup_id) return IMGUI_TRUE;
    for (index = 0; index < ctx->popup_stack_count; ++index) {
        if (ctx->popup_stack_ids[index] == popup_id) return IMGUI_TRUE;
    }
    return IMGUI_FALSE;
}

imgui_vec2 imgui_get_mouse_position_on_opening_current_popup(
    const imgui_context *ctx)
{
    if (ctx == NULL || !ctx->popup_open) {
        return imgui_make_vec2(0.0f, 0.0f);
    }
    return ctx->popup_opening_mouse_position;
}

imgui_scope imgui_popup_begin(imgui_context *ctx,
                              const char *id,
                              imgui_window_flags flags)
{
    imgui_scope scope;
    imgui_rect popup_rect;
    imgui_rect modal_rect;
    imgui_id popup_id;
    int popup_stack_index;
    if (!imgui_internal_require_building(ctx, "popup begin outside frame")) {
        return IMGUI_SCOPE_ERROR;
    }
    popup_id = imgui_get_id_string(ctx, id != NULL ? id : "");
    popup_stack_index = imgui_popup_stack_find(ctx, popup_id);
    if (ctx->popup_open && popup_stack_index >= 0) {
        ctx->popup_id = popup_id;
        ctx->popup_rect = ctx->popup_stack_rects[popup_stack_index];
        ctx->popup_rect_valid = ctx->popup_stack_rect_valid[
            popup_stack_index];
        ctx->current_popup_id = popup_id;
        ctx->popup_modal = (flags & IMGUI_WINDOW_MODAL) != 0 ?
                           IMGUI_TRUE : IMGUI_FALSE;
        ctx->popup_stack_modal[popup_stack_index] = ctx->popup_modal;
        ctx->frame_any_window_focused = IMGUI_TRUE;
        scope = imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_POPUP,
                                           IMGUI_TRUE);
        if (scope == IMGUI_SCOPE_ERROR) {
            return IMGUI_SCOPE_ERROR;
        }
        imgui_push_id_value(ctx, popup_id);
        if (popup_stack_index == 0 && !ctx->popup_render_valid) {
            ctx->popup_render_start = ctx->command_count;
        }
        /* OpenPopup() anchors once.  An open popup must not follow the
           pointer on later frames: Dear ImGui keeps its position stable so
           moving outside and clicking can reliably dismiss it. */
        if (!ctx->popup_rect_valid) {
            ctx->popup_rect.x1 = ctx->input.mouse_x + 8.0f;
            ctx->popup_rect.y1 = ctx->input.mouse_y + 8.0f;
            ctx->popup_rect.x2 = ctx->popup_rect.x1 + 220.0f;
            ctx->popup_rect.y2 = ctx->popup_rect.y1 + 200.0f;
            ctx->popup_rect_valid = IMGUI_TRUE;
        }
        popup_rect = ctx->popup_rect;
        if (popup_rect.x2 > ctx->frame_desc.display_size.x) {
            popup_rect.x1 -= popup_rect.x2 - ctx->frame_desc.display_size.x;
            popup_rect.x2 = ctx->frame_desc.display_size.x;
        }
        if (popup_rect.y2 > ctx->frame_desc.display_size.y) {
            popup_rect.y1 -= popup_rect.y2 - ctx->frame_desc.display_size.y;
            popup_rect.y2 = ctx->frame_desc.display_size.y;
        }
        if (popup_rect.x1 < 0.0f) popup_rect.x1 = 0.0f;
        if (popup_rect.y1 < 0.0f) popup_rect.y1 = 0.0f;
        ctx->popup_rect = popup_rect;
        ctx->popup_stack_rects[popup_stack_index] = popup_rect;
        ctx->popup_stack_rect_valid[popup_stack_index] = IMGUI_TRUE;
        if (ctx->input.mouse_x >= popup_rect.x1 &&
            ctx->input.mouse_x < popup_rect.x2 &&
            ctx->input.mouse_y >= popup_rect.y1 &&
            ctx->input.mouse_y < popup_rect.y2) {
            ctx->frame_any_window_hovered = IMGUI_TRUE;
        }
        ctx->clip_rect.x1 = 0.0f;
        ctx->clip_rect.y1 = 0.0f;
        ctx->clip_rect.x2 = ctx->frame_desc.display_size.x;
        ctx->clip_rect.y2 = ctx->frame_desc.display_size.y;
        if (ctx->popup_modal) {
            modal_rect.x1 = 0.0f;
            modal_rect.y1 = 0.0f;
            modal_rect.x2 = ctx->frame_desc.display_size.x;
            modal_rect.y2 = ctx->frame_desc.display_size.y;
            (void)imgui_mesh_add_rect(ctx, modal_rect,
                                      ctx->style.color_modal_dim);
        }
        if ((flags & IMGUI_WINDOW_NO_BACKGROUND) == 0) {
            (void)imgui_mesh_add_rect(ctx, popup_rect,
                                      ctx->style.color_popup_background);
            ctx->popup_background_vertex_offset = ctx->vertex_count - 4U;
            ctx->popup_background_active = IMGUI_TRUE;
        } else {
            ctx->popup_background_active = IMGUI_FALSE;
        }
        ctx->clip_rect = popup_rect;
        if (ctx->clip_rect.x1 < 0.0f) ctx->clip_rect.x1 = 0.0f;
        if (ctx->clip_rect.y1 < 0.0f) ctx->clip_rect.y1 = 0.0f;
        if (ctx->clip_rect.x2 > ctx->frame_desc.display_size.x) {
            ctx->clip_rect.x2 = ctx->frame_desc.display_size.x;
        }
        if (ctx->clip_rect.y2 > ctx->frame_desc.display_size.y) {
            ctx->clip_rect.y2 = ctx->frame_desc.display_size.y;
        }
        ctx->cursor.x = ctx->popup_rect.x1;
        ctx->cursor.y = ctx->popup_rect.y1;
        return scope;
    }
    return imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_POPUP,
                                      IMGUI_FALSE);
}

void imgui_popup_end(imgui_context *ctx)
{
    if (ctx != NULL && ctx->popup_background_active &&
        ctx->popup_background_vertex_offset + 3U < ctx->vertex_count) {
        imgui_u32 offset = ctx->popup_background_vertex_offset;
        imgui_rect rect = ctx->popup_rect;
        ctx->vertices[offset + 0U].position =
            imgui_make_vec2(rect.x1, rect.y1);
        ctx->vertices[offset + 1U].position =
            imgui_make_vec2(rect.x2, rect.y1);
        ctx->vertices[offset + 2U].position =
            imgui_make_vec2(rect.x2, rect.y2);
        ctx->vertices[offset + 3U].position =
            imgui_make_vec2(rect.x1, rect.y2);
        (void)imgui_mesh_add_window_border(
            ctx, rect, ctx->style.color_popup_border);
        ctx->popup_background_active = IMGUI_FALSE;
    }
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_POPUP);
    if (ctx != NULL && ctx->popup_stack_count > 1) {
        --ctx->popup_stack_count;
        imgui_popup_stack_sync_top(ctx);
        ctx->current_popup_id = ctx->popup_id;
        ctx->popup_modal = ctx->popup_stack_modal[
            ctx->popup_stack_count - 1];
    } else if (ctx != NULL) {
        /* The popup remains open for outside-click handling, but the caller
           has returned to the parent window scope. */
        ctx->popup_render_end = ctx->command_count;
        ctx->popup_render_valid = IMGUI_TRUE;
        ctx->current_popup_id = 0;
    }
}

void imgui_popup_close_current(imgui_context *ctx)
{
    if (imgui_internal_require_building(ctx, "popup close outside frame")) {
        ctx->popup_open = IMGUI_FALSE;
        imgui_popup_stack_clear(ctx);
    }
}

imgui_scope imgui_tooltip_begin(imgui_context *ctx)
{
    imgui_scope scope;
    imgui_rect rect;
    if (ctx == NULL || !imgui_internal_require_building(ctx,
                                                        "tooltip begin outside frame")) {
        return IMGUI_SCOPE_ERROR;
    }
    scope = imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_TOOLTIP,
                                       IMGUI_TRUE);
    if (scope == IMGUI_SCOPE_ERROR) return scope;
    ctx->tooltip_render_start = ctx->command_count;
    ctx->tooltip_render_tracking = IMGUI_TRUE;
    ctx->tooltip_saved_window_flags = ctx->window_flags;
    ctx->tooltip_saved_window_active = ctx->window_active;
    ctx->window_flags |= IMGUI_WINDOW_NO_MOUSE_INPUTS;
    ctx->window_active = IMGUI_FALSE;
    rect.x1 = ctx->input.mouse_x + 12.0f;
    rect.y1 = ctx->input.mouse_y + 12.0f;
    rect.x2 = rect.x1 + 2.0f * ctx->style.window_padding.x;
    rect.y2 = rect.y1 + 2.0f * ctx->style.window_padding.y;
    if (rect.x2 > ctx->frame_desc.display_size.x) {
        rect.x1 -= rect.x2 - ctx->frame_desc.display_size.x;
        rect.x2 = ctx->frame_desc.display_size.x;
    }
    if (rect.y2 > ctx->frame_desc.display_size.y) {
        rect.y1 -= rect.y2 - ctx->frame_desc.display_size.y;
        rect.y2 = ctx->frame_desc.display_size.y;
    }
    if (rect.x1 < 0.0f) rect.x1 = 0.0f;
    if (rect.y1 < 0.0f) rect.y1 = 0.0f;
    ctx->tooltip_rect = rect;
    ctx->tooltip_background_vertex_offset = ctx->vertex_count;
    ctx->tooltip_background_active = IMGUI_TRUE;
    (void)imgui_mesh_add_rect(ctx, rect, ctx->style.color_popup_background);
    ctx->clip_rect = rect;
    if (ctx->clip_rect.x1 < 0.0f) ctx->clip_rect.x1 = 0.0f;
    if (ctx->clip_rect.y1 < 0.0f) ctx->clip_rect.y1 = 0.0f;
    if (ctx->clip_rect.x2 > ctx->frame_desc.display_size.x) {
        ctx->clip_rect.x2 = ctx->frame_desc.display_size.x;
    }
    if (ctx->clip_rect.y2 > ctx->frame_desc.display_size.y) {
        ctx->clip_rect.y2 = ctx->frame_desc.display_size.y;
    }
    ctx->cursor.x = rect.x1 + ctx->style.window_padding.x;
    ctx->cursor.y = rect.y1 + ctx->style.window_padding.y;
    return scope;
}

imgui_scope imgui_item_tooltip_begin(imgui_context *ctx)
{
    if (ctx == NULL || !ctx->last_item_hovered) {
        return IMGUI_SCOPE_ERROR;
    }
    return imgui_tooltip_begin(ctx);
}

void imgui_tooltip_end(imgui_context *ctx)
{
    if (ctx != NULL && ctx->tooltip_background_active &&
        ctx->tooltip_background_vertex_offset + 3U < ctx->vertex_count) {
        imgui_u32 offset = ctx->tooltip_background_vertex_offset;
        imgui_rect rect = ctx->tooltip_rect;
        ctx->vertices[offset + 0U].position =
            imgui_make_vec2(rect.x1, rect.y1);
        ctx->vertices[offset + 1U].position =
            imgui_make_vec2(rect.x2, rect.y1);
        ctx->vertices[offset + 2U].position =
            imgui_make_vec2(rect.x2, rect.y2);
        ctx->vertices[offset + 3U].position =
            imgui_make_vec2(rect.x1, rect.y2);
        (void)imgui_mesh_add_window_border(ctx, rect,
                                           ctx->style.color_popup_border);
        ctx->tooltip_background_active = IMGUI_FALSE;
    }
    if (ctx != NULL && ctx->tooltip_render_tracking &&
        ctx->tooltip_render_start <= ctx->command_count) {
        ctx->tooltip_render_end = ctx->command_count;
        ctx->tooltip_render_valid = IMGUI_TRUE;
        ctx->tooltip_render_tracking = IMGUI_FALSE;
    }
    if (ctx != NULL) {
        ctx->window_flags = ctx->tooltip_saved_window_flags;
        ctx->window_active = ctx->tooltip_saved_window_active;
    }
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_TOOLTIP);
}

static void imgui_set_tooltip_v(imgui_context *ctx, imgui_bool item_only,
                                const char *format, va_list arguments)
{
    char text[2048];
    imgui_scope scope;
    if (ctx == NULL || format == NULL ||
        (item_only && !ctx->last_item_hovered)) return;
    (void)vsnprintf(text, sizeof(text), format, arguments);
    text[sizeof(text) - 1U] = '\0';
    scope = imgui_tooltip_begin(ctx);
    if (scope == IMGUI_SCOPE_ACTIVE) {
        imgui_text_unformatted(ctx, text, NULL);
        imgui_tooltip_end(ctx);
    }
}

void imgui_set_tooltip(imgui_context *ctx, const char *format, ...)
{
    va_list arguments;
    if (format == NULL) return;
    va_start(arguments, format);
    imgui_set_tooltip_v(ctx, IMGUI_FALSE, format, arguments);
    va_end(arguments);
}

void imgui_set_item_tooltip(imgui_context *ctx, const char *format, ...)
{
    va_list arguments;
    if (format == NULL) return;
    va_start(arguments, format);
    imgui_set_tooltip_v(ctx, IMGUI_TRUE, format, arguments);
    va_end(arguments);
}

imgui_scope imgui_tree_node_begin(imgui_context *ctx,
                                  const char *label,
                                  imgui_tree_flags flags)
{
    return imgui_tree_node_begin_with_id(ctx,
                                         imgui_get_id_string(ctx, label),
                                         label, flags);
}

static int imgui_tree_state_index(imgui_context *ctx, imgui_id id)
{
    int index;
    for (index = 0; index < ctx->tree_count; ++index) {
        if (ctx->tree_ids[index] == id) return index;
    }
    if (ctx->tree_count >= ctx->tree_capacity) {
        int capacity;
        imgui_id *ids;
        imgui_bool *open;
        imgui_bool *initialized;
        capacity = ctx->tree_capacity > 0 ? ctx->tree_capacity :
                   IMGUI_INTERNAL_TREE_CAPACITY;
        if (capacity > INT_MAX / 2) capacity = INT_MAX;
        else capacity *= 2;
        if ((size_t)capacity > (size_t)-1 / sizeof(*ids)) return -1;
        ids = (imgui_id *)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*ids));
        open = (imgui_bool *)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*open));
        initialized = (imgui_bool *)imgui_internal_allocate(
            &ctx->allocator, (size_t)capacity * sizeof(*initialized));
        if (ids == NULL || open == NULL || initialized == NULL) {
            imgui_internal_release(&ctx->allocator, ids);
            imgui_internal_release(&ctx->allocator, open);
            imgui_internal_release(&ctx->allocator, initialized);
            return -1;
        }
        if (ctx->tree_count != 0) {
            memcpy(ids, ctx->tree_ids,
                   (size_t)ctx->tree_count * sizeof(*ids));
            memcpy(open, ctx->tree_open,
                   (size_t)ctx->tree_count * sizeof(*open));
            memcpy(initialized, ctx->tree_initialized,
                   (size_t)ctx->tree_count * sizeof(*initialized));
        }
        imgui_internal_release(&ctx->allocator, ctx->tree_ids);
        imgui_internal_release(&ctx->allocator, ctx->tree_open);
        imgui_internal_release(&ctx->allocator, ctx->tree_initialized);
        ctx->tree_ids = ids;
        ctx->tree_open = open;
        ctx->tree_initialized = initialized;
        ctx->tree_capacity = capacity;
    }
    index = ctx->tree_count++;
    ctx->tree_ids[index] = id;
    ctx->tree_open[index] = IMGUI_FALSE;
    ctx->tree_initialized[index] = IMGUI_FALSE;
    return index;
}

imgui_scope imgui_tree_node_begin_with_id(imgui_context *ctx,
                                          imgui_id id,
                                          const char *label,
                                          imgui_tree_flags flags)
{
    imgui_vec2 size;
    int state_index;
    imgui_bool open;
    (void)flags;
    if (ctx == NULL ||
        !imgui_internal_require_building(ctx, "tree begin outside frame")) {
        return IMGUI_SCOPE_ERROR;
    }
    state_index = imgui_tree_state_index(ctx, id);
    if (state_index < 0) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_STATE,
                              "tree state capacity exceeded");
        return IMGUI_SCOPE_ERROR;
    }
    if (ctx->next_item_open_valid) {
        ctx->tree_open[state_index] = ctx->next_item_open;
        ctx->tree_initialized[state_index] = IMGUI_TRUE;
        ctx->next_item_open_valid = IMGUI_FALSE;
    } else if (!ctx->tree_initialized[state_index]) {
        ctx->tree_open[state_index] =
            (flags & IMGUI_TREE_DEFAULT_OPEN) != 0 ?
            IMGUI_TRUE : IMGUI_FALSE;
        ctx->tree_initialized[state_index] = IMGUI_TRUE;
    }
    size = imgui_make_vec2(imgui_label_measure_width(ctx, label) + 28.0f,
                           22.0f);
    if (!imgui_item_register(ctx, id, size)) return IMGUI_SCOPE_ERROR;
    ctx->last_item_toggled_open = IMGUI_FALSE;
    if (ctx->last_item_clicked && (flags & IMGUI_TREE_LEAF) == 0) {
        ctx->tree_open[state_index] = ctx->tree_open[state_index] ?
                                      IMGUI_FALSE : IMGUI_TRUE;
        ctx->last_item_toggled_open = IMGUI_TRUE;
    }
    open = ctx->tree_open[state_index];
    if (ctx->style.frame_rounding > 0.0f) {
        (void)imgui_draw_list_add_rect_rounded(
            ctx, &ctx->default_draw_list, ctx->last_item_rect,
            ctx->style.frame_rounding,
            open ? ctx->style.color_header_active :
            (ctx->last_item_hovered ? ctx->style.color_header_hovered :
                                      ctx->style.color_header),
            4, NULL);
    } else {
        (void)imgui_mesh_add_rect(ctx, ctx->last_item_rect,
                                  open ? ctx->style.color_header_active :
                                  (ctx->last_item_hovered ?
                                   ctx->style.color_header_hovered :
                                   ctx->style.color_header));
    }
    if ((flags & IMGUI_TREE_LEAF) == 0) {
        if (open) {
            (void)imgui_mesh_add_triangle(
                ctx, imgui_make_vec2(ctx->last_item_rect.x1 + 5.0f,
                                     ctx->last_item_rect.y1 + 8.0f),
                imgui_make_vec2(ctx->last_item_rect.x1 + 15.0f,
                                ctx->last_item_rect.y1 + 8.0f),
                imgui_make_vec2(ctx->last_item_rect.x1 + 10.0f,
                                ctx->last_item_rect.y1 + 14.0f),
                ctx->style.color_text);
        } else {
            (void)imgui_mesh_add_triangle(
                ctx, imgui_make_vec2(ctx->last_item_rect.x1 + 7.0f,
                                     ctx->last_item_rect.y1 + 6.0f),
                imgui_make_vec2(ctx->last_item_rect.x1 + 7.0f,
                                ctx->last_item_rect.y1 + 16.0f),
                imgui_make_vec2(ctx->last_item_rect.x1 + 13.0f,
                                ctx->last_item_rect.y1 + 11.0f),
                ctx->style.color_text);
        }
    }
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        const char *hidden = strstr(label, "##");
        imgui_text_draw_font(ctx, label, hidden != NULL ? hidden :
                             label + strlen(label), imgui_make_vec2(
                                 ctx->last_item_rect.x1 + 20.0f,
                                 imgui_item_label_y(ctx,
                                                    ctx->last_item_rect)));
    }
    {
        imgui_scope scope;
        scope = imgui_internal_scope_begin(
            ctx, IMGUI_INTERNAL_SCOPE_TREE,
            (flags & IMGUI_TREE_LEAF) != 0 ? IMGUI_FALSE : open);
        if (scope == IMGUI_SCOPE_ACTIVE) {
            if ((flags & IMGUI_TREE_NO_TREE_PUSH_ON_OPEN) != 0) {
                ctx->scope_tree_pushed[ctx->scope_depth - 1] = IMGUI_FALSE;
            } else {
                imgui_push_id_value(ctx, id);
                ctx->indent_width += ctx->style.indent_spacing;
                ctx->cursor.x += ctx->style.indent_spacing;
            }
        }
        return scope;
    }
}

void imgui_tree_node_end(imgui_context *ctx)
{
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_TREE);
}

imgui_bool imgui_collapsing_header(imgui_context *ctx, const char *label,
                                   imgui_tree_flags flags)
{
    imgui_scope scope;
    scope = imgui_tree_node_begin(ctx, label, flags);
    if (scope == IMGUI_SCOPE_ERROR) return IMGUI_FALSE;
    imgui_tree_node_end(ctx);
    return scope == IMGUI_SCOPE_ACTIVE ? IMGUI_TRUE : IMGUI_FALSE;
}

imgui_scope imgui_tab_bar_begin(imgui_context *ctx,
                                const char *id,
                                imgui_tab_flags flags)
{
    imgui_id tab_bar_id;
    (void)flags;
    if (ctx != NULL && imgui_internal_require_building(ctx,
                                                       "tab bar begin outside frame")) {
        tab_bar_id = imgui_get_id_string(ctx, id != NULL ? id : "");
        if (ctx->tab_bar_id != tab_bar_id) {
            ctx->tab_active_id = 0;
            ctx->tab_bar_id = tab_bar_id;
        }
        ctx->tab_bar_item_count = 0;
        ctx->tab_bar_row_start = ctx->cursor;
        ctx->tab_bar_next_x = ctx->cursor.x;
        ctx->tab_bar_row_y = ctx->cursor.y;
        ctx->tab_bar_content_cursor = ctx->cursor;
        ctx->tab_bar_content_valid = IMGUI_FALSE;
    }
    return imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_TAB_BAR,
                                      IMGUI_TRUE);
}

void imgui_tab_bar_end(imgui_context *ctx)
{
    if (ctx != NULL && ctx->tab_bar_content_valid) {
        ctx->cursor = ctx->tab_bar_content_cursor;
    }
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_TAB_BAR);
}

imgui_scope imgui_tab_item_begin(imgui_context *ctx,
                                 const char *label,
                                 imgui_bool *open,
                                 imgui_tab_flags flags)
{
    imgui_vec2 size;
    imgui_id id;
    imgui_id label_id;
    imgui_bool active;
    imgui_bool disabled;
    imgui_bool close_clicked;
    if (ctx == NULL) return IMGUI_SCOPE_ERROR;
    disabled = (flags & IMGUI_TAB_ITEM_DISABLED) != 0 ?
               IMGUI_TRUE : IMGUI_FALSE;
    if (disabled) ++ctx->disabled_depth;
    size = imgui_make_vec2(imgui_label_measure_width(ctx, label) +
                           (open != NULL ? 28.0f : 24.0f),
                           24.0f);
    label_id = imgui_get_id_string(ctx, label != NULL ? label : "");
    id = ctx != NULL && ctx->tab_bar_id != 0
        ? imgui_hash_bytes(&label_id, sizeof(label_id), ctx->tab_bar_id)
        : label_id;
    active = open == NULL || *open;
    if (ctx->tab_bar_id != 0 && ctx->tab_bar_item_count > 0) {
        ctx->cursor.x = ctx->tab_bar_next_x;
        ctx->cursor.y = ctx->tab_bar_row_y;
    }
    if (!imgui_item_register(ctx, id, size)) {
        if (disabled) --ctx->disabled_depth;
        return IMGUI_SCOPE_ERROR;
    }
    if (ctx->tab_bar_id != 0) {
        ++ctx->tab_bar_item_count;
        ctx->tab_bar_next_x = ctx->last_item_rect.x2 + ctx->item_spacing;
        ctx->tab_bar_row_y = ctx->last_item_rect.y1;
    }
    close_clicked = open != NULL && ctx->last_item_hovered &&
                    ctx->mouse_released[IMGUI_MOUSE_BUTTON_LEFT] &&
                    ctx->input.mouse_x >= ctx->last_item_rect.x2 - 18.0f &&
                    ctx->input.mouse_x < ctx->last_item_rect.x2 - 2.0f &&
                    ctx->input.mouse_y >= ctx->last_item_rect.y1 &&
                    ctx->input.mouse_y < ctx->last_item_rect.y2;
    if (close_clicked) {
        *open = IMGUI_FALSE;
        if (ctx->tab_active_id == id) ctx->tab_active_id = 0;
        active = IMGUI_FALSE;
    }
    if (ctx->tab_active_id == 0 && active && !disabled) {
        ctx->tab_active_id = id;
    }
    if (ctx->last_item_clicked && !close_clicked && active && !disabled) {
        ctx->tab_active_id = id;
    }
    active = active && ctx->tab_active_id == id;
    if (disabled) active = IMGUI_FALSE;
    if (active && ctx->tab_bar_id != 0) {
        ctx->cursor.x = ctx->tab_bar_row_start.x;
        ctx->cursor.y = ctx->tab_bar_row_start.y + 24.0f +
                        ctx->item_spacing;
    }
    (void)imgui_add_frame_surface(
        ctx, ctx->last_item_rect,
        disabled ? ctx->style.color_frame :
        (active ? (ctx->last_item_hovered ?
                   ctx->style.color_tab_active_hovered :
                   ctx->style.color_tab_active) :
                  (ctx->last_item_hovered ? ctx->style.color_tab_hovered :
                                            ctx->style.color_tab)));
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        const char *hidden = strstr(label, "##");
        imgui_text_draw_font(ctx, label, hidden != NULL ? hidden :
                             label + strlen(label), imgui_make_vec2(
                                 ctx->last_item_rect.x1 +
                                 ctx->style.frame_padding.x,
                                 imgui_item_label_y(ctx,
                                                     ctx->last_item_rect)));
    }
    if ((flags & IMGUI_TAB_ITEM_UNSAVED_DOCUMENT) != 0) {
        (void)imgui_draw_list_add_circle(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(ctx->last_item_rect.x2 -
                            (open != NULL ? 18.0f : 6.0f),
                            ctx->last_item_rect.y1 + 12.0f),
            2.0f, ctx->style.color_text, 8);
    }
    if (open != NULL) {
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(ctx->last_item_rect.x2 - 14.0f,
                            ctx->last_item_rect.y1 + 8.0f),
            imgui_make_vec2(ctx->last_item_rect.x2 - 6.0f,
                            ctx->last_item_rect.y1 + 16.0f),
            ctx->style.color_text, 1.0f);
        (void)imgui_draw_list_add_line(
            ctx, &ctx->default_draw_list,
            imgui_make_vec2(ctx->last_item_rect.x2 - 6.0f,
                            ctx->last_item_rect.y1 + 8.0f),
            imgui_make_vec2(ctx->last_item_rect.x2 - 14.0f,
                            ctx->last_item_rect.y1 + 16.0f),
            ctx->style.color_text, 1.0f);
    }
    if (disabled) --ctx->disabled_depth;
    {
        imgui_scope scope;
        scope = imgui_internal_scope_begin(
            ctx, IMGUI_INTERNAL_SCOPE_TAB_ITEM, active);
        if (scope == IMGUI_SCOPE_ACTIVE) {
            imgui_push_id_value(ctx, id);
        }
        return scope;
    }
}

void imgui_tab_item_end(imgui_context *ctx)
{
    if (ctx != NULL && ctx->scope_depth > 0 &&
        ctx->scopes[ctx->scope_depth - 1] == IMGUI_INTERNAL_SCOPE_TAB_ITEM &&
        ctx->scope_active[ctx->scope_depth - 1] &&
        ctx->tab_bar_id != 0) {
        ctx->tab_bar_content_cursor = ctx->cursor;
        ctx->tab_bar_content_valid = IMGUI_TRUE;
    }
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_TAB_ITEM);
}

static imgui_result imgui_table_reserve_columns(imgui_context *ctx,
                                                 int columns)
{
    int capacity;
    float *widths;
    const char **labels;
    imgui_table_column_flags *flags;
    imgui_table_sort_spec *sort_specs;
    if (ctx == NULL || columns <= 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (columns <= ctx->table_column_capacity) return IMGUI_RESULT_OK;
    capacity = ctx->table_column_capacity > 0 ? ctx->table_column_capacity :
               IMGUI_INTERNAL_TABLE_COLUMN_CAPACITY;
    while (capacity < columns) {
        if (capacity > INT_MAX / 2) {
            capacity = columns;
            break;
        }
        capacity *= 2;
    }
    if ((size_t)capacity > (size_t)-1 / sizeof(*widths)) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    widths = (float *)imgui_internal_allocate(
        &ctx->allocator, (size_t)capacity * sizeof(*widths));
    if (widths == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    labels = (const char **)imgui_internal_allocate(
        &ctx->allocator, (size_t)capacity * sizeof(*labels));
    if (labels == NULL) {
        imgui_internal_release(&ctx->allocator, widths);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    flags = (imgui_table_column_flags *)imgui_internal_allocate(
        &ctx->allocator, (size_t)capacity * sizeof(*flags));
    if (flags == NULL) {
        imgui_internal_release(&ctx->allocator, widths);
        imgui_internal_release(&ctx->allocator, labels);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    sort_specs = (imgui_table_sort_spec *)imgui_internal_allocate(
        &ctx->allocator, (size_t)capacity * sizeof(*sort_specs));
    if (sort_specs == NULL) {
        imgui_internal_release(&ctx->allocator, widths);
        imgui_internal_release(&ctx->allocator, labels);
        imgui_internal_release(&ctx->allocator, flags);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    if (ctx->table_column_capacity != 0) {
        memcpy(widths, ctx->table_column_widths,
               (size_t)ctx->table_column_capacity * sizeof(*widths));
        memcpy(labels, ctx->table_column_labels,
               (size_t)ctx->table_column_capacity * sizeof(*labels));
        memcpy(flags, ctx->table_column_flags,
               (size_t)ctx->table_column_capacity * sizeof(*flags));
        if (ctx->table_sort_spec_count != 0) {
            memcpy(sort_specs, ctx->table_sort_specs,
                   (size_t)ctx->table_sort_spec_count * sizeof(*sort_specs));
        }
    }
    if (capacity > ctx->table_column_capacity) {
        memset(labels + ctx->table_column_capacity, 0,
               (size_t)(capacity - ctx->table_column_capacity) *
               sizeof(*labels));
        memset(flags + ctx->table_column_capacity, 0,
               (size_t)(capacity - ctx->table_column_capacity) *
               sizeof(*flags));
    }
    imgui_internal_release(&ctx->allocator, ctx->table_column_widths);
    imgui_internal_release(&ctx->allocator, ctx->table_column_labels);
    imgui_internal_release(&ctx->allocator, ctx->table_column_flags);
    imgui_internal_release(&ctx->allocator, ctx->table_sort_specs);
    ctx->table_column_widths = widths;
    ctx->table_column_labels = labels;
    ctx->table_column_flags = flags;
    ctx->table_sort_specs = sort_specs;
    ctx->table_column_capacity = capacity;
    ctx->table_sort_spec_capacity = capacity;
    return IMGUI_RESULT_OK;
}

static int imgui_table_width_state_find(const imgui_context *ctx,
                                        imgui_id id)
{
    int index;
    if (ctx == NULL) return -1;
    for (index = 0; index < ctx->table_width_state_count; ++index) {
        if (ctx->table_width_states[index].id == id) return index;
    }
    return -1;
}

static imgui_result imgui_table_width_state_reserve(imgui_context *ctx,
                                                    int required)
{
    int capacity;
    imgui_internal_table_width_state *states;
    if (ctx == NULL || required < 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (required <= ctx->table_width_state_capacity) {
        return IMGUI_RESULT_OK;
    }
    capacity = ctx->table_width_state_capacity > 0 ?
               ctx->table_width_state_capacity : 8;
    while (capacity < required) {
        if (capacity > INT_MAX / 2) {
            capacity = required;
            break;
        }
        capacity *= 2;
    }
    if ((size_t)capacity > (size_t)-1 / sizeof(*states)) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    states = (imgui_internal_table_width_state *)imgui_internal_allocate(
        &ctx->allocator, (size_t)capacity * sizeof(*states));
    if (states == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    if (ctx->table_width_state_count != 0) {
        memcpy(states, ctx->table_width_states,
               (size_t)ctx->table_width_state_count * sizeof(*states));
    }
    imgui_internal_release(&ctx->allocator, ctx->table_width_states);
    ctx->table_width_states = states;
    ctx->table_width_state_capacity = capacity;
    return IMGUI_RESULT_OK;
}

static void imgui_table_finalize_row_background(imgui_context *ctx,
                                                float row_height)
{
    imgui_u32 vertex_offset;
    if (ctx == NULL || !ctx->table_row_background_valid ||
        row_height <= 0.0f) return;
    vertex_offset = ctx->table_row_background_vertex_offset;
    if (vertex_offset + 3U < ctx->vertex_count) {
        ctx->vertices[vertex_offset + 2U].position.y =
            ctx->table_row_start_y + row_height;
        ctx->vertices[vertex_offset + 3U].position.y =
            ctx->table_row_start_y + row_height;
    }
    ctx->table_row_background_valid = IMGUI_FALSE;
}

imgui_scope imgui_table_begin(imgui_context *ctx,
                              const char *id,
                              int columns,
                              imgui_table_flags flags)
{
    float available_width;
    imgui_id table_id;
    int table_width_index;
    float *persistent_widths;
    if (ctx == NULL || !imgui_internal_require_building(ctx,
                                                        "table begin outside frame")) {
        return IMGUI_SCOPE_ERROR;
    }
    if (columns <= 0) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_ARGUMENT,
                              "table requires at least one column");
        return imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_TABLE,
                                          IMGUI_FALSE);
    }
    if (imgui_table_reserve_columns(ctx, columns) != IMGUI_RESULT_OK) {
        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                              "table column storage allocation failed");
        return IMGUI_SCOPE_ERROR;
    }
    table_id = imgui_get_id_string(ctx, id != NULL ? id : "");
    available_width = ctx->window_origin.x + ctx->window_size.x -
                      ctx->style.window_padding.x - ctx->cursor.x;
    if (available_width < 0.0f) available_width = 0.0f;
    table_width_index = imgui_table_width_state_find(ctx, table_id);
    if (table_width_index < 0) {
        if (imgui_table_width_state_reserve(
                ctx, ctx->table_width_state_count + 1) != IMGUI_RESULT_OK) {
            imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                                  "table width state allocation failed");
            return IMGUI_SCOPE_ERROR;
        }
        table_width_index = ctx->table_width_state_count++;
        ctx->table_width_states[table_width_index].id = table_id;
        ctx->table_width_states[table_width_index].columns = 0;
        ctx->table_width_states[table_width_index].widths = NULL;
        ctx->table_width_states[table_width_index].sort_column = -1;
        ctx->table_width_states[table_width_index].sort_direction = 0;
        ctx->table_width_states[table_width_index].sort_specs = NULL;
        ctx->table_width_states[table_width_index].sort_spec_count = 0;
    }
    persistent_widths = ctx->table_width_states[table_width_index].widths;
    if (ctx->table_width_states[table_width_index].columns != columns) {
        persistent_widths = (float *)imgui_internal_allocate(
            &ctx->allocator, (size_t)columns * sizeof(*persistent_widths));
        if (persistent_widths == NULL) {
            imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                                  "table width allocation failed");
            return IMGUI_SCOPE_ERROR;
        }
        for (table_width_index = 0;
             table_width_index < columns;
             ++table_width_index) {
            persistent_widths[table_width_index] =
                available_width / (float)columns;
        }
        table_width_index = imgui_table_width_state_find(ctx, table_id);
        imgui_internal_release(
            &ctx->allocator,
            ctx->table_width_states[table_width_index].widths);
        ctx->table_width_states[table_width_index].widths = persistent_widths;
        ctx->table_width_states[table_width_index].columns = columns;
    }
    table_width_index = imgui_table_width_state_find(ctx, table_id);
    if (imgui_internal_scope_begin(ctx, IMGUI_INTERNAL_SCOPE_TABLE,
                                   IMGUI_TRUE) == IMGUI_SCOPE_ERROR) {
        return IMGUI_SCOPE_ERROR;
    }
    ctx->table_active = IMGUI_TRUE;
    ctx->table_active_id = table_id;
    imgui_push_id_value(ctx, table_id);
    ctx->table_columns = columns;
    /* BeginTable starts with column 0 active.  TableNextColumn() therefore
       advances to column 1, matching Dear ImGui's canonical
       `item; TableNextColumn(); item` pattern. */
    ctx->table_current_column = 0;
    ctx->table_start_x = ctx->cursor.x;
    ctx->table_start_y = ctx->cursor.y;
    ctx->table_content_start_x = ctx->cursor.x;
    ctx->table_content_start_y = ctx->cursor.y;
    ctx->table_freeze_origin_x = ctx->cursor.x;
    ctx->table_freeze_origin_y = ctx->cursor.y;
    if (ctx->current_window_index >= 0 &&
        ctx->current_window_index < ctx->window_count) {
        ctx->table_freeze_origin_x +=
            ctx->windows[ctx->current_window_index].scroll_x;
        ctx->table_freeze_origin_y +=
            ctx->windows[ctx->current_window_index].scroll_y;
    }
    ctx->table_frozen_height = 0.0f;
    ctx->table_freeze_columns = 0;
    ctx->table_freeze_rows = 0;
    ctx->table_row_start_y = ctx->cursor.y;
    ctx->table_row_height = 0.0f;
    ctx->table_row_background_vertex_offset = 0;
    ctx->table_row_background_valid = IMGUI_FALSE;
    ctx->table_column_width = available_width / (float)columns;
    ctx->table_flags = flags;
    ctx->table_row_index = 0;
    memset(ctx->table_column_labels, 0,
           (size_t)columns * sizeof(*ctx->table_column_labels));
    memset(ctx->table_column_flags, 0,
           (size_t)columns * sizeof(*ctx->table_column_flags));
    ctx->table_sort_column =
        ctx->table_width_states[table_width_index].sort_column;
    ctx->table_sort_direction =
        ctx->table_width_states[table_width_index].sort_direction;
    ctx->table_sort_spec_count = 0;
    if (ctx->table_width_states[table_width_index].sort_spec_count > 0) {
        ctx->table_sort_spec_count =
            ctx->table_width_states[table_width_index].sort_spec_count;
        memcpy(ctx->table_sort_specs,
               ctx->table_width_states[table_width_index].sort_specs,
               (size_t)ctx->table_sort_spec_count *
               sizeof(*ctx->table_sort_specs));
        {
            int sort_index;
            for (sort_index = 0; sort_index < ctx->table_sort_spec_count;
                 ++sort_index) {
                int sort_column =
                    ctx->table_sort_specs[sort_index].column_index;
                ctx->table_sort_specs[sort_index].column_user_id =
                    imgui_hash_bytes(&sort_column, sizeof(sort_column), table_id);
            }
        }
        ctx->table_sort_column = ctx->table_sort_specs[0].column_index;
        ctx->table_sort_direction = ctx->table_sort_specs[0].direction;
    } else if (ctx->table_sort_column >= 0 && ctx->table_sort_direction != 0) {
        ctx->table_sort_specs[0].column_index = ctx->table_sort_column;
        ctx->table_sort_specs[0].sort_order = 0;
        ctx->table_sort_specs[0].direction = ctx->table_sort_direction;
        ctx->table_sort_specs[0].column_user_id = imgui_hash_bytes(
            &ctx->table_sort_column, sizeof(ctx->table_sort_column),
            table_id);
        ctx->table_sort_spec_count = 1;
    }
    ctx->table_sort_specs_view.struct_size = sizeof(ctx->table_sort_specs_view);
    ctx->table_sort_specs_view.specs = ctx->table_sort_specs;
    ctx->table_sort_specs_view.count = ctx->table_sort_spec_count;
    ctx->table_sort_specs_view.dirty = IMGUI_FALSE;
    memcpy(ctx->table_column_widths,
           ctx->table_width_states[table_width_index].widths,
           (size_t)columns * sizeof(*ctx->table_column_widths));
    ctx->table_current_column = 0;
    return IMGUI_SCOPE_ACTIVE;
}

void imgui_table_next_row(imgui_context *ctx)
{
    float completed_row_height;
    if (ctx == NULL || !ctx->table_active ||
        !imgui_internal_require_building(ctx,
                                         "table row outside frame")) return;
    if (ctx->table_current_column >= 0 &&
        ctx->cursor.y - ctx->table_row_start_y > ctx->table_row_height) {
        ctx->table_row_height = ctx->cursor.y - ctx->table_row_start_y;
    }
    if (ctx->table_row_height <= 0.0f) ctx->table_row_height = 22.0f;
    completed_row_height = ctx->table_row_height;
    imgui_table_finalize_row_background(ctx, ctx->table_row_height);
    ctx->table_row_start_y += ctx->table_row_height;
    ctx->table_row_index += 1;
    ctx->table_row_height = 0.0f;
    ctx->table_current_column = -1;
    ctx->cursor.x = ctx->table_start_x;
    if (ctx->table_freeze_rows > 0 &&
        ctx->table_row_index <= ctx->table_freeze_rows) {
        ctx->table_frozen_height += completed_row_height;
    }
    if (ctx->table_row_index < ctx->table_freeze_rows) {
        ctx->table_row_start_y = ctx->table_freeze_origin_y +
                                  ctx->table_frozen_height;
    } else if (ctx->table_row_index == ctx->table_freeze_rows &&
               ctx->table_freeze_rows > 0) {
        ctx->table_row_start_y = ctx->table_content_start_y +
                                 ctx->table_frozen_height;
    }
    ctx->cursor.y = ctx->table_row_start_y;
}

static float imgui_table_column_x(const imgui_context *ctx, int column)
{
    float x;
    int index;
    if (ctx == NULL) return 0.0f;
    x = column < ctx->table_freeze_columns ?
        ctx->table_freeze_origin_x : ctx->table_content_start_x;
    for (index = 0; index < column; ++index) {
        x += ctx->table_column_widths[index];
    }
    return x;
}

void imgui_table_next_column(imgui_context *ctx)
{
    int column;
    float column_x;
    float table_width;
    if (ctx == NULL || !ctx->table_active ||
        !imgui_internal_require_building(ctx,
                                         "table column outside frame")) return;
    if (ctx->table_current_column >= 0 &&
        ctx->cursor.y - ctx->table_row_start_y > ctx->table_row_height) {
        ctx->table_row_height = ctx->cursor.y - ctx->table_row_start_y;
    }
    ctx->table_current_column += 1;
    if (ctx->table_current_column >= ctx->table_columns) {
        imgui_table_next_row(ctx);
        ctx->table_current_column = 0;
    }
    if (ctx->table_current_column == 0 &&
        (ctx->table_flags & IMGUI_TABLE_ROW_BACKGROUND) != 0) {
        imgui_rect row_rect;
        table_width = 0.0f;
        for (column = 0; column < ctx->table_columns; ++column) {
            table_width += ctx->table_column_widths[column];
        }
        row_rect.x1 = ctx->table_start_x;
        row_rect.y1 = ctx->table_row_start_y;
        row_rect.x2 = ctx->table_start_x + table_width;
        row_rect.y2 = row_rect.y1 + 22.0f;
        ctx->table_row_background_vertex_offset = ctx->vertex_count;
        (void)imgui_mesh_add_rect(
            ctx, row_rect,
            (ctx->table_row_index & 1) != 0 ?
                ctx->style.color_table_row_odd :
                ctx->style.color_table_row_even);
        if (ctx->vertex_count >= ctx->table_row_background_vertex_offset + 4U) {
            ctx->table_row_background_valid = IMGUI_TRUE;
        }
    }
    column_x = imgui_table_column_x(ctx, ctx->table_current_column);
    ctx->cursor.x = column_x;
    ctx->cursor.y = ctx->table_row_start_y;
}

void imgui_table_set_column_index(imgui_context *ctx, int column)
{
    int index;
    float column_x;
    float table_width;
    imgui_bool new_row;
    if (ctx == NULL || !ctx->table_active ||
        !imgui_internal_require_building(ctx,
                                         "table column outside frame") ||
        column < 0 || column >= ctx->table_columns) {
        return;
    }
    if (ctx->table_current_column >= 0 &&
        ctx->cursor.y - ctx->table_row_start_y > ctx->table_row_height) {
        ctx->table_row_height = ctx->cursor.y - ctx->table_row_start_y;
    }
    new_row = ctx->table_current_column < 0 ? IMGUI_TRUE : IMGUI_FALSE;
    ctx->table_current_column = column;
    if (new_row && !ctx->table_row_background_valid &&
        (ctx->table_flags & IMGUI_TABLE_ROW_BACKGROUND) != 0) {
        imgui_rect row_rect;
        table_width = 0.0f;
        for (index = 0; index < ctx->table_columns; ++index) {
            table_width += ctx->table_column_widths[index];
        }
        row_rect.x1 = ctx->table_start_x;
        row_rect.y1 = ctx->table_row_start_y;
        row_rect.x2 = ctx->table_start_x + table_width;
        row_rect.y2 = row_rect.y1 + 22.0f;
        ctx->table_row_background_vertex_offset = ctx->vertex_count;
        (void)imgui_mesh_add_rect(
            ctx, row_rect,
            (ctx->table_row_index & 1) != 0 ?
                ctx->style.color_table_row_odd :
                ctx->style.color_table_row_even);
        if (ctx->vertex_count >= ctx->table_row_background_vertex_offset + 4U) {
            ctx->table_row_background_valid = IMGUI_TRUE;
        }
    }
    column_x = imgui_table_column_x(ctx, column);
    ctx->cursor.x = column_x;
    ctx->cursor.y = ctx->table_row_start_y;
}

void imgui_table_set_column_width(imgui_context *ctx, float width)
{
    if (ctx == NULL || !ctx->table_active ||
        !imgui_float_is_finite(width) || width <= 0.0f ||
        ctx->table_current_column < 0 ||
        !imgui_internal_require_building(ctx,
                                         "table column width outside frame")) {
        return;
    }
    ctx->table_column_widths[ctx->table_current_column] = width;
}

imgui_result imgui_table_setup_column(imgui_context *ctx,
                                      int column,
                                      const char *label,
                                      imgui_table_column_flags flags)
{
    (void)flags;
    if (ctx == NULL || !ctx->table_active ||
        !imgui_internal_require_building(
            ctx, "table column setup outside frame") ||
        column < 0 || column >= ctx->table_columns) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    ctx->table_column_labels[column] = label;
    ctx->table_column_flags[column] = flags;
    {
        int sort_index;
        const char *sort_label = label != NULL ? label : "";
        for (sort_index = 0; sort_index < ctx->table_sort_spec_count;
             ++sort_index) {
            if (ctx->table_sort_specs[sort_index].column_index == column) {
                ctx->table_sort_specs[sort_index].column_user_id =
                    imgui_hash_bytes(sort_label, strlen(sort_label),
                                     ctx->table_active_id);
            }
        }
    }
    return IMGUI_RESULT_OK;
}

imgui_result imgui_table_setup_scroll_freeze(imgui_context *ctx,
                                              int columns,
                                              int rows)
{
    if (ctx == NULL || !ctx->table_active ||
        !imgui_internal_require_building(
            ctx, "table scroll freeze outside frame") ||
        columns < 0 || rows < 0 || columns > ctx->table_columns) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx->table_freeze_columns = columns;
    ctx->table_freeze_rows = rows;
    if (rows > 0) {
        ctx->table_row_start_y = ctx->table_freeze_origin_y;
    }
    return IMGUI_RESULT_OK;
}

imgui_bool imgui_table_header(imgui_context *ctx, const char *label)
{
    float width;
    imgui_bool clicked;
    imgui_table_column_flags column_flags;
    int preferred_direction;
    int next_direction;
    int sort_index;
    int sort_order;
    imgui_bool ctrl_down;
    imgui_u32 old_button_color;
    imgui_u32 old_button_hovered_color;
    imgui_u32 old_button_active_color;
    if (ctx == NULL || !ctx->table_active ||
        !imgui_internal_require_building(ctx,
                                         "table header outside frame") ||
        ctx->table_current_column < 0 ||
        ctx->table_current_column >= ctx->table_columns) {
        return IMGUI_FALSE;
    }
    width = ctx->table_column_widths[ctx->table_current_column];
    if (width <= 0.0f) width = ctx->table_column_width;
    old_button_color = ctx->style.color_button;
    old_button_hovered_color = ctx->style.color_button_hovered;
    old_button_active_color = ctx->style.color_button_active;
    ctx->style.color_button = ctx->style.color_header;
    ctx->style.color_button_hovered = ctx->style.color_header_hovered;
    ctx->style.color_button_active = ctx->style.color_header_active;
    clicked = imgui_button_sized(ctx, label != NULL ? label : "",
                                 imgui_make_vec2(width, 22.0f));
    ctx->style.color_button = old_button_color;
    ctx->style.color_button_hovered = old_button_hovered_color;
    ctx->style.color_button_active = old_button_active_color;
    column_flags = ctx->table_column_flags[ctx->table_current_column];
    if (clicked && (column_flags & IMGUI_TABLE_COLUMN_NO_SORT) == 0) {
        ctrl_down = ctx->input.keys_down[IMGUI_KEY_LEFT_CTRL] ||
                    ctx->input.keys_down[IMGUI_KEY_RIGHT_CTRL];
        preferred_direction = (column_flags &
            IMGUI_TABLE_COLUMN_PREFER_SORT_DESCENDING) != 0 ? -1 : 1;
        if ((column_flags & IMGUI_TABLE_COLUMN_PREFER_SORT_ASCENDING) != 0) {
            preferred_direction = 1;
        }
        sort_index = -1;
        for (sort_order = 0; sort_order < ctx->table_sort_spec_count;
             ++sort_order) {
            if (ctx->table_sort_specs[sort_order].column_index ==
                ctx->table_current_column) {
                sort_index = sort_order;
                break;
            }
        }
        if (!ctrl_down) {
            ctx->table_sort_spec_count = 0;
            sort_index = -1;
        }
        if (sort_index >= 0) {
            next_direction = -ctx->table_sort_specs[sort_index].direction;
            if (next_direction > 0 && (column_flags &
                IMGUI_TABLE_COLUMN_NO_SORT_ASCENDING) != 0) {
                next_direction = -1;
            }
            if (next_direction < 0 && (column_flags &
                IMGUI_TABLE_COLUMN_NO_SORT_DESCENDING) != 0) {
                next_direction = 1;
            }
            if ((next_direction > 0 && (column_flags &
                IMGUI_TABLE_COLUMN_NO_SORT_ASCENDING) != 0) ||
                (next_direction < 0 && (column_flags &
                IMGUI_TABLE_COLUMN_NO_SORT_DESCENDING) != 0)) {
                next_direction = preferred_direction;
            }
            if (next_direction == 0) next_direction = preferred_direction;
            ctx->table_sort_specs[sort_index].direction = next_direction;
        } else {
            next_direction = preferred_direction;
            if (next_direction > 0 && (column_flags &
                IMGUI_TABLE_COLUMN_NO_SORT_ASCENDING) != 0) {
                next_direction = -1;
            }
            if (next_direction < 0 && (column_flags &
                IMGUI_TABLE_COLUMN_NO_SORT_DESCENDING) != 0) {
                next_direction = 1;
            }
            if ((next_direction > 0 && (column_flags &
                IMGUI_TABLE_COLUMN_NO_SORT_ASCENDING) != 0) ||
                (next_direction < 0 && (column_flags &
                IMGUI_TABLE_COLUMN_NO_SORT_DESCENDING) != 0)) {
                next_direction = 0;
            } else {
                sort_order = ctx->table_sort_spec_count;
                ctx->table_sort_specs[sort_order].column_index =
                    ctx->table_current_column;
                ctx->table_sort_specs[sort_order].sort_order = sort_order;
                ctx->table_sort_specs[sort_order].direction = next_direction;
                ctx->table_sort_specs[sort_order].column_user_id =
                    imgui_hash_bytes(label != NULL ? label : "",
                                     label != NULL ? strlen(label) : 0,
                                     ctx->table_active_id);
                ctx->table_sort_spec_count += 1;
            }
        }
        if (ctx->table_sort_spec_count > 0) {
            ctx->table_sort_column = ctx->table_sort_specs[0].column_index;
            ctx->table_sort_direction = ctx->table_sort_specs[0].direction;
        } else {
            ctx->table_sort_column = -1;
            ctx->table_sort_direction = 0;
        }
        ctx->table_sort_specs_view.dirty = IMGUI_TRUE;
    }
    ctx->table_sort_specs_view.count = ctx->table_sort_spec_count;
    if (ctx->table_sort_column == ctx->table_current_column &&
        ctx->table_sort_direction != 0) {
        float arrow_x;
        float arrow_y;
        imgui_vec2 arrow_a;
        imgui_vec2 arrow_b;
        imgui_vec2 arrow_c;
        arrow_x = ctx->last_item_rect.x2 - 10.0f;
        arrow_y = (ctx->last_item_rect.y1 + ctx->last_item_rect.y2) * 0.5f;
        if (ctx->table_sort_direction > 0) {
            arrow_a = imgui_make_vec2(arrow_x - 4.0f, arrow_y + 2.0f);
            arrow_b = imgui_make_vec2(arrow_x + 4.0f, arrow_y + 2.0f);
            arrow_c = imgui_make_vec2(arrow_x, arrow_y - 3.0f);
        } else {
            arrow_a = imgui_make_vec2(arrow_x - 4.0f, arrow_y - 2.0f);
            arrow_b = imgui_make_vec2(arrow_x + 4.0f, arrow_y - 2.0f);
            arrow_c = imgui_make_vec2(arrow_x, arrow_y + 3.0f);
        }
        (void)imgui_mesh_add_triangle(ctx, arrow_a, arrow_b, arrow_c,
                                      ctx->style.color_text);
    }
    return clicked;
}

void imgui_table_headers_row(imgui_context *ctx)
{
    int column;
    if (ctx == NULL || !ctx->table_active ||
        !imgui_internal_require_building(ctx,
                                          "table headers outside frame")) {
        return;
    }
    for (column = 0; column < ctx->table_columns; ++column) {
        imgui_table_set_column_index(ctx, column);
        (void)imgui_table_header(ctx, ctx->table_column_labels[column]);
    }
    imgui_table_next_row(ctx);
}

imgui_bool imgui_table_get_sort(const imgui_context *ctx,
                                int *column,
                                int *direction)
{
    if (column != NULL) *column = -1;
    if (direction != NULL) *direction = 0;
    if (ctx == NULL || !ctx->table_active ||
        ctx->table_sort_column < 0 || ctx->table_sort_direction == 0) {
        return IMGUI_FALSE;
    }
    if (column != NULL) *column = ctx->table_sort_column;
    if (direction != NULL) *direction = ctx->table_sort_direction;
    return IMGUI_TRUE;
}

const imgui_table_sort_specs *imgui_table_get_sort_specs(
    const imgui_context *ctx)
{
    if (ctx == NULL || !ctx->table_active) return NULL;
    return &ctx->table_sort_specs_view;
}

int imgui_table_get_column_count(const imgui_context *ctx)
{
    return ctx != NULL && ctx->table_active ? ctx->table_columns : 0;
}

int imgui_table_get_column_index(const imgui_context *ctx)
{
    return ctx != NULL && ctx->table_active ? ctx->table_current_column : -1;
}

int imgui_table_get_row_index(const imgui_context *ctx)
{
    return ctx != NULL && ctx->table_active ? ctx->table_row_index : -1;
}

static int imgui_table_query_column(const imgui_context *ctx, int column)
{
    if (ctx == NULL || !ctx->table_active) return -1;
    if (column < 0) column = ctx->table_current_column;
    return column >= 0 && column < ctx->table_columns ? column : -1;
}

float imgui_table_get_column_width(const imgui_context *ctx, int column)
{
    column = imgui_table_query_column(ctx, column);
    return column >= 0 ? ctx->table_column_widths[column] : 0.0f;
}

const char *imgui_table_get_column_name(const imgui_context *ctx, int column)
{
    column = imgui_table_query_column(ctx, column);
    if (column < 0 || ctx->table_column_labels[column] == NULL) return "";
    return ctx->table_column_labels[column];
}

imgui_table_column_flags imgui_table_get_column_flags(
    const imgui_context *ctx, int column)
{
    column = imgui_table_query_column(ctx, column);
    return column >= 0 ? ctx->table_column_flags[column] : 0;
}

int imgui_table_get_hovered_column(const imgui_context *ctx)
{
    float x;
    float right;
    int column;
    if (ctx == NULL || !ctx->table_active ||
        ctx->current_window_index < 0 ||
        ctx->current_window_index >= ctx->window_count) {
        return -1;
    }
    if (ctx->input.mouse_y < ctx->table_start_y ||
        ctx->input.mouse_y >= ctx->table_row_start_y +
        (ctx->table_row_height > 0.0f ? ctx->table_row_height : 22.0f)) {
        return -1;
    }
    right = ctx->table_start_x;
    for (column = 0; column < ctx->table_columns; ++column) {
        right += ctx->table_column_widths[column];
    }
    if (ctx->input.mouse_x < ctx->table_start_x ||
        ctx->input.mouse_x >= right) {
        return -1;
    }
    x = ctx->table_start_x;
    for (column = 0; column < ctx->table_columns; ++column) {
        right = x + ctx->table_column_widths[column];
        if (ctx->input.mouse_x >= x && ctx->input.mouse_x < right) {
            return column;
        }
        x = right;
    }
    return -1;
}

void imgui_table_end(imgui_context *ctx)
{
    float table_end_x;
    float table_end_y;
    int column;
    int table_width_index;
    imgui_table_sort_spec *saved_sort_specs;
    if (ctx != NULL && ctx->table_active) {
        if (ctx->table_current_column >= 0 &&
            ctx->cursor.y - ctx->table_row_start_y > ctx->table_row_height) {
            ctx->table_row_height = ctx->cursor.y - ctx->table_row_start_y;
        }
        if (ctx->table_row_height <= 0.0f) ctx->table_row_height = 22.0f;
        imgui_table_finalize_row_background(ctx, ctx->table_row_height);
        table_end_x = ctx->table_start_x;
        for (column = 0; column < ctx->table_columns; ++column) {
            table_end_x += ctx->table_column_widths[column];
        }
        table_end_y = ctx->table_row_start_y + ctx->table_row_height;
        if ((ctx->table_flags & IMGUI_TABLE_BORDERS) != 0) {
            (void)imgui_draw_list_add_line(
                ctx, &ctx->default_draw_list,
                imgui_make_vec2(ctx->table_start_x, ctx->table_start_y),
                imgui_make_vec2(table_end_x, ctx->table_start_y),
                ctx->style.color_table_border, 1.0f);
            (void)imgui_draw_list_add_line(
                ctx, &ctx->default_draw_list,
                imgui_make_vec2(table_end_x, ctx->table_start_y),
                imgui_make_vec2(table_end_x, table_end_y),
                ctx->style.color_table_border, 1.0f);
            (void)imgui_draw_list_add_line(
                ctx, &ctx->default_draw_list,
                imgui_make_vec2(table_end_x, table_end_y),
                imgui_make_vec2(ctx->table_start_x, table_end_y),
                ctx->style.color_table_border, 1.0f);
            (void)imgui_draw_list_add_line(
                ctx, &ctx->default_draw_list,
                imgui_make_vec2(ctx->table_start_x, table_end_y),
                imgui_make_vec2(ctx->table_start_x, ctx->table_start_y),
            ctx->style.color_table_border, 1.0f);
            for (column = 1; column < ctx->table_columns; ++column) {
                table_end_x = ctx->table_start_x;
                for (ctx->table_current_column = 0;
                     ctx->table_current_column < column;
                     ++ctx->table_current_column) {
                    table_end_x += ctx->table_column_widths[
                        ctx->table_current_column];
                }
                (void)imgui_draw_list_add_line(
                    ctx, &ctx->default_draw_list,
                    imgui_make_vec2(table_end_x, ctx->table_start_y),
                    imgui_make_vec2(table_end_x, table_end_y),
                    ctx->style.color_table_border, 1.0f);
            }
            ctx->table_current_column = -1;
        }
        ctx->cursor.x = ctx->table_start_x;
        ctx->cursor.y = ctx->table_row_start_y + ctx->table_row_height +
                        ctx->item_spacing;
        table_width_index = imgui_table_width_state_find(
            ctx, ctx->table_active_id);
        if (table_width_index >= 0 &&
            ctx->table_width_states[table_width_index].columns ==
            ctx->table_columns) {
            memcpy(ctx->table_width_states[table_width_index].widths,
                   ctx->table_column_widths,
                   (size_t)ctx->table_columns *
                   sizeof(*ctx->table_column_widths));
            ctx->table_width_states[table_width_index].sort_column =
                ctx->table_sort_column;
            ctx->table_width_states[table_width_index].sort_direction =
                ctx->table_sort_direction;
            saved_sort_specs = NULL;
            if (ctx->table_sort_spec_count > 0) {
                saved_sort_specs = (imgui_table_sort_spec *)
                    imgui_internal_allocate(
                        &ctx->allocator,
                        (size_t)ctx->table_sort_spec_count *
                        sizeof(*saved_sort_specs));
                if (saved_sort_specs != NULL) {
                    memcpy(saved_sort_specs, ctx->table_sort_specs,
                           (size_t)ctx->table_sort_spec_count *
                           sizeof(*saved_sort_specs));
                } else {
                    imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                                          "table sort settings allocation failed");
                }
            }
            imgui_internal_release(&ctx->allocator,
                ctx->table_width_states[table_width_index].sort_specs);
            ctx->table_width_states[table_width_index].sort_specs =
                saved_sort_specs;
            ctx->table_width_states[table_width_index].sort_spec_count =
                saved_sort_specs != NULL ? ctx->table_sort_spec_count : 0;
        }
        ctx->table_active = IMGUI_FALSE;
    }
    imgui_internal_scope_end(ctx, IMGUI_INTERNAL_SCOPE_TABLE);
}

imgui_bool imgui_slider_float(imgui_context *ctx,
                              const char *label,
                              float *value,
                              float minimum,
                              float maximum)
{
    return imgui_slider_float_ex(ctx, label, value, minimum, maximum,
                                 NULL, 0);
}

static float imgui_slider_log_value(float fraction,
                                    float minimum,
                                    float maximum)
{
    float epsilon;
    float zero_fraction;
    float t;
    if (minimum > 0.0f) {
        return (float)exp(log((double)minimum) +
                          (double)fraction *
                          (log((double)maximum) - log((double)minimum)));
    }
    if (maximum < 0.0f) {
        return -(float)exp(log((double)(-minimum)) +
                           (double)fraction *
                           (log((double)(-maximum)) -
                            log((double)(-minimum))));
    }
    if (minimum < 0.0f && maximum > 0.0f) {
        epsilon = (maximum - minimum) * 0.000001f;
        if (epsilon < 0.000001f) epsilon = 0.000001f;
        zero_fraction = (-minimum) / (maximum - minimum);
        if (fraction <= zero_fraction) {
            if (zero_fraction <= 0.0f) return 0.0f;
            t = fraction / zero_fraction;
            if (t >= 1.0f) return 0.0f;
            return -(float)exp(log((double)(-minimum)) +
                               (double)t *
                               (log((double)epsilon) -
                                log((double)(-minimum))));
        }
        if (zero_fraction >= 1.0f) return 0.0f;
        t = (fraction - zero_fraction) / (1.0f - zero_fraction);
        if (t <= 0.0f) return 0.0f;
        return (float)exp(log((double)epsilon) +
                          (double)t *
                          (log((double)maximum) - log((double)epsilon)));
    }
    return minimum;
}

static float imgui_slider_log_fraction(float value,
                                       float minimum,
                                       float maximum)
{
    float epsilon;
    float zero_fraction;
    if (minimum > 0.0f && value > 0.0f) {
        return (float)((log((double)value) - log((double)minimum)) /
                       (log((double)maximum) - log((double)minimum)));
    }
    if (maximum < 0.0f && value < 0.0f) {
        return (float)((log((double)(-value)) - log((double)(-minimum))) /
                       (log((double)(-maximum)) -
                        log((double)(-minimum))));
    }
    if (minimum < 0.0f && maximum > 0.0f) {
        epsilon = (maximum - minimum) * 0.000001f;
        if (epsilon < 0.000001f) epsilon = 0.000001f;
        zero_fraction = (-minimum) / (maximum - minimum);
        if (value < 0.0f && zero_fraction > 0.0f) {
            return zero_fraction *
                   (float)((log((double)(-value)) -
                            log((double)(-minimum))) /
                           (log((double)epsilon) -
                            log((double)(-minimum))));
        }
        if (value > 0.0f && zero_fraction < 1.0f) {
            return zero_fraction + (1.0f - zero_fraction) *
                   (float)((log((double)value) - log((double)epsilon)) /
                           (log((double)maximum) - log((double)epsilon)));
        }
        return zero_fraction;
    }
    return 0.0f;
}

/* Dear ImGui rounds slider values to the displayed decimal precision unless
   the caller opts out.  Restrict this helper to fixed-point formats, whose
   precision has an unambiguous decimal scale. */
static float imgui_slider_round_to_format(float value, const char *format)
{
    const char *percent;
    const char *cursor;
    int precision;
    double scale;
    if (format == NULL) return value;
    percent = strchr(format, '%');
    if (percent == NULL) return value;
    cursor = percent + 1;
    while (*cursor != '\0' && strchr("-+ #0", *cursor) != NULL) ++cursor;
    while (*cursor >= '0' && *cursor <= '9') ++cursor;
    precision = -1;
    if (*cursor == '.') {
        ++cursor;
        precision = 0;
        while (*cursor >= '0' && *cursor <= '9') {
            if (precision < 1000) precision = precision * 10 +
                (int)(*cursor - '0');
            ++cursor;
        }
    }
    if (precision < 0 || (*cursor != 'f' && *cursor != 'F')) return value;
    scale = pow(10.0, (double)precision);
    if (value >= 0.0f) {
        return (float)(floor((double)value * scale + 0.5) / scale);
    }
    return (float)(ceil((double)value * scale - 0.5) / scale);
}

static float imgui_slider_keyboard_step(float range, const char *format)
{
    const char *percent;
    const char *cursor;
    int precision;
    double scale;
    if (range <= 0.0f) return 1.0f;
    percent = format != NULL ? strchr(format, '%') : NULL;
    if (percent == NULL) return range * 0.01f;
    cursor = percent + 1;
    while (*cursor != '\0' && strchr("-+ #0", *cursor) != NULL) ++cursor;
    while (*cursor >= '0' && *cursor <= '9') ++cursor;
    precision = -1;
    if (*cursor == '.') {
        ++cursor;
        precision = 0;
        while (*cursor >= '0' && *cursor <= '9') {
            if (precision < 1000) precision = precision * 10 +
                (int)(*cursor - '0');
            ++cursor;
        }
    }
    if (precision < 0) return range * 0.01f;
    scale = pow(10.0, (double)precision);
    if (scale <= 0.0) return range * 0.01f;
    if (1.0 / scale > (double)range) return range;
    return (float)(1.0 / scale);
}

static imgui_bool imgui_slider_float_internal(imgui_context *ctx,
                                              const char *label,
                                              float *value,
                                              float minimum,
                                              float maximum,
                                              imgui_bool drag,
                                              float speed,
                                              const char *format,
                                              imgui_slider_flags flags)
{
    imgui_vec2 size;
    float fraction;
    float old_value;
    float range;
    imgui_u32 track_color;
    imgui_rect track;
    imgui_rect fill;
    char formatted[64];
    imgui_vec2 formatted_size;
    imgui_bool keyboard_left;
    imgui_bool keyboard_right;
    imgui_bool logarithmic;
    imgui_bool clamp_value;
    float drag_fraction;
    float keyboard_step;
    if (ctx == NULL || value == NULL || minimum > maximum ||
        !imgui_float_is_finite(*value) ||
        !imgui_float_is_finite(minimum) ||
        !imgui_float_is_finite(maximum) ||
        !imgui_float_is_finite(speed)) {
        return IMGUI_FALSE;
    }
    size = imgui_make_vec2(220.0f, 22.0f);
    if (!imgui_item_register(ctx, imgui_get_id_string(ctx, label), size)) {
        return IMGUI_FALSE;
    }
    old_value = *value;
    range = maximum - minimum;
    logarithmic = (flags & IMGUI_SLIDER_LOGARITHMIC) != 0 &&
                  maximum > minimum &&
                  (minimum > 0.0f || maximum < 0.0f ||
                   (minimum < 0.0f && maximum > 0.0f)) ?
                  IMGUI_TRUE : IMGUI_FALSE;
    clamp_value = !drag ||
                  (flags & IMGUI_SLIDER_ALWAYS_CLAMP) != 0 ?
                  IMGUI_TRUE : IMGUI_FALSE;
    keyboard_left = ctx->focused_item_valid &&
        ctx->focused_item_id == ctx->last_item_id &&
        (ctx->input.keys_pressed[IMGUI_KEY_LEFT_ARROW] ||
         ctx->input.keys_repeated[IMGUI_KEY_LEFT_ARROW] ||
         ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_LEFT] ||
         ctx->input.keys_repeated[IMGUI_KEY_GAMEPAD_DPAD_LEFT] ||
         ctx->input.keys_pressed[IMGUI_KEY_DOWN_ARROW] ||
         ctx->input.keys_repeated[IMGUI_KEY_DOWN_ARROW] ||
         ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_DOWN] ||
         ctx->input.keys_repeated[IMGUI_KEY_GAMEPAD_DPAD_DOWN]);
    keyboard_right = ctx->focused_item_valid &&
        ctx->focused_item_id == ctx->last_item_id &&
        (ctx->input.keys_pressed[IMGUI_KEY_RIGHT_ARROW] ||
         ctx->input.keys_repeated[IMGUI_KEY_RIGHT_ARROW] ||
         ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_RIGHT] ||
         ctx->input.keys_repeated[IMGUI_KEY_GAMEPAD_DPAD_RIGHT] ||
         ctx->input.keys_pressed[IMGUI_KEY_UP_ARROW] ||
         ctx->input.keys_repeated[IMGUI_KEY_UP_ARROW] ||
         ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_UP] ||
         ctx->input.keys_repeated[IMGUI_KEY_GAMEPAD_DPAD_UP]);
    keyboard_step = imgui_slider_keyboard_step(range, format);
    if (range <= 0.0f) {
        *value = minimum;
    } else if (keyboard_left || keyboard_right) {
        if (keyboard_left) *value -= keyboard_step;
        if (keyboard_right) *value += keyboard_step;
    } else if (drag && ctx->last_item_active) {
        if (!ctx->drag_value_active || ctx->drag_value_id != ctx->last_item_id ||
            ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT]) {
            ctx->drag_value_active = IMGUI_TRUE;
            ctx->drag_value_id = ctx->last_item_id;
            ctx->drag_value_start = old_value;
            ctx->drag_value_mouse_start = ctx->input.mouse_x;
        }
        if (logarithmic) {
            drag_fraction = imgui_slider_log_fraction(
                ctx->drag_value_start, minimum, maximum);
            drag_fraction += (ctx->input.mouse_x -
                              ctx->drag_value_mouse_start) * speed /
                             range;
            *value = imgui_slider_log_value(drag_fraction,
                                            minimum, maximum);
        } else {
            *value = ctx->drag_value_start +
                     (ctx->input.mouse_x - ctx->drag_value_mouse_start) *
                     speed;
        }
    } else if ((ctx->last_item_hovered &&
                ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT]) ||
               ctx->last_item_active) {
        fraction = (ctx->input.mouse_x - ctx->last_item_rect.x1) /
                   (ctx->last_item_rect.x2 - ctx->last_item_rect.x1);
        if (fraction < 0.0f) fraction = 0.0f;
        if (fraction > 1.0f) fraction = 1.0f;
        if (drag && ctx->last_item_active) {
            fraction = (ctx->input.mouse_x - ctx->last_item_rect.x1) /
                       (ctx->last_item_rect.x2 - ctx->last_item_rect.x1);
        }
        if (logarithmic) {
            *value = imgui_slider_log_value(fraction, minimum, maximum);
        } else {
            *value = minimum + range * fraction;
        }
    }
    if (!drag || !ctx->last_item_active) {
        ctx->drag_value_active = IMGUI_FALSE;
        ctx->drag_value_id = 0;
    }
    if (clamp_value) {
        if (*value < minimum) *value = minimum;
        if (*value > maximum) *value = maximum;
    }
    if ((flags & IMGUI_SLIDER_NO_ROUND_TO_FORMAT) == 0 &&
        *value != old_value) {
        *value = imgui_slider_round_to_format(*value, format);
        if (clamp_value) {
            if (*value < minimum) *value = minimum;
            if (*value > maximum) *value = maximum;
        }
    }
    if (logarithmic) {
        fraction = imgui_slider_log_fraction(*value, minimum, maximum);
    } else {
        fraction = range > 0.0f ? (*value - minimum) / range : 0.0f;
    }
    if (fraction < 0.0f) fraction = 0.0f;
    if (fraction > 1.0f) fraction = 1.0f;
    track = ctx->last_item_rect;
    track.y1 += 7.0f;
    track.y2 -= 7.0f;
    track_color = ctx->last_item_hovered ? ctx->style.color_frame_hovered :
                                           ctx->style.color_frame;
    (void)imgui_add_frame_surface(ctx, track, track_color);
    fill = track;
    fill.x2 = fill.x1 + (fill.x2 - fill.x1) * fraction;
    if (fill.x2 > fill.x1) {
        (void)imgui_add_frame_surface(ctx, fill,
                                      ctx->style.color_frame_active);
    }
    if (ctx->font != NULL && ctx->font_texture != NULL && label != NULL) {
        const char *hidden = strstr(label, "##");
        imgui_text_draw_font(ctx, label, hidden != NULL ? hidden :
                             label + strlen(label), imgui_make_vec2(
                                 ctx->last_item_rect.x1 +
                                 ctx->style.frame_padding.x,
                                 imgui_item_label_y(ctx,
                                                    ctx->last_item_rect)));
    }
    if (ctx->font != NULL && ctx->font_texture != NULL && format != NULL) {
        (void)snprintf(formatted, sizeof(formatted), format, (double)*value);
        formatted[sizeof(formatted) - 1U] = '\0';
        formatted_size = imgui_font_measure_text(ctx->font, formatted, NULL,
                                                 0.0f);
        imgui_text_draw_font(ctx, formatted,
                             formatted + strlen(formatted),
                             imgui_make_vec2(ctx->last_item_rect.x2 -
                                             ctx->style.frame_padding.x -
                                             formatted_size.x,
                                             imgui_item_label_y(ctx,
                                                                ctx->last_item_rect)));
    }
    if (*value != old_value) ctx->last_item_edited = IMGUI_TRUE;
    return *value != old_value ? IMGUI_TRUE : IMGUI_FALSE;
}

imgui_bool imgui_slider_float_ex(imgui_context *ctx,
                                 const char *label,
                                 float *value,
                                 float minimum,
                                 float maximum,
                                 const char *format,
                                 imgui_slider_flags flags)
{
    if (format == NULL) format = "%.3f";
    return imgui_slider_float_internal(ctx, label, value, minimum, maximum,
                                       IMGUI_FALSE, 0.0f, format, flags);
}

imgui_bool imgui_vslider_float(imgui_context *ctx, const char *label,
                               imgui_vec2 size, float *value,
                               float minimum, float maximum)
{
    return imgui_vslider_float_ex(ctx, label, size, value, minimum, maximum,
                                  NULL, IMGUI_SLIDER_NONE);
}

imgui_bool imgui_vslider_float_ex(imgui_context *ctx, const char *label,
                                  imgui_vec2 size, float *value,
                                  float minimum, float maximum,
                                  const char *format,
                                  imgui_slider_flags flags)
{
    imgui_rect track;
    imgui_rect fill;
    imgui_vec2 label_size;
    imgui_vec2 label_position;
    float old_value;
    float range;
    float fraction;
    float keyboard_step;
    imgui_bool decrease;
    imgui_bool increase;
    char formatted[64];
    if (ctx == NULL || value == NULL || minimum > maximum ||
        !imgui_vec2_is_finite(size) || size.x <= 0.0f || size.y <= 0.0f ||
        !imgui_float_is_finite(*value) ||
        !imgui_float_is_finite(minimum) ||
        !imgui_float_is_finite(maximum)) {
        return IMGUI_FALSE;
    }
    if (format == NULL) format = "%.3f";
    if (!imgui_item_register(ctx, imgui_get_id_string(ctx,
                            label != NULL ? label : ""), size)) {
        return IMGUI_FALSE;
    }
    old_value = *value;
    range = maximum - minimum;
    decrease = ctx->focused_item_valid &&
        ctx->focused_item_id == ctx->last_item_id &&
        (ctx->input.keys_pressed[IMGUI_KEY_DOWN_ARROW] ||
         ctx->input.keys_repeated[IMGUI_KEY_DOWN_ARROW] ||
         ctx->input.keys_pressed[IMGUI_KEY_LEFT_ARROW] ||
         ctx->input.keys_repeated[IMGUI_KEY_LEFT_ARROW] ||
         ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_DOWN] ||
         ctx->input.keys_repeated[IMGUI_KEY_GAMEPAD_DPAD_DOWN]);
    increase = ctx->focused_item_valid &&
        ctx->focused_item_id == ctx->last_item_id &&
        (ctx->input.keys_pressed[IMGUI_KEY_UP_ARROW] ||
         ctx->input.keys_repeated[IMGUI_KEY_UP_ARROW] ||
         ctx->input.keys_pressed[IMGUI_KEY_RIGHT_ARROW] ||
         ctx->input.keys_repeated[IMGUI_KEY_RIGHT_ARROW] ||
         ctx->input.keys_pressed[IMGUI_KEY_GAMEPAD_DPAD_UP] ||
         ctx->input.keys_repeated[IMGUI_KEY_GAMEPAD_DPAD_UP]);
    keyboard_step = imgui_slider_keyboard_step(range, format);
    if (range <= 0.0f) {
        *value = minimum;
    } else if (decrease || increase) {
        if (decrease) *value -= keyboard_step;
        if (increase) *value += keyboard_step;
    } else if (ctx->last_item_hovered &&
               (ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT] ||
                ctx->last_item_active ||
                ctx->input.mouse_down[IMGUI_MOUSE_BUTTON_LEFT])) {
        fraction = 1.0f - (ctx->input.mouse_y - ctx->last_item_rect.y1) /
                   (ctx->last_item_rect.y2 - ctx->last_item_rect.y1);
        if (fraction < 0.0f) fraction = 0.0f;
        if (fraction > 1.0f) fraction = 1.0f;
        *value = minimum + range * fraction;
    }
    if ((flags & IMGUI_SLIDER_ALWAYS_CLAMP) != 0 ||
        (ctx->last_item_active && ctx->input.mouse_down[
            IMGUI_MOUSE_BUTTON_LEFT])) {
        if (*value < minimum) *value = minimum;
        if (*value > maximum) *value = maximum;
    }
    if ((flags & IMGUI_SLIDER_NO_ROUND_TO_FORMAT) == 0 &&
        *value != old_value) {
        *value = imgui_slider_round_to_format(*value, format);
        if (*value < minimum) *value = minimum;
        if (*value > maximum) *value = maximum;
    }
    fraction = range > 0.0f ? (*value - minimum) / range : 0.0f;
    if (fraction < 0.0f) fraction = 0.0f;
    if (fraction > 1.0f) fraction = 1.0f;
    track.x1 = ctx->last_item_rect.x1 + size.x * 0.5f - 5.0f;
    track.x2 = track.x1 + 10.0f;
    track.y1 = ctx->last_item_rect.y1 + 5.0f;
    track.y2 = ctx->last_item_rect.y2 - 5.0f;
    (void)imgui_add_frame_surface(ctx, track,
        ctx->last_item_hovered ? ctx->style.color_frame_hovered :
                                 ctx->style.color_frame);
    fill = track;
    fill.y1 = track.y2 - (track.y2 - track.y1) * fraction;
    (void)imgui_add_frame_surface(ctx, fill, ctx->style.color_frame_active);
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        (void)snprintf(formatted, sizeof(formatted), format, (double)*value);
        formatted[sizeof(formatted) - 1U] = '\0';
        label_size = imgui_font_measure_text(ctx->font, formatted, NULL, 0.0f);
        label_position = imgui_make_vec2(
            ctx->last_item_rect.x1 + (size.x - label_size.x) * 0.5f,
            ctx->last_item_rect.y1 + ctx->style.frame_padding.y);
        imgui_text_draw_font(ctx, formatted, formatted + strlen(formatted),
                             label_position);
        if (label != NULL) {
            label_size = imgui_font_measure_text(ctx->font, label, NULL, 0.0f);
            imgui_text_draw_font(ctx, label,
                                 label + strlen(label),
                                 imgui_make_vec2(
                                     ctx->last_item_rect.x1 +
                                     (size.x - label_size.x) * 0.5f,
                                     ctx->last_item_rect.y2 -
                                     label_size.y - ctx->style.frame_padding.y));
        }
    }
    if (*value != old_value) ctx->last_item_edited = IMGUI_TRUE;
    return *value != old_value ? IMGUI_TRUE : IMGUI_FALSE;
}

imgui_bool imgui_slider_integer(imgui_context *ctx,
                                const char *label,
                                int *value,
                                int minimum,
                                int maximum)
{
    return imgui_slider_integer_ex(ctx, label, value, minimum, maximum,
                                   NULL, IMGUI_SLIDER_NONE);
}

imgui_bool imgui_slider_integer_ex(imgui_context *ctx,
                                   const char *label,
                                   int *value,
                                   int minimum,
                                   int maximum,
                                   const char *format,
                                   imgui_slider_flags flags)
{
    float converted;
    int old_integer;
    if (value == NULL || minimum > maximum) return IMGUI_FALSE;
    old_integer = *value;
    converted = (float)old_integer;
    if (format == NULL) format = "%.0f";
    if (!imgui_slider_float_internal(ctx, label, &converted,
                                     (float)minimum, (float)maximum,
                                     IMGUI_FALSE, 0.0f, format, flags)) {
        return IMGUI_FALSE;
    }
    if (converted < (float)minimum) converted = (float)minimum;
    if (converted > (float)maximum) converted = (float)maximum;
    /* Round to nearest with symmetric behavior for negative ranges.  Adding
       0.5 unconditionally biases negative values toward zero (for example,
       -1.6 became -1), unlike Dear ImGui's integer slider semantics. */
    if (converted >= 0.0f) *value = (int)floor((double)converted + 0.5);
    else *value = (int)ceil((double)converted - 0.5);
    return *value != old_integer ? IMGUI_TRUE : IMGUI_FALSE;
}

imgui_bool imgui_vslider_integer(imgui_context *ctx, const char *label,
                                 imgui_vec2 size, int *value,
                                 int minimum, int maximum)
{
    return imgui_vslider_integer_ex(ctx, label, size, value, minimum,
                                    maximum, NULL, IMGUI_SLIDER_NONE);
}

imgui_bool imgui_vslider_integer_ex(imgui_context *ctx, const char *label,
                                    imgui_vec2 size, int *value,
                                    int minimum, int maximum,
                                    const char *format,
                                    imgui_slider_flags flags)
{
    float converted;
    imgui_bool changed;
    int old_value;
    if (ctx == NULL || value == NULL || minimum > maximum) {
        return IMGUI_FALSE;
    }
    old_value = *value;
    converted = (float)*value;
    if (format == NULL) format = "%.0f";
    changed = imgui_vslider_float_ex(ctx, label, size, &converted,
                                     (float)minimum, (float)maximum,
                                     format, flags);
    if (!changed) return IMGUI_FALSE;
    if (converted >= 0.0f) *value = (int)floor((double)converted + 0.5);
    else *value = (int)ceil((double)converted - 0.5);
    if (*value < minimum) *value = minimum;
    if (*value > maximum) *value = maximum;
    return *value != old_value ? IMGUI_TRUE : IMGUI_FALSE;
}

imgui_bool imgui_drag_float(imgui_context *ctx,
                            const char *label,
                            float *value,
                            float speed)
{
    return imgui_drag_float_ex(ctx, label, value, speed, 0.0f, 1.0f,
                               NULL, 0);
}

imgui_bool imgui_drag_float_ex(imgui_context *ctx,
                               const char *label,
                               float *value,
                               float speed,
                               float minimum,
                               float maximum,
                               const char *format,
                               imgui_slider_flags flags)
{
    if (format == NULL) format = "%.3f";
    return imgui_slider_float_internal(ctx, label, value, minimum, maximum,
                                       IMGUI_TRUE, speed, format, flags);
}

imgui_bool imgui_drag_integer(imgui_context *ctx,
                              const char *label,
                              int *value,
                              float speed)
{
    return imgui_drag_integer_ex(ctx, label, value, speed, 0, 0, NULL, 0);
}

imgui_bool imgui_drag_integer_ex(imgui_context *ctx,
                                 const char *label,
                                 int *value,
                                 float speed,
                                 int minimum,
                                 int maximum,
                                 const char *format,
                                 imgui_slider_flags flags)
{
    float converted;
    float converted_minimum;
    float converted_maximum;
    int old_integer;
    if (ctx == NULL || value == NULL || !imgui_float_is_finite(speed)) {
        return IMGUI_FALSE;
    }
    /* A zero range is the convenient unbounded form, matching
       imgui_drag_float().  For a bounded drag, preserve the caller's exact
       integer endpoints before converting through the shared drag geometry. */
    if (minimum > maximum) return IMGUI_FALSE;
    if (minimum == 0 && maximum == 0) {
        converted_minimum = -2147483648.0f;
        converted_maximum = 2147483647.0f;
    } else {
        converted_minimum = (float)minimum;
        converted_maximum = (float)maximum;
    }
    if (!imgui_float_is_finite(converted_minimum) ||
        !imgui_float_is_finite(converted_maximum)) return IMGUI_FALSE;
    old_integer = *value;
    converted = (float)old_integer;
    if (format == NULL) format = "%.0f";
    if (!imgui_slider_float_internal(ctx, label, &converted,
                                     converted_minimum,
                                     converted_maximum,
                                     IMGUI_TRUE, speed, format, flags)) {
        return IMGUI_FALSE;
    }
    if (converted >= 0.0f) {
        *value = (int)floor((double)converted + 0.5);
    } else {
        *value = (int)ceil((double)converted - 0.5);
    }
    if (!(minimum == 0 && maximum == 0)) {
        if (*value < minimum) *value = minimum;
        if (*value > maximum) *value = maximum;
    }
    if (*value != old_integer) {
        ctx->last_item_edited = IMGUI_TRUE;
        return IMGUI_TRUE;
    }
    return IMGUI_FALSE;
}

static imgui_bool imgui_vector_scalar_prepare(imgui_context *ctx,
                                              const char *label,
                                              float *values, int components,
                                              float minimum, float maximum,
                                              float *width)
{
    imgui_vec2 available;
    int index;
    if (ctx == NULL || values == NULL || components < 2 || components > 4 ||
        minimum > maximum || !imgui_float_is_finite(minimum) ||
        !imgui_float_is_finite(maximum)) return IMGUI_FALSE;
    for (index = 0; index < components; ++index) {
        if (!imgui_float_is_finite(values[index])) return IMGUI_FALSE;
    }
    if (label != NULL && *label != '\0') {
        imgui_text_unformatted(ctx, label, NULL);
        imgui_same_line(ctx);
    }
    available = imgui_get_content_region_available(ctx);
    *width = (available.x - (float)(components - 1) * ctx->item_spacing) /
             (float)components;
    if (*width < 1.0f) *width = 1.0f;
    return IMGUI_TRUE;
}

imgui_bool imgui_slider_float_n(imgui_context *ctx, const char *label,
                                float *values, int components,
                                float minimum, float maximum,
                                const char *format, imgui_slider_flags flags)
{
    int index;
    float width;
    imgui_bool changed;
    char component_label[64];
    if (!imgui_vector_scalar_prepare(ctx, label, values, components,
                                      minimum, maximum, &width)) {
        return IMGUI_FALSE;
    }
    changed = IMGUI_FALSE;
    for (index = 0; index < components; ++index) {
        if (index > 0) imgui_same_line(ctx);
        imgui_set_next_item_width(ctx, width);
        (void)snprintf(component_label, sizeof(component_label),
                       "##%s:%d", label != NULL ? label : "vector", index);
        if (imgui_slider_float_ex(ctx, component_label, &values[index],
                                  minimum, maximum, format, flags)) {
            changed = IMGUI_TRUE;
        }
    }
    imgui_new_line(ctx);
    return changed;
}

imgui_bool imgui_drag_float_n(imgui_context *ctx, const char *label,
                              float *values, int components, float speed,
                              float minimum, float maximum,
                              const char *format, imgui_slider_flags flags)
{
    int index;
    float width;
    imgui_bool changed;
    char component_label[64];
    if (!imgui_vector_scalar_prepare(ctx, label, values, components,
                                      minimum, maximum, &width) ||
        !imgui_float_is_finite(speed)) return IMGUI_FALSE;
    changed = IMGUI_FALSE;
    for (index = 0; index < components; ++index) {
        if (index > 0) imgui_same_line(ctx);
        imgui_set_next_item_width(ctx, width);
        (void)snprintf(component_label, sizeof(component_label),
                       "##%s:%d", label != NULL ? label : "vector", index);
        if (imgui_drag_float_ex(ctx, component_label, &values[index], speed,
                                minimum, maximum, format, flags)) {
            changed = IMGUI_TRUE;
        }
    }
    imgui_new_line(ctx);
    return changed;
}

imgui_bool imgui_input_float_n(imgui_context *ctx, const char *label,
                               float *values, int components,
                               const char *format)
{
    int index;
    float width;
    imgui_bool changed;
    char component_label[64];
    if (!imgui_vector_scalar_prepare(ctx, label, values, components,
                                      -FLT_MAX, FLT_MAX, &width)) {
        return IMGUI_FALSE;
    }
    changed = IMGUI_FALSE;
    for (index = 0; index < components; ++index) {
        if (index > 0) imgui_same_line(ctx);
        imgui_set_next_item_width(ctx, width);
        (void)snprintf(component_label, sizeof(component_label),
                       "##%s:%d", label != NULL ? label : "vector", index);
        if (imgui_input_float_ex(ctx, component_label, &values[index],
                                 format)) changed = IMGUI_TRUE;
    }
    imgui_new_line(ctx);
    return changed;
}

static imgui_bool imgui_vector_integer_prepare(imgui_context *ctx,
                                               const char *label,
                                               int *values, int components,
                                               int minimum, int maximum,
                                               float *width)
{
    imgui_vec2 available;
    int index;
    if (ctx == NULL || values == NULL || components < 2 || components > 4 ||
        minimum > maximum) return IMGUI_FALSE;
    (void)minimum;
    (void)maximum;
    for (index = 0; index < components; ++index) {
        (void)values[index];
    }
    if (label != NULL && *label != '\0') {
        imgui_text_unformatted(ctx, label, NULL);
        imgui_same_line(ctx);
    }
    available = imgui_get_content_region_available(ctx);
    *width = (available.x - (float)(components - 1) * ctx->item_spacing) /
             (float)components;
    if (*width < 1.0f) *width = 1.0f;
    return IMGUI_TRUE;
}

imgui_bool imgui_slider_integer_n(imgui_context *ctx, const char *label,
                                  int *values, int components,
                                  int minimum, int maximum,
                                  const char *format,
                                  imgui_slider_flags flags)
{
    int index;
    float width;
    imgui_bool changed;
    char component_label[64];
    if (!imgui_vector_integer_prepare(ctx, label, values, components,
                                       minimum, maximum, &width)) {
        return IMGUI_FALSE;
    }
    changed = IMGUI_FALSE;
    for (index = 0; index < components; ++index) {
        if (index > 0) imgui_same_line(ctx);
        imgui_set_next_item_width(ctx, width);
        (void)snprintf(component_label, sizeof(component_label),
                       "##%s:%d", label != NULL ? label : "vector", index);
        if (imgui_slider_integer_ex(ctx, component_label, &values[index],
                                    minimum, maximum, format, flags)) {
            changed = IMGUI_TRUE;
        }
    }
    imgui_new_line(ctx);
    return changed;
}

imgui_bool imgui_drag_integer_n(imgui_context *ctx, const char *label,
                                int *values, int components, float speed,
                                int minimum, int maximum,
                                const char *format,
                                imgui_slider_flags flags)
{
    int index;
    float width;
    imgui_bool changed;
    char component_label[64];
    if (!imgui_vector_integer_prepare(ctx, label, values, components,
                                       minimum, maximum, &width) ||
        !imgui_float_is_finite(speed)) return IMGUI_FALSE;
    changed = IMGUI_FALSE;
    for (index = 0; index < components; ++index) {
        if (index > 0) imgui_same_line(ctx);
        imgui_set_next_item_width(ctx, width);
        (void)snprintf(component_label, sizeof(component_label),
                       "##%s:%d", label != NULL ? label : "vector", index);
        if (imgui_drag_integer_ex(ctx, component_label, &values[index], speed,
                                  minimum, maximum, format, flags)) {
            changed = IMGUI_TRUE;
        }
    }
    imgui_new_line(ctx);
    return changed;
}

imgui_bool imgui_input_integer_n(imgui_context *ctx, const char *label,
                                 int *values, int components,
                                 const char *format)
{
    int index;
    float width;
    imgui_bool changed;
    char component_label[64];
    if (!imgui_vector_integer_prepare(ctx, label, values, components,
                                       INT_MIN, INT_MAX, &width)) {
        return IMGUI_FALSE;
    }
    changed = IMGUI_FALSE;
    for (index = 0; index < components; ++index) {
        if (index > 0) imgui_same_line(ctx);
        imgui_set_next_item_width(ctx, width);
        (void)snprintf(component_label, sizeof(component_label),
                       "##%s:%d", label != NULL ? label : "vector", index);
        if (imgui_input_integer_ex(ctx, component_label, &values[index],
                                   format)) changed = IMGUI_TRUE;
    }
    imgui_new_line(ctx);
    return changed;
}

static imgui_result imgui_numeric_buffer_reserve(imgui_context *ctx,
                                                 size_t required)
{
    size_t capacity;
    char *data;
    if (ctx == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (required <= ctx->numeric_buffer_capacity) return IMGUI_RESULT_OK;
    capacity = ctx->numeric_buffer_capacity;
    if (capacity < 128) capacity = 128;
    while (capacity < required) {
        if (capacity > (size_t)-1 / 2) {
            capacity = required;
            break;
        }
        capacity *= 2;
    }
    data = (char *)imgui_internal_allocate(&ctx->allocator, capacity);
    if (data == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    if (ctx->numeric_buffer != NULL && ctx->numeric_buffer_capacity != 0) {
        memcpy(data, ctx->numeric_buffer, ctx->numeric_buffer_capacity);
    }
    imgui_internal_release(&ctx->allocator, ctx->numeric_buffer);
    ctx->numeric_buffer = data;
    ctx->numeric_buffer_capacity = capacity;
    return IMGUI_RESULT_OK;
}

static imgui_result imgui_numeric_buffer_reserve_callback(
    void *user_data, size_t required_capacity, char **data, size_t *capacity)
{
    imgui_context *ctx;
    if (user_data == NULL || data == NULL || capacity == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    ctx = (imgui_context *)user_data;
    if (imgui_numeric_buffer_reserve(ctx, required_capacity) !=
            IMGUI_RESULT_OK) return IMGUI_RESULT_OUT_OF_MEMORY;
    *data = ctx->numeric_buffer;
    *capacity = ctx->numeric_buffer_capacity;
    return IMGUI_RESULT_OK;
}

static imgui_bool imgui_numeric_format_is_hex(const char *format)
{
    const char *percent;
    const char *cursor;
    if (format == NULL) return IMGUI_FALSE;
    percent = strchr(format, '%');
    if (percent == NULL) return IMGUI_FALSE;
    cursor = percent + 1;
    while (*cursor != '\0' && strchr("-+ #0", *cursor) != NULL) ++cursor;
    while (*cursor >= '0' && *cursor <= '9') ++cursor;
    if (*cursor == '.') {
        ++cursor;
        while (*cursor >= '0' && *cursor <= '9') ++cursor;
    }
    return (*cursor == 'x' || *cursor == 'X') ? IMGUI_TRUE : IMGUI_FALSE;
}

imgui_bool imgui_input_integer_ex(imgui_context *ctx,
                                  const char *label,
                                  int *value,
                                  const char *format)
{
    imgui_input_text_desc desc;
    imgui_id id;
    imgui_bool edited;
    char *end;
    imgui_text_buffer buffer;
    long parsed;
    imgui_bool hexadecimal;
    if (ctx == NULL || value == NULL) return IMGUI_FALSE;
    if (format == NULL) format = "%d";
    hexadecimal = imgui_numeric_format_is_hex(format);
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    /* Keep the displayed scalar synchronized with caller-owned storage when
       the widget is not being edited.  Dear ImGui's InputScalar re-seeds its
       temporary text from the value in this case; leaving the old buffer in
       place makes programmatic updates appear to be ignored until focus is
       lost. */
    if ((!ctx->numeric_buffer_valid || ctx->numeric_buffer_id != id) ||
        (ctx->active_text_id != id && ctx->numeric_buffer != NULL)) {
        char formatted[64];
        (void)snprintf(formatted, sizeof(formatted), format, *value);
        formatted[sizeof(formatted) - 1U] = '\0';
        if (imgui_numeric_buffer_reserve(ctx, strlen(formatted) + 1) !=
                IMGUI_RESULT_OK) return IMGUI_FALSE;
        memcpy(ctx->numeric_buffer, formatted, strlen(formatted) + 1);
        ctx->numeric_buffer_id = id;
        ctx->numeric_buffer_valid = IMGUI_TRUE;
    }
    imgui_input_text_desc_init(&desc, label);
    desc.use_explicit_id = IMGUI_TRUE;
    desc.id = id;
    desc.flags = hexadecimal ? IMGUI_INPUT_TEXT_HEXADECIMAL :
                               IMGUI_INPUT_TEXT_DECIMAL;
    imgui_text_buffer_init(&buffer);
    buffer.data = ctx->numeric_buffer;
    buffer.length = strlen(ctx->numeric_buffer);
    buffer.capacity = ctx->numeric_buffer_capacity;
    buffer.reserve = imgui_numeric_buffer_reserve_callback;
    buffer.user_data = ctx;
    edited = imgui_input_text_buffer_ex(ctx, &desc, &buffer);
    ctx->numeric_buffer = buffer.data;
    ctx->numeric_buffer_capacity = buffer.capacity;
    if (!edited) return IMGUI_FALSE;
    parsed = strtol(ctx->numeric_buffer, &end, hexadecimal ? 16 : 10);
    if (end == ctx->numeric_buffer || *end != '\0' ||
        parsed > (long)INT_MAX || parsed < (long)INT_MIN) {
        return IMGUI_FALSE;
    }
    if (*value != (int)parsed) {
        *value = (int)parsed;
        return IMGUI_TRUE;
    }
    return IMGUI_FALSE;
}

imgui_bool imgui_input_integer(imgui_context *ctx,
                               const char *label,
                               int *value)
{
    return imgui_input_integer_ex(ctx, label, value, "%d");
}

imgui_bool imgui_input_float_ex(imgui_context *ctx,
                                const char *label,
                                float *value,
                                const char *format)
{
    imgui_input_text_desc desc;
    imgui_id id;
    imgui_bool edited;
    char *end;
    imgui_text_buffer buffer;
    double parsed;
    if (ctx == NULL || value == NULL || !imgui_float_is_finite(*value)) {
        return IMGUI_FALSE;
    }
    if (format == NULL) format = "%.3f";
    id = imgui_get_id_string(ctx, label != NULL ? label : "");
    /* As with integer inputs, refresh an unfocused temporary buffer from the
       current value so external writes are visible on the next frame while
       preserving text the user is actively editing. */
    if ((!ctx->numeric_buffer_valid || ctx->numeric_buffer_id != id) ||
        (ctx->active_text_id != id && ctx->numeric_buffer != NULL)) {
        char formatted[64];
        (void)snprintf(formatted, sizeof(formatted), format, (double)*value);
        formatted[sizeof(formatted) - 1U] = '\0';
        if (imgui_numeric_buffer_reserve(ctx, strlen(formatted) + 1) !=
                IMGUI_RESULT_OK) return IMGUI_FALSE;
        memcpy(ctx->numeric_buffer, formatted, strlen(formatted) + 1);
        ctx->numeric_buffer_id = id;
        ctx->numeric_buffer_valid = IMGUI_TRUE;
    }
    imgui_input_text_desc_init(&desc, label);
    desc.use_explicit_id = IMGUI_TRUE;
    desc.id = id;
    desc.flags = IMGUI_INPUT_TEXT_DECIMAL | IMGUI_INPUT_TEXT_SCIENTIFIC;
    imgui_text_buffer_init(&buffer);
    buffer.data = ctx->numeric_buffer;
    buffer.length = strlen(ctx->numeric_buffer);
    buffer.capacity = ctx->numeric_buffer_capacity;
    buffer.reserve = imgui_numeric_buffer_reserve_callback;
    buffer.user_data = ctx;
    edited = imgui_input_text_buffer_ex(ctx, &desc, &buffer);
    ctx->numeric_buffer = buffer.data;
    ctx->numeric_buffer_capacity = buffer.capacity;
    if (!edited) {
        return IMGUI_FALSE;
    }
    parsed = strtod(ctx->numeric_buffer, &end);
    if (end == ctx->numeric_buffer || *end != '\0' ||
        !imgui_double_is_finite(parsed)) return IMGUI_FALSE;
    if (*value != (float)parsed) {
        *value = (float)parsed;
        return IMGUI_TRUE;
    }
    return IMGUI_FALSE;
}

imgui_bool imgui_input_float(imgui_context *ctx,
                             const char *label,
                             float *value)
{
    return imgui_input_float_ex(ctx, label, value, "%.3f");
}

imgui_bool imgui_input_text_fixed(imgui_context *ctx,
                                  const char *label,
                                  char *buffer,
                                  size_t capacity)
{
    imgui_input_text_desc desc;
    imgui_input_text_desc_init(&desc, label);
    return imgui_input_text_fixed_ex(ctx, &desc, buffer, capacity);
}

static size_t imgui_bounded_string_length(const char *text, size_t capacity)
{
    size_t length;
    if (text == NULL) return 0;
    length = 0;
    while (length < capacity && text[length] != '\0') ++length;
    return length;
}

static size_t imgui_utf8_previous_byte(const char *text, size_t length)
{
    if (length == 0) return 0;
    --length;
    while (length > 0 &&
           (((unsigned char)text[length]) & 0xc0U) == 0x80U) {
        --length;
    }
    return length;
}

static imgui_bool imgui_text_word_separator(unsigned char character)
{
    if (character == ' ' || character == '\t' || character == '\n' ||
        character == '\r') return IMGUI_TRUE;
    /* Keep UTF-8 continuation/lead bytes grouped as part of a codepoint,
       while treating ASCII punctuation as a word boundary. */
    if (character < 0x80U &&
        !((character >= '0' && character <= '9') ||
          (character >= 'A' && character <= 'Z') ||
          (character >= 'a' && character <= 'z') || character == '_')) {
        return IMGUI_TRUE;
    }
    return IMGUI_FALSE;
}

static size_t imgui_utf8_next_byte(const char *text,
                                   size_t length,
                                   size_t cursor)
{
    if (text == NULL || cursor >= length) return length;
    ++cursor;
    while (cursor < length &&
           (((unsigned char)text[cursor]) & 0xc0U) == 0x80U) {
        ++cursor;
    }
    return cursor;
}

static size_t imgui_utf8_display_offset(const char *text,
                                        size_t length,
                                        size_t byte_offset)
{
    size_t cursor;
    size_t next;
    size_t count;
    if (text == NULL) return 0;
    if (byte_offset > length) byte_offset = length;
    cursor = 0;
    count = 0;
    while (cursor < byte_offset) {
        next = imgui_utf8_next_byte(text, length, cursor);
        if (next > byte_offset) break;
        cursor = next;
        ++count;
    }
    return count;
}

static void imgui_text_event_apply(imgui_text_event *event,
                                   char **data,
                                   size_t *length,
                                   size_t *capacity)
{
    if (event == NULL || data == NULL || length == NULL || capacity == NULL ||
        event->data == NULL || event->length > event->capacity) {
        return;
    }
    *data = event->data;
    *length = event->length;
    *capacity = event->capacity;
    if (*length < *capacity) (*data)[*length] = '\0';
}

/* Dear ImGui edit callbacks may reposition the caret or selection while
   replacing the buffer.  Keep those callback-owned edits in the same UTF-8
   byte-coordinate state used by the normal editor path. */
static void imgui_text_event_apply_cursor(imgui_context *ctx,
                                          const imgui_text_event *event,
                                          size_t length)
{
    size_t cursor;
    size_t selection_start;
    size_t selection_end;
    if (ctx == NULL || event == NULL) return;
    cursor = event->cursor_byte < 0 ? 0U : (size_t)event->cursor_byte;
    selection_start = event->selection_start_byte < 0 ?
        0U : (size_t)event->selection_start_byte;
    selection_end = event->selection_end_byte < 0 ?
        0U : (size_t)event->selection_end_byte;
    if (cursor > length) cursor = length;
    if (selection_start > length) selection_start = length;
    if (selection_end > length) selection_end = length;
    ctx->text_cursor_byte = cursor;
    ctx->text_selection_start_byte = selection_start;
    ctx->text_selection_end_byte = selection_end;
    ctx->text_selection_anchor_byte = cursor;
}

static size_t imgui_utf8_previous_word(const char *text, size_t cursor)
{
    unsigned char character;
    while (cursor > 0) {
        character = (unsigned char)text[cursor - 1];
        if (!imgui_text_word_separator(character)) break;
        cursor = imgui_utf8_previous_byte(text, cursor);
    }
    while (cursor > 0) {
        character = (unsigned char)text[cursor - 1];
        if (imgui_text_word_separator(character)) break;
        cursor = imgui_utf8_previous_byte(text, cursor);
    }
    return cursor;
}

static size_t imgui_utf8_next_word(const char *text,
                                   size_t length,
                                   size_t cursor)
{
    while (cursor < length &&
           !imgui_text_word_separator((unsigned char)text[cursor])) {
        cursor = imgui_utf8_next_byte(text, length, cursor);
    }
    while (cursor < length &&
           imgui_text_word_separator((unsigned char)text[cursor])) {
        cursor = imgui_utf8_next_byte(text, length, cursor);
    }
    return cursor;
}

static size_t imgui_utf8_vertical_cursor(const char *text,
                                         size_t length,
                                         size_t cursor,
                                         int direction)
{
    size_t line_start;
    size_t line_end;
    size_t previous_newline;
    size_t column;
    size_t target;
    size_t next;
    if (text == NULL) return 0;
    if (cursor > length) cursor = length;
    line_start = cursor;
    while (line_start > 0 && text[line_start - 1] != '\n') {
        line_start = imgui_utf8_previous_byte(text, line_start);
    }
    column = 0;
    target = line_start;
    while (target < cursor) {
        next = imgui_utf8_next_byte(text, length, target);
        if (next > cursor) break;
        target = next;
        ++column;
    }
    if (direction < 0) {
        if (line_start == 0) return cursor;
        previous_newline = line_start - 1;
        line_end = previous_newline;
        while (line_end > 0 && text[line_end - 1] != '\n') {
            line_end = imgui_utf8_previous_byte(text, line_end);
        }
        target = line_end;
    } else {
        line_end = line_start;
        while (line_end < length && text[line_end] != '\n') {
            line_end = imgui_utf8_next_byte(text, length, line_end);
        }
        if (line_end >= length) return cursor;
        target = line_end + 1;
    }
    while (column != 0 && target < length && text[target] != '\n') {
        target = imgui_utf8_next_byte(text, length, target);
        --column;
    }
    return target;
}

static imgui_bool imgui_text_history_store(imgui_context *ctx,
                                            imgui_bool undo,
                                            const char *data,
                                            size_t length,
                                            imgui_id id)
{
    char **buffer;
    size_t *capacity;
    size_t needed;
    size_t new_capacity;
    char *storage;
    if (ctx == NULL || data == NULL || length > (size_t)-1 - 1U) {
        return IMGUI_FALSE;
    }
    buffer = undo ? &ctx->text_undo_buffer : &ctx->text_redo_buffer;
    capacity = undo ? &ctx->text_undo_capacity : &ctx->text_redo_capacity;
    needed = length + 1U;
    if (*capacity < needed) {
        new_capacity = *capacity == 0 ? 64U : *capacity;
        while (new_capacity < needed) {
            if (new_capacity > (size_t)-1 / 2U) {
                new_capacity = needed;
                break;
            }
            new_capacity *= 2U;
        }
        storage = (char *)imgui_internal_allocate(&ctx->allocator,
                                                   new_capacity);
        if (storage == NULL) return IMGUI_FALSE;
        imgui_internal_release(&ctx->allocator, *buffer);
        *buffer = storage;
        *capacity = new_capacity;
    }
    memcpy(*buffer, data, needed);
    if (undo) {
        ctx->text_undo_length = length;
        ctx->text_undo_id = id;
        ctx->text_undo_valid = IMGUI_TRUE;
    } else {
        ctx->text_redo_length = length;
        ctx->text_redo_id = id;
        ctx->text_redo_valid = IMGUI_TRUE;
    }
    return IMGUI_TRUE;
}

static size_t imgui_text_cursor_from_mouse_position(
    const imgui_context *ctx, const char *data, size_t length,
    float mouse_x, float mouse_y)
{
    size_t line_start;
    size_t line_end;
    size_t cursor;
    size_t next;
    int target_line;
    int line;
    float line_height;
    float line_top;
    float x;
    float advance;
    const imgui_font_glyph *glyph;
    if (ctx == NULL || data == NULL) return 0;
    line_height = ctx->font != NULL ? imgui_font_get_line_height(ctx->font) :
                  16.0f;
    line_top = ctx->last_item_rect.y1 + ctx->style.frame_padding.y;
    target_line = mouse_y > line_top && line_height > 0.0f ?
        (int)((mouse_y - line_top) / line_height) : 0;
    if (target_line < 0) target_line = 0;
    line_start = 0;
    line = 0;
    while (line < target_line && line_start < length) {
        if (data[line_start] == '\n') ++line;
        line_start = imgui_utf8_next_byte(data, length, line_start);
    }
    line_end = line_start;
    while (line_end < length && data[line_end] != '\n') {
        line_end = imgui_utf8_next_byte(data, length, line_end);
    }
    x = ctx->last_item_rect.x1 + ctx->style.frame_padding.x;
    cursor = line_start;
    while (cursor < line_end) {
        next = imgui_utf8_next_byte(data, line_end, cursor);
        advance = 8.0f;
        if (ctx->font != NULL) {
            glyph = imgui_font_find_glyph(ctx->font,
                imgui_text_decode_codepoint(data + cursor, next - cursor));
            if (glyph != NULL) advance = glyph->advance_x;
        }
        if (mouse_x < x + advance * 0.5f) return cursor;
        x += advance;
        cursor = next;
    }
    return line_end;
}

static imgui_bool imgui_text_ctrl_down(const imgui_context *ctx)
{
    if (ctx == NULL) return IMGUI_FALSE;
    return (ctx->input.keys_down[IMGUI_KEY_LEFT_CTRL] ||
            ctx->input.keys_down[IMGUI_KEY_RIGHT_CTRL]) ?
           IMGUI_TRUE : IMGUI_FALSE;
}

static imgui_bool imgui_key_pressed_or_repeated(const imgui_context *ctx,
                                                imgui_key key)
{
    if (ctx == NULL || key < 0 || key >= IMGUI_KEY_COUNT) {
        return IMGUI_FALSE;
    }
    return (ctx->input.keys_pressed[key] ||
            ctx->input.keys_repeated[key]) ? IMGUI_TRUE : IMGUI_FALSE;
}

static imgui_bool imgui_text_edit_common(imgui_context *ctx,
                                         imgui_id id,
                                         char **data,
                                         size_t *length,
                                         size_t *capacity,
                                         imgui_input_text_flags flags,
                                         imgui_text_reserve_fn reserve,
                                         void *reserve_user_data,
                                         imgui_text_event_fn event_callback,
                                         void *event_user_data)
{
    size_t available;
    size_t copy_length;
    size_t required;
    size_t selection_start;
    size_t selection_end;
    size_t selection_anchor;
    size_t remove_length;
    size_t previous;
    size_t new_capacity;
    char *new_data;
    imgui_bool changed;
    imgui_bool restored;
    const char *insert_text;
    size_t insert_length;
    size_t filter_index;
    size_t filtered_length;
    size_t event_next;
    size_t event_reject_until;
    unsigned char character;
    unsigned long event_codepoint;
    imgui_bool keep_character;
    imgui_bool extend_selection;
    imgui_text_event event;
    char *clipboard_selection;
    const char *clipboard_text;
    clipboard_selection = NULL;
    if (ctx == NULL || data == NULL || length == NULL || capacity == NULL ||
        ctx->disabled_depth > 0) {
        return IMGUI_FALSE;
    }
    /* Text fields become active on mouse-down so the same frame can place
       the caret, unlike button-style activation which reports on release. */
    if (ctx->last_item_clicked ||
        (ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT] &&
         ctx->last_item_hovered)) {
        ctx->active_text_id = id;
        ctx->text_input_active = IMGUI_TRUE;
        if (ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT] &&
            ctx->last_item_hovered) {
            ctx->text_cursor_byte = imgui_text_cursor_from_mouse_position(
                ctx, *data != NULL ? *data : "", *length,
                ctx->input.mouse_x, ctx->input.mouse_y);
            if (ctx->mouse_double_clicked[IMGUI_MOUSE_BUTTON_LEFT]) {
                ctx->text_selection_start_byte = imgui_utf8_previous_word(
                    *data, ctx->text_cursor_byte);
                ctx->text_selection_end_byte = imgui_utf8_next_word(
                    *data, *length, ctx->text_cursor_byte);
                ctx->text_cursor_byte = ctx->text_selection_end_byte;
                ctx->text_selection_anchor_byte =
                    ctx->text_selection_start_byte;
            } else {
                ctx->text_selection_anchor_byte = ctx->text_cursor_byte;
                ctx->text_selection_start_byte = ctx->text_cursor_byte;
                ctx->text_selection_end_byte = ctx->text_cursor_byte;
            }
        }
    }
    /* Dear ImGui keeps text selection active while the left button is held,
       even when the pointer leaves the input rectangle.  Recompute the
       UTF-8 caret from the dragged x position and extend from the original
       anchor instead of collapsing the selection to the new caret. */
    if (ctx->text_input_active && ctx->active_text_id == id &&
        ctx->active_item_valid && ctx->active_item_id == id &&
        ctx->input.mouse_down[IMGUI_MOUSE_BUTTON_LEFT] &&
        !ctx->mouse_clicked[IMGUI_MOUSE_BUTTON_LEFT] &&
        (ctx->input.mouse_x !=
             ctx->mouse_last_click_position[IMGUI_MOUSE_BUTTON_LEFT].x ||
         ctx->input.mouse_y !=
             ctx->mouse_last_click_position[IMGUI_MOUSE_BUTTON_LEFT].y)) {
        ctx->text_cursor_byte = imgui_text_cursor_from_mouse_position(
            ctx, *data != NULL ? *data : "", *length,
            ctx->input.mouse_x, ctx->input.mouse_y);
        selection_anchor = ctx->text_selection_anchor_byte;
        ctx->text_selection_start_byte = selection_anchor <
            ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
        ctx->text_selection_end_byte = selection_anchor >
            ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
    }
    if (!ctx->text_input_active || ctx->active_text_id != id ||
        (flags & IMGUI_INPUT_TEXT_READ_ONLY) != 0) {
        return IMGUI_FALSE;
    }
    if (event_callback != NULL) {
        memset(&event, 0, sizeof(event));
        event.type = IMGUI_TEXT_EVENT_ALWAYS;
        event.data = *data;
        event.length = *length;
        event.capacity = *capacity;
        event.cursor_byte = (int)ctx->text_cursor_byte;
        event.selection_start_byte = (int)ctx->text_selection_start_byte;
        event.selection_end_byte = (int)ctx->text_selection_end_byte;
        event.user_data = event_user_data;
        (void)event_callback(&event);
        imgui_text_event_apply(&event, data, length, capacity);
        imgui_text_event_apply_cursor(ctx, &event, *length);
    }
    if (ctx->text_edit_id != id) {
        ctx->text_edit_id = id;
        ctx->text_cursor_byte = *length;
        ctx->text_selection_anchor_byte = *length;
        ctx->text_selection_start_byte = *length;
        ctx->text_selection_end_byte = *length;
        if ((flags & IMGUI_INPUT_TEXT_AUTO_SELECT_ALL) != 0) {
            ctx->text_selection_start_byte = 0;
            ctx->text_selection_end_byte = *length;
            ctx->text_cursor_byte = *length;
            ctx->text_selection_anchor_byte = 0;
        }
    }
    if (ctx->text_cursor_byte > *length) ctx->text_cursor_byte = *length;
    if (ctx->text_selection_start_byte > *length) {
        ctx->text_selection_start_byte = *length;
    }
    if (ctx->text_selection_end_byte > *length) {
        ctx->text_selection_end_byte = *length;
    }
    if (imgui_text_ctrl_down(ctx) &&
        ctx->input.keys_pressed[IMGUI_KEY_A]) {
        ctx->text_selection_start_byte = 0;
        ctx->text_selection_end_byte = *length;
        ctx->text_cursor_byte = *length;
        ctx->text_selection_anchor_byte = 0;
    }
    extend_selection =
        ctx->input.keys_down[IMGUI_KEY_LEFT_SHIFT] ||
        ctx->input.keys_down[IMGUI_KEY_RIGHT_SHIFT];
    if (imgui_key_pressed_or_repeated(ctx, IMGUI_KEY_LEFT_ARROW)) {
        if (!extend_selection &&
            ctx->text_selection_start_byte != ctx->text_selection_end_byte) {
            ctx->text_cursor_byte = ctx->text_selection_start_byte;
        } else {
            if (extend_selection &&
                ctx->text_selection_start_byte ==
                ctx->text_selection_end_byte) {
                ctx->text_selection_anchor_byte = ctx->text_cursor_byte;
            }
            if (imgui_text_ctrl_down(ctx)) {
                ctx->text_cursor_byte = imgui_utf8_previous_word(
                    *data, ctx->text_cursor_byte);
            } else {
                ctx->text_cursor_byte = imgui_utf8_previous_byte(
                    *data, ctx->text_cursor_byte);
            }
        }
        if (extend_selection) {
            selection_anchor = ctx->text_selection_anchor_byte;
            ctx->text_selection_start_byte = selection_anchor <
                ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
            ctx->text_selection_end_byte = selection_anchor >
                ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
        } else {
            ctx->text_selection_anchor_byte = ctx->text_cursor_byte;
            ctx->text_selection_start_byte = ctx->text_cursor_byte;
            ctx->text_selection_end_byte = ctx->text_cursor_byte;
        }
    }
    if (event_callback != NULL &&
        (imgui_key_pressed_or_repeated(ctx, IMGUI_KEY_UP_ARROW) ||
         imgui_key_pressed_or_repeated(ctx, IMGUI_KEY_DOWN_ARROW))) {
        memset(&event, 0, sizeof(event));
        event.type = IMGUI_TEXT_EVENT_HISTORY;
        event.data = *data;
        event.length = *length;
        event.capacity = *capacity;
        event.cursor_byte = (int)ctx->text_cursor_byte;
        event.selection_start_byte = (int)ctx->text_selection_start_byte;
        event.selection_end_byte = (int)ctx->text_selection_end_byte;
        event.input_codepoint = imgui_key_pressed_or_repeated(
                                    ctx, IMGUI_KEY_UP_ARROW) ?
                                1UL : 2UL;
        event.user_data = event_user_data;
        (void)event_callback(&event);
        imgui_text_event_apply(&event, data, length, capacity);
        imgui_text_event_apply_cursor(ctx, &event, *length);
    }
    if (event_callback != NULL && ctx->input.keys_pressed[IMGUI_KEY_TAB]) {
        memset(&event, 0, sizeof(event));
        event.type = IMGUI_TEXT_EVENT_COMPLETION;
        event.data = *data;
        event.length = *length;
        event.capacity = *capacity;
        event.cursor_byte = (int)ctx->text_cursor_byte;
        event.selection_start_byte = (int)ctx->text_selection_start_byte;
        event.selection_end_byte = (int)ctx->text_selection_end_byte;
        event.user_data = event_user_data;
        (void)event_callback(&event);
        imgui_text_event_apply(&event, data, length, capacity);
        imgui_text_event_apply_cursor(ctx, &event, *length);
    }
    if (imgui_key_pressed_or_repeated(ctx, IMGUI_KEY_RIGHT_ARROW)) {
        if (!extend_selection &&
            ctx->text_selection_start_byte != ctx->text_selection_end_byte) {
            ctx->text_cursor_byte = ctx->text_selection_end_byte;
        } else {
            if (extend_selection &&
                ctx->text_selection_start_byte ==
                ctx->text_selection_end_byte) {
                ctx->text_selection_anchor_byte = ctx->text_cursor_byte;
            }
            if (ctx->text_cursor_byte < *length) {
                if (imgui_text_ctrl_down(ctx)) {
                    ctx->text_cursor_byte = imgui_utf8_next_word(
                        *data, *length, ctx->text_cursor_byte);
                } else {
                    ctx->text_cursor_byte = imgui_utf8_next_byte(
                        *data, *length, ctx->text_cursor_byte);
                }
            }
        }
        if (extend_selection) {
            selection_anchor = ctx->text_selection_anchor_byte;
            ctx->text_selection_start_byte = selection_anchor <
                ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
            ctx->text_selection_end_byte = selection_anchor >
                ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
        } else {
            ctx->text_selection_anchor_byte = ctx->text_cursor_byte;
            ctx->text_selection_start_byte = ctx->text_cursor_byte;
            ctx->text_selection_end_byte = ctx->text_cursor_byte;
        }
    }
    if (ctx->input.keys_pressed[IMGUI_KEY_HOME]) {
        ctx->text_cursor_byte = 0;
        if (extend_selection) {
            selection_anchor = ctx->text_selection_anchor_byte;
            ctx->text_selection_start_byte = selection_anchor <
                ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
            ctx->text_selection_end_byte = selection_anchor >
                ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
        } else {
            ctx->text_selection_anchor_byte = 0;
            ctx->text_selection_start_byte = 0;
            ctx->text_selection_end_byte = 0;
        }
    }
    if (ctx->input.keys_pressed[IMGUI_KEY_END]) {
        ctx->text_cursor_byte = *length;
        if (extend_selection) {
            selection_anchor = ctx->text_selection_anchor_byte;
            ctx->text_selection_start_byte = selection_anchor <
                ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
            ctx->text_selection_end_byte = selection_anchor >
                ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
        } else {
            ctx->text_selection_anchor_byte = *length;
            ctx->text_selection_start_byte = *length;
            ctx->text_selection_end_byte = *length;
        }
    }
    if (imgui_key_pressed_or_repeated(ctx, IMGUI_KEY_UP_ARROW) ||
        imgui_key_pressed_or_repeated(ctx, IMGUI_KEY_DOWN_ARROW)) {
        previous = ctx->text_cursor_byte;
        ctx->text_cursor_byte = imgui_utf8_vertical_cursor(
            *data, *length, ctx->text_cursor_byte,
            imgui_key_pressed_or_repeated(ctx, IMGUI_KEY_UP_ARROW) ? -1 : 1);
        if (extend_selection) {
            if (ctx->text_selection_start_byte ==
                ctx->text_selection_end_byte) {
                ctx->text_selection_anchor_byte = previous;
            }
            selection_anchor = ctx->text_selection_anchor_byte;
            ctx->text_selection_start_byte = selection_anchor <
                ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
            ctx->text_selection_end_byte = selection_anchor >
                ctx->text_cursor_byte ? selection_anchor : ctx->text_cursor_byte;
        } else {
            ctx->text_selection_anchor_byte = ctx->text_cursor_byte;
            ctx->text_selection_start_byte = ctx->text_cursor_byte;
            ctx->text_selection_end_byte = ctx->text_cursor_byte;
        }
    }
    selection_start = ctx->text_selection_start_byte;
    selection_end = ctx->text_selection_end_byte;
    if (selection_start > selection_end) {
        previous = selection_start;
        selection_start = selection_end;
        selection_end = previous;
    }
    if (imgui_text_ctrl_down(ctx) &&
        ctx->input.keys_pressed[IMGUI_KEY_C] &&
        selection_start != selection_end && *data != NULL &&
        (ctx->platform.capabilities & IMGUI_PLATFORM_CAP_CLIPBOARD) != 0 &&
        ctx->platform.clipboard_set != NULL &&
        selection_end - selection_start <= (size_t)-1 - 1U) {
        clipboard_selection = (char *)imgui_internal_allocate(
            &ctx->allocator, selection_end - selection_start + 1U);
        if (clipboard_selection != NULL) {
            memcpy(clipboard_selection, *data + selection_start,
                   selection_end - selection_start);
            clipboard_selection[selection_end - selection_start] = '\0';
            ctx->platform.clipboard_set(clipboard_selection,
                                        ctx->platform.user_data);
            imgui_internal_release(&ctx->allocator, clipboard_selection);
            clipboard_selection = NULL;
        }
    }
    if (imgui_text_ctrl_down(ctx) &&
        ctx->input.keys_pressed[IMGUI_KEY_V] &&
        (ctx->platform.capabilities & IMGUI_PLATFORM_CAP_CLIPBOARD) != 0 &&
        ctx->platform.clipboard_get != NULL) {
        clipboard_text = ctx->platform.clipboard_get(ctx->platform.user_data);
        if (clipboard_text != NULL) {
            (void)imgui_input_add_text_utf8(ctx, clipboard_text);
        }
    }
    changed = IMGUI_FALSE;
    restored = IMGUI_FALSE;
    if ((flags & IMGUI_INPUT_TEXT_ESCAPE_CLEARS) != 0 &&
        ctx->input.keys_pressed[IMGUI_KEY_ESCAPE] && *data != NULL) {
        if ((flags & IMGUI_INPUT_TEXT_NO_UNDO_REDO) == 0 &&
            imgui_text_history_store(ctx, IMGUI_TRUE, *data, *length, id)) {
            ctx->text_redo_valid = IMGUI_FALSE;
        }
        *length = 0;
        (*data)[0] = '\0';
        ctx->text_cursor_byte = 0;
        ctx->text_selection_start_byte = 0;
        ctx->text_selection_end_byte = 0;
        ctx->pending_text_length = 0;
        if (ctx->pending_text != NULL) ctx->pending_text[0] = '\0';
        changed = IMGUI_TRUE;
        restored = IMGUI_TRUE;
    }
    if ((flags & IMGUI_INPUT_TEXT_NO_UNDO_REDO) == 0 &&
        imgui_text_ctrl_down(ctx) &&
        ctx->input.keys_pressed[IMGUI_KEY_Y] &&
        ctx->text_redo_valid && ctx->text_redo_id == id) {
        if (*data != NULL) (void)imgui_text_history_store(
            ctx, IMGUI_TRUE, *data, *length, id);
        required = ctx->text_redo_length + 1;
        if (required <= *capacity && *data != NULL) {
            memcpy(*data, ctx->text_redo_buffer, required);
            *length = ctx->text_redo_length;
            ctx->text_cursor_byte = *length;
            ctx->text_selection_start_byte = *length;
            ctx->text_selection_end_byte = *length;
            ctx->text_redo_valid = IMGUI_FALSE;
            changed = IMGUI_TRUE;
            restored = IMGUI_TRUE;
        } else if (required > *capacity && reserve != NULL) {
            new_capacity = *capacity;
            if (new_capacity < 16) new_capacity = 16;
            while (new_capacity < required) {
                if (new_capacity > (size_t)-1 / 2U) return IMGUI_FALSE;
                new_capacity *= 2;
            }
            new_data = *data;
            if (reserve(reserve_user_data, new_capacity, &new_data,
                        &new_capacity) == IMGUI_RESULT_OK &&
                new_data != NULL && new_capacity >= required) {
                *data = new_data;
                *capacity = new_capacity;
                memcpy(*data, ctx->text_redo_buffer, required);
                *length = ctx->text_redo_length;
                ctx->text_cursor_byte = *length;
                ctx->text_selection_start_byte = *length;
                ctx->text_selection_end_byte = *length;
                ctx->text_redo_valid = IMGUI_FALSE;
                changed = IMGUI_TRUE;
                restored = IMGUI_TRUE;
            }
        }
    }
    if ((flags & IMGUI_INPUT_TEXT_NO_UNDO_REDO) == 0 &&
        imgui_text_ctrl_down(ctx) &&
        ctx->input.keys_pressed[IMGUI_KEY_Z] &&
        ctx->text_undo_valid && ctx->text_undo_id == id) {
        required = ctx->text_undo_length + 1;
        if (*data != NULL) (void)imgui_text_history_store(
            ctx, IMGUI_FALSE, *data, *length, id);
        if (required <= *capacity) {
            if (*data != NULL) {
                memcpy(*data, ctx->text_undo_buffer, required);
                *length = ctx->text_undo_length;
                ctx->text_cursor_byte = *length;
                ctx->text_selection_start_byte = *length;
                ctx->text_selection_end_byte = *length;
                ctx->text_undo_valid = IMGUI_FALSE;
                changed = IMGUI_TRUE;
                restored = IMGUI_TRUE;
            }
        } else if (reserve != NULL) {
            new_capacity = *capacity;
            if (new_capacity < 16) new_capacity = 16;
            while (new_capacity < required) {
                if (new_capacity > (size_t)-1 / 2U) return IMGUI_FALSE;
                new_capacity *= 2;
            }
            new_data = *data;
            if (reserve(reserve_user_data, new_capacity, &new_data,
                        &new_capacity) == IMGUI_RESULT_OK &&
                new_data != NULL && new_capacity >= required) {
                *data = new_data;
                *capacity = new_capacity;
                memcpy(*data, ctx->text_undo_buffer, required);
                *length = ctx->text_undo_length;
                ctx->text_cursor_byte = *length;
                ctx->text_selection_start_byte = *length;
                ctx->text_selection_end_byte = *length;
                ctx->text_undo_valid = IMGUI_FALSE;
                changed = IMGUI_TRUE;
                restored = IMGUI_TRUE;
            }
        }
    }
    if (!restored && ctx->pending_text_length != 0) {
        insert_text = ctx->pending_text;
        insert_length = ctx->pending_text_length;
        if ((flags & (IMGUI_INPUT_TEXT_DECIMAL |
                      IMGUI_INPUT_TEXT_SCIENTIFIC |
                      IMGUI_INPUT_TEXT_HEXADECIMAL |
                      IMGUI_INPUT_TEXT_NO_BLANK |
                      IMGUI_INPUT_TEXT_UPPERCASE |
                      IMGUI_INPUT_TEXT_ALLOW_TAB)) != 0 ||
            event_callback != NULL) {
            filtered_length = 0;
            event_reject_until = 0;
            for (filter_index = 0; filter_index < insert_length;
                 ++filter_index) {
                character = (unsigned char)insert_text[filter_index];
                keep_character = IMGUI_TRUE;
                if (filter_index < event_reject_until) {
                    keep_character = IMGUI_FALSE;
                } else if (event_callback != NULL &&
                           (character & 0xc0U) != 0x80U) {
                    event_next = imgui_utf8_next_byte(insert_text,
                                                      insert_length,
                                                      filter_index);
                    event_codepoint = imgui_text_decode_codepoint(
                        insert_text + filter_index,
                        event_next - filter_index);
                    memset(&event, 0, sizeof(event));
                    event.type = IMGUI_TEXT_EVENT_FILTER_CHARACTER;
                    event.data = *data;
                    event.length = *length;
                    event.capacity = *capacity;
                    event.input_codepoint = event_codepoint;
                    event.user_data = event_user_data;
                    (void)event_callback(&event);
                    if (event.input_codepoint == 0) {
                        keep_character = IMGUI_FALSE;
                        event_reject_until = event_next;
                    }
                }
                if ((flags & IMGUI_INPUT_TEXT_NO_BLANK) != 0 &&
                    (character == ' ' || character == '\t')) {
                    keep_character = IMGUI_FALSE;
                }
                if (character == '\t' &&
                    (flags & IMGUI_INPUT_TEXT_ALLOW_TAB) == 0) {
                    keep_character = IMGUI_FALSE;
                }
                if ((flags & IMGUI_INPUT_TEXT_DECIMAL) != 0 &&
                    !((character >= '0' && character <= '9') ||
                      character == '.' || character == '-' ||
                      character == '+' ||
                      (((flags & IMGUI_INPUT_TEXT_SCIENTIFIC) != 0) &&
                       (character == 'e' || character == 'E')))) {
                    keep_character = IMGUI_FALSE;
                }
                if ((flags & IMGUI_INPUT_TEXT_SCIENTIFIC) != 0 &&
                    !((character >= '0' && character <= '9') ||
                      character == '.' || character == '-' ||
                      character == '+' || character == 'e' ||
                      character == 'E')) {
                    keep_character = IMGUI_FALSE;
                }
                if ((flags & IMGUI_INPUT_TEXT_HEXADECIMAL) != 0 &&
                    !((character >= '0' && character <= '9') ||
                      (character >= 'a' && character <= 'f') ||
                      (character >= 'A' && character <= 'F') ||
                      character == 'x' || character == 'X')) {
                    keep_character = IMGUI_FALSE;
                }
                if ((flags & IMGUI_INPUT_TEXT_UPPERCASE) != 0 &&
                    character >= 'a' && character <= 'z') {
                    character = (unsigned char)(character - 'a' + 'A');
                }
                if (keep_character) {
                    ctx->pending_text[filtered_length++] = (char)character;
                }
            }
            ctx->pending_text[filtered_length] = '\0';
            insert_text = ctx->pending_text;
            insert_length = filtered_length;
        }
        if ((flags & IMGUI_INPUT_TEXT_NO_UNDO_REDO) == 0 &&
            *data != NULL && imgui_text_history_store(
                ctx, IMGUI_TRUE, *data, *length, id)) {
            ctx->text_redo_valid = IMGUI_FALSE;
        }
        selection_start = ctx->text_selection_start_byte;
        selection_end = ctx->text_selection_end_byte;
        if (selection_start > selection_end) {
            previous = selection_start;
            selection_start = selection_end;
            selection_end = previous;
        }
        if ((flags & IMGUI_INPUT_TEXT_ALWAYS_OVERWRITE) != 0 &&
            selection_start == selection_end &&
            ctx->text_cursor_byte == selection_start) {
            size_t overwrite_cursor = selection_end;
            size_t overwrite_index = 0;
            size_t insert_codepoints = 0;
            size_t insert_scan = 0;
            while (insert_scan < insert_length) {
                size_t next_insert = imgui_utf8_next_byte(
                    insert_text, insert_length, insert_scan);
                if (next_insert <= insert_scan) break;
                insert_scan = next_insert;
                ++insert_codepoints;
            }
            while (overwrite_index < insert_codepoints &&
                   overwrite_cursor < *length) {
                size_t next = imgui_utf8_next_byte(*data, *length,
                                                   overwrite_cursor);
                if (next <= overwrite_cursor) break;
                overwrite_cursor = next;
                ++overwrite_index;
            }
            selection_end = overwrite_cursor;
        }
        remove_length = selection_end - selection_start;
        required = *length - remove_length + insert_length + 1;
        if (required > *capacity) {
            if (reserve != NULL) {
                new_capacity = *capacity;
                if (new_capacity < 16) new_capacity = 16;
                while (new_capacity < required) {
                    if (new_capacity > (size_t)-1 / 2U) {
                        return IMGUI_FALSE;
                    }
                    new_capacity *= 2;
                }
                new_data = *data;
                if (reserve(reserve_user_data, new_capacity,
                            &new_data, &new_capacity) != IMGUI_RESULT_OK ||
                    new_data == NULL || new_capacity < required) {
                    return IMGUI_FALSE;
                }
                *data = new_data;
                *capacity = new_capacity;
            }
        }
        available = *capacity > *length - remove_length + 1 ?
            *capacity - (*length - remove_length) - 1 : 0;
        copy_length = insert_length < available ? insert_length : available;
        if (copy_length != 0 && *data != NULL) {
            memmove(*data + selection_start + copy_length,
                    *data + selection_end, *length - selection_end + 1);
            memcpy(*data + selection_start, insert_text, copy_length);
            *length = *length - remove_length + copy_length;
            (*data)[*length] = '\0';
            ctx->text_cursor_byte = selection_start + copy_length;
            ctx->text_selection_start_byte = ctx->text_cursor_byte;
            ctx->text_selection_end_byte = ctx->text_cursor_byte;
            changed = IMGUI_TRUE;
        }
        ctx->pending_text_length = 0;
        if (ctx->pending_text != NULL) ctx->pending_text[0] = '\0';
    }
    if (!restored && (ctx->input.keys_repeated[IMGUI_KEY_BACKSPACE] ||
         imgui_key_pressed_or_repeated(ctx, IMGUI_KEY_DELETE)) &&
        *data != NULL) {
        if ((flags & IMGUI_INPUT_TEXT_NO_UNDO_REDO) == 0 &&
            imgui_text_history_store(ctx, IMGUI_TRUE, *data, *length, id)) {
            ctx->text_redo_valid = IMGUI_FALSE;
        }
        selection_start = ctx->text_selection_start_byte;
        selection_end = ctx->text_selection_end_byte;
        if (selection_start > selection_end) {
            previous = selection_start;
            selection_start = selection_end;
            selection_end = previous;
        }
        if (selection_start != selection_end) {
            memmove(*data + selection_start, *data + selection_end,
                    *length - selection_end + 1);
            *length -= selection_end - selection_start;
            ctx->text_cursor_byte = selection_start;
            changed = IMGUI_TRUE;
        } else if (ctx->input.keys_repeated[IMGUI_KEY_BACKSPACE] &&
                   ctx->text_cursor_byte != 0) {
            if (ctx->input.keys_down[IMGUI_KEY_LEFT_CTRL] ||
                ctx->input.keys_down[IMGUI_KEY_RIGHT_CTRL]) {
                previous = imgui_utf8_previous_word(*data,
                                                    ctx->text_cursor_byte);
            } else {
                previous = imgui_utf8_previous_byte(*data,
                                                    ctx->text_cursor_byte);
            }
            memmove(*data + previous, *data + ctx->text_cursor_byte,
                    *length - ctx->text_cursor_byte + 1);
            *length -= ctx->text_cursor_byte - previous;
            ctx->text_cursor_byte = previous;
            changed = IMGUI_TRUE;
        } else if (imgui_key_pressed_or_repeated(ctx, IMGUI_KEY_DELETE) &&
                   ctx->text_cursor_byte < *length) {
            if (ctx->input.keys_down[IMGUI_KEY_LEFT_CTRL] ||
                ctx->input.keys_down[IMGUI_KEY_RIGHT_CTRL]) {
                previous = imgui_utf8_next_word(*data, *length,
                                                ctx->text_cursor_byte);
            } else {
                previous = imgui_utf8_next_byte(*data, *length,
                                                ctx->text_cursor_byte);
            }
            memmove(*data + ctx->text_cursor_byte, *data + previous,
                    *length - previous + 1);
            *length -= previous - ctx->text_cursor_byte;
            changed = IMGUI_TRUE;
        }
        ctx->text_selection_start_byte = ctx->text_cursor_byte;
        ctx->text_selection_end_byte = ctx->text_cursor_byte;
    }
    if (ctx->text_selection_start_byte == ctx->text_selection_end_byte) {
        ctx->text_selection_anchor_byte = ctx->text_cursor_byte;
    }
    if (changed) ctx->last_item_edited = IMGUI_TRUE;
    return changed;
}

static void imgui_input_text_render(imgui_context *ctx, const char *data,
                                    size_t length, imgui_id id,
                                    imgui_input_text_flags flags,
                                    const char *hint)
{
    char *password_data;
    const char *display_data;
    size_t display_length;
    size_t display_selection_start;
    size_t display_selection_end;
    size_t display_cursor;
    size_t mask_cursor;
    size_t mask_next;
    imgui_bool using_hint;
    imgui_u32 old_text_color;
    imgui_u32 color;
    imgui_vec2 measured;
    imgui_vec2 selection_start_measure;
    imgui_vec2 selection_end_measure;
    imgui_rect selection_rect;
    imgui_rect saved_clip;
    imgui_vec2 cursor_start;
    imgui_vec2 cursor_end;
    float text_origin_y;
    float text_origin_x;
    float available_width;
    float text_scroll_x;
    password_data = NULL;
    if (ctx == NULL) return;
    text_origin_y = (flags & IMGUI_INPUT_TEXT_WORD_WRAP) != 0 ?
        ctx->last_item_rect.y1 + ctx->style.frame_padding.y :
        imgui_item_label_y(ctx, ctx->last_item_rect);
    color = ctx->last_item_active ? ctx->style.color_frame_active :
            (ctx->last_item_hovered ? ctx->style.color_frame_hovered :
                                      ctx->style.color_frame);
    (void)imgui_mesh_add_rect(ctx, ctx->last_item_rect, color);
    saved_clip = ctx->clip_rect;
    if (ctx->last_item_rect.x1 > ctx->clip_rect.x1) {
        ctx->clip_rect.x1 = ctx->last_item_rect.x1;
    }
    if (ctx->last_item_rect.y1 > ctx->clip_rect.y1) {
        ctx->clip_rect.y1 = ctx->last_item_rect.y1;
    }
    if (ctx->last_item_rect.x2 < ctx->clip_rect.x2) {
        ctx->clip_rect.x2 = ctx->last_item_rect.x2;
    }
    if (ctx->last_item_rect.y2 < ctx->clip_rect.y2) {
        ctx->clip_rect.y2 = ctx->last_item_rect.y2;
    }
    if (data == NULL && hint == NULL) {
        ctx->clip_rect = saved_clip;
        return;
    }
    if (data == NULL) data = "";
    display_data = data;
    display_length = length;
    using_hint = length == 0 && hint != NULL ? IMGUI_TRUE : IMGUI_FALSE;
    if (using_hint) {
        display_data = hint;
        display_length = strlen(hint);
    }
    if (!using_hint && (flags & IMGUI_INPUT_TEXT_PASSWORD) != 0) {
        if (length > (size_t)-1 - 1U) {
            ctx->clip_rect = saved_clip;
            return;
        }
        password_data = (char *)imgui_internal_allocate(
            &ctx->allocator, length + 1U);
        if (password_data == NULL) {
            ctx->clip_rect = saved_clip;
            return;
        }
        display_length = 0;
        mask_cursor = 0;
        while (mask_cursor < length) {
            mask_next = imgui_utf8_next_byte(data, length, mask_cursor);
            password_data[display_length++] = '*';
            mask_cursor = mask_next;
        }
        password_data[display_length] = '\0';
        display_data = password_data;
    }
    if (ctx->text_scroll_id != id) {
        ctx->text_scroll_id = id;
        ctx->text_scroll_x = 0.0f;
    }
    text_scroll_x = ctx->text_scroll_x;
    available_width = ctx->last_item_rect.x2 - ctx->last_item_rect.x1 -
                      2.0f * ctx->style.frame_padding.x;
    if (available_width < 0.0f) available_width = 0.0f;
    if ((flags & IMGUI_INPUT_TEXT_NO_HORIZONTAL_SCROLL) != 0 ||
        (flags & IMGUI_INPUT_TEXT_WORD_WRAP) != 0 || !ctx->last_item_active ||
        using_hint) {
        text_scroll_x = 0.0f;
    } else if (ctx->active_text_id == id &&
               ctx->text_cursor_byte <= length) {
        size_t scroll_cursor;
        imgui_vec2 scroll_measure;
        scroll_cursor = (flags & IMGUI_INPUT_TEXT_PASSWORD) != 0 ?
            imgui_utf8_display_offset(data, length, ctx->text_cursor_byte) :
            (ctx->text_cursor_byte < display_length ?
             ctx->text_cursor_byte : display_length);
        scroll_measure = imgui_font_measure_text(
            ctx->font, display_data, display_data + scroll_cursor, 0.0f);
        if (scroll_measure.x - text_scroll_x > available_width) {
            text_scroll_x = scroll_measure.x - available_width;
        }
        if (scroll_measure.x - text_scroll_x < 0.0f) {
            text_scroll_x = scroll_measure.x;
        }
        if (text_scroll_x < 0.0f) text_scroll_x = 0.0f;
    }
    ctx->text_scroll_x = text_scroll_x;
    text_origin_x = ctx->last_item_rect.x1 + ctx->style.frame_padding.x -
                    text_scroll_x;
    if (ctx->font != NULL && ctx->font_texture != NULL) {
        if (ctx->active_text_id == id &&
            ctx->text_selection_start_byte != ctx->text_selection_end_byte &&
            ctx->text_selection_start_byte <= length &&
            ctx->text_selection_end_byte <= length) {
            display_selection_start = (flags & IMGUI_INPUT_TEXT_PASSWORD) != 0 ?
                imgui_utf8_display_offset(data, length,
                                          ctx->text_selection_start_byte) :
                (ctx->text_selection_start_byte < display_length ?
                 ctx->text_selection_start_byte : display_length);
            display_selection_end = (flags & IMGUI_INPUT_TEXT_PASSWORD) != 0 ?
                imgui_utf8_display_offset(data, length,
                                          ctx->text_selection_end_byte) :
                (ctx->text_selection_end_byte < display_length ?
                 ctx->text_selection_end_byte : display_length);
            selection_start_measure = imgui_font_measure_text(
                ctx->font, display_data,
                display_data + display_selection_start,
                0.0f);
            selection_end_measure = imgui_font_measure_text(
                ctx->font, display_data,
                display_data + display_selection_end,
                0.0f);
            selection_rect.x1 = ctx->last_item_rect.x1 +
                ctx->style.frame_padding.x + selection_start_measure.x -
                text_scroll_x;
            selection_rect.x2 = ctx->last_item_rect.x1 +
                ctx->style.frame_padding.x + selection_end_measure.x -
                text_scroll_x;
            if (selection_rect.x1 > selection_rect.x2) {
                float selection_swap = selection_rect.x1;
                selection_rect.x1 = selection_rect.x2;
                selection_rect.x2 = selection_swap;
            }
            selection_rect.y1 = text_origin_y;
            selection_rect.y2 = selection_rect.y1 +
                imgui_font_get_line_height(ctx->font);
            (void)imgui_mesh_add_rect(ctx, selection_rect,
                                      ctx->style.color_text_selection);
        }
        old_text_color = ctx->style.color_text;
        if (using_hint) ctx->style.color_text = ctx->style.color_text_disabled;
        if ((flags & IMGUI_INPUT_TEXT_WORD_WRAP) != 0) {
            imgui_text_draw_font_wrap(
                ctx, display_data, display_data + display_length,
                imgui_make_vec2(text_origin_x,
                                text_origin_y),
                ctx->last_item_rect.x2 - ctx->last_item_rect.x1 -
                2.0f * ctx->style.frame_padding.x);
        } else {
            imgui_text_draw_font(
                ctx, display_data, display_data + display_length,
                imgui_make_vec2(text_origin_x,
                                text_origin_y));
        }
        ctx->style.color_text = old_text_color;
        if (ctx->active_text_id == id && ctx->text_cursor_byte <= length) {
            display_cursor = (flags & IMGUI_INPUT_TEXT_PASSWORD) != 0 ?
                imgui_utf8_display_offset(data, length,
                                          ctx->text_cursor_byte) :
                (ctx->text_cursor_byte < display_length ?
                 ctx->text_cursor_byte : display_length);
            measured = imgui_font_measure_text(ctx->font, display_data,
                                                display_data + display_cursor,
                                                0.0f);
            cursor_start = imgui_make_vec2(
                text_origin_x + measured.x,
                text_origin_y);
            cursor_end = imgui_make_vec2(cursor_start.x,
                cursor_start.y + imgui_font_get_line_height(ctx->font));
            (void)imgui_draw_list_add_line(ctx, &ctx->default_draw_list,
                                           cursor_start, cursor_end,
                                           ctx->style.color_text, 1.0f);
        }
    }
    imgui_internal_release(&ctx->allocator, password_data);
    ctx->clip_rect = saved_clip;
}

static void imgui_input_text_queue_multiline_enter(
    imgui_context *ctx,
    imgui_id id,
    const imgui_input_text_desc *desc)
{
    imgui_bool ctrl_down;
    imgui_bool shift_down;
    if (ctx == NULL || desc == NULL || !desc->multiline ||
        (desc->flags & IMGUI_INPUT_TEXT_READ_ONLY) != 0 ||
        !ctx->input.keys_pressed[IMGUI_KEY_ENTER] ||
        (ctx->active_text_id != id && !ctx->last_item_clicked)) {
        return;
    }
    ctrl_down = ctx->input.keys_down[IMGUI_KEY_LEFT_CTRL] ||
                ctx->input.keys_down[IMGUI_KEY_RIGHT_CTRL];
    shift_down = ctx->input.keys_down[IMGUI_KEY_LEFT_SHIFT] ||
                 ctx->input.keys_down[IMGUI_KEY_RIGHT_SHIFT];
    if ((ctx->input.keys_down[IMGUI_KEY_LEFT_ALT] ||
         ctx->input.keys_down[IMGUI_KEY_RIGHT_ALT] ||
         ctx->input.keys_down[IMGUI_KEY_LEFT_SUPER] ||
         ctx->input.keys_down[IMGUI_KEY_RIGHT_SUPER])) {
        return;
    }
    if ((desc->flags & IMGUI_INPUT_TEXT_CTRL_ENTER_FOR_NEW_LINE) != 0) {
        if (ctrl_down || shift_down) (void)imgui_input_add_text_utf8(ctx, "\n");
    } else if (!ctrl_down || shift_down) {
        (void)imgui_input_add_text_utf8(ctx, "\n");
    }
}

imgui_bool imgui_input_text_fixed_ex(imgui_context *ctx,
                                     const imgui_input_text_desc *desc,
                                     char *buffer,
                                     size_t capacity)
{
    imgui_input_text_desc local_desc;
    imgui_id id;
    imgui_vec2 size;
    size_t length;
    imgui_bool edited;
    imgui_text_event event;
    if (ctx == NULL || buffer == NULL || capacity == 0 ||
        !imgui_input_text_desc_normalize(desc, &local_desc)) {
        return IMGUI_FALSE;
    }
    if (local_desc.multiline) local_desc.flags |= IMGUI_INPUT_TEXT_WORD_WRAP;
    desc = &local_desc;
    length = imgui_bounded_string_length(buffer, capacity);
    if (length == capacity) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_ARGUMENT,
                              "fixed text buffer is not terminated");
        return IMGUI_FALSE;
    }
    id = desc->use_explicit_id ? desc->id :
         imgui_get_id_string(ctx, desc->label != NULL ? desc->label : "");
    size = desc->size.x > 0.0f && desc->size.y > 0.0f ? desc->size :
           imgui_make_vec2(220.0f, desc->multiline ? 100.0f : 22.0f);
    if (!imgui_item_register(ctx, id, size)) return IMGUI_FALSE;
    imgui_input_text_queue_multiline_enter(ctx, id, desc);
    edited = imgui_text_edit_common(ctx, id, &buffer, &length, &capacity,
                                    desc->flags, NULL, NULL,
                                    desc->event_callback,
                                    desc->event_user_data);
    if (!edited && (desc->flags & IMGUI_INPUT_TEXT_ENTER_RETURNS_TRUE) != 0 &&
        ctx->input.keys_pressed[IMGUI_KEY_ENTER] &&
        ctx->active_text_id == id) {
        edited = IMGUI_TRUE;
    }
    if (edited && desc->event_callback != NULL) {
        memset(&event, 0, sizeof(event));
        event.type = IMGUI_TEXT_EVENT_EDIT;
        event.data = buffer;
        event.length = length;
        event.capacity = capacity;
        event.cursor_byte = (int)ctx->text_cursor_byte;
        event.selection_start_byte = (int)ctx->text_selection_start_byte;
        event.selection_end_byte = (int)ctx->text_selection_end_byte;
        event.data_changed = IMGUI_TRUE;
        event.user_data = desc->event_user_data;
        (void)desc->event_callback(&event);
        if (event.data != NULL && event.length <= capacity) {
            length = event.length < capacity ? event.length : capacity - 1U;
            if (event.data != buffer && length != 0) {
                memcpy(buffer, event.data, length);
            }
            buffer[length] = '\0';
        }
        imgui_text_event_apply_cursor(ctx, &event, length);
    }
    imgui_input_text_render(ctx, buffer, length, id, desc->flags,
                            desc->hint);
    return edited;
}

imgui_bool imgui_input_text_buffer(imgui_context *ctx,
                                   const char *label,
                                   imgui_text_buffer *buffer)
{
    imgui_input_text_desc desc;
    imgui_input_text_desc_init(&desc, label);
    return imgui_input_text_buffer_ex(ctx, &desc, buffer);
}

imgui_bool imgui_input_text_buffer_ex(imgui_context *ctx,
                                      const imgui_input_text_desc *desc,
                                      imgui_text_buffer *buffer)
{
    imgui_input_text_desc local_desc;
    imgui_id id;
    imgui_vec2 size;
    size_t length;
    size_t capacity;
    char *data;
    imgui_bool edited;
    imgui_text_event event;
    if (ctx == NULL || buffer == NULL || buffer->struct_size < sizeof(*buffer) ||
        !imgui_input_text_desc_normalize(desc, &local_desc)) {
        return IMGUI_FALSE;
    }
    if (local_desc.multiline) local_desc.flags |= IMGUI_INPUT_TEXT_WORD_WRAP;
    desc = &local_desc;
    if (buffer->length > buffer->capacity ||
        (buffer->length != 0 && buffer->data == NULL)) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_ARGUMENT,
                              "managed text buffer is invalid");
        return IMGUI_FALSE;
    }
    id = desc->use_explicit_id ? desc->id :
         imgui_get_id_string(ctx, desc->label != NULL ? desc->label : "");
    size = desc->size.x > 0.0f && desc->size.y > 0.0f ? desc->size :
           imgui_make_vec2(220.0f, desc->multiline ? 100.0f : 22.0f);
    if (!imgui_item_register(ctx, id, size)) return IMGUI_FALSE;
    imgui_input_text_queue_multiline_enter(ctx, id, desc);
    data = buffer->data;
    length = buffer->length;
    capacity = buffer->capacity;
    if (data != NULL && length < capacity) data[length] = '\0';
    edited = imgui_text_edit_common(ctx, id, &data, &length, &capacity,
                                    desc->flags, buffer->reserve,
                                    buffer->user_data,
                                    desc->event_callback,
                                    desc->event_user_data);
    if (!edited && (desc->flags & IMGUI_INPUT_TEXT_ENTER_RETURNS_TRUE) != 0 &&
        ctx->input.keys_pressed[IMGUI_KEY_ENTER] &&
        ctx->active_text_id == id) {
        edited = IMGUI_TRUE;
    }
    if (!edited) {
        imgui_input_text_render(ctx, data, length, id, desc->flags,
                                desc->hint);
        return IMGUI_FALSE;
    }
    if (desc->event_callback != NULL) {
        memset(&event, 0, sizeof(event));
        event.type = IMGUI_TEXT_EVENT_EDIT;
        event.data = data;
        event.length = length;
        event.capacity = capacity;
        event.cursor_byte = (int)ctx->text_cursor_byte;
        event.selection_start_byte = (int)ctx->text_selection_start_byte;
        event.selection_end_byte = (int)ctx->text_selection_end_byte;
        event.data_changed = IMGUI_TRUE;
        event.user_data = desc->event_user_data;
        (void)desc->event_callback(&event);
        if (event.data != NULL && event.length <= event.capacity) {
            data = event.data;
            length = event.length;
            capacity = event.capacity;
            if (length < capacity) data[length] = '\0';
        }
        imgui_text_event_apply_cursor(ctx, &event, length);
    }
    imgui_input_text_render(ctx, data, length, id, desc->flags,
                            desc->hint);
    buffer->data = data;
    buffer->length = length;
    buffer->capacity = capacity;
    return IMGUI_TRUE;
}

imgui_bool imgui_is_item_hovered(imgui_context *ctx, imgui_flags flags)
{
    imgui_bool hovered;
    imgui_bool rect_only;
    imgui_bool point_in_rect;
    if (ctx == NULL || (!ctx->window_active &&
                        (flags & IMGUI_HOVERED_RECT_ONLY) == 0)) {
        return IMGUI_FALSE;
    }
    rect_only = (flags & IMGUI_HOVERED_RECT_ONLY) != 0 ?
                IMGUI_TRUE : IMGUI_FALSE;
    point_in_rect = ctx->input.mouse_x >= ctx->last_item_rect.x1 &&
                    ctx->input.mouse_x < ctx->last_item_rect.x2 &&
                    ctx->input.mouse_y >= ctx->last_item_rect.y1 &&
                    ctx->input.mouse_y < ctx->last_item_rect.y2;
    if (!point_in_rect) return IMGUI_FALSE;
    if (ctx->last_item_disabled &&
        (flags & IMGUI_HOVERED_ALLOW_WHEN_DISABLED) == 0) {
        return IMGUI_FALSE;
    }
    /* Dear ImGui's AllowWhenOverlappedByItem option deliberately bypasses
       the topmost-item ownership test while retaining the item's rectangle,
       disabled-state, popup, and active-item blocking rules.  RECT_ONLY is
       the stronger diagnostic form of the same geometry test. */
    hovered = (rect_only ||
               (flags & IMGUI_HOVERED_ALLOW_WHEN_OVERLAPPED_BY_ITEM) != 0) ?
              IMGUI_TRUE : ctx->last_item_hovered;
    if (!rect_only &&
        (flags & IMGUI_HOVERED_ALLOW_WHEN_DISABLED) != 0 &&
        ctx->last_item_disabled) {
        hovered = ctx->input.focused &&
                  (ctx->window_flags & IMGUI_WINDOW_NO_MOUSE_INPUTS) == 0 ?
                  IMGUI_TRUE : IMGUI_FALSE;
    }
    if (!hovered) return IMGUI_FALSE;
    if (ctx->popup_open && ctx->current_popup_id == 0 &&
        ctx->popup_rect_valid &&
        ctx->input.mouse_x >= ctx->popup_rect.x1 &&
        ctx->input.mouse_x < ctx->popup_rect.x2 &&
        ctx->input.mouse_y >= ctx->popup_rect.y1 &&
        ctx->input.mouse_y < ctx->popup_rect.y2 &&
        !ctx->popup_modal &&
        (flags & IMGUI_HOVERED_ALLOW_WHEN_BLOCKED_BY_POPUP) == 0) {
        return IMGUI_FALSE;
    }
    if (ctx->popup_open && ctx->current_popup_id == 0 &&
        ctx->popup_modal &&
        (flags & IMGUI_HOVERED_ALLOW_WHEN_BLOCKED_BY_POPUP) == 0) {
        return IMGUI_FALSE;
    }
    if (ctx->active_item_valid && ctx->active_item_id != ctx->last_item_id &&
        (flags & IMGUI_HOVERED_ALLOW_WHEN_BLOCKED_BY_ACTIVE_ITEM) == 0) {
        return IMGUI_FALSE;
    }
    return hovered;
}

imgui_bool imgui_is_item_visible(imgui_context *ctx)
{
    if (ctx == NULL || !ctx->window_active) return IMGUI_FALSE;
    return ctx->last_item_rect.x2 > ctx->last_item_clip_rect.x1 &&
           ctx->last_item_rect.x1 < ctx->last_item_clip_rect.x2 &&
           ctx->last_item_rect.y2 > ctx->last_item_clip_rect.y1 &&
           ctx->last_item_rect.y1 < ctx->last_item_clip_rect.y2 ?
           IMGUI_TRUE : IMGUI_FALSE;
}

imgui_bool imgui_is_item_active(imgui_context *ctx)
{
    return ctx != NULL ? ctx->last_item_active : IMGUI_FALSE;
}

imgui_bool imgui_is_item_focused(imgui_context *ctx)
{
    return ctx != NULL && ctx->last_item_id != 0 &&
           ctx->focused_item_valid &&
           ctx->focused_item_id == ctx->last_item_id ?
           IMGUI_TRUE : IMGUI_FALSE;
}

imgui_bool imgui_is_item_activated(imgui_context *ctx)
{
    return ctx != NULL ? ctx->last_item_activated : IMGUI_FALSE;
}

imgui_bool imgui_is_item_deactivated(imgui_context *ctx)
{
    return ctx != NULL ? ctx->last_item_deactivated : IMGUI_FALSE;
}

imgui_bool imgui_is_item_deactivated_after_edit(imgui_context *ctx)
{
    return ctx != NULL ? ctx->last_item_deactivated_after_edit : IMGUI_FALSE;
}

imgui_bool imgui_is_item_edited(imgui_context *ctx)
{
    return ctx != NULL ? ctx->last_item_edited : IMGUI_FALSE;
}

imgui_bool imgui_is_item_toggled_open(imgui_context *ctx)
{
    return ctx != NULL ? ctx->last_item_toggled_open : IMGUI_FALSE;
}

imgui_bool imgui_is_item_toggled_selection(imgui_context *ctx)
{
    return ctx != NULL ? ctx->last_item_toggled_selection : IMGUI_FALSE;
}

imgui_bool imgui_is_item_clicked(imgui_context *ctx,
                                 imgui_mouse_button button)
{
    if (ctx == NULL || button < 0 || button >= IMGUI_MOUSE_BUTTON_COUNT) {
        return IMGUI_FALSE;
    }
    return ctx->last_item_hovered && ctx->mouse_clicked[(int)button];
}

imgui_bool imgui_is_mouse_hovering_rect(const imgui_context *ctx,
                                        imgui_vec2 minimum,
                                        imgui_vec2 maximum,
                                        imgui_bool clip)
{
    imgui_rect rect;
    if (ctx == NULL || !imgui_vec2_is_finite(minimum) ||
        !imgui_vec2_is_finite(maximum) || maximum.x < minimum.x ||
        maximum.y < minimum.y) {
        return IMGUI_FALSE;
    }
    rect.x1 = minimum.x;
    rect.y1 = minimum.y;
    rect.x2 = maximum.x;
    rect.y2 = maximum.y;
    if (clip) {
        if (rect.x1 < ctx->clip_rect.x1) rect.x1 = ctx->clip_rect.x1;
        if (rect.y1 < ctx->clip_rect.y1) rect.y1 = ctx->clip_rect.y1;
        if (rect.x2 > ctx->clip_rect.x2) rect.x2 = ctx->clip_rect.x2;
        if (rect.y2 > ctx->clip_rect.y2) rect.y2 = ctx->clip_rect.y2;
    }
    return ctx->input.mouse_x >= rect.x1 && ctx->input.mouse_x < rect.x2 &&
           ctx->input.mouse_y >= rect.y1 && ctx->input.mouse_y < rect.y2;
}

imgui_bool imgui_is_rect_visible(const imgui_context *ctx,
                                 imgui_vec2 minimum,
                                 imgui_vec2 maximum)
{
    if (ctx == NULL || !imgui_vec2_is_finite(minimum) ||
        !imgui_vec2_is_finite(maximum) || maximum.x <= minimum.x ||
        maximum.y <= minimum.y) {
        return IMGUI_FALSE;
    }
    return maximum.x > ctx->clip_rect.x1 && minimum.x < ctx->clip_rect.x2 &&
           maximum.y > ctx->clip_rect.y1 && minimum.y < ctx->clip_rect.y2;
}

imgui_bool imgui_is_any_item_hovered(const imgui_context *ctx)
{
    return ctx != NULL ? ctx->any_item_hovered : IMGUI_FALSE;
}

imgui_bool imgui_is_any_item_active(const imgui_context *ctx)
{
    return ctx != NULL ? ctx->any_item_active : IMGUI_FALSE;
}

imgui_bool imgui_is_any_item_focused(const imgui_context *ctx)
{
    return ctx != NULL ? ctx->any_item_focused : IMGUI_FALSE;
}

imgui_id imgui_get_item_id(const imgui_context *ctx)
{
    return ctx != NULL ? ctx->last_item_id : 0;
}

imgui_rect imgui_get_item_rect(imgui_context *ctx)
{
    imgui_rect rect;
    if (ctx != NULL) {
        return ctx->last_item_rect;
    }
    memset(&rect, 0, sizeof(rect));
    return rect;
}

imgui_vec2 imgui_get_item_rect_min(const imgui_context *ctx)
{
    return ctx != NULL ? imgui_make_vec2(ctx->last_item_rect.x1,
                                         ctx->last_item_rect.y1) :
                         imgui_make_vec2(0.0f, 0.0f);
}

imgui_vec2 imgui_get_item_rect_max(const imgui_context *ctx)
{
    return ctx != NULL ? imgui_make_vec2(ctx->last_item_rect.x2,
                                         ctx->last_item_rect.y2) :
                         imgui_make_vec2(0.0f, 0.0f);
}

imgui_vec2 imgui_get_item_rect_size(const imgui_context *ctx)
{
    return ctx != NULL ? imgui_make_vec2(
                             ctx->last_item_rect.x2 -
                             ctx->last_item_rect.x1,
                             ctx->last_item_rect.y2 -
                             ctx->last_item_rect.y1) :
                         imgui_make_vec2(0.0f, 0.0f);
}

imgui_bool imgui_is_window_hovered(imgui_context *ctx, imgui_flags flags)
{
    if (ctx != NULL && (flags & IMGUI_HOVERED_ANY_WINDOW) != 0) {
        if (ctx->popup_open && ctx->popup_modal &&
            ctx->current_popup_id == 0) {
            return IMGUI_FALSE;
        }
        if (ctx->popup_open && ctx->popup_rect_valid &&
            ctx->input.mouse_x >= ctx->popup_rect.x1 &&
            ctx->input.mouse_x < ctx->popup_rect.x2 &&
            ctx->input.mouse_y >= ctx->popup_rect.y1 &&
            ctx->input.mouse_y < ctx->popup_rect.y2) {
            return IMGUI_TRUE;
        }
        return ctx->frame_any_window_hovered;
    }
    if (ctx == NULL || !ctx->window_active ||
        (ctx->window_flags & IMGUI_WINDOW_NO_MOUSE_INPUTS) != 0) {
        return IMGUI_FALSE;
    }
    if (ctx->popup_open && ctx->current_popup_id == 0 &&
        ctx->popup_rect_valid &&
        ctx->input.mouse_x >= ctx->popup_rect.x1 &&
        ctx->input.mouse_x < ctx->popup_rect.x2 &&
        ctx->input.mouse_y >= ctx->popup_rect.y1 &&
        ctx->input.mouse_y < ctx->popup_rect.y2 &&
        (flags & IMGUI_HOVERED_ALLOW_WHEN_BLOCKED_BY_POPUP) == 0) {
        return IMGUI_FALSE;
    }
    if (ctx->popup_open && ctx->current_popup_id == 0 &&
        ctx->popup_modal &&
        (flags & IMGUI_HOVERED_ALLOW_WHEN_BLOCKED_BY_POPUP) == 0) {
        return IMGUI_FALSE;
    }
    if (ctx->current_window_index >= 0 &&
        !imgui_window_is_mouse_topmost(ctx, ctx->current_window_index)) {
        return IMGUI_FALSE;
    }
    return ctx->input.mouse_x >= ctx->window_origin.x &&
           ctx->input.mouse_x < ctx->window_origin.x + ctx->window_size.x &&
           ctx->input.mouse_y >= ctx->window_origin.y &&
           ctx->input.mouse_y < ctx->window_origin.y + ctx->window_size.y;
}

imgui_bool imgui_is_window_focused(imgui_context *ctx, imgui_flags flags)
{
    if (ctx != NULL && (flags & IMGUI_FOCUSED_ANY_WINDOW) != 0) {
        return ctx->frame_any_window_focused;
    }
    return ctx != NULL && ctx->window_active && ctx->window_focused;
}

imgui_draw_list *imgui_get_window_draw_list(imgui_context *ctx)
{
    if (!imgui_internal_require_building(ctx,
                                         "get draw list outside frame")) {
        return NULL;
    }
    return &ctx->default_draw_list;
}

static imgui_bool imgui_renderer_desc_normalize(
    const imgui_renderer_desc *source,
    imgui_renderer_desc *destination)
{
    size_t copy_size;
    if (source == NULL || destination == NULL ||
        source->struct_size < sizeof(source->struct_size)) {
        return IMGUI_FALSE;
    }
    imgui_renderer_desc_init(destination);
    copy_size = source->struct_size < sizeof(*destination) ?
        source->struct_size : sizeof(*destination);
    memcpy(destination, source, copy_size);
    destination->struct_size = sizeof(*destination);
    return destination->maximum_texture_width >= 0 &&
           destination->maximum_texture_height >= 0;
}

static imgui_bool imgui_platform_desc_normalize(
    const imgui_platform_desc *source,
    imgui_platform_desc *destination)
{
    size_t copy_size;
    if (source == NULL || destination == NULL ||
        source->struct_size < sizeof(source->struct_size)) {
        return IMGUI_FALSE;
    }
    imgui_platform_desc_init(destination);
    copy_size = source->struct_size < sizeof(*destination) ?
        source->struct_size : sizeof(*destination);
    memcpy(destination, source, copy_size);
    destination->struct_size = sizeof(*destination);
    return IMGUI_TRUE;
}

imgui_result imgui_renderer_configure(imgui_context *ctx,
                                      const imgui_renderer_desc *desc)
{
    imgui_renderer_desc local_desc;
    if (ctx == NULL || desc == NULL ||
        !imgui_renderer_desc_normalize(desc, &local_desc)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (ctx->frame_state == IMGUI_INTERNAL_FRAME_BUILDING) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    ctx->renderer = local_desc;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_platform_configure(imgui_context *ctx,
                                      const imgui_platform_desc *desc)
{
    int viewport_index;
    int rollback_index;
    imgui_platform_desc previous_platform;
    imgui_bool created_now[IMGUI_INTERNAL_VIEWPORT_CAPACITY];
    imgui_bool previously_created[IMGUI_INTERNAL_VIEWPORT_CAPACITY];
    imgui_platform_desc local_desc;
    if (ctx == NULL || desc == NULL ||
        !imgui_platform_desc_normalize(desc, &local_desc)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (ctx->frame_state == IMGUI_INTERNAL_FRAME_BUILDING) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    previous_platform = ctx->platform;
    memset(created_now, 0, sizeof(created_now));
    memset(previously_created, 0, sizeof(previously_created));
    for (viewport_index = 1; viewport_index < ctx->viewport_count;
         ++viewport_index) {
        if (ctx->viewport_configs[viewport_index].configured &&
            ctx->viewport_configs[viewport_index].platform_created) {
            previously_created[viewport_index] = IMGUI_TRUE;
        }
    }
    for (viewport_index = 1; viewport_index < ctx->viewport_count;
         ++viewport_index) {
        if (!previously_created[viewport_index]) continue;
        if (previous_platform.viewport_destroy == NULL) {
            return IMGUI_RESULT_INVALID_STATE;
        }
        previous_platform.viewport_destroy(
            ctx->viewport_configs[viewport_index].desc.viewport_id,
            previous_platform.user_data);
        ctx->viewport_configs[viewport_index].platform_created = IMGUI_FALSE;
    }
    ctx->platform = local_desc;
    if (ctx->platform.viewport_create != NULL) {
        for (viewport_index = 1; viewport_index < ctx->viewport_count;
             ++viewport_index) {
            if (ctx->viewport_configs[viewport_index].configured &&
                !ctx->viewport_configs[viewport_index].platform_created &&
                ctx->platform.viewport_create(
                    &ctx->viewport_configs[viewport_index].desc,
                    ctx->platform.user_data) != IMGUI_RESULT_OK) {
                if (ctx->platform.viewport_destroy != NULL) {
                    for (rollback_index = 1;
                         rollback_index < ctx->viewport_count;
                         ++rollback_index) {
                        if (created_now[rollback_index]) {
                            ctx->platform.viewport_destroy(
                                ctx->viewport_configs[rollback_index].desc.viewport_id,
                                ctx->platform.user_data);
                        }
                        ctx->viewport_configs[rollback_index].platform_created =
                            IMGUI_FALSE;
                    }
                }
                ctx->platform = previous_platform;
                if (previous_platform.viewport_create != NULL) {
                    for (rollback_index = 1;
                         rollback_index < ctx->viewport_count;
                         ++rollback_index) {
                        if (previously_created[rollback_index] &&
                            previous_platform.viewport_create(
                                &ctx->viewport_configs[rollback_index].desc,
                                previous_platform.user_data) ==
                            IMGUI_RESULT_OK) {
                            ctx->viewport_configs[rollback_index].platform_created =
                                IMGUI_TRUE;
                        }
                    }
                }
                return IMGUI_RESULT_INVALID_STATE;
            } else if (ctx->viewport_configs[viewport_index].configured) {
                ctx->viewport_configs[viewport_index].platform_created =
                    IMGUI_TRUE;
                created_now[viewport_index] = IMGUI_TRUE;
            }
        }
    }
    return IMGUI_RESULT_OK;
}

imgui_platform_capabilities imgui_platform_get_capabilities(
    const imgui_context *ctx)
{
    return ctx != NULL ? ctx->platform.capabilities : 0;
}

const imgui_platform_output *imgui_get_platform_output(
    const imgui_context *ctx)
{
    return ctx != NULL ? &ctx->platform_output : NULL;
}

const char *imgui_clipboard_get(imgui_context *ctx)
{
    if (ctx == NULL ||
        (ctx->platform.capabilities & IMGUI_PLATFORM_CAP_CLIPBOARD) == 0 ||
        ctx->platform.clipboard_get == NULL) {
        imgui_internal_report(ctx, IMGUI_ERROR_UNSUPPORTED,
                              "clipboard get is unavailable");
        return NULL;
    }
    return ctx->platform.clipboard_get(ctx->platform.user_data);
}

imgui_result imgui_clipboard_set(imgui_context *ctx, const char *text)
{
    if (ctx == NULL || text == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    if ((ctx->platform.capabilities & IMGUI_PLATFORM_CAP_CLIPBOARD) == 0 ||
        ctx->platform.clipboard_set == NULL) {
        imgui_internal_report(ctx, IMGUI_ERROR_UNSUPPORTED,
                              "clipboard set is unavailable");
        return IMGUI_RESULT_UNSUPPORTED;
    }
    ctx->platform.clipboard_set(text, ctx->platform.user_data);
    return IMGUI_RESULT_OK;
}

imgui_renderer_capabilities imgui_renderer_get_capabilities(
    const imgui_context *ctx)
{
    return ctx != NULL ? ctx->renderer.capabilities : 0;
}

imgui_bool imgui_renderer_has_capability(
    const imgui_context *ctx,
    imgui_renderer_capabilities capability)
{
    if (ctx == NULL) {
        return IMGUI_FALSE;
    }
    return (ctx->renderer.capabilities & capability) == capability
        ? IMGUI_TRUE
        : IMGUI_FALSE;
}

imgui_result imgui_draw_list_add_text(imgui_context *ctx,
                                      imgui_draw_list *draw_list,
                                      imgui_vec2 position,
                                      const char *text,
                                      imgui_u32 color)
{
    size_t length;
    imgui_u32 old_color;
    if (ctx == NULL || draw_list == NULL || !imgui_vec2_is_finite(position) ||
        draw_list->owner != ctx ||
        text == NULL || ctx->font == NULL || ctx->font_texture == NULL ||
        !imgui_internal_require_building(ctx,
                                          "draw text outside frame")) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    length = strlen(text);
    old_color = ctx->style.color_text;
    ctx->style.color_text = color;
    imgui_text_draw_font(ctx, text, text + length, position);
    ctx->style.color_text = old_color;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_draw_list_add_rect(imgui_context *ctx,
                                      imgui_draw_list *draw_list,
                                      imgui_rect rect,
                                      imgui_u32 color,
                                      imgui_texture *texture)
{
    imgui_result result;
    if (ctx == NULL || draw_list == NULL || !imgui_rect_is_finite(rect)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                         "rectangle outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (texture != NULL &&
        (texture->owner != ctx || !texture->alive)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (texture != NULL &&
        !imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    result = imgui_mesh_add_rect(ctx, rect, color);
    if (result == IMGUI_RESULT_OK && texture != NULL) {
        ctx->commands[ctx->command_count - 1].data.draw_indexed.texture =
            texture;
    }
    return result;
}

imgui_result imgui_draw_list_add_rect_rounded(
    imgui_context *ctx, imgui_draw_list *draw_list, imgui_rect rect,
    float radius, imgui_u32 color, int corner_segments,
    imgui_texture *texture)
{
    imgui_render_command command;
    imgui_rect clipped;
    imgui_u32 vertex_offset;
    imgui_u32 index_offset;
    imgui_u32 perimeter_count;
    imgui_u32 index;
    int corner;
    int segment;
    double angle;
    float center_x;
    float center_y;
    float width;
    float height;
    if (ctx == NULL || draw_list == NULL || !imgui_rect_is_finite(rect) ||
        !imgui_float_is_finite(radius) || radius < 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                          "rounded rectangle outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (texture != NULL &&
        (texture->owner != ctx || !texture->alive)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (texture != NULL &&
        !imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    width = rect.x2 - rect.x1;
    height = rect.y2 - rect.y1;
    if (width <= 0.0f || height <= 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (radius > width * 0.5f) radius = width * 0.5f;
    if (radius > height * 0.5f) radius = height * 0.5f;
    if (radius == 0.0f) {
        return imgui_draw_list_add_rect(ctx, draw_list, rect, color, texture);
    }
    if (corner_segments < 1) corner_segments = 1;
    perimeter_count = (imgui_u32)corner_segments * 4U;
#if !defined(IMGUI_RENDER_INDEX_32)
    if (ctx->vertex_count + perimeter_count + 1U > 65535U) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
#endif
    if (imgui_mesh_reserve(ctx, perimeter_count + 1U,
                           perimeter_count * 3U) != IMGUI_RESULT_OK) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    vertex_offset = ctx->vertex_count;
    index_offset = ctx->index_count;
    center_x = (rect.x1 + rect.x2) * 0.5f;
    center_y = (rect.y1 + rect.y2) * 0.5f;
    ctx->vertices[vertex_offset].position = imgui_make_vec2(center_x, center_y);
    ctx->vertices[vertex_offset].uv = imgui_make_vec2(0.5f, 0.5f);
    ctx->vertices[vertex_offset].color = color;
    index = 0;
    for (corner = 0; corner < 4; ++corner) {
        for (segment = 0; segment < corner_segments; ++segment) {
            angle = (-1.5707963267948966 +
                     (double)corner * 1.5707963267948966) +
                    1.5707963267948966 * (double)segment /
                    (double)corner_segments;
            if (corner == 0) {
                center_x = rect.x2 - radius;
                center_y = rect.y1 + radius;
            } else if (corner == 1) {
                center_x = rect.x2 - radius;
                center_y = rect.y2 - radius;
            } else if (corner == 2) {
                center_x = rect.x1 + radius;
                center_y = rect.y2 - radius;
            } else {
                center_x = rect.x1 + radius;
                center_y = rect.y1 + radius;
            }
            ctx->vertices[vertex_offset + 1U + index].position =
                imgui_make_vec2(center_x + (float)cos(angle) * radius,
                                center_y + (float)sin(angle) * radius);
            ctx->vertices[vertex_offset + 1U + index].uv = imgui_make_vec2(
                (ctx->vertices[vertex_offset + 1U + index].position.x - rect.x1) /
                    width,
                (ctx->vertices[vertex_offset + 1U + index].position.y - rect.y1) /
                    height);
            ctx->vertices[vertex_offset + 1U + index].color = color;
            ++index;
        }
    }
    for (index = 0; index < perimeter_count; ++index) {
        ctx->indices[index_offset + index * 3U] =
            (imgui_render_index)0U;
        ctx->indices[index_offset + index * 3U + 1U] =
            (imgui_render_index)(1U + index);
        ctx->indices[index_offset + index * 3U + 2U] =
            (imgui_render_index)(1U + ((index + 1U) % perimeter_count));
    }
    ctx->vertex_count += perimeter_count + 1U;
    ctx->index_count += perimeter_count * 3U;
    clipped = rect;
    if (clipped.x1 < ctx->clip_rect.x1) clipped.x1 = ctx->clip_rect.x1;
    if (clipped.y1 < ctx->clip_rect.y1) clipped.y1 = ctx->clip_rect.y1;
    if (clipped.x2 > ctx->clip_rect.x2) clipped.x2 = ctx->clip_rect.x2;
    if (clipped.y2 > ctx->clip_rect.y2) clipped.y2 = ctx->clip_rect.y2;
    memset(&command, 0, sizeof(command));
    command.type = IMGUI_RENDER_COMMAND_DRAW_INDEXED;
    command.data.draw_indexed.clip_rect = clipped;
    command.data.draw_indexed.index_offset = index_offset;
    command.data.draw_indexed.index_count = perimeter_count * 3U;
    command.data.draw_indexed.vertex_offset = vertex_offset;
    command.data.draw_indexed.texture = texture;
    {
        imgui_result result = imgui_render_command_append(ctx, &command, NULL);
        if (result != IMGUI_RESULT_OK) {
            ctx->vertex_count = vertex_offset;
            ctx->index_count = index_offset;
        }
        return result;
    }
}

static imgui_result imgui_add_frame_surface(imgui_context *ctx,
                                            imgui_rect rect,
                                            imgui_u32 color)
{
    if (ctx == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (ctx->style.frame_rounding > 0.0f) {
        return imgui_draw_list_add_rect_rounded(
            ctx, &ctx->default_draw_list, rect,
            ctx->style.frame_rounding, color, 4, NULL);
    }
    return imgui_mesh_add_rect(ctx, rect, color);
}

imgui_result imgui_draw_list_add_image(imgui_context *ctx,
                                       imgui_draw_list *draw_list,
                                       imgui_texture *texture,
                                       imgui_rect rect,
                                       imgui_vec2 uv_min,
                                       imgui_vec2 uv_max,
                                       imgui_u32 color)
{
    imgui_result result;
    imgui_u32 vertex_offset;
    if (ctx == NULL || draw_list == NULL || texture == NULL ||
        !imgui_rect_is_finite(rect) || !imgui_vec2_is_finite(uv_min) ||
        !imgui_vec2_is_finite(uv_max)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                          "image outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (!imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    if (texture->owner != ctx || !texture->alive) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    result = imgui_mesh_add_rect(ctx, rect, color);
    if (result != IMGUI_RESULT_OK) return result;
    vertex_offset = ctx->vertex_count - 4U;
    ctx->vertices[vertex_offset + 0].uv = uv_min;
    ctx->vertices[vertex_offset + 1].uv = imgui_make_vec2(uv_max.x,
                                                           uv_min.y);
    ctx->vertices[vertex_offset + 2].uv = uv_max;
    ctx->vertices[vertex_offset + 3].uv = imgui_make_vec2(uv_min.x,
                                                           uv_max.y);
    ctx->commands[ctx->command_count - 1].data.draw_indexed.texture = texture;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_draw_list_add_image_quad(
    imgui_context *ctx, imgui_draw_list *draw_list, imgui_texture *texture,
    imgui_vec2 a, imgui_vec2 b, imgui_vec2 c, imgui_vec2 d,
    imgui_vec2 uv_a, imgui_vec2 uv_b, imgui_vec2 uv_c, imgui_vec2 uv_d,
    imgui_u32 color)
{
    imgui_render_command command;
    imgui_u32 vertex_offset;
    imgui_u32 index_offset;
    if (ctx == NULL || draw_list == NULL || texture == NULL ||
        !imgui_vec2_is_finite(a) || !imgui_vec2_is_finite(b) ||
        !imgui_vec2_is_finite(c) || !imgui_vec2_is_finite(d) ||
        !imgui_vec2_is_finite(uv_a) || !imgui_vec2_is_finite(uv_b) ||
        !imgui_vec2_is_finite(uv_c) || !imgui_vec2_is_finite(uv_d)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                         "image quad outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (!imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    if (texture->owner != ctx || !texture->alive) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (imgui_mesh_reserve(ctx, 4U, 6U) != IMGUI_RESULT_OK) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    vertex_offset = ctx->vertex_count;
    index_offset = ctx->index_count;
    ctx->vertices[vertex_offset + 0U].position = a;
    ctx->vertices[vertex_offset + 1U].position = b;
    ctx->vertices[vertex_offset + 2U].position = c;
    ctx->vertices[vertex_offset + 3U].position = d;
    ctx->vertices[vertex_offset + 0U].uv = uv_a;
    ctx->vertices[vertex_offset + 1U].uv = uv_b;
    ctx->vertices[vertex_offset + 2U].uv = uv_c;
    ctx->vertices[vertex_offset + 3U].uv = uv_d;
    ctx->vertices[vertex_offset + 0U].color = color;
    ctx->vertices[vertex_offset + 1U].color = color;
    ctx->vertices[vertex_offset + 2U].color = color;
    ctx->vertices[vertex_offset + 3U].color = color;
    ctx->indices[index_offset + 0U] = 0U;
    ctx->indices[index_offset + 1U] = 1U;
    ctx->indices[index_offset + 2U] = 2U;
    ctx->indices[index_offset + 3U] = 0U;
    ctx->indices[index_offset + 4U] = 2U;
    ctx->indices[index_offset + 5U] = 3U;
    ctx->vertex_count += 4U;
    ctx->index_count += 6U;
    memset(&command, 0, sizeof(command));
    command.type = IMGUI_RENDER_COMMAND_DRAW_INDEXED;
    command.data.draw_indexed.clip_rect = ctx->clip_rect;
    command.data.draw_indexed.index_offset = index_offset;
    command.data.draw_indexed.index_count = 6U;
    command.data.draw_indexed.vertex_offset = vertex_offset;
    command.data.draw_indexed.texture = texture;
    if (imgui_render_command_append(ctx, &command, NULL) !=
            IMGUI_RESULT_OK) {
        ctx->vertex_count = vertex_offset;
        ctx->index_count = index_offset;
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    return IMGUI_RESULT_OK;
}

imgui_result imgui_draw_list_add_line(imgui_context *ctx,
                                      imgui_draw_list *draw_list,
                                      imgui_vec2 start,
                                      imgui_vec2 end,
                                      imgui_u32 color,
                                      float thickness)
{
    imgui_render_command command;
    imgui_rect clipped;
    imgui_vec2 delta;
    imgui_vec2 normal;
    imgui_u32 vertex_offset;
    imgui_u32 index_offset;
    float length;
    float half;
    if (ctx == NULL || draw_list == NULL ||
        !imgui_vec2_is_finite(start) || !imgui_vec2_is_finite(end) ||
        !imgui_float_is_finite(thickness) || thickness <= 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                         "line outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    delta = imgui_make_vec2(end.x - start.x, end.y - start.y);
    length = (float)sqrt((double)(delta.x * delta.x + delta.y * delta.y));
    if (length <= 0.0f) return IMGUI_RESULT_INVALID_ARGUMENT;
    half = thickness * 0.5f;
    normal = imgui_make_vec2(-delta.y / length * half,
                             delta.x / length * half);
    if (imgui_mesh_reserve(ctx, 4U, 6U) != IMGUI_RESULT_OK) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    vertex_offset = ctx->vertex_count;
    index_offset = ctx->index_count;
    ctx->vertices[vertex_offset + 0].position = imgui_make_vec2(
        start.x + normal.x, start.y + normal.y);
    ctx->vertices[vertex_offset + 1].position = imgui_make_vec2(
        end.x + normal.x, end.y + normal.y);
    ctx->vertices[vertex_offset + 2].position = imgui_make_vec2(
        end.x - normal.x, end.y - normal.y);
    ctx->vertices[vertex_offset + 3].position = imgui_make_vec2(
        start.x - normal.x, start.y - normal.y);
    ctx->vertices[vertex_offset + 0].uv = imgui_make_vec2(0.0f, 0.0f);
    ctx->vertices[vertex_offset + 1].uv = imgui_make_vec2(1.0f, 0.0f);
    ctx->vertices[vertex_offset + 2].uv = imgui_make_vec2(1.0f, 1.0f);
    ctx->vertices[vertex_offset + 3].uv = imgui_make_vec2(0.0f, 1.0f);
    ctx->vertices[vertex_offset + 0].color = color;
    ctx->vertices[vertex_offset + 1].color = color;
    ctx->vertices[vertex_offset + 2].color = color;
    ctx->vertices[vertex_offset + 3].color = color;
    ctx->indices[index_offset + 0] = (imgui_render_index)0U;
    ctx->indices[index_offset + 1] = (imgui_render_index)1U;
    ctx->indices[index_offset + 2] = (imgui_render_index)2U;
    ctx->indices[index_offset + 3] = (imgui_render_index)0U;
    ctx->indices[index_offset + 4] = (imgui_render_index)2U;
    ctx->indices[index_offset + 5] = (imgui_render_index)3U;
    ctx->vertex_count += 4U;
    ctx->index_count += 6U;
    clipped.x1 = start.x < end.x ? start.x : end.x;
    clipped.y1 = start.y < end.y ? start.y : end.y;
    clipped.x2 = start.x > end.x ? start.x : end.x;
    clipped.y2 = start.y > end.y ? start.y : end.y;
    clipped.x1 -= half;
    clipped.y1 -= half;
    clipped.x2 += half;
    clipped.y2 += half;
    if (clipped.x1 < ctx->clip_rect.x1) clipped.x1 = ctx->clip_rect.x1;
    if (clipped.y1 < ctx->clip_rect.y1) clipped.y1 = ctx->clip_rect.y1;
    if (clipped.x2 > ctx->clip_rect.x2) clipped.x2 = ctx->clip_rect.x2;
    if (clipped.y2 > ctx->clip_rect.y2) clipped.y2 = ctx->clip_rect.y2;
    memset(&command, 0, sizeof(command));
    command.type = IMGUI_RENDER_COMMAND_DRAW_INDEXED;
    command.data.draw_indexed.clip_rect = clipped;
    command.data.draw_indexed.index_offset = index_offset;
    command.data.draw_indexed.index_count = 6U;
    command.data.draw_indexed.vertex_offset = vertex_offset;
    {
        imgui_result result = imgui_render_command_append(ctx, &command, NULL);
        if (result != IMGUI_RESULT_OK) {
            ctx->vertex_count = vertex_offset;
            ctx->index_count = index_offset;
        }
        return result;
    }
}

imgui_result imgui_draw_list_add_triangle_filled(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_vec2 a, imgui_vec2 b, imgui_vec2 c, imgui_u32 color)
{
    if (ctx == NULL || draw_list == NULL ||
        !imgui_vec2_is_finite(a) || !imgui_vec2_is_finite(b) ||
        !imgui_vec2_is_finite(c)) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                          "filled triangle outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    return imgui_mesh_add_triangle(ctx, a, b, c, color);
}

imgui_result imgui_draw_list_add_triangle(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_vec2 a, imgui_vec2 b, imgui_vec2 c,
    imgui_u32 color, float thickness)
{
    imgui_vec2 points[3];
    if (ctx == NULL || draw_list == NULL ||
        !imgui_vec2_is_finite(a) || !imgui_vec2_is_finite(b) ||
        !imgui_vec2_is_finite(c) || !imgui_float_is_finite(thickness) ||
        thickness <= 0.0f) return IMGUI_RESULT_INVALID_ARGUMENT;
    points[0] = a; points[1] = b; points[2] = c;
    return imgui_draw_list_add_polyline(ctx, draw_list, points, 3,
                                        color, thickness, IMGUI_TRUE);
}

imgui_result imgui_draw_list_add_quad_filled(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_vec2 a, imgui_vec2 b, imgui_vec2 c, imgui_vec2 d,
    imgui_u32 color)
{
    imgui_u32 vertex_count;
    imgui_u32 index_count;
    imgui_result result;
    if (ctx == NULL || draw_list == NULL ||
        !imgui_vec2_is_finite(a) || !imgui_vec2_is_finite(b) ||
        !imgui_vec2_is_finite(c) || !imgui_vec2_is_finite(d)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                          "filled quad outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    vertex_count = ctx->vertex_count;
    index_count = ctx->index_count;
    result = imgui_mesh_add_triangle(ctx, a, b, c, color);
    if (result != IMGUI_RESULT_OK) return result;
    result = imgui_mesh_add_triangle(ctx, a, c, d, color);
    if (result != IMGUI_RESULT_OK) {
        ctx->vertex_count = vertex_count;
        ctx->index_count = index_count;
    }
    return result;
}

imgui_result imgui_draw_list_add_quad(
    imgui_context *ctx, imgui_draw_list *draw_list,
    imgui_vec2 a, imgui_vec2 b, imgui_vec2 c, imgui_vec2 d,
    imgui_u32 color, float thickness)
{
    imgui_vec2 points[4];
    if (ctx == NULL || draw_list == NULL ||
        !imgui_vec2_is_finite(a) || !imgui_vec2_is_finite(b) ||
        !imgui_vec2_is_finite(c) || !imgui_vec2_is_finite(d) ||
        !imgui_float_is_finite(thickness) || thickness <= 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    points[0] = a; points[1] = b; points[2] = c; points[3] = d;
    return imgui_draw_list_add_polyline(ctx, draw_list, points, 4,
                                        color, thickness, IMGUI_TRUE);
}

imgui_result imgui_draw_list_add_circle(imgui_context *ctx,
                                        imgui_draw_list *draw_list,
                                        imgui_vec2 center,
                                        float radius,
                                        imgui_u32 color,
                                        int segments)
{
    imgui_render_command command;
    imgui_rect clipped;
    imgui_u32 vertex_offset;
    imgui_u32 index_offset;
    imgui_u32 index;
    double angle;
    if (ctx == NULL || draw_list == NULL || !imgui_vec2_is_finite(center) ||
        !imgui_float_is_finite(radius) || radius <= 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                         "circle outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (segments < 3) segments = 12;
#if !defined(IMGUI_RENDER_INDEX_32)
    if (ctx->vertex_count + (imgui_u32)segments + 1U > 65535U) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
#endif
    if (imgui_mesh_reserve(ctx, (imgui_u32)segments + 1U,
                           (imgui_u32)segments * 3U) != IMGUI_RESULT_OK) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    vertex_offset = ctx->vertex_count;
    index_offset = ctx->index_count;
    ctx->vertices[vertex_offset].position = center;
    ctx->vertices[vertex_offset].uv = imgui_make_vec2(0.5f, 0.5f);
    ctx->vertices[vertex_offset].color = color;
    for (index = 0; index < (imgui_u32)segments; ++index) {
        angle = 6.28318530717958647692 * (double)index /
                (double)segments;
        ctx->vertices[vertex_offset + 1U + index].position = imgui_make_vec2(
            center.x + (float)cos(angle) * radius,
            center.y + (float)sin(angle) * radius);
        ctx->vertices[vertex_offset + 1U + index].uv = imgui_make_vec2(
            0.5f + (float)cos(angle) * 0.5f,
            0.5f + (float)sin(angle) * 0.5f);
        ctx->vertices[vertex_offset + 1U + index].color = color;
        ctx->indices[index_offset + index * 3U + 0U] =
            (imgui_render_index)0U;
        ctx->indices[index_offset + index * 3U + 1U] =
            (imgui_render_index)(1U + index);
        ctx->indices[index_offset + index * 3U + 2U] =
            (imgui_render_index)(1U +
                                 ((index + 1U) % (imgui_u32)segments));
    }
    ctx->vertex_count += (imgui_u32)segments + 1U;
    ctx->index_count += (imgui_u32)segments * 3U;
    clipped.x1 = center.x - radius;
    clipped.y1 = center.y - radius;
    clipped.x2 = center.x + radius;
    clipped.y2 = center.y + radius;
    if (clipped.x1 < ctx->clip_rect.x1) clipped.x1 = ctx->clip_rect.x1;
    if (clipped.y1 < ctx->clip_rect.y1) clipped.y1 = ctx->clip_rect.y1;
    if (clipped.x2 > ctx->clip_rect.x2) clipped.x2 = ctx->clip_rect.x2;
    if (clipped.y2 > ctx->clip_rect.y2) clipped.y2 = ctx->clip_rect.y2;
    memset(&command, 0, sizeof(command));
    command.type = IMGUI_RENDER_COMMAND_DRAW_INDEXED;
    command.data.draw_indexed.clip_rect = clipped;
    command.data.draw_indexed.index_offset = index_offset;
    command.data.draw_indexed.index_count = (imgui_u32)segments * 3U;
    command.data.draw_indexed.vertex_offset = vertex_offset;
    {
        imgui_result result = imgui_render_command_append(ctx, &command, NULL);
        if (result != IMGUI_RESULT_OK) {
            ctx->vertex_count = vertex_offset;
            ctx->index_count = index_offset;
        }
        return result;
    }
}

imgui_result imgui_draw_list_add_polyline(imgui_context *ctx,
                                          imgui_draw_list *draw_list,
                                          const imgui_vec2 *points,
                                          size_t point_count,
                                          imgui_u32 color,
                                          float thickness,
                                          imgui_bool closed)
{
    size_t index;
    imgui_result result;
    imgui_u32 old_vertex_count;
    imgui_u32 old_index_count;
    imgui_u32 old_command_count;
    if (ctx == NULL || draw_list == NULL || points == NULL ||
        point_count < 2 || !imgui_float_is_finite(thickness) ||
        thickness <= 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    for (index = 0; index < point_count; ++index) {
        if (!imgui_vec2_is_finite(points[index])) {
            return IMGUI_RESULT_INVALID_ARGUMENT;
        }
    }
    old_vertex_count = ctx->vertex_count;
    old_index_count = ctx->index_count;
    old_command_count = ctx->command_count;
    for (index = 0; index + 1 < point_count; ++index) {
        result = imgui_draw_list_add_line(ctx, draw_list, points[index],
                                          points[index + 1], color, thickness);
        if (result != IMGUI_RESULT_OK) {
            ctx->vertex_count = old_vertex_count;
            ctx->index_count = old_index_count;
            ctx->command_count = old_command_count;
            return result;
        }
    }
    if (closed) {
        result = imgui_draw_list_add_line(ctx, draw_list,
                                          points[point_count - 1], points[0],
                                          color, thickness);
        if (result != IMGUI_RESULT_OK) {
            ctx->vertex_count = old_vertex_count;
            ctx->index_count = old_index_count;
            ctx->command_count = old_command_count;
            return result;
        }
    }
    return IMGUI_RESULT_OK;
}

static imgui_result imgui_draw_list_path_validate(
    imgui_context *ctx, imgui_draw_list *draw_list)
{
    if (ctx == NULL || draw_list == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                          "path operation outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (!ctx->path_active) return IMGUI_RESULT_INVALID_STATE;
    return IMGUI_RESULT_OK;
}

static imgui_result imgui_draw_list_path_reserve(imgui_context *ctx,
                                                 int additional)
{
    size_t needed;
    size_t bytes;
    int capacity;
    imgui_vec2 *points;
    if (ctx == NULL || additional < 0 ||
        ctx->path_count > INT_MAX - additional) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    needed = (size_t)ctx->path_count + (size_t)additional;
    if (needed <= (size_t)ctx->path_capacity) return IMGUI_RESULT_OK;
    capacity = ctx->path_capacity > 0 ? ctx->path_capacity :
               IMGUI_INTERNAL_PATH_CAPACITY;
    while ((size_t)capacity < needed) {
        if (capacity > INT_MAX / 2) {
            capacity = INT_MAX;
            break;
        }
        capacity *= 2;
    }
    if ((size_t)capacity > (size_t)-1 / sizeof(*points)) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    bytes = (size_t)capacity * sizeof(*points);
    points = (imgui_vec2 *)imgui_internal_allocate(&ctx->allocator, bytes);
    if (points == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    if (ctx->path_count != 0) {
        memcpy(points, ctx->path_points,
               (size_t)ctx->path_count * sizeof(*points));
    }
    imgui_internal_release(&ctx->allocator, ctx->path_points);
    ctx->path_points = points;
    ctx->path_capacity = capacity;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_draw_list_path_begin(imgui_context *ctx,
                                        imgui_draw_list *draw_list)
{
    if (ctx == NULL || draw_list == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                          "path begin outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->path_active) return IMGUI_RESULT_INVALID_STATE;
    ctx->path_active = IMGUI_TRUE;
    ctx->path_count = 0;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_draw_list_path_line_to(imgui_context *ctx,
                                          imgui_draw_list *draw_list,
                                          imgui_vec2 point)
{
    imgui_result result;
    result = imgui_draw_list_path_validate(ctx, draw_list);
    if (result != IMGUI_RESULT_OK) return result;
    if (!imgui_vec2_is_finite(point)) return IMGUI_RESULT_INVALID_ARGUMENT;
    result = imgui_draw_list_path_reserve(ctx, 1);
    if (result != IMGUI_RESULT_OK) return result;
    ctx->path_points[ctx->path_count++] = point;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_draw_list_path_arc_to(
    imgui_context *ctx, imgui_draw_list *draw_list, imgui_vec2 center,
    float radius, float angle_min, float angle_max, int segments)
{
    imgui_result result;
    int index;
    float angle;
    float step;
    result = imgui_draw_list_path_validate(ctx, draw_list);
    if (result != IMGUI_RESULT_OK) return result;
    if (!imgui_vec2_is_finite(center) ||
        !imgui_float_is_finite(radius) ||
        !imgui_float_is_finite(angle_min) ||
        !imgui_float_is_finite(angle_max) || radius < 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (segments < 1) segments = 12;
    if (segments == INT_MAX) return IMGUI_RESULT_OUT_OF_MEMORY;
    result = imgui_draw_list_path_reserve(ctx, segments + 1);
    if (result != IMGUI_RESULT_OK) return result;
    step = (angle_max - angle_min) / (float)segments;
    for (index = 0; index <= segments; ++index) {
        angle = angle_min + step * (float)index;
        ctx->path_points[ctx->path_count++] = imgui_make_vec2(
            center.x + (float)cos((double)angle) * radius,
            center.y + (float)sin((double)angle) * radius);
    }
    return IMGUI_RESULT_OK;
}

static float imgui_path_cross(imgui_vec2 a, imgui_vec2 b, imgui_vec2 c)
{
    return (b.x - a.x) * (c.y - a.y) -
           (b.y - a.y) * (c.x - a.x);
}

static imgui_bool imgui_path_point_in_triangle(
    imgui_vec2 point, imgui_vec2 a, imgui_vec2 b, imgui_vec2 c,
    int winding)
{
    float ab;
    float bc;
    float ca;
    float epsilon;
    epsilon = 0.000001f;
    ab = imgui_path_cross(a, b, point) * (float)winding;
    bc = imgui_path_cross(b, c, point) * (float)winding;
    ca = imgui_path_cross(c, a, point) * (float)winding;
    return ab >= -epsilon && bc >= -epsilon && ca >= -epsilon;
}

imgui_result imgui_draw_list_path_fill(imgui_context *ctx,
                                       imgui_draw_list *draw_list,
                                       imgui_u32 color,
                                       imgui_texture *texture)
{
    imgui_result result;
    imgui_render_command command;
    imgui_u32 vertex_offset;
    imgui_u32 index_offset;
    imgui_u32 index;
    int *remaining;
    int remaining_count;
    int candidate;
    int previous;
    int current;
    int next;
    int check;
    int winding;
    int emitted;
    imgui_u32 old_vertex_count;
    imgui_u32 old_index_count;
    float area;
    imgui_bool ear;
    imgui_rect clip;
    if (texture != NULL && (ctx == NULL || texture->owner != ctx ||
                            !texture->alive)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    result = imgui_draw_list_path_validate(ctx, draw_list);
    if (result != IMGUI_RESULT_OK) return result;
    if (ctx->path_count < 3) return IMGUI_RESULT_INVALID_ARGUMENT;
    remaining = (int *)imgui_internal_allocate(
        &ctx->allocator, (size_t)ctx->path_count * sizeof(*remaining));
    if (remaining == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
#if !defined(IMGUI_RENDER_INDEX_32)
    if (ctx->vertex_count + (imgui_u32)ctx->path_count > 65535U) {
        imgui_internal_release(&ctx->allocator, remaining);
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
#endif
    if (imgui_mesh_reserve(ctx, (imgui_u32)ctx->path_count,
                           (imgui_u32)(ctx->path_count - 2) * 3U) !=
        IMGUI_RESULT_OK) {
        imgui_internal_release(&ctx->allocator, remaining);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    old_vertex_count = ctx->vertex_count;
    old_index_count = ctx->index_count;
    vertex_offset = ctx->vertex_count;
    index_offset = ctx->index_count;
    clip.x1 = ctx->path_points[0].x;
    clip.y1 = ctx->path_points[0].y;
    clip.x2 = clip.x1;
    clip.y2 = clip.y1;
    for (index = 0; index < (imgui_u32)ctx->path_count; ++index) {
        remaining[index] = (int)index;
        ctx->vertices[vertex_offset + index].position =
            ctx->path_points[index];
        ctx->vertices[vertex_offset + index].uv = imgui_make_vec2(0.0f, 0.0f);
        ctx->vertices[vertex_offset + index].color = color;
        if (ctx->path_points[index].x < clip.x1) clip.x1 =
            ctx->path_points[index].x;
        if (ctx->path_points[index].y < clip.y1) clip.y1 =
            ctx->path_points[index].y;
        if (ctx->path_points[index].x > clip.x2) clip.x2 =
            ctx->path_points[index].x;
        if (ctx->path_points[index].y > clip.y2) clip.y2 =
            ctx->path_points[index].y;
    }
    area = 0.0f;
    for (index = 0; index < (imgui_u32)ctx->path_count; ++index) {
        imgui_u32 next_index = (index + 1U) % (imgui_u32)ctx->path_count;
        area += ctx->path_points[index].x * ctx->path_points[next_index].y -
                ctx->path_points[next_index].x * ctx->path_points[index].y;
    }
    winding = area >= 0.0f ? 1 : -1;
    remaining_count = ctx->path_count;
    emitted = 0;
    while (remaining_count > 3 && emitted < ctx->path_count - 2) {
        ear = IMGUI_FALSE;
        candidate = -1;
        for (check = 0; check < remaining_count; ++check) {
            previous = remaining[(check + remaining_count - 1) %
                                 remaining_count];
            current = remaining[check];
            next = remaining[(check + 1) % remaining_count];
            if (imgui_path_cross(ctx->path_points[previous],
                                 ctx->path_points[current],
                                 ctx->path_points[next]) * (float)winding <=
                0.000001f) continue;
            ear = IMGUI_TRUE;
            for (index = 0; index < (imgui_u32)remaining_count; ++index) {
                int point_index = remaining[index];
                if (point_index == previous || point_index == current ||
                    point_index == next) continue;
                if (imgui_path_point_in_triangle(
                        ctx->path_points[point_index],
                        ctx->path_points[previous], ctx->path_points[current],
                        ctx->path_points[next], winding)) {
                    ear = IMGUI_FALSE;
                    break;
                }
            }
            if (ear) {
                candidate = check;
                break;
            }
        }
        if (!ear || candidate < 0) break;
        previous = remaining[(candidate + remaining_count - 1) %
                             remaining_count];
        current = remaining[candidate];
        next = remaining[(candidate + 1) % remaining_count];
        ctx->indices[index_offset + (imgui_u32)emitted * 3U] =
            (imgui_render_index)previous;
        ctx->indices[index_offset + (imgui_u32)emitted * 3U + 1U] =
            (imgui_render_index)current;
        ctx->indices[index_offset + (imgui_u32)emitted * 3U + 2U] =
            (imgui_render_index)next;
        ++emitted;
        memmove(&remaining[candidate], &remaining[candidate + 1],
                (size_t)(remaining_count - candidate - 1) *
                sizeof(*remaining));
        --remaining_count;
    }
    if (remaining_count == 3) {
        ctx->indices[index_offset + (imgui_u32)emitted * 3U] =
            (imgui_render_index)remaining[0];
        ctx->indices[index_offset + (imgui_u32)emitted * 3U + 1U] =
            (imgui_render_index)remaining[1];
        ctx->indices[index_offset + (imgui_u32)emitted * 3U + 2U] =
            (imgui_render_index)remaining[2];
        ++emitted;
    }
    if (emitted != ctx->path_count - 2) {
        for (index = 0; index < (imgui_u32)(ctx->path_count - 2); ++index) {
            ctx->indices[index_offset + index * 3U] =
                (imgui_render_index)0U;
            ctx->indices[index_offset + index * 3U + 1U] =
                (imgui_render_index)(index + 1U);
            ctx->indices[index_offset + index * 3U + 2U] =
                (imgui_render_index)(index + 2U);
        }
    }
    imgui_internal_release(&ctx->allocator, remaining);
    ctx->vertex_count += (imgui_u32)ctx->path_count;
    ctx->index_count += (imgui_u32)(ctx->path_count - 2) * 3U;
    if (clip.x1 < ctx->clip_rect.x1) clip.x1 = ctx->clip_rect.x1;
    if (clip.y1 < ctx->clip_rect.y1) clip.y1 = ctx->clip_rect.y1;
    if (clip.x2 > ctx->clip_rect.x2) clip.x2 = ctx->clip_rect.x2;
    if (clip.y2 > ctx->clip_rect.y2) clip.y2 = ctx->clip_rect.y2;
    memset(&command, 0, sizeof(command));
    command.type = IMGUI_RENDER_COMMAND_DRAW_INDEXED;
    command.data.draw_indexed.clip_rect = clip;
    command.data.draw_indexed.texture = texture;
    command.data.draw_indexed.index_offset = index_offset;
    command.data.draw_indexed.index_count =
        (imgui_u32)(ctx->path_count - 2) * 3U;
    command.data.draw_indexed.vertex_offset = vertex_offset;
    result = imgui_render_command_append(ctx, &command, NULL);
    if (result == IMGUI_RESULT_OK) {
        ctx->path_active = IMGUI_FALSE;
        ctx->path_count = 0;
    } else {
        ctx->vertex_count = old_vertex_count;
        ctx->index_count = old_index_count;
    }
    return result;
}

imgui_result imgui_draw_list_path_stroke(imgui_context *ctx,
                                         imgui_draw_list *draw_list,
                                         imgui_u32 color, float thickness,
                                         imgui_bool closed)
{
    imgui_result result;
    imgui_u32 old_vertex_count;
    imgui_u32 old_index_count;
    imgui_u32 old_command_count;
    if (!imgui_float_is_finite(thickness) || thickness <= 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    result = imgui_draw_list_path_validate(ctx, draw_list);
    if (result != IMGUI_RESULT_OK) return result;
    if (ctx->path_count < 2) return IMGUI_RESULT_INVALID_ARGUMENT;
    old_vertex_count = ctx->vertex_count;
    old_index_count = ctx->index_count;
    old_command_count = ctx->command_count;
    result = imgui_draw_list_add_polyline(ctx, draw_list,
                                          ctx->path_points,
                                          (size_t)ctx->path_count,
                                          color, thickness, closed);
    if (result == IMGUI_RESULT_OK) {
        ctx->path_active = IMGUI_FALSE;
        ctx->path_count = 0;
    } else {
        ctx->vertex_count = old_vertex_count;
        ctx->index_count = old_index_count;
        ctx->command_count = old_command_count;
    }
    return result;
}

imgui_result imgui_draw_list_add_texture_copy(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    const imgui_texture_copy_command *command)
{
    if (ctx == NULL || draw_list == NULL || command == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (!imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES) ||
        !imgui_renderer_has_capability(
            ctx, IMGUI_RENDERER_CAP_ORDERED_TEXTURE_COPY)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                          "texture copy outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (command->source == NULL || command->destination == NULL ||
        command->source->owner != ctx || command->destination->owner != ctx ||
        !command->source->alive || !command->destination->alive ||
        command->source_region.x < 0 || command->source_region.y < 0 ||
        command->source_region.width <= 0 || command->source_region.height <= 0 ||
        command->source_region.x > command->source->desc.width ||
        command->source_region.y > command->source->desc.height ||
        command->source_region.width >
            command->source->desc.width - command->source_region.x ||
        command->source_region.height >
            command->source->desc.height - command->source_region.y ||
        command->destination_x < 0 || command->destination_y < 0 ||
        command->destination_x > command->destination->desc.width ||
        command->destination_y > command->destination->desc.height ||
        command->source_region.width >
            command->destination->desc.width - command->destination_x ||
        command->source_region.height >
            command->destination->desc.height - command->destination_y) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    {
        imgui_render_command queued;
        memset(&queued, 0, sizeof(queued));
        queued.type = IMGUI_RENDER_COMMAND_TEXTURE_COPY;
        queued.data.texture_copy = *command;
        return imgui_render_command_append(ctx, &queued, NULL);
    }
}

imgui_result imgui_draw_list_add_texture_clear(
    imgui_context *ctx,
    imgui_draw_list *draw_list,
    const imgui_texture_clear_command *command)
{
    if (ctx == NULL || draw_list == NULL || command == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (!imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES) ||
        !imgui_renderer_has_capability(ctx,
                                       IMGUI_RENDERER_CAP_TEXTURE_CLEAR)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                          "texture clear outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (command->texture == NULL || command->texture->owner != ctx ||
        !command->texture->alive || command->region.x < 0 ||
        command->region.y < 0 || command->region.width <= 0 ||
        command->region.height <= 0 ||
        command->region.x > command->texture->desc.width ||
        command->region.y > command->texture->desc.height ||
        command->region.width >
            command->texture->desc.width - command->region.x ||
        command->region.height >
            command->texture->desc.height - command->region.y) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    {
        imgui_render_command queued;
        memset(&queued, 0, sizeof(queued));
        queued.type = IMGUI_RENDER_COMMAND_TEXTURE_CLEAR;
        queued.data.texture_clear = *command;
        return imgui_render_command_append(ctx, &queued, NULL);
    }
}

imgui_result imgui_draw_list_add_texture_update(
    imgui_context *ctx, imgui_draw_list *draw_list,
    const imgui_texture_update_command *command)
{
    imgui_render_command queued;
    size_t bytes;
    size_t row_bytes;
    size_t bytes_per_pixel;
    void *copy;
    if (ctx == NULL || draw_list == NULL || command == NULL ||
        command->texture == NULL || command->pixels == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (!imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES) ||
        !imgui_renderer_has_capability(ctx,
                                       IMGUI_RENDERER_CAP_TEXTURE_UPDATE)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                          "texture update outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (command->texture->owner != ctx || !command->texture->alive ||
        command->region.x < 0 || command->region.y < 0 ||
        command->region.width <= 0 || command->region.height <= 0 ||
        command->region.x > command->texture->desc.width ||
        command->region.y > command->texture->desc.height ||
        command->region.width > command->texture->desc.width -
            command->region.x ||
        command->region.height > command->texture->desc.height -
            command->region.y ||
        command->format != command->texture->desc.format) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    bytes_per_pixel = imgui_texture_bytes_per_pixel(command->format);
    row_bytes = (size_t)command->region.width * bytes_per_pixel;
    if (command->pitch == 0 || command->pitch < row_bytes ||
        (size_t)command->region.height > (size_t)-1 / command->pitch) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    bytes = command->pitch * (size_t)command->region.height;
    copy = imgui_internal_allocate(&ctx->allocator, bytes);
    if (copy == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    memcpy(copy, command->pixels, bytes);
    memset(&queued, 0, sizeof(queued));
    queued.type = IMGUI_RENDER_COMMAND_TEXTURE_UPDATE;
    queued.data.texture_update = *command;
    queued.data.texture_update.pixels = copy;
    if (imgui_render_command_append(ctx, &queued, copy) != IMGUI_RESULT_OK) {
        imgui_internal_release(&ctx->allocator, copy);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    return IMGUI_RESULT_OK;
}

imgui_result imgui_draw_list_add_reset_state(imgui_context *ctx,
                                             imgui_draw_list *draw_list)
{
    if (ctx == NULL || draw_list == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                         "reset state outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    {
        imgui_render_command queued;
        memset(&queued, 0, sizeof(queued));
        queued.type = IMGUI_RENDER_COMMAND_RESET_STATE;
        return imgui_render_command_append(ctx, &queued, NULL);
    }
}

imgui_result imgui_draw_list_add_sampler(imgui_context *ctx,
                                         imgui_draw_list *draw_list,
                                         imgui_sampler sampler,
                                         imgui_u32 custom_sampler_id)
{
    if (ctx == NULL || draw_list == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (sampler < IMGUI_SAMPLER_LINEAR || sampler > IMGUI_SAMPLER_CUSTOM) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                         "sampler command outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    {
        imgui_render_command queued;
        memset(&queued, 0, sizeof(queued));
        queued.type = IMGUI_RENDER_COMMAND_SET_SAMPLER;
        queued.data.set_sampler.sampler = sampler;
        queued.data.set_sampler.custom_sampler_id = custom_sampler_id;
        return imgui_render_command_append(ctx, &queued, NULL);
    }
}

imgui_result imgui_draw_list_add_custom_command(imgui_context *ctx,
                                                imgui_draw_list *draw_list,
                                                imgui_u32 command_id,
                                                const void *payload,
                                                size_t payload_size)
{
    if (ctx == NULL || draw_list == NULL ||
        (payload == NULL && payload_size != 0)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (!imgui_renderer_has_capability(ctx,
                                       IMGUI_RENDERER_CAP_CUSTOM_COMMANDS)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                         "custom command outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    {
        imgui_render_command queued;
        void *copy;
        copy = NULL;
        if (payload_size != 0) {
            copy = imgui_internal_allocate(&ctx->allocator, payload_size);
            if (copy == NULL) {
                return IMGUI_RESULT_OUT_OF_MEMORY;
            }
            memcpy(copy, payload, payload_size);
        }
        memset(&queued, 0, sizeof(queued));
        queued.type = IMGUI_RENDER_COMMAND_CUSTOM;
        queued.data.custom.command_id = command_id;
        queued.data.custom.payload = copy;
        queued.data.custom.payload_size = payload_size;
        if (imgui_render_command_append(ctx, &queued, copy) !=
            IMGUI_RESULT_OK) {
            imgui_internal_release(&ctx->allocator, copy);
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        return IMGUI_RESULT_OK;
    }
}

imgui_result imgui_draw_list_split(imgui_context *ctx,
                                   imgui_draw_list *draw_list,
                                   int channel_count)
{
    int channel;
    if (ctx == NULL || draw_list == NULL || channel_count < 1 ||
        channel_count > IMGUI_INTERNAL_DRAW_CHANNEL_CAPACITY) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                         "draw-list split outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (ctx->draw_channels_active) return IMGUI_RESULT_INVALID_STATE;
    ctx->draw_channels_active = IMGUI_TRUE;
    ctx->draw_channel_count = channel_count;
    ctx->draw_channel_current = 0;
    for (channel = 0; channel < IMGUI_INTERNAL_DRAW_CHANNEL_CAPACITY;
         ++channel) {
        ctx->draw_channel_commands[channel] = NULL;
        ctx->draw_channel_payloads[channel] = NULL;
        ctx->draw_channel_command_counts[channel] = 0;
        ctx->draw_channel_command_capacities[channel] = 0;
    }
    ctx->draw_channel_commands[0] = ctx->commands;
    ctx->draw_channel_payloads[0] = ctx->command_payloads;
    ctx->draw_channel_command_counts[0] = ctx->command_count;
    ctx->draw_channel_command_capacities[0] = ctx->command_capacity;
    ctx->commands = NULL;
    ctx->command_payloads = NULL;
    ctx->command_count = 0;
    ctx->command_capacity = 0;
    imgui_draw_channels_load(ctx, 0);
    return IMGUI_RESULT_OK;
}

imgui_result imgui_draw_list_set_channel(imgui_context *ctx,
                                         imgui_draw_list *draw_list,
                                         int channel_index)
{
    if (ctx == NULL || draw_list == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                         "draw-list channel outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (!ctx->draw_channels_active || channel_index < 0 ||
        channel_index >= ctx->draw_channel_count) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (channel_index == ctx->draw_channel_current) {
        return IMGUI_RESULT_OK;
    }
    imgui_draw_channels_save_current(ctx);
    imgui_draw_channels_load(ctx, channel_index);
    return IMGUI_RESULT_OK;
}

imgui_result imgui_draw_list_merge(imgui_context *ctx,
                                   imgui_draw_list *draw_list)
{
    imgui_u32 total;
    imgui_u32 offset;
    int channel;
    imgui_render_command *commands;
    void **payloads;
    if (ctx == NULL || draw_list == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (draw_list->owner != ctx ||
        !imgui_internal_require_building(ctx,
                                         "draw-list merge outside frame")) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    if (!ctx->draw_channels_active) return IMGUI_RESULT_INVALID_STATE;
    imgui_draw_channels_save_current(ctx);
    total = 0;
    for (channel = 0; channel < ctx->draw_channel_count; ++channel) {
        if (0xffffffffUL - total <
            ctx->draw_channel_command_counts[channel]) {
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        total += ctx->draw_channel_command_counts[channel];
    }
    commands = NULL;
    payloads = NULL;
    if (total != 0) {
        commands = (imgui_render_command *)imgui_internal_allocate(
            &ctx->allocator, (size_t)total * sizeof(*commands));
        payloads = (void **)imgui_internal_allocate(
            &ctx->allocator, (size_t)total * sizeof(*payloads));
        if (commands == NULL || payloads == NULL) {
            imgui_internal_release(&ctx->allocator, commands);
            imgui_internal_release(&ctx->allocator, payloads);
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
    }
    offset = 0;
    for (channel = 0; channel < ctx->draw_channel_count; ++channel) {
        if (ctx->draw_channel_command_counts[channel] != 0) {
            memcpy(commands + offset, ctx->draw_channel_commands[channel],
                   (size_t)ctx->draw_channel_command_counts[channel] *
                   sizeof(*commands));
            memcpy(payloads + offset, ctx->draw_channel_payloads[channel],
                   (size_t)ctx->draw_channel_command_counts[channel] *
                   sizeof(*payloads));
            offset += ctx->draw_channel_command_counts[channel];
        }
        imgui_internal_release(&ctx->allocator,
                               ctx->draw_channel_commands[channel]);
        imgui_internal_release(&ctx->allocator,
                               ctx->draw_channel_payloads[channel]);
        ctx->draw_channel_commands[channel] = NULL;
        ctx->draw_channel_payloads[channel] = NULL;
        ctx->draw_channel_command_counts[channel] = 0;
        ctx->draw_channel_command_capacities[channel] = 0;
    }
    ctx->commands = commands;
    ctx->command_payloads = payloads;
    ctx->command_count = total;
    ctx->command_capacity = total;
    ctx->draw_channels_active = IMGUI_FALSE;
    ctx->draw_channel_count = 0;
    ctx->draw_channel_current = -1;
    return IMGUI_RESULT_OK;
}

typedef struct imgui_packet_clone_storage {
    imgui_resource_operation *resources;
    void **resource_payloads;
    imgui_viewport_packet *viewports;
    imgui_render_list *lists;
    imgui_render_vertex **vertices;
    imgui_render_index **indices;
    imgui_render_command **commands;
    void ***command_payloads;
    imgui_texture **texture_sources;
    imgui_texture **textures;
    imgui_u32 viewport_count;
    imgui_u32 list_count;
    imgui_u32 resource_count;
    imgui_u32 texture_count;
    imgui_u32 texture_capacity;
} imgui_packet_clone_storage;

static imgui_texture *imgui_packet_clone_texture(
    imgui_packet_clone_storage *storage,
    const imgui_allocator *allocator,
    imgui_texture *source)
{
    imgui_u32 i;
    imgui_u32 capacity;
    imgui_texture **new_sources;
    imgui_texture **new_textures;
    imgui_texture *copy;
    if (source == NULL) {
        return NULL;
    }
    for (i = 0; i < storage->texture_count; ++i) {
        if (storage->texture_sources[i] == source) {
            return storage->textures[i];
        }
    }
    if (storage->texture_count == storage->texture_capacity) {
        capacity = storage->texture_capacity == 0 ? 4U :
                   storage->texture_capacity * 2U;
        new_sources = (imgui_texture **)imgui_internal_allocate(
            allocator, (size_t)capacity * sizeof(*new_sources));
        new_textures = (imgui_texture **)imgui_internal_allocate(
            allocator, (size_t)capacity * sizeof(*new_textures));
        if (new_sources == NULL || new_textures == NULL) {
            imgui_internal_release(allocator, new_sources);
            imgui_internal_release(allocator, new_textures);
            return NULL;
        }
        if (storage->texture_count != 0) {
            memcpy(new_sources, storage->texture_sources,
                   (size_t)storage->texture_count * sizeof(*new_sources));
            memcpy(new_textures, storage->textures,
                   (size_t)storage->texture_count * sizeof(*new_textures));
        }
        imgui_internal_release(allocator, storage->texture_sources);
        imgui_internal_release(allocator, storage->textures);
        storage->texture_sources = new_sources;
        storage->textures = new_textures;
        storage->texture_capacity = capacity;
    }
    copy = (imgui_texture *)imgui_internal_allocate(allocator, sizeof(*copy));
    if (copy == NULL) {
        return NULL;
    }
    *copy = *source;
    copy->owner = NULL;
    copy->next = NULL;
    storage->texture_sources[storage->texture_count] = source;
    storage->textures[storage->texture_count] = copy;
    storage->texture_count += 1;
    return copy;
}

static void imgui_packet_clone_release(
    imgui_packet_clone_storage *storage,
    const imgui_allocator *allocator)
{
    imgui_u32 i;
    if (storage == NULL) {
        return;
    }
    for (i = 0; i < storage->list_count; ++i) {
        imgui_u32 j;
        if (storage->command_payloads != NULL &&
            storage->command_payloads[i] != NULL) {
            if (storage->commands != NULL && storage->commands[i] != NULL) {
                for (j = 0; j < storage->lists[i].command_count; ++j) {
                    imgui_internal_release(allocator,
                        storage->command_payloads[i][j]);
                }
            }
            imgui_internal_release(allocator, storage->command_payloads[i]);
        }
        imgui_internal_release(allocator,
                               storage->vertices != NULL ? storage->vertices[i] : NULL);
        imgui_internal_release(allocator,
                               storage->indices != NULL ? storage->indices[i] : NULL);
        imgui_internal_release(allocator,
                               storage->commands != NULL ? storage->commands[i] : NULL);
    }
    if (storage->resource_payloads != NULL) {
        for (i = 0; i < storage->resource_count; ++i) {
            imgui_internal_release(allocator, storage->resource_payloads[i]);
        }
    }
    imgui_internal_release(allocator, storage->resource_payloads);
    imgui_internal_release(allocator, storage->resources);
    imgui_internal_release(allocator, storage->command_payloads);
    imgui_internal_release(allocator, storage->commands);
    imgui_internal_release(allocator, storage->indices);
    imgui_internal_release(allocator, storage->vertices);
    imgui_internal_release(allocator, storage->lists);
    imgui_internal_release(allocator, storage->viewports);
    for (i = 0; i < storage->texture_count; ++i) {
        imgui_internal_release(allocator, storage->textures[i]);
    }
    imgui_internal_release(allocator, storage->texture_sources);
    imgui_internal_release(allocator, storage->textures);
    imgui_internal_release(allocator, storage);
}

static imgui_bool imgui_resource_operation_payload_size(
    const imgui_resource_operation *operation, size_t *out_bytes)
{
    size_t row_count;
    if (operation == NULL || out_bytes == NULL) return IMGUI_FALSE;
    *out_bytes = 0;
    if (operation->type == IMGUI_RESOURCE_DESTROY_TEXTURE ||
        operation->pixels == NULL) {
        return IMGUI_TRUE;
    }
    if (operation->type == IMGUI_RESOURCE_CREATE_TEXTURE) {
        if (operation->texture_width <= 0 || operation->texture_height <= 0) {
            return IMGUI_FALSE;
        }
        row_count = (size_t)operation->texture_height;
    } else if (operation->type == IMGUI_RESOURCE_UPLOAD_TEXTURE ||
               operation->type == IMGUI_RESOURCE_UPDATE_TEXTURE) {
        if (operation->region.width <= 0 || operation->region.height <= 0) {
            return IMGUI_FALSE;
        }
        row_count = (size_t)operation->region.height;
    } else {
        return IMGUI_FALSE;
    }
    if (operation->pitch == 0 || row_count > (size_t)-1 / operation->pitch) {
        return IMGUI_FALSE;
    }
    *out_bytes = operation->pitch * row_count;
    return IMGUI_TRUE;
}

imgui_result imgui_render_packet_clone(const imgui_render_packet *source,
                                       const imgui_allocator *allocator,
                                       imgui_render_packet **out_packet)
{
    imgui_render_packet *packet;
    imgui_packet_clone_storage *storage;
    imgui_u32 viewport_index;
    imgui_u32 list_index;
    imgui_u32 list_offset;
    imgui_u32 total_lists;
    imgui_u32 command_index;
    imgui_u32 validation_index;
    const imgui_render_list *source_list;
    const imgui_render_command *source_command;
    void *copy;
    if (out_packet == NULL) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    *out_packet = NULL;
    if (source == NULL || source->struct_size < sizeof(*source) ||
        source->protocol_version == 0 ||
        (source->resource_operation_count != 0 &&
         source->resource_operations == NULL) ||
        (source->viewport_count != 0 && source->viewports == NULL)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    for (validation_index = 0;
         validation_index < source->resource_operation_count;
         ++validation_index) {
        const imgui_resource_operation *resource;
        size_t payload_bytes;
        size_t row_bytes;
        size_t bytes_per_pixel;
        resource = &source->resource_operations[validation_index];
        if (resource->texture == NULL ||
            resource->type > IMGUI_RESOURCE_DESTROY_TEXTURE ||
            !imgui_resource_operation_payload_size(resource, &payload_bytes)) {
            return IMGUI_RESULT_INVALID_ARGUMENT;
        }
        if (resource->pixels == NULL) continue;
        bytes_per_pixel = imgui_texture_bytes_per_pixel(resource->format);
        if (bytes_per_pixel == 0) return IMGUI_RESULT_INVALID_ARGUMENT;
        if (resource->type == IMGUI_RESOURCE_CREATE_TEXTURE) {
            if (resource->texture_width <= 0 || resource->texture_height <= 0) {
                return IMGUI_RESULT_INVALID_ARGUMENT;
            }
            row_bytes = (size_t)resource->texture_width * bytes_per_pixel;
        } else {
            if (resource->region.x < 0 || resource->region.y < 0 ||
                resource->region.width <= 0 || resource->region.height <= 0 ||
                resource->region.width > resource->texture->desc.width -
                    resource->region.x ||
                resource->region.height > resource->texture->desc.height -
                    resource->region.y) {
                return IMGUI_RESULT_INVALID_ARGUMENT;
            }
            row_bytes = (size_t)resource->region.width * bytes_per_pixel;
        }
        if (resource->pitch < row_bytes || payload_bytes == 0) {
            return IMGUI_RESULT_INVALID_ARGUMENT;
        }
    }
    total_lists = 0;
    for (viewport_index = 0; viewport_index < source->viewport_count;
         ++viewport_index) {
        const imgui_viewport_packet *viewport;
        viewport = &source->viewports[viewport_index];
        if (viewport->list_count != 0 && viewport->lists == NULL) {
            return IMGUI_RESULT_INVALID_ARGUMENT;
        }
        if (0xffffffffUL - total_lists < viewport->list_count) {
            return IMGUI_RESULT_INVALID_ARGUMENT;
        }
        total_lists += viewport->list_count;
        for (validation_index = 0; validation_index < viewport->list_count;
             ++validation_index) {
            const imgui_render_list *list;
            imgui_u32 command_validation_index;
            list = &viewport->lists[validation_index];
            if ((list->vertex_count != 0 && list->vertices == NULL) ||
                (list->index_count != 0 && list->indices == NULL) ||
                (list->command_count != 0 && list->commands == NULL) ||
                (size_t)list->vertex_count >
                    (size_t)-1 / sizeof(*list->vertices) ||
                (size_t)list->index_count >
                    (size_t)-1 / sizeof(*list->indices) ||
                (size_t)list->command_count >
                    (size_t)-1 / sizeof(*list->commands)) {
                return IMGUI_RESULT_INVALID_ARGUMENT;
            }
            for (command_validation_index = 0;
                 command_validation_index < list->command_count;
                 ++command_validation_index) {
                const imgui_render_command *command;
                command = &list->commands[command_validation_index];
                if (command->type > IMGUI_RENDER_COMMAND_TEXTURE_UPDATE) {
                    return IMGUI_RESULT_INVALID_ARGUMENT;
                }
                if (command->type == IMGUI_RENDER_COMMAND_CUSTOM &&
                    command->data.custom.payload_size != 0 &&
                    command->data.custom.payload == NULL) {
                    return IMGUI_RESULT_INVALID_ARGUMENT;
                }
                if (command->type == IMGUI_RENDER_COMMAND_TEXTURE_UPDATE) {
                    if (command->data.texture_update.texture == NULL ||
                        command->data.texture_update.pixels == NULL ||
                        command->data.texture_update.pitch == 0 ||
                        command->data.texture_update.region.width <= 0 ||
                        command->data.texture_update.region.height <= 0 ||
                        (size_t)command->data.texture_update.region.height >
                            (size_t)-1 / command->data.texture_update.pitch) {
                        return IMGUI_RESULT_INVALID_ARGUMENT;
                    }
                }
            }
        }
    }
    packet = (imgui_render_packet *)imgui_internal_allocate(
        allocator, sizeof(*packet));
    storage = (imgui_packet_clone_storage *)imgui_internal_allocate(
        allocator, sizeof(*storage));
    if (packet == NULL || storage == NULL) {
        imgui_internal_release(allocator, packet);
        imgui_internal_release(allocator, storage);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    memset(packet, 0, sizeof(*packet));
    memset(storage, 0, sizeof(*storage));
    storage->viewport_count = source->viewport_count;
    storage->list_count = total_lists;
    storage->resource_count = source->resource_operation_count;
    if (source->resource_operation_count != 0) {
        storage->resources = (imgui_resource_operation *)
            imgui_internal_allocate(allocator,
                (size_t)source->resource_operation_count * sizeof(*storage->resources));
        storage->resource_payloads = (void **)imgui_internal_allocate(
            allocator, (size_t)source->resource_operation_count *
                       sizeof(*storage->resource_payloads));
        if (storage->resources == NULL || storage->resource_payloads == NULL) {
            imgui_packet_clone_release(storage, allocator);
            imgui_internal_release(allocator, packet);
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        memcpy(storage->resources, source->resource_operations,
               (size_t)source->resource_operation_count * sizeof(*storage->resources));
        memset(storage->resource_payloads, 0,
               (size_t)source->resource_operation_count *
               sizeof(*storage->resource_payloads));
        for (command_index = 0;
             command_index < source->resource_operation_count;
             ++command_index) {
            size_t bytes;
            imgui_texture *mapped_texture;
            mapped_texture = imgui_packet_clone_texture(
                storage, allocator, storage->resources[command_index].texture);
            if (storage->resources[command_index].texture != NULL &&
                mapped_texture == NULL) {
                imgui_packet_clone_release(storage, allocator);
                imgui_internal_release(allocator, packet);
                return IMGUI_RESULT_OUT_OF_MEMORY;
            }
            storage->resources[command_index].texture = mapped_texture;
            if (!imgui_resource_operation_payload_size(
                    &storage->resources[command_index], &bytes)) {
                imgui_packet_clone_release(storage, allocator);
                imgui_internal_release(allocator, packet);
                return IMGUI_RESULT_INVALID_ARGUMENT;
            }
            if (bytes != 0 && storage->resources[command_index].pixels != NULL) {
                copy = imgui_internal_allocate(allocator, bytes);
                if (copy == NULL) {
                    imgui_packet_clone_release(storage, allocator);
                    imgui_internal_release(allocator, packet);
                    return IMGUI_RESULT_OUT_OF_MEMORY;
                }
                memcpy(copy, storage->resources[command_index].pixels, bytes);
                storage->resources[command_index].pixels = copy;
                storage->resource_payloads[command_index] = copy;
            }
        }
    }
    if (source->viewport_count != 0) {
        storage->viewports = (imgui_viewport_packet *)imgui_internal_allocate(
            allocator, (size_t)source->viewport_count * sizeof(*storage->viewports));
    }
    if (total_lists != 0) {
        storage->lists = (imgui_render_list *)imgui_internal_allocate(
            allocator, (size_t)total_lists * sizeof(*storage->lists));
        storage->vertices = (imgui_render_vertex **)imgui_internal_allocate(
            allocator, (size_t)total_lists * sizeof(*storage->vertices));
        storage->indices = (imgui_render_index **)imgui_internal_allocate(
            allocator, (size_t)total_lists * sizeof(*storage->indices));
        storage->commands = (imgui_render_command **)imgui_internal_allocate(
            allocator, (size_t)total_lists * sizeof(*storage->commands));
        storage->command_payloads = (void ***)imgui_internal_allocate(
            allocator, (size_t)total_lists * sizeof(*storage->command_payloads));
    }
    if ((source->viewport_count != 0 && storage->viewports == NULL) ||
        (total_lists != 0 && (storage->lists == NULL || storage->vertices == NULL ||
                              storage->indices == NULL || storage->commands == NULL ||
                              storage->command_payloads == NULL))) {
        imgui_packet_clone_release(storage, allocator);
        imgui_internal_release(allocator, packet);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    if (total_lists != 0) {
        memset(storage->lists, 0, (size_t)total_lists * sizeof(*storage->lists));
        memset(storage->vertices, 0, (size_t)total_lists * sizeof(*storage->vertices));
        memset(storage->indices, 0, (size_t)total_lists * sizeof(*storage->indices));
        memset(storage->commands, 0, (size_t)total_lists * sizeof(*storage->commands));
        memset(storage->command_payloads, 0,
               (size_t)total_lists * sizeof(*storage->command_payloads));
    }
    list_offset = 0;
    for (viewport_index = 0; viewport_index < source->viewport_count;
         ++viewport_index) {
        storage->viewports[viewport_index] = source->viewports[viewport_index];
        storage->viewports[viewport_index].lists = storage->lists + list_offset;
        for (list_index = 0;
             list_index < source->viewports[viewport_index].list_count;
             ++list_index, ++list_offset) {
            source_list = &source->viewports[viewport_index].lists[list_index];
            storage->lists[list_offset] = *source_list;
            if (source_list->vertex_count != 0) {
                storage->vertices[list_offset] = (imgui_render_vertex *)
                    imgui_internal_allocate(allocator,
                        (size_t)source_list->vertex_count * sizeof(*storage->vertices[list_offset]));
                if (storage->vertices[list_offset] == NULL) goto clone_oom;
                memcpy(storage->vertices[list_offset], source_list->vertices,
                       (size_t)source_list->vertex_count * sizeof(*storage->vertices[list_offset]));
                storage->lists[list_offset].vertices = storage->vertices[list_offset];
            }
            if (source_list->index_count != 0) {
                storage->indices[list_offset] = (imgui_render_index *)
                    imgui_internal_allocate(allocator,
                        (size_t)source_list->index_count * sizeof(*storage->indices[list_offset]));
                if (storage->indices[list_offset] == NULL) goto clone_oom;
                memcpy(storage->indices[list_offset], source_list->indices,
                       (size_t)source_list->index_count * sizeof(*storage->indices[list_offset]));
                storage->lists[list_offset].indices = storage->indices[list_offset];
            }
            if (source_list->command_count != 0) {
                storage->commands[list_offset] = (imgui_render_command *)
                    imgui_internal_allocate(allocator,
                        (size_t)source_list->command_count * sizeof(*storage->commands[list_offset]));
                storage->command_payloads[list_offset] = (void **)
                    imgui_internal_allocate(allocator,
                        (size_t)source_list->command_count * sizeof(void *));
                if (storage->commands[list_offset] == NULL ||
                    storage->command_payloads[list_offset] == NULL) goto clone_oom;
                memset(storage->command_payloads[list_offset], 0,
                       (size_t)source_list->command_count * sizeof(void *));
                memcpy(storage->commands[list_offset], source_list->commands,
                       (size_t)source_list->command_count * sizeof(*storage->commands[list_offset]));
                for (command_index = 0; command_index < source_list->command_count;
                     ++command_index) {
                    source_command = &source_list->commands[command_index];
                    if (source_command->type == IMGUI_RENDER_COMMAND_DRAW_INDEXED) {
                        storage->commands[list_offset][command_index].data.draw_indexed.texture =
                            imgui_packet_clone_texture(storage, allocator,
                                source_command->data.draw_indexed.texture);
                        if (source_command->data.draw_indexed.texture != NULL &&
                            storage->commands[list_offset][command_index].data.draw_indexed.texture == NULL) {
                            goto clone_oom;
                        }
                    } else if (source_command->type == IMGUI_RENDER_COMMAND_TEXTURE_COPY) {
                        storage->commands[list_offset][command_index].data.texture_copy.source =
                            imgui_packet_clone_texture(storage, allocator,
                                source_command->data.texture_copy.source);
                        storage->commands[list_offset][command_index].data.texture_copy.destination =
                            imgui_packet_clone_texture(storage, allocator,
                                source_command->data.texture_copy.destination);
                        if ((source_command->data.texture_copy.source != NULL &&
                             storage->commands[list_offset][command_index].data.texture_copy.source == NULL) ||
                            (source_command->data.texture_copy.destination != NULL &&
                             storage->commands[list_offset][command_index].data.texture_copy.destination == NULL)) {
                            goto clone_oom;
                        }
                    } else if (source_command->type ==
                               IMGUI_RENDER_COMMAND_TEXTURE_CLEAR) {
                        storage->commands[list_offset][command_index]
                            .data.texture_clear.texture =
                            imgui_packet_clone_texture(storage, allocator,
                                source_command->data.texture_clear.texture);
                        if (source_command->data.texture_clear.texture != NULL &&
                            storage->commands[list_offset][command_index]
                                .data.texture_clear.texture == NULL) {
                            goto clone_oom;
                        }
                    } else if (source_command->type ==
                               IMGUI_RENDER_COMMAND_TEXTURE_UPDATE) {
                        storage->commands[list_offset][command_index]
                            .data.texture_update.texture =
                            imgui_packet_clone_texture(storage, allocator,
                                source_command->data.texture_update.texture);
                        if (source_command->data.texture_update.texture != NULL &&
                            storage->commands[list_offset][command_index]
                                .data.texture_update.texture == NULL) {
                            goto clone_oom;
                        }
                    }
                    if (source_command->type == IMGUI_RENDER_COMMAND_CUSTOM &&
                        source_command->data.custom.payload_size != 0) {
                        copy = imgui_internal_allocate(allocator,
                            source_command->data.custom.payload_size);
                        if (copy == NULL) goto clone_oom;
                        memcpy(copy, source_command->data.custom.payload,
                               source_command->data.custom.payload_size);
                        storage->commands[list_offset][command_index].data.custom.payload = copy;
                        storage->command_payloads[list_offset][command_index] = copy;
                    }
                    if (source_command->type == IMGUI_RENDER_COMMAND_TEXTURE_UPDATE) {
                        size_t update_bytes;
                        update_bytes = source_command->data.texture_update.pitch *
                            (size_t)source_command->data.texture_update.region.height;
                        copy = imgui_internal_allocate(allocator, update_bytes);
                        if (copy == NULL) goto clone_oom;
                        memcpy(copy, source_command->data.texture_update.pixels,
                               update_bytes);
                        storage->commands[list_offset][command_index]
                            .data.texture_update.pixels = copy;
                        storage->command_payloads[list_offset][command_index] = copy;
                    }
                }
                storage->lists[list_offset].commands = storage->commands[list_offset];
            }
        }
    }
    packet->struct_size = sizeof(*packet);
    packet->protocol_version = source->protocol_version;
    packet->frame_index = source->frame_index;
    packet->resource_operations = storage->resources;
    packet->resource_operation_count = source->resource_operation_count;
    packet->viewports = storage->viewports;
    packet->viewport_count = source->viewport_count;
    packet->owned = IMGUI_TRUE;
    packet->private_data = storage;
    *out_packet = packet;
    return IMGUI_RESULT_OK;

clone_oom:
    imgui_packet_clone_release(storage, allocator);
    imgui_internal_release(allocator, packet);
    return IMGUI_RESULT_OUT_OF_MEMORY;
}

void imgui_render_packet_destroy(imgui_render_packet *packet,
                                 const imgui_allocator *allocator)
{
    if (packet == NULL) {
        return;
    }
    if (!packet->owned) {
        return;
    }
    imgui_packet_clone_release((imgui_packet_clone_storage *)
                               packet->private_data, allocator);
    imgui_internal_release(allocator, packet);
}

void imgui_trace_event_init(imgui_trace_event *event,
                            imgui_trace_event_type type)
{
    if (event == NULL) return;
    memset(event, 0, sizeof(*event));
    event->struct_size = sizeof(*event);
    event->type = type;
}

imgui_result imgui_trace_create(const imgui_allocator *allocator,
                                imgui_trace **out_trace)
{
    imgui_trace *trace;
    imgui_allocator selected;
    if (out_trace == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    *out_trace = NULL;
    memset(&selected, 0, sizeof(selected));
    if (allocator != NULL) selected = *allocator;
    if ((selected.allocate == NULL) != (selected.release == NULL)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    trace = (imgui_trace *)imgui_internal_allocate(&selected,
                                                    sizeof(*trace));
    if (trace == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    memset(trace, 0, sizeof(*trace));
    trace->allocator = selected;
    *out_trace = trace;
    return IMGUI_RESULT_OK;
}

void imgui_trace_destroy(imgui_trace *trace)
{
    imgui_allocator allocator;
    if (trace == NULL) return;
    allocator = trace->allocator;
    imgui_internal_release(&allocator, trace->events);
    imgui_internal_release(&allocator, trace);
}

void imgui_trace_clear(imgui_trace *trace)
{
    if (trace == NULL) return;
    trace->count = 0;
}

size_t imgui_trace_get_count(const imgui_trace *trace)
{
    return trace != NULL ? trace->count : 0;
}

const imgui_trace_event *imgui_trace_get_event(const imgui_trace *trace,
                                               size_t index)
{
    if (trace == NULL || index >= trace->count) return NULL;
    return &trace->events[index];
}

imgui_result imgui_trace_append_event(imgui_trace *trace,
                                      const imgui_trace_event *event)
{
    imgui_trace_event *events;
    size_t capacity;
    if (trace == NULL || event == NULL || event->struct_size < sizeof(*event) ||
        event->type < IMGUI_TRACE_MOUSE_POSITION ||
        event->type > IMGUI_TRACE_FRAME ||
        event->text_length >= sizeof(event->text)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (trace->count == trace->capacity) {
        capacity = trace->capacity == 0 ? 16 : trace->capacity * 2;
        events = (imgui_trace_event *)imgui_internal_allocate(
            &trace->allocator, capacity * sizeof(*events));
        if (events == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
        if (trace->count != 0) memcpy(events, trace->events,
            trace->count * sizeof(*events));
        imgui_internal_release(&trace->allocator, trace->events);
        trace->events = events;
        trace->capacity = capacity;
    }
    trace->events[trace->count] = *event;
    trace->events[trace->count].text[event->text_length] = '\0';
    trace->count += 1;
    return IMGUI_RESULT_OK;
}

static imgui_result imgui_trace_append_scalar(imgui_trace *trace,
                                              imgui_trace_event_type type,
                                              float x, float y, int code,
                                              imgui_bool down)
{
    imgui_trace_event event;
    imgui_trace_event_init(&event, type);
    event.x = x;
    event.y = y;
    event.code = code;
    event.down = down ? IMGUI_TRUE : IMGUI_FALSE;
    return imgui_trace_append_event(trace, &event);
}

imgui_result imgui_trace_append_mouse_position(imgui_trace *trace,
                                               float x, float y)
{
    if (!imgui_float_is_finite(x) || !imgui_float_is_finite(y)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    return imgui_trace_append_scalar(trace, IMGUI_TRACE_MOUSE_POSITION,
                                     x, y, 0, IMGUI_FALSE);
}

imgui_result imgui_trace_append_mouse_button(imgui_trace *trace,
                                             imgui_mouse_button button,
                                             imgui_bool down)
{
    if (button < 0 || button >= IMGUI_MOUSE_BUTTON_COUNT) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    return imgui_trace_append_scalar(trace, IMGUI_TRACE_MOUSE_BUTTON,
                                     0.0f, 0.0f, (int)button, down);
}

imgui_result imgui_trace_append_mouse_wheel(imgui_trace *trace,
                                            float horizontal, float vertical)
{
    if (!imgui_float_is_finite(horizontal) ||
        !imgui_float_is_finite(vertical)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    return imgui_trace_append_scalar(trace, IMGUI_TRACE_MOUSE_WHEEL,
                                     horizontal, vertical, 0, IMGUI_FALSE);
}

imgui_result imgui_trace_append_key(imgui_trace *trace, imgui_key key,
                                    imgui_bool down)
{
    if (key < 0 || key >= IMGUI_KEY_COUNT) return IMGUI_RESULT_INVALID_ARGUMENT;
    return imgui_trace_append_scalar(trace, IMGUI_TRACE_KEY, 0.0f, 0.0f,
                                     (int)key, down);
}

imgui_result imgui_trace_append_key_analog(imgui_trace *trace, imgui_key key,
                                           imgui_bool down, float value)
{
    imgui_trace_event event;
    if (key < 0 || key >= IMGUI_KEY_COUNT ||
        !imgui_float_is_finite(value) || value < 0.0f || value > 1.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    imgui_trace_event_init(&event, IMGUI_TRACE_KEY_ANALOG);
    event.code = (int)key;
    event.down = down ? IMGUI_TRUE : IMGUI_FALSE;
    event.analog_value = value;
    return imgui_trace_append_event(trace, &event);
}

imgui_result imgui_trace_append_text(imgui_trace *trace, const char *text)
{
    imgui_trace_event event;
    size_t length;
    if (text == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    length = strlen(text);
    if (length >= sizeof(event.text)) return IMGUI_RESULT_INVALID_ARGUMENT;
    imgui_trace_event_init(&event, IMGUI_TRACE_TEXT_UTF8);
    memcpy(event.text, text, length);
    event.text[length] = '\0';
    event.text_length = length;
    return imgui_trace_append_event(trace, &event);
}

imgui_result imgui_trace_append_codepoint(imgui_trace *trace,
                                          unsigned long codepoint)
{
    imgui_trace_event event;
    if (codepoint > 0x10ffffUL ||
        (codepoint >= 0xd800UL && codepoint <= 0xdfffUL)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    imgui_trace_event_init(&event, IMGUI_TRACE_CODEPOINT);
    event.codepoint = codepoint;
    return imgui_trace_append_event(trace, &event);
}

imgui_result imgui_trace_append_focus(imgui_trace *trace, imgui_bool focused)
{
    return imgui_trace_append_scalar(trace, IMGUI_TRACE_FOCUS, 0.0f, 0.0f,
                                     0, focused);
}

imgui_result imgui_trace_append_frame(imgui_trace *trace,
                                      float delta_time, double time)
{
    imgui_trace_event event;
    if (!imgui_float_is_finite(delta_time) ||
        !imgui_double_is_finite(time) || delta_time <= 0.0f) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    imgui_trace_event_init(&event, IMGUI_TRACE_FRAME);
    event.x = delta_time;
    event.time = time;
    return imgui_trace_append_event(trace, &event);
}

imgui_result imgui_trace_apply_event(imgui_context *ctx,
                                     const imgui_trace_event *event)
{
    if (ctx == NULL || event == NULL || event->struct_size < sizeof(*event)) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    switch (event->type) {
    case IMGUI_TRACE_MOUSE_POSITION:
        return imgui_input_add_mouse_position(ctx, event->x, event->y);
    case IMGUI_TRACE_MOUSE_BUTTON:
        return imgui_input_add_mouse_button(ctx,
            (imgui_mouse_button)event->code, event->down);
    case IMGUI_TRACE_MOUSE_WHEEL:
        return imgui_input_add_mouse_wheel(ctx, event->x, event->y);
    case IMGUI_TRACE_KEY:
        return imgui_input_add_key(ctx, (imgui_key)event->code, event->down);
    case IMGUI_TRACE_KEY_ANALOG:
        return imgui_input_add_key_analog(ctx, (imgui_key)event->code,
                                          event->down, event->analog_value);
    case IMGUI_TRACE_TEXT_UTF8:
        return imgui_input_add_text_utf8(ctx, event->text);
    case IMGUI_TRACE_CODEPOINT:
        return imgui_input_add_codepoint(ctx, event->codepoint);
    case IMGUI_TRACE_FOCUS:
        return imgui_input_add_focus(ctx, event->down);
    case IMGUI_TRACE_FRAME:
        return IMGUI_RESULT_OK;
    default:
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
}

imgui_result imgui_trace_replay(imgui_context *ctx, const imgui_trace *trace,
                                size_t first_event, size_t event_count)
{
    size_t index;
    size_t end;
    imgui_result result;
    if (ctx == NULL || trace == NULL || first_event > trace->count) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (event_count > trace->count - first_event) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    end = first_event + event_count;
    for (index = first_event; index < end; ++index) {
        result = imgui_trace_apply_event(ctx, &trace->events[index]);
        if (result != IMGUI_RESULT_OK) return result;
    }
    return IMGUI_RESULT_OK;
}

imgui_result imgui_trace_replay_frames(
    imgui_context *ctx, const imgui_trace *trace,
    size_t first_event, size_t event_count,
    imgui_trace_frame_callback callback, void *user_data)
{
    size_t index;
    size_t end;
    imgui_result result;
    if (ctx == NULL || trace == NULL || callback == NULL ||
        first_event > trace->count || event_count > trace->count - first_event) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    end = first_event + event_count;
    for (index = first_event; index < end; ++index) {
        if (trace->events[index].type == IMGUI_TRACE_FRAME) {
            result = callback(ctx, trace->events[index].x,
                              trace->events[index].time, user_data);
        } else {
            result = imgui_trace_apply_event(ctx, &trace->events[index]);
        }
        if (result != IMGUI_RESULT_OK) return result;
    }
    return IMGUI_RESULT_OK;
}

static imgui_bool imgui_trace_output_append(char *buffer, size_t capacity,
                                            size_t *length, const char *text)
{
    size_t text_length;
    if (buffer == NULL || length == NULL || text == NULL) return IMGUI_FALSE;
    text_length = strlen(text);
    if (*length > capacity || text_length >= capacity - *length) {
        return IMGUI_FALSE;
    }
    memcpy(buffer + *length, text, text_length);
    *length += text_length;
    buffer[*length] = '\0';
    return IMGUI_TRUE;
}

imgui_result imgui_trace_save(const imgui_trace *trace, char *buffer,
                              size_t capacity, size_t *required)
{
    size_t estimated;
    size_t length;
    size_t index;
    char *generated;
    char line[640];
    const imgui_trace_event *event;
    static const char hex[] = "0123456789abcdef";
    if (trace == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (trace->count > ((size_t)-1 - 32U) / 600U) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    estimated = 32U + trace->count * 600U;
    generated = (char *)imgui_internal_allocate(&trace->allocator, estimated);
    if (generated == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    generated[0] = '\0';
    length = 0;
    if (!imgui_trace_output_append(generated, estimated, &length,
                                   "IMGUI_C89_TRACE 1\n")) {
        imgui_internal_release(&trace->allocator, generated);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    for (index = 0; index < trace->count; ++index) {
        event = &trace->events[index];
        line[0] = '\0';
        switch (event->type) {
        case IMGUI_TRACE_MOUSE_POSITION:
            sprintf(line, "P %.9g %.9g\n", (double)event->x,
                    (double)event->y);
            break;
        case IMGUI_TRACE_MOUSE_BUTTON:
            sprintf(line, "B %d %d\n", event->code, event->down ? 1 : 0);
            break;
        case IMGUI_TRACE_MOUSE_WHEEL:
            sprintf(line, "W %.9g %.9g\n", (double)event->x,
                    (double)event->y);
            break;
        case IMGUI_TRACE_KEY:
            sprintf(line, "K %d %d\n", event->code, event->down ? 1 : 0);
            break;
        case IMGUI_TRACE_KEY_ANALOG:
            sprintf(line, "A %d %d %.9g\n", event->code,
                    event->down ? 1 : 0, (double)event->analog_value);
            break;
        case IMGUI_TRACE_TEXT_UTF8:
            {
                size_t char_index;
                size_t line_length;
                line[0] = 'T';
                line[1] = ' ';
                line_length = 2;
                for (char_index = 0; char_index < event->text_length;
                     ++char_index) {
                    unsigned char value = (unsigned char)event->text[char_index];
                    line[line_length++] = hex[value >> 4];
                    line[line_length++] = hex[value & 15U];
                }
                line[line_length++] = '\n';
                line[line_length] = '\0';
            }
            break;
        case IMGUI_TRACE_CODEPOINT:
            sprintf(line, "C %lu\n", event->codepoint);
            break;
        case IMGUI_TRACE_FOCUS:
            sprintf(line, "F %d\n", event->down ? 1 : 0);
            break;
        case IMGUI_TRACE_FRAME:
            sprintf(line, "R %.9g %.17g\n", (double)event->x,
                    event->time);
            break;
        default:
            imgui_internal_release(&trace->allocator, generated);
            return IMGUI_RESULT_CORRUPT_DATA;
        }
        if (!imgui_trace_output_append(generated, estimated, &length, line)) {
            imgui_internal_release(&trace->allocator, generated);
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
    }
    if (required != NULL) *required = length + 1;
    if (buffer == NULL || capacity < length + 1) {
        if (buffer != NULL && capacity != 0) buffer[0] = '\0';
        imgui_internal_release(&trace->allocator, generated);
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    memcpy(buffer, generated, length + 1);
    imgui_internal_release(&trace->allocator, generated);
    return IMGUI_RESULT_OK;
}

static int imgui_trace_hex_value(char value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

static int imgui_trace_read_line(const char *data, size_t length,
                                 size_t *offset, char *line,
                                 size_t line_capacity)
{
    size_t start;
    size_t line_length;
    if (data == NULL || offset == NULL || line == NULL ||
        *offset >= length) return 0;
    start = *offset;
    while (*offset < length && data[*offset] != '\n') ++*offset;
    line_length = *offset - start;
    if (line_length >= line_capacity) return -1;
    memcpy(line, data + start, line_length);
    line[line_length] = '\0';
    if (*offset < length) ++*offset;
    return 1;
}

imgui_result imgui_trace_load(imgui_trace *trace, const char *data,
                              size_t length)
{
    imgui_trace *temporary;
    imgui_trace_event event;
    char line[640];
    char *text;
    size_t offset;
    size_t text_length;
    size_t index;
    int parsed;
    int first;
    int second;
    imgui_result result;
    if (trace == NULL || data == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (imgui_trace_create(&trace->allocator, &temporary) != IMGUI_RESULT_OK) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    offset = 0;
    if (imgui_trace_read_line(data, length, &offset, line, sizeof(line)) != 1 ||
        strcmp(line, "IMGUI_C89_TRACE 1") != 0) {
        imgui_trace_destroy(temporary);
        return IMGUI_RESULT_CORRUPT_DATA;
    }
    while ((parsed = imgui_trace_read_line(data, length, &offset, line,
                                           sizeof(line))) > 0) {
        imgui_trace_event_init(&event, IMGUI_TRACE_FOCUS);
        if (line[0] == 'P') {
            if (sscanf(line + 1, "%f %f", &event.x, &event.y) != 2) {
                result = IMGUI_RESULT_CORRUPT_DATA; goto load_fail;
            }
            event.type = IMGUI_TRACE_MOUSE_POSITION;
        } else if (line[0] == 'B' || line[0] == 'K') {
            if (sscanf(line + 1, "%d %d", &event.code, &first) != 2) {
                result = IMGUI_RESULT_CORRUPT_DATA; goto load_fail;
            }
            event.type = line[0] == 'B' ? IMGUI_TRACE_MOUSE_BUTTON :
                                         IMGUI_TRACE_KEY;
            event.down = first ? IMGUI_TRUE : IMGUI_FALSE;
        } else if (line[0] == 'W') {
            if (sscanf(line + 1, "%f %f", &event.x, &event.y) != 2) {
                result = IMGUI_RESULT_CORRUPT_DATA; goto load_fail;
            }
            event.type = IMGUI_TRACE_MOUSE_WHEEL;
        } else if (line[0] == 'A') {
            if (sscanf(line + 1, "%d %d %f", &event.code, &first,
                       &event.analog_value) != 3) {
                result = IMGUI_RESULT_CORRUPT_DATA; goto load_fail;
            }
            event.type = IMGUI_TRACE_KEY_ANALOG;
            event.down = first ? IMGUI_TRUE : IMGUI_FALSE;
        } else if (line[0] == 'C') {
            if (sscanf(line + 1, "%lu", &event.codepoint) != 1) {
                result = IMGUI_RESULT_CORRUPT_DATA; goto load_fail;
            }
            event.type = IMGUI_TRACE_CODEPOINT;
        } else if (line[0] == 'F') {
            if (sscanf(line + 1, "%d", &first) != 1) {
                result = IMGUI_RESULT_CORRUPT_DATA; goto load_fail;
            }
            event.type = IMGUI_TRACE_FOCUS;
            event.down = first ? IMGUI_TRUE : IMGUI_FALSE;
        } else if (line[0] == 'R') {
            if (sscanf(line + 1, "%f %lf", &event.x, &event.time) != 2 ||
                event.x <= 0.0f) {
                result = IMGUI_RESULT_CORRUPT_DATA; goto load_fail;
            }
            event.type = IMGUI_TRACE_FRAME;
        } else if (line[0] == 'T') {
            text = line + 1;
            while (*text == ' ') ++text;
            text_length = strlen(text);
            if (text_length > 0 && text[text_length - 1] == '\r') {
                text[--text_length] = '\0';
            }
            if ((text_length & 1U) != 0 || text_length / 2 >=
                sizeof(event.text)) {
                result = IMGUI_RESULT_CORRUPT_DATA; goto load_fail;
            }
            for (index = 0; index < text_length / 2; ++index) {
                first = imgui_trace_hex_value(text[index * 2]);
                second = imgui_trace_hex_value(text[index * 2 + 1]);
                if (first < 0 || second < 0) {
                    result = IMGUI_RESULT_CORRUPT_DATA; goto load_fail;
                }
                event.text[index] = (char)((first << 4) | second);
            }
            event.text_length = text_length / 2;
            event.text[event.text_length] = '\0';
            event.type = IMGUI_TRACE_TEXT_UTF8;
        } else {
            result = IMGUI_RESULT_CORRUPT_DATA; goto load_fail;
        }
        if (imgui_trace_append_event(temporary, &event) != IMGUI_RESULT_OK) {
            result = IMGUI_RESULT_OUT_OF_MEMORY; goto load_fail;
        }
    }
    if (parsed < 0) {
        result = IMGUI_RESULT_CORRUPT_DATA; goto load_fail;
    }
    imgui_internal_release(&trace->allocator, trace->events);
    trace->events = temporary->events;
    trace->count = temporary->count;
    trace->capacity = temporary->capacity;
    temporary->events = NULL;
    imgui_trace_destroy(temporary);
    return IMGUI_RESULT_OK;

load_fail:
    imgui_trace_destroy(temporary);
    return result;
}

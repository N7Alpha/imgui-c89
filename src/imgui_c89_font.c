#include "imgui_c89_internal.h"
#include "../include/imgui_c89_font.h"

#include <limits.h>
#include <float.h>
#include <string.h>

static imgui_bool imgui_font_float_is_finite(float value)
{
    return value == value && value <= FLT_MAX && value >= -FLT_MAX;
}

/* Keep stb's implementation in exactly one translation unit. */
#define STB_TRUETYPE_IMPLEMENTATION
#include "../third_party/stb_truetype.h"

struct imgui_font {
    imgui_font_atlas *atlas;
    stbtt_fontinfo info;
    float scale;
    float ascent;
    float descent;
    float line_gap;
    unsigned char *font_data;
    size_t font_data_size;
    imgui_font_config config;
    imgui_font_glyph *glyphs;
    int glyph_count;
};

struct imgui_font_atlas {
    imgui_context *owner;
    imgui_allocator allocator;
    struct imgui_font_atlas *next;
    imgui_font **fonts;
    int font_count;
    int font_capacity;
    unsigned char *pixels;
    int width;
    int height;
    imgui_bool built;
};

static void imgui_font_zero_pixels(imgui_font_atlas *atlas)
{
    size_t size;
    if (atlas == NULL || atlas->pixels == NULL) return;
    if (atlas->width <= 0 || atlas->height <= 0 ||
        (size_t)atlas->width > (size_t)-1 / (size_t)atlas->height) return;
    size = (size_t)atlas->width * (size_t)atlas->height;
    memset(atlas->pixels, 0, size);
}

static imgui_bool imgui_font_prepare_glyph(int *x,
                                           int *y,
                                           int *row_height,
                                           int *atlas_height,
                                           int width,
                                           int padding,
                                           int glyph_width,
                                           int glyph_height)
{
    if (x == NULL || y == NULL || row_height == NULL ||
        atlas_height == NULL || width <= 0 || padding < 0 ||
        padding > INT_MAX / 2 || glyph_width < 0 || glyph_height < 0 ||
        padding > width || glyph_width > width - padding * 2) {
        return IMGUI_FALSE;
    }
    if (*x > width - padding ||
        glyph_width > width - *x - padding) {
        if (*y > INT_MAX - *row_height - padding) return IMGUI_FALSE;
        *x = padding;
        *y += *row_height + padding;
        *row_height = 0;
    }
    if (*y > INT_MAX - glyph_height - padding) return IMGUI_FALSE;
    if (*y + glyph_height + padding > *atlas_height) {
        *atlas_height = *y + glyph_height + padding;
    }
    return IMGUI_TRUE;
}

void imgui_font_config_init(imgui_font_config *config)
{
    if (config == NULL) return;
    memset(config, 0, sizeof(*config));
    config->struct_size = sizeof(*config);
    config->pixel_height = 16.0f;
    config->first_codepoint = 32UL;
    config->last_codepoint = 255UL;
    config->atlas_width = 512;
    config->padding = 1;
    config->font_index = 0;
}

imgui_result imgui_font_atlas_create(imgui_context *ctx,
                                     imgui_font_atlas **out_atlas)
{
    imgui_font_atlas *atlas;
    if (ctx == NULL || out_atlas == NULL) {
        imgui_internal_report(ctx, IMGUI_ERROR_INVALID_ARGUMENT,
                              "font atlas create requires context and output");
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    *out_atlas = NULL;
    atlas = (imgui_font_atlas *)imgui_internal_allocate(
        &ctx->allocator, sizeof(*atlas));
    if (atlas == NULL) {
        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                              "font atlas allocation failed");
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    memset(atlas, 0, sizeof(*atlas));
    atlas->owner = ctx;
    atlas->allocator = ctx->allocator;
    atlas->next = ctx->font_atlases;
    ctx->font_atlases = atlas;
    *out_atlas = atlas;
    return IMGUI_RESULT_OK;
}

void imgui_font_atlas_destroy(imgui_font_atlas *atlas)
{
    int index;
    imgui_context *ctx;
    imgui_font_atlas *cursor;
    imgui_font_atlas *previous;
    imgui_allocator allocator;
    if (atlas == NULL) return;
    ctx = atlas->owner;
    allocator = atlas->allocator;
    if (ctx != NULL && ctx->font != NULL && ctx->font->atlas == atlas) {
        ctx->font = NULL;
        ctx->font_texture = NULL;
    }
    if (ctx != NULL) {
        previous = NULL;
        cursor = ctx->font_atlases;
        while (cursor != NULL && cursor != atlas) {
            previous = cursor;
            cursor = cursor->next;
        }
        if (cursor == atlas) {
            if (previous != NULL) previous->next = atlas->next;
            else ctx->font_atlases = atlas->next;
        }
    }
    for (index = 0; index < atlas->font_count; ++index) {
        if (atlas->fonts[index] != NULL) {
            imgui_internal_release(&allocator,
                                   atlas->fonts[index]->font_data);
            imgui_internal_release(&allocator,
                                   atlas->fonts[index]->glyphs);
            imgui_internal_release(&allocator, atlas->fonts[index]);
        }
    }
    imgui_internal_release(&allocator, atlas->fonts);
    imgui_internal_release(&allocator, atlas->pixels);
    imgui_internal_release(&allocator, atlas);
}

void imgui_font_atlas_detach_context(imgui_context *ctx)
{
    imgui_font_atlas *atlas;
    if (ctx == NULL) return;
    atlas = ctx->font_atlases;
    while (atlas != NULL) {
        atlas->owner = NULL;
        atlas = atlas->next;
    }
    ctx->font_atlases = NULL;
    ctx->font = NULL;
    ctx->font_texture = NULL;
}

imgui_result imgui_font_atlas_add_ttf(imgui_font_atlas *atlas,
                                      const void *data, size_t data_size,
                                      const imgui_font_config *config,
                                      imgui_font **out_font)
{
    imgui_font_config local_config;
    size_t config_size;
    imgui_font *font;
    unsigned char *copy;
    int offset;
    int ascent;
    int descent;
    int line_gap;
    size_t glyph_storage_size;
    size_t glyph_count;
    imgui_context *ctx;
    imgui_font **new_fonts;
    if (atlas == NULL || data == NULL || data_size == 0 || out_font == NULL) {
        if (atlas != NULL) imgui_internal_report(
            atlas->owner, IMGUI_ERROR_INVALID_ARGUMENT,
            "font add requires font bytes and output");
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (atlas->owner == NULL) return IMGUI_RESULT_INVALID_STATE;
    ctx = atlas->owner;
    imgui_font_config_init(&local_config);
    if (config != NULL) {
        if (config->struct_size < sizeof(config->struct_size)) {
            imgui_internal_report(ctx, IMGUI_ERROR_INVALID_ARGUMENT,
                                  "font configuration is invalid");
            return IMGUI_RESULT_INVALID_ARGUMENT;
        }
        config_size = config->struct_size < sizeof(local_config) ?
            config->struct_size : sizeof(local_config);
        memcpy(&local_config, config, config_size);
        if (!imgui_font_float_is_finite(local_config.pixel_height) ||
            local_config.pixel_height <= 0.0f ||
            local_config.first_codepoint > local_config.last_codepoint ||
            local_config.last_codepoint > 0x10ffffUL ||
            local_config.last_codepoint > (unsigned long)INT_MAX ||
            local_config.atlas_width <= 0 || local_config.padding < 0 ||
            local_config.font_index < 0) {
            imgui_internal_report(ctx, IMGUI_ERROR_INVALID_ARGUMENT,
                                  "font configuration is invalid");
            return IMGUI_RESULT_INVALID_ARGUMENT;
        }
    }
    glyph_count = (size_t)(local_config.last_codepoint -
                           local_config.first_codepoint) + 1U;
    if (glyph_count > (size_t)INT_MAX ||
        glyph_count > (size_t)-1 / sizeof(*font->glyphs)) {
        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                              "font glyph storage size overflows");
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    copy = (unsigned char *)imgui_internal_allocate(&ctx->allocator, data_size);
    if (copy == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    memcpy(copy, data, data_size);
    font = (imgui_font *)imgui_internal_allocate(&ctx->allocator,
                                                  sizeof(*font));
    if (font == NULL) {
        imgui_internal_release(&ctx->allocator, copy);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    memset(font, 0, sizeof(*font));
    offset = stbtt_GetFontOffsetForIndex(copy, local_config.font_index);
    if (offset < 0 || !stbtt_InitFont(&font->info, copy, offset)) {
        imgui_internal_release(&ctx->allocator, copy);
        imgui_internal_release(&ctx->allocator, font);
        imgui_internal_report(ctx, IMGUI_ERROR_CORRUPT_DATA,
                              "font data is not a readable TrueType font");
        return IMGUI_RESULT_CORRUPT_DATA;
    }
    font->atlas = atlas;
    font->font_data = copy;
    font->font_data_size = data_size;
    font->config = local_config;
    font->scale = stbtt_ScaleForPixelHeight(&font->info,
                                            local_config.pixel_height);
    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &line_gap);
    font->ascent = (float)ascent * font->scale;
    font->descent = (float)descent * font->scale;
    font->line_gap = (float)line_gap * font->scale;
    glyph_storage_size = glyph_count * sizeof(*font->glyphs);
    font->glyphs = (imgui_font_glyph *)imgui_internal_allocate(
        &ctx->allocator, glyph_storage_size);
    if (font->glyphs == NULL) {
        imgui_internal_release(&ctx->allocator, copy);
        imgui_internal_release(&ctx->allocator, font);
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    memset(font->glyphs, 0, glyph_storage_size);
    font->glyph_count = (int)glyph_count;
    if (atlas->font_count == atlas->font_capacity) {
        int new_capacity = atlas->font_capacity == 0 ? 4 :
                           atlas->font_capacity;
        if (atlas->font_capacity != 0) {
            if (atlas->font_capacity > 0x3fffffff) {
                imgui_internal_release(&ctx->allocator, copy);
                imgui_internal_release(&ctx->allocator, font->glyphs);
                imgui_internal_release(&ctx->allocator, font);
                return IMGUI_RESULT_OUT_OF_MEMORY;
            }
            new_capacity *= 2;
        }
        if ((size_t)new_capacity > (size_t)-1 / sizeof(*new_fonts)) {
            imgui_internal_release(&ctx->allocator, copy);
            imgui_internal_release(&ctx->allocator, font->glyphs);
            imgui_internal_release(&ctx->allocator, font);
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        new_fonts = (imgui_font **)imgui_internal_allocate(
            &ctx->allocator, (size_t)new_capacity * sizeof(*new_fonts));
        if (new_fonts == NULL) {
            imgui_internal_release(&ctx->allocator, copy);
            imgui_internal_release(&ctx->allocator, font->glyphs);
            imgui_internal_release(&ctx->allocator, font);
            return IMGUI_RESULT_OUT_OF_MEMORY;
        }
        if (atlas->fonts != NULL) memcpy(new_fonts, atlas->fonts,
            (size_t)atlas->font_count * sizeof(*new_fonts));
        imgui_internal_release(&ctx->allocator, atlas->fonts);
        atlas->fonts = new_fonts;
        atlas->font_capacity = new_capacity;
    }
    atlas->fonts[atlas->font_count++] = font;
    atlas->built = IMGUI_FALSE;
    *out_font = font;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_font_add_codepoint_range(
    imgui_font *font, unsigned long first_codepoint,
    unsigned long last_codepoint)
{
    unsigned long new_first;
    unsigned long new_last;
    size_t glyph_count;
    size_t glyph_bytes;
    imgui_font_glyph *new_glyphs;
    size_t old_offset;
    if (font == NULL || font->atlas == NULL || font->atlas->owner == NULL ||
        first_codepoint > last_codepoint || last_codepoint > 0x10ffffUL ||
        last_codepoint > (unsigned long)INT_MAX) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    new_first = first_codepoint < font->config.first_codepoint ?
        first_codepoint : font->config.first_codepoint;
    new_last = last_codepoint > font->config.last_codepoint ?
        last_codepoint : font->config.last_codepoint;
    if (new_first == font->config.first_codepoint &&
        new_last == font->config.last_codepoint) {
        return IMGUI_RESULT_OK;
    }
    glyph_count = (size_t)(new_last - new_first) + 1U;
    if (glyph_count > (size_t)INT_MAX ||
        glyph_count > (size_t)-1 / sizeof(*new_glyphs)) {
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    glyph_bytes = glyph_count * sizeof(*new_glyphs);
    new_glyphs = (imgui_font_glyph *)imgui_internal_allocate(
        &font->atlas->allocator, glyph_bytes);
    if (new_glyphs == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    memset(new_glyphs, 0, glyph_bytes);
    old_offset = (size_t)(font->config.first_codepoint - new_first);
    memcpy(new_glyphs + old_offset, font->glyphs,
           (size_t)font->glyph_count * sizeof(*new_glyphs));
    imgui_internal_release(&font->atlas->allocator, font->glyphs);
    font->glyphs = new_glyphs;
    font->glyph_count = (int)glyph_count;
    font->config.first_codepoint = new_first;
    font->config.last_codepoint = new_last;
    font->atlas->built = IMGUI_FALSE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_font_atlas_build(imgui_font_atlas *atlas)
{
    int font_index;
    int codepoint;
    int x;
    int y;
    int row_height;
    int width;
    int height;
    int padding;
    imgui_font *font;
    int advance;
    int bearing;
    int glyph_width;
    int glyph_height;
    int bitmap_left;
    int bitmap_top;
    unsigned char *bitmap;
    imgui_context *ctx;
    if (atlas == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    if (atlas->owner == NULL) return IMGUI_RESULT_INVALID_STATE;
    if (atlas->built) return IMGUI_RESULT_OK;
    ctx = atlas->owner;
    width = 512;
    padding = 1;
    if (atlas->font_count > 0) {
        width = atlas->fonts[0]->config.atlas_width;
        padding = atlas->fonts[0]->config.padding;
    }
    if (width <= 0) return IMGUI_RESULT_INVALID_ARGUMENT;
    height = 1;
    x = padding;
    y = padding;
    row_height = 0;
    for (font_index = 0; font_index < atlas->font_count; ++font_index) {
        font = atlas->fonts[font_index];
        for (codepoint = (int)font->config.first_codepoint;
             codepoint <= (int)font->config.last_codepoint; ++codepoint) {
            stbtt_GetCodepointHMetrics(&font->info, codepoint,
                                       &advance, &bearing);
            bitmap = stbtt_GetCodepointBitmap(&font->info, 0.0f,
                                              font->scale, codepoint,
                                              &glyph_width, &glyph_height,
                                              &bitmap_left, &bitmap_top);
            if (!imgui_font_prepare_glyph(&x, &y, &row_height, &height,
                                          width, padding, glyph_width,
                                          glyph_height)) {
                if (bitmap != NULL) stbtt_FreeBitmap(bitmap, NULL);
                imgui_internal_report(ctx, IMGUI_ERROR_INVALID_ARGUMENT,
                                      "font atlas dimensions are invalid");
                return IMGUI_RESULT_INVALID_ARGUMENT;
            }
            x += glyph_width + padding;
            if (glyph_height > row_height) row_height = glyph_height;
            if (bitmap != NULL) stbtt_FreeBitmap(bitmap, NULL);
            if (codepoint == (int)font->config.last_codepoint) break;
        }
    }
    imgui_internal_release(&ctx->allocator, atlas->pixels);
    if ((size_t)width > (size_t)-1 / (size_t)height) {
        imgui_internal_report(ctx, IMGUI_ERROR_OUT_OF_MEMORY,
                              "font atlas dimensions overflow allocation size");
        atlas->pixels = NULL;
        atlas->built = IMGUI_FALSE;
        return IMGUI_RESULT_OUT_OF_MEMORY;
    }
    atlas->pixels = (unsigned char *)imgui_internal_allocate(
        &ctx->allocator, (size_t)width * (size_t)height);
    if (atlas->pixels == NULL) return IMGUI_RESULT_OUT_OF_MEMORY;
    atlas->width = width;
    atlas->height = height;
    imgui_font_zero_pixels(atlas);
    x = padding;
    y = padding;
    row_height = 0;
    for (font_index = 0; font_index < atlas->font_count; ++font_index) {
        font = atlas->fonts[font_index];
        for (codepoint = (int)font->config.first_codepoint;
             codepoint <= (int)font->config.last_codepoint; ++codepoint) {
            int glyph_index = codepoint - (int)font->config.first_codepoint;
            stbtt_GetCodepointHMetrics(&font->info, codepoint,
                                       &advance, &bearing);
            bitmap = stbtt_GetCodepointBitmap(&font->info, 0.0f,
                                              font->scale, codepoint,
                                              &glyph_width, &glyph_height,
                                              &bitmap_left, &bitmap_top);
            if (!imgui_font_prepare_glyph(&x, &y, &row_height, &height,
                                          width, padding, glyph_width,
                                          glyph_height)) {
                if (bitmap != NULL) stbtt_FreeBitmap(bitmap, NULL);
                imgui_internal_release(&ctx->allocator, atlas->pixels);
                atlas->pixels = NULL;
                atlas->built = IMGUI_FALSE;
                imgui_internal_report(ctx, IMGUI_ERROR_INVALID_ARGUMENT,
                                      "font atlas dimensions are invalid");
                return IMGUI_RESULT_INVALID_ARGUMENT;
            }
            font->glyphs[glyph_index].codepoint = (unsigned long)codepoint;
            font->glyphs[glyph_index].advance_x = (float)advance * font->scale;
            font->glyphs[glyph_index].offset_x = bitmap_left;
            /* stbtt reports the glyph top relative to the baseline in the
               same y-down convention used by ImGui's ImFontGlyph. */
            font->glyphs[glyph_index].offset_y = bitmap_top;
            font->glyphs[glyph_index].width = glyph_width;
            font->glyphs[glyph_index].height = glyph_height;
            font->glyphs[glyph_index].uv.x1 = (float)x / (float)width;
            font->glyphs[glyph_index].uv.y1 = (float)y / (float)height;
            font->glyphs[glyph_index].uv.x2 = (float)(x + glyph_width) /
                                              (float)width;
            font->glyphs[glyph_index].uv.y2 = (float)(y + glyph_height) /
                                              (float)height;
            if (bitmap != NULL) {
                int row;
                int column;
                for (row = 0; row < glyph_height; ++row) {
                    for (column = 0; column < glyph_width; ++column) {
                        atlas->pixels[(size_t)(y + row) * (size_t)width +
                                      (size_t)(x + column)] =
                            bitmap[row * glyph_width + column];
                    }
                }
                stbtt_FreeBitmap(bitmap, NULL);
            }
            x += glyph_width + padding;
            if (glyph_height > row_height) row_height = glyph_height;
            if (codepoint == (int)font->config.last_codepoint) break;
        }
    }
    atlas->built = IMGUI_TRUE;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_font_atlas_get_pixels(const imgui_font_atlas *atlas,
                                         imgui_font_atlas_pixels *out_pixels)
{
    if (atlas == NULL || out_pixels == NULL || !atlas->built) {
        return IMGUI_RESULT_INVALID_STATE;
    }
    out_pixels->pixels = atlas->pixels;
    out_pixels->width = atlas->width;
    out_pixels->height = atlas->height;
    out_pixels->bytes_per_pixel = 1;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_font_atlas_upload(imgui_context *ctx,
                                     const imgui_font_atlas *atlas,
                                     imgui_texture *texture)
{
    imgui_texture_update_command ordered_update;
    if (ctx == NULL || atlas == NULL || texture == NULL ||
        atlas->owner != ctx || !atlas->built || texture->owner != ctx ||
        !texture->alive || texture->desc.format != IMGUI_TEXTURE_FORMAT_ALPHA8 ||
        texture->desc.width != atlas->width ||
        texture->desc.height != atlas->height) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (ctx->frame_state == IMGUI_INTERNAL_FRAME_BUILDING &&
        imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURE_UPDATE)) {
        memset(&ordered_update, 0, sizeof(ordered_update));
        ordered_update.texture = texture;
        ordered_update.region.x = 0;
        ordered_update.region.y = 0;
        ordered_update.region.width = atlas->width;
        ordered_update.region.height = atlas->height;
        ordered_update.format = IMGUI_TEXTURE_FORMAT_ALPHA8;
        ordered_update.pixels = atlas->pixels;
        ordered_update.pitch = (size_t)atlas->width;
        return imgui_draw_list_add_texture_update(
            ctx, &ctx->default_draw_list, &ordered_update);
    }
    return imgui_texture_update(ctx, texture, 0, 0, atlas->width,
                                atlas->height, atlas->pixels,
                                (size_t)atlas->width);
}

imgui_result imgui_font_bind(imgui_context *ctx, const imgui_font *font,
                             imgui_texture *texture)
{
    if (ctx == NULL || font == NULL || texture == NULL ||
        font->atlas == NULL || font->atlas->owner != ctx ||
        texture->owner != ctx || !texture->alive ||
        texture->desc.format != IMGUI_TEXTURE_FORMAT_ALPHA8) {
        return IMGUI_RESULT_INVALID_ARGUMENT;
    }
    if (!imgui_renderer_has_capability(ctx, IMGUI_RENDERER_CAP_TEXTURES)) {
        return IMGUI_RESULT_UNSUPPORTED;
    }
    ctx->font = font;
    ctx->font_texture = texture;
    return IMGUI_RESULT_OK;
}

imgui_result imgui_font_unbind(imgui_context *ctx)
{
    if (ctx == NULL) return IMGUI_RESULT_INVALID_ARGUMENT;
    ctx->font = NULL;
    ctx->font_texture = NULL;
    return IMGUI_RESULT_OK;
}

const imgui_font_glyph *imgui_font_find_glyph(const imgui_font *font,
                                              unsigned long codepoint)
{
    unsigned long first;
    unsigned long last;
    if (font == NULL) return NULL;
    first = font->config.first_codepoint;
    last = font->config.last_codepoint;
    if (codepoint < first || codepoint > last) return NULL;
    return &font->glyphs[codepoint - first];
}

float imgui_font_get_line_height(const imgui_font *font)
{
    return font != NULL ? font->config.pixel_height : 0.0f;
}

float imgui_font_get_ascent(const imgui_font *font)
{
    return font != NULL ? font->ascent : 0.0f;
}

float imgui_font_get_kerning(const imgui_font *font,
                             unsigned long left_codepoint,
                             unsigned long right_codepoint)
{
    int advance;
    if (font == NULL || left_codepoint > 0x10ffffUL ||
        right_codepoint > 0x10ffffUL) {
        return 0.0f;
    }
    advance = stbtt_GetCodepointKernAdvance(&font->info,
                                             (int)left_codepoint,
                                             (int)right_codepoint);
    return (float)advance * font->scale;
}

static size_t imgui_font_utf8_decode(const char *cursor, const char *end,
                                     unsigned long *out_codepoint)
{
    const unsigned char *bytes;
    size_t remaining;
    unsigned char first;
    if (cursor == NULL || end == NULL || out_codepoint == NULL ||
        cursor >= end) {
        return 0;
    }
    bytes = (const unsigned char *)cursor;
    first = bytes[0];
    remaining = (size_t)(end - cursor);
    if (first < 0x80U) {
        *out_codepoint = (unsigned long)first;
        return 1;
    }
    if (first >= 0xc2U && first <= 0xdfU && remaining >= 2 &&
        (bytes[1] & 0xc0U) == 0x80U) {
        *out_codepoint = ((unsigned long)(first & 0x1fU) << 6) |
                         (unsigned long)(bytes[1] & 0x3fU);
        return 2;
    }
    if (first >= 0xe0U && first <= 0xefU && remaining >= 3 &&
        (bytes[1] & 0xc0U) == 0x80U &&
        (bytes[2] & 0xc0U) == 0x80U &&
        !(first == 0xe0U && bytes[1] < 0xa0U) &&
        !(first == 0xedU && bytes[1] >= 0xa0U)) {
        *out_codepoint = ((unsigned long)(first & 0x0fU) << 12) |
                         ((unsigned long)(bytes[1] & 0x3fU) << 6) |
                         (unsigned long)(bytes[2] & 0x3fU);
        return 3;
    }
    if (first >= 0xf0U && first <= 0xf4U && remaining >= 4 &&
        (bytes[1] & 0xc0U) == 0x80U &&
        (bytes[2] & 0xc0U) == 0x80U &&
        (bytes[3] & 0xc0U) == 0x80U &&
        !(first == 0xf0U && bytes[1] < 0x90U) &&
        !(first == 0xf4U && bytes[1] >= 0x90U)) {
        *out_codepoint = ((unsigned long)(first & 0x07U) << 18) |
                         ((unsigned long)(bytes[1] & 0x3fU) << 12) |
                         ((unsigned long)(bytes[2] & 0x3fU) << 6) |
                         (unsigned long)(bytes[3] & 0x3fU);
        return 4;
    }
    *out_codepoint = 0xfffdUL;
    return 1;
}

static float imgui_font_word_width(const imgui_font *font,
                                   const char *begin, const char *end)
{
    const char *cursor;
    unsigned long codepoint;
    unsigned long previous;
    size_t step;
    float width;
    const imgui_font_glyph *glyph;
    imgui_bool have_previous;
    if (font == NULL || begin == NULL || end == NULL) return 0.0f;
    cursor = begin;
    previous = 0;
    width = 0.0f;
    have_previous = IMGUI_FALSE;
    while (cursor < end && *cursor != ' ' && *cursor != '\t' &&
           *cursor != '\n' && *cursor != '\r') {
        step = imgui_font_utf8_decode(cursor, end, &codepoint);
        if (step == 0) break;
        glyph = imgui_font_find_glyph(font, codepoint);
        if (glyph == NULL) glyph = imgui_font_find_glyph(font, '?');
        if (glyph != NULL) {
            if (have_previous) width += imgui_font_get_kerning(
                font, previous, codepoint);
            width += glyph->advance_x;
            previous = glyph->codepoint;
            have_previous = IMGUI_TRUE;
        }
        cursor += step;
    }
    return width;
}

imgui_vec2 imgui_font_measure_text(const imgui_font *font, const char *text,
                                   const char *text_end, float wrap_width)
{
    imgui_vec2 result;
    const char *cursor;
    unsigned long codepoint;
    size_t step;
    float x;
    float line_height;
    const imgui_font_glyph *glyph;
    unsigned long previous_codepoint;
    imgui_bool have_previous;
    result.x = 0.0f;
    result.y = 0.0f;
    if (font == NULL || text == NULL) return result;
    cursor = text;
    x = 0.0f;
    previous_codepoint = 0;
    have_previous = IMGUI_FALSE;
    if (text_end == NULL) text_end = text + strlen(text);
    line_height = font->config.pixel_height;
    while (cursor < text_end) {
        if (*cursor == '\n') {
            if (x > result.x) result.x = x;
            x = 0.0f;
            have_previous = IMGUI_FALSE;
            cursor += 1;
            continue;
        }
        step = imgui_font_utf8_decode(cursor, text_end, &codepoint);
        if (step == 0) break;
        glyph = imgui_font_find_glyph(font, codepoint);
        if (glyph == NULL) glyph = imgui_font_find_glyph(font, '?');
        if (glyph != NULL && wrap_width > 0.0f && x > 0.0f &&
            codepoint != (unsigned long)' ' &&
            codepoint != (unsigned long)'\t' &&
            codepoint != (unsigned long)'\n' &&
            codepoint != (unsigned long)'\r') {
            float word_width = imgui_font_word_width(font, cursor, text_end);
            if (word_width <= wrap_width && x + word_width > wrap_width) {
                if (x > result.x) result.x = x;
                x = 0.0f;
                result.y += line_height;
                have_previous = IMGUI_FALSE;
            }
        }
        if (glyph != NULL && wrap_width > 0.0f && x > 0.0f &&
            (codepoint == (unsigned long)' ' ||
             codepoint == (unsigned long)'\t') &&
            x + glyph->advance_x > wrap_width) {
            if (x > result.x) result.x = x;
            x = 0.0f;
            result.y += line_height;
            have_previous = IMGUI_FALSE;
            cursor += step;
            continue;
        }
        if (have_previous) {
            x += imgui_font_get_kerning(font, previous_codepoint,
                                        codepoint);
        }
        if (glyph != NULL && wrap_width > 0.0f && x > 0.0f &&
            x + glyph->advance_x > wrap_width) {
            if (x > result.x) result.x = x;
            x = 0.0f;
            result.y += line_height;
            have_previous = IMGUI_FALSE;
        }
        if (glyph != NULL) {
            x += glyph->advance_x;
            previous_codepoint = glyph->codepoint;
            have_previous = IMGUI_TRUE;
        }
        cursor += step;
    }
    if (x > result.x) result.x = x;
    result.y += line_height;
    return result;
}

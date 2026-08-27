#ifndef IMGUI_C89_FONT_H
#define IMGUI_C89_FONT_H

#include "imgui_c89.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct imgui_font_atlas imgui_font_atlas;
typedef struct imgui_font imgui_font;

/*
 * Font baking uses the vendored stb_truetype.h TTF implementation. The stb
 * header is private to the implementation; callers only need this C89 API.
 */

typedef struct imgui_font_config {
    size_t struct_size;
    float pixel_height;
    unsigned long first_codepoint;
    unsigned long last_codepoint;
    int atlas_width;
    int padding;
    int font_index;
} imgui_font_config;

typedef struct imgui_font_glyph {
    unsigned long codepoint;
    float advance_x;
    int offset_x;
    int offset_y;
    int width;
    int height;
    imgui_rect uv;
} imgui_font_glyph;

typedef struct imgui_font_atlas_pixels {
    const unsigned char *pixels;
    int width;
    int height;
    int bytes_per_pixel;
} imgui_font_atlas_pixels;

IMGUI_API void imgui_font_config_init(imgui_font_config *config);
IMGUI_API imgui_result imgui_font_atlas_create(
    imgui_context *ctx, imgui_font_atlas **out_atlas);
IMGUI_API void imgui_font_atlas_destroy(imgui_font_atlas *atlas);
IMGUI_API imgui_result imgui_font_atlas_add_ttf(
    imgui_font_atlas *atlas, const void *data, size_t data_size,
    const imgui_font_config *config, imgui_font **out_font);
/* Extend an existing font's Unicode coverage. The atlas becomes dirty and
   must be rebuilt/uploaded after adding the range. Existing glyph metrics and
   the font object remain valid. */
IMGUI_API imgui_result imgui_font_add_codepoint_range(
    imgui_font *font, unsigned long first_codepoint,
    unsigned long last_codepoint);
IMGUI_API imgui_result imgui_font_atlas_build(imgui_font_atlas *atlas);
IMGUI_API imgui_result imgui_font_atlas_get_pixels(
    const imgui_font_atlas *atlas, imgui_font_atlas_pixels *out_pixels);
IMGUI_API imgui_result imgui_font_atlas_upload(
    imgui_context *ctx, const imgui_font_atlas *atlas, imgui_texture *texture);
IMGUI_API imgui_result imgui_font_bind(imgui_context *ctx,
                                       const imgui_font *font,
                                       imgui_texture *texture);
IMGUI_API imgui_result imgui_font_unbind(imgui_context *ctx);
IMGUI_API const imgui_font_glyph *imgui_font_find_glyph(
    const imgui_font *font, unsigned long codepoint);
IMGUI_API float imgui_font_get_line_height(const imgui_font *font);
IMGUI_API float imgui_font_get_ascent(const imgui_font *font);
IMGUI_API float imgui_font_get_kerning(const imgui_font *font,
                                       unsigned long left_codepoint,
                                       unsigned long right_codepoint);
IMGUI_API imgui_vec2 imgui_font_measure_text(
    const imgui_font *font, const char *text, const char *text_end,
    float wrap_width);

#ifdef __cplusplus
}
#endif

#endif

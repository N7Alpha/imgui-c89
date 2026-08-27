/*
 * Blind, idiomatic C89 proposal for ImDrawList_AddPolyline.
 *
 * This is an authorship artifact.  Its deliberately small abstract host
 * contract is included here so the implementation can later be adapted to
 * the concrete translated names without depending on generated structures.
 */

#include <math.h>

typedef unsigned int DrawU32;
typedef unsigned short DrawIdx; /* Or the build's configured index type. */

typedef struct Vec2 {
    float x;
    float y;
} Vec2;

typedef struct Vec4 {
    float x;
    float y;
    float z;
    float w;
} Vec4;

typedef struct DrawVert {
    Vec2 pos;
    Vec2 uv;
    DrawU32 col;
} DrawVert;

typedef struct DrawShared {
    Vec2 white_uv;
    const Vec4 *line_uv;       /* Indexed by integral thickness. */
    int line_uv_count;
} DrawShared;

typedef struct Context Context;

typedef struct DrawList {
    unsigned int flags;
    float fringe_scale;
    unsigned int current_vtx;
    DrawIdx *idx_write;
    DrawVert *vtx_write;
    DrawShared *shared;
} DrawList;

/* Updates idx_write and vtx_write to point at the reserved ranges. */
static void draw_reserve(DrawList *dl, int index_count, int vertex_count);

/* Returns suitably aligned disposable storage from a reusable shared buffer. */
static void *draw_scratch(Context *ctx, unsigned int byte_count);

/* Reports a user/API error; the host implementation may assert. */
static void draw_error(Context *ctx, const char *message);

#define DRAW_FLAG_CLOSED               0x01u
#define DRAWLIST_FLAG_AA_LINES         0x01u
#define DRAWLIST_FLAG_AA_LINES_USE_TEX 0x02u
#define DRAW_ALPHA_MASK                0xff000000u

static void
normalize_over_zero(float *x, float *y)
{
    float d2;
    float inv;

    d2 = (*x * *x) + (*y * *y);
    if (d2 > 0.0f) {
        inv = (float)(1.0 / sqrt((double)d2));
        *x *= inv;
        *y *= inv;
    }
}

static void
fix_join_normal(float *x, float *y)
{
    float d2;
    float inv_d2;

    d2 = (*x * *x) + (*y * *y);
    if (d2 > 0.000001f) {
        inv_d2 = 1.0f / d2;
        if (inv_d2 > 100.0f)
            inv_d2 = 100.0f;
        *x *= inv_d2;
        *y *= inv_d2;
    }
}

static void
write_vertex(DrawVert *v, float x, float y, Vec2 uv, DrawU32 col)
{
    v->pos.x = x;
    v->pos.y = y;
    v->uv = uv;
    v->col = col;
}

void
ImDrawList_AddPolyline(Context *ctx, DrawList *dl,
                       const Vec2 *points, int point_count,
                       DrawU32 color, float thickness,
                       unsigned int flags)
{
    int closed;
    int segment_count;
    int thick_line;
    float aa_size;
    DrawU32 transparent_color;
    Vec2 white_uv;

    if (point_count < 2 || (color & DRAW_ALPHA_MASK) == 0)
        return;

    /* Detect the common swapped thickness/flags call before any reservation. */
    if ((flags & ~DRAW_FLAG_CLOSED) != 0u) {
        draw_error(ctx,
            "Invalid draw flags: thickness and flags arguments may be swapped");
        return;
    }

    closed = (flags & DRAW_FLAG_CLOSED) != 0;
    segment_count = closed ? point_count : point_count - 1;
    white_uv = dl->shared->white_uv;

    if ((dl->flags & DRAWLIST_FLAG_AA_LINES) != 0) {
        int integer_thickness;
        float fractional_thickness;
        int use_texture;
        int vertex_stride;
        int index_count;
        int vertex_count;
        Vec2 *normals;
        Vec2 *expanded;
        unsigned int scratch_count;
        int i1;
        int i2;
        float dx;
        float dy;
        float half_width;
        unsigned int idx1;
        unsigned int idx2;

        aa_size = dl->fringe_scale;
        transparent_color = color & ~DRAW_ALPHA_MASK;

        /* This test deliberately precedes the minimum-thickness clamp. */
        thick_line = thickness > aa_size;
        if (thickness < 1.0f)
            thickness = 1.0f;

        integer_thickness = (int)thickness;
        fractional_thickness = thickness - (float)integer_thickness;

        use_texture =
            (dl->flags & DRAWLIST_FLAG_AA_LINES_USE_TEX) != 0 &&
            integer_thickness >= 0 &&
            integer_thickness < dl->shared->line_uv_count &&
            fractional_thickness <= 0.00001f &&
            aa_size == 1.0f;

        vertex_stride = use_texture ? 2 : (thick_line ? 4 : 3);
        index_count = segment_count *
                      (use_texture ? 6 : (thick_line ? 18 : 12));
        vertex_count = point_count * vertex_stride;

        draw_reserve(dl, index_count, vertex_count);

        /*
         * expanded uses two Vec2 per point in texture/thin mode and four
         * per point in thick mode.  Normals occupy the first point_count.
         */
        scratch_count = (unsigned int)point_count *
                        (unsigned int)(1 + (thick_line && !use_texture
                                             ? 4 : 2));
        normals = (Vec2 *)draw_scratch(
            ctx, scratch_count * (unsigned int)sizeof(Vec2));
        expanded = normals + point_count;

        for (i1 = 0; i1 < segment_count; ++i1) {
            i2 = i1 + 1;
            if (i2 == point_count)
                i2 = 0;

            dx = points[i2].x - points[i1].x;
            dy = points[i2].y - points[i1].y;
            normalize_over_zero(&dx, &dy);

            normals[i1].x = dy;
            normals[i1].y = -dx;
        }

        if (!closed)
            normals[point_count - 1] = normals[point_count - 2];

        if (use_texture || !thick_line) {
            half_width = use_texture
                       ? thickness * 0.5f + 1.0f
                       : aa_size;

            if (!closed) {
                dx = normals[0].x * half_width;
                dy = normals[0].y * half_width;
                expanded[0].x = points[0].x + dx;
                expanded[0].y = points[0].y + dy;
                expanded[1].x = points[0].x - dx;
                expanded[1].y = points[0].y - dy;

                i1 = point_count - 1;
                dx = normals[i1].x * half_width;
                dy = normals[i1].y * half_width;
                expanded[i1 * 2].x = points[i1].x + dx;
                expanded[i1 * 2].y = points[i1].y + dy;
                expanded[i1 * 2 + 1].x = points[i1].x - dx;
                expanded[i1 * 2 + 1].y = points[i1].y - dy;
            }

            idx1 = dl->current_vtx;

            for (i1 = 0; i1 < segment_count; ++i1) {
                i2 = i1 + 1;
                if (i2 == point_count)
                    i2 = 0;

                dx = (normals[i1].x + normals[i2].x) * 0.5f;
                dy = (normals[i1].y + normals[i2].y) * 0.5f;
                fix_join_normal(&dx, &dy);
                dx *= half_width;
                dy *= half_width;

                expanded[i2 * 2].x = points[i2].x + dx;
                expanded[i2 * 2].y = points[i2].y + dy;
                expanded[i2 * 2 + 1].x = points[i2].x - dx;
                expanded[i2 * 2 + 1].y = points[i2].y - dy;

                idx2 = dl->current_vtx +
                       (unsigned int)(i2 * vertex_stride);

                if (use_texture) {
                    dl->idx_write[0] = (DrawIdx)(idx2);
                    dl->idx_write[1] = (DrawIdx)(idx1);
                    dl->idx_write[2] = (DrawIdx)(idx1 + 1);
                    dl->idx_write[3] = (DrawIdx)(idx2 + 1);
                    dl->idx_write[4] = (DrawIdx)(idx1 + 1);
                    dl->idx_write[5] = (DrawIdx)(idx2);
                    dl->idx_write += 6;
                } else {
                    dl->idx_write[0]  = (DrawIdx)(idx2);
                    dl->idx_write[1]  = (DrawIdx)(idx1);
                    dl->idx_write[2]  = (DrawIdx)(idx1 + 2);
                    dl->idx_write[3]  = (DrawIdx)(idx1 + 2);
                    dl->idx_write[4]  = (DrawIdx)(idx2 + 2);
                    dl->idx_write[5]  = (DrawIdx)(idx2);
                    dl->idx_write[6]  = (DrawIdx)(idx2 + 1);
                    dl->idx_write[7]  = (DrawIdx)(idx1 + 1);
                    dl->idx_write[8]  = (DrawIdx)(idx1);
                    dl->idx_write[9]  = (DrawIdx)(idx1);
                    dl->idx_write[10] = (DrawIdx)(idx2);
                    dl->idx_write[11] = (DrawIdx)(idx2 + 1);
                    dl->idx_write += 12;
                }

                idx1 = idx2;
            }

            if (use_texture) {
                Vec4 tex_uv;
                Vec2 uv0;
                Vec2 uv1;

                tex_uv = dl->shared->line_uv[integer_thickness];
                uv0.x = tex_uv.x;
                uv0.y = tex_uv.y;
                uv1.x = tex_uv.z;
                uv1.y = tex_uv.w;

                for (i1 = 0; i1 < point_count; ++i1) {
                    write_vertex(dl->vtx_write,
                                 expanded[i1 * 2].x,
                                 expanded[i1 * 2].y, uv0, color);
                    ++dl->vtx_write;
                    write_vertex(dl->vtx_write,
                                 expanded[i1 * 2 + 1].x,
                                 expanded[i1 * 2 + 1].y, uv1, color);
                    ++dl->vtx_write;
                }
            } else {
                for (i1 = 0; i1 < point_count; ++i1) {
                    write_vertex(dl->vtx_write, points[i1].x, points[i1].y,
                                 white_uv, color);
                    ++dl->vtx_write;
                    write_vertex(dl->vtx_write,
                                 expanded[i1 * 2].x,
                                 expanded[i1 * 2].y,
                                 white_uv, transparent_color);
                    ++dl->vtx_write;
                    write_vertex(dl->vtx_write,
                                 expanded[i1 * 2 + 1].x,
                                 expanded[i1 * 2 + 1].y,
                                 white_uv, transparent_color);
                    ++dl->vtx_write;
                }
            }
        } else {
            float half_inner;
            float inner_x;
            float inner_y;
            float outer_x;
            float outer_y;

            half_inner = (thickness - aa_size) * 0.5f;

            if (!closed) {
                for (i1 = 0; i1 < 2; ++i1) {
                    int p;

                    p = i1 == 0 ? 0 : point_count - 1;
                    outer_x = normals[p].x * (half_inner + aa_size);
                    outer_y = normals[p].y * (half_inner + aa_size);
                    inner_x = normals[p].x * half_inner;
                    inner_y = normals[p].y * half_inner;

                    expanded[p * 4].x = points[p].x + outer_x;
                    expanded[p * 4].y = points[p].y + outer_y;
                    expanded[p * 4 + 1].x = points[p].x + inner_x;
                    expanded[p * 4 + 1].y = points[p].y + inner_y;
                    expanded[p * 4 + 2].x = points[p].x - inner_x;
                    expanded[p * 4 + 2].y = points[p].y - inner_y;
                    expanded[p * 4 + 3].x = points[p].x - outer_x;
                    expanded[p * 4 + 3].y = points[p].y - outer_y;
                }
            }

            idx1 = dl->current_vtx;

            for (i1 = 0; i1 < segment_count; ++i1) {
                i2 = i1 + 1;
                if (i2 == point_count)
                    i2 = 0;

                dx = (normals[i1].x + normals[i2].x) * 0.5f;
                dy = (normals[i1].y + normals[i2].y) * 0.5f;
                fix_join_normal(&dx, &dy);

                inner_x = dx * half_inner;
                inner_y = dy * half_inner;
                outer_x = dx * (half_inner + aa_size);
                outer_y = dy * (half_inner + aa_size);

                expanded[i2 * 4].x = points[i2].x + outer_x;
                expanded[i2 * 4].y = points[i2].y + outer_y;
                expanded[i2 * 4 + 1].x = points[i2].x + inner_x;
                expanded[i2 * 4 + 1].y = points[i2].y + inner_y;
                expanded[i2 * 4 + 2].x = points[i2].x - inner_x;
                expanded[i2 * 4 + 2].y = points[i2].y - inner_y;
                expanded[i2 * 4 + 3].x = points[i2].x - outer_x;
                expanded[i2 * 4 + 3].y = points[i2].y - outer_y;

                idx2 = dl->current_vtx + (unsigned int)(i2 * 4);

                dl->idx_write[0]  = (DrawIdx)(idx2 + 1);
                dl->idx_write[1]  = (DrawIdx)(idx1 + 1);
                dl->idx_write[2]  = (DrawIdx)(idx1 + 2);
                dl->idx_write[3]  = (DrawIdx)(idx1 + 2);
                dl->idx_write[4]  = (DrawIdx)(idx2 + 2);
                dl->idx_write[5]  = (DrawIdx)(idx2 + 1);

                dl->idx_write[6]  = (DrawIdx)(idx2);
                dl->idx_write[7]  = (DrawIdx)(idx1);
                dl->idx_write[8]  = (DrawIdx)(idx1 + 1);
                dl->idx_write[9]  = (DrawIdx)(idx1 + 1);
                dl->idx_write[10] = (DrawIdx)(idx2 + 1);
                dl->idx_write[11] = (DrawIdx)(idx2);

                dl->idx_write[12] = (DrawIdx)(idx2 + 2);
                dl->idx_write[13] = (DrawIdx)(idx1 + 2);
                dl->idx_write[14] = (DrawIdx)(idx1 + 3);
                dl->idx_write[15] = (DrawIdx)(idx1 + 3);
                dl->idx_write[16] = (DrawIdx)(idx2 + 3);
                dl->idx_write[17] = (DrawIdx)(idx2 + 2);
                dl->idx_write += 18;

                idx1 = idx2;
            }

            for (i1 = 0; i1 < point_count; ++i1) {
                write_vertex(dl->vtx_write,
                             expanded[i1 * 4].x, expanded[i1 * 4].y,
                             white_uv, transparent_color);
                ++dl->vtx_write;
                write_vertex(dl->vtx_write,
                             expanded[i1 * 4 + 1].x,
                             expanded[i1 * 4 + 1].y,
                             white_uv, color);
                ++dl->vtx_write;
                write_vertex(dl->vtx_write,
                             expanded[i1 * 4 + 2].x,
                             expanded[i1 * 4 + 2].y,
                             white_uv, color);
                ++dl->vtx_write;
                write_vertex(dl->vtx_write,
                             expanded[i1 * 4 + 3].x,
                             expanded[i1 * 4 + 3].y,
                             white_uv, transparent_color);
                ++dl->vtx_write;
            }
        }

        dl->current_vtx += (unsigned int)vertex_count;
    } else {
        int i1;
        int i2;
        float dx;
        float dy;
        float half_thickness;
        unsigned int base;

        draw_reserve(dl, segment_count * 6, segment_count * 4);
        half_thickness = thickness * 0.5f;

        for (i1 = 0; i1 < segment_count; ++i1) {
            i2 = i1 + 1;
            if (i2 == point_count)
                i2 = 0;

            dx = points[i2].x - points[i1].x;
            dy = points[i2].y - points[i1].y;
            normalize_over_zero(&dx, &dy);
            dx *= half_thickness;
            dy *= half_thickness;

            base = dl->current_vtx;

            dl->idx_write[0] = (DrawIdx)base;
            dl->idx_write[1] = (DrawIdx)(base + 1);
            dl->idx_write[2] = (DrawIdx)(base + 2);
            dl->idx_write[3] = (DrawIdx)base;
            dl->idx_write[4] = (DrawIdx)(base + 2);
            dl->idx_write[5] = (DrawIdx)(base + 3);
            dl->idx_write += 6;

            write_vertex(dl->vtx_write,
                         points[i1].x + dy, points[i1].y - dx,
                         white_uv, color);
            ++dl->vtx_write;
            write_vertex(dl->vtx_write,
                         points[i2].x + dy, points[i2].y - dx,
                         white_uv, color);
            ++dl->vtx_write;
            write_vertex(dl->vtx_write,
                         points[i2].x - dy, points[i2].y + dx,
                         white_uv, color);
            ++dl->vtx_write;
            write_vertex(dl->vtx_write,
                         points[i1].x - dy, points[i1].y + dx,
                         white_uv, color);
            ++dl->vtx_write;

            dl->current_vtx += 4;
        }
    }
}

/*
 * Branch/code-size notes:
 *
 * Non-AA emits four independent vertices and six indices per segment.
 * AA thin emits one opaque center plus two transparent fringe vertices per
 * input point.  AA thick emits transparent outer, opaque inner, opaque inner,
 * transparent outer vertices.  Texture AA emits two vertices per point and
 * obtains the interior/fringe profile from the baked line texture.
 *
 * Keeping index emission explicit protects byte-exact winding and order.
 * Sharing normal generation and the small write_vertex helper is the useful
 * code-size reduction that does not obscure those observable sequences.
 */

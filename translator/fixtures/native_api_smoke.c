#include "imgui_c89_api.h"
#include <stdio.h>

int main(void)
{
    ImGuiContext *ctx;
    ImGuiContext *other_ctx;
    ImGuiIO *io;
    ImGuiIO *other_io;
    ImDrawData *draw_data;
    unsigned char *pixels;
    int texture_width;
    int texture_height;
    imgui_scope began;
    imgui_scope child_began;
    ImVec2 child_position;
    ImVec2 child_size;
    int frame;
    unsigned char open;

    if (ImGuiWindowFlags_NoTitleBar != 1 ||
        ImGuiTableBgTarget_CellBg != 3 ||
        ImGuiCond_Always != 1)
        return 14;
    ctx = imgui_create_context(0);
    if (ctx == 0)
        return 1;
    other_ctx = imgui_create_context(0);
    if (other_ctx == 0)
        return 2;
    io = imgui_get_io(ctx);
    other_io = imgui_get_io(other_ctx);
    if (io == 0 || other_io == 0 || io == other_io)
        return 3;
    io->IniFilename = 0;
    other_io->IniFilename = 0;
    other_io->DisplaySize.x = 17.0f;
    /* Deliberately leave another context current. Ordinary C API calls below
     * must use their explicit argument without selecting/restoring it. */
    imgui_set_current_context(other_ctx);
    io->DisplaySize.x = 640.0f;
    io->DisplaySize.y = 480.0f;
    io->DeltaTime = 1.0f / 60.0f;
#ifdef IMGUI_EMBEDDED_EXTERNAL_FONT
    if (imgui_font_atlas_add_font_from_file_ttf(
            ctx, io->Fonts, "third_party/ProggyClean.ttf", 13.0f, 0, 0) == 0)
        return 13;
#endif
    imgui_font_atlas_get_tex_data_as_rgba32(
        ctx, io->Fonts, &pixels, &texture_width, &texture_height, 0);
    if (pixels == 0 || texture_width <= 0 || texture_height <= 0)
        return 4;
    open = 1;
    draw_data = 0;
    began = 0;
    for (frame = 0; frame < 2; ++frame)
    {
        imgui_new_frame(ctx);
        if (frame == 0)
        {
            began = imgui_begin_scope(ctx, "native-c89-inactive", &open, 0);
            if (began != IMGUI_SCOPE_ACTIVE)
                return 8;
            imgui_end(ctx);
        }
        else
        {
            imgui_set_window_collapsed_string_bool_cond(
                ctx, "native-c89-inactive", 1, 1);
            began = imgui_begin_scope(ctx, "native-c89-inactive", &open, 0);
            if (began != IMGUI_SCOPE_INACTIVE)
            {
                if (began == IMGUI_SCOPE_ACTIVE)
                    imgui_end(ctx);
                return 9;
            }
            /* The inactive Begin scope was discharged by the C adapter.
             * There must be no matching imgui_end() here. */
        }
        began = imgui_begin_scope(ctx, "native-c89-api", &open, 0);
        if (began != IMGUI_SCOPE_ACTIVE)
            return 10;
        imgui_text_unformatted(ctx, "C89, explicit context", 0);
        child_size.x = 100.0f;
        child_size.y = 100.0f;
        if (frame == 0)
        {
            child_began = imgui_begin_child_string_scope(
                ctx, "native-c89-inactive-child", &child_size, 0, 0);
            if (child_began != IMGUI_SCOPE_ACTIVE)
                return 11;
            imgui_end_child(ctx);
        }
        else
        {
            child_position.x = 100000.0f;
            child_position.y = 100000.0f;
            imgui_set_cursor_pos(ctx, &child_position);
            child_began = imgui_begin_child_string_scope(
                ctx, "native-c89-inactive-child", &child_size, 0, 0);
            if (child_began != IMGUI_SCOPE_INACTIVE)
            {
                if (child_began == IMGUI_SCOPE_ACTIVE)
                    imgui_end_child(ctx);
                return 12;
            }
            /* As above, inactive child scopes use the normal conditional-End
             * contract and are already closed. */
        }
        imgui_end(ctx);
        imgui_render(ctx);
        draw_data = imgui_get_draw_data(ctx);
    }
    if (draw_data == 0 || draw_data->CmdLists.Size <= 0 ||
        draw_data->TotalVtxCount <= 0 || draw_data->TotalIdxCount <= 0)
    {
        if (draw_data != 0)
            fprintf(stderr, "native API empty draw data: began=%d lists=%d vtx=%d idx=%d\n",
                    (int)began,
                    draw_data->CmdLists.Size, draw_data->TotalVtxCount,
                    draw_data->TotalIdxCount);
        return 5;
    }
    if (other_io->DisplaySize.x != 17.0f)
        return 6;
    if (imgui_get_current_context() != other_ctx)
        return 7;
    imgui_destroy_context(ctx);
    imgui_destroy_context(other_ctx);
    return 0;
}

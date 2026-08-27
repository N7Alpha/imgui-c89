#include "imgui_c89_api.h"

int main(void)
{
    ImGuiContext *ctx;
    ImGuiIO *io;
    ImDrawData *draw_data;
    unsigned char *pixels;
    int width;
    int height;
    int frame;
    unsigned char open;

    ctx = imgui_create_context(0);
    if (ctx == 0)
        return 1;
    io = imgui_get_io(ctx);
    io->IniFilename = 0;
    io->DisplaySize.x = 320.0f;
    io->DisplaySize.y = 240.0f;
    io->DeltaTime = 1.0f / 60.0f;
    imgui_font_atlas_get_tex_data_as_alpha8(
        ctx, io->Fonts, &pixels, &width, &height, 0);
    if (pixels == 0 || width <= 0 || height <= 0)
        return 2;

    open = 1;
    draw_data = 0;
    for (frame = 0; frame < 2; ++frame)
    {
        imgui_new_frame(ctx);
        if (imgui_begin_scope(ctx, "C89 distribution smoke", &open, 0)
                != IMGUI_SCOPE_ACTIVE)
            return 3;
        imgui_text_unformatted(ctx, "Dear ImGui through C89", 0);
        imgui_end(ctx);
        imgui_render(ctx);
        draw_data = imgui_get_draw_data(ctx);
    }
    if (draw_data == 0 || draw_data->CmdLists.Size <= 0 ||
        draw_data->TotalVtxCount <= 0 || draw_data->TotalIdxCount <= 0)
        return 4;
    imgui_destroy_context(ctx);
    return 0;
}

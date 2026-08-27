#include "imgui_c89_api.h"

int main(void)
{
    ImGuiContext *ctx;
    ImGuiIO *io;
    ImDrawData *draw_data;
    unsigned char *pixels;
    int texture_width;
    int texture_height;
    int frame;
    unsigned char open;

    ctx = imgui_create_context(0);
    if (ctx == 0)
        return 1;
#ifdef IMGUI_C89_ENABLE_FULL_FEATURES
    imgui_enable_full_features(ctx);
#endif
    io = imgui_get_io(ctx);
    io->IniFilename = 0;
    io->DisplaySize.x = 320.0f;
    io->DisplaySize.y = 240.0f;
    io->DeltaTime = 1.0f / 60.0f;
#ifdef IMGUI_EMBEDDED_EXTERNAL_FONT
    if (imgui_font_atlas_add_font_from_file_ttf(
            ctx, io->Fonts, "third_party/ProggyClean.ttf", 13.0f, 0, 0) == 0)
        return 4;
#endif
    imgui_font_atlas_get_tex_data_as_alpha8(
        ctx, io->Fonts, &pixels, &texture_width, &texture_height, 0);
    if (pixels == 0 || texture_width <= 0 || texture_height <= 0)
        return 2;

    open = 1;
    draw_data = 0;
    for (frame = 0; frame < 2; ++frame)
    {
        imgui_new_frame(ctx);
        if (imgui_begin(ctx, "embedded", &open, 0) == IMGUI_SCOPE_ACTIVE)
        {
            imgui_text_unformatted(ctx, "Dear ImGui C89", 0);
            imgui_end(ctx);
        }
        imgui_render(ctx);
        draw_data = imgui_get_draw_data(ctx);
    }
    if (draw_data == 0 || draw_data->TotalVtxCount <= 0 ||
        draw_data->TotalIdxCount <= 0)
        return 3;
    imgui_destroy_context(ctx);
    return 0;
}

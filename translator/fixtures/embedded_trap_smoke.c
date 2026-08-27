#include "imgui_c89_api.h"

int main(void)
{
    ImGuiContext *ctx = imgui_create_context(0);
    if (ctx == 0)
        return 2;
    imgui_end(ctx);
    return 0;
}

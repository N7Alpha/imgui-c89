#include "imgui_c89_api.h"

int main(void)
{
    ImGuiContext *ctx;

    ctx = imgui_create_context(0);
    if (ctx == 0)
        return 1;

    /* Begin outside a frame violates an upstream IM_ASSERT invariant. */
    (void)imgui_begin(ctx, "assert-probe", 0, 0);
    return 0;
}

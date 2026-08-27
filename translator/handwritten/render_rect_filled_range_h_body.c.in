    ImVec2 p0;
    ImVec2 p1;
    ImVec2 point;
    ImVec2 center;
    float input_rounding;
    float width_half;
    float height_half;
    float limit;
    float inv;
    float half_pi;
    float b;
    float e;
    float x;

    if (fill_x0 > fill_x1)
        return;

    p0.x = fill_x0;
    p0.y = rect->Min.y;
    p1.x = fill_x1;
    p1.y = rect->Max.y;

    if (rounding == 0.0f)
    {
        @ADD_RECT_FILLED@(imgui_c89_ctx, draw_list, &p0, &p1, col, 0.0f, 0);
        return;
    }

    input_rounding = rounding;
    width_half = (rect->Max.x - rect->Min.x) / 2.0f;
    height_half = (rect->Max.y - rect->Min.y) / 2.0f;
    limit = (width_half < height_half ? width_half : height_half) - 1.0f;
    if (limit < 0.0f)
        rounding = 0.0f;
    else if (limit > input_rounding)
        rounding = input_rounding;
    else
        rounding = limit;

    inv = 1.0f / rounding;
    half_pi = 3.14159274f / 2.0f;

    b = @ACOS01@(1.0f - (p0.x - rect->Min.x) * inv);
    e = @ACOS01@(1.0f - (p1.x - rect->Min.x) * inv);
    x = p0.x >= rect->Min.x + rounding ? p0.x : rect->Min.x + rounding;
    if (b == e)
    {
        point.x = x;
        point.y = p1.y;
        @PATH_LINE_TO@(imgui_c89_ctx, draw_list, &point);
        point.y = p0.y;
        @PATH_LINE_TO@(imgui_c89_ctx, draw_list, &point);
    }
    else if (b == 0.0f && e == half_pi)
    {
        center.x = x;
        center.y = p1.y - rounding;
        @PATH_ARC_TO_FAST@(imgui_c89_ctx, draw_list, &center, rounding, 3, 6);
        center.y = p0.y + rounding;
        @PATH_ARC_TO_FAST@(imgui_c89_ctx, draw_list, &center, rounding, 6, 9);
    }
    else
    {
        center.x = x;
        center.y = p1.y - rounding;
        @PATH_ARC_TO@(imgui_c89_ctx, draw_list, &center, rounding, 3.14159274f - e, 3.14159274f - b, 0);
        center.y = p0.y + rounding;
        @PATH_ARC_TO@(imgui_c89_ctx, draw_list, &center, rounding, 3.14159274f + b, 3.14159274f + e, 0);
    }

    if (p1.x > rect->Min.x + rounding)
    {
        b = @ACOS01@(1.0f - (rect->Max.x - p1.x) * inv);
        e = @ACOS01@(1.0f - (rect->Max.x - p0.x) * inv);
        x = p1.x < rect->Max.x - rounding ? p1.x : rect->Max.x - rounding;
        if (b == e)
        {
            point.x = x;
            point.y = p0.y;
            @PATH_LINE_TO@(imgui_c89_ctx, draw_list, &point);
            point.y = p1.y;
            @PATH_LINE_TO@(imgui_c89_ctx, draw_list, &point);
        }
        else if (b == 0.0f && e == half_pi)
        {
            center.x = x;
            center.y = p0.y + rounding;
            @PATH_ARC_TO_FAST@(imgui_c89_ctx, draw_list, &center, rounding, 9, 12);
            center.y = p1.y - rounding;
            @PATH_ARC_TO_FAST@(imgui_c89_ctx, draw_list, &center, rounding, 0, 3);
        }
        else
        {
            center.x = x;
            center.y = p0.y + rounding;
            @PATH_ARC_TO@(imgui_c89_ctx, draw_list, &center, rounding, -e, -b, 0);
            center.y = p1.y - rounding;
            @PATH_ARC_TO@(imgui_c89_ctx, draw_list, &center, rounding, b, e, 0);
        }
    }

    @PATH_FILL_CONVEX@(imgui_c89_ctx, draw_list, col);

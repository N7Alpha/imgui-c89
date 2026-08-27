/* FUNCTION: TableGetColumnAvailSortDirection(ImGuiTableColumn*,int) */
{
    return (ImGuiSortDirection)((column->SortDirectionsAvailList >> (n * 2)) & 3);
}

/* FUNCTION: ImGui::TableGetColumnNextSortDirection(ImGuiTableColumn*) */
{
    int n;

    if (column->SortOrder == -1)
        return @GET_AVAIL_DIRECTION@(column, 0);
    for (n = 0; n < 3; n++)
    {
        if (column->SortDirection == @GET_AVAIL_DIRECTION@(column, n))
            return @GET_AVAIL_DIRECTION@(column, (n + 1) % column->SortDirectionsAvailCount);
    }
    return @SORT_NONE@;
}

/* FUNCTION: ImGui::TableGetHeaderRowHeight() */
{
    ImGuiTable *table;
    float height;
    int column_n;

    table = imgui_c89_ctx->CurrentTable;
    height = imgui_c89_ctx->FontSize;
    for (column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        ImGuiTableColumn *column;
        const char *name;
        ImVec2 text_size;

        column = &table->Columns.Data[column_n];
        if ((table->EnabledMaskByIndex[column_n >> 5] & ((ImU32)1 << (column_n & 31))) == 0)
            continue;
        if ((column->Flags & @NO_HEADER_LABEL@) != 0)
            continue;
        name = @TABLE_GET_COLUMN_NAME@(table, column_n);
        text_size = @CALC_TEXT_SIZE@(imgui_c89_ctx, name, 0, false, 0.0f);
        if (height < text_size.y)
            height = text_size.y;
    }
    return height + imgui_c89_ctx->Style.CellPadding.y * 2.0f;
}

/* FUNCTION: ImGui::TableGetHeaderAngledMaxLabelWidth() */
{
    ImGuiTable *table;
    float width;
    int column_n;

    table = imgui_c89_ctx->CurrentTable;
    width = 0.0f;
    for (column_n = 0; column_n < table->ColumnsCount; column_n++)
    {
        ImGuiTableColumn *column;
        const char *name;
        ImVec2 text_size;

        column = &table->Columns.Data[column_n];
        if ((table->EnabledMaskByIndex[column_n >> 5] & ((ImU32)1 << (column_n & 31))) == 0)
            continue;
        if ((column->Flags & @ANGLED_HEADER@) == 0)
            continue;
        name = @TABLE_GET_COLUMN_NAME@(table, column_n);
        text_size = @CALC_TEXT_SIZE@(imgui_c89_ctx, name, 0, true, 0.0f);
        if (width < text_size.x)
            width = text_size.x;
    }
    return width + imgui_c89_ctx->Style.CellPadding.y * 2.0f;
}

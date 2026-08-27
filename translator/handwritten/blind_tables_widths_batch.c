/* FUNCTION: ImGui::TableCalcMaxColumnWidth(const ImGuiTable *,int) */
{
    const ImGuiTableColumn *column;
    float max_width;
    float min_distance;
    float cap;

    column = &table->Columns.Data[column_n];
    max_width = 3.402823466e38F;
    min_distance = table->MinColumnWidth + 2.0F * table->CellPaddingX + table->CellSpacingX1 + table->CellSpacingX2;
    if ((table->Flags & @SCROLL_X@) != 0)
    {
        if (column->DisplayOrder < table->FreezeColumnsRequest)
        {
            cap = table->InnerClipRect.Max.x - (table->FreezeColumnsRequest - column->DisplayOrder) * min_distance - column->MinX - table->OuterPaddingX - table->CellPaddingX - table->CellSpacingX2;
            max_width = cap;
        }
    }
    else if ((table->Flags & @NO_KEEP_VISIBLE@) == 0)
    {
        cap = table->WorkRect.Max.x - (table->ColumnsEnabledCount - column->IndexWithinEnabledSet - 1) * min_distance - column->MinX - table->CellSpacingX2 - 2.0F * table->CellPaddingX - table->OuterPaddingX;
        max_width = cap;
    }
    return max_width;
}

/* FUNCTION: ImGui::TableGetColumnWidthAuto(ImGuiTable *,ImGuiTableColumn *) */
{
    float content_max_x;
    float body_width;
    float header_width;
    float width_auto;

    content_max_x = column->ContentMaxXFrozen;
    if (column->ContentMaxXUnfrozen > content_max_x)
        content_max_x = column->ContentMaxXUnfrozen;
    body_width = content_max_x - column->WorkMinX;
    header_width = column->ContentMaxXHeadersIdeal - column->WorkMinX;
    width_auto = body_width;
    if ((column->Flags & @NO_HEADER_WIDTH@) == 0 && header_width > width_auto)
        width_auto = header_width;
    if ((column->Flags & @WIDTH_FIXED@) != 0 && column->InitStretchWeightOrWidth > 0.0F && ((table->Flags & @RESIZABLE@) == 0 || (column->Flags & @NO_RESIZE@) != 0))
        width_auto = column->InitStretchWeightOrWidth;
    if (width_auto < table->MinColumnWidth)
        width_auto = table->MinColumnWidth;
    return width_auto;
}

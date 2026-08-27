/* FUNCTION: ImGui::TableGetColumnCount() */
{
    ImGuiContext *ctx;
    ImGuiTable *table;

    ctx = imgui_c89_ctx;
    table = ctx->CurrentTable;
    if (table == 0)
        return 0;
    return table->ColumnsCount;
}

/* FUNCTION: ImGui::TableGetColumnName(int) */
{
    ImGuiContext *ctx;
    ImGuiTable *table;

    ctx = imgui_c89_ctx;
    table = ctx->CurrentTable;
    if (table == 0)
        return 0;
    if (column_n < 0)
        column_n = table->CurrentColumn;
    return @TABLE_GET_COLUMN_NAME@(table, column_n);
}

/* FUNCTION: ImGui::TableGetColumnName(const ImGuiTable *,int) */
{
    const ImGuiTableColumn *column;

    if (!table->IsLayoutLocked && column_n >= table->DeclColumnsCount)
        return "";
    column = &table->Columns.Data[column_n];
    if (column->NameOffset == -1)
        return "";
    return table->ColumnsNames.Buf.Data + column->NameOffset;
}

/* FUNCTION: ImGui::TableGetColumnFlags(int) */
{
    ImGuiContext *ctx;
    ImGuiTable *table;

    ctx = imgui_c89_ctx;
    table = ctx->CurrentTable;
    if (table == 0)
        return @NONE@;
    if (column_n < 0)
        column_n = table->CurrentColumn;
    if (column_n == table->ColumnsCount)
    {
        if (table->HoveredColumnBody == column_n)
            return @IS_HOVERED@;
        return @NONE@;
    }
    return table->Columns.Data[column_n].Flags;
}

/* FUNCTION: ImGui::TableGetCellBgRect(const ImGuiTable *,int) */
{
    const ImGuiTableColumn *column;
    ImRect rect;

    column = &table->Columns.Data[column_n];
    rect.Min.x = column->MinX > table->WorkRect.Min.x ? column->MinX : table->WorkRect.Min.x;
    rect.Min.y = table->RowPosY1;
    rect.Max.x = column->MaxX < table->WorkRect.Max.x ? column->MaxX : table->WorkRect.Max.x;
    rect.Max.y = table->RowPosY2;
    return rect;
}

/* FUNCTION: ImGui::TableGetColumnResizeID(ImGuiTable *,int,int) */
{
    ImGuiTableInstanceData *instance_data;

    if (instance_no == 0)
        instance_data = &table->InstanceDataFirst;
    else
        instance_data = &table->InstanceDataExtra.Data[instance_no - 1];
    return instance_data->TableInstanceID + 1 + column_n;
}

/* FUNCTION: ImGui::TableGetHoveredColumn() */
{
    ImGuiContext *ctx;
    ImGuiTable *table;

    ctx = imgui_c89_ctx;
    table = ctx->CurrentTable;
    if (table == 0)
        return -1;
    return (int)table->HoveredColumnBody;
}

/* FUNCTION: ImGui::TableGetHoveredRow() */
{
    ImGuiContext *ctx;
    ImGuiTable *table;
    ImGuiTableInstanceData *instance_data;

    ctx = imgui_c89_ctx;
    table = ctx->CurrentTable;
    if (table == 0)
        return -1;
    if (table->InstanceCurrent == 0)
        instance_data = &table->InstanceDataFirst;
    else
        instance_data = &table->InstanceDataExtra.Data[table->InstanceCurrent - 1];
    return instance_data->HoveredRowLast;
}

/* FUNCTION: ImGui::TableGetRowIndex() */
{
    ImGuiContext *ctx;
    ImGuiTable *table;

    ctx = imgui_c89_ctx;
    table = ctx->CurrentTable;
    if (table == 0)
        return 0;
    return table->CurrentRow;
}

/* FUNCTION: ImGui::TableGetColumnIndex() */
{
    ImGuiContext *ctx;
    ImGuiTable *table;

    ctx = imgui_c89_ctx;
    table = ctx->CurrentTable;
    if (table == 0)
        return 0;
    return table->CurrentColumn;
}

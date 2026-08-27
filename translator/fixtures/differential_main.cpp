/*
 * This exact source is compiled twice: once against native Dear ImGui and
 * once against the generated C++ facade backed by strict-C89 objects.
 * Its stdout is a canonical, pointer-free behavioral transcript.
 */
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_c89_software.h"

#include <float.h>
#include <stdio.h>
#include <string.h>
#include <vector>

struct DifferentialTexture
{
    const unsigned char* atlas_pixels;
    int atlas_width;
    int atlas_height;
    const unsigned char* user_pixels;
    int user_width;
    int user_height;
    bool flat;
};

struct DifferentialList
{
    std::vector<imgui_render_vertex> vertices;
    std::vector<imgui_render_index> indices;
    std::vector<imgui_render_command> commands;
};

struct ScenarioPoints
{
    ImVec2 button;
    ImVec2 input;
    ImVec2 plot;
};

static FILE* pixel_stream;

/* The standalone software rasterizer only needs texture equivalence for its
 * packet-comparison helpers. Differential rendering canonicalizes every
 * ImGui texture to the same atlas handle. */
extern "C" imgui_bool imgui_texture_equivalent(const imgui_texture* left,
                                                const imgui_texture* right)
{
    return left == right ? IMGUI_TRUE : IMGUI_FALSE;
}

static unsigned int hash_bytes(unsigned int hash, const void* data, size_t size)
{
    const unsigned char* bytes = (const unsigned char*)data;
    size_t index;
    for (index = 0; index < size; ++index)
    {
        hash ^= (unsigned int)bytes[index];
        hash *= 16777619U;
    }
    return hash;
}

static unsigned int float_bits(float value)
{
    unsigned int bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

template<typename T>
static unsigned int hash_data_type_samples(unsigned int hash,
                                           ImGuiDataType type,
                                           const T* values, int count)
{
    const int operations[2] = { '+', '-' };
    int operation_n;
    int a_n;
    int b_n;
    for (operation_n = 0; operation_n < 2; ++operation_n)
        for (a_n = 0; a_n < count; ++a_n)
            for (b_n = 0; b_n < count; ++b_n)
            {
                T result;
                ImGui::DataTypeApplyOp(type, operations[operation_n], &result,
                                       &values[a_n], &values[b_n]);
                hash = hash_bytes(hash, &result, sizeof(result));
            }
    return hash;
}

static unsigned int data_type_operation_hash()
{
    const ImS16 s16[] = { -32767 - 1, -1, 0, 1, 32767 };
    const ImU16 u16[] = { 0, 1, 32767, 32768, 65535 };
    const ImS32 s32[] = { -2147483647 - 1, -1, 0, 1, 2147483647 };
    const ImU32 u32[] = {
        0, 1, 0x7fffffffU, 0x80000000U, 0xffffffffU
    };
    const ImS64 s64[] = {
        -9223372036854775807LL - 1, -1, 0, 1, 9223372036854775807LL
    };
    const ImU64 u64[] = {
        0, 1, 0x7fffffffffffffffULL, 0x8000000000000000ULL,
        0xffffffffffffffffULL
    };
    const float f32[] = { -FLT_MAX, -1.25f, 0.0f, 1.25f, FLT_MAX };
    const double f64[] = { -DBL_MAX, -1.25, 0.0, 1.25, DBL_MAX };
    unsigned int hash = 2166136261U;
    int operation_n;
    int a_n;
    int b_n;
    const int operations[2] = { '+', '-' };

    for (operation_n = 0; operation_n < 2; ++operation_n)
        for (a_n = 0; a_n < 256; ++a_n)
            for (b_n = 0; b_n < 256; ++b_n)
            {
                ImS8 sa = (ImS8)(a_n - 128);
                ImS8 sb = (ImS8)(b_n - 128);
                ImS8 sr;
                ImU8 ua = (ImU8)a_n;
                ImU8 ub = (ImU8)b_n;
                ImU8 ur;
                ImGui::DataTypeApplyOp(ImGuiDataType_S8,
                                       operations[operation_n],
                                       &sr, &sa, &sb);
                ImGui::DataTypeApplyOp(ImGuiDataType_U8,
                                       operations[operation_n],
                                       &ur, &ua, &ub);
                hash = hash_bytes(hash, &sr, sizeof(sr));
                hash = hash_bytes(hash, &ur, sizeof(ur));
            }
    hash = hash_data_type_samples(hash, ImGuiDataType_S16, s16,
                                  IM_ARRAYSIZE(s16));
    hash = hash_data_type_samples(hash, ImGuiDataType_U16, u16,
                                  IM_ARRAYSIZE(u16));
    hash = hash_data_type_samples(hash, ImGuiDataType_S32, s32,
                                  IM_ARRAYSIZE(s32));
    hash = hash_data_type_samples(hash, ImGuiDataType_U32, u32,
                                  IM_ARRAYSIZE(u32));
    hash = hash_data_type_samples(hash, ImGuiDataType_S64, s64,
                                  IM_ARRAYSIZE(s64));
    hash = hash_data_type_samples(hash, ImGuiDataType_U64, u64,
                                  IM_ARRAYSIZE(u64));
    hash = hash_data_type_samples(hash, ImGuiDataType_Float, f32,
                                  IM_ARRAYSIZE(f32));
    hash = hash_data_type_samples(hash, ImGuiDataType_Double, f64,
                                  IM_ARRAYSIZE(f64));
    return hash;
}

static imgui_bool resolve_texture(const imgui_texture* identifier,
                                  imgui_software_texture_view* view,
                                  void* user_data)
{
    static const unsigned char white[4] = { 255, 255, 255, 255 };
    DifferentialTexture* texture = (DifferentialTexture*)user_data;
    if (view == NULL || texture == NULL)
        return IMGUI_FALSE;
    view->mutable_pixels = NULL;
    if (texture->flat)
    {
        view->pixels = white;
        view->width = 1;
        view->height = 1;
        view->stride = 4;
    }
    else
    {
        bool user = identifier == (const imgui_texture*)(size_t)2;
        view->pixels = user ? texture->user_pixels : texture->atlas_pixels;
        view->width = user ? texture->user_width : texture->atlas_width;
        view->height = user ? texture->user_height : texture->atlas_height;
        view->stride = (size_t)view->width * 4U;
    }
    view->format = IMGUI_TEXTURE_FORMAT_RGBA8;
    return view->pixels != NULL ? IMGUI_TRUE : IMGUI_FALSE;
}

static unsigned int render_hash(ImDrawData* draw_data,
                                DifferentialTexture* texture)
{
    std::vector<DifferentialList> storage;
    std::vector<imgui_render_list> lists;
    imgui_viewport_packet viewport;
    imgui_render_packet packet;
    imgui_software_target target;
    std::vector<unsigned char> framebuffer(640U * 480U * 4U);
    int list_index;

    storage.resize((size_t)draw_data->CmdLists.Size);
    lists.resize((size_t)draw_data->CmdLists.Size);
    for (list_index = 0; list_index < draw_data->CmdLists.Size; ++list_index)
    {
        const ImDrawList* source = draw_data->CmdLists[list_index];
        DifferentialList& destination = storage[(size_t)list_index];
        int index;
        destination.vertices.resize((size_t)source->VtxBuffer.Size);
        destination.indices.resize((size_t)source->IdxBuffer.Size);
        destination.commands.resize((size_t)source->CmdBuffer.Size);
        for (index = 0; index < source->VtxBuffer.Size; ++index)
        {
            destination.vertices[(size_t)index].position.x =
                source->VtxBuffer[index].pos.x;
            destination.vertices[(size_t)index].position.y =
                source->VtxBuffer[index].pos.y;
            destination.vertices[(size_t)index].uv.x =
                source->VtxBuffer[index].uv.x;
            destination.vertices[(size_t)index].uv.y =
                source->VtxBuffer[index].uv.y;
            destination.vertices[(size_t)index].color =
                (imgui_u32)source->VtxBuffer[index].col;
        }
        for (index = 0; index < source->IdxBuffer.Size; ++index)
            destination.indices[(size_t)index] =
                (imgui_render_index)source->IdxBuffer[index];
        for (index = 0; index < source->CmdBuffer.Size; ++index)
        {
            const ImDrawCmd& source_command = source->CmdBuffer[index];
            imgui_render_command& command =
                destination.commands[(size_t)index];
            memset(&command, 0, sizeof(command));
            if (source_command.UserCallback != NULL)
            {
                command.type = IMGUI_RENDER_COMMAND_RESET_STATE;
                continue;
            }
            command.type = IMGUI_RENDER_COMMAND_DRAW_INDEXED;
            command.data.draw_indexed.clip_rect.x1 = source_command.ClipRect.x;
            command.data.draw_indexed.clip_rect.y1 = source_command.ClipRect.y;
            command.data.draw_indexed.clip_rect.x2 = source_command.ClipRect.z;
            command.data.draw_indexed.clip_rect.y2 = source_command.ClipRect.w;
            command.data.draw_indexed.texture =
                (imgui_texture*)(size_t)source_command.GetTexID();
            command.data.draw_indexed.index_offset = source_command.IdxOffset;
            command.data.draw_indexed.index_count = source_command.ElemCount;
            command.data.draw_indexed.vertex_offset = source_command.VtxOffset;
        }
        lists[(size_t)list_index].vertices = destination.vertices.empty() ?
            NULL : &destination.vertices[0];
        lists[(size_t)list_index].vertex_count =
            (imgui_u32)destination.vertices.size();
        lists[(size_t)list_index].indices = destination.indices.empty() ?
            NULL : &destination.indices[0];
        lists[(size_t)list_index].index_count =
            (imgui_u32)destination.indices.size();
        lists[(size_t)list_index].commands = destination.commands.empty() ?
            NULL : &destination.commands[0];
        lists[(size_t)list_index].command_count =
            (imgui_u32)destination.commands.size();
    }

    memset(&viewport, 0, sizeof(viewport));
    viewport.display_position.x = draw_data->DisplayPos.x;
    viewport.display_position.y = draw_data->DisplayPos.y;
    viewport.display_size.x = draw_data->DisplaySize.x;
    viewport.display_size.y = draw_data->DisplaySize.y;
    viewport.framebuffer_scale.x = draw_data->FramebufferScale.x;
    viewport.framebuffer_scale.y = draw_data->FramebufferScale.y;
    viewport.lists = lists.empty() ? NULL : &lists[0];
    viewport.list_count = (imgui_u32)lists.size();
    memset(&packet, 0, sizeof(packet));
    packet.struct_size = sizeof(packet);
    packet.protocol_version = 1;
    packet.viewports = &viewport;
    packet.viewport_count = 1;
    imgui_software_target_init(&target);
    target.pixels = &framebuffer[0];
    target.width = 640;
    target.height = 480;
    target.stride = 640U * 4U;
    target.texture_resolver = resolve_texture;
    target.texture_user_data = texture;
    if (imgui_software_render_packet(&packet, &target) != IMGUI_RESULT_OK)
        return 0;
    if (pixel_stream != NULL && fwrite(&framebuffer[0], 1, framebuffer.size(),
                                       pixel_stream) != framebuffer.size())
        return 0;
    return hash_bytes(2166136261U, &framebuffer[0], framebuffer.size());
}

static void emit_draw_data(int frame, ImDrawData* draw_data,
                           DifferentialTexture* texture)
{
    ImGuiContext* state = ImGui::GetCurrentContext();
    int list_index;
    printf("frame %d state active=%08x hovered=%08x nav=%08x windows=%d "
           "capture_mouse=%d capture_keyboard=%d\n",
           frame, state->ActiveId, state->HoveredId, state->NavId,
           state->Windows.Size, ImGui::GetIO().WantCaptureMouse ? 1 : 0,
           ImGui::GetIO().WantCaptureKeyboard ? 1 : 0);
    printf("draw valid=%d frame=%d lists=%d idx=%d vtx=%d "
           "pos=%08x,%08x size=%08x,%08x scale=%08x,%08x\n",
           draw_data->Valid ? 1 : 0, draw_data->FrameCount,
           draw_data->CmdLists.Size, draw_data->TotalIdxCount,
           draw_data->TotalVtxCount,
           float_bits(draw_data->DisplayPos.x),
           float_bits(draw_data->DisplayPos.y),
           float_bits(draw_data->DisplaySize.x),
           float_bits(draw_data->DisplaySize.y),
           float_bits(draw_data->FramebufferScale.x),
           float_bits(draw_data->FramebufferScale.y));
    {
        int texture_count = draw_data->Textures == NULL ?
            0 : draw_data->Textures->Size;
        int texture_index;
        printf("textures count=%d\n", texture_count);
        for (texture_index = 0; texture_index < texture_count; ++texture_index)
        {
            const ImTextureData* data =
                (*draw_data->Textures)[texture_index];
            unsigned int pixel_hash = data->Pixels == NULL ? 0 : hash_bytes(
                2166136261U, data->Pixels, (size_t)data->Width *
                (size_t)data->Height * (size_t)data->BytesPerPixel);
            printf("texture %d unique=%d status=%d format=%d size=%dx%d "
                   "bpp=%d used=%u,%u,%u,%u update=%u,%u,%u,%u "
                   "updates=%d pixels=%08x\n", texture_index,
                   data->UniqueID, (int)data->Status, (int)data->Format,
                   data->Width, data->Height, data->BytesPerPixel,
                   data->UsedRect.x, data->UsedRect.y, data->UsedRect.w,
                   data->UsedRect.h, data->UpdateRect.x, data->UpdateRect.y,
                   data->UpdateRect.w, data->UpdateRect.h,
                   data->Updates.Size, pixel_hash);
        }
    }
    for (list_index = 0; list_index < draw_data->CmdLists.Size; ++list_index)
    {
        const ImDrawList* list = draw_data->CmdLists[list_index];
        int index;
        printf("list %d commands=%d indices=%d vertices=%d\n", list_index,
               list->CmdBuffer.Size, list->IdxBuffer.Size,
               list->VtxBuffer.Size);
        for (index = 0; index < list->CmdBuffer.Size; ++index)
        {
            const ImDrawCmd& command = list->CmdBuffer[index];
            printf("cmd %d clip=%08x,%08x,%08x,%08x elem=%u idx=%u vtx=%u "
                   "texture=%d callback=%d\n", index,
                   float_bits(command.ClipRect.x),
                   float_bits(command.ClipRect.y),
                   float_bits(command.ClipRect.z),
                   float_bits(command.ClipRect.w), command.ElemCount,
                   command.IdxOffset, command.VtxOffset,
                   command.GetTexID() == ImTextureID_Invalid ? 0 :
                   (command.GetTexID() == (ImTextureID)2 ? 2 : 1),
                   command.UserCallback == NULL ? 0 : 1);
        }
        for (index = 0; index < list->IdxBuffer.Size; ++index)
            printf("idx %d %u\n", index,
                   (unsigned int)list->IdxBuffer[index]);
        for (index = 0; index < list->VtxBuffer.Size; ++index)
        {
            const ImDrawVert& vertex = list->VtxBuffer[index];
            printf("vtx %d pos=%08x,%08x uv=%08x,%08x color=%08x\n",
                   index, float_bits(vertex.pos.x), float_bits(vertex.pos.y),
                   float_bits(vertex.uv.x), float_bits(vertex.uv.y),
                   vertex.col);
        }
    }
    texture->flat = false;
    printf("pixels atlas=%08x", render_hash(draw_data, texture));
    texture->flat = true;
    printf(" flat=%08x\n", render_hash(draw_data, texture));
}

static void build_scenario(int frame, bool* checked, float* value,
                           char* text, size_t text_size, bool* demo_open,
                           int* actions, int* key_hits,
                           ScenarioPoints* points)
{
    int row;
    int table_columns;
    const unsigned int quiet_nan_bits = 0x7fc00000U;
    float plot_values[5] = { -0.5f, 0.25f, 0.0f, 0.75f, 0.1f };
    memcpy(&plot_values[2], &quiet_nan_bits, sizeof(plot_values[2]));
    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(360.0f, 430.0f), ImGuiCond_Always);
    ImGui::Begin("Differential controls", NULL,
                 ImGuiWindowFlags_NoSavedSettings);
    ImGui::Text("Frame %d: deterministic UI", frame);
    if (frame == 10)
        ImGui::SetKeyboardFocusHere();
    if (ImGui::Button("Action", ImVec2(110.0f, 0.0f)))
        ++*actions;
    points->button = ImVec2(
        (ImGui::GetItemRectMin().x + ImGui::GetItemRectMax().x) * 0.5f,
        (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) * 0.5f);
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", checked);
    ImGui::SliderFloat("Value", value, -2.0f, 3.0f, "%.3f");
    if (frame == 4)
        ImGui::SetKeyboardFocusHere();
    ImGui::InputText("Text", text, text_size);
    points->input = ImVec2(
        (ImGui::GetItemRectMin().x + ImGui::GetItemRectMax().x) * 0.5f,
        (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) * 0.5f);
    if (ImGui::IsKeyPressed(ImGuiKey_A))
        ++*key_hits;
    ImGui::ProgressBar((float)(frame + 1) / 10.0f, ImVec2(180.0f, 0.0f));
    ImGui::PlotLines("NaN gap", plot_values, 5, 0, NULL, FLT_MAX, FLT_MAX,
                     ImVec2(180.0f, 36.0f));
    points->plot = ImVec2(
        (ImGui::GetItemRectMin().x + ImGui::GetItemRectMax().x) * 0.5f,
        (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) * 0.5f);
    printf("plot-item frame=%d id=%08x status=%08x\n", frame,
           ImGui::GetCurrentContext()->LastItemData.ID,
           ImGui::GetCurrentContext()->LastItemData.StatusFlags);
    table_columns = (frame >= 9 && frame < 12) ? 4 : 3;
    ImGui::GetCurrentContext()->DebugLogBuf.clear();
    ImGui::GetCurrentContext()->DebugLogIndex.clear();
    ImGui::GetCurrentContext()->DebugLogFlags |= ImGuiDebugLogFlags_EventTable;
    if (ImGui::BeginTable("values", table_columns,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                          ImGuiTableFlags_Resizable |
                          ImGuiTableFlags_Reorderable |
                          ImGuiTableFlags_Hideable |
                          ImGuiTableFlags_Sortable))
    {
        if (frame < 3 || frame >= 12)
        {
            ImGui::TableSetupColumn("Index");
            ImGui::TableSetupColumn("Square");
            ImGui::TableSetupColumn("Label");
        }
        else if (frame < 6)
        {
            ImGui::TableSetupColumn("Label");
            ImGui::TableSetupColumn("Index");
            ImGui::TableSetupColumn("Square");
        }
        else if (frame < 9)
        {
            ImGui::TableSetupColumn(NULL);
            ImGui::TableSetupColumn("Square");
            ImGui::TableSetupColumn("Label");
        }
        else
        {
            ImGui::TableSetupColumn("Square");
            ImGui::TableSetupColumn("Extra");
            ImGui::TableSetupColumn("Index");
            ImGui::TableSetupColumn("Label");
        }
        ImGui::TableHeadersRow();
        {
            ImGuiTable* query_table = ImGui::GetCurrentTable();
            ImGuiTableColumn* query_column = &query_table->Columns[0];
            ImRect query_bg = ImGui::TableGetCellBgRect(query_table, 0);
            printf("table-query frame=%d count=%d index=%d row=%d hover=%d/%d "
                   "name=%s flags=%08x resize=%08x bg=%.3f,%.3f,%.3f,%.3f "
                   "max=%.3f auto=%.3f sort-next=%d headers=%.3f/%.3f\n",
                   frame, ImGui::TableGetColumnCount(),
                   ImGui::TableGetColumnIndex(), ImGui::TableGetRowIndex(),
                   ImGui::TableGetHoveredColumn(), ImGui::TableGetHoveredRow(),
                   ImGui::TableGetColumnName(0), ImGui::TableGetColumnFlags(0),
                   ImGui::TableGetColumnResizeID(query_table, 0, 0),
                   query_bg.Min.x, query_bg.Min.y,
                   query_bg.Max.x, query_bg.Max.y,
                   ImGui::TableCalcMaxColumnWidth(query_table, 0),
                   ImGui::TableGetColumnWidthAuto(query_table, query_column),
                   ImGui::TableGetColumnNextSortDirection(query_column),
                   ImGui::TableGetHeaderRowHeight(),
                   ImGui::TableGetHeaderAngledMaxLabelWidth());
        }
        for (row = 0; row < 4; ++row)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", row);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", row * row);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("row-%d", row);
        }
        ImGui::EndTable();
    }
    ImGui::GetCurrentContext()->DebugLogFlags &= ~ImGuiDebugLogFlags_EventTable;
    printf("table-log frame=%d size=%d hash=%08x\n", frame,
           ImGui::GetCurrentContext()->DebugLogBuf.size(),
           hash_bytes(2166136261U,
                      ImGui::GetCurrentContext()->DebugLogBuf.c_str(),
                      (size_t)ImGui::GetCurrentContext()->DebugLogBuf.size()));
    ImGui::BeginChild("scrolling", ImVec2(0.0f, 90.0f),
                      ImGuiChildFlags_Borders);
    for (row = 0; row < 12; ++row)
        ImGui::Selectable(row == 7 ? "selected-row" : "ordinary-row",
                          row == 7);
    ImGui::EndChild();
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImDrawListFlags saved_flags = draw_list->Flags;
        ImVec2 polyline[5] = {
            ImVec2(cursor.x + 4.0f, cursor.y + 3.0f),
            ImVec2(cursor.x + 23.0f, cursor.y + 5.0f),
            ImVec2(cursor.x + 31.0f, cursor.y + 16.0f),
            ImVec2(cursor.x + 51.0f, cursor.y + 9.0f),
            ImVec2(cursor.x + 67.0f, cursor.y + 19.0f)
        };
        draw_list->AddRectFilled(cursor, ImVec2(cursor.x + 70.0f,
                                               cursor.y + 22.0f),
                                 IM_COL32(20, 100, 220, 180), 4.0f);
        draw_list->AddCircleFilled(ImVec2(cursor.x + 95.0f,
                                         cursor.y + 11.0f), 10.0f,
                                   IM_COL32(240, 80, 40, 220), 16);
        draw_list->AddPolyline(polyline, 5, IM_COL32(40, 220, 90, 230),
                               1.0f, ImDrawFlags_None);
        draw_list->AddPolyline(polyline, 5, IM_COL32(230, 190, 20, 210),
                               3.0f, ImDrawFlags_Closed);
        draw_list->Flags &= ~(ImDrawListFlags_AntiAliasedLines |
                              ImDrawListFlags_AntiAliasedLinesUseTex);
        draw_list->AddPolyline(polyline, 5, IM_COL32(190, 35, 220, 255),
                               2.0f, ImDrawFlags_None);
        draw_list->Flags = saved_flags;
        {
            ImRect range_rect(cursor.x + 125.0f, cursor.y,
                              cursor.x + 205.0f, cursor.y + 22.0f);
            ImGui::RenderRectFilledInRangeH(
                draw_list, range_rect, IM_COL32(210, 60, 90, 220),
                range_rect.Min.x + 3.0f, range_rect.Max.x - 17.0f, 8.0f);
            ImGui::RenderRectFilledInRangeH(
                draw_list, range_rect, IM_COL32(80, 180, 230, 180),
                range_rect.Min.x, range_rect.Max.x, 8.0f);
            ImGui::RenderRectFilledInRangeH(
                draw_list, range_rect, IM_COL32(240, 210, 40, 180),
                range_rect.Min.x + 12.0f, range_rect.Min.x + 30.0f, 0.0f);
            ImGui::RenderRectFilledInRangeH(
                draw_list, range_rect, IM_COL32(255, 255, 255, 255),
                range_rect.Max.x, range_rect.Min.x, 4.0f);
        }
        ImGui::Dummy(ImVec2(210.0f, 24.0f));
    }
    if (frame == 8)
        ImGui::OpenPopup("deterministic-popup");
    if (ImGui::BeginPopup("deterministic-popup"))
    {
        ImGui::TextUnformatted("popup-body");
        ImGui::EndPopup();
    }
    if (frame == 11)
        ImGui::NavMoveRequestTryWrapping(
            ImGui::GetCurrentWindow(), ImGuiNavMoveFlags_WrapX);
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(410.0f, 35.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(190.0f, 150.0f), ImGuiCond_Always);
    ImGui::Begin("Secondary", NULL,
                 ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoResize);
    ImGui::TextWrapped("A second window verifies ordering and clipping.");
    ImGui::ColorButton("color", ImVec4(0.2f, 0.7f, 0.3f, 0.8f));
    ImGui::Image(ImTextureRef((ImTextureID)2), ImVec2(48.0f, 32.0f));
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(390.0f, 205.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(245.0f, 260.0f), ImGuiCond_Always);
    ImGui::ShowDemoWindow(demo_open);
}

int main(int argc, char** argv)
{
    ImGuiContext* context;
    ImGuiIO* io;
    unsigned char* pixels;
    int atlas_width;
    int atlas_height;
    int bytes_per_pixel;
    DifferentialTexture texture;
    unsigned char user_pixels[4 * 4 * 4];
    bool checked = false;
    bool demo_open = true;
    float value = 0.25f;
    char text[64] = "seed";
    int actions = 0;
    int key_hits = 0;
    ScenarioPoints points;
    int frame;

    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "usage: %s framebuffer.rgba [font-file]\n", argv[0]);
        return 2;
    }
    pixel_stream = fopen(argv[1], "wb");
    if (pixel_stream == NULL)
        return 2;
    IMGUI_CHECKVERSION();
    context = ImGui::CreateContext();
    if (context == NULL)
        return 2;
    io = &ImGui::GetIO();
    io->DisplaySize = ImVec2(640.0f, 480.0f);
    io->DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io->DeltaTime = 1.0f / 60.0f;
    io->IniFilename = NULL;
    io->LogFilename = NULL;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    printf("datatype-ops hash=%08x\n", data_type_operation_hash());
#ifdef IMGUI_EMBEDDED_EXTERNAL_FONT
    if (io->Fonts->AddFontFromFileTTF(
            argc == 3 ? argv[2] : "third_party/ProggyClean.ttf", 13.0f) == NULL)
        return 2;
#endif
    io->Fonts->GetTexDataAsRGBA32(&pixels, &atlas_width, &atlas_height,
                                  &bytes_per_pixel);
    io->Fonts->SetTexID((ImTextureID)1);
    texture.atlas_pixels = pixels;
    texture.atlas_width = atlas_width;
    texture.atlas_height = atlas_height;
    texture.user_pixels = user_pixels;
    texture.user_width = 4;
    texture.user_height = 4;
    texture.flat = false;
    {
        int pixel_index;
        for (pixel_index = 0; pixel_index < 16; ++pixel_index)
        {
            bool alternate = ((pixel_index / 4) + (pixel_index % 4)) % 2 != 0;
            user_pixels[pixel_index * 4 + 0] = alternate ? 240 : 20;
            user_pixels[pixel_index * 4 + 1] = alternate ? 80 : 190;
            user_pixels[pixel_index * 4 + 2] = alternate ? 30 : 230;
            user_pixels[pixel_index * 4 + 3] = 255;
        }
    }
    printf("differential-v1 atlas=%dx%d bpp=%d hash=%08x\n",
           atlas_width, atlas_height, bytes_per_pixel,
           hash_bytes(2166136261U, pixels,
                      (size_t)atlas_width * (size_t)atlas_height *
                      (size_t)bytes_per_pixel));
    {
        int key_value;
        for (key_value = ImGuiKey_NamedKey_BEGIN;
             key_value < ImGuiKey_NamedKey_END; ++key_value)
            printf("key-name %d %s\n", key_value,
                   ImGui::GetKeyName((ImGuiKey)key_value));
    }

    points.button = ImVec2(55.0f, 82.0f);
    points.input = ImVec2(100.0f, 125.0f);
    points.plot = ImVec2(100.0f, 220.0f);
    for (frame = 0; frame < 14; ++frame)
    {
        if (frame == 0)
            io->AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        if (frame == 1)
            io->AddMousePosEvent(points.button.x, points.button.y);
        if (frame == 2)
            io->AddMouseButtonEvent(0, true);
        if (frame == 3)
            io->AddMouseButtonEvent(0, false);
        if (frame == 4)
            io->AddMousePosEvent(points.input.x, points.input.y);
        if (frame == 5)
            io->AddMouseButtonEvent(0, true);
        if (frame == 6)
        {
            io->AddMouseButtonEvent(0, false);
            io->AddInputCharactersUTF8("-input");
        }
        if (frame == 7)
            io->AddKeyEvent(ImGuiKey_A, true);
        if (frame == 8)
        {
            io->AddKeyEvent(ImGuiKey_A, false);
            io->AddMouseWheelEvent(0.0f, -1.0f);
        }
        if (frame == 11)
            io->AddKeyEvent(ImGuiKey_LeftArrow, true);
        if (frame == 12)
            io->AddKeyEvent(ImGuiKey_LeftArrow, false);
        if (frame == 13)
            io->AddMousePosEvent(points.plot.x, points.plot.y);
        io->DeltaTime = 1.0f / 60.0f;
        ImGui::NewFrame();
        if (frame == 9)
        {
            context->NavWindowingTarget =
                context->NavWindowingTargetAnim =
                    ImGui::FindWindowByName("Secondary");
            context->NavWindowingTimer = 1.0f;
        }
        if (frame == 10)
            context->NavWindowingTarget =
                context->NavWindowingTargetAnim = NULL;
#ifdef IMGUI_HAS_DOCK
        /* Exercise docking-branch state/draw paths without changing this
         * source between the native and generated-C builds. */
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);
#endif
        build_scenario(frame, &checked, &value, text, sizeof(text),
                       &demo_open, &actions, &key_hits, &points);
        ImGui::Render();
        emit_draw_data(frame, ImGui::GetDrawData(), &texture);
    }
    printf("final checked=%d value=%08x text=%s actions=%d keys=%d frame=%d\n",
           checked ? 1 : 0, float_bits(value), text, actions, key_hits,
           ImGui::GetFrameCount());
    {
        size_t first_size;
        size_t second_size;
        unsigned int first_hash;
        unsigned int second_hash;
        const char* first_ini = ImGui::SaveIniSettingsToMemory(&first_size);
        std::vector<char> saved_ini(first_ini, first_ini + first_size);
        first_hash = hash_bytes(2166136261U, first_ini, first_size);
        ImGui::ClearIniSettings();
        ImGui::LoadIniSettingsFromMemory(&saved_ini[0], saved_ini.size());
        second_hash = hash_bytes(
            2166136261U, ImGui::SaveIniSettingsToMemory(&second_size),
            second_size);
        printf("ini first=%u,%08x roundtrip=%u,%08x exact=%d\n",
               (unsigned int)first_size, first_hash,
               (unsigned int)second_size, second_hash,
               first_size == second_size && first_hash == second_hash ? 1 : 0);
    }
    ImGui::DestroyContext(context);
    if (fclose(pixel_stream) != 0)
        return 3;
    return 0;
}

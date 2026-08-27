/*
 * Optional differential runner for the pinned Dear ImGui C++ reference.
 *
 * Build this translation unit together with imgui.cpp, imgui_draw.cpp,
 * imgui_tables.cpp, and imgui_widgets.cpp from the snapshot named in
 * UPSTREAM.md. It intentionally uses only the same tiny trace vocabulary as
 * imgui_c89_trace_runner.c, then emits normalized draw-data statistics.
 */
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_c89_software.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <vector>

typedef struct reference_texture_data {
    const unsigned char* pixels;
    int width;
    int height;
    int flat;
} reference_texture_data;

static const unsigned char* reference_font_pixels;
static int reference_font_width;
static int reference_font_height;

static imgui_rect reference_make_rect(float x1, float y1, float x2, float y2)
{
    imgui_rect rect;
    rect.x1 = x1;
    rect.y1 = y1;
    rect.x2 = x2;
    rect.y2 = y2;
    return rect;
}

static imgui_bool reference_texture_resolver(
    const imgui_texture*, imgui_software_texture_view* view, void* user_data)
{
    reference_texture_data* data = (reference_texture_data*)user_data;
    if (data == NULL || view == NULL || data->pixels == NULL) return IMGUI_FALSE;
    if (data->flat) {
        static const unsigned char white[4] = {255, 255, 255, 255};
        view->pixels = white;
        view->mutable_pixels = NULL;
        view->width = 1;
        view->height = 1;
        view->stride = 4U;
        view->format = IMGUI_TEXTURE_FORMAT_RGBA8;
        return IMGUI_TRUE;
    }
    view->pixels = data->pixels;
    view->mutable_pixels = NULL;
    view->width = data->width;
    view->height = data->height;
    view->stride = (size_t)data->width * 4U;
    view->format = IMGUI_TEXTURE_FORMAT_RGBA8;
    return IMGUI_TRUE;
}

static unsigned int reference_pixel_hash(const unsigned char* pixels,
                                         size_t length)
{
    unsigned int hash = 2166136261U;
    size_t index;
    for (index = 0; index < length; ++index) {
        hash ^= (unsigned int)pixels[index];
        hash *= 16777619U;
    }
    return hash;
}

static bool reference_render_pixels(ImDrawData* draw_data,
                                    const unsigned char* font_pixels,
                                    int font_width, int font_height)
{
    std::vector<imgui_render_vertex> vertices;
    std::vector<imgui_render_index> indices;
    std::vector<imgui_render_command> commands;
    imgui_render_list render_list;
    imgui_viewport_packet viewport;
    imgui_render_packet packet;
    imgui_software_target target;
    reference_texture_data texture_data;
    std::vector<unsigned char> framebuffer(640U * 480U * 4U);
    if (draw_data == NULL || draw_data->CmdListsCount <= 0) {
        memset(framebuffer.data(), 0, framebuffer.size());
    } else {
        const ImDrawList* source = draw_data->CmdLists[0];
        int vertex_index;
        int index_index;
        int command_index;
        vertices.resize((size_t)source->VtxBuffer.Size);
        indices.resize((size_t)source->IdxBuffer.Size);
        commands.resize((size_t)source->CmdBuffer.Size);
        for (vertex_index = 0; vertex_index < source->VtxBuffer.Size;
             ++vertex_index) {
            vertices[(size_t)vertex_index].position = imgui_make_vec2(
                source->VtxBuffer[vertex_index].pos.x,
                source->VtxBuffer[vertex_index].pos.y);
            vertices[(size_t)vertex_index].uv = imgui_make_vec2(
                source->VtxBuffer[vertex_index].uv.x,
                source->VtxBuffer[vertex_index].uv.y);
            vertices[(size_t)vertex_index].color =
                (imgui_u32)source->VtxBuffer[vertex_index].col;
        }
        for (index_index = 0; index_index < source->IdxBuffer.Size;
             ++index_index)
            indices[(size_t)index_index] =
                (imgui_render_index)source->IdxBuffer[index_index];
        for (command_index = 0; command_index < source->CmdBuffer.Size;
             ++command_index) {
            const ImDrawCmd& source_command = source->CmdBuffer[command_index];
            memset(&commands[(size_t)command_index], 0,
                   sizeof(commands[(size_t)command_index]));
            if (source_command.UserCallback != NULL) continue;
            commands[(size_t)command_index].type =
                IMGUI_RENDER_COMMAND_DRAW_INDEXED;
            commands[(size_t)command_index].data.draw_indexed.clip_rect =
                reference_make_rect(source_command.ClipRect.x,
                                    source_command.ClipRect.y,
                                    source_command.ClipRect.z,
                                    source_command.ClipRect.w);
            commands[(size_t)command_index].data.draw_indexed.texture =
                (imgui_texture*)1;
            commands[(size_t)command_index].data.draw_indexed.index_offset =
                source_command.IdxOffset;
            commands[(size_t)command_index].data.draw_indexed.index_count =
                source_command.ElemCount;
            commands[(size_t)command_index].data.draw_indexed.vertex_offset =
                source_command.VtxOffset;
        }
    }
    memset(&render_list, 0, sizeof(render_list));
    render_list.vertices = vertices.empty() ? NULL : &vertices[0];
    render_list.vertex_count = (imgui_u32)vertices.size();
    render_list.indices = indices.empty() ? NULL : &indices[0];
    render_list.index_count = (imgui_u32)indices.size();
    render_list.commands = commands.empty() ? NULL : &commands[0];
    render_list.command_count = (imgui_u32)commands.size();
    memset(&viewport, 0, sizeof(viewport));
    viewport.viewport_id = 0;
    viewport.display_size = imgui_make_vec2(640.0f, 480.0f);
    viewport.framebuffer_scale = imgui_make_vec2(1.0f, 1.0f);
    viewport.lists = &render_list;
    viewport.list_count = 1;
    memset(&packet, 0, sizeof(packet));
    packet.struct_size = sizeof(packet);
    packet.protocol_version = 1;
    packet.viewports = &viewport;
    packet.viewport_count = 1;
    texture_data.pixels = font_pixels;
    texture_data.width = font_width;
    texture_data.height = font_height;
    texture_data.flat = 0;
    imgui_software_target_init(&target);
    target.pixels = &framebuffer[0];
    target.width = 640;
    target.height = 480;
    target.stride = 640U * 4U;
    target.texture_resolver = reference_texture_resolver;
    target.texture_user_data = &texture_data;
    if (imgui_software_render_packet(&packet, &target) != IMGUI_RESULT_OK)
        return false;
    printf("pixel_hash=%u\n",
           reference_pixel_hash(&framebuffer[0], framebuffer.size()));
    texture_data.flat = 1;
    memset(framebuffer.data(), 0, framebuffer.size());
    if (imgui_software_render_packet(&packet, &target) != IMGUI_RESULT_OK)
        return false;
    printf("flat_pixel_hash=%u\n",
           reference_pixel_hash(&framebuffer[0], framebuffer.size()));
    return true;
}

static unsigned int trace_hash_bytes(unsigned int hash, const void* data,
                                     size_t length)
{
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t index = 0; index < length; ++index) {
        hash ^= (unsigned int)bytes[index];
        hash *= 16777619U;
    }
    return hash;
}

static unsigned int trace_hash_vertex(unsigned int hash,
                                      const ImDrawVert* vertex)
{
    hash = trace_hash_bytes(hash, &vertex->pos, sizeof(vertex->pos));
    return trace_hash_bytes(hash, &vertex->col, sizeof(vertex->col));
}

static int parse_trace(FILE *file, ImVec2 *mouse_position, int dump)
{
    char line[256];
    if (file == NULL || mouse_position == NULL) return 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        float x;
        float y;
        float delta;
        double time;
        if (sscanf(line, "P %f %f", &x, &y) == 2) {
            mouse_position->x = x;
            mouse_position->y = y;
            continue;
        }
        if (sscanf(line, "R %f %lf", &delta, &time) == 2) {
            ImGuiIO& io = ImGui::GetIO();
            io.DeltaTime = delta > 0.0f ? delta : 1.0f / 60.0f;
            io.MousePos = *mouse_position;
            (void)time;
            ImGui::NewFrame();
            if (ImGui::Begin("Trace runner"))
                ImGui::TextUnformatted("Deterministic C89 trace frame");
            ImGui::End();
            ImGui::Render();
            {
                ImDrawData* draw_data = ImGui::GetDrawData();
                int command_count = 0;
                if (draw_data != NULL && draw_data->CmdListsCount > 0 &&
                    draw_data->CmdLists[0] != NULL)
                    command_count = draw_data->CmdLists[0]->CmdBuffer.Size;
                printf("frame=%d time=%.17g viewports=%d commands=%d "
                       "vertices=%d indices=%d\n",
                       ImGui::GetFrameCount(), time,
                       draw_data != NULL ? 1 : 0, command_count,
                       draw_data != NULL ? draw_data->TotalVtxCount : 0,
                       draw_data != NULL ? draw_data->TotalIdxCount : 0);
                if (dump && draw_data != NULL)
                    (void)reference_render_pixels(
                        draw_data, reference_font_pixels,
                        reference_font_width, reference_font_height);
                if (dump && GImGui->Windows.Size > 0) {
                    int window_index;
                    for (window_index = 0; window_index < GImGui->Windows.Size;
                         ++window_index) {
                        const ImGuiWindow* window = GImGui->Windows[window_index];
                        printf("window[%d] name=%s pos=%.9g,%.9g size=%.9g,%.9g\n",
                               window_index, window->Name,
                               (double)window->Pos.x, (double)window->Pos.y,
                               (double)window->Size.x, (double)window->Size.y);
                    }
                }
                if (dump && draw_data != NULL &&
                    draw_data->CmdListsCount > 0 &&
                    draw_data->CmdLists[0] != NULL) {
                    int list_index;
                    unsigned int geometry_hash = 2166136261U;
                    for (list_index = 0; list_index < draw_data->CmdListsCount;
                         ++list_index) {
                        const ImDrawList* list = draw_data->CmdLists[list_index];
                        int command_index;
                        printf("list=%d commands=%d vertices=%d indices=%d\n",
                               list_index, list->CmdBuffer.Size,
                               list->VtxBuffer.Size, list->IdxBuffer.Size);
                        for (command_index = 0;
                             command_index < list->CmdBuffer.Size;
                             ++command_index) {
                            const ImDrawCmd& command = list->CmdBuffer[command_index];
                            geometry_hash = trace_hash_bytes(
                                geometry_hash, &command.ClipRect,
                                sizeof(command.ClipRect));
                            for (unsigned int draw_index = 0;
                                 draw_index < command.ElemCount; ++draw_index) {
                                unsigned int index_offset = command.IdxOffset +
                                                            draw_index;
                                if (index_offset < (unsigned int)list->IdxBuffer.Size) {
                                    ImDrawIdx vertex_index =
                                        list->IdxBuffer[index_offset];
                                    unsigned int vertex_offset =
                                        command.VtxOffset +
                                        (unsigned int)vertex_index;
                                    if (vertex_offset <
                                        (unsigned int)list->VtxBuffer.Size) {
                                        geometry_hash = trace_hash_vertex(
                                            geometry_hash,
                                            &list->VtxBuffer[vertex_offset]);
                                    }
                                }
                            }
                            printf("draw type=0 clip=%.9g,%.9g,%.9g,%.9g "
                                   "elem=%u idx=%u vtx=%u\n",
                                   (double)command.ClipRect.x,
                                   (double)command.ClipRect.y,
                                   (double)command.ClipRect.z,
                                   (double)command.ClipRect.w,
                                   (unsigned int)command.ElemCount,
                                   (unsigned int)command.IdxOffset,
                                   (unsigned int)command.VtxOffset);
                            if (command.ElemCount != 0) {
                                unsigned int first_index = command.IdxOffset;
                                unsigned int last_index = command.IdxOffset +
                                    command.ElemCount - 1U;
                                if (first_index < (unsigned int)list->IdxBuffer.Size &&
                                    last_index < (unsigned int)list->IdxBuffer.Size) {
                                    ImDrawIdx first_vertex = list->IdxBuffer[first_index];
                                    ImDrawIdx last_vertex = list->IdxBuffer[last_index];
                                    if (first_vertex < (ImDrawIdx)list->VtxBuffer.Size &&
                                        last_vertex < (ImDrawIdx)list->VtxBuffer.Size) {
                                        printf("range first=%.9g,%.9g last=%.9g,%.9g\n",
                                               (double)list->VtxBuffer[first_vertex].pos.x,
                                               (double)list->VtxBuffer[first_vertex].pos.y,
                                               (double)list->VtxBuffer[last_vertex].pos.x,
                                               (double)list->VtxBuffer[last_vertex].pos.y);
                                    }
                                }
                            }
                        }
                    }
                    printf("geometry_hash=%u\n", geometry_hash);
                }
            }
        }
    }
    return ferror(file) == 0;
}

int main(int argc, char** argv)
{
    FILE* file;
    FILE* font_file;
    long font_size;
    void* font_data;
    unsigned char* pixels;
    int width;
    int height;
    int bytes_per_pixel;
    int dump;
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "usage: %s trace.txt [--dump]\n", argv[0]);
        return 2;
    }
    dump = argc == 3 && strcmp(argv[2], "--dump") == 0;
    if (argc == 3 && !dump) return 2;
    file = fopen(argv[1], "rb");
    if (file == NULL) {
        fprintf(stderr, "cannot open trace: %s\n", argv[1]);
        return 2;
    }
    ImGui::CreateContext();
    {
        ImGuiIO& io = ImGui::GetIO();
        font_file = fopen("third_party/ProggyClean.ttf", "rb");
        font_data = NULL;
        if (font_file != NULL && fseek(font_file, 0L, SEEK_END) == 0) {
            font_size = ftell(font_file);
            if (font_size > 0L && fseek(font_file, 0L, SEEK_SET) == 0) {
                font_data = malloc((size_t)font_size);
                if (font_data != NULL &&
                    fread(font_data, 1, (size_t)font_size, font_file) !=
                    (size_t)font_size) {
                    free(font_data);
                    font_data = NULL;
                }
            }
        }
        if (font_file != NULL) fclose(font_file);
        if (font_data != NULL) {
            ImFontConfig font_config;
            font_config.SizePixels = 13.0f;
            font_config.OversampleH = 1;
            font_config.OversampleV = 1;
            font_config.PixelSnapH = true;
            io.Fonts->Clear();
            io.Fonts->AddFontFromMemoryTTF(font_data, (int)font_size,
                                           13.0f, &font_config);
        }
        io.DisplaySize = ImVec2(640.0f, 480.0f);
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
        io.IniFilename = NULL;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height,
                                     &bytes_per_pixel);
        reference_font_pixels = pixels;
        reference_font_width = width;
        reference_font_height = height;
    }
    {
        ImVec2 mouse_position(-FLT_MAX, -FLT_MAX);
        int result = parse_trace(file, &mouse_position, dump) ? 0 : 1;
        fclose(file);
        ImGui::DestroyContext();
        return result;
    }
}

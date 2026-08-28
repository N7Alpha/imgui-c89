#include "imgui.h"
#ifndef IMGUI_TRANSLATED_HAVE_DEMO
#define IMGUI_TRANSLATED_HAVE_DEMO 0
#endif
#ifndef IMGUI_TRANSLATED_HAVE_NULL_BACKEND
#define IMGUI_TRANSLATED_HAVE_NULL_BACKEND 0
#endif
#ifndef IMGUI_TRANSLATED_HAVE_SDL3_BACKEND
#define IMGUI_TRANSLATED_HAVE_SDL3_BACKEND 0
#endif
#if IMGUI_TRANSLATED_HAVE_NULL_BACKEND
#include "backends/imgui_impl_null.h"
#endif
#if IMGUI_TRANSLATED_HAVE_SDL3_BACKEND
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL.h>
#endif

int main()
{
    bool open = true;
    ImGuiContext* context;
    int frame;
#if IMGUI_TRANSLATED_HAVE_SDL3_BACKEND
    SDL_Window* window;
    SDL_Renderer* renderer;
#endif
#if !IMGUI_TRANSLATED_HAVE_NULL_BACKEND
    unsigned char* pixels;
    int texture_width;
    int texture_height;
#endif

    IMGUI_CHECKVERSION();
    context = ImGui::CreateContext();
    if (context == 0)
        return 1;
#ifdef IMGUI_EMBEDDED_EXTERNAL_FONT
    if (ImGui::GetIO().Fonts->AddFontFromFileTTF(
            "third_party/ProggyClean.ttf", 13.0f) == 0)
        return 12;
#endif
#if IMGUI_TRANSLATED_HAVE_SDL3_BACKEND
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
    if (!SDL_Init(SDL_INIT_VIDEO))
        return 2;
    window = SDL_CreateWindow(
        "imgui-c89 translated smoke", 640, 480, SDL_WINDOW_HIDDEN);
    if (window == 0)
        return 3;
    renderer = SDL_CreateRenderer(window, 0);
    if (renderer == 0)
        return 4;
    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer))
        return 5;
    if (!ImGui_ImplSDLRenderer3_Init(renderer))
        return 6;
#elif IMGUI_TRANSLATED_HAVE_NULL_BACKEND
    if (!ImGui_ImplNull_Init())
        return 2;
#else
    ImGui::GetIO().DisplaySize = ImVec2(640.0f, 480.0f);
    ImGui::GetIO().DeltaTime = 1.0f / 60.0f;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(
        &pixels, &texture_width, &texture_height);
#endif
    for (frame = 0; frame < 2; ++frame)
    {
#if IMGUI_TRANSLATED_HAVE_SDL3_BACKEND
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
#elif IMGUI_TRANSLATED_HAVE_NULL_BACKEND
        ImGui_ImplNull_NewFrame();
#endif
        ImGui::NewFrame();
        if (frame == 0)
        {
            if (!ImGui::Begin("translated-inactive-cpp", &open))
            {
                ImGui::End();
                return 8;
            }
            ImGui::End();
        }
        else
        {
            ImGui::SetWindowCollapsed(
                "translated-inactive-cpp", true, ImGuiCond_Always);
            if (ImGui::Begin("translated-inactive-cpp", &open))
            {
                ImGui::End();
                return 9;
            }
            /* The exact C++ facade deliberately preserves upstream's legacy
               rule: End is mandatory even when Begin returns false. */
            ImGui::End();
        }
        if (!ImGui::Begin("translated-smoke", &open))
        {
            ImGui::End();
            return 3;
        }
        ImGui::Text("translated value: %d", 42);
        if (!ImGui::BeginTable("translated-table", 2))
        {
            ImGui::End();
            return 13;
        }
        ImGui::TableNextColumn();
        ImGui::Text("table value: %d", frame);
        ImGui::EndTable();
        if (frame == 0)
        {
            if (!ImGui::BeginChild(
                    "translated-inactive-cpp-child",
                    ImVec2(100.0f, 100.0f)))
            {
                ImGui::EndChild();
                ImGui::End();
                return 10;
            }
            ImGui::EndChild();
        }
        else
        {
            ImGui::SetCursorPos(ImVec2(100000.0f, 100000.0f));
            if (ImGui::BeginChild(
                    "translated-inactive-cpp-child",
                    ImVec2(100.0f, 100.0f)))
            {
                ImGui::EndChild();
                ImGui::End();
                return 11;
            }
            /* BeginChild has the same upstream-only exception. */
            ImGui::EndChild();
        }
        ImGui::End();
#if IMGUI_TRANSLATED_HAVE_DEMO
        if (frame == 0)
            ImGui::ShowDemoWindow(&open);
#endif
        ImGui::Render();
    }
    if (ImGui::GetDrawData() == 0)
        return 7;

#if IMGUI_TRANSLATED_HAVE_SDL3_BACKEND
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
#elif IMGUI_TRANSLATED_HAVE_NULL_BACKEND
    ImGui_ImplNull_Shutdown();
#endif
    ImGui::DestroyContext(context);
    return 0;
}

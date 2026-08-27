#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlgpu3.h"
#include <SDL3/SDL.h>
#include <stdio.h>

static void (*platform_create_original)(ImGuiViewport *viewport);
static void (*platform_destroy_original)(ImGuiViewport *viewport);
static void (*renderer_create_original)(ImGuiViewport *viewport);
static void (*renderer_destroy_original)(ImGuiViewport *viewport);
static int platform_creates;
static int platform_destroys;
static int renderer_creates;
static int renderer_destroys;

static void platform_create_counted(ImGuiViewport *viewport)
{
    ++platform_creates;
    platform_create_original(viewport);
}

static void platform_destroy_counted(ImGuiViewport *viewport)
{
    ++platform_destroys;
    platform_destroy_original(viewport);
}

static void renderer_create_counted(ImGuiViewport *viewport)
{
    ++renderer_creates;
    renderer_create_original(viewport);
}

static void renderer_destroy_counted(ImGuiViewport *viewport)
{
    ++renderer_destroys;
    renderer_destroy_original(viewport);
}

static int fail(const char *operation)
{
    fprintf(stderr, "%s: %s\n", operation, SDL_GetError());
    return 1;
}

int main()
{
    SDL_Window *window;
    SDL_GPUDevice *device;
    ImGuiContext *context;
    ImGuiIO *io;
    ImGuiPlatformIO *platform_io;
    ImGui_ImplSDLGPU3_InitInfo init_info;
    int frame;
    int secondary_viewports;
    int live_platform_windows;
    int live_renderer_windows;

    if (!SDL_Init(SDL_INIT_VIDEO))
        return fail("SDL_Init");
    window = SDL_CreateWindow(
        "imgui-c89 real viewport gate", 320, 240,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (window == 0)
        return fail("SDL_CreateWindow");
    device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL |
        SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_METALLIB,
        false, 0);
    if (device == 0)
        return fail("SDL_CreateGPUDevice");
    if (!SDL_ClaimWindowForGPUDevice(device, window))
        return fail("SDL_ClaimWindowForGPUDevice");
    if (!SDL_SetGPUSwapchainParameters(
            device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
            SDL_GPU_PRESENTMODE_IMMEDIATE))
        return fail("SDL_SetGPUSwapchainParameters");

    IMGUI_CHECKVERSION();
    context = ImGui::CreateContext();
    if (context == 0)
        return 2;
    io = &ImGui::GetIO();
    io->IniFilename = 0;
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    if (!ImGui_ImplSDL3_InitForSDLGPU(window))
        return fail("ImGui_ImplSDL3_InitForSDLGPU");
    init_info.Device = device;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(device, window);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
    init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
    init_info.PresentMode = SDL_GPU_PRESENTMODE_IMMEDIATE;
    if (!ImGui_ImplSDLGPU3_Init(&init_info))
        return fail("ImGui_ImplSDLGPU3_Init");

    platform_io = &ImGui::GetPlatformIO();
    platform_create_original = platform_io->Platform_CreateWindow;
    platform_destroy_original = platform_io->Platform_DestroyWindow;
    renderer_create_original = platform_io->Renderer_CreateWindow;
    renderer_destroy_original = platform_io->Renderer_DestroyWindow;
    if (platform_create_original == 0 || platform_destroy_original == 0 ||
        renderer_create_original == 0 || renderer_destroy_original == 0)
        return 3;
    platform_io->Platform_CreateWindow = platform_create_counted;
    platform_io->Platform_DestroyWindow = platform_destroy_counted;
    platform_io->Renderer_CreateWindow = renderer_create_counted;
    platform_io->Renderer_DestroyWindow = renderer_destroy_counted;

    for (frame = 0; frame < 4; ++frame)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
            ImGui_ImplSDL3_ProcessEvent(&event);
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(700.0f, 100.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(220.0f, 140.0f), ImGuiCond_Always);
        ImGui::Begin("detached-real-sdl-window", 0,
                     ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextUnformatted("real SDL_GPU viewport", 0);
        ImGui::End();
        ImGui::Render();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    secondary_viewports = platform_io->Viewports.Size - 1;
    live_platform_windows = 0;
    live_renderer_windows = 0;
    for (frame = 1; frame < platform_io->Viewports.Size; ++frame)
    {
        ImGuiViewport *viewport = platform_io->Viewports[frame];
        if (viewport->PlatformHandle != 0 && viewport->PlatformUserData != 0)
            ++live_platform_windows;
        if (viewport->RendererUserData != 0)
            ++live_renderer_windows;
    }
    if (secondary_viewports < 1 || live_platform_windows < 1 ||
        live_renderer_windows < 1 || platform_creates < 1 || renderer_creates < 1)
        return 4;

    ImGui::DestroyPlatformWindows();
    /* DestroyPlatformWindows also tears down the already-created main
       viewport data, whose create happened before our counters were hooked. */
    if (platform_destroys != platform_creates + 1 ||
        renderer_destroys != renderer_creates + 1)
    {
        fprintf(stderr, "destroy mismatch: platform=%d/%d renderer=%d/%d\n",
                platform_creates, platform_destroys,
                renderer_creates, renderer_destroys);
        return 5;
    }
    printf(
        "SDL_GPU viewport PASS: secondary=%d platform=%d/%d renderer=%d/%d\n",
        secondary_viewports, platform_creates, platform_destroys,
        renderer_creates, renderer_destroys);

    SDL_WaitForGPUIdle(device);
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext(context);
    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

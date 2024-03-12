// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "user_interface/user_interface.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

namespace dnf_composer
{
    namespace user_interface
    {
        UserInterface::UserInterface()
        {
            //closeUI = false;
            //windowHandle = nullptr;
            //windowClass = {};
        }

        void UserInterface::init()
        {

        }

        void UserInterface::step()
        {
            glfwSetErrorCallback(glfw_error_callback);
            if (!glfwInit())
                return;

            // Decide GL+GLSL versions
        #if defined(IMGUI_IMPL_OPENGL_ES2)
            // GL ES 2.0 + GLSL 100
            const char* glsl_version = "#version 100";
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        #elif defined(__APPLE__)
            // GL 3.2 + GLSL 150
            const char* glsl_version = "#version 150";
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
        #else
            // GL 3.0 + GLSL 130
            const char* glsl_version = "#version 130";
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
            //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
        #endif

            // Create window with graphics context
            GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
            if (window == nullptr)
                return;
            glfwMakeContextCurrent(window);
            glfwSwapInterval(1); // Enable vsync

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
            //io.ConfigViewportsNoAutoMerge = true;
            //io.ConfigViewportsNoTaskBarIcon = true;

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsLight();

            // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
            ImGuiStyle& style = ImGui::GetStyle();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }

            // Setup Platform/Renderer backends
            ImGui_ImplGlfw_InitForOpenGL(window, true);
        #ifdef __EMSCRIPTEN__
            ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
        #endif
            ImGui_ImplOpenGL3_Init(glsl_version);

            // Load Fonts
            // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
            // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
            // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
            // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
            // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
            // - Read 'docs/FONTS.md' for more instructions and details.
            // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
            // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
            //io.Fonts->AddFontDefault();
            //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
            //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
            //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
            //IM_ASSERT(font != nullptr);

            // Our state
            bool show_demo_window = true;
            bool show_another_window = false;
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

            // Main loop
        #ifdef __EMSCRIPTEN__
            // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
            // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
            io.IniFilename = nullptr;
            EMSCRIPTEN_MAINLOOP_BEGIN
        #else
            while (!glfwWindowShouldClose(window))
        #endif
            {
                // Poll and handle events (inputs, window resize, etc.)
                // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
                // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
                // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
                // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
                glfwPollEvents();

                // Start the Dear ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
                if (show_demo_window)
                    ImGui::ShowDemoWindow(&show_demo_window);

                // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
                {
                    static float f = 0.0f;
                    static int counter = 0;

                    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Another Window", &show_another_window);

                    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                        counter++;
                    ImGui::SameLine();
                    ImGui::Text("counter = %d", counter);

                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                    ImGui::End();
                }

                // 3. Show another simple window.
                if (show_another_window)
                {
                    ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                    ImGui::Text("Hello from another window!");
                    if (ImGui::Button("Close Me"))
                        show_another_window = false;
                    ImGui::End();
                }

                // Rendering
                ImGui::Render();
                int display_w, display_h;
                glfwGetFramebufferSize(window, &display_w, &display_h);
                //glViewport(0, 0, display_w, display_h);
                //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
                //glClear(GL_COLOR_BUFFER_BIT);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                // Update and Render additional Platform Windows
                // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
                //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                {
                    GLFWwindow* backup_current_context = glfwGetCurrentContext();
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                    glfwMakeContextCurrent(backup_current_context);
                }

                glfwSwapBuffers(window);
            }
        #ifdef __EMSCRIPTEN__
            EMSCRIPTEN_MAINLOOP_END;
        #endif

            // Cleanup
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            glfwDestroyWindow(window);
            glfwTerminate();
        }

        void UserInterface::close() const
        {

        }

        void UserInterface::activateWindow(const std::shared_ptr<UserInterfaceWindow>& window)
        {
            windows.push_back(window);
        }

        bool UserInterface::getCloseUI() const
        {
            return closeUI;
        }

        void UserInterface::render() const
        {
            for (const auto& window : windows)
                window->render();
        }
    }
		
}



// // Helper functions
// bool CreateDeviceD3D(HWND hWnd)
// {
//     // Setup swap chain
//     DXGI_SWAP_CHAIN_DESC1 sd;
//     {
//         ZeroMemory(&sd, sizeof(sd));
//         sd.BufferCount = NUM_BACK_BUFFERS;
//         sd.Width = 0;
//         sd.Height = 0;
//         sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//         sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
//         sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//         sd.SampleDesc.Count = 1;
//         sd.SampleDesc.Quality = 0;
//         sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//         sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
//         sd.Scaling = DXGI_SCALING_STRETCH;
//         sd.Stereo = FALSE;
//     }

//     // [DEBUG] Enable debug interface
// #ifdef DX12_ENABLE_DEBUG_LAYER
//     ID3D12Debug* pdx12Debug = nullptr;
//     if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
//         pdx12Debug->EnableDebugLayer();
// #endif

//     // Create device
//     D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
//     if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
//         return false;

//     // [DEBUG] Setup debug interface to break on any warnings/errors
// #ifdef DX12_ENABLE_DEBUG_LAYER
//     if (pdx12Debug != nullptr)
//     {
//         ID3D12InfoQueue* pInfoQueue = nullptr;
//         g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
//         pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
//         pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
//         pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
//         pInfoQueue->Release();
//         pdx12Debug->Release();
//     }
// #endif

//     {
//         D3D12_DESCRIPTOR_HEAP_DESC desc = {};
//         desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
//         desc.NumDescriptors = NUM_BACK_BUFFERS;
//         desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//         desc.NodeMask = 1;
//         if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
//             return false;

//         SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//         D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
//         for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
//         {
//             g_mainRenderTargetDescriptor[i] = rtvHandle;
//             rtvHandle.ptr += rtvDescriptorSize;
//         }
//     }

//     {
//         D3D12_DESCRIPTOR_HEAP_DESC desc = {};
//         desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//         desc.NumDescriptors = 1;
//         desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//         if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
//             return false;
//     }

//     {
//         D3D12_COMMAND_QUEUE_DESC desc = {};
//         desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
//         desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//         desc.NodeMask = 1;
//         if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
//             return false;
//     }

//     for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
//         if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
//             return false;

//     if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
//         g_pd3dCommandList->Close() != S_OK)
//         return false;

//     if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
//         return false;

//     g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
//     if (g_fenceEvent == nullptr)
//         return false;

//     {
//         IDXGIFactory4* dxgiFactory = nullptr;
//         IDXGISwapChain1* swapChain1 = nullptr;
//         if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
//             return false;
//         if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
//             return false;
//         if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
//             return false;
//         swapChain1->Release();
//         dxgiFactory->Release();
//         g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
//         g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
//     }

//     CreateRenderTarget();
//     return true;
// }

// void CleanupDeviceD3D()
// {
//     CleanupRenderTarget();
//     if (g_pSwapChain) { g_pSwapChain->SetFullscreenState(false, nullptr); g_pSwapChain->Release(); g_pSwapChain = nullptr; }
//     if (g_hSwapChainWaitableObject != nullptr) { CloseHandle(g_hSwapChainWaitableObject); }
//     for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
//         if (g_frameContext[i].CommandAllocator) { g_frameContext[i].CommandAllocator->Release(); g_frameContext[i].CommandAllocator = nullptr; }
//     if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = nullptr; }
//     if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = nullptr; }
//     if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = nullptr; }
//     if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = nullptr; }
//     if (g_fence) { g_fence->Release(); g_fence = nullptr; }
//     if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = nullptr; }
//     if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }

// #ifdef DX12_ENABLE_DEBUG_LAYER
//     IDXGIDebug1* pDebug = nullptr;
//     if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
//     {
//         pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
//         pDebug->Release();
//     }
// #endif
// }

// void CreateRenderTarget()
// {
//     for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
//     {
//         ID3D12Resource* pBackBuffer = nullptr;
//         g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
//         g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g_mainRenderTargetDescriptor[i]);
//         g_mainRenderTargetResource[i] = pBackBuffer;
//     }
// }

// void CleanupRenderTarget()
// {
//     WaitForLastSubmittedFrame();

//     for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
//         if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = nullptr; }
// }

// void WaitForLastSubmittedFrame()
// {
//     FrameContext* frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

//     UINT64 fenceValue = frameCtx->FenceValue;
//     if (fenceValue == 0)
//         return; // No fence was signaled

//     frameCtx->FenceValue = 0;
//     if (g_fence->GetCompletedValue() >= fenceValue)
//         return;

//     g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
//     WaitForSingleObject(g_fenceEvent, INFINITE);
// }

// FrameContext* WaitForNextFrameResources()
// {
//     UINT nextFrameIndex = g_frameIndex + 1;
//     g_frameIndex = nextFrameIndex;

//     HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, nullptr };
//     DWORD numWaitableObjects = 1;

//     FrameContext* frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
//     UINT64 fenceValue = frameCtx->FenceValue;
//     if (fenceValue != 0) // means no fence was signaled
//     {
//         frameCtx->FenceValue = 0;
//         g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
//         waitableObjects[1] = g_fenceEvent;
//         numWaitableObjects = 2;
//     }

//     WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

//     return frameCtx;
// }

// // Forward declare message handler from imgui_impl_win32.cpp
// extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// // Win32 message handler
// // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
// LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
// {
//     if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
//         return true;

//     switch (msg)
//     {
//     case WM_SIZE:
//         if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
//         {
//             WaitForLastSubmittedFrame();
//             CleanupRenderTarget();
//             HRESULT result = g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
//             //assert(SUCCEEDED(result) && "Failed to resize swapchain.");
//             CreateRenderTarget();
//         }
//         return 0;
//     case WM_SYSCOMMAND:
//         if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
//             return 0;
//         break;
//     case WM_DESTROY:
//         ::PostQuitMessage(0);
//         return 0;
//     }
//     return ::DefWindowProcW(hWnd, msg, wParam, lParam);
// }
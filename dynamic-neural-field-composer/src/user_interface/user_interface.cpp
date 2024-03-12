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
            closeUI = false;
            window = nullptr;
        }

        void UserInterface::init()
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
            window = glfwCreateWindow(windowWidth, windowHeight, "Dynamic Neural Field Composer C++", nullptr, nullptr);
            if (window == nullptr)
                return;
            glfwMakeContextCurrent(window);
            glfwSwapInterval(1); // Enable vsync

            // Setup Dear ImGui and ImPlot context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImPlot::CreateContext();
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
                 style.WindowBorderSize = 1.0f;
                style.WindowRounding = 0.25f;
                style.WindowTitleAlign = ImVec2(0.0f, 0.5f);

                // Set window colors

                style.Colors[ImGuiCol_Text] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Text color
                style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Disabled text color
                style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // Text background color when selected
                style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // Background color
                style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // Child window background color
                style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // Popup background color
                style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // Border shadow color (used by some widgets)
                style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f); // Frame background color
                style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // Frame background color when hovered
                style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f); // Frame background color when active
                style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // Tab color when unfocused
                style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // Docking empty box color
                style.Colors[ImGuiCol_PlotLines] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Plot lines color
                style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // Plot lines color when hovered
                style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Plot histogram color
                style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // Plot histogram color when hovered
                style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // Text background color when selected

                style.Colors[ImGuiCol_Separator] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Separator color
                style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f); // Separator color when hovered
                style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Separator color when active
                style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Resize grip color
                style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f); // Resize grip color when hovered
                style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Resize grip color when active
                style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f); // Tab color when unfocused and active
                style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Docking preview overlay color

                style.Colors[ImGuiCol_Button] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f); // Button color
                style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Button color when hovered
                style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f); // Button color when active
                style.Colors[ImGuiCol_TitleBg] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f); // Title bar background color
                style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Title bar background color when active
                style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f); // Title bar background color when collapsed
                style.Colors[ImGuiCol_Header] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f); // Header color
                style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Header color when hovered
                style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f); // Header color when active
                style.Colors[ImGuiCol_Tab] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f); // Tab color
                style.Colors[ImGuiCol_TabHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Tab color when hovered
                style.Colors[ImGuiCol_TabActive] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f); // Tab color when active
            }

            // Setup Platform/Renderer backends
            ImGui_ImplGlfw_InitForOpenGL(window, true);
        #ifdef __EMSCRIPTEN__
            ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
        #endif
            ImGui_ImplOpenGL3_Init(glsl_version);

            // Load Fonts
            std::string convertedPath = std::string(PROJECT_DIR) + "/resources/fonts/Lexend-Light.ttf";
            const ImFont* font = io.Fonts->AddFontFromFileTTF(convertedPath.c_str(), fontSize);

            // Our state
            bool show_demo_window = true;
            bool show_another_window = false;
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        }

        void UserInterface::step()
        {
            closeUI = glfwWindowShouldClose(window);

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

            this->render();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            ImGuiIO& io = ImGui::GetIO(); (void)io;
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

        void UserInterface::close() const
        {
            // Cleanup
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            glfwDestroyWindow(window);
            glfwTerminate();
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
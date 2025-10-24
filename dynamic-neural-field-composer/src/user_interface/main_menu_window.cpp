#include "user_interface/main_menu_window.h"

#include "application/application.h"

extern ID3D12Device*         g_pd3dDevice;       // or Application::GetD3DDevice()
extern ID3D12DescriptorHeap* g_pd3dSrvDescHeap ;  // the SAME heap ImGui DX12 backend uses
extern UINT                  g_pd3dSrvDescSize;  // device->GetDescriptorHandleIncrementSize(...)
extern ImFont* g_pIconFont;

// === Texture lifetime for the logo
static ID3D12Resource*           s_logoTex   = nullptr;
static D3D12_CPU_DESCRIPTOR_HANDLE s_logoCpu = {};
static D3D12_GPU_DESCRIPTOR_HANDLE s_logoGpu = {};
static ImTextureID               s_logoId ;
static int                       s_logoW     = 2596;
static int                       s_logoH     = 2365;

namespace dnf_composer::user_interface {

    MainMenuWindow::MainMenuWindow(const std::shared_ptr<Simulation>& simulation)
        : simulation(simulation)
    {
        sim_window_ = std::make_unique<dnf_composer::user_interface::SimulationWindow>(simulation);
    }

    void MainMenuWindow::render()
    {

        static float g_uiScalePct = 80.0f;
        auto windowHandle = ImGui::GetMainViewport()->PlatformHandle;
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = ImGui_ImplWin32_GetDpiScaleForHwnd(windowHandle) * (g_uiScalePct / 100.0f);

        // [only do this once]
        if (!s_logoTex)
        {
            static_assert(sizeof(ImTextureID) >= sizeof(D3D12_CPU_DESCRIPTOR_HANDLE), "D3D12_CPU_DESCRIPTOR_HANDLE is too large to fit in an ImTextureID");
            const UINT handle_increment = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            //const UINT handle_increment = 1;
            constexpr int descriptor_index = 2;
            s_logoCpu = g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
            s_logoCpu.ptr += (handle_increment * descriptor_index);
            s_logoGpu = g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
            s_logoGpu.ptr += (handle_increment * descriptor_index);

            // path to your PNG (adjust as needed)
            const std::string kLogoPath = std::string(PROJECT_DIR) + "/resources/images/logo-simple.png";

            const bool ret = LoadTextureFromFile(kLogoPath.c_str(),
                g_pd3dDevice,
                s_logoCpu,
                &s_logoTex,
                &s_logoW,
                &s_logoH);
            if (!ret)
                std::cerr << "Failed to load image." << std::endl;
            if (LoadTextureFromFile(kLogoPath.c_str(), g_pd3dDevice, s_logoCpu, &s_logoTex, &s_logoW, &s_logoH))
             {
             //ImGui DX12 expects ImTextureID = GPU SRV handle
             s_logoId = (ImTextureID)s_logoGpu.ptr;
            }
        }

        //ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;      // no docking
        io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;    // (optional) no multi-viewport OS windows

        ImGuiViewport* vp = ImGui::GetMainViewport();

        // Pin this window to the main application OS window
        ImGui::SetNextWindowViewport(vp->ID);
        ImGui::SetNextWindowPos(vp->WorkPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(vp->WorkSize, ImGuiCond_Always);

        constexpr ImGuiWindowFlags root_flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings |     // never restore old screen coords
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoBackground;

        ui = g_uiScalePct / 100.0f;

        if (ImGui::Begin("dnf-composer", nullptr, root_flags))
        {
            ImVec2 s = ImGui::GetWindowSize();
            ImVec2 p = ImGui::GetWindowPos();
            ImDrawList* draw = ImGui::GetWindowDrawList();

            // your palette
            const ImU32 kWhite       = ImColor(255,255,255);
            const ImU32 kPanelLight  = ImColor(245,247,250);
            const ImU32 kBorderLight = ImColor(225,229,235);
            constexpr ImVec4 kWhiteTransp = ImVec4(1.0f, 1.0f, 1.0f, 0.75f);
            //const ImVec4 kSidebarColor = ImVec4(0.89f, 0.96f, 0.95f, 0.90f);

            // --- layout constants
            const float margin = 20.0f * ui;
            const float radius = 10.0f * ui;

            static constexpr float kSidebarTargetBase = 350.0f;
            static constexpr float kSidebarMinBase    = 180.0f;
            static constexpr float kMainMinBase       = 480.0f;

            const float kSidebarTarget = kSidebarTargetBase * ui;
            const float kSidebarMin    = kSidebarMinBase * ui;
            const float kMainMin       = kMainMinBase * ui;

            float sidebar_w = kSidebarTarget;
            {
                const float totalMargins = margin * 2.0f;
                const float minNeeded    = kSidebarMin + kMainMin + totalMargins;

                if (s.x < minNeeded)
                    sidebar_w = ImMax(kSidebarMin, s.x - kMainMin - totalMargins);
                else if (s.x < (kSidebarTarget + kMainMin + totalMargins))
                    sidebar_w = ImClamp(s.x - kMainMin - totalMargins, kSidebarMin, kSidebarTarget);
            }

            // --- compute rects exactly like before
            ImVec2 rSidebarMin = ImVec2(p.x + margin,           p.y + margin);
            ImVec2 rSidebarMax = ImVec2(p.x + sidebar_w - margin, p.y + s.y - margin);

            rMainMin    = ImVec2(p.x + sidebar_w,        p.y + margin);
            rMainMax    = ImVec2(p.x + s.x - margin,     p.y + s.y - margin);

            // --- draw backgrounds + borders (same look as before)
            draw->AddRectFilled(rMainMin,    rMainMax,    ImColor(kWhiteTransp),      radius);
            draw->AddRect      (rMainMin,    rMainMax,    kBorderLight, radius);
            draw->AddRectFilled(rSidebarMin, rSidebarMax, kPanelLight, radius); // just white sidebar

            // // Custom sidebar
            // // colors
            // const ImU32 c_top    = ImColor(0.90f, 0.97f, 0.96f, 0.92f);
            // const ImU32 c_bottom = ImColor(0.86f, 0.94f, 0.95f, 0.92f);
            //
            // // Solid rounded base (use the bottom/mid tone)
            // draw->AddRectFilled(rSidebarMin, rSidebarMax, c_bottom, radius);
            //
            // // Rounded top cap to cover the curved area
            // const float cap_h = ceilf(radius * 1.15f); // a bit taller than r avoids AA seams
            // ImRect topCap(rSidebarMin, ImVec2(rSidebarMax.x, rSidebarMin.y + cap_h));
            // draw->AddRectFilled(topCap.Min, topCap.Max, c_top, radius, ImDrawFlags_RoundCornersTop);
            //
            // // Vertical gradient starting just below the cap (no rounding needed)
            // ImRect grad(ImVec2(rSidebarMin.x, topCap.Max.y - 0.5f),   // -0.5 to hide AA seam
            //             ImVec2(rSidebarMax.x, rSidebarMax.y - radius*0.5f));
            // draw->AddRectFilledMultiColor(
            //     grad.Min, grad.Max,
            //     c_top, c_top,            // top-left, top-right
            //     c_bottom, c_bottom       // bottom-left, bottom-right
            // );
            //
            // // Border (rounded)
            // draw->AddRect(rSidebarMin, rSidebarMax, kBorderLight, radius, ImDrawFlags_RoundCornersAll);
            //
            // // (optional) very subtle outer shadow with rounded corners
            // ImU32 shadow = ImColor(0, 0, 0, 25);
            // draw->AddRect(rSidebarMin, rSidebarMax, shadow, radius, ImDrawFlags_RoundCornersAll, 1.0f);


            // --- children (transparent) aligned to those rects
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0)); // transparent
            {
                // Sidebar child
                ImGui::SetCursorScreenPos(rSidebarMin);
                ImVec2 sidebarSize(rSidebarMax.x - rSidebarMin.x,
                                   rSidebarMax.y - rSidebarMin.y);
                ImGui::BeginChild("##sidebar", sidebarSize,
                                  false, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
                {
                    tabs();

                    if (s_logoId && s_logoW > 0 && s_logoH > 0)
                    {
                        const float pad = 12.0f * ui;

                        // Sidebar child rect in screen coords
                        ImVec2 childMin = ImGui::GetWindowPos();
                        ImVec2 childSz  = ImGui::GetWindowSize();

                        float targetW = childSz.x - pad * 2.0f;
                        float aspect  = (float)s_logoH / (float)s_logoW;
                        float targetH = targetW * aspect;

                        float x = childMin.x + pad;
                        float y = childMin.y + childSz.y - pad - targetH;

                        ImGui::SetCursorScreenPos(ImVec2(x, y));
                        ImGui::Image(s_logoId, ImVec2(targetW, targetH));
                    }
                }
                ImGui::EndChild();

                // Main child
                ImGui::SetCursorScreenPos(rMainMin);
                ImVec2 mainSize(rMainMax.x - rMainMin.x,
                                rMainMax.y - rMainMin.y);
                ImGui::BeginChild("##main", mainSize,
                                  false, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
                {

                    switch (tab)
                    {
                    case 0: build_tab(); break;
                    default: build_tab();
                        // case 1: adjust_tab(); break;
                        // case 2: evaluate_tab(); break;
                        // case 3: learning_tab(); break;
                        // case 4: monitoring_tab(); break;
                        // case 5: overview_tab(); break;
                        // case 6: node_graph_tab(); break;
                        // case 7: plotting_tab(); break;
                        // case 8: ui_tab(); break;
                        // case 9: simulation_tab(); break;
                    }
                    // main-area content goes here
                    // e.g. draw your node graph, etc.

                    // {
                    //     float pct = g_uiScalePct;
                    //     ImGui::SetNextItemWidth(160 * ui);
                    //     if (ImGui::SliderFloat("UI Scale", &pct, 50.0f, 200.0f, "%.0f%%"))
                    //         g_uiScalePct = pct;
                    // }
                }
                ImGui::EndChild();
            }
            ImGui::PopStyleColor(); // ChildBg
        }



        ImGui::End();
    }

    void MainMenuWindow::tabs()
    {
        ImGuiStyle& st = ImGui::GetStyle();
        const ImVec4 colText = st.Colors[ImGuiCol_Text];
        const ImVec4 colMuted = st.Colors[ImGuiCol_TextDisabled];

        auto TabText = [&](bool is_active) -> const ImVec4& {
            return is_active ? colText : colMuted;
        };

        auto Section = [&](const char* title)
        {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);// bold font
            ImGui::PushStyleColor(ImGuiCol_Text, imgui_kit::colours::Gray);
            ImGui::SetCursorPosX(20);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
            ImGui::TextUnformatted(title);
            ImGui::PopStyleColor();
            ImGui::PopFont();
            ImGui::Spacing();
        };

        ImGui::SetCursorPos(ImVec2(10, 20));
        ImGui::BeginGroup();
        {
             Section("Building");
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 0));
             if (widgets::tab(ICON_FA_HAMMER, "Build", tab == 0)) tab = 0; // ICON_FA_SCREWDRIVER_WRENCH, ICON_FA_TOOLBOX
             ImGui::PopStyleColor();

             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 1));
             if (widgets::tab(ICON_FA_SLIDERS, "Adjust", tab == 1)) tab = 1; // ICON_FA_WRENCH, ICON_FA_GEAR
             ImGui::PopStyleColor();

             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 3));
             if (widgets::tab(ICON_FA_CLIPBOARD_CHECK, "Evaluate", tab == 3)) tab = 3; //ICON_FA_CIRCLE_CHECK, ICON_FA_RANKING_STAR
             ImGui::PopStyleColor();

             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 4));
             if (widgets::tab(ICON_FA_BRAIN, "Learning", tab == 4)) tab = 4; //ICON_FA_GRADUATION_CAP, ICON_FA_BOOK_OPEN
             ImGui::PopStyleColor();

             ImGui::Spacing();

             Section("Monitoring");
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 5));
             if (widgets::tab(ICON_FA_EYE, "Overview", tab == 5)) tab = 5; // ICON_FA_GAUGE, ICON_FA_EYE
             ImGui::PopStyleColor();
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 6));
             if (widgets::tab(ICON_FA_SHARE_NODES, "Node graph", tab == 6)) tab = 6; // ICON_FA_SITEMAP, ICON_FA_DIAGRAM_PROJECT
             ImGui::PopStyleColor();
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 7));
             if (widgets::tab(ICON_FA_CHART_LINE, "Plotting", tab == 7)) tab = 7;
             ImGui::PopStyleColor();

             ImGui::Spacing();

             Section("Configuration"); // ICON_FA_GEAR, ICON_FA_GEARS, ICON_FA_WRENCH
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 8));
             if (widgets::tab(ICON_FA_WINDOW_MAXIMIZE, "User interface", tab == 8)) tab = 8; // ICON_FA_DISPLAY, ICON_FA_COMPUTER_MOUSE
             ImGui::PopStyleColor();
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 9));
             if (widgets::tab(ICON_FA_WRENCH, "Simulation", tab == 9)) tab = 9; // ICON_FA_SITEMAP, ICON_FA_DIAGRAM_PROJECT
             ImGui::PopStyleColor();
        }
        ImGui::EndGroup();



        // case 3: visuals_tab(); break;switch (tab) {
        // case 0: rage_tab();  break;
        // case 1: legit_tab(); break;
        // case 2: anti_tab();  break;
        // case 4: player_tab();  break;
        // case 5: misc_tab();    break;
        // case 6: configs_tab(); break;
        // case 7: lua_tab();     break;
        // }
    }

    void MainMenuWindow::build_tab()
    {
        ImVec2 main_min = rMainMin;
        ImVec2 main_max = rMainMax;

        ImVec2 content_pos  = ImVec2(main_min.x + 16.0f*ui, main_min.y + 16.0f*ui);
        float  content_w    = (main_max.x - main_min.x) - 32.0f*ui;

        // Card sizing (tweak freely)
        float card_w = ImClamp(content_w, 420.0f*ui, 740.0f*ui);
        float card_h = 420.0f*ui;

        // Layout: first card at top-left
        ImVec2 card_pos = content_pos;
        ImVec2 card_sz  = ImVec2(card_w, card_h);

        if (BeginCard("##card_simulation", card_pos, card_sz, "Simulation Control", ui))
        {
            // body-only rendering of your simulation panel
            sim_window_->renderPanelContents();
        }
        EndCard();
    }

}

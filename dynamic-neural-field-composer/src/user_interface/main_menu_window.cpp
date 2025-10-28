#include "user_interface/main_menu_window.h"

#include "application/application.h"

extern ID3D12Device*                g_pd3dDevice;
extern ID3D12DescriptorHeap*        g_pd3dSrvDescHeap ;
extern UINT                         g_pd3dSrvDescSize;



namespace dnf_composer::user_interface
{

    void SidebarLogo::load()
    {
        if (!logoTex)
        {
            static_assert(sizeof(ImTextureID) >= sizeof(D3D12_CPU_DESCRIPTOR_HANDLE), "D3D12_CPU_DESCRIPTOR_HANDLE is too large to fit in an ImTextureID");
            const UINT handle_increment = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            constexpr int descriptor_index = 2;
            logoCpu = g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
            logoCpu.ptr += (handle_increment * descriptor_index);
            logoGpu = g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
            logoGpu.ptr += (handle_increment * descriptor_index);

            const std::string logoPath = std::string(PROJECT_DIR) + "/resources/images/logo-simple.png";

            if (LoadTextureFromFile(logoPath.c_str(), g_pd3dDevice, logoCpu, &logoTex, &logoW, &logoH))
                logoId = logoGpu.ptr; //ImGui DX12 expects ImTextureID = GPU SRV handle
            else
                tools::logger::log(tools::logger::ERROR, "Failed to load sidebar logo texture");
        }
        loadingAttempted = true;
    }

    MainMenuWindow::MainMenuWindow(const std::shared_ptr<Simulation>& simulation)
        : simulation(simulation), logo(), layoutProperties()
    {
        simulationWindow = std::make_unique<SimulationWindow>(simulation);
        elementWindow = std::make_unique<ElementWindow>(simulation);
        nodeGraphWindow = std::make_unique<NodeGraphWindow>(simulation);
        logWindow = std::make_unique<imgui_kit::LogWindow>();
        mainAreaSize = {ImVec2(0, 0), ImVec2(0, 0)};
        selectedSidebarTab = 0;
    }

    void MainMenuWindow::render()
    {
        // [only do this once] load sidebar logo
        if (!logo.loadingAttempted)
            logo.load();

        const auto windowHandle = ImGui::GetMainViewport()->PlatformHandle;
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = ImGui_ImplWin32_GetDpiScaleForHwnd(windowHandle) * (layoutProperties.guiScale );
        io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;      // no docking
        io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;    // (optional) no multi-viewport OS windows

        const ImGuiViewport* vp = ImGui::GetMainViewport();

        // Pin this window to the main the application OS window
        ImGui::SetNextWindowViewport(vp->ID);
        ImGui::SetNextWindowPos(vp->WorkPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(vp->WorkSize, ImGuiCond_Always);

        constexpr ImGuiWindowFlags root_flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoBackground;

        layoutProperties.scale();

        if (ImGui::Begin("dnf-composer", nullptr, root_flags))
        {
            drawZones();
        }

        ImGui::End();
    }

    void MainMenuWindow::drawZones()
    {
        const ImVec2 s = ImGui::GetWindowSize();
        const ImVec2 p = ImGui::GetWindowPos();
        ImDrawList* draw = ImGui::GetWindowDrawList();

        // colour palette
        const ImU32 kWhite       = ImGui::GetColorU32(ImGuiCol_FrameBg);    // card / frame background
        const ImU32 kPanelLight  = ImGui::GetColorU32(ImGuiCol_MenuBarBg);  // light chrome (sidebar/menubar look)
        const ImU32 kBorderLight = ImGui::GetColorU32(ImGuiCol_Border);
        // Soft wash over the wallpaper = WindowBg with reduced alpha
        ImVec4 wash = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        wash.w *= 0.80f; // 75–80% translucency
        const ImU32 kWhiteTransparent = ImGui::GetColorU32(wash);

        float sidebar_w = layoutProperties.scaledSidebarWidthTarget;
        {
            const float totalMargins = layoutProperties.scaledMargin * 2.0f;
            const float minNeeded    = layoutProperties.scaledSidebarWidthMin + layoutProperties.scaledMainAreaMin + totalMargins;

            if (s.x < minNeeded)
                sidebar_w = ImMax(layoutProperties.scaledSidebarWidthMin, s.x - layoutProperties.scaledMainAreaMin - totalMargins);
            else if (s.x < (layoutProperties.scaledSidebarWidthTarget + layoutProperties.scaledMainAreaMin + totalMargins))
                sidebar_w = ImClamp(s.x - layoutProperties.scaledMainAreaMin - totalMargins, layoutProperties.scaledSidebarWidthMin, layoutProperties.scaledSidebarWidthTarget);
        }

        // compute rects exactly like before
        const auto rSidebarMin = ImVec2(p.x + layoutProperties.scaledMargin, p.y + layoutProperties.scaledMargin);
        const auto rSidebarMax = ImVec2(p.x + sidebar_w - layoutProperties.scaledMargin, p.y + s.y - layoutProperties.scaledMargin);

        mainAreaSize = {ImVec2(p.x + sidebar_w, p.y + layoutProperties.scaledMargin), ImVec2(p.x + s.x - layoutProperties.scaledMargin, p.y + s.y - layoutProperties.scaledMargin)};

        // draw backgrounds and borders (same look as before)
        draw->AddRectFilled(std::get<0>(mainAreaSize), std::get<1>(mainAreaSize), ImColor(kWhiteTransparent), layoutProperties.scaledRadius);
        draw->AddRect      (std::get<0>(mainAreaSize), std::get<1>(mainAreaSize), kBorderLight, layoutProperties.scaledRadius);
        draw->AddRectFilled(rSidebarMin, rSidebarMax, kPanelLight, layoutProperties.scaledRadius); // just white sidebar

        // // Custom sidebar
        // const ImU32 c_top    = ImColor(0.90f, 0.97f, 0.96f, 0.92f);
        // const ImU32 c_bottom = ImColor(0.86f, 0.94f, 0.95f, 0.92f);
        //
        // // Solid rounded base (use the bottom/mid-tone)
        // draw->AddRectFilled(rSidebarMin, rSidebarMax, c_bottom, layoutProperties.scaledRadius);
        //
        // // Rounded top cap to cover the curved area
        // const float cap_h = ceilf(layoutProperties.scaledRadius * 1.15f); // a bit taller than r avoids AA seams
        // const ImRect topCap(rSidebarMin, ImVec2(rSidebarMax.x, rSidebarMin.y + cap_h));
        // draw->AddRectFilled(topCap.Min, topCap.Max, c_top, layoutProperties.scaledRadius, ImDrawFlags_RoundCornersTop);
        //
        // // Vertical gradient starting just below the cap (no rounding needed)
        // ImRect grad(ImVec2(rSidebarMin.x, topCap.Max.y - 0.5f),   // -0.5 to hide AA seam
        //             ImVec2(rSidebarMax.x, rSidebarMax.y - layoutProperties.scaledRadius*0.5f));
        // draw->AddRectFilledMultiColor(
        //     grad.Min, grad.Max,
        //     c_top, c_top,            // top-left, top-right
        //     c_bottom, c_bottom       // bottom-left, bottom-right
        // );
        //
        // // Border (rounded)
        // draw->AddRect(rSidebarMin, rSidebarMax, kBorderLight, layoutProperties.scaledRadius, ImDrawFlags_RoundCornersAll);
        //
        // // (optional) very subtle outer shadow with rounded corners
        // ImVec4 dim = ImGui::GetStyleColorVec4(ImGuiCol_ModalWindowDimBg);
        // dim.w = 25.0f/255.0f; // keep it very subtle
        // const ImU32 shadow = ImGui::GetColorU32(dim);
        // draw->AddRect(rSidebarMin, rSidebarMax, shadow, layoutProperties.scaledRadius, ImDrawFlags_RoundCornersAll, 1.0f);

        // children (transparent) aligned to those rects
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0)); // transparent
        {
            // Sidebar child
            ImGui::SetCursorScreenPos(rSidebarMin);
            const ImVec2 sidebarSize(rSidebarMax.x - rSidebarMin.x,
                               rSidebarMax.y - rSidebarMin.y);
            ImGui::BeginChild("##sidebar", sidebarSize,
                              false, ImGuiWindowFlags_NoScrollbar);
            {
                renderSidebarTabs();
                renderSidebarLogo();
            }
            ImGui::EndChild();

            // Main child
            ImGui::SetCursorScreenPos(std::get<0>(mainAreaSize));
            const ImVec2 mainSize(std::get<1>(mainAreaSize).x - std::get<0>(mainAreaSize).x,
                            std::get<1>(mainAreaSize).y - std::get<0>(mainAreaSize).y);
            ImGui::BeginChild("##main", mainSize,
                              false, ImGuiWindowFlags_NoScrollbar);
            {

                switch (selectedSidebarTab)
                {
                case 0: renderBuild(); break;
                default: break;
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
            }
            ImGui::EndChild();
        }
        ImGui::PopStyleColor();
    }

    void MainMenuWindow::renderSidebarTabs()
    {
        const ImGuiStyle& st = ImGui::GetStyle();
        const ImVec4 colText = st.Colors[ImGuiCol_Text];
        const ImVec4 colMuted = st.Colors[ImGuiCol_TextDisabled];

        auto TabText = [&](const bool is_active) -> const ImVec4& {
            return is_active ? colText : colMuted;
        };

        auto Section = [&](const char* title)
        {
            ImGui::PushFont(g_BlackFont);
            ImGui::SetCursorPosX(20);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
            ImGui::Text(title);
            ImGui::PopFont();
            ImGui::Spacing();
        };

        ImGui::SetCursorPos(ImVec2(10, 20));
        ImGui::BeginGroup();
        {
             Section("Building");
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(selectedSidebarTab == 0));
             if (widgets::renderSidebarTab(ICON_FA_HAMMER, "Build", selectedSidebarTab == 0)) selectedSidebarTab = 0; // ICON_FA_SCREWDRIVER_WRENCH, ICON_FA_TOOLBOX
             ImGui::PopStyleColor();

             ImGui::PushStyleColor(ImGuiCol_Text, TabText(selectedSidebarTab == 1));
             if (widgets::renderSidebarTab(ICON_FA_SLIDERS, "Adjust", selectedSidebarTab == 1)) selectedSidebarTab = 1; // ICON_FA_WRENCH, ICON_FA_GEAR
             ImGui::PopStyleColor();

             ImGui::PushStyleColor(ImGuiCol_Text, TabText(selectedSidebarTab == 3));
             if (widgets::renderSidebarTab(ICON_FA_CLIPBOARD_CHECK, "Evaluate", selectedSidebarTab == 3)) selectedSidebarTab = 3; //ICON_FA_CIRCLE_CHECK, ICON_FA_RANKING_STAR
             ImGui::PopStyleColor();

             ImGui::PushStyleColor(ImGuiCol_Text, TabText(selectedSidebarTab == 4));
             if (widgets::renderSidebarTab(ICON_FA_BRAIN, "Learning", selectedSidebarTab == 4)) selectedSidebarTab = 4; //ICON_FA_GRADUATION_CAP, ICON_FA_BOOK_OPEN
             ImGui::PopStyleColor();

             ImGui::Spacing();

             Section("Monitoring");
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(selectedSidebarTab == 5));
             if (widgets::renderSidebarTab(ICON_FA_EYE, "Overview", selectedSidebarTab == 5)) selectedSidebarTab = 5; // ICON_FA_GAUGE, ICON_FA_EYE
             ImGui::PopStyleColor();
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(selectedSidebarTab == 6));
             if (widgets::renderSidebarTab(ICON_FA_SHARE_NODES, "Node graph", selectedSidebarTab == 6)) selectedSidebarTab = 6; // ICON_FA_SITEMAP, ICON_FA_DIAGRAM_PROJECT
             ImGui::PopStyleColor();
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(selectedSidebarTab == 7));
             if (widgets::renderSidebarTab(ICON_FA_CHART_LINE, "Plotting", selectedSidebarTab == 7)) selectedSidebarTab = 7;
             ImGui::PopStyleColor();

             ImGui::Spacing();

             Section("Configuration"); // ICON_FA_GEAR, ICON_FA_GEARS, ICON_FA_WRENCH
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(selectedSidebarTab == 8));
             if (widgets::renderSidebarTab(ICON_FA_WINDOW_MAXIMIZE, "User interface", selectedSidebarTab == 8)) selectedSidebarTab = 8; // ICON_FA_DISPLAY, ICON_FA_COMPUTER_MOUSE
             ImGui::PopStyleColor();
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(selectedSidebarTab == 9));
             if (widgets::renderSidebarTab(ICON_FA_WRENCH, "Simulation", selectedSidebarTab == 9)) selectedSidebarTab = 9; // ICON_FA_SITEMAP, ICON_FA_DIAGRAM_PROJECT
             ImGui::PopStyleColor();
        }
        ImGui::EndGroup();
    }

    void MainMenuWindow::renderSidebarLogo() const
    {
        if (logo.logoId && logo.logoW > 0 && logo.logoH > 0)
        {
            const float pad = 12.0f * layoutProperties.guiScale;

            // Sidebar child rect in screen coords
            const ImVec2 childMin = ImGui::GetWindowPos();
            const ImVec2 childSz  = ImGui::GetWindowSize();

            const float targetW = childSz.x - pad * 2.0f;
            const float aspect  = static_cast<float>(logo.logoH) / static_cast<float>(logo.logoW);
            const float targetH = targetW * aspect;

            const float x = childMin.x + pad;
            const float y = childMin.y + childSz.y - pad - targetH;

            ImGui::SetCursorScreenPos(ImVec2(x, y));
            ImGui::Image(logo.logoId, ImVec2(targetW, targetH));
        }
    }

    void MainMenuWindow::renderBuild() const
    {
        const ImVec2 mainMin = std::get<0>(mainAreaSize);
        const ImVec2 mainMax = std::get<1>(mainAreaSize);

        const float m   = 16.0f * layoutProperties.guiScale; // outer/inner margin
        const ImVec2 pos(mainMin.x + m, mainMin.y + m);
        const float  W  = (mainMax.x - mainMin.x) - m * 2.0f;
        const float  H  = (mainMax.y - mainMin.y) - m * 2.0f;

        // --- Column widths (A+B fixed-ish, C gets the rest) ---
        const float colGap = m;
        const float colA   = 620.0f * layoutProperties.guiScale;  // Simulation control
        const float colB   = 450.0f * layoutProperties.guiScale;  // Element control
        const float colC   = ImMax(420.0f * layoutProperties.guiScale,
                                   W - colA - colB - 2.0f*colGap); // right column gets the remainder

        // Safety: if the window is too small, clamp so we at least draw something
        if (colC < 220.0f * layoutProperties.guiScale) return;

        // --- Row heights inside Column C (right) ---
        const float rowGap = m;
        const float plotsH = H * 0.55f;                      // Plots ~55%
        const float nodeH  = H * 0.27f;                      // Node graph ~27%
        const float logsH  = H - plotsH - nodeH - 2.0f*rowGap; // remaining ~18%

        ImVec2 p = pos;

        // =========================
        // Column A: Simulation control (tall)
        // =========================
        {
            const widgets::Card cardA("##card_sim", p, ImVec2(colA, H), "Simulation control");
            if (cardA.beginCard(layoutProperties.guiScale))
            {
                simulationWindow->renderSimulationParametersCard();
                ImGui::Spacing();
                ImGui::Spacing();
                simulationWindow->renderSimulationControlsCard();
                ImGui::Spacing();
                ImGui::Spacing();
                simulationWindow->renderRunForIterationsCard();
                ImGui::Spacing();
                ImGui::Spacing();
                simulationWindow->renderAddElementCard();
                ImGui::Spacing();
                ImGui::Spacing();
                simulationWindow->renderRemoveElementCard();
                ImGui::Spacing();
                ImGui::Spacing();
                simulationWindow->renderSetInteractionCard();
                ImGui::Spacing();
                ImGui::Spacing();
                simulationWindow->renderLogElementParametersCard();
                ImGui::Spacing();
                ImGui::Spacing();
                simulationWindow->renderExportElementComponentCard();
            }
            widgets::Card::endCard();
        }

        // =========================
        // Column B: Element control (tall)
        // =========================
        p.x += colA + colGap;
        {
            const widgets::Card cardB("##card_element", p, ImVec2(colB, H), "Element control");
            if (cardB.beginCard(layoutProperties.guiScale))
            {
                elementWindow->renderElementControlCard();
            }
            widgets::Card::endCard();
        }

        // =========================
        // Column C: Right stack
        // =========================
        p.x += colB + colGap;

        // Plots (top)
        {
            const widgets::Card cardC1("##card_plots", p, ImVec2(colC, plotsH), "Plots");
            if (cardC1.beginCard(layoutProperties.guiScale))
            {
                ImGui::TextDisabled("Plots content");
            }
            widgets::Card::endCard();
            p.y += plotsH + rowGap;
        }

        // Node graph (middle)
        {
            const widgets::Card cardC2("##card_node_graph", p, ImVec2(colC, nodeH), "Node graph");
            if (cardC2.beginCard(layoutProperties.guiScale))
            {
                nodeGraphWindow->renderGraph();
            }
            widgets::Card::endCard();
            p.y += nodeH + rowGap;
        }

        // Log window (bottom)
        {
            const widgets::Card cardC3("##card_logs", p, ImVec2(colC, logsH), "Log window");
            if (cardC3.beginCard(layoutProperties.guiScale))
            {
                ImGui::TextDisabled("Logs content");
            }
            widgets::Card::endCard();
        }
    }

}

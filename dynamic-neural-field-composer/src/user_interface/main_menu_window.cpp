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

    MainMenuWindow::MainMenuWindow()
    {

    }

    void MainMenuWindow::render()
    {

        static float g_uiScalePct = 100.0f;
        auto windowHandle = ImGui::GetMainViewport()->PlatformHandle;
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = ImGui_ImplWin32_GetDpiScaleForHwnd(windowHandle) * (g_uiScalePct / 100.0f);

        // [only do this once]
        if (!s_logoTex)
        {
            static_assert(sizeof(ImTextureID) >= sizeof(D3D12_CPU_DESCRIPTOR_HANDLE), "D3D12_CPU_DESCRIPTOR_HANDLE is too large to fit in an ImTextureID");
            const UINT handle_increment = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            //const UINT handle_increment = 1;
            constexpr int descriptor_index = 1;
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




        const ImGuiWindowFlags root_flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings |     // never restore old screen coords
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoBackground;

        const float ui = g_uiScalePct / 100.0f;

        if (ImGui::Begin("dnf-composer", nullptr, root_flags))
        {
            ImVec2 s = ImGui::GetWindowSize();
            ImVec2 p = ImGui::GetWindowPos();
            ImDrawList* draw = ImGui::GetWindowDrawList();

            // your palette
            const ImU32 kWhite       = ImColor(255,255,255);
            const ImU32 kPanelLight  = ImColor(245,247,250);
            const ImU32 kBorderLight = ImColor(225,229,235);

            // --- layout constants
            const float margin = 20.0f * ui;
            const float radius = 6.0f * ui;

            static const float kSidebarTargetBase = 300.0f;
            static const float kSidebarMinBase    = 180.0f;
            static const float kMainMinBase       = 480.0f;

            float kSidebarTarget = kSidebarTargetBase * ui;
            float kSidebarMin    = kSidebarMinBase * ui;
            float kMainMin       = kMainMinBase * ui;

            float sidebar_w = kSidebarTarget;
            {
                const float totalMargins = margin * 2.0f;
                const float minNeeded    = kSidebarMin + kMainMin + totalMargins;

                if (s.x < minNeeded)
                    sidebar_w = ImMax(kSidebarMin, s.x - kMainMin - totalMargins);
                else if (s.x < (kSidebarTarget + kMainMin + totalMargins))
                    sidebar_w = ImClamp(s.x - kMainMin - totalMargins, kSidebarMin, kSidebarTarget);
                // } else {
                //     // wide enough: keep fixed width
                //     sidebar_w = kSidebarTarget;
                // }
            }

            // --- compute rects exactly like before
            ImVec2 rSidebarMin = ImVec2(p.x + margin,           p.y + margin);
            ImVec2 rSidebarMax = ImVec2(p.x + sidebar_w - margin, p.y + s.y - margin);

            ImVec2 rMainMin    = ImVec2(p.x + sidebar_w,        p.y + margin);
            ImVec2 rMainMax    = ImVec2(p.x + s.x - margin,     p.y + s.y - margin);

            // --- draw backgrounds + borders (same look as before)
            draw->AddRectFilled(rMainMin,    rMainMax,    kWhite,      radius);
            draw->AddRect      (rMainMin,    rMainMax,    kBorderLight, radius);

            draw->AddRectFilled(rSidebarMin, rSidebarMax, kPanelLight, radius);
            draw->AddRect      (rSidebarMin, rSidebarMax, kBorderLight, radius);

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
                    // put sidebar widgets here
                    // e.g. ImGui::TextUnformatted("Simulation control");
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

                        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
                        //ImGui::PushFont(g_pIconFont);
                        ImGui::Text("Hello world!");
                        ImGui::PopFont();

                        ImGui::Spacing();

                        float pct = g_uiScalePct;
                        ImGui::SetNextItemWidth(160 * ui);
                        if (ImGui::SliderFloat("UI Scale", &pct, 50.0f, 200.0f, "%.0f%%"))
                            g_uiScalePct = pct;

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
                    // main-area content goes here
                    // e.g. draw your node graph, etc.
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
        const ImVec4 colAccent = st.Colors[ImGuiCol_HeaderActive]; // your theme’s accent

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
            // --- AimBot
            Section("AimBot");
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 0));
            if (widgets::tab("A", "RageBot", tab == 0)) tab = 0;
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 1));
            if (widgets::tab("H", "LegitBot", tab == 1)) tab = 1;
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 2));
            if (widgets::tab("B", "Anti Aim", tab == 2)) tab = 2;
            ImGui::PopStyleColor();

            // --- Visuals
             ImGui::Spacing();
             Section("Visuals");
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 3));
            if (widgets::tab("J", "Visuals", tab == 3)) tab = 3;
            ImGui::PopStyleColor();
            ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 4));
            if (widgets::tab("D", "Players", tab == 4)) tab = 4;
            ImGui::PopStyleColor();

            // --- Misc
             ImGui::Spacing();
            Section("Miscellaneous");
             ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 5));
            if (widgets::tab("I", "Misc", tab == 5)) tab = 5;
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 6));
            if (widgets::tab("L", "Configs", tab == 6)) tab = 6;
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, TabText(tab == 7));
            if (widgets::tab("O", "Lua's", tab == 7)) tab = 7;
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
}

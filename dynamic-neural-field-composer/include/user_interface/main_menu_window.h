#pragma once

#include <imgui-platform-kit/user_interface_window.h>
#include "widgets.h"
#include <imgui-platform-kit/user_interface.h>

#include "element_window.h"
#include "node_graph_window.h"
#include "application/application.h"
#include "user_interface/simulation_window.h"

namespace dnf_composer::user_interface
{
    struct SidebarLogo
    {
        ID3D12Resource*              logoTex    = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE  logoCpu    = {};
        D3D12_GPU_DESCRIPTOR_HANDLE  logoGpu    = {};
        ImTextureID                  logoId     {};
        int                          logoW      = 2596;
        int                          logoH      = 2365;
        bool                         loadingAttempted     = false;
        void load();
    };

    struct LayoutProperties
    {
        float guiScale = 70.0f/100.0f;

        const float margin = 20.0f;
        const float radius = 10.0f;
        const float sidebarWidthTarget = 350.0f;
        const float sidebarWidthMin = 250.0f;
        const float mainAreaMin = 480.0f;

        float scaledSidebarWidthTarget = sidebarWidthTarget*guiScale;
        float scaledSidebarWidthMin = sidebarWidthMin*guiScale;
        float scaledMainAreaMin = mainAreaMin*guiScale;

        float scaledMargin = margin*guiScale;
        float scaledRadius = radius*guiScale;

        void scale()
        {
            scaledSidebarWidthTarget = sidebarWidthTarget*guiScale;
            scaledSidebarWidthMin = sidebarWidthMin*guiScale;
            scaledMainAreaMin = mainAreaMin*guiScale;
            scaledMargin = margin*guiScale;
            scaledRadius = radius*guiScale;
        }

        // show the ui scale in percentage to user
        // float pct = layoutProperties.guiScale*100.0f;
        // ImGui::SetNextItemWidth(160 * layoutProperties.guiScale);
        // if (ImGui::SliderFloat("UI Scale", &pct, 50.0f, 200.0f, "%.0f%%"))
        //     layoutProperties.guiScale = pct/100.0f;
    };

    class MainMenuWindow final : public imgui_kit::UserInterfaceWindow
    {
        std::shared_ptr<Simulation> simulation;
        std::shared_ptr<Visualization> visualization;
        std::unique_ptr<SimulationWindow> simulationWindow;
        std::unique_ptr<ElementWindow> elementWindow;
        std::unique_ptr<NodeGraphWindow> nodeGraphWindow;
        std::unique_ptr<imgui_kit::LogWindow> logWindow;
        std::tuple<ImVec2, ImVec2> mainAreaSize; // min, max
        int selectedSidebarTab;
        SidebarLogo logo;
        LayoutProperties layoutProperties;
    public:
        explicit MainMenuWindow(const std::shared_ptr<Simulation>& simulation);
        void render() override;
    private:
        void drawZones();
        void renderSidebarTabs();
        void renderSidebarLogo() const;
        void renderBuild() const;
        void renderNodeGraph() const;
    };

}
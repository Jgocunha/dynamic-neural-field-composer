#pragma once

#include <imgui-platform-kit/user_interface_window.h>
#include "widgets.h"


namespace dnf_composer::user_interface {

    class MainMenuWindow : public imgui_kit::UserInterfaceWindow {
    private:
        int currentTab = 1;      // Tracks the selected tab
        int currentSubTab = 0;   // Tracks the selected sub-tab
        float sliderValue = 0.0f; // Example slider control value
        bool checkBoxState = false; // Example checkbox state
        ImVec2 pos;
        ImDrawList* draw;
         int tabs = 1;
         int subtabs = 0;
         int sett = 0;
        float dpi_scale = 1.f;
         int sliderint = 0;

         bool checkbox = false;


        void renderTabs();       // Renders the main tabs
        void renderSubTabs();    // Renders sub-tabs based on the current tab
        void renderContent();    // Renders content related to tabs/sub-tabs

    public:
        MainMenuWindow();        // Constructor to initialize the window properties
        void render() override;  // Override the render method
    private:
        void decorations();
        void tabs_();
        void subtabs_();
        void function();
    };

}
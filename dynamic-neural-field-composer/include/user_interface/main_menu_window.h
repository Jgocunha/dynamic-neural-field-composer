#pragma once

#include <imgui-platform-kit/user_interface_window.h>
#include "widgets.h"
#include <imgui-platform-kit/user_interface.h>

namespace dnf_composer::user_interface
{
    class MainMenuWindow : public imgui_kit::UserInterfaceWindow
    {
        int tab;
    public:
        MainMenuWindow();        // Constructor to initialize the window properties
        void render() override;  // Override the render method
    private:
        void tabs();
    };

}
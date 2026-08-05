#include <array>
#include <format>
#include <utility>

#include "application/application.h"

#include <imgui-platform-kit/colour_palette.h>

#include "application/style.h"
#include "user_interface/fonts/IconsFontAwesome6.h"
#include "user_interface/fonts/fa.h"
#include "user_interface/log_window.h"
#include "tools/utils.h"

namespace dnf_composer
{
	float   Application::uiScalePct = 100.0F;

	Application::Application(const std::shared_ptr<Simulation>& simulation,
		const std::shared_ptr<Visualization>& visualization)
		: simulation(simulation ? simulation : std::make_shared<Simulation>("default",
			1.0, 0.0, 0.0)),
		visualization(visualization ? visualization : std::make_shared<Visualization>(this->simulation))
	{
		if (this->visualization->getSimulation() != this->simulation) {
			throw Exception(ErrorCode::APP_VIS_SIM_MISMATCH);
		}
		setGUIParameters();
	}

	void Application::init() const
	{
		// Wire the logger to the GUI (issue #123): tools/logger has no dependency on
		// the UI, so the UI hands it a callback here instead. Registering before any
		// other init step means every log() call from this point on (including the
		// "initialized successfully" message below) also reaches the log window,
		// matching the previous direct-call behavior exactly.
		tools::logger::Logger::setUiSink([](const tools::logger::LogLevel level, const std::string& message)
		{
			user_interface::LogWindow::addLog(user_interface::getLogLevelColorCodeGui(level), "%s", message.c_str());
		});

		simulation->init();
		gui->initialize();

		static std::string iniPath;
		iniPath = tools::utils::getResourceRoot() + "/resources/imgui.ini";
		ImGui::GetIO().IniFilename = iniPath.c_str();

		registerSettingsHandler();
		enableKeyboardShortcuts();
		appendFonts();
		defineImGuiStyle();
		log(tools::logger::LogLevel::INFO, "Application initialized successfully.");
	}

	void Application::registerSettingsHandler()
	{
		ImGuiSettingsHandler handler;
		handler.TypeName  = "AppSettings";
		handler.TypeHash  = ImHashStr("AppSettings");

		// called when the [AppSettings] section is encountered in imgui.ini
		handler.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler*, const char*) -> void*
		{
			return reinterpret_cast<void*>(1); // non-null signals "we own this entry"
		};

		// called for each key=value line inside the section
		handler.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler*, void*, const char* line)
		{
			float scale = 100.0F;
			if (sscanf(line, "UiScale=%f", &scale) == 1) {
				setUiScalePct(scale);
			}
		};

		// called when ImGui writes imgui.ini to disk
		handler.WriteAllFn = [](ImGuiContext*, ImGuiSettingsHandler* h, ImGuiTextBuffer* buf)
		{
			buf->appendf("[%s][Data]\n", h->TypeName);
			buf->appendf("UiScale=%.0F\n", Application::getUiScalePct());
			buf->appendf("\n");
		};

		ImGui::AddSettingsHandler(&handler);
	}

	void Application::step() const
	{
		simulation->step();
		if (guiActive)
		{
			ImGui::GetIO().FontGlobalScale = uiScalePct / 100.0F;
			gui->render();
		}
	}

	void Application::close() const
	{
		simulation->close();
		if (guiActive) {
			gui->shutdown();
		}
		log(tools::logger::LogLevel::INFO, "Application closed successfully.");
	}

	void Application::toggleGUI()
	{
		guiActive = !guiActive;
		log(tools::logger::LogLevel::INFO, std::format("GUI is {}", guiActive ? "enabled." : "disabled."));
	}

	void Application::requestQuit()
	{
		quitRequested = true;
	}

	bool Application::isQuitRequested()
	{
		return quitRequested;
	}

	void Application::resetQuitRequestForTesting()
	{
		quitRequested = false;
	}

	bool Application::hasGUIBeenClosed() const
	{
		// Checked before touching the GUI so a quit request is honoured even when
		// the GUI is disabled or not yet initialized.
		if (quitRequested) {
			return true;
		}
		if (guiActive) {
			return gui->isShutdownRequested();
		}
		return false;
	}

	bool Application::isGUIActive() const
	{
		return guiActive;
	}

	void Application::setGUIParameters()
	{
		using namespace imgui_kit;
		const std::string root = tools::utils::getResourceRoot();
		const WindowParameters winParams{ "Dynamic Neural Field Composer" };
		const FontParameters fontParams({
			{root + "/resources/fonts/Cera Pro Light.ttf",        18},
			{root + "/resources/fonts/Cera Pro Light.ttf",        12},
			{root + "/resources/fonts/Cera Pro Light.ttf",        24},
			{root + "/resources/fonts/Cera Pro Medium.ttf",       12},
			{root + "/resources/fonts/Cera Pro Medium.ttf",       18},
			{root + "/resources/fonts/Cera Pro Medium.ttf",       24},
			{root + "/resources/fonts/Cera Pro Bold.ttf",         12},
			{root + "/resources/fonts/Cera Pro Bold.ttf",         18},
			{root + "/resources/fonts/Cera Pro Bold.ttf",         24},
			{root + "/resources/fonts/Cera Pro Black.ttf",        20},
			{root + "/resources/fonts/Cera Pro Black.ttf",        24},
			{root + "/resources/fonts/Cera Pro Black.ttf",        30},
			{root + "/resources/fonts/JetBrainsMono-Regular.ttf", 16},
			{root + "/resources/fonts/JetBrainsMono-Regular.ttf", 20},
			{root + "/resources/fonts/JetBrainsMono-Regular.ttf", 26},
		});
		const StyleParameters styleParams{Theme::Light, imgui_kit::colours::White};
#ifdef _WIN32
		const IconParameters iconParams{ root + "/resources/icons/icon.ico" };
#else
		const IconParameters iconParams{ root + "/resources/icons/icon.png" };
#endif
		const BackgroundImageParameters bgParams{ root
			+ "/resources/images/background.jpg", ImageFitType::ZOOM_TO_FIT };
		const UserInterfaceParameters guiParameters{ winParams, fontParams, styleParams, iconParams, bgParams };

		gui = std::make_shared<UserInterface>(guiParameters);
		imgui_kit::setGlobalWindowFlags(ImGuiWindowFlags_NoCollapse);
		log(tools::logger::LogLevel::INFO, "GUI parameters set successfully.");
	}

	void Application::enableKeyboardShortcuts()
	{
		// ImGui::GetIO() returns ImGuiIO&; binding it by value here would copy the
		// struct and write the flag to a discarded temporary instead of the real
		// IO object (#114) — must bind by reference.
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	}

	void Application::appendFonts()
	{
		// Same reference-vs-copy pitfall as enableKeyboardShortcuts() above: binding
		// by value would silently drop the io.FontDefault assignment below (#114).
		ImGuiIO& io = ImGui::GetIO();

		ImFontConfig cfg{};
		cfg.OversampleH = 2;          // 2 is often crisper than 3 at small sizes
		cfg.OversampleV = 2;
		cfg.PixelSnapH  = true;       // snap glyphs to the pixel boundary for crispness
		cfg.FontLoaderFlags = ImGuiFreeTypeBuilderFlags_LightHinting   // gentle hinting (good for UI)
							 | ImGuiFreeTypeBuilderFlags_Bitmap;        // keep bitmap rendering (fast)
		// If you want *maximum* crispness at small sizes (old-school look), try:
		 cfg.FontLoaderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;

		if (std::cmp_less(io.Fonts->Fonts.size() , g_FontCount))
		{
			tools::logger::log(tools::logger::FATAL, "Not enough fonts in the font stack."
											" Please add more fonts to the font stack.");
			throw Exception(ErrorCode::APP_INIT);
		}

		// Assign text font pointers (indices match the order in setGUIParameters)
		g_LightMediumFont  = io.Fonts->Fonts[0];
		g_LightSmallFont   = io.Fonts->Fonts[1];
		g_LightLargeFont   = io.Fonts->Fonts[2];
		g_MediumSmallFont  = io.Fonts->Fonts[3];
		g_MediumMediumFont = io.Fonts->Fonts[4];
		g_MediumLargeFont  = io.Fonts->Fonts[5];
		g_BoldSmallFont    = io.Fonts->Fonts[6];
		g_BoldMediumFont   = io.Fonts->Fonts[7];
		g_BoldLargeFont    = io.Fonts->Fonts[8];
		g_BlackSmallFont   = io.Fonts->Fonts[9];
		g_BlackMediumFont  = io.Fonts->Fonts[10];
		g_BlackLargeFont   = io.Fonts->Fonts[11];
		g_MonoSmallFont    = io.Fonts->Fonts[12];
		g_MonoMediumFont   = io.Fonts->Fonts[13];
		g_MonoLargeFont    = io.Fonts->Fonts[14];

		// Set default font
		io.FontDefault = g_MediumMediumFont;

		// Add icon fonts (3 sizes, each standalone)
		static constexpr std::array<ImWchar, 3> icons_ranges = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = false;

		io.Fonts->AddFontFromMemoryCompressedTTF(FA_compressed_data,
			FA_compressed_size, 12.0F, &icons_config, icons_ranges.data());
		g_SmallIconsFont  = io.Fonts->Fonts[io.Fonts->Fonts.Size - 1];

		io.Fonts->AddFontFromMemoryCompressedTTF(FA_compressed_data,
			FA_compressed_size, 18.0F, &icons_config, icons_ranges.data());
		g_MediumIconsFont = io.Fonts->Fonts[io.Fonts->Fonts.Size - 1];

		io.Fonts->AddFontFromMemoryCompressedTTF(FA_compressed_data,
			FA_compressed_size, 26.0F, &icons_config, icons_ranges.data());
		g_LargeIconsFont  = io.Fonts->Fonts[io.Fonts->Fonts.Size - 1];

		io.Fonts->Build();
	}

	void Application::defineImGuiStyle()
	{
		const std::string root = tools::utils::getResourceRoot();
		applyImGuiStyle(root + "/resources/style_light_green_accent.json");
	}

}
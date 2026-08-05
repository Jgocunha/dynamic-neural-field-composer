#pragma once

// Headless ImGui/ImPlot fixture.
//
// The GUI layer is compiled into the same library as the core, so every
// user_interface/ translation unit lands in the coverage denominator. Nothing
// exercised it, because rendering appeared to need a window + GL context.
//
// It does not. ImGui is a pure software layer: NewFrame() builds draw lists in
// memory and Render() finalises them. Only the *backend* touches the GPU, and a
// backend is optional. Set DisplaySize, provide a font, and render() calls run
// headlessly — no GLFW, no OpenGL, no window.
//
// ImGui 1.92 made fonts dynamic: the atlas is rasterised on demand and the
// backend is expected to service texture requests. Declaring
// ImGuiBackendFlags_RendererHasTextures tells ImGui a backend will handle them;
// we then simply ignore the requests, which is correct for a null backend that
// never samples a texture.

#include <imgui.h>
#include <implot.h>

#include "application/application.h"

namespace dnf_composer::test
{
	/// RAII headless ImGui + ImPlot context. One per test (contexts are global).
	class HeadlessImGui
	{
	public:
		HeadlessImGui()
		{
			imguiContext = ImGui::CreateContext();
			ImGui::SetCurrentContext(imguiContext);

			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2(1280.0F, 800.0F);
			io.DeltaTime = 1.0F / 60.0F;
			io.IniFilename = nullptr;   // never touch imgui.ini from a test
			io.LogFilename = nullptr;

			// Claim texture handling so 1.92 does not expect a legacy atlas.
			io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
			io.BackendPlatformName = "headless-test";
			io.BackendRendererName = "headless-test";

			// Every window pushes these globals; a null ImFont* dereferences.
			ImFont* font = io.Fonts->AddFontDefault();
			setAllFonts(font);

			implotContext = ImPlot::CreateContext();
			ImPlot::SetCurrentContext(implotContext);
		}

		HeadlessImGui(const HeadlessImGui&) = delete;
		HeadlessImGui& operator=(const HeadlessImGui&) = delete;
		HeadlessImGui(HeadlessImGui&&) = delete;
		HeadlessImGui& operator=(HeadlessImGui&&) = delete;

		~HeadlessImGui()
		{
			if (implotContext != nullptr)
				ImPlot::DestroyContext(implotContext);
			if (imguiContext != nullptr)
				ImGui::DestroyContext(imguiContext);
			setAllFonts(nullptr);
		}

		/// Run one full frame around `body` — this is what drives a render() call.
		template <typename Callable>
		void frame(Callable&& body)
		{
			ImGui::NewFrame();
			body();
			ImGui::Render();          // finalises draw data; no GPU involved
		}

		/// Render several frames: catches state that only appears after frame 1
		/// (docking, ImGuiCond_FirstUseEver, cached per-window state).
		template <typename Callable>
		void frames(int count, Callable&& body)
		{
			for (int i = 0; i < count; ++i)
				frame(body);
		}

	private:
		/// The UI reads 18 inline font globals; point them all at one real font.
		static void setAllFonts(ImFont* f)
		{
			g_LightSmallFont = f;   g_LightMediumFont = f;   g_LightLargeFont = f;
			g_MediumSmallFont = f;  g_MediumMediumFont = f;  g_MediumLargeFont = f;
			g_BoldSmallFont = f;    g_BoldMediumFont = f;    g_BoldLargeFont = f;
			g_BlackSmallFont = f;   g_BlackMediumFont = f;   g_BlackLargeFont = f;
			g_MonoSmallFont = f;    g_MonoMediumFont = f;    g_MonoLargeFont = f;
			g_SmallIconsFont = f;   g_MediumIconsFont = f;   g_LargeIconsFont = f;
		}

		ImGuiContext* imguiContext = nullptr;
		ImPlotContext* implotContext = nullptr;
	};
}

#include "user_interface/field_metrics_window.h"

#include <array>
#include <algorithm>
#include <cstdio>

#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
#include "user_interface/colour_registry.h"
#include "user_interface/fonts/IconsFontAwesome6.h"

extern ImFont* g_MonoMediumFont;

namespace dnf_composer::user_interface
{
	static constexpr float  kCardRound  = 8.0F;
	static constexpr float  kCardBordSz = 1.5F;
	static constexpr float  kBarH       = 6.0F;
	static constexpr float  kDotR       = 5.0F;

	FieldMetricsWindow::FieldMetricsWindow(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{}

	void FieldMetricsWindow::render()
	{
		const ImGuiViewport* vp = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + 20.0F, vp->WorkPos.y + 80.0F), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(440.0F, 520.0F), ImGuiCond_FirstUseEver);
		const bool open = ImGui::Begin("Field Metrics##field_metrics", nullptr,
			imgui_kit::getGlobalWindowFlags() | ImGuiWindowFlags_NoTitleBar);
		if (open)
		{
			const float startY = ImGui::GetCursorPosY();
			const float yOff = (g_BlackLargeFont->LegacySize - g_MediumIconsFont->LegacySize) * 0.5F;
			ImGui::SetCursorPosY(startY + yOff);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_HEART_PULSE);
			ImGui::PopFont();
			ImGui::PopStyleColor();
			ImGui::SameLine(0, 8.0F);
			ImGui::SetCursorPosY(startY);
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::TextUnformatted("Neural Field Monitoring");
			ImGui::PopFont();
			ImGui::Separator();
			renderContents(simulation);
		}
		ImGui::End();
	}

	// NOLINTNEXTLINE(readability-function-cognitive-complexity) - linear ImGui immediate-mode layout; splitting would fragment widget state across functions
	void FieldMetricsWindow::renderContents(const std::shared_ptr<Simulation>& simulation)
	{
		bool anyNF = false;
		for (const auto& e : simulation->getElements())
		{
			const bool is1D = (e->getLabel() == element::ElementLabel::NEURAL_FIELD);
			const bool is2D = (e->getLabel() == element::ElementLabel::NEURAL_FIELD_2D);
			if (!is1D && !is2D) { continue;
}
			anyNF = true;

			const auto* nf1d = is1D ? dynamic_cast<const element::NeuralField*>(e.get())   : nullptr;
			const auto* nf2d = is2D ? dynamic_cast<const element::NeuralField2D*>(e.get()) : nullptr;
			if ((nf1d == nullptr) && (nf2d == nullptr)) { continue;
}

			const std::string& name   = e->getUniqueName();
			const bool  stable = is1D ? nf1d->isStable()             : nf2d->isStable();
			const float lo     = is1D ? static_cast<float>(nf1d->getLowestActivation())
			                          : static_cast<float>(nf2d->getLowestActivation());
			const float hi     = is1D ? static_cast<float>(nf1d->getHighestActivation())
			                          : static_cast<float>(nf2d->getHighestActivation());
			const auto  bumps1d = is1D ? nf1d->getBumps() : std::vector<element::NeuralFieldBump>{};
			const auto  bumps2d = is2D ? nf2d->getBumps() : std::vector<element::NeuralField2DBump>{};
			const int   bn      = is1D ? static_cast<int>(bumps1d.size())
			                           : static_cast<int>(bumps2d.size());

			const float padV      = ImGui::GetStyle().WindowPadding.y;
			const float spacing   = ImGui::GetStyle().ItemSpacing.y;
			const float lineH     = ImGui::GetTextLineHeightWithSpacing();
			const float monoLineH = (g_MonoMediumFont != nullptr ? g_MonoMediumFont->LegacySize
			                                          : ImGui::GetTextLineHeight()) + spacing;
			const float rowH = std::max(lineH, monoLineH);
			const float sepH = 1.0F + spacing;
			float cardH = padV * 2.0F
				+ lineH            // header row
				+ spacing          // Spacing() after header
				+ kBarH + spacing  // bar InvisibleButton
				+ spacing          // Spacing() after bar
				+ rowH             // Range row
				+ rowH;            // Bumps row

			if (bn > 0) {
				cardH += float(bn) * (sepH + lineH + 2.0F * rowH);
}

			const float avail = ImGui::GetContentRegionAvail().x;

			ImGui::PushStyleColor(ImGuiCol_ChildBg, colour::kCardBackground);
			ImGui::PushStyleColor(ImGuiCol_Border,  colour::kCardBorder);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   kCardRound);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, kCardBordSz);

			const std::string cid = "##mc_" + name;
			if (ImGui::BeginChild(cid.c_str(), { avail, cardH }, 1,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				const float innerW = ImGui::GetContentRegionAvail().x;
				const float maxX   = ImGui::GetContentRegionMax().x;
				ImDrawList* dl     = ImGui::GetWindowDrawList();

				// ── Header: dot + name + stable badge ─────────────────────────
				{
					const ImVec2 pos = ImGui::GetCursorScreenPos();
					const float  lh  = ImGui::GetTextLineHeight();
					dl->AddCircleFilled({ pos.x + kDotR, pos.y + lh * 0.5F }, kDotR,
						colour::kMetricCardDot);

					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + kDotR * 2.0F + 6.0F);
					ImGui::PushFont(g_BoldLargeFont);
					ImGui::TextUnformatted(name.c_str());
					ImGui::PopFont();

					const char*  badge    = stable ? "Stable" : "Unstable";
					const ImVec4 badgeCol = stable
						? colour::kMetricStableText
						: colour::kMetricUnstableText;
					const float badgeW = ImGui::CalcTextSize(badge).x;
					ImGui::SameLine();
					ImGui::SetCursorPosX(maxX - badgeW);
					ImGui::TextColored(badgeCol, "%s", badge);
				}

				ImGui::Spacing();

				// ── Range bar ─────────────────────────────────────────────────
				{
					const ImVec2 barMin = ImGui::GetCursorScreenPos();
					const ImVec2 barMax = { barMin.x + innerW, barMin.y + kBarH };
					const float  span   = hi - lo;

					dl->AddRectFilled(barMin, barMax, colour::kMetricBarTrack, 3.0F);

					if (span > 0.0001F)
					{
						const ImU32 fillCol = stable
							? colour::kMetricBarStableFill
							: colour::kMetricBarUnstableFill;

						if (hi > 0.0F)
						{
							const float zeroX = (lo < 0.0F)
								? barMin.x + innerW * (-lo / span)
								: barMin.x;
							dl->AddRectFilled({ zeroX, barMin.y }, barMax, fillCol, 3.0F);

							if (lo < 0.0F) {
								dl->AddLine({ zeroX, barMin.y - 1.0F }, { zeroX, barMax.y + 1.0F },
									colour::kMetricBarZeroTick, 1.5F);
}
						}
						else
						{
							dl->AddRectFilled(barMin, barMax, colour::kMetricBarNegativeTrack, 3.0F);
						}
					}

					ImGui::InvisibleButton("##bar_hover", { innerW, kBarH });
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip(
							"Activation range: %.2f to %.2f\n"
							"Colored fill = above-zero (excitatory) activation.\n"
							"White tick = zero crossing.",
							lo, hi);
}
				}

				ImGui::Spacing();

				// ── Range row ─────────────────────────────────────────────────
				{
					std::array<char, 64> buf{};
					snprintf(buf.data(), buf.size(), "%.2f ... %.2f", lo, hi);
					ImGui::PushFont(g_MonoMediumFont);
					const float valW = ImGui::CalcTextSize(buf.data()).x;
					ImGui::PopFont();
					ImGui::TextDisabled("Range");
					ImGui::SameLine();
					ImGui::SetCursorPosX(maxX - valW);
					ImGui::PushFont(g_MonoMediumFont);
					ImGui::TextUnformatted(buf.data());
					ImGui::PopFont();
				}

				// ── Bumps row ─────────────────────────────────────────────────
				{
					std::array<char, 16> buf{};
					snprintf(buf.data(), buf.size(), "%d", bn);
					ImGui::PushFont(g_MonoMediumFont);
					const float valW = ImGui::CalcTextSize(buf.data()).x;
					ImGui::PopFont();
					ImGui::TextDisabled("Bumps");
					ImGui::SameLine();
					ImGui::SetCursorPosX(maxX - valW);
					ImGui::PushFont(g_MonoMediumFont);
					ImGui::TextUnformatted(buf.data());
					ImGui::PopFont();
				}

				// ── Per-bump detail ───────────────────────────────────────────
				if (bn > 0)
				{
					ImGui::Separator();
					for (int i = 0; i < bn; ++i)
					{
						ImGui::PushFont(g_BoldMediumFont);
						ImGui::Text("Bump %d", i);
						ImGui::PopFont();
						if (is1D)
						{
							const auto& b = bumps1d[i];
							ImGui::TextDisabled("Pos");   ImGui::SameLine(0, 4);
							ImGui::PushFont(g_MonoMediumFont); ImGui::Text("%.2f", b.centroid);  ImGui::PopFont();
							ImGui::SameLine(0, 12);
							ImGui::TextDisabled("Amp");   ImGui::SameLine(0, 4);
							ImGui::PushFont(g_MonoMediumFont); ImGui::Text("%.2f", b.amplitude); ImGui::PopFont();

							ImGui::TextDisabled("Width"); ImGui::SameLine(0, 4);
							ImGui::PushFont(g_MonoMediumFont); ImGui::Text("%.2f", b.width);     ImGui::PopFont();
							ImGui::SameLine(0, 12);
							ImGui::TextDisabled("Vel");   ImGui::SameLine(0, 4);
							ImGui::PushFont(g_MonoMediumFont); ImGui::Text("%.2f", b.velocity);  ImGui::PopFont();
						}
						else
						{
							const auto& b = bumps2d[i];
							ImGui::TextDisabled("Pos");  ImGui::SameLine(0, 4);
							ImGui::PushFont(g_MonoMediumFont); ImGui::Text("(%.2f, %.2f)", b.centroid_x, b.centroid_y); ImGui::PopFont();
							ImGui::SameLine(0, 12);
							ImGui::TextDisabled("Amp");  ImGui::SameLine(0, 4);
							ImGui::PushFont(g_MonoMediumFont); ImGui::Text("%.2f", b.amplitude); ImGui::PopFont();

							ImGui::TextDisabled("Area"); ImGui::SameLine(0, 4);
							ImGui::PushFont(g_MonoMediumFont); ImGui::Text("%.2f", b.area);      ImGui::PopFont();
							ImGui::SameLine(0, 12);
							ImGui::TextDisabled("Vel");  ImGui::SameLine(0, 4);
							ImGui::PushFont(g_MonoMediumFont); ImGui::Text("(%.2f, %.2f)", b.velocity_x, b.velocity_y); ImGui::PopFont();
						}
						if (i < bn - 1) {
							ImGui::Separator();
}
					}
				}
			}
			ImGui::EndChild();
			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor(2);

			ImGui::Spacing();
		}

		if (!anyNF) {
			ImGui::TextDisabled("No neural fields in simulation.");
}
	}
}

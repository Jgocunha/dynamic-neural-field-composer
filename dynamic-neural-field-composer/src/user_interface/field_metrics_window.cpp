#include "user_interface/field_metrics_window.h"

#include <algorithm>
#include <cstdio>

#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
#include "user_interface/fonts/IconsFontAwesome6.h"

extern ImFont* g_MonoMediumFont;

namespace dnf_composer::user_interface
{
	static constexpr ImVec4 kCardBg     = { 0.96f, 0.97f, 0.98f, 1.0f };
	static constexpr ImVec4 kCardBorder = { 0.82f, 0.85f, 0.89f, 1.0f };
	static constexpr float  kCardRound  = 8.0f;
	static constexpr float  kCardBordSz = 1.5f;
	static constexpr float  kBarH       = 6.0f;
	static constexpr float  kDotR       = 5.0f;

	FieldMetricsWindow::FieldMetricsWindow(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{}

	void FieldMetricsWindow::render()
	{
		const bool open = ImGui::Begin("##field_metrics", nullptr,
			imgui_kit::getGlobalWindowFlags() | ImGuiWindowFlags_NoTitleBar);
		if (open)
		{
			const float startY = ImGui::GetCursorPosY();
			const float yOff = (g_BlackLargeFont->LegacySize - g_MediumIconsFont->LegacySize) * 0.5f;
			ImGui::SetCursorPosY(startY + yOff);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
			ImGui::PushFont(g_MediumIconsFont);
			ImGui::TextUnformatted(ICON_FA_HEART_PULSE);
			ImGui::PopFont();
			ImGui::PopStyleColor();
			ImGui::SameLine(0, 8.0f);
			ImGui::SetCursorPosY(startY);
			ImGui::PushFont(g_BlackLargeFont);
			ImGui::TextUnformatted("Neural Field Monitoring");
			ImGui::PopFont();
			ImGui::Separator();
			renderContents(simulation);
		}
		ImGui::End();
	}

	void FieldMetricsWindow::renderContents(const std::shared_ptr<Simulation>& simulation)
	{
		bool anyNF = false;
		for (const auto& e : simulation->getElements())
		{
			const bool is1D = (e->getLabel() == element::ElementLabel::NEURAL_FIELD);
			const bool is2D = (e->getLabel() == element::ElementLabel::NEURAL_FIELD_2D);
			if (!is1D && !is2D) continue;
			anyNF = true;

			const auto* nf1d = is1D ? dynamic_cast<const element::NeuralField*>(e.get())   : nullptr;
			const auto* nf2d = is2D ? dynamic_cast<const element::NeuralField2D*>(e.get()) : nullptr;
			if (!nf1d && !nf2d) continue;

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
			const float monoLineH = (g_MonoMediumFont ? g_MonoMediumFont->LegacySize
			                                          : ImGui::GetTextLineHeight()) + spacing;
			const float rowH = std::max(lineH, monoLineH);
			const float sepH = 1.0f + spacing;
			float cardH = padV * 2.0f
				+ lineH            // header row
				+ spacing          // Spacing() after header
				+ kBarH + spacing  // bar InvisibleButton
				+ spacing          // Spacing() after bar
				+ rowH             // Range row
				+ rowH;            // Bumps row

			if (bn > 0)
				cardH += float(bn) * (sepH + lineH + 2.0f * rowH);

			const float avail = ImGui::GetContentRegionAvail().x;

			ImGui::PushStyleColor(ImGuiCol_ChildBg, kCardBg);
			ImGui::PushStyleColor(ImGuiCol_Border,  kCardBorder);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   kCardRound);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, kCardBordSz);

			const std::string cid = "##mc_" + name;
			if (ImGui::BeginChild(cid.c_str(), { avail, cardH }, true,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				const float innerW = ImGui::GetContentRegionAvail().x;
				const float maxX   = ImGui::GetContentRegionMax().x;
				ImDrawList* dl     = ImGui::GetWindowDrawList();

				// ── Header: dot + name + stable badge ─────────────────────────
				{
					const ImVec2 pos = ImGui::GetCursorScreenPos();
					const float  lh  = ImGui::GetTextLineHeight();
					dl->AddCircleFilled({ pos.x + kDotR, pos.y + lh * 0.5f }, kDotR,
						IM_COL32(74, 144, 217, 255));

					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + kDotR * 2.0f + 6.0f);
					ImGui::PushFont(g_BoldLargeFont);
					ImGui::TextUnformatted(name.c_str());
					ImGui::PopFont();

					const char*  badge    = stable ? "Stable" : "Unstable";
					const ImVec4 badgeCol = stable
						? ImVec4(0.22f, 0.75f, 0.35f, 1.0f)
						: ImVec4(0.90f, 0.55f, 0.10f, 1.0f);
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

					dl->AddRectFilled(barMin, barMax, IM_COL32(60, 60, 60, 80), 3.0f);

					if (span > 0.0001f)
					{
						const ImU32 fillCol = stable
							? IM_COL32(56,  200, 90,  180)
							: IM_COL32(230, 140, 25,  180);

						if (hi > 0.0f)
						{
							const float zeroX = (lo < 0.0f)
								? barMin.x + innerW * (-lo / span)
								: barMin.x;
							dl->AddRectFilled({ zeroX, barMin.y }, barMax, fillCol, 3.0f);

							if (lo < 0.0f)
								dl->AddLine({ zeroX, barMin.y - 1.0f }, { zeroX, barMax.y + 1.0f },
									IM_COL32(255, 255, 255, 150), 1.5f);
						}
						else
						{
							dl->AddRectFilled(barMin, barMax, IM_COL32(180, 60, 60, 80), 3.0f);
						}
					}

					ImGui::InvisibleButton("##bar_hover", { innerW, kBarH });
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip(
							"Activation range: %.2f to %.2f\n"
							"Colored fill = above-zero (excitatory) activation.\n"
							"White tick = zero crossing.",
							lo, hi);
				}

				ImGui::Spacing();

				// ── Range row ─────────────────────────────────────────────────
				{
					char buf[64];
					snprintf(buf, sizeof(buf), "%.2f ... %.2f", lo, hi);
					ImGui::PushFont(g_MonoMediumFont);
					const float valW = ImGui::CalcTextSize(buf).x;
					ImGui::PopFont();
					ImGui::TextDisabled("Range");
					ImGui::SameLine();
					ImGui::SetCursorPosX(maxX - valW);
					ImGui::PushFont(g_MonoMediumFont);
					ImGui::TextUnformatted(buf);
					ImGui::PopFont();
				}

				// ── Bumps row ─────────────────────────────────────────────────
				{
					char buf[16];
					snprintf(buf, sizeof(buf), "%d", bn);
					ImGui::PushFont(g_MonoMediumFont);
					const float valW = ImGui::CalcTextSize(buf).x;
					ImGui::PopFont();
					ImGui::TextDisabled("Bumps");
					ImGui::SameLine();
					ImGui::SetCursorPosX(maxX - valW);
					ImGui::PushFont(g_MonoMediumFont);
					ImGui::TextUnformatted(buf);
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
						if (i < bn - 1)
							ImGui::Separator();
					}
				}
			}
			ImGui::EndChild();
			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor(2);

			ImGui::Spacing();
		}

		if (!anyNF)
			ImGui::TextDisabled("No neural fields in simulation.");
	}
}

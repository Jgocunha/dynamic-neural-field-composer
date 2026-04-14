#include "user_interface/field_metrics_window.h"

#include <algorithm>

namespace dnf_composer::user_interface
{
	// ── Card visual style ────────────────────────────────────────────────────
	static constexpr ImVec4  kCardBg      = { 0.96f, 0.97f, 0.98f, 1.0f };  // off-white with cool tint
	static constexpr ImVec4  kCardBorder  = { 0.82f, 0.85f, 0.89f, 1.0f };  // light blue-grey border
	static constexpr float   kCardGap     = 10.0f;
	static constexpr float   kCardMinW    = 240.0f;  // minimum card width
	static constexpr float   kCardRound   = 10.0f;
	static constexpr float   kCardBordSz  = 1.5f;

	// ── Height estimates (px) ─────────────────────────────────────────────────
	static constexpr float   kPadV        = 10.0f;  // WindowPadding.y
	static constexpr float   kNameLineH   = 30.0f;  // BoldLargeFont line
	static constexpr float   kTextLineH   = 26.0f;  // default font line
	static constexpr float   kSepH        = 14.0f;  // Separator height

	// Per-bump content lines:
	//   Bump N header  → 1 line
	//   Start / End    → 1 line
	//   Amp / Width    → 1 line
	//   Pos            → 1 line
	//   Vel / Acc      → 1 line
	//                    ──────
	//                    5 lines total, TextWrapped on all data rows as safety
	static constexpr float kBumpLinesH = kTextLineH * 5;

	// ── FieldMetricsWindow ───────────────────────────────────────────────────

	FieldMetricsWindow::FieldMetricsWindow(const std::shared_ptr<Simulation>& simulation)
		: simulation(simulation)
	{}

	void FieldMetricsWindow::render()
	{
		ImGui::PushFont(g_BlackLargeFont);
		const bool open = ImGui::Begin("Neural Field Monitoring", nullptr, imgui_kit::getGlobalWindowFlags());
		ImGui::PopFont();
		if (!open) { ImGui::End(); return; }
		renderCards();
		ImGui::End();
	}

	void FieldMetricsWindow::renderCards() const
	{
		// Collect neural fields in simulation order
		std::vector<std::shared_ptr<element::NeuralField>> fields;
		for (const auto& el : simulation->getElements())
			if (el->getLabel() == element::NEURAL_FIELD)
				fields.push_back(std::dynamic_pointer_cast<element::NeuralField>(el));

		if (fields.empty())
		{
			ImGui::TextDisabled("No neural fields in simulation.");
			return;
		}

		// Column count and card width — re-evaluated every frame so resizing works
		const float availW      = ImGui::GetContentRegionAvail().x;
		const int   cardsPerRow = std::max(1, static_cast<int>((availW + kCardGap) / (kCardMinW + kCardGap)));
		// Subtract 1px to absorb float rounding and avoid 1-pixel overflow
		const float cardW       = std::floor((availW - kCardGap * float(cardsPerRow - 1)) / float(cardsPerRow)) - 1.0f;

		for (int i = 0; i < int(fields.size()); ++i)
		{
			const auto& nf    = fields[i];
			const auto  bumps = nf->getBumps();
			const int   bn    = int(bumps.size());

			// Card height:
			//   base  = padding + name + 4 summary lines
			//   bumps = first separator + per-bump lines + inter-bump separators
			float cardH = kPadV * 2.0f + kNameLineH + kTextLineH * 4.0f;
			if (bn > 0)
				cardH += kSepH + float(bn) * kBumpLinesH + float(bn - 1) * kSepH;

			if (i % cardsPerRow != 0)
				ImGui::SameLine(0.0f, kCardGap);

			ImGui::PushStyleColor(ImGuiCol_ChildBg, kCardBg);
			ImGui::PushStyleColor(ImGuiCol_Border,  kCardBorder);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   kCardRound);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, kCardBordSz);

			const std::string cid = "##nf_card_" + nf->getUniqueName();
			if (ImGui::BeginChild(cid.c_str(), ImVec2(cardW, cardH), true,
			                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				renderCardContent(nf, bumps);
			}
			ImGui::EndChild();

			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor(2);
		}
	}

	void FieldMetricsWindow::renderCardContent(
		const std::shared_ptr<element::NeuralField>& nf,
		const std::vector<element::NeuralFieldBump>& bumps)
	{
		// ── Field name ───────────────────────────────────────────────────────
		ImGui::PushFont(g_BoldLargeFont);
		ImGui::Text("%s", nf->getUniqueName().c_str());
		ImGui::PopFont();

		// ── Summary ──────────────────────────────────────────────────────────
		ImGui::Text("Stability: %s",            nf->isStable() ? "Stable" : "Unstable");
		ImGui::Text("Lowest activation: %.2f",  nf->getLowestActivation());
		ImGui::Text("Highest activation: %.2f", nf->getHighestActivation());
		ImGui::Text("Number of bumps: %d",      int(bumps.size()));

		// ── Bumps ─────────────────────────────────────────────────────────────
		if (!bumps.empty())
		{
			ImGui::Separator();
			for (int j = 0; j < int(bumps.size()); ++j)
			{
				const auto& b = bumps[j];

				// Header
				ImGui::PushFont(g_BoldMediumFont);
				ImGui::Text("Bump %d", j);
				ImGui::PopFont();

				// Data — each line kept short enough to avoid clipping at min card width
				ImGui::Text("Start: %.2f, End: %.2f",   b.startPosition, b.endPosition);
				ImGui::Text("Amp.: %.2f, Width: %.2f",  b.amplitude, b.width);
				ImGui::Text("Pos.: %.2f",               b.centroid);
				ImGui::Text("Vel.: %.2f, Acc.: %.2f",   b.velocity, b.acceleration);

				if (j < int(bumps.size()) - 1)
					ImGui::Separator();
			}
		}
	}
}

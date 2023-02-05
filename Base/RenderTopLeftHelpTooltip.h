#pragma once

static inline void renderTopLeftHelpTooltip(const char *const string)
{
	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin("HelpTooltip", nullptr, windowFlags))
	{
		ImGui::Button("?");
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(string);
		}
	}
	ImGui::End();
}

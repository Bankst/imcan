#pragma once

#include <string>

#include "imgui.h"
#include "imgui_internal.h"

bool EditableText(std::string idStr, std::string *backingData) {
	bool selected = false;
	ImGuiContext &g = *ImGui::GetCurrentContext();
	ImGuiWindow *window = g.CurrentWindow;
	ImVec2 pos_before = window->DC.CursorPos;

	ImGui::PushID(idStr.c_str());
	ImGui::PushStyleVar(
		ImGuiStyleVar_ItemSpacing, ImVec2(g.Style.ItemSpacing.x, g.Style.FramePadding.y * 2.0f));
	bool ret = ImGui::Selectable(
		"##Selectable", &selected,
		ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowItemOverlap);

	ImGui::PopStyleVar();

	ImGuiID id = window->GetID("##Input");
	bool temp_input_is_active = ImGui::TempInputIsActive(id);
	bool temp_input_start = ret ? ImGui::IsMouseDoubleClicked(0) : false;

	if (temp_input_start) ImGui::SetActiveID(id, window);

	if (temp_input_is_active || temp_input_start) {
		ImVec2 pos_after = window->DC.CursorPos;
		window->DC.CursorPos = pos_before;
		ret = ImGui::TempInputText(
			g.LastItemData.Rect, id, "##Input", backingData->data(), 256,  //(int) backingData->size(),
			ImGuiInputTextFlags_None);
		window->DC.CursorPos = pos_after;
	} else {
		window->DrawList->AddText(pos_before, ImGui::GetColorU32(ImGuiCol_Text), backingData->data());
	}

	ImGui::PopID();
	return ret;
}
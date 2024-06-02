#include "GuiHelpers.h"
#include "imgui.h"
#include "imgui_stdlib.h"

namespace imcan {

bool gui::GenericModal(
	std::string title, std::string text, std::string btn1_text, std::string btn2_text,
	const std::function<void(const bool, const bool)> &onPress) {
	bool interacted = false;
	if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("%s", text.c_str());

		const auto btn1Str = btn1_text.c_str();
		const auto btn2Str = btn2_text.c_str();

		// all this just to right-align a button...
		ImGuiStyle &style = ImGui::GetStyle();
		float availSpace = ImGui::GetContentRegionAvail().x;
		availSpace -= ImGui::CalcTextSize(btn1Str).x;
		availSpace -= (style.ItemSpacing.x * 2);
		availSpace -= ImGui::CalcTextSize(btn2Str).x;

		bool yes = ImGui::Button(btn1Str);
		ImGui::SameLine(0, availSpace);
		bool no = ImGui::Button(btn2Str);

		interacted = yes || no;
		if (interacted) {
			ImGui::CloseCurrentPopup();
			onPress(yes, no);
		}
		ImGui::EndPopup();
	}
	return interacted;
}

// general-purpose yes-no modal
bool gui::YesNoModal(
	std::string title, std::string text, const std::function<void(const bool)> &onPress) {
	static const auto yesStr = "   Yes   ";
	static const auto noStr = "    No   ";

	return GenericModal(title, text, yesStr, noStr, [onPress](const bool btn1, const bool btn2) {
		onPress(btn1 && !btn2);
	});
}

void gui::PrototypeEditableField(std::string label, std::string *backingData) {
	ImGui::Text("TransmitterEdit: ");
	ImGui::SameLine();
	ImGui::InputText(
		"##TransmitterEdit", backingData,
		ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll |
			ImGuiInputTextFlags_EnterReturnsTrue);
}
}  // namespace imcan
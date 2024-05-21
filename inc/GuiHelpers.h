#pragma once

#include <functional>
#include <string>

#include "imgui.h"
#include "imgui_stdlib.h"

namespace imcan {

class gui {
 public:
	static bool GenericModal(
		std::string title, std::string text, std::string btn1_text, std::string btn2_text,
		const std::function<void(const bool, const bool)> &onPress);

	// general-purpose yes-no modal
	static bool YesNoModal(
		std::string title, std::string text, const std::function<void(const bool)> &onPress);

	static void PrototypeEditableField(std::string label, std::string *backingData);
};

}  // namespace imcan


// for (int y = 0; y < 8; y++) {
// 	for (int x = 7; x >= 0; x--) {  // LtR 7-0
// 		if (x < 7) ImGui::SameLine();
// 		int id = y * 8 + x;
// 		ImGui::PushID(id);

// 		std::string bitName = std::to_string(id);
// 		// TODO: select logic based on cur signal length
// 		if (ImGui::Selectable(bitName.c_str(), selecteds[y][x], 0, ImVec2(20, 20))) {
// 			selecteds[y][x] ^= 1;
// 			// TODO: on-click
// 			fmt::println(
// 				"id {} at x,y ({}, {}) is now {}", id, x, y, (selecteds[y][x] == 1 ? "ON" : "OFF"));
// 		}
// 		ImGui::PopID();
// 	}
// }
#include "Dbc.h"
#include "glass/Context.h"
// #include <CANObjects.h>
#include <iostream>

namespace imcan {

std::unique_ptr<DbcManager> DbcGui::sDbcManager;

bool DbcManager::addDatabase(std::filesystem::path path) {
	if (std::filesystem::exists(path)) {
		DbcDatabase newdb;
		newdb.filepath = path;
		newdb.filename = path.filename();

		bool isNew = true;
		for(auto db : sDatabases) {
			if (db.filepath == newdb.filepath) {
				isNew = false;
				break;
			}
		}

		// // TODO: parser
		// try {
		// 	newdb.db = CppCAN::CANDatabase::fromFile(newdb.filepath);
		// } catch (CppCAN::CANDatabaseException &ex) {
		// 	std::cout << "DBC shidded britch, " << ex.what() << std::endl;
		// 	return false;
		// }

		if (isNew) {
			sDatabases.push_back(newdb);
		}
		return isNew;
	} else return false;
}

static void DisplayDatabase(DbcDatabase& dbc) {
	if (ImGui::TreeNode(dbc.filename.c_str())) {
		ImGui::Text("Path: %s", dbc.filepath.c_str());
		// ImGui::Text("Size: %lu", dbc.db.size());
		// ImGui::TreeNode()
		ImGui::TreePop();
	}
}

void DbcManager::DisplayLoader() {
	if (ImGui::Button("Load DBC")) {
		std::vector<std::string> filter = {"DBC Files", "*.dbc"};
		auto file = std::make_unique<pfd::open_file>("Choose .dbc to open", "", filter);
		bool loadOk = false;
		std::filesystem::path filepath;
		if (file && !file->result().empty()) {
			// TODO: find way to indicate error
			filepath = file.get()->result()[0];
			loadOk = addDatabase(filepath);
		}

		ImGui::OpenPopup("DBC Load");
		if (ImGui::BeginPopupModal("DBC Load")) {

			if (loadOk) {
				ImGui::Text("Loaded %s in 0.69ms", filepath.c_str());
			} else if (file) {
				ImGui::Text("Failed to load DBC %s", filepath.c_str());
			} else {
				ImGui::Text("File fucked yo");
			}

			if (ImGui::Button("OK")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	ImGui::Text("DBCs");

	for (auto dbc : sDatabases) {
		DisplayDatabase(dbc);
	}
}

void DbcGui::GlobalInit() {
	auto& storageRoot = glass::GetStorageRoot("dbc");
	sDbcManager = std::make_unique<DbcManager>(storageRoot);

	sDbcManager->GlobalInit();

	if (auto win = sDbcManager->AddWindow("Loader", [&] { sDbcManager->DisplayLoader(); })) {
		win->SetName("Loader");
		win->DisableRenamePopup();
		win->SetFlags(ImGuiWindowFlags_AlwaysAutoResize);
		win->SetDefaultPos(5, 200);
	}
}

} // namespace imcan
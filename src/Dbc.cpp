#include "Dbc.h"

#include <iostream>

#include "DbcNetwork.h"
#include "DbcSignal.h"
#include "fmt/core.h"
#include "glass/Context.h"

namespace imcan {

std::unique_ptr<DbcManager> DbcGui::sDbcManager;

bool DbcManager::addDatabase(std::filesystem::path path) {
	if (std::filesystem::exists(path)) {
		bool isNew = true;
		for (auto &db : sDatabases) {
			if (db.filepath == path) {
				isNew = false;
				break;
			}
		}
		if (!isNew) { return false; }

		// TODO: ret optional, err handling, unique_ptr?
		auto net = dbcan::Network::createFromDBC(path);
		if (net) { sDatabases.emplace_back(path, net); }
		return net != nullptr;
	} else
		return false;
}

static void DisplaySignal(const dbcan::Signal &sig) {
	if (ImGui::TreeNode(sig.name.c_str())) {
		ImGui::Text(
			"ByteOrder: %s",
			(sig.byteOrder == dbcan::ByteOrder::BigEndian) ? "BigEndian" : "LittleEndian");
		auto mux = sig.muxData ? sig.muxData->c_str() : "N/A";
		ImGui::Text("MuxData: %s", mux);
		// ImGui::Text("Comment: %s", sig.Comment().c_str());
		ImGui::Text("StartBit: %d", sig.startBit);
		ImGui::Text("Length (bits): %d", sig.length);
		ImGui::Text("Scale: %f", sig.scale);
		ImGui::Text("Offset: %f", sig.offset);
		ImGui::Text("Range: [%f, %f]", sig.valueRange.first, sig.valueRange.second);
		auto unit = sig.unit ? sig.unit->c_str() : "None";
		ImGui::Text("Unit: %s", unit);

		ImGui::TreePop();
	}
}

static void DisplaySignalEditor(const dbcan::Signal &sig) {}

static void DisplayMessage(const dbcan::Message &msg) {
	std::string msgTitle = fmt::format("0x{:3x} ({})", msg.id, msg.name);
	if (ImGui::TreeNode(msgTitle.c_str())) {
		ImGui::Text("ID: 0x%lX (%lu)", msg.id, msg.id);
		ImGui::Text("Transmitter: %s", msg.transmitter.c_str());
		ImGui::Text("Signals: %lu", msg.signals.size());
		ImGui::Text("Size: %hhu", msg.length);
		// ImGui::Text("Comment: %s", msg.Comment().c_str());

		if (ImGui::Button("Add Signal")) { std::cout << "later dawg" << std::endl; }

		for (const dbcan::Signal &sig : msg.signals) { DisplaySignal(sig); }
		ImGui::TreePop();
	}
}

// static void DisplayNode(const dbcppp::INode &node) {
// 	if (ImGui::TreeNode(node.Name().c_str())) {
// 		ImGui::Text("Comment: %s", node.Comment().c_str());
// 		ImGui::TreePop();
// 	}
// }

static void DisplayDatabase(DbcDatabase &dbc) {
	ImGui::Text("Path: %s", dbc.filepath.c_str());
	// ImGui::Text("Node Size: %lu", dbc.getNetwork()->Nodes_Size());
	ImGui::Text("Messages Size: %lu", dbc.getNetwork()->messages.size());
	ImGui::Text("Version: %s", std::string(dbc.getNetwork()->version).c_str());

	if (ImGui::Button("Add Message")) {
		static int idIncr = 1000;
		std::cout << "later dawg" << std::endl;
		// todo: get input
		dbc.hasChanges = true;
	}

	for (auto [id, msg] : dbc.getNetwork()->messages) { DisplayMessage(msg); }
}

void DbcManager::DisplayLoader() {
	if (ImGui::Button("Load DBC")) {
		std::vector<std::string> filter = { "DBC Files", "*.dbc" };
		auto file = std::make_unique<pfd::open_file>("Choose .dbc to open", "", filter);
		bool loadOk = false;
		std::filesystem::path filepath;
		if (file && !file->result().empty()) {
			// TODO: find way to indicate error
			filepath = file.get()->result()[0];
			loadOk = addDatabase(filepath);
		}

		std::cout << "DBC Load " << (loadOk ? "OK" : "FAIL") << std::endl;

		ImGui::OpenPopup("DBC Load");
		if (ImGui::BeginPopup("DBC Load")) {
			if (loadOk) {
				ImGui::Text("Loaded %s in 0.69ms", filepath.c_str());
			} else if (file) {
				ImGui::Text("Failed to load DBC %s", filepath.c_str());
			} else {
				ImGui::Text("File fucked yo");
			}

			if (ImGui::Button("OK")) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
	}

	if (sDatabases.empty()) { return; }

	// ImGui::Text("DBCs");
	if (ImGui::BeginTabBar("DBCTabs")) {
		for (auto it = sDatabases.begin(); it != sDatabases.end();) {
			auto &dbc = *it;
			bool open = true;
			ImGuiTabItemFlags flags =
				dbc.hasChanges ? ImGuiTabItemFlags_UnsavedDocument : ImGuiTabItemFlags_None;
			if (ImGui::BeginTabItem(dbc.filename.c_str(), &open, flags)) {
				DisplayDatabase(dbc);
				ImGui::EndTabItem();
			}

			if (!open) {
				// pls go bye bye
				it = sDatabases.erase(it);
			} else {
				++it;
			}
		}

		ImGui::EndTabBar();
	}
}

void DbcGui::GlobalInit() {
	auto &storageRoot = glass::GetStorageRoot("dbc");
	sDbcManager = std::make_unique<DbcManager>(storageRoot);

	sDbcManager->GlobalInit();

	if (auto win = sDbcManager->AddWindow("Loader", [&] { sDbcManager->DisplayLoader(); })) {
		win->SetName("Loader");
		win->DisableRenamePopup();
		win->SetFlags(ImGuiWindowFlags_AlwaysAutoResize);
		win->SetDefaultPos(5, 200);
	}
}

}  // namespace imcan
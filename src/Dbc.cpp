#include "Dbc.h"
#include "fmt/format.h"
#include "glass/Context.h"
#include <fstream>
#include <iostream>

namespace imcan {

std::unique_ptr<DbcManager> DbcGui::sDbcManager;

bool DbcManager::addDatabase(std::filesystem::path path) {
	if (std::filesystem::exists(path)) {
		bool isNew = true;
		for(auto &db : sDatabases) {
			if (db.filepath == path) {
				isNew = false;
				break;
			}
		}
		if (!isNew) { return false; }

		auto filestream = std::ifstream{path};
		std::unique_ptr<dbcppp::INetwork> dbcNetwork = dbcppp::INetwork::LoadDBCFromIs(filestream);
		if (!dbcNetwork) { return false; }

		DbcDatabase newdb{path, std::move(dbcNetwork)};
		sDatabases.push_back(std::move(newdb));
		return true;
	} else return false;
}

static void DisplaySignal(const dbcppp::ISignal& sig) {
	if (ImGui::TreeNode(sig.Name().c_str())) {
		ImGui::Text("ByteOrder: %s", 
			(sig.ByteOrder() == dbcppp::ISignal::EByteOrder::BigEndian) ? 
				"BigEndian" : "LittleEndian"
		);
		ImGui::Text("Comment: %s", sig.Comment().c_str());
		ImGui::Text("Unit: %s", sig.Unit().c_str());
		ImGui::Text("Factor: %f", sig.Factor());
		ImGui::Text("Minimum: %f", sig.Minimum());
		ImGui::Text("Maximum: %f", sig.Maximum());
		ImGui::Text("Offset: %f", sig.Offset());

		ImGui::TreePop();
	}
}

static void DisplayMessage(const dbcppp::IMessage& msg) {
	std::string msgTitle = fmt::format("0x{:3x} ({})", msg.Id(), msg.Name());
	if(ImGui::TreeNode(msgTitle.c_str())) {
		ImGui::Text("ID: 0x%lX (%lu)", msg.Id(), msg.Id());
		ImGui::Text("Signals: %lu", msg.Signals_Size());
		ImGui::Text("Size: %lu", msg.MessageSize());
		ImGui::Text("Transmitter: %s", msg.Transmitter().c_str());

		for (const dbcppp::ISignal& sig : msg.Signals()) {
			DisplaySignal(sig);
		}
		ImGui::TreePop();
	}
}

static void DisplayDatabase(DbcDatabase& dbc) {
	if (ImGui::TreeNode(dbc.filename.c_str())) {
		ImGui::Text("Path: %s", dbc.filepath.c_str());
		ImGui::Text("Node Size: %lu", dbc.network->Nodes_Size());
		ImGui::Text("Messages Size: %lu", dbc.network->Messages_Size());
		ImGui::Text("Version: %s", dbc.network->Version().c_str());

		for (const dbcppp::IMessage& msg : dbc.network->Messages()) {
			DisplayMessage(msg);
		}
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

		std::cout << "DBC Load" << (loadOk ? "OK" : "FAIL") << std::endl;

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

	for (auto &dbc : sDatabases) {
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
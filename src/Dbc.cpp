#include <portable-file-dialogs.h>
#include <sys/types.h>

#include <iostream>

#include "Dbc.h"
#include "GuiHelpers.h"
#include "glass/Context.h"

namespace imcan {

std::unique_ptr<DbcManager> DbcGui::sDbcManager;

bool DbcManager::addDatabase(std::filesystem::path path) {
	if (std::filesystem::exists(path)) {
		bool isNew = true;
		for (auto &db : m_sDatabases) {
			if (db.filepath == path) {
				isNew = false;
				break;
			}
		}
		if (!isNew) { return false; }

		// TODO: ret optional, err handling, unique_ptr?
		auto net = dbcan::Network::createFromDBC(path);
		if (net) {
			m_sDatabases.emplace_back(path, net);
		} else {
			fmt::println("Err: failed to parse DBC!");
		}
		return net != nullptr;
	} else
		return false;
}

void DbcSignalView::DisplayCtxMenu() {
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { ImGui::CloseCurrentPopup(); }
		static const std::string delModalStr = "Delete Signal?";
		ImGui::Text("%s", m_sig->name.c_str());

		if (ImGui::Button("Delete")) { ImGui::OpenPopup(delModalStr.c_str()); }

		bool modal = gui::YesNoModal(
			delModalStr, fmt::format("Delete Signal \"{}\"", m_sig->name), [this](const bool yes) {
				if (yes) { m_msg->DeleteSignal(m_sig->msgIndex); }
			});
		if (modal) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}

void DbcSignalView::Display() {
	bool nodeOpen = ImGui::TreeNode(m_sig->name.c_str());
	DisplayCtxMenu();
	if (nodeOpen) {
		ImGui::Text(
			"ByteOrder: %s",
			(m_sig->byteOrder == dbcan::ByteOrder::BigEndian) ? "BigEndian" : "LittleEndian");
		auto mux = m_sig->muxData != "" ? m_sig->muxData.c_str() : "N/A";
		ImGui::Text("MuxData: %s", mux);
		ImGui::Text("StartBit: %d", m_sig->startBit);
		ImGui::Text("Length (bits): %d", m_sig->length);
		ImGui::Text("Scale: %f", m_sig->scale);
		ImGui::Text("Offset: %f", m_sig->offset);
		ImGui::Text("Range: [%f, %f]", m_sig->valueRange.first, m_sig->valueRange.second);
		auto unit = m_sig->unit == "" ? m_sig->unit.c_str() : "None";
		ImGui::Text("Unit: %s", unit);
		ImGui::Text("Comment: %s", m_sig->comment.c_str());

		ImGui::TreePop();
	}
}

void DbcSignalView::DisplayEditor() {}

void DbcNetworkView::Display() {
	ImGui::Text("Version: %s", std::string(m_net->version).c_str());

	if (ImGui::TreeNode(fmt::format("Messages: {}###Messages", m_net->messages.size()).c_str())) {
		std::vector<std::shared_ptr<DbcMessageView>> viewsWithEditors;
		for (auto [id, msg] : m_net->messages) {
			auto msgView = std::make_shared<DbcMessageView>(this, msg);
			msgView->Display();
			if (msgView->IsEditing()) { viewsWithEditors.push_back(msgView); }
		}
		ImGui::TreePop();

		// render editors outside of tree
		for (auto &view : viewsWithEditors) { view->DisplayEditor(); }
	}

	// handle changes
	if (m_msgToDelete.has_value()) {
		auto id = m_msgToDelete.value();
		auto netMsgErased = m_net->deleteMessage(id);
		if (netMsgErased == 1) {
			MadeChanges();
		} else {
			fmt::println("Err: failed to erase message {}", id);
		}
		m_msgToDelete = std::nullopt;
	}
}

void DbcGui::DisplayDatabase(DbcDatabase &dbc) {
	ImGui::TextWrapped("Path: %s", dbc.filepath.c_str());

	auto networkView = DbcNetworkView { dbc.getNetwork() };
	networkView.Display();
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
		auto dbcModalStr = "DBC Load";

		ImGui::OpenPopup(dbcModalStr);
		if (ImGui::BeginPopupModal(dbcModalStr, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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

	if (m_sDatabases.empty()) { return; }

	if (ImGui::BeginTabBar("DBCTabs")) {
		for (auto it = m_sDatabases.begin(); it != m_sDatabases.end();) {
			auto &dbc = *it;
			ImGuiTabItemFlags flags =
				dbc.getNetwork()->hasChanges ? ImGuiTabItemFlags_UnsavedDocument : ImGuiTabItemFlags_None;
			if (ImGui::BeginTabItem(dbc.filename.c_str(), &dbc.open, flags)) {
				DbcGui::DisplayDatabase(dbc);
				ImGui::EndTabItem();
			}

			if (dbc.getNetwork()->hasChanges && !dbc.open) {
				// todo: modal prompt
				fmt::println("Closed unsaved!!!");
			}

			if (!dbc.open) {
				// pls go bye bye
				it = m_sDatabases.erase(it);
			} else {
				++it;
			}
		}

		ImGui::EndTabBar();
	}
}

void DbcNetworkView::DeleteMessage(uint64_t id) { m_msgToDelete = id; }

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
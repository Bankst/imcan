#include "Dbc.h"

#include <iostream>

#include "DbcNetwork.h"
#include "DbcSignal.h"
#include "fmt/core.h"
#include "glass/Context.h"
#include "imgui.h"

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
		if (net) { m_sDatabases.emplace_back(path, net); }
		return net != nullptr;
	} else
		return false;
}

void DbcSignalView::DisplayCtxMenu() const {
	if (ImGui::BeginPopupContextItem()) {
		static const auto delModalStr = "Delete Signal?";
		ImGui::Text("%s", m_sig.name.c_str());

		if (ImGui::Button("Delete")) { ImGui::OpenPopup(delModalStr); }

		bool modalInteracted = false;
		if (ImGui::BeginPopupModal(delModalStr, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			const auto delStr = fmt::format("Delete Signal \"{}\"?", m_sig.name);
			ImGui::Text("%s", delStr.c_str());

			static const auto yesStr = "   Yes   ";
			static const auto noStr = "    No   ";

			// all this just to right-align a button...
			ImGuiStyle &style = ImGui::GetStyle();
			float availSpace = ImGui::GetContentRegionAvail().x;
			availSpace -= ImGui::CalcTextSize(yesStr).x;
			availSpace -= (style.ItemSpacing.x * 2);
			availSpace -= ImGui::CalcTextSize(noStr).x;

			bool yes = ImGui::Button(yesStr);
			ImGui::SameLine(0, availSpace);
			bool no = ImGui::Button(noStr);

			if (yes) { m_msg.DeleteSignal(m_sig.name); }

			modalInteracted = yes || no;
			if (modalInteracted) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
		if (modalInteracted) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}

void DbcSignalView::Display() const {
	bool open = ImGui::TreeNode(m_sig.name.c_str());
	DisplayCtxMenu();
	if (open) {
		ImGui::Text(
			"ByteOrder: %s",
			(m_sig.byteOrder == dbcan::ByteOrder::BigEndian) ? "BigEndian" : "LittleEndian");
		auto mux = m_sig.muxData ? m_sig.muxData->c_str() : "N/A";
		ImGui::Text("MuxData: %s", mux);
		ImGui::Text("StartBit: %d", m_sig.startBit);
		ImGui::Text("Length (bits): %d", m_sig.length);
		ImGui::Text("Scale: %f", m_sig.scale);
		ImGui::Text("Offset: %f", m_sig.offset);
		ImGui::Text("Range: [%f, %f]", m_sig.valueRange.first, m_sig.valueRange.second);
		auto unit = m_sig.unit == "" ? m_sig.unit.c_str() : "None";
		ImGui::Text("Unit: %s", unit);
		ImGui::Text("Comment: %s", m_sig.comment.c_str());

		ImGui::TreePop();
	}
}

void DbcSignalView::DisplayEditor() const {}

void DbcMessageView::Display() const {
	std::string msgTitle = fmt::format("0x{:3x} ({})", m_msg.id, m_msg.name);
	if (ImGui::TreeNode(msgTitle.c_str())) {
		ImGui::Text("ID: 0x%lX (%lu)", m_msg.id, m_msg.id);
		ImGui::Text("Transmitter: %s", m_msg.transmitter.c_str());
		ImGui::Text("Signals: %lu", m_msg.signals.size());
		ImGui::Text("Size: %hhu", m_msg.length);
		// ImGui::Text("Comment: %s", msg.Comment().c_str());

		if (ImGui::Button("Add Signal")) { std::cout << "later dawg" << std::endl; }

		for (const dbcan::Signal &sig : m_msg.signals) {
			auto sigView = DbcSignalView { *this, sig };
			sigView.Display();
		}
		ImGui::TreePop();
	}
}

void DbcMessageView::DeleteSignal(std::string name) const {
	// m_msg.
}

void DbcNetworkView::Display() {
	ImGui::Text("Messages Size: %lu", m_net->messages.size());
	ImGui::Text("Version: %s", std::string(m_net->version).c_str());

	// if (ImGui::Button("Add Message")) {
	// TODO: change tracking??
	// dbc.hasChanges = true;
	// }

	for (const auto [id, msg] : m_net->messages) {
		auto msgView = DbcMessageView { *this, msg };
		msgView.Display();
	}
}

void DbcGui::DisplayDatabase(DbcDatabase &dbc) {
	ImGui::Text("Path: %s", dbc.filepath.c_str());

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
			bool open = true;
			ImGuiTabItemFlags flags =
				dbc.getNetwork()->hasChanges ? ImGuiTabItemFlags_UnsavedDocument : ImGuiTabItemFlags_None;
			if (ImGui::BeginTabItem(dbc.filename.c_str(), &open, flags)) {
				DbcGui::DisplayDatabase(dbc);
				ImGui::EndTabItem();
			}

			if (!open) {
				// pls go bye bye
				it = m_sDatabases.erase(it);
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
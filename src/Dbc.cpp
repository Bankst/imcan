#include "Dbc.h"

#include <sys/types.h>

#include <cstdint>
#include <iostream>
#include <optional>

#include "DbcMessage.h"
#include "DbcNetwork.h"
#include "DbcSignal.h"
#include "fmt/core.h"
#include "glass/Context.h"
#include "imgui.h"
#include "imgui_stdlib.h"

namespace imcan {

std::unique_ptr<DbcManager> DbcGui::sDbcManager;
std::map<uint64_t, dbcan::Message::Ptr> DbcMessageView::sm_editingMsgs;

bool GenericModal(
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
bool YesNoModal(
	std::string title, std::string text, const std::function<void(const bool)> &onPress) {
	static const auto yesStr = "   Yes   ";
	static const auto noStr = "    No   ";

	return GenericModal(title, text, yesStr, noStr, [onPress](const bool btn1, const bool btn2) {
		onPress(btn1 && !btn2);
	});
}

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
		static const std::string delModalStr = "Delete Signal?";
		ImGui::Text("%s", m_sig->name.c_str());

		if (ImGui::Button("Delete")) { ImGui::OpenPopup(delModalStr.c_str()); }

		bool modal = YesNoModal(
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

void DbcMessageView::DisplayCtxMenu() {
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { ImGui::CloseCurrentPopup(); }
		static const std::string delModalStr = "Delete Message?";
		ImGui::Text("%s", m_longTitle.c_str());

		if (ImGui::Button("Delete")) { ImGui::OpenPopup(delModalStr.c_str()); }

		bool modal = YesNoModal(
			delModalStr, fmt::format("Delete Message \"{}\"", m_msg->name), [this](const bool yes) {
				if (yes) { m_net->DeleteMessage(m_msg->id); }
			});
		if (modal) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}

void PrototypeEditableField(std::string label, std::string *backingData) {
	ImGui::Text("TransmitterEdit: ");
	ImGui::SameLine();
	ImGui::InputText(
		"##TransmitterEdit", backingData,
		ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll |
			ImGuiInputTextFlags_EnterReturnsTrue);
}

void DbcMessageView::DisplayEditor() {
	bool editorOpen = ImGui::BeginPopup(m_editTitle.c_str());
	if (editorOpen) {
		if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { ImGui::CloseCurrentPopup(); }

		auto msg = sm_editingMsgs.at(m_msg->id);
		// TODO: make a generic editable field doodad?
		ImGui::Text("ID:");
		ImGui::SameLine();
		std::string idBuf = fmt::format("0x{:3x}", msg->id);
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("0x123456789").x);
		bool edited = ImGui::InputTextWithHint(
			"##IdField", "Hexadecimal value", &idBuf, ImGuiInputTextFlags_CharsHexadecimal);
		if (edited) { msg->id = std::stoull(idBuf); }

		ImGui::Text("Transmitter:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("ExampleTransmitterNameHere______").x);
		edited = ImGui::InputTextWithHint("##TxrField", "Transmitting Node", &msg->transmitter);
if (edited) {
			// do nothin ig
		}

		ImGui::Text("Length:");
		ImGui::SameLine();
		std::string lenBuf = fmt::format("{}", msg->length);
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("999").x);
		edited = ImGui::InputTextWithHint(
			"##IdField", "Hexadecimal or Integer value", &lenBuf, ImGuiInputTextFlags_CharsDecimal);
		if (edited) { msg->length = std::stoi(lenBuf); }

		// TODO: live-apply without full edit exit?
		if (ImGui::Button("Save")) {
			EndEdit();
			ImGui::CloseCurrentPopup();
		}

		// TODO: prompt on unsaved changes
		if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }

		ImGui::EndPopup();
	}
}

void DbcMessageView::Display() {
	if (IsEditing()) { ImGui::SetNextItemOpen(true); }
	bool nodeOpen = ImGui::TreeNode(m_longTitle.c_str());
	DisplayCtxMenu();
	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		if (BeginEdit()) {
			// popup window for editor
			ImGui::OpenPopup(m_editTitle.c_str());
		}
	}
	DisplayEditor();
	if (nodeOpen) {
		ImGui::Text("ID: 0x%lX (%lu)", m_msg->id, m_msg->id);
		ImGui::Text("Transmitter: %s", m_msg->transmitter.c_str());
		ImGui::Text("Signals: %lu", m_msg->signals.size());
		ImGui::Text("Size: %hhu", m_msg->length);
		ImGui::Text("Comment: %s", m_msg->comment.c_str());

		if (ImGui::Button("Add Signal")) { std::cout << "later dawg" << std::endl; }

		for (auto &[_, sig] : m_msg->signals) {
			auto sigView = DbcSignalView { this, sig };
			sigView.Display();
		}
		ImGui::TreePop();

		// handle deletion after iteration
		if (m_sigToDelete != 0) {
			bool ok = m_msg->deleteSignal(m_sigToDelete);
			if (!ok) {
				fmt::println("Err: failed to delete signal {} from message {}", m_sigToDelete, m_msg->name);
			} else {
				m_net->MadeChanges();
			}
			m_sigToDelete = 0;
		}
	}
}

void DbcMessageView::DeleteSignal(uint64_t sigId) { m_sigToDelete = sigId; }

void DbcNetworkView::Display() {
	ImGui::Text("Version: %s", std::string(m_net->version).c_str());

	if (ImGui::TreeNode(fmt::format("Messages: {}###Messages", m_net->messages.size()).c_str())) {
		for (auto [id, msg] : m_net->messages) {
			auto msgView = DbcMessageView { this, msg };
			msgView.Display();
		}
		ImGui::TreePop();
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
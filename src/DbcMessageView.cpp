#include <iostream>

#include "Dbc.h"
#include "GuiHelpers.h"
#include "imgui_stdlib.h"

namespace imcan {

std::map<uint64_t, bool> DbcMessageView::sm_modifiedStatuses;
std::map<uint64_t, DbcMessageView::EditCtxPtr> DbcMessageView::sm_editingMsgs;

DbcMessageView::DbcMessageView(DbcNetworkView *parentNet_, dbcan::Message::Ptr msg_)
		: m_net(parentNet_),
			m_msg(std::move(msg_)),
			// TODO: title based on a unique ID - needs dbcan changes
			m_longTitle(fmt::format("0x{:3x} ({})", m_msg->id, m_msg->name)),
			m_editTitle(fmt::format("Editing - 0x{:3x} ({})", m_msg->id, m_msg->name)),
			m_delModalStr(fmt::format("Delete Message?###{}", m_msg->id)) {
	if (sm_editingMsgs.contains(m_msg->id)) {
		// load editingMsg from map
		m_editContext = sm_editingMsgs.at(m_msg->id);
	}
}

void DbcMessageView::Display() {
	if (IsEditing()) { ImGui::SetNextItemOpen(true); }
	bool nodeOpen = ImGui::TreeNodeEx(m_longTitle.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap);

	ImGui::SameLine(0, 1);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
	bool edit = ImGui::SmallButton(fmt::format("e##EditBtn{}", m_msg->id).c_str());
	ImGui::SameLine(0, 1);
	bool del = ImGui::SmallButton(fmt::format("d##DelBtn{}", m_msg->id).c_str());
	if (HasUnsavedChanges()) {
		ImGui::SameLine(0, 1);
		ImGui::BeginDisabled();
		ImGui::SmallButton("*");
		ImGui::EndDisabled();
	}
	ImGui::PopStyleVar();

	if (edit) {
		if (BeginEdit()) {
			// nothin ig
		} else {
			std::cout << "[DMV] Error: Edit Failed!!!" << std::endl;
		}
	}

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

	// delete/edit popup handlers
	bool delModal = gui::YesNoModal(
		m_delModalStr, fmt::format("Delete Message \"0x{:3x} {}\"?", m_msg->id, m_msg->name),
		[this](const bool yes) {
			if (yes) { m_net->DeleteMessage(m_msg->id); }
		});
	if (delModal) { ImGui::CloseCurrentPopup(); }

	if (del) { ImGui::OpenPopup(m_delModalStr.c_str()); }
}

void DbcMessageView::DeleteSignal(uint64_t sigId) { m_sigToDelete = sigId; }

void DbcMessageView::DisplayEditor() {
	// idk
	DisplayEditorInternal();
}

void DbcMessageView::DisplayEditorInternal() {
	if (!IsEditing()) return;
	auto ctx = sm_editingMsgs.at(m_msg->id);

	int flags = ImGuiWindowFlags_AlwaysAutoResize;
	if (ctx->modified) { flags |= ImGuiWindowFlags_UnsavedDocument; }

	bool editorOpen = ImGui::Begin(m_editTitle.c_str(), nullptr, flags);
	if (editorOpen) {
		if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { ImGui::CloseCurrentPopup(); }
		auto msg = ctx->msg;
		// TODO: make a generic editable field doodad?

		ImGui::Text("Name:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("ExampleMessageNameHere______").x);
		bool edited = ImGui::InputTextWithHint(
			"##NameField", "Message Name", &msg->name, ImGuiInputTextFlags_CharsNoBlank);
		if (edited) { ctx->modified = true; }

		// Editing this ID breaks everything. need to update the editingMsgs map.
		ImGui::BeginDisabled(true);
		ImGui::Text("ID:");
		ImGui::SameLine();
		std::string idBuf = fmt::format("0x{:3x}", msg->id);
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("0x123456789").x);
		edited = ImGui::InputTextWithHint(
			"##IdField", "Hexadecimal value", &idBuf, ImGuiInputTextFlags_CharsHexadecimal);
		if (edited && !idBuf.empty()) {
			// todo: this can 💣
			msg->id = std::stoull(idBuf, nullptr, 16);
			ctx->modified = true;
		}
		ImGui::EndDisabled();

		ImGui::Text("Transmitter:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("ExampleTransmitterNameHere______").x);
		edited = ImGui::InputTextWithHint(
			"##TxrField", "Transmitting Node", &msg->transmitter, ImGuiInputTextFlags_CharsNoBlank);
		if (edited) {
			// TODO: this won't account for changing it back to what it was before
			ctx->modified = true;
		}

		ImGui::Text("Length:");
		ImGui::SameLine();
		std::string lenBuf = fmt::format("{}", msg->length);
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("999").x);
		edited = ImGui::InputTextWithHint(
			"##LengthField", "Hexadecimal or Integer value", &lenBuf, ImGuiInputTextFlags_CharsDecimal);
		if (edited && !lenBuf.empty()) {
			// todo: this can 💣
			int newLen = std::stoi(lenBuf);
			if (newLen >= 0 && newLen <= 64) {
				ctx->modified = true;
				msg->length = newLen;
			}
		}

		bool updateEdit = false;
		bool doSave = false;
		bool stopEdit = true;
		if (ImGui::Button("OK")) {
			// TODO: close??
			updateEdit = true;
			doSave = m_editContext->modified;
		}
		ImGui::SameLine();

		ImGui::BeginDisabled(!m_editContext->modified);
		if (ImGui::Button("Apply")) {
			// TODO: close??
			updateEdit = true;
			doSave = true;
			stopEdit = false;
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			// TODO: prompt on unsaved changes
			updateEdit = true;
		}
		if (updateEdit) { EndEdit(doSave, stopEdit); }
	}
	ImGui::End();
}

bool DbcMessageView::IsEditing() const { return m_editContext != nullptr; }

void DbcMessageView::UpdateEditingMsg() {
	m_editContext.reset();
	m_editContext = std::make_shared<EditCtx>(false, std::make_shared<dbcan::Message>(*m_msg.get()));
	sm_editingMsgs.insert_or_assign(m_msg->id, m_editContext);
}

bool DbcMessageView::BeginEdit() {
	if (IsEditing()) {
		ImGui::SetWindowFocus(m_editTitle.c_str());
		return true;
	}
	if (!sm_editingMsgs.contains(m_msg->id)) {
		// insert copy of message for editor to mutate
		UpdateEditingMsg();
		return true;
	}

	return false;
}

void DbcMessageView::EndEdit(bool apply, bool end) {
	if (m_editContext && sm_editingMsgs.contains(m_msg->id)) {
		if (end) { sm_editingMsgs.erase(m_editContext->msg->id); }

		// apply to orig msg, copy back if not ending
		if (apply) {
			std::swap(*m_editContext->msg, *m_msg);
			if (!end) { UpdateEditingMsg(); }

			// TODO: equality checker on members
			m_editContext->modified = false;
			sm_modifiedStatuses[m_msg->id] = true;
		}

		if (end) { m_editContext.reset(); }
	} else {
		throw new std::runtime_error("Shitballs 1.0");
	}
}

bool DbcMessageView::HasUnsavedChanges() const {
	return sm_modifiedStatuses[m_msg->id];

	// auto iter = sm_modifiedStatuses.find(m_msg->id);
	// if (iter != sm_modifiedStatuses.end()) { return iter->second; }
	// return false;
}

}  // namespace imcan

#pragma once

#include <portable-file-dialogs.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "DbcMessage.h"
#include "DbcNetwork.h"
#include "glass/WindowManager.h"

namespace imcan {

class DbcMessageView;  // forward-decl
class DbcNetworkView {
 public:
	DbcNetworkView(const std::shared_ptr<dbcan::Network> net_) : m_net(net_) {}

	void Display();
	void MadeChanges() { m_net->hasChanges = true; }
	void DeleteMessage(uint64_t id);

 private:
	const std::shared_ptr<dbcan::Network> m_net;
	std::optional<uint64_t> m_msgToDelete = std::nullopt;
};

class DbcMessageView {
 public:
	DbcMessageView(DbcNetworkView *parentNet_, dbcan::Message::Ptr msg_)
			: m_net(parentNet_),
				m_msg(std::move(msg_)),
				m_longTitle(fmt::format("0x{:3x} ({})", m_msg->id, m_msg->name)),
				m_editTitle(fmt::format("Editing - 0x{:3x} ({})", m_msg->id, m_msg->name)) {
		if (sm_editingMsgs.contains(m_msg->id)) {
			// load editingMsg from map
			m_editingMsg = sm_editingMsgs.at(m_msg->id);
		}
	}

	void Display();
	void DeleteSignal(uint64_t sigId);

 private:
	void DisplayCtxMenu();
	void DisplayEditor();

	DbcNetworkView *m_net;
	dbcan::Message::Ptr m_msg;
	dbcan::Message::Ptr m_editingMsg;

	const std::string m_editTitle;

	bool IsEditing() { return m_editingMsg != nullptr; }

	bool BeginEdit() {
		if (IsEditing()) return true;

		if (!sm_editingMsgs.contains(m_msg->id)) {
			// insert copy of message for editor to mutate
			m_editingMsg = std::make_shared<dbcan::Message>(*m_msg.get());
			sm_editingMsgs.insert({ m_msg->id, m_editingMsg });
			return true;
		}
		return false;
	}

	void EndEdit() {
		if (m_editingMsg && sm_editingMsgs.contains(m_msg->id)) {
			sm_editingMsgs.erase(m_editingMsg->id);
			// save to orig msg
			m_msg.swap(m_editingMsg);
			m_editingMsg.reset();
		} else {
			throw new std::runtime_error("Shitballs 1.0");
		}
	}

	const std::string m_longTitle;

	static std::map<uint64_t, dbcan::Message::Ptr> sm_editingMsgs;

	uint64_t m_sigToDelete = 0;  // signals are 1-indexed, so 0 means none
};

class DbcSignalView {
 public:
	DbcSignalView(DbcMessageView *parentMsg_, dbcan::Signal::Ptr sig_)
			: m_msg(parentMsg_), m_sig(std::move(sig_)) {}

	void Display();

 private:
	DbcMessageView *m_msg;
	dbcan::Signal::Ptr m_sig;

	void DisplayCtxMenu();
	void DisplayEditor();
};

class DbcDatabase {
 public:
	DbcDatabase(std::filesystem::path _filepath, std::shared_ptr<dbcan::Network> _network)
			: filepath(_filepath), filename(_filepath.filename()) {
		m_network = std::move(_network);
	}

	std::filesystem::path filepath;
	std::string filename;
	bool open = true;

	const std::shared_ptr<dbcan::Network> getNetwork() { return m_network; }

 private:
	std::shared_ptr<dbcan::Network> m_network;
};  // class DbcDatabase

class DbcManager : public glass::WindowManager {
	using databases_t = std::vector<DbcDatabase>;

 public:
	explicit DbcManager(glass::Storage &storage) : WindowManager { storage } {};

	void DisplayLoader();
	DbcDatabase *getCurDb();

 private:
	bool addDatabase(std::filesystem::path path);
	DbcDatabase *m_curDb;
	databases_t m_sDatabases;
};  // class DbcManager

class DbcGui {
 public:
	static void GlobalInit();

	static std::unique_ptr<DbcManager> sDbcManager;

	static void DisplayLoader();
	static void DisplayDatabase(DbcDatabase &db);
};  // class DbcGui

}  // namespace imcan
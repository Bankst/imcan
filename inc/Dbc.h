#pragma once

#include <portable-file-dialogs.h>

#include <filesystem>
#include <memory>
#include <string>
#include <utility>

#include "DbcMessage.h"
#include "DbcNetwork.h"
#include "glass/WindowManager.h"

namespace imcan {

class DbcNetworkView {
 public:
	DbcNetworkView(const std::shared_ptr<dbcan::Network> net_) : m_net(net_) {}

	void Display();
	void MadeChanges() { m_net->hasChanges = true; }

 private:
	const std::shared_ptr<dbcan::Network> m_net;
};

class DbcMessageView {
 public:
	DbcMessageView(DbcNetworkView *parentNet_, dbcan::Message::Ptr msg_)
			: m_net(parentNet_), m_msg(std::move(msg_)) {}

	void Display();
	void DeleteSignal(uint64_t sigId);

 private:
	DbcNetworkView *m_net;
	dbcan::Message::Ptr m_msg;

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
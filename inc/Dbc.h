#pragma once

#include <portable-file-dialogs.h>

#include <filesystem>
#include <memory>
#include <string>

#include "DbcNetwork.h"
#include "glass/WindowManager.h"

namespace imcan {

class DbcNetworkView {
 public:
	DbcNetworkView(const std::shared_ptr<dbcan::Network> net_) : m_net(net_) {}

	void Display();

 private:
	const std::shared_ptr<dbcan::Network> m_net;
};

class DbcMessageView {
 public:
	DbcMessageView(const DbcNetworkView &parentNet_, const dbcan::Message &msg_)
			: m_net(parentNet_), m_msg(msg_) {}

	void Display() const;
	void DeleteSignal(std::string name) const;

 private:
	const DbcNetworkView &m_net;
	const dbcan::Message &m_msg;
};

class DbcSignalView {
 public:
	DbcSignalView(const DbcMessageView &parentMsg_, const dbcan::Signal &sig_)
			: m_msg(parentMsg_), m_sig(sig_) {}

	void Display() const;

 private:
	const DbcMessageView &m_msg;
	const dbcan::Signal &m_sig;

	void DisplayCtxMenu() const;
	void DisplayEditor() const;
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
#include <iostream>

#include <wpigui.h>
#include <wpigui_openurl.h>

#include "glass/Context.h"
#include "glass/MainMenuBar.h"
#include "glass/Model.h"
#include "glass/Storage.h"
#include "glass/View.h"
#include "glass/other/Log.h"
#include "glass/other/Plot.h"

#include "Dbc.h"

namespace gui = wpi::gui;

static glass::MainMenuBar gMainMenu;
static bool gAbout = false;
static bool gDbcMgr = false;
static void (*gPrevKeyCallback)(GLFWwindow*, int, int, int, int);

#ifdef _WIN32
int __stdcall WinMain(void* hInstance, void* hPrevInstance, char* pCmdLine,
                      int nCmdShow) {
  int argc = __argc;
  char** argv = __argv;
#else
int main(int argc, char** argv) {
#endif
	std::string_view saveDir;
	if (argc == 2) {
		saveDir = argv[1];
	}

	gui::CreateContext();
	glass::CreateContext();

	gui::AddEarlyExecute([] { ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()); });
	gui::AddInit([] { ImGui::GetIO().ConfigDockingWithShift = true; });

	glass::SetStorageName("imcan");
	glass::SetStorageDir(saveDir.empty() ? gui::GetPlatformSaveFileDir() : saveDir);

	gui::AddInit([] { glass::ResetTime(); });

	gui::AddLateExecute([] {gMainMenu.Display(); });

	gMainMenu.AddMainMenu([] {
		if (ImGui::BeginMenu("Windows")) {
			if (ImGui::MenuItem("DBC Manager")) {
				imcan::DbcGui::sDbcManager->GetWindow("Loader")->SetVisible(true);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Info")) {
			if (ImGui::MenuItem("About")) {
				gAbout = true;
			}
			ImGui::EndMenu();
		}
	});

	gui::AddLateExecute([] {
		if (gAbout) {
			ImGui::OpenPopup("About");
			gAbout = false;
		}
		if (ImGui::BeginPopupModal("About")) {
			ImGui::Text("ImCan: A modern CAN workstation");
			ImGui::Separator();
			ImGui::Text("Save location: %s", glass::GetStorageDir().c_str());
			ImGui::Text("%.3f ms/frame (%.1f FPS)",
									1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	});

	gui::Initialize("ImCan", 1024, 768, ImGuiConfigFlags_DockingEnable);
	imcan::DbcGui::GlobalInit();
	gui::Main();

	glass::DestroyContext();
	gui::DestroyContext();

	return 0;
}
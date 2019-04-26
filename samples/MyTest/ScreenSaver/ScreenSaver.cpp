// ScreenSaver.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ScreenSaver.h"

#include "MainWindow.h"

#include <shellapi.h>
#pragma comment(lib,"Shell32.lib")

class MainThread : public nbase::FrameworkThread
{
public:
	MainThread() : nbase::FrameworkThread("MainThread") {}
	virtual ~MainThread() {}

private:
	virtual void Init() override
	{
		auto pData = this->GetTlsData();
		int nData = (int)pData->custom_data;

		nbase::ThreadManager::RegisterThread(kThreadUI);
		std::wstring theme_dir = QPath::GetAppPath();
		ui::GlobalManager::Startup(theme_dir + L"resources\\", ui::CreateControlCallback(), false);

		MainWindow* window = new MainWindow();
		window->Create(NULL, MainWindow::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
		window->CenterWindow();
		window->ShowWindow();
	}


	virtual void Cleanup() override
	{
		ui::GlobalManager::Shutdown();
		SetThreadWasQuitProperly(true);
		nbase::ThreadManager::UnregisterThread();
	}
};



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

	nbase::ThreadManager::RegisterThread(kThreadUI);
	std::wstring theme_dir = QPath::GetAppPath();
	ui::GlobalManager::Startup(theme_dir + L"resources\\", ui::CreateControlCallback(), false);

	int nArgc = 0;
	LPWSTR* szArgList = CommandLineToArgvW(lpCmdLine, &nArgc);
	if (szArgList)
	{
		if (nArgc == 2)
		{
			wstring wstr = szArgList[0];
			if (wstr.find_first_of('p') != 0)
			{
				wstring wsHwnd = szArgList[1];
				HWND hWnd = (HWND)(_wtoi(wsHwnd.c_str()));

				MainWindow* window = new MainWindow();
				window->Create(hWnd, MainWindow::kClassName.c_str(),WS_CHILD, 0);
				window->CenterWindow();
				window->ShowWindow();

				ui::GlobalManager::MessageLoop();

				if (window)
				{
					delete window;
					window = nullptr;
				}
			}
		}
	}

	//::MessageBox(NULL, lpCmdLine, L"Command", MB_OK);

	

	ui::GlobalManager::Shutdown();
	nbase::ThreadManager::UnregisterThread();
	/*MainThread thread;
	thread.RunOnCurrentThreadWithLoop(nbase::MessageLoop::kUIMessageLoop);*/
	return 0;
}




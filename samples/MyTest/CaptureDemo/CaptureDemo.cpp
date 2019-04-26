// CaptureDemo.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "CaptureDemo.h"

#include "MainWindow.h"

class MainThread : public nbase::FrameworkThread
{
public:
	MainThread() : nbase::FrameworkThread("MainThread") {}
	virtual ~MainThread() {}

private:
	virtual void Init() override
	{
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

	// 创建主线程
	MainThread thread;
	// 执行主线程循环
	thread.RunOnCurrentThreadWithLoop(nbase::MessageLoop::kUIMessageLoop);
	
	return 0;
}
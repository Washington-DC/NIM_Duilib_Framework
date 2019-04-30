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

#ifdef _DEBUG
		ui::GlobalManager::Startup(theme_dir + L"resources\\", ui::CreateControlCallback(), false);
#else
		// Release 模式下使用资源中的压缩包作为资源
		// 资源被导入到资源列表分类为 THEME，资源名称为 IDR_THEME
		// 如果资源使用的是本地的 zip 文件而非资源中的 zip 压缩包
		// 可以使用 OpenResZip 另一个重载函数打开本地的资源压缩包
		ui::GlobalManager::OpenResZip(MAKEINTRESOURCE(IDR_THEME), L"THEME", "");
		//// ui::GlobalManager::OpenResZip(L"resources.zip", "");
		ui::GlobalManager::Startup(L"resources\\", ui::CreateControlCallback(), false);
#endif
		auto vecDevices = TUtil::Win32::GetAllDisplayInfo();
		for (size_t i = 0; i < vecDevices.size(); i++)
		{
			MainWindow* window = new MainWindow();
			window->SetShadowCorner(ui::UiRect(0,0,0,0));
			window->SetDisplayRect(i,false,vecDevices[i]);
			HWND hWnd = window->Create(NULL, MainWindow::kClassName.c_str(), 
				WS_POPUPWINDOW/*WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX*/,
				WS_EX_TOOLWINDOW|WS_EX_TOPMOST,true,ui::UiRect(vecDevices[i]));
			window->CenterWindow();
			window->ShowWindow();
		}
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
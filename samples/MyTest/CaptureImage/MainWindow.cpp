#include "stdafx.h"
#include "MainWindow.h"
#include <shlwapi.h>

const std::wstring MainWindow::kClassName = L"MainWindow";

MainWindow::MainWindow()
{
}


MainWindow::~MainWindow()
{
}

std::wstring MainWindow::GetSkinFolder()
{
	return L"capture";
}

std::wstring MainWindow::GetSkinFile()
{
	return L"MainWindow.xml";
}

std::wstring MainWindow::GetWindowClassName() const
{
	return kClassName;
}

void MainWindow::InitWindow()
{
	//ui::ImageInfo sImageInfo;
	{
		//TODO  此处应该直接保存到内存中，而不是保存到本地,待完善
		wstring strImageName = L"Screen_" + std::to_wstring(_nScreenNum) + L".bmp";
		wstring strImagePath = ui::GlobalManager::GetCurrentPath() + L"\\" + strImageName;
		strImagePath = ui::StringHelper::ReparsePath(strImagePath);

#ifndef _DEBUG
		if (ui::GlobalManager::IsUseZip())
		{
			strImagePath = ui::GlobalManager::GetZipFilePath(strImagePath);
		}
#endif
		{
			HBITMAP hBmp = TUtil::Win32::CreateDisplayBitmap(_rect);

			ui::ImageInfo sImageInfo;
			auto pBmp = std::unique_ptr<Gdiplus::Bitmap>(TUtil::Win32::CreateBitmapFromHBITMAP(hBmp));
			auto pImageInfo = ui::ImageInfo::LoadImageByBitmap(pBmp, strImagePath);

			pInfo.reset(pImageInfo.release(),ui::GlobalManager::OnImageInfoDestroy);

			ui::GlobalManager::AddImageCached(pInfo);
		}
		
		ui::Box* box = dynamic_cast<ui::Box*>(FindControl(L"capture_main_box"));
		if (box)
		{
			box->SetBkImage(strImagePath);
		}

#ifndef _DEBUG
		box = dynamic_cast<ui::Box*>(FindControl(L"capture_caption"));
		if (box)
		{
			box->SetFixedHeight(0);
		}
#endif

		SetWindowPos(m_hWnd,HWND_TOPMOST,_rect.left,_rect.top,_rect.right - _rect.left,_rect.bottom - _rect.top, SWP_SHOWWINDOW);
	}
}

LRESULT MainWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

LRESULT MainWindow::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	bHandled = FALSE;

	if (wParam == VK_ESCAPE)
	{
		PostQuitMessage(0L);
	}
	return __super::OnKeyDown(uMsg, wParam, lParam, bHandled);
}

void MainWindow::SetDisplayRect(int n, bool bPrimary, RECT rect)
{
	_rect = rect;
	_bPrimary = bPrimary;
	_nScreenNum = n;
}


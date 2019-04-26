#include "stdafx.h"
#include "MainWindow.h"


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
}

LRESULT MainWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

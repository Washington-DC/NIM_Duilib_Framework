#include "stdafx.h"
#include "Util.h"

HBITMAP CreateDesktopBitmap(HWND hWnd)
{
	HWND hWndDesktop = ::GetDesktopWindow();
	RECT rect;
	GetWindowRect(hWndDesktop, &rect);
	rect.right -= rect.left;
	rect.bottom -= rect.top;
	rect.left = rect.top = 0;

	HDC hDcDesktop = GetDC(hWndDesktop);
	HDC hDcMem = CreateCompatibleDC(hDcDesktop);
	HBITMAP hBmp = CreateCompatibleBitmap(hDcDesktop, rect.right, rect.bottom);
	HGDIOBJ hOld = SelectObject(hDcMem, hBmp);
	::BitBlt(hDcMem, 0, 0, rect.right, rect.bottom, hDcDesktop, 0, 0, SRCCOPY | CAPTUREBLT);
	::ReleaseDC(hWndDesktop, hDcDesktop);
	::DeleteDC(hDcMem);
	return hBmp;
}

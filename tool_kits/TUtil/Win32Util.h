#pragma once

#include <vector>
#include <iostream>
#include <tchar.h>
#include <string>
#include <windows.h>
#include <comdef.h>  //http://blog.sina.com.cn/s/blog_b69140010102vui6.html
#include <GdiPlus.h>
#pragma comment(lib, "gdiplus.lib") 
using namespace Gdiplus;


namespace TUtil
{
	namespace Win32
	{
		HBITMAP CreateDesktopBitmap();
		HBITMAP CreateDisplayBitmap(RECT rect);
		std::vector<RECT> GetAllDisplayInfo();
		//vector<MONITORINFO> GetAllMonitorInfo();

		Bitmap* CreateBitmapFromHBITMAP(HBITMAP hBitmap);

		BOOL SaveBitmap(HBITMAP hBitmap, std::wstring strFileName);
	}
}



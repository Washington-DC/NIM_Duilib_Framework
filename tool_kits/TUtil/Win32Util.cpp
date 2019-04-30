#include "stdafx.h"
#include "Win32Util.h"

namespace TUtil
{
	namespace Win32
	{
		HBITMAP CreateDesktopBitmap()
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
			SelectObject(hDcMem, hBmp);
			::BitBlt(hDcMem, 0, 0, rect.right, rect.bottom, hDcDesktop, 0, 0, SRCCOPY | CAPTUREBLT);
			::ReleaseDC(hWndDesktop, hDcDesktop);
			::DeleteDC(hDcMem);
			return hBmp;
		}


		HBITMAP CreateDisplayBitmap(RECT rect)
		{
			HDC hDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);

			HDC hDcMem = CreateCompatibleDC(hDC);
			HBITMAP hBmp = CreateCompatibleBitmap(hDC, rect.right - rect.left, rect.bottom - rect.top);
			SelectObject(hDcMem, hBmp);
			::BitBlt(hDcMem, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hDC, rect.left, rect.top, SRCCOPY | CAPTUREBLT);
			::DeleteDC(hDcMem);
			::DeleteDC(hDC);
			return hBmp;
		}

		std::vector<RECT> GetAllDisplayInfo()
		{
			std::vector<RECT> vecMonitorInfo;
			int i = 0;
			DWORD           DispNum = 0;
			DISPLAY_DEVICE  DisplayDevice;
			DEVMODE   defaultMode;

			// initialize DisplayDevice
			ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
			DisplayDevice.cb = sizeof(DisplayDevice);

			// get all display devices
			while (EnumDisplayDevices(NULL, i, &DisplayDevice, 0))
			{
				ZeroMemory(&defaultMode, sizeof(DEVMODE));
				defaultMode.dmSize = sizeof(DEVMODE);
				if (!EnumDisplaySettings((LPCTSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &defaultMode))
					OutputDebugString(L"Store default failed\n");

				OutputDebugString(DisplayDevice.DeviceName);
				if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
				{
					RECT rc;
					rc.left = defaultMode.dmPosition.x;
					rc.top = defaultMode.dmPosition.y;
					rc.right = rc.left + defaultMode.dmPelsWidth;
					rc.bottom = rc.top + defaultMode.dmPelsHeight;
					vecMonitorInfo.push_back(rc);

					DispNum++;

					OutputDebugString((L"---------" + std::to_wstring(DispNum)).c_str());
				}
				// Reinit DisplayDevice just to be extra clean

				ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
				DisplayDevice.cb = sizeof(DisplayDevice);
				i++;
			} // end while for all display devices

			return vecMonitorInfo;
		}

		Bitmap* CreateBitmapFromHBITMAP(HBITMAP hBitmap)
		{
			BITMAP bmp = { 0 };
			if (0 == GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bmp))
			{
				return FALSE;
			}

			BYTE *piexlsSrc = NULL;
			LONG cbSize = bmp.bmWidthBytes * bmp.bmHeight;
			piexlsSrc = new BYTE[cbSize];

			BITMAPINFO bmpInfo = { 0 };
			bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmpInfo.bmiHeader.biWidth = bmp.bmWidth;
			bmpInfo.bmiHeader.biHeight = bmp.bmHeight;
			bmpInfo.bmiHeader.biPlanes = bmp.bmPlanes;
			bmpInfo.bmiHeader.biBitCount = bmp.bmBitsPixel;
			bmpInfo.bmiHeader.biCompression = BI_RGB;

			HDC hdcScreen = CreateDC(L"DISPLAY", NULL, NULL, NULL);
			LONG cbCopied = GetDIBits(hdcScreen, hBitmap, 0,
				bmp.bmHeight, piexlsSrc, &bmpInfo, DIB_RGB_COLORS);
			DeleteDC(hdcScreen);
			if (0 == cbCopied)
			{
				delete[] piexlsSrc;
				return FALSE;
			}

			Bitmap *pBitmap = new Bitmap(bmp.bmWidth, bmp.bmHeight, PixelFormat32bppPARGB);

			BitmapData bitmapData;
			Gdiplus::Rect rect(0, 0, bmp.bmWidth, bmp.bmHeight);
			if (Gdiplus::Ok != pBitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppPARGB, &bitmapData))
			{
				if (NULL != pBitmap) {
					delete (pBitmap);
					(pBitmap) = NULL;
				}
				return NULL;
			}

			BYTE *pixelsDest = (BYTE*)bitmapData.Scan0;
			int nLinesize = bmp.bmWidth * sizeof(UINT);
			int nHeight = bmp.bmHeight;


			for (int y = 0; y < nHeight; y++)
			{
				memcpy_s((pixelsDest + y * nLinesize), nLinesize, (piexlsSrc + (nHeight - y - 1) * nLinesize), nLinesize);
			}

			if (Gdiplus::Ok != pBitmap->UnlockBits(&bitmapData))
			{
				delete pBitmap;
			}

			delete[] piexlsSrc;
			return pBitmap;
		}


		BOOL SaveBitmap(HBITMAP  hBitmap, std::wstring strFileName)
		{
			HDC     hDC;
			//当前分辨率下每象素所占字节数         
			int     iBits;
			//位图中每象素所占字节数         
			WORD     wBitCount;
			//定义调色板大小，     位图中像素字节大小     ，位图文件大小     ，     写入文件字节数             
			DWORD     dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
			//位图属性结构             
			BITMAP     Bitmap;
			//位图文件头结构         
			BITMAPFILEHEADER     bmfHdr;
			//位图信息头结构             
			BITMAPINFOHEADER     bi;
			//指向位图信息头结构                 
			LPBITMAPINFOHEADER     lpbi;
			//定义文件，分配内存句柄，调色板句柄             
			HANDLE     fh, hDib, hPal, hOldPal = NULL;

			int aa = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 288 * 352 * 24 / 8;

			//计算位图文件每个像素所占字节数             
			hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
			iBits = GetDeviceCaps(hDC, BITSPIXEL)     *     GetDeviceCaps(hDC, PLANES);
			DeleteDC(hDC);
			if (iBits <= 1)
				wBitCount = 1;
			else  if (iBits <= 4)
				wBitCount = 4;
			else if (iBits <= 8)
				wBitCount = 8;
			else
				wBitCount = 24;

			GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
			bi.biSize = sizeof(BITMAPINFOHEADER);
			bi.biWidth = Bitmap.bmWidth;
			bi.biHeight = Bitmap.bmHeight;
			bi.biPlanes = 1;
			bi.biBitCount = wBitCount;
			bi.biCompression = BI_RGB;
			bi.biSizeImage = Bitmap.bmWidth * Bitmap.bmHeight * 3;
			bi.biXPelsPerMeter = 0;
			bi.biYPelsPerMeter = 0;
			bi.biClrImportant = 0;
			bi.biClrUsed = 0;

			dwBmBitsSize = ((Bitmap.bmWidth *wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

			//为位图内容分配内存             
			hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
			lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
			*lpbi = bi;

			//     处理调色板                 
			hPal = GetStockObject(DEFAULT_PALETTE);
			if (hPal)
			{
				hDC = ::GetDC(NULL);
				hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
				RealizePalette(hDC);
			}

			//     获取该调色板下新的像素值             
			GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
				(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
				(BITMAPINFO *)lpbi, DIB_RGB_COLORS);

			//恢复调色板                 
			if (hOldPal)
			{
				::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
				RealizePalette(hDC);
				::ReleaseDC(NULL, hDC);
			}

			//创建位图文件                 
			fh = CreateFile(strFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

			if (fh == INVALID_HANDLE_VALUE)
				return FALSE;

			//     设置位图文件头             
			bmfHdr.bfType = 0x4D42;     //     "BM"             
			dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
			bmfHdr.bfSize = dwDIBSize;
			bmfHdr.bfReserved1 = 0;
			bmfHdr.bfReserved2 = 0;
			bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
			//     写入位图文件头             
			WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
			//     写入位图文件其余内容             
			WriteFile(fh, (LPSTR)lpbi, dwDIBSize - 14, &dwWritten, NULL);
			//清除                 
			GlobalUnlock(hDib);
			GlobalFree(hDib);
			CloseHandle(fh);

			return     TRUE;
		}

		/*
		class EnumAllMonitor
		{
		public:
			void BeginEnum()
			{
				_vecInfo.swap(decltype(_vecInfo)());
				FUNC callback = (FUNC)&EnumAllMonitor::MyMonitorEnumProc;
				EnumDisplayMonitors(NULL, NULL, callback, 0);
			}

			BOOL WINAPI MyMonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
			{
				static BOOL first = TRUE;   //标志
				MONITORINFO monitorinfo;
				monitorinfo.cbSize = sizeof(MONITORINFO);
				GetMonitorInfo(hMonitor, &monitorinfo);

				//若检测到主屏
				if (monitorinfo.dwFlags == MONITORINFOF_PRIMARY)
				{
					if (first)  //第一次检测到主屏
					{
						return TRUE;
					}
					else //第二次检测到主屏,说明所有的监视器都已经检测了一遍，故可以停止检测了
					{
						first = TRUE;    //标志复位
						return FALSE;    //结束检测
					}
				}
				_vecInfo.push_back(monitorinfo);
				return TRUE;
			}

			vector<MONITORINFO> GetAllMonitorInfo()
			{
				return _vecInfo;
			}

			vector<MONITORINFO> _vecInfo;
		};

		vector<MONITORINFO> GetAllMonitorInfo()
		{
			EnumAllMonitor::BeginEnum();
			return EnumAllMonitor::GetAllMonitorInfo();
		}*/

	}
}
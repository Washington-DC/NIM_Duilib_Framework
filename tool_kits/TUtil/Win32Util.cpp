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
			//��ǰ�ֱ�����ÿ������ռ�ֽ���         
			int     iBits;
			//λͼ��ÿ������ռ�ֽ���         
			WORD     wBitCount;
			//�����ɫ���С��     λͼ�������ֽڴ�С     ��λͼ�ļ���С     ��     д���ļ��ֽ���             
			DWORD     dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
			//λͼ���Խṹ             
			BITMAP     Bitmap;
			//λͼ�ļ�ͷ�ṹ         
			BITMAPFILEHEADER     bmfHdr;
			//λͼ��Ϣͷ�ṹ             
			BITMAPINFOHEADER     bi;
			//ָ��λͼ��Ϣͷ�ṹ                 
			LPBITMAPINFOHEADER     lpbi;
			//�����ļ��������ڴ�������ɫ����             
			HANDLE     fh, hDib, hPal, hOldPal = NULL;

			int aa = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 288 * 352 * 24 / 8;

			//����λͼ�ļ�ÿ��������ռ�ֽ���             
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

			//Ϊλͼ���ݷ����ڴ�             
			hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
			lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
			*lpbi = bi;

			//     �����ɫ��                 
			hPal = GetStockObject(DEFAULT_PALETTE);
			if (hPal)
			{
				hDC = ::GetDC(NULL);
				hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
				RealizePalette(hDC);
			}

			//     ��ȡ�õ�ɫ�����µ�����ֵ             
			GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
				(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
				(BITMAPINFO *)lpbi, DIB_RGB_COLORS);

			//�ָ���ɫ��                 
			if (hOldPal)
			{
				::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
				RealizePalette(hDC);
				::ReleaseDC(NULL, hDC);
			}

			//����λͼ�ļ�                 
			fh = CreateFile(strFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

			if (fh == INVALID_HANDLE_VALUE)
				return FALSE;

			//     ����λͼ�ļ�ͷ             
			bmfHdr.bfType = 0x4D42;     //     "BM"             
			dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
			bmfHdr.bfSize = dwDIBSize;
			bmfHdr.bfReserved1 = 0;
			bmfHdr.bfReserved2 = 0;
			bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
			//     д��λͼ�ļ�ͷ             
			WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
			//     д��λͼ�ļ���������             
			WriteFile(fh, (LPSTR)lpbi, dwDIBSize - 14, &dwWritten, NULL);
			//���                 
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
				static BOOL first = TRUE;   //��־
				MONITORINFO monitorinfo;
				monitorinfo.cbSize = sizeof(MONITORINFO);
				GetMonitorInfo(hMonitor, &monitorinfo);

				//����⵽����
				if (monitorinfo.dwFlags == MONITORINFOF_PRIMARY)
				{
					if (first)  //��һ�μ�⵽����
					{
						return TRUE;
					}
					else //�ڶ��μ�⵽����,˵�����еļ��������Ѿ������һ�飬�ʿ���ֹͣ�����
					{
						first = TRUE;    //��־��λ
						return FALSE;    //�������
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
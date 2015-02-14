/*
* The MIT License (MIT)
*
* This file is part of pomfit project.
* pomfit - Not that cute image & file uploader for <http://pomf.se/>
* Copyright (c) 2014-2015 Senketsu <senketsu.dev@gmail.com>
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include "resource.h"
#ifdef _WIN32
    #define _WIN32_IE 0x0500
    #define __WIN32_WINNT 0x0501
    #define _WIN32_WINNT 0x0500
    #define WIN32_LEAN_AND_MEAN
	#include <windows.h>
    #include <wingdi.h>
	#include <ShellAPI.h>
    #include <shlobj.h>
    #include <gdiplus.h>
    #include <gdiplus/gdiplusimagecodec.h>
	#include <gdk/gdkwin32.h>

    #include <string>
    #include <iostream>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "screenshot.h"
#include "uploader.h"
#include "log.h"

#ifdef _WIN32
extern HWND hWndSel;
extern HWND hWndActWnd;
extern long SmallestX;
extern bool CaptureWindow;
extern bool DrawRectangle;
extern bool UploadSS;
#endif
extern bool bUP_Keep_SS;
extern char DirHome[64];
extern char DirScrn[512];
extern char DirUpScrn[512];


#ifdef _WIN32
std::wstring towstring(const char *src)
{
    std::wstring output;
    int src_len = strlen(src);
    if (src_len > 0)
    {
        int out_len = MultiByteToWideChar(CP_ACP, 0, src, src_len, NULL, 0);
        if (out_len > 0)
        {
            output.resize(out_len);
            MultiByteToWideChar(CP_ACP, 0, src, src_len, &output[0], out_len);
        }
    }
    return output;
}

void screenshot_win32_save_bitmap (HDC MyDC , HBITMAP hBitmap , bool Upload)
{
    char FilePath[768];
    time_t now = time(NULL);
	char FileName[128];
	char TimeString[80];
	struct tm  tstruct;
	tstruct = *localtime(&now);
	strftime(TimeString, sizeof(TimeString), "%Y-%m-%d-%H_%M_%S", &tstruct);
	sprintf(FileName , "%s_%s" , TimeString , Upload == true ? "pomfup" : "ss");

    using namespace Gdiplus;
    GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	{

        if(Upload)
            sprintf(FilePath, "%s\\%s.png",DirUpScrn,FileName);
        else
            sprintf(FilePath, "%s\\%s.png",DirScrn,FileName);

        Gdiplus::Bitmap PNGbitmap(hBitmap, NULL);
        CLSID clsid;
        screenshot_win32_get_encoder(L"image/png", &clsid);
        PNGbitmap.Save(towstring(FilePath).c_str(), &clsid,NULL);
	}
    GdiplusShutdown(gdiplusToken);

    if(Upload)  {
        void *pFilePath[1];
        pFilePath[0] = FilePath;
        uploader_curl_file(pFilePath, 1);
		if(!bUP_Keep_SS && Upload)
			if(remove(FilePath) != 0)
				log_error(true,"***Error: Deleting file %s failed",FilePath);
        pFilePath[0] = NULL;
    }

}

void screenshot_win32_cap_act_win(bool Upload) {

    HDC hdc = GetWindowDC(hWndActWnd);
    if (hdc) {
        HDC hdcMem = CreateCompatibleDC(hdc);
        if (hdcMem) {
            RECT rc;
            GetWindowRect(hWndActWnd, &rc);
            int Width = rc.right-rc.left;
            int Height = rc.bottom-rc.top;
            bool IsMaximized = false;

            if(Width > GetSystemMetrics(SM_CXVIRTUALSCREEN))
                Width = GetSystemMetrics(SM_CXVIRTUALSCREEN);

            if(GetSystemMetrics(SM_CMONITORS) < 2 ) {
                RECT WorkArea;
                SystemParametersInfo(SPI_GETWORKAREA , 0, &WorkArea,0);
                int WorkAreaH = WorkArea.bottom-WorkArea.top;
                if(Height > WorkAreaH) {
                    Height = WorkAreaH;
                    IsMaximized = true;
                }
            } else {
                if(Height > GetSystemMetrics(SM_CYVIRTUALSCREEN))
                    Height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
            }

            HBITMAP hbitmap = CreateCompatibleBitmap(hdc, Width, Height);
            if (hbitmap) {
                SelectObject(hdcMem, hbitmap);

                if(IsMaximized)
                    BitBlt(hdcMem,0,0,Width,Height,hdc,8,8,SRCCOPY);
                else
                    BitBlt(hdcMem,0,0,Width,Height,hdc,0,0,SRCCOPY);

                screenshot_win32_save_bitmap(hdcMem, hbitmap, Upload);


                DeleteObject(hbitmap);
            }
            DeleteObject(hdcMem);
        }
        ReleaseDC(hWndActWnd, hdc);
    }
    DeleteObject(hdc);
}

void screenshot_win32_cap_full (bool Upload) {

    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
    HBITMAP hCaptureBitmap;
    int nScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int nScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    HWND hDesktopWnd = GetDesktopWindow();
    HDC hDesktopDC = GetDC(hDesktopWnd);
    HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
    hCaptureBitmap =CreateCompatibleBitmap(hDesktopDC,
                            nScreenWidth, nScreenHeight);
    SelectObject(hCaptureDC,hCaptureBitmap);
    BitBlt(hCaptureDC, 0,0,nScreenWidth,nScreenHeight,
           hDesktopDC, SmallestX,0,SRCCOPY|CAPTUREBLT);
    screenshot_win32_save_bitmap(NULL, hCaptureBitmap , Upload);

    ReleaseDC(hDesktopWnd,hDesktopDC);
    DeleteDC(hCaptureDC);
    DeleteObject(hCaptureBitmap);
}

void screenshot_win32_draw_selection(HWND hWnd, const RECT* pRect) {

    HDC hDC;
    HPEN hOldPen;
    HBRUSH hOldBrush;

    hDC = GetDC(hWnd);

    hOldPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 1, RGB(128, 128, 128)));
    hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

    SetROP2(hDC, R2_XORPEN);
    Rectangle(hDC, pRect->left, pRect->top, pRect->right, pRect->bottom);

    SelectObject(hDC, hOldBrush);
    DeleteObject(SelectObject(hDC, hOldPen));
    DeleteObject(SelectObject(hDC, hOldBrush));  // ?
    ReleaseDC(hWnd, hDC);
}

int screenshot_win32_get_encoder(const WCHAR* format, CLSID* pClsid) {

	using namespace Gdiplus;
	UINT  num = 0;
	UINT  size = 0;

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j) {
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 ) {
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}

	free(pImageCodecInfo);
	return 0;
}

BOOL CALLBACK MonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData){

    if(lprcMonitor->left < SmallestX)
        SmallestX = lprcMonitor->left;
    return true;
}

#endif


void screenshot_glob_take_ss (bool Upload , int mode) {
#ifdef __linux__
	bool UploadMode = false;
	if(mode > 0 && mode < 4)
		UploadMode = true;

	char Command[128]= "";
	char FileName[64];
	char FilePath[128];
	void *pFilePath[1];
	char NewFilePath[256];
	time_t now = time(NULL);
	char TimeString[80];
	struct tm  tstruct;
	tstruct = *localtime(&now);
	strftime(TimeString, sizeof(TimeString), "%Y-%m-%d-%H_%M_%S", &tstruct);
	sprintf(FileName , "%s_%s.png" , TimeString , UploadMode == true ? "pomfup" : "ss");

	if(mode == 1)
		sprintf(Command, "sleep 0.3 && scrot -s '%s'", FileName);
	else if(mode == 2)
		sprintf(Command, "sleep 0.3 && scrot -u '%s'", FileName);
	else if(mode == 3)
		sprintf(Command, "sleep 0.3 && scrot -m '%s'", FileName);
	else if(mode == 4)
		sprintf(Command, "sleep 0.3 && scrot -s '%s'", FileName);
	else if(mode == 5)
		sprintf(Command, "sleep 0.3 && scrot -u '%s'", FileName);
	else if(mode == 6)
		sprintf(Command, "sleep 0.3 && scrot -m '%s'", FileName);
	else
		return;

	system(Command);

	sprintf(FilePath,"%s/%s",DirHome,FileName);
	if(bUP_Keep_SS && UploadMode) {
		sprintf(NewFilePath,"%s/%s", UploadMode == true ? DirUpScrn : DirScrn, FileName);
		rename(FilePath,NewFilePath);
		pFilePath[0] = NewFilePath;
		uploader_curl_file(pFilePath, 1);
	} else if(!bUP_Keep_SS && UploadMode) {

		pFilePath[0] = FilePath;
		uploader_curl_file(pFilePath, 1);
		remove(FilePath);
	} else {
		sprintf(NewFilePath,"%s/%s", UploadMode == true ? DirUpScrn : DirScrn, FileName);
		rename(FilePath,NewFilePath);
	}

	pFilePath[0] = NULL;
#elif _WIN32
    Sleep(400);                 // To prevent context menu on screenshots
    if(mode == 0) {           // Mode 0 - Capture Whole Screen
        screenshot_win32_cap_full(Upload);
    }
    else if (mode == 1) {   // Mode 1 - Capture Window
        CaptureWindow = true;
        SetWindowPos(hWndSel, 0, 0, 0, 0, 0, SWP_NOSIZE);
        ShowWindow(hWndSel,SW_SHOW);
        UploadSS = Upload;

    }
    else if (mode == 2) {    // Mode 2 - Capture Rectangle
        SetWindowPos(hWndSel, 0, 0, 0, 0, 0, SWP_NOSIZE);
        ShowWindow(hWndSel,SW_SHOW);
        DrawRectangle = true;
        UploadSS = Upload;
    }
#endif

}

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

#ifndef POMFIT_SCREENSHOT_H
	#define POMFIT_SCREENSHOT_H

#ifdef __cplusplus
extern "C"
{
#endif
    void screenshot_glob_take_ss (bool Upload , int mode);
    #ifdef _WIN32
    void screenshot_win32_cap_full (bool Upload);
    void screenshot_win32_cap_act_win(bool Upload);
    void screenshot_win32_save_bitmap (HDC MyDC , HBITMAP hBitmap , bool Upload);
    void screenshot_win32_draw_selection(HWND hWnd, const RECT* pRect);
    BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor,LPARAM dwData);
    #endif
#ifdef __cplusplus
}
#endif
    #ifdef _WIN32
    int screenshot_win32_get_encoder(const WCHAR* format, CLSID* pClsid);
    #endif
#endif

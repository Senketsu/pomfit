/*
 * This file is part of pomfit project.
 * pomfit - simple cross platform image & file uploader for <http://pomf.se/>
 * Copyright (C) 2014  Senketsu <senketsu.dev@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "resource.h"
#include "header.h"
#include "pomfit.h"

#include <string>
#include <iostream>

#define HOTKEY_COUNT 8

int PrimWidth , PrimHeight;
long SmallestX;
POINT CurMPos;
MONITORINFO  CurMonInfo;
LPOSVERSIONINFO lpVersionInfo;

char HomeDir[64] = "C:\\";
bool CaptureWindow = false;
bool DrawRectangle = false;
bool IsLoged = false;
bool UploadSS;

POINT Begpt , Endpt , Temppt;
HDC DeskDC, MemDC;
NOTIFYICONDATA  nid;

HWND hStatus;
HWND hWnd;
HWND hWndSel;
HWND hWndActWnd;
HWND hWndDialogButton;
HWND hWndEntry1;
HWND hWndEntry2;
HWND hWndLoginBut;
HWND hWndLoginInfo;
DWORD RLThreadID;
HANDLE RLHandle;

HWND KeepUpSSButt;
HWND SetHotSel1[HOTKEY_COUNT];
HWND SetHotSel2[HOTKEY_COUNT];
HWND SetHotEntry[HOTKEY_COUNT];
char bKey[HOTKEY_COUNT];
int iMod1[HOTKEY_COUNT];
int iMod2[HOTKEY_COUNT];


#ifdef __cplusplus
extern "C"
{
#endif
void LoadSettings (void);
void LoadDefaults (void);
void SaveSettings (void);
void ChooseFiles (void);
void GetLoginData (void);
void UserLogout (void);
void CaptureFullScreen (bool Upload);
void CaptureActiveWindow(bool Upload);
void SaveBitmap (HDC MyDC , HBITMAP hBitmap , bool Upload);
static void DrawSelectionRect(HWND hWnd, const RECT* pRect);
static LRESULT CALLBACK SelWndProc( HWND hWnd,  UINT uMsg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor,LPARAM dwData);
#ifdef __cplusplus
}
#endif
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

BOOL ShowPopupMenu( HWND hWnd, POINT *curpos, int wDefaultItem ) {

    char MenuBuffer[512];
	HMENU hPop = CreatePopupMenu();

    if(LoginEmail[0] != '\0')
        InsertMenu( hPop, 0, MF_BYPOSITION | MF_STRING, ID_POMFACC, LoginEmail );
    else
        InsertMenu( hPop, 0, MF_BYPOSITION | MF_STRING, ID_POMFACC,  "You are not logged in.." );
	InsertMenu( hPop, 1, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

    sprintf(MenuBuffer,"Upload File(s)\t| %s %s %c",
            iMod1[6] == 0 ? "Ctrl +" :  iMod1[6] == 1 ? "Shift +" :  iMod1[6] == 2 ? "Alt +" : "",
            iMod2[6] == 0 ? "Ctrl +" :  iMod2[6] == 1 ? "Shift +" :  iMod2[6] == 2 ? "Alt +" : "",bKey[6]);
    InsertMenu( hPop, 2, MF_BYPOSITION | MF_STRING, ID_FILEUP, MenuBuffer);
    InsertMenu( hPop, 3, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
    sprintf(MenuBuffer,"Up Area SS\t| %s %s %c",
            iMod1[5] == 0 ? "Ctrl +" :  iMod1[5] == 1 ? "Shift +" :  iMod1[5] == 2 ? "Alt +" : "",
            iMod2[5] == 0 ? "Ctrl +" :  iMod2[5] == 1 ? "Shift +" :  iMod2[5] == 2 ? "Alt +" : "",bKey[5]);
    InsertMenu( hPop, 4, MF_BYPOSITION | MF_STRING, ID_AREA_SS_UP, MenuBuffer);
    sprintf(MenuBuffer,"Up Active Window SS\t| %s %s %c",
            iMod1[4] == 0 ? "Ctrl +" :  iMod1[4] == 1 ? "Shift +" :  iMod1[4] == 2 ? "Alt +" : "",
            iMod2[4] == 0 ? "Ctrl +" :  iMod2[4] == 1 ? "Shift +" :  iMod2[4] == 2 ? "Alt +" : "",bKey[4]);
    InsertMenu( hPop, 5, MF_BYPOSITION | MF_STRING, ID_WIN_SS_UP, MenuBuffer);
    sprintf(MenuBuffer,"Up Full SS\t| %s %s %c",
            iMod1[3] == 0 ? "Ctrl +" :  iMod1[3] == 1 ? "Shift +" :  iMod1[3] == 2 ? "Alt +" : "",
            iMod2[3] == 0 ? "Ctrl +" :  iMod2[3] == 1 ? "Shift +" :  iMod2[3] == 2 ? "Alt +" : "",bKey[3]);
	InsertMenu( hPop, 6, MF_BYPOSITION | MF_STRING, ID_FULL_SS_UP, MenuBuffer);
	InsertMenu( hPop, 7, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
    sprintf(MenuBuffer,"Area / Window SS\t| %s %s %c",
            iMod1[2] == 0 ? "Ctrl +" :  iMod1[2] == 1 ? "Shift +" :  iMod1[2] == 2 ? "Alt +" : "",
            iMod2[2] == 0 ? "Ctrl +" :  iMod2[2] == 1 ? "Shift +" :  iMod2[2] == 2 ? "Alt +" : "",bKey[2]);
    InsertMenu( hPop, 8, MF_BYPOSITION | MF_STRING, ID_AREA_SS_CAP, MenuBuffer);
    sprintf(MenuBuffer,"Active Window SS\t| %s %s %c",
            iMod1[1] == 0 ? "Ctrl +" :  iMod1[1] == 1 ? "Shift +" :  iMod1[1] == 2 ? "Alt +" : "",
            iMod2[1] == 0 ? "Ctrl +" :  iMod2[1] == 1 ? "Shift +" :  iMod2[1] == 2 ? "Alt +" : "",bKey[1]);
    InsertMenu( hPop, 9, MF_BYPOSITION | MF_STRING, ID_WIN_SS_CAP, MenuBuffer);
    sprintf(MenuBuffer,"Full SS\t| %s %s %c",
            iMod1[0] == 0 ? "Ctrl +" :  iMod1[0] == 1 ? "Shift +" :  iMod1[0] == 2 ? "Alt +" : "",
            iMod2[0] == 0 ? "Ctrl +" :  iMod2[0] == 1 ? "Shift +" :  iMod2[0] == 2 ? "Alt +" : "",bKey[0]);
	InsertMenu( hPop, 10, MF_BYPOSITION | MF_STRING, ID_FULL_SS_CAP, MenuBuffer);

    InsertMenu( hPop, 11, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
    sprintf(MenuBuffer,"Open Link(s)\t| %s %s %c",
            iMod1[7] == 0 ? "Ctrl +" :  iMod1[7] == 1 ? "Shift +" :  iMod1[7] == 2 ? "Alt +" : "",
            iMod2[7] == 0 ? "Ctrl +" :  iMod2[7] == 1 ? "Shift +" :  iMod2[7] == 2 ? "Alt +" : "",bKey[7]);
	InsertMenu( hPop, 12, MF_BYPOSITION | MF_STRING, ID_OPEN, MenuBuffer);
	InsertMenu( hPop, 13, MF_BYPOSITION | MF_STRING, ID_SHOW_SETTINGS, "Settings" );
	InsertMenu( hPop, 14, MF_BYPOSITION | MF_STRING, ID_EXIT , "Exit" );

	SetMenuDefaultItem( hPop, ID_SHOW_SETTINGS, FALSE );
	SetFocus ( hWnd );
	SendMessage ( hWnd, WM_INITMENUPOPUP, (WPARAM)hPop, 0 );

	POINT pt;
	if (!curpos) {
		GetCursorPos( &pt );
		curpos = &pt;
	}

	WORD cmd = TrackPopupMenu( hPop,
                        TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY ,
                        curpos->x, curpos->y, 0, hWnd, NULL );

	SendMessage( hWnd, WM_COMMAND, cmd, 0 );

	DestroyMenu(hPop);

	return 0;
}

void RemoveTrayIcon( HWND hWnd, UINT uID ) {
	NOTIFYICONDATA  nid;
	nid.hWnd = hWnd;
	nid.uID  = uID;

	Shell_NotifyIcon( NIM_DELETE, &nid );
}

void AddTrayIcon( HWND hWnd, UINT uID, UINT uCallbackMsg, UINT uIcon ,HINSTANCE hInst) {

	nid.hWnd = hWnd;
	nid.uID = uID;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.uCallbackMessage = uCallbackMsg;
    nid.hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_MYICON ) );
	strcpy(nid.szTip, POMFIT_VERSION);

	Shell_NotifyIcon( NIM_ADD, &nid );
}

static LRESULT CALLBACK WndProc( HWND hWnd,  UINT uMsg, WPARAM wParam, LPARAM lParam ) {

	switch (uMsg) {

        case WM_CLOSE:
            ShowWindow(hWnd, 0);
            return 0;
        case WM_HOTKEY:
        {
            switch (LOWORD(wParam)) {
                case HOTKEY_FILEUP:
                    PostMessage( hWnd, WM_COMMAND, ID_FILEUP, 0 );
                    break;
                case HOTKEY_AREA_SS_UP:
                    PostMessage( hWnd, WM_COMMAND , ID_AREA_SS_UP, 0 );
                    break;
                case HOTKEY_WIN_SS_UP:
                    hWndActWnd = GetForegroundWindow();
                    CaptureActiveWindow(true);
                    break;
                case HOTKEY_FULL_SS_UP:
                    PostMessage( hWnd, WM_COMMAND , ID_FULL_SS_UP, 0 );
                    break;
                 case HOTKEY_AREA_SS_CAP:
                    PostMessage( hWnd, WM_COMMAND , ID_AREA_SS_CAP, 0 );
                    break;
                case HOTKEY_WIN_SS_CAP:
                    hWndActWnd = GetForegroundWindow();
                    CaptureActiveWindow(false);
                    break;
                case HOTKEY_FULL_SS_CAP:
                    PostMessage( hWnd, WM_COMMAND, ID_FULL_SS_CAP, 0 );
                    break;
                case HOTKEY_OPEN:
                    PostMessage( hWnd, WM_COMMAND, ID_OPEN, 0 );
                    break;
                }
        }
		return DefWindowProc( hWnd, uMsg, wParam, lParam );

        case WM_COMMAND:
        {
            switch (LOWORD(wParam)) {
            /* Buttons IDs */
            case IDC_KEEP_UP_CHECKBUTT:
                KeepUploadedSS = Button_GetCheck(KeepUpSSButt);
                break;
			case IDC_LOGIN_BUTTON:
			    if(IsLoged == false)
                    GetLoginData();
                else
                    UserLogout();
                break;
			case IDC_CHOOSE_BUTTON:
                ChooseFiles();
                break;
            case IDC_SAVE_BUTTON:
                SaveSettings();
                ShowWindow(hWnd, 0);
                break;
            /* Tray menu IDs */
            case ID_POMFACC:
                open_acc_page(NULL,NULL);
                break;
            case ID_FILEUP:
                ChooseFiles();
                break;
            case ID_AREA_SS_UP:
                pomfit_take_ss(true, 2);
                break;
            case ID_WIN_SS_UP:
                pomfit_take_ss(true, 1);
                break;
            case ID_FULL_SS_UP:
                pomfit_take_ss(true, 0);
                break;
            case ID_AREA_SS_CAP:
                pomfit_take_ss(false, 2);
                break;
            case ID_WIN_SS_CAP:
                pomfit_take_ss(false, 1);
                break;
            case ID_FULL_SS_CAP:
                pomfit_take_ss(false, 0);
                break;
            case ID_OPEN:
                open_all_links(NULL, NULL);
                break;
            case ID_SHOW_SETTINGS:
              ShowWindow(hWnd,1);
              break;
            case ID_EXIT:
              RemoveTrayIcon (hWnd, 1);
              if(IsLoged == true ) {
                pomfit_curl_logout();
              }
              PostQuitMessage(0);
              break;
		  }
		 }
		 /* Tray icon handling */
         case WM_APP:
         {
            switch (lParam) {
                case WM_LBUTTONUP:
                    break;
                case WM_RBUTTONUP:
                    ShowPopupMenu(hWnd, NULL, -1 );
                    PostMessage( hWnd, WM_APP + 1, 0, 0 );
                    break;
            }
		 }
		 break;
	}
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE prev, LPSTR cmdline, int show ) {

    TCHAR szFolderPath[MAX_PATH];
     if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szFolderPath)))
    {
        strcpy(ConfDir,szFolderPath);
        strcat(ConfDir,"\\Pomfit");
    }
    else
        sprintf(ConfDir,"C:\\Pomfit");

    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, 0, szFolderPath)))
    {
        sprintf(SsDir, "%s\\Screenshots",szFolderPath);
        sprintf(UploadedSsDir, "%s\\Uploaded",SsDir);
    }
    else {
        sprintf(SsDir, "%s\\Screenshots",ConfDir);
        sprintf(UploadedSsDir, "%s\\Uploaded",SsDir);
    }
    pomfit_mkdir(ConfDir);
    pomfit_mkdir(SsDir);
    pomfit_mkdir(UploadedSsDir);
    sprintf(CookieFile,"%s\\pomfit_phpsessid",ConfDir);

	HWND hPrev = NULL;
	if (( hPrev = FindWindow( THIS_CLASSNAME, TEXT("Pomf It !") ) ) ){
		MessageBox(NULL, TEXT("Pomfit is already running!"), TEXT("Pomf It !"), MB_OK );
		return 0;
	}

	WNDCLASSEX wclx;
	memset(&wclx, 0, sizeof(wclx));
	wclx.cbSize         = sizeof( wclx );
	wclx.style          = 0;
	wclx.lpfnWndProc    = &WndProc;
	wclx.cbClsExtra     = 0;
	wclx.cbWndExtra     = 0;
	wclx.hInstance      = hInst;
    wclx.hIcon          = LoadIcon( hInst, MAKEINTRESOURCE( IDI_MYICON ) );
	wclx.hCursor        = LoadCursor( NULL, IDC_ARROW );
	wclx.hbrBackground  = (HBRUSH)( COLOR_BTNFACE + 1 );
	wclx.lpszMenuName   = NULL;
	wclx.lpszClassName  = THIS_CLASSNAME;
	RegisterClassEx( &wclx );

	WNDCLASSEX selWindow;
	memset(&selWindow, 0, sizeof(selWindow));
	selWindow.cbSize         = sizeof( selWindow );
	selWindow.style          = 0;
	selWindow.lpfnWndProc    = &SelWndProc;
	selWindow.cbClsExtra     = 0;
	selWindow.cbWndExtra     = 0;
	selWindow.hInstance      = hInst;
    selWindow.hIcon          = NULL;
	selWindow.hCursor        = LoadCursor( NULL, IDC_ARROW );
	selWindow.hbrBackground  = (HBRUSH)0;
	selWindow.lpszMenuName   = NULL;
	selWindow.lpszClassName  = "SelectionClass";
	RegisterClassEx( &selWindow );

    hWndSel = CreateWindow( "SelectionClass", 0,
        (WS_BORDER | WS_EX_LAYERED | WS_EX_NOACTIVATE) ,
        0, 0, GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
        NULL, NULL, hInst, NULL );
	if ( !hWndSel ) {
		fprintf(stderr ,"Error: Can't create window!");
		MessageBox(NULL, TEXT("Error: Can't create window!"), TEXT("Pomf It !"), MB_OK );
		return 1;
	}
	SetWindowLong(hWndSel, GWL_STYLE, 0);
	SetWindowPos(hWndSel, 0, 0, 0, 0, 0, SWP_NOSIZE);
/* ************************************************************* */
    hWnd = CreateWindow( THIS_CLASSNAME, TEXT("'Pomf It !' Settings"),
	 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU ,
	  GetSystemMetrics(SM_CXSCREEN) - 400, GetSystemMetrics(SM_CYSCREEN) - 500, 320, 395, NULL, NULL, hInst, NULL );
	if ( !hWnd ) {
		fprintf(stderr,"Error: Can't create window!");
		MessageBox(NULL, TEXT("Error: Can't create window!"), TEXT("Pomf It !"), MB_OK );
		return 1;
	}

    HWND hWndLabel1 = CreateWindowEx(0,
            "STATIC", " Username:",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
             10,5,120,18,
             hWnd,NULL,hInst,NULL);

    HWND hWndLabel2 = CreateWindowEx(0,
            "STATIC", " Password:",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
             160,5,120,18,
             hWnd,NULL,hInst,NULL);

    hWndEntry1 = CreateWindowEx(0,"EDIT", NULL,
             WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL,
             10,23,140,18,
             hWnd,NULL,hInst,NULL);
    hWndEntry2 = CreateWindowEx(0,"EDIT", NULL,
             WS_VISIBLE | WS_CHILD | ES_LEFT | ES_PASSWORD | ES_AUTOHSCROLL,
             160,23,140,18,
             hWnd,NULL,hInst,NULL);

    hWndLoginInfo = CreateWindowEx(0,
            "STATIC", "You are not logged in..",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
             10,45,160,18,
             hWnd,NULL,hInst,NULL);

    hWndLoginBut = CreateWindowEx(0,
			"BUTTON", "Login",
			WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
			170, 45, 140, 24,
			hWnd, (HMENU)IDC_LOGIN_BUTTON, GetModuleHandle(NULL), NULL);

    HWND hWndHotLab = CreateWindowEx(0,
            "STATIC", "Hotkey Settings\t(Leave entry empty to unbind)",
             WS_VISIBLE | WS_CHILD | SS_LEFT,
             10,80,300,18,
             hWnd,NULL,hInst,NULL);

    HWND SetHotLab[HOTKEY_COUNT];
    SetHotLab[0] = CreateWindowEx(0,
            "STATIC", "Full SS",
             WS_VISIBLE | WS_CHILD | SS_LEFT,
             10,105,120,18,
             hWnd,NULL,hInst,NULL);
    SetHotLab[1] = CreateWindowEx(0,
            "STATIC", "Active Window SS",
             WS_VISIBLE | WS_CHILD | SS_LEFT,
             10,128,120,18,
             hWnd,NULL,hInst,NULL);
    SetHotLab[2] = CreateWindowEx(0,
            "STATIC", "Area / Window SS",
             WS_VISIBLE | WS_CHILD | SS_LEFT,
             10,153,120,18,
             hWnd,NULL,hInst,NULL);

    SetHotLab[3] = CreateWindowEx(0,
            "STATIC", "Upload Full SS",
             WS_VISIBLE | WS_CHILD | SS_LEFT,
             10,178,120,18,
             hWnd,NULL,hInst,NULL);
    SetHotLab[4] = CreateWindowEx(0,
            "STATIC", "Upload Active Window SS",
             WS_VISIBLE | WS_CHILD | SS_LEFT,
             10,203,120,18,
             hWnd,NULL,hInst,NULL);
    SetHotLab[5] = CreateWindowEx(0,
            "STATIC", "Upload Area / Window SS",
             WS_VISIBLE | WS_CHILD | SS_LEFT,
             10,228,120,18,
             hWnd,NULL,hInst,NULL);
    SetHotLab[6] = CreateWindowEx(0,
            "STATIC", "Upload File chooser",
             WS_VISIBLE | WS_CHILD | SS_LEFT,
             10,253,120,18,
             hWnd,NULL,hInst,NULL);
    SetHotLab[7] = CreateWindowEx(0,
            "STATIC", "Open Last Link(s)",
             WS_VISIBLE | WS_CHILD | SS_LEFT,
             10,278,120,18,
             hWnd,NULL,hInst,NULL);

    int j;
    int k = 25;
    for(j = 0 ; j < HOTKEY_COUNT ; ++j ) {
    SetHotSel1[j] = CreateWindow(WC_COMBOBOX, TEXT(""),
         CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
         140, 100+(j*k), 70, 18, hWnd, NULL, GetModuleHandle(NULL),
         NULL);
    SetHotSel2[j] = CreateWindow(WC_COMBOBOX, TEXT(""),
         CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
         215, 100+(j*k), 70, 18, hWnd, NULL, GetModuleHandle(NULL),
         NULL);
    SetHotEntry[j] = CreateWindowEx(0,"EDIT", NULL,
        WS_VISIBLE | WS_CHILD | ES_CENTER | ES_UPPERCASE,
        290,103+(j*k),15,18,
        hWnd,NULL,hInst,NULL);
    }
    KeepUpSSButt = CreateWindowEx(0,
			"BUTTON", "Keep uploaded screen shots",
			WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX,
			10, 310, 300, 24,
			hWnd, (HMENU)IDC_KEEP_UP_CHECKBUTT, GetModuleHandle(NULL), NULL);

    HWND SetSaveButton = CreateWindowEx(0,
			"BUTTON", "Save Settings",
			WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
			90, 335, 140, 24,
			hWnd, (HMENU)IDC_SAVE_BUTTON, GetModuleHandle(NULL), NULL);

    HFONT PomfFont = CreateFont(16, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, "Segoe UI");
    TCHAR Keys[4][10] =
    {
        TEXT("Control"), TEXT("Shift"), TEXT("Alt"), TEXT("(none)")
    };

    for (j = 0 ; j < HOTKEY_COUNT ; ++j) {
        for (k = 0; k < 4; ++k)
        {
            SendMessage(SetHotSel1[j],(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) Keys[k]);
            SendMessage(SetHotSel2[j],(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) Keys[k]);
        }
        SendMessage(SetHotSel1[j], WM_SETFONT, (WPARAM)PomfFont, TRUE);
        SendMessage(SetHotSel2[j], WM_SETFONT, (WPARAM)PomfFont, TRUE);
        SendMessage(SetHotEntry[j], WM_SETFONT, (WPARAM)PomfFont, TRUE);
        SendMessage(SetHotLab[j], WM_SETFONT, (WPARAM)PomfFont, TRUE);
    }
    SendMessage(hWndDialogButton, WM_SETFONT, (WPARAM)PomfFont, TRUE);
    SendMessage(hWndEntry1, WM_SETFONT, (WPARAM)PomfFont, TRUE);
    SendMessage(hWndLabel1, WM_SETFONT, (WPARAM)PomfFont, TRUE);
    SendMessage(hWndEntry2, WM_SETFONT, (WPARAM)PomfFont, TRUE);
    SendMessage(hWndLabel2, WM_SETFONT, (WPARAM)PomfFont, TRUE);
    SendMessage(hWndLoginBut, WM_SETFONT, (WPARAM)PomfFont, TRUE);
    SendMessage(hWndLoginInfo, WM_SETFONT, (WPARAM)PomfFont, TRUE);
    SendMessage(hWndHotLab, WM_SETFONT, (WPARAM)PomfFont, TRUE);
    SendMessage(SetSaveButton, WM_SETFONT, (WPARAM)PomfFont, TRUE);
    SendMessage(KeepUpSSButt, WM_SETFONT, (WPARAM)PomfFont, TRUE);

    LoadSettings();

    AddTrayIcon( hWnd, 1, WM_APP, 0 , GetModuleHandle (NULL));

    PrimWidth = GetSystemMetrics(SM_CXSCREEN);
    PrimWidth = GetSystemMetrics(SM_CYSCREEN);
    SmallestX = 0;

	MSG msg;
	while (GetMessage ( &msg, NULL, 0, 0 ) ) {
		TranslateMessage( &msg );
		DispatchMessage ( &msg );
	}
    UnregisterClass( THIS_CLASSNAME, hInst );
    UnregisterClass( "SelectionClass", hInst );

	return msg.wParam;
}


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

void SaveBitmap (HDC MyDC , HBITMAP hBitmap , bool Upload)
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
            sprintf(FilePath, "%s\\%s.png",UploadedSsDir,FileName);
        else
            sprintf(FilePath, "%s\\%s.png",SsDir,FileName);

        Gdiplus::Bitmap PNGbitmap(hBitmap, NULL);
        CLSID clsid;
        GetEncoderClsid(L"image/png", &clsid);
        PNGbitmap.Save(towstring(FilePath).c_str(), &clsid,NULL);
	}
    GdiplusShutdown(gdiplusToken);

    if(Upload)  {
        void *pFilePath[1];
        pFilePath[0] = FilePath;
        pomfit_upload_file(pFilePath, 1);
        pFilePath[0] = NULL;
    }
    if(KeepUploadedSS == false)
        if(remove(FilePath) != 0)
            fprintf(stderr,"Error deleting file %s",FilePath);

}

void ChooseFiles (void)
{
    if(IsUploading) {
        return;
    }
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT ;
    ofn.lpstrDefExt = "*";

    if(GetOpenFileName(&ofn)) {
        char szFileList[50][512];
        char szFileTok[260];
        char *p;
        p = ofn.lpstrFile;
        int i = 0;
        while(*p) {
            p += lstrlen(p) + 1;
            wsprintf(szFileTok, "%s", p);
            if(strlen(szFileTok) < 2 && i > 0)
                break;
            else if(strlen(szFileTok) < 2 && i == 0) {
                wsprintf(szFileList[i], "%s",ofn.lpstrFile);
                ++i;
                break;
            }
            else
                wsprintf(szFileList[i], "%s\\%s",ofn.lpstrFile, szFileTok);
                ++i;
        }
        void *pFileList[i];
        int j = 0;
        for(j = 0; j < i ; ++j)
            pFileList[j]= szFileList[j];

        pomfit_upload_file(pFileList,i);

        for(j = 0; j < i ; ++j)
            pFileList[j]= NULL;
    }
    else
        return;
}

void Clean_Login_Data(void)
{
    UINT k;
    for(k = 0 ; k < strlen(LoginEmail); ++k)
        LoginEmail[k] = '\0';
    for(k = 0 ; k < strlen(LoginEmail); ++k)
        LoginPass[k] = '\0';
}

DWORD WINAPI ReloginTimer(LPVOID lpParameter)
{
    time_t StartWait;
	while(true) {
        StartWait = time(NULL);
        while((StartWait + 7200) > time(NULL)) {
            Sleep(5000);
        }
        if(pomfit_curl_logout() == 1)   // If logout fails, let's wait 10 minutes in case of net issues
        {
            StartWait = time(NULL);
            while((StartWait + 600) > time(NULL))
                Sleep(5000);
            if(pomfit_curl_logout() == 1) {
				pomfit_notify_me("You are now logged out.\nYour uploaded files are no longer linked to your account.",
                                 "Automatic relogin failed!");
				return 0;
			}
        }
        if(pomfit_curl_login() == 1)
            IsLoged = true;
        else{                               // If login fails, let's wait 10 minutes in case of net issues
            StartWait = time(NULL);
            while((StartWait + 600) > time(NULL))
                Sleep(5000);
             if(pomfit_curl_login() == 1)
                IsLoged = true;
            else{
                pomfit_notify_me("You are now logged out.\nYour uploaded files are no longer linked to your account.",
                                 "Automatic relogin failed!");
                SetWindowText(hWndLoginInfo, "Not Logged in..");
                SetWindowText(hWndLoginBut, "Login");
                Clean_Login_Data();
                IsLoged = false;
                return 0;
            }
        }
    }
    return 0;
}

void GetLoginData (void)
{
    GetWindowText(hWndEntry1, LoginEmail, 128);
    GetWindowText(hWndEntry2, LoginPass, 64);

    if(pomfit_curl_login() == 1) {
        SetWindowText(hWndLoginInfo, LoginEmail);
        SetWindowText(hWndLoginBut, "Log out");
        UpdateWindow(hWnd);
        IsLoged = true;

        RLHandle = CreateThread(0, 0, ReloginTimer, 0, 0, &RLThreadID);
    } else  {
        SetWindowText(hWndLoginInfo, "Login Failed.. Wrong data?");
        UpdateWindow(hWnd);
        IsLoged = false;
        Clean_Login_Data();
    }
    SetWindowText(hWndEntry2, "");
}

void UserLogout (void)
{
     if(pomfit_curl_logout() == 0) {
        SetWindowText(hWndLoginInfo, "You are not logged in..");
        SetWindowText(hWndLoginBut, "Login");
        IsLoged = false;
        TerminateThread(RLHandle,0);
        Clean_Login_Data();
    }
    else
        pomfit_notify_me("Please try again.","Curl logout failed!");
}

void CaptureActiveWindow(bool Upload)
{

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

                SaveBitmap(hdcMem, hbitmap, Upload);


                DeleteObject(hbitmap);
            }
            DeleteObject(hdcMem);
        }
        ReleaseDC(hWndActWnd, hdc);
    }
    DeleteObject(hdc);
}

void CaptureFullScreen (bool Upload)
{
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
    SaveBitmap(NULL, hCaptureBitmap , Upload);

    ReleaseDC(hDesktopWnd,hDesktopDC);
    DeleteDC(hCaptureDC);
    DeleteObject(hCaptureBitmap);
}

static void DrawSelectionRect(HWND hWnd, const RECT* pRect)
{
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

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
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

static LRESULT CALLBACK SelWndProc( HWND hWnd,  UINT uMsg, WPARAM wParam, LPARAM lParam ) {

    int height, width;
    HBITMAP hBitmap;
    HDC DlgDC;

    static RECT rcSelection;
    static bool IsSelecting;

	switch (uMsg) {

        case WM_LBUTTONDOWN:
            if(DrawRectangle) {
            if(IsSelecting)
                ReleaseCapture();

            IsSelecting = true;
            GetCursorPos(&Begpt);
            rcSelection.left = (short)LOWORD(lParam);
            rcSelection.top = (short)HIWORD(lParam);
            rcSelection.right = (short)LOWORD(lParam);
            rcSelection.bottom = (short)HIWORD(lParam);

            DrawSelectionRect(hWnd, &rcSelection);
            SetCapture(hWndSel);
            }
            else if(CaptureWindow) {
                GetCursorPos(&Begpt);
                ShowWindow(hWndSel,0);
                hWndActWnd= WindowFromPoint(Begpt);
                CaptureActiveWindow(UploadSS);
                CaptureWindow = false;
                UploadSS = false;
            }
            break;
        case WM_MOUSEMOVE:
            if(DrawRectangle && IsSelecting) {
                RECT rcClient;

                DrawSelectionRect(hWnd, &rcSelection);

                rcSelection.right = (short)LOWORD(lParam);
                rcSelection.bottom = (short)HIWORD(lParam);

                GetClientRect(hWnd, &rcClient);
                if(rcSelection.right < 0)
                    rcSelection.right = 0;
                if(rcSelection.right > rcClient.right)
                    rcSelection.right = rcClient.right;
                if(rcSelection.bottom < 0)
                    rcSelection.bottom = 0;
                if(rcSelection.bottom > rcClient.bottom)
                    rcSelection.bottom = rcClient.bottom;

                DrawSelectionRect(hWnd, &rcSelection);
            }
            if(DrawRectangle && !IsSelecting){
                if((short)LOWORD(lParam) >= PrimWidth || (short)LOWORD(lParam) <= 0 ) {
                    Sleep(1000);
                    GetCursorPos(&CurMPos);
                    ShowWindow(hWndSel,0);
                    HMONITOR CurMonitor = MonitorFromPoint(CurMPos,MONITOR_DEFAULTTONULL);
                    CurMonInfo.cbSize = sizeof(MONITORINFO);
                    GetMonitorInfo(CurMonitor,(LPMONITORINFO)&CurMonInfo);
                    SetWindowPos(hWndSel, 0, CurMonInfo.rcMonitor.left, 0, 0, 0, SWP_NOSIZE);
                    ShowWindow(hWndSel,SW_SHOW);
                    UpdateWindow(hWndSel);
                }
            }
            break;
        case WM_LBUTTONUP:
            if(DrawRectangle == true) {
                GetCursorPos(&Endpt);
                DrawRectangle = false;
                IsSelecting = false;
                ShowWindow(hWndSel,0);

                rcSelection.right = (short)LOWORD(lParam);
                rcSelection.bottom = (short)HIWORD(lParam);

                width= Endpt.x >Begpt.x ?Endpt.x-Begpt.x : Begpt.x -Endpt.x;
                height= Endpt.y >Begpt.y ?Endpt.y-Begpt.y : Begpt.y -Endpt.y;

                if(width <= 1 && height <= 1) {
                    GetCursorPos(&Begpt);
                    ShowWindow(hWndSel,0);
                    hWndActWnd= WindowFromPoint(Begpt);
                    CaptureActiveWindow(UploadSS);
                    CaptureWindow = false;
                    UploadSS = false;
                    break;
                }
                if(Begpt.x > Endpt.x && Begpt.y > Endpt.y) {
                    Temppt = Begpt;
                    Begpt =Endpt;
                    Endpt = Begpt;
                }
                if(Begpt.x >Endpt.x && Begpt.y < Endpt.y) {
                    Temppt.x = Begpt.x;
                    Begpt.x =Endpt.x;
                    Endpt.x = Begpt.x;
                }
                if(Begpt.x < Endpt.x && Begpt.y > Endpt.y)  {
                    Temppt.y = Begpt.y;
                    Begpt.y =Endpt.y;
                    Endpt.y = Begpt.y;
                }
                DeskDC = GetDC(NULL);
                DlgDC = GetDC(hWnd);
                MemDC = CreateCompatibleDC(DeskDC);
                hBitmap = CreateCompatibleBitmap(DlgDC,width,height);
                SelectObject(MemDC,hBitmap);
                BitBlt(MemDC,0,0,width,height ,DeskDC,Begpt.x,Begpt.y,SRCCOPY);
                SaveBitmap(MemDC, hBitmap,UploadSS);

                ReleaseDC(hWnd,DlgDC);
                ReleaseDC(NULL,DeskDC);
                DeleteDC(MemDC);
                DeleteObject(hBitmap);

                UploadSS = false;
            }
            break;
    }
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

BOOL CALLBACK MonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData){

    if(lprcMonitor->left < SmallestX)
        SmallestX = lprcMonitor->left;
    return true;
}

void SaveSettings (void) {

    int j;
    int SelBox1[HOTKEY_COUNT], SelBox2[HOTKEY_COUNT];
    char EntryChar[HOTKEY_COUNT][2];


    for( j = 0 ; j < HOTKEY_COUNT ; ++j) {
        SelBox1[j] = ComboBox_GetCurSel(SetHotSel1[j]);
        SelBox2[j] = ComboBox_GetCurSel(SetHotSel2[j]);
        GetWindowText(SetHotEntry[j], EntryChar[j], 2);
	}

    char ConfFilePath [512];
	sprintf(ConfFilePath ,"%s\\pomfit.ini" ,ConfDir);
	FILE *config = fopen(ConfFilePath , "w");
	if (config == NULL) {
		fprintf(stderr,"Couldn't create conf file\n");
		return;
	}

	fprintf(config, "Hotkey_Full_SS = %d_%d_%s\n",SelBox1[0],SelBox2[0], EntryChar[0]);
	fprintf(config, "Hotkey_Win_SS = %d_%d_%s\n",SelBox1[1],SelBox2[1],  EntryChar[1]);
	fprintf(config, "Hotkey_Area_SS = %d_%d_%s\n",SelBox1[2],SelBox2[2], EntryChar[2]);
    fprintf(config, "Hotkey_Full_SS_UP = %d_%d_%s\n",SelBox1[3],SelBox2[3], EntryChar[3]);
	fprintf(config, "Hotkey_Win_SS_UP = %d_%d_%s\n",SelBox1[4],SelBox2[4], EntryChar[4]);
	fprintf(config, "Hotkey_Area_SS_UP = %d_%d_%s\n",SelBox1[5],SelBox2[5], EntryChar[5]);
	fprintf(config, "Hotkey_File_UP = %d_%d_%s\n",SelBox1[6],SelBox2[6], EntryChar[6]);
	fprintf(config, "Hotkey_Open_Links = %d_%d_%s\n",SelBox1[7],SelBox2[7], EntryChar[7]);
	fprintf(config, "Keep_Uploaded_SS = %d\n",Button_GetCheck(KeepUpSSButt) );
    fclose(config);

    LoadSettings();
}

void LoadSettings (void) {
    char ConfFilePath [512];
	sprintf(ConfFilePath ,"%s\\pomfit.ini" ,ConfDir);
	FILE *config = fopen(ConfFilePath , "r");
	if (config == NULL) {
        LoadDefaults ();
		fprintf(stderr ,"Couldn't load config file..using defaults\n");
		return;
	}
	const char *pBuff = NULL;
	char *pGetline = NULL;
	char Mod[2];
    int j = 0;
	pGetline = (char*) malloc (sizeof(char)*512);
	if(pGetline == NULL) {
		fprintf(stderr ,"Memory error");
	}
    while(fgets (pGetline , 512 , config) != NULL) {
	    if(j < HOTKEY_COUNT) {
            UnregisterHotKey(hWnd, 200+j);
            pBuff = strstr(pGetline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            iMod1[j] = atoi(Mod);
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            iMod2[j] = atoi(Mod);
            pBuff += 2;
            bKey[j] = pBuff[0];
            if(isprint(bKey[j]) == false) {
                ++j;
                continue;
            }
            UINT SetMod[2];
            SetMod[0] = (iMod1[j] == 0 ?  2 : iMod1[j] == 1 ?  4 :  iMod1[j] == 2 ? 1 : 0);
            SetMod[1] = (iMod2[j] == 0 ?  2 : iMod2[j] == 1 ?  4 :  iMod2[j] == 2 ? 1 : 0);
            RegisterHotKey(hWnd, 200+j , SetMod[0] | SetMod[1] , bKey[j]);
            SendMessage(SetHotSel1[j], CB_SETCURSEL, (WPARAM)iMod1[j], (LPARAM)0);
            SendMessage(SetHotSel2[j], CB_SETCURSEL, (WPARAM)iMod2[j], (LPARAM)0);
            SetWindowText( SetHotEntry[j],  pBuff );

            ++j;
	    }
	    if(strncmp(pGetline, "Keep_Uploaded_SS" , strlen("Keep_Uploaded_SS")) == 0) {
            pBuff = strstr(pGetline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            KeepUploadedSS = atoi(Mod);
            Button_SetCheck(KeepUpSSButt, KeepUploadedSS);
	    }
	}

	free(pGetline);
	pBuff = NULL;

}

void LoadDefaults (void) {
    int j;
    for (j = 0 ; j < HOTKEY_COUNT ; ++j) {
        SendMessage(SetHotSel1[j], CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        SendMessage(SetHotSel2[j], CB_SETCURSEL, (WPARAM)j < 3 ? 2 : 1, (LPARAM)0);
    }
    SetWindowText( SetHotEntry[0],  "1" );
    SetWindowText( SetHotEntry[1],  "2" );
    SetWindowText( SetHotEntry[2],  "3" );
    SetWindowText( SetHotEntry[3],  "1" );
    SetWindowText( SetHotEntry[4],  "2" );
    SetWindowText( SetHotEntry[5],  "3" );
    SetWindowText( SetHotEntry[6],  "F" );
    SetWindowText( SetHotEntry[7],  "B" );

    RegisterHotKey(hWnd, HOTKEY_AREA_SS_UP, MOD_SHIFT | MOD_CONTROL, '3');
    RegisterHotKey(hWnd, HOTKEY_WIN_SS_UP, MOD_SHIFT | MOD_CONTROL, '2');
    RegisterHotKey(hWnd, HOTKEY_FULL_SS_UP, MOD_SHIFT | MOD_CONTROL, '1');
    RegisterHotKey(hWnd, HOTKEY_AREA_SS_CAP, MOD_ALT | MOD_CONTROL, '3');
    RegisterHotKey(hWnd, HOTKEY_WIN_SS_CAP, MOD_ALT | MOD_CONTROL, '2');
    RegisterHotKey(hWnd, HOTKEY_FULL_SS_CAP, MOD_ALT | MOD_CONTROL, '1');
    RegisterHotKey(hWnd, HOTKEY_FILEUP, MOD_SHIFT | MOD_CONTROL, 'F');
    RegisterHotKey(hWnd, HOTKEY_OPEN, MOD_SHIFT | MOD_CONTROL, 'B');

    KeepUploadedSS = true;
    Button_SetCheck(KeepUpSSButt,BST_CHECKED);

    SaveSettings();

}

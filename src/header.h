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

#ifdef _WIN32
    #define _WIN32_IE 0x0500
    #define __WIN32_WINNT 0x0501
    #define _WIN32_WINNT 0x0500
    #define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <windowsx.h>
    #include <winuser.h>
    #include <wingdi.h>
	#include <ShellAPI.h>
    #include <commctrl.h>
    #include <Commdlg.h>
    #include <shlobj.h>
    #include <gdiplus.h>
    #include <gdiplus/gdiplusimagecodec.h>
	#include "resource.h"
	#include "C:\libcurl\include\curl\curl.h"
#elif __linux__
	#include <gtk/gtk.h>
	#include <libnotify/notify.h>
	#include <keybinder.h>
	#include <curl/curl.h>
#endif
	#include <stdio.h>
	#include <stdlib.h>
	#include <time.h>
	#include <string.h>
	#include <stdbool.h>
	#include <unistd.h>

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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <regex.h>
#include "other.h"
#include "log.h"
#include "structs.h"
#ifdef __linux__
    #include <libnotify/notify.h>
#elif _WIN32
	#define _WIN32_IE 0x0500
    #define __WIN32_WINNT 0x0501
    #define _WIN32_WINNT 0x0500
    #define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <ShellAPI.h>
    #include <shlobj.h>
	extern NOTIFYICONDATA  nid;
#endif

extern UpProfile ActiveProfile;
extern GdkPixbuf *p_icon;
extern char *BatchLinks;
extern char ActProfile[256];
extern void show_uri(const char *keystring, void *uri);

regex_t URL_split;
char RegexMatchURL[128];
char RegexMatchDEL[128];

int pomfit_check_size(const char *File, int LimitMB) {

	long long FileSize = 0;
	long long FileSizeLimit = LimitMB*1000000;
	char Buff[2][128];
	char *FileName = NULL;

	FILE *file_check;
	file_check = fopen(File, "rb");
	if(!file_check)
		return 1;
	else
	{
		fseek(file_check, 0 , SEEK_END);
		FileSize = ftell(file_check);
		fclose(file_check);
	}
	if(FileSize>FileSizeLimit) {
#ifdef __linux__
		FileName = strrchr(File, '/');
#elif _WIN32
        FileName = strrchr(File, '\\');
#endif
		FileName += 1;
		sprintf(Buff[0], "File '%s' is too big !", FileName);
		sprintf(Buff[1], "File size: %0.2f MiB (%0.2f MB)\n"
						"Size limit: %d MB",
						(float)FileSize/1048576, (float)FileSize/1000000, LimitMB);
		pomfit_notify(Buff[1],Buff[0]);
        FileName = NULL;
		return 1;
	}
    FileName = NULL;
	return 0;
}

void pomfit_notify (const char *BotMsg, const char *TopMsg) {

#ifdef __linux__
	NotifyNotification *Uploaded;
	notify_init("Uploaded");
	Uploaded = notify_notification_new(TopMsg, BotMsg , NULL);
	notify_notification_set_icon_from_pixbuf(Uploaded , p_icon);
	notify_notification_show (Uploaded, NULL);
	g_object_unref(G_OBJECT(Uploaded));
	notify_uninit();
#elif _WIN32
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uTimeout = 5000;
    nid.uFlags = NIF_INFO;
    nid.dwInfoFlags = NIIF_INFO;
    strcpy(nid.szInfoTitle, TopMsg);
    strcpy(nid.szInfo, BotMsg);
    Shell_NotifyIcon( NIM_MODIFY, &nid );
#endif
}

char *double_char (char *String, char Char) {

    static char temp[4096];
    static char buffer[4096];
    strcpy(temp, String);
    int i , k = 0;
    for ( i = 0, k = 0 ; i < strlen(temp) ; ++i, ++k) {

        if(temp[i] == Char ) {
            buffer[k] = temp[i];
            buffer[k+1] = temp[i];
            ++k;
        } else
            buffer[k] = temp[i];
    }
    buffer[k] = '\0';
    sprintf(String,"%s",buffer);
    return String;
}

char *replace_str(char *str, char *orig, const char *rep, int start) {

	static char temp[4096];
	static char buffer[4096];
	char *p;

	strcpy(temp, str + start);

	if(!(p = strstr(temp, orig)))
		return temp;

	strncpy(buffer, temp, p-temp);
	buffer[p-temp] = '\0';

	sprintf(buffer + (p - temp), "%s%s", rep, p + strlen(orig));
	sprintf(str + start, "%s", buffer);

	return str;
}

void remove_char(char *str, char garbage)
{
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

void pomfit_open_links(const char *keystring, void *user_data) {

	if(BatchLinks == NULL) {
		show_uri(NULL, "http://pomf.se");
		return;
	}
	char *pTok = NULL;
	char buffer[strlen(BatchLinks)+1];
	strcpy(buffer,BatchLinks);
	pTok = strtok(buffer, " ");
	while (pTok != NULL) {
		show_uri(NULL,pTok);
		pTok = strtok (NULL, " ");
	}
}


int pomfit_regex_compile (char *RegExString) {

	int status = regcomp (&URL_split, RegExString, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
		char ErrMsg[512];
		regerror (status, &URL_split, ErrMsg, 512);
		log_error(true,"***Error Regex: compiling failed: %s\n", ErrMsg);
		return 1;
    }
    return 0;
}

int pomfit_regex_search (char *Path, char *Regex, bool Delete) {

    int i = 0;
	char Buffer[256];
	char Getline[512];
	const char *pStuff = NULL;
    const int n_matches = 10;
    regmatch_t m[n_matches];
	printf("DEBUG: Regex '%s'\n",Regex);

	if(pomfit_regex_compile(Regex) == 0) {

		FILE *pOutputFile = fopen(Path, "r");
		if(!pOutputFile) {

			log_error(true,"***Error: Regex couldn't read output file");
			return -1;
		}
		while(fgets (Getline , 511 , pOutputFile) != NULL ) {

			pStuff = Getline;

			while (1) {
				int nomatch = regexec (&URL_split, pStuff, n_matches, m, 0);
				if (nomatch)
					break;

				for (i = 0; i < n_matches; i++) {
					int start;
					int finish;
					if (m[i].rm_so == -1) {
						break;
					}
					start = m[i].rm_so + (pStuff - Getline);
					finish = m[i].rm_eo + (pStuff - Getline);

					printf ("Regout: i=%d - '%.*s' (bytes %d:%d)\n",i, (finish - start), Getline + start, start, finish);
					if(i == 1) {
						snprintf(Buffer,sizeof(Buffer),"%.*s", (finish - start), Getline + start);
						if(Delete) {

							if(ActiveProfile.bPrepend) {
								if(strstr(Buffer,"http://") != NULL)
									snprintf(RegexMatchDEL,sizeof(RegexMatchDEL),"%s" ,Buffer);
								else
									snprintf(RegexMatchDEL,sizeof(RegexMatchDEL),"%s%s" ,
															ActiveProfile.PrepString,Buffer);

							} else
								snprintf(RegexMatchDEL,sizeof(RegexMatchDEL),"%s" ,Buffer);

						} else {

							if(ActiveProfile.bPrepend) {

								if(strstr(Buffer,"http://") != NULL)
									snprintf(RegexMatchURL,sizeof(RegexMatchURL),"%s" ,Buffer);
								else
									snprintf(RegexMatchURL,sizeof(RegexMatchURL),"%s%s" ,
														ActiveProfile.PrepString,Buffer);

							} else
								snprintf(RegexMatchURL,sizeof(RegexMatchURL),"%s" ,Buffer);

						}
						fclose(pOutputFile);
						return 0;
					}
				}
				pStuff += m[0].rm_eo;
			}
		}
		if(pOutputFile)
            fclose(pOutputFile);
		return 0;
	} else
		return 1;
}

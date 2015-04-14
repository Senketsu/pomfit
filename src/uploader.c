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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <gtk/gtk.h>
#include <curl/curl.h>
#include <unistd.h>
#include "database.h"
#include "log.h"
#include "structs.h"
#include "uploader.h"
#include "other.h"

extern char DirHome[64];
extern char DirScrn[512];
extern char DirUpScrn[512];
extern char PathCookie[256];
extern char PathCurlOut[256];
extern bool bUP_Keep_SS;

extern GtkWidget *up_but;
extern GtkWidget *link_but;
extern GtkWidget *status_bar;

extern UpProfile ActiveProfile;
extern char RegexMatchURL[128];
extern char RegexMatchDEL[128];

char *BatchLinks;
bool IsUploading;
time_t TimeUpStarted;

void uploader_curl_file (void **apFilesPaths , int ListCount) {

	void *pChoosedFile = NULL;
	char *FileName = NULL;
	long long FileSize = 0;
	double TotalTime = 0;
	double BatchTotalTime = 0;
	float BatchSize = 0;
	char buff[256];
	bool CancelReq = false;
	int i, UpDone = 0;

	if(access(PathCurlOut, F_OK ) != -1)
		remove(PathCurlOut);
	if(ListCount == 0)
		return;

	BatchLinks = (char*) realloc(BatchLinks,(ListCount*sizeof(RegexMatchURL)));
	if(BatchLinks == NULL)
			log_error(true,"***Error: Memory error @ L62 - pomfit.c");

	for(i = 0 ; i < ListCount; ++i) {

		pChoosedFile = apFilesPaths[i];
		if(pomfit_check_size(pChoosedFile,ActiveProfile.MaxSize) == 1)
			continue;

		CURL *curl;
		CURLcode res;
		struct curl_httppost *post = NULL;
		struct curl_httppost *last = NULL;
		pChoosedFile = apFilesPaths[i];
		FILE *pUploadFile = fopen(pChoosedFile, "rb");
		if(!pUploadFile) {

			log_error(true,"***Error: Uploader failed to read file %s",pChoosedFile);
			pChoosedFile = NULL;
			return;
		} else {

			fseek(pUploadFile, 0 , SEEK_END);
			FileSize = ftell(pUploadFile);
			rewind(pUploadFile);
		}
		FILE *pOutputFile;
		pOutputFile = fopen(PathCurlOut, "a");
		if(!pOutputFile) {

			log_error(true,"***Error: Uploader failed to create output file %s",PathCurlOut);
			if(pUploadFile)
				fclose(pUploadFile);
			return;
		}
		/// Get name from filepath
		#ifdef __linux__
		FileName = strrchr(pChoosedFile, '/');
		#elif _WIN32
		FileName = strrchr(pChoosedFile, '\\');
		#endif
		FileName += 1;
		gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_NONE);
		gtk_button_set_label(GTK_BUTTON(up_but), "Cancel");
		IsUploading = TRUE;

		TimeUpStarted = time(NULL);
		curl = curl_easy_init();
		if(curl)
		{
			if(ActiveProfile.bFormName) {
				char *Suffix = strrchr(FileName, '.');
				char CustomFileName[512];
				sprintf(CustomFileName, "%s%s",ActiveProfile.FormFileName,Suffix);
				printf("%s\n",CustomFileName);
				curl_formadd(&post, &last,
					CURLFORM_COPYNAME, ActiveProfile.FormName,
					CURLFORM_FILE, pChoosedFile,
					CURLFORM_FILENAME, CustomFileName,
					CURLFORM_CONTENTTYPE, ActiveProfile.FormType,
					CURLFORM_END);

			} else if (!ActiveProfile.bNames){
				curl_formadd(&post, &last,
					CURLFORM_COPYNAME, ActiveProfile.FormName,
					CURLFORM_FILE, pChoosedFile,
					CURLFORM_FILENAME, "Pomfit_Upload",
					CURLFORM_CONTENTTYPE, ActiveProfile.FormType,
					CURLFORM_END);
			} else {
				curl_formadd(&post, &last,
					CURLFORM_COPYNAME, ActiveProfile.FormName,
					CURLFORM_FILE, pChoosedFile,
					CURLFORM_CONTENTTYPE, ActiveProfile.FormType,
					CURLFORM_END);
			}
			curl_easy_setopt(curl, CURLOPT_URL, ActiveProfile.URL);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, uploader_curl_progress);
			curl_easy_setopt(curl, CURLOPT_READDATA, pUploadFile);
			curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)FileSize);
			curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, POMFIT_USER_AGENT);

			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
			curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, pOutputFile);
			if(ActiveProfile.bCookie) {
				curl_easy_setopt(curl, CURLOPT_COOKIESESSION, TRUE);
				curl_easy_setopt(curl, CURLOPT_COOKIEJAR, PathCookie);
				curl_easy_setopt(curl, CURLOPT_COOKIEFILE, PathCookie);
			}
			res = curl_easy_perform(curl);
			if(res != CURLE_OK) {

				log_error(true, "***Error: curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
				if(ListCount == 1 || strstr(curl_easy_strerror(res),"aborted") || CancelReq) {

					gtk_statusbar_push(GTK_STATUSBAR(status_bar),
										1,"Uploading canceled.");
					gtk_button_set_relief(GTK_BUTTON(up_but),
											GTK_RELIEF_NORMAL);
					gtk_button_set_label(GTK_BUTTON(up_but), "Upload");
					IsUploading = FALSE;
					pomfit_notify(curl_easy_strerror(res),"Uploading canceled");
					pChoosedFile = NULL;
					if(pUploadFile)
						fclose(pUploadFile);
					if(pOutputFile)
						fclose(pOutputFile);
					curl_formfree(post);
					curl_easy_cleanup(curl);
					if(remove(PathCurlOut) != 0)
                        perror("Error deleting temp file");

					return;
				} else {

                    pChoosedFile = NULL;
					if(pUploadFile)
						fclose(pUploadFile);
					if(pOutputFile)
						fclose(pOutputFile);
					curl_formfree(post);
					curl_easy_cleanup(curl);
					CancelReq = true;
					continue;
                }
			} else {
				fprintf(pOutputFile,"\n");
				BatchSize += (float)FileSize/1000000;;
				UpDone += 1;
			}
			curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &TotalTime);
			curl_formfree(post);
			curl_easy_cleanup(curl);
			BatchTotalTime += TotalTime;
		}
		fclose(pOutputFile);
		if(pUploadFile)
			fclose(pUploadFile);

		if (uploader_curl_output(PathCurlOut,pChoosedFile) == 0) {

			if(i == 0)
				sprintf(BatchLinks,"%s ", RegexMatchURL);
			else
				sprintf(strchr(BatchLinks, '\0'),"%s ", RegexMatchURL);


			RegexMatchURL[0] = '\0';
			RegexMatchDEL[0] = '\0';

		}
	}
	if(UpDone == 0)
		return;
    sprintf(buff, "%d File%s(%.2f MB) in %dmin %dsec",
				UpDone,UpDone < 2 ? "" : "s",BatchSize ,
				(int)BatchTotalTime > 60 ? (int)BatchTotalTime/60 : 0,
				(int)BatchTotalTime%60 > 0 ? (int)BatchTotalTime%60 : 1);
	GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_can_store (clipboard,NULL,0);
	gtk_clipboard_set_text(clipboard, BatchLinks, strlen(BatchLinks)-1);
	gtk_clipboard_store(clipboard);
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, buff);
	sprintf(strchr(buff, '\0'),"\nLink%s copied to clipboard ", UpDone < 2 ? "" : "s");
	pomfit_notify(buff,"Uploading Finished");
	sprintf(buff,"Click to open all links.\nRight click to copy links.");
	gtk_widget_set_tooltip_text(link_but, buff);
	gtk_link_button_set_uri(GTK_LINK_BUTTON(link_but), BatchLinks);
	sprintf(buff,"%d/%d File%s Uploaded",
				UpDone, ListCount, UpDone < 2 ? "" : "s");
	gtk_button_set_label(GTK_BUTTON(link_but), buff);
	gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_NORMAL);
	gtk_button_set_label(GTK_BUTTON(up_but), "Upload");
	IsUploading = FALSE;

	pChoosedFile = NULL;
	for(i = 0 ; i < ListCount; ++i)
		apFilesPaths[i] = NULL;

}

int uploader_curl_progress (void* ptr, double TotalToDownload, double NowDownloaded,
							double TotalToUpload, double NowUploaded) {

	time_t TimeNow = time(NULL);
	char UpStatus [64];
	double UpSizeLeft = TotalToUpload - NowUploaded;
	double UpPercent = NowUploaded / TotalToUpload;
	double UpSpeed = NowUploaded/(difftime(TimeNow,TimeUpStarted));
	double UpETA = UpSizeLeft/UpSpeed;
	sprintf(UpStatus, "%3.2f%% | %6.2f KiB/s | %5.0fs left",
							UpPercent*100, UpSpeed/1024, UpETA);

	if(gtk_button_get_relief(GTK_BUTTON(up_but)) == GTK_RELIEF_HALF)
		return 1;
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, UpStatus);
	while (gtk_events_pending ())
		gtk_main_iteration ();

    return 0;
}

int uploader_curl_output (char *OutputFile, char *FilePath) {

	FILE *pOutputFile = NULL;
	char Getline[512] = "";
	char URL[128] = "";
	char DEL_URL[128] = "\0";
	char *pFileName = NULL;
	char *pBuff = NULL;

	#ifdef __linux__
	pFileName = strrchr(FilePath, '/');
	#elif _WIN32
	pFileName = strrchr(FilePath, '\\');
	#endif
	pFileName += 1;

/* Shitty regex */
	if(strlen(ActiveProfile.RegEx_URL) > 1) {
		if(pomfit_regex_search(OutputFile,ActiveProfile.RegEx_URL, false) == 0) {
			if(strlen(ActiveProfile.RegEx_DEL) > 1)
				pomfit_regex_search(OutputFile,ActiveProfile.RegEx_DEL,true);

            SQLite.execute2("INSERT OR REPLACE INTO uploads (file_name, url, del_url, file_path) "
                "VALUES (?, ?, ?, ?)", 4, pFileName, RegexMatchURL, RegexMatchDEL, FilePath);

			if(remove(OutputFile) != 0)
				log_error(true,"***Error: Deleting curl temp file failed %s",OutputFile);
			return 0;
		} else
			return 1;
	} else {
/* Shitty fallback */
		pOutputFile = fopen(OutputFile, "r");
		if(!pOutputFile) {
			log_error(true,"***Error: Uploader couldn't read output file");
			return 1;
		}
		if(fgets (Getline , 511 , pOutputFile) != NULL ) {
			if((pBuff = strstr(Getline , "http://")) != NULL) {

				char Buffer[512];
				strcpy(Buffer,pBuff);
				pBuff = strtok(Buffer, " \n\\<>");
				sprintf(URL ,"%s", pBuff);
                 SQLite.execute2("INSERT OR REPLACE INTO uploads (file_name, url, del_url, file_path) "
                        "VALUES (?, ?, ?, ?)", 4, pFileName, URL, DEL_URL, FilePath);
			}

			strcpy(RegexMatchURL,URL);
			pFileName = NULL;
			pBuff = NULL;
		}
		fclose(pOutputFile);
		if(remove(OutputFile) != 0)
			log_error(true,"***Error: Deleting curl temp file failed %s",OutputFile);
		return 0;
	}
}

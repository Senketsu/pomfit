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

#include "header.h"
#include "pomfit.h"

extern char HomeDir[64];
#ifdef __linux__
extern GdkPixbuf *p_icon;
extern GtkWidget *up_but;
extern GtkWidget *link_but;
extern GtkWidget *status_bar;
extern GtkWidget *entry_pass, *entry_email;
#elif _WIN32
extern NOTIFYICONDATA  nid;
extern HWND hWnd;
extern HWND hWndSel;
extern bool DrawRectangle;
extern bool CaptureWindow;
extern bool UploadSS;
#endif

void pomfit_upload_file (void **apFilesPaths , int ListCount) {
	char *pBuff = NULL;
	char *pGetline = NULL;
	void *pChoosedFile = NULL;
	long long FileSize = 0;
	double TotalTime = 0;
	double BatchTotalTime = 0;
	float BatchSize = 0;
	char buff[256];
	char OutputFilePath[96];
	char FileName[256];
	char UpFileUrl[64];
#ifdef __linux__
	sprintf(OutputFilePath ,"%s/curl_output" , ConfDir);
#elif _WIN32
    sprintf(OutputFilePath ,"%s\\curl_output" , ConfDir);
#endif
	int i = 0;
	int UpDone = 0;
	if(ListCount == 0)
	{
		perror("No files selected");
		return;
	}
	for(i = 0 ; i < ListCount; ++i)
	{
		pChoosedFile = apFilesPaths[i];
		if(pomfit_check_size(pChoosedFile) == 1)
			continue;
		CURL *curl;
		CURLcode res;
		struct curl_httppost *post = NULL;
		struct curl_httppost *last = NULL;
		pChoosedFile = apFilesPaths[i];
		FILE *pUploadFile = fopen(pChoosedFile, "rb");
		if(!pUploadFile)
		{
			perror("Invalid path to file");
			pChoosedFile = NULL;
			return;
		}
		else
		{
			fseek(pUploadFile, 0 , SEEK_END);
			FileSize = ftell(pUploadFile);
			rewind(pUploadFile);
		}
		FILE *pOutputFile;
		pOutputFile = fopen(OutputFilePath, "a");
		if(!pOutputFile)
		{
			perror("Couldn't create output file");
			if(pUploadFile)
				fclose(pUploadFile);
			return;
		}
#ifdef __linux__
		gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_NONE);
		gtk_button_set_label(GTK_BUTTON(up_but), "Cancel");
		IsUploading = TRUE;
#elif _WIN32
        IsUploading = true;
#endif
		TimeUpStarted = time(NULL);
		curl = curl_easy_init();
		if(curl)
		{
			curl_formadd(&post, &last,
					CURLFORM_COPYNAME, "files[]",
					CURLFORM_FILE, pChoosedFile,
					CURLFORM_CONTENTTYPE, "application/octet-stream",
					CURLFORM_END);
			curl_easy_setopt(curl, CURLOPT_URL, "http://pomf.se/upload.php");
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, pomfit_curl_progress);
			curl_easy_setopt(curl, CURLOPT_READDATA, pUploadFile);
			curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)FileSize);
			curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, POMFIT_USER_AGENT);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
			curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, pOutputFile);
			curl_easy_setopt(curl, CURLOPT_COOKIEFILE, CookieFile);
			res = curl_easy_perform(curl);
			if(res != CURLE_OK)
			{
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
				if(ListCount == 1 || strstr(curl_easy_strerror(res),"aborted"))
				{
#ifdef __linux__
					gtk_statusbar_push(GTK_STATUSBAR(status_bar),
										1,"Uploading canceled.");
					gtk_button_set_relief(GTK_BUTTON(up_but),
											GTK_RELIEF_NORMAL);
					gtk_button_set_label(GTK_BUTTON(up_but), "Upload");
					IsUploading = FALSE;
#elif _WIN32
                    IsUploading = false;
#endif
					pChoosedFile = NULL;
					if(pUploadFile)
						fclose(pUploadFile);
					if(pOutputFile)
						fclose(pOutputFile);
					curl_formfree(post);
					curl_easy_cleanup(curl);
					if(remove(OutputFilePath) != 0)
                        perror("Error deleting temp file");
					return;
				}
				else
                {
                    pChoosedFile = NULL;
					if(pUploadFile)
						fclose(pUploadFile);
					if(pOutputFile)
						fclose(pOutputFile);
					curl_formfree(post);
					curl_easy_cleanup(curl);
					continue;
                }
			}
			else
			{
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
	}
	FILE *pOutputFile = fopen(OutputFilePath, "r");
	if(!pOutputFile)
	{
		perror("Couldn't read output file");
		return;
	}
	else
	{
		fseek(pOutputFile, 0 , SEEK_END);
		FileSize = ftell(pOutputFile);
		rewind(pOutputFile);
	}

	pGetline = (char*) malloc (sizeof(char)*512);
	if(pGetline == NULL)
		perror("Memory error");

	BatchLinks = (char*) realloc(BatchLinks,(UpDone*sizeof(UpFileUrl)));
	if(BatchLinks == NULL)
		perror("Memory error");

	i = 0;
	while(fgets (pGetline , 511 , pOutputFile) != NULL )
	{
		pBuff = strstr(pGetline , "url\":\"");
		pBuff += strlen("url\":\"");
		pBuff = strtok(pBuff , "\"");
		if(strstr(pBuff,"\\") != NULL )
			remove_char(pBuff, '\\');
		sprintf(UpFileUrl ,"http://a.pomf.se/%s" , pBuff);
		pBuff = strstr(pGetline , "name\":\"");
		pBuff += strlen("name\":\"");
		pBuff = strtok(pBuff , "\"");
		sprintf(FileName ,"%s" , pBuff);
		pomfit_log_upload(FileName , UpFileUrl);
		if(i == 0)
			sprintf(BatchLinks,"%s ", UpFileUrl);
		else
			sprintf(strchr(BatchLinks, '\0'),"%s ", UpFileUrl);
		++i;
	}

    sprintf(buff, "%d File%s(%.2f MB) in %dmin %dsec",
				UpDone,UpDone < 2 ? "" : "s",BatchSize ,
				(int)BatchTotalTime > 60 ? (int)BatchTotalTime/60 : 0,
				(int)BatchTotalTime%60 > 0 ? (int)BatchTotalTime%60 : 1);
#ifdef __linux__
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
							BatchLinks, strlen(BatchLinks));
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, buff);
	sprintf(strchr(buff, '\0'),"\nLinks copied to clipboard ");
	pomfit_notify_me(buff,"Batch Upload Finished");
	sprintf(buff,"Click to open all links.\nRight click to copy links.");
	gtk_widget_set_tooltip_text(link_but, buff);
	gtk_link_button_set_uri(GTK_LINK_BUTTON(link_but), BatchLinks);
	sprintf(buff,"%d/%d File%s Uploaded",
				UpDone, ListCount, UpDone < 2 ? "" : "s");
	gtk_button_set_label(GTK_BUTTON(link_but), buff);
	gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_NORMAL);
	gtk_button_set_label(GTK_BUTTON(up_but), "Upload");
	IsUploading = FALSE;
#elif _WIN32
    IsUploading = false;
    sprintf(strchr(buff, '\0'),"\nLink%s copied to clipboard ",UpDone < 2 ? "" : "s");

    size_t len = strlen(BatchLinks)+1;
    HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), BatchLinks, len);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
    pomfit_notify_me(buff,"Upload(s) finished!"); //
#endif
	fclose(pOutputFile);
	if(remove(OutputFilePath) != 0)
		perror("Error deleting temp file");
	pBuff = NULL;
	pChoosedFile = NULL;
	free(pGetline);
	for(i = 0 ; i < ListCount; ++i)
		apFilesPaths[i] = NULL;

}

void pomfit_take_ss (bool Upload , int mode) {
#ifdef __linux__
	char Command[128];
	char FileName[64];
	char FilePath[128];
	void *pFilePath[1];
	char NewFilePath[256];
	time_t now = time(NULL);
	char TimeString[80];
	struct tm  tstruct;
	tstruct = *localtime(&now);
	strftime(TimeString, sizeof(TimeString), "%Y-%m-%d-%H_%M_%S", &tstruct);
	sprintf(FileName , "%s_%s.png" , TimeString , Upload == true ? "pomfup" : "ss");
	
	sprintf(Command, "sleep 0.3 && scrot -s '%s'", FileName);
	system(Command);

	sprintf(FilePath,"%s/%s",HomeDir,FileName);
	pFilePath[0] = FilePath;
	if(Upload)
		pomfit_upload_file (pFilePath, 1);
	
	if(!KeepUploadedSS && Upload)
		remove(FilePath);
	else {
		sprintf(NewFilePath,"%s/%s", Upload == TRUE ? UploadedSsDir : SsDir, FileName);
		rename(FilePath,NewFilePath);
	}
	pFilePath[0] = NULL;
#elif _WIN32
    Sleep(400);                 // To prevent context menu on screenshots
    if(mode == 0) {           // Mode 0 - Capture Whole Screen
        CaptureFullScreen(Upload);
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

void pomfit_notify_me (const char *BotMsg, const char *TopMsg) {
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

int pomfit_curl_progress (void* ptr, double TotalToDownload, double NowDownloaded,
							double TotalToUpload, double NowUploaded) {
	time_t TimeNow = time(NULL);
	char UpStatus [64];
	double UpSizeLeft = TotalToUpload - NowUploaded;
	double UpPercent = NowUploaded / TotalToUpload;
	double UpSpeed = NowUploaded/(difftime(TimeNow,TimeUpStarted));
	double UpETA = UpSizeLeft/UpSpeed;
	sprintf(UpStatus, "%3.2f%% | %6.2f KiB/s | %5.0fs left",
							UpPercent*100, UpSpeed/1024, UpETA);
#ifdef __linux__
	if(gtk_button_get_relief(GTK_BUTTON(up_but)) == GTK_RELIEF_HALF)
		return 1;
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, UpStatus);
	while (gtk_events_pending ())
		gtk_main_iteration ();
#elif _WIN32

#endif
    return 0;
}

int pomfit_check_size(const char *File) {

	long long FileSize = 0;
	long long FileSizeLimit = 52428800;
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
						"Size limit: 50.00 MiB (52.42 MB)",
						(float)FileSize/1048576, (float)FileSize/1000000);
		pomfit_notify_me(Buff[1],Buff[0]);
        FileName = NULL;
		return 1;
	}
    FileName = NULL;
	return 0;
}

void pomfit_log_upload (char *FileName , char *url) {
	char LogPath[96];
	sprintf(LogPath, "%s/Pomfit_url_log.txt", HomeDir);
	FILE *fpUrlLog = fopen(LogPath, "a");
	if(!fpUrlLog)
	{
			perror("Error opening log file\n");
			return;
	}
	fprintf(fpUrlLog,"%s\t- %s\n", url, FileName);
	fclose(fpUrlLog);
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

void open_all_links(const char *keystring, void *user_data) {
#ifdef __linux__
	GdkScreen *DefScreen = gdk_screen_get_default();
	if(BatchLinks == NULL) {
		gtk_show_uri(DefScreen, "http://pomf.se", GDK_CURRENT_TIME, NULL);
		return;
	}
	char *pTok;
	char buffer[strlen(BatchLinks)+1];
	strcpy(buffer,BatchLinks);
	pTok = strtok(buffer, " ");
	while (pTok != NULL)
	{
		gtk_show_uri(DefScreen, pTok, GDK_CURRENT_TIME, NULL);
		pTok = strtok (NULL, " ");
	}
	return;
#elif _WIN32
    if(BatchLinks == NULL) {
        ShellExecute(NULL, "open", "http://pomf.se/", NULL , NULL , 0);
        return;
    }
    char *pTok;
	char buffer[strlen(BatchLinks)+1];
	strcpy(buffer,BatchLinks);
	pTok = strtok(buffer, " ");
	while (pTok != NULL)
	{
		ShellExecute(NULL, "open", pTok , NULL , NULL , 0);
		pTok = strtok (NULL, " ");
	}
	return;
#endif
}

void open_acc_page(const char *keystring, void *user_data)
{
#ifdef __linux__
	GdkScreen *DefScreen = gdk_screen_get_default();
	gtk_show_uri(DefScreen, "http://pomf.se/user/", GDK_CURRENT_TIME, NULL);
#elif _WIN32
    ShellExecute(NULL, "open", "http://pomf.se/user/", NULL , NULL , 0);
#endif
}

int pomfit_curl_login(void)
{
	char PostData[512];
	char LoginOutputFile[256];
	bool IsSuccess = false;
	sprintf(LoginOutputFile ,"%s/login_output" , ConfDir);
    sprintf(PostData,"email=%s&pass=%s",LoginEmail,LoginPass);

	CURL *curl;
	CURLcode res;

	FILE *pOutputFile;
	pOutputFile = fopen(LoginOutputFile, "a");
	if(!pOutputFile)
	{
		perror("Couldn't create output file");
		return IsSuccess;
	}
	curl = curl_easy_init();
	if(curl) {
	curl_easy_setopt(curl, CURLOPT_URL, "http://pomf.se/user/includes/api.php?do=login");
	curl_easy_setopt(curl, CURLOPT_USERAGENT,POMFIT_USER_AGENT);
	curl_easy_setopt(curl, CURLOPT_POST, TRUE);
	curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1 );
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, PostData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, pOutputFile);
	curl_easy_setopt(curl, CURLOPT_COOKIESESSION, TRUE);
	curl_easy_setopt(curl, CURLOPT_COOKIEJAR, CookieFile);
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, CookieFile);

	res = curl_easy_perform(curl);
	if(res != CURLE_OK)
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		return IsSuccess;
	}
	curl_easy_cleanup(curl);
	}
	fclose(pOutputFile);
	pOutputFile = fopen(LoginOutputFile, "r");
	if(!pOutputFile)
	{
		fprintf(stderr,"Couldn't create output file");
		return IsSuccess;
	}
	char *pGetLine = NULL;
	pGetLine = (char*) malloc (sizeof(char)*512);
	if(pGetLine == NULL)
		fprintf(stderr,"Memory error");
	while(fgets (pGetLine , 511 , pOutputFile) != NULL ) {
		if(strstr(pGetLine, "Moe Panel") != NULL) {
			IsSuccess = true;
			break;
		}
	}
	fclose(pOutputFile);
	free(pGetLine);
	if(remove(LoginOutputFile) != 0)
		perror("Error deleting temp file");
    if(IsSuccess == false)
        if(remove(CookieFile) != 0)
            fprintf(stderr,"Error deleting cookie file");

    return IsSuccess;
}

int pomfit_curl_logout(void)
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if(curl) {
	curl_easy_setopt(curl, CURLOPT_URL, "http://pomf.se/user/includes/api.php?do=logout");
	curl_easy_setopt(curl, CURLOPT_USERAGENT,POMFIT_USER_AGENT);
	curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1 );
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_COOKIESESSION, TRUE);
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, CookieFile);

	res = curl_easy_perform(curl);
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		return 1;
	}
	curl_easy_cleanup(curl);
	}
	if(remove(CookieFile) != 0)
		fprintf(stderr,"Error deleting cookie file");

    return 0;
}

void pomfit_mkdir(const char* Directory)
{
	int test_dir = chdir(Directory);
	chdir(HomeDir);
	if(test_dir != 0)
	{
#ifdef __linux__
		mkdir(Directory, 0777);
#elif _WIN32
		mkdir(Directory);
#endif
	}
}


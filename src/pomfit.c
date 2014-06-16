/*
 * This file is part of pomfit project.
 * pomfit - simple gtk image & file uploader for <http://pomf.se/>
 * Copyright (C) 2014  Senketsu <senketsu.dev@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
extern char ConfDir[128];
extern char CookieFile[160];
extern char LastUrl[64];
extern char LoggedEmail[256];
extern char *BatchLinks;
extern GdkPixbuf *p_icon;
extern GtkWidget *up_but;
extern GtkWidget *link_but;
extern GtkWidget *status_bar;
extern GtkWidget *entry_pass, *entry_email;
extern gboolean IsUploading;
extern gboolean IsBatchLinks;
extern gboolean IsLoggedIn;
time_t TimeUpStarted;

void quick_upload_pic(const char *keystring, void *user_data)
{
	char FileName[32];
	char FilePath[96];
	char buff[128] = "";
	char UpFileUrl[40];
	char OutputFilePath[96];
	char *pBuff = NULL;
	char *pGetline = NULL;
	size_t len = 0;
	sprintf(OutputFilePath ,"%s/curl_output.txt" , ConfDir);
	system("sleep 0.3");	
	system("cd && scrot -s '%Y-%m-%d_%H%M%S_pomfup.png'");
	FILE *fp = popen("cd && ls -Art *pomfup.png | tail -n 1","r");
	if(!fp)
		return;
	fscanf(fp, "%s",FileName);
	pclose(fp);
	sprintf(FilePath ,"%s/%s" , HomeDir ,FileName);
	CURL *curl;
	CURLcode res;
	struct curl_httppost *post = NULL;
	struct curl_httppost *last = NULL;
	FILE *pUploadFile;
	pUploadFile = fopen(FilePath, "rb");
	if(!pUploadFile)
	{
		perror("Invalid path to file");
		return;
	}
	FILE *pOutputFile;
	pOutputFile = fopen(OutputFilePath, "w"); 
	if(!pOutputFile)
	{
		perror("Couldn't create output file");
		if(pUploadFile)
			fclose(pUploadFile);
		return;
	}
	IsUploading = TRUE;
	curl = curl_easy_init();
	if(curl) 
	{
		curl_formadd(&post, &last,
				CURLFORM_COPYNAME, "files[]",
				CURLFORM_FILE, FilePath,
				CURLFORM_CONTENTTYPE, "application/octet-stream",
				CURLFORM_END);
		curl_easy_setopt(curl, CURLOPT_URL, "http://pomf.se/upload.php");
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_READDATA, pUploadFile);
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
			IsUploading = FALSE;
		}
		curl_formfree(post);
		curl_easy_cleanup(curl);
	}
	fclose(pOutputFile);
	if(pUploadFile)
		fclose(pUploadFile);
	pOutputFile = fopen(OutputFilePath, "r");
	if(!pOutputFile)
	{
		perror("Couldn't read output file");
		return;
	}
	getline(&pGetline, &len, pOutputFile);
	pBuff = strstr(pGetline , "url\":\"");
	pBuff += strlen("url\":\"");
	pBuff = strtok(pBuff , "\"");
	if(strstr(pBuff,"\\") != NULL )
		remove_char(pBuff, '\\');
	sprintf(UpFileUrl ,"http://a.pomf.se/%s" , pBuff);
	fclose(pOutputFile);
	sprintf(buff , "rm %s" , OutputFilePath);
	system(buff);
	sprintf(buff , "rm %s" , FilePath);
	system(buff);
	pBuff = NULL;
	pGetline = NULL;
	save_to_log(FileName , UpFileUrl);
	if(BatchLinks != NULL)
		free(BatchLinks);
	IsBatchLinks = FALSE;
	gtk_link_button_set_uri(GTK_LINK_BUTTON(link_but), UpFileUrl);
	gtk_button_set_label(GTK_BUTTON(link_but), FileName);
	sprintf(buff, "%s\n%s",FileName ,UpFileUrl);
	gtk_widget_set_tooltip_text(link_but, buff);
	notify_me(UpFileUrl,"Screencap upload finished...");
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, POMFIT_VERSION);
	strcpy(LastUrl, UpFileUrl);
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), 
							UpFileUrl, strlen(UpFileUrl));
	IsUploading = FALSE;
}

void curl_upload_file(gpointer **apFilesPaths , int ListCount)
{
	char *pBuff = NULL;
	char *pGetline = NULL;
	void *pChoosedFile = NULL;
	size_t len = 0;
	struct stat FileStat;
	long long FileSize = 0;
	long long FileSizeLimit = 52428800;
	double TotalTime;
	double BatchTotalTime;
	float BatchSize = 0;
	char buff[256];
	char OutputFilePath[96];
	char FileName[256];
	char UpFileUrl[64];
	sprintf(OutputFilePath ,"%s/curl_output.txt" , ConfDir);
	int i,Skipping = 0;
	int UpDone = 0;
	int Skip[20] = { 0 };
	if(ListCount == 0)
	{
		perror("No files selected");
		return;
	}
	for(i = 0 ; i < ListCount; ++i)
	{
		pChoosedFile = apFilesPaths[i];
		FILE *file_check;
		file_check = fopen(pChoosedFile, "r");
		if(!file_check)
		{
			Skip[i]=1;
			Skipping += 1;
			continue;
		}
		else
			fclose(file_check);
		stat(pChoosedFile, &FileStat);
		FileSize = FileStat.st_size;
		if(FileSize>FileSizeLimit)
		{
			if(ListCount == 1)
			{
				sprintf(buff, "File size: %0.2f MiB (%0.2f MB)\n"
							"Size limit: 50.00 MiB (52.42 MB)",
							(float)FileSize/1048576, (float)FileSize/1000000);
				notify_me(buff,"Error: File is too big !");
				return;
			}
			else
			{
				Skip[i]=1;
				Skipping += 1;
			}
		}
		if(Skipping > 0 && i+1 == ListCount)
		{
		  if(Skipping == ListCount)
		  {
			notify_me("Limit: 50.00 MiB (52.42 MB)","All files are too big !");
			return;
		  }
		  sprintf(buff,"Skipping %2d /%2d files\nLimit: 50.00 MiB (52.42 MB)",
					Skipping,ListCount);
		  notify_me(buff,"Error:Some files are too big");
		}
	}
	for(i = 0 ; i < ListCount; ++i)
	{
		if(Skip[i] == 1)
			continue;
		CURL *curl;
		CURLcode res;
		struct curl_httppost *post = NULL;
		struct curl_httppost *last = NULL;
		pChoosedFile = apFilesPaths[i];
		stat(pChoosedFile, &FileStat);
		FILE *pUploadFile = fopen(pChoosedFile, "rb");
		if(!pUploadFile)
		{
			perror("Invalid path to file");
			pChoosedFile = NULL;
			return;
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
		gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_NONE);
		gtk_button_set_label(GTK_BUTTON(up_but), "Cancel");
		TimeUpStarted = time(NULL);
		IsUploading = TRUE;
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
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, up_progress);
			curl_easy_setopt(curl, CURLOPT_READDATA, pUploadFile);
			curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)FileStat.st_size);
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
				if(ListCount == 1)
				{
					gtk_statusbar_push(GTK_STATUSBAR(status_bar),
										1,"Upload canceled..");
					gtk_button_set_relief(GTK_BUTTON(up_but),
											GTK_RELIEF_NORMAL);
					gtk_button_set_label(GTK_BUTTON(up_but), "Upload");
					IsUploading = FALSE;
					pChoosedFile = NULL;
					if(pUploadFile)
						fclose(pUploadFile);
					if(pOutputFile)
						fclose(pOutputFile);
					curl_formfree(post);
					curl_easy_cleanup(curl);
					return;
				}
				else
					continue;
			}
			else
			{
				fprintf(pOutputFile,"\n");
				BatchSize += (float)FileStat.st_size/1000000;;
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
	if(UpDone == 1)
	{
		IsBatchLinks = FALSE;
		getline(&pGetline, &len, pOutputFile);
		pBuff = strstr(pGetline , "url\":\"");
		pBuff += strlen("url\":\"");
		pBuff = strtok(pBuff , "\"");
		if(strstr(pBuff,"\\") != NULL )
				remove_char(pBuff, '\\');
		sprintf(UpFileUrl ,"http://a.pomf.se/%s" , pBuff);
		getline(&pGetline, &len, pOutputFile);
		pBuff = strstr(pGetline , "name\":\"");
		pBuff += strlen("name\":\"");
		pBuff = strtok(pBuff , "\"");
		sprintf(FileName ,"%s" , pBuff);
		save_to_log(FileName , UpFileUrl);
		sprintf(buff,"%.24s%s",FileName ,strlen(FileName) > 24 ? "..." : " ");
		gtk_link_button_set_uri(GTK_LINK_BUTTON(link_but), UpFileUrl);
		gtk_button_set_label(GTK_BUTTON(link_but), buff);
		notify_me(UpFileUrl,buff);
		strcpy(LastUrl, UpFileUrl);
		gtk_link_button_set_uri(GTK_LINK_BUTTON(link_but), UpFileUrl);
		sprintf(buff, "%2.2f MB file uploaded in %.0fs",
								(float)FileSize/1000000, TotalTime);
		gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, buff);
		sprintf(buff, "%s\n%s",FileName ,UpFileUrl);
		gtk_widget_set_tooltip_text(link_but, buff);
		gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), 
							UpFileUrl, strlen(UpFileUrl));
	}
	else
	{
		IsBatchLinks = TRUE;
		BatchLinks = realloc(BatchLinks,(UpDone*sizeof(UpFileUrl)));
		for(i = 0 ; i < UpDone; ++i)
		{
			void *pFileName = NULL;
			getline(&pGetline, &len, pOutputFile);
			pFileName = pGetline;
			pBuff = strstr(pGetline , "url\":\"");
			pBuff += strlen("url\":\"");
			pBuff = strtok(pBuff , "\"");
			if(strstr(pBuff,"\\") != NULL )
				remove_char(pBuff, '\\');
			sprintf(UpFileUrl ,"http://a.pomf.se/%s" , pBuff);
			pBuff = strstr(pFileName , "name\":\"");
			pBuff += strlen("name\":\"");
			pBuff = strtok(pBuff , "\"");
			sprintf(FileName ,"%s" , pBuff);
			save_to_log(FileName , UpFileUrl);
			if(i == 0)
				sprintf(BatchLinks,"%s ", UpFileUrl);
			else
				sprintf(strchr(BatchLinks, '\0'),"%s ", UpFileUrl);
		}
		gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), 
							BatchLinks, strlen(BatchLinks));
		sprintf(buff, "%d File%s(%.2f MB) in %dmin %dsec",
					UpDone,UpDone < 2 ? "" : "s",BatchSize ,
					(int)BatchTotalTime > 60 ? (int)BatchTotalTime/60 : 0,
					(int)BatchTotalTime%60 > 0 ? (int)BatchTotalTime%60 : 1);
		gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, buff);
		sprintf(strchr(buff, '\0'),"\nLinks copied to clipboard ");
		notify_me(buff,"Batch Upload Finished");
		sprintf(buff,"Click to open all links.\nRight click to copy links.");
		gtk_widget_set_tooltip_text(link_but, buff);
		strcpy(LastUrl, UpFileUrl);
		gtk_link_button_set_uri(GTK_LINK_BUTTON(link_but), BatchLinks);
		sprintf(buff,"%d/%d File%s Uploaded",
					UpDone, ListCount, UpDone < 2 ? "" : "s");
		gtk_button_set_label(GTK_BUTTON(link_but), buff);
	}
	fclose(pOutputFile);
	gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_NORMAL);
	gtk_button_set_label(GTK_BUTTON(up_but), "Upload");
	pBuff = NULL;
	pChoosedFile = NULL;
	pGetline = NULL;
	for(i = 0 ; i < ListCount; ++i)
		apFilesPaths[i] = NULL;
	sprintf(buff ,"rm %s" , OutputFilePath);
	system(buff);
	IsUploading = FALSE;
}	

int up_progress (void* ptr, double TotalToDownload, double NowDownloaded, 
							double TotalToUpload, double NowUploaded) 
{
	if(gtk_button_get_relief(GTK_BUTTON(up_but)) == GTK_RELIEF_HALF)
		return 1;
	time_t TimeNow = time(NULL);
	char UpStatus [64];
	double UpSizeLeft = TotalToUpload - NowUploaded;
	double UpPercent = NowUploaded / TotalToUpload;
	double UpSpeed = NowUploaded/(difftime(TimeNow,TimeUpStarted));
	double UpETA = UpSizeLeft/UpSpeed;
	sprintf(UpStatus, "%3.2f%% | %6.2f KiB/s | %5.0fs left", 
							UpPercent*100, UpSpeed/1024, UpETA);
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, UpStatus);
	while (gtk_events_pending ())
		gtk_main_iteration ();
	return 0;
}

void save_to_log (char *FileName , char *url)
{
	char LogPath[96];
	sprintf(LogPath, "%s/Pomfit_url_log.txt", HomeDir);
	FILE *url_log;
	url_log = fopen(LogPath, "a");
	if(!url_log)
	{
			perror("Error opening log file\n");
			return;
	}
	fprintf(url_log,"%s\t- %s\n", url, FileName);
	fclose(url_log);
}

void notify_me(char *output_url, char *FileName)
{
	NotifyNotification *Uploaded;
	notify_init("Uploaded");
	Uploaded = notify_notification_new(FileName, output_url , NULL);
	notify_notification_set_icon_from_pixbuf(Uploaded , p_icon);
	notify_notification_show (Uploaded, NULL);
	g_object_unref(G_OBJECT(Uploaded));
	notify_uninit();
}

void open_last_link(const char *keystring, void *user_data)
{
	if(IsBatchLinks)
	{
		open_all_links(NULL,NULL);
		return;
	}
	else
	{
		GdkScreen *DefScreen = gdk_screen_get_default();
		gtk_show_uri(DefScreen, LastUrl, GDK_CURRENT_TIME, NULL);
	}
}

gboolean open_all_links(GtkLinkButton *button, gpointer user_data)
{
	if(IsBatchLinks)
	{
		GdkScreen *DefScreen = gdk_screen_get_default();
		char *pTok;
		char buffer[strlen(BatchLinks)+1];
		strcpy(buffer,BatchLinks);
		pTok = strtok(buffer, " ");
		while (pTok != NULL)
		{
			gtk_show_uri(DefScreen, pTok, GDK_CURRENT_TIME, NULL);
			pTok = strtok (NULL, " ");
		}
		return TRUE;
	}
	else
	{
		open_last_link(NULL,NULL);
		return TRUE;
	}
}

void open_acc_page(const char *keystring, void *user_data)
{
	GdkScreen *DefScreen = gdk_screen_get_default();
	gtk_show_uri(DefScreen, "http://pomf.se/user/", GDK_CURRENT_TIME, NULL);
}

void take_screenshot(const char *keystring, void *user_data)
{
	system("sleep 0.3");
	system("cd && scrot -s '%Y-%m-%d_%H%M%S_cap.png'");
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


void curl_login(GtkWidget *widget,gpointer   data)
{
	const gchar *pLine_email;
	const gchar *pLine_pass;
	char PostData[512];
	char Buffer[256];
	char LoginOutputFile[256];
	sprintf(LoginOutputFile ,"%s/login_output" , ConfDir);
	
	pLine_email = gtk_entry_get_text(GTK_ENTRY(entry_email));
	pLine_pass = gtk_entry_get_text(GTK_ENTRY(entry_pass));
	sprintf(PostData,"email=%s&pass=%s",pLine_email,pLine_pass);
	strcpy(LoggedEmail,pLine_email);
	pLine_email = NULL;
	pLine_pass = NULL;
	
	
	CURL *curl;
	CURLcode res;
	
	FILE *pOutputFile;
	pOutputFile = fopen(LoginOutputFile, "a");
	if(!pOutputFile)
	{
		perror("Couldn't create output file");
		return;
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
		return;
	}
	curl_easy_cleanup(curl);
	}
	fclose(pOutputFile);
	pOutputFile = fopen(LoginOutputFile, "r");
	if(!pOutputFile)
	{
		perror("Couldn't create output file");
		return;
	}
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, pOutputFile) != -1) 
	{
		if(strstr(line, "Moe Panel") != NULL) {
			IsLoggedIn = TRUE;
			login_handle();
			break;
		}
		else {
			IsLoggedIn = FALSE;
			gtk_entry_set_text(GTK_ENTRY(entry_email), "Invalid Login");
			gtk_entry_set_text(GTK_ENTRY(entry_pass), "");
		}
	}
	fclose(pOutputFile);

	sprintf(Buffer,"rm %s",LoginOutputFile);
	system(Buffer);
}

void curl_logout(GtkWidget *widget,gpointer   data)
{
	
	char RMCookie[256];
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
	if(res != CURLE_OK)
	{ 
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		return;
	}
	curl_easy_cleanup(curl);
	}
	
	IsLoggedIn = FALSE;
	login_handle();
	sprintf(RMCookie,"rm %s",CookieFile);
	system(RMCookie);
}

void pomfit_mkdir(void)
{
	char string_mkdir[256];
	gboolean test_dir = chdir(ConfDir);
	chdir(HomeDir);
	if(test_dir != 0)
	{
		sprintf(string_mkdir, "mkdir %s",ConfDir);
		system(string_mkdir);
	}
}

void pomfit_save_state(void)
{
	char ConfFile [256];
	sprintf(ConfFile ,"%s/account_state" ,ConfDir);
	FILE *config = fopen(ConfFile , "w");
	if (config == NULL) 
	{
		perror("Couldn't create file\n");
		return;
	}
	if(IsLoggedIn)
	{
		fprintf(config, "IsLoggedIn=TRUE\n");
		fprintf(config, "Email=%s\n",LoggedEmail);
		
	}
	else
	{
		fprintf(config, "IsLoggedIn=FALSE\n");
	}
	fclose(config);
}

void pomfit_load_state(void)
{
	
	char *line = NULL;
	size_t len = 0;
	char *strtok_buffer = NULL;
	char ConfFile [256];
	sprintf(ConfFile ,"%s/account_state" ,ConfDir);
	FILE *config = fopen(ConfFile , "r");
	if (config == NULL) 
	{
		perror("Couldn't read file\n");
		return;
	}
	while (getline(&line, &len, config) != -1) 
	{
		if (strncmp(line, "IsLoggedIn=", strlen("IsLoggedIn=")) == 0) {
			strtok_r(line, "\n", &strtok_buffer);
			if (strstr(line, "TRUE") != NULL) 
				IsLoggedIn=TRUE;
			else
				IsLoggedIn=FALSE;
		}
		else if (strncmp(line, "Email=", strlen("Email=")) == 0) 
		{
			strtok_r(line, "\n", &strtok_buffer);
			strcpy(LoggedEmail, line + strlen("Email="));
		}
	}
	fclose(config);
}

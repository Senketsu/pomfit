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

extern char home_dir[64];
extern char last_upload[64];
extern GtkWidget *sel_but;
extern GtkWidget *up_but;
extern GtkWidget *link_but;
extern GdkPixbuf *p_icon;
extern GtkWidget *status_bar;
gboolean is_uploading = FALSE;
time_t time2;

void quick_upload_pic(const char *keystring, void *user_data)
{
	char scan[64];
	char file[128];
	char buff[144];
	char output_url[64];
	char temp_file[96];
	char *pBuff = NULL;
	char *pGetline = NULL;
	ssize_t len = 0;
	sprintf(temp_file ,"%s/curl_output.txt" , home_dir);
	system("sleep 0.3");	
	system("cd && scrot -s '%Y-%m-%d_%H%M%S_pomfup.png'");
	
	FILE *fp = popen("cd && ls -Art *pomfup.png | tail -n 1","r");
	if(!fp)
		return;
	fscanf(fp, "%s",scan);
	pclose(fp);
	sprintf(file ,"%s/%s" , home_dir ,scan);
	CURL *curl;
	CURLcode res;
	struct curl_httppost *post = NULL;
	struct curl_httppost *last = NULL;
	FILE *fd;
	fd = fopen(file, "rb");
	if(!fd)
	{
		perror("Error opening file to upload ..");
		return;
	}
	FILE *output;
	output = fopen(temp_file, "w"); 
	if(!output)
	{
		perror("Error opening file to write");
		return;
	}
	curl = curl_easy_init();
	if(curl) 
	{
		curl_formadd(&post, &last,
				CURLFORM_COPYNAME, "files[]",
				CURLFORM_FILE, file,
				CURLFORM_CONTENTTYPE, "application/octet-stream",
				CURLFORM_END);
		curl_easy_setopt(curl, CURLOPT_URL, "http://pomf.se/upload.php");
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_READDATA, fd);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, POMFIT_USER_AGENT);
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{ 
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
			perror("Curl failed to upload file");
		}
		curl_formfree(post);
		curl_easy_cleanup(curl);
	}
	fclose(output);
	if(fd)
		fclose(fd);
	output = fopen(temp_file, "r");
	if(!output)
	{
		perror("failed to open output file");
		return;
	}
	getline(&pGetline, &len, output);
	pBuff = strstr(pGetline , "url\":\"");
	pBuff += strlen("url\":\"");
	pBuff = strtok(pBuff , "\"");
	sprintf(output_url ,"http://a.pomf.se/%s" , pBuff);
	gtk_link_button_set_uri(GTK_LINK_BUTTON(link_but), output_url);
	gtk_button_set_label(GTK_BUTTON(link_but), output_url);
	fclose(output);
	sprintf(buff , "rm %s" , temp_file);
	system(buff);
	sprintf(buff , "rm %s" , file);
	system(buff);
	pBuff = NULL;
	pGetline = NULL;
	save_to_log(scan , output_url);
	notify_me(output_url,"Screencap upload finished...");
	strcpy(last_upload, output_url);
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), 
							output_url, strlen(output_url));
}

void curl_upload_file(char *file_path , gboolean user_data)
{
	char *pBuff = NULL;
	char *pGetline = NULL;
	ssize_t len = 0;
	struct stat file_stat;
	long long size = 0;
	long long size_limit = 52428800;
	double total_time;
	char *pChoosedFile = NULL;
	char buff[256] = "";
	char temp_file[96];
	char FileName[256];
	char output_url[64];
	sprintf(temp_file ,"%s/curl_output.txt" , home_dir);
	pChoosedFile = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(sel_but));
	if(is_uploading)
	{
		gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_HALF);
		gtk_button_set_label(GTK_BUTTON(up_but), "Upload");
		return;
	}/*
	if(file_path == NULL & !is_uploading)
	{
		stat(pChoosedFile, &file_stat);
		size = file_stat.st_size;
		if(size>size_limit)
		{
			sprintf(buff, "File size: %0.2f MiB (%0.2f MB)\n"
							"Size limit: 50.00 MiB (52.42 MB)",
								(float)size/1048576, (float)size/1000000);
			notify_me(buff,"Error: File size too big !");
			pChoosedFile=NULL;
			gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(sel_but));
			return;
		}
	}*/
	if(user_data == TRUE)
	{
		stat(file_path, &file_stat);
		size = file_stat.st_size;
		if(size>size_limit)
		{
			sprintf(buff, "File size: %0.2f MiB (%0.2f MB)\n"
							"Size limit: 50.00 MiB (52.42 MB)",
								(float)size/1048576, (float)size/1000000);
			notify_me(buff,"Error: File size too big !");
			return;
		}
	}
	if(user_data == FALSE && pChoosedFile != NULL)
	{
		stat(pChoosedFile, &file_stat);
		size = file_stat.st_size;
		if(size>size_limit)
		{
			sprintf(buff, "File size: %0.2f MiB (%0.2f MB)\n"
							"Size limit: 50.00 MiB (52.42 MB)",
								(float)size/1048576, (float)size/1000000);
			notify_me(buff,"Error: File size too big !");
			pChoosedFile=NULL;
			gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(sel_but));
			return;
		}
	}
	CURL *curl;
	CURLcode res;
	struct curl_httppost *post = NULL;
	struct curl_httppost *last = NULL;
	FILE *fd;
	if(pChoosedFile==NULL)
		fd = fopen(file_path, "rb");
	else
		fd = fopen(pChoosedFile, "rb");
	if(!fd)
	{
		perror("Error opening file to upload");
		return;
	}
	
	FILE *output;
	output = fopen(temp_file, "w");
	if(!output)
	{
		perror("Error opening file to write");
		return;
	}
	if(gtk_button_get_relief(GTK_BUTTON(up_but)) == GTK_RELIEF_NORMAL)
	{
		gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_NONE);
		gtk_button_set_label(GTK_BUTTON(up_but), "Cancel");
	}
	time2 = time(NULL);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(sel_but));
	is_uploading = TRUE;
	curl = curl_easy_init();
	if(curl) 
	{
		if(pChoosedFile==NULL)
		{
			curl_formadd(&post, &last,
				CURLFORM_COPYNAME, "files[]",
				CURLFORM_FILE, file_path,
				CURLFORM_CONTENTTYPE, "application/octet-stream",
				CURLFORM_END);
		}
		else
		{
			curl_formadd(&post, &last,
				CURLFORM_COPYNAME, "files[]",
				CURLFORM_FILE, pChoosedFile,
				CURLFORM_CONTENTTYPE, "application/octet-stream",
				CURLFORM_END);
		}
		curl_easy_setopt(curl, CURLOPT_URL, "http://pomf.se/upload.php");
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, up_progress);
		curl_easy_setopt(curl, CURLOPT_READDATA, fd);
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)file_stat.st_size);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, POMFIT_USER_AGENT);
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{ 
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
			gtk_statusbar_push(GTK_STATUSBAR(status_bar),1,"Upload canceled..");
			gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_NORMAL);
			gtk_button_set_label(GTK_BUTTON(up_but), "Upload");
			is_uploading = FALSE;
			return;
		}
		else
			curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
		
		curl_formfree(post);
		curl_easy_cleanup(curl);
	}
	fclose(output);
	if(fd)
		fclose(fd);
	output = fopen(temp_file, "r");
	if(!output)
	{
		perror("failed to open output file");
		return;
	}
	getline(&pGetline, &len, output);
	pBuff = strstr(pGetline , "url\":\"");
	pBuff += strlen("url\":\"");
	pBuff = strtok(pBuff , "\"");
	sprintf(output_url ,"http://a.pomf.se/%s" , pBuff);
	getline(&pGetline, &len, output);
	pBuff = strstr(pGetline , "name\":\"");
	pBuff += strlen("name\":\"");
	pBuff = strtok(pBuff , "\"");
	sprintf(FileName ,"%s" , pBuff);
	fclose(output);
	sprintf(buff ,"rm %s" , temp_file);
	system(buff);
	pBuff = NULL;
	pChoosedFile = NULL;
	pGetline = NULL;
	file_path = NULL;
	is_uploading = FALSE;
	gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_NORMAL);
	gtk_button_set_label(GTK_BUTTON(up_but), "Upload");
	sprintf(buff, "%.24s%s", FileName , strlen(FileName) > 24 ? "..." : " ");
	gtk_link_button_set_uri(GTK_LINK_BUTTON(link_but), output_url);
	gtk_button_set_label(GTK_BUTTON(link_but), buff);
	save_to_log(FileName , output_url);
	notify_me(output_url,buff);
	strcpy(last_upload, output_url);
	sprintf(buff, "%2.2f MiB file uploaded in %.0fs",
								(float)size/1048576, total_time);
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, buff);
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), 
							output_url, strlen(output_url));
}

int up_progress (void* ptr, double TotalToDownload, double NowDownloaded, 
							double TotalToUpload, double NowUploaded) 
{
	if(gtk_button_get_relief(GTK_BUTTON(up_but)) == GTK_RELIEF_HALF)
		return 1;
	time_t time1 = time(NULL);
	char up_status [64];
	double up_size_left = TotalToUpload - NowUploaded;
	double fup = NowUploaded / TotalToUpload;
	double up_speed = NowUploaded/(difftime(time1,time2));
	double up_estimated = up_size_left/up_speed;
	sprintf(up_status, "%3.2f%% | %6.2f KiB/s | %5.0fs left", 
							fup*100, up_speed/1024, up_estimated);
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, up_status);
	while (gtk_events_pending ())
		gtk_main_iteration ();
	printf("\r");
	fflush(stdout);
	
}

void save_to_log (char *FileName , char *url)
{
	char log_path[96];
	sprintf(log_path, "%s/Pomfit_url_log.txt", home_dir);
	FILE *url_log;
	url_log = fopen(log_path, "a");
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
	char command[96];
	sprintf(command , "xdg-open %s &", last_upload);
	system(command);
}

void take_screenshot(const char *keystring, void *user_data)
{
	system("sleep 0.3");
	system("cd && scrot -s '%Y-%m-%d_%H%M%S_cap.png'");
}

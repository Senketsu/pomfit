/*
 * This file is part of pomfit project.
 * pomfit - simple gtk image & file uploader for <http://pomf.se/>
 * Copyright (C) 2014  Miroslav Dulka <senketsu.dev@gmail.com>
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
extern GtkWidget *link_but;
extern GdkPixbuf *p_icon;

void quick_upload_pic(const char *keystring, void *user_data)
{
	char scan[200];
	char file[512];
	char buff[512];
	strcpy(file , home_dir);
	strcat(file , "/");
	system("sleep 0.3");	
	system("cd && scrot -s '%Y-%m-%d_%H%M%S_pomfup.png'");
	FILE *fp = popen("cd && ls -Art *pomfup.png | tail -n 1","r");
	if(!fp)
		return;
	fscanf(fp, "%s",scan);
	pclose(fp);
	strcat(file , scan);
	CURL *curl;
	CURLcode res;
	char output_url[256]= "http://a.pomf.se/";
	char temp_file[256];
	strcpy(temp_file , home_dir);
	strcat(temp_file , "/curl_output.txt");
	char *pBuff;
	struct curl_httppost *post = NULL;
	struct curl_httppost *last = NULL;
	FILE *fd;
	fd = fopen(file, "rb");
	if(!fd)
	{
		notify_error("Error opening file to upload ..");
		return;
	}
	FILE *output;
	output = fopen(temp_file, "w"); 
	if(!output)
	{
		notify_error("Error opening file to write");
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
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.34.0");
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{ 
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
			notify_error("Curl failed to upload file");
		}
		curl_formfree(post);
		curl_easy_cleanup(curl);
	}
	fclose(output);
	output = fopen(temp_file, "r");
	fscanf(output ,"%s", buff);
	pBuff=strstr(buff, "url\":\"");
	pBuff += strlen("url\":\"");
	pBuff=strtok(pBuff, "\"");
	strcat(output_url, pBuff);
	gtk_link_button_set_uri(GTK_LINK_BUTTON(link_but), output_url);
	gtk_button_set_label(GTK_BUTTON(link_but), output_url);
	fclose(output);
	strcpy(buff, "rm ");
	strcat(buff,temp_file);
	system(buff);
	if(fd)
		fclose(fd);
	strcpy(buff, "rm ");
	strcat(buff,file);
	system(buff);
	pBuff = NULL;
	NotifyNotification *Uploaded;
	notify_init("Uploaded");
	Uploaded = notify_notification_new("Upload finished", output_url , NULL);
	notify_notification_set_icon_from_pixbuf(Uploaded , p_icon);
	notify_notification_show (Uploaded, NULL);
	g_object_unref(G_OBJECT(Uploaded));
	notify_uninit();
	strcpy(last_upload, output_url);
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), 
							output_url, strlen(output_url));
}

void curl_upload_file(void)
{
	CURL *curl;
	CURLcode res;
	char *pChoosedFile = NULL;
	char output_url[64]= "http://a.pomf.se/";
	char temp_file[256];
	strcpy(temp_file , home_dir);
	strcat(temp_file , "/curl_output.txt");
	char buff[512];
	char *pBuff;
	pChoosedFile = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(sel_but));
	if(!pChoosedFile)
		return;
	struct curl_httppost *post = NULL;
	struct curl_httppost *last = NULL;
	FILE *fd;
	fd = fopen(pChoosedFile, "rb");
	if(!fd)
	{
		notify_error("Error opening file to upload");
		return;
	}
	FILE *output;
	output = fopen(temp_file, "w");
	if(!output)
	{
		notify_error("Error opening file to write");
		return;
	}
	curl = curl_easy_init();
	if(curl) 
	{
		curl_formadd(&post, &last,
				CURLFORM_COPYNAME, "files[]",
				CURLFORM_FILE, pChoosedFile,
				CURLFORM_CONTENTTYPE, "application/octet-stream",
				CURLFORM_END);
		curl_easy_setopt(curl, CURLOPT_URL, "http://pomf.se/upload.php");
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_READDATA, fd);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.34.0");
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{ 
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
			notify_error("Curl failed to upload file");
		}
		gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(sel_but));
		curl_formfree(post);
		curl_easy_cleanup(curl);
	}
	fclose(output);
	output = fopen(temp_file, "r");
	fscanf(output ,"%s", buff);
	pBuff=strstr(buff, "url\":\"");
	pBuff += strlen("url\":\"");
	pBuff=strtok(pBuff, "\"");
	strcat(output_url, pBuff);
	gtk_link_button_set_uri(GTK_LINK_BUTTON(link_but), output_url);
	gtk_button_set_label(GTK_BUTTON(link_but), output_url);
	fclose(output);
	strcpy(buff, "rm ");
	strcat(buff,temp_file);
	system(buff);
	if(fd)
		fclose(fd);
	pBuff = NULL;
	pChoosedFile = NULL;
	NotifyNotification *Uploaded;
	notify_init("Uploaded");
	Uploaded = notify_notification_new("Upload finished", output_url , NULL);
	notify_notification_set_icon_from_pixbuf(Uploaded , p_icon);
	notify_notification_show (Uploaded, NULL);
	g_object_unref(G_OBJECT(Uploaded));
	notify_uninit();
	strcpy(last_upload, output_url);
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), 
							output_url, strlen(output_url));
}

void notify_error(char *str_error)
{
	NotifyNotification *Error;
	notify_init("Error");
	Error = notify_notification_new ("Error occurred", str_error , NULL );
	notify_notification_set_icon_from_pixbuf(Error , p_icon);
	notify_notification_set_timeout(Error,3000);
	notify_notification_show (Error, NULL);
	g_object_unref(G_OBJECT(Error));
	notify_uninit();
}

void open_last_link(const char *keystring, void *user_data)
{
	char command[128]="xdg-open ";
	strcat(command, last_upload);
	strcat(command, " &");
	system(command);
}

void take_screenshot(const char *keystring, void *user_data)
{
	system("sleep 0.3");	
	system("scrot -s '%Y-%m-%d_%H%M%S_cap.png' -e 'mv $f ~/Pictures/'");
}
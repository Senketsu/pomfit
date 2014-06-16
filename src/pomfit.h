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

#ifndef POMFIT_H
	#define POMFIT_H
	#define POMFIT_USER_AGENT "pomfit/0.0.6"
	#define POMFIT_VERSION "\"Pomf it !\" Uploader version 0.0.6"
	void quick_upload_pic(const char *keystring, void *user_data);
	void curl_upload_file(gpointer **apFilesPaths , int ListCount);
	void notify_me(char *output_url, char *FileName);
	void take_screenshot(const char *keystring, void *user_data);
	void open_last_link(const char *keystring, void *user_data);
	gboolean open_all_links(GtkLinkButton *button, gpointer user_data);
	void save_to_log (char* FileName , char* url);
	void open_acc_page(const char *keystring, void *user_data);
	void remove_char(char *str, char garbage);
	void pomfit_mkdir(void);
	void pomfit_load_state(void);
	void pomfit_save_state(void);
	void curl_logout(GtkWidget *widget,gpointer user_data);
	void curl_login(GtkWidget *widget,gpointer user_data);
	int up_progress (void* ptr, double TotalToDownload, double NowDownloaded, 
							double TotalToUpload, double NowUploaded);
#endif


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

#define POMFIT_USER_AGENT "pomfit/0.1.0"
#define POMFIT_VERSION "\"Pomf it !\" v.0.1.0"

#ifdef __cplusplus
extern "C"
{
#endif
char SsDir[512];
char ConfDir[512];
char LoginPass[256];
char LoginEmail[256];
char CookieFile[256];
char UploadedSsDir[512];
char *BatchLinks;
bool IsUploading;
bool KeepUploadedSS;
time_t TimeUpStarted;

int pomfit_curl_login(void);
int pomfit_curl_logout(void);
void remove_char(char *str, char garbage);
void pomfit_mkdir(const char* Directory);
void pomfit_upload_ss (const char *keystring, void *user_data);
void pomfit_take_ss (bool Upload , int mode) ;
void pomfit_notify_me (const char *TopMsg, const char *BotMsg);
void pomfit_upload_file(void **apFilesPaths , int ListCount);
void pomfit_log_upload (char *FileName , char *url);

int pomfit_check_size(const char *File);
int pomfit_curl_progress (void* ptr, double TotalToDownload, double NowDownloaded,
							double TotalToUpload, double NowUploaded);

void open_all_links(const char *keystring, void *user_data);
void open_acc_page(const char *keystring, void *user_data);


#ifdef __cplusplus
}
#endif


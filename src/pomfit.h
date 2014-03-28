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
	void quick_upload_pic(const char *keystring, void *user_data);
	void curl_upload_file(char *file_path);
	void notify_me(char *output_url, char *FileName);
	void take_screenshot(const char *keystring, void *user_data);
	void open_last_link(const char *keystring, void *user_data);
	void save_to_log (char* FileName , char* url);
#endif


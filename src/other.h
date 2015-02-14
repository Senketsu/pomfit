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

#ifndef POMFIT_OTHER_H
	#define POMFIT_OTHER_H
#ifdef __cplusplus
extern "C"
{
#endif
	int pomfit_check_size(const char *File, int LimitMB);
	void pomfit_notify (const char *BotMsg, const char *TopMsg);
	void pomfit_open_links(const char *keystring, void *user_data);

	void remove_char(char *str, char garbage);
	char *double_char (char *String, char Char);
	char *replace_str(char *str, char *orig, const char *rep, int start);

	int pomfit_regex_compile (char *RegExString);
	int pomfit_regex_search (char *Path, char *Regex, bool Delete);
#ifdef __cplusplus
}
#endif
#endif

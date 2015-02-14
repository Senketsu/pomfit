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
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include "log.h"

bool ColorizeOutput = false;
extern char DirHome[64];
extern char DirConf[512];

void log_error(bool IsError, const char *format,...) {

	char Status[512] = "";
	va_list va_Args;
	va_start(va_Args, format);
	vsnprintf(Status, sizeof(Status), format ,va_Args);
	va_end(va_Args);

	if(ColorizeOutput) {

		if(IsError)
			fprintf(stdout, "\033[1;%dm%s\033[0m\n", FG_RED, Status);
		else if(strstr(Status,"*Notice") != NULL)
			fprintf(stdout, "\033[1;%dm%s\033[0m\n", FG_BLUE, Status);
		else if(strstr(Status,"**Warning") != NULL)
			fprintf(stdout, "\033[1;%dm%s\033[0m\n", FG_YELLOW, Status);
		else
			fprintf(stdout, "%s\n", Status);

	} else
		fprintf(stdout, "%s\n", Status);

	if(IsError) {

		char TimeString[128];
		time_t curtime;
		time(&curtime);
		strftime(TimeString, 127, "%Y-%m-%d - %H:%M:%S", localtime(&curtime));

		FILE *ErrorRep = NULL;
		char PathErrorLog[128];
		sprintf(PathErrorLog,"%s/Error.log",DirConf);
		if((ErrorRep = fopen(PathErrorLog,"a")) != NULL) {

			fprintf(ErrorRep,"%s | %s\n",TimeString,Status);
			fflush(ErrorRep);
		}
		fclose(ErrorRep);
	}
}
/// Deprecated
void log_upload (char *FileName , char *FileUrl) {

	char PathUpLog[96];
	sprintf(PathUpLog, "%s/Pomfit_Upload_Log.txt", DirHome);
	FILE *fpUpLog = fopen(PathUpLog, "a");
	if(!fpUpLog) {
			log_error(true,"***Error: Couldn't write to file '%s'",PathUpLog);
			return;
	}
	fprintf(fpUpLog,"%s\t- %s\n", FileUrl, FileName);
	fclose(fpUpLog);
}

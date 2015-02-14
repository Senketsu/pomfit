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
#include <stdarg.h>
#include <stdbool.h>
#include <sqlite3.h>
#ifdef _WIN32
    #include <windows.h>
#endif
#include "database.h"
#include "log.h"

extern char PathDB[512];
extern void manager_tree_clear(void);
extern void manager_tree_add_row (char *filename, char *up_url, char *del_url, char *file_path);

sqlite3 *PomfitDB;
sqlite3_stmt *PomfitDB_stmt;

int pomfit_db_init (void) {

	int rc = 0;
	rc = sqlite3_open(PathDB, &PomfitDB);
	if( rc ) {
		log_error(true,"***Error SQLite: %s", sqlite3_errmsg(PomfitDB));
		return 1;
	}
	SQLite.execute 	= pomfit_db_execute;
	SQLite.execute2 	= pomfit_db_execute2;
	SQLite.fetchone_c	= pomfit_db_fetch_one_col;
	return 0;
}

int pomfit_db_create (void) {

	int rc = 0;
	rc = sqlite3_open(PathDB, &PomfitDB);
	if( rc ) {
		log_error(true,"***Error SQLite: Create - %s", sqlite3_errmsg(PomfitDB));
		return 1;
	}
	pomfit_db_execute("CREATE TABLE uploads (id integer primary key autoincrement, "
					"file_name text , url varchar(128), del_url varchar(128), file_path text,unique (url))");
	#ifdef __linux__
	sleep(1);
	#elif _WIN32
	Sleep(1000);
	#endif
	pomfit_db_execute("CREATE TABLE profiles (id integer primary key autoincrement, "
					"name text , url varchar(128), form_name varchar(128), form_type varchar(128), "
					"form_file_name varchar(128), size_limit integer, send_cookie integer, send_name integer, "
					"send_form_name integer, b_prep integer, regex_url text, regex_del text, "
					"prep_string varchar(128) ,unique (name))");
	#ifdef __linux__
	sleep(1);
	#elif _WIN32
	Sleep(1000);
	#endif
	pomfit_db_execute2("INSERT OR REPLACE INTO profiles "
			"(name, url, form_name, form_type ,form_file_name, size_limit, "
			"send_cookie, send_name, send_form_name, b_prep, regex_url, regex_del, prep_string) "
			"VALUES (\"Pomf.se\", \"http://pomf.se/upload.php\", \"files[]\","
			" \"application/octet-stream\", \"pomfit_upload\" , 52 , 1, 1, 0, 1, ? , ? , \"http://a.pomf.se/\" )",
			2 , "url\":\"([^ : \"]+)", "del_url\":\"([^ : \"]+)" );

	#ifdef __linux__
	sleep(1);
	#elif _WIN32
	Sleep(1000);
	#endif
	pomfit_db_execute("INSERT OR REPLACE INTO profiles "
			"(name, url, form_name, form_type ,form_file_name, size_limit, "
			"send_cookie, send_name, send_form_name,b_prep, regex_url, regex_del,prep_string) "
			"VALUES (\"Uguu.se\", \"http://uguu.se/api.php?d=upload\", \"file\","
			" \"application/octet-stream\", \"pomfit_upload\" , 150 , 1, 1, 0,0, \" \", \" \", \" \")");


	return 0;
}

int pomfit_db_delete (void) {

	int rc = 0;
	sqlite3_finalize(PomfitDB_stmt);
	rc = sqlite3_close(PomfitDB);
	if( rc ) {
		log_error(true,"***Error SQLite: %s", sqlite3_errmsg(PomfitDB));
		return 1;
	} else {
		if(remove(PathDB) != 0) {
			log_error(true,"***Error: Removing pomfit DB file failed");
			return 2;
		} else
			pomfit_db_create();
	}
	return 0;
}

int pomfit_db_execute2(const char *sQuery,int argc,...) {

	va_list va_Args;
	if(PomfitDB == NULL)
		if(pomfit_db_init()!= 0)
			return -1;
		
	if(sqlite3_prepare_v2(PomfitDB,sQuery,-1,&PomfitDB_stmt,0) != SQLITE_OK) {
		log_error(true,"***Error SQLite: %s",sqlite3_errmsg(PomfitDB));
		return -1;
	}
	int i ;
	const char *pArgs[argc];
    va_start(va_Args, argc);
	for( i = 0 ; i < argc; i++ )
    {
        pArgs[i] = va_arg( va_Args, char* );
        sqlite3_bind_text(PomfitDB_stmt, i+1, pArgs[i], -1, SQLITE_TRANSIENT);
    }
    va_end(va_Args);

	SQLite.iCol = sqlite3_column_count(PomfitDB_stmt);
	if(strstr(sQuery,"SELECT") == NULL) {
		int rv;
		while(rv  != SQLITE_DONE)
			rv = sqlite3_step(PomfitDB_stmt);
		sqlite3_finalize(PomfitDB_stmt);
	}
	for( i = 0 ; i < argc; i++ )
		pArgs[i] = NULL;

	return 0;
}

int pomfit_db_execute(const char *format,...) {

	va_list va_Args;
	char sQuery[4096] = "";

	if(PomfitDB == NULL)
		if(pomfit_db_init()!= 0)
			return -1;

	va_start(va_Args, format);
	vsnprintf(sQuery, sizeof(sQuery), format ,va_Args);
	va_end(va_Args);

	if(sqlite3_prepare_v2(PomfitDB,sQuery,-1,&PomfitDB_stmt,0) != SQLITE_OK) {
		log_error(true,"***Error SQLite: %s",sqlite3_errmsg(PomfitDB));
		return -1;
	}

	SQLite.iCol = sqlite3_column_count(PomfitDB_stmt);
	if(strstr(sQuery,"SELECT") == NULL) {
		int rv;
		while(rv  != SQLITE_DONE)
			rv = sqlite3_step(PomfitDB_stmt);
		sqlite3_finalize(PomfitDB_stmt);
	}


	return 0;
}

int pomfit_db_fill_tree (void) {

	int i ,rv = 0;
    char ColumnCont[5][1024];
	manager_tree_clear();
	while(true) {
		rv = sqlite3_step(PomfitDB_stmt);
		if(rv == SQLITE_ROW) {
			for( i = 0 ; i < SQLite.iCol ; i++ ) {
				snprintf(ColumnCont[i],sizeof(ColumnCont[i]), "%s",sqlite3_column_text(PomfitDB_stmt,i));
			}
			manager_tree_add_row(ColumnCont[1],ColumnCont[2],ColumnCont[3],ColumnCont[4]);

		} else if(rv == SQLITE_DONE) {

			sqlite3_finalize(PomfitDB_stmt);
			return 0;
		} else
			return -1;
	}
}

const char *pomfit_db_fetch_one_col(char *column) {

	int i  , rv = 0;
	const unsigned char *val = NULL;
	rv = sqlite3_step(PomfitDB_stmt);
	if(rv == SQLITE_ROW) {

		for( i = 0 ; i < SQLite.iCol ; i++ ) {

			if(strcmp(column, sqlite3_column_name(PomfitDB_stmt,i)) == 0) {
				val = sqlite3_column_text(PomfitDB_stmt,i);
                return (const char*)val;
            }
		}
		return NULL;

	} else if(rv == SQLITE_DONE) {
        sqlite3_finalize(PomfitDB_stmt);
		return NULL;
	} else
		return NULL;
}

int pomfit_db_profiles_select(UpProfile *Profile) {

	int i ,rv = 0;
	const unsigned  char *pValue = NULL;
	const char *pCol = NULL;
	rv = sqlite3_step(PomfitDB_stmt);
	if(rv == SQLITE_ROW) {
		for( i = 0 ; i < SQLite.iCol ; i++ ) {

			pCol = sqlite3_column_name(PomfitDB_stmt,i);
			pValue = sqlite3_column_text(PomfitDB_stmt,i);
			if(strcmp("name", pCol) == 0)
				snprintf (Profile->Name, sizeof(Profile->Name), "%s",pValue);
			if(strcmp("url", pCol) == 0)
				snprintf (Profile->URL, sizeof(Profile->URL), "%s",pValue);
			if(strcmp("form_name", pCol) == 0)
				snprintf (Profile->FormName, sizeof(Profile->FormName), "%s",pValue);
			if(strcmp("form_type", pCol) == 0)
				snprintf (Profile->FormType, sizeof(Profile->FormType), "%s",pValue);
			if(strcmp("form_file_name", pCol) == 0)
				snprintf (Profile->FormFileName, sizeof(Profile->FormFileName), "%s",pValue);
			if(strcmp("size_limit", pCol) == 0)
				Profile->MaxSize = atoi((const char *)pValue);
			if(strcmp("send_cookie", pCol) == 0)
				Profile->bCookie = atoi((const char *)pValue);
			if(strcmp("send_name", pCol) == 0)
				Profile->bNames = atoi((const char *)pValue);
			if(strcmp("send_form_name", pCol) == 0)
				Profile->bFormName = atoi((const char *)pValue);
			if(strcmp("b_prep", pCol) == 0)
				Profile->bPrepend = atoi((const char *)pValue);
			if(strcmp("regex_url", pCol) == 0)
				snprintf (Profile->RegEx_URL, sizeof(Profile->RegEx_URL), "%s",pValue);
			if(strcmp("regex_del", pCol) == 0)
				snprintf (Profile->RegEx_DEL, sizeof(Profile->RegEx_DEL), "%s",pValue);
			if(strcmp("prep_string", pCol) == 0)
				snprintf (Profile->PrepString, sizeof(Profile->PrepString), "%s",pValue);
		}
		pValue = NULL;
		pCol = NULL;
        sqlite3_finalize(PomfitDB_stmt);
		return 0;

	} else if(rv == SQLITE_DONE) {

        sqlite3_finalize(PomfitDB_stmt);
		return 0;
	} else
		return -1;
}

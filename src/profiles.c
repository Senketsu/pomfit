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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <gtk/gtk.h>
#include "profiles.h"
#include "database.h"

extern UpProfile ActiveProfile;
extern char ActProfile[256];
extern GtkWidget *win_set_main, *com_upm_pl, *lab_profile;
GtkWidget *win_upm_main;
GtkWidget *entry_api, *entry_form_type, *entry_form_name, *entry_fsize_limit, *entry_prof_name;
GtkWidget *sw_cookies, *sw_filename, *sw_filename_u, *sw_reout_prep, *entry_form_fname;
GtkWidget *entry_regex_url, *entry_regex_del, *entry_reout_prep;


void profiles_window_create (void) {

	if(win_upm_main != NULL) {
		gtk_widget_hide(win_upm_main);
		gtk_widget_show(win_upm_main);
		return;
	}

	GtkWidget *but_set_close, *but_set_add, *but_set_tem,*vbox_upm_m, *lab_e;

	win_upm_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(win_upm_main), "Pomf it! - Upload Profiles");
	g_signal_connect(win_upm_main, "destroy", G_CALLBACK(profiles_window_destroy), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(win_upm_main), 10);
	gtk_window_resize(GTK_WINDOW(win_upm_main), 600, 270);
	gtk_window_set_position(GTK_WINDOW(win_upm_main), GTK_WIN_POS_CENTER);

	vbox_upm_m = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(win_upm_main), vbox_upm_m);

	GtkWidget *grid_upm = gtk_grid_new();
	gtk_box_pack_start(GTK_BOX(vbox_upm_m), grid_upm , FALSE, FALSE, 0);

	lab_e = gtk_label_new("Profile Name:");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 0, 0, 3,  1);
	entry_prof_name = gtk_entry_new();
	gtk_grid_attach (GTK_GRID (grid_upm),entry_prof_name, 3, 0, 3,  1);

	lab_e = gtk_label_new("API Location:");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 6, 0, 3,  1);
	entry_api = gtk_entry_new();
	gtk_grid_attach (GTK_GRID (grid_upm),entry_api, 9, 0, 3,  1);

	lab_e = gtk_label_new("Form-Name:");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 0, 1, 3,  1);
	entry_form_name = gtk_entry_new();
	gtk_grid_attach (GTK_GRID (grid_upm),entry_form_name, 3, 1, 3,  1);

	lab_e = gtk_label_new("Form-Type:");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 6, 1, 3,  1);
	entry_form_type = gtk_entry_new();
	gtk_grid_attach (GTK_GRID (grid_upm),entry_form_type, 9, 1, 3,  1);

	lab_e = gtk_label_new("Form-Filename:");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 0, 2, 3,  1);
	entry_form_fname = gtk_entry_new();
	gtk_grid_attach (GTK_GRID (grid_upm),entry_form_fname, 3, 2, 3,  1);

	lab_e = gtk_label_new("Size Limit(MB):");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 6, 2, 3,  1);
	entry_fsize_limit = gtk_entry_new();
	gtk_grid_attach (GTK_GRID (grid_upm),entry_fsize_limit, 9, 2, 3,  1);

	lab_e = gtk_label_new("Regex URL:");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 0, 3, 3,  1);
	entry_regex_url = gtk_entry_new();
	gtk_grid_attach (GTK_GRID (grid_upm),entry_regex_url, 3, 3, 3,  1);

	lab_e = gtk_label_new("Regex Del URL:");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 6, 3, 3,  1);
	entry_regex_del = gtk_entry_new();
	gtk_grid_attach (GTK_GRID (grid_upm),entry_regex_del, 9, 3, 3,  1);

	lab_e = gtk_label_new("Prepend Regex Output");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 0, 4, 3, 1);
	sw_reout_prep = gtk_switch_new();
	gtk_grid_attach (GTK_GRID (grid_upm),sw_reout_prep, 3, 4, 3, 1);

	lab_e = gtk_label_new("Prepend String:");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 6, 4, 3,  1);
	entry_reout_prep = gtk_entry_new();
	gtk_grid_attach (GTK_GRID (grid_upm),entry_reout_prep, 9, 4, 3,  1);

	lab_e = gtk_label_new("Send Cookies");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 0, 5, 3, 1);
	sw_cookies = gtk_switch_new();
	gtk_grid_attach (GTK_GRID (grid_upm),sw_cookies, 3, 5, 3, 1);

	lab_e = gtk_label_new("Send Filenames");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 6, 5, 3, 1);
	sw_filename = gtk_switch_new();
	gtk_grid_attach (GTK_GRID (grid_upm),sw_filename, 9, 5, 3, 1);

	lab_e = gtk_label_new("Send Form-Filename");
	gtk_grid_attach (GTK_GRID (grid_upm),lab_e, 0, 6, 3,  1);
	sw_filename_u = gtk_switch_new();
	gtk_grid_attach (GTK_GRID (grid_upm),sw_filename_u, 3, 6, 3,  1);

	but_set_tem = gtk_button_new_with_label("    Template    ");
	g_signal_connect(but_set_tem,"clicked",G_CALLBACK(profiles_profile_template), NULL);
	gtk_grid_attach (GTK_GRID (grid_upm),but_set_tem, 6, 6, 3,  1);

	but_set_add = gtk_button_new_with_label("    Save Profile    ");
	g_signal_connect(but_set_add,"clicked",G_CALLBACK(profiles_profile_add), NULL);
	gtk_grid_attach (GTK_GRID (grid_upm),but_set_add, 9, 6, 3,  1);

	but_set_close = gtk_button_new_with_label("    Close    ");
	g_signal_connect(but_set_close,"clicked",G_CALLBACK(profiles_window_destroy), NULL);
	gtk_box_pack_end(GTK_BOX(vbox_upm_m),GTK_WIDGET(but_set_close),FALSE,FALSE,0);

	gtk_widget_show_all(win_upm_main);
}


void profiles_window_destroy (void) {

	gtk_widget_destroy(win_upm_main);
	win_upm_main = NULL;
}

void profiles_profile_add (void) {

	const char *pEntryBuff = NULL;
	char ProfileName[256] = "";
	char API_URL[128] = "";
	char FormName[128] = "";
	char FormType[128] = "";
	char FormFileName[128] = "";
	char RegexUrl[128] = "";
	char RegexDel[128] = "";
	char PrependString[128] = "";
	long int SizeLimit = 0;
	bool bSendCookie;
	bool bSendName;
	bool bSendFormName;
	bool bPrependString;

	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_prof_name));
	if(strcmp(pEntryBuff,"Pomf.se") == 0 || strcmp(pEntryBuff,"Uguu.se") ==0) {

		GtkWidget *dialog_ok;
		dialog_ok = gtk_message_dialog_new (GTK_WINDOW(win_upm_main),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
							"Overwritting default profiles is forbidden, rename your profile name.");
		if(gtk_dialog_run(GTK_DIALOG (dialog_ok))==GTK_RESPONSE_OK) {

			gtk_widget_destroy (dialog_ok);
		}
		return;
	}
	snprintf(ProfileName,sizeof(ProfileName),"%s",pEntryBuff);
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_api));
	snprintf(API_URL,sizeof(API_URL),"%s",pEntryBuff);
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_form_name));
	snprintf(FormName,sizeof(FormName),"%s",pEntryBuff);
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_form_type));
	snprintf(FormType,sizeof(FormType),"%s",pEntryBuff);
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_form_fname));
	snprintf(FormFileName,sizeof(FormFileName),"%s",pEntryBuff);
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_reout_prep));
	snprintf(PrependString,sizeof(PrependString),"%s",pEntryBuff);

	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_fsize_limit));
	if((SizeLimit = strtol(pEntryBuff,NULL,10)) == 0) {

		GtkWidget *dialog_ok;
		dialog_ok = gtk_message_dialog_new (GTK_WINDOW(win_upm_main),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
							"Invalid Size Limit value.");
		if(gtk_dialog_run(GTK_DIALOG (dialog_ok))==GTK_RESPONSE_OK) {

			gtk_widget_destroy (dialog_ok);
		}
		return;
	}
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_regex_url));
	snprintf(RegexUrl,sizeof(RegexUrl),"%s",pEntryBuff);
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_regex_del));
	snprintf(RegexDel,sizeof(RegexDel),"%s",pEntryBuff);

	bSendCookie = gtk_switch_get_active(GTK_SWITCH(sw_cookies));
	bSendName = gtk_switch_get_active(GTK_SWITCH(sw_filename));
	bSendFormName = gtk_switch_get_active(GTK_SWITCH(sw_filename_u));
	bPrependString = gtk_switch_get_active(GTK_SWITCH(sw_reout_prep));
	pEntryBuff = NULL;

	char Query[2048];
	snprintf(Query,sizeof(Query),"INSERT OR REPLACE INTO profiles "
			"(name, url, form_name, form_type ,form_file_name, size_limit, "
			"send_cookie, send_name, send_form_name, b_prep, regex_url, regex_del, prep_string) "
			"VALUES ( ?, ?, ?, ?, ? , %d, %d , %d, %d, %d, ? , ? , ?)",(int)SizeLimit, bSendCookie,
									bSendName, bSendFormName, bPrependString);
	SQLite.execute2( Query , 8 , ProfileName, API_URL, FormName ,FormType ,FormFileName,
			RegexUrl, RegexDel, PrependString);

	profiles_combo_fill();
	profiles_window_destroy();
}

void profiles_profile_template (void) {

	gtk_entry_set_text(GTK_ENTRY(entry_prof_name),"Pomf.se");
	gtk_entry_set_text(GTK_ENTRY(entry_api),"http://pomf.se/upload.php");
	gtk_entry_set_text(GTK_ENTRY(entry_form_name),"files[]");
	gtk_entry_set_text(GTK_ENTRY(entry_form_type),"application/octet-stream");
	gtk_entry_set_text(GTK_ENTRY(entry_form_fname),"pomfit_upload");
	gtk_entry_set_text(GTK_ENTRY(entry_fsize_limit),"52");
	gtk_entry_set_text(GTK_ENTRY(entry_regex_url),"url\":\"([^: \"]+)");
	gtk_entry_set_text(GTK_ENTRY(entry_regex_del),"del_url\":\"([^: \"]+)");
	gtk_entry_set_text(GTK_ENTRY(entry_reout_prep),"http://a.pomf.se/");
	gtk_switch_set_active(GTK_SWITCH(sw_cookies),true);
	gtk_switch_set_active(GTK_SWITCH(sw_filename),true);
	gtk_switch_set_active(GTK_SWITCH(sw_filename_u),false);
	gtk_switch_set_active(GTK_SWITCH(sw_reout_prep),true);

}

void profiles_profile_delete (void) {

	char *ProfileName = NULL;
	ProfileName = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(com_upm_pl));
	if(ProfileName == NULL)
		return;
	if(strcmp(ProfileName,"Pomf.se") == 0 || strcmp(ProfileName,"Uguu.se") ==0) {

		GtkWidget *dialog_ok;
		dialog_ok = gtk_message_dialog_new (GTK_WINDOW(win_set_main),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
							"Deleting default profiles is forbidden.");
		if(gtk_dialog_run(GTK_DIALOG (dialog_ok))==GTK_RESPONSE_OK) {

			gtk_widget_destroy (dialog_ok);
		}
		return;
	}
	if(ProfileName) {
		SQLite.execute2("DELETE FROM profiles WHERE name = ?",1,ProfileName);
		profiles_combo_fill();
		ProfileName = NULL;

	} else
		printf("Profile = NULL\n");
}

void profiles_combo_fill (void) {

	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(com_upm_pl));
	SQLite.execute("SELECT name FROM profiles");
	const char *pProfile = NULL;
	while((pProfile = SQLite.fetchone_c("name")) != NULL)
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_upm_pl), NULL,pProfile);

    pProfile = NULL;
}

void profiles_profile_activate (GtkWidget *widget, char *Default) {

	if(Default == NULL) {
		char *ProfileName = NULL;
		ProfileName = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(com_upm_pl));
		if(ProfileName) {
			SQLite.execute2("SELECT * FROM profiles WHERE name = ?",1, ProfileName);
			pomfit_db_profiles_select(&ActiveProfile);
			gtk_label_set_text (GTK_LABEL(lab_profile), ProfileName);
			strcpy(ActProfile,ProfileName);
			ProfileName = NULL;
		}

	} else {
		SQLite.execute2("SELECT * FROM profiles WHERE name = ?",1, Default);
		pomfit_db_profiles_select(&ActiveProfile);
	}

}

void profiles_profile_edit (void) {

	UpProfile Buffer;
	char *ProfileName = NULL;
	ProfileName = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(com_upm_pl));
	if(ProfileName) {
		SQLite.execute2("SELECT * FROM profiles WHERE name = ?",1, ProfileName);
		pomfit_db_profiles_select(&Buffer);

		profiles_window_create();

		gtk_entry_set_text(GTK_ENTRY(entry_prof_name),Buffer.Name);
		gtk_entry_set_text(GTK_ENTRY(entry_api),Buffer.URL);
		gtk_entry_set_text(GTK_ENTRY(entry_form_name),Buffer.FormName);
		gtk_entry_set_text(GTK_ENTRY(entry_form_type),Buffer.FormType);
		gtk_entry_set_text(GTK_ENTRY(entry_form_fname),Buffer.FormFileName);
		char MaxSize[10];
		sprintf(MaxSize,"%d",Buffer.MaxSize);
		gtk_entry_set_text(GTK_ENTRY(entry_fsize_limit),MaxSize);
		gtk_entry_set_text(GTK_ENTRY(entry_regex_url),Buffer.RegEx_URL);
		gtk_entry_set_text(GTK_ENTRY(entry_regex_del),Buffer.RegEx_DEL);
		gtk_entry_set_text(GTK_ENTRY(entry_reout_prep),Buffer.PrepString);
		gtk_switch_set_active(GTK_SWITCH(sw_cookies),Buffer.bCookie);
		gtk_switch_set_active(GTK_SWITCH(sw_filename),Buffer.bNames);
		gtk_switch_set_active(GTK_SWITCH(sw_filename_u),Buffer.bFormName);
		gtk_switch_set_active(GTK_SWITCH(sw_reout_prep),Buffer.bPrepend);


		ProfileName = NULL;

	} else
		return;
}

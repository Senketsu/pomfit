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
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <gtk/gtk.h>
#ifdef _WIN32
#define _WIN32_IE 0x0500
#define __WIN32_WINNT 0x0501
#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ShellAPI.h>
#include <Shlobj.h>
#endif

#include "settings.h"
#include "profiles.h"
#include "log.h"

extern char DirHome[64];
extern char DirConf[512];
extern char DirScrn[512];
extern char DirUpScrn[512];
extern char PathCookie[256];
extern char PathCurlOut[256];
extern char PathDB[512];
extern char PathIcon[256];
extern char PathIcon_Loc[256];
extern char LastFolder[512];
extern char ActProfile[256];

bool bUP_Keep_SS;
bool bUM_Preview;
bool bUM_AnimPreview;
bool bUM_DoubleClick;
int iUM_DC_Action;
int iUM_Win_W;
int iUM_Win_H;
int iUM_Prev_Size_W;
int iUM_Prev_Size_H;
int iUM_Win_Pos;
int iUQ_Win_Pos;
int iUQ_Win_W;
int iUQ_Win_H;

GtkWidget *win_set_main;
/// UM Sett
GtkWidget *entry_um_pw_w, *entry_um_pw_h, *entry_um_win_h, *entry_um_win_w;
GtkWidget *sw_um_pp, *sw_um_ap, *sw_um_dc, *com_um_dca, *com_um_wp;
/// UP Sett
GtkWidget *sw_up_kss, *com_upm_pl;
/// UQ Sett
GtkWidget *entry_uq_win_w, *entry_uq_win_h, *com_uq_wp, *lab_profile;

void settings_window_create (void) {

	if(win_set_main != NULL) {
		gtk_widget_hide(win_set_main);
		gtk_widget_show(win_set_main);
		return;
	}

	GtkWidget *vbox_set_m, *hbox_set_m, *but_set_close, *but_set_save, *but_set_reset;
	GtkWidget *nb_set_m, *lab_e, *vbox_nb_up, *vbox_nb_um, *vbox_nb_uq;
	GtkWidget *hbox_um_s, *grid_uq, *grid_up, *grid_um;

	win_set_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(win_set_main), "Pomf it! - Settings");
	g_signal_connect(win_set_main, "destroy", G_CALLBACK(settings_window_destroy), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(win_set_main), 10);
	gtk_window_resize(GTK_WINDOW(win_set_main), 320, 200);
	gtk_window_set_position(GTK_WINDOW(win_set_main), GTK_WIN_POS_CENTER);

	vbox_set_m = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(win_set_main), vbox_set_m);

	nb_set_m = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(vbox_set_m),nb_set_m);

	vbox_nb_up = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(nb_set_m), vbox_nb_up);
	lab_e = gtk_label_new("Uploader");
	g_object_set(G_OBJECT(lab_e), "can-focus", FALSE, NULL);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb_set_m),
		gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb_set_m), 0),lab_e);

	vbox_nb_um = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(nb_set_m), vbox_nb_um);
	lab_e = gtk_label_new("Manager");
	g_object_set(G_OBJECT(lab_e), "can-focus", FALSE, NULL);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb_set_m),
		gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb_set_m), 1),lab_e);

	vbox_nb_uq = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(nb_set_m), vbox_nb_uq);
	lab_e = gtk_label_new("Queue");
	g_object_set(G_OBJECT(lab_e), "can-focus", FALSE, NULL);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb_set_m),
		gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb_set_m), 2),lab_e);

	grid_up = gtk_grid_new ();
	gtk_box_pack_start(GTK_BOX(vbox_nb_up), grid_up , FALSE, FALSE, 0);

	lab_e = gtk_label_new("Profiles:	");
	gtk_grid_attach (GTK_GRID (grid_up),lab_e, 0, 0, 2,  1);

	com_upm_pl = gtk_combo_box_text_new();
	gtk_grid_attach (GTK_GRID (grid_up),com_upm_pl, 2, 0, 2,  1);
	profiles_combo_fill();

	hbox_um_s = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 0);
	gtk_grid_attach (GTK_GRID (grid_up),hbox_um_s, 4, 0, 4,  1);

	GtkWidget *but_up_act = gtk_button_new_with_label("Activate");
	g_signal_connect(but_up_act,"clicked",G_CALLBACK(profiles_profile_activate), NULL);
	gtk_box_pack_start(GTK_BOX(hbox_um_s),GTK_WIDGET(but_up_act),TRUE,TRUE,0);

	GtkWidget *but_up_edit = gtk_button_new_with_label("Edit");
	g_signal_connect(but_up_edit,"clicked",G_CALLBACK(profiles_profile_edit), NULL);
	gtk_box_pack_start(GTK_BOX(hbox_um_s),GTK_WIDGET(but_up_edit),TRUE,TRUE,0);

	hbox_um_s = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 0);
	gtk_grid_attach (GTK_GRID (grid_up),hbox_um_s, 8, 0, 4,  1);

	GtkWidget *but_up_add = gtk_button_new_with_label("Add New");
	g_signal_connect(but_up_add,"clicked",G_CALLBACK(profiles_window_create), NULL);
	gtk_box_pack_end(GTK_BOX(hbox_um_s),GTK_WIDGET(but_up_add),TRUE,FALSE,0);

	GtkWidget *but_up_del = gtk_button_new_with_label("Delete");
	g_signal_connect(but_up_del,"clicked",G_CALLBACK(profiles_profile_delete), NULL);
	gtk_box_pack_start(GTK_BOX(hbox_um_s),GTK_WIDGET(but_up_del),TRUE,FALSE,0);

	lab_e = gtk_label_new("Active Profile:");
	gtk_grid_attach (GTK_GRID (grid_up),lab_e, 0, 1, 2,  1);

	lab_profile = gtk_label_new("none");
	gtk_grid_attach (GTK_GRID (grid_up),lab_profile, 2, 1, 2,  1);

	lab_e = gtk_label_new("Keep SS:");
	gtk_grid_attach (GTK_GRID (grid_up),lab_e, 4, 1, 2,  1);

	sw_up_kss = gtk_switch_new();
	gtk_grid_attach (GTK_GRID (grid_up),sw_up_kss, 6, 1, 2,  1);

    grid_um = gtk_grid_new ();
	gtk_box_pack_start(GTK_BOX(vbox_nb_um), grid_um , FALSE, FALSE, 0);

	lab_e = gtk_label_new("Picture Preview:");
	gtk_grid_attach (GTK_GRID (grid_um),lab_e, 0, 0, 2,  1);

	sw_um_pp = gtk_switch_new();
	gtk_grid_attach (GTK_GRID (grid_um),sw_um_pp, 2, 0, 2,  1);

	lab_e = gtk_label_new("Window Size:");
	gtk_grid_attach (GTK_GRID (grid_um),lab_e, 4, 0, 2,  1);

	hbox_um_s = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 0);
	gtk_grid_attach (GTK_GRID (grid_um),hbox_um_s, 6, 0, 4,  1);

	entry_um_win_w = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(hbox_um_s), entry_um_win_w);
	gtk_entry_set_width_chars(GTK_ENTRY(entry_um_win_w),5);

	lab_e = gtk_label_new("X");
	gtk_container_add(GTK_CONTAINER(hbox_um_s), lab_e);

	entry_um_win_h = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(hbox_um_s), entry_um_win_h);
	gtk_entry_set_width_chars(GTK_ENTRY(entry_um_win_h),5);

	lab_e = gtk_label_new("Animate Preview:");
	gtk_grid_attach (GTK_GRID (grid_um),lab_e, 0, 1, 2,  1);

	sw_um_ap = gtk_switch_new();
	gtk_grid_attach (GTK_GRID (grid_um),sw_um_ap, 2, 1, 2,  1);

	lab_e = gtk_label_new("Preview Size:");
	gtk_grid_attach (GTK_GRID (grid_um),lab_e, 4, 1, 2,  1);

	hbox_um_s = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 0);
	gtk_grid_attach (GTK_GRID (grid_um),hbox_um_s, 6, 1, 4,  1);

	entry_um_pw_w = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(hbox_um_s), entry_um_pw_w);
	gtk_entry_set_width_chars(GTK_ENTRY(entry_um_pw_w),5);

	lab_e = gtk_label_new("X");
	gtk_container_add(GTK_CONTAINER(hbox_um_s), lab_e);

	entry_um_pw_h = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(hbox_um_s), entry_um_pw_h);
	gtk_entry_set_width_chars(GTK_ENTRY(entry_um_pw_h),5);

	lab_e = gtk_label_new("Window Position:");
	gtk_grid_attach (GTK_GRID (grid_um),lab_e, 4, 2, 2,  1);

	com_um_wp = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_um_wp), NULL, "Default");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_um_wp), NULL, "Center");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_um_wp), NULL, "Mouse");
	gtk_grid_attach (GTK_GRID (grid_um),com_um_wp, 6, 2, 2,  1);

	lab_e = gtk_label_new("Enable Double Click:");
	gtk_grid_attach (GTK_GRID (grid_um),lab_e, 0, 3, 2,  1);

	sw_um_dc = gtk_switch_new();
	gtk_grid_attach (GTK_GRID (grid_um),sw_um_dc, 2, 3, 2,  1);

	lab_e = gtk_label_new("Action:");
	gtk_grid_attach (GTK_GRID (grid_um),lab_e, 4, 3, 2,  1);

	com_um_dca = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_um_dca), NULL, "Open File");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_um_dca), NULL, "Open Link");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_um_dca), NULL, "Open Folder");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_um_dca), NULL, "Copy URL");
	gtk_grid_attach (GTK_GRID (grid_um),com_um_dca, 6, 3, 2,  1);

	grid_uq = gtk_grid_new ();
	gtk_box_pack_start(GTK_BOX(vbox_nb_uq), grid_uq , FALSE, FALSE, 0);

	lab_e = gtk_label_new("Window Position:");
	gtk_grid_attach (GTK_GRID (grid_uq),lab_e, 0, 0, 2,  1);

	com_uq_wp = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_uq_wp), NULL, "Default");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_uq_wp), NULL, "Center");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_uq_wp), NULL, "Mouse");
	gtk_grid_attach (GTK_GRID (grid_uq),com_uq_wp, 2, 0, 2,  1);

	lab_e = gtk_label_new("Window Size:");
	gtk_grid_attach (GTK_GRID (grid_uq),lab_e, 4, 0, 2,  1);

	hbox_um_s = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 0);
	gtk_grid_attach (GTK_GRID (grid_uq),hbox_um_s, 6, 0, 4,  1);

	entry_uq_win_w = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(hbox_um_s), entry_uq_win_w);
	gtk_entry_set_width_chars(GTK_ENTRY(entry_uq_win_w),5);

	lab_e = gtk_label_new("X");
	gtk_container_add(GTK_CONTAINER(hbox_um_s), lab_e);

	entry_uq_win_h = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(hbox_um_s), entry_uq_win_h);
	gtk_entry_set_width_chars(GTK_ENTRY(entry_uq_win_h),5);

	hbox_set_m = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 0);
	gtk_box_pack_end(GTK_BOX(vbox_set_m), hbox_set_m, FALSE, FALSE, 0);

	but_set_reset = gtk_button_new_with_label("Reset Settings");
	g_signal_connect(but_set_reset,"clicked",G_CALLBACK(settings_conf_default), NULL);
	gtk_container_add(GTK_CONTAINER(hbox_set_m), but_set_reset);

	but_set_close = gtk_button_new_with_label("    Close    ");
	g_signal_connect(but_set_close,"clicked",G_CALLBACK(settings_window_destroy), NULL);
	gtk_box_pack_end(GTK_BOX(hbox_set_m),GTK_WIDGET(but_set_close),FALSE,FALSE,0);

	but_set_save = gtk_button_new_with_label("Save & Close");
	g_signal_connect(but_set_save,"clicked",G_CALLBACK(settings_conf_save), NULL);
	gtk_box_pack_end(GTK_BOX(hbox_set_m),GTK_WIDGET(but_set_save),FALSE,FALSE,0);

	settings_window_create_set_actives();
	gtk_widget_show_all(win_set_main);
}


void settings_window_destroy (void) {

	gtk_widget_destroy(win_set_main);
	win_set_main = NULL;
}

void settings_window_create_set_actives (void) {

	char Buff[32];
	gtk_switch_set_active (GTK_SWITCH(sw_um_pp), bUM_Preview);
	gtk_switch_set_active (GTK_SWITCH(sw_um_ap), bUM_AnimPreview);
	gtk_switch_set_active (GTK_SWITCH(sw_um_dc), bUM_DoubleClick);
	gtk_switch_set_active (GTK_SWITCH(sw_up_kss), bUP_Keep_SS);
	gtk_combo_box_set_active(GTK_COMBO_BOX(com_um_dca), iUM_DC_Action);
	gtk_combo_box_set_active(GTK_COMBO_BOX(com_um_wp), iUM_Win_Pos);
	snprintf(Buff,sizeof(Buff),"%d",iUM_Win_W);
	gtk_entry_set_text(GTK_ENTRY(entry_um_win_w),Buff);
	snprintf(Buff,sizeof(Buff),"%d",iUM_Win_H);
	gtk_entry_set_text(GTK_ENTRY(entry_um_win_h),Buff);
	snprintf(Buff,sizeof(Buff),"%d",iUM_Prev_Size_W);
	gtk_entry_set_text(GTK_ENTRY(entry_um_pw_w),Buff);
	snprintf(Buff,sizeof(Buff),"%d",iUM_Prev_Size_H);
	gtk_entry_set_text(GTK_ENTRY(entry_um_pw_h),Buff);
	gtk_combo_box_set_active(GTK_COMBO_BOX(com_uq_wp), iUQ_Win_Pos);
	snprintf(Buff,sizeof(Buff),"%d",iUQ_Win_W);
	gtk_entry_set_text(GTK_ENTRY(entry_uq_win_w),Buff);
	snprintf(Buff,sizeof(Buff),"%d",iUQ_Win_H);
	gtk_entry_set_text(GTK_ENTRY(entry_uq_win_h),Buff);
	gtk_label_set_text (GTK_LABEL(lab_profile), ActProfile);
}

void settings_conf_save (void) {

	char PathConfFile [512];
	#ifdef __linux__
	sprintf(PathConfFile ,"%s/settings.ini" ,DirConf);
	#elif __WIN32
	sprintf(PathConfFile ,"%s\\settings.ini" ,DirConf);
	#endif
	FILE *fpConfig = fopen(PathConfFile , "w");
	if (fpConfig == NULL) {
		log_error(true,"***Error: Couldn't write to config file 'settings.ini'");
		return;
	}
	const char *pEntryBuff;

	fprintf(fpConfig,"UM_Preview = %d\n",
					gtk_switch_get_active(GTK_SWITCH(sw_um_pp)));
	fprintf(fpConfig,"UM_AnimPreview = %d\n",
					gtk_switch_get_active(GTK_SWITCH(sw_um_ap)));
	fprintf(fpConfig,"UM_DoubleClick = %d\n",
					gtk_switch_get_active(GTK_SWITCH(sw_um_dc)));
	fprintf(fpConfig,"UM_DC_Action = %d\n",
					gtk_combo_box_get_active(GTK_COMBO_BOX(com_um_dca)));
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_um_win_w));
	fprintf(fpConfig,"UM_Win_W = %s\n", pEntryBuff );
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_um_win_h));
	fprintf(fpConfig,"UM_Win_H = %s\n", pEntryBuff );
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_um_pw_w));
	fprintf(fpConfig,"UM_Prev_Size_W = %s\n", pEntryBuff );
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_um_pw_h));
	fprintf(fpConfig,"UM_Prev_Size_H = %s\n", pEntryBuff );
	fprintf(fpConfig,"UM_Win_Pos = %d\n",
					gtk_combo_box_get_active(GTK_COMBO_BOX(com_um_wp)));

	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_uq_win_w));
	fprintf(fpConfig,"UQ_Win_W = %s\n", pEntryBuff );
	pEntryBuff = gtk_entry_get_text(GTK_ENTRY(entry_uq_win_h));
	fprintf(fpConfig,"UQ_Win_H = %s\n", pEntryBuff );
	fprintf(fpConfig,"UQ_Win_Pos = %d\n",
					gtk_combo_box_get_active(GTK_COMBO_BOX(com_uq_wp)));

	fprintf(fpConfig,"UP_ProfileName = %s\n",
					gtk_label_get_text(GTK_LABEL(lab_profile)));
	fprintf(fpConfig,"UP_Keep_SS = %d\n",
					gtk_switch_get_active(GTK_SWITCH(sw_up_kss)));

	fclose(fpConfig);
	pEntryBuff = NULL;

	settings_window_destroy();
	settings_conf_load();
}

void settings_conf_load (void) {

	char PathConfFile [512];
	#ifdef __linux__
	sprintf(PathConfFile ,"%s/settings.ini" ,DirConf);
	#elif __WIN32
	sprintf(PathConfFile ,"%s\\settings.ini" ,DirConf);
	#endif
	FILE *fpConfig = fopen(PathConfFile , "r");
	if (fpConfig == NULL) {
		log_error(true,"**Warning: Couldn't load config file 'settings.ini' - Using defaults.");
		//~ settings_conf_default ();
        settings_conf_create ();
		return;
	}

	char *pBuff = NULL;
	char Getline[512] = "";
	char Mod[2] = "";
	char Size[16] = "";
    while(fgets (Getline , 512 , fpConfig) != NULL) {
		if(strncmp(Getline, "UM_Preview" , strlen("UM_Preview")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bUM_Preview = atoi(Mod);
	    } else if(strncmp(Getline, "UM_AnimPreview" , strlen("UM_AnimPreview")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bUM_AnimPreview = atoi(Mod);
	    } else if(strncmp(Getline, "UM_DoubleClick" , strlen("UM_DoubleClick")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bUM_DoubleClick = atoi(Mod);
	    } else if(strncmp(Getline, "UM_DC_Action" , strlen("UM_DC_Action")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            iUM_DC_Action = atoi(Mod);
	    } else if(strncmp(Getline, "UM_Win_W" , strlen("UM_Win_W")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
			snprintf(Size, strlen(pBuff),"%s",pBuff);
            iUM_Win_W = atoi(Size);
	    } else if(strncmp(Getline, "UM_Win_H" , strlen("UM_Win_H")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
			snprintf(Size, strlen(pBuff),"%s",pBuff);
            iUM_Win_H = atoi(Size);
	    } else if(strncmp(Getline, "UM_Prev_Size_W" , strlen("UM_Prev_Size_W")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
			snprintf(Size, strlen(pBuff),"%s",pBuff);
            iUM_Prev_Size_W = atoi(Size);
	    } else if(strncmp(Getline, "UM_Prev_Size_H" , strlen("UM_Prev_Size_H")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
			snprintf(Size, strlen(pBuff),"%s",pBuff);
            iUM_Prev_Size_H = atoi(Size);
	    } else if(strncmp(Getline, "UM_Win_Pos" , strlen("UM_Win_Pos")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            iUM_Win_Pos = atoi(Mod);
	    } else if(strncmp(Getline, "UQ_Win_W" , strlen("UQ_Win_W")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
			snprintf(Size, strlen(pBuff),"%s",pBuff);
            iUQ_Win_W = atoi(Size);
	    } else if(strncmp(Getline, "UQ_Win_H" , strlen("UQ_Win_H")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
			snprintf(Size, strlen(pBuff),"%s",pBuff);
            iUQ_Win_H = atoi(Size);
	    } else if(strncmp(Getline, "UQ_Win_Pos" , strlen("UQ_Win_Pos")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            iUQ_Win_Pos = atoi(Mod);
	    }else if(strncmp(Getline, "UP_Keep_SS" , strlen("UP_Keep_SS")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bUP_Keep_SS = atoi(Mod);
	    }  else if(strncmp(Getline, "UP_ProfileName" , strlen("UP_ProfileName")) == 0) {
            pBuff = strstr(Getline , "=");
            pBuff += 2;
            snprintf(ActProfile, strlen(pBuff),"%s",pBuff);
	    }
	}
	pBuff = NULL;
	fclose(fpConfig);
}

void settings_conf_create (void) {

	char PathConfFile [512];
	#ifdef __linux__
	sprintf(PathConfFile ,"%s/settings.ini" ,DirConf);
	#elif __WIN32
	sprintf(PathConfFile ,"%s\\settings.ini" ,DirConf);
	#endif
	FILE *fpConfig = fopen(PathConfFile , "w");
	if (fpConfig == NULL) {
		log_error(true,"***Error: Couldn't write to config file 'settings.ini'");
		return;
	}
	fprintf(fpConfig, "UM_Preview = %d\n", false );
	fprintf(fpConfig, "UM_AnimPreview = %d\n", false );
	fprintf(fpConfig, "UM_DoubleClick = %d\n", false );
	fprintf(fpConfig, "UM_DC_Action = 0\n");
	fprintf(fpConfig, "UM_Win_W = 600\n");
	fprintf(fpConfig, "UM_Win_H = 300\n");
	fprintf(fpConfig, "UM_Prev_Size_W = 192\n");
	fprintf(fpConfig, "UM_Prev_Size_H = 192\n");
	fprintf(fpConfig, "UM_Win_Pos = 0\n");

	fprintf(fpConfig, "UQ_Win_W = 360\n");
	fprintf(fpConfig, "UQ_Win_H = 240\n");
	fprintf(fpConfig, "UQ_Win_Pos = 0\n");

	fprintf(fpConfig, "UP_ProfileName = Pomf.se\n" );
	fprintf(fpConfig, "UP_Keep_SS = %d\n", false );


	fclose(fpConfig);
	settings_conf_load();
}

void settings_conf_default (void) {

	GtkWidget *dialog;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_message_dialog_new (GTK_WINDOW(win_set_main), flags,
									 GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
									 "Reset all settings ?");
	if(gtk_dialog_run(GTK_DIALOG (dialog))==GTK_RESPONSE_YES) {
		settings_conf_create();
		settings_window_create_set_actives();
	}

	gtk_widget_destroy (dialog);
}

static char *settings_get_home(void) {

	char *pHome = NULL;
	#ifdef __linux__
	pHome = getenv("HOME");
	#elif _WIN32
	pHome = getenv("HOMEDRIVE");
	#endif
	return pHome;
}

void settings_mkdir(const char* Directory) {

	int test_dir = chdir(Directory);
	chdir(DirHome);
	if(test_dir != 0) {
#ifdef __linux__
		mkdir(Directory, 0777);
#elif _WIN32
		mkdir(Directory);
#endif
	}
}

void settings_paths_default(void) {

	snprintf(DirHome, sizeof(DirHome),"%s",settings_get_home());
	#ifdef __linux__

	snprintf(LastFolder,sizeof(LastFolder),"%s", DirHome);
	snprintf(DirConf,sizeof(DirConf),"%s/.config/pomfit",DirHome);
	settings_mkdir(DirConf);
	snprintf(DirScrn,sizeof(DirScrn),"%s/Pictures",DirHome);
	settings_mkdir(DirScrn);
	snprintf(DirScrn,sizeof(DirScrn),"%s/Pictures/Screenshots",DirHome);
	settings_mkdir(DirScrn);
	snprintf(DirUpScrn,sizeof(DirUpScrn),"%s/Uploaded",DirScrn);
	settings_mkdir(DirUpScrn);
	snprintf(PathIcon,sizeof(PathIcon),"/usr/share/pixmaps/pomfit.png");
	snprintf(PathIcon_Loc,sizeof(PathIcon_Loc),"/usr/local/share/pixmaps/pomfit.png");
	snprintf(PathCookie,sizeof(PathCookie),"%s/pomfit_cookies",DirConf);
	snprintf(PathDB,sizeof(PathDB),"%s/Pomfit.db",DirConf);
	snprintf(PathCurlOut ,sizeof(PathCurlOut),"%s/curl_output" , DirConf);

	#elif _WIN32
	snprintf(LastFolder,sizeof(LastFolder),"%s\\", DirHome);
	TCHAR szFolderPath[MAX_PATH];
     if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szFolderPath)))
        snprintf(DirConf,sizeof(DirConf),"%s\\Pomfit",szFolderPath);
    else
        snprintf(DirConf,sizeof(DirConf),"%s\\Pomfit",DirHome);
    settings_mkdir(DirConf);

    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, 0, szFolderPath))) {
        snprintf(DirScrn,sizeof(DirScrn), "%s\\Pomfit",szFolderPath);
        snprintf(DirUpScrn, sizeof(DirUpScrn),"%s\\Uploaded",DirScrn);
    } else {
        snprintf(DirScrn,sizeof(DirScrn), "%s\\Screenshots",DirConf);
        snprintf(DirUpScrn, sizeof(DirUpScrn), "%s\\Uploaded",DirScrn);
    }
	settings_mkdir(DirScrn);
	settings_mkdir(DirUpScrn);
	snprintf(PathCookie,sizeof(PathCookie),"%s\\pomfit_cookies",DirConf);
	snprintf(PathDB,sizeof(PathDB),"%s\\Pomfit.db",DirConf);
	snprintf(PathCurlOut ,sizeof(PathCurlOut),"%s\\curl_output" , DirConf);
	#endif
}

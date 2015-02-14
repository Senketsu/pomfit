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
#include <gtk/gtk.h>
#include <stdbool.h>
#ifdef __linux__
    #include <keybinder.h>
#elif _WIN32
    #include <windows.h>
    #include "resource.h"
#endif
#include "log.h"
#include "keybinds.h"
#include "other.h"

char Hotkey_FUP[64];
char Hotkey_SS_A_UP[64];
char Hotkey_SS_W_UP[64];
char Hotkey_SS_F_UP[64];
char Hotkey_SS_A_CAP[64];
char Hotkey_SS_W_CAP[64];
char Hotkey_SS_F_CAP[64];
char Hotkey_OPEN[64];

bool bHotkey_FUP;
bool bHotkey_SS_A_UP;
bool bHotkey_SS_W_UP;
bool bHotkey_SS_F_UP;
bool bHotkey_SS_A_CAP;
bool bHotkey_SS_W_CAP;
bool bHotkey_SS_F_CAP;
bool bHotkey_OPEN;


extern char DirConf[512];
extern void file_chooser_start(const char *keystring, void *user_data);
extern void take_screenshot (const char *keystring, void *Upload);
extern void open_acc_page(const char *keystring, void *user_data);
extern GtkWidget *menuOpen, *menuCap, *menuCapUp, *menuFileUp;

GtkWidget *win_kb_main;
GtkWidget *com_kb_mod1[HOTKEY_COUNT], *com_kb_mod2[HOTKEY_COUNT];
GtkWidget *com_kb_key[HOTKEY_COUNT], *sw_kb[HOTKEY_COUNT];

#ifdef _WIN32
extern HWND hWndGTK;
#endif

void keybinds_window_create (void) {

	int i;
	GtkWidget *vbox_kb_m, *hbox_kb_s, *hbox_kb_lay;
	GtkWidget *but_kb_close, *but_kb_save, *but_kb_reset;
	GtkWidget *lab_kb_e, *hbox_kb_bind[4], *entry_kb_test;
	GtkWidget *vbox_kb_s1, *vbox_kb_s2, *hbox_kb_m;

	win_kb_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(win_kb_main), "Pomf it! - Keybinds Settings");
	g_signal_connect(win_kb_main, "destroy", G_CALLBACK(keybinds_window_destroy), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(win_kb_main), 10);
	gtk_window_resize(GTK_WINDOW(win_kb_main), 500, 200);
	gtk_window_set_position(GTK_WINDOW(win_kb_main), GTK_WIN_POS_CENTER);

	vbox_kb_m = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(win_kb_main), vbox_kb_m);

	hbox_kb_m = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 0);
	gtk_container_add(GTK_CONTAINER(vbox_kb_m), hbox_kb_m);

	vbox_kb_s1 = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(hbox_kb_m), vbox_kb_s1);

	vbox_kb_s2 = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(hbox_kb_m), vbox_kb_s2);

    lab_kb_e = gtk_label_new("File Upload:");
	gtk_box_pack_start (GTK_BOX(vbox_kb_s1), GTK_WIDGET(lab_kb_e), FALSE, FALSE, 5);
	lab_kb_e = gtk_label_new("Capture Area & UP:");
	gtk_box_pack_start (GTK_BOX(vbox_kb_s1), GTK_WIDGET(lab_kb_e), TRUE, FALSE, 5);
	lab_kb_e = gtk_label_new("Capture Window & UP:");
	gtk_box_pack_start (GTK_BOX(vbox_kb_s1), GTK_WIDGET(lab_kb_e), FALSE, FALSE, 5);
	lab_kb_e = gtk_label_new("Capture Fullscreen & UP:");
	gtk_box_pack_start (GTK_BOX(vbox_kb_s1), GTK_WIDGET(lab_kb_e), TRUE, FALSE, 5);
	lab_kb_e = gtk_label_new("Capture Area:");
	gtk_box_pack_start (GTK_BOX(vbox_kb_s1), GTK_WIDGET(lab_kb_e), FALSE, FALSE, 5);
	lab_kb_e = gtk_label_new("Capture Window:");
	gtk_box_pack_start (GTK_BOX(vbox_kb_s1), GTK_WIDGET(lab_kb_e), TRUE, FALSE, 5);
	lab_kb_e = gtk_label_new("Capture Fullscreen:");
	gtk_box_pack_start (GTK_BOX(vbox_kb_s1), GTK_WIDGET(lab_kb_e), FALSE, FALSE, 5);
	lab_kb_e = gtk_label_new("Open URL(s):");
	gtk_box_pack_start (GTK_BOX(vbox_kb_s1), GTK_WIDGET(lab_kb_e), TRUE, FALSE, 5);

	for ( i = 0 ; i < HOTKEY_COUNT ; ++i) {

		hbox_kb_bind[i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL ,4);
		gtk_container_add (GTK_CONTAINER(vbox_kb_s2),hbox_kb_bind[i]);

		com_kb_mod1[i] = gtk_combo_box_text_new();
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_mod1[i]), NULL, "");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_mod1[i]), NULL, "<CTRL>");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_mod1[i]), NULL, "<SHIFT>");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_mod1[i]), NULL, "<ALT>");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_mod1[i]), NULL, "<WIN>");
		gtk_container_add(GTK_CONTAINER(hbox_kb_bind[i]), com_kb_mod1[i]);

		com_kb_mod2[i] = gtk_combo_box_text_new();
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_mod2[i]), NULL, "");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_mod2[i]), NULL, "<CTRL>");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_mod2[i]), NULL, "<SHIFT>");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_mod2[i]), NULL, "<ALT>");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_mod2[i]), NULL, "<WIN>");
		gtk_container_add(GTK_CONTAINER(hbox_kb_bind[i]), com_kb_mod2[i]);

		com_kb_key[i] = gtk_combo_box_text_new();
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "!");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "\"");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "#");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "$");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "%");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "&");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "'");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "(");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, ")");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "*");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "+");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, ",");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "-");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, ".");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "/");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "0");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "1");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "2");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "3");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "4");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "5");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "6");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "7");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "8");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "9");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, ":");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, ";");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "<");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "=");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, ">");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "?");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "@");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "A");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "B");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "C");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "D");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "E");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "F");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "G");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "H");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "I");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "J");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "K");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "L");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "M");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "N");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "O");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "P");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "Q");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "R");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "S");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "T");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "U");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "V");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "W");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "X");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "Y");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "Z");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "[");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "\\");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "]");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "^");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "_");
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(com_kb_key[i]), NULL, "`");
		gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_key[i]), 0);
		gtk_combo_box_set_wrap_width (GTK_COMBO_BOX(com_kb_key[i]), 8);
		gtk_container_add(GTK_CONTAINER(hbox_kb_bind[i]), com_kb_key[i]);

		sw_kb[i] = gtk_switch_new();
		gtk_container_add(GTK_CONTAINER(hbox_kb_bind[i]), sw_kb[i]);

	}
	keybinds_window_create_set_actives();

	hbox_kb_lay = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 5);
	gtk_container_add(GTK_CONTAINER(vbox_kb_m), hbox_kb_lay);

	lab_kb_e = gtk_label_new("Wait...What's my keyboard layout again ? ..");
	gtk_box_pack_start (GTK_BOX(hbox_kb_lay), GTK_WIDGET(lab_kb_e), FALSE, FALSE, 5);

	entry_kb_test = gtk_entry_new ();
	gtk_container_add(GTK_CONTAINER(hbox_kb_lay), entry_kb_test);

	hbox_kb_s = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 0);
	gtk_box_pack_end(GTK_BOX(vbox_kb_m), hbox_kb_s, FALSE, FALSE, 0);

	but_kb_reset = gtk_button_new_with_label("Reset Settings");
	g_signal_connect(but_kb_reset,"clicked",G_CALLBACK(keybinds_conf_default), NULL);
	gtk_container_add(GTK_CONTAINER(hbox_kb_s), but_kb_reset);

	but_kb_close = gtk_button_new_with_label("    Close    ");
	g_signal_connect(but_kb_close,"clicked",G_CALLBACK(keybinds_window_destroy), NULL);
	gtk_box_pack_end(GTK_BOX(hbox_kb_s),GTK_WIDGET(but_kb_close),FALSE,FALSE,0);

	but_kb_save = gtk_button_new_with_label("Save & Close");
	g_signal_connect(but_kb_save,"clicked",G_CALLBACK(keybinds_conf_save), NULL);
	gtk_box_pack_end(GTK_BOX(hbox_kb_s),GTK_WIDGET(but_kb_save),FALSE,FALSE,0);

	gtk_widget_show_all(win_kb_main);
}


void keybinds_window_create_set_actives (void) {

	int i , len;
	int Key = 0;
	char *pBuff = NULL;
	char Bind[HOTKEY_COUNT][64];

	strcpy(Bind[0],Hotkey_FUP);
	strcpy(Bind[1],Hotkey_SS_A_UP);
	strcpy(Bind[2],Hotkey_SS_W_UP);
	strcpy(Bind[3],Hotkey_SS_F_UP);
	strcpy(Bind[4],Hotkey_SS_A_CAP);
	strcpy(Bind[5],Hotkey_SS_W_CAP);
	strcpy(Bind[6],Hotkey_SS_F_CAP);
	strcpy(Bind[7],Hotkey_OPEN);

	for (i = 0 ; i < HOTKEY_COUNT ; ++i) {

		len = strlen(Bind[i]);
		Key = Bind[i][len-1];
		gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_key[i]), Key-33);

		pBuff = strtok(Bind[i], ">");
		if(pBuff != NULL) {
			if(pBuff[1] == 'C')
				gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_mod1[i]), 1);
			else if (pBuff[1] == 'S')
				gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_mod1[i]), 2);
			else if (pBuff[1] == 'A')
				gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_mod1[i]), 3);
			else if (pBuff[1] == 'W')
				gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_mod1[i]), 4);
			else
				gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_mod1[i]), 0);
		} else
			continue;

		pBuff = strtok(NULL, ">");
		if(pBuff != NULL) {
			if(pBuff[1] == 'C')
				gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_mod2[i]), 1);
			else if (pBuff[1] == 'S')
				gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_mod2[i]), 2);
			else if (pBuff[1] == 'A')
				gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_mod2[i]), 3);
			else if (pBuff[1] == 'W')
				gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_mod2[i]), 4);
			else
				gtk_combo_box_set_active(GTK_COMBO_BOX(com_kb_mod2[i]), 0);
		}

	}
	pBuff = NULL;

	gtk_switch_set_active (GTK_SWITCH(sw_kb[0]), bHotkey_FUP);
	gtk_switch_set_active (GTK_SWITCH(sw_kb[1]), bHotkey_SS_A_UP);
	gtk_switch_set_active (GTK_SWITCH(sw_kb[2]), bHotkey_SS_W_UP);
	gtk_switch_set_active (GTK_SWITCH(sw_kb[3]), bHotkey_SS_F_UP);
	gtk_switch_set_active (GTK_SWITCH(sw_kb[4]), bHotkey_SS_A_CAP);
	gtk_switch_set_active (GTK_SWITCH(sw_kb[5]), bHotkey_SS_W_CAP);
	gtk_switch_set_active (GTK_SWITCH(sw_kb[6]), bHotkey_SS_F_CAP);
	gtk_switch_set_active (GTK_SWITCH(sw_kb[7]), bHotkey_OPEN);

}

void keybinds_window_destroy (void) {

	gtk_widget_destroy(win_kb_main);
	win_kb_main = NULL;
}

void keybinds_conf_save (void) {

	int i;
	int Mod1[HOTKEY_COUNT], Mod2[HOTKEY_COUNT] , Key[HOTKEY_COUNT];
	char sMods[5][10] = { "" , "<CTRL>", "<SHIFT>", "<ALT>", "<WIN>" };
	char ConfFilePath [512];
	FILE *fpConfig = NULL;
	#ifdef __linux__
	sprintf(ConfFilePath ,"%s/keybinds.ini" ,DirConf);
	#elif __WIN32
	sprintf(ConfFilePath ,"%s\\keybinds.ini" ,DirConf);
	#endif
	fpConfig = fopen(ConfFilePath , "w");
	if (fpConfig == NULL) {
		log_error(true,"***Error: Couldn't write to config file 'keybinds.ini'");
		return;
	}

	for ( i = 0 ; i < HOTKEY_COUNT ; ++i) {
		Mod1[i] = gtk_combo_box_get_active(GTK_COMBO_BOX(com_kb_mod1[i]));
		Mod2[i] = gtk_combo_box_get_active(GTK_COMBO_BOX(com_kb_mod2[i]));
		Key[i] = gtk_combo_box_get_active(GTK_COMBO_BOX(com_kb_key[i]));

	}

	fprintf(fpConfig, "Hotkey_FUP = %s%s%c\n",
		Mod1[0] != Mod2[0] ? sMods[Mod1[0]] : "" , sMods[Mod2[0]], Key[0]+33);
	fprintf(fpConfig, "Hotkey_SS_A_UP = %s%s%c\n",
		Mod1[1] != Mod2[1] ? sMods[Mod1[1]] : "" , sMods[Mod2[1]], Key[1]+33);
	fprintf(fpConfig, "Hotkey_SS_W_UP = %s%s%c\n",
		Mod1[2] != Mod2[2] ? sMods[Mod1[2]] : "" , sMods[Mod2[2]], Key[2]+33);
	fprintf(fpConfig, "Hotkey_SS_F_UP = %s%s%c\n",
		Mod1[3] != Mod2[3] ? sMods[Mod1[3]] : "" , sMods[Mod2[3]], Key[3]+33);
    fprintf(fpConfig, "Hotkey_SS_A_CAP = %s%s%c\n",
		Mod1[4] != Mod2[4] ? sMods[Mod1[4]] : "" , sMods[Mod2[4]], Key[4]+33);
	fprintf(fpConfig, "Hotkey_SS_W_CAP = %s%s%c\n",
		Mod1[5] != Mod2[5] ? sMods[Mod1[5]] : "" , sMods[Mod2[5]], Key[5]+33);
	fprintf(fpConfig, "Hotkey_SS_F_CAP = %s%s%c\n",
		Mod1[6] != Mod2[6] ? sMods[Mod1[6]] : "" , sMods[Mod2[6]], Key[6]+33);
	fprintf(fpConfig, "Hotkey_OPEN = %s%s%c\n",
		Mod1[7] != Mod2[7] ? sMods[Mod1[7]] : "" , sMods[Mod2[7]], Key[7]+33);
	fprintf(fpConfig, "Enable_HK_FUP = %d\n", gtk_switch_get_active (GTK_SWITCH(sw_kb[0])));
	fprintf(fpConfig, "Enable_HK_SS_A_UP = %d\n", gtk_switch_get_active (GTK_SWITCH(sw_kb[1])));
	fprintf(fpConfig, "Enable_HK_SS_W_UP = %d\n", gtk_switch_get_active (GTK_SWITCH(sw_kb[2])));
	fprintf(fpConfig, "Enable_HK_SS_F_UP = %d\n",gtk_switch_get_active (GTK_SWITCH(sw_kb[3])));
	fprintf(fpConfig, "Enable_HK_SS_A_CAP = %d\n", gtk_switch_get_active (GTK_SWITCH(sw_kb[4])));
	fprintf(fpConfig, "Enable_HK_SS_W_CAP = %d\n", gtk_switch_get_active (GTK_SWITCH(sw_kb[5])));
	fprintf(fpConfig, "Enable_HK_SS_F_CAP = %d\n", gtk_switch_get_active (GTK_SWITCH(sw_kb[6])));
	fprintf(fpConfig, "Enable_HK_OPEN = %d\n",gtk_switch_get_active (GTK_SWITCH(sw_kb[7])));

	fclose(fpConfig);

	keybinds_window_destroy();
	keybinds_conf_load();
}


void keybinds_conf_load (void) {

    FILE *fpConfig;
	char ConfFilePath [512];
	char Getline[512];
	char Mod[2];
	char *pBuff = NULL;
	#ifdef __linux__
	sprintf(ConfFilePath ,"%s/keybinds.ini" ,DirConf);
	#elif __WIN32
	sprintf(ConfFilePath ,"%s\\keybinds.ini" ,DirConf);
	#endif
	fpConfig = fopen(ConfFilePath , "r");
	if (fpConfig == NULL) {
		log_error(true,"**Warning: Couldn't load config file 'keybinds.ini' - Using defaults.");
        keybinds_conf_create ();
		return;
	}

	#ifdef __linux__
    keybinder_unbind_all(Hotkey_FUP);
    keybinder_unbind_all(Hotkey_SS_A_UP);
    keybinder_unbind_all(Hotkey_SS_W_UP);
    keybinder_unbind_all(Hotkey_SS_F_UP);
    keybinder_unbind_all(Hotkey_SS_A_CAP);
    keybinder_unbind_all(Hotkey_SS_W_CAP);
    keybinder_unbind_all(Hotkey_SS_F_CAP);
    keybinder_unbind_all(Hotkey_OPEN);
	#elif _WIN32
	int i;
	for ( i = 0 ; i < HOTKEY_COUNT ; ++i)
        UnregisterHotKey(hWndGTK, 200+i);
	#endif


	while(fgets (Getline , 512 , fpConfig) != NULL) {
		if(strncmp(Getline, "Hotkey_FUP" , strlen("Hotkey_FUP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            snprintf(Hotkey_FUP, strlen(pBuff),"%s",pBuff);

		} else if(strncmp(Getline, "Hotkey_SS_A_UP" , strlen("Hotkey_SS_A_UP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            snprintf(Hotkey_SS_A_UP,strlen(pBuff),"%s",pBuff);

		} else if(strncmp(Getline, "Hotkey_SS_W_UP" , strlen("Hotkey_SS_W_UP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            snprintf(Hotkey_SS_W_UP,strlen(pBuff),"%s",pBuff);

		} else if(strncmp(Getline, "Hotkey_SS_F_UP" , strlen("Hotkey_SS_F_UP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            snprintf(Hotkey_SS_F_UP,strlen(pBuff),"%s",pBuff);

		} else if(strncmp(Getline, "Hotkey_SS_A_CAP" , strlen("Hotkey_SS_A_CAP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            snprintf(Hotkey_SS_A_CAP,strlen(pBuff),"%s",pBuff);

		} else if(strncmp(Getline, "Hotkey_SS_W_CAP" , strlen("Hotkey_SS_W_CAP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            snprintf(Hotkey_SS_W_CAP,strlen(pBuff),"%s",pBuff);

		} else if(strncmp(Getline, "Hotkey_SS_F_CAP" , strlen("Hotkey_SS_F_CAP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            snprintf(Hotkey_SS_F_CAP,strlen(pBuff),"%s",pBuff);

		} else if(strncmp(Getline, "Hotkey_OPEN" , strlen("Hotkey_OPEN")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            snprintf(Hotkey_OPEN,strlen(pBuff),"%s",pBuff);

		} else if(strncmp(Getline, "Enable_HK_FUP" , strlen("Enable_HK_FUP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bHotkey_FUP = atoi(Mod);
	    } else if(strncmp(Getline, "Enable_HK_SS_A_UP" , strlen("Enable_HK_SS_A_UP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bHotkey_SS_A_UP = atoi(Mod);
	    } else if(strncmp(Getline, "Enable_HK_SS_W_UP" , strlen("Enable_HK_SS_W_UP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bHotkey_SS_W_UP = atoi(Mod);
	    } else if(strncmp(Getline, "Enable_HK_SS_F_UP" , strlen("Enable_HK_SS_F_UP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bHotkey_SS_F_UP = atoi(Mod);
	    } else if(strncmp(Getline, "Enable_HK_SS_A_CAP" , strlen("Enable_HK_SS_A_CAP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bHotkey_SS_A_CAP = atoi(Mod);
	    } else if(strncmp(Getline, "Enable_HK_SS_W_CAP" , strlen("Enable_HK_SS_W_CAP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bHotkey_SS_W_CAP = atoi(Mod);
	    } else if(strncmp(Getline, "Enable_HK_SS_F_CAP" , strlen("Enable_HK_SS_F_CAP")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bHotkey_SS_F_CAP = atoi(Mod);
	    } else if(strncmp(Getline, "Enable_HK_OPEN" , strlen("Enable_HK_OPEN")) == 0) {

            pBuff = strstr(Getline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            bHotkey_OPEN = atoi(Mod);
	    }
	}
	pBuff = NULL;
	fclose(fpConfig);

	#ifdef __linux__
	char MenuLabel[128];
	if(bHotkey_FUP) {
		keybinder_bind_full(Hotkey_FUP, file_chooser_start, NULL, NULL);
		sprintf(MenuLabel,"Upload File(s)\t\t%s",Hotkey_FUP );
		gtk_menu_item_set_label ( GTK_MENU_ITEM(menuFileUp), MenuLabel);
	} else
		gtk_menu_item_set_label ( GTK_MENU_ITEM(menuFileUp), "Upload File");

	if (bHotkey_SS_A_UP) {

		keybinder_bind_full(Hotkey_SS_A_UP, take_screenshot, (void*)1, NULL);
		sprintf(MenuLabel,"Capture Area & UP\t%s", Hotkey_SS_A_UP);
		gtk_menu_item_set_label (GTK_MENU_ITEM(menuCapUp), MenuLabel);
	} else
		gtk_menu_item_set_label (GTK_MENU_ITEM(menuCapUp), "Capture Area & UP");

	if (bHotkey_SS_W_UP)
		keybinder_bind_full(Hotkey_SS_W_UP, take_screenshot, (void*)2, NULL);
	if (bHotkey_SS_F_UP)
		keybinder_bind_full(Hotkey_SS_F_UP, take_screenshot, (void*)3, NULL);

	if (bHotkey_SS_A_CAP) {

		keybinder_bind_full(Hotkey_SS_A_CAP, take_screenshot, (void*)4, NULL);
		sprintf(MenuLabel,"Capture Area\t\t%s", Hotkey_SS_A_CAP);
		gtk_menu_item_set_label (GTK_MENU_ITEM(menuCap) , MenuLabel);
	} else
		gtk_menu_item_set_label (GTK_MENU_ITEM(menuCap) , "Capture Area");


	if (bHotkey_SS_W_CAP)
		keybinder_bind_full(Hotkey_SS_W_CAP, take_screenshot, (void*)5, NULL);
	if (bHotkey_SS_F_CAP)
		keybinder_bind_full(Hotkey_SS_F_CAP, take_screenshot, (void*)6, NULL);

	if (bHotkey_OPEN) {

		keybinder_bind_full(Hotkey_OPEN, pomfit_open_links, NULL, NULL);
		sprintf(MenuLabel,"Open URL(s)\t\t%s", Hotkey_OPEN);
		gtk_menu_item_set_label (GTK_MENU_ITEM(menuOpen) , MenuLabel);
	} else
		gtk_menu_item_set_label (GTK_MENU_ITEM(menuOpen) , "Open URL(s)");
#elif _WIN32
     /* Win Key Code { 0, 2, 1, 4, 8} NULL , ALT , CTRL , SHIFT , WIN */
	int len;
	int Key[HOTKEY_COUNT] ;
	int Modes[HOTKEY_COUNT][2];
	char Bind[HOTKEY_COUNT][64];
	strcpy(Bind[0],Hotkey_FUP);
    strcpy(Bind[1],Hotkey_SS_A_UP);
    strcpy(Bind[2],Hotkey_SS_W_UP);
    strcpy(Bind[3],Hotkey_SS_F_UP);
    strcpy(Bind[4],Hotkey_SS_A_CAP);
    strcpy(Bind[5],Hotkey_SS_W_CAP);
    strcpy(Bind[6],Hotkey_SS_F_CAP);
    strcpy(Bind[7],Hotkey_OPEN);

	for (i = 0 ; i < HOTKEY_COUNT ; ++i) {

		len = strlen(Bind[i]);
		Key[i] = Bind[i][len-1];

		pBuff = strtok(Bind[i], ">");
		if(pBuff != NULL) {
			if(pBuff[1] == 'C')
				Modes[i][0] = 2;
			else if (pBuff[1] == 'S')
				Modes[i][0] = 4;
			else if (pBuff[1] == 'A')
				Modes[i][0] = 1;
			else if (pBuff[1] == 'W')
				Modes[i][0] = 8;
			else
				Modes[i][0] = 0;
		} else
			continue;

		pBuff = strtok(NULL, ">");
		if(pBuff != NULL) {
			if(pBuff[1] == 'C')
				Modes[i][1] = 2;
			else if (pBuff[1] == 'S')
				Modes[i][1] = 4;
			else if (pBuff[1] == 'A')
				Modes[i][1] = 1;
			else if (pBuff[1] == 'W')
				Modes[i][1] = 8;
			else
				Modes[i][1] = 0;
		}

	}
	pBuff = NULL;


    if(bHotkey_FUP)
		RegisterHotKey(hWndGTK, HOTKEY_FILEUP, Modes[0][1] | Modes[0][0], Key[0]);

	if (bHotkey_SS_A_UP)
		RegisterHotKey(hWndGTK, HOTKEY_AREA_SS_UP, Modes[1][1] | Modes[1][0], Key[1]);

	if (bHotkey_SS_W_UP)
		RegisterHotKey(hWndGTK, HOTKEY_WIN_SS_UP, Modes[2][1] | Modes[2][0], Key[2]);

	if (bHotkey_SS_F_UP)
		RegisterHotKey(hWndGTK, HOTKEY_FULL_SS_UP, Modes[3][1] | Modes[3][0], Key[3]);

	if (bHotkey_SS_A_CAP)
		RegisterHotKey(hWndGTK, HOTKEY_AREA_SS_CAP, Modes[4][1] | Modes[4][0], Key[4]);

	if (bHotkey_SS_W_CAP)
		RegisterHotKey(hWndGTK, HOTKEY_WIN_SS_CAP, Modes[5][1] | Modes[5][0], Key[5]);

	if (bHotkey_SS_F_CAP)
		RegisterHotKey(hWndGTK, HOTKEY_FULL_SS_CAP, Modes[6][1] | Modes[6][0], Key[6]);

	if (bHotkey_OPEN)
		RegisterHotKey(hWndGTK, HOTKEY_OPEN, Modes[7][1] | Modes[7][0], Key[7]);

#endif
}

void keybinds_conf_create (void) {

	char ConfFilePath [512];
	#ifdef __linux__
	sprintf(ConfFilePath ,"%s/keybinds.ini" ,DirConf);
	#elif __WIN32
	sprintf(ConfFilePath ,"%s\\keybinds.ini" ,DirConf);
	#endif
	FILE *fpConfig = fopen(ConfFilePath , "w");
	if (fpConfig == NULL) {
		log_error(true,"***Error: Couldn't write to config file 'keybinds.ini'");
		return;
	}
	fprintf(fpConfig, "Hotkey_FUP = <Ctrl><Alt>F\n");
	fprintf(fpConfig, "Hotkey_SS_A_UP = <Ctrl><Shift>1\n");
	fprintf(fpConfig, "Hotkey_SS_W_UP = <Ctrl><Shift>2\n");
	fprintf(fpConfig, "Hotkey_SS_F_UP = <Ctrl><Shift>3\n");
	fprintf(fpConfig, "Hotkey_SS_A_CAP = <Ctrl><Alt>1\n");
	fprintf(fpConfig, "Hotkey_SS_W_CAP = <Ctrl><Alt>2\n");
	fprintf(fpConfig, "Hotkey_SS_F_CAP = <Ctrl><Alt>3\n");
	fprintf(fpConfig, "Hotkey_OPEN = <Ctrl><Alt>B\n");
	fprintf(fpConfig, "Enable_HK_FUP = %d\n", true );
	fprintf(fpConfig, "Enable_HK_SS_A_UP = %d\n", true );
	fprintf(fpConfig, "Enable_HK_SS_W_UP = %d\n", false );
	fprintf(fpConfig, "Enable_HK_SS_F_UP = %d\n", false );
	fprintf(fpConfig, "Enable_HK_SS_A_CAP = %d\n", true );
	fprintf(fpConfig, "Enable_HK_SS_W_CAP = %d\n", false );
	fprintf(fpConfig, "Enable_HK_SS_F_CAP = %d\n", false );
	fprintf(fpConfig, "Enable_HK_OPEN = %d\n", false );

	fclose(fpConfig);

	keybinds_conf_load();
}

void keybinds_conf_default (void) {

	GtkWidget *dialog;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_message_dialog_new (GTK_WINDOW(win_kb_main), flags,
									 GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
									 "Reset all keybind settings ?");
	if(gtk_dialog_run(GTK_DIALOG (dialog))==GTK_RESPONSE_YES) {
		keybinds_conf_create();
		keybinds_window_create_set_actives();
	}

	gtk_widget_destroy (dialog);
}

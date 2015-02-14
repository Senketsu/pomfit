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

#include "resource.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
    #include <wingdi.h>
	#include <ShellAPI.h>
    #include <shlobj.h>
    #include <gdiplus.h>
    #include <gdiplus/gdiplusimagecodec.h>
	#include <gdk/gdkwin32.h>
#elif __linux__
	#include <keybinder.h>
#endif
#include "database.h"
#include "settings.h"
#include "log.h"
#include "manager.h"
#include "uploader.h"
#include "queue.h"
#include "other.h"
#include "keybinds.h"
#include "screenshot.h"
#include "profiles.h"

#ifdef __cplusplus
extern "C"
{
#endif
void take_screenshot (const char *keystring, void *Upload);
void file_chooser_start(const char *keystring, void *user_data);
void show_uri(const char *keystring, void *user_data);

static GtkWidget *file_chooser_create  (void);
static GtkStatusIcon *tray_icon_create(GtkWidget *window,GdkPixbuf *icon);
static gboolean open_all_links_g (GtkLinkButton *button, gpointer user_data);
static void file_chooser_get_list (void);
static void file_chooser_preview_update (GtkFileChooser *file_chooser, gpointer data);
static void tray_action_click (GtkStatusIcon *status_icon, gpointer user_data);
static void tray_action_menu (GtkStatusIcon *status_icon, guint button,
							guint activate_time, gpointer user_data);

#ifdef _WIN32
static LRESULT CALLBACK SelWndProc( HWND hWnd,  UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK WndProc( HWND hWnd,  UINT uMsg, WPARAM wParam, LPARAM lParam);
void tray_win32_create( HWND hWnd_In, UINT uID, UINT uCallbackMsg, UINT uIcon ,HINSTANCE hInst);
void tray_win32_remove( HWND hWnd_In, UINT uID);
void change_WndProc(HWND hWnd_p);
BOOL tray_win32_show_menu( HWND hWnd_In, POINT *curpos, int wDefaultItem);
#endif

#ifdef __cplusplus
}
#endif

char DirHome[64];
char DirConf[512];
char DirScrn[1024];
char DirUpScrn[1040];
char PathCookie[256];
char PathCurlOut[256];
char PathDB[512];
char PathIcon[256];
char PathIcon_Loc[256];
char LastFolder[512];
char ActProfile[256];
UpProfile ActiveProfile;

extern char *BatchLinks;
extern bool IsUploading;
extern bool bUP_Keep_SS;

extern char Hotkey_FUP[64];
extern char Hotkey_SS_A_UP[64];
extern char Hotkey_SS_W_UP[64];
extern char Hotkey_SS_F_UP[64];
extern char Hotkey_SS_A_CAP[64];
extern char Hotkey_SS_W_CAP[64];
extern char Hotkey_SS_F_CAP[64];
extern char Hotkey_OPEN[64];

extern bool bHotkey_FUP;
extern bool bHotkey_SS_A_UP;
extern bool bHotkey_SS_W_UP;
extern bool bHotkey_SS_F_UP;
extern bool bHotkey_SS_A_CAP;
extern bool bHotkey_SS_W_CAP;
extern bool bHotkey_SS_F_CAP;
extern bool bHotkey_OPEN;

gboolean IsGifToggled;
gboolean IsFirstRun = TRUE;

GtkApplication *pomfit;
GdkPixbuf *p_icon;
GtkStatusIcon *tray_icon;
GtkWidget *window, *sel_but, *link_but, *up_but, *check_but, *dialog, *status_bar, *preview;
GtkWidget *menu, *menuOpen, *menuCap, *menuCapUp, *menuFileUp;
#ifdef _WIN32
int PrimWidth , PrimHeight;
long SmallestX;
POINT CurMPos;
MONITORINFO  CurMonInfo;
LPOSVERSIONINFO lpVersionInfo;

bool CaptureWindow = false;
bool DrawRectangle = false;
bool IsLoged = false;
bool UploadSS;

POINT Begpt , Endpt , Temppt;
HDC DeskDC, MemDC;
HWND hWnd;
HWND hWndGTK;
HWND hWndSel;
HWND hWndActWnd;
HINSTANCE hInst;
WNDPROC OldWinProc;
NOTIFYICONDATA  nid;
#endif

static void pomfit_main_quit(GApplication *application, gpointer user_data) {

    #ifdef _WIN32
    tray_win32_remove(hWndGTK, 0);
    #endif
	free(BatchLinks);
	/// Remember it was fucking up thats why V
	if(user_data) {
        GApplication *application2 = (GApplication *)user_data;
        g_application_quit(application2);
    } else
        g_application_quit(application);
}

static void signal_open(GApplication  *application, GFile **files, gint n_files) {

	queue_window_create();

	int k ;
	char *pCliFile[n_files];
	for( k = 0 ; k < n_files ; ++k) {
		pCliFile[k] = g_file_get_path (files[k]);
		queue_tree_add_row(pCliFile[k]);
	}
	for( k = 0 ; k < n_files ; ++k)
		g_free(pCliFile[k]);

}
// Cause not all distros/DEs have tray
static void signal_activate(void) {

	if(IsFirstRun) {

		while (gtk_events_pending ())
			gtk_main_iteration ();
		gboolean IsEmbedded;
		#ifdef __linux__
		IsEmbedded = gtk_status_icon_is_embedded(GTK_STATUS_ICON(tray_icon));
		#elif _WIN32    // No need to check
		IsEmbedded = true;
		#endif
		if(!IsEmbedded) {
			gtk_widget_show(window);
			gtk_widget_set_no_show_all(GTK_WIDGET(window), FALSE);
		}
		else
			gtk_widget_set_no_show_all(GTK_WIDGET(window), TRUE);

		IsFirstRun = FALSE;
	} else {
		gtk_widget_show(window);
		gtk_widget_set_no_show_all(GTK_WIDGET(window), FALSE);
	}
}

static void signal_startup (GtkApplication *pomfit, gpointer user_data) {

	int j = 0;
	settings_paths_default();

	GError *error = NULL;
	GtkWidget *vbox_main, *nb_main, *vbox_nb_m_up, *vbox_nb_m_set, *vbox_nb_m_tools;
	GtkWidget *hbox, *but_main_quit, *but_main_set, *but_main_kb, *but_main_um, *but_main_uq;
	GtkWidget *lab_e;

	window = gtk_application_window_new(pomfit);
	gtk_window_set_title(GTK_WINDOW(window), "Pomf it!");
	g_signal_connect(window, "destroy", G_CALLBACK(pomfit_main_quit), pomfit);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	gtk_window_resize(GTK_WINDOW(window), 320, 100);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	
    #ifdef _WIN32
    /// Why ? Cause i'm faggot and why not..
    GtkWidget *wndProcWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_show_all(wndProcWin);
	gtk_widget_hide(wndProcWin);
    
	hWndGTK = (HWND)(GDK_WINDOW_HWND(gtk_widget_get_window(wndProcWin)));
    change_WndProc(hWndGTK);
    tray_win32_create( hWndGTK, 1, WM_APP, 0 , GetModuleHandle (NULL));
	
	#elif __linux__
	if(access(PathIcon_Loc, F_OK ) != 0 )
		p_icon = gdk_pixbuf_new_from_file_at_size (PathIcon, 64, 64, &error);
	else
		p_icon = gdk_pixbuf_new_from_file_at_size (PathIcon_Loc, 64, 64, &error);
	gtk_window_set_default_icon(GDK_PIXBUF(p_icon));
	
	GtkWidget *menuSep2, *menuUpQueue,*menuUpMan, *menuSet, *menuView, *menuExit;
	menu = gtk_menu_new();
	menuFileUp = gtk_menu_item_new_with_label("Upload File(s)\t\t");
	menuCapUp = gtk_menu_item_new_with_label("Capture Area & UP\t");
	menuCap = gtk_menu_item_new_with_label("Capture Area\t\t");
	menuOpen = gtk_menu_item_new_with_label("Open URL(s)\t");
	menuSep2 = gtk_separator_menu_item_new ();
	menuUpMan = gtk_menu_item_new_with_label("Uploads Manager\t");
	menuUpQueue = gtk_menu_item_new_with_label("Upload Queue\t");
	menuSet = gtk_menu_item_new_with_label("Settings");
	menuView = gtk_menu_item_new_with_label("Show / Hide");
	menuExit = gtk_menu_item_new_with_label("Exit");
	g_signal_connect(G_OBJECT(menuCapUp), "activate",
						G_CALLBACK(take_screenshot), (void*)1);
	g_signal_connect(G_OBJECT(menuCap), "activate",
						G_CALLBACK(take_screenshot), (void*)4);
	g_signal_connect(G_OBJECT(menuFileUp), "activate",
						G_CALLBACK(file_chooser_start), NULL);
	g_signal_connect(G_OBJECT(menuOpen), "activate",
						G_CALLBACK(pomfit_open_links), NULL);
	g_signal_connect(G_OBJECT(menuUpMan), "activate",
						G_CALLBACK(manager_window_create), NULL);
	g_signal_connect(G_OBJECT(menuUpQueue), "activate",
						G_CALLBACK(queue_window_create), NULL);
	g_signal_connect(G_OBJECT(menuSet), "activate",
						G_CALLBACK(settings_window_create), NULL);
	g_signal_connect(G_OBJECT(menuView), "activate",
						G_CALLBACK(tray_action_click), NULL);
	g_signal_connect(G_OBJECT(menuExit), "activate",
						G_CALLBACK(pomfit_main_quit), pomfit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuFileUp);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuCapUp);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuCap);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuOpen);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuSep2);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuUpMan);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuUpQueue);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuSet);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuView);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuExit);
	gtk_widget_show_all(menu);
	tray_icon = tray_icon_create(window, p_icon);
    #endif

	vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL , 4);
	gtk_container_add(GTK_CONTAINER(window), vbox_main);
	nb_main = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(vbox_main),nb_main);

	vbox_nb_m_up = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(nb_main), vbox_nb_m_up);
	lab_e = gtk_label_new("Upload");
	g_object_set(G_OBJECT( lab_e ), "can-focus", FALSE, NULL);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb_main),
		gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb_main), j ), lab_e);
	++j;
	vbox_nb_m_set = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(nb_main), vbox_nb_m_set);
	lab_e = gtk_label_new("Settings");
	g_object_set(G_OBJECT( lab_e ), "can-focus", FALSE, NULL);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb_main),
		gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb_main), j ), lab_e);
	++j;

	vbox_nb_m_tools = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(nb_main), vbox_nb_m_tools);
	lab_e = gtk_label_new("Tools");
	g_object_set(G_OBJECT( lab_e ), "can-focus", FALSE, NULL);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb_main),
		gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb_main), j ), lab_e);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 4);
	gtk_container_add(GTK_CONTAINER(vbox_nb_m_up), hbox);

	dialog = file_chooser_create();
	sel_but = gtk_file_chooser_button_new_with_dialog (dialog);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(sel_but),TRUE,TRUE,4);

	up_but = gtk_button_new_with_label("Upload");
	g_signal_connect(up_but, "clicked", G_CALLBACK(file_chooser_get_list), NULL);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(up_but),TRUE,TRUE,4);

	link_but =  gtk_link_button_new_with_label("http://pomf.se", "Pomf it !");
	g_signal_connect(link_but,"activate-link", G_CALLBACK(open_all_links_g),NULL);
	gtk_box_pack_start(GTK_BOX(vbox_nb_m_up),GTK_WIDGET(link_but),FALSE,TRUE,0);

	status_bar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox_nb_m_up),GTK_WIDGET(status_bar),FALSE,TRUE,0);
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, POMFIT_VERSION);

	but_main_set = gtk_button_new_with_label("Settings");
	g_signal_connect(but_main_set,"clicked",G_CALLBACK(settings_window_create), NULL);
	gtk_box_pack_start(GTK_BOX(vbox_nb_m_set),GTK_WIDGET(but_main_set),FALSE,FALSE,0);

	but_main_kb = gtk_button_new_with_label("Keybinds");
	g_signal_connect(but_main_kb,"clicked",G_CALLBACK(keybinds_window_create), NULL);
	gtk_box_pack_start(GTK_BOX(vbox_nb_m_set),GTK_WIDGET(but_main_kb),FALSE,FALSE,0);

#ifdef __linux__
	keybinder_init();
#endif
	keybinds_conf_load();

	but_main_um = gtk_button_new_with_label("Uploads Manager");
	g_signal_connect(but_main_um,"clicked",G_CALLBACK(manager_window_create), NULL);
	gtk_box_pack_start(GTK_BOX(vbox_nb_m_tools),GTK_WIDGET(but_main_um),FALSE,FALSE,0);

	but_main_uq = gtk_button_new_with_label("Upload Queue");
	g_signal_connect(but_main_uq,"clicked",G_CALLBACK(queue_window_create), NULL);
	gtk_box_pack_start(GTK_BOX(vbox_nb_m_tools),GTK_WIDGET(but_main_uq),FALSE,FALSE,0);

	but_main_quit = gtk_button_new_with_label("Exit");
	g_signal_connect(but_main_quit,"clicked",G_CALLBACK(pomfit_main_quit), pomfit);
	gtk_box_pack_start(GTK_BOX(vbox_main),GTK_WIDGET(but_main_quit),FALSE,FALSE,0);

	if(access(PathDB, F_OK ) != 0 )
		pomfit_db_create();

	pomfit_db_init();
	settings_conf_load();
	profiles_profile_activate(NULL,ActProfile);
	gtk_widget_show_all(window);

	gtk_notebook_set_current_page (GTK_NOTEBOOK(nb_main), (j -2));
	gtk_widget_hide(window);

#ifdef _WIN32
	/* SS selection window - transparent */
    hInst = GetModuleHandle(NULL);

	WNDCLASSEX wclx;
	memset(&wclx, 0, sizeof(wclx));
	wclx.cbSize         = sizeof( wclx );
	wclx.style          = 0;
	wclx.lpfnWndProc    = &SelWndProc;
	wclx.cbClsExtra     = 0;
	wclx.cbWndExtra     = 0;
	wclx.hInstance      = hInst;
    wclx.hIcon          = NULL;
	wclx.hCursor        = LoadCursor( NULL, IDC_ARROW );
	wclx.hbrBackground  = (HBRUSH)0;
	wclx.lpszMenuName   = NULL;
	wclx.lpszClassName  = "SelectionClass";
	RegisterClassEx( &wclx );

    hWndSel = CreateWindow( "SelectionClass", 0,
        (WS_BORDER | WS_EX_LAYERED | WS_EX_NOACTIVATE) ,
        0, 0, GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
        NULL, NULL, hInst, NULL );
	if ( !hWndSel ) {
		fprintf(stderr ,"Error: Can't create window!");
		MessageBox(NULL, TEXT("Error: Can't create window!"), TEXT("Pomf It !"), MB_OK );
	}

	SetWindowLong(hWndSel, GWL_STYLE, 0);
	SetWindowPos(hWndSel, 0, 0, 0, 0, 0, SWP_NOSIZE);
    UnregisterClass( "SelectionClass", hInst );

    PrimWidth = GetSystemMetrics(SM_CXSCREEN);
    PrimWidth = GetSystemMetrics(SM_CYSCREEN);
    SmallestX = 0;

#endif
}

int main (int argc,char *argv[]) {

	int status;

	pomfit=gtk_application_new("pomfit.uploader", G_APPLICATION_HANDLES_OPEN);
	g_signal_connect(pomfit, "startup", G_CALLBACK (signal_startup), NULL);
	g_signal_connect(pomfit, "activate", G_CALLBACK (signal_activate), NULL);
	g_signal_connect(pomfit, "open", G_CALLBACK (signal_open), argv);
	status = g_application_run(G_APPLICATION (pomfit), argc, argv);
	g_object_unref (pomfit);
	return status;
}

static void tray_action_click(GtkStatusIcon *status_icon, gpointer user_data) {

	gboolean NoShow;
	NoShow = gtk_widget_get_no_show_all(GTK_WIDGET(window));
	if(NoShow) {
		gtk_widget_show(window);
		gtk_widget_set_no_show_all(GTK_WIDGET(window), FALSE);
	} else {
		gtk_widget_hide(window);
		gtk_widget_set_no_show_all(GTK_WIDGET(window), TRUE);
    }
}

static void tray_action_menu(GtkStatusIcon *status_icon, guint button,
							guint activate_time, gpointer user_data) {

	gtk_menu_popup(GTK_MENU(user_data), NULL, NULL,
					gtk_status_icon_position_menu, status_icon, button, activate_time);
}

static GtkStatusIcon *tray_icon_create(GtkWidget *window, GdkPixbuf *icon) {

	GtkStatusIcon *tray_icon;
	tray_icon = gtk_status_icon_new();
	g_signal_connect(G_OBJECT(tray_icon), "activate",
						G_CALLBACK(tray_action_click), window);
	g_signal_connect(G_OBJECT(tray_icon), "popup-menu",
						G_CALLBACK(tray_action_menu), menu);
	gtk_status_icon_set_from_pixbuf(tray_icon, GDK_PIXBUF(icon));
	gtk_status_icon_set_tooltip_text(tray_icon,POMFIT_VERSION);
	gtk_status_icon_set_visible(tray_icon, TRUE);
	return tray_icon;
}

static GtkWidget *file_chooser_create (void) {

	GtkWidget *dialog_new;

	check_but = gtk_check_button_new_with_label ("Enable animation preview.");
	preview = gtk_image_new();
	gtk_widget_set_size_request (GTK_WIDGET(preview), 192, 192);

	GtkFileFilter *filter_all = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter_all, "*");
	gtk_file_filter_set_name(filter_all, "All files");
	GtkFileFilter *filter_img = gtk_file_filter_new ();
	GtkFileFilter *filter_aud = gtk_file_filter_new ();
	GtkFileFilter *filter_vid = gtk_file_filter_new ();
	gtk_file_filter_add_mime_type (filter_img, "image/*");
	gtk_file_filter_add_mime_type (filter_aud, "audio/*");
	gtk_file_filter_add_mime_type (filter_vid, "video/*");
	gtk_file_filter_set_name(filter_img, "Images");
	gtk_file_filter_set_name(filter_aud, "Audio");
	gtk_file_filter_set_name(filter_vid, "Video");

	dialog_new = gtk_file_chooser_dialog_new ("Upload File", NULL,
							GTK_FILE_CHOOSER_ACTION_OPEN,
							"Cancel", GTK_RESPONSE_CANCEL,
							"Select", GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog_new),LastFolder);
	gtk_file_chooser_set_use_preview_label(GTK_FILE_CHOOSER(dialog_new), FALSE);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog_new),filter_all);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog_new),filter_img);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog_new),filter_aud);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog_new),filter_vid);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog_new), TRUE);
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog_new),GTK_WIDGET(preview));
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog_new),check_but);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_but),IsGifToggled);

	g_signal_connect(dialog_new,"update-preview",G_CALLBACK(file_chooser_preview_update),
							preview);
	return dialog_new;
}

void file_chooser_start(const char *keystring, void *user_data) {

	if(IsUploading)
		return;
	GtkWidget *dialog_up;
	dialog_up = file_chooser_create();
	gchar *pLastFolder;
	if(gtk_dialog_run(GTK_DIALOG (dialog_up))==GTK_RESPONSE_ACCEPT) {

		GSList *pChoosedFiles = NULL;
		int i ,list;
		pChoosedFiles = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog_up));
		list = g_slist_length(pChoosedFiles);
		gpointer pFiles[list];
		for (i = 0 ; i < list ; ++i)
			pFiles[i]=NULL;
		for( i = 0; i < list ; ++i) {

			pFiles[i] = pChoosedFiles->data;
			pChoosedFiles = g_slist_next(pChoosedFiles);
		}
		g_slist_free_full (pChoosedFiles , g_free);
		pLastFolder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog_up));
		sprintf(LastFolder, "%s" ,pLastFolder);
		IsGifToggled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_but));
		gtk_widget_destroy(dialog_up);
		while (gtk_events_pending ())
			gtk_main_iteration ();

        char Buffer[1024];
		if(user_data != NULL) {
			for( i = 0; i < list ; ++i) {
                strcpy(Buffer,(char *)pFiles[i]);
                queue_tree_add_row(Buffer);
            }
		} else
			uploader_curl_file(pFiles , list);
		for (i = 0 ; i < list ; ++i)
			pFiles[i]=NULL;
	} else {

		pLastFolder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog_up));
		sprintf(LastFolder, "%s" ,pLastFolder);
		IsGifToggled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_but));
		gtk_widget_destroy(dialog_up);

		g_free(pLastFolder);
	}
}

static void file_chooser_get_list (void) {

	if(IsUploading) {
		gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_HALF);
		gtk_button_set_label(GTK_BUTTON(up_but), "Upload");
		return;
	}
	GSList *pChoosedFiles = NULL;
	gchar *pLastFolder;
	int i ,list;
	pChoosedFiles = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
	list = g_slist_length(pChoosedFiles);
	if(list < 1) {

		g_slist_free_full (pChoosedFiles , g_free);
		return;
	}
	gpointer pFiles[list];
	for (i = 0 ; i < list ; ++i)
		pFiles[i]=NULL;
	for( i = 0; i < list ; ++i) {

		pFiles[i] = pChoosedFiles->data;
		pChoosedFiles = g_slist_next(pChoosedFiles);
	}
	g_slist_free_full (pChoosedFiles , g_free);
	pLastFolder = gtk_file_chooser_get_current_folder(
						GTK_FILE_CHOOSER(dialog));
	sprintf(LastFolder, "%s" ,pLastFolder);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(dialog));
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(sel_but));
	uploader_curl_file(pFiles , list);
	for (i = 0 ; i < list ; ++i)
		pFiles[i]=NULL;

}

static void file_chooser_preview_update (GtkFileChooser *file_chooser, gpointer data) {

	bool IsStatic;
	gboolean IsCertain = FALSE;
	GdkPixbuf *pixbuf;
	GdkPixbufAnimation *pixbuf_anim;
	GtkWidget *preview_fc = GTK_WIDGET (data);

	char *pFilePath = NULL;
	char *pMimeType = NULL;
	pFilePath = gtk_file_chooser_get_preview_filename (file_chooser);
	if(!pFilePath)
		return;

	char *pContentType = g_content_type_guess (pFilePath, NULL, 0, &IsCertain);
	if (pContentType != NULL) {

		pMimeType = g_content_type_get_mime_type (pContentType);
//		g_print ("FILE :'%s'\n\tContent type:\t%s (certain: %s)\n\tMIME type:\t%s\n",
//						pFilePath,  pContentType,  IsCertain ? "yes" : "no", pMimeType);  /// Debug Line
		g_free (pContentType);
	}
	/// GTK+3.6.4-win32 gdk_pixbuf_new_from_file_at_size broken on images other than png - Temp solution duh
	#ifdef __linux__
	if(strncmp(pMimeType,"image",5) != 0  ) {
    #elif _WIN32
    if(strncmp(pMimeType,"image/png",9) != 0  ) {
    #endif
		pMimeType = NULL;
		pFilePath = NULL;
		gtk_image_clear (GTK_IMAGE (preview_fc));
		return;
	}
	pMimeType = NULL;
	IsGifToggled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_but));

	if(IsGifToggled) {

		pixbuf_anim = gdk_pixbuf_animation_new_from_file(pFilePath, NULL);
		IsStatic = gdk_pixbuf_animation_is_static_image (pixbuf_anim);
		if(IsStatic) {
			pixbuf = gdk_pixbuf_new_from_file_at_size (pFilePath,
														192 , 192, NULL);
			gtk_image_set_from_pixbuf (GTK_IMAGE (preview_fc), pixbuf);
			g_object_unref (pixbuf);
			g_object_unref (pixbuf_anim);
		} else {
			gtk_image_set_from_animation (GTK_IMAGE (preview_fc), pixbuf_anim);
			g_object_unref (pixbuf_anim);
		}
	} else {
		pixbuf = gdk_pixbuf_new_from_file_at_size (pFilePath,
														192, 192,NULL);
		gtk_image_set_from_pixbuf (GTK_IMAGE (preview_fc), pixbuf);
		g_object_unref (pixbuf);
	}
	pFilePath = NULL;
}

void show_uri(const char *keystring, void *uri) {
    #ifdef __linux__
	GdkScreen *DefScreen = gdk_screen_get_default();
	gtk_show_uri(DefScreen, (char *)uri, GDK_CURRENT_TIME, NULL);
	#elif _WIN32
	if(keystring != NULL)
        ShellExecute(NULL,NULL, "explorer.exe", (LPCSTR)uri  ,NULL, SW_SHOWNORMAL);
    else
        ShellExecute(NULL, "open", (LPCSTR)uri, NULL , NULL , 0);
    #endif
}

static gboolean open_all_links_g (GtkLinkButton *button, gpointer user_data) {
	pomfit_open_links(NULL,NULL);
	return TRUE;
}

void take_screenshot (const char *keystring, void *Upload) {

	char sMode[2];
	snprintf(sMode,sizeof(sMode),"%d",Upload);
	int Mode = atoi(sMode);
	screenshot_glob_take_ss(false, Mode);

}
#ifdef _WIN32

void change_WndProc(HWND hWnd_p) {

   OldWinProc = (WNDPROC) GetWindowLong(hWnd_p, GWL_WNDPROC);
   SetWindowLong(hWnd_p, GWL_WNDPROC, (LONG)(WNDPROC)WndProc);
}

BOOL tray_win32_show_menu( HWND hWnd_In, POINT *curpos, int wDefaultItem ) {

    char MenuBuffer[512];
	HMENU hPop = CreatePopupMenu();

    sprintf(MenuBuffer,"Upload File(s)\t %s", bHotkey_FUP ? Hotkey_FUP : "");
    InsertMenu( hPop, 2, MF_BYPOSITION | MF_STRING, ID_FILEUP, MenuBuffer);
    InsertMenu( hPop, 3, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
    sprintf(MenuBuffer,"Capture Area & UP\t %s",  bHotkey_SS_A_UP ? Hotkey_SS_A_UP : "");
    InsertMenu( hPop, 4, MF_BYPOSITION | MF_STRING, ID_AREA_SS_UP, MenuBuffer);
    sprintf(MenuBuffer,"Capture Window & UP\t %s", bHotkey_SS_W_UP ? Hotkey_SS_W_UP : "");
    InsertMenu( hPop, 5, MF_BYPOSITION | MF_STRING, ID_WIN_SS_UP, MenuBuffer);
    sprintf(MenuBuffer,"Capture Fullscreen & UP\t %s", bHotkey_SS_F_UP ? Hotkey_SS_F_UP : "");
	InsertMenu( hPop, 6, MF_BYPOSITION | MF_STRING, ID_FULL_SS_UP, MenuBuffer);
	InsertMenu( hPop, 7, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
    sprintf(MenuBuffer,"Capture Area\t %s",  bHotkey_SS_A_CAP ? Hotkey_SS_A_CAP : "");
    InsertMenu( hPop, 8, MF_BYPOSITION | MF_STRING, ID_AREA_SS_CAP, MenuBuffer);
    sprintf(MenuBuffer,"Capture Window\t %s",  bHotkey_SS_W_CAP ? Hotkey_SS_W_CAP : "");
    InsertMenu( hPop, 9, MF_BYPOSITION | MF_STRING, ID_WIN_SS_CAP, MenuBuffer);
    sprintf(MenuBuffer,"Capture Fullscreen\t %s",  bHotkey_SS_F_CAP ? Hotkey_SS_F_CAP : "");
	InsertMenu( hPop, 10, MF_BYPOSITION | MF_STRING, ID_FULL_SS_CAP, MenuBuffer);

    InsertMenu( hPop, 11, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
    sprintf(MenuBuffer,"Open URL(s)\t %s", bHotkey_OPEN ? Hotkey_OPEN : "");
	InsertMenu( hPop, 12, MF_BYPOSITION | MF_STRING, ID_OPEN, MenuBuffer);
	InsertMenu( hPop, 13, MF_BYPOSITION | MF_STRING, ID_SHOW_UM, "Uploads Manager" );
	InsertMenu( hPop, 14, MF_BYPOSITION | MF_STRING, ID_SHOW_UQ, "Upload Queue" );
	InsertMenu( hPop, 15, MF_BYPOSITION | MF_STRING, ID_SHOW_SETTINGS, "Settings" );
	InsertMenu( hPop, 16, MF_BYPOSITION | MF_STRING, ID_EXIT , "Exit" );

	SetMenuDefaultItem( hPop, ID_SHOW_SETTINGS, FALSE );
	SetFocus ( hWnd_In );
	SendMessage ( hWnd_In, WM_INITMENUPOPUP, (WPARAM)hPop, 0 );

	POINT pt;
	if (!curpos) {
		GetCursorPos( &pt );
		curpos = &pt;
	}

	WORD cmd = TrackPopupMenu( hPop,
                        TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY ,
                        curpos->x, curpos->y, 0, hWnd_In, NULL );

	SendMessage( hWnd_In, WM_COMMAND, cmd, 0 );

	DestroyMenu(hPop);

	return 0;
}

void tray_win32_remove( HWND hWnd_In, UINT uID ) {

	NOTIFYICONDATA  nid_rem;
	nid_rem.hWnd = hWnd_In;
	nid_rem.uID  = uID;
	Shell_NotifyIcon( NIM_DELETE, &nid_rem );
}

void tray_win32_create( HWND hWnd_In, UINT uID, UINT uCallbackMsg, UINT uIcon ,HINSTANCE hInst) {

	nid.hWnd = hWnd_In;
	nid.uID = uID;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.uCallbackMessage = uCallbackMsg;
    nid.hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_MYICON ) );
	strcpy(nid.szTip, POMFIT_VERSION);

	Shell_NotifyIcon( NIM_ADD, &nid );
}

static LRESULT CALLBACK WndProc( HWND hWnd,  UINT uMsg, WPARAM wParam, LPARAM lParam ) {

	switch (uMsg) {

        case WM_HOTKEY:
        {
            switch (LOWORD(wParam)) {
                case HOTKEY_FILEUP:
                    PostMessage( hWnd, WM_COMMAND, ID_FILEUP, 0 );
                    break;
                case HOTKEY_AREA_SS_UP:
                    PostMessage( hWnd, WM_COMMAND , ID_AREA_SS_UP, 0 );
                    break;
                case HOTKEY_WIN_SS_UP:
                    hWndActWnd = GetForegroundWindow();
                    screenshot_win32_cap_act_win(true);
                    break;
                case HOTKEY_FULL_SS_UP:
                    PostMessage( hWnd, WM_COMMAND , ID_FULL_SS_UP, 0 );
                    break;
                 case HOTKEY_AREA_SS_CAP:
                    PostMessage( hWnd, WM_COMMAND , ID_AREA_SS_CAP, 0 );
                    break;
                case HOTKEY_WIN_SS_CAP:
                    hWndActWnd = GetForegroundWindow();
                    screenshot_win32_cap_act_win(false);
                    break;
                case HOTKEY_FULL_SS_CAP:
                    PostMessage( hWnd, WM_COMMAND, ID_FULL_SS_CAP, 0 );
                    break;
                case HOTKEY_OPEN:
                    PostMessage( hWnd, WM_COMMAND, ID_OPEN, 0 );
                    break;
                }
        }
		return DefWindowProc( hWnd, uMsg, wParam, lParam );

        case WM_COMMAND:
        {
            switch (LOWORD(wParam)) {
                case  ID_FILEUP:
                    file_chooser_start(NULL,NULL);
                    break;
                case ID_AREA_SS_UP:
                    screenshot_glob_take_ss(true, 2);
                    break;
                case ID_WIN_SS_UP:
                    screenshot_glob_take_ss(true, 1);
                    break;
                case ID_FULL_SS_UP:
                    screenshot_glob_take_ss(true, 0);
                    break;
                case ID_AREA_SS_CAP:
                    screenshot_glob_take_ss(false, 2);
                    break;
                case ID_WIN_SS_CAP:
                    screenshot_glob_take_ss(false, 1);
                    break;
                case ID_FULL_SS_CAP:
                    screenshot_glob_take_ss(false, 0);
                    break;
                case ID_OPEN:
                    pomfit_open_links(NULL,NULL);
                    break;
                case ID_SHOW_UQ:
                    queue_window_create();
                    break;
                case ID_SHOW_UM:
                    manager_window_create();
                    break;
                case ID_SHOW_SETTINGS:
                    settings_window_create();
                    break;
                case ID_EXIT:
                    pomfit_main_quit(NULL,pomfit);
                    break;
            }
        case WM_APP:
         {
            switch (lParam) {
                case WM_LBUTTONUP:
                    tray_action_click(NULL,NULL);
                    break;
                case WM_RBUTTONUP:
                    tray_win32_show_menu(hWnd, NULL, -1 );
                    PostMessage( hWnd, WM_APP + 1, 0, 0 );
                    break;
            }
		 }
		 break;
        }
	}
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}


static LRESULT CALLBACK SelWndProc( HWND hWnd,  UINT uMsg, WPARAM wParam, LPARAM lParam ) {

    int height, width;
    HBITMAP hBitmap;
    HDC DlgDC;

    static RECT rcSelection;
    static bool IsSelecting;

	switch (uMsg) {

        case WM_LBUTTONDOWN:
            if(DrawRectangle) {
            if(IsSelecting)
                ReleaseCapture();

            IsSelecting = true;
            GetCursorPos(&Begpt);
            rcSelection.left = (short)LOWORD(lParam);
            rcSelection.top = (short)HIWORD(lParam);
            rcSelection.right = (short)LOWORD(lParam);
            rcSelection.bottom = (short)HIWORD(lParam);

            screenshot_win32_draw_selection(hWnd, &rcSelection);
            SetCapture(hWndSel);
            }
            else if(CaptureWindow) {
                GetCursorPos(&Begpt);
                ShowWindow(hWndSel,0);
                hWndActWnd= WindowFromPoint(Begpt);
                screenshot_win32_cap_act_win(UploadSS);
                CaptureWindow = false;
                UploadSS = false;
            }
            break;
        case WM_MOUSEMOVE:
            if(DrawRectangle && IsSelecting) {
                RECT rcClient;

                screenshot_win32_draw_selection(hWnd, &rcSelection);

                rcSelection.right = (short)LOWORD(lParam);
                rcSelection.bottom = (short)HIWORD(lParam);

                GetClientRect(hWnd, &rcClient);
                if(rcSelection.right < 0)
                    rcSelection.right = 0;
                if(rcSelection.right > rcClient.right)
                    rcSelection.right = rcClient.right;
                if(rcSelection.bottom < 0)
                    rcSelection.bottom = 0;
                if(rcSelection.bottom > rcClient.bottom)
                    rcSelection.bottom = rcClient.bottom;

                screenshot_win32_draw_selection(hWnd, &rcSelection);
            }
            if(DrawRectangle && !IsSelecting){
                if((short)LOWORD(lParam) >= PrimWidth || (short)LOWORD(lParam) <= 0 ) {
                    Sleep(1000);
                    GetCursorPos(&CurMPos);
                    ShowWindow(hWndSel,0);
                    HMONITOR CurMonitor = MonitorFromPoint(CurMPos,MONITOR_DEFAULTTONULL);
                    CurMonInfo.cbSize = sizeof(MONITORINFO);
                    GetMonitorInfo(CurMonitor,(LPMONITORINFO)&CurMonInfo);
                    SetWindowPos(hWndSel, 0, CurMonInfo.rcMonitor.left, 0, 0, 0, SWP_NOSIZE);
                    ShowWindow(hWndSel,SW_SHOW);
                    UpdateWindow(hWndSel);
                }
            }
            break;
        case WM_LBUTTONUP:
            if(DrawRectangle == true) {
                GetCursorPos(&Endpt);
                DrawRectangle = false;
                IsSelecting = false;
                ShowWindow(hWndSel,0);

                rcSelection.right = (short)LOWORD(lParam);
                rcSelection.bottom = (short)HIWORD(lParam);

                width= Endpt.x >Begpt.x ?Endpt.x-Begpt.x : Begpt.x -Endpt.x;
                height= Endpt.y >Begpt.y ?Endpt.y-Begpt.y : Begpt.y -Endpt.y;

                if(width <= 1 && height <= 1) {
                    GetCursorPos(&Begpt);
                    ShowWindow(hWndSel,0);
                    hWndActWnd= WindowFromPoint(Begpt);
                    screenshot_win32_cap_act_win(UploadSS);
                    CaptureWindow = false;
                    UploadSS = false;
                    break;
                }
                if(Begpt.x > Endpt.x && Begpt.y > Endpt.y) {
                    Temppt = Begpt;
                    Begpt =Endpt;
                    Endpt = Begpt;
                }
                if(Begpt.x >Endpt.x && Begpt.y < Endpt.y) {
                    Temppt.x = Begpt.x;
                    Begpt.x =Endpt.x;
                    Endpt.x = Begpt.x;
                }
                if(Begpt.x < Endpt.x && Begpt.y > Endpt.y)  {
                    Temppt.y = Begpt.y;
                    Begpt.y =Endpt.y;
                    Endpt.y = Begpt.y;
                }
                DeskDC = GetDC(NULL);
                DlgDC = GetDC(hWnd);
                MemDC = CreateCompatibleDC(DeskDC);
                hBitmap = CreateCompatibleBitmap(DlgDC,width,height);
                SelectObject(MemDC,hBitmap);
                BitBlt(MemDC,0,0,width,height ,DeskDC,Begpt.x,Begpt.y,SRCCOPY);
                screenshot_win32_save_bitmap(MemDC, hBitmap,UploadSS);

                ReleaseDC(hWnd,DlgDC);
                ReleaseDC(NULL,DeskDC);
                DeleteDC(MemDC);
                DeleteObject(hBitmap);

                UploadSS = false;
            }
            break;
    }
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}
#endif

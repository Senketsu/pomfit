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

#include "header.h"
#include "pomfit.h"

char Hotkey_FUP[64];
char Hotkey_PUP[64];
char Hotkey_CAP[64];
char Hotkey_OPEN[64];

void gtk_login_handle(int Mode);
void GetLoginData (void);
void UserLogout (void);
static char* get_home();
static void get_file_list (void);
static void LoadDefaults (void);
static void LoadSettings (void);
static void SaveSettings (void);
static GtkWidget* make_dialog (int id);
static gboolean open_all_links_g (GtkLinkButton *button, gpointer user_data);
static void take_screenshot (const char *keystring, void *Upload);
static GtkStatusIcon *create_tray_icon(GtkWidget *window) ;
static void choose_file(const char *keystring, void *user_data);
static void update_preview (GtkFileChooser *file_chooser, gpointer data);
static void tray_on_click(GtkStatusIcon *status_icon, gpointer user_data);
static void tray_on_menu(GtkStatusIcon *status_icon, guint button,
							guint activate_time, gpointer user_data);

char HomeDir[64];
char *OpenQueue = NULL;
char LastFolder[512];
int fd[2];
bool IsLoged = false;
bool KeepUploadedSS;

GtkStatusIcon *tray_icon;
GdkPixbuf *p_icon;
GtkWidget *window;
GtkWidget *sel_but;
GtkWidget *link_but;
GtkWidget *up_but;
GtkWidget *menu;
GtkWidget *check_but0;
GtkWidget *check_but;
GtkWidget *check_but_keepSS;
GtkWidget *dialog;
GtkWidget *status_bar;
GtkWidget *preview;
GtkWidget *entry_conf[4];

GtkWidget *login_but , *logout_but;
GtkWidget *entry_pass, *entry_email;
GtkWidget *vbox1, *vbox3, *label_pass, *label_email;
GtkWidget *hbox_log1, *hbox_log2, *label_session;
GtkWidget *menuOpen, *menuCap, *menuCapUp, *menuFileUp, *menuAcc;

gboolean IsGifToggled;
gboolean IsFirstRun = TRUE;
gboolean IsDestroyed = TRUE;
gboolean IsQueued = FALSE;
gboolean Queue = FALSE;

static void pomfit_main_quit(GApplication *application, gpointer user_data)
{
	if(access(CookieFile, F_OK ) != -1 ) {
	pomfit_curl_logout();
	}
	write(fd[1], "KillChild", (strlen("KillChild")+1));
	GApplication *application2 = user_data;
	g_application_quit(application2);
}

void handle_queue (void)
{
	int j = 0;
	int i = 0;
	size_t size = strlen(OpenQueue);
	char TokQueue[size+1];
	char CopyQueue[size+1];
	void *pTokQueue;
	strcpy(CopyQueue,OpenQueue);
	strcpy(TokQueue,OpenQueue);
	pTokQueue = strtok(TokQueue, "$");
	while (pTokQueue != NULL)
	{
		j+=1;
		pTokQueue = strtok (NULL, "$");
	}
	gpointer pQueuedFiles[j];
	for(i = 0; i < j ; ++i)
		pQueuedFiles[i] = NULL;
	i = 0;
	pTokQueue = strtok(CopyQueue, "$");
	while (pTokQueue != NULL)
	{
		pQueuedFiles[i] = pTokQueue;
		i+=1;
		pTokQueue = strtok (NULL, "$");
	}
	Queue = FALSE;
	pomfit_upload_file(pQueuedFiles, j);
	for(i = 0; i < j ; ++i)
		pQueuedFiles[i] = NULL;
}

static void open_file(GApplication  *application, GFile **files, gint n_files,
						const gchar *hint)
{
	if(IsFirstRun)
		gtk_widget_set_no_show_all(GTK_WIDGET(window), TRUE);
	if(n_files > 1)
	{
		int k ;
		gpointer pCliFile[n_files];
		for( k = 0 ; k < n_files ; ++k)
			pCliFile[k] = (void*)g_file_get_path (files[k]);

		pomfit_upload_file(pCliFile, n_files);
		for( k = 0 ; k < n_files ; ++k)
			g_free(pCliFile[k]);
		return;
	}
	else
	{
		gchar *pQueue;
		pQueue = g_file_get_path (files[0]);
		size_t size_file = strlen(pQueue);
		if(!Queue)
		{
			OpenQueue = realloc(OpenQueue,(sizeof(char)*(size_file+2)));
			sprintf(OpenQueue,"%s$",pQueue);
		}
		else
		{
			size_t size = strlen(OpenQueue);

			OpenQueue = realloc(OpenQueue,(size+size_file+2));
			sprintf(strchr(OpenQueue, '\0'),"%s$",pQueue);
		}
		if(!Queue)
		{
			Queue = TRUE;
			time_t timer = time(NULL);
			while(time(NULL) < (timer + 5))
				while (gtk_events_pending ())
					gtk_main_iteration ();
			handle_queue();
			free(OpenQueue);
			pQueue = NULL;
		}
	}

}

static void activate(void)
{
	if(IsFirstRun)
	{
		while (gtk_events_pending ())
			gtk_main_iteration ();
		gboolean IsEmbedded;
		IsEmbedded = gtk_status_icon_is_embedded(GTK_STATUS_ICON(tray_icon));
		if(!IsEmbedded)
		{
			gtk_widget_show(window);
			gtk_widget_set_no_show_all(GTK_WIDGET(window), FALSE);
		}
		else
			gtk_widget_set_no_show_all(GTK_WIDGET(window), TRUE);
		IsFirstRun = FALSE;
	}
	else
	{
		gtk_widget_show(window);
		gtk_widget_set_no_show_all(GTK_WIDGET(window), FALSE);
	}
}

static void startup (GtkApplication *pomfit, gpointer user_data)
{
	strcpy(HomeDir, get_home());
	strcpy(LastFolder, HomeDir);
	sprintf(ConfDir,"%s/.config/Pomfit",HomeDir);
	pomfit_mkdir(ConfDir);
	sprintf(SsDir,"%s/Pictures",HomeDir);
	pomfit_mkdir(SsDir);
	sprintf(SsDir,"%s/Pictures/Screenshots",HomeDir);
	pomfit_mkdir(SsDir);
	sprintf(UploadedSsDir,"%s/Uploaded",SsDir);
	pomfit_mkdir(UploadedSsDir);
	sprintf(CookieFile,"%s/pomfit_phpsessid",ConfDir);

	GError *error = NULL;
	GtkWidget *vbox_main, *nb_main, *label_login, *label_upload, *label_settings;
	GtkWidget *vbox2, *vbox3, *vbox4;
	GtkWidget *hbox, *hbox_conf[4],*label_empty,*label_keybinds;
	GtkWidget *quit_but, *save_but;
	GtkWidget *menuSep1, *menuSep2, *menuView, *menuExit;

	window = gtk_application_window_new(pomfit);
	gtk_window_set_title(GTK_WINDOW(window), "Pomf it!");
	g_signal_connect(window, "destroy", G_CALLBACK(pomfit_main_quit), pomfit);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	gtk_window_resize(GTK_WINDOW(window), 320, 100);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	p_icon=gdk_pixbuf_new_from_file("/usr/local/share/pixmaps/pomfit.png",&error);
	gtk_window_set_default_icon(GDK_PIXBUF(p_icon));

	menu = gtk_menu_new();
	menuAcc = gtk_menu_item_new_with_label("You are not logged in..");
	menuSep1 = gtk_separator_menu_item_new ();
	menuCapUp = gtk_menu_item_new_with_label("Upload Screencap\t");
	menuFileUp = gtk_menu_item_new_with_label("Upload File\t\t");
	menuCap = gtk_menu_item_new_with_label("Screencap\t\t");
	menuSep2 = gtk_separator_menu_item_new ();
	menuOpen = gtk_menu_item_new_with_label("Open Last Link(s)\t");
	menuView = gtk_menu_item_new_with_label("Show / Hide");
	menuExit = gtk_menu_item_new_with_label("Exit");
	g_signal_connect(G_OBJECT(menuAcc), "activate",
						G_CALLBACK(open_acc_page), NULL);
	g_signal_connect(G_OBJECT(menuCapUp), "activate",
						G_CALLBACK(take_screenshot), (void*)1);
	g_signal_connect(G_OBJECT(menuCap), "activate",
						G_CALLBACK(take_screenshot), (void*)0);
	g_signal_connect(G_OBJECT(menuFileUp), "activate",
						G_CALLBACK(choose_file), NULL);
	g_signal_connect(G_OBJECT(menuOpen), "activate",
						G_CALLBACK(open_all_links), NULL);
	g_signal_connect(G_OBJECT(menuView), "activate",
						G_CALLBACK(tray_on_click), NULL);
	g_signal_connect(G_OBJECT(menuExit), "activate",
						G_CALLBACK(pomfit_main_quit), pomfit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuAcc);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuSep1);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuCapUp);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuFileUp);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuCap);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuSep2);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuOpen);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuView);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuExit);
	gtk_widget_show_all(menu);
	tray_icon = create_tray_icon(window);

	keybinder_init();

	vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL , 4);
	gtk_container_add(GTK_CONTAINER(window), vbox_main);
	nb_main = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(vbox_main),nb_main);

	vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(nb_main), vbox1);
	label_login = gtk_label_new("Login");
	g_object_set(G_OBJECT(label_login), "can-focus", FALSE, NULL);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb_main),
		gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb_main), 0),label_login);


	vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(nb_main), vbox2);
	label_upload = gtk_label_new("Upload");
	g_object_set(G_OBJECT(label_upload), "can-focus", FALSE, NULL);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb_main),
		gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb_main), 1),label_upload);
	
	vbox3 = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(nb_main), vbox3);
	label_settings = gtk_label_new("Settings");
	g_object_set(G_OBJECT(label_settings), "can-focus", FALSE, NULL);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb_main),
		gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb_main), 2),label_settings);
	
	vbox4 = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(nb_main), vbox4);
	label_keybinds = gtk_label_new("Keybinds");
	g_object_set(G_OBJECT(label_keybinds), "can-focus", FALSE, NULL);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb_main),
		gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb_main), 3),label_keybinds);

	/* Login page */
	gtk_login_handle(0);
	/* Upload page */
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 4);
	gtk_container_add(GTK_CONTAINER(vbox2), hbox);

	dialog = make_dialog(0);
	sel_but = gtk_file_chooser_button_new_with_dialog (dialog);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(sel_but),TRUE,TRUE,4);

	up_but = gtk_button_new_with_label("Upload");
	g_signal_connect(up_but, "clicked", G_CALLBACK(get_file_list), NULL);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(up_but),TRUE,TRUE,4);

	link_but =  gtk_link_button_new_with_label("http://pomf.se", "Pomf it !");
	g_signal_connect(link_but,"activate-link",
								G_CALLBACK(open_all_links_g),NULL);
	gtk_box_pack_start(GTK_BOX(vbox2),GTK_WIDGET(link_but),FALSE,TRUE,0);

	status_bar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox2),GTK_WIDGET(status_bar),FALSE,TRUE,0);
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, POMFIT_VERSION);
	/* Settings */
	check_but_keepSS = gtk_check_button_new_with_label("Keep uploaded"
			" screen shots");
	gtk_box_pack_start(GTK_BOX(vbox3),GTK_WIDGET(check_but_keepSS),TRUE,TRUE,4);
	
	save_but = gtk_button_new_with_label("Save Settings & Keybinds");
	g_signal_connect(save_but,"clicked",G_CALLBACK(SaveSettings), NULL);
	gtk_box_pack_start(GTK_BOX(vbox3),GTK_WIDGET(save_but),FALSE,FALSE,0);
	/* Keybinds */
	int i;
	for ( i = 0 ; i < 4 ; ++i) {
		hbox_conf[i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL ,4);
		gtk_container_add (GTK_CONTAINER(vbox4),hbox_conf[i]);
		entry_conf[i] = gtk_entry_new();
		gtk_box_pack_end (GTK_BOX(hbox_conf[i]),
								GTK_WIDGET(entry_conf[i]), TRUE, TRUE, 0);
	}
	label_empty = gtk_label_new("File Up:");
	gtk_container_add (GTK_CONTAINER(hbox_conf[0]), label_empty);
	
	label_empty = gtk_label_new("Ss Up: ");
	gtk_container_add (GTK_CONTAINER(hbox_conf[1]), label_empty);
	
	label_empty = gtk_label_new("SS:	  ");
	gtk_container_add (GTK_CONTAINER(hbox_conf[2]), label_empty);
	
	label_empty = gtk_label_new("Open: ");
	gtk_container_add (GTK_CONTAINER(hbox_conf[3]), label_empty);
	
	
	
	quit_but = gtk_button_new_with_label("Exit");
	g_signal_connect(quit_but,"clicked",G_CALLBACK(pomfit_main_quit), pomfit);
	gtk_box_pack_start(GTK_BOX(vbox_main),GTK_WIDGET(quit_but),FALSE,FALSE,0);

	LoadSettings();
	
	gtk_widget_show_all(window);
	gtk_notebook_set_current_page (GTK_NOTEBOOK(nb_main),1);
	gtk_widget_hide(window);

}

int main (int argc,char *argv[])
{
	GtkApplication *pomfit;
	int status;

	pomfit=gtk_application_new("pomfit.uploader", G_APPLICATION_HANDLES_OPEN);
	g_signal_connect(pomfit, "startup", G_CALLBACK (startup), NULL);
	g_signal_connect(pomfit, "activate", G_CALLBACK (activate), NULL);
	g_signal_connect(pomfit, "open", G_CALLBACK (open_file), argv);
	status = g_application_run(G_APPLICATION (pomfit), argc, argv);
	g_object_unref (pomfit);
	return status;
}

static void tray_on_click(GtkStatusIcon *status_icon, gpointer user_data)
{
	gboolean NoShow;
	NoShow = gtk_widget_get_no_show_all(GTK_WIDGET(window));
	if(NoShow)
	{
		gtk_widget_show(window);
		gtk_widget_set_no_show_all(GTK_WIDGET(window), FALSE);
	}
	else
	{
		gtk_widget_hide(window);
		gtk_widget_set_no_show_all(GTK_WIDGET(window), TRUE);
    }
}

static void tray_on_menu(GtkStatusIcon *status_icon, guint button,
							guint activate_time, gpointer user_data)
{
	gtk_menu_popup(GTK_MENU(user_data), NULL, NULL,
					gtk_status_icon_position_menu,
					status_icon, button, activate_time);
}

static GtkStatusIcon *create_tray_icon(GtkWidget *window)
{
	GtkStatusIcon *tray_icon;
	tray_icon = gtk_status_icon_new();
	g_signal_connect(G_OBJECT(tray_icon), "activate",
						G_CALLBACK(tray_on_click), window);
	g_signal_connect(G_OBJECT(tray_icon), "popup-menu",
						G_CALLBACK(tray_on_menu), menu);
	gtk_status_icon_set_from_pixbuf(tray_icon, GDK_PIXBUF(p_icon));
	gtk_status_icon_set_tooltip_text(tray_icon,POMFIT_VERSION);
	gtk_status_icon_set_visible(tray_icon, TRUE);
	return tray_icon;
}

static GtkWidget* make_dialog (int id)
{
	GtkWidget *dialog_new;
	if(id == 1)
		check_but = gtk_check_button_new_with_label (
								"Enable animation preview.");
	else
		check_but0 = gtk_check_button_new_with_label (
								"Enable animation preview.");
	preview = gtk_image_new();
	gtk_widget_set_size_request (GTK_WIDGET(preview), 192, 192);

	GtkFileFilter *filter_all = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter_all, "*");
	gtk_file_filter_set_name(filter_all, "All files");
	GtkFileFilter *filter_img = gtk_file_filter_new ();
	gtk_file_filter_add_mime_type (filter_img, "image/*");
	gtk_file_filter_set_name(filter_img, "Images");
	dialog_new = gtk_file_chooser_dialog_new ("Upload File",
							NULL,
							GTK_FILE_CHOOSER_ACTION_OPEN,
							"Cancel", GTK_RESPONSE_CANCEL,
							"Upload", GTK_RESPONSE_ACCEPT,
							NULL);
	if(id == 1)
	 gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog_new),LastFolder);
	else
	 gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog_new),HomeDir);
	gtk_file_chooser_set_use_preview_label(GTK_FILE_CHOOSER(dialog_new), FALSE);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog_new),filter_all);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog_new),filter_img);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog_new), TRUE);
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog_new),
										GTK_WIDGET(preview));
	if(id == 1)
	{
	 gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog_new),check_but);
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_but),IsGifToggled);
	 IsGifToggled = FALSE;
	 IsDestroyed = FALSE;
	}
	else
	 gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog_new),check_but0);
	g_signal_connect(dialog_new,"update-preview",G_CALLBACK(update_preview),
							preview);
	return dialog_new;
}

static void choose_file(const char *keystring, void *user_data)
{
	if(IsUploading)
		return;
	GtkWidget *dialog_up;
	dialog_up = make_dialog(1);
	gchar *pLastFolder;
	if(gtk_dialog_run(GTK_DIALOG (dialog_up))==GTK_RESPONSE_ACCEPT)
	{
		GSList *pChoosedFiles = NULL;
		int i ,list;
		pChoosedFiles=gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog_up));
		list = g_slist_length(pChoosedFiles);
		gpointer pFiles[list];
		for (i = 0 ; i < list ; ++i)
			pFiles[i]=NULL;
		for( i = 0; i < list ; ++i)
		{
			pFiles[i] = pChoosedFiles->data;
			pChoosedFiles = g_slist_next(pChoosedFiles);
		}
		g_slist_free_full (pChoosedFiles , g_free);
		pLastFolder = gtk_file_chooser_get_current_folder(
						GTK_FILE_CHOOSER(dialog_up));
		sprintf(LastFolder, "%s" ,pLastFolder);
		gtk_widget_destroy(dialog_up);
		IsDestroyed = TRUE;
		while (gtk_events_pending ())
			gtk_main_iteration ();
		pomfit_upload_file(pFiles , list);
		for (i = 0 ; i < list ; ++i)
			pFiles[i]=NULL;
	}
	else
	{
	  pLastFolder = gtk_file_chooser_get_current_folder(
							GTK_FILE_CHOOSER(dialog_up));
	  sprintf(LastFolder, "%s" ,pLastFolder);
	  gtk_widget_destroy(dialog_up);
	  IsDestroyed = TRUE;
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_but0),IsGifToggled);
	  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),LastFolder);
	  g_free(pLastFolder);
	}
}

static void get_file_list (void)
{
	if(IsUploading)
	{
		gtk_button_set_relief(GTK_BUTTON(up_but), GTK_RELIEF_HALF);
		gtk_button_set_label(GTK_BUTTON(up_but), "Upload");
		return;
	}
	GSList *pChoosedFiles = NULL;
	gchar *pLastFolder;
	int i ,list;
	pChoosedFiles = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
	list = g_slist_length(pChoosedFiles);
	gpointer pFiles[list];
	for (i = 0 ; i < list ; ++i)
		pFiles[i]=NULL;
	for( i = 0; i < list ; ++i)
	{
		pFiles[i] = pChoosedFiles->data;
		pChoosedFiles = g_slist_next(pChoosedFiles);
	}
	g_slist_free_full (pChoosedFiles , g_free);
	pLastFolder = gtk_file_chooser_get_current_folder(
						GTK_FILE_CHOOSER(dialog));
	sprintf(LastFolder, "%s" ,pLastFolder);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(dialog));
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(sel_but));
	pomfit_upload_file(pFiles , list);
	for (i = 0 ; i < list ; ++i)
		pFiles[i]=NULL;

}

static char* get_home()
{
	char *pHome = NULL;
	pHome = getenv("HOME");
	return pHome;
}

static void update_preview (GtkFileChooser *file_chooser, gpointer data)
{
	GtkWidget *preview;
	gchar *FileName;
	GdkPixbuf *pixbuf;
	GdkPixbufAnimation *pixbuf_anim;
	gboolean IsStatic;
	gboolean HavePreview;

	preview = GTK_WIDGET (data);
	FileName = gtk_file_chooser_get_preview_filename (file_chooser);
	if(!FileName)
		return;
	pixbuf_anim = gdk_pixbuf_animation_new_from_file(FileName, NULL);
	if(pixbuf_anim)
		IsStatic = gdk_pixbuf_animation_is_static_image (pixbuf_anim);
	if(IsStatic)
	{
		pixbuf = gdk_pixbuf_new_from_file_at_size (FileName, 192, 192, NULL);
		gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
	}
	else
	{
	 if(!IsDestroyed)
		IsGifToggled=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_but));
	 if(IsDestroyed)
		IsGifToggled=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_but0));
	 if(IsGifToggled)
		gtk_image_set_from_animation (GTK_IMAGE (preview), pixbuf_anim);
	 else
	 {
		pixbuf = gdk_pixbuf_new_from_file_at_size (FileName, 192, 192, NULL);
		gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
	 }
	}
	HavePreview = (pixbuf != NULL || pixbuf_anim != NULL);
	g_free (FileName);
	if (pixbuf)
		g_object_unref (pixbuf);
	if (pixbuf_anim)
		g_object_unref (pixbuf_anim);
	gtk_file_chooser_set_preview_widget_active (file_chooser, HavePreview);
}


void gtk_login_handle(int Mode)
{
	if(vbox3)
		gtk_widget_destroy(vbox3);


	if(Mode == 0) {
	vbox3 = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(vbox1), vbox3);

	hbox_log1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 4);
	gtk_container_add(GTK_CONTAINER(vbox3), hbox_log1);

	label_email = gtk_label_new ("Email:");
	gtk_container_add(GTK_CONTAINER(hbox_log1), label_email);
	entry_email = gtk_entry_new();
	gtk_box_pack_end(GTK_BOX(hbox_log1),GTK_WIDGET(entry_email), TRUE, TRUE, 0);

	hbox_log2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 4);
	gtk_container_add(GTK_CONTAINER(vbox3), hbox_log2);

	label_pass = gtk_label_new ("Pass: ");
	gtk_container_add(GTK_CONTAINER(hbox_log2), label_pass);
	entry_pass = gtk_entry_new();
	gtk_box_pack_end(GTK_BOX(hbox_log2),GTK_WIDGET(entry_pass), TRUE, TRUE, 0);
	gtk_entry_set_visibility(GTK_ENTRY(entry_pass), FALSE);

	login_but = gtk_button_new_with_label("Login");
	g_signal_connect(login_but, "clicked", G_CALLBACK(GetLoginData), NULL);
	gtk_box_pack_end(GTK_BOX(vbox3),GTK_WIDGET(login_but),TRUE,TRUE,4);
	
	gtk_menu_item_set_label (GTK_MENU_ITEM(menuAcc) , "You are not logged in..");
	}
	else {

	vbox3 = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(vbox1), vbox3);

	label_session = gtk_label_new("You are logged in as:\n");
	gtk_container_add(GTK_CONTAINER(vbox3), label_session);

	label_session = gtk_label_new(LoginEmail);
	gtk_container_add(GTK_CONTAINER(vbox3), label_session);
	
	gtk_menu_item_set_label (GTK_MENU_ITEM(menuAcc) , LoginEmail);

	logout_but = gtk_button_new_with_label("Log out");
	g_signal_connect(logout_but, "clicked", G_CALLBACK(UserLogout), NULL);
	gtk_box_pack_end(GTK_BOX(vbox3),GTK_WIDGET(logout_but),TRUE,TRUE,4);


	}

	gtk_widget_show_all(vbox3);
	while (gtk_events_pending ())
		gtk_main_iteration ();
}

void Clean_Login_Data(void)
{
    int k;
    for(k = 0 ; k < strlen(LoginEmail); ++k)
        LoginEmail[k] = '\0';
    for(k = 0 ; k < strlen(LoginEmail); ++k)
        LoginPass[k] = '\0';
}

void GetLoginData (void)
{
	char Buffer[128];
	const char *pLine_email;
	const char *pLine_pass;

	pLine_email = gtk_entry_get_text(GTK_ENTRY(entry_email));
	pLine_pass = gtk_entry_get_text(GTK_ENTRY(entry_pass));

	strcpy(LoginEmail,pLine_email);
	strcpy(LoginPass, pLine_pass);
	pLine_email = NULL;
	pLine_pass = NULL;

    if(pomfit_curl_login() == 1)
    {
		sprintf(Buffer,"Loged in as \"%s\"",LoginEmail);
		gtk_login_handle(1);
        IsLoged = true;

		pipe(fd);
		pid_t pid;
		pid = fork();
		if (pid == -1)
		{
			perror("Forking failed!\n");
	    }
		else if (pid == 0)
		{
			close(fd[1]);
			char pipeBuff[80];
			time_t StartWait;
			while(true) {
				StartWait = time(NULL);
				while((StartWait + 7200) > time(NULL)) {
					read(fd[0], pipeBuff, sizeof(pipeBuff));
					if(strcmp(pipeBuff,"KillChild") == 0)
						_exit(0);
					sleep(5);
				}
				if(pomfit_curl_logout() == 1) {
					// If logout fails, let's wait 10 minutes in case of net issues
					StartWait = time(NULL);
					while((StartWait + 600) > time(NULL)) {
						read(fd[0], pipeBuff, sizeof(pipeBuff));
						if(strcmp(pipeBuff,"KillChild") == 0)
							_exit(0);
						sleep(5);
					}
					if(pomfit_curl_logout() == 1) {
						pomfit_notify_me("You are now logged out.\nYour uploaded"
							" files are no longer linked to your account.",
								"Automatic relogin failed!");
						gtk_login_handle(0);
						_exit(0);
					}
				}
				if(pomfit_curl_login() == 1)
					IsLoged = true;
				else{   // If login fails, let's wait 10 minutes in case of net issues                            
					StartWait = time(NULL);
					while((StartWait + 600) > time(NULL)) {
						read(fd[0], pipeBuff, sizeof(pipeBuff));
						if(strcmp(pipeBuff,"KillChild") == 0)
							_exit(0);
						sleep(5);
					}
					if(pomfit_curl_login() == 1)
						IsLoged = true;
					else {
						pomfit_notify_me("You are now logged out.\nYour uploaded files are no longer linked to your account.",
										 "Automatic relogin failed!");
						
						Clean_Login_Data();
						gtk_login_handle(0);
						IsLoged = false;
						_exit(0);
					}
				}
			}
		}
		else
			close(fd[0]);
	}
	else
	{
		gtk_entry_set_text(GTK_ENTRY(entry_email), "Invalid Login");
		gtk_entry_set_text(GTK_ENTRY(entry_pass), "");
	}
}

void UserLogout (void)
{
    if(pomfit_curl_logout() == 0) {
		write(fd[1], "KillChild", (strlen("KillChild")+1));
		gtk_login_handle(0);
		IsLoged = false;
	}else
        pomfit_notify_me("Please try again.","Curl logout failed!");
}

static void take_screenshot (const char *keystring, void *Upload) {
	
	bool ToUpload = (int*)Upload;
	pomfit_take_ss( ToUpload, 0);
	
}

static gboolean open_all_links_g (GtkLinkButton *button, gpointer user_data) {
	open_all_links(NULL,NULL);
	return TRUE;
}

static void SaveSettings (void) {
	
	char ConfFilePath [512];
	sprintf(ConfFilePath ,"%s/pomfit.ini" ,ConfDir);
	FILE *config = fopen(ConfFilePath , "w");
	if (config == NULL) {
		fprintf(stderr,"Couldn't create conf file\n");
		return;
	}
	const gchar *line;
	line = gtk_entry_get_text(GTK_ENTRY(entry_conf[0]));
	fprintf(config, "Hotkey_File_UP = %s\n",line);
	line = gtk_entry_get_text(GTK_ENTRY(entry_conf[1]));
	fprintf(config, "Hotkey_SS_UP = %s\n",line);
	line = gtk_entry_get_text(GTK_ENTRY(entry_conf[2]));
	fprintf(config, "Hotkey_SS = %s\n",line);
	line = gtk_entry_get_text(GTK_ENTRY(entry_conf[3]));
	fprintf(config, "Hotkey_Open_Links = %s\n",line);
	KeepUploadedSS = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_but_keepSS));
	fprintf(config, "Keep_Uploaded_SS = %d\n",KeepUploadedSS);
	
	fclose(config);
	
	LoadSettings();
}

static void LoadSettings (void) {
	char ConfFilePath [512];
	sprintf(ConfFilePath ,"%s/pomfit.ini" ,ConfDir);
	FILE *config = fopen(ConfFilePath , "r");
	if (config == NULL) {
        LoadDefaults ();
		fprintf(stderr ,"Couldn't load config file..using defaults\n");
		return;
	}
	const char *pBuff = NULL;
	char *pGetline = NULL;
	char Mod[2];
	pGetline = (char*) malloc (sizeof(char)*512);
	if(pGetline == NULL) {
		fprintf(stderr ,"Memory error");
	}
	char MenuLabel[128];
    while(fgets (pGetline , 512 , config) != NULL) {
		if(strncmp(pGetline, "Hotkey_File_UP" , strlen("Hotkey_File_UP")) == 0) {
			keybinder_unbind_all(Hotkey_FUP);
            pBuff = strstr(pGetline , "=");
            pBuff += 2;
            snprintf(Hotkey_FUP, strlen(pBuff),"%s",pBuff);
            gtk_entry_set_text(GTK_ENTRY(entry_conf[0]), Hotkey_FUP);
            keybinder_bind_full(Hotkey_FUP, choose_file, NULL, NULL);
            sprintf(MenuLabel,"Upload File\t\t%s",Hotkey_FUP );
            gtk_menu_item_set_label ( GTK_MENU_ITEM(menuFileUp), MenuLabel);
		}
		else if(strncmp(pGetline, "Hotkey_SS_UP" , strlen("Hotkey_SS_UP")) == 0) {
            keybinder_unbind_all(Hotkey_PUP);
            pBuff = strstr(pGetline , "=");
            pBuff += 2;
            snprintf(Hotkey_PUP,strlen(pBuff),"%s",pBuff);
            gtk_entry_set_text(GTK_ENTRY(entry_conf[1]), Hotkey_PUP);
            keybinder_bind_full(Hotkey_PUP, take_screenshot, (void*)1, NULL);
            sprintf(MenuLabel,"Upload Screencap\t%s", Hotkey_PUP);
            gtk_menu_item_set_label (GTK_MENU_ITEM(menuCapUp), MenuLabel);
		}
		else if(strncmp(pGetline, "Hotkey_SS" , strlen("Hotkey_SS")) == 0) {
			keybinder_unbind_all(Hotkey_CAP);
            pBuff = strstr(pGetline , "=");
            pBuff += 2;
            snprintf(Hotkey_CAP,strlen(pBuff),"%s",pBuff);
            gtk_entry_set_text(GTK_ENTRY(entry_conf[2]), Hotkey_CAP);
            keybinder_bind_full(Hotkey_CAP, take_screenshot, (void*)0, NULL);
            sprintf(MenuLabel,"Screencap\t\t%s", Hotkey_CAP);
            gtk_menu_item_set_label (GTK_MENU_ITEM(menuCap) , MenuLabel);
		}
		else if(strncmp(pGetline, "Hotkey_Open_Links" , strlen("Hotkey_Open_Links")) == 0) {
			keybinder_unbind_all(Hotkey_OPEN);
            pBuff = strstr(pGetline , "=");
            pBuff += 2;
            snprintf(Hotkey_OPEN,strlen(pBuff),"%s",pBuff);
            gtk_entry_set_text(GTK_ENTRY(entry_conf[3]), Hotkey_OPEN);
            keybinder_bind_full(Hotkey_OPEN, open_all_links, NULL, NULL);
            sprintf(MenuLabel,"Open Last Link(s)\t%s", Hotkey_OPEN);
            gtk_menu_item_set_label (GTK_MENU_ITEM(menuOpen) , MenuLabel);
		}
		else if(strncmp(pGetline, "Keep_Uploaded_SS" , strlen("Keep_Uploaded_SS")) == 0) {
            pBuff = strstr(pGetline , "=");
            pBuff += 2;
            strncpy(Mod,pBuff,1);
            KeepUploadedSS = atoi(Mod);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_but_keepSS),KeepUploadedSS);
	    }
	}
	free(pGetline);
	pBuff = NULL;
	fclose(config);
}

static void LoadDefaults (void) {
	
	sprintf(Hotkey_FUP,"<Ctrl><Alt>f");
	sprintf(Hotkey_PUP,"<Ctrl><Alt>u");
	sprintf(Hotkey_CAP,"<Ctrl><Alt>c");
	sprintf(Hotkey_OPEN,"<Ctrl><Alt>b");
	
	keybinder_bind_full(Hotkey_FUP, choose_file, NULL, NULL);
	gtk_entry_set_text(GTK_ENTRY(entry_conf[0]), Hotkey_FUP);
	keybinder_bind_full(Hotkey_PUP, take_screenshot, (void*)1, NULL);
	gtk_entry_set_text(GTK_ENTRY(entry_conf[1]), Hotkey_PUP);
	keybinder_bind_full(Hotkey_CAP, take_screenshot, (void*)0, NULL);
	gtk_entry_set_text(GTK_ENTRY(entry_conf[2]), Hotkey_CAP);
	keybinder_bind_full(Hotkey_OPEN, open_all_links, NULL, NULL);
	gtk_entry_set_text(GTK_ENTRY(entry_conf[3]), Hotkey_OPEN);
	KeepUploadedSS = true;	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_but_keepSS),KeepUploadedSS);
	
	SaveSettings();
}

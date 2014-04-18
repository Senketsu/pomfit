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

#include "header.h"
#include "pomfit.h"

#define BIND_PUP "<Ctrl><Alt>u"
#define BIND_FUP "<Ctrl><Alt>f"
#define BIND_CAP "<Ctrl><Alt>c"
#define BIND_OPEN "<Ctrl><Alt>b"

static GtkStatusIcon *create_tray_icon();
static void get_home();
static void quick_file_upload(const char *keystring, void *user_data);
static char* choose_file(void);
static GtkWidget* make_dialog (int id);
static void update_preview (GtkFileChooser *file_chooser, gpointer data);
static void trayCapUp(GtkMenuItem *item, gpointer user_data);
static void trayFileUp(GtkMenuItem *item, gpointer user_data);
static void trayCap(GtkMenuItem *item, gpointer user_data);
static void trayView(GtkMenuItem *item, gpointer window);
static void trayExit(GtkMenuItem *item, gpointer user_data);
static void tray_on_click(GtkStatusIcon *status_icon, gpointer user_data);
static void tray_on_menu(GtkStatusIcon *status_icon, guint button, 
							guint activate_time, gpointer user_data);

gboolean hidden = TRUE;
gboolean first_run = TRUE;
char home_dir[64];
char last_upload[64]="http://pomf.se";
char last_folder[256];
GtkWidget *window;
GtkWidget *sel_but;
GtkWidget *link_but;
GtkWidget *up_but;
GdkPixbuf *p_icon;
GtkWidget *menu;
GtkWidget *check_but0;
GtkWidget *check_but;
GtkWidget *dialog;
GtkWidget *status_bar;
GtkWidget *preview;
gboolean toggled_gif;
gboolean destroyed = TRUE;

static void quit (GSimpleAction *action, GVariant *parameter,
						gpointer user_data)
{
	GApplication *application = user_data;
	g_application_quit(application);
}

static void open_file(GApplication  *application, GFile **files, gint n_files,
						const gchar *hint)
{
	char *path = g_file_get_path (files[0]);
	curl_upload_file(path , TRUE);
	path = NULL;
}

static void activate(void)
{
	if(first_run)
	{
		first_run = FALSE;
		return;
	}
	gtk_widget_show(window);
	hidden = FALSE;
}

static void startup (GtkApplication *pomfit, gpointer user_data)
{
	get_home();
	sprintf(last_folder, "%s", home_dir);
	GError *error = NULL;
	gchar *pLast_folder_m;
	GtkWidget *vbox1;
	GtkWidget *hbox;
	GtkWidget *quit_but;
	GtkWidget *preview;
	GtkStatusIcon *tray_icon;
	GtkWidget *menuExit , *menuView, *menuCap, *menuCapUp, *menuFileUp;
	
	window = gtk_application_window_new(pomfit);
	gtk_window_set_title(GTK_WINDOW(window), "Pomf it!");
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	gtk_window_resize(GTK_WINDOW(window), 320, 100);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	p_icon = gdk_pixbuf_new_from_file("/usr/local/share/pixmaps/pomfit.png",&error);
	gtk_window_set_default_icon(GDK_PIXBUF(p_icon));
	
	menu = gtk_menu_new();
	menuCapUp = gtk_menu_item_new_with_label("Upload Screencap");
	menuFileUp = gtk_menu_item_new_with_label("Upload File");
	menuCap = gtk_menu_item_new_with_label("Screencap");
	menuView = gtk_menu_item_new_with_label("Show / Hide");
	menuExit = gtk_menu_item_new_with_label("Exit");
	g_signal_connect(G_OBJECT(menuCapUp), "activate", 
						G_CALLBACK(trayCapUp), NULL);
	g_signal_connect(G_OBJECT(menuCap), "activate", 
						G_CALLBACK(trayCap), NULL);
	g_signal_connect(G_OBJECT(menuFileUp), "activate", 
						G_CALLBACK(trayFileUp), NULL);
	g_signal_connect(G_OBJECT(menuView), "activate", 
						G_CALLBACK(trayView), window);
	g_signal_connect(G_OBJECT(menuExit), "activate", 
						G_CALLBACK(trayExit), pomfit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuCapUp);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuFileUp);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuCap);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuView);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuExit);
	gtk_widget_show_all(menu);	
	tray_icon = create_tray_icon();
	
	keybinder_init();
	keybinder_bind(BIND_FUP, quick_file_upload, NULL);
	keybinder_bind(BIND_PUP, quick_upload_pic, NULL);
	keybinder_bind(BIND_CAP, take_screenshot, NULL);
	keybinder_bind(BIND_OPEN, open_last_link, NULL);
	
	vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(window), vbox1);
	
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 4);
	gtk_container_add(GTK_CONTAINER(vbox1), hbox);

	dialog = make_dialog(0);
	sel_but = gtk_file_chooser_button_new_with_dialog (dialog);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(sel_but),TRUE,TRUE,4);
	
	up_but = gtk_button_new_with_label("Upload");
	g_signal_connect(up_but, "clicked", G_CALLBACK(curl_upload_file), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(up_but),TRUE,TRUE,4);
  
	link_but =  gtk_link_button_new_with_label("http://pomf.se", "Pomf it !");
	gtk_box_pack_start(GTK_BOX(vbox1),GTK_WIDGET(link_but),TRUE,TRUE,0);
	
	status_bar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox1),GTK_WIDGET(status_bar),TRUE,TRUE,0);
	gtk_statusbar_push (GTK_STATUSBAR(status_bar), 1, POMFIT_VERSION);
	
	quit_but = gtk_button_new_with_label("Exit");
	g_signal_connect(quit_but, "clicked", G_CALLBACK(quit), pomfit);
	gtk_box_pack_end(GTK_BOX(vbox1),GTK_WIDGET(quit_but),TRUE,TRUE,0);

	gtk_widget_show_all(window);
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


static void tray_on_click(GtkStatusIcon *status_icon, gpointer window)
{
	if(hidden){
		gtk_widget_show(window);
		gtk_window_deiconify(window); 
		hidden = FALSE;
	}
	else{
		gtk_widget_hide(window);
		hidden = TRUE;
	}
}

static void tray_on_menu(GtkStatusIcon *status_icon, guint button, 
							guint activate_time, gpointer user_data)
{
	gtk_menu_popup(GTK_MENU(user_data), NULL, NULL, 
					gtk_status_icon_position_menu, 
					status_icon, button, activate_time);
}

static GtkStatusIcon *create_tray_icon() 
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

static void trayExit(GtkMenuItem *item, gpointer user_data) 
{
	quit(NULL,NULL,user_data);
}

static void trayCapUp(GtkMenuItem *item, gpointer user_data) 
{
	quick_upload_pic(NULL,NULL);
}

static void trayCap(GtkMenuItem *item, gpointer user_data) 
{
	take_screenshot(NULL,NULL);
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
	 gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog_new),last_folder);
	else
	 gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog_new),home_dir);
	gtk_file_chooser_set_use_preview_label(GTK_FILE_CHOOSER(dialog_new), FALSE);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog_new),filter_all);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog_new),filter_img);
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog_new),
										GTK_WIDGET(preview));
	if(id == 1)
	{
	 gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog_new),check_but);
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_but),toggled_gif);
	 toggled_gif = FALSE;
	 destroyed = FALSE;
	}
	else
	 gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog_new),check_but0);
	g_signal_connect(dialog_new,"update-preview",G_CALLBACK(update_preview),
							preview);
	return dialog_new;
}

static char* choose_file(void)
{
	GtkWidget *dialog_up;
	dialog_up = make_dialog(1);
	gchar *pLast_folder;
	char *filename = NULL;
	if(gtk_dialog_run(GTK_DIALOG (dialog_up))==GTK_RESPONSE_ACCEPT)
	{
	  filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog_up));
	  pLast_folder = gtk_file_chooser_get_current_folder(
						GTK_FILE_CHOOSER(dialog_up));
	  sprintf(last_folder, "%s" ,pLast_folder);
	  gtk_widget_destroy(dialog_up);
	  destroyed = TRUE;
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_but0),toggled_gif);
	  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),last_folder);
	  g_free(pLast_folder);
	  return filename;
	}
	else
	{
	  pLast_folder = gtk_file_chooser_get_current_folder(
							GTK_FILE_CHOOSER(dialog_up));
	  sprintf(last_folder, "%s" ,pLast_folder);
	  gtk_widget_destroy(dialog_up);
	  destroyed = TRUE;
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_but0),toggled_gif);
	  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),last_folder);
	  g_free(pLast_folder);
	  return NULL;
	}
}

static void quick_file_upload(const char *keystring, void *user_data)
{
	char* pFileName = NULL;
	pFileName = choose_file();
	while (gtk_events_pending ())
		gtk_main_iteration ();
	if(pFileName!=NULL)
		curl_upload_file(pFileName , TRUE);
	pFileName = NULL;
}

static void trayFileUp(GtkMenuItem *item, gpointer user_data) 
{
	char* pFileName = NULL;
	pFileName = choose_file();
	while (gtk_events_pending ())
		gtk_main_iteration ();
	if(pFileName!=NULL)
		curl_upload_file(pFileName , TRUE);
	pFileName = NULL;
}

static void trayView(GtkMenuItem *item, gpointer window) 
{
	if(hidden)
	{
		gtk_widget_show(window);
		gtk_window_deiconify(window); 
		hidden = FALSE;
	}
	else
	{
		gtk_widget_hide(window);
		hidden = TRUE;
	}
}

static void get_home()
{
	char *home = NULL;
	home = getenv("HOME");
	strcpy(home_dir, home);
	home = NULL; 
}

static void update_preview (GtkFileChooser *file_chooser, gpointer data)
{
	GtkWidget *preview;
	gchar *filename;
	GdkPixbuf *pixbuf;
	GdkPixbufAnimation *pixbuf_anim;
	gboolean is_static;
	gboolean have_preview;

	preview = GTK_WIDGET (data);
	filename = gtk_file_chooser_get_preview_filename (file_chooser);
	if(!filename)
		return;
	pixbuf_anim = gdk_pixbuf_animation_new_from_file(filename, NULL);
	if(pixbuf_anim)
		is_static = gdk_pixbuf_animation_is_static_image (pixbuf_anim);
	if(is_static)
	{
		pixbuf = gdk_pixbuf_new_from_file_at_size (filename, 192, 192, NULL);
		gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
	}
	else
	{
	 if(!destroyed)
		toggled_gif=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_but));
	 if(destroyed)
		toggled_gif=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_but0));
	 if(toggled_gif)
		gtk_image_set_from_animation (GTK_IMAGE (preview), pixbuf_anim);
	 else
	 {
		pixbuf = gdk_pixbuf_new_from_file_at_size (filename, 192, 192, NULL);
		gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
	 }
	}
	have_preview = (pixbuf != NULL || pixbuf_anim != NULL);
	g_free (filename);
	if (pixbuf)
		g_object_unref (pixbuf);
	if (pixbuf_anim)
		g_object_unref (pixbuf_anim);
	gtk_file_chooser_set_preview_widget_active (file_chooser, have_preview);
}

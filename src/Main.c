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

#define BIND_UP "<Super>u"
#define BIND_CAP "<Super>c"
#define BIND_OPEN "<Super>w"

static GtkStatusIcon *create_tray_icon();
static void get_home();
static void trayCapUp(GtkMenuItem *item, gpointer user_data);
static void trayFileUp(GtkMenuItem *item, gpointer user_data);
static void trayCap(GtkMenuItem *item, gpointer user_data);
static void trayView(GtkMenuItem *item, gpointer window);
static void trayExit(GtkMenuItem *item, gpointer user_data);
static void tray_on_click(GtkStatusIcon *status_icon, gpointer user_data);
static void tray_on_menu(GtkStatusIcon *status_icon, guint button, 
							guint activate_time, gpointer user_data);

bool hidden = TRUE;
bool first_run = TRUE;
char home_dir[64];
char last_upload[64]="http://pomf.se";
GtkWidget *window;
GtkWidget *sel_but;
GtkWidget *link_but;
GdkPixbuf *p_icon;
GtkWidget *menu;

static void quit (GSimpleAction *action, GVariant *parameter,
						gpointer user_data)
{
	GApplication *application = user_data;
	g_application_quit(application);
}

static void open_file(GApplication  *application,
						GFile        **files,
						gint           n_files,
						const gchar   *hint)
{
	char *path = g_file_get_path (files[0]);
	curl_upload_file(path);
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
	
	GError *error = NULL;
	GtkWidget *vbox1;
	GtkWidget *hbox;
	GtkWidget *up_but;
	GtkWidget *quit_but;
	GtkFileFilter *filter_img;
	GtkStatusIcon *tray_icon;
	GtkWidget *menuExit , *menuView, *menuCap, *menuCapUp, *menuFileUp;

	window = gtk_application_window_new(pomfit);
	gtk_window_set_title(GTK_WINDOW(window), "Pomf it!");
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	gtk_window_set_resizable(GTK_WINDOW(window) , FALSE);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);
	p_icon = gdk_pixbuf_new_from_file("/usr/local/share/pixmaps/pomfit.png",&error);
	gtk_window_set_default_icon(GDK_PIXBUF(p_icon));
	
	menu = gtk_menu_new();
	menuCapUp = gtk_menu_item_new_with_label("Upload Screencap");
	menuFileUp = gtk_menu_item_new_with_label("Upload File");
	menuCap = gtk_menu_item_new_with_label("Screencap");
	menuView = gtk_menu_item_new_with_label("View");
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
	keybinder_bind(BIND_CAP, take_screenshot, NULL);
	keybinder_bind(BIND_UP, quick_upload_pic, NULL);
	keybinder_bind(BIND_OPEN, open_last_link, NULL);
	
	vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(window), vbox1);
	
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 4);
	gtk_container_add(GTK_CONTAINER(vbox1), hbox);
	
	sel_but = gtk_file_chooser_button_new(" ",GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(sel_but),home_dir);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(sel_but),TRUE,TRUE,4);
	
	up_but = gtk_button_new_with_label("Upload");
	g_signal_connect(up_but, "clicked", G_CALLBACK(curl_upload_file), NULL);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(up_but),TRUE,TRUE,4);
  
	link_but =  gtk_link_button_new_with_label("http://pomf.se", "Pomf it !");
	gtk_box_pack_start(GTK_BOX(vbox1),GTK_WIDGET(link_but),TRUE,TRUE,0);
	
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
	
	pomfit = gtk_application_new ("pomfit.uploader", G_APPLICATION_HANDLES_OPEN);
	g_signal_connect (pomfit, "startup", G_CALLBACK (startup), NULL);
	g_signal_connect (pomfit, "activate", G_CALLBACK (activate), NULL);
	g_signal_connect (pomfit, "open", G_CALLBACK (open_file), argv);
	status = g_application_run (G_APPLICATION (pomfit), argc, argv);
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
	gtk_status_icon_set_tooltip_text(tray_icon,"\"Pomf it !\" Uploader");
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

static char* choose_file(void)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Upload File",
							NULL,
							GTK_FILE_CHOOSER_ACTION_OPEN,
							"Cancel", GTK_RESPONSE_CANCEL,
							"Select", GTK_RESPONSE_ACCEPT,
							NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),home_dir);
	char *filename = NULL;
	if(gtk_dialog_run(GTK_DIALOG (dialog))==GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_widget_destroy(dialog);
		return filename;
	}
	else
		gtk_widget_destroy(dialog);
		return NULL;
}

static void trayFileUp(GtkMenuItem *item, gpointer user_data) 
{
	char* pFileName = NULL;
	pFileName = choose_file();
	if(pFileName!=NULL)
		curl_upload_file(pFileName);
		
	pFileName = NULL;
}

static void trayView(GtkMenuItem *item, gpointer window) 
{
	gtk_widget_show(window);
	gtk_window_deiconify(window); 
	hidden = FALSE;
}

static void get_home()
{
	char *home = NULL;
	home = getenv("HOME");
	strcpy(home_dir, home);
	home = NULL; 
}

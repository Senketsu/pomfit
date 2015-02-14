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
#include <stdbool.h>
#include <string.h>
#include <gtk/gtk.h>
#include "manager.h"
#include "database.h"
#include "log.h"

GtkWidget *win_um_main, *win_um_scrl, *entry_um, *combo_um, *tree_um, *preview_um;

GtkTreeViewColumn *column;
GtkTreeIter iter,Fill_Iter;
GtkTreePath *tree_path_um;
GtkTreeStore *tree_store_um;
extern void show_uri(const char *keystring, void *uri);
extern bool bUM_Preview;
extern bool bUM_AnimPreview;
extern bool bUM_DoubleClick;
extern int iUM_DC_Action;
extern int iUM_Win_W;
extern int iUM_Win_H;
extern int iUM_Prev_Size_W;
extern int iUM_Prev_Size_H;
extern int iUM_Win_Pos;

void manager_window_create (void) {

	if(win_um_main != NULL) {
		gtk_widget_hide(win_um_main);
		gtk_widget_show(win_um_main);
		return;
	}

	manager_tree_create();
	GtkWidget *but_um_close, *but_um_search, *but_um_clear, *but_um_list, *but_um_del;
	GtkWidget *vbox_um, *hbox_um, *hbox_ump;

	win_um_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(win_um_main), "Pomf it! Upload Manager");
	g_signal_connect(win_um_main, "destroy", G_CALLBACK(manager_window_destroy), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(win_um_main), 10);
	gtk_window_resize(GTK_WINDOW(win_um_main), iUM_Win_W, iUM_Win_H);
	gtk_window_set_position(GTK_WINDOW(win_um_main), iUM_Win_Pos);

	vbox_um = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(win_um_main), vbox_um);

	hbox_ump = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 0);
	gtk_box_pack_start(GTK_BOX(vbox_um), hbox_ump, TRUE, TRUE, 0);

	win_um_scrl = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(hbox_ump), win_um_scrl, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(win_um_scrl), GTK_WIDGET(tree_um));

	if(bUM_Preview == true) {

		preview_um = gtk_image_new();
		gtk_widget_set_size_request (GTK_WIDGET(preview_um), iUM_Prev_Size_W, iUM_Prev_Size_H);
		gtk_container_add(GTK_CONTAINER(hbox_ump), preview_um);
	}

	hbox_um = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 0);
	gtk_box_pack_end(GTK_BOX(vbox_um), hbox_um, FALSE, FALSE, 0);

	combo_um = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_um), NULL, "File Name");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_um), NULL, "URL");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_um), NULL, "File Path");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_um), 0);
	gtk_container_add(GTK_CONTAINER(hbox_um), combo_um);

	entry_um = gtk_entry_new();
	g_signal_connect(entry_um,"activate",G_CALLBACK(manager_db_search), NULL);
	gtk_container_add(GTK_CONTAINER(hbox_um), entry_um);

	but_um_search = gtk_button_new_with_label("Search");
	g_signal_connect(but_um_search,"clicked",G_CALLBACK(manager_db_search), NULL);
	gtk_container_add(GTK_CONTAINER(hbox_um), but_um_search);

	but_um_list = gtk_button_new_with_label("List All");
	g_signal_connect(but_um_list,"clicked",G_CALLBACK(manager_tree_list_all), NULL);
	gtk_container_add(GTK_CONTAINER(hbox_um), but_um_list);

	but_um_clear = gtk_button_new_with_label("  Clear  ");
	g_signal_connect(but_um_clear,"clicked",G_CALLBACK(manager_tree_clear), NULL);
	gtk_container_add(GTK_CONTAINER(hbox_um), but_um_clear);

	but_um_del = gtk_button_new_with_label("Wipe DB");
	g_signal_connect(but_um_del,"clicked",G_CALLBACK(manager_db_delete), NULL);
	gtk_box_pack_start(GTK_BOX(hbox_um),GTK_WIDGET(but_um_del),FALSE,FALSE,10);

	but_um_close = gtk_button_new_with_label("   Close   ");
	g_signal_connect(but_um_close,"clicked",G_CALLBACK(manager_window_destroy), NULL);
	gtk_box_pack_end(GTK_BOX(hbox_um),GTK_WIDGET(but_um_close),FALSE,FALSE,0);

	GtkTreeSelection *tree_selection;
	tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_um));
	gtk_tree_selection_set_mode (tree_selection,  GTK_SELECTION_MULTIPLE);

	gtk_widget_show_all(win_um_main);
}

void manager_window_destroy (GtkWidget *window) {

	gtk_widget_destroy(win_um_main);
	win_um_main = NULL;
}

void manager_window_cancel (void) {

    manager_tree_clear();
	gtk_widget_hide(win_um_main);
}

void manager_window_hide (void) {

	gtk_widget_hide(win_um_main);
}

void manager_tree_create (void) {

	GtkCellRenderer *renderer;
	tree_store_um = gtk_tree_store_new (N_COLUMNS,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING);
	tree_um = gtk_tree_view_new_with_model (GTK_TREE_MODEL (tree_store_um));
	g_signal_connect(tree_um,"row-activated",G_CALLBACK(manager_tree_row_dclick),NULL);
	g_signal_connect(tree_um, "button-press-event",
									G_CALLBACK(manager_tree_button_cb), NULL);
	g_signal_connect(tree_um, "popup-menu", G_CALLBACK(manager_tree_menu_cb), tree_um);
	g_object_unref (G_OBJECT (tree_store_um));

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),"foreground", "orange", NULL);
	column = gtk_tree_view_column_new_with_attributes ("File Name", renderer,
												"text", FILENAME_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_um), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("URL",renderer,
												"text", PURL_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_um), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Delete URL",renderer,
												"text", DELURL_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_um), column);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),"foreground", "gray", NULL);
	column = gtk_tree_view_column_new_with_attributes ("File Path",renderer,
												"text", FPATH_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_um), column);
}

void manager_tree_add_row (char *filename, char *up_url, char *del_url, char *file_path) {

	gtk_tree_store_append(tree_store_um, &Fill_Iter, NULL);
	gtk_tree_store_set(tree_store_um, &Fill_Iter,
								FILENAME_COLUMN, filename,
								PURL_COLUMN, up_url,
								DELURL_COLUMN, del_url,
								FPATH_COLUMN, file_path,-1);
}

void manager_tree_clear (void) {

	gtk_tree_store_clear(tree_store_um);
	if(bUM_Preview == true)
        gtk_image_clear (GTK_IMAGE (preview_um));
}

void manager_tree_list_all (void) {

	SQLite.execute("SELECT * FROM uploads");
	pomfit_db_fill_tree();
}

void manager_db_delete (void) {

	GtkWidget *dialog;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_message_dialog_new (GTK_WINDOW(win_um_main), flags,
									 GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
									 "Delete all records from database ?");
	if(gtk_dialog_run(GTK_DIALOG (dialog))==GTK_RESPONSE_YES)
		SQLite.execute("DELETE FROM uploads");

	gtk_widget_destroy (dialog);
	manager_tree_clear();
}

void manager_db_search (void) {

	int id = 0;
	char column[3][128] = { "file_name", "url", "file_path"};
	const gchar *line = NULL;
	line = gtk_entry_get_text(GTK_ENTRY(entry_um));
	if(line == NULL || strlen(line) < 1) {
		manager_tree_list_all();
		return;
	}
	id = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_um));

	char sQuery[2048];
	snprintf(sQuery,sizeof(sQuery),"SELECT * FROM uploads WHERE %s LIKE '%%' || ? || '%%' "
		"ORDER BY CASE WHEN %s = ? THEN 0 "
		"WHEN %s LIKE ? || '%%' THEN 1 "
		"WHEN %s LIKE '%%' || ? || '%%' THEN 2 "
		"WHEN %s LIKE '%%' || ? THEN 3 "
		"ELSE 4 END, %s ASC",column[id],column[id],column[id],column[id],column[id],column[id]);

	SQLite.execute2(sQuery,5, line, line, line, line, line  );

	pomfit_db_fill_tree();
	line = NULL;
}

void manager_tree_menu (GtkWidget *treeview, GdkEventButton *event,
						gpointer userdata ,GtkTreePath *path) {

	GtkWidget *tree_menu, *tree_menu_copy_url, *tree_menu_open_url, *tree_menu_sep;
	GtkWidget *tree_menu_del_db, *tree_menu_del_web, *tree_menu_sep2;
	GtkWidget *tree_menu_open_file, *tree_menu_open_folder, *tree_menu_copy_path;
	GtkWidget *tree_menu_copy_del_url;
	tree_menu = gtk_menu_new();
	tree_menu_open_file = gtk_menu_item_new_with_label("Open File(s)");
	tree_menu_open_folder = gtk_menu_item_new_with_label("Open Folder(s)");
	tree_menu_copy_path = gtk_menu_item_new_with_label("Copy File Path(s)");
	tree_menu_sep = gtk_separator_menu_item_new ();
	tree_menu_copy_url = gtk_menu_item_new_with_label("Copy URL(s)");
	tree_menu_copy_del_url = gtk_menu_item_new_with_label("Copy Delete URL(s)");
	tree_menu_open_url = gtk_menu_item_new_with_label("Open URL(s)");
	tree_menu_sep2 = gtk_separator_menu_item_new ();
	tree_menu_del_db = gtk_menu_item_new_with_label("Delete from DB");
	//~ tree_menu_del_web = gtk_menu_item_new_with_label("Delete from WEB");

	g_signal_connect(tree_menu_open_file, "activate",
							G_CALLBACK (manager_tree_menu_open_file), treeview);
	g_signal_connect(tree_menu_open_folder, "activate",
							G_CALLBACK (manager_tree_menu_open_folder), treeview);
	g_signal_connect(tree_menu_copy_path, "activate",
							G_CALLBACK (manager_tree_menu_copy_path), treeview);
	g_signal_connect(tree_menu_copy_url, "activate",
							G_CALLBACK (manager_tree_menu_copy_url), treeview);
	g_signal_connect(tree_menu_copy_del_url, "activate",
							G_CALLBACK (manager_tree_menu_copy_del_url), treeview);
	g_signal_connect(tree_menu_open_url, "activate",
							G_CALLBACK (manager_tree_menu_open_url), treeview);
	g_signal_connect(tree_menu_del_db, "activate",
							G_CALLBACK (manager_tree_menu_del_db), treeview);
	//~ g_signal_connect(tree_menu_del_web, "activate",
							//~ G_CALLBACK (manager_tree_menu_del_web), treeview);

	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_open_file);
	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_open_folder);
	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_copy_path);
	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_sep);
	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_copy_url);
	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_copy_del_url);
	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_open_url);
	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_sep2);
	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_del_db);
	//~ gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_del_web);
	gtk_widget_show_all(tree_menu);
	gtk_menu_popup(GTK_MENU(tree_menu), NULL, NULL, NULL, NULL,
					(event != NULL) ? event->button : 0,
					gdk_event_get_time((GdkEvent*)event));
}

void manager_tree_menu_copy_path (GtkWidget *menuitem, gpointer userdata) {

	GtkTreeIter iter;
	GtkTreeView *view = GTK_TREE_VIEW(tree_um);
	GtkTreeModel *model_get = gtk_tree_view_get_model(view);
	GtkTreeSelection *selection= gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_um));
	GList *PathsList = gtk_tree_selection_get_selected_rows (selection, &model_get);
	guint ListLen = g_list_length (PathsList);
	guint i = 0;
	guint Allocated = 256*ListLen;
	gchar *FilePath = NULL;
	gchar *MoreLinks = NULL;
	gchar *Links = g_malloc0((sizeof(char)*Allocated));
	if(!Links) {
		log_error(true,"***Error: Memory g_malloc0 @ manager path copy");
		return;
	}
	for (PathsList = PathsList; PathsList != NULL; PathsList = PathsList->next)
	{
		GtkTreePath *path = PathsList->data;
		if (gtk_tree_model_get_iter(model_get, &iter, path)) {

			gtk_tree_model_get(model_get, &iter, FPATH_COLUMN, &FilePath, -1);

			if((strlen(Links) + strlen(FilePath)+1) > Allocated) {
				gchar *MoreLinks = (char *)g_realloc (Links, (sizeof(char)*(Allocated +strlen(FilePath)+1)));
				if(MoreLinks) {
					Allocated += strlen(FilePath)+1;
					Links = MoreLinks;
				} else {
					log_error(true,"***Error: Memory g_realloc @ manager path copy");
					break;
				}
			}
			if(i == 0)
				sprintf(Links,"%s ", FilePath);
			else
				sprintf(strchr(Links, '\0'),"%s ", FilePath);
			++i;
		}
	}
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
							Links, strlen(Links));
	g_free(FilePath);
	g_free(Links);
	g_free(MoreLinks);
	g_list_free_full (PathsList, (GDestroyNotify) gtk_tree_path_free);

}

void manager_tree_menu_copy_del_url (GtkWidget *menuitem, gpointer userdata) {

	GtkTreeIter iter;
	GtkTreeView *view = GTK_TREE_VIEW(tree_um);
	GtkTreeModel *model_get = gtk_tree_view_get_model(view);
	GtkTreeSelection *selection= gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_um));
	GList *PathsList = gtk_tree_selection_get_selected_rows (selection, &model_get);
	guint ListLen = g_list_length (PathsList);
	guint i = 0;
	guint Allocated = 128*ListLen;
	gchar *Url = NULL;
	gchar *MoreLinks = NULL;
	gchar *Links = g_malloc0((sizeof(char)*Allocated));
	if(!Links) {
		log_error(true,"***Error: Memory g_malloc0 @ manager copy url");
		return;
	}
	for (PathsList = PathsList; PathsList != NULL; PathsList = PathsList->next)
	{
		GtkTreePath *path = PathsList->data;
		if (gtk_tree_model_get_iter(model_get, &iter, path)) {

			gtk_tree_model_get(model_get, &iter, DELURL_COLUMN, &Url, -1);

			if((strlen(Links) + strlen(Url)+1) > Allocated) {
				gchar *MoreLinks = (char *)g_realloc (Links, (sizeof(char)*(Allocated +strlen(Url)+1)));
				if(MoreLinks) {
					Allocated += strlen(Url)+1;
					Links = MoreLinks;
				} else {
					log_error(true,"***Error: Memory g_realloc @ manager copy url");
					break;
				}
			}
			if(i == 0)
				sprintf(Links,"%s ", Url);
			else
				sprintf(strchr(Links, '\0'),"%s ", Url);
			++i;
		}
	}
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
							Links, strlen(Links));
	g_free(Url);
	g_free(Links);
	g_free(MoreLinks);
	g_list_free_full (PathsList, (GDestroyNotify) gtk_tree_path_free);

}

void manager_tree_menu_copy_url (GtkWidget *menuitem, gpointer userdata) {

	GtkTreeIter iter;
	GtkTreeView *view = GTK_TREE_VIEW(tree_um);
	GtkTreeModel *model_get = gtk_tree_view_get_model(view);
	GtkTreeSelection *selection= gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_um));
	GList *PathsList = gtk_tree_selection_get_selected_rows (selection, &model_get);
	guint ListLen = g_list_length (PathsList);
	guint i = 0;
	guint Allocated = 128*ListLen;
	gchar *Url = NULL;
	gchar *MoreLinks = NULL;
	gchar *Links = g_malloc0((sizeof(char)*Allocated));
	if(!Links) {
		log_error(true,"***Error: Memory g_malloc0 @ manager copy url");
		return;
	}
	for (PathsList = PathsList; PathsList != NULL; PathsList = PathsList->next)
	{
		GtkTreePath *path = PathsList->data;
		if (gtk_tree_model_get_iter(model_get, &iter, path)) {

			gtk_tree_model_get(model_get, &iter, PURL_COLUMN, &Url, -1);

			if((strlen(Links) + strlen(Url)+1) > Allocated) {
				gchar *MoreLinks = (char *)g_realloc (Links, (sizeof(char)*(Allocated +strlen(Url)+1)));
				if(MoreLinks) {
					Allocated += strlen(Url)+1;
					Links = MoreLinks;
				} else {
					log_error(true,"***Error: Memory g_realloc @ manager copy url");
					break;
				}
			}
			if(i == 0)
				sprintf(Links,"%s ", Url);
			else
				sprintf(strchr(Links, '\0'),"%s ", Url);
			++i;
		}
	}
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
							Links, strlen(Links));
	g_free(Url);
	g_free(Links);
	g_free(MoreLinks);
	g_list_free_full (PathsList, (GDestroyNotify) gtk_tree_path_free);

}

void manager_tree_menu_open_file (GtkWidget *menuitem, gpointer userdata) {

	char GFP[4096] = "";
	gchar *FilePath = NULL;
	GtkTreeIter iter;
	GtkTreeView *view = GTK_TREE_VIEW(tree_um);
	GtkTreeModel *model_get = gtk_tree_view_get_model(view);
	GtkTreeSelection *selection= gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_um));
	GList *PathsList = gtk_tree_selection_get_selected_rows (selection, &model_get);
	for (PathsList = PathsList; PathsList != NULL; PathsList = PathsList->next)
	{
		GtkTreePath *path = PathsList->data;
		if (gtk_tree_model_get_iter(model_get, &iter, path)) {

			gtk_tree_model_get (model_get, &iter, FPATH_COLUMN, &FilePath, -1);
			sprintf(GFP,"file://%s",FilePath);
			show_uri(NULL,GFP);
		}
	}
	g_free(FilePath);
	g_list_free_full (PathsList, (GDestroyNotify) gtk_tree_path_free);
}

void manager_tree_menu_open_folder (GtkWidget *menuitem, gpointer userdata) {

	char GFP[4096] = "";
	gchar *FilePath = NULL;
	gchar *FileName = NULL;
	GtkTreeIter iter;
	GtkTreeView *view = GTK_TREE_VIEW(tree_um);
	GtkTreeModel *model_get = gtk_tree_view_get_model(view);
	GtkTreeSelection *selection= gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_um));
	GList *PathsList = gtk_tree_selection_get_selected_rows (selection, &model_get);
	for (PathsList = PathsList; PathsList != NULL; PathsList = PathsList->next)
	{
		GtkTreePath *path = PathsList->data;
		if (gtk_tree_model_get_iter(model_get, &iter, path)) {

			gtk_tree_model_get(model_get, &iter, FPATH_COLUMN, &FilePath, -1);
			gtk_tree_model_get(model_get, &iter, FILENAME_COLUMN, &FileName, -1);
			#ifdef __linux__
			snprintf(GFP,7+(strlen(FilePath)-strlen(FileName)),"file://%s",FilePath);
			show_uri(NULL,GFP);
			#elif _WIN32
			snprintf(GFP,1+(strlen(FilePath)-(strlen(FileName))),"%s",FilePath);
			show_uri("EX",GFP);
			#endif
		}
	}
	g_free(FilePath);
	g_free(FileName);
	g_list_free_full (PathsList, (GDestroyNotify) gtk_tree_path_free);
}

void manager_tree_menu_open_url (GtkWidget *menuitem, gpointer userdata) {

	gchar *Url = NULL;
	GtkTreeIter iter;
	GtkTreeView *view = GTK_TREE_VIEW(tree_um);
	GtkTreeModel *model_get = gtk_tree_view_get_model(view);
	GtkTreeSelection *selection= gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_um));
	GList *PathsList = gtk_tree_selection_get_selected_rows (selection, &model_get);
	for (PathsList = PathsList; PathsList != NULL; PathsList = PathsList->next)
	{
		GtkTreePath *path = PathsList->data;
		if (gtk_tree_model_get_iter(model_get, &iter, path)) {

			gtk_tree_model_get(model_get, &iter, PURL_COLUMN, &Url, -1);
			show_uri(NULL,Url);
		}
	}
	g_free(Url);
	g_list_free_full (PathsList, (GDestroyNotify) gtk_tree_path_free);
}

void manager_tree_menu_del_db (GtkWidget *menuitem, gpointer userdata) {

	gchar *Url = NULL;
	GtkTreeIter iter;
	GtkTreeView *view = GTK_TREE_VIEW(tree_um);
	GtkTreeModel *model_get = gtk_tree_view_get_model(view);
	GtkTreeSelection *selection= gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_um));
	GList *PathsList = gtk_tree_selection_get_selected_rows (selection, &model_get);
	for (PathsList = PathsList ; PathsList != NULL ; PathsList = PathsList->next)
	{
		GtkTreePath *path = PathsList->data;
		if (gtk_tree_model_get_iter(model_get, &iter, path)) {

			gtk_tree_model_get(model_get, &iter, PURL_COLUMN, &Url, -1);
			SQLite.execute2("DELETE FROM uploads WHERE url = ?",1,Url);
		}
	}
	g_free(Url);
	g_list_free_full (PathsList, (GDestroyNotify) gtk_tree_path_free);
	manager_db_search();
}

void manager_tree_menu_del_web (GtkWidget *menuitem, gpointer userdata) {


	/* Will be added when it can be tested.. e.g neku adds delete urls
	gchar *Url = NULL;
	GtkTreeIter iter;
	GtkTreeView *view = GTK_TREE_VIEW(tree_um);
	GtkTreeModel *model_get = gtk_tree_view_get_model(view);
	GtkTreeSelection *selection= gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_um));
	GList *PathsList = gtk_tree_selection_get_selected_rows (selection, &model_get);
	for (PathsList = PathsList ; PathsList != NULL; PathsList = PathsList->next)
	{
		GtkTreePath *path = PathsList->data;
		if (gtk_tree_model_get_iter(model_get, &iter, path)) {


		}
	}
	g_free(Url);
	g_list_free_full (PathsList, (GDestroyNotify) gtk_tree_path_free);
	*/
}

gboolean manager_tree_menu_cb (GtkWidget *treeview, gpointer userdata) {

	manager_tree_menu(treeview, NULL, userdata , NULL);
	return TRUE;
}

void manager_tree_row_dclick (GtkTreeView *tree_um,
										GtkTreeViewColumn  *col, gpointer userdata) {

	if (bUM_DoubleClick) {

		if(iUM_DC_Action == 0)
			manager_tree_menu_open_file(NULL,tree_um);
		else if(iUM_DC_Action == 1)
			manager_tree_menu_open_url(NULL,tree_um);
		else if(iUM_DC_Action == 2)
			manager_tree_menu_open_folder(NULL,tree_um);
		else if(iUM_DC_Action == 3)
			manager_tree_menu_copy_url(NULL,tree_um);
		else
			return;
	}
}

gboolean manager_tree_button_cb (GtkWidget *treeview, GdkEventButton *event,
								gpointer userdata) {
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3) {

		if (gtk_tree_selection_count_selected_rows(selection)  <= 1) {

			if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint) event->x,
											(gint) event->y, &tree_path_um, NULL, NULL, NULL))
			{
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, tree_path_um);
			}
			if(tree_path_um != NULL)
				manager_tree_menu(treeview, event, userdata , tree_path_um);
		} else {

			gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint) event->x,
											(gint) event->y, &tree_path_um, NULL, NULL, NULL);
			if(tree_path_um != NULL)
				manager_tree_menu(treeview, event, userdata , tree_path_um);
		}
		return TRUE;
	}
	if (event->type == GDK_BUTTON_PRESS  &&  event->button == 1 && bUM_Preview) {

		if (gtk_tree_selection_count_selected_rows(selection)  <= 1) {
			gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint) event->x,
											(gint) event->y, &tree_path_um, NULL, NULL, NULL);

			if(tree_path_um != NULL)
				manager_tree_preview_cb(treeview, event, userdata , tree_path_um);
		}
		return FALSE;
	}
	return FALSE;
}

void manager_tree_preview_cb (GtkWidget *treeview, GdkEventButton *event,
						gpointer userdata ,GtkTreePath *path) {

	GtkTreeView *tree_view = GTK_TREE_VIEW(treeview);
	GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
	char *FilePath = NULL;
	if (gtk_tree_model_get_iter(model, &iter, path)) {

		gtk_tree_model_get(model, &iter, FPATH_COLUMN, &FilePath, -1);
		manager_tree_preview_update(FilePath);
	}
	FilePath = NULL;
}

void manager_tree_preview_update (char *FilePath) {

	GdkPixbuf *pixbuf;
	GdkPixbufAnimation *pixbuf_anim;
	gboolean IsCertain;
	bool IsStatic;
	char *pMimeType = NULL;
	char *pContentType = g_content_type_guess (FilePath, NULL, 0, &IsCertain);
	if (pContentType != NULL) {

		pMimeType = g_content_type_get_mime_type (pContentType);
		/* g_print ("FILE :'%s'\n\tContent type:\t%s (certain: %s)\n\tMIME type:\t%s\n",
						FilePath,  pContentType,  IsCertain ? "yes" : "no", pMimeType); */  /// Debug Line
		g_free (pContentType);
	}
	/// GTK+3.6.4-win32 gdk_pixbuf_new_from_file_at_size broken on images other than png - Temp solution duh
	#ifdef __linux__
	if(strncmp(pMimeType,"image",5) != 0  ) {
    #elif _WIN32
    if(strncmp(pMimeType,"image/png",9) != 0  ) {
    #endif
		pMimeType = NULL;
		gtk_image_clear (GTK_IMAGE (preview_um));
		return;
	}
	pMimeType = NULL;

	if(bUM_AnimPreview) {

		pixbuf_anim = gdk_pixbuf_animation_new_from_file(FilePath, NULL);
		IsStatic = gdk_pixbuf_animation_is_static_image (pixbuf_anim);
		if(IsStatic) {
			pixbuf = gdk_pixbuf_new_from_file_at_size (FilePath,
														iUM_Prev_Size_W, iUM_Prev_Size_H, NULL);
			gtk_image_set_from_pixbuf (GTK_IMAGE (preview_um), pixbuf);
			g_object_unref (pixbuf);
			g_object_unref (pixbuf_anim);
		} else {
			gtk_image_set_from_animation (GTK_IMAGE (preview_um), pixbuf_anim);
			g_object_unref (pixbuf_anim);
		}
	} else {
		pixbuf = gdk_pixbuf_new_from_file_at_size (FilePath,
														iUM_Prev_Size_W, iUM_Prev_Size_H, NULL);
		gtk_image_set_from_pixbuf (GTK_IMAGE (preview_um), pixbuf);
		g_object_unref (pixbuf);
	}
}

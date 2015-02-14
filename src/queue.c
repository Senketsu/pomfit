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
#include <gdk/gdk.h>
#include <sys/stat.h>
#include "structs.h"
#include "queue.h"
#include "log.h"
#include "uploader.h"

extern int iUQ_Win_Pos;
extern int iUQ_Win_W;
extern int iUQ_Win_H;
extern UpProfile ActiveProfile;

extern void file_chooser_start(const char *keystring, void *user_data);
extern void show_uri(const char *keystring, void *user_data);

GtkWidget *win_que_main, *win_que_scrl, *tree_queue , *status_bar_uq;
GtkTreeIter iter,Fill_Iter;
GtkTreePath *tree_path_que;
GtkTreeStore *tree_store_que;

enum { FILENAME_COLUMN, STATUS_COLUMN, SIZE_COLUMN,
						FPATH_COLUMN, N_COLUMNS };

void queue_window_create (void) {

	if(win_que_main != NULL) {
		gtk_widget_hide(win_que_main);
		gtk_widget_show(win_que_main);
		return;
	}
	queue_tree_create();
	GtkWidget *vbox_que_m, *hbox_que_m, *lab_e;
	GtkWidget *but_que_hide, *but_que_browse, *but_que_cancel, *but_que_upload, *but_que_clear;

	win_que_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(win_que_main), "Pomf it! - Upload Queue");
	g_signal_connect(win_que_main, "destroy", G_CALLBACK(queue_window_destroy), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(win_que_main), 10);
	gtk_window_resize(GTK_WINDOW(win_que_main), iUQ_Win_W, iUQ_Win_H);
	gtk_window_set_position(GTK_WINDOW(win_que_main), iUQ_Win_Pos);

	vbox_que_m = gtk_box_new(GTK_ORIENTATION_VERTICAL , 0);
	gtk_container_add(GTK_CONTAINER(win_que_main), vbox_que_m);

	win_que_scrl = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox_que_m), win_que_scrl, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(win_que_scrl), GTK_WIDGET(tree_queue));

	status_bar_uq = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox_que_m),GTK_WIDGET(status_bar_uq),FALSE,TRUE,0);
	gtk_statusbar_push (GTK_STATUSBAR(status_bar_uq), 1, "Queue Status Bar..");

	hbox_que_m = gtk_box_new(GTK_ORIENTATION_HORIZONTAL , 0);
	gtk_box_pack_end(GTK_BOX(vbox_que_m), hbox_que_m, FALSE, FALSE, 0);

	but_que_cancel = gtk_button_new_with_label("Cancel");
	g_signal_connect(but_que_cancel,"clicked",G_CALLBACK(queue_window_cancel), NULL);
	gtk_container_add(GTK_CONTAINER(hbox_que_m), but_que_cancel);

	but_que_clear = gtk_button_new_with_label(" Clear ");
	g_signal_connect(but_que_clear,"clicked",G_CALLBACK(queue_tree_clear), NULL);
	gtk_container_add(GTK_CONTAINER(hbox_que_m), but_que_clear);

	lab_e = gtk_label_new("  ");
	gtk_container_add(GTK_CONTAINER(hbox_que_m), lab_e);

	but_que_browse = gtk_button_new_with_label(" Hide ");
	g_signal_connect(but_que_browse,"clicked",G_CALLBACK(queue_window_hide), NULL);
	gtk_container_add(GTK_CONTAINER(hbox_que_m), but_que_browse);

	lab_e = gtk_label_new("  ");
	gtk_container_add(GTK_CONTAINER(hbox_que_m), lab_e);

	but_que_hide = gtk_button_new_with_label("Add Files");
	g_signal_connect(but_que_hide,"clicked",G_CALLBACK(queue_file_chooser), NULL);
	gtk_container_add(GTK_CONTAINER(hbox_que_m), but_que_hide);

	lab_e = gtk_label_new("  ");
	gtk_container_add(GTK_CONTAINER(hbox_que_m), lab_e);

	but_que_upload = gtk_button_new_with_label("Upload Files");
	g_signal_connect(but_que_upload,"clicked",G_CALLBACK(queue_tree_evaluate), NULL);
	gtk_box_pack_end(GTK_BOX(hbox_que_m),GTK_WIDGET(but_que_upload),FALSE,FALSE,0);

	gtk_widget_show_all(win_que_main);
}

void queue_window_destroy (void) {

	gtk_widget_destroy(win_que_main);
	win_que_main = NULL;
}

void queue_window_cancel (void) {

    queue_tree_clear();
	gtk_widget_hide(win_que_main);
}

void queue_window_hide (void) {

	gtk_widget_hide(win_que_main);
}

void queue_file_chooser (void) {

	file_chooser_start(NULL, "DONGS");
}

void queue_tree_clear (void) {

	gtk_tree_store_clear(tree_store_que);
	gtk_statusbar_push (GTK_STATUSBAR(status_bar_uq), 1, "No valid files to upload..");
}

void queue_tree_create (void) {

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	tree_store_que = gtk_tree_store_new (N_COLUMNS,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING);
	tree_queue = gtk_tree_view_new_with_model (GTK_TREE_MODEL (tree_store_que));
	g_signal_connect(tree_queue, "button-press-event",
									G_CALLBACK(queue_tree_button_cb), tree_queue);
	g_signal_connect(tree_queue, "popup-menu", G_CALLBACK(queue_tree_menu_cb), tree_queue);
	g_object_unref (G_OBJECT (tree_store_que));

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),"foreground", "cyan", NULL);
	column = gtk_tree_view_column_new_with_attributes ("File Name", renderer,
												"text", FILENAME_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_queue), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("STATUS",renderer,
												"text", STATUS_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_queue), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Size (MB)",renderer,
												"text", SIZE_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_queue), column);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),"foreground", "gray", NULL);
	column = gtk_tree_view_column_new_with_attributes ("File Path",renderer,
												"text", FPATH_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_queue), column);
}

int queue_tree_add_row (char *FilePath) {

	long int FileSize = 0;
	long int FileSizeLimit = ActiveProfile.MaxSize*1000000;
	char sSize[64];
	char *FileName = NULL;

	FILE *fpCheck;
	fpCheck = fopen(FilePath, "rb");
	if(!fpCheck)
		return -1;
	else {
		fseek(fpCheck, 0 , SEEK_END);
		FileSize = ftell(fpCheck);
		fclose(fpCheck);
	}
	#ifdef __linux__
	FileName = strrchr(FilePath, '/');
	#elif _WIN32
	FileName = strrchr(FilePath, '\\');
	#endif
	FileName += 1;

	sprintf(sSize,"%.2f MB",(float)FileSize/1000000);

	gtk_tree_store_append(tree_store_que, &Fill_Iter, NULL);
	gtk_tree_store_set(tree_store_que, &Fill_Iter,
								FILENAME_COLUMN, FileName,
								STATUS_COLUMN, FileSize>FileSizeLimit ? "BIG" : "OK",
								SIZE_COLUMN, sSize,
								FPATH_COLUMN, FilePath,-1);

	FileName = NULL;
	queue_status_update();
	return 0;
}

void queue_status_update (void) {

	gboolean valid;
	GtkTreeIter read_Iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_queue));

	gchar *StatusData = NULL;
	gchar *FilePath = NULL;
	int iValidFiles = 0;
	int iBadFiles = 0;
	long long iTotalSize = 0;
	long long FileSize = 0;

	valid = gtk_tree_model_get_iter_first (model, &read_Iter);
	while (valid) {

		gtk_tree_model_get (model, &read_Iter, STATUS_COLUMN, &StatusData,  -1);
		if(strcmp(StatusData,"OK") == 0) {
			++iValidFiles;
			gtk_tree_model_get (model, &read_Iter, FPATH_COLUMN, &FilePath,  -1);

			FILE *fpCheck;
			fpCheck = fopen(FilePath, "rb");
			if(!fpCheck)
				;
			else {
				fseek(fpCheck, 0 , SEEK_END);
				FileSize = ftell(fpCheck);
				fclose(fpCheck);
			}
			iTotalSize += FileSize;
			g_free (FilePath);

		} else
			++iBadFiles;
		g_free (StatusData);
		valid = gtk_tree_model_iter_next (model,  &read_Iter);

	}
	if(iValidFiles > 0) {
		char sSizeMiB[24] = "";
		char sSizeMB[24] = "";
		char Status[512] = "";
		sprintf(sSizeMiB,"%.2f MiB",(float)iTotalSize/1048576);
		sprintf(sSizeMB,"%.2f MB",(float)iTotalSize/1000000);
		sprintf(Status,"OK Files:%3d | Bad:%2d | Total Size: %s (%s)",
								iValidFiles,iBadFiles,sSizeMiB,sSizeMB );
		gtk_statusbar_push (GTK_STATUSBAR(status_bar_uq), 1, Status);
	} else
		gtk_statusbar_push (GTK_STATUSBAR(status_bar_uq), 1, "No valid files to upload..");

}


void queue_tree_evaluate (void) {

	gtk_widget_hide(win_que_main);

	gboolean valid;
	GtkTreeIter read_Iter;
	GtkTreeModel *model;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_queue));
	gint row_count = 0;
	int iQueueLen = 0;
	int iValidFiles = 0;
	int i ,j = 0;

	gchar *StatusData = NULL;

	valid = gtk_tree_model_get_iter_first (model, &read_Iter);
	while (valid) {

		gtk_tree_model_get (model, &read_Iter, STATUS_COLUMN, &StatusData,  -1);
		if(strcmp(StatusData,"OK") == 0)
			++iValidFiles;
		g_free (StatusData);
		iQueueLen++;
		valid = gtk_tree_model_iter_next (model,  &read_Iter);

	}
	if(iValidFiles == 0)
		return;
	gchar *PathData[iQueueLen];
	gpointer pUpQueue[iValidFiles];

	for(i = 0 ; i < iValidFiles ; ++i)
		pUpQueue[i] = NULL;

	valid = gtk_tree_model_get_iter_first (model, &read_Iter);
	while (valid) {

	   gtk_tree_model_get (model, &read_Iter,  FPATH_COLUMN, &PathData[row_count],
						   STATUS_COLUMN, &StatusData,  -1);
		if(strcmp(StatusData,"OK") == 0) {
			pUpQueue[j] = (void*)PathData[row_count];
			++j;
		}

		g_free (StatusData);
		valid = gtk_tree_model_iter_next (model,  &read_Iter);
		row_count++;
	}
	uploader_curl_file(pUpQueue,(int)iValidFiles);
	for(i = 0 ; i < iQueueLen ; ++i)
		g_free (PathData[i]);
	for(i = 0 ; i < iValidFiles ; ++i)
		pUpQueue[i] = NULL;

	queue_tree_clear();
	queue_window_destroy();
}

gboolean queue_tree_menu_cb (GtkWidget *treeview, gpointer userdata) {

	queue_tree_menu(treeview, NULL, userdata , NULL);
	return TRUE;
}

gboolean queue_tree_button_cb (GtkWidget *treeview, GdkEventButton *event,
								gpointer userdata) {

	if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3) {

		GtkTreeSelection *selection;
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

		if (gtk_tree_selection_count_selected_rows(selection)  <= 1)
		{
			if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint) event->x,
											(gint) event->y, &tree_path_que, NULL, NULL, NULL))
			{
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, tree_path_que);
			}
			if(tree_path_que != NULL)
				queue_tree_menu(treeview, event, userdata , tree_path_que);
		}
		return TRUE;
	}
	return FALSE;
}

void queue_tree_menu (GtkWidget *treeview, GdkEventButton *event,
						gpointer userdata ,GtkTreePath *path) {
	GtkWidget *tree_menu;
	GtkWidget *tree_menu_open_file, *tree_menu_open_folder, *tree_menu_remove;
	tree_menu = gtk_menu_new();
	tree_menu_open_file = gtk_menu_item_new_with_label("Open File");
	tree_menu_open_folder = gtk_menu_item_new_with_label("Open Folder");
	tree_menu_remove = gtk_menu_item_new_with_label("Remove");

	g_signal_connect(tree_menu_open_file, "activate",
							G_CALLBACK (queue_tree_menu_open_file), treeview);
	g_signal_connect(tree_menu_open_folder, "activate",
							G_CALLBACK (queue_tree_menu_open_folder), treeview);
	g_signal_connect(tree_menu_remove, "activate",
							G_CALLBACK (queue_tree_menu_remove), treeview);

	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_open_file);
	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_open_folder);
	gtk_menu_shell_append(GTK_MENU_SHELL(tree_menu), tree_menu_remove);
	gtk_widget_show_all(tree_menu);
	gtk_menu_popup(GTK_MENU(tree_menu), NULL, NULL, NULL, NULL,
					(event != NULL) ? event->button : 0,
					gdk_event_get_time((GdkEvent*)event));
}

void queue_tree_menu_open_file (GtkWidget *menuitem, gpointer userdata) {

	GtkTreeView *treeview = GTK_TREE_VIEW(userdata);
	GtkTreeModel *model;
	model = gtk_tree_view_get_model(treeview);
	char *FilePath = NULL;
	if (gtk_tree_model_get_iter(model, &iter, tree_path_que)) {

		gtk_tree_model_get(model, &iter, FPATH_COLUMN, &FilePath, -1);
		char GFP[4096] = "";
		sprintf(GFP,"file://%s",FilePath);
		show_uri(NULL,GFP);
	}
	FilePath = NULL;
}

void queue_tree_menu_open_folder (GtkWidget *menuitem, gpointer userdata) {

	GtkTreeView *treeview = GTK_TREE_VIEW(userdata);
	GtkTreeModel *model;
	model = gtk_tree_view_get_model(treeview);
	char *FilePath = NULL;
	char *FileName = NULL;
	if (gtk_tree_model_get_iter(model, &iter, tree_path_que)) {

		gtk_tree_model_get(model, &iter, FPATH_COLUMN, &FilePath, -1);
		gtk_tree_model_get(model, &iter, FILENAME_COLUMN, &FileName, -1);
		char GFP[1024] = "";
		snprintf(GFP,7+(strlen(FilePath)-strlen(FileName)),"file://%s",FilePath);
		show_uri(NULL,GFP);
	}
	FilePath = NULL;
	FileName = NULL;
}

void queue_tree_menu_remove (GtkWidget *menuitem, gpointer userdata) {

	GtkTreeView *treeview = GTK_TREE_VIEW(userdata);
	GtkTreeModel *model;
	model = gtk_tree_view_get_model(treeview);
	if (gtk_tree_model_get_iter(model, &iter, tree_path_que)) {

		gtk_tree_store_remove (tree_store_que,  &iter);
	}

	queue_status_update();
}

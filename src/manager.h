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

#ifndef POMFIT_MANAGER_H
	#define POMFIT_MANAGER_H
#ifdef __cplusplus
extern "C"
{
#endif
	void manager_tree_add_row (char *filename, char *up_url, char *del_url, char *file_path);
	void manager_tree_clear (void);
	void manager_tree_list_all (void);
	void manager_tree_create (void);

	void manager_window_create (void);
	void manager_window_destroy (GtkWidget *window) ;
    void manager_window_cancel (void);
    void manager_window_hide (void);
	void manager_db_delete (void);
	void manager_db_search (void);

	void manager_tree_menu_del_web (GtkWidget *menuitem, gpointer userdata);
	void manager_tree_menu_del_db (GtkWidget *menuitem, gpointer userdata);
	void manager_tree_menu_copy_url (GtkWidget *menuitem, gpointer userdata);
	void manager_tree_menu_copy_path (GtkWidget *menuitem, gpointer userdata);
	void manager_tree_menu_open_url (GtkWidget *menuitem, gpointer userdata);
	void manager_tree_menu_copy_del_url (GtkWidget *menuitem, gpointer userdata);
	void manager_tree_menu_open_file (GtkWidget *menuitem, gpointer userdata);
	void manager_tree_menu_open_folder (GtkWidget *menuitem, gpointer userdata);
	void manager_tree_menu (GtkWidget *treeview, GdkEventButton *event,
												gpointer userdata ,GtkTreePath *path);

	void manager_tree_row_dclick (GtkTreeView *tree_um,
										GtkTreeViewColumn  *col, gpointer userdata);

	gboolean manager_tree_menu_cb (GtkWidget *treeview, gpointer userdata);
	gboolean manager_tree_button_cb (GtkWidget *treeview, GdkEventButton *event,
															gpointer userdata);

	void manager_tree_preview_update (char *path);
	void manager_tree_preview_cb (GtkWidget *treeview, GdkEventButton *event,
														gpointer userdata ,GtkTreePath *path);
enum
{
	FILENAME_COLUMN,
	PURL_COLUMN,
	DELURL_COLUMN,
	FPATH_COLUMN,
	N_COLUMNS
};
#ifdef __cplusplus
}
#endif
#endif


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

#ifndef POMFIT_QUEUE_H
	#define POMFIT_QUEUE_H

#ifdef __cplusplus
extern "C"
{
#endif
	void queue_window_create (void);
	void queue_window_destroy (void);
	void queue_window_hide (void);
    void queue_window_cancel (void) ;
	void queue_status_update (void);
	void queue_file_chooser (void);

	void queue_tree_clear (void);
	void queue_tree_create (void);
	void queue_tree_evaluate (void);

	int queue_tree_add_row (char *FilePath);

	void queue_tree_menu_remove (GtkWidget *menuitem, gpointer userdata);
	void queue_tree_menu_open_file (GtkWidget *menuitem, gpointer userdata);
	void queue_tree_menu_open_folder (GtkWidget *menuitem, gpointer userdata);
	void queue_tree_menu (GtkWidget *treeview, GdkEventButton *event,
						gpointer userdata ,GtkTreePath *path);

	gboolean queue_tree_menu_cb (GtkWidget *treeview, gpointer userdata);
	gboolean queue_tree_button_cb (GtkWidget *treeview, GdkEventButton *event,
								gpointer userdata);


#ifdef __cplusplus
}
#endif

#endif

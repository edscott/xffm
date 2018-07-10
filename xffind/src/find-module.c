
/*
 * Copyright (C) 2002-2014 Edscott Wilson Garcia
 * EMail: edscott@users.sf.net
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "rodent.h"

#define HISTORY_LIMIT 20
#define GLOB "fgr"
#define FIND "find"


typedef struct filechooser_t {
    GtkWidget *parent;
    void *combo_info;
    int filechooser_action;
    const gchar *folder;
    GtkEntry *entry;
} filechooser_t;

typedef struct find_struct_t {
    widgets_t *widgets_p;
    GSList *controllers;
    void *find_combo_info;
    void *findpath_combo_info;
    void *findgrep_combo_info;
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    GtkComboBoxEntry *combo;
    GtkComboBoxEntry *combopath;
    GtkComboBoxEntry *combogrep;
#else
    GtkComboBox *combo;
    GtkComboBox *combopath;
    GtkComboBox *combogrep;
#endif
    filechooser_t filechooser_v;
    GtkWidget *window;
    GtkWidget *dialog;
    GtkWidget *diagnostics;
} find_struct_t;

#include "combobox-module.h"

#include "find-module.h"
#include "find-module_gui.h"

// this static variables mean we cannot have two or more threads
// running find dialogs. This is OK today because the dialog
// is transient and blocks the main thread from creating any more
// dialogs (dialog run in main thread, not generated thread)
// This may change in the future and for that we will need a method
// which is safe on multiple calls to destroy function...
#include "find-module.i"

G_MODULE_EXPORT void *
module_active (void){
    return GINT_TO_POINTER(1);
}

static void *
do_find_basic (const gchar *path) {
    if(!path || !g_file_test(path, G_FILE_TEST_IS_DIR)) {
	path = "/";
    }
    NOOP ("FIND: path=%s\n", path);
	
    find_struct_t *find_struct_p =
	    (find_struct_t *) malloc (sizeof (find_struct_t));
    if (!find_struct_p) g_error("malloc: %s", strerror(errno));
   
    memset (find_struct_p, 0, sizeof (find_struct_t));

    GtkWidget *dialog = find_struct_p->dialog = create_find_dialog ();
    g_object_set_data(G_OBJECT(dialog),"find_struct_p", find_struct_p);
    

    find_struct_p->window = g_object_get_data(G_OBJECT(dialog), "window");
    find_struct_p->widgets_p = g_object_get_data(G_OBJECT(dialog), "widgets_p");

    NOOP ("FGR: find_dialog=0x%lx\n", (unsigned long)dialog);

    g_signal_connect (g_object_get_data(G_OBJECT(dialog), "apply_button"),
                      "clicked", G_CALLBACK (on_apply_clicked), 
		      (gpointer) find_struct_p);
    g_signal_connect (g_object_get_data(G_OBJECT(dialog), "find_button"),
                      "clicked", G_CALLBACK (on_find_clicked), 
		      (gpointer) find_struct_p);
    g_signal_connect (g_object_get_data(G_OBJECT(dialog), "close_button"),
                      "clicked", G_CALLBACK (on_find_close), 
		      (gpointer) find_struct_p);
    g_signal_connect (g_object_get_data(G_OBJECT(dialog), "cancel_button"),
                      "clicked", G_CALLBACK (cancel_callback), 
		      (gpointer) find_struct_p);

    find_struct_p->combo = 
	g_object_get_data(G_OBJECT(dialog), "filter_combo"); 
    if (!find_struct_p->combo) {g_error("cannot find widget filter_combo"); }
    find_struct_p->combopath = 
	g_object_get_data(G_OBJECT(dialog), "path_combo");
    if (!find_struct_p->combopath) {g_error("cannot find widget path_combo"); }
    find_struct_p->combogrep =
	g_object_get_data(G_OBJECT(dialog), "grep_combo"); 
    if (!find_struct_p->combogrep) {g_error("cannot find widget grep_combo"); }
    gtk_widget_show (dialog);


    gchar *f = g_build_filename (FIND_GREP_DBH_FILE, NULL);

    find_struct_p->findgrep_combo_info = COMBOBOX_init_combo (find_struct_p->combogrep, MATCH_NONE);
    if (find_struct_p->findgrep_combo_info == NULL) {
	DBG("cannot initialize findgrep_combo_info\n");
    }


    COMBOBOX_set_activate_function(
	    find_struct_p->findgrep_combo_info, on_find_clicked_wrapper);
    COMBOBOX_set_activate_user_data(
	    find_struct_p->findgrep_combo_info, find_struct_p);

    COMBOBOX_read_history (find_struct_p->findgrep_combo_info, f);
    COMBOBOX_set_combo (find_struct_p->findgrep_combo_info);
    g_free (f);

    f = g_build_filename (FIND_DBH_FILE, NULL);

    find_struct_p->find_combo_info = COMBOBOX_init_combo (find_struct_p->combo, MATCH_NONE);
    COMBOBOX_set_activate_function(
	    find_struct_p->find_combo_info, on_find_clicked_wrapper);
    COMBOBOX_set_activate_user_data(
	    find_struct_p->find_combo_info, find_struct_p);

    COMBOBOX_read_history (find_struct_p->find_combo_info, f);
    COMBOBOX_set_combo (find_struct_p->find_combo_info);
    g_free (f);
    COMBOBOX_set_default(find_struct_p->find_combo_info);

    f = g_build_filename (GOTO_DBH_FILE, NULL);
    //  g_build_filename(g_get_user_data_dir(), FIND_PATHS_DBH_FILE, NULL);
    find_struct_p->findpath_combo_info = COMBOBOX_init_combo (find_struct_p->combopath, MATCH_FILE);

    COMBOBOX_set_activate_function(
	    find_struct_p->findpath_combo_info, on_find_clicked_wrapper);
    COMBOBOX_set_activate_user_data(
	    find_struct_p->findpath_combo_info, find_struct_p);

    COMBOBOX_read_history (find_struct_p->findpath_combo_info, f);
    COMBOBOX_set_combo (find_struct_p->findpath_combo_info);
    g_free (f);

    find_struct_p->filechooser_v.folder = path;
    find_struct_p->filechooser_v.entry = GTK_ENTRY(
	    gtk_bin_get_child(GTK_BIN (find_struct_p->combopath)));

    COMBOBOX_set_entry (find_struct_p->findpath_combo_info, path);

    if (rfm_void(RFM_MODULE_DIR, "combobox", "module_active")){

	g_object_set_data(
	    G_OBJECT(COMBOBOX_get_entry_widget(find_struct_p->findgrep_combo_info)),
	    "dialog", dialog);
	g_object_set_data(
	    G_OBJECT(COMBOBOX_get_entry_widget(find_struct_p->find_combo_info)),
	    "dialog", dialog);
	g_object_set_data(
	    G_OBJECT(COMBOBOX_get_entry_widget(find_struct_p->findpath_combo_info)),
	    "dialog", dialog);

    } else 
    {
	g_object_set_data(
	    G_OBJECT( gtk_bin_get_child(
		    GTK_BIN (find_struct_p->combo))),
	    "dialog", dialog);
	g_object_set_data(
	    G_OBJECT( gtk_bin_get_child(
		    GTK_BIN (find_struct_p->combopath))),
	    "dialog", dialog);
	g_object_set_data(
	    G_OBJECT( gtk_bin_get_child(
		    GTK_BIN (find_struct_p->combogrep))),
	    "dialog", dialog);
		
	gtk_entry_set_text (GTK_ENTRY (gtk_bin_get_child(
			GTK_BIN (find_struct_p->combopath))),path);
	
    }

    find_struct_p->filechooser_v.parent = dialog;
    find_struct_p->filechooser_v.filechooser_action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    find_struct_p->filechooser_v.combo_info = find_struct_p->findpath_combo_info;

    g_signal_connect (g_object_get_data(G_OBJECT(dialog), "fileselector"), 
		    "clicked", G_CALLBACK (path_filechooser), 
		    (gpointer) (&find_struct_p->filechooser_v));

    int i;
    GSList *type_list = NULL;    
    for(i = 0; ftypes[i] != NULL; i++) {
	type_list = g_slist_append (type_list, _(ftypes[i]));
    }

    fill_string_option_menu ((GtkComboBox *)g_object_get_data(
		G_OBJECT(dialog), "file_type_om"), 
			     type_list);
    g_slist_free(type_list);

    g_signal_connect(G_OBJECT (gtk_bin_get_child(GTK_BIN(find_struct_p->combogrep))),
	    "changed", G_CALLBACK (grep_options), (gpointer) find_struct_p);
    g_signal_connect (G_OBJECT (dialog), 
	    "destroy_event", G_CALLBACK (destroy_find), (gpointer) find_struct_p);
    g_signal_connect (G_OBJECT (dialog), 
	    "delete_event", G_CALLBACK (destroy_find), (gpointer) find_struct_p);
    
    return dialog;
}

static gboolean
set_up_dialog(void *data){
    gchar *path = data;
    //GtkWidget *dialog = 
	do_find_basic(path);
    return FALSE;
}
    
	

void *
do_find_standalone (gchar *path){
    g_idle_add(set_up_dialog, path);
    gtk_main();
    return GINT_TO_POINTER(1);
}



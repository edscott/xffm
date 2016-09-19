/*
 * Copyright (C) 2003-2012 Edscott Wilson Garcia
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
 * along with this program; 
 */

#define __COMBO_C__
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "rfm.h"

#include "combobox-module.h"
/* this should be first 2 lines after headers: */
G_MODULE_EXPORT LIBRFM_MODULE 

/************** private ***************/

#define MAX_COMBO_ELEMENTS 13

#define HISTORY_ITEMS MAX_COMBO_ELEMENTS
/*************************************************************************/
void *set_default (void *p);
void *get_entry (void *p);
void *get_entry_widget (void *p);
void *init_combo (void *p, void *q);
void *destroy_combo (void *p);
void *clear_history (void *p);
void *is_in_history (void *p, void *q);
void *set_combo (void *p);
void *set_entry (void *p, void *q);
void *save_to_history (void *p, void *q);
void *remove_from_history (void *p, void *q);
void *set_extra_key_completion_function (void *p, void *q);
void *set_extra_key_completion_data (void *p, void *q);
void *set_activate_function (void *p, void *q);
void *set_cancel_function (void *p, void *q);
void *set_activate_user_data (void *p, void *q);
void *set_cancel_user_data (void *p, void *q);
void *read_history (void *p, void *q);
#include "combobox-module.i"
/*************** public *****************/
 
G_MODULE_EXPORT void *
module_active (void){
    return GINT_TO_POINTER(1);
}


G_MODULE_EXPORT void *
g_module_check_init (GModule * module) {
    rfm_mutex_init(sweep_mutex);
    return NULL;
}

G_MODULE_EXPORT void
g_module_unload (GModule * module) {
    rfm_mutex_free(sweep_mutex);
    return;
}

G_MODULE_EXPORT void *
is_in_history (void *p, void *q) {
    if (!p) {
	DBG("is_in_history: dbh_file==NULL!\n");
	return NULL;
    }
    gchar *dbh_file=p;
    gchar *path2save=q;
    GString *gs;
    DBHashTable *d;
    //history_dbh_t *history_dbh;
    gboolean found = FALSE;

    if(!path2save)
        return GINT_TO_POINTER(FALSE);
    if(strlen (path2save) > 255)
        return GINT_TO_POINTER(FALSE);
    TRACE("opening %s...\n",dbh_file); 
    if((d = dbh_new (dbh_file, NULL, DBH_READ_ONLY|DBH_PARALLEL_SAFE)) == NULL)
        return GINT_TO_POINTER(FALSE);
    dbh_set_parallel_lock_timeout(d, 3);
    TRACE("open %s.\n",dbh_file); 
    gs = g_string_new (path2save);
    sprintf ((char *)DBH_KEY (d), "%10u", g_string_hash (gs));
    g_string_free (gs, TRUE);

    //history_dbh = (history_dbh_t *) DBH_DATA (d);
    if(dbh_load (d)) found = TRUE;
    dbh_close (d);
    return GINT_TO_POINTER(found);
}

G_MODULE_EXPORT void *
set_combo (void *p) {
    return internal_set_combo(p, NULL);
}


G_MODULE_EXPORT void *
set_default (void *p) {
    if (!p) {
	DBG("set_default: combo_info==NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info = p;
    GSList *list=combo_info->limited_list;
    if (list) {
	set_entry (p, list->data);
    }
    return NULL;
}



G_MODULE_EXPORT void *
set_entry (void *p, void *q) {
    if (!p) {
	DBG("set_entry: combo_info==NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;

    gtk_entry_set_text(combo_info->entry, (gchar *)q);
    return NULL;
}

G_MODULE_EXPORT void *
get_entry_widget (void *p) {
    if (!p) {
	DBG("get_entry: combo_info==NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
    return (void *) combo_info->entry;
}

G_MODULE_EXPORT void *
get_entry (void *p) {
    if (!p) {
	DBG("get_entry: combo_info==NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
    const gchar *choice = gtk_entry_get_text (GTK_ENTRY (combo_info->entry));

    if(choice && strlen (choice) && combo_info->association_hash) {
        gchar *local_choice = g_hash_table_lookup (combo_info->association_hash, choice);
        NOOP ("converting back to non utf8 value %s ---> %s\n", choice, local_choice);
        if(local_choice)
            choice = local_choice;
    }
    if(choice)
        return (void *)choice;
    return (void *)"";
}

G_MODULE_EXPORT void *
save_to_history (void *p, void *q) {
    if (!p) {
	DBG("save_to_history: dbh_file==NULL!\n");
	return NULL;
    }
    gchar *dbh_file=p;
    const gchar *path2save=q;
    GString *gs;
    DBHashTable *d;
    history_dbh_t *history_dbh;
    int size;

    if(!path2save) return NULL;
    if(strlen (path2save) > 255) return NULL;

    /* directory test */

    gchar *g = g_path_get_dirname (dbh_file);
    g_mkdir_with_parents (g, 0700);
    if(!rfm_g_file_test (g, G_FILE_TEST_IS_DIR)) {
        DBG ("%s is not a directory\n", g);
        g_free (g);
        return NULL;
    }
    g_free (g);

    // Since this is a user driven command, thread collisions
    // are nearly impossible.
    if((d = dbh_new (dbh_file, NULL, DBH_PARALLEL_SAFE)) == NULL) {
        NOOP ("Creating history file: %s", dbh_file);
	unsigned char keylength=11;
        gchar *directory = g_path_get_dirname(dbh_file);
        if (!g_file_test(directory, G_FILE_TEST_IS_DIR)){
            g_mkdir_with_parents(directory, 0700);
        }
        g_free(directory);
        if((d = dbh_new (dbh_file, &keylength, DBH_PARALLEL_SAFE|DBH_CREATE)) == NULL) {
                return NULL;
        }
    }
    dbh_set_parallel_lock_timeout(d, 3);
    gs = g_string_new (path2save);
    sprintf ((char *)DBH_KEY (d), "%10u", g_string_hash (gs));
    g_string_free (gs, TRUE);

    history_dbh = (history_dbh_t *) DBH_DATA (d);
    if(dbh_load (d)) {
        history_dbh->hits++;
    } else {
        strncpy (history_dbh->path, path2save, 255);
        history_dbh->hits = 1;
    }
    history_dbh->last_hit = time (NULL);
    /*NOOP("updating %s to %d\n",history_dbh->path,history_dbh->last_hit); */
    /* don't write more to disk than that which is necessary: */
    size = sizeof (history_dbh_t) + strlen (history_dbh->path) - 255;
    dbh_set_recordsize (d, size);
    dbh_update (d);
    dbh_close (d);
    return  NULL;
}

G_MODULE_EXPORT void *
remove_from_history (void *p, void *q) {
    if (!p) {
	DBG("remove_from_history: dbh_file==NULL!\n");
	return NULL;
    }
    gchar *dbh_file=p;
    gchar *path2save=q;
    GString *gs;
    DBHashTable *d;
    //history_dbh_t *history_dbh;

    if(strlen (path2save) > 255)
        return NULL;
    
    // Since this is a user driven command, thread collisions
    // are nearly impossible.
    if((d = dbh_new (dbh_file, NULL, DBH_PARALLEL_SAFE)) == NULL) {
        NOOP ("Creating history file: %s", dbh_file);
	unsigned char keylength=11;
        gchar *directory = g_path_get_dirname(dbh_file);
        if (!g_file_test(directory, G_FILE_TEST_IS_DIR)){
            g_mkdir_with_parents(directory, 0700);
        }
        g_free(directory);
        if((d = dbh_new (dbh_file, &keylength, DBH_PARALLEL_SAFE|DBH_CREATE)) == NULL) {
                return NULL;
        }
    }
    dbh_set_parallel_lock_timeout(d, 3);

    gs = g_string_new (path2save);
    sprintf ((char *)DBH_KEY (d), "%10u", g_string_hash (gs));
    g_string_free (gs, TRUE);

    //history_dbh = (history_dbh_t *) DBH_DATA (d);
    if(dbh_load (d)) {
        dbh_erase (d);
    }
    dbh_close (d);
    return NULL;
}

G_MODULE_EXPORT void *
set_quick_activate(void *p, void *q){
    if (!p) {
	DBG("set_quick_activate: combo_info == NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
    combo_info->quick_activate = GPOINTER_TO_INT(q);
	return NULL;
}


G_MODULE_EXPORT void *
set_extra_key_completion_function(void *p, void *q){
    if (!p) {
	DBG("set_extra_key_completion_function: combo_info == NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
    combo_info->extra_key_completion=q;
	return NULL;
}

G_MODULE_EXPORT void *
set_extra_key_completion_data(void *p, void *q){
    if (!p) {
	DBG("set_extra_key_completion_data: combo_info == NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
    combo_info->extra_key_data=q;
	return NULL;
}

G_MODULE_EXPORT void *
set_activate_function(void *p, void *q){
    if (!p) {
	DBG("set_activate_function: combo_info == NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
    combo_info->activate_func=q;
	return NULL;
}

G_MODULE_EXPORT void *
set_cancel_function(void *p, void *q){
    if (!p) {
	DBG("set_cancel_function: combo_info == NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
    combo_info->cancel_func=q;
	return NULL;
}

G_MODULE_EXPORT void *
set_activate_user_data(void *p, void *q){
    if (!p) {
	DBG("set_activate_user_data: combo_info == NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
    combo_info->activate_user_data=q;
	return NULL;
}

G_MODULE_EXPORT void *
set_cancel_user_data(void *p, void *q){
    if (!p) {
	DBG("set_cancel_user_data: combo_info == NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
    combo_info->cancel_user_data=q;
	return NULL;
}


G_MODULE_EXPORT void *
init_combo (void *p, void *q) {
    if (!p) {
	DBG("init_combo: comboboxentry == NULL!\n");
	return NULL;
    }
    gint completion_type = GPOINTER_TO_INT(q);
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    GtkComboBoxEntry * comboboxentry=p;
#else
    GtkComboBox * comboboxentry=p;
    if (!gtk_combo_box_get_has_entry(comboboxentry)){
	g_error("FIXME: gtk_combo_box_get_has_entry(comboboxentry) == NULL (Set \"has-entry\" property as TRUE on creation of combobox)"); 
    }
#endif

    combobox_info_t *combo_info;

    combo_info = (combobox_info_t *) malloc (sizeof (combobox_info_t));
    if(!combo_info){
	g_error("cannot allocate memory for combobox_info_t!");
    }
    memset(combo_info, 0, sizeof (combobox_info_t)); 

#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    GtkEntry *entry=GTK_ENTRY (GTK_BIN (comboboxentry)->child);
#else
    GtkEntry *entry=GTK_ENTRY (gtk_bin_get_child (GTK_BIN (comboboxentry)));
#endif
    //gtk_bin_get_child((GtkBin *)comboboxentry);
    
    g_signal_connect (G_OBJECT (comboboxentry), "changed", G_CALLBACK (on_changed), (gpointer) combo_info);

    g_signal_connect (G_OBJECT (entry), "key_press_event", G_CALLBACK (on_key_press), (gpointer) combo_info);
    g_signal_connect (G_OBJECT (entry), "key_press_event", G_CALLBACK (on_key_press_history), (gpointer) combo_info);
#if 0
    g_signal_connect (G_OBJECT (combo->list), "select_child", G_CALLBACK (on_select_child), NULL);
#endif

    combo_info->completion_type = completion_type;
    combo_info->comboboxentry = comboboxentry;
    combo_info->entry = (GtkEntry *) entry;
    combo_info->active_dbh_file = NULL;
    combo_info->list = NULL;
    combo_info->cancel_user_data = NULL;
    combo_info->activate_user_data = NULL;
    combo_info->cancel_func = NULL;
    combo_info->activate_func = NULL;

    combo_info->dead_key=0;
    combo_info->shift_pos = -1;
    combo_info->cursor_pos = -1;
    combo_info->active = -1;
	
    combo_info->limited_list = NULL;
    combo_info->association_hash = NULL;
    combo_info->model=(GtkTreeModel *)gtk_list_store_new (1, G_TYPE_STRING);
    gtk_combo_box_set_model((GtkComboBox *)comboboxentry, combo_info->model);
                                                         
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    gtk_combo_box_entry_set_text_column(comboboxentry, 0);
#else
    gtk_combo_box_set_entry_text_column(comboboxentry, 0);
#endif

    NOOP("combo_info 0x%lx initialized\n", (long)combo_info);
    return (void *)combo_info;
}

G_MODULE_EXPORT void *
destroy_combo (void *p) {
    if (!p) {
	DBG("destroy_combo: combo_info==NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
            
    if (combo_info->association_hash) {
	g_hash_table_destroy (combo_info->association_hash);
    }
    g_free (combo_info->active_dbh_file);
    // free:
    //    ->treemodel
    if (GTK_IS_TREE_STORE (combo_info->model)) {
	gtk_tree_store_clear ((GtkTreeStore *)combo_info->model);
    }
    g_object_unref(combo_info->model);
    //    ->list
    clean_history_list (&(combo_info->list));
    //    ->limited_list
    clean_history_list (&(combo_info->limited_list));
    //    ->old_list
    g_free (combo_info);
    return NULL;
}

G_MODULE_EXPORT void *
read_history (void *p, void *q) {
    if (!p) {
	DBG("read_history: combo_info==NULL!\n");
	return NULL;
    }
    if (!q) {
	DBG("dbh_file==NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
    gchar * dbh_file=q;
/*	NOOP("NOOP:at read_history_list with %s \n",dbh_file);*/
    g_free (combo_info->active_dbh_file);
    combo_info->active_dbh_file = g_strdup (dbh_file);
    if(access (combo_info->active_dbh_file, F_OK) != 0) {
        clean_history_list (&(combo_info->list));
        combo_info->list = NULL;
    }
    get_history_list (&(combo_info->list), combo_info->active_dbh_file, "");
    /* turn asian off to start with. If the combo object does not
     * do a read_history to start, then it has no business being a combo
     * object */
    combo_info->asian = FALSE;
    return NULL;
}

G_MODULE_EXPORT void *
clear_history (void *p) {
    if (!p) {
	DBG("clear_history: combo_info==NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
/*	NOOP("NOOP:at read_history_list with %s \n",dbh_file);*/
    clean_history_list (&(combo_info->list));
    combo_info->list = NULL;
    return NULL;
}


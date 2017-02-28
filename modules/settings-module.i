#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 * Copyright (c) 2006-2012 Edscott Wilson Garcia <edscott@users.sf.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library;  
 */
//#include "primary-options.h"
#include "xmltree.h"


#define LS_OPTIONS get_ls_options()
#define CP_OPTIONS get_cp_options()
#define MV_OPTIONS get_mv_options()
#define LN_OPTIONS get_ln_options()
#define RM_OPTIONS get_rm_options()
#define SHRED_OPTIONS get_shred_options()
/*
#define LS_OPTIONS rfm_void(RFM_MODULE_DIR, "callbacks","get_ls_options")
#define CP_OPTIONS rfm_void(RFM_MODULE_DIR, "callbacks","get_cp_options")
#define MV_OPTIONS rfm_void(RFM_MODULE_DIR, "callbacks","get_mv_options")
#define LN_OPTIONS rfm_void(RFM_MODULE_DIR, "callbacks","get_ln_options")
#define RM_OPTIONS rfm_void(RFM_MODULE_DIR, "callbacks","get_rm_options")
#define SHRED_OPTIONS rfm_void(RFM_MODULE_DIR, "callbacks","get_shred_options")
*/

static GtkWidget *rfm_check_button_new(){
	return gtk_check_button_new ();
}
static void
rfm_check_button_set_active(GtkWidget *button, gboolean state){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), state);
}
static void
scroll_to_top(void *user_data){
    rfm_threadwait(); //sleep(1);
    widgets_t *widgets_p = user_data;
    rfm_context_function(rfm_scroll_to_top, widgets_p); 
}


void *
mcs_set_var (const gchar * setting_name, const gchar * setting_value);

#define SETTINGS_TIMERVAL 750

static void
clear_bgimage (GtkButton * button, gpointer data) {
    settings_t *settings_p=data;
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(settings_p->desktopimage_button), "gtk gives a warning if we use NULL here (that is not good) so we just use this text to set to NULL");
}

void 
margin_changed (GtkSpinButton *spinbutton, gpointer data)  {
    settings_t *settings_p=g_object_get_data(G_OBJECT(spinbutton), "settings_p");
    if(!settings_p || settings_p->disable_options){
        return;
    }
    gint which_margin = GPOINTER_TO_INT(data);


    if(rfm_options[which_margin].value)
        g_free (rfm_options[which_margin].value);

    gdouble value=gtk_spin_button_get_value (spinbutton);
    NOOP ("margin:changed(): %4.0lf\n", value);
    rfm_options[which_margin].value = g_strdup_printf ("%lf", value);
    mcs_manager_set_string (mcs_manager, rfm_options[which_margin].name,
	    CHANNEL, rfm_options[which_margin].value);
    mcs_manager_notify (mcs_manager, CHANNEL);

    return;

}

void 
preview_size_changed (GtkSpinButton *spinbutton, gpointer data)  {
    settings_t *settings_p=g_object_get_data(G_OBJECT(spinbutton), "settings_p");
    if(!settings_p || settings_p->disable_options){
        return;
    }

    if(rfm_options[RFM_PREVIEW_IMAGE_SIZE].value)
        g_free (rfm_options[RFM_PREVIEW_IMAGE_SIZE].value);

    gdouble value=gtk_spin_button_get_value (spinbutton);
    NOOP ("RFM_PREVIEW_IMAGE_SIZE:changed(): %4.0lf\n", value);
    gint iv = value;
    rfm_options[RFM_PREVIEW_IMAGE_SIZE].value = g_strdup_printf ("%d", iv);
    mcs_manager_set_string (mcs_manager, rfm_options[RFM_PREVIEW_IMAGE_SIZE].name,
	    CHANNEL, rfm_options[RFM_PREVIEW_IMAGE_SIZE].value);
    mcs_manager_notify (mcs_manager, CHANNEL);

    return;

}

static gboolean
transparency_changed (GtkRange * range, 
	GtkScrollType scroll, 
	gdouble value, 
	gpointer user_data)
{
    settings_t *settings_p=g_object_get_data(G_OBJECT(range), "settings_p");
    if(settings_p->disable_options)
        return FALSE;
    int i = (int)((long)user_data);
    if(rfm_options[i].value)
        g_free (rfm_options[i].value);
    rfm_options[i].value = g_strdup_printf ("%lf", value);
    mcs_manager_set_string (mcs_manager, rfm_options[i].name,
	    CHANNEL, rfm_options[i].value);
    mcs_manager_notify (mcs_manager, CHANNEL);

    return FALSE;
}

static gint
settings_monitor (gpointer data) {
    NOOP("g_timeout: settings-module.i settings_monitor()\n");
    mcs_shm_t *mcs_shm_p = (mcs_shm_t *) mp->m;
    if(shm_settings_serial < 0)
        return FALSE;
    if(shm_settings_serial != mcs_shm_p->serial) {
	NOOP(stderr, "+++ settings_monitor: refreshing environment...\n");
        int i;
        for(i = 0; i < RFM_OPTIONS; i++) {
            // false positive. We are not comparing the array, we are comparing
            // the preset value within the array of structures, which may or may
            // not be null.
            // coverity[array_null : FALSE]
            if(mcs_shm_p->data[i].value) {
                g_free (rfm_options[i].value);
                rfm_options[i].value = g_strdup (mcs_shm_p->data[i].value);
                rfm_setenv (rfm_options[i].name, rfm_options[i].value, TRUE);
            } else {
                rfm_setenv (rfm_options[i].name, NULL, TRUE);
            }
	    if (strlen(rfm_options[i].value)){
		NOOP ("settings_monitor(): serial %d (was %d)  %s->%s\n",
		    mcs_shm_p->serial, shm_settings_serial, 
		    rfm_options[i].name, rfm_options[i].value);
	    }
	
        }
        shm_settings_serial = mcs_shm_p->serial;
    } else {
        COMMENT("shm_settings_serial OK\n"); 
    }
    return TRUE;
}

static int
mcs_shm_fileread (void) {
    static gchar *rcfile = NULL;

    if(!rcfile) {
        rcfile = g_build_filename (MCS_SHM_PLUGIN_FILE, NULL);
    }

    if(rfm_g_file_test (rcfile, G_FILE_TEST_EXISTS)) {
        FILE *f;
        NOOP("mcs_shm_fileread(): %s\n", rcfile);
        f = fopen (rcfile, "r");
        if(f) {
            size_t items_read = fread (mp->m, sizeof (mcs_shm_t), 1, f);
            if(items_read == 1){
                msync (mp->m, sizeof (mcs_shm_t), MS_SYNC);
            } else {
                DBG ("mcs_shm_fileread() fread: %s\n", strerror (errno));
            }
            fclose (f);
        }
        return 1;
    } 
    return 0;
}

static int
mcs_shm_filewrite (void) {
    FILE *f;
    static gchar *rcfile = NULL;

    if(!rcfile) {
        rcfile = g_build_filename (MCS_SHM_PLUGIN_FILE, NULL);
    }
    NOOP ("mcs_shm_filewrite(): %s\n", rcfile);
    msync (mp->m, sizeof (mcs_shm_t), MS_SYNC);
    f = fopen (rcfile, "w");
    if(f) {
        mcs_shm_t *mcs_shm_p = (mcs_shm_t *) mp->m;
        mcs_shm_p->serial++;
        if(fwrite (mp->m, sizeof (mcs_shm_t), 1, f) < 1)
            DBG ("fwrite: %s\n", strerror (errno));
        fclose (f);
        return 1;
    }
    msync (mp->m, sizeof (mcs_shm_t), MS_SYNC);
    return 0;
}

static void
mcs_shm_bringforth (void) {
    int i,
      j;
    mcs_shm_t *mcs_shm_p = (mcs_shm_t *) mp->m;
    COMMENT ("mcs_shm_bringforth\n");
    for(i = 0; i < RFM_OPTIONS; i++) {
        g_free (rfm_options[i].value);
        rfm_options[i].value = g_strdup ("");
    }
    for(i = 0; i < RFM_OPTIONS; i++){
        for(j = 0; j < RFM_OPTIONS; j++) {
            // false positive. We are not comparing the array, we are comparing
            // the preset value within the array of structures, which may or may
            // not be null.
            // coverity[array_null : FALSE]
            if(rfm_options[i].name && mcs_shm_p->data[j].name) {
                if(strcmp (rfm_options[i].name, mcs_shm_p->data[j].name))
                    continue;
                g_free (rfm_options[i].value);
		    rfm_options[i].value = g_strdup (mcs_shm_p->data[j].value);
		if (strlen(rfm_options[i].value)) {
		    NOOP ("mcs_shm_bringforth:  %s->%s\n", 
			    rfm_options[i].name, rfm_options[i].value);
		}
                break;
            }
        }
    }
}

static void
mcs_shm_init (void) {
    int i;
    mcs_shm_t *mcs_shm_p = (mcs_shm_t *) mp->m;
    COMMENT ("mcs_shm_init\n");
    for(i = 0; i < RFM_OPTIONS; i++) {
        memset (mcs_shm_p->data[i].name, 0, SHM_STRING_LENGTH);
        memset (mcs_shm_p->data[i].value, 0, SHM_VALUE_LENGTH);
        strncpy (mcs_shm_p->data[i].name, rfm_options[i].name, SHM_STRING_LENGTH-1);
        if(rfm_options[i].value) {
            strncpy (mcs_shm_p->data[i].value, rfm_options[i].value, SHM_VALUE_LENGTH-1);
            mcs_shm_p->data[i].value[SHM_VALUE_LENGTH - 1] = 0;
        }
        COMMENT ("*SETTINGS: initialising %s->%s\n",
		rfm_options[i].name, rfm_options[i].value);
    }
    msync (mp->m, sizeof (mcs_shm_t), MS_SYNC);
}

static void
start_desktop (gboolean on) {
    // Automatic on/off is only applicable for local displays.
    if (!localhost_check()) {
	return;
    }

    Window xid;
    GError *error = NULL;
    if(on) {
        rfm_global_t *rfm_global_p = rfm_global();
        Atom selection_atom = XInternAtom (rfm_global_p->Xdisplay, "RODENT_DESK_ATOM", False);
        if((xid = XGetSelectionOwner (rfm_global_p->Xdisplay, selection_atom))) {
            NOOP ("start_desktop(): rodent-desk already running\n");
        } else if(!g_spawn_command_line_async ("rodent-desk", &error)) {
            DBG ("start_desktop(): %s\n", error->message);
        } else {
	    NOOP("start_desktop(): rodent-desk\n");
	}
    } else {
	 if(!g_spawn_command_line_async ("killall -TERM rodent-desk", &error)) {
            DBG ("%s\n", error->message);
            g_error_free (error);
        }
    }

}

static void
mcs_manager_set_string (McsManager * mcs_m, 
	const gchar * setting_name, 
	const gchar * channel, 
	const gchar * setting_value) 
{
    mcs_shm_t *mcs_shm_p = (mcs_shm_t *) mp->m;
    int i;
    COMMENT ("mcs_manager_set_string\n");

    for(i = 0; i < RFM_OPTIONS; i++) {
        COMMENT ("%d/%d %s==%s?\n",
		i+1, RFM_OPTIONS, setting_name, mcs_shm_p->data[i].name);
        if(strcmp (setting_name, mcs_shm_p->data[i].name) == 0) {
            memset (mcs_shm_p->data[i].value, 0, SHM_VALUE_LENGTH);
            if(setting_value) {
              if(strlen (setting_value) > 0) {
		NOOP (stderr, "mcs_manager_set_string(): %s -> %s (%zd)\n",
		    setting_name, setting_value, strlen (setting_value));
	      }
              if(i == RFM_ENABLE_DESKTOP) {
                if(strlen (setting_value) > 0) {
                    NOOP (stderr, "mcs_manager_set_string(): Start desktop (%ld)\n",
			    (long)strlen (setting_value));
                    start_desktop (TRUE);
                } else {
                    NOOP (stderr, "mcs_manager_set_string(): Kill desktop (%ld)\n", 
			    (long)strlen (setting_value));
                    start_desktop (FALSE);
                }
              }
#ifdef HAVE_SETENV
        setenv ( mcs_shm_p->data[i].name, setting_value, 1);
#endif
              strncpy (mcs_shm_p->data[i].value, setting_value, SHM_VALUE_LENGTH);
              mcs_shm_p->data[i].value[SHM_VALUE_LENGTH - 1] = 0;
	      gchar *oldvalue = rfm_options[i].value;
              rfm_options[i].value = g_strdup (setting_value);
              g_free (oldvalue);
            }
        }
    }
    msync (mp->m, sizeof (mcs_shm_t), MS_SYNC);
    /* here we update the shmblock */
    return;
}

static void
mcs_manager_notify (McsManager * mcs_m, const gchar * channel) {
    /* here we ++ the shmblock serial (or something), this should be
     * monitored by other instances so that changes are ACK.*/
    COMMENT ("mcs_manager_notify\n");
    mcs_shm_t *mcs_shm_p = (mcs_shm_t *) mp->m;
    mcs_shm_p->serial++;
    msync (mp->m, sizeof (mcs_shm_t), MS_SYNC);
    mcs_shm_filewrite ();
    return;
}

/* the dialog ******************************************/
/*static void
add_spacer (GtkBox * box) {

    GtkWidget *eventbox = gtk_alignment_new (0, 0, 0, 0);

    gtk_widget_set_size_request (eventbox, SKIP, SKIP);
    gtk_widget_show (eventbox);
    gtk_box_pack_start (box, eventbox, FALSE, TRUE, 0);
}*/

static void update_tree (GtkTreeModel *model, gint id);

static gboolean
test_command(const gchar *command){
    if (!command) return FALSE;
    gchar *c=g_strdup(command);
    if (strchr(c, ' ')) *strchr(c, ' ') = 0;
    gchar *cmd = g_find_program_in_path (c);
    g_free(c);
    if (!cmd) return FALSE;
    g_free(cmd);
    return TRUE;
}


static gchar **get_program_options(gint i){
    if (i >= RFM_OPTIONS) return NULL;
    environ_t *environ_v = rfm_get_environ();
    return environ_v[i].env_options;
}

static gboolean
environment_changed (GtkCellRendererText * cell, 
	const gchar * path_string, 
	const gchar * new_text, 
	gpointer data) {
    settings_t *settings_p=data;
    if(settings_p->disable_options)
        return FALSE;

    GtkTreeModel *model = GTK_TREE_MODEL(settings_p->model);
    GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
    GtkTreeIter iter;

    gint *column;

    column = g_object_get_data (G_OBJECT (cell), "column");

    gtk_tree_model_get_iter (model, &iter, path);

    if(GPOINTER_TO_INT (column) == COLUMN_VALUE) {
        COMMENT ("column=%d path=%s newtext=%s",
		GPOINTER_TO_INT (column), path_string, new_text);

        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, column, &old_text, -1);
        g_free (old_text);
        old_text = NULL;
        i = gtk_tree_path_get_indices (path)[0];

        if(new_text && strlen (new_text) >= SHM_VALUE_LENGTH) {
            DBG ("strlen($%s) <  %d not met. Ignoring it.\n", rfm_options[i].name, SHM_VALUE_LENGTH);
            gtk_list_store_set (GTK_LIST_STORE (model), &iter, column, rfm_options[i].value, -1);
            return FALSE;
        }
	// bitflag tests 
	gchar **aa=get_program_options(i);

	if (aa) {
	    gboolean notfound = TRUE;
	    for(; aa && *aa; aa++){
		if (new_text && strcmp(*aa, new_text)==0){
		    notfound = FALSE;
		    break;
		}
	    }
	    if (notfound){
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, column, rfm_options[i].value, -1);
                return FALSE;
	    }
	}

        /* test for valid commands */
        if(i == TERMINAL_CMD || i == EDITOR) {
            gboolean dump = FALSE;
            if(!new_text || !test_command(new_text)){
                    dump = TRUE;
            }
            if(dump) {
		
		gchar *g=g_strdup_printf("%s: Command not found", (new_text)?new_text:"");
		rfm_show_text(settings_p->widgets_p);
		rfm_diagnostics(settings_p->widgets_p, "xffm/stock_dialog-error", NULL);
		rfm_diagnostics(settings_p->widgets_p, "xffm_tag/stderr", g, "\n", NULL);
		g_free(g);
                gtk_list_store_set (GTK_LIST_STORE (model), &iter, column, rfm_options[i].value, -1);
                return FALSE;
            }
        }
        /* test for valid iconsizes */
        if(i == RFM_DEFAULT_ICON_SIZE) {
	    // New values entered in advanced mode: should they be
	    // input in translated or nontranslated form?
	    // Non translated, of course.
            gboolean dump = TRUE;
            if(new_text) {
		const gchar **p=rfm_get_icon_sizes();
		for (;p && *p; p++){
		    if (strcmp(*p, new_text) == 0){
			dump = FALSE; 
			break;
		    }
		}
            }
            if(dump) {
                gtk_list_store_set (GTK_LIST_STORE (model),
			&iter, column, rfm_options[i].value, -1);
                return FALSE;
            }
        }
        if(i == RFM_FIXED_FONT_SIZE
		|| i == RFM_VARIABLE_FONT_SIZE
		|| i == RFM_FIXED_FONT_FAMILY
		|| i == RFM_VARIABLE_FONT_FAMILY
		) {
	    // New values entered in advanced mode: should they be
	    // input in translated or nontranslated form?
	    // Non translated, of course.
            gboolean dump = TRUE;
            if(new_text) {
		const gchar **p=rfm_get_font_sizes();
		for (;p && *p; p++){
		    if (strcmp(*p, new_text) == 0){
			dump = FALSE; 
			break;
		    }
		}
            }
            if(dump) {
                gtk_list_store_set (GTK_LIST_STORE (model),
			&iter, column, rfm_options[i].value, -1);
                return FALSE;
            }
        }

        if(rfm_options[i].value) {
            g_free (rfm_options[i].value);
            rfm_options[i].value = NULL;
        }
        if(new_text && strlen (new_text)) {
            rfm_options[i].value = g_strdup (new_text);
        } else {
            rfm_options[i].value = g_strdup ("");
        }
	COMMENT ("environment_changed:  %s->%s\n",
		rfm_options[i].name, rfm_options[i].value);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column, rfm_options[i].value, -1);
        mcs_manager_set_string (mcs_manager, rfm_options[i].name, CHANNEL, rfm_options[i].value);
        mcs_manager_notify (mcs_manager, CHANNEL);
        /*write_options(); */
    }
    gtk_tree_path_free (path);

    return FALSE;
}
static gboolean animation_stop=FALSE;
/* the dialog */
static void
dialog_delete (GtkWidget * dialog, gpointer data) {
    GError *error = NULL;
    if(GPOINTER_TO_INT(data) == 1) {
	if (rfm_void(RFM_MODULE_DIR, "icons", "module_active") ){
	    g_spawn_command_line_async ("rodent-iconmgr", &error);
	} else {
	gchar *text = g_strdup_printf("<b>%s</b>\n\n%s: <i>%s</i>\n",
		_("List of disabled plugins"), _("Icon Themes"), _("Disabled"));
	    rfm_confirm(NULL, GTK_MESSAGE_INFO, text, NULL, NULL);
	    g_free(text);
	    return;
	}
    } else if(GPOINTER_TO_INT(data) == 2) {
          /* thumbnail cache */
          gchar *cache_dir = g_build_filename (RFM_THUMBNAIL_DIR, NULL);
          NOOP ("dialog_delete(): clearing thumbnail cache: %s", cache_dir);
          gchar *command = g_strdup_printf ("rm -rf \"%s\"", cache_dir);
          g_spawn_command_line_async (command, &error);
          g_free (cache_dir);
          g_free (command);
    }
    
    // writeout current config, to disable welcome dialog.
    mcs_shm_filewrite();
    

    widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
    if (widgets_p) {
	g_free(widgets_p);
    }
    g_object_set_data(G_OBJECT(dialog), "widgets_p", NULL);
    gtk_widget_hide(dialog);
    animation_stop = TRUE;
    settings_t *settings_p=g_object_get_data(G_OBJECT(dialog), "settings_p");
    g_free(settings_p);
    g_object_set_data(G_OBJECT(dialog), "settings_p", NULL);
    settings_dialog=NULL;
    gtk_widget_destroy (dialog);
    //
    //rfm_unload_module("xmltree");
    
}

static void
update_bitflag_entry (settings_t *settings_p, gint id) {
    GtkEntry *entry =
	    g_object_get_data(G_OBJECT(settings_p->dialog), rfm_options[id].name);
    gchar *new_text = rfm_options[id].value;
    if (GTK_IS_ENTRY(entry)) gtk_entry_set_text(GTK_ENTRY(entry), new_text?new_text:"");
}

static void
update_bitflag_combo (settings_t *settings_p, gint id) {
    gchar **aa=get_program_options(id);
    gint item=0;
    if (aa) {
	gchar *new_text = rfm_options[id].value;
	gboolean notfound = TRUE;
	for(; aa && *aa; aa++,item++){
	    if (strcmp(*aa, new_text)==0){
		notfound = FALSE;
		break;
	    }
	}
	if (notfound) {
	    NOOP("aa not found %s\n", new_text);
	    return ;
	}
	GtkComboBox *combo_box =
	    g_object_get_data(G_OBJECT(settings_p->dialog), rfm_options[id].name);
	    
	NOOP(stderr, "update_bitflag_combo() %s box=0x%x\n", rfm_options[id].name, GPOINTER_TO_INT(combo_box));
	if (GTK_IS_COMBO_BOX(combo_box)) gtk_combo_box_set_active (combo_box, item);

    } else {
	NOOP("update_bitflag_combo(): get_program_options(%s) is NULL\n", rfm_options[id].name);
    }

}

static void
update_combo (GtkComboBox * combo_box, gint id, gboolean translated) {
    const gchar *string=(translated)?_(rfm_options[id].value):rfm_options[id].value;
    gint item=0;

    GSList *list=g_object_get_data(G_OBJECT(combo_box), "list");
    GSList *tmp=list;
    for (;tmp && tmp->data; tmp=tmp->next, item++){
	if (strcmp(string, (gchar *)tmp->data)==0) {
	    break;
	}
    }

    // Is the string in the combobox list?
    gboolean in_combobox = (item < g_slist_length(list));
	// Yes. Then it must be selected.
    if (in_combobox) {
	if (GTK_IS_COMBO_BOX(combo_box))gtk_combo_box_set_active (combo_box, item);
    } else {
	// No. The string is not in the combobox list, the string must
	// be inserted and selected.
	COMMENT("Adding %s\n",  string);
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
	gtk_combo_box_insert_text (combo_box, 0, string);
#else
	gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT(combo_box), 0, string);
#endif
	if (GTK_IS_COMBO_BOX(combo_box))gtk_combo_box_set_active (combo_box, 0);
    }
}

static void
update_tree (GtkTreeModel *model, gint id) {
    GtkTreePath *treepath = gtk_tree_path_new_from_indices (id, -1);
    GtkTreeIter iter;
    if(gtk_tree_model_get_iter (model, &iter, treepath)) {
        gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		COLUMN_VALUE, g_strdup (rfm_options[id].value), 
		COLUMN_EDITABLE, TRUE, -1);
    }
    gtk_tree_path_free (treepath);
}

gdouble get_spin_value(gint i){
    gdouble value=0;
    environ_t *environ_v = rfm_get_environ();
    if (getenv(environ_v[i].env_var) && strlen(getenv(environ_v[i].env_var))) {
	value = atof(getenv(environ_v[i].env_var));
	COMMENT("got env_var=%s\n", getenv(environ_v[i].env_var));
    } else if (environ_v[i].env_string) {
	value = atof(environ_v[i].env_string);
	COMMENT("got env_string=%s\n", environ_v[i].env_string);
    }
    return value;
}

static void
set_bit_toggles( settings_t *settings_p, const gchar *boxname,  RfmProgramOptions *options_p, gint option_flag) {
    GObject *box = g_object_get_data(G_OBJECT(settings_p->dialog), boxname);
    if (!box || !G_IS_OBJECT (box)) {
        DBG("Not an object: %s\n", boxname);
        return;
    }
    errno = 0;
    gint64 flag = strtoll(rfm_options[option_flag].value, NULL, 16);
    if (errno) flag = 0;
    gint i;
    for (i=0;options_p && options_p->option; options_p++, i++){
	if (strcmp(options_p->option,"submodule-indent")==0) continue;
	if (strcmp(options_p->option,"submodule-unindent")==0) continue;
	if (strcmp(options_p->option,"submodule-label")==0) continue;

	const gchar *name = options_p->option;
	GtkWidget *button = g_object_get_data(box, name);
	if (button){
	    gint64 k = flag & (ONE64<<i);
	    rfm_check_button_set_active(button, k?TRUE:FALSE);
	} else {
	    DBG("No toggle button for %s\n", name);
	}
    }

}

static void
update_combo_entry(settings_t *settings_p) {
#ifdef GNU_CP
    update_bitflag_combo (settings_p, RFM_CP_backup);
    update_bitflag_combo (settings_p, RFM_CP_preserve);
    update_bitflag_combo (settings_p, RFM_CP_no_preserve);
    update_bitflag_combo (settings_p, RFM_CP_reflink);
    update_bitflag_combo (settings_p, RFM_CP_sparse);
    update_bitflag_combo (settings_p, RFM_CP_suffix);
#endif
#ifdef GNU_MV
    update_bitflag_combo (settings_p, RFM_MV_backup);
    update_bitflag_combo (settings_p, RFM_MV_suffix);
#endif

#ifdef GNU_LN
    update_bitflag_combo (settings_p, RFM_LN_backup);
    update_bitflag_combo (settings_p, RFM_LN_suffix);
#endif

#ifdef GNU_RM
    update_bitflag_combo (settings_p, RFM_RM_interactive);
#endif

#ifdef GNU_LS
    update_bitflag_entry (settings_p, RFM_LS_ignore);
    update_bitflag_entry (settings_p, RFM_LS_tabsize);
    update_bitflag_entry (settings_p, RFM_LS_blocksize);
    update_bitflag_combo (settings_p, RFM_LS_format);
    update_bitflag_entry (settings_p, RFM_LS_hide);
    update_bitflag_combo (settings_p, RFM_LS_istyle);
    update_bitflag_combo (settings_p, RFM_LS_qstyle);
    update_bitflag_combo (settings_p, RFM_LS_sort);
    update_bitflag_combo (settings_p, RFM_LS_time);
    update_bitflag_combo (settings_p, RFM_LS_tstyle);
    update_bitflag_entry (settings_p, RFM_LS_width);
#endif

    gchar *shred_bin = g_find_program_in_path("shred");
    if (shred_bin) {
	update_bitflag_combo (settings_p, RFM_SHRED_iterations);
	update_bitflag_combo (settings_p, RFM_SHRED_size);
	g_free(shred_bin);
    }

}

GtkWidget *toggle_button[RFM_OPTIONS];
static void
set_option_buttons ( settings_t *settings_p) {

    int i;
    settings_p->disable_options = TRUE;

    set_bit_toggles(settings_p, "ls_box", LS_OPTIONS,RFM_LS_FLAGS);
    set_bit_toggles(settings_p, "cp_box", CP_OPTIONS, RFM_CP_FLAGS);
    set_bit_toggles(settings_p, "mv_box", MV_OPTIONS, RFM_MV_FLAGS);
    set_bit_toggles(settings_p, "ln_box", LN_OPTIONS, RFM_LN_FLAGS);
    set_bit_toggles(settings_p, "rm_box", RM_OPTIONS, RFM_RM_FLAGS);
    gchar *shred_bin = g_find_program_in_path("shred");
    if (shred_bin) {
	set_bit_toggles(settings_p, "shred_box", SHRED_OPTIONS, RFM_SHRED_FLAGS);
	g_free(shred_bin);
    }

    set_bit_toggles(settings_p, "plugins_box", rfm_get_lite_plugin_options(), RFM_PLUGIN_FLAGS);
    set_bit_toggles(settings_p, "modules_box", rfm_get_lite_module_options(), RFM_MODULE_FLAGS);

    update_combo_entry(settings_p);

    // spin buttons:
    for (i = RFM_DESKTOP_TOP_MARGIN; i <= RFM_DESKTOP_LEFT_MARGIN; i++){
	gdouble value = get_spin_value(i);
	gtk_spin_button_set_value (
		GTK_SPIN_BUTTON(settings_p->desktop_margin_spinbutton[i-RFM_DESKTOP_TOP_MARGIN]),
                value);
	update_tree (GTK_TREE_MODEL(settings_p->model), i);
    }
    {
	gdouble value = rfm_get_preview_image_size();
	gtk_spin_button_set_value (
		GTK_SPIN_BUTTON(settings_p->preview_size_spinbutton),
                value);
	update_tree (GTK_TREE_MODEL(settings_p->model), RFM_PREVIEW_IMAGE_SIZE);
    }
    for(i = 0; i < RFM_OPTIONS; i++)
        if(toggle_button[i]) {
            if(rfm_options[i].value && strlen (rfm_options[i].value)) {
                gtk_toggle_button_set_active ((GtkToggleButton *)
                                              toggle_button[i], TRUE);
            } else {
                gtk_toggle_button_set_active ((GtkToggleButton *)
                                              toggle_button[i], FALSE);
            }
            GtkTreePath *treepath = gtk_tree_path_new_from_indices (i, -1);
            GtkTreeIter iter;
            if(gtk_tree_model_get_iter (
			GTK_TREE_MODEL(settings_p->model), &iter, treepath))
	    {
                gtk_list_store_set (settings_p->model, &iter, 
			COLUMN_VALUE, g_strdup (rfm_options[i].value), 
			COLUMN_EDITABLE, TRUE, -1);
            }
            gtk_tree_path_free (treepath);
        }
    update_combo (GTK_COMBO_BOX(settings_p->fontsize_box), RFM_FIXED_FONT_SIZE, TRUE);
    update_combo (GTK_COMBO_BOX(settings_p->vfontsize_box), RFM_VARIABLE_FONT_SIZE, TRUE);
    update_combo (GTK_COMBO_BOX(settings_p->fontfamily_box), RFM_FIXED_FONT_FAMILY, TRUE);
    update_combo (GTK_COMBO_BOX(settings_p->vfontfamily_box), RFM_VARIABLE_FONT_FAMILY, TRUE);
    update_combo (GTK_COMBO_BOX(settings_p->iconsize_box), RFM_DEFAULT_ICON_SIZE, TRUE);
    update_combo (GTK_COMBO_BOX(settings_p->terminal_box), TERMINAL_CMD, FALSE);
    update_combo (GTK_COMBO_BOX(settings_p->editor_box), EDITOR, FALSE);
    if(getenv ("RFM_DESKTOP_DIR") && strlen (getenv ("RFM_DESKTOP_DIR"))) {
	COMMENT("set_option_buttons: setting %s\n",getenv ("RFM_DESKTOP_DIR"));
        gtk_entry_set_text (GTK_ENTRY (settings_p->desktopdir_entry), 
		getenv ("RFM_DESKTOP_DIR"));
    }
    if(getenv ("RFM_DESKTOP_IMAGE") && strlen (getenv ("RFM_DESKTOP_IMAGE"))) {
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (settings_p->desktopimage_button),
		getenv ("RFM_DESKTOP_IMAGE"));
    } 
    if(getenv ("RFM_DESKTOP_COLOR") && strlen (getenv ("RFM_DESKTOP_COLOR"))) {
#if GTK_MAJOR_VERSION==3 && GTK_MINOR_VERSION>=4
        GdkRGBA color;
        if(gdk_rgba_parse (&color, getenv ("RFM_DESKTOP_COLOR") )) {
	    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(settings_p->desktopcolor_button), &color);
        }

#else
        GdkColor color;
        if(gdk_color_parse (getenv ("RFM_DESKTOP_COLOR"), &color)) {
            gtk_color_button_set_color (
		    GTK_COLOR_BUTTON(settings_p->desktopcolor_button), &color);
        }
#endif
    }
    if(getenv ("RFM_ICONVIEW_COLOR") && strlen (getenv ("RFM_ICONVIEW_COLOR")))
    {
#if GTK_MAJOR_VERSION==3 && GTK_MINOR_VERSION>=4
        GdkRGBA color;
        if(gdk_rgba_parse (&color, getenv ("RFM_ICONVIEW_COLOR") )) {
	    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(settings_p->iconviewcolor_button), &color);
        }

#else
        GdkColor color;
        if(gdk_color_parse (getenv ("RFM_ICONVIEW_COLOR"), &color)) {
            gtk_color_button_set_color (
		    GTK_COLOR_BUTTON(settings_p->desktopcolor_button), &color);
        }
#endif
    }
    update_tree (GTK_TREE_MODEL(settings_p->model), VERSION_CONTROL);

#ifdef GNU_CP

    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_FLAGS);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_CP_FLAGS);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_MV_FLAGS);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LN_FLAGS);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_RM_FLAGS);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_SHRED_FLAGS);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_PLUGIN_FLAGS);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_MODULE_FLAGS);

    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_CP_backup);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_CP_preserve);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_CP_no_preserve);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_CP_reflink);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_CP_sparse);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_CP_suffix);
#endif

#ifdef GNU_MV
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_MV_backup);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_MV_suffix);
#endif

#ifdef GNU_LN
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LN_backup);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LN_suffix);
#endif

#ifdef GNU_RM
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_RM_interactive);
#endif

#ifdef GNU_LS
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_ignore);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_tabsize);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_blocksize);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_format);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_hide);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_istyle);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_qstyle);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_sort);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_time);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_tstyle);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_LS_width);
#endif

    shred_bin = g_find_program_in_path("shred");
    if (shred_bin) {
	update_tree (GTK_TREE_MODEL(settings_p->model), RFM_SHRED_iterations);
	update_tree (GTK_TREE_MODEL(settings_p->model), RFM_SHRED_size);
	g_free(shred_bin);
    }

    
    
    update_tree (GTK_TREE_MODEL(settings_p->model),RFM_FIXED_FONT_SIZE );
    update_tree (GTK_TREE_MODEL(settings_p->model),RFM_VARIABLE_FONT_SIZE );
    update_tree (GTK_TREE_MODEL(settings_p->model),RFM_FIXED_FONT_FAMILY );
    update_tree (GTK_TREE_MODEL(settings_p->model),RFM_VARIABLE_FONT_FAMILY );
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_DEFAULT_ICON_SIZE);
    update_tree (GTK_TREE_MODEL(settings_p->model), TERMINAL_CMD);
    update_tree (GTK_TREE_MODEL(settings_p->model), EDITOR);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_DESKTOP_DIR);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_DESKTOP_IMAGE);
    update_tree (GTK_TREE_MODEL(settings_p->model), RFM_DESKTOP_COLOR);



    settings_p->disable_options = FALSE;

}



static void
option_toggled (GtkToggleButton * togglebutton, gpointer user_data) {
    NOOP(stderr, "at option_toggled...\n");
    settings_t *settings_p=g_object_get_data(G_OBJECT(togglebutton), "settings_p");
    if(settings_p->disable_options) return;
    gint i = GPOINTER_TO_INT(user_data);
    NOOP(stderr, "at option_toggled...%d\n", i);

    if(rfm_options[i].value) {
        g_free (rfm_options[i].value);
    }

    if (i==RFM_USE_GTK_ICON_THEME) {
        GtkSettings *settings = gtk_settings_get_default();
         g_object_get( G_OBJECT(settings), 
                "gtk-icon-theme-name", &(rfm_options[i].value),
                NULL); 
         rfm_show_text(settings_p->widgets_p);
        rfm_diagnostics(settings_p->widgets_p, "xffm/stock_dialog-info", NULL);
        rfm_diagnostics(settings_p->widgets_p, "xffm_tag/stderr", _("Please restart application for full changes to take effect"), ".\n ", NULL);
    }
   
    if(gtk_toggle_button_get_active (togglebutton)) {
	if (i==RFM_USE_GTK_ICON_THEME) {
	    GtkSettings *settings = gtk_settings_get_default();
	     g_object_get( G_OBJECT(settings), 
		    "gtk-icon-theme-name", &(rfm_options[i].value),
		    NULL); 
            rfm_diagnostics(settings_p->widgets_p, "xffm/stock_dialog-info", NULL);
            rfm_diagnostics(settings_p->widgets_p, "xffm_tag/stderr", _("Please restart application for full changes to take effect"), ".\n ", NULL);
#if 0
	     // First we zap the thumbnail directory 
	     // (composite icons are thumbnailed cached)
	     gchar *thumbnails = g_build_path(RFM_THUMBNAIL_DIR,NULL);
	     pid_t pid = fork();
	     if (!pid){
		 gchar *argv[]={"rm", "-rf", thumbnails, NULL};
		 execvp(argv[0], argv);
		 _exit(123);
	     }
	     gint status;
	     waitpid(pid, &status, 0);
#endif
	     // This guy should regenerate the cache...
             // But is already done in rodent-fm
	     // rfm_void(RFM_MODULE_DIR,"icons", "create_cache");
	}
	else {
	    rfm_options[i].value = g_strdup ("yes");
	}
    } else {
        rfm_options[i].value = g_strdup ("");
    }
    mcs_manager_set_string (mcs_manager, rfm_options[i].name, 
	    CHANNEL, rfm_options[i].value);
    mcs_manager_notify (mcs_manager, CHANNEL);
    /*write_options(); */

}

static void bit_option_toggled (GtkToggleButton * togglebutton, 
#if GTK_MAJOR_VERSION>=3
gpointer *something_here,  
#endif
				gpointer data) {
    settings_t *settings_p=g_object_get_data(G_OBJECT(togglebutton), "settings_p");
    if(settings_p->disable_options) return;
     
    NOOP("notify:active signal...\n");
 
    gint i;
    gint64 new_value = 0;
    const gchar *box_name = data;

    GObject *box = g_object_get_data(G_OBJECT(settings_p->dialog), box_name);
    RfmProgramOptions *options_p = g_object_get_data(box, "options_p");
    for (i=0;options_p && options_p->option; options_p++,i++){
   	const gchar *name = options_p->option;
	GtkWidget *button = g_object_get_data(box, name);
	if (button) {
	    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) new_value |= (ONE64<<i);
	}
    }
    i = GPOINTER_TO_INT(g_object_get_data(box, "var_name"));
    if(rfm_options[i].value) {
        g_free (rfm_options[i].value);
    }
    rfm_options[i].value = g_strdup_printf ("0x%llx", (long long)new_value);
    NOOP(stderr, "bitflag toggle %s\n", rfm_options[i].value);
    mcs_manager_set_string (mcs_manager, rfm_options[i].name, 
	    CHANNEL, rfm_options[i].value);
    mcs_manager_notify (mcs_manager, CHANNEL);
}

static void
switch_page (GtkNotebook * notebook, GtkWidget * page, guint page_num, gpointer user_data) {
    settings_t *settings_p=g_object_get_data(G_OBJECT(notebook), "settings_p");
    //    if (page_num < )
    {
        COMMENT("pagenum=%d",page_num);
        set_option_buttons (settings_p);

    }
}

static void
gint_changed (GtkComboBox * combo_box, gint id) {
    settings_t *settings_p=g_object_get_data(G_OBJECT(combo_box), "settings_p");
    if(settings_p->disable_options){
        return;
    }
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    gchar *gint_selected_translated = gtk_combo_box_get_active_text (combo_box);
#else
    gchar *gint_selected_translated = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(combo_box));
#endif
    if (!gint_selected_translated) {
	DBG("gint_changed(): this should never happen\n");
	return;
    }
    gchar *gint_selected=NULL;
    if (strcmp(_("Normal"), gint_selected_translated)==0){
	gint_selected=g_strdup("Normal");
    } else 
    if (strcmp(_("Compact"), gint_selected_translated)==0){
	gint_selected=g_strdup("Compact");
    } else 
    if (strcmp(_("Details"), gint_selected_translated)==0){
	gint_selected=g_strdup("Details");
    } else 
    if (strcmp(_("Big"), gint_selected_translated)==0){
	gint_selected=g_strdup("Big");
    } else 
    if (strcmp(_("Huge"), gint_selected_translated)==0){
	gint_selected=g_strdup("Huge");
    } else {
	gint_selected=g_strdup("");
    }

    if(rfm_options[id].value){
        g_free (rfm_options[id].value);
    }
    rfm_options[id].value = gint_selected;
    COMMENT ("gint_changed:  %s->%s\n", rfm_options[id].name, rfm_options[id].value);

    mcs_manager_set_string (mcs_manager, rfm_options[id].name, CHANNEL, gint_selected);
    mcs_manager_notify (mcs_manager, CHANNEL);
    rfm_show_text(settings_p->widgets_p);
    rfm_diagnostics(settings_p->widgets_p, "xffm/stock_dialog-info", NULL);
    rfm_diagnostics(settings_p->widgets_p, "xffm_tag/stderr", _("Please be patient"), ": ",
	    _("Reload All Tabs in All Windows"), ".  ", NULL);
    rfm_diagnostics(settings_p->widgets_p, "xffm_tag/blue", _("Default Size"), " --> ", gint_selected_translated, "\n",NULL);
    g_free(gint_selected_translated);
}

static void
path_changed (GtkComboBox * combo_box, gint id) {
    settings_t *settings_p=g_object_get_data(G_OBJECT(combo_box), "settings_p");
    if(settings_p->disable_options)
        return;
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    gchar *path_selected = gtk_combo_box_get_active_text (combo_box);
#else
    gchar *path_selected = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(combo_box));
#endif
    if(!test_command(path_selected)) {
        gchar *m = g_strdup_printf ("%s (%s): %s", strerror (ENOEXEC), path_selected,
                                    strerror (ENOENT));
        rfm_confirm (NULL, GTK_MESSAGE_ERROR, m, NULL, NULL);
        g_free (m);
        return;
    }

    if(rfm_options[id].value)
        g_free (rfm_options[id].value);
    rfm_options[id].value = path_selected;
    COMMENT ("path_changed:  %s->%s\n", rfm_options[id].name, rfm_options[id].value);

    mcs_manager_set_string (mcs_manager, rfm_options[id].name, CHANNEL, path_selected);
    mcs_manager_notify (mcs_manager, CHANNEL);
}

static void
terminal_changed (GtkComboBox * combo_box, gpointer user_data) {
    path_changed (combo_box, TERMINAL_CMD);
}

static void
editor_changed (GtkComboBox * combo_box, gpointer user_data) {
    path_changed (combo_box, EDITOR);
}

static void
 iconsize_changed(GtkComboBox * combo_box, gpointer user_data) {
     gint_changed (combo_box, RFM_DEFAULT_ICON_SIZE);
}

static void
 fontsize_f(GtkComboBox * combo_box, gpointer user_data, gint item) {
   settings_t *settings_p=g_object_get_data(G_OBJECT(combo_box), "settings_p");
    if(settings_p->disable_options){
        return;
    }
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    gchar *selected = gtk_combo_box_get_active_text (combo_box);
#else
    gchar *selected = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(combo_box));
#endif
    if (!selected) {
	DBG("fontsize_changed(): this should never happen\n");
	return;
    }
    if(rfm_options[item].value){
        g_free (rfm_options[item].value);
    }
    rfm_options[item].value = selected;
    COMMENT ("fontsize_f():  %s->%s\n", 
	    rfm_options[item].name,
	    rfm_options[item].value);

    mcs_manager_set_string (mcs_manager, 
	    rfm_options[item].name,
	    CHANNEL, selected);
    mcs_manager_notify (mcs_manager, CHANNEL);
}

static void
 fontfamily_changed(GtkComboBox * combo_box, gpointer user_data){
     fontsize_f(combo_box, user_data, RFM_FIXED_FONT_FAMILY);
 }
static void
 vfontfamily_changed(GtkComboBox * combo_box, gpointer user_data){
     fontsize_f(combo_box, user_data, RFM_VARIABLE_FONT_FAMILY);
 }
static void
 vfontsize_changed(GtkComboBox * combo_box, gpointer user_data) {
     fontsize_f(combo_box, user_data, RFM_VARIABLE_FONT_SIZE);
 }
static void
 fontsize_changed(GtkComboBox * combo_box, gpointer user_data) {
     fontsize_f(combo_box, user_data, RFM_FIXED_FONT_SIZE);
 }

static gboolean
entry_changed (GtkWidget *entry, GdkEvent  *event, gpointer data){
    const gchar *value = gtk_entry_get_text(GTK_ENTRY(entry));
    gint i = GPOINTER_TO_INT(data);
    if(rfm_options[i].value) g_free (rfm_options[i].value);

    rfm_options[i].value = g_strdup (value);

    mcs_manager_set_string (mcs_manager, rfm_options[i].name, CHANNEL, rfm_options[i].value);
    mcs_manager_notify (mcs_manager, CHANNEL);
                
    rfm_setenv (rfm_options[i].name, rfm_options[i].value, TRUE);

    return FALSE;
}

static void
combo_changed (GtkComboBox * combo_box, gpointer data) {
    settings_t *settings_p=g_object_get_data(G_OBJECT(combo_box), "settings_p");
    if(settings_p->disable_options) return;
    gint i = GPOINTER_TO_INT(data);
    if(rfm_options[i].value) g_free (rfm_options[i].value);

#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    rfm_options[i].value = 
	g_strdup (gtk_combo_box_get_active_text (combo_box));
#else
    rfm_options[i].value = 
	g_strdup (gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(combo_box)));
#endif

    mcs_manager_set_string (mcs_manager, rfm_options[i].name, CHANNEL, rfm_options[i].value);
    mcs_manager_notify (mcs_manager, CHANNEL);
    rfm_setenv (rfm_options[i].name, rfm_options[i].value, TRUE);
}

static void
deskdir_entry (GtkEntry * entry, gpointer user_data) {
    settings_t *settings_p=user_data;
    if(settings_p->disable_options) return;
    const gchar *new_value=gtk_entry_get_text(entry);
    if (!new_value) new_value="";
    if (strchr(new_value, '/') && new_value[strlen(new_value)-1]=='/') {
	gchar *v=g_strdup(new_value);
	*strrchr(v, '/')=0;
	gtk_entry_set_text(entry, v);
	g_free(v);
	new_value=gtk_entry_get_text(entry);
    }


    if (strcmp(new_value, rfm_options[RFM_DESKTOP_DIR].value)==0){
	return;
    }
    if(rfm_options[RFM_DESKTOP_DIR].value){
        g_free (rfm_options[RFM_DESKTOP_DIR].value);
    }
    rfm_options[RFM_DESKTOP_DIR].value = g_strdup(new_value);
    
    COMMENT ("2.file_set:  %s->%s\n", rfm_options[RFM_DESKTOP_DIR].name, rfm_options[RFM_DESKTOP_DIR].value);
    mcs_manager_set_string (mcs_manager, rfm_options[RFM_DESKTOP_DIR].name, 
	    CHANNEL, rfm_options[RFM_DESKTOP_DIR].value);
    mcs_manager_notify (mcs_manager, CHANNEL);
}

static void
deskdir_filechooser (GtkButton * button, gpointer user_data) {
    settings_t *settings_p = user_data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new (
	    _("Select Folder"), 
	    NULL, //parent
            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            _("Cancel"),
            GTK_RESPONSE_CANCEL,
            _("Open"),
            GTK_RESPONSE_ACCEPT,
            NULL);
    gtk_file_chooser_set_use_preview_label(GTK_FILE_CHOOSER (dialog), FALSE);
    gtk_file_chooser_set_preview_widget_active (GTK_FILE_CHOOSER (dialog), FALSE);
    // XXX gtk_file_chooser_set_show_hidden isn't working. Gtk bug or what?
    gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER (dialog), TRUE);
    g_object_set(G_OBJECT(dialog), "show-hidden", TRUE, NULL);

    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), rfm_options[RFM_DESKTOP_DIR].value);

    gint     response = rfm_dialog_run_response(dialog);

    if(response == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        COMMENT ("Got %s\n", filename);
	// if entry changed, set entry
	gtk_entry_set_text(GTK_ENTRY(settings_p->desktopdir_entry), filename);
	// entry activate callback, do your thing
	deskdir_entry (GTK_ENTRY(settings_p->desktopdir_entry), settings_p);
        g_free (filename);
    } 	
    gtk_widget_destroy (dialog);

}

static void
file_set (GtkFileChooserButton * chooser, gpointer user_data) {
    settings_t *settings_p=g_object_get_data(G_OBJECT(chooser), "settings_p");
    if(settings_p->disable_options)
        return;
    gint i = GPOINTER_TO_INT(user_data);
    gchar *g=gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(chooser));
    COMMENT ("1.file_set:  %s->%s (got: %s)\n", rfm_options[i].name, rfm_options[i].value, g);

      
    if(rfm_options[i].value){
        g_free (rfm_options[i].value);
    }
    rfm_options[i].value = g;
    
    COMMENT ("2.file_set:  %s->%s\n", rfm_options[i].name, rfm_options[i].value);
    mcs_manager_set_string (mcs_manager, rfm_options[i].name, 
	    CHANNEL, rfm_options[i].value);
    mcs_manager_notify (mcs_manager, CHANNEL);
}

static void
value_clear(GtkButton * button, gpointer user_data){
    gint i=GPOINTER_TO_INT(user_data);
    if(rfm_options[i].value){
        g_free (rfm_options[i].value);
    }
    rfm_options[i].value = g_strdup("");
    
    COMMENT ("bg_image_clear:  %s->%s\n", rfm_options[i].name, rfm_options[i].value);
    mcs_manager_set_string (mcs_manager, rfm_options[i].name, 
	    CHANNEL, rfm_options[i].value);
    mcs_manager_notify (mcs_manager, CHANNEL);

}

static void
color_changed (GtkColorButton * chooser, gpointer user_data) {
    settings_t *settings_p=g_object_get_data(G_OBJECT(chooser), "settings_p");
    if(settings_p->disable_options)
        return;
    int i = (int)((long)user_data);
    if(rfm_options[i].value)
        g_free (rfm_options[i].value);
#if GTK_MAJOR_VERSION==3
    GdkRGBA rgba;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(chooser), &rgba);
    int red = 65535 * rgba.red;
    int green = 65535 * rgba.green;
    int blue = 65535 * rgba.blue;
    rfm_options[i].value = g_strdup_printf("#%04x%04x%04x",
            red, green, blue);
#else
    GdkColor color;
    gtk_color_button_get_color ((GtkColorButton *) chooser, &color);
    rfm_options[i].value = gdk_color_to_string (&color);
#endif
    mcs_manager_set_string (mcs_manager, rfm_options[i].name, 
	    CHANNEL, rfm_options[i].value);
    mcs_manager_notify (mcs_manager, CHANNEL);
}


static void
command_help (GtkWidget * button, gpointer data) {
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
    gchar *argv[]={data, "--help", NULL};
    rfm_clear_text (widgets_p); 
    rfm_show_text (widgets_p); 
    rfm_thread_run_argv_full (widgets_p, argv, FALSE, NULL, rfm_operate_stdout, NULL, scroll_to_top);
    //argv[1] = "--version";
    //rfm_thread_run_argv_full (widgets_p, argv, FALSE, NULL, rfm_operate_stdout, NULL, rfm_null_function);

//    rfm_thread_run_argv_full (widgets_p, argv, FALSE, NULL, rfm_markup_stdout_f, NULL, rfm_null_function);  
    //rfm_thread_run_argv_with_stderr (widgets_p, argv, FALSE, rfm_dump_output);
}



static void
rtfm (GtkWidget * button, gpointer data) {
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
    gchar *argv[]={"man", data, NULL};
    rfm_clear_text (widgets_p); 
    rfm_show_text (widgets_p); 
    rfm_thread_run_argv_full (widgets_p, argv, FALSE, NULL, rfm_operate_stdout, NULL, scroll_to_top);
//    rfm_thread_run_argv_full (widgets_p, argv, FALSE, NULL, stdout_f2, NULL, rfm_null_function);
    //rfm_thread_run_argv_with_stderr (widgets_p, argv, FALSE, rfm_dump_output);
}
// FIXME: these two functions are in xmltree
static gchar *
mod_string(guint mask){
    if (!mask) return NULL;
	gchar *mod=g_strdup("");
	if (mask & GDK_SHIFT_MASK) {
	    gchar *g = g_strconcat (mod,_("Shift"), "+", NULL);
	    g_free(mod);
	    mod = g;
	}
	if (mask & GDK_CONTROL_MASK) {
	    gchar *g = g_strconcat (mod,_("Control"), "+", NULL);
	    g_free(mod);
	    mod = g;
	}
	if (mask & GDK_MOD1_MASK)  {
	     gchar *g = g_strconcat (mod,_("Alt"), "+", NULL);
	    g_free(mod);
	    mod = g;
	}
	if (strlen(mod)==0) {
	    g_free(mod); 
	    mod = g_strdup_printf ("0x%x+", mask);
	}
	return mod;
}
static gchar *key_string(guint keyval){
	gchar *key=NULL;
	if ((keyval > 0x40 && keyval < 0x5b) ||(keyval >0x02f  && keyval < 0x03a)) {
	    key = g_strdup_printf("%c", keyval);
	}
	else if (keyval > 0x60 && keyval < 0x7b) {
	    key = g_strdup_printf("%c", keyval);
	}
	else if (keyval > 0xffbd && keyval < 0xffca) { // function keys f1-f12
	    key = g_strdup_printf("F%d", keyval-0xffbd);
	}
	else { // other keys 
	    switch (keyval){
		case GDK_KEY_Home: key = g_strdup(_("Home")); break;
		case GDK_KEY_Left: key = g_strdup(_("Left")); break;
		case GDK_KEY_Up: key = g_strdup(_("Up")); break;
		case GDK_KEY_Right: key = g_strdup(_("Right")); break;
		case GDK_KEY_Down: key = g_strdup(_("Down")); break;
		case GDK_KEY_Page_Up: key = g_strdup(_("Page up")); break;
		case GDK_KEY_Page_Down: key = g_strdup(_("Page down")); break;
		case GDK_KEY_End: key = g_strdup(_("End")); break;
		case GDK_KEY_Begin: key = g_strdup(_("Begin")); break;
		case GDK_KEY_Delete: key = g_strdup(_("Delete")); break;
		case GDK_KEY_Insert: key = g_strdup(_("Insert")); break;
		case GDK_KEY_equal: key = g_strdup(_("Equal")); break;
		case GDK_KEY_plus: key = g_strdup(_("Plus")); break;
		case GDK_KEY_minus: key = g_strdup(_("Minus")); break;
		case GDK_KEY_KP_Add: key = g_strdup(_("Add")); break;
		case GDK_KEY_KP_Subtract: key = g_strdup(_("Subtract")); break;
	    } 
	}
	if (!key) key = g_strdup_printf("0x%x", keyval);
	return key;
}

static void
set_default_keybindings (xmltree_t *xmltree_p){
	NOOP(stderr, "keybindings:\n");
    RodentCallback *p = (RodentCallback *) rfm_natural(RFM_MODULE_DIR, "callbacks", NULL, "get_menu_callback");
    xmltree_item *top_item_p = XMLTREE_get_tag_item(xmltree_p, NULL, "keys");
    if (!top_item_p) top_item_p = XMLTREE_tag_item_add(xmltree_p, NULL, "keys");

    if (!p) DBG("unable to get menu_callback_p\n");

    for (; p && p->function_id >= 0; p++){
	NOOP(stderr, "checking: %s\n", p->string);
	gchar *function_id_s = g_strdup_printf("%d", p->function_id);
	gchar *mod=mod_string(p->mask);
	gchar *key=key_string(p->key);
	const gchar *icon = p->icon;
	if (!icon && p->type == CHECKITEM_TYPE)icon = "xffm/emblem_synchronized"; 
	else if (!icon && p->type == RADIOITEM_TYPE)icon = "xffm/emblem_favorite"; 
	xmltree_item *Tag_item_p=NULL;

	GSList *list = XMLTREE_get_tag_item_list(xmltree_p, top_item_p, "keybind");
	GSList *tmp = list;
	gboolean found = FALSE;
	for (;tmp && tmp->data; tmp = tmp->next){
	    Tag_item_p = tmp->data;
	    xmltree_attribute *a = XMLTREE_get_attribute(Tag_item_p, "function_id");
	    const gchar *value = XMLTREE_get_attribute_value(a);
	    if (value && strcasecmp(value, function_id_s)==0){
		found = TRUE;
		break;
	    }
	}
	g_slist_free(list);
	if (found){
	    g_free(function_id_s);
	    // Replace text item with current translation
            // add function will add if not present and replace if present.
	    XMLTREE_set_attribute_parent(xmltree_p, Tag_item_p);
	    XMLTREE_attribute_item_add(xmltree_p, "text", _(p->string));
	    continue;
	}
	// Function id not found

	Tag_item_p = XMLTREE_tag_item_add(xmltree_p, top_item_p, "keybind");
	gchar *key_s = g_strdup_printf("%d", p->key);
	gchar *mask_s = g_strdup_printf("%d", p->mask);
	gchar *sequence = g_strdup_printf("%s%s", (mod)?mod:"", key);
	// This item is just to check validation routine during test runs
	// attribute_item_add(Tag_p, Tag_item_p, "string", p->string, NULL);
	    XMLTREE_set_attribute_parent(xmltree_p, Tag_item_p);
	XMLTREE_attribute_item_add(xmltree_p,  "icon_id", _(p->icon) );
	XMLTREE_attribute_item_add(xmltree_p,  "text", _(p->string) );
	XMLTREE_attribute_item_add(xmltree_p,  "Keybinding", sequence );
	XMLTREE_attribute_item_add(xmltree_p,  "key", key_s );
	XMLTREE_attribute_item_add(xmltree_p,  "mask", mask_s );
	XMLTREE_attribute_item_add(xmltree_p,  "function_id", function_id_s );
	NOOP(stderr, "adding: %s\n", p->string);
	g_free(key);
	g_free(mod);
	g_free(key_s);
	g_free(mask_s);
	g_free(function_id_s);
    }
}


static void
k_callback (GtkButton * button, gpointer data) {
    void *xmltree = XMLTREE_new();
    XMLTREE_set_defaults_function(xmltree, set_default_keybindings, xmltree);
    XMLTREE_set_title(xmltree, _("Configuration of keybindings"));
    
    gchar *xmlfile = g_build_filename(KEYBINDINGS_FILE, NULL); 
    XMLTREE_set_xml(xmltree, xmlfile);
    g_free(xmlfile);

    gchar *schemafile = g_build_filename(KEYBINDINGS_SCHEMA, NULL); 
    XMLTREE_set_schema(xmltree, schemafile);
    g_free(schemafile);

    XMLTREE_set_editable_attribute(xmltree, "Keybinding", XMLTREE_key_type);
    XMLTREE_text_activates_top_attribute(xmltree, 1);

    //set_default_keybindings (xmltree);

    XMLTREE_run(xmltree);
    XMLTREE_free(xmltree);

    
    //rfm_rational(RFM_MODULE_DIR, "xmltree", (void *)button, (void *)data, "xmltree");
}


gint
rfm_dialog_run_response(GtkWidget *dialog){
    gint response = GTK_RESPONSE_NONE;

    // gtk_dialog_run() is thread unfriendly if readlock on population
    // is enabled. Hold on read lock may block release of gdk mutex.
	
    response = gtk_dialog_run (GTK_DIALOG (dialog));

    return response;
}

void 
t_callback (GtkButton * button, gpointer data) {
    gint64 rfm_toolbar = DEFAULT_TOOLBAR_BUTTONS;
    const gchar *rfm_toolbar_s = getenv("RFM_TOOLBAR");
    if (rfm_toolbar_s && strlen(rfm_toolbar_s)){
	errno = 0;
	rfm_toolbar = strtoll(rfm_toolbar_s, NULL, 16);
	if (errno){
	    DBG("t_callback(): %s\n", strerror(errno));
	    rfm_toolbar = DEFAULT_TOOLBAR_BUTTONS;
	}
    }

    gint response = GTK_RESPONSE_NONE;
    GtkWidget *dialog = gtk_dialog_new_with_buttons (
	    _("Toolbar Settings"), NULL, 
	    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            _("Cancel"), GTK_RESPONSE_CANCEL, 
            _("Ok"), GTK_RESPONSE_YES,
	    NULL);
    if(!dialog){
	DBG("t_callback(): cannot create dialog\n");
        return;
    }
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG(dialog));
    GtkWidget *frame  = gtk_frame_new("");
    gtk_container_add(GTK_CONTAINER(box), frame);
    gtk_widget_show(frame);
    GtkWidget *t_label = gtk_label_new("");
    gchar *markup = g_strdup_printf("<b>%s</b>",
	    _("Configure which items should appear in the toolbar(s)."));
    gtk_label_set_markup(GTK_LABEL(t_label), markup);
    g_free(markup);
    gtk_widget_show(t_label);
    gtk_frame_set_label_widget(GTK_FRAME(frame),t_label);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_NONE);
    box = rfm_vbox_new(TRUE, 3);
    gtk_container_add(GTK_CONTAINER(frame), box);
    gtk_widget_show(box);

    GtkWidget *scrolled_window = gtk_scrolled_window_new  (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, -1 , 375);
    gtk_box_pack_start (GTK_BOX (box), scrolled_window, TRUE, TRUE, 0);
    GtkWidget *content = rfm_vbox_new(TRUE, 3);
#if GTK_MAJOR_VERSION==3 && GTK_MINOR_VERSION>=8
    gtk_container_add(GTK_CONTAINER(scrolled_window), content);

#else
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window),content);
#endif
    gtk_widget_show(scrolled_window);
    gtk_widget_show(content);

    GtkWidget *check_button[BUTTON_OVERFLOW_ID];
    gint i;
    RodentButtonDefinition *button_callback_p = 
            rodent_get_button_definitions();    
    for (i=0; i<BUTTON_OVERFLOW_ID; i++){
	const gchar *string = (button_callback_p+i)->callback.string;
	const gchar *icon = (button_callback_p+i)->callback.icon;
	const gchar *text = (button_callback_p+i)->text;

	GtkWidget *hbox = rfm_hbox_new(FALSE, 2);
	gtk_box_pack_start (GTK_BOX(content), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
	check_button[i] = gtk_check_button_new();
	gtk_box_pack_start (GTK_BOX(hbox), check_button[i], FALSE, FALSE, 0);
	gtk_widget_show(check_button[i]);

	if (rfm_toolbar &(ONE64<<(button_callback_p+i)->id)) {
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button[i]), TRUE);
	}

	GdkPixbuf *pb = rfm_get_pixbuf(icon, 18);
	if (pb) {
	    GtkWidget *image = gtk_image_new_from_pixbuf(pb);
	    g_object_unref(pb);
	    gtk_box_pack_start (GTK_BOX(hbox), image, FALSE, FALSE, 0);
	    gtk_widget_show(image);
	}else if (text){
	    GtkWidget *label = gtk_label_new("");
	    gchar *markup = g_strdup_printf("<span foreground=\"black\" background=\"white\" size=\"xx-small\">%s</span>", _(text));
	    gtk_label_set_markup(GTK_LABEL(label), markup);
	    g_free(markup);
	    gtk_widget_show (label);
	    gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
	}
	GtkWidget *label = gtk_label_new(_(string));
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);
    }
    gtk_widget_show_all (dialog);
    // This does not work for fvwm2 (shucks)
    // gdk_window_set_keep_above(gtk_widget_get_window(dialog), TRUE);
    // This works for fvwm2
    gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(settings_dialog));

    response = rfm_dialog_run_response(dialog);

    gtk_widget_hide (dialog);

    if(response == GTK_RESPONSE_YES){
	rfm_toolbar = 0;
	for (i=0; i<BUTTON_OVERFLOW_ID; i++){
	    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button[i]))){
		rfm_toolbar |= (ONE64<<(button_callback_p+i)->id);
	    }
	}

	gchar *rfm_toolbar_v = g_strdup_printf("0x%llx", (long long) rfm_toolbar);
	mcs_set_var("RFM_TOOLBAR", rfm_toolbar_v);
	g_free(rfm_toolbar_v);
	// set environ
    }
    gtk_widget_destroy (dialog);
        
    return ;

}

static GtkWidget *
label_new (void) {
    GtkWidget *label;
    label = gtk_label_new ("");
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    return (label);
}

static void
mk_bit_toggles(settings_t *settings_p, GtkWidget *vbox, 
	const gchar *boxname, gint id, RfmProgramOptions *options_p){

    g_object_set_data(G_OBJECT(settings_p->dialog), boxname, vbox);
    g_object_set_data(G_OBJECT(vbox), "var_name", GINT_TO_POINTER(id));
    g_object_set_data(G_OBJECT(vbox), "options_p", options_p);

    GtkGrid *grid;
#if GTK_MAJOR_VERSION>=3
    grid = GTK_GRID(gtk_grid_new());
#else
    grid = GTK_TABLE(gtk_table_new (1, 1, TRUE));
#endif
        
    gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(grid), TRUE, FALSE, 0);

    gboolean indent = FALSE;
    gint row=0;

    if (!options_p) DBG("mk_bit_toggles(%s): options_p is NULL.\n", boxname);
    for (;options_p && options_p->option; options_p++){
	if (strcmp(options_p->option,"submodule-indent")==0) {
	    indent = TRUE;
	    continue;
	}
	if (strcmp(options_p->option,"submodule-unindent")==0) {
	    indent = FALSE;
	    continue;
	}
	GtkWidget *button_box = rfm_hbox_new(FALSE, 0);
	const gchar *label = (options_p->text)?options_p->text:options_p->option;
	if (strcmp(options_p->option,"submodule-label")==0){
	    GtkWidget *w = gtk_label_new("");
	    gchar *t = g_strdup_printf("<i>%s:</i>", label);
	    gtk_label_set_markup(GTK_LABEL(w), t);
	    g_free(t);
	    gtk_grid_attach(grid, w, 0, row, 1, 1);
	    row++;
	    continue;
	}
	if (indent){
	    GtkWidget *w = gtk_label_new("   ");
	    gtk_box_pack_start (GTK_BOX(button_box), w, FALSE, FALSE, 0);
	}
	GtkWidget *t_button = rfm_check_button_new ();
	
	gtk_widget_set_sensitive(t_button, options_p->sensitive);
	gtk_widget_set_sensitive(button_box, options_p->sensitive);
        gtk_box_pack_start (GTK_BOX(button_box), t_button, FALSE, FALSE, 0);
	if (options_p->option && strlen(options_p->option)){
	    GtkWidget *option_text = gtk_label_new("");
	    gchar *markup = g_strdup_printf("<span weight=\"bold\">%s</span>",
		options_p->option);
	    gtk_label_set_markup(GTK_LABEL(option_text), markup);
	    g_free(markup);
	    gtk_box_pack_start (GTK_BOX(button_box), option_text,
		    FALSE, FALSE, 0);
	}

	g_object_set_data(G_OBJECT(t_button), "settings_p", settings_p);
	const gchar *name = options_p->option;
	g_object_set_data(G_OBJECT(vbox), name, t_button);



#if GTK_MAJOR_VERSION>=3
        g_signal_connect (t_button, "notify::active", 
		G_CALLBACK (bit_option_toggled), (void *)boxname);

#else
        g_signal_connect (t_button, "toggled", 
		G_CALLBACK (bit_option_toggled), (void *)boxname);
#endif

	gboolean combo_choice = FALSE;
	if (options_p->choices){
	    combo_choice = TRUE;
	    GtkWidget *box = rfm_hbox_new(FALSE,0);
	    GtkWidget *cbox;
	    if (GPOINTER_TO_INT(options_p->choices) == -1) {
		NOOP(stderr, "entry for %s\n", rfm_options[options_p->choice_id].name);
		cbox = gtk_entry_new();
	    } else {
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
		cbox = gtk_combo_box_new_text ();
#else
		cbox = gtk_combo_box_text_new ();
#endif
	    }
	    gtk_widget_set_sensitive(box, options_p->sensitive);
	    gtk_widget_set_sensitive(cbox, options_p->sensitive);
	    g_object_set_data(G_OBJECT(settings_p->dialog), 
		    rfm_options[options_p->choice_id].name, cbox);
	    NOOP(stderr, "set %s box=0x%x\n", rfm_options[options_p->choice_id].name,
		    GPOINTER_TO_INT(cbox));
	
	    gchar **p=options_p->choices;
	    gint place = 0;
	    if (GPOINTER_TO_INT(options_p->choices) == -1) {
                environ_t *environ_v = rfm_get_environ();
		if (getenv(environ_v[options_p->sensitive].env_var)) {
		    gtk_entry_set_text(GTK_ENTRY(cbox), 
			getenv(environ_v[options_p->sensitive].env_var));
		}
		g_signal_connect (cbox, "key-release-event", G_CALLBACK (entry_changed), GINT_TO_POINTER(options_p->choice_id));
	    } else {
		for (;p && *p; p++){
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
		    gtk_combo_box_insert_text (GTK_COMBO_BOX(cbox), 
			place++, *p);
#else
		    gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT(cbox), 
			place++, *p);
#endif	
		}
		gtk_combo_box_set_active (GTK_COMBO_BOX(cbox), 0);
		g_signal_connect (cbox, "changed", G_CALLBACK (combo_changed), GINT_TO_POINTER(options_p->choice_id));
	    }
	    g_object_set_data(G_OBJECT(cbox), "settings_p", settings_p);
	    gtk_box_pack_start (GTK_BOX(box), cbox, FALSE, FALSE, 0);

	    gtk_box_pack_start (GTK_BOX(button_box), box, FALSE, FALSE, 0);
	    gtk_widget_set_sensitive(cbox, options_p->sensitive);
	}
	gtk_grid_attach(grid, button_box, 0, row, 1,1);

	row++;
	if (options_p->text) {
	    gchar *text =(strstr(options_p->text, "%d"))?
		    g_strdup_printf(_(options_p->text), 3):
		    g_strdup(_(options_p->text));
	    // Let's do some kinky stuff here...
	    gchar **kinky;
	    
	    // Replace tabs for spaces.
	    gchar *text_p = text;
	    for (; text_p && *text_p; text_p++){
		if (*text_p == '\t') *text_p = ' ';
	    }
	    // collapse extra spaces...
	    if (strstr(text, "  ")){
		kinky = g_strsplit(text, " ", -1);
		gchar **k = kinky;
		gchar *collapsed = g_strdup("");
		for (;k && *k;k++) if (strlen(*k)) {
		    NOOP(stderr, "concating \"%s\"\n", *k);
		    gchar *g = g_strconcat(collapsed, " ", *k, NULL);
		    g_free(collapsed);
		    collapsed = g;
		}
		g_strfreev(kinky);
		g_free(text);
		text = collapsed;
	    }

	    //
	    // 1. Split on \n
	    if (strchr(text, '\n')){
		kinky = g_strsplit(text, "\n", -1);
	    } else {
		kinky = (gchar **)malloc(sizeof(gchar *)*2);
		if (!kinky) g_error("malloc: %s\n", strerror(errno));
		memset(kinky, 0, sizeof(gchar *) *2);
		kinky[0] = g_strdup(text);

	    }


	    gchar **k = kinky;
	    //if (options_p->sensitive) 
	    for (;k && *k;k++){
		GtkWidget *w = label_new();
		gtk_widget_set_sensitive(w, options_p->sensitive);
		const gchar *spacing =" ";
		gchar *markup;
		if (combo_choice) spacing = "           ";
		if (k != kinky) {
		    spacing = "           ";
		    markup = g_strdup_printf("%s<span style=\"italic\">%s</span>", spacing, *k);
		} else if (options_p->option && strlen(options_p->option)) {
		    markup = g_strdup_printf("%s<span  style=\"italic\">%s</span>", spacing, *k);

		} else {
		    markup = g_strdup_printf("<span weight=\"bold\">%s</span>", *k);
		}

		gtk_label_set_markup(GTK_LABEL(w), markup);
		g_free(markup);
		if (combo_choice){
		    gtk_grid_attach(grid, w, 0, row, 1,1);
		    row++;
		} else if (k == kinky) {
		    gtk_box_pack_start (GTK_BOX(button_box), w, 
			    FALSE, FALSE, 0);

		} else {
		    gtk_grid_attach(grid, w, 0, row, 1,1);
		    row++;
		}

	    }
	    g_free(text);
	    g_strfreev(kinky);
	} else {
	    row++;
	}
	gtk_widget_set_sensitive(button_box, options_p->sensitive);
    }
}


static void 
subtitle(GtkWidget *parent, GtkWidget *dialog, const gchar *text, gchar *button_command){
     
    GtkWidget *label_box=rfm_hbox_new(FALSE, 0);
    gtk_box_pack_start ((GtkBox *) parent, label_box, FALSE, FALSE, 0);
    gchar *tab_text = g_strdup_printf ("<b><i>%s</i></b>   ", text);
    GtkWidget *label = gtk_label_new (tab_text);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    //gtk_frame_set_label_widget (GTK_FRAME (framebox), label);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start ((GtkBox *) label_box, label, FALSE, FALSE, 0);
    if (button_command){
	GtkWidget *button;
        gboolean do_help_button=FALSE;
#ifdef GNU_LS
        if (strcmp(button_command, "ls")==0) do_help_button=TRUE;
#endif

#ifdef GNU_CP
        if (strcmp(button_command, "cp")==0) do_help_button=TRUE;
#endif

#ifdef GNU_LN
        if (strcmp(button_command, "ln")==0) do_help_button=TRUE;
#endif

#ifdef GNU_MV
        if (strcmp(button_command, "mv")==0) do_help_button=TRUE;
#endif

#ifdef GNU_RM
        if (strcmp(button_command, "rm")==0) do_help_button=TRUE;
#endif
        if (strcmp(button_command, "shred")==0) do_help_button=TRUE;

        if (do_help_button) {
            button = rfm_dialog_button ("xffm/stock_dialog-question", NULL);
            g_object_set_data(G_OBJECT(button), "dialog", dialog);
            g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (command_help), button_command);
            gchar *g=g_strdup_printf("%s --help", button_command);
	    rfm_add_custom_tooltip(button, NULL, g);
            g_free(g);
            gtk_box_pack_start ((GtkBox *) label_box, button, FALSE, FALSE, 0);
            gtk_widget_show(button);
        } else {
	    button = rfm_dialog_button ("xffm/stock_dialog-question", NULL);
	    g_object_set_data(G_OBJECT(button), "dialog", dialog);
	    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (rtfm), button_command);
	    gchar *g=g_strdup_printf("man %s", button_command);
	    rfm_add_custom_tooltip(button, NULL, g);
	    g_free(g);
	    gtk_box_pack_start ((GtkBox *) label_box, button, FALSE, FALSE, 0);
	    gtk_widget_show(button);
	}
    }
    gtk_widget_show_all (label_box);
    g_free(tab_text);
}

static GtkWidget *
create_tab (GtkNotebook * notebook, char *label, char *frame_label) {
    GtkWidget *tab_label1 = gtk_label_new (label);
    gtk_widget_show (tab_label1);
    GtkWidget *page_box = rfm_vbox_new (FALSE, 6);
    GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), 
	    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_show (sw);
    if(gtk_notebook_append_page (notebook, sw, tab_label1) < 0) {
        g_error ("Cannot append page to gtk_notebook!");
    }

#if GTK_MAJOR_VERSION==3 && GTK_MINOR_VERSION>=8
    gtk_container_add(GTK_CONTAINER(sw), page_box);

#else
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw),page_box);
#endif
    gtk_widget_show (page_box);

    // deprecated: add_spacer (GTK_BOX (page_box));

    GtkWidget *header = gtk_label_new ("");
    gchar *g = g_strdup_printf("<b>%s</b>", frame_label);
    gtk_label_set_markup (GTK_LABEL(header), g);  
    g_free(g);
    GtkWidget *hbox = rfm_hbox_new(FALSE, 3);
    gtk_box_pack_start (GTK_BOX (page_box), hbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), header, FALSE, FALSE, 0);
    GtkWidget *vbox = rfm_vbox_new (FALSE, 2);
    gtk_box_pack_start (GTK_BOX (page_box), vbox, FALSE, FALSE, 0);

    gtk_widget_show (vbox);
    g_object_set_data(G_OBJECT(vbox), "page", sw);
#if 0
    GtkWidget *frame = xfce_framebox_new (frame_label, TRUE);
    GtkWidget *frame = rfm_vbox_new (frame_label, TRUE);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (page_box), frame, FALSE, FALSE, 0);

    GtkWidget *vbox = rfm_vbox_new (FALSE, 6);
    xfce_framebox_add (XFCE_FRAMEBOX (frame), vbox);
#endif
    return vbox;

}

static GtkWidget *
make_gint_combo_box (GtkWidget * vbox, gint id, const gchar **options, void *callback) {
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    // this is deprecated...
    GtkWidget *box = gtk_combo_box_new_text ();
#else
    GtkWidget *box = gtk_combo_box_text_new ();
#endif
    int place = 0;
    GtkWidget *hbox = rfm_hbox_new (FALSE, 6);
    gchar *text;
    environ_t *environ_v = rfm_get_environ();
    if (id == RFM_DEFAULT_ICON_SIZE) {
	text = g_strdup_printf("%s (%s)", 
		_(environ_v[id].env_text), _("default"));
    } else {
	text = g_strdup(_(environ_v[id].env_text));
    }
    GtkWidget *label = gtk_label_new (text);
    g_free(text);
    gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
    GSList *list=NULL;

    const gchar **p=options;
    if(getenv (environ_v[id].env_var)
       && strlen (getenv (environ_v[id].env_var))) {
	for (;p && *p; p++){
	    if (strcmp(*p, environ_v[id].env_var)==0) {
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
		gtk_combo_box_insert_text (GTK_COMBO_BOX(box), 
			place++, _(environ_v[id].env_var));
#else
		 gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT(box), 
			place++, _(environ_v[id].env_var));
#endif
		list=g_slist_prepend(list, _(environ_v[id].env_var));
		break;
	    }
	}

    }
    
    for(p = options; *p; p++) {
       if(place == 0) {
	    /* default values. Null values are not functionally acceptable */
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
	    gtk_combo_box_insert_text (GTK_COMBO_BOX(box), place++, _(*p));
#else
	    gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT(box), place++, _(*p));
#endif
	    list=g_slist_prepend(list, (void *)_(*p));
	    if(!rfm_options[id].value || !strlen (rfm_options[id].value)) {
		g_free (rfm_options[id].value);
		rfm_options[id].value = g_strdup (_(*p));
		mcs_manager_set_string (mcs_manager, rfm_options[id].name,
			CHANNEL, *p);
		mcs_manager_notify (mcs_manager, CHANNEL);
	    }
	} else {
	    list=g_slist_append(list, (void *)_(*p));
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
	    gtk_combo_box_append_text (GTK_COMBO_BOX(box), _(*p));
#else
	    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(box), _(*p));
#endif
	}
    }

    g_object_set_data(G_OBJECT(box), "list", list);
    gtk_combo_box_set_active (GTK_COMBO_BOX(box), 0);
    gtk_box_pack_start (GTK_BOX(hbox), box, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    g_signal_connect (box, "changed", G_CALLBACK (callback), NULL);
    return box;
}

static GtkWidget *
make_exec_combo_box (GtkWidget * vbox, gint id, const gchar **options, void *callback) {
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    GtkWidget *box = gtk_combo_box_new_text ();
#else
    GtkWidget *box = gtk_combo_box_text_new ();
#endif
    GtkWidget *hbox = rfm_hbox_new (FALSE, 6);
    environ_t *environ_v = rfm_get_environ();
    GtkWidget *label = gtk_label_new (_(environ_v[id].env_text));
    gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
    GSList *list=NULL;
    gint place=0;

    if(getenv (environ_v[id].env_var)
       && strlen (getenv (environ_v[id].env_var))) {
        if(test_command(getenv (environ_v[id].env_var)) ){
	    gchar *path = g_strdup (getenv (environ_v[id].env_var));
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
            gtk_combo_box_insert_text (GTK_COMBO_BOX(box), place++, path);
#else
            gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT(box), place++, path);
#endif
	    list=g_slist_prepend(list, path);
	}
    }
    const gchar **p;
    for(p = options; *p; p++) {
        if(test_command(*p)) {
            if(place){
		list=g_slist_append(list, (void *)*p);
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
                gtk_combo_box_append_text (GTK_COMBO_BOX(box), (void *)*p);

#else
                gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(box), (void *)*p);
#endif
	    } else {
		list=g_slist_prepend(list, (void *)*p);
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
                gtk_combo_box_insert_text (GTK_COMBO_BOX(box), place++, *p);
#else
                gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT(box), place++, *p);
#endif
                if(!rfm_options[id].value || !strlen (rfm_options[id].value)) {
                    g_free (rfm_options[id].value);
		}
                rfm_options[id].value = g_strdup (*p);
                mcs_manager_set_string (mcs_manager, rfm_options[id].name,
			    CHANNEL, *p);
                mcs_manager_notify (mcs_manager, CHANNEL);
            } 
        }
    }

    g_object_set_data(G_OBJECT(box), "list", list);
    gtk_combo_box_set_active (GTK_COMBO_BOX(box), 0);
    gtk_box_pack_start (GTK_BOX(hbox), box, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    g_signal_connect (box, "changed", G_CALLBACK (callback), NULL);
    return box;
}

static GtkWidget *
make_string_combo_box (gchar **options, const gchar *id, void *data) {
    GtkWidget *combo_box;
    gchar **p=options;
    gboolean first=TRUE;
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    combo_box = gtk_combo_box_new_text ();
    for (;p && *p; p++){
	if (first){
	    gtk_combo_box_insert_text (GTK_COMBO_BOX(combo_box), 0, *p);
	    first = FALSE;
	} else	gtk_combo_box_append_text (GTK_COMBO_BOX(combo_box), *p);
    }
#else
    combo_box = gtk_combo_box_text_new ();
    for (;p && *p; p++){
	if (first){
	    gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT(combo_box), 0, *p);
	    first = FALSE;
	} else	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(combo_box), *p);
    }
#endif
    g_object_set_data(G_OBJECT(combo_box), id, data);    

    return combo_box;
}

static GtkWidget *
make_file_chooser_button (int id, gboolean is_folder, GtkWidget * hbox) {
    environ_t *environ_v = rfm_get_environ();
    GtkWidget *label = gtk_label_new (_(environ_v[id].env_text));
    gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
    GtkWidget *button;
    if(is_folder) {
        button = gtk_file_chooser_button_new (_("Select a folder"), 
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    } else {
        button = gtk_file_chooser_button_new (_("Select Files..."), 
		GTK_FILE_CHOOSER_ACTION_OPEN);
    }


    if (rfm_options[id].value && 
		rfm_g_file_test(rfm_options[id].value, G_FILE_TEST_EXISTS)){
	COMMENT(stderr, "1.make_file_chooser_button: setting %s->%s\n",rfm_options[id].name,rfm_options[id].value);
	if (rfm_g_file_test(rfm_options[id].value, G_FILE_TEST_IS_DIR)){
	    gtk_file_chooser_set_current_folder (
		GTK_FILE_CHOOSER (button), rfm_options[id].value);
	} else {
	    gtk_file_chooser_set_filename (
		GTK_FILE_CHOOSER (button), rfm_options[id].value);
	}
    } 
    gtk_file_chooser_button_set_title ((GtkFileChooserButton *) (button), 
	    (const gchar *)_(environ_v[id].env_text));
    gtk_file_chooser_button_set_width_chars ((GtkFileChooserButton *) button, 15);
    gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, 0);
     


    g_signal_connect (button, 
	    "file-set", G_CALLBACK (file_set), GINT_TO_POINTER(id));
    return button;
}
static void
sit_message(const gchar *file){
    widgets_t *widgets_p = rfm_get_widget("widgets_p");
    rfm_diagnostics(widgets_p, "xffm/stock_dialog-info",NULL);
    gchar *text = g_strdup_printf(_("Creating a new file (%s)"), file);
    rfm_diagnostics(widgets_p, "xffm_tag/blue", text, "\n",NULL);
    g_free(text);
}

static GtkWidget *
get_example_image(void){
    GtkWidget *image=NULL;
    GdkPixbuf *pixbuf;
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
    gchar *example = NULL;
    if (icon_theme) example = gtk_icon_theme_get_example_icon_name(icon_theme);
    GtkIconInfo *icon_info = NULL;
    if (example && icon_theme) {
	icon_info = gtk_icon_theme_lookup_icon(icon_theme, example, SIZE_DIALOG, 0);
    }

    if (icon_info){
	const gchar *path = gtk_icon_info_get_filename(icon_info);
	pixbuf=rfm_get_pixbuf(path, SIZE_DIALOG);
    } else {
	pixbuf=rfm_get_pixbuf("xffm/emblem_unreadable", SIZE_DIALOG);
    }
#if GTK_MAJOR_VERSION==3 && GTK_MINOR_VERSION>=8
    if (icon_info)g_object_unref(G_OBJECT(icon_info));
#else
    if (icon_info) gtk_icon_info_free (icon_info); 
#endif

    if (pixbuf) {
	image=gtk_image_new_from_pixbuf(pixbuf);
    }
    if (image) {   
	rfm_add_custom_tooltip(image, pixbuf, _("example"));
    }

    return image;
}

static void
update_icon_example(GtkWidget *image_box){
    if (!image_box){
	DBG("update_icon_example(): image_box should not be NULL\n");
	return;
    }
    GList *list = gtk_container_get_children(GTK_CONTAINER(image_box));
    if (list) {
	GtkWidget *image = list->data;
	if (image && GTK_IS_WIDGET(image)) {
	    gtk_container_remove(GTK_CONTAINER(image_box), image);
	}
    }
    g_list_free(list);
    GtkWidget *image = get_example_image();
    gtk_container_add (GTK_CONTAINER(image_box), image);    
    gtk_widget_show(image);
}

#if GTK_MAJOR_VERSION>=3
static void
write_keyfile(GKeyFile *key_file, const gchar *file){
    TRACE( "group_options_write_keyfile: %s\n", file);
    // Write out key_file:
    gsize file_length;
    gchar *file_string = g_key_file_to_data (key_file, &file_length, NULL);
    gchar *config_directory = g_path_get_dirname(file);
    if (!g_file_test(config_directory, G_FILE_TEST_IS_DIR)){
	TRACE( "creating directory %s\n", config_directory);
	g_mkdir_with_parents(config_directory, 0700);
    }
    g_free(config_directory);
    gint fd = creat(file, O_WRONLY | S_IRWXU);
    if (fd >= 0){
	if (write(fd, file_string, file_length) < 0){
	    DBG("write_keyfile(): cannot write to %s: %s\n", file, strerror(errno));
	}
	close(fd);
    } else {
	DBG("write_keyfile(): cannot open %s for write: %s\n", file, strerror(errno));
    }
}

#define GTK3_SETTINGS_KEYFILE g_get_user_config_dir(),"gtk-3.0","settings.ini"
static void 
save_icon_theme(GtkComboBox *combo_box, gpointer data){
    gchar *themename =
	gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_box));// 2.24

    GtkSettings *settings = gtk_settings_get_default();
    g_object_set( G_OBJECT(settings), 
		    "gtk-icon-theme-name", themename,
		    NULL);
    GKeyFile *key_file = g_key_file_new();
    gchar *file = g_build_filename(GTK3_SETTINGS_KEYFILE, NULL);
    gboolean loaded = g_key_file_load_from_file(key_file, file,
	    G_KEY_FILE_KEEP_COMMENTS |  G_KEY_FILE_KEEP_TRANSLATIONS, NULL);
    if (!loaded) sit_message(file);
    settings_t *settings_p=g_object_get_data(G_OBJECT(combo_box), "settings_p");
    update_icon_example(g_object_get_data(G_OBJECT(combo_box), "image_box"));

    g_key_file_set_string (key_file, "Settings", "gtk-icon-theme-name", themename);
    write_keyfile(key_file, file);
    g_free(file);
    g_free(themename);
    GtkToggleButton *togglebutton = data;
    NOOP(stderr, "toggle state is %d\n", gtk_toggle_button_get_active (togglebutton));
    //if(gtk_toggle_button_get_active (togglebutton)) 
    {
	gboolean state = settings_p->disable_options;
	settings_p->disable_options = FALSE;	
	option_toggled (togglebutton, GINT_TO_POINTER(RFM_USE_GTK_ICON_THEME));
	settings_p->disable_options = state;
    }
}
#else
#define GTK2_SETTING_FILE g_get_home_dir(),".gtkrc-2.0"
static void 
save_icon_theme(GtkComboBox *combo_box, gpointer data){
    gchar *file = g_build_filename(GTK2_SETTING_FILE, NULL);
    GtkSettings *settings = gtk_settings_get_default();
    gchar *themename = NULL;
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    themename = gtk_combo_box_get_active_text (combo_box);
#else
    themename = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(combo_box));
#endif
    g_object_set( G_OBJECT(settings), 
		    "gtk-icon-theme-name", themename,
		    NULL);
    settings_t *settings_p=g_object_get_data(G_OBJECT(combo_box), "settings_p");
    update_icon_example(g_object_get_data(G_OBJECT(combo_box), "image_box"));

    gchar *buffer=g_strdup_printf("gtk-icon-theme-name = \"%s\"\n", themename);
    g_free(themename);
    gchar line[256];
    memset (line, 0, 256);
    FILE *settings_file=fopen(file, "r");
    if (settings_file){
	while (fgets(line, 255, settings_file) && !feof(settings_file)){
	    if (!strstr(line, "gtk-icon-theme-name")){
		gchar *g = g_strconcat(buffer, line, NULL);
		g_free(buffer);
		buffer = g;
	    }
	}
	fclose(settings_file);
    } else sit_message(file);

    settings_file=fopen(file, "w");
    if (settings_file){
	fprintf(settings_file, "%s", buffer);
	fclose(settings_file);
    }
    g_free(buffer);
    GtkToggleButton *togglebutton = data;
    NOOP(stderr, "toggle state is %d\n", gtk_toggle_button_get_active (togglebutton));
    //if(gtk_toggle_button_get_active (togglebutton)) 
    {
	gboolean state = settings_p->disable_options;
	settings_p->disable_options = FALSE;	
	option_toggled (togglebutton, GINT_TO_POINTER(RFM_USE_GTK_ICON_THEME));
	settings_p->disable_options = state;
    }
}

#endif
static gboolean 
anim_f(void *data){
    GdkPixbufAnimation *animation = data;
    GdkPixbufAnimationIter *a_iter = g_object_get_data(G_OBJECT(animation), "a_iter");
    GtkWidget *abox = g_object_get_data(G_OBJECT(animation), "abox");
    if (animation_stop) {
	g_object_unref(a_iter);
	g_object_unref(animation);
	return FALSE;
    }
    // Gtk  bug workaround:
    // suspend animations if tooltip mapped.
    if (rfm_tooltip_is_mapped()) return TRUE;
    gdk_pixbuf_animation_iter_advance(a_iter, NULL);
							 
    GdkPixbuf *pixbuf = gdk_pixbuf_animation_iter_get_pixbuf(a_iter);
    GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
    GList *children = gtk_container_get_children  (GTK_CONTAINER(abox));
    if (children && children->data) {
	GtkWidget *child = children->data;
	if (GTK_IS_WIDGET(child))
            gtk_container_remove (GTK_CONTAINER(abox), child);
    }
    g_list_free(children);
    gtk_container_add  (GTK_CONTAINER(abox), image);
    gtk_widget_show(image);
    return TRUE;
}

static void *
context_run_rfm_settings_dialog (gpointer data) {

    /*GtkWidget *test_dialog = gtk_color_chooser_dialog_new ("test title", NULL);
    g_object_set(G_OBJECT(test_dialog), "show-editor", TRUE, NULL);
    gtk_widget_show(test_dialog);*/
    


    if (!running) return NULL;
    settings_t *settings_p=data;
    settings_dialog = settings_p->dialog = gtk_dialog_new();
    gtk_window_set_type_hint(GTK_WINDOW(settings_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_title(GTK_WINDOW(settings_dialog), mp->caption);
    
    environ_t *environ_v = rfm_get_environ();
    GtkWidget *button = rfm_dialog_button ("xffm/stock_refresh", _("Rebuild Thumbnails"));
    gtk_widget_show (button);
    gtk_dialog_add_action_widget (GTK_DIALOG (settings_p->dialog), button, 2);

    button = rfm_dialog_button ("xffm/stock_select-color", _("Icons"));
    gtk_widget_show (button);
    gtk_dialog_add_action_widget (GTK_DIALOG (settings_p->dialog), button, 1);
    button = rfm_dialog_button ("xffm/stock_ok", _("Ok"));
    gtk_widget_show (button);
    gtk_dialog_add_action_widget (GTK_DIALOG (settings_p->dialog), button, 0);
/*	gtk_dialog_new_with_buttons (mp->caption,
                                          NULL, 0,
                                          _("Rebuild Thumbnails"), 2,
					  _("Icons"), 1, 
					  _("Close"), 0, 
					  NULL);*/

    //gtk_window_set_modal (GTK_WINDOW(settings_dialog), TRUE);
    rfm_global_t *rfm_global_p = rfm_global();
    if (!rfm_global_p) g_error("settings-module.i: !rfm_global_p\n");
    //gtk_window_set_transient_for(GTK_WINDOW(settings_dialog), GTK_WINDOW(rfm_global_p->window));
    g_object_set_data(G_OBJECT(settings_p->dialog), "settings_p", settings_p);
    widgets_t *widgets_p = (widgets_t *)malloc(sizeof(widgets_t));
    if (!widgets_p) g_error("malloc: %s", strerror(errno));
    settings_p->widgets_p = widgets_p;
    if (rfm_global_p){
	rfm_global_p->settings_widgets_p = widgets_p;
    }
    memset(widgets_p, 0, sizeof(widgets_t));
    g_object_set_data(G_OBJECT(settings_p->dialog), "widgets_p", widgets_p);
    gtk_window_stick(GTK_WINDOW (settings_p->dialog));
    gtk_window_set_keep_above(GTK_WINDOW (settings_p->dialog), TRUE);

    g_signal_connect (settings_p->dialog, 
	    "response", G_CALLBACK (dialog_delete), NULL);
    g_signal_connect (settings_p->dialog, 
	    "delete_event", G_CALLBACK (dialog_delete), NULL);
    g_signal_connect (settings_p->dialog, 
	    "destroy", G_CALLBACK (dialog_delete), NULL);
    //vbox = GTK_DIALOG (settings_p->dialog)->vbox;
#if 10
    int i;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkWidget *treeview;
    GtkWidget *label;
    GtkWidget *vbox = rfm_vbox_new (FALSE, 6);
    gtk_box_pack_start (
	    GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG (settings_p->dialog))), 
	    vbox, TRUE, TRUE, 0);
    gtk_widget_show(vbox);
    GtkWidget *hbox = rfm_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
    gtk_widget_show(hbox);

    GtkWidget *header = gtk_label_new ("");
    gtk_label_set_markup (GTK_LABEL(header), mp->caption);       

    GtkWidget *abox = rfm_hbox_new(FALSE,0);
    gtk_box_pack_start (GTK_BOX (hbox), abox, FALSE, TRUE, 0);
    gchar *iconpath = g_strdup_printf("%s/icons/rfm/animated/rodent-96.gif", PACKAGE_DATA_DIR);
    if (!g_file_test(iconpath, G_FILE_TEST_EXISTS)){
	GdkPixbuf *pixbuf = rfm_get_pixbuf("xffm/stock_preferences", 96);
	if (pixbuf){
	    GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
	    g_object_unref(pixbuf);
	    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, TRUE, 0);
	}
    } else {
	GdkPixbufAnimation *animation = gdk_pixbuf_animation_new_from_file (iconpath, NULL);
	g_free(iconpath);
	GdkPixbufAnimationIter *a_iter = gdk_pixbuf_animation_get_iter  (animation, NULL);
	GdkPixbuf *pixbuf = gdk_pixbuf_animation_iter_get_pixbuf(a_iter);
	g_object_set_data(G_OBJECT(animation), "a_iter", a_iter);
	g_object_set_data(G_OBJECT(animation), "abox", abox);

	GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_show(image);
	gtk_box_pack_start (GTK_BOX (abox), image, FALSE, TRUE, 0);
	animation_stop=FALSE;
	g_timeout_add (20, anim_f, animation);
    }

 
    
    
    gtk_box_pack_start (GTK_BOX (hbox), header, FALSE, TRUE, 0);
    
    GtkWidget *vpane = rfm_vpaned_new ();
    g_object_set_data(G_OBJECT(settings_dialog), "vpane", vpane);
    // hack:
    widgets_p->paper = settings_dialog;

    gtk_widget_show (vpane);
    gtk_box_pack_start (GTK_BOX (vbox),vpane, TRUE, TRUE, 0);
    gtk_paned_set_position (GTK_PANED (vpane), 1000);
    vbox = rfm_vbox_new (FALSE, 6);
    gtk_paned_pack1 (GTK_PANED (vpane), 
	    GTK_WIDGET (vbox), FALSE, TRUE);
    gtk_widget_show(vbox);

    widgets_p->diagnostics = gtk_text_view_new ();
    GtkWidget *scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_widget_show ((widgets_p->diagnostics));
    gtk_paned_pack2 (GTK_PANED (vpane), scrolledwindow, TRUE, TRUE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), 
	    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_container_add (GTK_CONTAINER (scrolledwindow), (widgets_p->diagnostics));
    gtk_container_set_border_width (GTK_CONTAINER ((widgets_p->diagnostics)), 2);
    gtk_widget_set_can_focus((widgets_p->diagnostics), FALSE);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW ((widgets_p->diagnostics)), GTK_WRAP_WORD);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW ((widgets_p->diagnostics)), FALSE);


    GtkNotebook *notebook = GTK_NOTEBOOK(gtk_notebook_new ());
    g_object_set_data(G_OBJECT(notebook), "settings_p", settings_p);
    gtk_notebook_set_scrollable (notebook, TRUE);
    gtk_notebook_popup_enable (notebook);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (notebook), TRUE, TRUE, 0);

    ////////////////// General
    //
    vbox = create_tab (notebook, _("General"), _("General Options"));

    {
	gchar *k_text=g_strdup(_("Configuration of keybindings"));
	gpointer k_data=NULL;
	GtkWidget *k_box = rfm_hbox_new(FALSE, 0);
	GtkWidget *k_button =  
	    rfm_mk_little_button ("xffm/emblem_keyboard", 
		      (gpointer)k_callback, k_data, k_text);
	GtkWidget *k_label = gtk_label_new (k_text);
	gtk_box_pack_start (GTK_BOX(k_box), k_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(k_box), k_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), k_box, FALSE, FALSE, 0);
	g_free(k_text);
    }

    {
	gchar *t_text=g_strdup(_("Toolbar Settings"));
	gpointer t_data=NULL;
	GtkWidget *t_box = rfm_hbox_new(FALSE, 0);
	GtkWidget *t_button =  
	    rfm_mk_little_button ("xffm/stock_preferences", 
		     (gpointer)t_callback, t_data, 
		      t_text);
	GtkWidget *t_label = gtk_label_new (t_text);
	gtk_box_pack_start (GTK_BOX(t_box), t_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(t_box), t_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), t_box, FALSE, FALSE, 0);
	g_free(t_text);
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<20
	gtk_widget_set_sensitive(GTK_WIDGET(t_button), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(t_label), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(t_box), FALSE);
#endif
    }
   
    for(i = RFM_DOUBLE_CLICK_NAVIGATION; i <= RFM_ENABLE_LABEL_TIPS; i++) {
	
	if (i == RFM_USE_GTK_ICON_THEME) {
	    GtkSettings *settings = gtk_settings_get_default();
	    gchar *themename = NULL;
	    /*g_object_set( G_OBJECT(settings), 
		    "gtk-icon-theme-name", "gnome",
		    NULL);*/
	    g_object_get( G_OBJECT(settings), 
		    "gtk-icon-theme-name", &themename,
		    NULL);
	    DBG( "icon theme name = %s\n", themename);

	    gchar **th_options = rfm_get_iconthemes();
	    GtkWidget *theme_combo = make_string_combo_box(th_options, "settings_p", settings_p);
	    gint combo_index=0;
	    gchar **p = th_options;
	    gint j;
	    for (j=0; p && *p; p++, j++){
		if (strcmp(themename, *p)==0) combo_index=j;
	    }

	    gtk_combo_box_set_active (GTK_COMBO_BOX(theme_combo), combo_index);
    
	    g_strfreev(th_options);
	    

	    g_free(themename);

	    GtkWidget *theme_box = rfm_hbox_new(FALSE,3);
	    gtk_box_pack_start (GTK_BOX(vbox), theme_box, FALSE, FALSE, 0);
	    toggle_button[i] = gtk_check_button_new_with_label (_(environ_v[i].env_text));
	    gtk_box_pack_start (GTK_BOX(theme_box), toggle_button[i], FALSE, FALSE, 0);
	    gtk_box_pack_start (GTK_BOX(theme_box), theme_combo, FALSE, FALSE, 0);
	    g_signal_connect (G_OBJECT (theme_combo), "changed", G_CALLBACK (save_icon_theme), toggle_button[i]);

	    GtkWidget *image_box = rfm_hbox_new(FALSE, 0);
	    gtk_box_pack_start (GTK_BOX(theme_box), image_box, FALSE, FALSE, 0);
	    g_object_set_data(G_OBJECT(theme_combo), "image_box", image_box);
	    
	    GtkWidget *image=get_example_image();
	    if (image) gtk_box_pack_start (GTK_BOX(image_box), image, FALSE, FALSE, 0);	    
	} else {
	    gchar *text;
	    if (i == RFM_CONTENT_FOLDER_ICONS) {
		text = g_strdup_printf("%s (%s)", 
		    _(environ_v[i].env_text), _("Content Type"));
	    } else if (i == RFM_ENABLE_TIPS) {
		text = g_strdup_printf("%s (%s)",
			_(environ_v[i].env_text), _("Icons"));
	    } else if (i == RFM_ENABLE_LABEL_TIPS) {
		text = g_strdup_printf("%s (%s)",
			_(environ_v[i].env_text), _("Labels"));
	    } else {
		text = g_strdup(_(environ_v[i].env_text));
	    }

	    toggle_button[i] = gtk_check_button_new_with_label (text);
	    g_free(text);
	    gtk_box_pack_start (GTK_BOX(vbox), toggle_button[i], FALSE, FALSE, 0);
	}
	g_object_set_data(G_OBJECT(toggle_button[i]), "settings_p", settings_p);
        g_signal_connect (toggle_button[i], 
		"toggled", G_CALLBACK (option_toggled), GINT_TO_POINTER(i));
    }
    // preview size
	GtkWidget *preview_size_hbox = rfm_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX(vbox), preview_size_hbox, FALSE, FALSE, 0);
	label = gtk_label_new (_(environ_v[RFM_PREVIEW_IMAGE_SIZE].env_text));
	gtk_box_pack_start (GTK_BOX(preview_size_hbox), label, FALSE, FALSE, 0);
	settings_p->preview_size_spinbutton =
	    gtk_spin_button_new_with_range (100.0, 1000.0, 10.0);

	gdouble value = rfm_get_preview_image_size();
	gtk_spin_button_set_value (
		GTK_SPIN_BUTTON(settings_p->preview_size_spinbutton),
                value);
	gtk_box_pack_start (GTK_BOX(preview_size_hbox), settings_p->preview_size_spinbutton, FALSE, FALSE, 0);
	g_object_set_data(
		G_OBJECT(settings_p->preview_size_spinbutton),
		"settings_p", settings_p);
	g_object_set_data(
		G_OBJECT(settings_p->preview_size_spinbutton),
		"which_margin", GINT_TO_POINTER(i));
        g_signal_connect (settings_p->preview_size_spinbutton, 
    	    "value-changed", G_CALLBACK (preview_size_changed), NULL);
    //////   FONTS
    static gchar **fixed_families=NULL;
    static gchar **variable_families=NULL;

    if (!fixed_families){
	PangoFontFamily **pfamilies=NULL;
	gint n_families;
	PangoContext *context =
	    gtk_widget_get_pango_context (rfm_global_p->window);
	pango_context_list_families (context, &pfamilies, &n_families);
	gint k=0;
	variable_families = (gchar **)malloc((n_families+1) * sizeof(gchar *));
	fixed_families = (gchar **)malloc((n_families+1) * sizeof(gchar *));
	if (!fixed_families) g_error("malloc: %s\n", strerror(errno));
	if (!variable_families) g_error("malloc: %s\n", strerror(errno));
	memset(fixed_families, 0, (n_families+1) * sizeof(gchar *));
	memset(variable_families, 0, (n_families+1) * sizeof(gchar *));
	gint kk=0;
	gint kkk=0;
	for (;k<n_families;k++){
	    if (pango_font_family_is_monospace(pfamilies[k])){
		fixed_families[kk++] =
		    g_strdup(pango_font_family_get_name(pfamilies[k]));

	    }
	    variable_families[kkk++] =
		    g_strdup(pango_font_family_get_name(pfamilies[k]));
	}
	g_free(pfamilies);
    }

    {
	GtkWidget *font_vbox=rfm_vbox_new(FALSE, 2);
	GtkWidget *font_box=rfm_hbox_new(FALSE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), font_vbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(font_vbox), font_box, FALSE, FALSE, 0);

	GtkWidget *font_label = gtk_label_new("");
	gtk_box_pack_start (GTK_BOX(font_box), font_label, FALSE, FALSE, 0);
	gchar *label_markup =g_strdup_printf("<b>%s:</b> ", _("Fixed width font"));
	gtk_label_set_markup(GTK_LABEL(font_label), label_markup);
	g_free(label_markup);
	gtk_widget_show(font_label);

	font_box=rfm_hbox_new(FALSE, 2);
	gtk_box_pack_start (GTK_BOX(font_box), gtk_label_new("   "), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(font_vbox), font_box, FALSE, FALSE, 0);
	settings_p->fontsize_box = 
	    make_gint_combo_box (font_box, RFM_FIXED_FONT_SIZE, rfm_get_font_sizes(), (void *)fontsize_changed);
	g_object_set_data(G_OBJECT(settings_p->fontsize_box), "settings_p", settings_p);

	font_box=rfm_hbox_new(FALSE, 2);
	gtk_box_pack_start (GTK_BOX(font_box), gtk_label_new("   "), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(font_vbox), font_box, FALSE, FALSE, 0);
        settings_p->fontfamily_box = 
	    make_gint_combo_box (font_box, RFM_FIXED_FONT_FAMILY, (const gchar **)fixed_families, (void *)fontfamily_changed);
	g_object_set_data(G_OBJECT(settings_p->fontfamily_box), "settings_p", settings_p);
    }


    {
	GtkWidget *font_vbox=rfm_vbox_new(FALSE, 2);
	GtkWidget *font_box=rfm_hbox_new(FALSE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), font_vbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(font_vbox), font_box, FALSE, FALSE, 0);

	GtkWidget *font_label = gtk_label_new("");
	gtk_box_pack_start (GTK_BOX(font_box), font_label, FALSE, FALSE, 0);
	gchar *label_markup =g_strdup_printf("<b>%s:</b> ", _("Variable width font"));
	gtk_label_set_markup(GTK_LABEL(font_label), label_markup);
	g_free(label_markup);
	gtk_widget_show(font_label);


	font_box=rfm_hbox_new(FALSE, 2);
	gtk_box_pack_start (GTK_BOX(font_box), gtk_label_new("   "), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(font_vbox), font_box, FALSE, FALSE, 0);
	settings_p->vfontsize_box = 
	    make_gint_combo_box (font_box, RFM_VARIABLE_FONT_SIZE, rfm_get_font_sizes(), (void *)vfontsize_changed);
	g_object_set_data(G_OBJECT(settings_p->vfontsize_box), "settings_p", settings_p);

	font_box=rfm_hbox_new(FALSE, 2);
	gtk_box_pack_start (GTK_BOX(font_box), gtk_label_new("   "), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(font_vbox), font_box, FALSE, FALSE, 0);
	settings_p->vfontfamily_box = 
	    make_gint_combo_box (font_box, RFM_VARIABLE_FONT_FAMILY, (const gchar **)variable_families, (void *)vfontfamily_changed);
	g_object_set_data(G_OBJECT(settings_p->vfontfamily_box), "settings_p", settings_p);
    }

///////////////////////////////
    settings_p->iconsize_box = 
	make_gint_combo_box (vbox, RFM_DEFAULT_ICON_SIZE, rfm_get_icon_sizes(), (void *)iconsize_changed);
    g_object_set_data(G_OBJECT(settings_p->iconsize_box), "settings_p", settings_p);
    settings_p->terminal_box = 
	make_exec_combo_box (vbox, TERMINAL_CMD, rfm_get_terminals(), (void *)terminal_changed);
    g_object_set_data(G_OBJECT(settings_p->terminal_box), "settings_p", settings_p);
    settings_p->editor_box = 
	make_exec_combo_box (vbox, EDITOR, rfm_get_editors(), (void *)editor_changed);
    g_object_set_data(G_OBJECT(settings_p->editor_box), "settings_p", settings_p);
    ////
    hbox = rfm_hbox_new (FALSE, 6);
    label = gtk_label_new (_(environ_v[RFM_ICONVIEW_COLOR].env_text));
    gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
    settings_p->iconviewcolor_button = gtk_color_button_new ();
    if(getenv ("RFM_ICONVIEW_COLOR") && strlen (getenv ("RFM_ICONVIEW_COLOR")))
    {
#if GTK_MAJOR_VERSION==3 && GTK_MINOR_VERSION>=4
        GdkRGBA color;
        if(gdk_rgba_parse (&color, getenv ("RFM_ICONVIEW_COLOR") )) {
	    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(settings_p->iconviewcolor_button), &color);
        }
        /*gtk_color_chooser_add_palette (
                    GTK_COLOR_CHOOSER(settings_p->iconviewcolor_button),
                    GTK_ORIENTATION_HORIZONTAL,
                    7,
                    49,
                    X11_colors);*/
#else
        GdkColor color;
        if(gdk_color_parse (getenv ("RFM_ICONVIEW_COLOR"), &color)) {
            gtk_color_button_set_color (
		    GTK_COLOR_BUTTON(settings_p->desktopcolor_button), &color);
        }
#endif
    }
    gtk_color_button_set_title (GTK_COLOR_BUTTON(settings_p->iconviewcolor_button), 
	    (const gchar *) _(environ_v[RFM_ICONVIEW_COLOR].env_text));
    gtk_box_pack_start (GTK_BOX(hbox), settings_p->iconviewcolor_button, 
	    FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(settings_p->iconviewcolor_button), "settings_p", settings_p);
    g_signal_connect (settings_p->iconviewcolor_button, 
	    "color-set", G_CALLBACK (color_changed), 
	    GINT_TO_POINTER(RFM_ICONVIEW_COLOR));
    //g_object_set(G_OBJECT(settings_p->iconviewcolor_button),"show-editor", TRUE, NULL);
    if(gtk_widget_is_composited (settings_p->dialog)) {
        hbox = rfm_hbox_new (FALSE, 6);
        label = gtk_label_new (_(environ_v[RFM_TRANSPARENCY].env_text));
        gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
        double transparency=0.0;
        if(getenv ("RFM_TRANSPARENCY") && strlen (getenv ("RFM_TRANSPARENCY"))) {
            errno = 0;
            transparency = strtod (getenv ("RFM_TRANSPARENCY"), NULL);
            if(errno != 0 || transparency < 0.0)
                transparency = 0.0;
            else if(transparency > 0.75)
                transparency = 0.75;
        }
        GtkWidget *transparency_slider = rfm_hscale_new_with_range (0.0, 0.75, 0.01);
	g_object_set_data(G_OBJECT(transparency_slider), "settings_p", settings_p);
        gtk_scale_set_digits (GTK_SCALE(transparency_slider), 2);
        gtk_range_set_value (GTK_RANGE(transparency_slider), transparency);
        gtk_box_pack_start (GTK_BOX(hbox), transparency_slider, TRUE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
        g_signal_connect (transparency_slider, "change-value",
                          G_CALLBACK (transparency_changed), 
			  GINT_TO_POINTER(RFM_TRANSPARENCY));

    }
    //////////////////////////////  version control

    gtk_widget_show_all (vbox);

    ///////////////////////// Desktop options
    gchar *tab_text = g_strdup_printf(" %s:", _("Options"));
    vbox = create_tab (notebook, _("Desktop"),  tab_text);
    g_free(tab_text);

    for(i = RFM_ENABLE_DESKTOP; i <= RFM_NAVIGATE_DESKTOP; i++) {
	gchar *text;
	if (i == RFM_NAVIGATE_DESKTOP) {
	    text = g_strdup_printf("%s (%s)", 
		_(environ_v[i].env_text), _("Allow"));
	} else if (i == RFM_ENABLE_DESKTOP) {
	    text = g_strdup_printf("%s (%s)", 
		_(environ_v[i].env_text), _("localhost"));
	} else {
	    text = g_strdup(_(environ_v[i].env_text));
	}
	toggle_button[i] = 
		gtk_check_button_new_with_label (text);
	g_free(text);
	
	g_object_set_data(G_OBJECT(toggle_button[i]), "settings_p", settings_p);
        g_signal_connect (toggle_button[i], 
		"toggled", G_CALLBACK (option_toggled), GINT_TO_POINTER(i));
        gtk_box_pack_start (GTK_BOX(vbox), toggle_button[i], FALSE, FALSE, 0);
	if (i==RFM_ENABLE_DESKTOP && !localhost_check()){
	    gtk_widget_set_sensitive(toggle_button[i], FALSE);
	}
    }

   // desktop image selection
    hbox = rfm_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    settings_p->desktopimage_button = make_file_chooser_button (RFM_DESKTOP_IMAGE, FALSE, hbox);
    g_object_set_data(G_OBJECT(settings_p->desktopimage_button), "settings_p", settings_p);
    gchar *g=g_strdup_printf("%s (%s)", _("Clear"), _("Background image"));
    GtkWidget *clear_button = rfm_mk_little_button ("xffm/stock_clear",
	    clear_bgimage, settings_p, g);
    gtk_widget_show(clear_button);
    g_free(g);
    gtk_box_pack_start (GTK_BOX(hbox), clear_button, FALSE, FALSE, 0);
    g_signal_connect (clear_button, 
	    "clicked", G_CALLBACK (value_clear), 
	    GINT_TO_POINTER(RFM_DESKTOP_IMAGE));


    // desktop directory selection
    hbox = rfm_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    label=gtk_label_new(_(environ_v[RFM_DESKTOP_DIR].env_text));
    settings_p->desktopdir_entry = gtk_entry_new();
    g_signal_connect (settings_p->desktopdir_entry, 
	    "activate", G_CALLBACK (deskdir_entry), settings_p);

    settings_p->desktopdir_button = 
	rfm_mk_little_button ("xffm/emblem_desktop",
	    deskdir_filechooser, settings_p, _("Select Folder"));
    g_object_set_data(G_OBJECT(settings_p->desktopdir_button), "settings_p", settings_p);
    gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX(hbox), settings_p->desktopdir_entry, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX(hbox), settings_p->desktopdir_button, FALSE, FALSE, 0);
    



     ////
    hbox = rfm_hbox_new (FALSE, 6);
    label = gtk_label_new (_(environ_v[RFM_DESKTOP_COLOR].env_text));
    gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
    settings_p->desktopcolor_button = gtk_color_button_new ();
    if(getenv ("RFM_DESKTOP_COLOR") && strlen (getenv ("RFM_DESKTOP_COLOR"))) {
#if GTK_MAJOR_VERSION==3 && GTK_MINOR_VERSION>=4
        GdkRGBA color;
        if(gdk_rgba_parse (&color, getenv ("RFM_DESKTOP_COLOR") )) {
	    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(settings_p->desktopcolor_button), &color);
        }

#else
        GdkColor color;
        if(gdk_color_parse (getenv ("RFM_DESKTOP_COLOR"), &color)) {
            gtk_color_button_set_color (
		    GTK_COLOR_BUTTON(settings_p->desktopcolor_button), &color);
        }
#endif
    }
    gtk_color_button_set_title ((GtkColorButton *) (settings_p->desktopcolor_button), (const gchar *)
                                _(environ_v[RFM_DESKTOP_COLOR].env_text));
    gtk_box_pack_start (GTK_BOX(hbox), settings_p->desktopcolor_button, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(settings_p->desktopcolor_button), "settings_p", settings_p);
    g_signal_connect (settings_p->desktopcolor_button, 
	    "color-set", G_CALLBACK (color_changed), GINT_TO_POINTER(RFM_DESKTOP_COLOR));

    // spin buttons: margins
    for (i = RFM_DESKTOP_TOP_MARGIN; i <= RFM_DESKTOP_LEFT_MARGIN; i++){
	hbox = rfm_hbox_new (FALSE, 6);
	label = gtk_label_new (_(environ_v[i].env_text));
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);
	settings_p->desktop_margin_spinbutton[i-RFM_DESKTOP_TOP_MARGIN] =
	    gtk_spin_button_new_with_range (0.0, 200.0, 1.0);
	gdouble value = get_spin_value(i);

	gtk_spin_button_set_value (
		GTK_SPIN_BUTTON(settings_p->desktop_margin_spinbutton[i-RFM_DESKTOP_TOP_MARGIN]),
                value);
	gtk_box_pack_start (GTK_BOX(hbox), settings_p->desktop_margin_spinbutton[i-RFM_DESKTOP_TOP_MARGIN], FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	g_object_set_data(
		G_OBJECT(settings_p->desktop_margin_spinbutton[i-RFM_DESKTOP_TOP_MARGIN]),
		"settings_p", settings_p);
	g_object_set_data(
		G_OBJECT(settings_p->desktop_margin_spinbutton[i-RFM_DESKTOP_TOP_MARGIN]),
		"which_margin", GINT_TO_POINTER(i));
        g_signal_connect (settings_p->desktop_margin_spinbutton[i-RFM_DESKTOP_TOP_MARGIN], 
    	    "value-changed", G_CALLBACK (margin_changed), GINT_TO_POINTER(i));
    }


    //////////////////////// Ls options
    tab_text = g_strdup_printf(" %s:", _("Options"));
    vbox = create_tab (notebook, "ls", tab_text);
    g_free(tab_text);
    subtitle(vbox, settings_dialog, _("File Information..."), "ls");

    mk_bit_toggles(settings_p, vbox, "ls_box", RFM_LS_FLAGS, LS_OPTIONS);
    gtk_widget_show_all (vbox);


    //////////////////////// Copy options
    tab_text = g_strdup_printf(" %s:", _("Options"));
    vbox = create_tab (notebook, "cp", tab_text);
    g_free(tab_text);
    subtitle(vbox, settings_dialog, _("Copy"), "cp");

    mk_bit_toggles(settings_p, vbox, "cp_box", RFM_CP_FLAGS, CP_OPTIONS);
    gtk_widget_show_all (vbox);

    
    //////////////////////// Move options
    tab_text = g_strdup_printf(" %s:", _("Options"));
    vbox = create_tab (notebook, "mv", tab_text);
    g_object_set_data(G_OBJECT(settings_p->dialog), "mv_box", vbox);
    g_free(tab_text);

    tab_text = g_strdup_printf("%s", _("Move"));
    subtitle(vbox, settings_dialog, tab_text, "mv");
    g_free(tab_text);
    mk_bit_toggles(settings_p, vbox, "mv_box", RFM_MV_FLAGS, MV_OPTIONS);
    gtk_widget_show_all (vbox);

    //////////////////////// link options
    tab_text = g_strdup_printf(" %s:", _("Options"));
    vbox = create_tab (notebook, "ln", tab_text);
    g_object_set_data(G_OBJECT(settings_p->dialog), "ln_box", vbox);
    g_free(tab_text);

    subtitle(vbox, settings_dialog, _("Symbolic Link"), "ln");
    mk_bit_toggles(settings_p, vbox, "ln_box", RFM_LN_FLAGS, LN_OPTIONS);

    gtk_widget_show_all (vbox);

    //////////////////////// remove options
    tab_text = g_strdup_printf(" %s:", _("Options"));
    vbox = create_tab (notebook, "rm", tab_text);
    g_object_set_data(G_OBJECT(settings_p->dialog), "rm_box", vbox);
    g_free(tab_text);



    subtitle(vbox, settings_dialog, _("Delete"), "rm");
    mk_bit_toggles(settings_p, vbox, "rm_box", RFM_RM_FLAGS, RM_OPTIONS);
    gtk_widget_show_all (vbox);

    //////////////////////// shred options
    DBG("testing for shred command...\n");
    gchar *shred_bin = g_find_program_in_path("shred");
    if (shred_bin) {
	DBG("shred bin is at %s\n", shred_bin);
	tab_text = g_strdup_printf(" %s:", _("Options"));
	vbox = create_tab (notebook, "shred", tab_text);
	g_object_set_data(G_OBJECT(settings_p->dialog), "shred_box", vbox);
	g_free(tab_text);
	subtitle(vbox, settings_dialog, _("Shred"), "shred");
	mk_bit_toggles(settings_p, vbox, "shred_box", RFM_SHRED_FLAGS, SHRED_OPTIONS);
	gtk_widget_show_all (vbox);
	g_free(shred_bin);
    }
#if 0
    // Plugin restriction is not very useful...
    // Just remove the plugin if you don't want it!
    //////////////////////// rodent plugins
    tab_text = g_strdup_printf(" %s:", _("Plugins"));
    vbox = create_tab (notebook, _("Plugins"), tab_text);
    g_object_set_data(G_OBJECT(settings_p->dialog), "plugins_box", vbox);
    g_free(tab_text);
    subtitle(vbox, settings_dialog, _("Rodent"), "rodent");
    // find plugins and modules and create toggles, or...
    // use a predefined toggle list as with cp.
    mk_bit_toggles(settings_p, vbox, "plugins_box", RFM_PLUGIN_FLAGS, rfm_get_lite_plugin_options());

    gtk_widget_show_all (vbox);
    //////////////////////// rodent modules
    tab_text = g_strdup_printf(" %s:", _("Modules"));
    vbox = create_tab (notebook, _("Modules"), tab_text);
    g_object_set_data(G_OBJECT(settings_p->dialog), "modules_box", vbox);
    g_free(tab_text);
    subtitle(vbox, settings_dialog, _("Rodent"), "rodent");
    // find plugins and modules and create toggles, or...
    // use a predefined toggle list as with cp.
    mk_bit_toggles(settings_p, vbox, "modules_box", RFM_MODULE_FLAGS, rfm_get_lite_module_options());

    gtk_widget_show_all (vbox);
#endif

    ///////////////////////////
    //    advanced options (environment variables)
    ///////////////////////////
     vbox = create_tab (notebook, _("Environment Variables"),
               _("Edit the list of environment variables and associated values"));

    GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request(sw, -1 , 275);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_show (sw);
    gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

    settings_p->model = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
    for(i = 0; i < RFM_OPTIONS; i++) {
        gtk_list_store_append (settings_p->model, &iter);
        gtk_list_store_set (settings_p->model, &iter,
                            COLUMN_VARIABLE,
                            g_strdup (rfm_options[i].name),
                            COLUMN_VALUE, g_strdup (rfm_options[i].value),
			    COLUMN_EDITABLE, TRUE, -1);
    }
    treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (settings_p->model));
    g_object_unref (G_OBJECT (settings_p->model));
    // gtk_tree_view_set_rules_hint is deprecated 
    // and WTF do we need it for anyways?
    // gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)), 
	    GTK_SELECTION_SINGLE);
    /* variable column */
    renderer = gtk_cell_renderer_text_new ();
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (environment_changed), settings_p);
    g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_VARIABLE);

    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 -1, _("Variable"), renderer, "text", 
						 COLUMN_VARIABLE, NULL);
    /* value column */
    renderer = gtk_cell_renderer_text_new ();
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (environment_changed), settings_p);
    g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_VALUE);

    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 -1, _("Value"), renderer, "text", 
						 COLUMN_VALUE, "editable", 
						 COLUMN_EDITABLE, NULL);

    gtk_widget_set_size_request (treeview, 300, 200);
    gtk_widget_show (treeview);
    gtk_container_add (GTK_CONTAINER (sw), treeview);

    set_option_buttons (settings_p);
    g_signal_connect (notebook, "switch-page", G_CALLBACK (switch_page), NULL);

    gtk_widget_set_size_request (settings_p->dialog, 650, 500);
    gtk_window_set_decorated (GTK_WINDOW (settings_p->dialog), TRUE);
    gtk_window_set_type_hint (GTK_WINDOW (settings_p->dialog), GDK_WINDOW_TYPE_HINT_NORMAL);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (settings_p->dialog), TRUE);
    gtk_window_stick (GTK_WINDOW (settings_p->dialog));
    gchar *title = g_strdup_printf ("%s %s (%s)",
            PACKAGE_NAME, TAG, _("Personal settings"));
    gtk_window_set_title (GTK_WINDOW (settings_p->dialog), title);
    g_free (title);
    GdkPixbuf *icon_pixbuf = rfm_get_pixbuf ("xffm/stock_preferences", SIZE_ICON);
    if(icon_pixbuf) {
        gtk_window_set_icon (GTK_WINDOW (settings_p->dialog), icon_pixbuf);
	g_object_unref(icon_pixbuf);
    }

#endif
    gtk_window_set_type_hint(GTK_WINDOW(settings_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_widget_show_all (settings_p->dialog);
    return NULL;
}


static
void *
options_dialog_f( void *data){
    void **arg = data;
    widgets_t *widgets_p = arg[0];
    const gchar *command = arg[1];
    if (!command) return NULL;
    settings_t *settings_p=(settings_t *)malloc(sizeof(settings_t));
    if (!settings_p) g_error("malloc: %s", strerror(errno));
    memset(settings_p, 0, sizeof(settings_t));
    RfmProgramOptions *options_p;
    const gchar *id = NULL;
    gint flags;
    if (strcmp(command, "cp")==0){
	options_p = CP_OPTIONS;
	id = "cp_box";
	flags = RFM_CP_FLAGS;
    } else if (strcmp(command, "ls")==0){
	options_p = LS_OPTIONS;
	id = "ls_box";
	flags = RFM_LS_FLAGS;
    } else if (strcmp(command, "mv")==0){
	options_p = MV_OPTIONS;
	id = "mv_box";
	flags = RFM_MV_FLAGS;
    } else if (strcmp(command, "ln")==0){
	options_p = LN_OPTIONS;
	id = "ln_box";
	flags = RFM_LN_FLAGS;
    } else if (strcmp(command, "rm")==0){
	options_p = RM_OPTIONS;
	id = "rm_box";
	flags = RFM_RM_FLAGS;
    } else if (strcmp(command, "shred")==0){
	options_p = SHRED_OPTIONS;
	id = "shred_box";
	flags = RFM_SHRED_FLAGS;
    }
    if (!id) {
	DBG("options_dialog(): no option_t for %s\n", command);
        g_free(settings_p);
	return NULL;
    }
    settings_p->dialog = gtk_dialog_new();
    gtk_window_set_type_hint(GTK_WINDOW(settings_p->dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    gchar *title = g_strdup_printf(_("Options: %s"), command);
    gtk_window_set_title(GTK_WINDOW(settings_p->dialog), title);
    g_free(title);
    GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG(settings_p->dialog));

    GtkLabel *label = GTK_LABEL(gtk_label_new(""));
    gchar *slabel = g_strdup_printf("<span foreground=\"blue\" background=\"#cccccc\" weight=\"bold\">%s</span>", _("Ask the user to get additional parameters"));
    gtk_label_set_markup(label, slabel);
    g_free(slabel);
    gtk_widget_show(GTK_WIDGET(label));
    gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(label), FALSE, FALSE, 0);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start (GTK_BOX (box), scrolled_window, TRUE, TRUE, 0);
    GtkWidget *vbox = rfm_vbox_new(FALSE,0);
    
	
    gtk_widget_set_size_request (settings_p->dialog, -1, 300);

#if GTK_MAJOR_VERSION==3 && GTK_MINOR_VERSION>=8
    gtk_container_add(GTK_CONTAINER(scrolled_window), vbox);

#else
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), vbox);
#endif
  
    mk_bit_toggles(settings_p, vbox, id, flags, options_p);
    set_bit_toggles(settings_p, id, options_p, flags);
    update_combo_entry(settings_p);

    // XXX create ok and cancel buttons 

    GtkWidget *button = rfm_dialog_button ("xffm/stock_cancel", _("Cancel"));
    gtk_widget_show (button);
    gtk_dialog_add_action_widget (GTK_DIALOG (settings_p->dialog), button, GTK_RESPONSE_NO);
    g_object_set_data (G_OBJECT (settings_p->dialog), "action_false_button", button);
    
    button = rfm_dialog_button ("xffm/stock_ok", _("Ok"));
    gtk_widget_show (button);
    g_object_set_data (G_OBJECT (settings_p->dialog), "action_true_button", button);
    gtk_dialog_add_action_widget (GTK_DIALOG (settings_p->dialog), button, GTK_RESPONSE_YES);

    if(widgets_p) {
	view_t *view_p=widgets_p->view_p;
        if(view_p && view_p->flags.type == DESKVIEW_TYPE) {
	    gtk_window_set_keep_above (GTK_WINDOW(settings_p->dialog), TRUE);
	    gtk_window_stick (GTK_WINDOW(settings_p->dialog));
	} else {   
            rfm_global_t *rfm_global_p = rfm_global();
            gtk_window_set_modal (GTK_WINDOW (settings_p->dialog), TRUE);
            if(rfm_global_p) gtk_window_set_transient_for (GTK_WINDOW (settings_p->dialog), GTK_WINDOW (rfm_global_p->window));
        } 
    } else {
	gtk_window_set_modal (GTK_WINDOW (settings_p->dialog), TRUE);
    }
    
    gtk_widget_show_all(settings_p->dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(settings_p->dialog));
    gtk_widget_hide(settings_p->dialog);
    gtk_widget_destroy (settings_p->dialog);    
    g_free(settings_p);
    if(response == GTK_RESPONSE_YES) return GINT_TO_POINTER(TRUE);
        
    return GINT_TO_POINTER(FALSE);
}

#if 0
// gtk 3 only. but not used as of yet...
GdkRGBA X11_colors[]={
 {0.3882,0.7216,1.0000,1.0}, // SteelBlue1 (1)
 {0.8588,0.8588,0.8588,1.0}, // grey86 (2)
 {0.3608,0.6745,0.9333,1.0}, // SteelBlue2 (3)
 {0.4627,0.9333,0.0000,1.0}, // chartreuse2 (4)
 {0.4980,1.0000,0.8314,1.0}, // aquamarine1 (5)
 {0.8902,0.8902,0.8902,1.0}, // grey89 (6)
 {0.5451,0.2275,0.2275,1.0}, // IndianRed4 (7)
 {1.0000,0.9255,0.5451,1.0}, // LightGoldenrod1 (8)
 {0.5451,0.4941,0.4000,1.0}, // wheat4 (9)
 {0.5451,0.3882,0.4235,1.0}, // pink4 (10)
 {0.5451,0.3961,0.0314,1.0}, // DarkGoldenrod4 (11)
 {1.0000,0.6275,0.4784,1.0}, // LightSalmon1 (12)
 {0.5137,0.5451,0.5451,1.0}, // azure4 (13)
 {0.7294,0.7294,0.7294,1.0}, // grey73 (14)
 {0.4784,0.4039,0.9333,1.0}, // SlateBlue2 (15)
 {1.0000,0.5137,0.9804,1.0}, // orchid1 (16)
 {0.0627,0.3059,0.5451,1.0}, // DodgerBlue4 (17)
 {0.5451,0.5333,0.4706,1.0}, // cornsilk4 (18)
 {0.3725,0.6196,0.6275,1.0}, // CadetBlue (19)
 {0.9412,0.9020,0.5490,1.0}, // khaki (20)
 {0.2314,0.2314,0.2314,1.0}, // grey23 (21)
 {0.9333,0.6824,0.9333,1.0}, // plum2 (22)
 {0.0000,0.6980,0.9333,1.0}, // DeepSkyBlue2 (23)
 {1.0000,0.4314,0.7059,1.0}, // HotPink1 (24)
 {0.5412,0.1686,0.8863,1.0}, // BlueViolet (25)
 {0.7412,0.7176,0.4196,1.0}, // DarkKhaki (26)
 {0.8039,0.1961,0.4706,1.0}, // VioletRed3 (27)
 {0.7569,0.8039,0.8039,1.0}, // azure3 (28)
 {0.9294,0.9294,0.9294,1.0}, // grey93 (29)
 {0.1333,0.5451,0.1333,1.0}, // ForestGreen (30)
 {0.4902,0.1490,0.8039,1.0}, // purple3 (31)
 {0.5608,0.7373,0.5608,1.0}, // DarkSeaGreen (32)
 {0.0000,0.5255,0.5451,1.0}, // turquoise4 (33)
 {0.9412,0.9725,1.0000,1.0}, // AliceBlue (34)
 {0.9882,0.9882,0.9882,1.0}, // grey99 (35)
 {0.1608,0.1608,0.1608,1.0}, // grey16 (36)
 {1.0000,0.4980,0.3137,1.0}, // coral (37)
 {0.9804,0.9412,0.9020,1.0}, // linen (38)
 {0.4000,0.8039,0.6667,1.0}, // aquamarine3 (39)
 {0.2196,0.2196,0.2196,1.0}, // grey22 (40)
 {1.0000,0.8275,0.6078,1.0}, // burlywood1 (41)
 {0.8667,0.6275,0.8667,1.0}, // plum (42)
 {0.9333,0.9020,0.5216,1.0}, // khaki2 (43)
 {0.9333,0.7961,0.6784,1.0}, // PeachPuff2 (44)
 {0.6784,1.0000,0.1843,1.0}, // GreenYellow (45)
 {0.5490,0.5490,0.5490,1.0}, // grey55 (46)
 {0.9333,0.6039,0.2863,1.0}, // tan2 (47)
 {0.3765,0.4824,0.5451,1.0}, // LightSkyBlue4 (48)
 {0.5451,0.5451,0.4784,1.0}, // LightYellow4 (49)
 {0.0000,0.0000,0.5451,1.0}, // DarkBlue (50)
 {0.6275,0.3216,0.1765,1.0}, // sienna (51)
 {1.0000,1.0000,0.0000,1.0}, // yellow1 (52)
 {0.0588,0.0588,0.0588,1.0}, // grey6 (53)
 {0.6902,0.6902,0.6902,1.0}, // grey69 (54)
 {0.6706,0.5098,1.0000,1.0}, // MediumPurple1 (55)
 {0.0000,0.7725,0.8039,1.0}, // turquoise3 (56)
 {0.9333,0.4784,0.9137,1.0}, // orchid2 (57)
 {0.0000,0.9608,1.0000,1.0}, // turquoise1 (58)
 {0.2706,0.2706,0.2706,1.0}, // grey27 (59)
 {0.8039,0.4000,0.0000,1.0}, // DarkOrange3 (60)
 {0.8039,0.6784,0.0000,1.0}, // gold3 (61)
 {0.9333,0.3608,0.2588,1.0}, // tomato2 (62)
 {0.7922,0.8824,1.0000,1.0}, // LightSteelBlue1 (63)
 {0.6902,0.8784,0.9020,1.0}, // PowderBlue (64)
 {0.5451,0.2706,0.0000,1.0}, // DarkOrange4 (65)
 {1.0000,0.7137,0.7569,1.0}, // LightPink (66)
 {0.9333,0.4627,0.0000,1.0}, // DarkOrange2 (67)
 {0.1255,0.6980,0.6667,1.0}, // LightSeaGreen (68)
 {0.8392,0.8392,0.8392,1.0}, // grey84 (69)
 {0.9333,0.1725,0.1725,1.0}, // firebrick2 (70)
 {0.4863,0.8039,0.4863,1.0}, // PaleGreen3 (71)
 {0.6235,0.7137,0.8039,1.0}, // SlateGray3 (72)
 {0.5294,0.8078,0.9804,1.0}, // LightSkyBlue (73)
 {0.5137,0.4353,1.0000,1.0}, // SlateBlue1 (74)
 {0.2706,0.5451,0.0000,1.0}, // chartreuse4 (75)
 {0.3255,0.5255,0.5451,1.0}, // CadetBlue4 (76)
 {0.5451,0.1098,0.3843,1.0}, // maroon4 (77)
 {1.0000,0.5098,0.6706,1.0}, // PaleVioletRed1 (78)
 {0.8980,0.8980,0.8980,1.0}, // grey90 (79)
 {0.6078,0.1882,1.0000,1.0}, // purple1 (80)
 {0.7569,1.0000,0.7569,1.0}, // DarkSeaGreen1 (81)
 {0.2902,0.4392,0.5451,1.0}, // SkyBlue4 (82)
 {0.6980,0.8745,0.9333,1.0}, // LightBlue2 (83)
 {0.5882,0.5882,0.5882,1.0}, // grey59 (84)
 {0.3294,1.0000,0.6235,1.0}, // SeaGreen1 (85)
 {0.4000,0.5451,0.5451,1.0}, // PaleTurquoise4 (86)
 {1.0000,1.0000,0.8784,1.0}, // LightYellow1 (87)
 {0.5216,0.5216,0.5216,1.0}, // grey52 (88)
 {0.8471,0.7490,0.8471,1.0}, // thistle (89)
 {0.3647,0.2784,0.5451,1.0}, // MediumPurple4 (90)
 {0.9333,0.4157,0.6549,1.0}, // HotPink2 (91)
 {0.0000,0.9333,0.9333,1.0}, // cyan2 (92)
 {1.0000,0.0000,0.0000,1.0}, // red1 (93)
 {0.6980,0.2275,0.9333,1.0}, // DarkOrchid2 (94)
 {0.8706,0.8706,0.8706,1.0}, // grey87 (95)
 {0.4784,0.7725,0.8039,1.0}, // CadetBlue3 (96)
 {0.9608,0.8706,0.7020,1.0}, // wheat (97)
 {0.4627,0.9333,0.7765,1.0}, // aquamarine2 (98)
 {0.0000,0.0000,0.5020,1.0}, // NavyBlue (99)
 {0.0314,0.0314,0.0314,1.0}, // grey3 (100)
 {1.0000,0.2706,0.0000,1.0}, // OrangeRed1 (101)
 {1.0000,0.9608,0.9333,1.0}, // seashell1 (102)
 {0.8039,0.6078,0.1137,1.0}, // goldenrod3 (103)
 {1.0000,0.3882,0.2784,1.0}, // tomato1 (104)
 {0.5451,0.2784,0.3647,1.0}, // PaleVioletRed4 (105)
 {0.9333,0.6353,0.6784,1.0}, // LightPink2 (106)
 {0.4980,1.0000,0.0000,1.0}, // chartreuse1 (107)
 {0.9333,0.8980,0.8706,1.0}, // seashell2 (108)
 {0.4667,0.5333,0.6000,1.0}, // LightSlateGrey (109)
 {0.2588,0.2588,0.2588,1.0}, // grey26 (110)
 {0.8549,0.6471,0.1255,1.0}, // goldenrod (111)
 {0.6275,0.1255,0.9412,1.0}, // purple (112)
 {0.6902,0.8863,1.0000,1.0}, // LightSkyBlue1 (113)
 {0.3608,0.3608,0.3608,1.0}, // grey36 (114)
 {0.8039,0.5216,0.2471,1.0}, // tan3 (115)
 {0.8039,0.1490,0.1490,1.0}, // firebrick3 (116)
 {0.9333,0.9098,0.8039,1.0}, // cornsilk2 (117)
 {0.5451,0.1373,0.1373,1.0}, // brown4 (118)
 {0.9333,0.6627,0.7216,1.0}, // pink2 (119)
 {0.7490,0.9373,1.0000,1.0}, // LightBlue1 (120)
 {0.4392,0.5020,0.5647,1.0}, // SlateGrey (121)
 {0.9333,0.8118,0.6314,1.0}, // NavajoWhite2 (122)
 {0.8706,0.7216,0.5294,1.0}, // burlywood (123)
 {0.6784,0.8471,0.9020,1.0}, // LightBlue (124)
 {1.0000,0.9804,0.9804,1.0}, // snow1 (125)
 {0.6039,0.7529,0.8039,1.0}, // LightBlue3 (126)
 {0.7098,0.7098,0.7098,1.0}, // grey71 (127)
 {0.8039,0.2000,0.2000,1.0}, // brown3 (128)
 {0.8039,0.6667,0.4902,1.0}, // burlywood3 (129)
 {0.8039,0.5686,0.6196,1.0}, // pink3 (130)
 {0.5451,0.2118,0.1490,1.0}, // tomato4 (131)
 {1.0000,0.7569,0.1451,1.0}, // goldenrod1 (132)
 {0.9569,0.6431,0.3765,1.0}, // SandyBrown (133)
 {0.7608,0.7608,0.7608,1.0}, // grey76 (134)
 {0.0000,1.0000,0.4980,1.0}, // SpringGreen1 (135)
 {0.4157,0.3529,0.8039,1.0}, // SlateBlue (136)
 {0.7294,0.3333,0.8275,1.0}, // MediumOrchid (137)
 {0.5098,0.5098,0.5098,1.0}, // grey51 (138)
 {0.2510,0.8784,0.8157,1.0}, // turquoise (139)
 {0.4784,0.4784,0.4784,1.0}, // grey48 (140)
 {0.9333,0.4745,0.6235,1.0}, // PaleVioletRed2 (141)
 {0.9490,0.9490,0.9490,1.0}, // grey95 (142)
 {0.1020,0.1020,0.1020,1.0}, // grey10 (143)
 {0.0196,0.0196,0.0196,1.0}, // grey2 (144)
 {1.0000,0.5490,0.4118,1.0}, // salmon1 (145)
 {0.9608,1.0000,0.9804,1.0}, // MintCream (146)
 {0.6706,0.6706,0.6706,1.0}, // grey67 (147)
 {0.5451,0.0000,0.5451,1.0}, // DarkMagenta (148)
 {0.8039,0.5059,0.3843,1.0}, // LightSalmon3 (149)
 {0.8039,0.4078,0.5373,1.0}, // PaleVioletRed3 (150)
 {0.6078,0.8039,0.6078,1.0}, // DarkSeaGreen3 (151)
 {0.3490,0.3490,0.3490,1.0}, // grey35 (152)
 {1.0000,0.4157,0.4157,1.0}, // IndianRed1 (153)
 {0.9333,0.7059,0.7059,1.0}, // RosyBrown2 (154)
 {0.9333,0.9333,0.8196,1.0}, // LightYellow2 (155)
 {0.9333,0.3882,0.3882,1.0}, // IndianRed2 (156)
 {0.4784,0.5451,0.5451,1.0}, // LightCyan4 (157)
 {0.9333,0.1882,0.6549,1.0}, // maroon2 (158)
 {0.7020,0.9333,0.2275,1.0}, // OliveDrab2 (159)
 {0.0000,0.0000,0.9333,1.0}, // blue2 (160)
 {0.8784,0.9333,0.9333,1.0}, // azure2 (161)
 {0.4784,0.2157,0.5451,1.0}, // MediumOrchid4 (162)
 {0.5765,0.4392,0.8588,1.0}, // MediumPurple (163)
 {0.5686,0.1725,0.9333,1.0}, // purple2 (164)
 {0.9333,0.2275,0.5490,1.0}, // VioletRed2 (165)
 {0.9333,0.8784,0.8980,1.0}, // LavenderBlush2 (166)
 {0.3922,0.5843,0.9294,1.0}, // CornflowerBlue (167)
 {0.4863,0.9882,0.0000,1.0}, // LawnGreen (168)
 {1.0000,0.2431,0.5882,1.0}, // VioletRed1 (169)
 {0.1294,0.1294,0.1294,1.0}, // grey13 (170)
 {1.0000,0.9647,0.5608,1.0}, // khaki1 (171)
 {0.9333,0.9333,0.8784,1.0}, // ivory2 (172)
 {0.4588,0.4588,0.4588,1.0}, // grey46 (173)
 {0.5686,0.5686,0.5686,1.0}, // grey57 (174)
 {0.3333,0.1020,0.5451,1.0}, // purple4 (175)
 {0.8039,0.7176,0.7098,1.0}, // MistyRose3 (176)
 {0.6588,0.6588,0.6588,1.0}, // grey66 (177)
 {0.9922,0.9608,0.9020,1.0}, // OldLace (178)
 {0.7569,0.8039,0.7569,1.0}, // honeydew3 (179)
 {0.9412,0.9412,0.9412,1.0}, // grey94 (180)
 {0.7412,0.7412,0.7412,1.0}, // grey74 (181)
 {0.7020,0.7020,0.7020,1.0}, // grey70 (182)
 {0.6353,0.8039,0.3529,1.0}, // DarkOliveGreen3 (183)
 {0.4941,0.7529,0.9333,1.0}, // SkyBlue2 (184)
 {0.0000,1.0000,1.0000,1.0}, // cyan1 (185)
 {0.8039,0.7098,0.8039,1.0}, // thistle3 (186)
 {0.5451,0.0392,0.3137,1.0}, // DeepPink4 (187)
 {0.2275,0.3725,0.8039,1.0}, // RoyalBlue3 (188)
 {0.0706,0.0706,0.0706,1.0}, // grey7 (189)
 {0.0000,0.9804,0.6039,1.0}, // MediumSpringGreen (190)
 {0.7216,0.5255,0.0431,1.0}, // DarkGoldenrod (191)
 {0.8039,0.4078,0.2235,1.0}, // sienna3 (192)
 {0.8196,0.3725,0.9333,1.0}, // MediumOrchid2 (193)
 {0.5451,0.1333,0.3216,1.0}, // VioletRed4 (194)
 {0.8039,0.3608,0.3608,1.0}, // IndianRed (195)
 {0.6392,0.6392,0.6392,1.0}, // grey64 (196)
 {0.0000,0.5451,0.2706,1.0}, // SpringGreen4 (197)
 {0.9333,0.8745,0.8000,1.0}, // AntiqueWhite2 (198)
 {0.8039,0.3765,0.5647,1.0}, // HotPink3 (199)
 {0.4392,0.4392,0.4392,1.0}, // grey44 (200)
 {0.3294,0.3294,0.3294,1.0}, // grey33 (201)
 {0.1882,0.1882,0.1882,1.0}, // grey19 (202)
 {0.7059,0.8039,0.8039,1.0}, // LightCyan3 (203)
 {0.2824,0.2392,0.5451,1.0}, // DarkSlateBlue (204)
 {0.0000,0.6039,0.8039,1.0}, // DeepSkyBlue3 (205)
 {0.9333,0.7882,0.0000,1.0}, // gold2 (206)
 {0.5451,0.3412,0.2588,1.0}, // LightSalmon4 (207)
 {0.5451,0.3529,0.1686,1.0}, // tan4 (208)
 {1.0000,0.8941,0.7686,1.0}, // bisque1 (209)
 {0.4314,0.5451,0.2392,1.0}, // DarkOliveGreen4 (210)
 {0.5529,0.9333,0.9333,1.0}, // DarkSlateGray2 (211)
 {1.0000,0.1882,0.1882,1.0}, // firebrick1 (212)
 {0.6235,0.4745,0.9333,1.0}, // MediumPurple2 (213)
 {0.5451,0.1020,0.1020,1.0}, // firebrick4 (214)
 {0.4314,0.4314,0.4314,1.0}, // grey43 (215)
 {0.5451,0.0000,0.0000,1.0}, // DarkRed (216)
 {0.6902,0.7686,0.8706,1.0}, // LightSteelBlue (217)
 {0.3882,0.3882,0.3882,1.0}, // grey39 (218)
 {0.8039,0.4392,0.3294,1.0}, // salmon3 (219)
 {0.6784,0.6784,0.6784,1.0}, // grey68 (220)
 {0.9333,0.7059,0.1333,1.0}, // goldenrod2 (221)
 {0.8627,0.8627,0.8627,1.0}, // gainsboro (222)
 {0.5569,0.8980,0.9333,1.0}, // CadetBlue2 (223)
 {1.0000,0.0784,0.5765,1.0}, // DeepPink1 (224)
 {0.3333,0.4196,0.1843,1.0}, // DarkOliveGreen (225)
 {0.2784,0.2353,0.5451,1.0}, // SlateBlue4 (226)
 {0.8039,0.5216,0.0000,1.0}, // orange3 (227)
 {1.0000,0.8941,0.7098,1.0}, // moccasin (228)
 {0.5294,0.8078,0.9216,1.0}, // SkyBlue (229)
 {0.7373,0.5608,0.5608,1.0}, // RosyBrown (230)
 {0.6431,0.8275,0.9333,1.0}, // LightSkyBlue2 (231)
 {0.9804,0.9804,0.8235,1.0}, // LightGoldenrodYellow (232)
 {0.9333,0.6039,0.0000,1.0}, // orange2 (233)
 {0.7490,0.2431,1.0000,1.0}, // DarkOrchid1 (234)
 {0.7529,1.0000,0.2431,1.0}, // OliveDrab1 (235)
 {1.0000,0.6824,0.7255,1.0}, // LightPink1 (236)
 {0.9333,0.4745,0.2588,1.0}, // sienna2 (237)
 {0.8118,0.8118,0.8118,1.0}, // grey81 (238)
 {1.0000,0.7529,0.7961,1.0}, // pink (239)
 {1.0000,0.4980,0.0000,1.0}, // DarkOrange1 (240)
 {1.0000,0.7569,0.7569,1.0}, // RosyBrown1 (241)
 {0.8039,0.4000,0.1137,1.0}, // chocolate3 (242)
 {1.0000,0.7098,0.7725,1.0}, // pink1 (243)
 {0.9333,0.8353,0.8235,1.0}, // MistyRose2 (244)
 {1.0000,0.6471,0.3098,1.0}, // tan1 (245)
 {1.0000,0.8549,0.7255,1.0}, // PeachPuff1 (246)
 {0.0000,0.7490,1.0000,1.0}, // DeepSkyBlue1 (247)
 {0.9412,1.0000,0.9412,1.0}, // honeydew1 (248)
 {0.7686,0.7686,0.7686,1.0}, // grey77 (249)
 {0.5451,0.2784,0.1490,1.0}, // sienna4 (250)
 {0.5451,0.5059,0.2980,1.0}, // LightGoldenrod4 (251)
 {0.2000,0.2000,0.2000,1.0}, // grey20 (252)
 {0.9804,0.9216,0.8431,1.0}, // AntiqueWhite (253)
 {0.0118,0.0118,0.0118,1.0}, // grey1 (254)
 {1.0000,1.0000,1.0000,1.0}, // grey100 (255)
 {0.8275,0.8275,0.8275,1.0}, // LightGray (256)
 {0.2745,0.5098,0.7059,1.0}, // SteelBlue (257)
 {0.9412,0.5020,0.5020,1.0}, // LightCoral (258)
 {0.9333,0.8667,0.5098,1.0}, // LightGoldenrod (259)
 {0.9333,0.0706,0.5373,1.0}, // DeepPink2 (260)
 {0.8510,0.8510,0.8510,1.0}, // grey85 (261)
 {0.8588,0.4392,0.5765,1.0}, // PaleVioletRed (262)
 {0.8039,0.4118,0.7882,1.0}, // orchid3 (263)
 {0.2784,0.2784,0.2784,1.0}, // grey28 (264)
 {0.0510,0.0510,0.0510,1.0}, // grey5 (265)
 {0.8039,0.8039,0.0000,1.0}, // yellow3 (266)
 {0.4980,0.4980,0.4980,1.0}, // grey50 (267)
 {0.5451,0.5451,0.5137,1.0}, // ivory4 (268)
 {0.5451,0.2431,0.1843,1.0}, // coral4 (269)
 {0.2706,0.5451,0.4549,1.0}, // aquamarine4 (270)
 {0.9333,0.8471,0.6824,1.0}, // wheat2 (271)
 {0.7333,1.0000,1.0000,1.0}, // PaleTurquoise1 (272)
 {0.0902,0.0902,0.0902,1.0}, // grey9 (273)
 {0.1098,0.1098,0.1098,1.0}, // grey11 (274)
 {0.9608,0.9608,0.8627,1.0}, // beige (275)
 {0.6980,0.1333,0.1333,1.0}, // firebrick (276)
 {0.8039,0.7176,0.6196,1.0}, // bisque3 (277)
 {0.9333,0.0000,0.9333,1.0}, // magenta2 (278)
 {1.0000,0.4471,0.3373,1.0}, // coral1 (279)
 {0.0000,0.0000,1.0000,1.0}, // blue1 (280)
 {0.3098,0.5804,0.8039,1.0}, // SteelBlue3 (281)
 {0.1961,0.8039,0.1961,1.0}, // LimeGreen (282)
 {0.0000,0.9333,0.4627,1.0}, // SpringGreen2 (283)
 {0.9333,0.4157,0.3137,1.0}, // coral2 (284)
 {1.0000,0.4118,0.7059,1.0}, // HotPink (285)
 {0.9333,0.5098,0.3843,1.0}, // salmon2 (286)
 {1.0000,1.0000,0.9412,1.0}, // ivory1 (287)
 {0.8784,1.0000,1.0000,1.0}, // LightCyan1 (288)
 {0.6039,1.0000,0.6039,1.0}, // PaleGreen1 (289)
 {0.9333,0.4627,0.1294,1.0}, // chocolate2 (290)
 {0.8235,0.7059,0.5490,1.0}, // tan (291)
 {0.8235,0.4118,0.1176,1.0}, // chocolate (292)
 {0.6471,0.1647,0.1647,1.0}, // brown (293)
 {0.8039,0.8039,0.7569,1.0}, // ivory3 (294)
 {0.6510,0.6510,0.6510,1.0}, // grey65 (295)
 {0.4118,0.5451,0.4118,1.0}, // DarkSeaGreen4 (296)
 {0.7922,1.0000,0.4392,1.0}, // DarkOliveGreen1 (297)
 {0.4118,0.4118,0.4118,1.0}, // grey41 (298)
 {1.0000,0.9373,0.8588,1.0}, // AntiqueWhite1 (299)
 {0.6039,0.8039,0.1961,1.0}, // OliveDrab3 (300)
 {0.8039,0.5490,0.5843,1.0}, // LightPink3 (301)
 {0.5451,0.4902,0.4196,1.0}, // bisque4 (302)
 {0.2627,0.8039,0.5020,1.0}, // SeaGreen3 (303)
 {0.0000,0.5451,0.0000,1.0}, // green4 (304)
 {0.8039,0.7765,0.4510,1.0}, // khaki3 (305)
 {0.1843,0.3098,0.3098,1.0}, // DarkSlateGrey (306)
 {0.6353,0.7098,0.8039,1.0}, // LightSteelBlue3 (307)
 {0.5451,0.4824,0.5451,1.0}, // thistle4 (308)
 {0.9333,0.2510,0.0000,1.0}, // OrangeRed2 (309)
 {1.0000,0.2510,0.2510,1.0}, // brown1 (310)
 {0.1176,0.5647,1.0000,1.0}, // DodgerBlue1 (311)
 {0.0000,1.0000,0.0000,1.0}, // green1 (312)
 {0.4078,0.1333,0.5451,1.0}, // DarkOrchid4 (313)
 {0.3216,0.3216,0.3216,1.0}, // grey32 (314)
 {0.4196,0.5569,0.1373,1.0}, // OliveDrab (315)
 {0.0941,0.4549,0.8039,1.0}, // DodgerBlue3 (316)
 {0.8039,0.5882,0.8039,1.0}, // plum3 (317)
 {0.8039,0.3569,0.2706,1.0}, // coral3 (318)
 {0.0000,0.0000,0.0000,1.0}, // grey0 (319)
 {0.5922,1.0000,1.0000,1.0}, // DarkSlateGray1 (320)
 {0.9333,0.5098,0.9333,1.0}, // violet (321)
 {0.4235,0.4824,0.5451,1.0}, // SlateGray4 (322)
 {0.9333,0.2314,0.2314,1.0}, // brown2 (323)
 {0.5451,0.2980,0.2235,1.0}, // salmon4 (324)
 {0.0000,0.0000,0.8039,1.0}, // blue3 (325)
 {1.0000,0.9804,0.8039,1.0}, // LemonChiffon1 (326)
 {0.8039,0.7725,0.7490,1.0}, // seashell3 (327)
 {1.0000,0.6471,0.0000,1.0}, // orange1 (328)
 {0.8039,0.1608,0.5647,1.0}, // maroon3 (329)
 {0.9333,0.6784,0.0549,1.0}, // DarkGoldenrod2 (330)
 {0.9608,0.9608,0.9608,1.0}, // grey96 (331)
 {0.2118,0.2118,0.2118,1.0}, // grey21 (332)
 {0.3294,0.5451,0.3294,1.0}, // PaleGreen4 (333)
 {0.9333,0.8235,0.9333,1.0}, // thistle2 (334)
 {0.4196,0.4196,0.4196,1.0}, // grey42 (335)
 {1.0000,0.9412,0.9608,1.0}, // LavenderBlush1 (336)
 {0.5294,0.8078,1.0000,1.0}, // SkyBlue1 (337)
 {0.8039,0.7294,0.5882,1.0}, // wheat3 (338)
 {0.5451,0.4667,0.3961,1.0}, // PeachPuff4 (339)
 {0.2824,0.4627,1.0000,1.0}, // RoyalBlue1 (340)
 {0.5451,0.2784,0.5373,1.0}, // orchid4 (341)
 {0.8784,0.9333,0.8784,1.0}, // honeydew2 (342)
 {0.5804,0.0000,0.8275,1.0}, // DarkViolet (343)
 {0.0000,0.3922,0.0000,1.0}, // DarkGreen (344)
 {1.0000,0.5490,0.0000,1.0}, // DarkOrange (345)
 {0.5647,0.9333,0.5647,1.0}, // LightGreen (346)
 {1.0000,0.9059,0.7294,1.0}, // wheat1 (347)
 {0.2392,0.2392,0.2392,1.0}, // grey24 (348)
 {0.6196,0.6196,0.6196,1.0}, // grey62 (349)
 {0.8039,0.0000,0.0000,1.0}, // red3 (350)
 {0.5451,0.5255,0.3059,1.0}, // khaki4 (351)
 {0.8039,0.7843,0.6941,1.0}, // cornsilk3 (352)
 {0.8000,0.8000,0.8000,1.0}, // grey80 (353)
 {0.1490,0.1490,0.1490,1.0}, // grey15 (354)
 {0.5451,0.3725,0.3961,1.0}, // LightPink4 (355)
 {0.8039,0.7529,0.6902,1.0}, // AntiqueWhite3 (356)
 {0.8196,0.8196,0.8196,1.0}, // grey82 (357)
 {0.9412,1.0000,1.0000,1.0}, // azure1 (358)
 {0.5451,0.4000,0.5451,1.0}, // plum4 (359)
 {0.6118,0.6118,0.6118,1.0}, // grey61 (360)
 {0.9020,0.9020,0.9804,1.0}, // lavender (361)
 {0.5608,0.5608,0.5608,1.0}, // grey56 (362)
 {0.4510,0.4510,0.4510,1.0}, // grey45 (363)
 {0.3804,0.3804,0.3804,1.0}, // grey38 (364)
 {0.8039,0.7569,0.7725,1.0}, // LavenderBlush3 (365)
 {0.5882,0.8039,0.8039,1.0}, // PaleTurquoise3 (366)
 {0.8157,0.1255,0.5647,1.0}, // VioletRed (367)
 {0.6863,0.9333,0.9333,1.0}, // PaleTurquoise (368)
 {0.9098,0.9098,0.9098,1.0}, // grey91 (369)
 {0.8039,0.6863,0.5843,1.0}, // PeachPuff3 (370)
 {0.5451,0.5137,0.5255,1.0}, // LavenderBlush4 (371)
 {0.7490,0.7490,0.7490,1.0}, // grey75 (372)
 {0.4902,0.4902,0.4902,1.0}, // grey49 (373)
 {0.3098,0.3098,0.3098,1.0}, // grey31 (374)
 {0.0784,0.0784,0.0784,1.0}, // grey8 (375)
 {0.2824,0.8196,0.8000,1.0}, // MediumTurquoise (376)
 {0.9333,0.9137,0.7490,1.0}, // LemonChiffon2 (377)
 {0.1216,0.1216,0.1216,1.0}, // grey12 (378)
 {0.5294,0.5294,0.5294,1.0}, // grey53 (379)
 {0.5961,0.9843,0.5961,1.0}, // PaleGreen (380)
 {0.4745,0.8039,0.8039,1.0}, // DarkSlateGray3 (381)
 {0.0000,0.8039,0.4000,1.0}, // SpringGreen3 (382)
 {0.1686,0.1686,0.1686,1.0}, // grey17 (383)
 {0.5451,0.4510,0.3333,1.0}, // burlywood4 (384)
 {0.2353,0.7020,0.4431,1.0}, // MediumSeaGreen (385)
 {0.8196,0.9333,0.9333,1.0}, // LightCyan2 (386)
 {0.4000,0.8039,0.0000,1.0}, // chartreuse3 (387)
 {0.5451,0.4118,0.4118,1.0}, // RosyBrown4 (388)
 {0.5451,0.4588,0.0000,1.0}, // gold4 (389)
 {0.5451,0.5255,0.5098,1.0}, // seashell4 (390)
 {0.5451,0.3529,0.0000,1.0}, // orange4 (391)
 {0.4118,0.5451,0.1333,1.0}, // OliveDrab4 (392)
 {0.0000,0.9333,0.0000,1.0}, // green2 (393)
 {0.7451,0.7451,0.7451,1.0}, // grey (394)
 {0.9725,0.9725,1.0000,1.0}, // GhostWhite (395)
 {0.0000,0.4078,0.5451,1.0}, // DeepSkyBlue4 (396)
 {0.5451,0.4118,0.0784,1.0}, // goldenrod4 (397)
 {1.0000,0.4980,0.1412,1.0}, // chocolate1 (398)
 {1.0000,0.5098,0.2784,1.0}, // sienna1 (399)
 {0.5451,0.4745,0.3686,1.0}, // NavajoWhite4 (400)
 {0.1098,0.5255,0.9333,1.0}, // DodgerBlue2 (401)
 {0.5412,0.5412,0.5412,1.0}, // grey54 (402)
 {0.4706,0.4706,0.4706,1.0}, // grey47 (403)
 {0.9804,0.5020,0.4471,1.0}, // salmon (404)
 {0.5451,0.4902,0.4824,1.0}, // MistyRose4 (405)
 {0.9804,0.9804,0.9804,1.0}, // grey98 (406)
 {0.3686,0.3686,0.3686,1.0}, // grey37 (407)
 {0.5137,0.5451,0.5137,1.0}, // honeydew4 (408)
 {0.5451,0.2275,0.3843,1.0}, // HotPink4 (409)
 {0.5451,0.2706,0.0745,1.0}, // chocolate4 (410)
 {0.8039,0.7882,0.7882,1.0}, // snow3 (411)
 {0.5451,0.5451,0.0000,1.0}, // yellow4 (412)
 {0.1804,0.5451,0.3412,1.0}, // SeaGreen4 (413)
 {1.0000,0.2039,0.7020,1.0}, // maroon1 (414)
 {0.5804,0.5804,0.5804,1.0}, // grey58 (415)
 {1.0000,0.9216,0.8039,1.0}, // BlanchedAlmond (416)
 {0.9333,0.9137,0.9137,1.0}, // snow2 (417)
 {0.9686,0.9686,0.9686,1.0}, // grey97 (418)
 {0.0000,0.8980,0.9333,1.0}, // turquoise2 (419)
 {1.0000,0.0000,1.0000,1.0}, // magenta1 (420)
 {1.0000,0.8706,0.6784,1.0}, // NavajoWhite1 (421)
 {0.1529,0.2510,0.5451,1.0}, // RoyalBlue4 (422)
 {0.2118,0.3922,0.5451,1.0}, // SteelBlue4 (423)
 {0.4314,0.4824,0.5451,1.0}, // LightSteelBlue4 (424)
 {0.9333,0.7725,0.5686,1.0}, // burlywood2 (425)
 {0.7255,0.8275,0.9333,1.0}, // SlateGray2 (426)
 {0.0000,0.8039,0.8039,1.0}, // cyan3 (427)
 {0.6824,0.9333,0.9333,1.0}, // PaleTurquoise2 (428)
 {0.9333,0.8627,0.5098,1.0}, // LightGoldenrod2 (429)
 {0.6314,0.6314,0.6314,1.0}, // grey63 (430)
 {0.8039,0.3333,0.3333,1.0}, // IndianRed3 (431)
 {0.9333,0.8353,0.7176,1.0}, // bisque2 (432)
 {0.4235,0.6510,0.8039,1.0}, // SkyBlue3 (433)
 {0.8784,0.4000,1.0000,1.0}, // MediumOrchid1 (434)
 {1.0000,0.7255,0.0588,1.0}, // DarkGoldenrod1 (435)
 {0.4118,0.3490,0.8039,1.0}, // SlateBlue3 (436)
 {0.0000,0.8078,0.8196,1.0}, // DarkTurquoise (437)
 {0.7804,0.7804,0.7804,1.0}, // grey78 (438)
 {0.5451,0.5137,0.4706,1.0}, // AntiqueWhite4 (439)
 {1.0000,0.8824,1.0000,1.0}, // thistle1 (440)
 {0.5451,0.1451,0.0000,1.0}, // OrangeRed4 (441)
 {0.9333,0.9333,0.0000,1.0}, // yellow2 (442)
 {0.4824,0.4078,0.9333,1.0}, // MediumSlateBlue (443)
 {0.8784,0.8784,0.8784,1.0}, // grey88 (444)
 {1.0000,0.8941,0.8824,1.0}, // MistyRose1 (445)
 {0.9333,0.9098,0.6667,1.0}, // PaleGoldenrod (446)
 {0.8039,0.7451,0.4392,1.0}, // LightGoldenrod3 (447)
 {0.8314,0.8314,0.8314,1.0}, // grey83 (448)
 {0.7765,0.8863,1.0000,1.0}, // SlateGray1 (449)
 {0.0980,0.0980,0.4392,1.0}, // MidnightBlue (450)
 {0.8039,0.6078,0.6078,1.0}, // RosyBrown3 (451)
 {1.0000,0.8431,0.0000,1.0}, // gold1 (452)
 {1.0000,0.7333,1.0000,1.0}, // plum1 (453)
 {0.2902,0.2902,0.2902,1.0}, // grey29 (454)
 {0.7059,0.9333,0.7059,1.0}, // DarkSeaGreen2 (455)
 {0.1412,0.1412,0.1412,1.0}, // grey14 (456)
 {0.8039,0.2157,0.0000,1.0}, // OrangeRed3 (457)
 {0.8039,0.3098,0.2235,1.0}, // tomato3 (458)
 {0.0000,0.8039,0.0000,1.0}, // green3 (459)
 {0.8039,0.5843,0.0471,1.0}, // DarkGoldenrod3 (460)
 {0.7373,0.9333,0.4078,1.0}, // DarkOliveGreen2 (461)
 {0.8549,0.4392,0.8392,1.0}, // orchid (462)
 {0.6902,0.1882,0.3765,1.0}, // maroon (463)
 {0.5451,0.5373,0.4392,1.0}, // LemonChiffon4 (464)
 {0.7059,0.3216,0.8039,1.0}, // MediumOrchid3 (465)
 {1.0000,0.9804,0.9412,1.0}, // FloralWhite (466)
 {0.5373,0.4078,0.8039,1.0}, // MediumPurple3 (467)
 {0.6039,0.1961,0.8039,1.0}, // DarkOrchid3 (468)
 {0.9216,0.9216,0.9216,1.0}, // grey92 (469)
 {0.7373,0.8235,0.9333,1.0}, // LightSteelBlue2 (470)
 {0.6000,0.1961,0.8000,1.0}, // DarkOrchid (471)
 {0.9137,0.5882,0.4784,1.0}, // DarkSalmon (472)
 {0.0000,0.5451,0.5451,1.0}, // DarkCyan (473)
 {0.2549,0.4118,0.8824,1.0}, // RoyalBlue (474)
 {0.8039,0.0627,0.4627,1.0}, // DeepPink3 (475)
 {0.5529,0.7137,0.8039,1.0}, // LightSkyBlue3 (476)
 {0.9333,0.5843,0.4471,1.0}, // LightSalmon2 (477)
 {0.7216,0.7216,0.7216,1.0}, // grey72 (478)
 {0.3020,0.3020,0.3020,1.0}, // grey30 (479)
 {1.0000,0.9725,0.8627,1.0}, // cornsilk1 (480)
 {0.8039,0.7882,0.6471,1.0}, // LemonChiffon3 (481)
 {0.3216,0.5451,0.5451,1.0}, // DarkSlateGray4 (482)
 {0.6627,0.6627,0.6627,1.0}, // DarkGray (483)
 {0.7804,0.0824,0.5216,1.0}, // MediumVioletRed (484)
 {0.7882,0.7882,0.7882,1.0}, // grey79 (485)
 {0.1804,0.1804,0.1804,1.0}, // grey18 (486)
 {0.3059,0.9333,0.5804,1.0}, // SeaGreen2 (487)
 {0.5451,0.5373,0.5373,1.0}, // snow4 (488)
 {0.4078,0.5137,0.5451,1.0}, // LightBlue4 (489)
 {0.8039,0.0000,0.8039,1.0}, // magenta3 (490)
 {1.0000,0.9373,0.8353,1.0}, // PapayaWhip (491)
 {0.4000,0.4000,0.4000,1.0}, // grey40 (492)
 {0.2627,0.4314,0.9333,1.0}, // RoyalBlue2 (493)
 {0.2510,0.2510,0.2510,1.0}, // grey25 (494)
 {0.0392,0.0392,0.0392,1.0}, // grey4 (495)
 {0.9333,0.0000,0.0000,1.0}, // red2 (496)
 {0.8039,0.7020,0.5451,1.0}, // NavajoWhite3 (497)
 {0.5961,0.9608,1.0000,1.0}, // CadetBlue1 (498)
 {0.3412,0.3412,0.3412,1.0}, // grey34 (499)
 {0.6000,0.6000,0.6000,1.0}, // grey60 (500)
 {0.8039,0.8039,0.7059,1.0}, // LightYellow3 (501)
 {0.5176,0.4392,1.0000,1.0} // LightSlateBlue (502)
};
#endif


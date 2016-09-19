#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
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

#define CURSOR_KEYSTROKE(x) (								\
				x == GDK_KEY_Right || x == GDK_KEY_KP_Right ||			\
				x == GDK_KEY_Left || x == GDK_KEY_KP_Left ||			\
				x == GDK_KEY_Home || x == GDK_KEY_KP_Home ||			\
				x == GDK_KEY_End || x == GDK_KEY_KP_End ||			\
				x == GDK_KEY_Begin || x == GDK_KEY_KP_Begin  ||			\
				x == GDK_KEY_Page_Up || x == GDK_KEY_KP_Page_Up ||		\
				x == GDK_KEY_Page_Down || x == GDK_KEY_KP_Page_Down ||		\
				x == GDK_KEY_KP_Prior || x == GDK_KEY_KP_Prior			\
		)
#define IGNORE_KEYSTROKE(x) (								\
				x == GDK_KEY_Up || x == GDK_KEY_KP_Up ||			\
				x == GDK_KEY_Down || x == GDK_KEY_KP_Down ||			\
				x == GDK_KEY_Insert || x == GDK_KEY_KP_Insert ||		\
				x == GDK_KEY_Scroll_Lock || x == GDK_KEY_Pause ||		\
				x == GDK_KEY_Print || x == GDK_KEY_Cancel 			\
		)

#define BASIC_KEYSTROKE(x) (	x == GDK_KEY_KP_Divide || x == GDK_KEY_KP_Multiply ||		\
				x == GDK_KEY_KP_Subtract || x == GDK_KEY_KP_Add	||		\
				x == GDK_KEY_BackSpace || x == GDK_KEY_Delete ||		\
				x == GDK_KEY_KP_Delete || x == GDK_KEY_KP_Space ||		\
				(x >= GDK_KEY_KP_0 && x <= GDK_KEY_KP_9) ||			\
     				(x >= GDK_KEY_space && x <= GDK_KEY_asciitilde)	||		\
				(x >= GDK_KEY_Agrave && x <= GDK_KEY_Greek_switch)		\
		)

#define ENTRY_KEYSTROKE(x) (								\
				x == GDK_KEY_Escape || x == GDK_KEY_KP_Enter ||			\
				x == GDK_KEY_Return || x == GDK_KEY_Tab ||			\
				CURSOR_KEYSTROKE(x) ||					\
				BASIC_KEYSTROKE(x)					\
		)


     /* This is private : */

typedef struct _combobox_info_t combobox_info_t;
struct _combobox_info_t {
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    GtkComboBoxEntry *comboboxentry;
#else
    GtkComboBox *comboboxentry;
#endif
    GtkEntry *entry;
    GtkTreeModel *model;
    gchar *active_dbh_file;
    gpointer cancel_user_data;
    gpointer activate_user_data;
    void (*cancel_func) (GtkEntry * entry, gpointer cancel_user_data);
    void (*activate_func) (GtkEntry * entry, gpointer activate_user_data);
    /* 
     * This is private (ro): */
    gint dead_key;
    gint shift_pos;
    gint cursor_pos;
    gint active;

    gint completion_type;

    gboolean asian; 
    gboolean quick_activate; 

    GSList *list;
    GSList *limited_list;
    GSList *old_list;
    GHashTable *association_hash;
    /* imported or null */
    int (*extra_key_completion) (gpointer extra_key_data);
    gpointer extra_key_data;
};

static GMutex *sweep_mutex=NULL;
static time_t last_hit = 0;

///////////////////////////////////////////////////////////////////////////////

static void clean_history_list (GSList ** list);
static gint translate_key (gint x);
static gint on_key_press (GtkWidget * entry, GdkEventKey * event, gpointer data);
static gint on_key_press_history (GtkWidget * entry, GdkEventKey * event, gpointer data);
static int history_compare (gconstpointer a, gconstpointer b);
static void history_mklist (DBHashTable * d);
static void get_history_list (GSList ** in_list, char *dbh_file, char *top);
static gchar * combo_valid_utf_pathstring (const gchar * string);
////////////////////////////////////////////////////////////////////////////

static int path_compare (gconstpointer a, gconstpointer b){
    return strcmp((gchar *)a, (gchar *)b);
}

static void 
on_changed (GtkComboBox *combo_box, gpointer data) {
    combobox_info_t * combo_info=data;  
    gint active = gtk_combo_box_get_active (combo_box);
    NOOP("active=%d\n", active); 
    if(combo_info->extra_key_completion){
        (*(combo_info->extra_key_completion)) (combo_info->extra_key_data);
    }
    if(combo_info->quick_activate &&
	    combo_info->active != active && 
	    combo_info->activate_func) {
            (*(combo_info->activate_func)) ((GtkEntry *) combo_info->entry, combo_info->activate_user_data);
    }
}

static void
clear_association_hash (gpointer key, gpointer value, gpointer user_data) {
    g_free (key);
    if(!value)
        return;
    g_free (value);
    return;
}

static void *
internal_set_combo (void *p, void *q) {
    if (!p) {
	DBG("set_combo: combo_info==NULL!\n");
	return NULL;
    }
    combobox_info_t * combo_info=p;
    gchar *token=q;
    int count;
    GSList *tmp;
    GSList **limited_list;
    gboolean match = FALSE;

    /*if (!combo_info->list || !combo_info->active_dbh_file) { */
    if(!combo_info->list)
        return GINT_TO_POINTER(match);

    combo_info->old_list = combo_info->limited_list;
    combo_info->limited_list = NULL;
    limited_list = &(combo_info->limited_list);
    NOOP ("token=%s\n", ((token) ? token : "null"));


    // sort items after the first
    GSList *first=NULL;
    tmp = combo_info->list;
    for(count = 0; tmp && tmp->data; tmp = tmp->next) {
        gchar *p = (gchar *) tmp->data;
        if(!p)
            continue;
        if(!token || strncmp (token, p, strlen (token)) == 0) {
            if(token)
                match = TRUE;
	    if (!first) {
		first = tmp;
	    } else {
		*limited_list = g_slist_insert_sorted (*limited_list, g_strdup (p), path_compare);
	    }
            if(++count >= MAX_COMBO_ELEMENTS)
                break;
        }
    }
    //preprend first item
    if (first) {
	*limited_list = 
	    g_slist_prepend (*limited_list, g_strdup ((gchar *) first->data));
    }



    if(*limited_list) {
        /* make sure we have utf-8 in combo. This may not match
         * character set of actual system, such as euc-jp, so
         * we better keep correct value in an association hash. */
        if(combo_info->association_hash) {
            /* clean old hash */
            g_hash_table_foreach (combo_info->association_hash, clear_association_hash, NULL);
            g_hash_table_destroy (combo_info->association_hash);
            combo_info->association_hash = NULL;
        }

        combo_info->association_hash = g_hash_table_new (g_str_hash, g_str_equal);

        if(combo_info->association_hash) {
            GSList *tmp;
            /* create new hash */
            for(tmp = *limited_list; tmp; tmp = tmp->next) {
                gchar *utf_string = combo_valid_utf_pathstring ((gchar *) (tmp->data));
                NOOP("utf_string=%s\n",utf_string); 
                if(strcmp (utf_string, (gchar *) (tmp->data))) {
                    NOOP ("combo hash table %s ---> %s\n", (gchar *) (tmp->data), utf_string);
                    g_hash_table_insert (combo_info->association_hash, utf_string, tmp->data);
		    g_free(tmp->data);
                    tmp->data = utf_string;
                } else {
		    g_free(utf_string);
		}
            }
        }
	// Set popdown list:
	rfm_set_store_data_from_list ((GtkListStore *)combo_info->model, limited_list);

	// Set tooltip to reflect values in popdown list:
	gchar *tooltip_text=NULL;
	GSList *tmp=*limited_list;
	GdkPixbuf *tooltip_icon=NULL;
	for (; tmp && tmp->data; tmp=tmp->next){
	    gchar *p=tooltip_text;
	    if (p) tooltip_text = g_strconcat(p,"\n ", (gchar *)(tmp->data),NULL);
	    else tooltip_text = g_strconcat("<b>", _("History:"),"</b>\n ",(gchar *)(tmp->data), NULL);
	    g_free(p);
	}
	tooltip_icon=rfm_get_pixbuf("xffm/emblem_bookmark", SIZE_DIALOG);

	rfm_add_custom_tooltip(GTK_WIDGET (combo_info->comboboxentry), tooltip_icon, tooltip_text);
//	gtk_widget_set_tooltip_markup (GTK_WIDGET (combo_info->comboboxentry), tooltip_text);
//	gtk_widget_set_tooltip_text (GTK_WIDGET (combo_info->comboboxentry), tooltip_text); 
	g_free(tooltip_text);
	
        //gtk_combo_set_popdown_strings (combo_info->combo, *limited_list);
        clean_history_list (&(combo_info->old_list));
    } else {
        combo_info->limited_list = combo_info->old_list;
	combo_info->old_list = NULL;
    }
    return GINT_TO_POINTER(match);
}

static void
set_blank (void *p) {
    set_entry (p, "");
}


static gchar *
recursive_utf_string (const gchar * path) {
    gchar *dir,
     *base,
     *valid,
     *utf_base,
     *utf_dir;
    if(!path)
        return NULL;
    if(g_utf8_validate (path, -1, NULL))
        return g_strdup (path);
    dir = g_path_get_dirname (path);
    NOOP ("dir=%s\n", dir);
    if(!dir || !strlen (dir) || strcmp (dir, "./") == 0 || strcmp (dir, ".") == 0) {
        /* short circuit non-paths */
        g_free (dir);
        return rfm_utf_string (path);
    }
    /* otherwise asume a mixed utf/locale string */
    base = g_path_get_basename (path);
    utf_dir = recursive_utf_string (dir);
    if(!g_utf8_validate (base, -1, NULL)) {
        utf_base = rfm_utf_string (base);
        g_free (base);
    } else {
        utf_base = base;
    }

    valid = g_strconcat (utf_dir, G_DIR_SEPARATOR_S, utf_base, NULL);

    NOOP("dir=%s base=%s valide=%s\n",dir, base, valid); 
    g_free (utf_base);
    g_free (utf_dir);
    g_free (dir);
    return valid;
}

static gchar *
combo_valid_utf_pathstring (const gchar * string) {
    gchar *utf_string = NULL;
    utf_string = recursive_utf_string (string);
    NOOP ("string=%s utf_string=%s\n", string, utf_string);
    return utf_string;
}

static void
clean_history_list (GSList ** list) {
    GSList *tmp;
    if(!*list)
        return;
    for(tmp = *list; tmp; tmp = tmp->next) {
        /*NOOP("freeing %s\n",(char *)tmp->data); */
        g_free (tmp->data);
        tmp->data = NULL;
    }
    g_slist_free (*list);
    *list = NULL;
    return;
}

static gint
translate_key (gint x) {
    switch (x) {
    case GDK_KEY_KP_Divide:
        return GDK_KEY_slash;
    case GDK_KEY_KP_Subtract:
        return GDK_KEY_minus;
    case GDK_KEY_KP_Multiply:
        return GDK_KEY_asterisk;
    case GDK_KEY_KP_Add:
        return GDK_KEY_plus;
    case GDK_KEY_KP_Space:
        return GDK_KEY_space;
    case GDK_KEY_KP_0:
        return GDK_KEY_0;
    case GDK_KEY_KP_1:
        return GDK_KEY_1;
    case GDK_KEY_KP_2:
        return GDK_KEY_2;
    case GDK_KEY_KP_3:
        return GDK_KEY_3;
    case GDK_KEY_KP_4:
        return GDK_KEY_4;
    case GDK_KEY_KP_5:
        return GDK_KEY_5;
    case GDK_KEY_KP_6:
        return GDK_KEY_6;
    case GDK_KEY_KP_7:
        return GDK_KEY_7;
    case GDK_KEY_KP_8:
        return GDK_KEY_8;
    case GDK_KEY_KP_9:
        return GDK_KEY_9;
    }
    return x;
}

static int
compose_key (int key, int dead_key) {
    switch (dead_key) {
    case GDK_KEY_dead_grave:
        switch (key) {
        case GDK_KEY_A:
            return GDK_KEY_Agrave;
        case GDK_KEY_a:
            return GDK_KEY_agrave;
        case GDK_KEY_E:
            return GDK_KEY_Egrave;
        case GDK_KEY_e:
            return GDK_KEY_egrave;
        case GDK_KEY_I:
            return GDK_KEY_Igrave;
        case GDK_KEY_i:
            return GDK_KEY_igrave;
        case GDK_KEY_O:
            return GDK_KEY_Ograve;
        case GDK_KEY_o:
            return GDK_KEY_ograve;
        case GDK_KEY_U:
            return GDK_KEY_Ugrave;
        case GDK_KEY_u:
            return GDK_KEY_ugrave;
        }
        break;
    case GDK_KEY_dead_acute:
        NOOP ("dead key=0x%x composing %c\n", (unsigned)dead_key, (char)key);
        switch (key) {
        case GDK_KEY_A:
            return GDK_KEY_Aacute;
        case GDK_KEY_a:
            return GDK_KEY_aacute;
        case GDK_KEY_E:
            return GDK_KEY_Eacute;
        case GDK_KEY_e:
            return GDK_KEY_eacute;
        case GDK_KEY_I:
            return GDK_KEY_Iacute;
        case GDK_KEY_i:
            return GDK_KEY_iacute;
        case GDK_KEY_O:
            return GDK_KEY_Oacute;
        case GDK_KEY_o:
            return GDK_KEY_oacute;
        case GDK_KEY_U:
            return GDK_KEY_Uacute;
        case GDK_KEY_u:
            return GDK_KEY_uacute;
        case GDK_KEY_Y:
            return GDK_KEY_Yacute;
        case GDK_KEY_y:
            return GDK_KEY_yacute;
        case GDK_KEY_S:
            return GDK_KEY_Sacute;
        case GDK_KEY_Z:
            return GDK_KEY_Zacute;
        case GDK_KEY_s:
            return GDK_KEY_sacute;
        case GDK_KEY_z:
            return GDK_KEY_zacute;
        case GDK_KEY_R:
            return GDK_KEY_Racute;
        case GDK_KEY_r:
            return GDK_KEY_racute;
        case GDK_KEY_L:
            return GDK_KEY_Lacute;
        case GDK_KEY_l:
            return GDK_KEY_lacute;
        case GDK_KEY_C:
            return GDK_KEY_Cacute;
        case GDK_KEY_c:
            return GDK_KEY_cacute;
        case GDK_KEY_N:
            return GDK_KEY_Nacute;
        case GDK_KEY_n:
            return GDK_KEY_nacute;
        }
        break;
    case GDK_KEY_dead_diaeresis:
        switch (key) {
        case GDK_KEY_A:
            return GDK_KEY_Adiaeresis;
        case GDK_KEY_a:
            return GDK_KEY_adiaeresis;
        case GDK_KEY_E:
            return GDK_KEY_Ediaeresis;
        case GDK_KEY_e:
            return GDK_KEY_ediaeresis;
        case GDK_KEY_I:
            return GDK_KEY_Idiaeresis;
        case GDK_KEY_i:
            return GDK_KEY_idiaeresis;
        case GDK_KEY_O:
            return GDK_KEY_Odiaeresis;
        case GDK_KEY_o:
            return GDK_KEY_odiaeresis;
        case GDK_KEY_U:
            return GDK_KEY_Udiaeresis;
        case GDK_KEY_u:
            return GDK_KEY_udiaeresis;
        case GDK_KEY_Y:
            return GDK_KEY_Ydiaeresis;
        case GDK_KEY_y:
            return GDK_KEY_ydiaeresis;
        }
        break;
    case GDK_KEY_dead_cedilla:
        switch (key) {
        case GDK_KEY_C:
            return GDK_KEY_Ccedilla;
        case GDK_KEY_c:
            return GDK_KEY_ccedilla;
        case GDK_KEY_S:
            return GDK_KEY_Scedilla;
        case GDK_KEY_s:
            return GDK_KEY_scedilla;
        case GDK_KEY_T:
            return GDK_KEY_Tcedilla;
        case GDK_KEY_t:
            return GDK_KEY_tcedilla;
        case GDK_KEY_R:
            return GDK_KEY_Rcedilla;
        case GDK_KEY_r:
            return GDK_KEY_rcedilla;
        case GDK_KEY_L:
            return GDK_KEY_Lcedilla;
        case GDK_KEY_l:
            return GDK_KEY_lcedilla;
        case GDK_KEY_G:
            return GDK_KEY_Gcedilla;
        case GDK_KEY_g:
            return GDK_KEY_gcedilla;
        case GDK_KEY_N:
            return GDK_KEY_Ncedilla;
        case GDK_KEY_n:
            return GDK_KEY_ncedilla;
        case GDK_KEY_K:
            return GDK_KEY_Kcedilla;
        case GDK_KEY_k:
            return GDK_KEY_kcedilla;
        }
        break;
    case GDK_KEY_dead_circumflex:
        switch (key) {
        case GDK_KEY_A:
            return GDK_KEY_Acircumflex;
        case GDK_KEY_a:
            return GDK_KEY_acircumflex;
        case GDK_KEY_E:
            return GDK_KEY_Ecircumflex;
        case GDK_KEY_e:
            return GDK_KEY_ecircumflex;
        case GDK_KEY_I:
            return GDK_KEY_Icircumflex;
        case GDK_KEY_i:
            return GDK_KEY_icircumflex;
        case GDK_KEY_O:
            return GDK_KEY_Ocircumflex;
        case GDK_KEY_o:
            return GDK_KEY_ocircumflex;
        case GDK_KEY_U:
            return GDK_KEY_Ucircumflex;
        case GDK_KEY_u:
            return GDK_KEY_ucircumflex;
        case GDK_KEY_H:
            return GDK_KEY_Hcircumflex;
        case GDK_KEY_h:
            return GDK_KEY_hcircumflex;
        case GDK_KEY_J:
            return GDK_KEY_Jcircumflex;
        case GDK_KEY_j:
            return GDK_KEY_jcircumflex;
        case GDK_KEY_C:
            return GDK_KEY_Ccircumflex;
        case GDK_KEY_c:
            return GDK_KEY_ccircumflex;
        case GDK_KEY_G:
            return GDK_KEY_Gcircumflex;
        case GDK_KEY_g:
            return GDK_KEY_gcircumflex;
        case GDK_KEY_S:
            return GDK_KEY_Scircumflex;
        case GDK_KEY_s:
            return GDK_KEY_scircumflex;
        }
        break;
    }
    return key;
}

static gint
on_key_press (GtkWidget * entry, GdkEventKey * event, gpointer data) {
    combobox_info_t *combo_info = (combobox_info_t *) data;
    NOOP ("on_key_press: got key= 0x%x\n", event->keyval);
    if(event->keyval == GDK_KEY_Escape && combo_info->cancel_func) {
        (*(combo_info->cancel_func)) ((GtkEntry *) entry, combo_info->cancel_user_data);
        return TRUE;
    }
    return FALSE;
}

static int
deadkey (int key) {
    /* support for deadkeys */
    switch (key) {
        /* spanish */
    case GDK_KEY_dead_acute:
    case GDK_KEY_dead_diaeresis:
        return key;
        /* french */
    case GDK_KEY_dead_cedilla:
    case GDK_KEY_dead_grave:
    case GDK_KEY_dead_circumflex:
        return key;
        /* others (if you want any of these, submit a request) */
    case GDK_KEY_dead_tilde:
    case GDK_KEY_dead_macron:
    case GDK_KEY_dead_breve:
    case GDK_KEY_dead_abovedot:
    case GDK_KEY_dead_abovering:
    case GDK_KEY_dead_doubleacute:
    case GDK_KEY_dead_caron:
    case GDK_KEY_dead_ogonek:
    case GDK_KEY_dead_iota:
    case GDK_KEY_dead_voiced_sound:
    case GDK_KEY_dead_semivoiced_sound:
    case GDK_KEY_dead_belowdot:
/* these two are > gtk-2.2: */
/*     case GDK_KEY_dead_hook:*/
/*     case GDK_KEY_dead_horn:*/
        return 0;
    default:
        return 0;
    }
}

static gint
on_key_press_history (GtkWidget * entry, GdkEventKey * event, gpointer data) {
    gchar *utf_fulltext = NULL;
    gboolean find_match = FALSE;
    int i;
    gchar *text[2] = { NULL, NULL };
    gchar c[] = { 0, 0, 0, 0, 0 };
    gchar *fulltext = NULL;
    combobox_info_t *combo_info = (combobox_info_t *) data;
    GtkEditable *editable = (GtkEditable *) entry;
    gint pos1, pos2, pos;
    gboolean preselection;

    NOOP("on_key_press_history: got key= 0x%x\n", event->keyval);

    /* asian input methods: turns off autocompletion */
    if(event->keyval == GDK_KEY_space && (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))) {
        combo_info->asian = TRUE;
        return FALSE;
    } else if(event->keyval == GDK_KEY_space && (event->state & GDK_MOD1_MASK)) {
        /* turn autocompletion back on */
        combo_info->asian = FALSE;
        return TRUE;
    }

    if(combo_info->asian && !(event->keyval == GDK_KEY_Return)
       && !(event->keyval == GDK_KEY_KP_Enter)){
        return FALSE;
    }
    if( IGNORE_KEYSTROKE(event->keyval)) {
        return TRUE;
    }

    if(event->keyval == GDK_KEY_Menu || event->keyval == GDK_KEY_ISO_Level3_Shift)
        return TRUE;
    
    pos = gtk_editable_get_position (editable);
    if(event->keyval == GDK_KEY_Shift_L || event->keyval == GDK_KEY_Shift_R) {
        combo_info->cursor_pos = combo_info->shift_pos = pos;
    }


    if(combo_info->dead_key) {
        if(event->keyval != GDK_KEY_Shift_L && event->keyval != GDK_KEY_Shift_R) {
            event->keyval = compose_key (event->keyval, combo_info->dead_key);
            NOOP ("composing to %c\n", (char)event->keyval);
            combo_info->dead_key = 0;
        }
    } else {
        combo_info->dead_key = deadkey (event->keyval);
        NOOP ("deadkey is  0x%x\n", (unsigned)combo_info->dead_key);
        if(combo_info->dead_key)
            return TRUE;
    }

    preselection = gtk_editable_get_selection_bounds (editable, &pos1, &pos2);


    if(!preselection)
        pos1 = pos2 = -1;

    NOOP ("NOOP(2):pos= %d, combo_info->shift_pos=%d cursor_pos=%d\n", pos, combo_info->shift_pos, combo_info->cursor_pos);
    NOOP ("NOOP(2):got key= 0x%x\n", event->keyval);

    if(event->keyval == GDK_KEY_KP_Down && (event->state & GDK_MOD1_MASK)) {
        goto returnFALSE;
    }
    if(event->keyval == GDK_KEY_KP_Up && (event->state & GDK_MOD1_MASK)) {
        goto returnFALSE;
    }
    if(event->keyval == GDK_KEY_Down && (event->state & GDK_MOD1_MASK)) {
        goto returnFALSE;
    }
    if(event->keyval == GDK_KEY_Up && (event->state & GDK_MOD1_MASK)) {
        goto returnFALSE;
    }

    g_signal_handlers_block_by_func (G_OBJECT (entry), (gpointer) on_key_press_history, data);
    if(event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
        /*NOOP("NOOP: got return;\n"); */
        if(combo_info->activate_func) {
            (*(combo_info->activate_func)) ((GtkEntry *) entry, combo_info->activate_user_data);
        }
        goto end;
    }

    if(event->keyval == GDK_KEY_BackSpace && (event->state & GDK_CONTROL_MASK)) {
        gchar *p;
        fulltext = gtk_editable_get_chars (editable, 0, -1);
        p = strrchr (fulltext, ' ');
        if(!p)
            p = strrchr (fulltext, G_DIR_SEPARATOR);
        if(!p) {
            gtk_editable_delete_text (editable, 0, -1);
        } else {
            gtk_editable_delete_text (editable, strlen (fulltext) - strlen (p), -1);
        }
        g_free (fulltext);
        fulltext = NULL;
        goto end;
    }

    if(event->keyval == GDK_KEY_Tab) {
        gint start;
        gint finish;
	if(gtk_editable_get_selection_bounds (editable, &start, &finish)) {
	    gtk_editable_set_position (editable, -1);
	}


	goto complete_text;
    } 
    else if(CURSOR_KEYSTROKE (event->keyval)) {
        if(event->keyval == GDK_KEY_Right || event->keyval == GDK_KEY_KP_Right) {
            NOOP("NOOP: right\n"); 
            if(event->state & GDK_SHIFT_MASK) {
                combo_info->cursor_pos++;
                if(combo_info->cursor_pos < combo_info->shift_pos)
                    gtk_editable_select_region (editable, combo_info->cursor_pos, combo_info->shift_pos);
                else
                    gtk_editable_select_region (editable, combo_info->shift_pos, combo_info->cursor_pos);
            } else {
                gtk_editable_set_position (editable, pos + 1);
                combo_info->cursor_pos = pos + 1;
            }
        } else if(event->keyval == GDK_KEY_Left || event->keyval == GDK_KEY_KP_Left){
            NOOP("NOOP: left\n"); 
            /* cranky gtk design, position must be at end of selection... */
            if(combo_info->cursor_pos)
                combo_info->cursor_pos--;
            if(event->state & GDK_SHIFT_MASK) {
                if(combo_info->cursor_pos < combo_info->shift_pos)
                    gtk_editable_select_region (editable, combo_info->cursor_pos, combo_info->shift_pos);
                else
                    gtk_editable_select_region (editable, combo_info->shift_pos, combo_info->cursor_pos);
            } else if(pos - 1 >= 0) {
                gtk_editable_set_position (editable, pos - 1);
                combo_info->cursor_pos = pos - 1;
            }
        } else if(event->keyval == GDK_KEY_Home || event->keyval == GDK_KEY_KP_Home ||
                  event->keyval == GDK_KEY_Begin || event->keyval == GDK_KEY_KP_Begin ||
                  event->keyval == GDK_KEY_Page_Up || event->keyval == GDK_KEY_KP_Page_Up){
            NOOP("NOOP: page right: 0x%x\n", event->keyval); 
            /* cranky gtk design, position must be at end of selection... */
            gtk_editable_set_position (editable, 0);
        } else if(event->keyval == GDK_KEY_End || event->keyval == GDK_KEY_KP_End ||
                  event->keyval == GDK_KEY_Page_Down || event->keyval == GDK_KEY_KP_Page_Down ){
            NOOP("NOOP: page left: 0x%x\n", event->keyval); 
            gtk_editable_set_position (editable, -1);
            /* cranky gtk design, position must be at end of selection... */
	    if (event->keyval == GDK_KEY_Page_Down || event->keyval == GDK_KEY_KP_Page_Down) {
		NOOP("SELECT: page left: 0x%x\n", event->keyval); 
                    gtk_editable_select_region (editable, 0, -1);
	    }
        }
        goto end;
    }

    /* construct search string... (limited to 128 bytes) */

    /* control-delete will remove the selected item from the
     * history dbh file (to remove stale history items) */

    if(BASIC_KEYSTROKE (event->keyval)) {
        /* get entry text */
	NOOP("BASIC_KEYSTROKE\n");
        if(event->keyval == GDK_KEY_BackSpace) {
            if(preselection) {
                gtk_editable_delete_text (editable, pos1, pos2);
                NOOP("NOOP:pos1=%d,pos2=%d\n",pos1,pos2); 
                goto end;
            }
            if(pos == 0) {
                goto end;
            }
            text[0] = gtk_editable_get_chars (editable, 0, pos - 1);
            text[1] = gtk_editable_get_chars (editable, pos, -1);
            fulltext = g_strconcat (text[0], text[1], NULL);
            g_free (text[0]);
            g_free (text[1]);
            text[0] = text[1] = NULL;
            NOOP("deleting entry text\n"); 
            gtk_editable_delete_text (editable, 0, -1);
            pos1 = 0;
            if(fulltext && strlen (fulltext)) {
                gtk_editable_insert_text (editable, (const gchar *)fulltext, strlen (fulltext), &pos1);
                gtk_editable_set_position (editable, pos - 1);
                combo_info->cursor_pos = pos - 1;
                NOOP("NOOP: inserting %s\n",fulltext); 
            } else {
                set_blank (combo_info);
            }
            g_free (fulltext);
            fulltext = NULL;
            goto end;
        } else if(event->keyval == GDK_KEY_Delete || event->keyval == GDK_KEY_KP_Delete) {
            if(combo_info->active_dbh_file && event->state & GDK_CONTROL_MASK) {        /* remove stale entries */
                fulltext = gtk_editable_get_chars (editable, 0, -1);
                if(fulltext && strlen (fulltext)
                   && combo_info->association_hash) {
                    gchar *local_choice = g_hash_table_lookup (combo_info->association_hash,
                                                               fulltext);
                    NOOP ("converting back to non utf8 value %s ---> %s\n", fulltext, local_choice);
                    if(local_choice) {
                        g_free (fulltext);
                        fulltext = local_choice;
                    }
                }

                if(fulltext) remove_from_history (combo_info->active_dbh_file, fulltext);
                set_blank (combo_info);

                g_free (fulltext);
                fulltext = NULL;
                if(combo_info->cancel_func)
                    (*(combo_info->cancel_func)) ((GtkEntry *) entry, combo_info->cancel_user_data);
                goto end;
            } else {
                NOOP("NOOP: pos=%d, pos1=%d\n",pos,pos1); 
                g_free (fulltext);
                fulltext = NULL;
                if(preselection) {
		    NOOP("preselection gtk_editable_delete_text\n"); 
                    /* gtk_editable_delete_text (editable,pos1,-1); */
                    gtk_editable_delete_text (editable, pos1, pos2);
                    goto end;
                }
                text[0] = gtk_editable_get_chars (editable, 0, pos);
                text[1] = gtk_editable_get_chars (editable, pos + 1, -1);
                /* do conversion to locale here? no. */
                fulltext = g_strconcat (text[0], text[1], NULL);
                g_free (text[0]);
                g_free (text[1]);
                text[0] = text[1] = NULL;
		NOOP("preselection2 gtk_editable_delete_text\n"); 

                gtk_editable_delete_text (editable, 0, -1);
                if(fulltext && strlen (fulltext)) {
                    pos1 = 0;
                    gtk_editable_insert_text (editable, (const gchar *)fulltext, strlen (fulltext), &pos1);
                    gtk_editable_set_position (editable, pos);
                    combo_info->cursor_pos = pos;
                    NOOP("NOOP: inserting %s\n",fulltext); 
                } else {
                    set_blank (combo_info);
                }
                g_free (fulltext);
                fulltext = NULL;
                goto end;
            }
        } else {                /* normal key */
	    NOOP("normal key\n");
            gchar *utf_c = NULL;
            *c = translate_key (event->keyval);

            if(!g_utf8_validate (c, -1, NULL)) {
                const char *fc;
                GError *error = NULL;
                gchar *from_codeset = NULL;
                gsize r_bytes,
                  w_bytes;
                g_get_charset (&fc);
                if(fc)
                    from_codeset = g_strdup (fc);
                else // fallback, western europe.
                    from_codeset = g_strdup ("ISO-8859-1");
                utf_c = g_convert (c, strlen (c), "UTF-8", from_codeset, &r_bytes, &w_bytes, &error);
                g_free (from_codeset);
                if(error) {
                    NOOP ("NOOP: %s\n", error->message);
                    g_error_free (error);
                }

            } else
                utf_c = g_strdup (c);
            if(preselection) {
                gtk_editable_delete_text (editable, pos1, -1);
                text[0] = gtk_editable_get_chars (editable, 0, -1);

                fulltext = g_strconcat (text[0], c, NULL);

                utf_fulltext = g_strconcat (text[0], utf_c, NULL);
                text[1] = NULL;
                pos = 0;
                gtk_editable_delete_text (editable, 0, -1);
                gtk_editable_insert_text (editable, (const gchar *)utf_fulltext, strlen (utf_fulltext), &pos);
                gtk_editable_set_position (editable, pos);
                combo_info->cursor_pos = pos;
            } else {
                NOOP("NOOP: pos=%d\n",pos); 
                /*FIXME convert text[0],text[1] to locale for fulltext (huh?)*/
                text[0] = gtk_editable_get_chars (editable, 0, pos);
                text[1] = gtk_editable_get_chars (editable, pos, -1);
                /* convert to locale */
                fulltext = g_strconcat (text[0], c, text[1], NULL);

                utf_fulltext = g_strconcat (text[0], utf_c, text[1], NULL);
                NOOP("NOOP: pos=%d fulltext=%s\n",pos,fulltext);
                pos1 = 0;
                gtk_editable_delete_text (editable, 0, -1);
                gtk_editable_insert_text (editable, (const gchar *)utf_fulltext, strlen (utf_fulltext), &pos1);
                gtk_editable_set_position (editable, pos + 1);
                combo_info->cursor_pos = pos;
            }
            g_free (utf_c);
        }
        g_free (text[0]);
        g_free (text[1]);
        text[0] = text[1] = NULL;
    } else if(event->keyval != GDK_KEY_Tab) {
        g_signal_handlers_unblock_by_func (G_OBJECT (entry), (gpointer) on_key_press_history, data);
        goto returnFALSE;
    }


    for(i = 0; i < strlen (fulltext); i++){
        if(fulltext[i] != ' '){
            find_match = TRUE;
	}
    }
    if(find_match && combo_info->comboboxentry) {
        NOOP("NOOP: setting limited list and emitting signal...fulltext=%s\n",(fulltext)?fulltext:"null"); 
        if(internal_set_combo (combo_info, fulltext)) {
            if(combo_info->limited_list && 
		    g_slist_length (combo_info->limited_list) > 1) 
	    {
                g_signal_emit_by_name ((gpointer) (entry), "activate", NULL);
            }
        }
    }

  complete_text:
    if(fulltext) {
        /* look for in ordered GSList */
        NOOP ("NOOP:fulltext is %s\n", fulltext);

	gchar *token;
	gint position=gtk_editable_get_position(editable);
	token = gtk_editable_get_chars (editable, 0, position);
	gchar *remainder = gtk_editable_get_chars (editable, position, -1);

	gchar *suggest=NULL;
	switch(combo_info->completion_type) {
	    case MATCH_FILE:
		//suggest=rfm_file_completion(NULL, token);
		suggest=rfm_rational(RFM_MODULE_DIR, "completion", 
			NULL, token, "rfm_file_completion");
		break;
	    case MATCH_COMMAND:
		//suggest=rfm_bash_complete(NULL, token, strlen(token));
		suggest=rfm_complex(RFM_MODULE_DIR, "completion", 
			NULL, token, GINT_TO_POINTER(strlen(token)),
			"rfm_bash_complete");
		break;
	    case MATCH_HISTORY:
		//suggest=rfm_history_completion(NULL, token);
		suggest=rfm_rational(RFM_MODULE_DIR, "completion", 
			NULL, token, 
			"rfm_history_completion");
		break;
	    default:
		suggest=NULL;
	}

	NOOP("token=\"%s\", suggested=\"%s\" remainder=\"%s\"\n", 
		token, suggest, remainder);
	// if found, complete with untyped part selected. 
	if (suggest && token && strlen(suggest) > strlen(token)) {
	    gtk_editable_delete_text (editable, 0, -1);
	    pos1 = 0;
	    gtk_editable_insert_text (editable, suggest, strlen (suggest), &pos1);
	    pos2=pos1;
	    gtk_editable_insert_text (editable, remainder, strlen (remainder), &pos2);
	    NOOP("selecting region %d -1\n", position);
	    // Set position must come before select region,
	    // other wise standard gtk callback will unselect the region
	    gtk_editable_set_position (editable, position);
	    gtk_editable_select_region (editable, position, -1);

	}
	g_free(token);
	g_free(suggest);

        g_free (fulltext);
        fulltext = NULL;
    }
  end:
    g_signal_handlers_unblock_by_func (G_OBJECT (entry), (gpointer) on_key_press_history, data);
 /*   if(combo_info->extra_key_completion){
        (*(combo_info->extra_key_completion)) (combo_info->extra_key_data);
    }*/
    return (TRUE);
  returnFALSE:
 /*   if(combo_info->extra_key_completion){
        (*(combo_info->extra_key_completion)) (combo_info->extra_key_data);
    }*/
    return (FALSE);
}


static int
history_compare (gconstpointer a, gconstpointer b) {
    history_dbh_t *da = (history_dbh_t *) a;
    history_dbh_t *db = (history_dbh_t *) b;

    if(db->last_hit >= last_hit && da->last_hit < last_hit) {
        return 1;
    }
    if(da->last_hit >= last_hit && db->last_hit < last_hit) {
        return -1;
    }
    if(db->hits != da->hits)
        return (db->hits - da->hits);
    return (strcmp (da->path, db->path));
}

static void
history_lasthit (DBHashTable * d) {
    history_dbh_t *history_mem = (history_dbh_t *) DBH_DATA (d);
    if(!history_mem)
        g_assert_not_reached ();
    if(history_mem->last_hit >= last_hit) {
        last_hit = history_mem->last_hit;
    }
}
    
static void
history_mklist (DBHashTable * d) {
    GSList **the_list = d->sweep_data;
    /*if(*the_list==NULL) { //nah!
        g_warning("history_mklist(): *the_list==NULL\n");
        return;
    }*/

    history_dbh_t *history_mem = (history_dbh_t *) malloc (sizeof (history_dbh_t));
    if(!history_mem){
        g_warning("malloc(): %s\n", strerror(errno));
        return;
    }
    memcpy (history_mem, DBH_DATA (d), sizeof (history_dbh_t));
    // false positive. history_mem->path is a string, not an array.
    // coverity[array_null : FALSE]
    if(history_mem->path && strlen (history_mem->path)) {
        *the_list = g_slist_insert_sorted (*the_list, history_mem, history_compare);
        NOOP("NOOP: inserted %s\n",(char *)history_mem->path); 
    }
    else g_free(history_mem);
    // history_mem does not go out of scope. It is now managed by the glist 
    // located at the address pointed to by the_list, and is cleaned when
    // combobox is destroyed.
}

/* if top==NULL, the top entry is left blank,
 * if top=="", the top entry is the one with the greatest access time for last hit
 * if top=="anything", the entry is set to "anything"
 *
 * (only "" is used now)
 * */

static void
get_history_list (GSList ** in_list, char *dbh_file, char *top) {
    DBHashTable *d;
    GSList **the_list;
    GSList *tmp;
/*   char *first=NULL;*/
    the_list = in_list;

    NOOP("NOOP:at get_history_list with %s \n",dbh_file); 

    g_mutex_lock(sweep_mutex);
    clean_history_list (the_list);
    last_hit = 0;
    TRACE("opening %s...\n",dbh_file); 
    if((d = dbh_new (dbh_file, NULL, DBH_PARALLEL_SAFE)) != NULL) {
	dbh_set_parallel_lock_timeout(d, 3);
	// Last hit is the top item
        dbh_foreach_sweep (d, history_lasthit);
	d->sweep_data=the_list;
        dbh_foreach_sweep (d, history_mklist);
        dbh_close (d);
    } else {
        // if dbh_file cannot be opened, create a new one
        NOOP ("Creating history file: %s", dbh_file);
	unsigned char keylength=11;
        if((d = dbh_new (dbh_file, &keylength, 0)) != NULL) {
            dbh_close (d);
        }
    }
    TRACE("open %s.\n",dbh_file); 
    /* leave only strings in the history list: */
    for(tmp = *the_list; tmp; tmp = tmp->next) {
        history_dbh_t *history_mem = (history_dbh_t *) tmp->data;
        gchar *p = g_strdup (history_mem->path);
        NOOP ("%s, hits=%d\n", history_mem->path, history_mem->hits);
        tmp->data = p;
        g_free (history_mem);
        history_mem = NULL;
    }

    if(*the_list == NULL) {
        *the_list = g_slist_prepend (*the_list, g_strdup (""));
    }
    g_mutex_unlock(sweep_mutex);
    return;
}


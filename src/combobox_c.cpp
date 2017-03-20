
#include "combobox_c.hpp"

#define CURSOR_KEYSTROKE(x) (	\
	x == GDK_KEY_Right || x == GDK_KEY_KP_Right ||	\
	x == GDK_KEY_Left || x == GDK_KEY_KP_Left ||	\
	x == GDK_KEY_Home || x == GDK_KEY_KP_Home ||	\
	x == GDK_KEY_End || x == GDK_KEY_KP_End ||	\
	x == GDK_KEY_Begin || x == GDK_KEY_KP_Begin  ||	\
	x == GDK_KEY_Page_Up || x == GDK_KEY_KP_Page_Up ||\
	x == GDK_KEY_Page_Down || x == GDK_KEY_KP_Page_Down ||	\
	x == GDK_KEY_KP_Prior || x == GDK_KEY_KP_Prior		\
	)
#define IGNORE_KEYSTROKE(x) (	\
	x == GDK_KEY_Up || x == GDK_KEY_KP_Up ||	\
	x == GDK_KEY_Down || x == GDK_KEY_KP_Down ||	\
	x == GDK_KEY_Insert || x == GDK_KEY_KP_Insert ||\
	x == GDK_KEY_Scroll_Lock || x == GDK_KEY_Pause ||\
	x == GDK_KEY_Print || x == GDK_KEY_Cancel 	\
	)

#define BASIC_KEYSTROKE(x) (	\
	x == GDK_KEY_KP_Divide || x == GDK_KEY_KP_Multiply ||\
	x == GDK_KEY_KP_Subtract || x == GDK_KEY_KP_Add	||\
	x == GDK_KEY_BackSpace || x == GDK_KEY_Delete ||\
	x == GDK_KEY_KP_Delete || x == GDK_KEY_KP_Space ||\
	(x >= GDK_KEY_KP_0 && x <= GDK_KEY_KP_9) ||	\
     	(x >= GDK_KEY_space && x <= GDK_KEY_asciitilde)	||\
	(x >= GDK_KEY_Agrave && x <= GDK_KEY_Greek_switch)\
	)

#define ENTRY_KEYSTROKE(x) (	\
	x == GDK_KEY_Escape || x == GDK_KEY_KP_Enter ||	\
	x == GDK_KEY_Return || x == GDK_KEY_Tab ||	\
	CURSOR_KEYSTROKE(x) ||			\
	BASIC_KEYSTROKE(x)			\
	)

#define MAX_COMBO_ELEMENTS 13

#define HISTORY_ITEMS MAX_COMBO_ELEMENTS
/*************************************************************************/

static void on_changed (GtkComboBox *, gpointer );
static gint on_key_press (GtkWidget * , GdkEventKey * , gpointer );
static gint on_key_press_history (GtkWidget * , GdkEventKey * , gpointer );
static int history_compare (gconstpointer , gconstpointer );
static int path_compare (gconstpointer , gconstpointer );
static void clear_association_hash (gpointer , gpointer , gpointer );
/*************** public *****************/
 
// FIXME: Check that all pthread mutexes created in classes
//        are subsequently destroyed by destructor
//        (some are missing this in other classes)
// Constructor 
combobox_c::combobox_c (GtkComboBox *data1, gint data2) {
    sweep_mutex=PTHREAD_MUTEX_INITIALIZER;
    completion_type = data2;
    comboboxentry=data1;

    if (!gtk_combo_box_get_has_entry(comboboxentry)){
	g_error("FIXME: gtk_combo_box_get_has_entry(comboboxentry) == NULL (Set \"has-entry\" property as TRUE on creation of combobox)"); 
    }


    entry=GTK_ENTRY (gtk_bin_get_child (GTK_BIN (comboboxentry)));
    
    g_signal_connect (G_OBJECT (comboboxentry), "changed", G_CALLBACK (on_changed), (gpointer) (void *)this);
    g_signal_connect (G_OBJECT (entry), "key_press_event", G_CALLBACK (on_key_press), (gpointer) (void *)this);
    g_signal_connect (G_OBJECT (entry), "key_press_event", G_CALLBACK (on_key_press_history), (gpointer) (void *)this);

    active_dbh_file = NULL;
    list = NULL;
    cancel_user_data = NULL;
    activate_user_data = NULL;
    cancel_func = NULL;
    activate_func = NULL;

    dead_key=0;
    shift_pos = -1;
    cursor_pos = -1;
    active = -1;
    extra_key_completion = NULL;
    extra_key_data = NULL;
	
    limited_list = NULL;
    association_hash = NULL;
    model=(GtkTreeModel *)gtk_list_store_new (1, G_TYPE_STRING);
    gtk_combo_box_set_model((GtkComboBox *)comboboxentry, model);
                                                         
    gtk_combo_box_set_entry_text_column(comboboxentry, 0);

    return;
}


combobox_c::~combobox_c(void){
    pthread_mutex_destroy(&sweep_mutex);
            
    if (association_hash) {
	g_hash_table_destroy (association_hash);
    }
    g_free (active_dbh_file);
    // free:
    //    ->treemodel
    if (GTK_IS_TREE_STORE (model)) {
	gtk_tree_store_clear ((GtkTreeStore *)model);
    }
    g_object_unref(model);
    //    ->list
    clean_history_list (&(list));
    //    ->limited_list
    clean_history_list (&(limited_list));
    //    ->old_list
    return;
}

gboolean
combobox_c::is_in_history (const gchar *data1, const gchar *data2) {
    if (!data1) {
	DBG("is_in_history: dbh_file==NULL!\n");
	return NULL;
    }
    const gchar *dbh_file = data1;
    const gchar *path2save = data2;
    GString *gs;
    DBHashTable *d;
    //history_dbh_t *history_dbh;
    gboolean found = FALSE;

    if(!path2save) return FALSE;
    if(strlen (path2save) > 255) return FALSE;
    TRACE("opening %s...\n",dbh_file); 
    d = dbh_new (dbh_file, NULL, DBH_READ_ONLY|DBH_PARALLEL_SAFE);
    if(d == NULL) return FALSE;
    dbh_set_parallel_lock_timeout(d, 3);
    TRACE("open %s.\n",dbh_file); 
    gs = g_string_new (path2save);
    sprintf ((char *)DBH_KEY (d), "%10u", g_string_hash (gs));
    g_string_free (gs, TRUE);

    //history_dbh = (history_dbh_t *) DBH_DATA (d);
    if(dbh_load (d)) found = TRUE;
    dbh_close (d);
    return found;
}

gboolean
combobox_c::set_default (void) {
    GSList *list=limited_list;
    if (list) {
	set_entry (list->data);
	return TRUE;
    }
    return FALSE;
}



gboolean
combobox_c::set_entry (const gchar *data) {
    gtk_entry_set_text(entry, data);
    return NULL;
}

GtkEntry *
combobox_c::get_entry_widget (void) {
    return entry;
}

const gchar *
combobox_c::get_entry_text (void) {
    const gchar *choice = gtk_entry_get_text (entry);

    if(choice && strlen (choice) && association_hash) {
        gchar *local_choice = g_hash_table_lookup (association_hash, choice);
        NOOP ("converting back to non utf8 value %s ---> %s\n", choice, local_choice);
        if(local_choice) choice = local_choice;
    }
    if(choice) return choice;
    return "";
}

gboolean
combobox_c::save_to_history (const gchar *data1, const gchar *data2) {
    if (!data1) {
	DBG("save_to_history: dbh_file==NULL!\n");
	return FALSE;
    }
    const gchar *dbh_file = data1;
    const gchar *path2save = data2;
    GString *gs;
    DBHashTable *d;
    history_dbh_t *history_dbh;
    gint size;

    if(!path2save) return FALSE;
    if(strlen (path2save) > 255) return FALSE;

    /* directory test */

    gchar *g = g_path_get_dirname (dbh_file);
    g_mkdir_with_parents (g, 0700);
    if(!g_file_test (g, G_FILE_TEST_IS_DIR)) {
        DBG ("%s is not a directory\n", g);
        g_free (g);
        return FALSE;
    }
    g_free (g);

    // Since this is a user driven command, thread collisions
    // are nearly impossible.
    d = dbh_new (dbh_file, NULL, DBH_PARALLEL_SAFE);
    if(d == NULL) {
        NOOP ("Creating history file: %s", dbh_file);
	unsigned char keylength=11;
        gchar *directory = g_path_get_dirname(dbh_file);
        if (!g_file_test(directory, G_FILE_TEST_IS_DIR)){
            g_mkdir_with_parents(directory, 0700);
        }
        g_free(directory);
        d = dbh_new (dbh_file, &keylength, DBH_PARALLEL_SAFE|DBH_CREATE);
        if(d == NULL)  return FALSE;
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
    return  TRUE;
}

gboolean
combobox_c::remove_from_history (const gchar *data1, const gchar *data2) {
    if (!data1) {
	DBG("save_to_history: dbh_file==NULL!\n");
	return FALSE;
    }
    const gchar *dbh_file = data1;
    const gchar *path2save = data2;
    GString *gs;
    DBHashTable *d;
    //history_dbh_t *history_dbh;

    if(strlen (path2save) > 255)
        return FALSE;
    
    // Since this is a user driven command, thread collisions
    // are nearly impossible.
    d = dbh_new (dbh_file, NULL, DBH_PARALLEL_SAFE);
    if(d == NULL) {
        NOOP ("Creating history file: %s", dbh_file);
	unsigned char keylength=11;
        gchar *directory = g_path_get_dirname(dbh_file);
        if (!g_file_test(directory, G_FILE_TEST_IS_DIR)){
            g_mkdir_with_parents(directory, 0700);
        }
        g_free(directory);
        d = dbh_new (dbh_file, &keylength, DBH_PARALLEL_SAFE|DBH_CREATE);
        if(d == NULL) {
                return FALSE;
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
    return TRUE;
}

void 
combobox_c::set_quick_activate(gboolean data){
    quick_activate = data;
    return;
}

void 
combobox_c::set_extra_key_completion_data(gpointer data){
    extra_key_data = data;
    return;
}

void 
combobox_c::set_extra_key_completion_function(gint (*func)(gpointer)){
    extra_key_completion = func;
    return;
}

void 
combobox_c::set_activate_user_data(gpointer data){
    activate_user_data = data;
    return NULL;
}

void 
combobox_c::set_activate_function(void (*func)(GtkEntry *, gpointer)){
     activate_func = func;
    return NULL;
}

void 
combobox_c::set_cancel_user_data(gpointer data){
    cancel_user_data = data;
	return NULL;
}

void 
combobox_c::set_cancel_function(void (*func)(GtkEntry *, gpointer)){
    cancel_func = func;
    return NULL;
}


gboolean 
combobox_c::read_history (const gchar *data) {
    if (!data) {
	DBG("dbh_file==NULL!\n");
	return FALSE;
    }
    gchar * dbh_file = data;
/*	NOOP("NOOP:at read_history_list with %s \n",dbh_file);*/
    g_free (active_dbh_file);
    active_dbh_file = g_strdup (dbh_file);
    if(access (active_dbh_file, F_OK) != 0) {
        clean_history_list (&(list));
        list = NULL;
    }
    get_history_list (&(list), active_dbh_file, "");
    /* turn asian off to start with. If the combo object does not
     * do a read_history to start, then it has no business being a combo
     * object */
    asian = FALSE;
    return NULL;
}

void 
combobox_c::clear_history (void) {
/*	NOOP("NOOP:at read_history_list with %s \n",dbh_file);*/
    clean_history_list (&(list));
    list = NULL;
    return;
}


////////////////////////////////////////////////////////////////////////////
     /* This is private : */
////////////////////////////////////////////////////////////////////////////

gboolean 
combobox_c::set_combo (const gchar *data) {
    const gchar *token = data;
    int count;
    GSList *tmp;
    GSList **limited_list_p;
    gboolean match = FALSE;

    if(!list) return match;

    old_list = limited_list;
    limited_list = NULL;
    limited_list_p = &(limited_list);
    NOOP ("token=%s\n", ((token) ? token : "null"));


    // sort items after the first
    GSList *first=NULL;
    tmp = list;
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
		*limited_list_p = g_slist_insert_sorted (*limited_list_p, g_strdup (p), path_compare);
	    }
            if(++count >= MAX_COMBO_ELEMENTS)
                break;
        }
    }
    //preprend first item
    if (first) {
	*limited_list_p = 
	    g_slist_prepend (*limited_list_p, g_strdup ((gchar *) first->data));
    }



    if(*limited_list_p) {
        /* make sure we have utf-8 in combo. This may not match
         * character set of actual system, such as euc-jp, so
         * we better keep correct value in an association hash. */
        if(association_hash) {
            /* clean old hash */
            g_hash_table_foreach (association_hash, clear_association_hash, NULL);
            g_hash_table_destroy (association_hash);
            association_hash = NULL;
        }

        association_hash = g_hash_table_new (g_str_hash, g_str_equal);

        if(association_hash) {
            GSList *tmp;
            /* create new hash */
            for(tmp = *limited_list_p; tmp; tmp = tmp->next) {
                gchar *utf_string = valid_utf_pathstring ((gchar *) (tmp->data));
                NOOP("utf_string=%s\n",utf_string); 
                if(strcmp (utf_string, (gchar *) (tmp->data))) {
                    NOOP ("combo hash table %s ---> %s\n", (gchar *) (tmp->data), utf_string);
                    g_hash_table_insert (association_hash, utf_string, tmp->data);
		    g_free(tmp->data);
                    tmp->data = utf_string;
                } else {
		    g_free(utf_string);
		}
            }
        }
	// Set popdown list:
	set_store_data_from_list ((GtkListStore *)model, limited_list_p);

	// Set tooltip to reflect values in popdown list:
	gchar *tooltip_text=NULL;
	GSList *tmp=*limited_list_p;
	GdkPixbuf *tooltip_icon=NULL;
	for (; tmp && tmp->data; tmp=tmp->next){
	    gchar *p=tooltip_text;
	    if (p) tooltip_text = g_strconcat(p,"\n ", (gchar *)(tmp->data),NULL);
	    else tooltip_text = g_strconcat("<b>", _("History:"),"</b>\n ",(gchar *)(tmp->data), NULL);
	    g_free(p);
	}

        // Custom tooltip? Not now, maybe later...
#if 0
	tooltip_icon=rfm_get_pixbuf("xffm/emblem_bookmark", SIZE_DIALOG);
	rfm_add_custom_tooltip(GTK_WIDGET (comboboxentry), tooltip_icon, tooltip_text);
#else
	gtk_widget_set_tooltip_markup (GTK_WIDGET (comboboxentry), tooltip_text);
	gtk_widget_set_tooltip_text (GTK_WIDGET (comboboxentry), tooltip_text); 
#endif
	g_free(tooltip_text);
	
        //gtk_combo_set_popdown_strings (combo_info->combo, *limited_list_p);
        clean_history_list (&(old_list));
    } else {
        limited_list_p = old_list;
	old_list = NULL;
    }
    return match;
}

void
combobox_c::set_blank (void) {
    set_entry ("");
}

void
combobox_c::clean_history_list (GSList ** list) {
    GSList *tmp;
    if(!*list)
        return;
    for(tmp = *list; tmp; tmp = tmp->next) {
        /*NOOP("freeing %s\n",(char *)tmp->data); */
        g_free (tmp->data);
    }
    g_slist_free (*list);
    *list = NULL;
    return;
}

void
combobox_c::history_lasthit (DBHashTable * d) {
    history_dbh_t *history_mem = (history_dbh_t *) DBH_DATA (d);
    if(!history_mem)
        g_assert_not_reached ();
    if(history_mem->last_hit >= last_hit) {
        last_hit = history_mem->last_hit;
    }
}
    
void
combobox_c::history_mklist (DBHashTable * d) {
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

void
combobox_c::get_history_list (GSList ** in_list, char *dbh_file, char *top) {
    DBHashTable *d;
    GSList **the_list;
    GSList *tmp;
/*   char *first=NULL;*/
    the_list = in_list;

    NOOP("NOOP:at get_history_list with %s \n",dbh_file); 

    pthread_mutex_lock(&sweep_mutex);
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
    pthread_mutex_unlock(&sweep_mutex);
    return;
}



gboolean
combobox_c::on_key_press_history(GtkWidget * entry, GdkEventKey * event){
    GtkEditable *editable = (GtkEditable *) entry;
    gint pos1, pos2, pos;
    gboolean preselection;

    NOOP("on_key_press_history: got key= 0x%x\n", event->keyval);

    /* asian input methods: turns off autocompletion */
    if(event->keyval == GDK_KEY_space && (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))) {
        asian = TRUE;
        return FALSE;
    } else if(event->keyval == GDK_KEY_space && (event->state & GDK_MOD1_MASK)) {
        /* turn autocompletion back on */
        asian = FALSE;
        return TRUE;
    }

    if(asian && !(event->keyval == GDK_KEY_Return)
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
        cursor_pos = shift_pos = pos;
    }


    if(dead_key) {
        if(event->keyval != GDK_KEY_Shift_L && event->keyval != GDK_KEY_Shift_R) {
            event->keyval = compose_key (event->keyval, dead_key);
            NOOP ("composing to %c\n", (char)event->keyval);
            dead_key = 0;
        }
    } else {
        dead_key = deadkey (event->keyval);
        NOOP ("deadkey is  0x%x\n", (unsigned)dead_key);
        if(dead_key) return TRUE;
    }

    preselection = gtk_editable_get_selection_bounds (editable, &pos1, &pos2);


    if(!preselection)
        pos1 = pos2 = -1;

    NOOP ("NOOP(2):pos= %d, shift_pos=%d cursor_pos=%d\n", pos, shift_pos, cursor_pos);
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
        if(activate_func) {
            (*(activate_func)) ((GtkEntry *) entry, activate_user_data);
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
                cursor_pos++;
                if(cursor_pos < shift_pos)
                    gtk_editable_select_region (editable, cursor_pos, shift_pos);
                else
                    gtk_editable_select_region (editable, shift_pos, cursor_pos);
            } else {
                gtk_editable_set_position (editable, pos + 1);
                cursor_pos = pos + 1;
            }
        } else if(event->keyval == GDK_KEY_Left || event->keyval == GDK_KEY_KP_Left){
            NOOP("NOOP: left\n"); 
            /* cranky gtk design, position must be at end of selection... */
            if(cursor_pos)
                cursor_pos--;
            if(event->state & GDK_SHIFT_MASK) {
                if(cursor_pos < shift_pos)
                    gtk_editable_select_region (editable, cursor_pos, shift_pos);
                else
                    gtk_editable_select_region (editable, shift_pos, cursor_pos);
            } else if(pos - 1 >= 0) {
                gtk_editable_set_position (editable, pos - 1);
                cursor_pos = pos - 1;
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
                cursor_pos = pos - 1;
                NOOP("NOOP: inserting %s\n",fulltext); 
            } else {
                set_blank ();
            }
            g_free (fulltext);
            fulltext = NULL;
            goto end;
        } else if(event->keyval == GDK_KEY_Delete || event->keyval == GDK_KEY_KP_Delete) {
            if(active_dbh_file && event->state & GDK_CONTROL_MASK) {        /* remove stale entries */
                fulltext = gtk_editable_get_chars (editable, 0, -1);
                if(fulltext && strlen (fulltext)
                   && association_hash) {
                    gchar *local_choice = g_hash_table_lookup (association_hash,
                                                               fulltext);
                    NOOP ("converting back to non utf8 value %s ---> %s\n", fulltext, local_choice);
                    if(local_choice) {
                        g_free (fulltext);
                        fulltext = local_choice;
                    }
                }

                if(fulltext) remove_from_history (active_dbh_file, fulltext);
                set_blank ();

                g_free (fulltext);
                fulltext = NULL;
                if(cancel_func)
                    (*(cancel_func)) ((GtkEntry *) entry, cancel_user_data);
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
                    cursor_pos = pos;
                    NOOP("NOOP: inserting %s\n",fulltext); 
                } else {
                    set_blank ();
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
                cursor_pos = pos;
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
                cursor_pos = pos;
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
    if(find_match && comboboxentry) {
        NOOP("NOOP: setting limited list and emitting signal...fulltext=%s\n",(fulltext)?fulltext:"null"); 
        if(combobox_p->set_combo (fulltext)) {
            if(limited_list && 
		    g_slist_length (limited_list) > 1) 
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
	gint match_count;
	switch(completion_type) {
	    case MATCH_FILE:
		suggest = base_file_suggestion(NULL, token, &match_count);
		break;
	    case MATCH_COMMAND:
		suggest = base_exec_suggestion(NULL, token, &match_count);
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
    if(extra_key_completion){
        (*(extra_key_completion)) (extra_key_data);
    }
    return (TRUE);
returnFALSE:
    if(extra_key_completion){
        (*(extra_key_completion)) (extra_key_data);
    }
    return (FALSE);
}

void
combobox_c::on_changed(GtkComboBox *combo_box){
    gint active = gtk_combo_box_get_active (combo_box);
    NOOP("active=%d\n", active); 
    if(extra_key_completion){
        (*(extra_key_completion)) (extra_key_data);
    }
    if(quick_activate &&
	    active != active && 
	    activate_func) {
            (*(activate_func)) ((GtkEntry *) entry, activate_user_data);
    }
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///  gtk specific callbacks
//


static gint
on_key_press (GtkWidget * entry, GdkEventKey * event, gpointer data) {
    combobox_c *combobox_p = (combobox_c *) data;
    NOOP ("on_key_press: got key= 0x%x\n", event->keyval);
    if(event->keyval == GDK_KEY_Escape && combobox_p->cancel_func) {
        (*(combobox_p->cancel_func)) ((GtkEntry *) entry, combobox_p->cancel_user_data);
        return TRUE;
    }
    return FALSE;
}


static gint
on_key_press_history (GtkWidget * entry, GdkEventKey * event, gpointer data) {
    gchar *utf_fulltext = NULL;
    gboolean find_match = FALSE;
    int i;
    gchar *text[2] = { NULL, NULL };
    gchar c[] = { 0, 0, 0, 0, 0 };
    gchar *fulltext = NULL;
    combobox_c *combobox_p = (combobox_c *) data;
    return combobox_p->on_key_press_history(entry, event);
}

static int path_compare (gconstpointer a, gconstpointer b){
    return strcmp((gchar *)a, (gchar *)b);
}

static void 
on_changed (GtkComboBox *combo_box, gpointer data) {
    combobox_c * combobox_p = (combobox_c *)data;
    combobox_p->on_changed(combo_box);
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
clear_association_hash (gpointer key, gpointer value, gpointer user_data) {
    g_free (key);
    if(!value) return;
    g_free (value);
    return;
}


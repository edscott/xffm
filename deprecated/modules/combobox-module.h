
#ifndef _LIB_COMBOBOX_H_
# define _LIB_COMBOBOX_H_

# include <gtk/gtk.h>
# include <gmodule.h>
# include <time.h>

#if 10
# define COMBOBOX_init_combo(x,y)  \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), GINT_TO_POINTER(y),"init_combo")

# define COMBOBOX_destroy_combo(x)  \
    rfm_natural(RFM_MODULE_DIR,"combobox",(void *)(x),"destroy_combo")

# define COMBOBOX_read_history(x,y) \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "read_history")

# define COMBOBOX_save_to_history(x,y) \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "save_to_history")

# define COMBOBOX_is_in_history(x,y) \
    GPOINTER_TO_INT(rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "is_in_history"))

# define COMBOBOX_remove_from_history(x,y) \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "remove_from_history")

# define COMBOBOX_clear_history(x)  \
    rfm_natural(RFM_MODULE_DIR,"combobox",(void *)(x),"clear_history")

# define COMBOBOX_set_combo(x) \
    rfm_natural(RFM_MODULE_DIR,"combobox",(void *)(x), "set_combo")

# define COMBOBOX_set_default(x)  \
    rfm_natural(RFM_MODULE_DIR,"combobox",(void *)(x),"set_default")

# define COMBOBOX_set_entry(x,y) \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "set_entry")

# define COMBOBOX_get_entry(x)  \
    (const gchar *)(rfm_natural(RFM_MODULE_DIR,"combobox",(void *)(x),"get_entry"))

# define COMBOBOX_get_entry_widget(x)  \
    (GtkEntry *)(rfm_natural(RFM_MODULE_DIR,"combobox",(void *)(x),"get_entry_widget"))

# define COMBOBOX_set_activate_function(x,y) \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "set_activate_function")

# define COMBOBOX_set_activate_user_data(x,y) \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "set_activate_user_data")

# define COMBOBOX_set_quick_activate(x,y) \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "set_quick_activate")

# define COMBOBOX_set_cancel_function(x,y) \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "set_cancel_function")

# define COMBOBOX_set_cancel_user_data(x,y) \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "set_cancel_user_data")

# define COMBOBOX_set_extra_key_completion_function(x,y) \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "set_extra_key_completion_function")

# define COMBOBOX_set_extra_key_completion_data(x,y) \
    rfm_rational(RFM_MODULE_DIR,"combobox",(void *)(x), (void *)(y), "set_extra_key_completion_data")

#else
/* this section is to include the module within a library instead */
void *set_default (void *p);
void *get_entry (void *p);
void *get_entry_widget (void *p);
void *init_combo (void *p);
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

# define COMBOBOX_set_default(x)  set_default((void *)x)
# define COMBOBOX_get_entry(x)  get_entry((void *)x)
# define COMBOBOX_get_entry_widget(x)  get_entry_widget((void *)x)
# define COMBOBOX_init_combo(x)  init_combo((void *)x)
# define COMBOBOX_destroy_combo(x)  destroy_combo((void *)x)
# define COMBOBOX_clear_history(x)  clear_history((void *)x)
# define COMBOBOX_is_in_history(x,y)   is_in_history((void *)(x), (void *)(y))
# define COMBOBOX_set_combo(x)   set_combo((void *)(x))
# define COMBOBOX_set_entry(x,y)   set_entry((void *)(x), (void *)(y))
# define COMBOBOX_save_to_history(x,y)    save_to_history((void *)(x), (void *)(y))
# define COMBOBOX_remove_from_history(x,y)    remove_from_history((void *)(x), (void *)(y))
# define COMBOBOX_read_history(x,y)    read_history((void *)(x), (void *)(y))
# define COMBOBOX_set_extra_key_completion_function(x,y)    set_extra_key_completion_function((void *)(x), (void *)(y))
# define COMBOBOX_set_extra_key_completion_data(x,y)   set_extra_key_completion_data((void *)(x), (void *)(y)) 
# define COMBOBOX_set_activate_function(x,y)    set_activate_function((void *)(x), (void *)(y))
# define COMBOBOX_set_cancel_function(x,y)    set_cancel_function((void *)(x), (void *)(y))
# define COMBOBOX_set_activate_user_data(x,y)   set_activate_user_data((void *)(x), (void *)(y)) 
# define COMBOBOX_set_cancel_user_data(x,y)   set_cancel_user_data((void *)(x), (void *)(y))


#endif


/**
 * is_in_history:
 * @dbh_file: path to dbh file. This is a null terminated string.
 * @path: This is the string to look for in
 * the dbh file. It may be a path, but also can be any null terminated string.
 *
 * Call: if(is_in_history(path2dbh_file,path2find){do_something}
 *
 * Returns: TRUE if operation was successful and the string was found
 * in the dbh file. If string is not found, returns FALSE. Other possible
 * reasons for returning FALSE may include that @dbh_file is not a valid 
 * path or not a valid dbh_file, ergo, the string is not found either.
 *
 **/

//G_MODULE_EXPORT gboolean is_in_history (char *dbh_file, char *path);

/**
 * set_combo:
 *  @combo_info: the autocompletion combo object
 *  @token: the string with which to do autocompletion to produce a
 *  reduced glist for the combo popup. All elements of the popup will
 *  be related to @token by autocompletion 
 * 
 *  sets the glist for the combo, reduced by autocompletion done with @token.
 * summary of keystoke interpretations:
 * control-tab moves to next slash or end of string,
 * tab cycles to the next history element that matches,
 * control-del removes the item from the history.
 *
 * Call: if(set_combo(combo_info, token)){do_something}
 *
 * Returns: TRUE is autocompletion is possible. FALSE is autocompletion
 * produces the null set.

 **/

//G_MODULE_EXPORT const gchar *get_combo (combobox_info_t * combo_info, char *token);

/**
 * get_combo:
 *  @combo_info: the autocompletion combo object
 * 
 *  gets the entry for the combo.
 *
 *  use this function instead of gtk_entry_get_text() or
 *  similar if there is the slightest chance that the
 *  value you want is not utf-8 (the entry text is utf-8)
 *  This happens with euc-jp (japanese) for example.
 *
 *
 * Call: const gchar *g=(get_combo(combo_info));
 *
 * Returns: The const gchar * which corresponds to the
 * combo entry, making any conversion from utf-8 to
 * local character set so that the value will match
 * that which is tabulated in the history file.

 **/

//G_MODULE_EXPORT gboolean set_combo (combobox_info_t * combo_info, char *token);

/**
 * set_blank: 
 * @combo_info: the autocompletion combo object.
 *
 * sets the initial glist for the combo, and puts a blank
 * as the value for the combo entry. The initial list will be made up of
 * the elements with the greatest hit count.
 *
 * Call:  set_blank(combo_info);
 *
 **/

//G_MODULE_EXPORT void  set_blank (combobox_info_t * combo_info);

/**
 * get_entry: 
 * @combo_info: the autocompletion combo object.
 *
 * gets the entry string from the combo.
 * Use this instead of gtk_entry_get_text since this 
 * will give you the non-utf8 (if applicable) value which is actually
 * stored in the history file. This is common in euc-jp 
 * (japanese).
 * 
 * Call:  const gchar *g=get_entry(combo_info);
 *
 *
 * Returns: the string in history which corresponds to the 
 * utf8 string which is displayed in the combo.
 **/

//G_MODULE_EXPORT const gchar *get_entry (combobox_info_t * combo_info);
/**
 * set_entry: 
 * @combo_info: the autocompletion combo object.
 * @top: the null terminated atring to write into the combo entry.
 *
 * sets the initial glist for the combo, and puts a string
 * as the value for the combo entry. The initial list will be made up of
 * the elements with the greatest hit count. The glist is not reduced by
 * the value of @top (use xfce_set_combo() for that).
 *
 * Call:  set_entry(combo_info, entry_string);
 *
 *
 **/

//G_MODULE_EXPORT void  set_entry (combobox_info_t * combo_info, char *top);

/**
 *  @save_to_history: This saves path to dbhashtable or increments
 *  the hit counter if @path is already in the dbhashtable.
 *
 *  @dbh_file: null terminated string pointing to the path of the 
 *  dbh file.
 *  @path: null terminated string which will be added to the dbh file
 *  (if not already in the dbh file), or that will have its hit counter
 *  incremented (if already in dbh file).
 *
 * Call: save_to_history(path2dbh_file,path);
 *
 *  Returns: nothing.
 **/

//G_MODULE_EXPORT void  save_to_history (char *dbh_file, const char *path);

/**
 * remove_from_history:
 *  @dbh_file: null terminated string pointing to the path of the 
 *  dbh file.
 *  @path: null terminated string which will be removed from the dbh file
 * 
 *  This removes @path from the dbhashtable, if it exists.
 * 
 *  Call: remove_from_history(path2dbh_file, path2remove);
 *  
 **/

//G_MODULE_EXPORT void  remove_from_history (char *dbh_file, char *path);

/**
 * init_combo: 
 * @combo: the GtkCombo to associate the autocompletion combo with.
 *
 * Creates a autocompletion combo object from a GtkCombo.
 * This should be called before any other function. It will allocate the
 * necessary memory for the combo object and initialize variables.
 * 
 * Call (example): combo_info = init_combo(combo_info);
 * 
 * Returns: pointer to newly created combo object or NULL if fails.
 * The combo object returned should be freed with destroy_combo()
 * when no longer required.
 * 
 **/

//G_MODULE_EXPORT combobox_info_t *init_combo (GtkComboEntry * comboboxentry);

/**
 * destroy_combo:
 * @combo_info: combo object to free.
 *
 * Destroy and free memory used by a combo autocompletion
 * object.
 * 
 * Call (example): 
 *      combo_info = destroy_combo(combo_info);
 * 
 * Returns: NULL. 
 **/

//G_MODULE_EXPORT combobox_info_t *destroy_combo (combobox_info_t * combo_info);

/**
 * read_history:
 * @combo_info: The autocompletion combo object.
 * @dbh_file: null terminated string with a valid path to the dbh file
 * to be used.
 * 
 * This reads the dbhashtable. This is to be done before all other functions
 * except init_combo() and need be done only once ---except if you want to
 * switch glists on the same combo. See libs/input.c for an example
 * on how to use multiple lists on the same combo. 
 * 
 * Call (example): read_history(combo_info, path2dbh_file);
 *
 **/

//G_MODULE_EXPORT void  read_history (combobox_info_t * combo_info, gchar * dbh_file);

/**
 * clear_history: 
 * @combo_info: The autocompletion combo object
 *
 * This clears the glist associated with the combo. 
 * Only need it if you
 * plan to use multiple glists on the same combo (see libs/input.c 
 * for an example) to avoid memory leaks. Normally not necessary since
 * it is implied by xfce_destroy_combo().
 *
 * Call (example): clear_history(combo_info);
 *
 **/

//G_MODULE_EXPORT void clear_history (combobox_info_t * combo_info);

#endif

#ifndef COMBOBOX_C_HPP
#define COMBOBOX_C_HPP
#include "xffm+.h"


#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"rfm"
#define USER_DBH_CACHE_DIR	USER_RFM_CACHE_DIR,"dbh"
#define RUN_DBH_FILE 		USER_DBH_CACHE_DIR,"run_hash.dbh"
#define RUN_FLAG_FILE 		USER_DBH_CACHE_DIR,"runflag64.dbh"

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

class combobox_c {
    public:
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
	
};

    private:
	GMutex *sweep_mutex;
	time_t last_hit;

	GtkComboBox *comboboxentry;
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

	    

#endif


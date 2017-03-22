#ifndef COMBOBOX_C_HPP
#define COMBOBOX_C_HPP
#include "xffm+.h"
#include <dbh.h>


#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"rfm"
#define USER_DBH_CACHE_DIR	USER_RFM_CACHE_DIR,"dbh"
#define RUN_DBH_FILE 		USER_DBH_CACHE_DIR,"run_hash.dbh"
#define RUN_FLAG_FILE 		USER_DBH_CACHE_DIR,"runflag64.dbh"

#include "base_completion_c.hpp"
#include "utility_c.hpp"

class combobox_c: public base_completion_c, virtual utility_c {
    public:
	// Constructor/destructor
	combobox_c(GtkComboBox *, gint);
	~combobox_c(void);
	gboolean set_default (void);
	const gchar *get_entry_text (void);
	GtkEntry *get_entry_widget (void);
	void clear_history (void);
	gboolean is_in_history (const gchar *, const gchar *);
	gboolean set_combo (void);
        
	gboolean set_entry (const gchar *);
	gboolean save_to_history (const gchar *, const gchar *);
	gboolean remove_from_history (const gchar *, const gchar *);
	gboolean read_history (const gchar *);
	
	void set_extra_key_completion_function (gint (*func)(gpointer));
	void set_extra_key_completion_data (gpointer data);

	void set_activate_user_data (gpointer data);
	void set_activate_function (void (*)(GtkEntry *, gpointer));

	void (*cancel_func) (GtkEntry * entry, gpointer cancel_user_data);
	gpointer cancel_user_data;
        void set_cancel_user_data (gpointer data);
	void set_cancel_function (void (*)(GtkEntry *, gpointer));

	void combo_changed(GtkComboBox *);
	gboolean combo_key_press_history(GtkWidget *, GdkEventKey *, gpointer );

        void set_quick_activate(gboolean);

    private:
	gboolean set_combo (const gchar *);
        void set_blank (void);
        void clean_history_list (GSList **);
        void get_history_list (GSList **, const gchar *, const gchar *);


	pthread_mutex_t sweep_mutex;
	time_t last_hit;

	GtkComboBox *comboboxentry;
	GtkEntry *entry;
	GtkTreeModel *model;
	gchar *active_dbh_file;
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
	gpointer extra_key_data;
	gpointer activate_user_data;

	gint (*extra_key_completion) (gpointer extra_key_data);
	void (*activate_func) (GtkEntry * entry, gpointer activate_user_data);
};

typedef struct history_dbh_t {
    gint64 hits;  // 32/64 issue: gint
    gint64 last_hit; // time_t last_hit;
    gchar path[256];
} history_dbh_t;
        


	    

#endif


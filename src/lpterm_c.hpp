#ifndef LPTERM_C_HPP
#define LPTERM_C_HPP
#include "xffm+.h"
#include "run_c.hpp"
#include "run_button_c.hpp"
#include "data_c.hpp"
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

class lpterm_c: public run_c, virtual utility_c {
    public:
        lpterm_c(data_c *, void *);
        ~lpterm_c(void);
        gboolean is_iconview_key(GdkEventKey *);
        gboolean is_lpterm_key(GdkEventKey *);
        gboolean lp_get_active(void);
        void lp_set_active(gboolean);
	gboolean window_keyboard_event(GdkEventKey *, void *);
        void reference_run_button(run_button_c *);
        void unreference_run_button(run_button_c *);
        void *shell_command(const gchar *);
        void *shell_command(const gchar *, gboolean);
        void open_terminal(void);
	gboolean execute (const gchar *, GList *);

/*
        void recover_flags (gchar * in_cmd, gboolean * interm, gboolean * hold);
        const gchar *what_term (void);
        const gchar *term_exec_option(const gchar *terminal);
*/

    private:
        gboolean active;
	gboolean lpterm_keyboard_event(GdkEventKey *, void *);
        void run_lp_command(void);

        gchar *sudo_fix(const gchar *);
        gboolean process_internal_command (const gchar *);
        gboolean internal_cd(gchar **);
        GtkIconView *iconview;
        GtkWidget *status_button;
        GtkWidget *status_icon;
        GtkWidget *iconview_icon;

        GList *run_button_list;
        pthread_mutex_t *rbl_mutex;
	data_c *data_p;


}; 

#endif

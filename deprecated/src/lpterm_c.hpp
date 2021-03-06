#ifndef LPTERM_C_HPP
#define LPTERM_C_HPP
#include "xffm+.h"
#include "run_c.hpp"
#include "run_button_c.hpp"
#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"rfm"
#define USER_DBH_CACHE_DIR	USER_RFM_CACHE_DIR,"dbh"
#define RUN_DBH_FILE 		USER_DBH_CACHE_DIR,"run_hash.dbh"
#define RUN_FLAG_FILE 		USER_DBH_CACHE_DIR,"runflag64.dbh"


class lpterm_c: public run_c {
    public:
        lpterm_c(void *);
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


}; 

#endif

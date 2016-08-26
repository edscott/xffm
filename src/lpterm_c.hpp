#ifndef LPTERM_C_HPP
#define LPTERM_C_HPP
#include "xffm+.h"
#include "print_c.hpp"

class lpterm_c: public print_c {
    public:
        lpterm_c(void *);
        gboolean is_iconview_key(GdkEventKey *);
        gboolean is_lpterm_key(GdkEventKey *);
        gboolean lp_get_active(void);
        void lp_set_active(gboolean);
	gboolean window_keyboard_event(GdkEventKey *, void *);
/*
        void recover_flags (gchar * in_cmd, gboolean * interm, gboolean * hold);
        const gchar *what_term (void);
        const gchar *term_exec_option(const gchar *terminal);
*/

    private:
        gboolean active;
        void place_cursor(void);
        gchar *get_current_text (void);
        gchar *get_text_to_cursor (void);
	gboolean lpterm_keyboard_event(GdkEventKey *, void *);
        void run_lp_command(void);
        void bash_completion(void);

        GtkWidget *iconview;
        GtkWidget *status_button;
        GtkWidget *status_icon;
        GtkWidget *iconview_icon;

        gboolean csh_completion(gint);
        void place_command(const gchar *);
        void *load_sh_command_history (void);
        void save_sh_command_history (const gchar *);
        gboolean is_valid_command (const gchar *);
        gboolean offset_history(gint);

        pthread_mutex_t command_history_mutex;
        GList *sh_command;
        gint sh_command_counter;

}; 

#endif

#ifndef LPTERM_C_HPP
#define LPTERM_C_HPP
#include "xffm+.h"
#include "csh_completion_c.hpp"

class lpterm_c: public csh_completion_c {
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
	gboolean lpterm_keyboard_event(GdkEventKey *, void *);
        void run_lp_command(void);
        gboolean process_internal_command (const gchar *);
        gboolean internal_cd(gchar **);
        GtkWidget *iconview;
        GtkWidget *status_button;
        GtkWidget *status_icon;
        GtkWidget *iconview_icon;


}; 

#endif

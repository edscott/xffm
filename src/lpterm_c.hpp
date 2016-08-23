#ifndef LPTERM_C_HPP
#define LPTERM_C_HPP
#include "xffm+.h"

class lpterm_c {
    public:
        lpterm_c(GtkWidget *, GtkWidget *);
        gboolean is_iconview_key(GdkEventKey *);
        gboolean is_lpterm_key(GdkEventKey *);
        gboolean lp_get_active(void);
        void lp_set_active(gboolean, void *data);
	gboolean window_keyboard_event(GdkEventKey *, void *);
    private:
        gboolean active;
        void place_cursor(GtkTextView *, gint);
        gchar *get_current_text (GtkTextView *);
        gchar *get_text_to_cursor ( GtkTextView *);
	gboolean lpterm_keyboard_event(GdkEventKey *, void *);
        GtkWidget *status;
        GtkWidget *diagnostics;

};


#endif

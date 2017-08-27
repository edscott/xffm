#ifndef TOOLTIP_C_HPP
#define TOOLTIP_C_HPP
#include "xffm+.h"


class tooltip_c {
    public:
        static void init_tooltip_c(void);
        static GtkWidget * get_tt_window(void);
        static GtkWidget *get_tt_window(const GdkPixbuf *, const gchar *, const gchar *);
        
        static GHashTable *get_tooltip_text_hash(void);
        static void set_tooltip_map(gboolean);
        static gboolean get_tooltip_map(void);
        static void reset_tooltip(void);
        static void custom_tooltip(GtkWidget *, GdkPixbuf *, const gchar *);
        static void set_box_gradient(GtkWidget *wbox);

        
    //private:
        static GHashTable *tooltip_text_hash;
        static GtkWidget *tt_window;
        static GdkPixbuf *shadow_it(const GdkPixbuf *);
        static void tooltip_placement_bug_workaround(GtkWidget *);
        static gboolean tooltip_is_mapped;
};

#endif

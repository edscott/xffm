#ifndef TOOLTIP_C_HPP
#define TOOLTIP_C_HPP
#include "xffm+.h"
#include "utility_c.hpp"


class tooltip_c {
    public:
        tooltip_c(void);
        ~tooltip_c(void);
        GtkWidget * get_tt_window(void);
        GtkWidget *get_tt_window(const GdkPixbuf *, const gchar *, const gchar *);
        
        GHashTable *get_tooltip_text_hash(void);
        void set_tooltip_map(gboolean);
        gboolean get_tooltip_map(void);
        void reset_tooltip(void);
        void custom_tooltip(GtkWidget *, GdkPixbuf *, const gchar *);
        void set_box_gradient(GtkWidget *wbox);

    protected:
        
    private:
        GHashTable *tooltip_text_hash;
        GtkWidget *tt_window = NULL;
        GdkPixbuf *shadow_it(const GdkPixbuf *);
        void tooltip_placement_bug_workaround(GtkWidget *);

        gboolean tooltip_is_mapped;
        utility_c *utility_p;
};

#endif

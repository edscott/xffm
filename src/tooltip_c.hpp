#ifndef TOOLTIP_C_HPP
#define TOOLTIP_C_HPP
#include "xffm+.h"
#include "data_c.hpp"
#include "utility_c.hpp"


class tooltip_c: virtual utility_c {
    public:
        tooltip_c(data_c *);
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
        GtkWidget *tt_window = NULL;
        GdkPixbuf *shadow_it(const GdkPixbuf *);
        void tooltip_placement_bug_workaround(GtkWidget *);
	data_c *data_p;
        gboolean tooltip_is_mapped;
};

#endif

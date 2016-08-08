#ifndef TOOLTIP_C_HPP
#define TOOLTIP_C_HPP
#include "utility_c.hpp"

class tooltip_c: protected utility_c {
    public:
        tooltip_c(void);
        ~tooltip_c(void);
        void set_tt_window(GtkWidget *);
        GtkWidget * get_tt_window(void);
        GHashTable *get_tooltip_text_hash(void);

    protected:
        void custom_tooltip(GtkWidget *, GdkPixbuf *, const gchar *);
        
    private:
        GHashTable *tooltip_text_hash;
        GtkWidget *tt_window = NULL;
        gboolean tooltip_is_mapped;
        GdkPixbuf *shadow_it(const GdkPixbuf *);
        void set_box_gradient(GtkWidget *wbox);
        void tooltip_placement_bug_workaround(GtkWidget *);

};

#endif

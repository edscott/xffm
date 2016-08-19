#ifndef GTK_C_HPP
#define GTK_C_HPP
#include "xffm+.h"
#include "tooltip_c.hpp"
#include "pixbuf_c.hpp"

class gtk_c: public tooltip_c, public pixbuf_c {
    public:
        void setup_image_button (GtkWidget *, const gchar *, const gchar *);    
        GtkWidget *new_add_page_tab(GtkWidget *, GtkWidget **);

};

#endif

#ifndef GTK_C_HPP
#define GTK_C_HPP
#include "xffm+.h"
#include "utility_c.hpp"
#include "tooltip_c.hpp"
#include "pixbuf_c.hpp"

class gtk_c: public utility_c, public tooltip_c, public pixbuf_c {
    public:
        void setup_image_button (GtkWidget *, const gchar *, const gchar *);    
        GtkWidget *new_add_page_tab(GtkWidget *, GtkWidget **);
	void set_bin_contents(GtkWidget *, const char *, const char *, gint);
	void set_bin_markup(GtkWidget *, const char *);
    protected:
    private:
	void set_bin_image(GtkWidget *, const gchar *, gint);
};

#endif

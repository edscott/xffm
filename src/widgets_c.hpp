#ifndef WIDGETS_C_HPP
#define WIDGETS_C_HPP
#include <gtk/gtk.h>

class widgets_c {
    public:
    protected:
	GtkWidget * 
	    mk_little_button (const gchar *, 
		    void *, 
		    void *, 
		    const gchar *);
	void 
	    add_custom_tooltip(GtkWidget *widget, 
		    GtkWidget *image,
		    const gchar *tooltip_text);
    private:

};

#endif

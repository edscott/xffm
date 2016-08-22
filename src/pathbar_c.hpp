#ifndef PATHBAR_C_HPP
#define PATHBAR_C_HPP
#include "gtk_c.hpp"

class pathbar_c: utility_c {
    public:
	pathbar_c(void *window_v, GtkNotebook *data);
	GtkWidget *get_pathbar(void);
        void pathbar_ok(GtkButton *);
        void toggle_pathbar(const gchar *);
	void update_pathbar(const gchar *);
        GtkWidget *pathbar_button (const char *, const char *);
    protected:
	GtkWidget *pathbar;
    private:
        GtkNotebook *notebook;
        void *window_p;
        gtk_c *gtk_p;

};

#endif

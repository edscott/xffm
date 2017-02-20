#ifndef PATHBAR_C_HPP
#define PATHBAR_C_HPP
#include "gtk_c.hpp"

class pathbar_c: virtual utility_c, public gtk_c {
    public:
	pathbar_c(data_c *, void *window_v, GtkNotebook *data);
	GtkWidget *get_pathbar(void);
        void pathbar_ok(GtkButton *);
        void toggle_pathbar(const gchar *);
	void update_pathbar(const gchar *);
        GtkWidget *pathbar_button (const char *, const char *);
	GtkWidget *pathbar;
    protected:
    private:
        GtkNotebook *notebook;
        void *window_p;
        void *view_p;

};

#endif

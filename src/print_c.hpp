#ifndef PRINT_C_HPP
#define PRINT_C_HPP
#include "xffm+.h"
#include "gtk_c.hpp"
#include "utility_c.hpp"


class print_c: public utility_c {
    public:
        print_c(void *);
        void print_error(const gchar *format, ...);
        void print_debug(const gchar *format, ...);
        void print(const gchar *format, ...);
        void print_tag(const gchar *tag, const gchar *format, ...);
	void print_icon(const gchar *iconname, const gchar *format, ...);
        void print_icon_tag(const gchar *iconname, const gchar *tag, const gchar *format, ...);

        void print_status(const gchar *format, ...);
	GtkTextView *get_diagnostics(void);
	GtkLabel *get_status_label(void); 

        void clear_text(void);
        void show_text(void);
    private:
        GtkTextView *diagnostics;
        GtkLabel *status_label;
        void *view_v;
	gtk_c *gtk_p;

};



#endif

#ifndef PRINT_C_HPP
#define PRINT_C_HPP
#include "xffm+.h"
#include "gtk_c.hpp"
#include "utility_c.hpp"
#include "data_c.hpp"


class print_c: virtual utility_c, public gtk_c {
    public:
        print_c(data_c *, void *);
        void print_error(gchar *);
        void print_debug(gchar *);
        void print(gchar *);
        void print_tag(const gchar *tag, gchar *);
	void print_icon(const gchar *iconname, gchar *);
        void print_icon_tag(const gchar *iconname, const gchar *tag, gchar *);

        void print_status(gchar *);
        void print_status_label(gchar *);
	GtkTextView *get_diagnostics(void);
	GtkTextView *get_status(void); 
	GtkLabel *get_status_label(void); 

        void clear_text(void);
        void show_text(void);

        void *scroll_to_top(void);
        void *scroll_to_bottom(void);
        
        gboolean trim_diagnostics(GtkTextBuffer *);
        GtkTextTag **resolve_tags(GtkTextBuffer *, const gchar *);       
        void insert_string (GtkTextBuffer *, const gchar *, GtkTextTag **);
    protected:

        gchar *get_tilde_dir(const gchar *);
        gchar *get_current_text (void);
	gchar *get_text_to_cursor (void);
        const gchar *get_workdir(void);
        void clear_status(void);

        GtkTextView *status;
        GtkTextView *diagnostics;
        GtkLabel *status_label;
        void *view_v;
        
    private:



};



#endif

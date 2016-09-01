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
        void print_status_label(const gchar *format, ...);
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
	gtk_c *gtk_p;
        
    private:



};



#endif

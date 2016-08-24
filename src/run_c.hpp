#ifndef RUN_C_HPP
#define RUN_C_HPP
#include "xffm+.h"
#include "view_c.hpp"


class run_c {
    public:
        run_c(view_c *);
        void print_error(const gchar *format, ...);
        void print_debug(const gchar *format, ...);
        void print_diagnostics(const gchar *tag, const gchar *format, ...);
        void print_status(const gchar *tag, const gchar *format, ...);

        void clear_text_buffer(GtkWidget *data);
        pid_t thread_run(gchar **arguments);
        void *gtk_context(void * (*function)(gpointer), void * function_data);
        void print(GtkWidget *textview, const gchar *tag, const gchar *string);
        void set_debug(gboolean);
    private:
        GtkTextView *diagnostics;
        GtkTextView *status;
        gboolean debug;

};



#endif

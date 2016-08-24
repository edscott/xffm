#ifndef PRINT_C_HPP
#define PRINT_C_HPP
#include "xffm+.h"
#include "utility_c.hpp"


class print_c: public utility_c {
    public:
        print_c(void *);
        /*void print_error(const gchar *format, ...);
        void print_debug(const gchar *format, ...);*/
        void print(const gchar *tag, const gchar *format, ...);
        void print_status(const gchar *tag, const gchar *format, ...);
        //void print_full(const gchar *iconname, const gchar *tag, const gchar *format, ...);

        // XXX duplicated in other class?
        void clear_text_buffer(GtkWidget *data);
    private:
        GtkTextView *diagnostics;
        GtkLabel *status_label;
        void *view_v;

};



#endif

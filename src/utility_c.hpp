#ifndef UTILITY_C_HPP
#define UTILITY_C_HPP
#include "xffm+.h"

#include <pthread.h>
#include <string.h>

class utility_c{
    public:
        utility_c(void);
        ~utility_c(void);
        void *context_function(void * (*function)(gpointer), void * function_data);
        gchar *utf_string (const gchar *);

    protected:
	const gchar *chop_excess (gchar *);
        gchar *compact_line(const gchar *);
        GList *find_in_string_list(GList *, const gchar *);
        const gchar *u_shell(void);
        gchar *esc_string (const gchar *);
        const gchar *what_term (void);
        const gchar *term_exec_option(const gchar *);
        const gchar **get_terminals(void);
        const gchar **get_editors(void);
    private:
        const gchar *default_shell(void);
        gboolean program_in_path(const gchar *);


};

#endif

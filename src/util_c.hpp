#ifndef UTIL_C_HPP
#define UTIL_C_HPP
#include "xffm+.h"

class util_c{
    public:
        static void threadwait (void);
        static gchar *utf_string (const gchar *);
        static void *context_function(void * (*function)(gpointer), void * function_data);
        //gchar *wrap_utf_string(const gchar *, gint);
        static gint length_equal_string(const gchar *, const gchar *);
        static gchar *get_tilde_dir(const gchar *);
	static const gchar *chop_excess (gchar *);
        static gchar *compact_line(const gchar *);
        static GList *find_in_string_list(GList *, const gchar *);
        static const gchar *u_shell(void);
        static gchar *esc_string (const gchar *);
        static const gchar *what_term (void);
        static const gchar *term_exec_option(const gchar *);
        static const gchar **get_terminals(void);
        static const gchar **get_editors(void);
        static gchar *get_text_editor(void);
        static void set_store_data_from_list(GtkListStore *, GSList **);
        static gchar *recursive_utf_string (const gchar *);
	static gint deadkey(gint);
        static gint compose_key(gint, gint);
        static gint translate_key(gint);
        static gchar *valid_utf_pathstring (const gchar *);
        static const gchar *default_shell(void);
        static gboolean program_in_path(const gchar *);

};

#endif


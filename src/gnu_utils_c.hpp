#ifndef GNU_UTILS_C_HPP
#define GNU_UTILS_C_HPP
#include "xffm+.h"

class gnu_utils_c{
    public:
        static gboolean cp(void *, GList *, const gchar *, const gchar *);
        static gboolean mv(void *, GList *, const gchar *, const gchar *);
        static gboolean ln(void *, GList *, const gchar *, const gchar *);
        static gboolean rm(void *, GList *, const gchar *);
        static gboolean shred(void *, GList *, const gchar *);
        
        static pid_t execute_command(void *, const gchar **);
        static gchar **get_command_argv(const gchar *,GList *, const gchar *, const gchar *);
        static gchar **non_empty_strsplit(const gchar *, const gchar *);

};


#endif

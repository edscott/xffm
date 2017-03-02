#ifndef GNU_UTILS_C_HPP
#define GNU_UTILS_C_HPP
#include "xffm+.h"

class gnu_utils_c{
    public:
        gnu_utils_c(){};
        ~gnu_utils_c(){};
    protected:
        gboolean cp(GList *, const gchar *, const gchar *);
        gboolean mv(GList *, const gchar *, const gchar *);
        gboolean ln(GList *, const gchar *, const gchar *);
        gboolean rm(GList *, const gchar *);
        gboolean shred(GList *, const gchar *);
    private:
        gchar *execute_command(gchar *);
        gchar *get_command(const gchar *,GList *, const gchar *, const gchar *);

};


#endif

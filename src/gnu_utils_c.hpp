#ifndef GNU_UTILS_C_HPP
#define GNU_UTILS_C_HPP
#include "xffm+.h"

class gnu_utils_c{
    public:
        gnu_utils_c(void);
        ~gnu_utils_c(void){};
    protected:
        gboolean cp(void *, GList *, const gchar *, const gchar *);
        gboolean mv(void *, GList *, const gchar *, const gchar *);
        gboolean ln(void *, GList *, const gchar *, const gchar *);
        gboolean rm(void *, GList *, const gchar *);
        gboolean shred(void *, GList *, const gchar *);
    private:
        gchar *execute_command(void *, gchar *);
        gchar *execute_command(void *, gchar **);
        gchar *get_command(const gchar *,GList *, const gchar *, const gchar *);

};


#endif

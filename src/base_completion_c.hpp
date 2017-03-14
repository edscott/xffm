#ifndef BASE_COMPLETION_C_HPP
#define BASE_COMPLETION_C_HPP
#include "xffm+.h"
#include "utility_c.hpp"
#define BASH_COMPLETION_OPTIONS maximum_completion_options()
class base_completion_c: virtual utility_c {
    public:

    protected:
        GSList *base_file_completion(const char *, const char *, gint *);
        gchar *base_file_suggestion(const char *, const char *, gint *);
        gchar *top_match (GSList **);
        glong maximum_completion_options(void);
	
    private:
};

#endif

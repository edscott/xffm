#ifndef BASE_COMPLETION_C_HPP
#define BASE_COMPLETION_C_HPP
#include "xffm+.h"
#define BASH_COMPLETION_OPTIONS maximum_completion_options()
class base_completion_c {
    public:

    protected:
        GSList *base_file_completion(const char *, const char *, gint *);
        gchar *base_file_suggestion(const char *, const char *, gint *);
        GSList *base_exec_completion(const char *, const char *, gint *);
        gchar *base_exec_suggestion(const char *, const char *, gint *);
        gchar *top_match (GSList **);
        glong maximum_completion_options(void);
        void free_match_list(GSList *);
	
    private:
        gchar *get_token(const char *, gint *);
        gchar *base_suggestion(gint, const char *, const char *, gint *);

};

#endif

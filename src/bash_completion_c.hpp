#ifndef BASH_COMPLETION_C_HPP
#define BASH_COMPLETION_C_HPP
#include "xffm+.h"
#include "print_c.hpp"


class bash_completion_c: public print_c {
    public:
	bash_completion_c(void *);

    protected:
        void bash_completion(void);
        gchar *bash_suggetion(const gchar *, gint);

	
    private:
        glong maximum_completion_options(void);
        void msg_too_many_matches(void);
        const gchar *get_match_type_text(gint match_type);
        void msg_show_match(gint match_type, const gchar *match);
        void msg_help_text(void);
        void msg_result_text(gint match_type);
        const gchar *get_workdir(void);
        gchar *list_matches (GSList **matches_p, gint match_type);
        gchar *complete_it(GSList **matches_p, gint match_type);

        gchar *file_completion(gchar *);
        gchar *bash_file_completion(const char *in_file_token, gint *match_count_p);
        gchar *bash_exec_completion(const char *in_token, gint *match_count_p);
        gchar *variable_complete(const gchar *token, gint *match_p);
        gchar *userdir_complete(const gchar *token, gint *match_p);
        gchar *hostname_complete(const gchar *token, gint *match_p);
        gchar *extra_completion(const gchar *token, gint *matches_p);
        gchar *extra_space(gchar *suggest, gint *matches_p);
        gchar *bash_complete(const gchar *token, gint *matches_p);
        gchar *bash_complete_with_head(const gchar *in_token, gint *matches_p);
        gboolean valid_token(const gchar *token);

};
#endif

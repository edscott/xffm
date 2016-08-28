#ifndef PRINT_C_HPP
#define PRINT_C_HPP
#include "xffm+.h"
#include "gtk_c.hpp"
#include "utility_c.hpp"


class print_c: public utility_c {
    public:
        print_c(void *);
        void print_error(const gchar *format, ...);
        void print_debug(const gchar *format, ...);
        void print(const gchar *format, ...);
        void print_tag(const gchar *tag, const gchar *format, ...);
	void print_icon(const gchar *iconname, const gchar *format, ...);
        void print_icon_tag(const gchar *iconname, const gchar *tag, const gchar *format, ...);

        void print_status(const gchar *format, ...);
        void print_status_label(const gchar *format, ...);
	GtkTextView *get_diagnostics(void);
	GtkTextView *get_status(void); 
	GtkLabel *get_status_label(void); 

        void clear_text(void);
        void show_text(void);

        gchar *suggest_bash_complete(const gchar *, gint);
        gchar *file_completion(gchar *);
        gchar *get_tilde_dir(const gchar *);
        void *scroll_to_top(void);
        void *scroll_to_bottom(void);
    protected:
        GtkTextView *status;
        GtkTextView *diagnostics;
        GtkLabel *status_label;
        void *view_v;
	gtk_c *gtk_p;
        
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

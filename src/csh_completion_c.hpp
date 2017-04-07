#ifndef COMPLETION_C_HPP
#define COMPLETION_C_HPP
#include "xffm+.h"
#include "bash_completion_c.hpp"
#define CSH_HISTORY 	USER_RFM_CACHE_DIR,"lp_terminal_history"


class csh_completion_c: public bash_completion_c, virtual utility_c {
    public:
	csh_completion_c(void *);
        gboolean csh_is_valid_command (const gchar *);

    protected:
        gboolean is_completing(void);
        gboolean query_cursor_position(void);
        gboolean csh_completion(gint);
        gboolean csh_history(gint);
        void csh_save_history (const gchar *);
        void csh_clean_start(void);
        void csh_dirty_start(void);
    private:
        const gchar *csh_find(const gchar *, gint);
        void csh_place_command(const gchar *);
        void *csh_load_history (void);
	
	void place_cursor(void);
	
        GList *csh_history_list;

	gchar *csh_cmd_save;
	gint csh_nth;
	gint csh_history_counter;
        gboolean csh_completing;

};

#endif

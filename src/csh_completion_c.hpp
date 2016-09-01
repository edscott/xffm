#ifndef COMPLETION_C_HPP
#define COMPLETION_C_HPP
#include "xffm+.h"
#include "bash_completion_c.hpp"
#define CSH_HISTORY 	USER_RFM_CACHE_DIR,"lp_terminal_history"


class csh_completion_c: public bash_completion_c {
    public:
	csh_completion_c(void *);

    protected:
        gboolean csh_completion(gint);
        void csh_save_history (const gchar *);
        void csh_clean_start(void);
        void csh_dirty_start(void);
    private:
        const gchar *csh_find(const gchar *, gint);
        void csh_place_command(const gchar *);
        void *csh_load_history (void);
        gboolean csh_is_valid_command (const gchar *);
        gboolean csh_offset_history(gint);
	
	void place_cursor(void);
	
        GList *csh_command_list;

	gchar *csh_cmd_save;
	gint csh_nth;
	gint csh_command_counter;
        gboolean csh_completing;

};

#endif

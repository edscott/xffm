#ifndef COMPLETION_C_HPP
#define COMPLETION_C_HPP
#include "xffm+.h"
#include "bash_completion_c.hpp"
#define CSH_HISTORY 	USER_RFM_CACHE_DIR,"lp_terminal_history"


class csh_completion_c: public bash_completion_c {
    public:
	csh_completion_c(void *);

    protected:
	void csh_completion_init(void);
        gboolean csh_completion(gint, gint);
        void csh_save_history (const gchar *);
        void csh_set_completing(gboolean);

	
    private:
        void csh_place_command(const gchar *);
        void *csh_load_history (void);
        gboolean csh_is_valid_command (const gchar *);
        gboolean csh_offset_history(gint);
	
	void place_cursor(void);
	
        pthread_mutex_t csh_command_mutex;
        GList *csh_command_list;
        gint csh_command_counter;

	gint csh_cmd_len;
	gchar *csh_cmd_save;
	gint csh_nth;
        gboolean csh_completing;

};

#endif

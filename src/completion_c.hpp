#ifndef COMPLETION_C_HPP
#define COMPLETION_C_HPP
#include "xffm+.h"
#include "print_c.hpp"
#define CSH_HISTORY 	USER_RFM_CACHE_DIR,"lp_terminal_history"


class completion_c: public print_c {
    public:
	completion_c(void *);
    protected:
	void completion_init(void);
        gboolean csh_completion(gint, gint);
	void *view_v;
	gchar *get_current_text (void);
	gchar *get_text_to_cursor (void);
	
    private:
        void csh_place_command(const gchar *);
        void *csh_load_history (void);
        void csh_save_history (const gchar *);
        gboolean csh_is_valid_command (const gchar *);
        gboolean csh_offset_history(gint);
	
	void place_cursor(void);
	
        pthread_mutex_t csh_command_mutex;
        GList *csh_command_list;
        gint csh_command_counter;

	gint csh_cmd_len;
	gchar *csh_cmd_save;
	gint csh_nth;

};

#endif

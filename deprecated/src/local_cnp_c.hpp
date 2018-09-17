#ifndef LOCAL_CNP_C_HPP
#define LOCAL_CNP_C_HPP
#include "xffm+.h"

class  local_cnp_c{
    public:
        local_cnp_c(void *);
    protected:
    private:
        void *view_v;

        void clear_paste_buffer(void);
        void store_paste_buffer(gchar *, gint);
        gint get_paste_length(void);
        gchar *get_paste_buffer (void);
        gint pasteboard_status (void);

        gchar **pasteboard_v(void);
        gint in_pasteboard (const gchar *);
        gboolean update_pasteboard (void);

        gchar *xbuffer;
        
};

#endif

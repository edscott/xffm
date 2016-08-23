#ifndef LPTERM_C_HPP
#define LPTERM_C_HPP
#include "xffm+.h"

class lpterm_c {
    public:
        lpterm_c(void);
        gboolean is_iconview_key(GdkEventKey *);
        gboolean is_lpterm_key(GdkEventKey *);
        gboolean lp_get_active(void);
        void lp_set_active(gboolean);
	gboolean window_keyboard_event(GdkEventKey *);
    private:
        gboolean active;


};


#endif

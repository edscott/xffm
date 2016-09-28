#ifndef PROPERTIES_C_HPP
#define PROPERTIES_C_HPP
#include "xffm+.h"

class properties_c {
    public:
        properties_c(void);
        ~properties_c(void);
        void do_prop(const gchar *);
        void do_prop(GList *);

    private:
        GList *selection_list;
        void clear_selection_list(void);
        void *setup(const gchar *);

};

#endif

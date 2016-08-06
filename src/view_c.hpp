#ifndef VIEW_C_HPP
#define VIEW_C_HPP
#include <gtk/gtk.h>
#include <pthread.h>

#include "notebook_page_c.hpp"


class view_c:public notebook_page_c {
    public:
        view_c::view_c(void);
        view_c::~view_c(void);

    protected:

    private:
        void init(void);

        gchar *workdir;		
        pthread_mutex_t *population_mutex;
        pthread_cond_t *population_cond;
        gint population_condition;
        pthread_rwlock_t *population_lock;

};

#endif

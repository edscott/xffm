#ifndef VIEW_C_HPP
#define VIEW_C_HPP
#include <string.h>
#include "utility_c.hpp"
//#include "signals_c.hpp"
#include "widgets_c.hpp"
#include "xfdir_c.hpp"



class view_c:public widgets_c {
    public:
        view_c(void *, GtkNotebook *);
        ~view_c(void);
        GtkTreeModel *get_treemodel(void);
        void set_treemodel(xfdir_c *);
        void clear_diagnostics(void);
        void *get_window_p(void);
        GtkWidget *get_page_child_box(void);

    protected:

    private:
        GtkWidget *icon_view;          
        
        void init(void);
        void pack();
        void signals();

        utility_c *utility_p;

        void *window_p;

        gchar *workdir;		
        pthread_mutex_t population_mutex;
        pthread_cond_t population_cond;
        gint population_condition;
        pthread_rwlock_t population_lock;

        gchar *last_motion_name;

        xfdir_c *xfdir_p;

};

#endif

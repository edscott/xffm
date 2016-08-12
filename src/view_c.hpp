#ifndef VIEW_C_HPP
#define VIEW_C_HPP
#include <string.h>
#include "utility_c.hpp"
#include "signals_c.hpp"
#include "widgets_c.hpp"



class view_c:public widgets_c {
    public:
        view_c(void *, GtkWidget *, GtkWidget *);
        ~view_c(void);
        void set_treemodel(GtkTreeModel *);
        void clear_diagnostics(void);
        void *get_window_p(void);
        GtkWidget *get_page_child_box(void);

    protected:

    private:
        GtkWidget *new_tab_child;
        GtkWidget *icon_view;          
        GtkTreeModel *tree_model;
        
        void init(void);
        void pack();
        void signals();
        GtkListStore *list_store;

        signals_c *signals_p;
        utility_c *utility_p;

        void *window_p;

        gchar *workdir;		
        pthread_mutex_t population_mutex;
        pthread_cond_t population_cond;
        gint population_condition;
        pthread_rwlock_t population_lock;

};

#endif

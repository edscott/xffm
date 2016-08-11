#ifndef VIEW_C_HPP
#define VIEW_C_HPP
#include <string.h>
#include "utility_c.hpp"
#include "signals_c.hpp"
#include "widgets_c.hpp"



class view_c:public widgets_c {
    public:
        view_c(GtkWidget *, GtkWidget *);
        ~view_c(void);
        void set_treemodel(GtkTreeModel *);
        void clear_diagnostics(void);

    protected:
        void clear_text (GtkWidget *);
        void hide_text (GtkWidget *);
        GtkWidget *icon_view;           // drawing area

    private:
        GtkWidget *new_tab_child;
        
        void init(void);
        void pack();
        void signals();
        GtkListStore *list_store;

        signals_c *signals_p;
        utility_c *utility_p;

        gchar *workdir;		
        pthread_mutex_t population_mutex;
        pthread_cond_t population_cond;
        gint population_condition;
        pthread_rwlock_t population_lock;

};

#endif

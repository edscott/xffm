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
        void *get_window_p(void);
	gint get_dir_count(void);
	
        GtkTreeModel *get_treemodel(void);
        void set_treemodel(xfdir_c *);
        void clear_diagnostics(void);
        void set_highlight(gdouble, gdouble);
        void highlight(void);
        void clear_highlights(const gchar *);
        void reload(const gchar *);
        void remove_page(void);

    protected:

    private:
	xfdir_c *get_xfdir_p(void);
        
        GtkIconView *icon_view;          
        GHashTable *highlight_hash;
        gboolean dirty_hash = FALSE;

        
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

        gint highlight_x;
        gint highlight_y;

        xfdir_c *xfdir_p;

};

#endif

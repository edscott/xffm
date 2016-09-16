#ifndef VIEW_C_HPP
#define VIEW_C_HPP
#include "xffm+.h"
#include <string.h>
#include "widgets_c.hpp"
#include "lpterm_c.hpp"
#include "print_c.hpp"
#include "xfdir_c.hpp"
#include "thread_control_c.hpp"



class view_c:public widgets_c, public thread_control_c {
    public:
        view_c(void *, GtkNotebook *);
        ~view_c(void);

	gint get_dir_count(void);

        const gchar *get_path(void);
	
        void clear_diagnostics(void);
        void clear_status(void);
        void show_diagnostics(void);
        void set_highlight(gdouble, gdouble);
        void highlight(void);
        void clear_highlights(const gchar *);
        void remove_page(void);
        void reload(const gchar *);
        void set_treemodel(xfdir_c *);
        GtkTreeModel *get_tree_model(void);
        void set_page_label(void) ;
	gint get_icon_size(const gchar *);
	void set_window_title(void);
	void set_application_icon(void);
	void set_window_title(gint);
	void set_application_icon(gint);
	xfdir_c *get_xfdir_p(void);
        GtkWindow *get_window(void);

        void root(void);

	gboolean window_keyboard_event(GdkEventKey *, void *);
        lpterm_c *get_lpterm_p(void);

    protected:

    private:
	void update_tab_label_icon(void);
	void set_view_details(void);
        
        GHashTable *highlight_hash;
        gboolean dirty_hash = FALSE;
        void init(void);
        void signals();

        gint highlight_x;
        gint highlight_y;

        lpterm_c *lpterm_p;
        xfdir_c *xfdir_p;

};

#endif

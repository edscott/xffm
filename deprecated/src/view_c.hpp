#ifndef VIEW_C_HPP
#define VIEW_C_HPP
#include "xffm+.h"
#include "widgets_c.hpp"
#include "lpterm_c.hpp"
#include "print_c.hpp"

#include "thread_control_c.hpp"

#include "xfdir_root_c.hpp"
#include "xfdir_local_c.hpp"
#define  ROOT_CLASS	xfdir_root_c 	
#define  LOCAL_CLASS	xfdir_local_c 	

enum {
    ROOT_TYPE,
    LOCAL_TYPE,
    UNDEFINED_TYPE
};
class view_c: public widgets_c, public thread_control_c {
    public:
        view_c(void *, GtkNotebook *, const gchar *);
        ~view_c(void);
        void reload(const gchar *);
        GtkTreeModel *get_tree_model(void);
        const gchar *get_path(void);
	gboolean window_keyboard_event(GdkEventKey *, void *);
        void highlight(void);
        void highlight(gdouble, gdouble);
        void remove_page(void);
        GtkWindow *get_window(void);
        lpterm_c *get_lpterm_p(void);
	xfdir_c *get_xfdir_p(void);
	void set_window_title(void);
	void set_window_title(gint);
	gint get_dir_count(void);
	void set_application_icon(gint);
        void show_diagnostics(void);
        void clear_diagnostics(void);
        void clear_status(void);
        void root(void);
        void setup_tooltip(gint, gint);
        gboolean shows_hidden(void);
        void toggle_show_hidden(void);
        void set_drag_mode(gint);
        gint get_drag_mode(void);
        GtkTargetList *get_target_list(void);
        void free_selection_list(void);
        void set_selection_list(GList *);
        GList *get_selection_list(void);
        void set_click_cancel(gint);
        gboolean get_click_cancel(void);

	gboolean all_set_up;
   protected:
	// This class is base for none

    private:
        void set_page_label(void) ;
	void set_application_icon(void);

	void update_tab_label_icon(void);
	void set_view_details(void);
        
        void init(void);
        void signals();

        gint highlight_x;
        gint highlight_y;

        gint button_x;
        gint button_y;

        lpterm_c *lpterm_p;
        xfdir_c *xfdir_p;
    
        void create_target_list (void);
        GtkTargetList	*target_list;
        GList *selection_list;

        gint drag_mode;
        gint click_cancel;

	gint xfdir_type;
	gint get_xfdir_type(const gchar *);
	xfdir_c *create_xfdir_p(gint, const gchar *);
	void delete_xfdir_c(gint, xfdir_c *);
        void set_treemodel(gint, xfdir_c *);
};

#endif

#ifndef XFDIR_LOCAL_C_HPP
#define XFDIR_LOCAL_C_HPP
#include "xfdir_c.hpp"
#include "local_monitor_c.hpp"

class xfdir_local_c: public local_monitor_c, public local_dnd_c{
    public:
	xfdir_local_c(const gchar *, void *);
        ~xfdir_local_c(void);
        void reload(const gchar *);
        gchar *make_tooltip_text (GtkTreePath *);
        void item_activated (GtkIconView *, GtkTreePath *, void *);
	const gchar *get_xfdir_iconname(void);
	
	gboolean set_dnd_data(GtkSelectionData *, GList *);
	gboolean receive_dnd(const gchar *, GtkSelectionData *, GdkDragAction);
        virtual void highlight_drop(GtkTreePath *);
        
	gboolean popup(GtkTreePath *);
        void open_new_tab(void);
	
       
    protected:

    private:
        GtkMenu *get_directory_menu(void){return directory_menu;}
        GtkTreeModel *mk_tree_model(void);
        gint heartbeat;
        GList *read_items (gint *); 
        void insert_list_into_model(GList *, GtkListStore *);

	gboolean remove_url_file_prefix (gchar *);
	gboolean remove_file_prefix_from_uri_list (GList *);
	gint parse_url_list(const gchar *, GList **);

        GList *sort_directory_list(GList *);
    private:
        void create_menu(void);
        void destroy_tree_model(void);    
        gchar *get_path_info (GtkTreeModel *, GtkTreePath *, const gchar *);

        gchar *path_info(const gchar *, struct stat *, const gchar *, 
                const gchar *, const gchar *);
        gint count_files (const gchar *);
        gint count_hidden_files (const gchar *);
        gchar ftypelet (mode_t);
        
        gchar *user_string (struct stat *);
        gchar *group_string (struct stat *);
        gchar *mode_string (mode_t mode);
        gchar *date_string (time_t);
        gchar *sizetag (off_t, gint);

        pthread_mutex_t user_string_mutex;
        pthread_mutex_t group_string_mutex;
        pthread_mutex_t date_string_mutex;

        GtkMenu *selection_menu;
        GtkMenu *directory_menu;

	static pthread_mutex_t readdir_mutex;
        void *view_v;

};


#endif

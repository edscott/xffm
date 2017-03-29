#ifndef LOCAL_DND_C_HPP
#define LOCAL_DND_C_HPP
#include "xffm+.h"
#include "gnu_utils_c.hpp"

class  local_dnd_c: virtual gnu_utils_c{
    public:
        local_dnd_c(void *);
        gboolean _set_dnd_data(GtkSelectionData * , GList *, GtkTreeModel *, const gchar *, gint);
        gboolean _receive_dnd(const gchar *, const gchar *, GtkSelectionData *, GdkDragAction );
    protected:
    private:
        gchar * get_options(GdkDragAction , GList *);
        gboolean remove_url_file_prefix (gchar * );
        gboolean remove_file_prefix_from_uri_list (GList * );
        gint parse_url_list(const gchar * DnDtext, GList ** );

        void free_src_list(GList *);
        gchar *get_fulltarget(const gchar *, const gchar *, GList *);
        gboolean is_nonsense(const gchar *);
        void show_message_dialog(GtkDialog *);
        
        void touch_rc_file(void);
        gchar *get_rc_option(GList *);
        gchar *get_default_options(GdkDragAction);
        gboolean user_selected_options(gchar **, GList *);
        gboolean get_dialog_options(gchar **, GList *);

        const gchar *vv;
        const gchar *operation;
        const gchar *which;
        gchar *no_c;
        gchar *fulltarget;
        gchar *kfile;
        GKeyFile *key_file;

        void *view_p;
};

#endif

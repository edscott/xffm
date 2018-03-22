#ifndef DND_C_HPP
#define DND_C_HPP
#include "xffm+.h"
class  dnd_c{
    public:
        virtual gboolean receive_dnd(const gchar *view_target, const gchar *target, GtkSelectionData *data, GdkDragAction action)=0;
        virtual gboolean set_dnd_data(GtkSelectionData * , GList *, GtkTreeModel *, const gchar *, gint)=0;
        /*
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
        */
};

#endif

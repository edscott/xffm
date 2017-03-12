#ifndef XFDIR_C_HPP
#define XFDIR_C_HPP

#include "xffm+.h"

#include "utility_c.hpp"
#include "menu_c.hpp"
#define SET_DIR(x) x|=0x01
#define IS_DIR (x&0x01)


enum
{
  DISPLAY_PIXBUF,
  NORMAL_PIXBUF,
  HIGHLIGHT_PIXBUF,
  TOOLTIP_PIXBUF,
  DISPLAY_NAME,
  ACTUAL_NAME,
  TOOLTIP_TEXT,
  ICON_NAME,
  BASIC_COLS
};

enum
{
  COL_DISPLAY_PIXBUF,
  COL_NORMAL_PIXBUF,
  COL_HIGHLIGHT_PIXBUF,
  COL_TOOLTIP_PIXBUF,
  COL_DISPLAY_NAME,
  COL_ACTUAL_NAME,
  COL_TOOLTIP_TEXT,
  COL_ICON_NAME,
  COL_TYPE,
  COL_MIMETYPE, 
  COL_MIMEFILE, 
  COL_STAT,
  COL_PREVIEW_PATH,
  COL_PREVIEW_TIME,
  COL_PREVIEW_PIXBUF,
  NUM_COLS
};

class xfdir_c: virtual utility_c, public menu_c {
    public:
        xfdir_c(data_c *, const gchar *);
        ~xfdir_c(void);

	// Pure
        virtual void reload(const gchar *)=0;
	virtual const gchar *get_xfdir_iconname(void)=0;
        virtual void item_activated (GtkIconView *, GtkTreePath *, void *)=0;
	// Overloadable with default
        virtual gchar *make_tooltip_text (GtkTreePath *);
        virtual gchar *get_verbatim_name (GtkTreePath *);
        virtual void highlight_drop(GtkTreePath *);
	virtual gboolean set_dnd_data(GtkSelectionData *, GList *);
	virtual gboolean receive_dnd(const gchar *, GtkSelectionData *, GdkDragAction);
	// Common
	GdkPixbuf *get_normal_pixbuf(GtkTreePath *);
        GdkPixbuf *get_tooltip_pixbuf(GtkTreePath *);
        void set_tooltip_pixbuf(GtkTreePath *, GdkPixbuf *);
        gchar *get_tooltip_text(GtkTreePath *);
        void set_tooltip_text(GtkTreePath *,const  gchar *);
	gint get_dir_count(void);
        GtkTreeModel *get_tree_model(void);
        gint get_icon_highlight_size(const gchar *);
        const gchar *get_label();
	gchar *get_window_name (void);
        const gchar *get_path(void);
        gint get_icon_column(void);
        gint get_text_column(void);
        void clear_highlights(void);
        void highlight(GtkTreePath *);
        void set_show_hidden(gboolean);

        void new_items_hash(void);
        GHashTable *get_items_hash(void);
    protected:
	// Pure
        virtual GtkTreeModel *mk_tree_model(void) = 0;

        GtkTreeModel *treemodel;
        gchar *path;
	gint dir_count;   
        gboolean shows_hidden;
        GHashTable *items_hash;
    private:
        data_c *data_p;

};

#endif

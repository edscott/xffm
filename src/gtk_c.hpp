#ifndef GTK_C_HPP
#define GTK_C_HPP
#include "xffm+.h"
#include "tooltip_c.hpp"
#include "pixbuf_c.hpp"
#include "mime_c.hpp"
#define SIZE_BUTTON	20
#define SIZE_DIALOG	36
#define SIZE_ICON	48
#define SIZE_PREVIEW	96
#define SIZE_TIP	128

class gtk_c:  public tooltip_c{
    public:
        static void init(void);
        static void setup_image_button (GtkWidget *, const gchar *, const gchar *);    
        static GtkWidget *new_add_page_tab(GtkWidget *, GtkWidget **);
	static void set_bin_contents(GtkWidget *, const char *, const char *, gint);
	static void set_bin_markup(GtkWidget *, const char *);
        static gint get_icon_size(const gchar *);
        static GtkWidget *hbox_new(gboolean, gint);
        static GtkWidget *vbox_new(gboolean, gint);
        static GtkWidget *dialog_button (const char *, const char *);
        static GtkWidget *toggle_button (const char *, const char *);

        static GtkWidget *menu_item_new(const gchar *, const gchar *);
        static GtkMenu *mk_menu(const gchar **);
        static GtkMenu *mk_menu(const gchar **,  void (*menu_callback)(GtkWidget *, gpointer));
    //private:
	static void set_bin_image(GtkWidget *, const gchar *, gint);
        static GHashTable *iconname_hash;
        static void populate_iconname_hash(void);
	
};

#endif

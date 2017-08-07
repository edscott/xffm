#ifndef GTK_C_HPP
#define GTK_C_HPP
#include "xffm+.h"
#include "utility_c.hpp"
#include "tooltip_c.hpp"
#include "pixbuf_c.hpp"
#include "mime_c.hpp"
#define SIZE_BUTTON	20
#define SIZE_DIALOG	36
#define SIZE_ICON	48
#define SIZE_PREVIEW	96
#define SIZE_TIP	128

class gtk_c: virtual utility_c, public tooltip_c, public pixbuf_c{
    public:
        gtk_c(void);
        ~gtk_c(void);
        void setup_image_button (GtkWidget *, const gchar *, const gchar *);    
        GtkWidget *new_add_page_tab(GtkWidget *, GtkWidget **);
	void set_bin_contents(GtkWidget *, const char *, const char *, gint);
	void set_bin_markup(GtkWidget *, const char *);
        gint get_icon_size(const gchar *);
        GtkWidget *hbox_new(gboolean, gint);
        GtkWidget *vbox_new(gboolean, gint);
        GtkWidget *dialog_button (const char *, const char *);
        GtkWidget *toggle_button (const char *, const char *);

    protected:
        GtkWidget *menu_item_new(const gchar *, const gchar *);
        GtkMenu *mk_menu(const gchar **);
        GtkMenu *mk_menu(const gchar **,  void (*menu_callback)(GtkWidget *, gpointer));
    private:
	void set_bin_image(GtkWidget *, const gchar *, gint);
        static GHashTable *iconname_hash;
        static void populate_iconname_hash(void);
	
};

#endif

#ifndef XFGTK_HH
#define XFGTK_HH
#include "xfpixbuf.hh"

static GHashTable *iconname_hash=NULL;

namespace xf
{

template <class Type>
class Gtk{
    typedef Pixbuf<double> pixbuf_c;
public:
    static gint 
    get_icon_size(const gchar *name){
	if (strcmp(name, "..")==0) return GTK_ICON_SIZE_DND;
	return GTK_ICON_SIZE_DIALOG;
    }

    static void 
    initHash(void){
	iconname_hash = g_hash_table_new(g_str_hash, g_str_equal);
	populate_iconname_hash();
	//FIXME: init_tooltip_c();
	//fprintf(stderr, "gtk_c init OK\n");
    }
// FIXME: iconname_hash should be in a class template for fm
    static void
    populate_iconname_hash(void){
	g_hash_table_insert(iconname_hash, _("Open terminal"), (void *)"terminal");
	g_hash_table_insert(iconname_hash, _("Execute Shell Command"), (void *)"execute");
	g_hash_table_insert(iconname_hash, _("Paste"), (void *)"edit-paste");
	g_hash_table_insert(iconname_hash, _("Add bookmark"), (void *)"list-add");
	g_hash_table_insert(iconname_hash, _("Remove bookmark"), (void *)"list-remove");
	g_hash_table_insert(iconname_hash, _("Search"), (void *)"edit-find");
	g_hash_table_insert(iconname_hash, _("Close"), (void *)"cancel");
	g_hash_table_insert(iconname_hash, _("Exit"), (void *)"window-close");
	g_hash_table_insert(iconname_hash, _("Open with"), (void *)"execute");
	g_hash_table_insert(iconname_hash, _("Cut"), (void *)"cut");
	g_hash_table_insert(iconname_hash, _("Copy"), (void *)"copy");
	g_hash_table_insert(iconname_hash, _("Delete"), (void *)"delete");
	g_hash_table_insert(iconname_hash, _("Shred"), (void *)"dialog-warning");
	g_hash_table_insert(iconname_hash, _("bcrypt"), (void *)"emblem-keyhole");
	g_hash_table_insert(iconname_hash, _("Open in New Tab"), (void *)"open");
    }

    static void
    setup_image_button (GtkWidget *button, const gchar *icon_name, const gchar *icon_tip){
	gtk_widget_set_can_focus (button, FALSE);
	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	GtkWidget *image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
	GdkPixbuf *pixbuf = 
		pixbuf_c::get_pixbuf(icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
	if (image) {
	    gtk_container_add (GTK_CONTAINER (button), image);
	    // On page remove, reference count will be decreased
	    // Last reference belongs to the pixbuf hash
	    g_object_ref(G_OBJECT(pixbuf));
	    gtk_widget_show (image);
	}
	// Elaborate tooltip
	fprintf(stderr, "gtk_c.cpp:: custom_tooltip not working right in Wayland\n");
	//custom_tooltip(button, pixbuf, icon_tip);
	// Simple tooltip:
	 gtk_widget_set_tooltip_text (button, icon_tip);
	
    }  

    static GtkWidget *
    new_add_page_tab(GtkWidget *notebook, GtkWidget **new_button_p){
	GtkWidget *page_child_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *page_label_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *page_label_icon_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *page_label_button = gtk_button_new ();
	gtk_box_pack_start (GTK_BOX (page_label_box), page_label_icon_box, TRUE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (page_label_box), page_label_button, TRUE, TRUE, 0);
	gtk_widget_show_all (page_label_box);
	gtk_widget_show (page_child_box);
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), page_child_box, page_label_box);
	gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK(notebook), page_child_box, FALSE);
	setup_image_button(page_label_button, "list-add", _("Open a new tab (Ctrl+T)"));
	if (new_button_p) *new_button_p = page_label_button;
	return page_child_box;
    }

    static void
    set_bin_image(GtkWidget *bin, const gchar *icon_id, gint size){
	if (!bin || !GTK_IS_WIDGET(bin)) {
	    g_warning("rfm_set_bin_image(): incorrect function call\n");
	    return;
	}
	GtkWidget *box = gtk_bin_get_child(GTK_BIN(bin));
	GtkWidget *icon = (GtkWidget *)g_object_get_data(G_OBJECT(bin),"icon");

	if (icon){
	    gtk_container_remove(GTK_CONTAINER(box), icon);
	}
	if(icon_id) {
	    GdkPixbuf *pb = pixbuf_c::get_pixbuf (icon_id, size);
	    GtkWidget *image = gtk_image_new_from_pixbuf (pb);
	    gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE,0);
	    g_object_set_data(G_OBJECT(bin), "icon", image);
	    gtk_widget_show(image);
	    g_object_unref(pb);
	} else {
	    g_object_set_data(G_OBJECT(bin), "icon", NULL);
	}
    }

    static void 
    set_bin_markup(GtkWidget *bin, const char *text){
	if (!bin || !GTK_IS_WIDGET(bin)) {
	    g_warning("rfm_set_bin_markup(): incorrect function call\n");
	    return;
	}
	GtkLabel *label = (GtkLabel *)g_object_get_data(G_OBJECT(bin), "label");
	gtk_label_set_markup(label, (text)?text:"");
    }

    static void
    set_bin_contents(GtkWidget *bin, const char *icon_id, const char *text, gint size){
	GtkWidget *child = gtk_bin_get_child(GTK_BIN(bin));
	if (child) gtk_container_remove(GTK_CONTAINER(bin), child);
	child = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_size_request (child, -1, 12);
	gtk_container_add(GTK_CONTAINER(bin), child);
				
	GtkWidget *label = gtk_label_new("");
	g_object_set_data(G_OBJECT(bin), "label", label);
	gtk_box_pack_end(GTK_BOX(child), label, TRUE, FALSE,0);
	
	set_bin_markup(bin, text);
	set_bin_image(bin, icon_id, size);
	gtk_widget_show_all (bin);
    }


    static GtkWidget * 
    menu_item_new(const gchar *icon_id, const gchar *text)
    {
	GdkPixbuf *pb = (icon_id)? pixbuf_c::get_pixbuf (icon_id, GTK_ICON_SIZE_SMALL_TOOLBAR): NULL;    
	GtkWidget *w = gtk_menu_item_new_with_label ("");
	GtkWidget *replacement = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
	GtkWidget *label = gtk_bin_get_child(GTK_BIN(w));
	if (label && GTK_IS_WIDGET(label)) {
	    g_object_ref(label);
	    gtk_container_remove(GTK_CONTAINER(w), label);
	}

	if (pb){
	    GtkWidget *image = gtk_image_new_from_pixbuf (pb);
	    gtk_widget_show (image);
	    gtk_box_pack_start(GTK_BOX(replacement), image, FALSE,FALSE,0);
	    g_object_set_data(G_OBJECT(w), "image", image);
	}
	if (label && GTK_IS_WIDGET(label)) {
	    gtk_label_set_markup(GTK_LABEL(label), text);
	    gtk_box_pack_start(GTK_BOX(replacement), label, FALSE,FALSE,3);
	    g_object_set_data(G_OBJECT(w), "label", label);
	    gtk_widget_show(label);
	    g_object_unref(label);
	}
	gtk_widget_show(replacement);
	gtk_container_add(GTK_CONTAINER(w), replacement);
	return w;
    }


    static GtkBox *vboxNew(gboolean homogeneous, gint spacing){
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing);
	gtk_box_set_homogeneous (GTK_BOX(box), homogeneous);
	return GTK_BOX(box);
    }

    static GtkBox *hboxNew(gboolean homogeneous, gint spacing){
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
	gtk_box_set_homogeneous (GTK_BOX(box), homogeneous);
	return GTK_BOX(box);
    }


    static GtkButton *
    dialog_button (const char *icon_id, const char *text) {
	GtkWidget *button = gtk_button_new ();
	set_bin_contents(button, icon_id, text, SIZE_BUTTON);
	return GTK_BUTTON(button);

    }

    static void
    quick_help (GtkWindow *parent, const gchar *message)
    {
     GtkWidget *dialog, *label, *content_area;
     GtkDialogFlags flags;

	std::cerr<<"fixme: signals::quick_help\n";

     // Create the widgets
     flags = GTK_DIALOG_DESTROY_WITH_PARENT;
     dialog = gtk_dialog_new_with_buttons (_("Help"),
					   parent,
					   flags,
					   _("_OK"),
					   GTK_RESPONSE_NONE,
					   NULL);
     content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
     label = gtk_label_new (message);

     // Ensure that the dialog box is destroyed when the user responds

     g_signal_connect_swapped (dialog,
			       "response",
			       G_CALLBACK (gtk_widget_destroy),
			       dialog);

     // Add the label, and show everything we have added

     gtk_container_add (GTK_CONTAINER (content_area), label);
     gtk_widget_show_all (dialog);
    }

    static GtkToggleButton *
    toggle_button (const char *icon_id, const char *text) {
	GtkWidget *button = gtk_toggle_button_new ();
	set_bin_contents(button, icon_id, text, SIZE_BUTTON);
	return GTK_TOGGLE_BUTTON(button);
    }

    static GtkMenu *
    mk_menu(const gchar **data, void (*menu_callback)(GtkWidget *, gpointer)){
	
	GtkMenu *menu = GTK_MENU(gtk_menu_new());
	const gchar **p = data;
	gint i;
	if (!iconname_hash) initHash();
	for (i=0;p && *p; p++,i++){
	    const gchar *icon_name = (const gchar *)g_hash_table_lookup(iconname_hash, _(*p));
	    GtkWidget *v = menu_item_new(icon_name, _(*p));
	    g_object_set_data(G_OBJECT(v), "menu", (void *)menu);
	    gtk_container_add (GTK_CONTAINER (menu), v);
	    g_signal_connect ((gpointer) v, "activate", G_CALLBACK (*menu_callback), (void *)_(*p));
	    gtk_widget_show (v);
	}
	gtk_widget_show (GTK_WIDGET(menu));
	return menu;
    }

};
}
#endif

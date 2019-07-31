#ifndef XFGTK_HH
#define XFGTK_HH
#include "pixbuf.hh"


static GHashTable *iconname_hash=NULL;

namespace xf
{

template <class Type>
class Gtk{
    typedef Pixbuf<double> pixbuf_c;
public:
    
    static gboolean
    isImage(const gchar *mimetype){
	static GSList *pix_mimetypes = NULL;
	static gsize initialized = 0;
	if (g_once_init_enter(&initialized)){
	    // This gdk call is thread safe. 
	    GSList *pix_formats = gdk_pixbuf_get_formats ();// check OK
	    GSList *list = pix_formats;
	    for(; list && list->data; list = list->next) {
		gchar **pix_mimetypes_p;
		GdkPixbufFormat *fmt = (GdkPixbufFormat *)list->data;
		// This gdk call is thread safe.
		pix_mimetypes_p = gdk_pixbuf_format_get_mime_types (fmt);// check OK
		pix_mimetypes = g_slist_prepend(pix_mimetypes, pix_mimetypes_p);
	    }
	    g_slist_free(pix_formats);
	    g_once_init_leave(&initialized, 1);
	}
	/* check for image support types */
	GSList *list = pix_mimetypes;
	for(; list && list->data; list = list->next) {
	    gchar **pix_mimetypes_p = (gchar **)list->data;
	    for(; pix_mimetypes_p && *pix_mimetypes_p; pix_mimetypes_p++) {
		TRACE("allowable pix_format=%s --> %s\n", *pix_mimetypes_p, mimetype);
		if(g_ascii_strcasecmp (*pix_mimetypes_p, mimetype) == 0) {
		    TRACE("gotcha: allowable pix_format=%s --> %s\n", *pix_mimetypes_p, mimetype);
		    return TRUE;
		}
	    }
	}
	return FALSE;
    }

    static gint 
    get_icon_size(const gchar *name){
	if (strcmp(name, "..")==0) return GTK_ICON_SIZE_DND;
	return GTK_ICON_SIZE_DIALOG;
    }

    static void 
    initHash(void){
	iconname_hash = g_hash_table_new_full(g_str_hash, g_str_equal,NULL, NULL);
	populate_iconname_hash();
	//FIXME: init_tooltip_c();
	TRACE("gtk_c init OK\n");
    }
// FIXME: iconname_hash should be in a class template for fm
//        not currently used functionality. Should update iconname ids.
    static void
    populate_iconname_hash(void){
	g_hash_table_insert(iconname_hash, 
			(void *)_("Open terminal"), (void *)"terminal");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Execute Shell Command"), (void *)"execute");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Paste"), (void *)"edit-paste");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Add bookmark"), (void *)"list-add");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Remove bookmark"), (void *)"list-remove");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Search"), (void *)"edit-find");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Close"), (void *)"cancel");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Exit"), (void *)"window-close");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Open with"), (void *)"execute");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Cut"), (void *)"cut");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Copy"), (void *)"copy");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Delete"), (void *)"delete");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Shred"), (void *)"dialog-warning");
	//g_hash_table_insert(iconname_hash, "bcrypt", (void *)"emblem-keyhole");
	g_hash_table_insert(iconname_hash, 
			(void *)_("Open in New Tab"), (void *)"open");
    }

    static void
    setup_image_button (GtkButton *button, const gchar *icon_name, const gchar *icon_tip){
	gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
	gtk_button_set_relief (button, GTK_RELIEF_NONE);

	//auto image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
//	auto pixbuf = pixbuf_c::get_pixbuf(icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
	auto pixbuf = pixbuf_c::get_pixbuf(icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
        auto image = gtk_image_new_from_pixbuf(pixbuf);
	if (image) {
	    gtk_container_add (GTK_CONTAINER (button), image);
	    // On page remove, reference count will be decreased
	    // Last reference belongs to the pixbuf hash
	    g_object_ref(G_OBJECT(pixbuf));
	    gtk_widget_show (image);
	}
	// Elaborate tooltip
	static gboolean waylandWarn = TRUE;
	if (waylandWarn) {
	    DBG("gtk_c.cpp:: custom_tooltip not working right in Wayland\n");
	    waylandWarn = FALSE;
	}
	//custom_tooltip(button, pixbuf, icon_tip);
	// Simple tooltip:
	 gtk_widget_set_tooltip_text (GTK_WIDGET(button), icon_tip);
	
    }  

    static GtkBox *
    new_add_page_tab(GtkWidget *notebook, GtkButton **new_button_p){
	auto page_child_box = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
	auto page_label_box = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	auto page_label_icon_box = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	auto page_label_button = GTK_BUTTON(gtk_button_new ());
	gtk_box_pack_start (page_label_box, GTK_WIDGET(page_label_icon_box), TRUE, TRUE, 0);
	gtk_box_pack_end (page_label_box, GTK_WIDGET(page_label_button), TRUE, TRUE, 0);
	gtk_widget_show_all (GTK_WIDGET(page_label_box));
	gtk_widget_show (GTK_WIDGET(page_child_box));
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), 
                GTK_WIDGET(page_child_box), GTK_WIDGET(page_label_box));
	gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK(notebook), 
                GTK_WIDGET(page_child_box), FALSE);
	setup_image_button(page_label_button, "list-add", _("New Tab"));
	if (new_button_p) *new_button_p = page_label_button;
	return page_child_box;
    }

    static void
    set_container_image(GtkContainer *container, const gchar *icon_id, gint size){
	if (!container || !GTK_IS_WIDGET(container)) {
	    g_warning("incorrect function call\n");
	    return;
	}
        auto list = gtk_container_get_children (container);
        for (GList *p = list; p && p->data; p= p->next){
            gtk_container_remove(container, GTK_WIDGET(p->data));
	}
        g_list_free(list);
	if(icon_id) {
	    auto pb = pixbuf_c::get_pixbuf (icon_id, size);
	    auto image = gtk_image_new_from_pixbuf (pb);
            gtk_container_add(container, image);
	    gtk_widget_show(image);
	    g_object_ref(pb);
	    // no no no no... g_object_unref(pb);
	} 
    }

    static void
    set_bin_image(GtkBin *bin, const gchar *icon_id, gint size){
	if (!bin || !GTK_IS_WIDGET(bin)) {
	    g_warning("rfm_set_bin_image(): incorrect function call\n");
	    return;
	}
	auto box = GTK_BOX(gtk_bin_get_child(bin));
	auto icon = (GtkWidget *)g_object_get_data(G_OBJECT(bin),"icon");

	if (icon){
	    gtk_container_remove(GTK_CONTAINER(box), icon);
	}
	if(icon_id) {
	    auto pb = pixbuf_c::get_pixbuf (icon_id, size);
	    auto image = gtk_image_new_from_pixbuf (pb);
	    gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE,0);
	    g_object_set_data(G_OBJECT(bin), "icon", image);
	    gtk_widget_show(image);
	    // Pixbufs are hashed, so references must be kept.
	    g_object_ref(pb);
	    //g_object_unref(pb);
	} else {
	    g_object_set_data(G_OBJECT(bin), "icon", NULL);
	}
    }

    static void 
    set_bin_markup(GtkBin *bin, const char *text){
	if (!bin || !GTK_IS_WIDGET(bin)) {
	    g_warning("rfm_set_bin_markup(): incorrect function call\n");
	    return;
	}
	auto label = GTK_LABEL(g_object_get_data(G_OBJECT(bin), "label"));
	gtk_label_set_markup(label, (text)?text:"");
    }

    static void
    set_bin_label(GtkBin *bin, const char *text){
	auto child = gtk_bin_get_child(bin);
	if (child) gtk_container_remove(GTK_CONTAINER(bin), child);
	auto label = GTK_LABEL(gtk_label_new(""));
	//g_object_set_data(G_OBJECT(bin), "label", label);
	g_object_set_data(G_OBJECT(bin), "name", text?g_strdup(text):g_strdup("RFM_ROOT"));
	gtk_container_add(GTK_CONTAINER(bin), GTK_WIDGET(label));
	set_bin_markup(bin, text);
	gtk_widget_show_all (GTK_WIDGET(bin));
    }

    static void
    set_bin_contents(GtkBin *bin, const char *icon_id, const char *text, gint size){
	auto child = gtk_bin_get_child(bin);
	if (child) gtk_container_remove(GTK_CONTAINER(bin), child);
	child = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    
        gtk_widget_set_size_request (child, -1, size);
	gtk_container_add(GTK_CONTAINER(bin), child);
				
	auto label = GTK_LABEL(gtk_label_new(""));
	g_object_set_data(G_OBJECT(bin), "label", label);
	gtk_box_pack_end(GTK_BOX(child), GTK_WIDGET(label), TRUE, FALSE,0);
	
	set_bin_markup(bin, text);
	if (icon_id) set_bin_image(bin, icon_id, size);
	gtk_widget_show_all (GTK_WIDGET(bin));
  }

    static void 
    menu_item_content(GtkMenuItem *menuItem, const gchar *icon_id, const gchar *text, gint size){
        auto image = GTK_IMAGE(g_object_get_data(G_OBJECT(menuItem), "image"));
        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(menuItem), "label"));
        auto box = (GtkBox *)g_object_get_data(G_OBJECT(menuItem), "box");
        gtk_container_remove(GTK_CONTAINER(menuItem), GTK_WIDGET(box));
	box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0));
        g_object_set_data(G_OBJECT(menuItem), "box", box);

	GdkPixbuf *pb = (icon_id)? pixbuf_c::get_pixbuf (icon_id, size): NULL;    
	if (pb){
	    image = GTK_IMAGE(gtk_image_new_from_pixbuf (pb));
	    gtk_widget_show (GTK_WIDGET(image));
	    gtk_box_pack_start(box, GTK_WIDGET(image), FALSE,FALSE,0);
	    g_object_set_data(G_OBJECT(menuItem), "image", image);
	}
        label = GTK_LABEL(gtk_label_new(""));
        gtk_label_set_markup(label, text);
        gtk_box_pack_start(box, GTK_WIDGET(label), FALSE,FALSE,0);
	gtk_widget_show (GTK_WIDGET(label));
        g_object_set_data(G_OBJECT(menuItem), "label", label);
	gtk_widget_show(GTK_WIDGET(box));
	gtk_container_add(GTK_CONTAINER(menuItem), GTK_WIDGET(box));
        return;
    }
    static GtkWidget * 
    menu_item_new(const gchar *icon_id, const gchar *text, gint size)
    {
	GdkPixbuf *pb = (icon_id)? pixbuf_c::get_pixbuf (icon_id, size): NULL;    
	auto w = gtk_menu_item_new_with_label ("");
	auto box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0));
        g_object_set_data(G_OBJECT(w), "box", box);
	auto label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(w)));
	if (label && GTK_IS_WIDGET(label)) {
	    g_object_ref(label);
	    gtk_container_remove(GTK_CONTAINER(w), GTK_WIDGET(label));
	}

	if (pb){
	    auto image = gtk_image_new_from_pixbuf (pb);
	    gtk_widget_show (image);
	    gtk_box_pack_start(box, image, FALSE,FALSE,0);
	    g_object_set_data(G_OBJECT(w), "image", image);
	}
	if (label && GTK_IS_WIDGET(label)) {
            gtk_label_set_markup(label, text);
            /*gchar *markup = g_strdup_printf("<span size=\"larger\" color=\"red\"><b><i>%s</i></b></span>", text);
            gtk_label_set_markup(label, markup);
            g_free(markup);*/
	    gtk_box_pack_start(box, GTK_WIDGET(label), FALSE,FALSE,3);
	    g_object_set_data(G_OBJECT(w), "label", label);
	    gtk_widget_show(GTK_WIDGET(label));
	    g_object_unref(label);
	}
	gtk_widget_show(GTK_WIDGET(box));
	gtk_container_add(GTK_CONTAINER(w), GTK_WIDGET(box));
	return w;
    }

    static GtkWidget * 
    menu_item_new(const gchar *icon_id, const gchar *text)
    {
        return menu_item_new(icon_id, text, GTK_ICON_SIZE_SMALL_TOOLBAR);
    }


    static GtkBox *vboxNew(gboolean homogeneous, gint spacing){
	auto box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing));
	gtk_box_set_homogeneous (box, homogeneous);
	return box;
    }

    static GtkBox *hboxNew(gboolean homogeneous, gint spacing){
	auto box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing));
	gtk_box_set_homogeneous (box, homogeneous);
	return box;
    }

    static GtkButton *newButton(const gchar *icon, const gchar *tooltipText){
	auto button =  GTK_BUTTON(gtk_button_new());
        setup_image_button(button, icon, tooltipText);
	gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
	gtk_button_set_relief (button, GTK_RELIEF_NONE);
        gtk_widget_set_tooltip_markup (GTK_WIDGET(button),tooltipText);
	return button;
    }

    static GtkMenuButton *newMenuButton(const gchar *icon, const gchar *tooltipText){
	auto button =  GTK_MENU_BUTTON(gtk_menu_button_new());
        setup_image_button(GTK_BUTTON(button), icon, tooltipText);
	gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
	gtk_button_set_relief (GTK_BUTTON(button), GTK_RELIEF_NONE);
        gtk_widget_set_tooltip_markup (GTK_WIDGET(button),tooltipText);
	return button;
    }
    
/*    static GtkButton *newButtonL(const gchar *icon, const gchar *tooltipText){
	auto button =  gtk_c::dialog_button(icon, NULL);
	gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
	gtk_button_set_relief (button, GTK_RELIEF_NONE);
        gtk_widget_set_tooltip_markup (GTK_WIDGET(button),tooltipText);
	return button;
    }*/

    static GtkButton *
    dialog_button (const char *icon_id, const char *text) {
	auto button = GTK_BUTTON(gtk_button_new ());
	set_bin_contents(GTK_BIN(button), icon_id, text, TINY_BUTTON);
	gtk_button_set_relief (button, GTK_RELIEF_NONE);        
	return button;

    }

    static void
    quick_help (GtkWindow *parent, const gchar *message){
        auto dialog = _quickHelp(parent, message, NULL, NULL );
        gtk_widget_show_all (GTK_WIDGET(dialog));
    }

    static GtkWidget *
    quickHelp (GtkWindow *parent, const gchar *message, const gchar *icon){
        auto dialog = _quickHelp(parent, message, icon, _("Help"));
        gtk_widget_show_all (GTK_WIDGET(dialog));
        return dialog;
    }

    static GtkWidget *
    quickHelp (GtkWindow *parent, const gchar *message, const gchar *icon, const gchar *title)
    {
        auto dialog = _quickHelp(parent, message, icon, _("Help"));
        gtk_widget_show_all (GTK_WIDGET(dialog));
        return dialog;
    }

    static GtkWidget *
    _quickHelp (GtkWindow *parent, const gchar *message, const gchar *icon, const gchar *title)
    {

     static GtkWidget *dialog = NULL;
     static gchar *last_message = NULL;
     // Already mapped? Destroy.
     if (dialog && GTK_IS_WIDGET(dialog)){
         gtk_widget_hide(dialog);
         gtk_widget_destroy(dialog);

         // Last mapped is the same? Do not remap.
         if (last_message && strcmp(last_message, message) == 0){
             g_free(last_message);
             last_message = NULL;
             dialog = NULL;
             return NULL;
         }
     }
     // Map dialog.parent, 
     g_free(last_message);
     last_message = g_strdup(message);
     dialog = quickDialog(parent, message, icon, title);

      return dialog;
    }

    static void
    closeQuickDialog(GtkWidget *widget, GdkEventKey * event, void *data){
	gtk_widget_hide(widget);
	DBG("closeQuickDialog\n");
	gtk_widget_destroy(widget);
    }

    static GtkWidget *
    quickDialog (GtkWindow *parent, const gchar *message, const gchar *icon, const gchar *title)
    {
     GtkWidget *dialog = NULL;
     GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

     // Create the widgets
     dialog = gtk_dialog_new_with_buttons (title,
					   parent,
					   flags,
					   _("Cancel"),
					   GTK_RESPONSE_NONE,
					   NULL);
     auto content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
     auto label = GTK_LABEL(gtk_label_new (""));
     gtk_label_set_markup(label, message);
     

     // Ensure that the dialog box is destroyed when the user responds

     if (parent){
         g_signal_connect_swapped(dialog, "response", G_CALLBACK (gtk_widget_show),
			       parent);
     }
     /*g_signal_connect_swapped (dialog,
			       "response",
			       G_CALLBACK (gtk_widget_destroy),
			       dialog);*/
     g_signal_connect_swapped (dialog,
			       "response",
			       G_CALLBACK (closeQuickDialog),
			       dialog);


     // Add the label, and show everything we have added
     auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
     g_object_set_data(G_OBJECT(dialog), "vbox", vbox);
     
     gtk_container_add (GTK_CONTAINER (content_area), GTK_WIDGET(vbox));
     if (icon){
	auto pixbuf = Pixbuf<Type>::get_pixbuf(icon, -48);
        if (pixbuf) {
            auto image = gtk_image_new_from_pixbuf(pixbuf);
	    if (image) {
	        gtk_box_pack_start(vbox, image, FALSE, FALSE,0);
	        gtk_widget_show (image);
            }
	}
     }
     auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
     gtk_box_pack_start(vbox, GTK_WIDGET(hbox), FALSE, FALSE,0);
     gtk_box_pack_start(hbox, GTK_WIDGET(label), FALSE, FALSE,0);

     return dialog;
    }

    static GtkToggleButton *
    toggle_button (const char *icon_id, const char *text) {
	auto button = GTK_TOGGLE_BUTTON(gtk_toggle_button_new ());
	set_bin_contents(GTK_BIN(button), icon_id, text, SIZE_BUTTON);
	return button;
    }

    static GtkMenu *
    mk_menu(const gchar **data, void (*menu_callback)(GtkWidget *, gpointer)){
	auto menu = GTK_MENU(gtk_menu_new());
	auto p = data;
	if (!iconname_hash) initHash();
	for (gint i=0;p && *p; p++,i++){
	    auto icon_name = (const gchar *)g_hash_table_lookup(iconname_hash, _(*p));
	    auto v = menu_item_new(icon_name, _(*p));
	    g_object_set_data(G_OBJECT(v), "menu", (void *)menu);
	    gtk_container_add (GTK_CONTAINER (menu), GTK_WIDGET(v));
	    g_signal_connect ((gpointer) v, "activate", G_CALLBACK (*menu_callback), (void *)_(*p));
	    gtk_widget_show (GTK_WIDGET(v));
	}
	gtk_widget_show (GTK_WIDGET(menu));
	return menu;
    }

};
}
#endif

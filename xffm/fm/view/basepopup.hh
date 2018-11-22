#ifndef XF_BASEPOPUP__HH
# define XF_BASEPOPUP__HH

namespace xf {
template <class Type>
class BasePopUp {
    using gtk_c = Gtk<double>;
    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
    using pixbuf_icons_c = Icons<double>;
    
public: 
    static void changeTitle(GtkMenu *menu, const gchar *iconName, 
	    const gchar *name, const gchar *path, 
            const gchar *mimetype, const gchar *fileInfo)
    {
	// change title
	auto title = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(menu), "title"));
        if (!title) {
            ERROR("BasePopUp::changeTitle(): menu has no data object \"title\"\n");
            return;
        }
	TRACE("changeTitle, title = %p\n",(void *)title);
	gchar *statLine=NULL;
        if (g_file_test(path, G_FILE_TEST_EXISTS)){
            statLine = util_c::statInfo(path);
        }
	gchar *markup = g_strdup_printf("<span size=\"larger\" color=\"red\"><b><i>%s</i></b></span><span color=\"#aa0000\">%s%s</span>\n<span color=\"blue\">%s</span>\n<span color=\"green\">%s</span>", 
		name, 
		mimetype?": ":"",
		mimetype?mimetype:"",
		fileInfo?fileInfo:"", 
		statLine?statLine:"");
	TRACE("iconName=%s, markup=%s\n", iconName, markup);
	gtk_c::menu_item_content(title, iconName, markup, -48);
	g_free(statLine);
	g_free(markup);
        gtk_widget_show(GTK_WIDGET(title));
    }

    static void
    noop(GtkMenuItem *menuItem, gpointer data) { DBG("noop\n"); }

    static GtkMenu *
    createPopup(menuItem_t *item){
        auto menu = GTK_MENU(gtk_menu_new());
	// Create title element
	GtkWidget *title = gtk_c::menu_item_new(NULL, ""); 
	gtk_widget_set_sensitive(title, FALSE);
	gtk_widget_show (title);
	g_object_set_data(G_OBJECT(menu), "title", title);
	gtk_container_add (GTK_CONTAINER (menu), title);
	auto p = item;
	for (gint i=0;p && p->label; p++,i++){
            GtkWidget *v;
            if (p->toggleID){
                v = gtk_check_menu_item_new_with_label(_(p->label));
                if (Settings<Type>::getSettingInteger("LocalView", p->toggleID) > 0){
                   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(v), TRUE);
                } 
            } else {
                v = gtk_c::menu_item_new(NULL, _(p->label));
            }
	    g_object_set_data(G_OBJECT(menu), p->label, v);
	    gtk_widget_set_sensitive(v, FALSE);
	    gtk_container_add (GTK_CONTAINER (menu), v);
	    g_signal_connect ((gpointer) v, "activate", 
                    (p->callback)?MENUITEM_CALLBACK (p->callback):MENUITEM_CALLBACK (noop), 
                    (p->callbackData)?p->callbackData:(void *) menu);
	    gtk_widget_show (v);
	}
	gtk_widget_show (GTK_WIDGET(menu));
	return menu;

    }


};
}

#endif

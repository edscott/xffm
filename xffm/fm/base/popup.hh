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

    static void changeTitle(GtkMenu *menu, const gchar *text, const gchar *iconName)
    {
	// change title
	auto title = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(menu), "title"));
        if (!title) {
            ERROR("base/popup.hh::changeTitle(): menu has no data object \"title\"\n");
            return;
        }
	TRACE("changeTitle, title = %p\n",(void *)title);
	gchar *markup = g_strdup_printf("<span color=\"blue\" size=\"large\">%s</span>", text);
	Gtk<Type>::menu_item_content(title, iconName, markup, -48);
	g_free(markup);
        gtk_widget_set_sensitive(GTK_WIDGET(title), FALSE);
        gtk_widget_show(GTK_WIDGET(title));
    }
    static void changeTitle(GtkMenu *menu, const gchar *markup)
    {
	// change title
	auto title = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(menu), "title"));
        if (!title) {
            ERROR("base/popup.hh::changeTitle(): menu has no data object \"title\"\n");
            return;
        }
        auto iconName = (gchar *)g_object_get_data(G_OBJECT(menu), "iconName");

	TRACE("iconName=%s, markup=%s\n", iconName, markup);
	gtk_c::menu_item_content(title, iconName, markup, -48);
        gtk_widget_set_sensitive(GTK_WIDGET(title), FALSE);
        gtk_widget_show(GTK_WIDGET(title));
    }


    // deprecated:
    static void changeTitle(GtkMenu *menu)
    {
	// change title
	auto title = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(menu), "title"));
        if (!title) {
            ERROR("base/popup.hh::changeTitle(): menu has no data object \"title\"\n");
            return;
        }
	TRACE("changeTitle, title = %p\n",(void *)title);
        auto iconName = (gchar *)g_object_get_data(G_OBJECT(menu), "iconName");
        auto path = (gchar *)g_object_get_data(G_OBJECT(menu), "path");
        auto mimetype = (gchar *)g_object_get_data(G_OBJECT(menu), "mimetype");
        auto fileInfo = (gchar *)g_object_get_data(G_OBJECT(menu), "fileInfo");
        auto display_name = (gchar *)g_object_get_data(G_OBJECT(menu), "displayName");
        auto statLine = (gchar *)g_object_get_data(G_OBJECT(menu), "statLine");
	if (fileInfo && strchr(fileInfo, '&')) *(strchr(fileInfo, '&')) = '+';

	gchar *markup = g_strdup_printf("<span color=\"red\"><b><i>%s</i></b></span><span color=\"#aa0000\">%s%s</span>\n<span color=\"blue\">%s</span>\n<span color=\"green\">%s</span>", 
		display_name, 
		mimetype?": ":"",
		mimetype?mimetype:"",
		fileInfo?fileInfo:"", 
		statLine?statLine:"");
	TRACE("iconName=%s, markup=%s\n", iconName, markup);
	gtk_c::menu_item_content(title, iconName, markup, -48);
	g_free(markup);
        gtk_widget_set_sensitive(GTK_WIDGET(title), FALSE);
        gtk_widget_show(GTK_WIDGET(title));
    }

    static void
    noop(GtkMenuItem *menuItem, gpointer data) { DBG("noop\n"); }

    static GtkMenu *
    createPopup(menuItem_t *item){
        auto menu = GTK_MENU(gtk_menu_new());
	// Create title element
	GtkWidget *title = gtk_c::menu_item_new(NULL, ""); 
	gtk_widget_set_sensitive(title, TRUE);
	gtk_widget_show (title);
	g_object_set_data(G_OBJECT(menu), "title", title);
	gtk_container_add (GTK_CONTAINER (menu), title);
	auto p = item;
	for (gint i=0;p && p->label; p++,i++){
            GtkWidget *v;
            if (p->toggleID){
                v = gtk_check_menu_item_new_with_label(_(p->label));
                if (Settings<Type>::getSettingInteger(p->toggleID, (const gchar *)p->callbackData) > 0){
                   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(v), TRUE);
                } 
            } else {
                v = gtk_c::menu_item_new(NULL, _(p->label));
            }
	    g_object_set_data(G_OBJECT(menu), p->label, v);
	    gtk_widget_set_sensitive(v, TRUE);
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

#ifndef XF_BASEPOPUP__HH
# define XF_BASEPOPUP__HH
namespace xf
{
template <class Type>
class Popup{
    GtkMenu *menu_;
public:
    Popup(menuItem_t *item, const gchar *key[]=NULL, const gchar *keyIcon[]=NULL, gboolean small=FALSE){
        menu_ = createMenu(item);
        decorateItems(menu_, key, keyIcon, small);
        g_object_set_data(G_OBJECT(menu_), "popup", this);
	DBG("Popup constructed\n" );
    }

    GtkMenu *menu(void){ return menu_;}

    static void
    noop(GtkMenuItem *menuItem, gpointer data) { DBG("noop\n"); }

    void changeTitle(const gchar *text)
    {
	// change title
	auto title = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(menu_), "title"));
        if (!title) {
            ERROR("base/popup.hh::changeTitle(): menu has no data object \"title\"\n");
            return;
        }
        auto iconName = (gchar *)g_object_get_data(G_OBJECT(menu_), "iconName");

	TRACE("iconName=%s, markup=%s\n", iconName, markup);
        if (!strstr(text, "</span>")){
            gchar *markup = g_strdup_printf("<span color=\"blue\" size=\"large\">%s</span>", text);
            Gtk<Type>::menu_item_content(title, iconName, markup, -48);
            g_free(markup);
        } else { // already marked up...
            Gtk<Type>::menu_item_content(title, iconName, text, -48);
        }
        gtk_widget_set_sensitive(GTK_WIDGET(title), FALSE);
        gtk_widget_show(GTK_WIDGET(title));
    }

    void changeTitle(const gchar *text, const gchar *iconName)
    {
        if (!text) return;
	// change title
	auto title = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(menu_), "title"));
        if (!title) {
            ERROR("base/popup.hh::changeTitle(): menu has no data object \"title\"\n");
            return;
        }
	TRACE("changeTitle, title = %p\n",(void *)title);
        if (!strstr(text, "</span>")){
            gchar *markup = g_strdup_printf("<span color=\"blue\" size=\"large\">%s</span>", text);
            Gtk<Type>::menu_item_content(title, iconName, markup, -48);
            g_free(markup);
        } else { // already marked up...
            Gtk<Type>::menu_item_content(title, iconName, text, -48);
        }
        gtk_widget_set_sensitive(GTK_WIDGET(title), FALSE);
        gtk_widget_show(GTK_WIDGET(title));
    }
    
    static void changeTitle(GtkMenu *menu, const gchar *text){
        auto popup = (Popup<Type> *)g_object_get_data(G_OBJECT(menu), "popup");
        if (popup) popup->changeTitle(text);
    }
    
    static void changeTitle(GtkMenu *menu, const gchar *text,const gchar *iconName){
        auto popup = (Popup<Type> *)g_object_get_data(G_OBJECT(menu), "popup");
        if (popup) popup->changeTitle(text, iconName);
    }
public:

    static const gchar *
    setMenuItemData(GtkMenu *menu, const gchar *key, const gchar *data){
	if (!menu) ERROR("base signals: setMenuItemData() menu is null\n");
	g_free(g_object_get_data(G_OBJECT(menu), key));
        g_object_set_data(G_OBJECT(menu), key, g_strdup(data));
	return data;
    }

    static const gchar *
    getMenuItemData(GtkMenu *menu, const gchar *key){
	if (!menu) ERROR("base signals: getMenuItemData() menu is null\n");
        auto data =(const gchar *)g_object_get_data(G_OBJECT(menu), key);
	if (!data) ERROR("base signals: getMenuItemData() data %s is null\n", key);
	return data;
    }
 
private:
    static GtkMenu *
    createMenu(menuItem_t *item){
	DBG("createMenu\n" );
        auto menu = GTK_MENU(gtk_menu_new());
	// Create title element
	GtkWidget *title = Gtk<Type>::menu_item_new(NULL, ""); 
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
                v = Gtk<Type>::menu_item_new(NULL, _(p->label));
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

    static void
    decorateItems(GtkMenu *menu, const gchar *key[], const gchar *keyIcon[], gboolean small){
	DBG("decorateItems, menu = %p\n", menu );
        if (!key || !keyIcon) return;
        gint i=0;
        for (auto k=key; k && key[i] && keyIcon[i]; k++, i++){
            auto mItem = (GtkMenuItem *)g_object_get_data(G_OBJECT(menu), *k);
            if (!mItem) continue;
            gchar *markup;
            if (small) {
                markup = g_strdup_printf("<span size=\"small\">%s</span>", _(*k));
	        Gtk<Type>::menu_item_content(mItem, keyIcon[i], markup, -16);
            } else {
                markup = g_strdup_printf("<span color=\"blue\">%s</span>", _(*k));
	        Gtk<Type>::menu_item_content(mItem, keyIcon[i], markup, -24);
            }
	    g_free(markup);
        }
    }

};
}

#endif

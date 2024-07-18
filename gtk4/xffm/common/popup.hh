#ifndef XF_BASEPOPUP_HH
# define XF_BASEPOPUP_HH
namespace xf
{
template <class Type>
class Popup{
    GtkMenu *menu_;
public:
    Popup(menuItem2_t *item){
        TRACE("Popup constructor\n" );
        menu_ = createMenu(item);
        g_object_set_data(G_OBJECT(menu_), "popup", this);
        TRACE("Popup constructed\n" );
    }
    Popup(menuItem_t *item, const gchar *key[]=NULL, const gchar *keyIcon[]=NULL, gboolean small=FALSE){
        TRACE("Popup constructor\n" );
        menu_ = createMenu(item);
        decorateItems(menu_, key, keyIcon, small);
        g_object_set_data(G_OBJECT(menu_), "popup", this);
        TRACE("Popup constructed\n" );
    }

    GtkMenu *menu(void){ return menu_;}

    static void 
    configureMenuItem(GtkMenu*menu, const gchar *key, gboolean test, const gchar *path){
        auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(menu), key));
        if (w) {
            if (test) gtk_widget_show(w);
            else gtk_widget_hide(w);
            gtk_widget_set_sensitive(w, path != NULL);
            Popup<Type>::setWidgetData(w, "path", path);
        }
    }

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

        //TRACE("iconName=%s, markup=%s\n", iconName, markup);
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

    void changeTitle(const gchar *text, const gchar *iconName,
            gchar *titleTooltip){
        changeTitle(text, iconName);
        auto title = GTK_WIDGET(g_object_get_data(G_OBJECT(menu_), "title"));
        gtk_widget_set_tooltip_text(title, titleTooltip);
        gtk_widget_set_sensitive(GTK_WIDGET(title), FALSE);
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
        TRACE("changeTitle, title = %p, iconName=%s\n",(void *)title, iconName);
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

    void changeTitle(const gchar *text, GdkPixbuf *pixbuf)
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
            Gtk<Type>::menu_item_content(title, pixbuf, markup);
            g_free(markup);
        } else { // already marked up...
            Gtk<Type>::menu_item_content(title, pixbuf, text);
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
    
    static void changeTitle(GtkMenu *menu, const gchar *text,GdkPixbuf *pixbuf){
        auto popup = (Popup<Type> *)g_object_get_data(G_OBJECT(menu), "popup");
        if (popup) popup->changeTitle(text, pixbuf);
    }
public:


    static const gchar *
    setWidgetData(GtkWidget *w, const gchar *key, const gchar *data){
        if (!w) ERROR("base signals: setMenuItemData() menu is null\n");
        g_free(g_object_get_data(G_OBJECT(w), key));
        TRACE("setWidgetData(%s) -> %s\n", key, data);
        g_object_set_data(G_OBJECT(w), key, g_strdup(data));
        return data;
    }

    static const gchar *
    setWidgetData(GtkMenu *w, const gchar *key, const gchar *data){
        return setWidgetData(GTK_WIDGET(w), key, data);
    }

    static const gchar *
    getWidgetData(GtkWidget *w, const gchar *key){
        if (!w) ERROR("base signals: getWidgetData() menu is null\n");
        auto data =(const gchar *)g_object_get_data(G_OBJECT(w), key);
        if (!data) ERROR("base signals: getWidgetData() data %s is null\n", key);
        TRACE("getWidgetData(%s) -> %s\n", key, data);
        return data;
    }
    static const gchar *
    getWidgetData(GtkMenu *w, const gchar *key){
        return getWidgetData(GTK_WIDGET(w), key);
    }
    static GtkMenu *
    createMenu(menuItem_t *item){
        TRACE("createMenu\n" );
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
                if (Settings<Type>::getInteger(p->toggleID, (const gchar *)p->callbackData) > 0){
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

    static GtkMenu *
    createMenu(menuItem2_t *item){
        TRACE("createMenu\n" );
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
            v = Gtk<Type>::menu_item_new(NULL, _(p->label));
            
            g_object_set_data(G_OBJECT(menu), p->label, v);
            gtk_widget_set_sensitive(v, TRUE);
            gtk_container_add (GTK_CONTAINER (menu), v);
            g_signal_connect ((gpointer) v, "activate", 
                    (p->callback)?MENUITEM_CALLBACK (p->callback):MENUITEM_CALLBACK (noop), 
                    (p->callbackData)?p->callbackData:(void *) menu);

            gchar *markup;
            auto color = p->protect?" color=\"#990000\"":" color=\"#009900\"";
            markup = g_strdup_printf("<span size=\"small\" %s>%s</span>", color, _(p->label));
            TRACE("*** markup=%s\n", color);
            Gtk<Type>::menu_item_content(GTK_MENU_ITEM(v), p->icon, markup, -16);
            if (p->tooltip) {
                //Tooltip<Type>::custom_tooltip(GTK_WIDGET(v), NULL, markup);
                //Tooltip<Type>::custom_tooltip(GTK_WIDGET(v), pixbuf, markup);
                gtk_widget_set_tooltip_text(GTK_WIDGET(v), p->tooltip);
            }
            g_free(markup);
            //gtk_widget_set_sensitive(GTK_WIDGET(v), p->sensitive);
            gtk_widget_show (v);
        }
        gtk_widget_show (GTK_WIDGET(menu));
        return menu;
    }
private:

    static void
    decorateItems(GtkMenu *menu, const gchar *key[], const gchar *keyIcon[], gboolean small){
        TRACE("decorateItems, menu = %p\n", menu );
        if (!key || !keyIcon) {
            DBG("!key || !keyIcon\n");
            return;
        }
        gint i=0;
        for (auto k=key; key[i] && keyIcon[i]; k++, i++){
          TRACE("key=%s\n", *k);
            auto mItem = (GtkMenuItem *)g_object_get_data(G_OBJECT(menu), *k);
            if (!mItem) {
                DBG("g_object_get_data(G_OBJECT(menu), \"%s\") failed.\n", *k);
                continue;
            }
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

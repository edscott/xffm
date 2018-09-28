#ifndef XF_VIEW__HH
# define XF_VIEW__HH
namespace xf
{

template <class Type>
class BaseView{

public:
    BaseView(void){
        iconView_=createIconview();
    }
    GtkIconView *iconView(void){return iconView_;}
private:
    GtkIconView *createIconview(void){
        auto icon_view = GTK_ICON_VIEW(gtk_icon_view_new());
        g_object_set(G_OBJECT(icon_view), "has-tooltip", TRUE, NULL);
        gtk_icon_view_set_item_width (icon_view, 60);
        gtk_icon_view_set_activate_on_single_click(icon_view, TRUE);
        return icon_view;
    }

    GtkIconView *iconView_;
};
}
#endif

#ifndef XF_VBUTTONBOX
#define XF_VBUTTONBOX
#include "hbuttonbox.hh"
namespace xf {

template <class Type>
class VButtonBox {
    using gtk_c = Gtk<double>;
        // FIXME: hidden_button icon is custom and button should be at tab level.
        //        this does not belong here
	/*auto hidden_button =  gtk_c::toggle_button("semi-starred", NULL);
	 g_object_set_data(G_OBJECT(vButtonBox_), "hidden_button", hidden_button);
	gtk_box_pack_end (vButtonBox_, GTK_WIDGET(hidden_button), FALSE, FALSE, 0);*/    
public:
    GtkBox *vButtonBox(void){return vButtonBox_;}
    VButtonBox(void){
	vButtonBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

        GError *error=NULL;
	GtkStyleContext *style_context = gtk_widget_get_style_context (GTK_WIDGET(vButtonBox_));
	gtk_style_context_add_class(style_context, GTK_STYLE_CLASS_BUTTON );
	GtkCssProvider *css_provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data (css_provider, 
 //     background-color: #dcdad5;
    "\
    box * {\
      background-color: #333333;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    ", 
	    -1, &error);
	gtk_style_context_add_provider (style_context, GTK_STYLE_PROVIDER(css_provider),
				    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	auto help = HButtonBox<double>::newButton("help-about-symbolic", _("Help"));
	gtk_box_pack_end (vButtonBox_, GTK_WIDGET(help), FALSE, FALSE, 0);

	auto home = HButtonBox<double>::newButton("go-home", _("Home Directory"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(home), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(home), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::home), NULL);
	auto fstab = HButtonBox<double>::newButton("media-eject", _("Disk Image Mounter"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(fstab), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(fstab), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::fstab), NULL);
	auto pkg = HButtonBox<double>::newButton("x-package-repository", _("Software Updater"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(pkg), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(pkg), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::pkg), NULL);
	auto trash = HButtonBox<double>::newButton("user-trash", _("Trash bin"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(trash), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(trash), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::trash), NULL);
	auto terminal = HButtonBox<double>::newButton("utilities-terminal", _("Open terminal"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(terminal), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(terminal), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::terminal), NULL);
	auto newWindow = HButtonBox<double>::newButton("window-new", _("Open a New Window"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(newWindow), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(newWindow), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::newWindow), NULL);
	auto search = HButtonBox<double>::newButton("system-search", _("Search"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(search), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(search), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::search), NULL);
	gtk_widget_show_all(GTK_WIDGET(vButtonBox_));
	return ;
    }
protected:
private:
    GtkBox *vButtonBox_;
    static GtkScale *newSizeScale(void){
	auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 6.0, 24.0, 6.0));
        gtk_range_set_value(GTK_RANGE(size_scale), 12);
        gtk_range_set_increments (GTK_RANGE(size_scale), 2.0, 6.0);
	gtk_widget_set_size_request (GTK_WIDGET(size_scale),-1,75);
        return size_scale;
    }
};



}



#endif

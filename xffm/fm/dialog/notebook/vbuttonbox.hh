#ifndef XF_VBUTTONBOX
#define XF_VBUTTONBOX
namespace xf {
    

template <class Type>
class VButtonBox {
    using gtk_c = Gtk<double>;
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
        if (error){
            ERROR("vbuttonbox.hh::VButtonBox(): %s\n", error->message);
            g_error_free(error);
        } else {
	    gtk_style_context_add_provider (style_context, GTK_STYLE_PROVIDER(css_provider),
				    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
#ifdef HAVE_PKG
	auto pkg = 
	    gtk_c::newButton("emblem-bsd", _("Software Updater"));
#else
# ifdef HAVE_PACMAN
	auto pkg = 
	    gtk_c::newButton("emblem-archlinux", _("Software Updater"));
# else
#  if HAVE_EMERGE
	auto pkg = 
	    gtk_c::newButton("emblem-gentoo", _("Software Updater"));
#  else
	auto pkg = 
	    gtk_c::newButton("help-about-symbolic", _("Software Updater"));
#  endif
# endif
#endif
	gtk_box_pack_end (vButtonBox_, GTK_WIDGET(pkg), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(pkg), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::pkg), NULL);

	auto userButtonFiles = Settings<Type>::getSettingString("userbuttons", "files");
	if (userButtonFiles){
            gchar **files;
            if (strchr(userButtonFiles ,':')) {
                files = g_strsplit(userButtonFiles, ":",-1);
            } else {
               files = (gchar **)calloc(2, sizeof(gchar *)); 
               files[0] = g_strdup(userButtonFiles);
            }
            for (auto p=files; p && *p; p++){
                if (g_file_test(*p, G_FILE_TEST_EXISTS)){
                    auto userbutton = UserResponse<Type>::userbutton(*p);
                    if (userbutton) gtk_box_pack_end (vButtonBox_, GTK_WIDGET(userbutton), FALSE, FALSE, 0);
                }
            }
            g_strfreev(files);
	}
	g_free(userButtonFiles);

        auto search = gtk_c::newButton("system-search", _("Search"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(search), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(search), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::search), NULL);

        auto terminal = gtk_c::newButton("utilities-terminal", _("Open terminal"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(terminal), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(terminal), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::terminal), NULL);

        auto newWindow = gtk_c::newButton("window-new", _("Open a New Window"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(newWindow), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(newWindow), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::open), 
                (void *)"xffm");

        auto differences = gtk_c::newButton("differences", _("Differences"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(differences), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(differences), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::open), 
                (void *)"rodent-diff");

        auto home = gtk_c::newButton("go-home", _("Home Directory"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(home), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(home), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::home), NULL);

        auto fstab = gtk_c::newButton("media-eject", _("Disk Image Mounter"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(fstab), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(fstab), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::fstab), NULL);

	auto trash = gtk_c::newButton("user-trash", _("Trash bin"));
	gtk_box_pack_start (vButtonBox_, GTK_WIDGET(trash), FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(trash), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::trash), NULL);

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

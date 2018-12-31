#ifndef XF_DIALOG
#define XF_DIALOG

#include "notebook.hh"
#include "dialogsignals.hh"
#include <memory>

namespace xf {

template <class Type>
class Dialog :public Notebook<Type> {
    using pixbuf_c = Pixbuf<double>;
    using print_c = Print<double>;
    GFileMonitor *deviceMonitor_;
    GCancellable *cancellable_;
    GFile *gfile_;
public:
    Dialog(const gchar *path){
	init(path);
        Mime<Type>::mimeBuildHashes();
        auto page = this->currentPageObject();
	// Default into iconview...
        page->setDefaultIconview(TRUE);
	page->showIconview(1);
        startDeviceMonitor();
    }

 /*   Dialog(const gchar *title, const gchar *icon){
        Dialog();
	setDialogIcon(icon);
	setDialogTitle(title);
    } */
    ~Dialog(void){
	g_file_monitor_cancel(deviceMonitor_);
    }

    void init(const gchar *path){
	Settings<Type>::readSettings();
        isTreeView = (Settings<Type>::getSettingInteger("window", "TreeView") > 0);
	mainWindow = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
        g_object_set_data(G_OBJECT(mainWindow), "dialogObject", (void *)this);
        g_signal_connect (G_OBJECT (mainWindow), "delete-event", EVENT_CALLBACK (dialogSignals<Type>::delete_event), NULL);

        gtk_widget_set_has_tooltip (GTK_WIDGET(mainWindow), TRUE);
        // FIXME: enable tooltip
        //g_signal_connect (G_OBJECT (mainWindow), "query-tooltip", G_CALLBACK (window_tooltip_f), (void *)this);
        g_signal_connect (G_OBJECT (mainWindow), "key-press-event", KEY_EVENT_CALLBACK (dialogSignals<Type>::window_keyboard_event), (void *)this);

	gtk_widget_get_preferred_width (GTK_WIDGET(mainWindow), &dialogMinW_, &dialogNatW_);
	gtk_widget_get_preferred_height (GTK_WIDGET(mainWindow), &dialogMinH_, &dialogNatH_);
	gtk_window_set_type_hint(mainWindow, GDK_WINDOW_TYPE_HINT_DIALOG);
	//setWindowMaxSize(mainWindow);
	gtk_window_set_position (mainWindow, GTK_WIN_POS_MOUSE);
        this->insertNotebook(mainWindow);     

        this->addPage(path); 
        TRACE("dialog this=%p\n", (void *)this);
        //this->insertPageChild(this->notebook());
        
        
	gint max, current;
        auto vpane = this->vpane();
	g_object_get(G_OBJECT(vpane), "max-position", &max, NULL);
	g_object_get(G_OBJECT(vpane), "position", &current, NULL);
	g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(current));
	g_object_set_data(G_OBJECT(vpane), "oldMax", GINT_TO_POINTER(max));
        
	g_signal_connect (G_OBJECT (mainWindow), "size-allocate", 
		SIZE_CALLBACK(dialogSignals<Type>::onSizeAllocate), (void *)this);
        setDefaultSize();
        setDefaultFixedFontSize();
        gtk_window_present (mainWindow);
        while (gtk_events_pending()) gtk_main_iteration();
	return;
    }

    void
    startDeviceMonitor(void){
        const gchar *path = "/dev/disk/by-id";
        gfile_ = g_file_new_for_path (path);
        GError *error=NULL;
        cancellable_ = g_cancellable_new ();
        
        deviceMonitor_ = g_file_monitor (gfile_, G_FILE_MONITOR_WATCH_MOVES, cancellable_,&error);
        if (error){
            ERROR("g_file_monitor_directory(%s) failed: %s\n",
                    path, error->message);
            g_object_unref(gfile_);
            return;
        }
        g_signal_connect (deviceMonitor_, "changed", 
                G_CALLBACK (monitor_f), (void *)this);
    }

    static void
    monitor_f (GFileMonitor      *mon,
              GFile             *first,
              GFile             *second,
              GFileMonitorEvent  event,
              gpointer           data)
    {

        // Here we enter with full path to partiuuid...
        gchar *f= first? g_file_get_path (first):g_strdup("--");
        gchar *s= second? g_file_get_path (second):g_strdup("--");
       
        gchar *base = g_path_get_basename(f);
        TRACE("*** monitor_f call...\n");
        gchar *fsType;
        switch (event){
            case G_FILE_MONITOR_EVENT_DELETED:
            case G_FILE_MONITOR_EVENT_MOVED_OUT:
            if (!strstr(f, "part")){
                gchar *path = FstabView<Type>::id2Partition(f);
                gchar *markup = g_strdup_printf("%s %s", _("Removed"), base);
                TimeoutResponse<Type>::dialog(NULL, markup, "drive-harddisk/SE/go-down/3.0/180");
                g_free(markup);
                g_free(path);
                DBG("*** Device has been removed: %s\n", f);
            }
            break;
            case G_FILE_MONITOR_EVENT_CREATED:
            case G_FILE_MONITOR_EVENT_MOVED_IN:
            if (!strstr(f, "part")){
                gchar *g = g_strdup_printf(_("Inserted %s"), "" );
                gchar *path = FstabView<Type>::id2Partition(f);
                gchar *label = FstabView<Type>::e2Label(path);
                gchar *markup = g_strdup_printf("%s    <span color=\"red\">%s</span>    <span color=\"green\">%s</span>\n%s\n", g, path, label?label:"", base );
                TimeoutResponse<Type>::dialog(NULL, markup, "drive-harddisk/SE/go-up/3.0/180");
                g_free(markup);
                g_free(path);
                g_free(label);
                DBG("*** Device has been added: %s\n", f);
            }
            break;
        }

        g_free(f);
        g_free(s);
    }
   

    void setDialogTitle(const gchar *title){
	gtk_window_set_title (mainWindow, title);
    }
    void setDialogIcon(const gchar *icon){
	auto pixbuf = pixbuf_c::get_pixbuf(icon, SIZE_ICON);
	gtk_window_set_icon (mainWindow, pixbuf);
	g_object_unref(pixbuf);
    }

    void resizeWindow(gint fontSize){
        if (fontSize == 0){
            ERROR("fontSize cannot be zero\n");
            return;
        }
        if (naturalSize_.width == 0 ||
                naturalSize_.height == 0){
            gtk_widget_get_preferred_size (GTK_WIDGET(mainWindow),
                               &minimumSize_,
                               &naturalSize_);
        }
        // First try a saved width/height
        gint width = Settings<Type>::getSettingInteger("window", "width");
        gint height = Settings<Type>::getSettingInteger("window", "height");
        if (width >= naturalSize_.width && height >= naturalSize_.height){
            gtk_window_resize (GTK_WINDOW(mainWindow), width, height);
            return;
        }
        // Now adapt a window size to the selected font
        gint Dw = 8*maximumSize_.width/9 - naturalSize_.width ;
        if (Dw < 0) Dw = 0;
        gint Dh = 12*maximumSize_.height/13 - naturalSize_.height;
        if (Dh < 0) Dh = 0;
        double fraction = (double)(fontSize - 6)/(24 - 6);
        gint w = (fraction * Dw) + naturalSize_.width;
        gint h = (fraction * Dh) + naturalSize_.height;
        WARN("resize window %d: %lf --> %d,%d (min: %d, %d)\n",
               fontSize, fraction, w, h,
               naturalSize_.width,
               naturalSize_.height);
        gtk_window_resize (GTK_WINDOW(mainWindow), w, h);
        
    } 

protected:



    void setDefaultFixedFontSize(void){
        auto page = this->currentPageObject();
	gint size = page->fontSize();
//        page->setSizeScale(size);
        print_c::set_font_size(GTK_WIDGET(page->output()), size);
        print_c::set_font_size(GTK_WIDGET(page->input()), size);
        resizeWindow(size);
    }

private:
        GtkRequisition minimumSize_;
        GtkRequisition naturalSize_;
        GtkRequisition chosenSize_;
        GtkRequisition maximumSize_;
    void setDefaultSize(void){
        gtk_widget_get_preferred_size (GTK_WIDGET(mainWindow),
                               &minimumSize_,
                               &naturalSize_);
        setWindowMaxSize();
        TRACE("Size: minimum=%d,%d, natural=%d,%d, max=%d,%d\n",
                minimumSize_.width, minimumSize_.height,
                naturalSize_.width, naturalSize_.height,
                maximumSize_.width, maximumSize_.height);
    }

    void setDefaultSize(gint w, gint h){
        chosenSize_.width = w;
        chosenSize_.height = h;
    }
    void setWindowMaxSize(void){
	gint x_return, y_return;
	guint w_return, h_return, d_return, border_return;
	Window root_return;
	auto drawable = gdk_x11_get_default_root_xwindow ();
	//Visual Xvisual = gdk_x11_visual_get_xvisual(gdk_visual_get_system());
	auto display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
	XGetGeometry(display, drawable, &root_return,
		&x_return, &y_return, 
		&w_return, &h_return, 
		&border_return, 
		&d_return);
	GdkGeometry geometry;
	geometry.max_width = w_return - 25;
	geometry.max_height = h_return -25;
        maximumSize_.width = geometry.max_width;
        maximumSize_.height = geometry.max_height;
	gtk_window_set_geometry_hints (GTK_WINDOW(mainWindow), GTK_WIDGET(mainWindow), &geometry, GDK_HINT_MAX_SIZE);
    }

private:
    gint dialogMinW_, dialogNatW_, dialogMinH_, dialogNatH_;
};
}

#endif

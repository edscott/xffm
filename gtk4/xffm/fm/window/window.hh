#ifndef XF_WINDOW_HH
#define XF_WINDOW_HH
#define NOTEBOOK_CALLBACK(X)  G_CALLBACK((void (*)(GtkNotebook *,GtkWidget *, guint, gpointer)) X)

namespace xf {

class MainWindow  {
private:
    GList *pageList_=NULL;
    GtkWindow *mainWindow_ = NULL;
    GtkNotebook *notebook_;
    GtkBox *vButtonBox_;
    double windowH_ = 400;    
    double windowW_ = 400*1.618;    
    GList *run_button_list=NULL;
    pthread_mutex_t *rbl_mutex; // run button list mutex

    GtkWidget *longPressImage_=NULL;
    GHashTable *pageHash_=NULL;
// Constructor  
public:
    GtkNotebook *getNotebook(void) {return notebook_;}
    MainWindow(const gchar *path){
        createWindow();        
        // for page: startDeviceMonitor();
        auto box = contentBox(path);
        gtk_window_set_child(mainWindow_, box);
        addPage(path);
        showWindow();
    }

    ~MainWindow(void){
       // for each page: g_file_monitor_cancel(deviceMonitor_);
    }
    
// Free functions (for signals)
public:
    static void
    on_new_page(GtkButton *button, void *data){
        MainWindow *w = (MainWindow *)data;
        w->addPage("foo");
    }
    static void
    on_zap_page(GtkButton *button, void *data){
        MainWindow *w = (MainWindow *)data;
        w->zapPage();
    }
    static void
    on_switch_page(GtkNotebook *notebook,
                   GtkWidget   *page,
                   guint        new_page,
                   gpointer     data){
        MainWindow *w = (MainWindow *)data;
        TRACE("new page=%d\n", new_page);
        w->switchPage(new_page);
    }

private:

    void createWindow(void){
        mainWindow_ = GTK_WINDOW(gtk_window_new ());
        g_object_set_data(G_OBJECT(mainWindow_), "windowObject", (void *)this);
        gtk_window_set_default_size(mainWindow_, windowW_, windowH_);


   
  //      g_signal_connect (G_OBJECT (mainWindow_), "size-allocate", 
    //            SIZE_CALLBACK(DialogSignals<Type>::onSizeAllocate), (void *)this);
        //setDefaultFixedFontSize();
        return;
    }

    void showWindow(){

        GdkDisplay *displayGdk = gdk_display_get_default();
        Display *display = gdk_x11_display_get_xdisplay(displayGdk);

        XClassHint *wm_class = (XClassHint *)calloc(1, sizeof(XClassHint));
        wm_class->res_name = g_strdup("xffm");
        wm_class->res_class = g_strdup("Xffm");

        GtkWidget *widget = GTK_WIDGET(mainWindow_);
        gtk_widget_realize(widget);
        GtkNative *native = gtk_widget_get_native(widget);
        GdkSurface *surface = gtk_native_get_surface(native);
        Window w = gdk_x11_surface_get_xid (surface);
        XSetClassHint(display, w, wm_class);


        gtk_window_present (mainWindow_);

    }

    void addPage(const gchar *path){
      gchar *tag = path? g_path_get_basename(path):g_strdup(".");

      GtkWidget *child = gtk_label_new(tag);// XXX temporary
      
      pageList_ = g_list_append(pageList_, child);
      auto label = tabLabel(path, (void *)this);
      auto close = g_object_get_data(G_OBJECT(label), "close");
      g_object_set_data(G_OBJECT(child), "close", close);

      auto num = gtk_notebook_append_page (notebook_, child, label);
      g_free(tag);
      if (num >= 0) {
        while (num != gtk_notebook_get_current_page(notebook_)) 
          gtk_notebook_next_page(notebook_);
      }
    }

    void zapPage(){
      int num = gtk_notebook_get_current_page(notebook_);
      GtkWidget *child = gtk_notebook_get_nth_page (notebook_, num);
      GList *item = g_list_find(pageList_, child);
      pageList_ = g_list_remove(pageList_, child);
      if (g_list_length(pageList_) == 0){
        gtk_widget_set_visible (GTK_WIDGET(mainWindow_), FALSE);
        exit(0);
      }
        
      gtk_notebook_remove_page(notebook_, num);
    }

    void switchPage (gint new_page) {
      // hide all close buttons
      for (GList *l=pageList_; l && l->data; l=l->next){
        GtkWidget *box = (GtkWidget *)l->data;
        GtkWidget *w = GTK_WIDGET(g_object_get_data(G_OBJECT(box), "close"));
        gtk_widget_set_visible (w, FALSE);
      }

      // Show current close button
      auto child = gtk_notebook_get_nth_page(notebook_, new_page);
      auto close = GTK_WIDGET(g_object_get_data(G_OBJECT(child), "close"));
      gtk_widget_set_visible (close, TRUE);


/*   
      static gboolean startup = TRUE;
        TRACE("switch_page: new page=%d last page=%d\n", new_page, lastPage);
        auto notebook_p = (Notebook<Type> *)data;
        auto page_p = (Page<Type> *)notebook_p->currentPageObject(new_page);
        if (!page_p){
            if (!startup) ERROR("fm/dialog/notebook/signals.hh::page_p is null\n");
            return;
        }
        startup = FALSE;
        page_p->setDialogTitle();
        gtk_widget_set_sensitive(GTK_WIDGET(page_p->pageLabelButton()), TRUE);
        gtk_widget_show(GTK_WIDGET(page_p->pageLabelButton()));
        auto pages = gtk_notebook_get_n_pages(notebook);

        if (lastPage >= 0 && lastPage != new_page && lastPage < pages){
            page_p = (Page<Type> *)notebook_p->currentPageObject(lastPage);
            if(page_p) {
                gtk_widget_set_sensitive(GTK_WIDGET(page_p->pageLabelButton()), FALSE);
                gtk_widget_hide(GTK_WIDGET(page_p->pageLabelButton()));
            }
        }
        lastPage = new_page;
*/
    }

    GtkButton *newButton(const gchar *icon, const gchar *tooltipText){
      auto button = GTK_BUTTON(gtk_button_new_from_icon_name(icon));
      auto t =g_strconcat("<span color=\"yellow\"><i>", tooltipText, "</i></span>", NULL);
      gtk_widget_set_tooltip_markup (GTK_WIDGET(button),t);
      g_free(t);
      gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
      gtk_button_set_has_frame(button, FALSE);
      return button;
    }

    GtkWidget *tabLabel(const gchar *path, void *data){
      MainWindow *w = (MainWindow *)data;
      auto tabBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_widget_set_hexpand(GTK_WIDGET(tabBox), FALSE);
      gtk_widget_set_vexpand(GTK_WIDGET(tabBox), FALSE);
      gchar *tag = path? g_path_get_basename(path):g_strdup(".");
      GtkWidget *label = gtk_label_new(tag);
      compat<bool>::boxPack0(tabBox, label,  FALSE, FALSE, 0);

      auto close = newButton(WINDOW_CLOSE, _("Close"));
      g_signal_connect(G_OBJECT(close), "clicked", 
              BUTTON_CALLBACK(w->on_zap_page), data);    
      g_object_set_data(G_OBJECT(tabBox), "close", close);
      
      compat<bool>::boxPack0(tabBox, GTK_WIDGET(close),  FALSE, FALSE, 0);
      return GTK_WIDGET(tabBox);
    }
    
    GtkBox *mkVbuttonBox(){
        auto hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), FALSE);
        vButtonBox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(vButtonBox_), TRUE);

        const char *bIcon[]={SEARCH, OPEN_TERMINAL, OPEN_FILEMANAGER, GO_HOME, DRIVE_HARDDISK, TRASH_ICON, GLIST_ADD, GLIST_REMOVE, NULL};
        const char *bText[]={_("Search"),_("Open terminal"),_("Open a New Window"),_("Home Directory"),_("Disk Image Mounter"),_("Trash bin"),_("Reset image size"),_("Reset image size"), NULL};
        auto q = bText;
        for (auto p=bIcon; p && *p; p++, q++){
          auto button = newButton(*p, *q);
          compat<bool>::boxPack0(vButtonBox_, GTK_WIDGET(button),  FALSE, FALSE, 0);
        }
        
        compat<bool>::boxPack0(hbox, GTK_WIDGET(vButtonBox_),  FALSE, FALSE, 0);
/*
 // Color setting deprecated in 4.10, no viable replacement found (doesn't mean there isn't one).
        GtkStyleContext *style_context = gtk_widget_get_style_context (GTK_WIDGET(vButtonBox_));
        gtk_style_context_add_class(style_context, GTK_STYLE_CLASS_BUTTON );
        GtkCssProvider *css_provider = gtk_css_provider_new();
        gtk_css_provider_load_from_string (css_provider, 
 //     background-color: #dcdad5;
    "\
    box * {\
      background-color: #888888;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    ");
         {
            gtk_style_context_add_provider (style_context, GTK_STYLE_PROVIDER(css_provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
*/

        return hbox;
    }

    void mkNotebook(){
      notebook_ = GTK_NOTEBOOK(gtk_notebook_new());
      //g_object_set_data(G_OBJECT(this->menuButton_), "notebook_p", (void *)this);
      //pageHash_ =g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
      gtk_notebook_set_scrollable (notebook_, TRUE);

      longPressImage_ = gtk_label_new("");
      gtk_widget_set_visible (GTK_WIDGET(longPressImage_), FALSE);
      auto text = g_strdup_printf("<span color=\"red\">%s</span>",_("Long press time"));
      gtk_label_set_markup(GTK_LABEL(longPressImage_),text);
      g_free(text);

      auto actionWidget = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
      auto tabButtonBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

      auto newTabButton = newButton("list-add", _("New Tab"));


      g_signal_connect(G_OBJECT(newTabButton), "clicked", 
              BUTTON_CALLBACK(MainWindow::on_new_page), (void *)this);    
      g_signal_connect (notebook_, "switch-page", 
                NOTEBOOK_CALLBACK (on_switch_page), (void *)this);
      
      gtk_widget_set_hexpand(GTK_WIDGET(actionWidget), FALSE);

      compat<bool>::boxPack0(tabButtonBox, GTK_WIDGET(longPressImage_),  TRUE, FALSE, 0);
      compat<bool>::boxPack0(tabButtonBox, GTK_WIDGET(newTabButton),  TRUE, FALSE, 0);
      //compat<bool>::boxPack0(tabButtonBox, GTK_WIDGET(this->menuButton()),  TRUE, FALSE, 0);
      compat<bool>::boxPack0(actionWidget, GTK_WIDGET(tabButtonBox),  TRUE, FALSE, 0);

      gtk_notebook_set_action_widget (notebook_, GTK_WIDGET(actionWidget), GTK_PACK_END);

    }

    GtkWidget *contentBox(const gchar *path){
      pthread_mutexattr_t r_attr;
      pthread_mutexattr_init(&r_attr);
      pthread_mutexattr_settype(&r_attr, PTHREAD_MUTEX_RECURSIVE);
      rbl_mutex = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
      pthread_mutex_init(rbl_mutex, &r_attr);


      auto mainBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
      auto hbox1 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
      compat<bool>::boxPack0(mainBox, GTK_WIDGET(hbox1),  TRUE, TRUE, 0);
      gtk_widget_set_hexpand(GTK_WIDGET(hbox1), TRUE);

      mkNotebook();
      compat<bool>::boxPack0(hbox1, GTK_WIDGET(notebook_),  TRUE, TRUE, 0);

      auto hbox2 = mkVbuttonBox();    
      compat<bool>::boxPack0(mainBox, GTK_WIDGET(hbox2),  FALSE, FALSE, 0);



      return GTK_WIDGET(mainBox);

    }

};



}



/*    static void removePage(GtkButton *self, void *data){
      GtkNotebook *notebook = GTK_NOTEBOOK(data);
      int num = gtk_notebook_get_current_page(notebook);
      gtk_notebook_remove_page(notebook, num);
    }*/



    
/*
 
    GFileMonitor *deviceMonitor_;
    GCancellable *cancellable_;
    GFile *gfile_;

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
        TRACE("*** dialog.hh:monitor_f call for %s\n", f);
        gchar *fsType;
        switch (event){
            case G_FILE_MONITOR_EVENT_DELETED:
            case G_FILE_MONITOR_EVENT_MOVED_OUT:
                TRACE("moved out: %s\n", f);
            if (!strstr(f, "part")){ // no partition id when device is removed.
#if 0
                gchar *partition = FstabView<Type>::id2Partition(f);
                if (partition) {
                    gchar *markup = g_strdup_printf("%s %s", _("Removed"), base);
                    TimeoutResponse<Type>::dialog(NULL, markup, "drive-harddisk/SE/go-down/3.0/180");
                    g_free(markup);
                }
                g_free(partition);
                TRACE(" Device has been removed: %s (%s)\n", f, partition);
#endif

            }
            break;
            case G_FILE_MONITOR_EVENT_CREATED:
            case G_FILE_MONITOR_EVENT_MOVED_IN:
                TRACE("moved in: %s\n", f);
#ifdef xENABLE_FSTAB_MODULE
            if (strstr(f, "part")){ // When device is added, we have partition id.
                gchar *path = FstabView<Type>::id2Partition(f); // path to partition
                gchar *label = NULL;
                if (path){
                    label = FstabView<Type>::e2Label(path);
                }
                TRACE(" Device has been added: %s label=%s path=%s \n", 
                        f, label, path);
                g_free(label);
                g_free(path);
            }
#endif
            break;
            case G_FILE_MONITOR_EVENT_CHANGED:
            case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
            case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
            case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
            case G_FILE_MONITOR_EVENT_UNMOUNTED:
            case G_FILE_MONITOR_EVENT_MOVED:
            case G_FILE_MONITOR_EVENT_RENAMED:
            break;
        }

        g_free(f);
        g_free(s);
    }
 
    void
    startDeviceMonitor(void){
        const gchar *path = "/dev/disk/by-id";
        gfile_ = g_file_new_for_path (path);
        GError *error=NULL;
        cancellable_ = g_cancellable_new ();
        
        deviceMonitor_ = g_file_monitor (gfile_, G_FILE_MONITOR_WATCH_MOVES, cancellable_,&error);
        if (error){
            ERROR("fm/dialog.hh::g_file_monitor_directory(%s) failed: %s\n",
                    path, error->message);
            g_object_unref(gfile_);
            return;
        }
        g_signal_connect (deviceMonitor_, "changed", 
                G_CALLBACK (monitor_f), (void *)this);
    }

    // delete/destroy
    static gboolean delete_event (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   user_data){
        gtk_widget_set_visible(widget, FALSE);
        gtk_window_destroy(GTK_WINDOW(widget));
        //_exit(123);
        return TRUE;
    }
    
*/

   
/*
    void setDialogTitle(const gchar *title){
        gtk_window_set_title (mainWindow_, title);
    }
    void setDialogIcon(const gchar *icon){
        auto pixbuf = pixbuf_c::getPixbuf(icon, SIZE_ICON);
        gtk_window_set_icon (mainWindow_, pixbuf);
        g_object_unref(pixbuf);
    }
*/
    /*
    void resizeWindow(gint fontSize){
        if (fontSize == 0){
            ERROR("fm/dialog.hh::fontSize cannot be zero\n");
            return;
        }
        if (naturalSize_.width == 0 ||
                naturalSize_.height == 0){
            gtk_widget_get_preferred_size (GTK_WIDGET(mainWindow_),
                               &minimumSize_,
                               &naturalSize_);
        }
        // First try a saved width/height
        gint width = Settings<Type>::getInteger("window", "width");
        gint height = Settings<Type>::getInteger("window", "height");
        if (width >= naturalSize_.width && height >= naturalSize_.height){
            gtk_window_resize (GTK_WINDOW(mainWindow_), width, height);
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
        TRACE("resize window %d: %lf --> %d,%d (min: %d, %d)\n",
               fontSize, fraction, w, h,
               naturalSize_.width,
               naturalSize_.height);
        gtk_window_resize (GTK_WINDOW(mainWindow_), w, h);
        
    } 
*/

/*

    void setDefaultFixedFontSize(void){
        auto page = this->currentPageObject();
        gint size = page->fontSize();
//        page->setSizeScale(size);
        print_c::set_font_size(GTK_WIDGET(page->output()), size);
        print_c::set_font_size(GTK_WIDGET(page->input()), size);
        resizeWindow(size);
    }
*/


#endif

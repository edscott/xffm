#ifndef XF_FMBUTTONBOX_HH
#define XF_FMBUTTONBOX_HH
//#define BUTTON_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,GdkDragContext *,gpointer)) X)
namespace xf {
  template <class Type> class MenuCallbacks;
  class EmptyButtonBox{
    public:
    GtkBox *mkVbuttonBox(){
      return GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));  
    }
  };
  class FMbuttonBox{
    private:
      GtkBox *vButtonBox_;
    public:
    GtkBox *mkVbuttonBox(){
         const char *bIcon[]={
           SEARCH, 
           OPEN_TERMINAL, 
           GO_HOME, 
           //OPEN_FILEMANAGER, 
           //EDIT_CLEAR,
           "emblem-fifo",
           NULL
         };
        const char *bText[]={
          _("Search"),
          _("Open terminal"),
          _("Home"),
          //_("Clear Log"),
          _("Toggle Text Mode"),
          NULL
        };
        void *bCallback[]={
          (void *)MenuCallbacks<bool>::openFind,
          (void *)MenuCallbacks<bool>::openTerminal,
          (void *)MenuCallbacks<bool>::goHome,
          (void *)MenuCallbacks<bool>::toggleVpane,
          NULL
        };
        
        auto scale = newSizeScale(_("Icon Size"));
        auto hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), FALSE);
        vButtonBox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        
        gtk_widget_add_css_class (GTK_WIDGET(vButtonBox_), "vbox" );


        gtk_widget_set_hexpand(GTK_WIDGET(vButtonBox_), TRUE);
        auto q = bText;
        auto r = bCallback;
        /*const char *bIcon[]={OPEN_FILEMANAGER, GO_HOME, DRIVE_HARDDISK, TRASH_ICON, NULL};
        const char *bText[]={_("Open a New Window"),_("Home Directory"),_("Disk Image Mounter"),_("Trash bin"),_ NULL};*/
        for (auto p=bIcon; p && *p; p++, q++){
          auto button = Basic::newButton(*p, *q);
          Basic::boxPack0(vButtonBox_, GTK_WIDGET(button),  FALSE, FALSE, 0);
          if (*r) {
            g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK(*r), NULL);
            (r)++;
          }
        }

        auto colorButton = Basic::newMenuButton(DOCUMENT_PROPERTIES, _("Color settings"));
        auto myColorMenu = new Menu<IconColorMenu<bool> >(_("Colors"));
        myColorMenu->setMenu(colorButton);
        delete myColorMenu;
        Basic::boxPack0(vButtonBox_, GTK_WIDGET(colorButton),  FALSE, FALSE, 0);
      
        Basic::boxPack0(vButtonBox_, GTK_WIDGET(scale),  FALSE, FALSE, 0);        
        Basic::boxPack0(hbox, GTK_WIDGET(vButtonBox_),  FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(MainWidget), "buttonBox", vButtonBox_);

        return hbox;
    }
private:
      static gboolean enterRange(GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data){
        TRACE("enterRange\n");
        auto range = GTK_RANGE(data);
        int value = gtk_range_get_value(range);
        g_object_set_data(G_OBJECT(range), "valor", GINT_TO_POINTER(value));
        //Texture<bool>::redlight(); obsolete
        return FALSE;
      }
      static gboolean leaveRange(GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data){
        TRACE("leaveRange\n");
        auto range = GTK_RANGE(data);
        int value = gtk_range_get_value(range);
        if (value != GPOINTER_TO_INT(g_object_get_data(G_OBJECT(range), "valor"))){
          TRACE("getWorkdir\n");
          auto wd = Workdir<bool>::getWorkdir();
          if (g_file_test(wd, G_FILE_TEST_EXISTS)) {
            // race condition here, workdir null or garbage.
            // Workdir::reset();      
          }
        }
        //Texture<bool>::greenlight(); obsolete
        return FALSE;
      }
    
      static
      void addMotionController(GtkWidget  *widget){
        auto controller = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), controller);
        g_signal_connect (G_OBJECT (controller), "enter", 
            G_CALLBACK (enterRange), widget);
        g_signal_connect (G_OBJECT (controller), "leave", 
            G_CALLBACK (leaveRange), widget);
    }
    
    GtkScale *newSizeScale(const gchar *tooltipText){
        double value;
        auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 24.0, 384.0, 12.0));
        // Load saved value fron xffm+/settings.ini file (if any)
        auto size = Settings::getInteger("xfterm", "iconsize");
        if (size < 0) value = 48; else value = size;
        TRACE("range set value=%lf\n", value);
        gtk_range_set_value(GTK_RANGE(size_scale), value);

        gtk_range_set_increments (GTK_RANGE(size_scale), 12.0, 12.0);
        gtk_widget_set_size_request (GTK_WIDGET(size_scale),-1,75);

        gtk_scale_set_value_pos (size_scale,GTK_POS_BOTTOM);
        //gtk_adjustment_set_upper (gtk_range_get_adjustment(GTK_RANGE(size_scale)), 24.0);
        Basic::setTooltip (GTK_WIDGET(size_scale),tooltipText);   
        g_signal_connect(G_OBJECT(size_scale), "value-changed", G_CALLBACK(changeSize), NULL);
        addMotionController(GTK_WIDGET(size_scale));
        return size_scale;
    }
    static void
    changeSize (GtkRange* self, gpointer user_data){
      if (Workdir<bool>::pleaseWait()) return;
      auto value = gtk_range_get_value(self);
      Settings::setInteger("xfterm", "iconsize", value);
      
      // reload all pages
      auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(MainWidget), "notebook"));
      auto n = gtk_notebook_get_n_pages(notebook);
      

      for (int i=0; i<n; i++){
        auto child = gtk_notebook_get_nth_page(notebook, i);
        auto path = Child::getWorkdir(child);
        Workdir<LocalDir>::setWorkdir(path, child);
               
      }
    }

    static void
    upImage (GtkButton *self, void *data){
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto workDir = Child::getWorkdir(childWidget);

      auto pixels = Settings::getInteger("ImageSize", workDir);
      if (pixels < 48) pixels = 48;
      else if (pixels < 384) pixels = 384;
      else if (pixels < 768) pixels = 768;
      Settings::setInteger("ImageSize", workDir, pixels);
      Print::print(output, g_strdup_printf("FIXME upImage:: pixels set at %d\n", pixels));
      /* FIXME
        if (pixels !=  Settings::getInteger("ImageSize", workDir)) {
          page->setImageSize(pixels);
          if (pixels <= MAX_PIXBUF_SIZE) view->reloadModel();
        }
      */
      return;
    }
    
    static void
    downImage (GtkButton *self, void *data){
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto workDir = Child::getWorkdir(childWidget);

      auto pixels = Settings::getInteger("ImageSize", workDir);
      if (pixels  > 384)  pixels = 384;
      else  if (pixels  > 48 ) pixels = 48;
      else pixels = 0;
      Settings::setInteger("ImageSize", workDir, pixels);
      Print::print(output, g_strdup_printf("FIXME downImage:: pixels set at %d\n", pixels));
      /* FIXME
      page->setImageSize(pixels);
      view->reloadModel();
      */
      return ;
    }

    
  };
}

#endif

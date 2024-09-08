#ifndef XF_FMBUTTONBOX_HH
#define XF_FMBUTTONBOX_HH
#include "iconColorMenu.hh"
namespace xf {

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
           OPEN_TERMINAL, 
           GO_HOME, 
           SEARCH, 
           //OPEN_FILEMANAGER, 
           //EDIT_CLEAR,
           "media-view-subtitles",
           NULL
         };
        const char *bText[]={
          _("Open terminal"),
          _("Home"),
          _("Search"),
          // in main menu _("Open a New Window"),
          //_("Clear Log"),
          _("Show/hide grid."),
          NULL
        };
        void *bCallback[]={
          (void *)openTerminal,
          (void *)goHome,
          (void *)openFind,
          // (void *)openXffm,
          //(void *)clear,
          (void *)toggleVpane,
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
          auto button = Util::newButton(*p, *q);
          Util::boxPack0(vButtonBox_, GTK_WIDGET(button),  FALSE, FALSE, 0);
          if (*r) {
            g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK(*r), NULL);
            (r)++;
          }
        }

        auto colorButton = Util::newMenuButton(DOCUMENT_PROPERTIES, _("Color settings"));
        auto myColorMenu = new Menu<IconColorMenu>(_("Colors"));
        myColorMenu->setMenu(colorButton);
        delete myColorMenu;
        Util::boxPack0(vButtonBox_, GTK_WIDGET(colorButton),  FALSE, FALSE, 0);
      
        Util::boxPack0(vButtonBox_, GTK_WIDGET(scale),  FALSE, FALSE, 0);        
        Util::boxPack0(hbox, GTK_WIDGET(vButtonBox_),  FALSE, FALSE, 0);
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
        Texture::redlight();
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
          auto wd = Workdir::getWorkdir();
          if (g_file_test(wd, G_FILE_TEST_EXISTS)) {
            // race condition here, workdir null or garbage.
            // Workdir::reset();      
          }
        }
        Texture::greenlight();
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
        Util::setTooltip (GTK_WIDGET(size_scale),tooltipText);   
        g_signal_connect(G_OBJECT(size_scale), "value-changed", G_CALLBACK(changeSize), NULL);
        addMotionController(GTK_WIDGET(size_scale));
        return size_scale;
    }
    static void
    changeSize (GtkRange* self, gpointer user_data){
      if (Workdir::pleaseWait()) return;
      auto value = gtk_range_get_value(self);
      Settings::setInteger("xfterm", "iconsize", value);
      
      // reload all pages
      auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(MainWidget), "notebook"));
      auto n = gtk_notebook_get_n_pages(notebook);
      

      for (int i=0; i<n; i++){
        auto child = gtk_notebook_get_nth_page(notebook, i);
        auto path = Child::getWorkdir(child);
        Util::setWorkdir(path, child);
               
      }
    }
    
    static void
    goHome(GtkButton *self, void *data){
      //DBG("goHome....\n");
      auto child = Util::getChild();
      
      //Util::setWorkdir(g_get_home_dir());
        
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
      auto pathbar = GTK_BOX(g_object_get_data(G_OBJECT(output), "pathbar"));
      const char *v[]={"cd", g_get_home_dir(), NULL};
      auto retval = Util::cd((const gchar **)v, child);

      auto path = Child::getWorkdir(child);
      // FIXME UtilPathbar::updatePathbar(path, pathbar, true);
      if (retval){
        //Util::print(output, g_strdup_printf("%s\n", Child::getWorkdir(child)));
        if (!History::add("cd")) DBG("History::add(%s) failed\n", "cd" );
      } else {
        Util::print(output, g_strdup_printf(_("failed to chdir to $HOME")));
      }
      return;
    }
    static void
    openTerminal(GtkButton *self, void *data){
      auto childWidget =Util::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));
      auto workDir = Child::getWorkdir(childWidget);
        TRACE ("openTerminal::childWidget= %p, buttonSpace = %p workdir=%s\n", 
            childWidget, buttonSpace, workDir);

      auto terminal = Util::getTerminal();
      pid_t childPid = Run<bool>::shell_command(output, terminal, false, false);

      auto runButton = new (RunButton);
      runButton->init(runButton, terminal, childPid, output, workDir, buttonSpace);
      return;
    }
    static void
    openFind(GtkButton *self, void *data){
      auto childWidget =Util::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));
      auto workDir = Child::getWorkdir(childWidget);

      auto find = g_strdup_printf("xffm --find %s", workDir);
      pid_t childPid = Run<bool>::shell_command(output, find, false, false);

      auto runButton = new (RunButton);
      runButton->init(runButton, find, childPid, output, workDir, buttonSpace);
      g_free(find);
      return;
    }

    static void
    upImage (GtkButton *self, void *data){
      auto childWidget =Util::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto workDir = Child::getWorkdir(childWidget);

      auto pixels = Settings::getInteger("ImageSize", workDir);
      if (pixels < 48) pixels = 48;
      else if (pixels < 384) pixels = 384;
      else if (pixels < 768) pixels = 768;
      Settings::setInteger("ImageSize", workDir, pixels);
      Util::print(output, g_strdup_printf("FIXME upImage:: pixels set at %d\n", pixels));
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
      auto childWidget =Util::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto workDir = Child::getWorkdir(childWidget);

      auto pixels = Settings::getInteger("ImageSize", workDir);
      if (pixels  > 384)  pixels = 384;
      else  if (pixels  > 48 ) pixels = 48;
      else pixels = 0;
      Settings::setInteger("ImageSize", workDir, pixels);
      Util::print(output, g_strdup_printf("FIXME downImage:: pixels set at %d\n", pixels));
      /* FIXME
      page->setImageSize(pixels);
      view->reloadModel();
      */
      return ;
    }
    
    static void
    toggleVpane (GtkButton *self, void *data){
      auto vpane =Util::getPane();
      auto position = gtk_paned_get_position(vpane);
      int height = gtk_widget_get_height(GTK_WIDGET(vpane));
      TRACE("position=%d, height=%d, 3/4height=%d\n", position, height, height * 3 / 4);
      if (position < height * 3 / 4) gtk_paned_set_position(vpane, height);
      else gtk_paned_set_position(vpane, 0);
      return ;
    }

    
  };
}

#endif

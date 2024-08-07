#ifndef XF_FMBUTTONBOX_HH
#define XF_FMBUTTONBOX_HH
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
           SEARCH, 
           OPEN_FILEMANAGER, 
           GLIST_ADD, 
           GLIST_REMOVE, 
           EDIT_CLEAR,
           "media-view-subtitles",
           NULL
         };
        const char *bText[]={
          _("Open terminal"),
          _("Search"),
          _("Open a New Window"),
          _("Reset image size"),
          _("Reset image size"), 
          _("Clear Log"),
          _("Show/hide grid."),
          NULL
        };
        void *bCallback[]={
          (void *)openTerminal,
          (void *)openFind,
          (void *)openXffm,
          (void *)upImage,
          (void *)downImage,
          (void *)clear,
          (void *)toggleVpane,
          NULL
        };
        auto scale = Util::newSizeScale(_("Terminal font"));

        auto hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), FALSE);
        vButtonBox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        
        gtk_widget_add_css_class (GTK_WIDGET(vButtonBox_), "vbox" );


        gtk_widget_set_hexpand(GTK_WIDGET(vButtonBox_), TRUE);

        /*const char *bIcon[]={OPEN_FILEMANAGER, GO_HOME, DRIVE_HARDDISK, TRASH_ICON, NULL};
        const char *bText[]={_("Open a New Window"),_("Home Directory"),_("Disk Image Mounter"),_("Trash bin"),_ NULL};*/

        auto q = bText;
        auto r = bCallback;
        for (auto p=bIcon; p && *p; p++, q++){
          auto button = Util::newButton(*p, *q);
          Util::boxPack0(vButtonBox_, GTK_WIDGET(button),  FALSE, FALSE, 0);
          if (*r) {
            g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK(*r), NULL);
            (r)++;
          }
        }

        Util::boxPack0(vButtonBox_, GTK_WIDGET(scale),  FALSE, FALSE, 0);        
        Util::boxPack0(hbox, GTK_WIDGET(vButtonBox_),  FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(MainWidget), "buttonBox", vButtonBox_);

        return hbox;
    }
private:

    static void
    openTerminal(GtkButton *self, void *data){
      auto childWidget =Util::getCurrentChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));
      auto workDir = Util::getWorkdir();
        DBG ("openTerminal::childWidget= %p, buttonSpace = %p workdir=%s\n", 
            childWidget, buttonSpace, workDir);

      auto terminal = Util::getTerminal();
      pid_t childPid = Run::shell_command(output, terminal, false, false);

      auto runButton = new (RunButton);
      runButton->init(runButton, terminal, childPid, output, workDir, buttonSpace);
      return;
    }
    static void
    openFind(GtkButton *self, void *data){
      auto childWidget =Util::getCurrentChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));
      auto workDir = Util::getWorkdir();

      auto find = g_strdup_printf("xffm --find %s", workDir);
      pid_t childPid = Run::shell_command(output, find, false, false);

      auto runButton = new (RunButton);
      runButton->init(runButton, find, childPid, output, workDir, buttonSpace);
      g_free(find);
      return;
    }
    static void
    openXffm(GtkButton *self, void *data){
      auto childWidget =Util::getCurrentChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto buttonSpace = GTK_BOX(g_object_get_data(G_OBJECT(childWidget), "buttonSpace"));
      auto workDir = Util::getWorkdir();

      auto xffm = g_strdup_printf("xffm -f %s", workDir);
      pid_t childPid = Run::shell_command(output, xffm, false, false);

      auto runButton = new (RunButton);
      runButton->init(runButton, xffm, childPid, output, workDir, buttonSpace);
      g_free(xffm);
      return;
    }

    static void
    upImage (GtkButton *self, void *data){
      auto childWidget =Util::getCurrentChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto workDir = Util::getWorkdir();

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
      auto childWidget =Util::getCurrentChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      auto workDir = Util::getWorkdir();

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
    clear (GtkButton *self, void *data){
      auto childWidget =Util::getCurrentChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      Util::clear_text(output);
      return ;
    }
    static void
    toggleVpane (GtkButton *self, void *data){
      auto vpane =Util::getCurrentPane();
      auto pos = gtk_paned_get_position(vpane);
      if (pos > 1) gtk_paned_set_position(vpane, 0);
      else  gtk_paned_set_position(vpane, 2000);
      return ;
    }

    
  };
}

#endif

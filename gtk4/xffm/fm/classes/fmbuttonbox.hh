#ifndef XF_FMBUTTONBOX_HH
#define XF_FMBUTTONBOX_HH
//#define BUTTON_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,GdkDragContext *,gpointer)) X)
namespace xf {
  template <class Type> class MenuCallbacks;

  class FMbuttonBox{
    private:
      GtkBox *vButtonBox_;
    public:
    GtkBox *mkVbuttonBox(GtkWindow *mainWindow){
        auto myColorMenu = new Menu<IconColorMenu<LocalDir> >(_("Colors"));
        auto colorMenu =  myColorMenu->mkMenu(_("Colors"));
        delete myColorMenu;
        gtk_popover_set_has_arrow(colorMenu, true);

         const char *bIcon[]={
           EMBLEM_TOGGLE,
           EMBLEM_FIND, 
           EMBLEM_TTY, //EMBLEM_KEYBOARD, //EMBLEM_TTY, 
           GO_HOME, 
           EMBLEM_RUN, 
           NULL
         };

        const char *bText[]={
          _("Switch button"),
          _("Search"),
          _("Open terminal"),
          _("Home"),
          _("Open a New Window"),
          NULL
        };
        void *bCallback[]={
          (void *)MenuCallbacks<LocalDir>::moveWS,
          (void *)MenuCallbacks<LocalDir>::openFind,
          (void *)MenuCallbacks<LocalDir>::openTerminal,
          (void *)MenuCallbacks<LocalDir>::goHome,
          (void *)MenuCallbacks<LocalDir>::newWindow,
          NULL
        };
        void *bdata[]={
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL
        };
        
        //auto scale = newSizeScale(_("Icon Size"));
        auto scale2 = newSizeScale2(_("Icon Size"));
        auto hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), FALSE);
        vButtonBox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        
        gtk_widget_add_css_class (GTK_WIDGET(vButtonBox_), "vbox" );


        gtk_widget_set_hexpand(GTK_WIDGET(vButtonBox_), TRUE);
        auto q = bText;
        auto r = bCallback;
        auto d = bdata;
        for (auto p=bIcon; p && *p; p++, q++, r++, d++){
          auto button = UtilBasic::imageButton(40,32,*p, *q, *r, *d);
          Basic::boxPack0(vButtonBox_, GTK_WIDGET(button),  FALSE, FALSE, 0);
        }

        auto colorButton = UtilBasic::imageButton(40,32,EMBLEM_COLOR, 
            _("Color settings"), (void *)colorPopover, (void *)colorMenu, false);
//            _("Color settings"), NULL, NULL);
        gtk_popover_set_default_widget(colorMenu, GTK_WIDGET(colorButton));
        gtk_widget_set_parent(GTK_WIDGET(colorMenu),GTK_WIDGET(colorButton));
            
        Basic::boxPack0(vButtonBox_, GTK_WIDGET(colorButton),  FALSE, FALSE, 0);

        //Basic::boxPack0(vButtonBox_, GTK_WIDGET(scale),  FALSE, FALSE, 0);        
        Basic::boxPack0(vButtonBox_, GTK_WIDGET(scale2),  FALSE, FALSE, 0);        
        Basic::boxPack0(hbox, GTK_WIDGET(vButtonBox_),  FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(mainWindow), "buttonBox", vButtonBox_);
        return hbox;
    }
private:

    static void colorPopover(GtkWidget *self, gint n_press, gdouble x, gdouble y, GtkPopover *menu){
      gtk_popover_popup(menu);
      
    }

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
          auto wd = Workdir<LocalDir>::getWorkdir(NULL);
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

    GtkScale *newSizeScale2(const gchar *tooltipText){
        double value = 24.0;
        //auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 24.0, 384.0, 4.0));
        auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 24.0, 768.0, 4.0));
        // Load saved value fron xffm4/settings.ini file (if any)
        auto size = Settings::getInteger("xfterm", "iconsize", 48);
        TRACE("range set value=%lf\n", value);
        gtk_range_set_value(GTK_RANGE(size_scale), value);

        gtk_range_set_increments (GTK_RANGE(size_scale), 4.0, 4.0);
//        gtk_range_set_increments (GTK_RANGE(size_scale), 12.0, 12.0);
        gtk_widget_set_size_request (GTK_WIDGET(size_scale),-1,75);

        gtk_scale_set_value_pos (size_scale,GTK_POS_BOTTOM);
        //gtk_adjustment_set_upper (gtk_range_get_adjustment(GTK_RANGE(size_scale)), 24.0);
        Basic::setTooltip (GTK_WIDGET(size_scale),tooltipText);   
        g_signal_connect(G_OBJECT(size_scale), "value-changed", G_CALLBACK(changeSize2), NULL);
        addMotionController(GTK_WIDGET(size_scale));
        return size_scale;
    }
 

    static void
    changeSize2 (GtkRange* self, gpointer user_data){
      auto gridView_p = (GridView<LocalDir> *)Child::getGridviewObject(NULL);
      auto flags = gridView_p->flags();
      if (Workdir<LocalDir>::pleaseWait()) return;
      auto value = gtk_range_get_value(self);
      int valueI = value;
      if (valueI == 48) {
        gridView_p->flagOff(0x200); // Icon size off
      } else {
        gridView_p->flagOn(0x200); // Icon size on
      }
      auto child = Child::getChild();
      auto path = Workdir<LocalDir>::getWorkdir(child);

      if (gridView_p->flags() == 0x40 || gridView_p->flags() == 0x0) {
        Settings::removeGroup(path);
      } else if (path && g_file_test(path, G_FILE_TEST_IS_DIR)) {
        Settings::setInteger(path, "flags", gridView_p->flags());
        Settings::setInteger(path, "iconsize", valueI);
      }
      Workdir<LocalDir>::setWorkdir(path, child); // reload.
      Print::printInfo(Child::getOutput(NULL), 
          g_strdup_printf("%s %.0lf", _("Icon Size:"), value ));
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

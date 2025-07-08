#ifndef MENU_HH
#define MENU_HH

//#define ICONHASH mHash[0];
//#define CALLBACKHASH mHash[1];
//#define DATAHASH mHash[2];
//#define checkboxHASH mHash[3];
//#define radioboxHASH mHash[4];

typedef struct MenuInfo_t {
  const char *key;
  void *data;
} MenuInfo_t;

namespace xf {
  typedef struct MenuInfo_t {
    const char *key;
    void *data;
  } MenuInfo_t;
    

  /* */
  template <class subMenuClass>
  class Menu {
    private:
      using clipboard_t = ClipBoard<LocalDir>;
      GHashTable *mHash[5];
      const char **keys_;
      MenuInfo_t *iconNames_;
      MenuInfo_t *callbacks_;
      MenuInfo_t *data_;
      const char **checkboxes_;
      const char **radioboxes_;
      char *title_;
    public:
      const char **keys(void) { return keys_;}
      ~Menu(void){
        g_free(title_);
        g_hash_table_destroy(mHash[0]);
        g_hash_table_destroy(mHash[1]);
        g_hash_table_destroy(mHash[2]);
        g_hash_table_destroy(mHash[3]);
        g_hash_table_destroy(mHash[4]);
      }
      Menu(const char *title){
        TRACE("title=\"%s\"\n", title);
        title_ = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>",title);
        // Icon/callback/data hashes:
        mHash[0] = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
        mHash[1] = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        mHash[2] = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        mHash[3] = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        mHash[4] = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        auto myMenuClass_p = new subMenuClass;
        keys_ = myMenuClass_p->keys();
        iconNames_ = myMenuClass_p->iconNames();
        callbacks_ = myMenuClass_p->callbacks();
        data_ = myMenuClass_p->data();
        checkboxes_ = myMenuClass_p->checkboxes();
        radioboxes_ = myMenuClass_p->radioboxes();
        for (auto p=iconNames_; p && p->key; p++){
          auto iconName = (const char *)p->data;
          if (iconName) g_hash_table_insert(mHash[0], g_strdup(p->key), g_strdup(iconName));
        }
        for (auto p=callbacks_; p && p->key; p++){
          auto callback = p->data;
          TRACE("callback %p for %s\n", callback, p->key);
          if (callback)
            g_hash_table_insert(mHash[1], g_strdup(p->key), callback);
        }
        for (auto p=data_; p && p->key; p++){
          auto data = p->data;
          if (data) g_hash_table_insert(mHash[2], g_strdup(p->key), data);
        }
        for (auto p=checkboxes_; p && *p; p++){
          g_hash_table_insert(mHash[3], g_strdup(*p), GINT_TO_POINTER(1));
        }
        for (auto p=radioboxes_; p && *p; p++){
          g_hash_table_insert(mHash[4], g_strdup(*p), GINT_TO_POINTER(1));
        }
        delete myMenuClass_p;
      }

      static void openMenuButton(GtkWidget *self, void *data){
        auto menu = GTK_POPOVER(data);
        TRACE("openMenuButton....\n");
        gtk_popover_popup(menu);
      }

      GtkPopover *setMenu(GtkMenuButton *button){
        auto menu = mkMenu(title_);
        gtk_menu_button_set_popover (button, GTK_WIDGET(menu)); 
        g_object_set_data(G_OBJECT(button), "menu", menu);
       // g_signal_connect(G_OBJECT(button), "activate", G_CALLBACK(openMenuButton), (void *)menu);
        return menu;
      }

      /*GtkPopover *getMenu(GtkMenuButton *button){
        auto menu = mkMenu(title_);
        gtk_menu_button_set_popover (button, GTK_WIDGET(menu)); 
        return menu;
      }*/
      
      GtkPopover *setMenu(GtkWidget *widget, GtkWidget *parent, const char *path, bool isTextView){
        auto menu = mkMenu(title_);
        g_object_set_data(G_OBJECT(menu), "isTextView", GINT_TO_POINTER(isTextView));
        g_object_set_data(G_OBJECT(menu), "path", (void *)path);
        gtk_popover_set_default_widget(menu, widget);
        gtk_widget_set_parent(GTK_WIDGET(menu), parent);
        g_object_set_data(G_OBJECT(widget), "menu", (void *)menu);
        gtk_popover_set_position(GTK_POPOVER(menu), GTK_POS_RIGHT);
        
        auto gesture = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),3);
        g_signal_connect (G_OBJECT(gesture) , "pressed", G_CALLBACK (openMenu), (void *)menu);
        gtk_widget_add_controller(GTK_WIDGET(parent), GTK_EVENT_CONTROLLER(gesture));
        gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
            GTK_PHASE_CAPTURE);

        return menu;        
      }
      
      GtkPopover *setGridviewMenu(GtkWidget *widget, GtkWidget *parent, const char *path){
        auto menu = mkMenu(title_);
        g_object_set_data(G_OBJECT(menu), "path", (void *)path);
        gtk_popover_set_default_widget(menu, widget);
        gtk_widget_set_parent(GTK_WIDGET(menu), parent);
        g_object_set_data(G_OBJECT(widget), "menu", (void *)menu);
        gtk_popover_set_position(GTK_POPOVER(menu), GTK_POS_RIGHT);
        
        auto gesture = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),3);
        g_signal_connect (G_OBJECT(gesture) , "pressed", G_CALLBACK (openGridviewMenu), (void *)menu);
        gtk_widget_add_controller(GTK_WIDGET(parent), GTK_EVENT_CONTROLLER(gesture));
        gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
            GTK_PHASE_CAPTURE);

        return menu;        
      }

      GtkPopover *setMenu(GtkWidget *widget, GtkWidget *parent, const char *path){
        return setMenu(widget, parent, path, false);
      }

    private:
    static gboolean openGridviewMenu(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
      auto menu = GTK_POPOVER(data);       
      
      // position is relative to the parent/default widget.
      TRACE("menu=%p, position %lf,%lf\n", menu, x, y);
      gtk_popover_popup(menu);
      return TRUE;
    }
 
    static gboolean openMenu(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
      auto menu = GTK_POPOVER(data);
      

     auto paste = g_object_get_data(G_OBJECT(menu), _("Paste"));
     auto isTextView = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(menu), "isTextView"));
     TRACE("paste is at button %p\n", paste);
     if (!isTextView && paste) {
       auto c = (clipboard_t *)g_object_get_data(G_OBJECT(Child::mainWidget()), "ClipBoard");
       gtk_widget_set_visible(GTK_WIDGET(paste), c->validClipBoard());
       auto show =  g_object_get_data(G_OBJECT(menu), _("Show Clipboard"));
       if (show) gtk_widget_set_visible(GTK_WIDGET(show), c->validClipBoard());
       auto empty = g_object_get_data(G_OBJECT(menu), _("Clipboard is empty."));
       if (empty) gtk_widget_set_visible(GTK_WIDGET(empty), !c->validClipBoard());
     }
     if (isTextView && paste) {
       gtk_widget_set_visible(GTK_WIDGET(paste), clipboard_t::clipBoardSize() > 0);
       auto empty = g_object_get_data(G_OBJECT(menu), _("Clipboard is empty."));
       if (empty) gtk_widget_set_visible(GTK_WIDGET(empty), clipboard_t::clipBoardSize() == 0);
     }
       
      
      // position is relative to the parent/default widget.
      //TRACE("position %lf,%lf\n", x, y);
      gtk_popover_popup(menu);
      return TRUE;
    }
 
    public:
    static void 
    setTitle(GtkPopover *menu, const char *title){
      auto label = GTK_LABEL(g_object_get_data(G_OBJECT(menu), "titleLabel"));
      auto markup = g_strconcat("<span color=\"blue\">", title, "</span>", NULL);
      gtk_label_set_markup(label, markup);
      g_free(markup);
    }
      
    private:
    static 
    void boxPack(  
        GtkBox* box,
        GtkWidget* child,
        gboolean expand,
        gboolean fill,
        guint padding)
    {
      GtkOrientation orientation = gtk_orientable_get_orientation(GTK_ORIENTABLE(box));
      gtk_box_append(box, child);
      gtk_widget_set_halign (GTK_WIDGET(child),fill?GTK_ALIGN_FILL: GTK_ALIGN_START);
      // other options: GTK_ALIGN_START, GTK_ALIGN_END, GTK_ALIGN_BASELINE
      if (orientation == GTK_ORIENTATION_HORIZONTAL){
        gtk_widget_set_hexpand(GTK_WIDGET(child), expand);
        gtk_widget_set_margin_start(GTK_WIDGET(child), padding);
        gtk_widget_set_margin_end(GTK_WIDGET(child), padding);
      } else if (orientation == GTK_ORIENTATION_VERTICAL){
        gtk_widget_set_vexpand(GTK_WIDGET(child), expand);
        gtk_widget_set_margin_top(GTK_WIDGET(child), padding);
        gtk_widget_set_margin_bottom(GTK_WIDGET(child), padding);
      } else {
        fprintf(stderr, "boxPack(): programming error. Exit(2)\n");
        exit(2);
      }
    }
public:    

    GtkPopover *mkMenu(const char *markup){
      auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_add_css_class (GTK_WIDGET(vbox), "inquireBox" );
        gtk_widget_set_hexpand(GTK_WIDGET(vbox), FALSE);
        gtk_widget_set_vexpand(GTK_WIDGET(vbox), FALSE);
      auto titleBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_add_css_class (GTK_WIDGET(titleBox), "inquireBox" );
      gtk_box_append (vbox, GTK_WIDGET(titleBox));

      auto titleLabel = GTK_LABEL(gtk_label_new(""));
      gtk_box_append (titleBox, GTK_WIDGET(titleLabel));
      if (markup) gtk_label_set_markup(titleLabel, markup);    

      GtkPopover *menu = GTK_POPOVER(gtk_popover_new ());
        gtk_widget_set_vexpand(GTK_WIDGET(vbox), FALSE);       
        gtk_widget_add_css_class (GTK_WIDGET(menu), "inquireBox" );
        
      g_object_set_data(G_OBJECT(menu), "titleBox", titleBox);
      g_object_set_data(G_OBJECT(menu), "titleLabel", titleLabel);
      g_object_set_data(G_OBJECT(menu), "vbox", vbox);

      gtk_popover_set_autohide(GTK_POPOVER(menu), TRUE);
//      gtk_popover_set_has_arrow(GTK_POPOVER(menu), TRUE);
      gtk_popover_set_has_arrow(GTK_POPOVER(menu), true);
      gtk_widget_add_css_class (GTK_WIDGET(menu), "inquire" );

      GtkCheckButton *firstRadio = NULL;

      for (const char **p=keys_; p && *p; p++){
        auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_add_css_class (GTK_WIDGET(hbox), "inquireBox" );
        auto label = GTK_LABEL(gtk_label_new(""));
        auto iconName = (const char *) g_hash_table_lookup(mHash[0], *p);
        //TRACE("icon is %s\n",icon);
#if 10
        if (iconName){
          auto image = Texture<bool>::getImage(iconName, 16);
//          auto image = gtk_image_new_from_icon_name(iconName);
          boxPack(hbox, GTK_WIDGET(image),  FALSE, FALSE, 0);
        }
#endif
        boxPack(hbox, GTK_WIDGET(label),  FALSE, FALSE, 5);
        gtk_label_set_markup(label, *p);

        /*if (GPOINTER_TO_INT(g_hash_table_lookup(mHash[2], *p)) == -1) {
          // if the callback data is equal to -1, then item is not an active button,
          // (submenu... experimental, YMMV).
          continue;
        } */

        auto callback = g_hash_table_lookup(mHash[1], *p);
        if (callback) {
          auto isCheck = g_hash_table_lookup(mHash[3], *p);
          auto isRadio = g_hash_table_lookup(mHash[4], *p);
          if (isCheck || isRadio){
              // a checkbutton
            GtkCheckButton *button = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(*p));
            if (isRadio){
              gtk_check_button_set_group(button, firstRadio);
              if (!firstRadio) firstRadio = button;
           }
            // gtk_widget_add_css_class (GTK_WIDGET(button), "inquireButton" );
            g_object_set_data(G_OBJECT(menu), *p, button);
            g_object_set_data(G_OBJECT(button), "menu", menu);
            g_object_set_data(G_OBJECT(button), "key", (void *)(*p));/// XXX no need to strdup!
            g_object_set_data(G_OBJECT(menu), *p, button);
            TRACE("data set %p %s -->button %p\n", menu, *p, button);

            boxPack(vbox, GTK_WIDGET(button),  FALSE, FALSE, 0);
            gtk_widget_set_visible(GTK_WIDGET(button), TRUE);
          
            auto data = g_hash_table_lookup(mHash[2], *p);
            g_signal_connect (G_OBJECT (button), "toggled", G_CALLBACK(callback), data);
            continue;

         } else { 
#if 0
            // imagebutton crash GTK_IS_POPOVER (popover)' failed
            auto data = g_hash_table_lookup(mHash[2], *p);
            GtkBox *button = UtilBasic::imageButtonText(iconName, *p, (void *)callback, data);
            g_object_set_data(G_OBJECT(menu), *p, button);
            g_object_set_data(G_OBJECT(button), "menu", menu);
            g_object_set_data(G_OBJECT(button), "key",(void *)(*p));/// XXX no need to strdup!
            g_object_set_data(G_OBJECT(menu), *p, button);
            boxPack(vbox, GTK_WIDGET(button),  FALSE, FALSE, 0);
            gtk_widget_set_visible(GTK_WIDGET(button), TRUE);
#else 
            // A button.
            GtkButton *button = GTK_BUTTON(gtk_button_new());
            gtk_widget_add_css_class (GTK_WIDGET(button), "inquireButton" );
            g_object_set_data(G_OBJECT(menu), *p, button);
            g_object_set_data(G_OBJECT(button), "menu", menu);
            g_object_set_data(G_OBJECT(button), "key",(void *)(*p));/// XXX no need to strdup!
            g_object_set_data(G_OBJECT(menu), *p, button);
            TRACE("data set %p %s -->button %p\n", menu, *p, button);

            gtk_button_set_child(GTK_BUTTON(button), GTK_WIDGET(hbox));
            boxPack(vbox, GTK_WIDGET(button),  FALSE, FALSE, 0);
            gtk_button_set_has_frame(GTK_BUTTON(button), FALSE);
            gtk_widget_set_visible(GTK_WIDGET(button), TRUE);
          
            auto data = g_hash_table_lookup(mHash[2], *p);
            g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK(callback), data);
#endif
            continue;
          }
        }
        // No callback: it is a label.
        char *textMarkup = NULL;
        if (strlen(*p)>=2) textMarkup = g_strdup_printf("<i><b>%s</b></i>", *p);
        else if (strlen(*p)==1) textMarkup = g_strdup_printf("<span color=\"gray\"><i>_________________________</i></span>");
        else textMarkup = g_strdup_printf("<span color=\"blue\"><i><b>_________________________</b></i></span>");
        gtk_label_set_markup(label, textMarkup);
        g_free(textMarkup);
        g_object_set_data(G_OBJECT(label), "menu", menu);  
        // FIXME leak: *p      
        g_object_set_data(G_OBJECT(label), "key", g_strdup(*p));
        g_object_set_data(G_OBJECT(menu), *p, hbox);
          TRACE("data set %p %s -->hbox %p\n", menu, *p, hbox);
        boxPack(vbox, GTK_WIDGET(hbox),  FALSE, FALSE, 0);
        gtk_widget_set_visible(GTK_WIDGET(label), TRUE);
        continue;
      }
      gtk_popover_set_child (menu, GTK_WIDGET(vbox));
      return menu;
    }
  };
}
#endif



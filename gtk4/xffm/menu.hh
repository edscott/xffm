#ifndef MENU_HH
#define MENU_HH

//#define ICONHASH mHash[0];
//#define CALLBACKHASH mHash[1];
//#define DATAHASH mHash[2];

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
  template <class menuClass>
  class Menu {
    private:
      GHashTable *mHash[3];
      const char **keys_;
      MenuInfo_t *iconNames_;
      MenuInfo_t *callbacks_;
      MenuInfo_t *data_;
      char *title_;
    public:
      ~Menu(void){
        g_free(title_);
      }
      Menu(const char *title){
        title_ = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>",title);
        // Icon/callback/data hashes:
        mHash[0] = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
        mHash[1] = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        mHash[2] = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        auto myMenuClass_p = new menuClass;
        keys_ = myMenuClass_p->keys();
        iconNames_ = myMenuClass_p->iconNames();
        callbacks_ = myMenuClass_p->callbacks();
        data_ = myMenuClass_p->data();
        for (auto p=iconNames_; p && p->key; p++){
          auto iconName = (const char *)p->data;
          if (iconName) g_hash_table_insert(mHash[0], g_strdup(p->key), g_strdup(iconName));
        }
        for (auto p=callbacks_; p && p->key; p++){
          auto callback = p->data;
          if (callback) g_hash_table_insert(mHash[1], g_strdup(p->key), callback);
        }
        for (auto p=data_; p && p->key; p++){
          auto data = p->data;
          if (data) g_hash_table_insert(mHash[2], g_strdup(p->key), data);
        }
        delete myMenuClass_p;
      }

      static void openMenuButton(GtkWidget *self, void *data){
        auto menu = GTK_POPOVER(data);
        gtk_popover_popup(menu);
      }

      void setMenu(GtkMenuButton *button){
        auto menu = mkMenu(title_);
        gtk_menu_button_set_popover (button, GTK_WIDGET(menu)); 
        g_object_set_data(G_OBJECT(button), "menu", menu);
       // g_signal_connect(G_OBJECT(button), "activate", G_CALLBACK(openMenuButton), (void *)menu);
        return;
      }

      /*GtkPopover *getMenu(GtkMenuButton *button){
        auto menu = mkMenu(title_);
        gtk_menu_button_set_popover (button, GTK_WIDGET(menu)); 
        return menu;
      }*/
      
      void setMenu(GtkWidget *widget, GtkWidget *parent, const char *path){
        auto menu = mkMenu(title_);
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

        return;        
      }

    private:
    static gboolean openMenu(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
      auto menu = GTK_POPOVER(data);

     auto paste = g_object_get_data(G_OBJECT(menu), _("Paste"));
     DBG("paste is at button %p\n", paste);
     if (paste) {
       auto c = (ClipBoard *)g_object_get_data(G_OBJECT(MainWidget), "ClipBoard");
       gtk_widget_set_visible(GTK_WIDGET(paste), c->validClipBoard());
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
    
    GtkPopover *mkMenu(const char *markup){
      auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
      auto titleBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_box_append (vbox, GTK_WIDGET(titleBox));

      auto titleLabel = GTK_LABEL(gtk_label_new(""));
      gtk_box_append (titleBox, GTK_WIDGET(titleLabel));
      if (markup) gtk_label_set_markup(titleLabel, markup);    

      GtkPopover *menu = GTK_POPOVER(gtk_popover_new ());
      g_object_set_data(G_OBJECT(menu), "titleBox", titleBox);
      g_object_set_data(G_OBJECT(menu), "titleLabel", titleLabel);
      g_object_set_data(G_OBJECT(menu), "vbox", vbox);

      gtk_popover_set_autohide(GTK_POPOVER(menu), TRUE);
      gtk_popover_set_has_arrow(GTK_POPOVER(menu), FALSE);
      gtk_widget_add_css_class (GTK_WIDGET(menu), "inquire" );


      for (const char **p=keys_; p && *p; p++){
        auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto label = GTK_LABEL(gtk_label_new(""));
        auto iconName = (const char *) g_hash_table_lookup(mHash[0], *p);
        //DBG("icon is %s\n",icon);
        if (iconName){
          auto image = gtk_image_new_from_icon_name(iconName);
          boxPack(hbox, GTK_WIDGET(image),  FALSE, FALSE, 0);
        }
        boxPack(hbox, GTK_WIDGET(label),  FALSE, FALSE, 5);


        /*if (GPOINTER_TO_INT(g_hash_table_lookup(mHash[2], *p)) == -1) {
          // if the callback data is equal to -1, then item is not an active button,
          // (submenu... experimental, YMMV).
          continue;
        } */

        auto callback = g_hash_table_lookup(mHash[1], *p);
        if (callback) {
          // A button.
          gtk_label_set_markup(label, *p);
          GtkButton *button = GTK_BUTTON(gtk_button_new());
          g_object_set_data(G_OBJECT(menu), *p, button);
          g_object_set_data(G_OBJECT(button), "menu", menu);
          g_object_set_data(G_OBJECT(button), "key", g_strdup(*p));
          g_object_set_data(G_OBJECT(menu), *p, button);

          gtk_button_set_child(GTK_BUTTON(button), GTK_WIDGET(hbox));
          boxPack(vbox, GTK_WIDGET(button),  FALSE, FALSE, 0);
          gtk_button_set_has_frame(GTK_BUTTON(button), FALSE);
          gtk_widget_set_visible(GTK_WIDGET(button), TRUE);
        
          auto data = g_hash_table_lookup(mHash[2], *p);
          g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK(callback), data);
          continue;
        }
        // Not a button.
        auto textMarkup = g_strdup_printf("<i>%s ------------</i>", *p);
        gtk_label_set_markup(label, textMarkup);
        g_free(textMarkup);
        g_object_set_data(G_OBJECT(label), "menu", menu);
        g_object_set_data(G_OBJECT(label), "key", g_strdup(*p));
        g_object_set_data(G_OBJECT(menu), *p, label);
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



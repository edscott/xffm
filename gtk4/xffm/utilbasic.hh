#ifndef UTILBASIC_HH
#define UTILBASIC_HH

namespace xf {

  class UtilBasic: 
    public Child,
    public Clipboard
  {
    public:
    static void
    concat(gchar **fullString, const gchar* addOn){
        if (!(*fullString)) {
          *fullString = g_strdup(addOn);
        }
        auto newString = g_strconcat(*fullString, addOn, NULL);
        g_free(*fullString);
        *fullString = newString;
    }
    static void
    flushGTK(void){
      while (g_main_context_pending(NULL))
        g_main_context_iteration(NULL, TRUE);
    }
  static 
  void boxPack0(  
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
      fprintf(stderr, "boxPack0(): programming error. Exit(2)\n");
      exit(2);
    }
  }

    static GList *getChildren(GtkBox *box){
      GList *list = NULL;
      GtkWidget *w = gtk_widget_get_first_child(GTK_WIDGET(box));
      if (w) {
        list = g_list_append(list, w);
        while ((w=gtk_widget_get_next_sibling(w)) != NULL) list = g_list_append(list, w);
      }
      return list;
    }


  static void *context_function(void * (*function)(gpointer), void * function_data){
      pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
      pthread_cond_t signal = PTHREAD_COND_INITIALIZER; 
      auto result=GINT_TO_POINTER(-1);
      void *arg[] = {
          (void *)function,
          (void *)function_data,
          (void *)&mutex,
          (void *)&signal,
          (void *)&result
      };
      gboolean owner = g_main_context_is_owner(g_main_context_default());
      if (owner){
          context_function_f(arg);
      } else {
          g_main_context_invoke(NULL, CONTEXT_CALLBACK(context_function_f), arg);
          pthread_mutex_lock(&mutex);
          if (result == GINT_TO_POINTER(-1)) pthread_cond_wait(&signal, &mutex);
          pthread_mutex_unlock(&mutex);
      }
      pthread_mutex_destroy(&mutex);
      pthread_cond_destroy(&signal);
      return result;
  }
  static gboolean
  context_function_f(gpointer data){
      void **arg = (void **)data;
      void * (*function)(gpointer) = (void* (*)(void*))arg[0];
      gpointer function_data = arg[1];
      pthread_mutex_t *mutex = (pthread_mutex_t *)arg[2];
      pthread_cond_t *signal = (pthread_cond_t *)arg[3];
      auto result_p = (void **)arg[4];
      void *result = (*function)(function_data);
      pthread_mutex_lock(mutex);
      *result_p = result;
      pthread_cond_signal(signal);
      pthread_mutex_unlock(mutex);
      return FALSE;
  }
    private:


 
    public:
    static GtkBox *
    pathbarLabelButton (const char *text) {
        auto label = GTK_LABEL(gtk_label_new(""));
        auto eventBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        if (text) {
            auto v = UtilBasic::utf_string(text);
            auto g = g_markup_escape_text(v, -1);
            g_free(v);
            auto markup = g_strdup_printf("   <span size=\"small\">  %s  </span>   ", g);
            g_free(g);
            gtk_label_set_markup(label, markup);
            g_free(markup);
        } else {
            gtk_label_set_markup(label, "");
        }
        UtilBasic::boxPack0 (eventBox, GTK_WIDGET(label), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(eventBox), "label", label);
        g_object_set_data(G_OBJECT(eventBox), "name", text?g_strdup(text):g_strdup("RFM_ROOT"));
        return eventBox;
    }
 
    static void 
    setMenuTitle(GtkPopover *menu, const char *title){
     if (title) {      
        auto titleBox = GTK_BOX(g_object_get_data(G_OBJECT(menu), "titleBox"));
        auto titleLabel = GTK_LABEL(g_object_get_data(G_OBJECT(menu), "titleLabel"));
        auto markup = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", title);
        gtk_label_set_markup(titleLabel, markup);
        g_free(markup);
      }

    }
    static GtkPopover *mkMenu(const char **text, GHashTable **mHash, const gchar *title){
      auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
      auto titleBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_box_append (vbox, GTK_WIDGET(titleBox));

      auto titleLabel = GTK_LABEL(gtk_label_new(""));

      gtk_box_append (titleBox, GTK_WIDGET(titleLabel));
      
      

      GtkPopover *menu = GTK_POPOVER(gtk_popover_new ());
      g_object_set_data(G_OBJECT(menu), "titleBox", titleBox);
      g_object_set_data(G_OBJECT(menu), "titleLabel", titleLabel);
      g_object_set_data(G_OBJECT(menu), "vbox", vbox);
      setMenuTitle(menu, title);

      gtk_popover_set_autohide(GTK_POPOVER(menu), TRUE);
      gtk_popover_set_has_arrow(GTK_POPOVER(menu), FALSE);
      gtk_widget_add_css_class (GTK_WIDGET(menu), "inquire" );


      for (const char **p=text; p && *p; p++){
        auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto label = GTK_LABEL(gtk_label_new(""));
        gtk_label_set_markup(label, *p);
        auto icon = (const char *) g_hash_table_lookup(mHash[0], *p);
        //DBG("icon is %s\n",icon);
        if (icon){
          auto image = gtk_image_new_from_icon_name(icon);
          boxPack0(hbox, GTK_WIDGET(image),  FALSE, FALSE, 0);
        }
        boxPack0(hbox, GTK_WIDGET(label),  FALSE, FALSE, 5);

        if (GPOINTER_TO_INT(g_hash_table_lookup(mHash[2], *p)) == -1) {
          boxPack0(vbox, GTK_WIDGET(hbox),  FALSE, FALSE, 0);
          g_object_set_data(G_OBJECT(menu), *p, hbox);
          gtk_widget_set_visible(GTK_WIDGET(hbox), TRUE);
          continue;
        } else {
          GtkButton *button = GTK_BUTTON(gtk_button_new());
          g_object_set_data(G_OBJECT(button), "menu", menu);
          gtk_button_set_child(GTK_BUTTON(button), GTK_WIDGET(hbox));
          boxPack0(vbox, GTK_WIDGET(button),  FALSE, FALSE, 0);
          g_object_set_data(G_OBJECT(menu), *p, button);
          gtk_button_set_has_frame(GTK_BUTTON(button), FALSE);
          gtk_widget_set_visible(GTK_WIDGET(button), TRUE);
        
          auto callback = g_hash_table_lookup(mHash[1], *p);
          TRACE("mkMenu() callback=%p, icon=%s\n", callback, icon);
          if (callback) {
            auto data = g_hash_table_lookup(mHash[2], *p);
            g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK(callback), data);
          }
          g_object_set_data(G_OBJECT(button), "menu", menu);
          g_object_set_data(G_OBJECT(menu), *p, button);
          continue;
        }
      }
          

      gtk_popover_set_child (menu, GTK_WIDGET(vbox));
      return menu;
    }
   
  };
}
#endif


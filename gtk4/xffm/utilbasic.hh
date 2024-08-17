#ifndef UTILBASIC_HH
#define UTILBASIC_HH

namespace xf {
  class UtilBasic{
    public:
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
  static gchar *
  utf_string (const gchar * t) {
      if(!t) { return g_strdup (""); }
      if(g_utf8_validate (t, -1, NULL)) { return g_strdup (t); }
      /* so we got a non-UTF-8 */
      /* but is it a valid locale string? */
      gchar *actual_tag;
      actual_tag = g_locale_to_utf8 (t, -1, NULL, NULL, NULL);
      if(actual_tag)
          return actual_tag;
      /* So it is not even a valid locale string... 
       * Let us get valid utf-8 caracters then: */
      const gchar *p;
      actual_tag = g_strdup ("");
      for(p = t; *p; p++) {
          // short circuit end of string:
          gchar *r = g_locale_to_utf8 (p, -1, NULL, NULL, NULL);
          if(g_utf8_validate (p, -1, NULL)) {
              gchar *qq = g_strconcat (actual_tag, p, NULL);
              g_free (actual_tag);
              actual_tag = qq;
              break;
          } else if(r) {
              gchar *qq = g_strconcat (actual_tag, r, NULL);
              g_free (r);
              g_free (actual_tag);
              actual_tag = qq;
              break;
          }
          // convert caracter to utf-8 valid.
          gunichar gu = g_utf8_get_char_validated (p, 2);
          if(gu == (gunichar) - 1) {
              gu = g_utf8_get_char_validated ("?", -1);
          }
          gchar outbuf[8];
          memset (outbuf, 0, 8);
          gint outbuf_len = g_unichar_to_utf8 (gu, outbuf);
          if(outbuf_len < 0) {
              ERROR ("utf_string: unichar=%d char =%c outbuf_len=%d\n", gu, p[0], outbuf_len);
          }
          gchar *qq = g_strconcat (actual_tag, outbuf, NULL);
          g_free (actual_tag);
          actual_tag = qq;
      }
      return actual_tag;
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
    static
    void setWindowTitle(GtkWidget *child){
        auto path = (const char *) g_object_get_data(G_OBJECT(child), "path");
        gchar *gg = get_terminal_name(path);
        auto user = g_get_user_name();
        auto host = g_strdup(g_get_host_name());
        if (strchr(host, '.')) *strchr(host, '.')=0;
        gchar *g = g_strconcat(user,"@",host,":",gg, NULL);
        g_free(host);
        g_free(gg); 
        gtk_window_set_title(GTK_WINDOW(MainWidget), g);
        g_free(g);
        auto basename = g_path_get_basename(path);
        auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(MainWidget), "notebook"));
        auto tabWidget = gtk_notebook_get_tab_label(notebook, child);
        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(tabWidget), "label"));
        gtk_label_set_markup(label, basename);
        g_free(basename);
    }
    private:

    static     gchar *
    get_terminal_name (const char *path) {
        gchar *iconname;
        if(!path) {
            iconname = utf_string (g_get_host_name());
        } else if(g_path_is_absolute(path) &&
                g_file_test (path, G_FILE_TEST_EXISTS)) {
            gchar *basename = g_path_get_basename (path);
            gchar *pathname = g_strdup (path);
            gchar *b = utf_string (basename);   // non chopped
            chop_excess (pathname);
            gchar *q = utf_string (pathname);   // non chopped

            g_free (basename);
            g_free (pathname);
            //iconname = g_strconcat (display_host, ":  ", b, " (", q, ")", NULL);
            iconname = g_strconcat (b, " (", q, ")", NULL);
            g_free (q);
            g_free (b);
        } else {
            iconname = utf_string (path);
            chop_excess (iconname);
        }

        return (iconname);
    }

#define MAX_PATH_LABEL 40
#define MAX_PATH_START_LABEL 18
    static const gchar *
    chop_excess (gchar * b) {
        // chop excess length...

        const gchar *home = g_get_home_dir();
        gchar *bb;
        if (strncmp(home, b, strlen(home))==0){
            if (strlen(home) == strlen(b)) return b;
            bb = g_strconcat("~/", b + strlen(home)+1, NULL);
        } else {
            bb = g_strdup(b);
        }
        
        int len = strlen (bb);

        if(len < MAX_PATH_LABEL) {
            strcpy(b, bb);
            g_free(bb);
            return (b);
        }
            
        bb[MAX_PATH_START_LABEL - 3] = 0;

        gchar *g = g_strconcat(bb, "...", b + (len - MAX_PATH_LABEL + MAX_PATH_START_LABEL), NULL);
        strcpy (b, g);
        g_free(bb);
        g_free(g);

        return b;
    }
 
    public:
 
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
      setMenuTitle(menu, title);

      gtk_popover_set_autohide(GTK_POPOVER(menu), TRUE);
      gtk_popover_set_has_arrow(GTK_POPOVER(menu), FALSE);
      gtk_widget_add_css_class (GTK_WIDGET(menu), "inquire" );


      for (const char **p=text; p && *p; p++){
        auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto label = GTK_LABEL(gtk_label_new(""));
        gtk_label_set_markup(label, *p);
        auto icon = (const char *) g_hash_table_lookup(mHash[0], *p);
        DBG("icon is %s\n",icon);
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


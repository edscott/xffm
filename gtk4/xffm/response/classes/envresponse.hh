#ifndef TXTRESPONSE_HH
#define TXTRESPONSE_HH


namespace xf {
class TxtDialog {
   const char *title_;
   const char *iconName_;
   const char *label_;
   GtkBox *mainBox_ = NULL;
   GtkWindow *dialog_ = NULL;
public:
    const char *title(void){ return _("Environment variables");}
    const char *iconName(void){ return EMBLEM_INFO;}
    const char *label(void){ return _("");}
    GtkWindow *dialog(void){return dialog_;}
    void dialog(GtkWindow *value){dialog_ = value;}

    GtkBox *mainBox(const char *markup){
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), true);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), true);
        auto label = gtk_label_new("");
        gtk_label_set_markup(GTK_LABEL(label), markup);

        auto sw = gtk_scrolled_window_new();
        auto topBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), topBox); 
        gtk_box_append(mainBox_, sw);
        gtk_widget_set_size_request(topBox, 400, 400);
        gtk_widget_set_size_request(sw, 400, 400);


        addBox(GTK_BOX(topBox), "LD_LIBRARY_PATH");


        for (auto p=environment; p && *p; p++){
          if (strstr(*p, "LD_LIBRARY_PATH")) continue;
          auto v = g_strsplit(*p, "=", 2);
          if (!v) continue;
          auto box =  GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_box_append(GTK_BOX(topBox), GTK_WIDGET(box));
          auto label1 = gtk_label_new(v[0]);
          gtk_box_append(box, label1);
          auto entry = gtk_entry_new();
          gtk_widget_set_hexpand(GTK_WIDGET(entry), true);
          auto buffer =gtk_entry_get_buffer(GTK_ENTRY(entry));
          TRACE("-- %s = \"%s\"\n", v[0], v[1]);
          gtk_entry_buffer_set_text(buffer, v[1], -1);
          gtk_box_append(box, entry);
          g_strfreev(v);
        }
        return mainBox_;
    }
    void addBox(GtkBox *topBox, const char *envp){
      auto value = getenv(envp);
      auto box =  GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_box_append(topBox, GTK_WIDGET(box));
      auto label0 = gtk_label_new(envp);
      gtk_box_append(box, label0);
      auto entry = gtk_entry_new();
      gtk_widget_set_hexpand(GTK_WIDGET(entry), true);
      if (value){
          auto v=g_strsplit(value, "=", 2);
          auto buffer =gtk_entry_get_buffer(GTK_ENTRY(entry));
          gtk_entry_buffer_set_text(buffer, v[1], -1);
          g_strfreev(v);
      }
      gtk_box_append(box, entry);
      return;
    }

    static void *asyncYes(void *data){
      return asyncNo(data);
    }

    static void *asyncNo(void *data){
      // Cancel
      // send interrupt signal to parent
      pid_t parent = getppid();
      kill(parent, SIGINT);
      return NULL;
    }
};
}


#endif

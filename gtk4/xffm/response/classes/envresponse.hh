#ifndef TXTRESPONSE_HH
#define TXTRESPONSE_HH


namespace xf {
class EnvDialog {
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

      static gboolean
      frameLabelClick(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              void *data){
        //FindSignals<Type>::onFindButton(NULL, data);
        return true;
      }

    GtkBox *mainBox(const char *markup){
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), true);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), true);
        auto label = gtk_label_new("");
        gtk_label_set_markup(GTK_LABEL(label), markup);
       
        auto frame = gtk_frame_new("");

        auto frameWidget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        auto frameLabel = Basic::hyperLabelLarge("blue",_("Apply changes"),
              (void *)frameLabelClick,
              (void *)this);
        gtk_box_append(GTK_BOX(frameWidget), GTK_WIDGET(frameLabel));
        //auto frameLabel = gtk_label_new("foo");
        //gtk_box_append(GTK_BOX(frameWidget), frameLabel);
        
        gtk_frame_set_label_widget(GTK_FRAME(frame),frameWidget);
        gtk_frame_set_label_align(GTK_FRAME(frame), 1.0);

        gtk_box_append(mainBox_, frame);
        auto frameBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_frame_set_child(GTK_FRAME(frame), GTK_WIDGET(frameBox));


        auto sw = gtk_scrolled_window_new();
        auto topBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), topBox); 
        gtk_box_append(GTK_BOX(frameBox), sw);
        gtk_widget_set_size_request(topBox, 400, 400);
        gtk_widget_set_size_request(sw, 400, 400);



        addBox(GTK_BOX(topBox), "LD_LIBRARY_PATH");


        for (auto p=environment; p && *p; p++){
          // FIXME: this should be in the addBox routine, somehow...
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
      DBG("hello world\n");
      return asyncNo(data);
    }

    static void *asyncNo(void *data){
      DBG("goodbye world\n");
      return NULL;
    }
};
}


#endif

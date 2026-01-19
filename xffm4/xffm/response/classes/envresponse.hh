#ifndef TXTRESPONSE_HH
#define TXTRESPONSE_HH
typedef struct envStruct_t {
  GtkEntry *entry;
  char *variable;
  char *value;
}envStruct_t;

namespace xf {
class EnvDialog {
   const char *title_;
   const char *iconName_;
   const char *label_;
   GtkLabel *noChanges_ = NULL;
   GtkLabel *frameLabel_ = NULL;
   GtkBox *mainBox_ = NULL;
   GtkWindow *dialog_ = NULL;
   GList *envList_ = NULL;

   GList *basicEnvList(void){
     GList *list = NULL;
     for (auto p=basicEnv(); p && *p; p++){
       auto data = (envStruct_t *)calloc(1,sizeof(envStruct_t));
       data->entry = GTK_ENTRY(gtk_entry_new());
       data->variable = g_strdup(*p);
       // First check in environment.
       auto value = getenv(*p);
       TRACE("getenv(%s) = %s\n", *p, value);
       if (value && strlen(value)){        
          data->value = g_strdup(value);
       } else {
          data->value = g_strdup("");
       }
       list = g_list_append(list, data);
     }
     return list;
   }

   GList *configEnvList(void){
     GList *list = envList_;
     // Now override with xffm config values, if any.
     auto keyfile = Settings::getKeyFile();
     // Get keys.
     auto v = g_key_file_get_keys (keyfile, "ENVIRONMENT", NULL, NULL);
     if (v != NULL){
       // For each key.
       for (auto p=v; p && *p; p++){
         // Get value.
         auto value = g_key_file_get_string(keyfile, "ENVIRONMENT", *p, NULL);
         if (value && strlen(value)) {
           // Find in list.
           bool found = false;
           for (auto l=list; l && l->data; l=l->next){
             auto data = (envStruct_t *)l->data;
             if (strcmp(data->variable, *p) == 0){
               // Found in list: modify.
               g_free(data->value);
               data->value = value;
               found = true;
               break;
             }
           }
           if (!found){
             // Not found in list: add to list.
             auto data = (envStruct_t *)calloc(1,sizeof(envStruct_t));
             data->entry = GTK_ENTRY(gtk_entry_new());
             data->variable = g_strdup(*p);
             data->value = value;
             list = g_list_append(list, data);
           }
         }
       }
       g_strfreev(v);
     }
     g_key_file_free(keyfile);
     return list;
   }
public:
    static const char **basicEnv(void){
      static const char *basicEnv_[10]={
         "PKG_CONFIG_PATH",
         "LD_LIBRARY_PATH",
         "EDITOR",
         "TERMINAL",
         "TERMINAL_CMD",
         "SUDO_ASKPASS",
         "SSH_ASKPASS",
         "SSH_ASKPASS_REQUIRE",
         "LC_ALL",
         NULL
       };
      return (const char **)basicEnv_;
    }
    const char *title(void){ return _("Environment variables");}
    const char *iconName(void){ return EMBLEM_INFO;}
    const char *label(void){ return "";}
    GtkWindow *dialog(void){return dialog_;}
    void dialog(GtkWindow *value){dialog_ = value;}
    GtkWidget *noChanges(void){return GTK_WIDGET(noChanges_);}
    GtkWidget *frameLabel(void){return GTK_WIDGET(frameLabel_);}
    GList *envList(void){ return envList_;}


    ~EnvDialog(void){
      for (auto l=envList_; l && l->data; l=l->next){
         auto data = (envStruct_t *)l->data;
         g_free(data->variable);
         g_free(data->value);
         g_free(data);
      }
      g_list_free(envList_);
    }
      
    GtkBox *mainBox(const char *markup){
        envList_ = basicEnvList();
        envList_ = configEnvList();
        
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), true);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), true);
        auto label = gtk_label_new("");
        gtk_label_set_markup(GTK_LABEL(label), markup);
       
        auto frame = gtk_frame_new("");
        auto frameWidget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        noChanges_ = GTK_LABEL(gtk_label_new(_("No changes need to be saved")));
        frameLabel_ = Basic::hyperLabelLarge("blue",_("Apply changes"),
              (void *)frameLabelClick,
              (void *)this);
        gtk_box_append(GTK_BOX(frameWidget), GTK_WIDGET(noChanges_));
        gtk_box_append(GTK_BOX(frameWidget), GTK_WIDGET(frameLabel_));
        gtk_widget_set_visible(GTK_WIDGET(frameLabel_),false);
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

        for (auto l=envList_; l && l->data; l= l->next){
          addBox(GTK_BOX(topBox), (envStruct_t *)l->data);
        }

//        addBox(GTK_BOX(topBox), "LD_LIBRARY_PATH");
/*
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
*/
        return mainBox_;
    }

private:
    static bool itemChanged(envStruct_t *data){
      auto buffer = gtk_entry_get_buffer(data->entry);
      auto txt = gtk_entry_buffer_get_text(buffer);
      TRACE("txt=%s value=%s\n", txt, data->value);
      if (txt && !data->value){return true;}
      if (!txt && data->value){return true;}
      if (txt && data->value && strcmp(txt, data->value)) {
        return true;
      }
      return false;
    }

    static bool isChanged(EnvDialog *object){
      for (auto l=object->envList(); l && l->data; l=l->next){
        auto data = (envStruct_t *)l->data;
        if (itemChanged(data)) return true;
      }
      return false;
    }

    static gboolean
    frameLabelClick(GtkGestureClick* self,
            gint n_press,
            gdouble x,
            gdouble y,
            void *data){
      auto object = (EnvDialog *)data;
      auto child = Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
      for (auto l=object->envList(); l && l->data; l=l->next){
        auto data = (envStruct_t *)l->data;
        if (itemChanged(data)) {
          auto buffer = gtk_entry_get_buffer(data->entry);
          auto txt = gtk_entry_buffer_get_text(buffer);
          g_free(data->value);
          data->value = g_strdup(txt);
          Settings::setString("ENVIRONMENT", data->variable, data->value);
          if (strlen(data->value)) setenv(data->variable, data->value, 1);
          else  unsetenv(data->variable);

          Print::showText(output);
          auto msg = g_strdup_printf("%s: %s=%s\n",
              _("Environment Variable"), data->variable, data->value);
          Print::print(output, msg);
        }
      }

      changeToggle(object);
      auto dialog = object->dialog();
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(1));
      
      //FindSignals<Type>::onFindButton(NULL, data);
      return true;
    }
    
    static void  changeToggle(EnvDialog *object){
      bool changed = isChanged(object);
      if (changed){
        gtk_widget_set_visible(object->frameLabel(),true);
        gtk_widget_set_visible(object->noChanges(),false);
      } else {
        gtk_widget_set_visible(object->noChanges(),true);
        gtk_widget_set_visible(object->frameLabel(),false);
      }
      return;
    }


    static gboolean
    on_keypress (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){
      auto object = (EnvDialog *)data;
      changeToggle(object);
      return FALSE;
    }

    void addBox(GtkBox *topBox, envStruct_t *data){
      auto value = data->value;
      auto box =  GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_box_append(topBox, GTK_WIDGET(box));
      auto label0 = gtk_label_new(data->variable);
      gtk_box_append(box, label0);
      gtk_widget_set_hexpand(GTK_WIDGET(data->entry), true);
      TRACE("addbox, %s --> %s\n", data->variable, value);
      if (value && strlen(value)){
          auto buffer =gtk_entry_get_buffer(data->entry);
          gtk_entry_buffer_set_text(buffer, value, -1);
      }        
      auto keyController = gtk_event_controller_key_new();
      gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_BUBBLE);
//      gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(data->entry), keyController);
      g_signal_connect (G_OBJECT (keyController), "key-released", 
//      g_signal_connect (G_OBJECT (keyController), "key-pressed", 
          G_CALLBACK (this->on_keypress), (void *)this);

      gtk_box_append(box, GTK_WIDGET(data->entry));
      return;
    }
public:
    static void *asyncYes(void *data){
      TRACE("hello world\n");
      return asyncNo(data);
    }

    static void *asyncNo(void *data){
      TRACE("goodbye world\n");
      return NULL;
    }
};
}


#endif

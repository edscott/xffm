#ifndef BFISHRESPONSE_HH
#define BFISHRESPONSE_HH

namespace xf {

template <class Type>
class  bfishResponse {
   using subClass_t = bfishResponse<Type>;
   const char *title_;
   const char *iconName_;
   GtkWindow *dialog_ = NULL;
   char *path_ = NULL;
   GtkBox *mainBox_ = NULL;
   char *labelText_=NULL;
  public:
    // Set a pointer to the GtkWindow in the FileResponse
    // object so that it can be referred to in the
    // async main context thread callbacks.
    // 
    void dialog(GtkWindow *value){ dialog_ = value; }
    GtkWindow *dialog(void){return dialog_;}
    const char *path(void){return path_;}
    const char *label(void){return labelText_;}
public:
    
    ~bfishResponse(void){
     TRACE("*** ~bfishResponse...\n");
     g_free(path_);
     g_free(labelText_);
    }
    bfishResponse(void){
     TRACE("*** bfishResponse...\n");
    }
    

    GtkBox *mainBox(const char *folder, const char *path) {
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        path_ = g_strdup(path);
        gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 450, -1);
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);

        auto hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
        gtk_box_append(mainBox_, GTK_WIDGET(hbox));

        auto image = Texture<bool>::getImage(EMBLEM_BLOWFISH, 48);
        gtk_box_append(hbox, GTK_WIDGET(image));

       

        auto basename = g_path_get_basename(path_);
        bool decrypt = (strrchr(basename, '.') && strcmp(strrchr(basename, '.'), ".bfe")==0);
        auto label = gtk_label_new("");
        char *string;
        if (decrypt) string = g_strconcat("<span color=\"blue\"><b>",_("Decrypt File..."),
            "</b></span>   <span color=\"red\"><b>", basename, "</b></span>", NULL);
        else string = g_strconcat("<span color=\"blue\"><b>",_("Encrypt:"),
            "</b></span>   <span color=\"red\"><b>", basename, "</b></span> ---> ",
            basename, ".bfe", NULL);

        gtk_label_set_markup(GTK_LABEL(label), string);
        g_free(string);
        g_free(basename);
        gtk_box_append(hbox, GTK_WIDGET(label));

        auto check1 = gtk_check_button_new_with_label(_("Remove Input Source"));
        gtk_check_button_set_active(GTK_CHECK_BUTTON(check1), true);
        gtk_box_append(mainBox_, GTK_WIDGET(check1));

        if (!decrypt) {
          auto check2 = gtk_check_button_new_with_label(_("Compress file"));
          gtk_check_button_set_active(GTK_CHECK_BUTTON(check2), true);
          gtk_box_append(mainBox_, GTK_WIDGET(check2));

          auto spinbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_box_append(mainBox_, GTK_WIDGET(spinbox));

          auto spinLabel = gtk_label_new(_("Randomize:"));
          gtk_box_append(spinbox, GTK_WIDGET(spinLabel));

          auto spin = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1.0, 10.0, 1.));
          gtk_spin_button_set_digits(spin, 0);
          gtk_spin_button_set_value(spin, 3);
          gtk_box_append(spinbox, GTK_WIDGET(spin));
        }

 
        auto action_area = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(action_area), false);
        gtk_widget_set_hexpand(GTK_WIDGET(action_area), false);
        gtk_box_append(mainBox_, GTK_WIDGET(action_area));

        auto cancelButton = UtilBasic::mkButton(EMBLEM_RED_BALL, _("Cancel"));
        gtk_box_append(action_area,  GTK_WIDGET(cancelButton));
        gtk_widget_set_vexpand(GTK_WIDGET(cancelButton), false);

        auto bcryptButton = UtilBasic::mkButton (EMBLEM_BLOWFISH, _("Apply"));
        gtk_box_append(action_area,  GTK_WIDGET(bcryptButton));
        gtk_widget_set_vexpand(GTK_WIDGET(bcryptButton), false);
     

        g_signal_connect (G_OBJECT (bcryptButton), "clicked", G_CALLBACK (button_bcrypt), this);
        g_signal_connect (G_OBJECT (cancelButton), "clicked", G_CALLBACK (button_cancel), this);
      
        return mainBox_;
    }

    const char *title(void){ return _("bcrypt (blowfish encryption)");}
    const char *iconName(void){ return EMBLEM_BLOWFISH;}
    //const char *label(void){ return _("Mount Device");}


    static void *asyncNo(void *data){
      DBG("bfishResponse asyncNo\n"); 
       return NULL;

    }
    
    
    static void *asyncYes(void *data){
      DBG("bfishResponse asyncYes\n"); 
     /* auto dialogObject = (DialogComplex<subClass_t> *)data;
      auto entry = dialogObject->subClass()->remoteEntry();
      auto buffer = gtk_entry_get_buffer(entry);
      auto target = (const char *)gtk_entry_buffer_get_text(buffer);
      auto output = Child::getOutput();*/
      //DBG(" bfishResponse, target = %s\n", target ); 

  /*    auto mountSrc = (const char *)dialogObject->subClass()->mountSrc();
      if (EfsResponse<Type>::isEfsMount(mountSrc)){
        DBG("*** do the efs mount for \"%s\"\n", mountSrc);
        // get mount command
        // get mount options
        // prepare mount arguments
        // Run<bool>::thread_run(output, arg, true);
      } else {
        const char *arg[]={"sudo", "-A", "mount", "-v", (const char *)mountSrc, (const char *)target, NULL};
        Run<bool>::thread_run(output, arg, true);
      }*/

       return NULL;
    }
private:

    static void
    button_bcrypt (GtkButton * button, gpointer data) {
      auto subClass = (subClass_t *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(1));
    }

    static void
    button_cancel (GtkButton * button, gpointer data) {
      auto subClass = (subClass_t *)data;
      g_object_set_data(G_OBJECT(subClass->dialog()), "response", GINT_TO_POINTER(-1));
    }

};
}
#endif


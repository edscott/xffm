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

   GtkEntryBuffer *passwdBuffer_ = NULL;
   GtkEntryBuffer *confirmBuffer_ = NULL;

   GtkCheckButton *check1_ = NULL;
   GtkCheckButton *check2_ = NULL;
   GtkCheckButton *check3_ = NULL;
   GtkSpinButton  *spin_ = NULL;
   GtkWidget *confirmFail_ = NULL;
           
  public:
    // Set a pointer to the GtkWindow in the FileResponse
    // object so that it can be referred to in the
    // async main context thread callbacks.
    // 
    void dialog(GtkWindow *value){ dialog_ = value; }
    GtkWindow *dialog(void){return dialog_;}
    const char *path(void){return path_;}
    const char *label(void){return labelText_;}

   GtkEntryBuffer *passwdBuffer(void) {return passwdBuffer_;}
   GtkEntryBuffer *confirmBuffer(void) {return confirmBuffer_;}

   GtkCheckButton *check1(void) {return check1_;}
   GtkCheckButton *check2(void) {return check2_;}
   GtkCheckButton *check3(void) {return check3_;}
   GtkSpinButton  *spin(void) {return spin_;}

   GtkWidget *confirmFail(void) {return confirmFail_;}
   GtkBox *mainBox(void) {return mainBox_;}

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
        if (decrypt) g_object_set_data(G_OBJECT(mainBox_), "decrypt", GINT_TO_POINTER(1));
        auto label = gtk_label_new("");
        char *string;
        char *uncrypted = g_strdup(basename);
        if (decrypt) *strrchr(uncrypted, '.') = 0;
        if (decrypt) string = g_strconcat("<span color=\"blue\"><b>",_("Decrypt File..."),
            "</b></span>   <span color=\"red\"><b>", basename, 
            "</b></span> <b>---></b><span color=\"green\"><b> ",uncrypted, 
            "</b></span>", NULL);
        else string = g_strconcat("<span color=\"blue\"><b>",_("Encrypt:"),
            "</b></span>   <span color=\"red\"><b>", basename,
            "</b></span> <b>---></b><span color=\"green\"><b> ", basename,
            ".bfe</b></span>", NULL);

        gtk_label_set_markup(GTK_LABEL(label), string);
        g_free(string);
        g_free(basename);
        gtk_box_append(hbox, GTK_WIDGET(label));

        check1_ = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Remove Input Source")));
        gtk_check_button_set_active(GTK_CHECK_BUTTON(check1_), !decrypt);
        gtk_box_append(mainBox_, GTK_WIDGET(check1_));
         
        check2_ = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Compress file")));
        spin_ = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1.0, 10.0, 1.));
        
        check3_ = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Writes text on standard output.")));

        if (!decrypt) {
          gtk_check_button_set_active(GTK_CHECK_BUTTON(check2_), true);
          gtk_box_append(mainBox_, GTK_WIDGET(check2_));

          auto spinbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_box_append(mainBox_, GTK_WIDGET(spinbox));

          auto spinLabel = gtk_label_new(_("Randomize:"));
          gtk_box_append(spinbox, GTK_WIDGET(spinLabel));

          gtk_spin_button_set_digits(spin_, 0);
          gtk_spin_button_set_value(spin_, 3);
          gtk_box_append(spinbox, GTK_WIDGET(spin_));
        } else {
          gtk_box_append(mainBox_, GTK_WIDGET(check3_));
          gtk_check_button_set_active(GTK_CHECK_BUTTON(check3_), false);
        }


       auto passwdBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
       gtk_widget_set_hexpand(GTK_WIDGET(passwdBox), false);
       gtk_widget_set_halign (GTK_WIDGET(passwdBox),GTK_ALIGN_START);
       gtk_box_append(mainBox_, GTK_WIDGET(passwdBox));

       auto confirmBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
       gtk_widget_set_hexpand(GTK_WIDGET(confirmBox), false);
       gtk_widget_set_halign (GTK_WIDGET(confirmBox),GTK_ALIGN_START);

       auto passwdLabel = gtk_label_new(_("Password:"));
       gtk_box_append(passwdBox, GTK_WIDGET(passwdLabel));
       auto confirmLabel = gtk_label_new(_("Confirm password:"));
       gtk_box_append(confirmBox, GTK_WIDGET(confirmLabel));
        
       auto passwdEntry = GTK_ENTRY(gtk_entry_new ());
       g_object_set_data(G_OBJECT(passwdEntry), "name", (void *)"passwd");
       gtk_entry_set_visibility (passwdEntry, false);
       auto confirmEntry = GTK_ENTRY(gtk_entry_new ());
       g_object_set_data(G_OBJECT(confirmEntry), "name", (void *)"confirm");
       gtk_entry_set_visibility (confirmEntry, false);
       
       passwdBuffer_ = gtk_password_entry_buffer_new();
       confirmBuffer_ = gtk_password_entry_buffer_new();
       gtk_entry_set_buffer (passwdEntry,passwdBuffer_);
       gtk_entry_set_buffer (confirmEntry,confirmBuffer_);
       addKeyController(GTK_WIDGET(passwdEntry));
       addKeyController(GTK_WIDGET(confirmEntry));

       gtk_box_append(passwdBox, GTK_WIDGET(passwdEntry));
       gtk_box_append(confirmBox, GTK_WIDGET(confirmEntry));
       confirmFail_ = gtk_label_new("");
       auto markup = g_strconcat("<span color=\"red\">", _("Passwords do not match."), "</span>", NULL);
       gtk_label_set_markup(GTK_LABEL(confirmFail_), markup);
       g_free(markup);
       gtk_box_append(mainBox_, GTK_WIDGET(confirmBox));
       gtk_box_append(mainBox_, GTK_WIDGET(confirmFail_));
       gtk_widget_set_visible(GTK_WIDGET(confirmFail_), false);

       if (decrypt) gtk_widget_set_visible(GTK_WIDGET(confirmBox), false);
  
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
        g_signal_connect (G_OBJECT (check1_), "toggled", G_CALLBACK (toggleRandom), this);
        g_signal_connect (G_OBJECT (check3_), "toggled", G_CALLBACK (toggleOutput), this);
      
        return mainBox_;
    }

    const char *title(void){ return _("bcrypt (blowfish encryption)");}
    const char *iconName(void){ return EMBLEM_BLOWFISH;}
    //const char *label(void){ return _("Mount Device");}


    static void *asyncNo(void *data){
      auto output = Child::getOutput();
      auto message = g_strdup_printf("bcrypt: %s\n", _("User cancelled."));
      Print::printWarning(output, message);
      return NULL;
    }

    static void *asyncYes(void *data){
      auto dialogObject = (DialogComplex<subClass_t> *)data;

      auto p = (subClass_t *) dialogObject->subClass();
      auto output = Child::getOutput();
      DBG("bfishResponse asyncYes\n"); 

      auto passwd =gtk_entry_buffer_get_text(p->passwdBuffer());
      auto confirm =gtk_entry_buffer_get_text(p->confirmBuffer());
      auto decrypt = g_object_get_data(G_OBJECT(p->mainBox()), "decrypt");

      if (!passwd || !strlen(passwd)) {
        auto message = g_strdup_printf("bcrypt: %s\n", _("No password set"));
        Print::printWarning(output, message);
        return NULL;        
      }
      if (!decrypt){
        if (!confirm || strcmp(confirm, passwd)){
          auto message = g_strdup_printf("bcrypt: %s\n", _("Passwords Do Not Match"));
          Print::printWarning(output, message);
          return NULL;
        }
      }    

      bool remove = gtk_check_button_get_active(p->check1());
      bool compress = gtk_check_button_get_active(p->check2());
      bool standardOut = gtk_check_button_get_active(p->check3());
      double randomize = gtk_spin_button_get_value(p->spin());
      int spinValue = randomize;
      auto basename = g_path_get_basename(p->path());
      char *message = NULL;
      if (decrypt){
        message = g_strdup_printf(" bcrypt %s %s %s\n", 
            remove?"":"-r", standardOut?"-o":"",
            basename);
      } else {
        if (!gtk_widget_get_sensitive(GTK_WIDGET(p->check1()))) remove = false;
        message = g_strdup_printf("bcrypt -s%d %s %s %s\n", 
            spinValue, 
            remove?"":"-r", compress?"":"-c",
            basename);
      }
      if (message) {
        Print::showText(output);
        Print::print(output, EMBLEM_GREEN_BALL, "green", message);
      }
        


      g_free(basename);



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

    void addKeyController(GtkWidget  *widget){
        auto keyController = gtk_event_controller_key_new();
        gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)this);
    }

    static gboolean
    on_keypress (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){
      auto p = (bfishResponse *)data;
      auto widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
      auto name = (const char *)g_object_get_data(G_OBJECT(widget), "name");
      auto buffer = gtk_entry_get_buffer(GTK_ENTRY(widget));
      auto newString = g_strdup_printf("%s%c", gtk_entry_buffer_get_text(buffer), keyval);

      auto decrypt = g_object_get_data(G_OBJECT(p->mainBox()), "decrypt");
      if (!decrypt) {
        if (strcmp(name, "passwd") ==0){
          auto confirm = gtk_entry_buffer_get_text(p->confirmBuffer());
          gtk_widget_set_visible(p->confirmFail(), strcmp(newString, confirm));
        } else {
          auto passwd = gtk_entry_buffer_get_text(p->passwdBuffer());
          gtk_widget_set_visible(p->confirmFail(), strcmp(passwd, newString));

        }
      }
      g_free(newString);
      return false;
    }
    

    static void
    button_bcrypt (GtkButton * button, gpointer data) {
      auto p = (bfishResponse *)data;
      g_object_set_data(G_OBJECT(p->dialog()), "response", GINT_TO_POINTER(1));
    }

    static void
    button_cancel (GtkButton * button, gpointer data) {
      auto p = (bfishResponse *)data;
      g_object_set_data(G_OBJECT(p->dialog()), "response", GINT_TO_POINTER(-1));
    }

    static void
    toggleRandom (GtkCheckButton *check, gpointer data) {
      auto p = (bfishResponse *)data;
      gtk_widget_set_sensitive(GTK_WIDGET(p->spin()), gtk_check_button_get_active(check));
    }

    static void
    toggleOutput (GtkCheckButton *check, gpointer data) {
      auto p = (bfishResponse *)data;
      gtk_widget_set_sensitive(GTK_WIDGET(p->check1()), !gtk_check_button_get_active(check));
    }

};
}
#endif


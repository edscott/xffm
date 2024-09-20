#ifndef ENTRYRESPONSE_HH
#define ENTRYRESPONSE_HH

namespace xf {
template <class Type> class Prompt;
template <class Type> class Run;

template <class Type>
class OpenWith {
    
    GtkWindow *dialog_;
    time_t timeout_;
    char *path_;
    GtkProgressBar *timeoutProgress_;
    GtkCheckButton *checkbutton_;
    GtkTextView *input_;
    GtkWidget *child_;
    
   
protected:
    GtkWindow *dialog(void){return dialog_;}
    const char *path(void){ return (const char *)path_;}
    time_t timeout(void){return timeout_;}
    void timeout(time_t value){timeout_ = value;}
    GtkProgressBar *progress(void){return timeoutProgress_;}
    GtkCheckButton *checkbutton(void){ return checkbutton_;}
    GtkTextView *input(void){ return input_;}
    GtkWidget *child(void){ return child_;}
        
    Prompt<Type> *prompt_p;
    GtkBox *buttonSpace;
public:
    
    ~OpenWith (void){
       gtk_widget_set_visible(GTK_WIDGET(dialog_), FALSE);
       g_free(path_);
       gtk_window_destroy(dialog_);
    }

    OpenWith (GtkWindow *parent, const gchar *inPath){
      const gchar *windowTitle = _("Open With...");
      const gchar *icon = "emblem-run";
      child_ = Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child_), "output"));

      path_ = g_strdup(inPath);
      timeout_ = 10;

      dialog_ = GTK_WINDOW(gtk_window_new ());
      gtk_window_set_title (GTK_WINDOW (dialog_), windowTitle);
      g_signal_connect (G_OBJECT (dialog_), "close-request", G_CALLBACK (OpenWith::dialogClose), this);

      gtk_window_set_transient_for (GTK_WINDOW (dialog_), GTK_WINDOW (parent));
      gtk_window_set_resizable (GTK_WINDOW (dialog_), FALSE);
      gtk_window_set_destroy_with_parent(dialog_, TRUE);

      auto vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      gtk_window_set_child(dialog_, GTK_WIDGET(vbox));
      // icon title
      if (icon){
          auto paintable = Texture::load(icon, 48);
          auto image = gtk_image_new_from_paintable(paintable);
          gtk_widget_set_size_request(image, 48, 48);
          Basic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(image), TRUE, TRUE, 0);
      }
      // path title
      auto label = GTK_LABEL(gtk_label_new (""));
      auto markup = g_strconcat("<span color=\"blue\" size=\"large\"> <b>", path_, "</b></span>", NULL);
      gtk_label_set_markup(label, markup);
      g_free(markup);
      Basic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(label), TRUE, TRUE, 0);
      // mimetype title
      //auto mimeBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0)); 
      auto mimeBox = Basic::mkEndBox(); 
      auto labelMime = GTK_LABEL(gtk_label_new (""));
      auto mimetype = MimeMagic::mimeMagic(path_); 
      auto markup2 = g_strconcat("<span color=\"brown\" size=\"small\">", mimetype, "</span>", NULL);
      gtk_label_set_markup(labelMime, markup2);
      g_free(markup2);
      auto apps = MimeApplication::locate_apps(mimetype);
 
      Basic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(mimeBox), TRUE, TRUE, 0);
      Basic::boxPack0(GTK_BOX (mimeBox), GTK_WIDGET(labelMime), FALSE, FALSE, 0);
      // execute button
      if (g_file_test(path_, G_FILE_TEST_IS_EXECUTABLE))
      {
        auto labelExe = gtk_label_new(_("Is executable"));
        Basic::boxPack0(GTK_BOX (mimeBox), GTK_WIDGET(labelExe), FALSE, FALSE, 5);
        auto execute = gtk_button_new();
        auto run = gtk_image_new_from_icon_name("emblem-run");
        auto ebox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto elabel = gtk_label_new(_("Execute"));
        Basic::boxPack0(GTK_BOX (ebox),GTK_WIDGET(run), FALSE, FALSE, 0);
        Basic::boxPack0(GTK_BOX (ebox),GTK_WIDGET(elabel), FALSE, FALSE, 0);
        gtk_button_set_child(GTK_BUTTON(execute), GTK_WIDGET(ebox));
        g_signal_connect (G_OBJECT (execute), "clicked", G_CALLBACK (OpenWith::dialogProceed), this);
        Basic::boxPack0(mimeBox, GTK_WIDGET(execute), FALSE,FALSE, 0);
      }

      // prompt

      auto child = Child::getChild();
      buttonSpace = Child::getButtonSpace(child);
      prompt_p = (Prompt<Type> *) new Prompt<Type>(child);
      auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
      gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
      auto label2 = GTK_LABEL(gtk_label_new (windowTitle));
      input_ = prompt_p->input();
      const char *extension = NULL;
      if (strchr(path_, '.')) extension = strrchr(path_, '.') + 1;
      char *defaultApp = NULL;
      if (extension){
        defaultApp = Settings::getString("MimeTypeApplications", extension);
      }

      if (defaultApp){
        Print::print(input_, g_strdup(defaultApp));
      } else if (apps){
        Print::print(input_, g_strdup(apps[0])); // first item
      }

      gtk_widget_set_size_request(GTK_WIDGET(input_), 200, -1);
      
      auto mimeButton = GTK_MENU_BUTTON(gtk_menu_button_new());
      gtk_menu_button_set_icon_name(mimeButton, "go-down");
      auto popover = GTK_POPOVER(gtk_popover_new());
      auto popoverBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
      gtk_popover_set_child(popover, GTK_WIDGET(popoverBox));
      gtk_menu_button_set_popover(mimeButton, GTK_WIDGET(popover)); // Sets popover parent?
      for (auto p=apps; p && *p; p++){
        auto box = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto label = gtk_label_new(*p);
        g_object_set_data(G_OBJECT(label), "openWith", this);
        g_object_set_data(G_OBJECT(label), "popover", popover);
        Basic::boxPack0(GTK_BOX (box), GTK_WIDGET(label), FALSE, FALSE, 0);
        Basic::boxPack0(GTK_BOX (popoverBox), GTK_WIDGET(box), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(label), "input", input_);
        addGestureClick(label, (void *)labelClick); 
        addMotionController(GTK_WIDGET(box));
      }
      g_free(mimetype);



      Basic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(label2), FALSE, FALSE, 3);
      Basic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(input_), TRUE, TRUE, 3);
      Basic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(mimeButton), FALSE, FALSE, 3);
      Basic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(hbox), TRUE, TRUE, 0);

        // check button
      {
        auto info = _("By default applications will be run in the background and only their "\
"output will be displayed in a toolview. This makes it impossible to interact "\
"with applications requiring user input from a terminal emulator. To run such "\
"applications, you should use an external terminal.");

        auto box = Basic::mkEndBox();
        Basic::boxPack0(GTK_BOX (vbox),GTK_WIDGET(box), TRUE, TRUE, 5);
        auto label = gtk_label_new(_("Use External Terminal:"));
        gtk_widget_set_tooltip_markup(label, info);
        checkbutton_ = GTK_CHECK_BUTTON(gtk_check_button_new());

        Basic::boxPack0(GTK_BOX (box),GTK_WIDGET(label), FALSE, FALSE, 0);
        Basic::boxPack0(GTK_BOX (box),GTK_WIDGET(checkbutton_), FALSE, FALSE, 0);
        //DBG("default app=%s\n", defaultApp);
        //DBG("default apps[0]=%s\n", apps[0]);
        char *key = NULL;
        if (defaultApp){
          key = g_strdup(defaultApp);
          g_free (defaultApp);
        } else if (apps){
          key = g_strdup(apps[0]);
        }
        if (key && strchr(key, ' ')) *strrchr(key, ' ') = 0;
        //DBG("key=%s\n", key);
        if (key) {
          if (Settings::getInteger("ExternalTerminal", key) == 1){
            gtk_check_button_set_active(checkbutton_, true);
          } else {
            gtk_check_button_set_active(checkbutton_, false);
          }
          g_free(key);
        }
      }

        // button box
        auto buttonBox = Basic::mkEndBox();
        Basic::boxPack0(GTK_BOX (vbox),GTK_WIDGET(buttonBox), TRUE, TRUE, 10);

        // no button

        auto no = gtk_button_new();
        auto red = gtk_image_new_from_icon_name("emblem-redball");
        auto rbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto rlabel = gtk_label_new(_("Cancel"));
        Basic::boxPack0(GTK_BOX (rbox),GTK_WIDGET(red), FALSE, FALSE, 0);
        Basic::boxPack0(GTK_BOX (rbox),GTK_WIDGET(rlabel), FALSE, FALSE, 0);
        gtk_button_set_child(GTK_BUTTON(no), GTK_WIDGET(rbox));
        g_signal_connect (G_OBJECT (no), "clicked", G_CALLBACK (OpenWith::dialogCancel), this);
        Basic::boxPack0(buttonBox, GTK_WIDGET(no), FALSE,FALSE, 0);

       // yes button
        auto yes = gtk_button_new();
        auto green = gtk_image_new_from_icon_name("emblem-greenball");
        auto gbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto glabel = gtk_label_new(_("Proceed"));
        Basic::boxPack0(GTK_BOX (gbox),GTK_WIDGET(green), FALSE, FALSE, 0);
        Basic::boxPack0(GTK_BOX (gbox),GTK_WIDGET(glabel), FALSE, FALSE, 0);
        gtk_button_set_child(GTK_BUTTON(yes), GTK_WIDGET(gbox));
        g_signal_connect (G_OBJECT (yes), "clicked", G_CALLBACK (OpenWith::dialogProceed), this);
        Basic::boxPack0(buttonBox, GTK_WIDGET(yes), FALSE,FALSE, 0);
 
        // progress bar timeout       
        timeoutProgress_ = GTK_PROGRESS_BAR(gtk_progress_bar_new());
        Basic::boxPack0 (GTK_BOX (vbox),GTK_WIDGET(timeoutProgress_), TRUE, TRUE, 0);
        g_timeout_add(500, OpenWith::updateProgress, (void *)this);
 
    
        auto keyController = gtk_event_controller_key_new();
        gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(dialog_), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)this);
    


        // finish up
        gtk_widget_realize (GTK_WIDGET(dialog_));
        gtk_widget_grab_focus(GTK_WIDGET(input_));
        Basic::setAsDialog(GTK_WIDGET(dialog_), "xfDialog", "XfDialog");
        gtk_window_present(dialog_);
        return;
    }
    
    static void
    run(OpenWith *object){
       if (!Child::valid(object->child())){
         DBG("Child widget (%p) is no longer valid.\n", object->child());
         return;
       }

        auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(object->child()), "output"));
        bool inTerminal = gtk_check_button_get_active(object->checkbutton());
        // get input text
        auto buffer = gtk_text_view_get_buffer(object->input());
        GtkTextIter  start, end;
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        auto inputText = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        g_strstrip(inputText);  
        auto executable = g_strdup(inputText);
        if (strchr(executable, ' ')) *strchr(executable, ' ') = 0;
        if (!g_find_program_in_path(executable)){
          auto message = g_strdup_printf(_("Cannot find executable for \"%s\""), executable);
          Basic::concat(&message, "\n");
          Print::printError(output, message);
          Print::showText(output);
          g_free(inputText);
          g_free(executable);
          return;
        } 
        g_free(executable);
        const char *extension = NULL;
        if (strchr(object->path(), '.')) extension = strrchr(object->path(), '.') + 1;
        if (extension) {
          Settings::setString("MimeTypeApplications", extension, inputText);
        }
        char *key = g_strdup(inputText);
        if (Basic::alwaysTerminal(inputText)) inTerminal = true;
        char *command = NULL;
        if (inTerminal) {
          command = Run<Type>::mkTerminalLine(inputText, object->path());
          Settings::setInteger("ExternalTerminal", key, 1);
        }
        else {
          command = Run<Type>::mkCommandLine(inputText, object->path());
          Settings::setInteger("ExternalTerminal", key, 0);
        }
        g_free(key);
        object->prompt_p->run(output, command, true, true, object->buttonSpace);
        g_free(command);
        g_free(inputText);
    }

    static gboolean
    on_keypress (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){

       auto object = (OpenWith *)data;
       auto progress = object->progress();
        if (!progress || !GTK_IS_PROGRESS_BAR(progress)) return FALSE;
        gtk_progress_bar_set_fraction(progress, 0.0);
        if(keyval == GDK_KEY_Return) { 
          run(object);
          object->timeout_=-1;
          // Do not call return from Prompt class.
          return TRUE;
        }
        if(keyval == GDK_KEY_Tab) { 
          auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(object->child()), "output"));
          Print::showText(output);
        }
        return FALSE;
      
    }

    static gboolean
    updateProgress(void * data){
        auto object = (OpenWith *)data;
        auto arg = (void **)data;
        auto timeout = object->timeout();
        auto progress = object->progress();
        auto dialog  = object->dialog();


        // Shortcircuit to delete.
        if (timeout < 0) {
          TRACE("timeout done...\n");
            delete object->prompt_p;
            delete object;
            return G_SOURCE_REMOVE;
        }
            
        // While window is active, pause progress bar.
        if (gtk_window_is_active(GTK_WINDOW(dialog))){
          TRACE("gtk_window_is_active\n");
            return G_SOURCE_CONTINUE;
        }
         TRACE("timeout = %lf\n", timeout);

        if (timeout > 1.0) {
            // Add fraction to progress bar.
            auto fraction = gtk_progress_bar_get_fraction(progress);
            if (fraction < 1.0) {
                fraction += (1.0 / timeout /2.0);
                TRACE("gtk_progress_bar_set_fraction %lf\n", fraction);
                gtk_progress_bar_set_fraction(progress, fraction);
            } 
            // Complete fraction, delete object.
            if (fraction >= 1.0) {
                TRACE("cancel dialog\n");
                object->timeout(-1);
                //gtk_dialog_response(dialog, GTK_RESPONSE_CANCEL);
                return G_SOURCE_CONTINUE; 
            }
        }
        return G_SOURCE_CONTINUE;
    }

private:
    

    static void
    dialogProceed (GtkButton *button, void *data) {
        auto object = (OpenWith *)data;
        run(object);
        object->timeout_=-1;
    }

    static void
    dialogCancel (GtkButton *button, void *data) {
      auto object = (OpenWith *)data;
      object->timeout_=-1;

//      delete object;
    }

    static gboolean 
    dialogClose(GtkWindow *dialog, void *data){
      auto object = (OpenWith *)data;
      object->timeout_=-1;
//      delete object;
      return TRUE;
    }

    static void 
    dialogActivate(GtkEntry *entry, void *data){
      dialogProceed(NULL, data);
      DBG("FIXME: gtk_dialog_response (GTK_DIALOG(dialog),dialogActivate)\n");
    }

    static gboolean
    labelClick(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
      auto label = GTK_LABEL(data);
      auto object = (OpenWith *)g_object_get_data(G_OBJECT(label), "openWith");
      auto popover = GTK_POPOVER(g_object_get_data(G_OBJECT(label), "popover"));
      auto text = gtk_label_get_text(label);
      auto key = g_strdup(text);
      if (strchr(key, ' ')) *strrchr(key,' ') = 0;
      auto input = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(label), "input"));
      Print::clear_text(input);
      Print::print(input, g_strdup(text));
      if (Settings::getInteger("ExternalTerminal", key) == 1){
        gtk_check_button_set_active(object->checkbutton(), true);
      } else {
        gtk_check_button_set_active(object->checkbutton(), false);
      }
      g_free(key);
      

      gtk_popover_popdown(popover);
      gtk_widget_grab_focus(GTK_WIDGET(input));
      
      //DBG("label text =\"%s\"\n", text);
      return TRUE;
    }
      static
      void addMotionController(GtkWidget  *widget){
        auto controller = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), controller);
        g_signal_connect (G_OBJECT (controller), "enter", 
            G_CALLBACK (negative), NULL);
        g_signal_connect (G_OBJECT (controller), "leave", 
            G_CALLBACK (positive), NULL);
    }
    
    static void addGestureClick(GtkWidget *label, void *callback){
      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1); 
      // 1 for action released, 3 for popover pressed
      // Add a different gtk_gesture_click_new for 3 and menu.
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (callback), (void *)label);
      gtk_widget_add_controller(GTK_WIDGET(label), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);
    }    
  
    static gboolean
    negative ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        Basic::flushGTK();
        return FALSE;
    }
    static gboolean
    positive ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_remove_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        Basic::flushGTK();
        return FALSE;
    }


};
}
#endif

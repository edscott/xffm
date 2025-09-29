#ifndef OPENWITH_HH
#define OPENWITH_HH

namespace xf {
template <class Type> class Prompt;
template <class Type> class Run;

template <class Type>
class OpenWith {
   using dialog_t = DialogBasic<LocalDir >;
    GtkEventController *raiseController_ = NULL;
    GtkWindow *dialog_ = NULL;
    GtkWindow *parent_= GTK_WINDOW(Child::mainWidget());
    time_t timeout_;
    char *path_ = NULL;
    GtkProgressBar *timeoutProgress_ = NULL;
    GtkCheckButton *checkbutton_ = NULL;
    GtkTextView *input_ = NULL;
    GtkTextView *output_ = NULL;
    GtkWidget *child_ = NULL;
    GList *selectionList_ = NULL;
    bool withTextview_ = false;
   
protected:
    GtkEventController *raiseController(void){return raiseController_;}
    GtkWindow *parent(void){ return parent_;}
    GtkWindow *dialog(void){return dialog_;}
    time_t timeout(void){return timeout_;}
    void timeout(time_t value){timeout_ = value;}
    GtkProgressBar *progress(void){return timeoutProgress_;}
    GtkCheckButton *checkbutton(void){ return checkbutton_;}
    GtkTextView *input(void){ return input_;}
    GtkTextView *output(void){ return output_;}
    GtkWidget *child(void){ return child_;}
        
    Prompt<Type> *prompt_p = NULL;
    GtkBox *buttonSpace = NULL;

    const char *path(void){ return (const char *)path_; }
    char *paths(void){ 
      if (g_list_length(selectionList_) < 2) return NULL;
      auto g = g_strdup("");
      for (auto l=selectionList_; l && l->data; l=l->next){
        auto info = G_FILE_INFO(l->data);
        auto p = Basic::getPath(info);
        auto e = Basic::esc_string (p);
        Basic::concat(&g, e);
        g_free(e);
        g_free(p);
        Basic::concat(&g, " ");
      }
      g_strstrip(g);
      return g;
    }

     void setRaise(void){
      auto content = GTK_WIDGET(g_object_get_data(G_OBJECT(parent_), "frame"));
      gtk_widget_set_sensitive(GTK_WIDGET(content), false);
      raiseController_ = gtk_event_controller_motion_new();
      gtk_event_controller_set_propagation_phase(raiseController_, GTK_PHASE_CAPTURE);
      gtk_widget_add_controller(GTK_WIDGET(parent_), raiseController_);
      g_signal_connect (G_OBJECT (raiseController_), "enter", 
              G_CALLBACK (dialog_t::presentDialog), dialog_);      
    }
     
    static void *unsetRaise_f(void *data){
      auto object = (OpenWith<Type> *)data;
      auto content = GTK_WIDGET(g_object_get_data(G_OBJECT(object->parent()), "frame"));
      gtk_widget_set_sensitive(GTK_WIDGET(content), true);
      gtk_widget_remove_controller(GTK_WIDGET(object->parent()), 
          object->raiseController());
      return NULL;
    }

public:
    void withTextview(bool value){withTextview_ = value;}
    bool withTextview(void){return withTextview_;}
    void freeSelectionList(void){
      if (!selectionList_) return;
      Basic::freeSelectionList(selectionList_);
    } 
    ~OpenWith (void){
       Basic::context_function(unsetRaise_f, this);
       gtk_widget_set_visible(GTK_WIDGET(dialog_), FALSE);
       g_free(path_);
       //Basic::popDialog(dialog_);
       gtk_window_destroy(dialog_);
    }

    OpenWith (GtkTextView *output, const gchar *inPath){
      if (!inPath || !g_file_test(inPath, G_FILE_TEST_EXISTS)) throw 1;
      if (!output) throw 2;
      output_ = output;
      path_ = g_strdup(inPath);
      mkDialog(output);

    }

    OpenWith (GtkWindow *parent, const gchar *inPath, GList *selectionList){
      selectionList_ = selectionList;
      child_ = Child::getChild();
      output_ = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child_), "output"));

      if (inPath) path_ = g_strdup(inPath);
      else {
        if (!selectionList_){
          ERROR_("*** Error:: OpenWith(): selectionList_ is NULL.\n");
          Basic::Exit("*** Error:: OpenWith(): selectionList_ is NULL.\n");
        }
        auto info = G_FILE_INFO(selectionList_->data);
        path_ = Basic::getPath(info);
      }
      mkDialog(NULL);

         
        return;
    }

private:
    void mkDialog(GtkTextView *textView){
      const gchar *windowTitle = _("Open With...");
      timeout_ = 10;
      dialog_ = GTK_WINDOW(gtk_window_new ());
      //Basic::pushDialog(dialog_);
      gtk_window_set_decorated(dialog_, true);

      gtk_window_set_title (GTK_WINDOW (dialog_), windowTitle);
      g_signal_connect (G_OBJECT (dialog_), "close-request", G_CALLBACK (OpenWith::dialogClose), this);
/*
      gtk_window_set_transient_for (GTK_WINDOW (dialog_), GTK_WINDOW (parent));
      gtk_window_set_destroy_with_parent(dialog_, TRUE);
      */
      gtk_window_set_resizable (GTK_WINDOW (dialog_), FALSE);

      auto frame = GTK_FRAME(gtk_frame_new(NULL));
      gtk_frame_set_label_align(frame, 1.0);
      auto closeBox = Dialog::buttonBox("close", _("Close"), (void *)cancelCallback, this);
      gtk_frame_set_label_widget(frame, GTK_WIDGET(closeBox));
      //gtk_widget_set_vexpand(GTK_WIDGET(frame), false);
      //gtk_widget_set_hexpand(GTK_WIDGET(frame), false);

      gtk_window_set_child(dialog_, GTK_WIDGET(frame));
      g_object_set_data(G_OBJECT(dialog_), "frame", frame);

      auto vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      gtk_widget_set_vexpand(GTK_WIDGET(vbox), false);
      gtk_widget_set_hexpand(GTK_WIDGET(vbox), false);
      //gtk_window_set_child(dialog_, GTK_WIDGET(vbox));
      gtk_frame_set_child(GTK_FRAME(frame), GTK_WIDGET(vbox));
      

      // icon 
//        auto picture = Texture<bool>::getPicture(EMBLEM_RUN, 50);
//        gtk_box_append(GTK_BOX (vbox), GTK_WIDGET(picture));
//      auto image = Texture<bool>::getImage(EMBLEM_RUN, 50); // image now ignores size (gtk>=4.20)

      // path title
      auto label = GTK_LABEL(gtk_label_new (""));
      auto markup = g_strdup_printf("<span color=\"%s\" size=\"large\"> <b>%s</b></span>", 
          (selectionList_==NULL)?"blue":"red",(selectionList_==NULL)?path_:_("Multiple selections"));
      gtk_label_set_markup(label, markup);
      g_free(markup);
      Basic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(label), TRUE, TRUE, 0);

      // prompt
      if (textView == NULL) {
        auto child = Child::getChild();
        buttonSpace = Child::getButtonSpace(child); // XXX Apparently not used anymore.
        prompt_p = (Prompt<Type> *) new Prompt<Type>(child);
        g_object_set_data(G_OBJECT(child), "prompt", prompt_p); // Flexible prompt object.
      } else {
        prompt_p = (Prompt<Type> *) new Prompt<Type>(textView);
      }

      auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
      //gtk_box_prepend(GTK_BOX(hbox), GTK_WIDGET(image));
      gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
      gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
      auto label2 = GTK_LABEL(gtk_label_new (windowTitle));
      input_ = prompt_p->input();
      gtk_widget_set_size_request(GTK_WIDGET(input_), 200, -1);

      gtk_box_append(GTK_BOX (hbox), GTK_WIDGET(label2));
      gtk_box_append(GTK_BOX (hbox), GTK_WIDGET(input_));
      gtk_box_append(GTK_BOX (vbox), GTK_WIDGET(hbox));

      /*Basic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(label2), FALSE, FALSE, 3);
      Basic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(input_), TRUE, TRUE, 3);
      Basic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(hbox), TRUE, TRUE, 0);*/

      if (path_){
        // mimetype title
        //auto mimeBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0)); 
        //auto mimeBox = Basic::mkEndBox(); 
        auto mimeBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0); 
        auto labelMime = GTK_LABEL(gtk_label_new (""));
        auto mimetype = MimeMagic::mimeMagic(path_); 
        auto markup2 = g_strconcat("<span color=\"brown\" size=\"small\">", mimetype, "</span>", NULL);
        gtk_label_set_markup(labelMime, markup2);
        g_free(markup2);
        auto apps = MimeApplication::locate_apps(mimetype);
   
        gtk_box_append(GTK_BOX (vbox), GTK_WIDGET(mimeBox));
        //Basic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(mimeBox), TRUE, TRUE, 0);
        gtk_box_append(GTK_BOX (mimeBox), GTK_WIDGET(labelMime));
        //Basic::boxPack0(GTK_BOX (mimeBox), GTK_WIDGET(labelMime), FALSE, FALSE, 0);
        // execute button
        if (g_file_test(path_, G_FILE_TEST_IS_EXECUTABLE))
        {
          auto labelExe = gtk_label_new(_("Is executable"));
          Basic::boxPack0(GTK_BOX (mimeBox), GTK_WIDGET(labelExe), FALSE, FALSE, 5);

          auto execute = Dialog::buttonBox(EMBLEM_RUN, _("Execute"), (void *)runIt, this);
          Basic::boxPack0(GTK_BOX (mimeBox), GTK_WIDGET(execute), FALSE, FALSE, 3);


     /*     auto execute = gtk_button_new();
          auto run = Texture<bool>::getImage(EMBLEM_RUN, 48);
          auto ebox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          auto elabel = gtk_label_new(_("Execute"));
          gtk_box_append(GTK_BOX (ebox), GTK_WIDGET(run));
          gtk_box_append(GTK_BOX (ebox), GTK_WIDGET(elabel));
          gtk_button_set_child(GTK_BUTTON(execute), GTK_WIDGET(ebox));
          g_signal_connect (G_OBJECT (execute), "clicked", G_CALLBACK (OpenWith::dialogProceed), this);
          gtk_box_append(GTK_BOX (mimeBox), GTK_WIDGET(execute));*/
        }

        gchar *fileInfo = Basic::fileInfo(path_);
        gchar *defaultApp = Run<Type>::defaultExtApp(path_);
        if (!defaultApp) defaultApp = Run<Type>::defaultMimeTypeApp(mimetype);
        if (!defaultApp) defaultApp = Run<Type>::defaultTextApp(fileInfo);
          if (defaultApp){
            char *g = g_strdup_printf("%s", defaultApp);
            Print::print(input_, g);
          } else if (apps){
            char *g = g_strdup_printf("%s", defaultApp);
            Print::print(input_, g); // first item
          }
    
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
          Basic::addMotionController(GTK_WIDGET(box));
        }
        g_object_set_data(G_OBJECT(popover), "mimetype", mimetype);
        g_object_set_data(G_OBJECT(dialog_), "mimetype", mimetype);
        //g_free(mimetype);

        if (textView != NULL) {
          withTextview(true);
        }
        //auto cancel = Dialog::buttonBox("no", _("Cancel"), (void *)cancelCallback, this);
        auto yesBox = Dialog::buttonBox("apply", _("Apply"), (void *)ok, this);
        Basic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(mimeButton), FALSE, FALSE, 3);
        Basic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(yesBox), FALSE, FALSE, 3);
        //Basic::boxPack0(GTK_BOX (hbox),GTK_WIDGET(cancel), FALSE, FALSE, 10);
     

        // check button
      
        auto text = _("By default applications will be run in the background and only their "\
"output will be displayed in a toolview. This makes it impossible to interact "\
"with applications requiring user input from a terminal emulator. To run such "\
"applications, you should use an external terminal.");
        auto box = Basic::mkEndBox();
        Basic::boxPack0(GTK_BOX (vbox),GTK_WIDGET(box), TRUE, TRUE, 5);



        
       /* auto cancel = UtilBasic::mkButton("no", _("Cancel")); //4
        g_signal_connect(G_OBJECT(cancel), "pressed", G_CALLBACK(cancelCallback), this);
        Basic::boxPack0(GTK_BOX (box),GTK_WIDGET(cancel), TRUE, TRUE, 5);*/


        auto label = gtk_label_new(_("Use External Terminal:"));
        Basic::setTooltip(label, text);
        checkbutton_ = GTK_CHECK_BUTTON(gtk_check_button_new());

        Basic::boxPack0(GTK_BOX (box),GTK_WIDGET(label), FALSE, FALSE, 0);
        Basic::boxPack0(GTK_BOX (box),GTK_WIDGET(checkbutton_), FALSE, FALSE, 0);
        char *key = NULL;
        if (defaultApp){
          key = g_strdup(defaultApp);
          g_free (defaultApp);
        } else if (apps){
          key = g_strdup(apps[0]);
        }
        if (key) {
          if (Settings::getInteger("ExternalTerminal", key) == 1){
            gtk_check_button_set_active(checkbutton_, true);
          } else {
            gtk_check_button_set_active(checkbutton_, false);
          }
          g_free(key);
        }
      }
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
        Basic::setAsDialog(GTK_WINDOW(dialog_));
        gtk_window_present(dialog_);

        setRaise();
   }
    
    static void
    run(OpenWith *object){
       if (!Child::valid(object->child()) && object->withTextview()==false){
         ERROR_("Child widget (%p) is not valid, output=%p.\n",
             object->child(), object->output());
         return;
       }
        auto output = GTK_TEXT_VIEW(object->output());
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
        auto p = object->paths();
        auto e = Basic::esc_string (object->path());
        if (inTerminal) {
          command = Run<Type>::mkTerminalLine(inputText, p?p:e);
          Settings::setInteger("ExternalTerminal", key, 1);
        }
        else {
          command = Run<Type>::mkCommandLine(inputText, p?p:e);
          Settings::setInteger("ExternalTerminal", key, 0);
          TRACE("command line is \'%s\'\n", command);
        }
        g_free(p);
        g_free(e);
        g_free(key);
        if (object->withTextview()) {
          object->prompt_p->run(output, command, false, false, NULL);
        } else {
          object->prompt_p->run(output, command, true, true, object->buttonSpace);
        }
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
        if(keyval == GDK_KEY_Escape) { 
          auto dialog = object->dialog();
          auto mimetype = g_object_get_data(G_OBJECT(dialog), "mimetype");
          g_free(mimetype);
          object->freeSelectionList();
          object->timeout_=-1;
          return TRUE;
        }
        if(keyval == GDK_KEY_Return) { 
          run(object);
          object->freeSelectionList();
          auto dialog = object->dialog();
          auto mimetype = g_object_get_data(G_OBJECT(dialog), "mimetype");
          g_free(mimetype);
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
    
 /*   static void
    okTextview (GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data) {
        auto object = (OpenWith *)data;
        object->withTextview(true);
        run(object);
        object->timeout_=-1;
        //gtk_window_present(GTK_WINDOW(Child::mainWidget()));
    }*/
    
    static void
    ok (GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data) {
        auto object = (OpenWith *)data;
        run(object);
        object->freeSelectionList();
        object->timeout_=-1;
        gtk_window_present(GTK_WINDOW(Child::mainWidget()));
    }
    
    static void
    runIt (GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data) {
        OpenWith::dialogProceed(NULL, data);
    }
 

    static void
    cancelCallback (GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data) {
      auto object = (OpenWith *)data;
      object->freeSelectionList();
      object->timeout_=-1;
        gtk_window_present(GTK_WINDOW(Child::mainWidget()));
    }
    
    static void
    dialogProceed (GtkButton *button, void *data) {
        auto object = (OpenWith *)data;
        bool inTerminal = gtk_check_button_get_active(object->checkbutton());
        auto path = object->path();
        char *command = NULL;
        if (inTerminal) {
          command = Run<Type>::mkTerminalLine(path, "");
        }
        else {
          command = Run<Type>::mkCommandLine(path, "");
          TRACE("command line is \'%s\'\n", command);
        }
        if (command) 
          object->prompt_p->run(Child::getOutput(), command, true, true, object->buttonSpace);
        g_free(command);
        object->freeSelectionList();
        object->timeout_=-1;
        gtk_window_present(GTK_WINDOW(Child::mainWidget()));
    }
/*
    static void
    dialogCancel (GtkButton *button, void *data) {
      auto object = (OpenWith *)data;
      object->timeout_=-1;

//      delete object;
    }
*/
    static gboolean 
    dialogClose(GtkWindow *dialog, void *data){
      auto object = (OpenWith *)data;
      object->timeout_=-1;
      object->freeSelectionList();
        gtk_window_present(GTK_WINDOW(Child::mainWidget()));

      return TRUE;
    }
/*
    static void 
    dialogActivate(GtkEntry *entry, void *data){
      dialogProceed(NULL, data);
    }
*/
    static gboolean
    labelClick(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
      auto label = GTK_LABEL(data);
      auto object = (OpenWith *)g_object_get_data(G_OBJECT(label), "openWith");
      auto popover = GTK_POPOVER(g_object_get_data(G_OBJECT(label), "popover"));
      auto mimetype = (const char *)(g_object_get_data(G_OBJECT(popover), "mimetype"));
      auto text = gtk_label_get_text(label);
      auto key = g_strdup(text);
      //if (strchr(key, ' ')) *strrchr(key,' ') = 0;
      auto input = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(label), "input"));
      Print::clear_text(input);
      Print::print(input, g_strdup(text));
      if (Settings::getInteger("ExternalTerminal", key) == 1){
        gtk_check_button_set_active(object->checkbutton(), true);
      } else {
        gtk_check_button_set_active(object->checkbutton(), false);
      }
      TRACE("settings: MimeTypeApplications  %s %s\n", mimetype, key);
      Settings::setString("MimeTypeApplications", mimetype, key);
      g_free(key);
      

      gtk_popover_popdown(popover);
      gtk_widget_grab_focus(GTK_WIDGET(input));
      
      return TRUE;
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


};
}
#endif

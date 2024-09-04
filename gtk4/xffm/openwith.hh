#ifndef ENTRYRESPONSE_HH
#define ENTRYRESPONSE_HH

namespace xf {
template <class Type> class Prompt;
template <class Type> class Run;
template <class Type>
class OpenWith {
    
    time_t timeout_;
    time_t lastTimeout_;
    GtkLabel *responseLabel_;
    GtkLabel *entryLabel_;
    GtkLabel *checkboxLabel_;
    GtkEntry *entry_;
    GtkCheckButton *checkbutton_;
    //GtkEntryCompletion *bashCompletion_;
   
protected:
    char *path_;
    GtkButton *yes_;
    GtkButton *no_;
    GtkWindow *dialog_;
    GtkWindow *response_;
    GtkBox *hbox_;
    GtkBox *vbox2_;
    //GtkListStore *bashCompletionStore_;
    GtkProgressBar *timeoutProgress_;

    GtkLabel *comboLabel(void) {return entryLabel_;}
        
    void connectBashCompletion(const gchar *wd, GtkEntry *entry){
      //FIXME
      /*  g_object_set_data(G_OBJECT(entry), "workDir", (void *)wd);
        gtk_entry_set_completion (entry, bashCompletion_);
        g_signal_connect (G_OBJECT(entry),
                              "key_release_event", 
                              KEY_EVENT_CALLBACK(EntryResponse::onExecCompletion), 
                              (void *)bashCompletionStore_);*/
    }
        
    void connectBashFileCompletion(const gchar *wd, GtkEntry *entry){
      //FIXME
      /*  TRACE("connectBashFileCompletion(%s, %p)\n", wd, entry);
        g_object_set_data(G_OBJECT(entry), "workDir", (void *)wd);
        gtk_entry_set_completion (entry, bashCompletion_);
        g_signal_connect (G_OBJECT(entry),
                              "key_release_event", 
                              KEY_EVENT_CALLBACK(EntryResponse::onFileCompletion), 
                              (void *)bashCompletionStore_);*/
    }

public:
    GtkWindow *dialog(void){return dialog_;}
    GtkEntry *entry(void){return entry_;}

    time_t timeout(void){return timeout_;}
    void timeout(time_t value){timeout_ = value;}
    GtkProgressBar *progress(void){return timeoutProgress_;}
    GtkBox *vbox2(void) {return vbox2_;}
    
    
    ~OpenWith (void){
       gtk_widget_set_visible(GTK_WIDGET(dialog_), FALSE);
       g_free(path_);
       gtk_window_destroy(dialog_);
        //if (bashCompletionStore_) gtk_list_store_clear(bashCompletionStore_);
        //gtk_widget_destroy(GTK_WIDGET(response_));
    }



    OpenWith (GtkWindow *parent, const gchar *inPath){
      const gchar *windowTitle = _("Open With...");
      const gchar *icon = "emblem-run";

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
        // icon
        if (icon){
            auto paintable = Texture::load(icon, 48);
            auto image = gtk_image_new_from_paintable(paintable);
            gtk_widget_set_size_request(image, 48, 48);
            UtilBasic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(image), TRUE, TRUE, 0);
        }
        // path
        auto label = GTK_LABEL(gtk_label_new (""));
        auto markup = g_strconcat("<span color=\"blue\" size=\"large\"> <b>", path_, "</b></span>", NULL);
        gtk_label_set_markup(label, markup);
        g_free(markup);
        UtilBasic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(label), FALSE, FALSE, 0);
        // mimetype
        auto labelMime = GTK_LABEL(gtk_label_new (""));
        auto mimetype = MimeMagic::mimeMagic(path_); 
        auto markup2 = g_strconcat("<span color=\"brown\" size=\"small\">", mimetype, "</span>", NULL);
        gtk_label_set_markup(labelMime, markup2);
        g_free(markup2);
        g_free(mimetype);
        UtilBasic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(labelMime), FALSE, FALSE, 0);

        // entry
        auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        UtilBasic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(hbox), FALSE, FALSE, 0);

        auto label2 = GTK_LABEL(gtk_label_new (windowTitle));
        UtilBasic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(label2), FALSE, FALSE, 3);

        entry_ = GTK_ENTRY(gtk_entry_new ());
        UtilBasic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(entry_), TRUE, TRUE, 0);
        gtk_window_set_default_widget(dialog_, GTK_WIDGET(entry_));

        g_object_set_data(G_OBJECT(dialog_),"entry", entry_);
        g_signal_connect (G_OBJECT (entry_), "activate", 
                G_CALLBACK (OpenWith::dialogActivate), this);
       
        auto yes = UtilBasic::newButton ("object-select", _("Proceed"));
        UtilBasic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(yes), FALSE,FALSE, 0);
        g_object_set_data (G_OBJECT (dialog_), "yes", yes);
        g_signal_connect (G_OBJECT (yes), "clicked", G_CALLBACK (OpenWith::dialogProceed), this);

        // check button
        checkbutton_ = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Open in Terminal")));
        UtilBasic::boxPack0(GTK_BOX (vbox),GTK_WIDGET(checkbutton_), FALSE, FALSE, 0);

        // cancel button
        auto buttonBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(buttonBox), false);
        UtilBasic::boxPack0(GTK_BOX (vbox),GTK_WIDGET(buttonBox), TRUE, TRUE, 0);

        auto no = gtk_button_new();
        auto ball = gtk_image_new_from_icon_name("emblem-redball");
        auto bbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto blabel = gtk_label_new(_("Cancel"));
        UtilBasic::boxPack0(GTK_BOX (bbox),GTK_WIDGET(ball), FALSE, FALSE, 0);
        UtilBasic::boxPack0(GTK_BOX (bbox),GTK_WIDGET(blabel), FALSE, FALSE, 0);
        gtk_button_set_child(GTK_BUTTON(no), GTK_WIDGET(bbox));

        auto spacer = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(spacer), true);
        auto vspacer = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

        UtilBasic::boxPack0(GTK_BOX (buttonBox),GTK_WIDGET(spacer), TRUE, TRUE, 0);
        UtilBasic::boxPack0(buttonBox, GTK_WIDGET(vspacer), FALSE,FALSE, 0);
        UtilBasic::boxPack0(vspacer, GTK_WIDGET(no), FALSE,FALSE, 0);
        
        g_object_set_data (G_OBJECT (dialog_), "no", no);
        g_signal_connect (G_OBJECT (no), "clicked", G_CALLBACK (OpenWith::dialogCancel), this);

        // progress bar timeout       
        timeoutProgress_ = GTK_PROGRESS_BAR(gtk_progress_bar_new());
        UtilBasic::boxPack0 (GTK_BOX (vbox),GTK_WIDGET(timeoutProgress_), TRUE, TRUE, 0);
        //g_signal_connect (G_OBJECT (entry_), "key-press-event", G_CALLBACK (progressReset), timeoutProgress_);
        g_timeout_add(500, OpenWith::updateProgress, (void *)this);
 
    
        auto keyController = gtk_event_controller_key_new();
        gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(dialog_), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)timeoutProgress_);
    


        // finish up
        gtk_widget_realize (GTK_WIDGET(dialog_));
        gtk_widget_grab_focus(GTK_WIDGET(entry_));
        UtilBasic::setAsDialog(GTK_WIDGET(dialog_), "xfDialog", "XfDialog");
        gtk_window_present(dialog_);
        return;
    }
    
    static gboolean
    on_keypress (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){
       auto progress = (GtkProgressBar *)data;
        if (!progress || !GTK_IS_PROGRESS_BAR(progress)) return FALSE;
        gtk_progress_bar_set_fraction(progress, 0.0);
        /*if(event->keyval == GDK_KEY_Tab) { 
            gtk_editable_set_position(GTK_EDITABLE(w), -1);
            return TRUE;
        }*/
        return FALSE;
      
    }
    
    void unsetTimeout(void){
        lastTimeout_ = timeout_;
        DBG("timeoutProgress_ = %p\n", timeoutProgress_);
        gtk_progress_bar_set_fraction(timeoutProgress_, 0.0);
        timeout_ = 0;
    }

    void resetTimeout(void){
        timeout_ = lastTimeout_;
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

        // FIXME:

    /*void setInLineCompletion(gboolean state){
        gtk_entry_completion_set_popup_completion(bashCompletion_, !state);
        gtk_entry_completion_set_inline_completion(bashCompletion_, state);
        gtk_entry_completion_set_inline_selection(bashCompletion_, state);
    }*/
    
                              
    void setEntryBashCompletion(const gchar *wd){
        connectBashCompletion(wd, entry_);
    }
                              
    void setEntryBashFileCompletion(const gchar *wd){
        TRACE("setEntryBashFileCompletion(%s)\n", wd);
        connectBashFileCompletion(wd, entry_);
    }

    GtkCheckButton *checkButton(void){ return checkbutton_;}

    gboolean checkButtonState(void){
        return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_));
    }

    void setCheckButton(const gchar *text){
        gtk_check_button_set_label(checkbutton_, text);
    }

   
    void setCheckButton(gboolean state){
        // Set the toggle state.
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_), state);
    }
    void setCheckButtonEntryCallback(void *clickAction){
        // Set the toggle action.
        g_signal_connect (G_OBJECT (checkbutton_), "clicked", 
                    BUTTON_CALLBACK(clickAction), 
                    (void *)entry_);
        //gtk_widget_show(GTK_WIDGET(checkbutton_));
    }

    void setCheckButtonEntryCallback(void *clickAction, void * data){
        // Set the toggle action.
        g_signal_connect (G_OBJECT (checkbutton_), "clicked", 
                    BUTTON_CALLBACK(clickAction), 
                    data);
        //gtk_widget_show(GTK_WIDGET(checkbutton_));
    }
    void setEntryCallback(void *changeAction){
        // Set the toggle action.
        g_signal_connect (G_OBJECT (entry_), "key-release-event", 
                    G_CALLBACK(changeAction), this);
        //gtk_widget_show(GTK_WIDGET(entry_));
    }

/*
    static void // XXX Check for correct working in i3 multimonitor XXX
    placeDialog(GtkWindow *dialog){
        gtk_widget_realize(GTK_WIDGET(dialog));
        Display *display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
        Drawable w = gdk_x11_window_get_xid(gtk_widget_get_window(GTK_WIDGET(dialog)));
        Window root;
        Window child;
        gint mouseX, mouseY, childX, childY;
        guint rootW, rootH;
        guint mask;
        XQueryPointer(display, w, &root, &child, &mouseX, &mouseY, &childX, &childY, &mask);
        guint windowW,windowH;
        getWindowDimensions(w, &windowW, &windowH);
        getRootDimensions(&rootW, &rootH);
        TRACE("*** rootW,H= (%d,%d) window=(%d,%d) mouse=(%d,%d)\n", 
                rootW,rootH,windowW,windowH, mouseX,mouseY);
        if (mouseX+windowW > rootW) mouseX = rootW - windowW;
        if (mouseY+windowH > rootH) mouseY = rootH - windowH;
        TRACE("***2 rootW,H= (%d,%d) window=(%d,%d) mouse=(%d,%d)\n", 
                rootW,rootH,windowW,windowH, mouseX,mouseY);

        gtk_window_move(dialog, mouseX, mouseY);
    }
    */

private:

//FIXME
/*
    static gint
    onExecCompletion (GtkWidget * widget, GdkEventKey * event, gpointer data) {
        return on_completion (widget, event, data, FALSE) ;
    }
    static gint
    onFileCompletion (GtkWidget * widget, GdkEventKey * event, gpointer data) {
        return on_completion (widget, event, data, TRUE);
    }
    static gint
    on_completion (GtkWidget * widget, GdkEventKey * event, gpointer data, gboolean fileCompletion) 
    {
        TRACE("on_completion()\n");
        auto store = (GtkListStore *)data;
        // get entry text
        auto entry = GTK_ENTRY(widget);
        const gchar *text = gtk_entry_get_text(entry);
        if (!text || strlen(text)<2) return FALSE;

        // Determine if Terminal check button should be depressed
        auto checkButton = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(entry), "checkButton"));
        // FIXME gtk_toggle_button_set_active(checkButton, Mime<Type>::runInTerminal(text));
        // Hard coded exceptions:
        // nano vi and others...
        if (Run::fixedInTerminal(text)){
            gchar *a = Run::baseCommand(text);
            gtk_toggle_button_set_active(checkButton, TRUE);
            Settings::setInteger("Terminal", a, 1);
            g_free(a);
        }

        // Get GSlist of bash completion

        auto wd = (const gchar *)g_object_get_data(G_OBJECT(entry), "workDir");
        if (!wd) wd = g_get_home_dir();
        
        GSList *slist;
        if (fileCompletion) {
            slist = BaseCompletion::baseFileCompletionList(wd, text);
        } else {
            slist = BaseCompletion::baseExecCompletionList(wd, text);
        }
        // remove all old model entries
        gtk_list_store_clear(store);
        // add new entries from GSList
        GSList *p;
        GtkTreeIter iter;
        for (p=slist; p && p->data; p=p->next){
            TRACE("completion list: %s\n", (const gchar *)p->data);
            gtk_list_store_append (store, &iter);
            gtk_list_store_set(store, &iter, 0, (const gchar *)p->data, -1);
            g_free(p->data);
        }
        g_slist_free(slist);

        auto completion = gtk_entry_get_completion(GTK_ENTRY(widget));
        gtk_entry_completion_complete (completion);
        return FALSE;
    }
    */

    static void
    dialogProceed (GtkButton *button, void *data) {
        auto object = (OpenWith *)data;
        auto dialog = object->dialog();
        auto entry = object->entry();
        auto path = object->path_;
        auto buffer = gtk_entry_get_buffer(entry);
        auto text = gtk_entry_buffer_get_text(buffer);
        auto output = Child::getCurrentOutput();
        auto buttonSpace = Child::getCurrentButtonSpace();

        auto command = Run<Type>::mkCommandLine(text, path);
        DBG ("run %s \n", command); 
        auto line = g_strconcat(command, "\n", NULL);
        Print::print(output, line);
        //Run<Type>::shell_command(output, command, false, false);
        Prompt<Type>::run(output, command, true, true, buttonSpace);
        
        gtk_widget_set_visible(GTK_WIDGET(dialog), false);

        g_free(command);
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

  


};
}
#endif

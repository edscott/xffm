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
    
   
protected:
    GtkWindow *dialog(void){return dialog_;}
    time_t timeout(void){return timeout_;}
    void timeout(time_t value){timeout_ = value;}
    GtkProgressBar *progress(void){return timeoutProgress_;}
    GtkCheckButton *checkbutton(void){ return checkbutton_;}
    GtkTextView *input(void){ return input_;}
        

public:
    
    ~OpenWith (void){
       gtk_widget_set_visible(GTK_WIDGET(dialog_), FALSE);
       g_free(path_);
       gtk_window_destroy(dialog_);
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
      // icon title
      if (icon){
          auto paintable = Texture::load(icon, 48);
          auto image = gtk_image_new_from_paintable(paintable);
          gtk_widget_set_size_request(image, 48, 48);
          UtilBasic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(image), TRUE, TRUE, 0);
      }
      // path title
      auto label = GTK_LABEL(gtk_label_new (""));
      auto markup = g_strconcat("<span color=\"blue\" size=\"large\"> <b>", path_, "</b></span>", NULL);
      gtk_label_set_markup(label, markup);
      g_free(markup);
      UtilBasic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(label), FALSE, FALSE, 0);
      // mimetype title
      auto labelMime = GTK_LABEL(gtk_label_new (""));
      auto mimetype = MimeMagic::mimeMagic(path_); 
      auto markup2 = g_strconcat("<span color=\"brown\" size=\"small\">", mimetype, "</span>", NULL);
      gtk_label_set_markup(labelMime, markup2);
      g_free(markup2);
      g_free(mimetype);
      UtilBasic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(labelMime), FALSE, FALSE, 0);

      // prompt

      auto child = Child::getCurrentChild();
      auto prompt_p = (Prompt<Type> *) new Prompt<Type>(child);
      auto hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
      gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
      auto label2 = GTK_LABEL(gtk_label_new (windowTitle));
      input_ = prompt_p->input();
   /*     char *size = Settings::getString("xfterm", "size");
        if (!size) size = g_strdup("font4"); // medium
        gtk_widget_add_css_class (GTK_WIDGET(input_), size );
        gtk_text_view_set_pixels_above_lines (input_, 5);
        gtk_text_view_set_pixels_below_lines (input_, 5);
        gtk_text_view_set_monospace (input_, TRUE);
        gtk_text_view_set_editable (input_, TRUE);
        gtk_text_view_set_cursor_visible (input_, TRUE);
        gtk_text_view_place_cursor_onscreen(input_);
        gtk_text_view_set_wrap_mode (input_, GTK_WRAP_CHAR);
        gtk_widget_set_can_focus(GTK_WIDGET(input_), TRUE);
*/
      
      auto mimeButton = gtk_button_new_from_icon_name("go-down");
      UtilBasic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(label2), FALSE, FALSE, 3);
      UtilBasic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(input_), TRUE, TRUE, 3);
      UtilBasic::boxPack0(GTK_BOX (hbox), GTK_WIDGET(mimeButton), FALSE, FALSE, 3);
      UtilBasic::boxPack0(GTK_BOX (vbox), GTK_WIDGET(hbox), TRUE, TRUE, 0);

        // check button
        checkbutton_ = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(_("Open in Terminal")));
        UtilBasic::boxPack0(GTK_BOX (vbox),GTK_WIDGET(checkbutton_), FALSE, FALSE, 0);

        // button box
        auto buttonBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(buttonBox), false);
        UtilBasic::boxPack0(GTK_BOX (vbox),GTK_WIDGET(buttonBox), TRUE, TRUE, 0);
        auto spacer = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(spacer), true);
        auto rTgt = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        auto gTgt = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        UtilBasic::boxPack0(GTK_BOX (buttonBox),GTK_WIDGET(spacer), TRUE, TRUE, 0);
        UtilBasic::boxPack0(buttonBox, GTK_WIDGET(rTgt), FALSE,FALSE, 0);
        UtilBasic::boxPack0(buttonBox, GTK_WIDGET(gTgt), FALSE,FALSE, 0);

        // no button

        auto no = gtk_button_new();
        auto red = gtk_image_new_from_icon_name("emblem-redball");
        auto rbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto rlabel = gtk_label_new(_("Cancel"));
        UtilBasic::boxPack0(GTK_BOX (rbox),GTK_WIDGET(red), FALSE, FALSE, 0);
        UtilBasic::boxPack0(GTK_BOX (rbox),GTK_WIDGET(rlabel), FALSE, FALSE, 0);
        gtk_button_set_child(GTK_BUTTON(no), GTK_WIDGET(rbox));
        g_signal_connect (G_OBJECT (no), "clicked", G_CALLBACK (OpenWith::dialogCancel), this);
        UtilBasic::boxPack0(rTgt, GTK_WIDGET(no), FALSE,FALSE, 0);

       // yes button
        auto yes = gtk_button_new();
        auto green = gtk_image_new_from_icon_name("emblem-greenball");
        auto gbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto glabel = gtk_label_new(_("Proceed"));
        UtilBasic::boxPack0(GTK_BOX (gbox),GTK_WIDGET(green), FALSE, FALSE, 0);
        UtilBasic::boxPack0(GTK_BOX (gbox),GTK_WIDGET(glabel), FALSE, FALSE, 0);
        gtk_button_set_child(GTK_BUTTON(yes), GTK_WIDGET(gbox));
        g_signal_connect (G_OBJECT (yes), "clicked", G_CALLBACK (OpenWith::dialogProceed), this);
        UtilBasic::boxPack0(gTgt, GTK_WIDGET(yes), FALSE,FALSE, 0);
 
        // progress bar timeout       
        timeoutProgress_ = GTK_PROGRESS_BAR(gtk_progress_bar_new());
        UtilBasic::boxPack0 (GTK_BOX (vbox),GTK_WIDGET(timeoutProgress_), TRUE, TRUE, 0);
        g_timeout_add(500, OpenWith::updateProgress, (void *)this);
 
    
        auto keyController = gtk_event_controller_key_new();
        gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(dialog_), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-pressed", 
            G_CALLBACK (this->on_keypress), (void *)this);
    


        // finish up
        gtk_widget_realize (GTK_WIDGET(dialog_));
        gtk_widget_grab_focus(GTK_WIDGET(input_));
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

       auto object = (OpenWith *)data;
       auto progress = object->progress();
        if (!progress || !GTK_IS_PROGRESS_BAR(progress)) return FALSE;
        gtk_progress_bar_set_fraction(progress, 0.0);
        if(keyval == GDK_KEY_Return) { 
          object->timeout_=-1;
        }
        if(keyval == GDK_KEY_Tab) { 
          auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(object->input()), "output"));
          DBG("showText %p\n", output);
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
      /*
        auto object = (OpenWith *)data;
        auto dialog = object->dialog();
        //auto entry = object->entry();
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
        */
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

#ifndef UTILBASIC_HH
#define UTILBASIC_HH

namespace xf {

  class UtilBasic 
  {
    public:

    static GtkButton *mkButton(const char *iconName, const char *markup){
      auto button = gtk_button_new();
      auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      if (iconName){
        auto image = GTK_WIDGET(Texture<bool>::getImage(iconName, 18));
        //auto image = gtk_image_new_from_icon_name(iconName);
        gtk_box_append (box, GTK_WIDGET(image));
      }
      if (markup){
        auto label = gtk_label_new("");
        auto g = g_strdup_printf("  %s", markup);
        gtk_label_set_markup(GTK_LABEL(label), g);
        g_free(g);
        gtk_box_append (box, GTK_WIDGET(label));
      }
      gtk_button_set_child(GTK_BUTTON(button), GTK_WIDGET(box));
      return GTK_BUTTON(button);
    }
     
    static GtkBox *imageButton(int width, int height, const char *iconName, const char *tooltipText, void *callback, void *data){
        auto toggleBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL,5));
        gtk_widget_set_size_request(GTK_WIDGET(toggleBox), width, height);
        GtkWidget *toggle;
        if (iconName){
          toggle = GTK_WIDGET(Texture<bool>::getImage(iconName, height-4));
          if (tooltipText) {
            Basic::setTooltip(GTK_WIDGET(toggle), tooltipText);
          }
        } else {
          toggle = gtk_label_new("");
        }
        gtk_box_prepend(toggleBox, GTK_WIDGET(toggle));
        gtk_widget_add_css_class (GTK_WIDGET(toggleBox), "input" );
        gtk_widget_add_css_class (GTK_WIDGET(toggle), "input" );
        if (callback) {
          auto gesture = gtk_gesture_click_new();
          gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1);
          gtk_widget_add_controller(GTK_WIDGET(toggleBox), GTK_EVENT_CONTROLLER(gesture));
          gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
              GTK_PHASE_CAPTURE);
          g_signal_connect (G_OBJECT(gesture) , "pressed", G_CALLBACK (callback), data);
        }
       
        auto controllerIn = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controllerIn, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(toggleBox), controllerIn);
        g_signal_connect (G_OBJECT (controllerIn), "enter", 
            G_CALLBACK (buttonMotion), GINT_TO_POINTER(1));
       
        auto controllerOut = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controllerOut, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(toggleBox), controllerOut);
        g_signal_connect (G_OBJECT (controllerOut), "leave", 
            G_CALLBACK (buttonMotion), NULL);
        
        return toggleBox;
    }
   
    static GtkBox *imageButton(const char *iconName, const char *tooltipText, void *callback, void *data){
      return imageButton(30,16,iconName,tooltipText,callback,data);
    }
    static GtkBox *imageButtonText(const char *iconName, 
        const char *markup, void *callback, void *data)
    {
        auto toggleBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5));
        //gtk_widget_set_size_request(GTK_WIDGET(toggleBox), -1, 16);
        GtkWidget *toggle;
        if (iconName){
          toggle = GTK_WIDGET(Texture<bool>::getImage(iconName, 12));
        } else {
          toggle = gtk_label_new("");
        }
        gtk_box_prepend(toggleBox, GTK_WIDGET(toggle));
        gtk_widget_add_css_class (GTK_WIDGET(toggleBox), "input" );
        gtk_widget_add_css_class (GTK_WIDGET(toggle), "input" );
        auto gesture = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),1);
        gtk_widget_add_controller(GTK_WIDGET(toggleBox), GTK_EVENT_CONTROLLER(gesture));
        gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
            GTK_PHASE_CAPTURE);
        g_signal_connect (G_OBJECT(gesture) , "pressed", G_CALLBACK (callback), data);
       
        auto controllerIn = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controllerIn, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(toggleBox), controllerIn);
        g_signal_connect (G_OBJECT (controllerIn), "enter", 
            G_CALLBACK (buttonMotion), GINT_TO_POINTER(1));
       
        auto controllerOut = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controllerOut, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(toggleBox), controllerOut);
        g_signal_connect (G_OBJECT (controllerOut), "leave", 
            G_CALLBACK (buttonMotion), NULL);
        auto label = gtk_label_new("");
        gtk_label_set_markup(GTK_LABEL(label), markup);
        gtk_box_append(toggleBox, label);
       
        return toggleBox;
    }

    private:
    static gboolean buttonMotion( GtkEventControllerMotion* self,
                    double x, double y, void *data) {
      auto controller = GTK_EVENT_CONTROLLER(self);
      auto widget = gtk_event_controller_get_widget(controller);
      if (data) {
        gtk_widget_remove_css_class (GTK_WIDGET(widget), "input" );
        gtk_widget_add_css_class (GTK_WIDGET(widget), "pathbarboxNegative" );
      } else {
        gtk_widget_remove_css_class (GTK_WIDGET(widget), "pathbarboxNegative" );
        gtk_widget_add_css_class (GTK_WIDGET(widget), "input" );
      }
      return TRUE;
    }

    public:

    static void setFontCss(GtkWidget *widget){
      auto size = Settings::getInteger("xfterm", "fontcss", 3);
      if (size > 7) {
        size=7;
        Settings::setInteger("xfterm", "fontcss", 7);
      }
        // FIXME leak: css      
      auto css = g_strdup_printf("font%d", size);
      gtk_widget_add_css_class (widget, css);
      g_object_set_data(G_OBJECT(widget), "css", (void *)css);
    }
      
    static
    GtkTextView *createInput(void){
        GtkTextView *input = GTK_TEXT_VIEW(gtk_text_view_new ());
        
        gtk_text_view_set_pixels_above_lines (input, 5);
        gtk_text_view_set_pixels_below_lines (input, 5);
        gtk_text_view_set_monospace (input, TRUE);
        gtk_text_view_set_editable (input, TRUE);
        gtk_text_view_set_cursor_visible (input, TRUE);
        gtk_text_view_place_cursor_onscreen(input);
        gtk_text_view_set_wrap_mode (input, GTK_WRAP_CHAR);
        gtk_widget_set_can_focus(GTK_WIDGET(input), TRUE);
        
        gtk_widget_add_css_class (GTK_WIDGET(input), "input" );
        gtk_widget_add_css_class (GTK_WIDGET(input), "inputview" );
        
        UtilBasic::setFontCss(GTK_WIDGET(input));
       
        return input;
    }

    private:


 
    public:
    static GtkBox *
    pathbarLabelButton (const char *text) {
        auto label = GTK_LABEL(gtk_label_new(""));
        gtk_widget_set_vexpand(GTK_WIDGET(label), false);
        gtk_widget_set_hexpand(GTK_WIDGET(label), false);

        auto eventBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(eventBox), false);
        gtk_widget_set_hexpand(GTK_WIDGET(eventBox), false);

        if (text) {
            auto v = Basic::utf_string(text);
            auto g = g_markup_escape_text(v, -1);
            g_free(v);
            auto markup = g_strdup_printf(" %s ", g);
            g_free(g);
            gtk_label_set_markup(label, markup);
            g_free(markup);
        } else {
            gtk_label_set_markup(label, "");
        }
        gtk_box_append(eventBox, GTK_WIDGET(label));
        g_object_set_data(G_OBJECT(eventBox), "label", label);
        // FIXME leak:
        g_object_set_data(G_OBJECT(eventBox), "name", text?g_strdup(text):g_strdup("RFM_ROOT"));
        return eventBox;
    }
 
#if 0
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
      g_object_set_data(G_OBJECT(menu), "vbox", vbox);
      setMenuTitle(menu, title);

      gtk_popover_set_autohide(GTK_POPOVER(menu), TRUE);
      gtk_popover_set_has_arrow(GTK_POPOVER(menu), true);
      gtk_widget_add_css_class (GTK_WIDGET(menu), "inquire" );


      for (const char **p=text; p && *p; p++){
        auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto label = GTK_LABEL(gtk_label_new(""));
        gtk_label_set_markup(label, *p);
        auto icon = (const char *) g_hash_table_lookup(mHash[0], *p);
        if (icon){
          auto image = gtk_image_new_from_icon_name(icon);
          Basic::boxPack0(hbox, GTK_WIDGET(image),  FALSE, FALSE, 0);
        }
        Basic::boxPack0(hbox, GTK_WIDGET(label),  FALSE, FALSE, 5);

        if (GPOINTER_TO_INT(g_hash_table_lookup(mHash[2], *p)) == -1) {
          Basic::boxPack0(vbox, GTK_WIDGET(hbox),  FALSE, FALSE, 0);
          g_object_set_data(G_OBJECT(menu), *p, hbox);
          gtk_widget_set_visible(GTK_WIDGET(hbox), TRUE);
          continue;
        } else {
          GtkButton *button = GTK_BUTTON(gtk_button_new());
          g_object_set_data(G_OBJECT(button), "menu", menu);
          gtk_button_set_child(GTK_BUTTON(button), GTK_WIDGET(hbox));
          Basic::boxPack0(vbox, GTK_WIDGET(button),  FALSE, FALSE, 0);
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

#endif
    
    static gchar *fileInfo(const gchar *path){
        gchar *file = g_find_program_in_path("file");
        if (!file) return g_strdup("\"file\" command not in path!");
        gchar *result = NULL; 
        gchar *command = g_strdup_printf("%s \'%s\'", file, path);
        result = pipeCommand(command);
        g_free(command);
        g_free(file);

        if (result){
            gchar *retval;
            if (strchr(result, ':')) retval=g_strdup(strchr(result, ':')+1);
            else retval = g_strdup(result);
            g_free(result);
            gchar *p=retval;
            for(;p && *p; p++) {
                if (*p == '<') *p='[';
                else if (*p == '>') *p=']';
            }
            lineBreaker(retval, 40);
            return retval;
        }
        return NULL;
    }
#define PAGE_LINE 256

    static gchar *pipeCommand(const gchar *command){
        FILE *pipe = popen (command, "r");
        if(pipe) {
            gchar line[PAGE_LINE];
            line[PAGE_LINE - 1] = 0;
            if (!fgets (line, PAGE_LINE - 1, pipe)){
                  TRACE("fgets(%s): %s\n", command, "no characters read.");
            } else {
              if (strchr(line, '\n'))*(strchr(line, '\n'))=0;
            }
            pclose (pipe);
            return g_strdup(line);
        } 
        return NULL;
    }

    static gchar *pipeCommandFull(const gchar *command){
        FILE *pipe = popen (command, "r");
        if(pipe) {
            gchar *result=g_strdup("");
            gchar line[PAGE_LINE];
            while (fgets (line, PAGE_LINE - 1, pipe) && !feof(pipe)){
                line[PAGE_LINE - 1] = 0;
                auto g = g_strconcat(result, line, NULL);
                g_free(result);
                result = g;
            }
            pclose (pipe);
            return result;
        } 
        return NULL;
    }

    static void 
    lineBreaker(gchar *inputLine, gint lineLength){
        if (strlen(inputLine) > lineLength){
            gchar *remainder;
            gchar *p = inputLine+lineLength;
            do{
                if (*p ==' ' || *p =='_') {
                    *p = '\n';
                    remainder = p+1;
                    break;
                }
                p++;
            } while (*p);
            if (*p != 0) lineBreaker(remainder, lineLength);
        }
    }
    
    static gboolean backupType(const gchar *file){
        if (!file) return FALSE;
        // GNU backup type:
         if(file[strlen (file) - 1] == '~' || 
                 file[strlen (file) - 1] == '%'|| 
                 file[strlen (file) - 1] == '#') return TRUE;
        // MIME backup type:
        const gchar *e = strrchr(file, '.');
        if (e){
            if (strcmp(e,".old")==0) return TRUE;
            else if (strcmp(e,".bak")==0) return TRUE;
            else if (strcmp(e,".sik")==0) return TRUE;
        }
        return FALSE;
    }

   
  };
}
#endif


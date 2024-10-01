#ifndef UTILBASIC_HH
#define UTILBASIC_HH

namespace xf {

  class UtilBasic 
  {
    public:
      
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
        
        return input;
    }

    private:


 
    public:
    static GtkBox *
    pathbarLabelButton (const char *text) {
        auto label = GTK_LABEL(gtk_label_new(""));
        auto eventBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        if (text) {
            auto v = Basic::utf_string(text);
            auto g = g_markup_escape_text(v, -1);
            g_free(v);
            auto markup = g_strdup_printf("   <span size=\"small\">  %s  </span>   ", g);
            g_free(g);
            gtk_label_set_markup(label, markup);
            g_free(markup);
        } else {
            gtk_label_set_markup(label, "");
        }
        Basic::boxPack0 (eventBox, GTK_WIDGET(label), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(eventBox), "label", label);
        g_object_set_data(G_OBJECT(eventBox), "name", text?g_strdup(text):g_strdup("RFM_ROOT"));
        return eventBox;
    }
 
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
      gtk_popover_set_has_arrow(GTK_POPOVER(menu), FALSE);
      gtk_widget_add_css_class (GTK_WIDGET(menu), "inquire" );


      for (const char **p=text; p && *p; p++){
        auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto label = GTK_LABEL(gtk_label_new(""));
        gtk_label_set_markup(label, *p);
        auto icon = (const char *) g_hash_table_lookup(mHash[0], *p);
        //DBG("icon is %s\n",icon);
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
                  DBG("fgets(%s): %s\n", command, "no characters read.");
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


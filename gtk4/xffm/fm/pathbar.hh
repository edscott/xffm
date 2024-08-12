#ifndef PATHBAR_HH
#define PATHBAR_HH
namespace xf {
  class Pathbar
  {
    GtkBox *pathbar_;

  public:
   GtkBox *pathbar(void){return pathbar_;} 
   ~Pathbar(void){}
   Pathbar(void) {
        pathbar_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

        auto eventBox1 = eventButton("previous", "RFM_GOTO", "xffm:back", (void *)go_back);
        auto eventBox2 = eventButton("jump", "RFM_GOTO", "xffm:goto", (void *)go_jump);

        gtk_widget_set_tooltip_markup(GTK_WIDGET(eventBox1),_("Previous"));
        gtk_widget_set_tooltip_markup(GTK_WIDGET(eventBox2),_("Go to"));

        gtk_widget_add_css_class (GTK_WIDGET(eventBox1), "pathbarbox" );
        gtk_widget_add_css_class (GTK_WIDGET(eventBox2), "pathbarbox" );

        Util::boxPack0 (GTK_BOX (pathbar_), GTK_WIDGET(eventBox2), FALSE, FALSE, 0);
        Util::boxPack0 (GTK_BOX (pathbar_), GTK_WIDGET(eventBox1), FALSE, FALSE, 0);


        // xffm:root button:
        auto pb_button = pathbarLabelButton(".");

        
        Util::boxPack0 (GTK_BOX (pathbar_), GTK_WIDGET(pb_button), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(pb_button), "name", g_strdup("RFM_ROOT"));
        g_object_set_data(G_OBJECT(pb_button), "path", g_strdup("xffm:root"));

    // FIXME : this iluminate background of "button".
    /*    
        g_signal_connect (G_OBJECT(pb_button) , "button-press-event", EVENT_CALLBACK (pathbar_go), (void *)this);
        g_signal_connect (G_OBJECT(pb_button) , "enter-notify-event", EVENT_CALLBACK (pathbar_white), (void *)this);
        g_signal_connect (G_OBJECT(pb_button) , "leave-notify-event", EVENT_CALLBACK (pathbar_blue), (void *)this);
        */
        
        //gtk_widget_show(GTK_WIDGET(pb_button));

    }

  private:
    GtkBox *
    pathbarLabelButton (const char *text) {
        auto label = GTK_LABEL(gtk_label_new(""));
        auto eventBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        if (text) {
            auto v = Util::utf_string(text);
            auto g = g_markup_escape_text(v, -1);
            g_free(v);
            auto markup = g_strdup_printf("   <span size=\"small\">  %s  </span>   ", g);
            g_free(g);
            gtk_label_set_markup(label, markup);
            g_free(markup);
        } else {
            gtk_label_set_markup(label, "");
        }
        Util::boxPack0 (eventBox, GTK_WIDGET(label), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(eventBox), "label", label);
        g_object_set_data(G_OBJECT(eventBox), "name", text?g_strdup(text):g_strdup("RFM_ROOT"));
        return eventBox;
    }
    
    GtkBox *eventButton(const gchar *icon, const gchar *name, 
            const gchar *path, void *callback) 
    {
        auto eventBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        g_object_set_data(G_OBJECT(eventBox), "name", g_strdup(name));
        g_object_set_data(G_OBJECT(eventBox), "path", g_strdup(path));

        auto eventImage = gtk_image_new_from_icon_name(icon);
        Util::boxPack0 (eventBox, GTK_WIDGET(eventImage), FALSE, FALSE, 0);
        // FIXME:
        // g_signal_connect (G_OBJECT(eventBox) , "button-press-event", EVENT_CALLBACK (callback), (void *)this);
        return eventBox;        
    }

     static gboolean
    go_back (GtkWidget *eventBox,
               GdkEvent  *event,
               gpointer   data) {
      //FIXME
/*        Pathbar *pathbar_p = (Pathbar *)data;
        auto page = (Page<Type> *)pathbar_p;
        auto view = (View<Type> *)
                g_object_get_data(G_OBJECT(page->topScrolledWindow()), "view");
        view->goBack();*/
        return FALSE;
    }

    static gboolean
    go_jump (GtkWidget *eventBox,
               GdkEvent  *event,
               gpointer   data) {
      //FIXME
/*        Pathbar *pathbar_p = (Pathbar *)data;
        // File chooser
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Go to"), "go-jump");
        auto markup = 
            g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", _("Go to"));  
        
        entryResponse->setResponseLabel(markup);
        g_free(markup);

        entryResponse->setEntryLabel(_("Specify Output Directory..."));
        // get last used arguments...
        gchar *dirname = NULL;
        if (Settings<Type>::keyFileHasGroupKey("GoTo", "Default")){
            dirname = Settings<Type>::getString("GoTo", "Default");
        } 
        if (!dirname || !g_file_test(dirname, G_FILE_TEST_IS_DIR) ) {
            g_free(dirname);
            dirname = g_strdup("");
        }
        entryResponse->setEntryDefault(dirname);
        g_free(dirname);
        auto page = (Page<Type> *)pathbar_p;
        const gchar *wd = page->workDir();
        if (!wd) wd = g_get_home_dir();
        entryResponse->setEntryBashCompletion(wd);
        entryResponse->setInLineCompletion(TRUE);
        
        auto response = entryResponse->runResponse();
        TRACE("response=%s\n", response);
        
        if (!response) return FALSE;
        if (strlen(response) > 1 && response[strlen(response)-1] == G_DIR_SEPARATOR){
            response[strlen(response)-1] = 0;
        }
        if (!g_file_test(response, G_FILE_TEST_IS_DIR)){
            gchar *message = g_strdup_printf("\n  %s:  \n  %s  \n", response, _("Not a directory"));
            Dialogs<Type>::quickHelp(GTK_WINDOW(mainWindow), message, "dialog-error");
            g_free(message);
        } else {
            auto view = (View<Type> *)
                g_object_get_data(G_OBJECT(page->topScrolledWindow()), "view");
            view->loadModel(response);
        }
        g_free(response);*/
        return FALSE;

    }

    

  };
}
#endif

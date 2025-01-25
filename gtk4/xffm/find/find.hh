#ifndef FIND__HH
# define FIND__HH
//
//#include "fgr.hh"
//#include "dialog.hh"
//
//msgid "Show search results for this query"
//msgid "Clear the search results."
//msgid "Show my search results"
typedef struct opt_t{
  const char *text;
  const char *id;
  gboolean defaultValue;
} opt_t;

namespace xf
{
    template <class Type>
    class FindResponse {
      using dialog_t = DialogComplex<FindResponse>;
   
      GtkBox *mainBox_ = NULL;
      GtkBox *topPaneVbox_ = NULL;
      GtkBox *topPaneHbox_ = NULL;
      GtkPaned *vpane_ = NULL;
      GtkWindow *findDialog = NULL;
      //GtkBox *mainVbox_ = NULL;

public:

      FindResponse (void){
      }

      ~FindResponse (void){
      }

     
      const char *title(void){ return _("Find files");}
      const char *iconName(void){ return EMBLEM_FIND;}

       static void *asyncYes(void *data){
        auto dialogObject = (dialog_t *)data;
        DBG("%s", "hello world asyncYes\n");
        return NULL;
      }

      static void *asyncNo(void *data){
        auto dialogObject = (dialog_t *)data;
        DBG("%s", "goodbye world asyncNo\n");
        return NULL;
      }

      GtkBox *mainBox(void) {
          mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 550, 400);
          gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
          gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);
          gtk_window_set_child(findDialog, GTK_WIDGET(mainVbox_));
          mkVpane();

          mkPathEntry();
          // so far today saturday 2025-01-25 8:22
          // continue tomorrow or later...
          // 
          mkFilterEntry();
          ////////////////  grep options.... /////////////////////////
          mkGrepEntry();
         
          mkButtonBox(); 

          gtk_widget_realize(GTK_WIDGET(findDialog));
       
      }

private:
          
      void mkVpane(void){

          topPaneVbox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 2));
          gtk_widget_set_vexpand(GTK_WIDGET(topPaneVbox_), FALSE);
          
          mkTopPaneHbox();
          gtk_box_append(topPaneVbox_, GTK_WIDGET(topPaneHbox_);
          //compat<bool>::boxPack0 (topPaneVbox_, GTK_WIDGET(topPaneHbox_), FALSE, TRUE, 0);

          vpane_ = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
          
          // Cursor does not change over handle in OpenBSD,
          // so now we default to wide handle. (legacy gtk3. In gtk4?)
          gtk_paned_set_wide_handle (vpane_,TRUE);
          
          g_object_set_data(G_OBJECT(findDialog), "vpane", (gpointer)vpane_);
          gtk_box_append(mainBox_, GTK_WIDGET(vpane_);
          //compat<bool>::boxPack0 (mainBox_, GTK_WIDGET(vpane_), TRUE, TRUE, 0);
          // hack: widgets_p->paper = findDialog;
          //gtk_container_set_border_width (GTK_CONTAINER (topPaneVbox_), 5);



          auto sw = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
          gtk_paned_set_start_child (vpane_, GTK_WIDGET(sw));
          //gtk_paned_pack1 (GTK_PANED (vpane_), GTK_WIDGET (sw), FALSE, TRUE);
          gtk_scrolled_window_set_child(sw, GTK_WIDGET(topPaneVbox_));
          //gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(topPaneVbox_));
          gtk_scrolled_window_set_policy (sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

          auto diagnostics = GTK_TEXT_VIEW(gtk_text_view_new ());
          g_object_set_data(G_OBJECT(findDialog), "diagnostics", (gpointer)diagnostics);
          g_object_set_data(G_OBJECT(diagnostics), "vpane", (gpointer)vpane_);
          
          auto sw2 = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
          gtk_paned_set_end_child (vpane_, GTK_WIDGET(sw2));
          //gtk_paned_pack2 (GTK_PANED (vpane_), GTK_WIDGET(sw2), FALSE, TRUE);
          //gtk_paned_pack2 (GTK_PANED (vpane_), GTK_WIDGET(scrolledwindow), TRUE, TRUE);
          gtk_scrolled_window_set_policy (sw2, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
          gtk_scrolled_window_set_child(sw2, GTK_WIDGET(diagnostics));
          //gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET(diagnostics));

          //gtk_container_set_border_width (GTK_CONTAINER (diagnostics), 2);
          gtk_widget_set_can_focus(GTK_WIDGET(diagnostics), FALSE);
          gtk_text_view_set_wrap_mode (diagnostics, GTK_WRAP_WORD);
          gtk_text_view_set_cursor_visible (diagnostics, FALSE);
           
          return ;
      }

      void mkTopPaneHbox(){
          topPaneHbox_= gtk_c::hboxNew(FALSE, 0);
          gtk_widget_set_hexpand(GTK_WIDGET(topPaneHbox_), TRUE);

          auto vbox1 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          auto vbox2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
          auto vbox3 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

          ////////////   closeButton... obsolete /////////////////////////
          /*auto closeButton =  Basic::mkButton gtk_c::dialog_button(WINDOW_CLOSE, "");

          g_object_set_data(G_OBJECT(findDialog), "close_button", closeButton);
          g_object_set_data(G_OBJECT(closeButton), "findDialog", findDialog);
          g_signal_connect (G_OBJECT (closeButton), "clicked",
                  BUTTON_CALLBACK(Type::onCloseButton), (gpointer)findDialog);
          compat<bool>::boxPack0 (vbox2, GTK_WIDGET(closeButton), TRUE, TRUE, 0);
          auto h = g_strconcat(_("Close find"), "\n\n", NULL); 
          tooltip_c::custom_tooltip(GTK_WIDGET(closeButton), NULL, h);
          g_free(h);*/

          ////////////   findButton... /////////////////////////
          auto findButton =   Basic::mkButton gtk_c::dialog_button(iconName(), "");
          gtk_widget_set_can_default(GTK_WIDGET(findButton), TRUE);
          g_signal_connect (G_OBJECT (findButton), "clicked",
                  BUTTON_CALLBACK(Type::onFindButton), (gpointer)findDialog);
          gtk_box_append(vbox3, GTK_WIDGET(findButton));

          gtk_widget_set_tooltip_markup(GTK_WIDGET(findButton), _("Show search results for this query"));

          ////////////   advanced options... FIXME /////////////////////////
#if 0
          auto advancedDialog = advancedOptions();
          gtk_window_set_transient_for(GTK_WINDOW(advancedDialog), GTK_WINDOW(findDialog));
         


          auto advancedButton = gtk_c::toggle_button(DOCUMENT_PROPERTIES, NULL);
  //        auto advancedButton = gtk_c::toggle_button(NULL, _("Details"));
          auto h3 = g_strconcat(_("Details"), "\n\n", NULL); 
          tooltip_c::custom_tooltip(GTK_WIDGET(advancedButton), NULL, h3);
          g_free(h3);
          g_object_set_data(G_OBJECT(findDialog), "advancedButton", advancedButton);
          g_object_set_data(G_OBJECT(findDialog), "advancedDialog", advancedDialog);
          compat<bool>::boxPack0 (vbox1, GTK_WIDGET(advancedButton), TRUE, TRUE, 5);
          g_signal_connect (advancedButton,
                            "clicked", WIDGET_CALLBACK(Type::onDetails), 
                            (gpointer)findDialog);
#endif
          auto t=g_strdup_printf("<span color=\"blue\" size=\"large\"><b>%s</b></span>  ", _("Find in Files"));
          auto label = GTK_LABEL(gtk_label_new (t));
          gtk_label_set_use_markup (label, TRUE);
          g_free(t);

          gtk_box_append(topPaneHbox_, GTK_WIDGET(vbox1));
          gtk_box_append(topPaneHbox_, GTK_WIDGET(label));
          gtk_box_append(topPaneHbox_, GTK_WIDGET(vbox3));
          //compat<bool>::boxPack0 (topPaneHbox_, GTK_WIDGET(vbox1), FALSE, FALSE, 0);
         // compat<bool>::boxPack0 (topPaneHbox_, GTK_WIDGET(label), TRUE, TRUE, 0);
          //compat<bool>::boxPack0 (topPaneHbox_, GTK_WIDGET(vbox3), FALSE, FALSE, 0);


      }
      void mkPathEntry(void){
          GtkWidget *path_label;

        /*  gchar *default_path=NULL;
          if (path) default_path = g_strdup(path);*/

          auto path_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_widget_set_hexpand(GTK_WIDGET(path_box), TRUE);
          gtk_box_append(topPaneVbox_, GTK_WIDGET(path_box));
          //compat<bool>::boxPack0 (GTK_BOX (topPaneVbox_), GTK_WIDGET(path_box), FALSE, TRUE, 0);


          // FIXME: auto historyPath = g_build_filename(PATH_HISTORY);
          // FIXME: auto path_entry = mkCompletionEntry(historyPath);

          auto text=g_strdup_printf("%s:", _("Path"));
          auto path_entry = addEntry(mainBox_, "path_entry", text, this);
          g_free(text);

          //gtk_widget_set_size_request (GTK_WIDGET(path_entry), 50, -1);
          g_object_set_data(G_OBJECT(findDialog), "path_entry", path_entry);
          // FIXME: g_free(historyPath);

          auto buffer = gtk_entry_get_buffer(GTK_ENTRY(path_entry);    
          gtk_entry_buffer_set_text(path_entry, fullPath_, -1);

          gtk_box_append(path_box, GTK_WIDGET(path_entry));
          //compat<bool>::boxPack0 (path_box, GTK_WIDGET(path_entry), TRUE, TRUE, 0);
          gtk_box_append(path_box, GTK_WIDGET(path_entry));

          g_signal_connect (path_entry,
                            "activate", BUTTON_CALLBACK(Type::onFindButton), 
                            (gpointer)findDialog);


      }

       GtkEntry *addEntry(GtkBox *child, const char *id, const char *text){
          auto hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
          gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
          gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
          auto label = gtk_label_new(text);
          gtk_widget_set_hexpand(GTK_WIDGET(label), false);
          auto entry = gtk_entry_new();

          //auto buffer = gtk_entry_buffer_new(NULL, -1);
          //auto entry = gtk_entry_new_with_buffer(buffer);
          gtk_widget_set_hexpand(GTK_WIDGET(entry), true);
          g_object_set_data(G_OBJECT(child), id, entry);
          //gtk_widget_set_sensitive(GTK_WIDGET(entry), true); // FIXME: put to false 
                                                             // when filedialog button
                                                             // is working.
          auto button = Basic::mkButton(EMBLEM_FOLDER_OPEN, NULL);
          g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(getDirectory), this);

          gtk_box_append(hbox, label);
          gtk_box_append(hbox, entry);
          gtk_box_append(hbox, GTK_WIDGET(button));
          gtk_box_append(child, GTK_WIDGET(hbox));
          return GTK_ENTRY(entry);
        }


    };


}
namespace xf
{

template <class Type>
class Find {

    char *fullPath_ = NULL

public:

    Find(const gchar *path){
        if (!whichGrep()){
            ERROR("grep command not found\n");
            exit(1);
        }
        fullPath(path);
        gchar *fullPath = NULL;
        createDialog();
    }

    ~Find(void){
        g_free(fullPath_);
    }

    void fullPath(const char *path){
        if (!path || !g_file_test(path, G_FILE_TEST_EXISTS)) {
          auto current = g_get_current_dir();
          fullpath_ = realpath(current);
          g_free(current);
          return;
        }
        fullPath = realpath(path, NULL);
        return;
    }



private:

    gboolean whichGrep(void){
        gchar *grep = g_find_program_in_path ("grep");
        if (!grep) return FALSE;
        FILE *pipe;
        const gchar *cmd = "grep --version";
        gnuGrep_ = FALSE;
        pipe = popen (cmd, "r");
        if(pipe) {
            gchar line[256];
            memset (line, 0, 256);
            if(fgets (line, 255, pipe) == NULL){
                // Definitely not GNU grep. BSD version.
                TRACE("pipe for \"grep --version\"\n"); 
                TRACE ("fgets: %s\n", strerror (errno));
    } else {
                if(strstr (line, "GNU")) gnuGrep_ = TRUE;
            }
            pclose (pipe);
        }
        g_free (grep);
        return TRUE;
    }

    void createDialog(){

    }

};
} // namespace xf
#endif

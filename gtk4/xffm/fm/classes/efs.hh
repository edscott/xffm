#ifndef EFS_HH
#define EFS_HH

#include "ecryptfs.i"

namespace xf {

  class EfsResponse{
   char *title_;
   const char *iconName_;
public:
    const char *title(void){ return title_;}
    const char *iconName(void){ return "emblem-run";}
    const char *label(void){return "xffm::efs";}

    ~EfsResponse (void){
      g_free(title_);
        //if (bashCompletionStore_) gtk_list_store_clear(bashCompletionStore_);
        //gtk_window_destroy(response_);
    }

    EfsResponse (void){
      title_ = g_strdup_printf("%s ecryptfs", _("New"));
      /*pthread_t thread;
      int retval = pthread_create(&thread, NULL, response_f, (void *)this);
      pthread_detach(thread);*/
    }

     static void *asyncYes(void *data){
      DBG("%s", "hello world\n");
      return NULL;
    }

    static void *asyncNo(void *data){
      DBG("%s", "goodbye world\n");
      return NULL;
    }
  };

  class EFS {
    static char *efsKeyFile(void){
      return  g_strconcat(g_get_user_config_dir(),G_DIR_SEPARATOR_S, "xffm+",G_DIR_SEPARATOR_S, "efs.ini", NULL);}

    public:
    static gchar **
    getSavedItems(void){
        gchar *file = g_build_filename(efsKeyFile(), NULL);
        GKeyFile *key_file = g_key_file_new ();
        g_key_file_load_from_file (key_file, file, (GKeyFileFlags)(G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS), NULL);
        g_free(file);
        auto retval = g_key_file_get_groups (key_file, NULL);
        g_key_file_free(key_file);
        return retval;
    }

    
    static void ohYeah(GtkButton *button, void *data){
      DBG("oh yeah\n");
      auto dialog = GTK_WINDOW(data);
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(1));
    }

    static void newEfs(void){
      auto dialogObject = new DialogComplex<EfsResponse>;
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      auto dialog = dialogObject->dialog();
      gtk_window_set_decorated(dialog, true);
      auto vbox = dialogObject->vbox();

      auto button = Basic::mkButton("emblem-start-here", "yeah");
      g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(ohYeah), (void *)dialog);

      auto hello = gtk_label_new("hello world");
      gtk_box_prepend(vbox, hello);
      gtk_box_prepend(vbox, GTK_WIDGET(button));
      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(GTK_WIDGET(dialog), "dialog", "Dialog");
      gtk_window_present(dialog);

      dialogObject->run();
      

    }


  };


}
#endif


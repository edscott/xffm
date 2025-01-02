#ifndef MOUNT_HH
#define MOUNT_HH

namespace xf {

template <class Type>
class Mount {
   using dialog_t = DialogComplex<mountResponse<Type> >;
   char *folder_ = NULL;
   char *path_ = NULL;
   public:
   Mount(const char *folder, const char *path){
      folder_ = g_strdup(folder);
      path_ = g_strdup(path);
      auto dialogObject = new dialog_t(folder, path);
      
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      auto dialog = dialogObject->dialog();
      gtk_window_set_decorated(dialog, true);
      dialogObject->setSubClassDialog();

      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(GTK_WIDGET(dialog), "dialog", "Dialog");
      gtk_window_present(dialog);

      dialogObject->run();
    }
   ~Mount(void){
     g_free(folder_);
     g_free(path_);
   }

};
}
#endif


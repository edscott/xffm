#ifndef TAR_HH
#define TAR_HH

namespace xf {

template <class Type>
class Tar {
   using dialog_t = DialogComplex<tarResponse<Type> >;
   char *folder_ = NULL;
   char *path_ = NULL;
   public:
   Tar(const char *folder, const char *path){
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
   ~Tar(void){
     g_free(folder_);
     g_free(path_);
   }

};
}
#endif


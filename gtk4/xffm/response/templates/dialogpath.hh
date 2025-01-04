#ifndef DIALOGPATH_HH
#define DIALOGPATH_HH
// Simple entry dialog, no file chooser.

namespace xf {

template <class Type>
class dialogPath {
    
public:
    static void action(const char *path){    

      auto dialogObject = new DialogEntry<Type>;
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      auto dialog = dialogObject->dialog();
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      g_object_set_data(G_OBJECT(entry), "path", g_strdup(path));

      dialogObject->subClass()->setDefaults(dialog, dialogObject->label());
      
      dialogObject->run();
    }

 };
}
#endif

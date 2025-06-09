#ifndef DIALOGPATH_HH
#define DIALOGPATH_HH
// Simple entry dialog, no file chooser.

namespace xf {

template <class Type>
class dialogPath {
    static DialogEntry<Type> *getObject(const char *path){
      auto dialogObject = new DialogEntry<Type>;
      dialogObject->setParent(GTK_WINDOW(Child::mainWidget()));
      auto dialog = dialogObject->dialog();
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      g_object_set_data(G_OBJECT(entry), "path", g_strdup(path));

      dialogObject->subClass()->setDefaults(dialog, dialogObject->label());
      return dialogObject;
    }  

public:
#if 0
    static void action(const char *path){    
      auto dialogObject = getObject(path);
      dialogObject->run();
    }
#else
    static void action(const char *path){
  
      auto dialogObject = getObject(path);
      auto dialog = dialogObject->dialog();
      dialogObject->run();
    }
    static void actionMove(const char *path){  
      auto dialogObject = getObject(path);
      auto dialog = dialogObject->dialog();
      Basic::moveToPointer(dialog); // only for X11
      dialogObject->run();
    }
#endif
 };
}
#endif

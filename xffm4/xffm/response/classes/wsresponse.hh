#ifndef WSRESPONSE_HH
#define WSRESPONSE_HH
namespace xf {

template <class Type> class MenuCallbacks;
template <class Type>
class wsResponse0: public entryResponse {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Workspace Switcher");}

    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
    }

    static void *asyncYes(void *data){
      auto dialogObject = (DialogEntry<wsResponse0<Type>> *)data;
      
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialogObject->dialog()),"entry"));
      auto buffer = gtk_entry_get_buffer(entry);
      auto nextWS = gtk_entry_buffer_get_text(buffer);
      //auto ok = MenuCallbacks<Type>::workSpaceExists(txt);
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      MenuCallbacks<Type>::moveTo(nextWS, 0);
       return NULL;
    }
};

template <class Type>
class wsResponse1: public entryResponse {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Workspace Switcher");}

    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
    }

    static void *asyncYes(void *data){
      auto dialogObject = (DialogEntry<wsResponse1<Type>> *)data;
      
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialogObject->dialog()),"entry"));
      auto buffer = gtk_entry_get_buffer(entry);
      auto nextWS = gtk_entry_buffer_get_text(buffer);
      //auto ok = MenuCallbacks<Type>::workSpaceExists(txt);
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      MenuCallbacks<Type>::moveTo(nextWS, 1);
       return NULL;
    }
};

template <class Type>
class wsResponse2: public entryResponse {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Workspace Switcher");}

    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
    }

    static void *asyncYes(void *data){
      auto dialogObject = (DialogEntry<wsResponse1<Type>> *)data;
      
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialogObject->dialog()),"entry"));
      auto buffer = gtk_entry_get_buffer(entry);
      auto nextWS = gtk_entry_buffer_get_text(buffer);
      //auto ok = MenuCallbacks<Type>::workSpaceExists(txt);
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      MenuCallbacks<Type>::switchWS(nextWS);
       return NULL;
    }
};

}
#endif

#ifndef WSRESPONSE_HH
#define WSRESPONSE_HH
namespace xf {

template <class Type> class MenuCallbacks;
template <class Type>
class wsResponse: public entryResponse {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Workspace Switcher");}

    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
    }

    static void *asyncYes(void *data){
      auto dialogObject = (DialogEntry<wsResponse<Type>> *)data;
      
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialogObject->dialog()),"entry"));
      auto buffer = gtk_entry_get_buffer(entry);
      auto txt = gtk_entry_buffer_get_text(buffer);
      auto ok = MenuCallbacks<Type>::workSpaceExists(txt);
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      if (ok){
        Print::showText(output);
        MenuCallbacks<Type>::moveTo(txt);
        //hmmm... race with i3: either moveto or switchWS
        //MenuCallbacks<Type>::switchWS(txt);
      } else {
        Print::showText(output);
        auto a = g_strconcat("\"",txt, "\"", NULL);
        auto msg = g_strdup_printf(_("Workspace %s"), a);
        auto msg2 = g_strconcat(" ",msg, " ",  _("does not exist"), "\n",NULL);
        Print::printError(output, g_strdup_printf("%s\n", msg2));
        g_free(a);
        g_free(msg);
      }


       return NULL;
    }
};

}
#endif

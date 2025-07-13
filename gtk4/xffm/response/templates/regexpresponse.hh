#ifndef REGEXPRESPONSE_HH
#define REGEXPRESPONSE_HH
namespace xf {

template <class Type> class MenuCallbacks;
template <class Type>
class regexpResponse: public entryResponse {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return _("foo");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Match regular expression");}

    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
    }

    static void *asyncYes(void *data){
      auto dialogObject = (DialogEntry<regexpResponse<Type>> *)data;
      
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialogObject->dialog()),"entry"));
      auto buffer = gtk_entry_get_buffer(entry);
      auto txt = gtk_entry_buffer_get_text(buffer);
      //auto ok = MenuCallbacks<Type>::workSpaceExists(txt);
      auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
    /*  if (ok){
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
      }*/
      auto path = Child::getWorkdir();
      DBG(" path = %s, regular expresion = \"%s\"\n", path,txt);
      int bit = 0x100;
      auto gridView_p = (GridView<Type> *)Child::getGridviewObject();
      auto flags = gridView_p->flags();

      bool update = false;
      if (!strlen(txt)) {
        gridView_p->flagOff(0x100); // Regexp off
        Settings::removeKey(path, "regexp");
        if (gridView_p->flags() == 0x40) {
          Settings::removeGroup(path);
        }

      } else {
        gridView_p->flagOn(0x100); // Regexp on
        update = true;
        Settings::setString(path, "regexp", txt);
        Settings::setInteger(path,"flags",gridView_p->flags()); 
      }

      // Need to update?


      // Check if valid regexp...



       return NULL;
    }
};

}
#endif

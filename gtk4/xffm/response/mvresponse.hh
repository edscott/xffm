#ifndef MVRESPONSE_HH
#define MVRESPONSE_HH
namespace xf {

class mvResponse: public pathResponse {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return _("Path");}
    const char *iconName(void){ return "dialog-question";}
    const char *label(void){ return _("Rename");}

    static void setDefaults(GtkWindow *dialog, GtkLabel *label){
      auto entry = GTK_ENTRY( g_object_get_data(G_OBJECT(dialog),"entry"));
      auto path = (const char *)g_object_get_data(G_OBJECT(entry), "path");
      auto base = g_path_get_basename(path);
      auto buffer = gtk_entry_get_buffer(entry);
      gtk_entry_buffer_set_text(buffer, base, -1);
      auto string = g_strconcat("<span color=\"green\"><b>",_("Rename"), ":\n</b></span><span color=\"blue\"><b>", base, "</b></span>", NULL);
      gtk_label_set_markup(label, string);
      g_free(base);
      g_free(string);
    }

    static void *asyncYes(void *data){
       asyncYesArg(data, "mv");      
       return NULL;
    }
};

}
#endif

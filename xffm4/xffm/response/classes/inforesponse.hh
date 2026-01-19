#ifndef INFORESPONSE_HH
#define INFORESPONSE_HH

namespace xf {
class infoResponse {
   const char *title_ = _("Information");
   const char *iconName_ = "emblem-important";
   GtkWindow *dialog_ = NULL;
   const char *label_ = "foo";
public:
    GtkWindow *dialog(void){return dialog_;}
    void dialog(GtkWindow *value){dialog_ = value;}
    const char *title(void){ return title_;}
    const char *iconName(void){ return iconName_;}
    const char *label(void){ return label_;}
    void label(const char *value){label_ = value;}


    infoResponse(void){
      //buttons = (GtkWidget **)calloc(3,sizeof(GtkWidget *));
      /*buttons[0] = gtk_button_new_from_icon_name(_("Copy"));
      buttons[1] = gtk_button_new_from_icon_name(_("Move"));
      buttons[2] = gtk_button_new_from_icon_name(_("Link"));*/
    }

    ~infoResponse(void){
      //g_free(buttons);
    }
    

    static void *asyncYes(void *data){
      //auto dialogObject = (DialogTimeoutButtons<infoResponse> *)data;
      //dialogObject->timeout(-1);
      
      TRACE("%s", "hello world\n");
      return NULL;
    }

    static void *asyncNo(void *data){
      //auto dialogObject = (DialogTimeoutButtons<infoResponse> *)data;
      //dialogObject->timeout(-1);
      TRACE("%s", "bye world\n");
      return NULL;
    }
 
private:

    static void activate(GtkEntry *entry, void *dialog){
      g_object_set_data(G_OBJECT(dialog), "response", GINT_TO_POINTER(2));
    }



    static gboolean 
    response_delete(GtkWidget *dialog, GdkEvent *event, gpointer data){
        return TRUE;
    }

};


}
#endif

#ifndef DNDRESPONSE_HH
#define DNDRESPONSE_HH

namespace xf {
class dndResponse {
   const char *title_;
   const char *iconName_;
   GtkWidget **buttons=NULL;
public:
    const char *title(void){ return _("Drop Target");}
    const char *iconName(void){ return "emblem-important";}
    const char *label(void){ return "";}
    GtkWidget **getButtons(void){ return buttons;}

    dndResponse(void){
      buttons = (GtkWidget **)calloc(4,sizeof(GtkWidget *));
      buttons[0] = GTK_WIDGET(Basic::mkButton("list-add", _("Copy")));
      buttons[1] = GTK_WIDGET(Basic::mkButton("list-remove", _("Move")));
      buttons[2] = GTK_WIDGET(Basic::mkButton("emblem-symbolic-link", _("Link")));
    }

    ~dndResponse(void){
      g_free(buttons);
    }
    

    static void *asyncYes(void *data){
      auto dialogObject = (DialogButtons<dndResponse> *)data;
      //dialogObject->timeout(-1);
      auto response = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialogObject->dialog()), "response"));
      switch (response) {
        case 1:
          DBG("hello world, response is %d: copy\n", response);
          break;
        case 2:
          DBG("hello world, response is %d: move\n", response);
          break;
        case 3:
          DBG("hello world, response is %d: link\n", response);
          break;
        default:
          DBG("*** Error:: dndResponse::asyncYes(): response %d is not appropriate.\n", response);
      }
      gtk_window_present(GTK_WINDOW(MainWidget));

      return NULL;
    }

    static void *asyncNo(void *data){
      //auto dialogObject = (DialogTimeoutButtons<infoResponse> *)data;
      //dialogObject->timeout(-1);
      DBG("%s", "Drop cancelled.\n");
      gtk_window_present(GTK_WINDOW(MainWidget));
      return NULL;
    }
 
private:

    /*static gboolean 
    response_delete(GtkWidget *dialog, GdkEvent *event, gpointer data){
        return TRUE;
    }*/

};


}
#endif

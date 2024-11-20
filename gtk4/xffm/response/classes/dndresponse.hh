#ifndef DNDRESPONSE_HH
#define DNDRESPONSE_HH

namespace xf {
class dndResponse {
   const char *title_;
   const char *iconName_;
   GtkWidget **buttons=NULL;
   char *target_ = NULL;
   char *uriList_ = NULL;
public:
    const char *title(void){ return _("Drop Target");}
    const char *iconName(void){ return "emblem-important";}
    const char *label(void){ return "";}
    GtkWidget **getButtons(void){ return buttons;}

    dndResponse(void){
      buttons = (GtkWidget **)calloc(5,sizeof(GtkWidget *));
      buttons[0] = GTK_WIDGET(Basic::mkButton("list-add", _("Copy")));
      buttons[1] = GTK_WIDGET(Basic::mkButton("list-remove", _("Move")));
      buttons[2] = GTK_WIDGET(Basic::mkButton("emblem-symbolic-link", _("Link")));
      buttons[3] = GTK_WIDGET(Basic::mkButton("no", _("Cancel")));
    }

    ~dndResponse(void){
      g_free(target_);
      g_free(uriList_);
      g_free(buttons);
    }
    
    int uriCount(void){
      int count=0;
      auto files = g_strsplit(uriList_, "\n", -1);
      for (auto p=files; p && *p; p++){
        if (strlen(*p) > 0) count++;
      }
      g_strfreev(files);
      return count;
    }
    void setUriList(const char *uriList){
      uriList_ = g_strdup(uriList);
    }
    void setTarget(const char *target){
      target_ = g_strdup(target);
    }
    const char *target(void) { return target_;}
    const char *uriList(void) { return uriList_;}

    static void *asyncYes(void *data){
      auto dialogObject = (DialogButtons<dndResponse> *)data;
      auto target = dialogObject->subClass()->target();
      auto uriList = dialogObject->subClass()->uriList();
      auto count = dialogObject->subClass()->uriCount();
      auto c = (ClipBoard *)g_object_get_data(G_OBJECT(MainWidget), "ClipBoard");
      
      //dialogObject->timeout(-1);
      auto response = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialogObject->dialog()), "response"));
      char *clipContent = g_strdup("");
      switch (response) {
        case 1:
          DBG("hello world, response is %d: copy to %s\n%s\n", response, target,uriList);
          Basic::concat(&clipContent, "copy\n");
          break;
        case 2:
          DBG("hello world, response is %d: move to %s\n%s\n", response, target, uriList);
          Basic::concat(&clipContent, "move\n");
          break;
        case 3:
          DBG("hello world, response is %d: link \n", response);
          Basic::concat(&clipContent, "link\n");
          break;
        case 4:
          DBG("hello world, response is %d: cancel\n", response);
          goto done;
        default:
          DBG("*** Error:: dndResponse::asyncYes(): response %d is not appropriate.\n", response);
          goto done;
      }
      Basic::concat(&clipContent, uriList);
      c->resetClipBoardCache(clipContent);
      g_free(clipContent);
      cpDropResponse::openDialog(target);
done:
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

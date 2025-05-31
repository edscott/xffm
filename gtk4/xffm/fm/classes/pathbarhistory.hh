#ifndef PATHBARHISTORY_HH
#define PATHBARHISTORY_HH
namespace xf {

  class PathbarHistory {
    GList *historyBack_=NULL;
    GList *historyNext_=NULL;
  public:

   ~PathbarHistory(void){
     for (auto l=historyBack_; l && l->data; l=l->next) g_free(l->data);
     for (auto l=historyNext_; l && l->data; l=l->next) g_free(l->data);
     g_list_free(historyBack_);
     g_list_free(historyNext_);
     return;
   }

   bool historyNext(void) {
     if (historyNext_) return true; 
     return false;
   }
   bool historyBack(void) {
     if (historyBack_ && g_list_length(historyBack_) > 1) return true;
     return false;
   }

   const char *nextHistory(void){
     if (!historyNext_ || historyNext_->data == NULL){
      //if (!historyNext_){ TRACE("no next history List\n"); } else { TRACE("no next history next\n"); }
      return NULL;
     }
     char *current = (char *) historyNext_->data;
     TRACE("next path is %s\n", (const char *) current);
     historyBack_ = g_list_prepend(historyBack_, current);
     historyNext_ = g_list_remove(historyNext_, current);
     return current;
   }

   const char *backHistory(void){
     if (!historyBack_ || historyBack_->next == NULL) {
      //if (!historyBack_){ TRACE("no back history List\n"); } else { TRACE("no back history next\n"); }
      return NULL;
     }
     auto current = (const char *) historyBack_->data;
     auto previous = (const char *) historyBack_->next->data;
     TRACE("Back path is %s\n", previous);
     TRACE("Next path is %s\n", current);
     // No need to free memory, since we just move from one list to the other.
     historyNext_ = g_list_prepend(historyNext_, (void *)current);
     historyBack_ = g_list_remove(historyBack_,  (void *)current);
     //for (GList *l=historyNext_; l && l->data; l=l->next){TRACE("historyNext list = %s\n", (char *)l->data);}
     return previous;    
   }

   void push(const char *path){
          TRACE("BasicPAthbar:: pushing %s\n", path);
      if (historyBack_ && historyBack_->data != NULL){
        if (strcmp(path, (const char *)historyBack_->data) != 0){
          // update with different or non existing path.
          historyBack_ = g_list_prepend(historyBack_, g_strdup(path));
          TRACE("push: 1 updating historyBack_ with path = %s\n", path);
        }
      } else {
          // update with new path. 
          historyBack_ = g_list_prepend(historyBack_, g_strdup(path));
          TRACE("push: 2 updating historyBack with path = %s\n", path);
      }
      // wipe next history 
      for (GList *l=historyNext_; l && l->data; l=l->next) g_free(l->data);
      g_list_free(historyNext_);
      historyNext_ = NULL;
      return;
   }
  
  };
}
#endif

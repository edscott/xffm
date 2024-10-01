#ifndef XF_ENTRYRESPONSE_HH
# define XF_ENTRYRESPONSE_HH
namespace xf
{

class EntryResponse {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return "fooBar";}
    const char *iconName(void){ return "emblem-run";}
    const char *label(void){return "label foobar";}

    ~EntryResponse (void){
        //if (bashCompletionStore_) gtk_list_store_clear(bashCompletionStore_);
        //gtk_window_destroy(response_);
    }

    EntryResponse (void){
      /*pthread_t thread;
      int retval = pthread_create(&thread, NULL, response_f, (void *)this);
      pthread_detach(thread);*/
    }

     static void *asyncYes(void *data){
      DBG("%s", "hello world\n");
      return NULL;
    }

    static void *asyncNo(void *data){
      DBG("%s", "goodbye world\n");
      return NULL;
    }
 

 };
}
#endif


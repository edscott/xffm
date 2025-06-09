#ifndef ENTRYRESPONSE_HH
# define ENTRYRESPONSE_HH
namespace xf
{
class entryResponse {
   const char *title_;
   const char *iconName_;
public:
    const char *title(void){ return "fooBar";}
    const char *iconName(void){ return "emblem-run";}
    const char *label(void){return "label foobar";}

    ~entryResponse (void){
    }

    entryResponse (void){
    }

     static void *asyncYes(void *data){
      TRACE("%s", "hello world\n");
      return NULL;
    }

    static void *asyncNo(void *data){
      TRACE("%s", "goodbye world\n");
      return NULL;
    }
 

 };
}
#endif


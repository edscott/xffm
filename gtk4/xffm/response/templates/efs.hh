#ifndef EFS_HH
#define EFS_HH

namespace xf {


  template <class Type>
  class EFS {
    using subClass_t = EfsResponse<Type>;
    using dialog_t = DialogComplex<subClass_t>;
    
public:

    EFS(GtkWindow *parent, const char *folder){
      auto dialogObject = new dialog_t(parent, folder);
    
    }

    ~EFS(void){
    }

//////////////////////////////////////////////////////////////////////////////////////


public:    

    static gchar **
    getSavedItems(void){
        auto key_file = subClass_t::getKeyFile();
        auto retval = g_key_file_get_groups (key_file, NULL);
        g_key_file_free(key_file);
        return retval;
    }
  };

  
}
#endif


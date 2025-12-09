#ifndef EFS_HH
#define EFS_HH

namespace xf {


  template <class Type>
  class EFS {
    using subClass_t = EfsResponse<Type>;
    using complexDialog_t = DialogComplex<subClass_t>;
    
public:

    EFS(GtkWindow *parent, const char *folder){
      DBG("EFS no load\n");
      auto dialogObject = new complexDialog_t(parent, folder);
    
    }

    EFS(GtkWindow *parent, const char *path, bool load){
      DBG("EFS load\n");
      auto keyfile = getKeyfile();
      auto dialogObject = new complexDialog_t(parent, path);
      auto mountPoint = g_key_file_get_string(keyfile, path, "mountPoint", NULL);
      auto mountOptions = g_key_file_get_string(keyfile, path, "mountOptions", NULL);
      auto efsOptions = g_key_file_get_string(keyfile, path, "efsOptions", NULL);
      DBG("workdir.hh: mountPoint = '%s'\n", mountPoint);
          g_key_file_free(keyfile);

      dialogObject->subClass()->setup(path,mountPoint,mountOptions,efsOptions);
   
    }

    ~EFS(void){
    }

//////////////////////////////////////////////////////////////////////////////////////


public:    
    static GKeyFile *getKeyfile(void){
      return subClass_t::getKeyFile();
    }

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


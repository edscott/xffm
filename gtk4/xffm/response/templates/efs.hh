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

private:
    static char *efsKeyFile(void){
      return  g_strconcat(g_get_user_config_dir(),G_DIR_SEPARATOR_S, "xffm+",G_DIR_SEPARATOR_S, "efs.ini", NULL);}

public:    

    static gchar **
    getSavedItems(void){
        gchar *file = g_build_filename(efsKeyFile(), NULL);
        GKeyFile *key_file = g_key_file_new ();
        g_key_file_load_from_file (key_file, file, (GKeyFileFlags)(G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS), NULL);
        g_free(file);
        auto retval = g_key_file_get_groups (key_file, NULL);
        g_key_file_free(key_file);
        return retval;
    }

  };

  
}
#endif


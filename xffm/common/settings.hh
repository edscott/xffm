#ifndef XF_SETTINGS_HH
#define XF_SETTINGS_HH


#include "tubo.hh"
#include "print.hh"


static GKeyFile *keyFile = NULL;
static const gchar *settingsfile = NULL;
gsize mTime;
namespace xf {

template <class Type>
class Settings {

public:

    static void
    readSettings(void){
        keyFile = g_key_file_new();
        settingsfile = (gchar *)g_build_filename(g_get_user_config_dir(),"xffm+","settings.ini", NULL);
        gboolean loaded = g_key_file_load_from_file(keyFile, settingsfile,
               (GKeyFileFlags) (G_KEY_FILE_KEEP_COMMENTS |  G_KEY_FILE_KEEP_TRANSLATIONS),
                NULL);
        if (!loaded) {
            gchar *text = g_strdup_printf("%s %s\n", _("New File:"), settingsfile);
            TRACE("%s", text);
            g_free(text);
            writeSettings();
        }
        struct stat st;
        if (stat(settingsfile, &st) < 0){
            ERROR("settings.hh:: cannot access %s\n", settingsfile);
            exit(1);
        }
        mTime = st.st_mtime;
    }
private:
    static void
    reloadSettings(void){
        struct stat st;
        if (stat(settingsfile, &st) < 0){
            ERROR("settings.hh:: cannot access %s\n", settingsfile);
            exit(1);
        }
        if (st.st_mtime == mTime){
            return;

        }
        TRACE("%s reload %ld -> %ld \n", settingsfile, mTime, st.st_mtime);
        g_key_file_free(keyFile);
        readSettings();
    }

    static void
    writeSettings(void){
        TRACE( "group_options_write_keyfile: %s\n", settingsfile);
        // Write out keyFile:
	if (!keyFile) readSettings();
        gsize file_length;
        gchar *file_string = g_key_file_to_data (keyFile, &file_length, NULL);
        gint fd;

	if (!g_file_test(settingsfile, G_FILE_TEST_EXISTS)) {
	    TRACE("!g_file_test(settingsfile, G_FILE_TEST_EXISTS)\n");
	    gchar *config_directory = g_path_get_dirname(settingsfile);
	    if (!g_file_test(config_directory, G_FILE_TEST_IS_DIR)){
		TRACE( "creating directory %s\n", config_directory);
		if (g_mkdir_with_parents(config_directory, 0700) < 0){
                    ERROR("Cannot create %s: %s\n", config_directory, strerror(errno));
                    exit(1);
                }
	    }
	    g_free(config_directory);
	    fd = creat(settingsfile, O_WRONLY | S_IRWXU);
	} else {
	    TRACE("open(settingsfile, O_WRONLY | S_IRWXU)\n");
	    fd = open(settingsfile, O_WRONLY|O_TRUNC);
	}
        if (fd >= 0){
            if (write(fd, file_string, file_length) < 0){
                ERROR("writeSettings(): file_string, file_length %s,%ld\n", file_string, (long)file_length);
                ERROR("writeSettings(): cannot write to %s: %s\n", settingsfile, strerror(errno));
            }
            close(fd);
        } else {
            ERROR("writeSettings(): cannot open %s for write: %s\n", settingsfile, strerror(errno));
        }
    }
 
public:
    static gchar **
    getKeys(const gchar *group){
         reloadSettings();
         return g_key_file_get_keys (keyFile, group, NULL, NULL);
   
    }
    static gchar **
    getKeys(const gchar *group, gsize *size){
         reloadSettings();
         return g_key_file_get_keys (keyFile, group, size, NULL);
   
    }

   static void
   setSettingInteger(const gchar *group, const gchar *item, int value){
       reloadSettings();
       g_key_file_set_integer (keyFile, group, item, value);
       writeSettings();
   }
    
   static void
   setSettingString(const gchar *group, const gchar *item, const gchar *value){
       reloadSettings();
       g_key_file_set_string (keyFile, group, item, value);
       writeSettings();
   }
    
   static gchar *
   getSettingString(const gchar *group, const gchar *item){
        reloadSettings();
        gchar *value=NULL;
	GError *error = NULL;
	value = g_key_file_get_string (keyFile, group, item, &error);
	if (error){
	    TRACE("%s\n", error->message);
	    g_error_free(error);
	    value = NULL;
        } 
        return value;
   }
   
   static gboolean
   removeKey(const gchar *group, const gchar *key){
       reloadSettings();
       auto retval = g_key_file_remove_key (keyFile, group, key, NULL);
       writeSettings();
       return retval;
   }

   static gint 
   getSettingInteger(const gchar *group, const gchar *item){
        reloadSettings();
        gint value=-1;
	GError *error = NULL;
	value = g_key_file_get_integer (keyFile, group, item, &error);
	if (error){
	    TRACE("%s\n", error->message);
	    g_error_free(error);
	    value = -1;
	}
        return value;
   }
    static gboolean
    keyFileHasGroupKey(const gchar *group, const gchar *key) {
        reloadSettings();
	if (g_key_file_has_group(keyFile, group) &&
	g_key_file_has_key (keyFile, group, key, NULL) && 
	getSettingInteger(group, key))
	    return TRUE;
	return FALSE;
    }

};
}
#endif

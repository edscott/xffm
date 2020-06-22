#ifndef XF_SETTINGS_HH
#define XF_SETTINGS_HH


#include "tubo.hh"
#include "print.hh"
#define SETTINGS_FILE g_get_user_config_dir(),"xffm+","settings.ini"
//pthread_mutex_t settingsMutex = PTHREAD_MUTEX_INITIALIZER;
namespace xf {

template <class Type>
class Settings {

public:
    static gint 
    getSettingInteger(const gchar *group, const gchar *item){
        //pthread_mutex_lock(&settingsMutex);
        auto keyFile = getKeyFile();
        gint value = -1;
        GError *error = NULL;
        if (!g_key_file_has_key(keyFile, group, item, &error)) {
            // DBG crashes: Glib bug with GError->message:
            //TRACE("getSettingInteger(): %s\n", error->message);
            if (error) g_error_free(error);
        } else {
            value = g_key_file_get_integer (keyFile, group, item, &error);
            if (error){
                // DBG crashes: Glib bug with GError->message:
                //TRACE("getSettingInteger(): %s\n", error->message);
                g_error_free(error);
                value = -1;
            }
        }
        g_key_file_free(keyFile);
        //pthread_mutex_unlock(&settingsMutex);
        return value;
    }
 
    static gchar * 
    getSettingString(const gchar *group, const gchar *item){
        //pthread_mutex_lock(&settingsMutex);
        auto keyFile = getKeyFile();
        gchar *value = NULL;
        GError *error = NULL;
        if (!g_key_file_has_key(keyFile, group, item, &error)) {
            // DBG crashes: Glib bug with GError->message:
            //DBG(".getSettingString(): %s\n", error->message);
            if (error) g_error_free(error);
        } else {
            value = g_key_file_get_string (keyFile, group, item, &error);
            if (error){
                // DBG crashes: Glib bug with GError->message:
                //DBG("..getSettingString(): %s\n", error->message);
                g_error_free(error);
                value = NULL;
            }
        }
        g_key_file_free(keyFile);
       // pthread_mutex_unlock(&settingsMutex);
        return value;
    }
       
    static void
    setSettingInteger(const gchar *group, const gchar *item, int value){
        //pthread_mutex_lock(&settingsMutex);
        auto keyFile = getKeyFile();
        g_key_file_set_integer (keyFile, group, item, value);
        writeKeyFile(keyFile);
        g_key_file_free(keyFile);
        //pthread_mutex_unlock(&settingsMutex);
        return;
    }
    
    static void
    setSettingString(const gchar *group, const gchar *item, const gchar *value){
        //pthread_mutex_lock(&settingsMutex);
        auto keyFile = getKeyFile();
        g_key_file_set_string (keyFile, group, item, value);
        writeKeyFile(keyFile);
        g_key_file_free(keyFile);
        //pthread_mutex_unlock(&settingsMutex);
        return;
    }
    
   
    static gboolean
    removeKey(const gchar *group, const gchar *key){
        //pthread_mutex_lock(&settingsMutex);
        auto keyFile = getKeyFile();
        auto retval = g_key_file_remove_key (keyFile, group, key, NULL);
        if (retval) writeKeyFile(keyFile);
        g_key_file_free(keyFile);
        //pthread_mutex_unlock(&settingsMutex);
        return retval;
    }

    static gboolean
    keyFileHasGroupKey(const gchar *group, const gchar *key) {
        //pthread_mutex_lock(&settingsMutex);
        auto keyFile = getKeyFile();
        if (g_key_file_has_group(keyFile, group) &&
            g_key_file_has_key (keyFile, group, key, NULL))
            return TRUE;
        //pthread_mutex_unlock(&settingsMutex);
        return FALSE;
    }
   
private:

    static void
    getFileLock(gchar *settingsfile){
        gint count=0;
        auto lock = g_strconcat(settingsfile,".lock", NULL);
        while (g_file_test(lock, G_FILE_TEST_EXISTS)){ 
            usleep(250000);
            count++;
            if (count > 4){
                DBG("removing stale lock\n");
                unlink(lock);
            }
        }
        fclose(fopen(lock, "w"));
        g_free(lock);
    }

    static void
    removeFileLock(gchar *settingsfile){
        auto lock = g_strconcat(settingsfile,".lock", NULL);
        unlink(lock);
        g_free(lock);
    }

    static GKeyFile *
    getKeyFile(void){
        auto keyFile = g_key_file_new();
        auto settingsfile = g_build_filename(SETTINGS_FILE, NULL);

        auto loaded = g_key_file_load_from_file(keyFile, settingsfile,
               //(GKeyFileFlags) (G_KEY_FILE_KEEP_COMMENTS |  G_KEY_FILE_KEEP_TRANSLATIONS),
               (GKeyFileFlags) (0),
                NULL);
        if (!loaded) {
            TRACE("%s %s\n", _("New File:"), settingsfile);
        }
        g_free(settingsfile);
        return keyFile;
    }

    static void
    writeKeyFile(GKeyFile *keyFile){
        auto settingsfile = g_build_filename(SETTINGS_FILE, NULL);
        gchar *config_directory = g_path_get_dirname(settingsfile);
        if (!g_file_test(config_directory, G_FILE_TEST_IS_DIR)) {
            if (g_mkdir_with_parents(config_directory, 0700) < 0){
                WARN("Cannot create %s: %s\n", config_directory, strerror(errno));
                g_free(settingsfile);
                g_free(config_directory);
                return;
            }
        }
        getFileLock(settingsfile);

        gsize file_length;
        gchar *file_string = g_key_file_to_data (keyFile, &file_length, NULL);
        gint fd;

        if (!g_file_test(settingsfile, G_FILE_TEST_EXISTS)) {
            fd = creat(settingsfile, O_WRONLY | S_IRWXU);
        } else {
            TRACE("open(settingsfile, O_WRONLY | S_IRWXU)\n");
            fd = open(settingsfile, O_WRONLY|O_TRUNC);
        }
        if (fd >= 0){
            if (write(fd, file_string, file_length) < 0){
                WARN("writeKeyFile(): file_string, file_length %s,%ld\n", file_string, (long)file_length);
                WARN("writeKeyFile(): cannot write to %s: %s\n", settingsfile, strerror(errno));
            }
            close(fd);
        } else {
            WARN("writeKeyFile(): cannot open %s for write: %s\n", settingsfile, strerror(errno));
        }
        removeFileLock(settingsfile);
        g_free(file_string);
        g_free(settingsfile);
        g_free(config_directory);
        return;
    }
};
}
#endif

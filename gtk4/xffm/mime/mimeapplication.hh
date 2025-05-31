#ifndef XF_MIMEAPPLICATION_HH
#define XF_MIMEAPPLICATION_HH
#include <pthread.h>


namespace xf {

static pthread_mutex_t mimeAppHashMutex=PTHREAD_MUTEX_INITIALIZER;
/*static GHashTable *application_hash_text=NULL;
static GHashTable *application_hash_text2=NULL;
static GHashTable *application_hash_output=NULL;
static GHashTable *application_hash_output_ext=NULL;*/
static GHashTable *applicationHash=NULL;

class MimeApplication {
    static void freeStrV(void *data){
        auto p = (gchar **)data;
        g_strfreev(p);
    }
public:
    static void constructAppHash (void) {
        TRACE("**** constructAppHash\n");
        applicationHash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, freeStrV);
        loadAppHash();
    }
    
    static const gchar **
    locate_apps (const gchar * mimetype) {

        //load_hashes ();
        ///  now look in hash...
        if (!mimetype) return NULL;
        pthread_mutex_lock(&mimeAppHashMutex);
        if (!applicationHash) {
            TRACE("applicationHash does not exist.\n");
            pthread_mutex_unlock(&mimeAppHashMutex);
            return NULL;
        }
        TRACE("**** loading apps for mimetype: \"%s\"\n", mimetype);
        auto apps = (const gchar **)g_hash_table_lookup (applicationHash, mimetype);
        pthread_mutex_unlock(&mimeAppHashMutex);
        return apps;
    }

public:   
    static void
    add2ApplicationHash(const gchar *type, const gchar *command, gboolean prepend){
        // Always use basic mimetype: avoid hashing alias mimetypes...
        // Suffix hash is currently not enabled.
        // const gchar *basic_type = MimeSuffix::getBasicType(type);
        // if (!basic_type) basic_type = type;
        auto basic_type = type;
        pthread_mutex_lock(&mimeAppHashMutex);
        auto apps = (gchar **)g_hash_table_lookup(applicationHash, basic_type);
        if (apps) {
            int size = 1; // final 0
            gchar **p;
            // Check if command is already tabulated...
            for (p=apps; p && *p; p++) {
                if (strcmp(*p, command)==0) {
                    pthread_mutex_unlock(&mimeAppHashMutex);
                    TRACE("command \"%s\" already in hash\n", command);
                    return;
                }
            }
            // get vector size.
            for (p=apps; p && *p; p++) size++;
            TRACE("application vector size=%d\n", size);

            gchar **newApps = (gchar **)calloc(size+1, sizeof(gchar *));
            if (!newApps){
                ERROR_("add2ApplicationHash: calloc() %s\n", strerror(errno));
                exit(1);
            }
            int i=0;
            if (prepend) newApps[i++] = g_strdup(command);
            for (p=apps; p && *p; p++){
                newApps[i++] = g_strdup(*p);
            }
            if (!prepend) newApps[i++] = g_strdup(command);
            g_hash_table_replace(applicationHash, g_strdup(basic_type), (void *)newApps);
            TRACE("replacing hash value %s -> %s\n", basic_type, command);
        } else {
            gchar **newApps = (gchar **)calloc(2, sizeof(gchar *));
            newApps[0] = g_strdup(command);
            g_hash_table_insert(applicationHash, g_strdup(basic_type), (void *)newApps);
            TRACE("adding NEW hash value %s -> %s\n", basic_type, command);
        } 
        pthread_mutex_unlock(&mimeAppHashMutex);
        return;

    }
    
private:

    static gboolean loadAppHash(void){
      loadAppHash_f(NULL);
      //  new Thread("MimeApplication::loadAppHash(): loadAppHash_f", loadAppHash_f, NULL);
        return TRUE;
    }

    static void *loadAppHash_f(void *data){
        processApplicationDir("/usr/share/applications");
        processApplicationDir("/usr/local/share/applications");
        return NULL;
    }
public:
    static void
    processApplicationDir(const gchar *dir){
        DIR *directory = opendir(dir);
        if (!directory) {
            TRACE("mime_c:: opendir %s: %s\n", dir, strerror(errno));
            return;
        }
        TRACE("Now reading directory: %s\n", dir);
        readApplicationDir(dir, directory);
        closedir (directory);
    }
private:
    static void 
    readApplicationDir(const gchar *dir, DIR *directory){
        //  mutex protect...
        //pthread_mutex_lock(&readdir_mutex);
        struct dirent *d; // static pointer
        errno=0;
        while ((d = readdir(directory))  != NULL){
            TRACE("Now reading file: %s\n", d->d_name);
            parseDesktopFile(dir, d);
        }
    }

    static void
    parseDesktopFile(const gchar *dir, struct dirent *d){
        TRACE( "%p  %s\n", d, d->d_name);
        if(strstr(d->d_name, ".desktop") == NULL) return;
        // get [Desktop Entry] Exec
        // get [Desktop Entry] TryExec
        // get [Desktop Entry] MimeType
        //
        GKeyFile *key_file = g_key_file_new();
        gchar *file = g_build_filename(dir,d->d_name, NULL);
        gboolean loaded = g_key_file_load_from_file(key_file, file, 
                (GKeyFileFlags) (0), NULL);
        g_free(file);
        if (!loaded) return;
        const gchar *group = "Desktop Entry";
        if (!g_key_file_has_group (key_file, group)){
            g_key_file_free(key_file);
            return;
        }
        GError *error = NULL;
        if (!g_key_file_has_key (key_file, group, "MimeType", &error)){
            g_key_file_free(key_file);
            return;
        }
        error = NULL;
        gchar *exec = g_key_file_get_string (key_file, group, "Exec", &error);
        if (error){ exec = NULL; error=NULL;}
        gchar *tryExec = g_key_file_get_string (key_file, group, "TryExec", &error);
        if (error){ tryExec = NULL; error=NULL; }
        gchar *terminal = g_key_file_get_string (key_file, group, "Terminal", &error);
        if (error){ terminal = NULL; error=NULL; }
        gchar *icon = g_key_file_get_string (key_file, group, "Icon", &error);
        if (error){ icon = NULL;  error=NULL;}
        gchar *mimeType = g_key_file_get_string (key_file, group, "MimeType", &error);

        if (mimeType){
            gchar **types = g_strsplit(mimeType, ";", -1);
            gchar **p;
            for (p=types; p && *p; p++){
                gchar *e = (exec)?exec:tryExec;
                if (e && strchr(e,' ')){
                    auto ee = g_strdup(e);
                    *strchr(ee,' ') = 0;
                    auto epath = g_find_program_in_path(ee);
                    TRACE("%s (%s)--> %s\n", e, ee, epath);
                    if (!epath){
                        TRACE("ignoring app: %s\n", e);
                        e=NULL;
                    }
                    g_free(ee);
                    g_free(epath);
                }
                
                if (*p && strlen(*p) && e){
                    if (strstr(e, "%U")) *(strstr(e, "%U")+1) = 's';
                    if (strstr(e, "%u")) *(strstr(e, "%u")+1) = 's';
                    if (strstr(e, "%F")) *(strstr(e, "%F")+1) = 's';
                    if (strstr(e, "%f")) *(strstr(e, "%f")+1) = 's';
                    TRACE("** Adding application \"%s\" --> %s\n", *p, e);
                    add2ApplicationHash(*p, e, TRUE);
                }
            }
            g_strfreev(types);
        }
        g_free(mimeType);
        g_free(exec);
        g_free(tryExec);
        g_free(terminal);
        g_free(icon);
        g_key_file_free(key_file);
    }


};
}
#endif

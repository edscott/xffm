#ifndef XF_SETTINGS_HH
#define XF_SETTINGS_HH
static GKeyFile *keyFile = NULL;
static const gchar *settingsfile = NULL;
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
            gchar *text = g_strdup_printf(_("Creating a new file (%s)"), settingsfile);
            DBG("%s", text);
            g_free(text);
        }
    }
/*
    static void
    saveSettings(const gchar *group, const gchar *item, gint value){
        g_key_file_set_integer (keyFile, group, item, value);
        writeSettings();
    }

    static void
    saveSettings(void){
        GtkAllocation allocation;
        gtk_widget_get_allocation(GTK_WIDGET(dialog_), &allocation);
        g_key_file_set_integer (keyFile, "window", "width", allocation.width);
        g_key_file_set_integer (keyFile, "window", "height", allocation.height);
        auto page = this->currentPageObject();
        g_key_file_set_integer (keyFile, "xfterm", "fontSize", page->fontSize());
        writeSettings();
    }
*/
    static void
    writeSettings(void){
        TRACE( "group_options_write_keyfile: %s\n", settingsfile);
        // Write out keyFile:
	if (!keyFile) readSettings();
        gsize file_length;
        gchar *file_string = g_key_file_to_data (keyFile, &file_length, NULL);
        gint fd;

	if (!g_file_test(settingsfile, G_FILE_TEST_EXISTS)) {
	    WARN("!g_file_test(settingsfile, G_FILE_TEST_EXISTS)\n");
	    gchar *config_directory = g_path_get_dirname(settingsfile);
	    if (!g_file_test(config_directory, G_FILE_TEST_IS_DIR)){
		TRACE( "creating directory %s\n", config_directory);
		g_mkdir_with_parents(config_directory, 0700);
	    }
	    g_free(config_directory);
	    fd = creat(settingsfile, O_WRONLY | S_IRWXU);
	} else {
	    WARN("open(settingsfile, O_WRONLY | S_IRWXU)\n");
	    fd = open(settingsfile, O_WRONLY|O_TRUNC);
	}
        if (fd >= 0){
            if (write(fd, file_string, file_length) < 0){
                ERROR("writeSettings(): file_string, file_length %s,%ld\n", file_string, file_length);
                ERROR("writeSettings(): cannot write to %s: %s\n", settingsfile, strerror(errno));
            }
            close(fd);
        } else {
            ERROR("writeSettings(): cannot open %s for write: %s\n", settingsfile, strerror(errno));
        }
    }
 
public:
   static void
   setSettingInteger(const gchar *group, const gchar *item, int value){
        g_key_file_set_integer (keyFile, group, item, value);
   }
    
   static void
   setSettingString(const gchar *group, const gchar *item, const gchar *value){
        g_key_file_set_string (keyFile, group, item, value);
   }
    
   static gchar *
   getSettingString(const gchar *group, const gchar *item){
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
   static gint 
   getSettingInteger(const gchar *group, const gchar *item){
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
	if (g_key_file_has_group(keyFile, group) &&
	g_key_file_has_key (keyFile, group, key, NULL) && 
	getSettingInteger(group, key))
	    return TRUE;
	return FALSE;
    }

};
}
#endif

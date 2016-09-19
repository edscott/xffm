#ifndef MIME_C_HPP
#define MIME_C_HPP
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "xffm+.h"

#define USER_RFM_DIR            g_get_user_config_dir(),"rfm"
#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"rfm"
#define USER_DBH_DIR		USER_RFM_DIR,"dbh"
#define USER_DBH_CACHE_DIR	USER_RFM_CACHE_DIR,"dbh"
#define SYSTEM_MODULE_DIR	PACKAGE_DATA_DIR,"rfm","rmodules"
#define USER_APPLICATIONS 	USER_RFM_DIR,"user-applications.2"
#define APPLICATION_MIME_FILE 	SYSTEM_MODULE_DIR,"mime-module.xml"

class mime_c {
    public:
        mime_c(void);
        ~mime_c(void);
        const gchar *find_mimetype_in_hash(const gchar *);
        gchar *mime_type_plain (const gchar *);
        gchar *mime_type (const gchar *, struct stat *);
       /* void *mime_magic (void *p);
        void *mime_encoding (void *p);
        void *mime_file (void *p);
        void *mime_function(const gchar *, const gchar *);*/
        gboolean mime_is_valid_command (const char *);
        const gchar *mime_command_text (gchar *p) ;
        const gchar *mime_command_text2 (gchar *p);
        const gchar *mime_command_icon (gchar *p);
        const gchar *mime_command_output (gchar *p);
        const gchar *mime_command_output_ext (gchar *p);
        gchar *mime_command (const char *p);
        gchar **mime_apps (const char *p);
        void *mime_add (gchar *, gchar *);
        void *mime_append (gchar *, gchar *);
        gchar *mime_mk_command_line (const gchar *, const gchar *);
        gchar *mime_mk_terminal_line (const gchar *p);
        void mime_generate_cache(void);
        gchar *mime_get_alias_type(const gchar *p);

    private:
        pthread_mutex_t cache_mutex;
        pthread_mutex_t mimetype_hash_mutex;
        pthread_mutex_t alias_hash_mutex;
        pthread_mutex_t application_hash_mutex;

        GHashTable *mimetype_hash;
        GHashTable *alias_hash;
        GHashTable *application_hash_type;
        GHashTable *application_hash_sfx;
        GHashTable *application_hash_icon;
        GHashTable *application_hash_text;
        GHashTable *application_hash_text2;
        GHashTable *application_hash_output;
        GHashTable *application_hash_output_ext;

        gboolean load_hashes_from_cache(void);
        long long read_cache_sum (void);
        long long get_cache_sum (void);
        void write_cache_sum (long long);
        gchar *get_cache_path (const gchar *);
        gint check_dir (char *);
        void load_text_hash(GHashTable *, const gchar *);

        void mime_build_hashes (void);   
        void destroy_application_hash_sfx (void);
        void destroy_application_hash_type (void);
        gchar *get_hash_key (const gchar * );
        void add_type_to_hashtable(const gchar *, const gchar *, gboolean );
        const gchar *mimeable_file (struct stat *);
        void save_text_cache(GHashTable *, const gchar *);
        gboolean generate_caches (void);
        gpointer gencache (gpointer );
        const gchar *locate_mime_t (const gchar * );
        gchar **locate_apps (const gchar * );
        void *put_mimetype_in_hash(const gchar *, const gchar *);
        gchar *mimetype1(const gchar *);
        gchar *mimetype2(const gchar *);
        gchar *get_hash_key_strstrip (void *);

};



#endif

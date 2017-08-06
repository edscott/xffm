#ifndef MIME_C_HPP
#define MIME_C_HPP
#include "xffm+.h"

#include "utility_c.hpp"
#include "mime_magic_c.hpp"
#include "lite_c.hpp"

#define USER_RFM_DIR            g_get_user_config_dir(),"rfm"
#define USER_DBH_DIR		USER_RFM_DIR,"dbh"
#define USER_DBH_CACHE_DIR	USER_RFM_CACHE_DIR,"dbh"
#define SYSTEM_MODULE_DIR	PACKAGE_DATA_DIR,"rfm","rmodules"

#include "mime_sfxhash_c.hh"
#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"rfm"
#define APPLICATION_MIME_FILE 	SYSTEM_MODULE_DIR,"mime-module.xml"
#define USER_APPLICATIONS 	USER_RFM_DIR,"user-applications.2"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <errno.h>
#include <dbh.h>

class mime_c: virtual utility_c, public lite_c, public mime_magic_c {
    public:
        mime_c(void);
        ~mime_c(void);
        gchar *mime_type (const gchar *);
        gchar *mime_type (const gchar *, struct stat *);
        gchar *mime_function(const gchar *, const gchar *);



        const gchar *find_mimetype_in_hash(const gchar *);
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
        const gchar *get_mimetype_iconname(const gchar *);
        
        gchar *mime_command (const char *p);
        gchar **mime_apps (const char *p);
        void *mime_add (gchar *, gchar *);
        void *mime_append (gchar *, gchar *);
        gchar *mime_mk_command_line (const gchar *, const gchar *);
        gchar *mime_mk_terminal_line (const gchar *p);
        gchar *mime_get_alias_type(const gchar *p);
        gboolean generate_caches (void);
        void *mime_gencache(gchar *);

        static string4_hash_t sfx_data;
    private:

	mime_sfxhash_c<const string4_hash_t> app_sfx_hash; // key is g_utf8_strdown ((gchar *)value(value), -1);
//	mime_hash_t<const string4_hash_t> app_sfx_hash; // key is g_utf8_strdown ((gchar *)value(value), -1);
        
	/*
	mime_hash_t<string_hash_c("key","value",NULL)> app_sfx_hash; // key is g_utf8_strdown ((gchar *)value(value), -1);
	mime_hash_t<string_hash_c("alias","type",NULL)> app_alias_hash; // key is g_utf8_strdown ((gchar *)value(type), -1);
	mime_hash_t<string_hash_c("generic-icon","name",NULL)> app_generic_icon_hash; // key is g_utf8_strdown ((gchar *)value(name), -1);

	mime_hash_t<string_hash_c("application",NULL,"command")> app_type_hash; // key is get_hash_key(type)
	mime_hash_t<string_hash_c("application","command","icon")> app_icon_hash; // key is get_hash_key(value(command))
	mime_hash_t<string_hash_c("application","command","text")> app_text_hash; // key is get_hash_key(value(command))
	mime_hash_t<string_hash_c("application","command","text2")> app_text2_hash; // key is get_hash_key(value(command))
	mime_hash_t<string_hash_c("application","command","output")> app_output_hash; // key is get_hash_key(value(command))
	mime_hash_t<string_hash_c("application","command","output_ext")> app_output_ext_hash; // key is get_hash_key(value(command))

*/

        gchar *mime_magic (const gchar *);

        gboolean load_hashes_from_cache(void);
        long long read_cache_sum (void);
        gchar *get_cache_path (const gchar *);
        void load_text_hash(GHashTable *, const gchar *);

        void mime_build_hashes (void);   
        void destroy_application_hash_sfx (void);
        void destroy_application_hash_type (void);
        gchar *get_hash_key (const gchar * );
        void add_type_to_hashtable(const gchar *, const gchar *, gboolean );
        const gchar *mimeable_file (struct stat *);
        void save_text_cache(GHashTable *, const gchar *);
        const gchar *locate_mime_t (const gchar * );
        gchar **locate_apps (const gchar * );
        void *put_mimetype_in_hash(const gchar *, const gchar *);
        gchar *mimetype1(const gchar *);
        gchar *mimetype2(const gchar *);
        gchar *get_hash_key_strstrip (void *);
        long long get_cache_sum (void);
        void write_cache_sum (long long);
        gint check_dir (char *);

        static pthread_mutex_t cache_mutex;
        static pthread_mutex_t mimetype_hash_mutex;
        static pthread_mutex_t alias_hash_mutex;
        static pthread_mutex_t application_hash_mutex;


        static GHashTable *generic_icon_hash;
        static GHashTable *mimetype_hash;
        static GHashTable *alias_hash;
        static GHashTable *application_hash_type;
        static GHashTable *application_hash_sfx;
        static GHashTable *application_hash_icon;
        static GHashTable *application_hash_text;
        static GHashTable *application_hash_text2;
        static GHashTable *application_hash_output;
        static GHashTable *application_hash_output_ext;

};



#endif

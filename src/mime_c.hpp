#ifndef MIME_C_HPP
#define MIME_C_HPP
#include "xffm+.h"

#include "utility_c.hpp"
#include "mime_magic_c.hpp"
#include "lite_c.hpp"

#define USER_RFM_DIR            g_get_user_config_dir(),"rfm"
#define SYSTEM_MODULE_DIR	PACKAGE_DATA_DIR,"rfm","rmodules"

#include "mime_hash_c.hh"
#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"rfm"
#define APPLICATION_MIME_FILE 	SYSTEM_MODULE_DIR,"mime-module.xml"
#define USER_APPLICATIONS 	USER_RFM_DIR,"user-applications.2"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <errno.h>
#include <dbh.h>

enum {
    SFX,
    ALIAS,
    COMMAND,
    GENERIC_ICON,
    COMMAND_ICON,
    COMMAND_TEXT,
    COMMAND_TEXT2,
    COMMAND_OUTPUT,
    COMMAND_OUTPUT_EXT,
    MIME_HASHES
};

#define GET_COMMAND_TEXT(X) mime_c::command_text_hash.lookup(X, mime_c::hash_data[COMMAND_TEXT])
#define GET_COMMAND_TEXT2(X) mime_c::command_text2_hash.lookup(X, mime_c::hash_data[COMMAND_TEXT2])
#define GET_COMMAND_ICON(X) mime_c::command_icon_hash.lookup(X, mime_c::hash_data[COMMAND_ICON])
#define GET_COMMAND_OUTPUT(X) mime_c::command_output_hash.lookup(X, mime_c::hash_data[COMMAND_OUTPUT])
#define GET_COMMAND_OUTPUT_EXT(X) mime_c::command_ext_hash.lookup(X, mime_c::hash_data[COMMAND_OUTPUT_EXT])

#define LOCATE_APPS(X) mime_application_hash_c<txt_hash_t>::lookup(type, hash_data[COMMAND])
#define LOCATE_MIME_T(X) mime_sfx_hash_c<txt_hash_t>::get_type_from_sfx(X, hash_data[SFX])
#define MIME_GET_ALIAS_TYPE(X) mime_aliashash_c<txt_hash_t>::get_alias_type(X, hash_data[ALIAS])


class mime_c: virtual utility_c, public lite_c, public mime_magic_c {
    public:

        static void init_hashes(void);
        static void create_hashes(xmlDocPtr);
        static void build_hashes(xmlDocPtr, const gchar *);

        static void mime_build_hashes (void); // to be zapped  

        static void add_type_to_hashtable(const gchar *, const gchar *, gboolean );

   
        static gchar *mime_type (const gchar *);
        static gchar *mime_type (const gchar *, struct stat *);
        static gchar *mimetype1(const gchar *);
        static gchar *mimetype2(const gchar *);
        static gchar *get_hash_key (const gchar * );
        static const gchar *mimeable_file (struct stat *);
        static void *put_mimetype_in_hash(const gchar *, const gchar *);
        static gchar *mime_magic (const gchar *);
        static gchar *mime_function(const gchar *, const gchar *);
        static const gchar *find_mimetype_in_hash(const gchar *);
        static gchar *mime_command (const char *p);
        static gboolean mime_is_valid_command (const char *);
        static const gchar *get_mimetype_iconname(const gchar *);

    public:
        static txt_hash_t hash_data[MIME_HASHES];

        static mime_sfx_hash_c<const txt_hash_t> app_sfx_hash; 
	static mime_aliashash_c<const txt_hash_t> app_alias_hash;
	static mime_hash_c<const txt_hash_t> app_genericicon_hash; 
	
        static mime_application_hash_c<const txt_hash_t> app_command_hash;

	static mime_command_hash_c<const txt_hash_t> command_icon_hash;
	static mime_command_hash_c<const txt_hash_t> command_text_hash;
	static mime_command_hash_c<const txt_hash_t> command_text2_hash;
	static mime_command_hash_c<const txt_hash_t> command_output_hash;
	static mime_command_hash_c<const txt_hash_t> command_ext_hash;

        const gchar *mime_command_text (gchar *p) ;
        const gchar *mime_command_text2 (gchar *p);
        const gchar *mime_command_icon (gchar *p);
        const gchar *mime_command_output (gchar *p);
        const gchar *mime_command_output_ext (gchar *p);
        
        gchar **mime_apps (const char *p);
        void *mime_add (gchar *, gchar *);
        void *mime_append (gchar *, gchar *);
        gchar *mime_mk_command_line (const gchar *, const gchar *);
        gchar *mime_mk_terminal_line (const gchar *p);
        gboolean generate_caches (void);
        void *mime_gencache(gchar *);

    private:
        //void create_hash(txt_hash_t &,  xmlDocPtr, const gchar *, const gchar *);
        //void create_hash(txt_hash_t &,  xmlDocPtr, const gchar *, const gchar *, const gchar *);

        
	/*
	mime_hash_t<string_hash_c("key","value",NULL)> app_sfx_hash; // key is g_utf8_strdown ((gchar *)value(value), -1);
	mime_hash_t<string_hash_c("alias","type",NULL)> app_alias_hash; // key is g_utf8_strdown ((gchar *)value(type), -1);

	mime_hash_t<string_hash_c("generic-icon","name",NULL)> app_generic_icon_hash; // key is g_utf8_strdown ((gchar *)value(name), -1);

	mime_hash_t<string_hash_c("application",NULL,"command")> app_type_hash; // key is get_hash_key(type)
	mime_hash_t<string_hash_c("application","command","icon")> command_icon_hash; // key is get_hash_key(value(command))
	mime_hash_t<string_hash_c("application","command","text")> command_text_hash; // key is get_hash_key(value(command))
	mime_hash_t<string_hash_c("application","command","text2")> command_text2_hash; // key is get_hash_key(value(command))
	mime_hash_t<string_hash_c("application","command","output")> command_output_hash; // key is get_hash_key(value(command))
	mime_hash_t<string_hash_c("application","command","output_ext")> command_ext_hash; // key is get_hash_key(value(command))

*/


        gchar *get_hash_key_strstrip (void *);

        static pthread_mutex_t alias_hash_mutex;


        static GHashTable *application_hash_type;

    private:


        static pthread_mutex_t mimetype_hash_mutex;
        static GHashTable *mimetype_hash;

};



#endif

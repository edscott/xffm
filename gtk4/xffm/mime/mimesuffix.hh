#ifndef XF_MIMESUFFIX_HH
#define XF_MIMESUFFIX_HH

#include <string.h>
#include <errno.h>

#ifndef FREEDESKTOP_GLOBS
# define FREEDESKTOP_GLOBS "FREEDESKTOP_GLOBS"
#endif

#ifndef FREEDESKTOP_ICONS
# define FREEDESKTOP_ICONS "FREEDESKTOP_ICONS"
#endif

#ifndef FREEDESKTOP_ALIAS
# define FREEDESKTOP_ALIAS "FREEDESKTOP_ALIAS"
#endif
namespace xf {

static GHashTable *mimeHashSfx=NULL;
    
static GHashTable *mimeHashAlias=NULL;
static GHashTable *mimeHashIcon=NULL;


class MimeSuffix {
    static const gchar *
    lookupBySuffix(const gchar *file, const gchar *sfx){
        gchar *key;
        if (sfx) key = g_strdup(sfx);
        else key = g_path_get_basename (file);
        // duplicate suffix?
        auto constantType = (const gchar *)g_hash_table_lookup (mimeHashSfx, key);
        g_free (key);
        if (constantType) return constantType;
        return NULL;
    }

    static void
    mimeBuildHashes (void) {

        mimeHashSfx = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

        mimeHashAlias = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
        mimeHashIcon = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);


        FILE *input;
        if ((input = fopen(FREEDESKTOP_GLOBS, "r")) != NULL) {
            gchar buffer[256]; memset(buffer, 0, 256);
            while (fgets(buffer, 255, input) && !feof(input)){
                if (!strchr(buffer, ':'))continue;
                gchar **x = g_strsplit(buffer, ":", -1);
                gint offset = 0;
                if (strchr(x[1], '\n')) *(strchr(x[1], '\n')) = 0;
                if (strncmp(x[1], "*.", strlen("*."))==0) offset = strlen("*.");
                const gchar *key = x[1]+offset;
                add2sfx_hash(key,x[0]);
                g_strfreev(x);
            }
            fclose(input);
        }else ERROR_("Cannot open %s\n", FREEDESKTOP_GLOBS);

        if ((input = fopen(FREEDESKTOP_ICONS, "r")) != NULL) {
            gchar buffer[256]; memset(buffer, 0, 256);
            while (fgets(buffer, 255, input) && !feof(input)){
                if (!strchr(buffer, ':'))continue;
                gchar **x = g_strsplit(buffer, ":", -1);
                if (strchr(x[1], '\n')) *(strchr(x[1], '\n')) = 0;
                const gchar *key = x[0];
                if(key) {
                     TRACE("ICON mime-module,replacing hash element \"%s\" with key %s --> %s\n", 
                                    x[0], key, x[1]);
                     g_hash_table_replace (mimeHashIcon,  g_strdup(key), g_strdup(x[1]));
                }
                g_strfreev(x);
            }
            fclose(input);
        }else ERROR_("Cannot open %s\n", FREEDESKTOP_ICONS);

        if ((input = fopen(FREEDESKTOP_ALIAS, "r")) != NULL) {
            gchar buffer[256]; memset(buffer, 0, 256);
            while (fgets(buffer, 255, input) && !feof(input)){
                if (!strchr(buffer, ' '))continue;
                gchar **x = g_strsplit(buffer, " ", -1);
                if (strchr(x[1], '\n')) *(strchr(x[1], '\n')) = 0;
                const gchar *key = x[0];
                if(key) {
                     TRACE("ALIAS mime-module,replacing hash element \"%s\" with key %s --> %s\n", 
                                    x[0], key, x[1]);
                     g_hash_table_replace (mimeHashAlias,  g_strdup(key), g_strdup(x[1]));
                }
                g_strfreev(x);
            }
            fclose(input);
        } else ERROR_("Cannot open %s\n", FREEDESKTOP_ALIAS);
/*
        // FIXME: break the following code into routines...
        // mimetype registered applications...
        // /usr/share/applications
        // /usr/local/share/applications
        const gchar *directories[] = {
            "/usr/share/applications",
            "/usr/local/share/applications"
        };
        for (int i=0; i<2; i++) {
            processApplicationDir(directories[i]);
        }
        
 */
            
    }

public:

    static void
    add2sfx_hash(const gchar *key, const gchar *value){
        // single line function access from outside.
        if (!mimeHashSfx) mimeBuildHashes();
        g_hash_table_replace (mimeHashSfx, g_strdup(key), g_strdup(value));
    }

    static const gchar *
    mimeIcon (const gchar *file){
        const gchar *retval = locate_icon(file);
        if (retval) {
            TRACE("locate_icon: %s --> %s\n", file, retval);
        }
        return retval;
   } 

    static gchar *
    getBasicType(const gchar *mimetype) {
        gchar *retval;
        const gchar *alias = (const gchar *)g_hash_table_lookup (mimeHashAlias, mimetype);
        if (alias) retval = g_strdup(alias);
        else retval = g_strdup(mimetype);
        return retval;
    }

    static const gchar *
    locate_icon (const gchar *mimetype) {
        const gchar *icon;
        if (!mimeHashIcon) {
            ERROR_("!mimeHashIcon\n");
            return NULL;
        }
        TRACE("mime-module, locate_icon looking in icon hash for \"%s\"\n", mimetype);
        
        const gchar *basicType = getBasicType(mimetype);
        icon = (const gchar *)g_hash_table_lookup (mimeHashIcon, mimetype);

        if (!icon){
            const gchar *alias = getBasicType(mimetype);
            if (alias) {
                icon = (const gchar *)g_hash_table_lookup (mimeHashIcon, mimetype);
            }
        }
        return icon;
    }

public:
    static gchar *
    mimeType (const gchar * file) {
    //extensionMimeType (const gchar * file) {
        const gchar *type = NULL;
        gchar *p;
        if (!mimeHashSfx) {
            mimeBuildHashes();
            if (!mimeHashSfx) {
                ERROR_("!mimeHashSfx\n");
                return NULL;
            }
        }
        TRACE("mime-module, extensionMimeType looking in sfx hash for \"%s\"\n", file);

        
        ///  look in sfx hash...
        gchar *basename = g_path_get_basename (file);
        if (strchr (basename, '.')) p = strrchr (basename, '.')+1;
        else {
            // no file extension.
            return NULL;
        }
#if 10
        auto sfx = g_utf8_strdown (p, -1);
        type = lookupBySuffix(NULL, sfx);
        g_free(basename);
        if (!type) return g_strdup(_("unknown"));
        return g_strdup(type);
    }
#else

        // Right to left:
        for (;p && *p; p = strchr (p, '.'))
        {
            while (*p=='.') p++;
            if (*p == 0) break;
            
            gchar *sfx;
            /* try all lower case (hash table keys are set this way) */
            sfx = g_utf8_strdown (p, -1);
           // TRACE("mime-module, lOOking for \"%s\" with key=%s\n", sfx, key);

            type = lookupBySuffix(NULL, sfx);
            if(type) {
                DBG("mime-module(2), FOUND sfx %s: %s\n", sfx, type);
                g_free (sfx);
                return g_strdup(type);
            } 
            g_free (sfx);
        }
        // Left to right, test all extensions.
        gchar **q = g_strsplit(basename, ".", -1);
        gchar **q_p = q+1;
        
        for (;q_p && *q_p; q_p++){
            gchar *sfx;
            /* try all lower case (hash table keys are set this way) */
            sfx = g_utf8_strdown (*q_p, -1);
            type = lookupBySuffix(NULL, sfx);
            if(type) {
                TRACE("mime-module(3), FOUND %s: %s\n", sfx, type);
                g_free (sfx);
                g_strfreev(q);
                return g_strdup(type);
            }
            g_free (sfx);
        }
        g_strfreev(q);
        return NULL;
    }
#endif

};

}
#endif

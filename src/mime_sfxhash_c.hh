#ifndef MIME_SFXHASH_C_HH
#define MIME_SFXHASH_C_HH

#include "mime_hash_c.hh"
struct string4_hash_t {
	    xmlDocPtr doc;
            gchar const *keys[4];
            gchar *mimefile;
	    GHashTable *hash;
};

template <class Type>
class mime_sfxhash_c: public  mime_hash_t<Type>{
#if 0
    public:
// XXX this is the site to modify...
static const gchar *
locate_mime_t (const gchar * file, Type T ) {
    const gchar *type;
    const gchar *p;
    //load_hashes ();
    TRACE("mime-module, locate_mime_t looking in sfx hash for \"%s\"\n", file);

    if (!strchr(file,'.')) return NULL;
    ///  now look in sfx hash...
    gchar *basename = g_path_get_basename (file);

    p = strchr (basename, '.');
    // Right to left:
    for (;p && *p; p = strchr (p, '.'))
    {
        while (*p=='.') p++;
        if (*p == 0) break;
        
        gchar *sfx;
        /* try all lower case (hash table keys are set this way) */
        sfx = g_utf8_strdown (p, -1);
        gchar *key = get_hash_key (sfx,T);
        TRACE("mime-module, lOOking for \"%s\" with key=%s\n", sfx, key);

        type = (const gchar *)g_hash_table_lookup (get_hash(T), key);
        g_free (key);
        if(type) {
            NOOP(stderr,"mime-module, FOUND %s: %s\n", sfx, type);
            g_free (sfx);
            g_free (basename);
            return type;
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
	gchar *key = get_hash_key (sfx,T);
	type = (const gchar *)g_hash_table_lookup (get_hash(T), key);
	g_free (key);
	if(type) {
	    NOOP(stderr,"mime-module(2), FOUND %s: %s\n", sfx, type);
	    g_free (sfx);
	    g_free (basename);
	    g_strfreev(q);
	    return type;
	}
	g_free (sfx);
    }
    g_strfreev(q);
    g_free (basename);
    return NULL;
}
#endif
};

#endif

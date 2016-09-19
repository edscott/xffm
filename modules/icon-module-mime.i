#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 *
 * (c) Edscott Wilson Garcia 2001-2011.
 *
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; 
 */


static void
start_element (GMarkupParseContext * context,
               const gchar * element_name,
               const gchar ** attribute_names, 
	       const gchar ** attribute_values, 
	       gpointer user_data, 
	       GError ** error) 
{
    const gchar *name = NULL;
    const gchar *icon = NULL;
    NOOP ("icon-module, start -> %s\n",element_name); 
    /* here we should have the type and icon attributes set,
     * but they are not :-(
     * */
    if(strcmp (element_name, "mime-type")) return;
    if(attribute_names) {
        if(attribute_names[0] && attribute_values[0]){
            name = attribute_values[0];
	}
        if(attribute_names[1] && attribute_values[1]){
	    icon = attribute_values[1];
	}
        if(name && icon) {
	    gchar *hash_key=rfm_get_hash_key(name, 0);
            g_hash_table_replace (basename_hash, hash_key, g_strdup(icon));
            NOOP(stderr, "0x%x: hashing %s=%s (%s)\n", 
		    GPOINTER_TO_INT(g_thread_self()), name, icon, hash_key);
        } else if(name) {
            NOOP("icon-module, no icon defined for %s\n", name);
        }
    }
    return;
}

static void
glib_parser (const gchar * mimefile) {
    FILE *f;
    size_t l;
    gchar line[81];
    GMarkupParseContext *context;
    GError *error = NULL;
    GMarkupParser parser = {
        start_element,
        NULL,
        NULL,                   /*text_fun, */
        NULL,
        NULL
    };
    gpointer user_data = NULL;

    NOOP("glib_parser(icon-module): parsing %s\n", mimefile);
    context = g_markup_parse_context_new (&parser, 0, user_data, NULL);
    f = fopen (mimefile, "r");
    if(!f) {
        DBG ("cannot open %s\n", mimefile);
        return;
    }
    while(!feof (f) && (l = fread (line, 1, 80, f)) != 0) {
        line[l] = 0;
        g_markup_parse_context_parse (context, line, l, &error);
    }
    fclose (f);

    g_markup_parse_context_free (context);
}

static gboolean
create_icon_hash (const gchar * mimefile) {

    NOOP("icon-module, create_icon_hash...\n");
    if(!mimefile) {
        return FALSE;
    }
    rfm_rw_lock_writer_lock(&basename_lock);
    if(basename_hash) {
	return TRUE;
    }

    basename_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    glib_parser (mimefile);
    rfm_rw_lock_writer_unlock(&basename_lock);
    NOOP("icon-module, basename_hash created!\n");
    return TRUE;
}

/************************************************************************/

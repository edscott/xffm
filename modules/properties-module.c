/*
 * Edscott Wilson Garcia Copyright 2001-2011
 *
 * Copyright (C) 1999 Rasca, Berlin
 * EMail: thron@gmx.de
 *
 * Olivier Fourdan (fourdan@xfce.org)
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
 * along with this program; .
 */
#define noRECURSIVE_BUTTON
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "rodent.h"
#include "rfm_modules.h"
/* this should be first 2 lines after headers: */
G_MODULE_EXPORT LIBRFM_MODULE

#define box_pack_start(box,w) \
	gtk_box_pack_start(GTK_BOX(box),w,TRUE,FALSE,0)
#define box_pack_end(box,w) \
	gtk_box_pack_end(GTK_BOX(box),w,TRUE,FALSE,0)
#define X_PAD 8
#define Y_PAD 1
#define TBL_XOPT GTK_FILL
#define TBL_YOPT GTK_SHRINK
/* flags
 */
#define IS_MULTI		1
#define IS_STALE_LINK	2
/* question dialogs: */
#define DLG_OK			0x11
#define DLG_CLOSE		0x120
#define DLG_ALL			0x140
#define DLG_SKIP		0x180
#define DLG_RC_CANCEL		0
#define DLG_RC_OK		1
#define DLG_RC_ALL		2
#define DLG_RC_CONTINUE		3
#define DLG_RC_SKIP		4
#define DLG_RC_DESTROY		5
#define DLG_RC_RECURSIVE	6
/* */
#define DLG_OK_CANCEL	(DLG_OK|DLG_CANCEL)
#define DLG_YES_NO	(DLG_YES|DLG_NO)
/* */

#include "properties-module.i"

// properties module requires a GList of record_entry_t as input to function.
// this GList and associated memory must be malloc'd. Memory cleanup 
// is done by this function (to be thread safe).
// in other words: properties are just displayed and toggled, but must
// be figured out by stat() before hand. This is for program speed optimization
// reasons.
//
// The input g_list should be malloc'd. It will be freed when no longer used.
// each select_list->data (record_entry_t *) must be private and will be
// freed on dialog destroy by thread.
//
// 
//
//

G_MODULE_EXPORT void *
module_active (void){
    return GINT_TO_POINTER(1);
}

static properties_t *
setup_properties_p(widgets_t *widgets_p){
    if (!widgets_p){
	DBG("setup_properties_p(widgets_p==NULL])!\n");
	return NULL;
    }
    view_t *view_p=widgets_p->view_p;

    properties_t *properties_p;
    properties_p=(properties_t *)malloc(sizeof(properties_t));
    if (!properties_p) g_error("malloc: %s", strerror(errno));
    memset (properties_p, 0, sizeof(properties_t));
    // widgets may be invalid since properties dialog
    // may remain after view is destroyed:
    // properties_p->widgets_p = widgets_p;
    
    // copy selection list to private thread safe selection_list... 
    GSList *tmp=view_p->selection_list;
    for (;tmp && tmp->data; tmp=tmp->next){
	record_entry_t *en=tmp->data;
	record_entry_t *new_en = rfm_copy_entry(en);
	properties_p->select_list=g_slist_append(properties_p->select_list, new_en);
	// updated stat information:
	if(new_en->path && new_en->st){
	    if (stat(new_en->path, new_en->st) < 0) {
                DBG("setup_properties_p(); stat %s (%s)\n", new_en->path, strerror(errno));
                continue;
            }
	}
    }
    if (g_slist_length(view_p->selection_list)==1){
	NOOP("selection list count ==1\n");
	// entry contains the disk valid st record
        properties_p->en = (record_entry_t *) (properties_p->select_list)->data;
        if(!properties_p->en) {
            DBG ("%s : invalid entry (!en || !rfm_g_file_test(en->path,G_FILE_TEST_EXISTS)\n", strerror (EINVAL));
            GSList *list = properties_p->select_list;
            for (;list && list->data; list=list->next){
                record_entry_t *en = list->data;
                rfm_entry_destroy(en);
            }
            g_slist_free(properties_p->select_list);
            g_free(properties_p);
	    return GINT_TO_POINTER(1);
        }
	// this st record will be the value set by the user in dialog,
	// not saved to disk until button pressed and dialog closed.
        memcpy (&properties_p->st, properties_p->en->st, sizeof (struct stat));
	properties_p->module_txt = 
	    rfm_natural (PLUGIN_DIR, view_p->module, properties_p->en, "entry_tip");

	// short circuit to previously loaded preview pixbuf
	// will not work here because our selection list contains
	// record_entry_t elements, not population_p
    }
    return properties_p;
}

G_MODULE_EXPORT 
void *
do_prop (void *p) {

    if (!p){
	DBG("p (widgets_p) == NULL\n");
	return GINT_TO_POINTER(1);
    }
    widgets_t *widgets_p=p;
    //view_t *view_p=widgets_p->view_p;
    properties_t *properties_p=setup_properties_p(widgets_p);
    NOOP("do_prop\n");
    if(properties_p->select_list==NULL){
	DBG("select_list==NULL\n");
        g_free(properties_p);
        return GINT_TO_POINTER(1);
    }

    if(!g_slist_length (properties_p->select_list)) {
	DBG("select_count == 0\n");
        g_slist_free(properties_p->select_list);
        g_free(properties_p);
        return GINT_TO_POINTER(1);
    } 
    rfm_context_function(dlg_prop,  (void *)properties_p);

    return GINT_TO_POINTER(1);

}

G_MODULE_EXPORT const gchar *
g_module_check_init (GModule * module) {
#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
# endif
    NOOP ("domain=%s", GETTEXT_PACKAGE);
#endif
    return NULL;
}



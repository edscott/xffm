/*
 * Copyright (C) 2002-2012 Edscott Wilson Garcia
 * EMail: edscott@users.sf.net
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "rfm.h"
/* this should be first 2 lines after headers: */
G_MODULE_EXPORT LIBRFM_MODULE 

/* public symbols */

gchar *
rfm_file_completion(widgets_t *widgets_p,  gchar *token);
gchar *
rfm_get_tilde_dir(const gchar *token);
gchar *
rfm_bash_complete(widgets_t *widgets_p,  const gchar *in_token, void *len_p);
gchar *
rfm_history_completion (widgets_t * widgets_p,  gchar *complete);
    
/* private symbols */

#include "completion.i"
gchar *
rfm_file_completion(widgets_t *widgets_p,  gchar *token){
    if (!token) {
	msg_help_text(widgets_p);
	return NULL;
    }
    if (token) g_strchug (token);
    if (strlen(token) == 0) {
	msg_help_text(widgets_p);
	return NULL;
    }
    gint match_count;
    return bash_file_completion(widgets_p, token, &match_count);
}

gchar *
rfm_get_tilde_dir(const gchar *token){
    struct passwd *pw;
    gchar *tilde_dir = NULL;
    while((pw = getpwent ()) != NULL) {
	gchar *id = g_strdup_printf("~%s/", pw->pw_name);
	if (strncmp(token, id, strlen(id))==0){
	    tilde_dir = g_strdup_printf("%s/", pw->pw_dir);
	    g_free(id);
	    break;
	}
	g_free(id);
    }
    endpwent ();
    return tilde_dir;
}

gchar *
rfm_bash_complete(widgets_t *widgets_p,  const gchar *in_token, void *len_p){
    if (!valid_token(widgets_p,  in_token)) return NULL;
    gint token_len = GPOINTER_TO_INT(len_p);
	NOOP("rfm_bash_complete(%d) ...%s\n", token_len, in_token);
    gchar *token = g_strdup(in_token);
    gchar *tail = NULL;
    if (token_len) {
	tail = g_strdup(in_token + token_len);
	token[token_len] = 0;
    }
    gint matches;
    gchar *suggest = bash_complete_with_head(widgets_p, token, &matches);
    if (suggest){
	if (tail) {
	    gchar *g = g_strconcat(suggest, tail, NULL);
	    g_free(suggest);
	    suggest = g;
	} else if (matches == 1 && suggest[strlen(suggest)-1] != '/') {
	    gchar *g = g_strconcat(suggest, " ", NULL);
	    g_free(suggest);
	    suggest = g;
	}
    }
    g_free(token);
    g_free(tail);
    return suggest;
}


gchar *
rfm_history_completion (widgets_t * widgets_p,  gchar *complete) {
    if (!widgets_p) return NULL;
    view_t *view_p = widgets_p->view_p;
    NOOP(stderr, "rfm_history_completion\n");
    if (!complete) {
	msg_help_text(widgets_p);
	return NULL;
    }
    if (complete) g_strchug (complete);
    if (strlen(complete) == 0) {
	msg_help_text(widgets_p);
	return NULL;
    }
    GSList *matches=NULL;
    GList *p;
    for(p = view_p->sh_command; p && p->data; p = p->next) {
	gchar *data = (char *) p->data;
	if(strncmp (complete, data, strlen (complete)) == 0) {
	    if(g_slist_find_custom (matches, data, ya_strcmp) == NULL) {
		NOOP(stderr, "COMPLETE: match! %s <= %s\n", complete, data);
		matches = g_slist_append (matches, p->data);
	    }
	}
	NOOP(stderr, "COMPLETE: ?? %s\n", data);
    }

    gchar *suggest = complete_it(widgets_p, &matches, MATCH_HISTORY);
    g_slist_free (matches);
    
    if(!suggest && strncmp (complete, "history", strlen (complete)) == 0) {
	suggest = g_strdup ("history");
    }
    return suggest;
}



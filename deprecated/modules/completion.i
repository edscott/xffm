#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 * Copyright (C) 2003-2012 Edscott Wilson Garcia
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

#define BASH_COMPLETION_OPTIONS maximum_completion_options()
static glong
maximum_completion_options(void){
    const gchar *env_maximum=getenv("RFM_MAXIMUM_COMPLETION_OPTIONS");
    if (!env_maximum || !strlen(env_maximum)) return 104;
    errno=0;
    glong amount = strtol(env_maximum, (char **) NULL, 10);
    if (errno) return 104;
    return amount;
}


static void
msg_too_many_matches(widgets_t *widgets_p){
    if (!widgets_p) return;
    rfm_show_text(widgets_p);
    gchar *message1=g_strdup_printf("%s (> %ld)",
	    _("Too many matches"), BASH_COMPLETION_OPTIONS);
    gchar *message2=g_strdup_printf("%s %s", _("Options:"), message1);
    rfm_diagnostics (widgets_p, "xffm/stock_dialog-info", NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/blue", "[",  _("Text Completion"),"] ", NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/red", message2, "\n", NULL);
    g_free(message1);
    g_free(message2);
}

static const gchar *
get_match_type_text(gint match_type){
    switch (match_type){
	case MATCH_COMMAND:
	    return _("Command");
	    break;
	case MATCH_FILE:
	    return _("File");
	    break;
	case MATCH_HISTORY:
	    return _("Command history");
	    break;
	case MATCH_USER:
	    return _("User");
	    break;
	case MATCH_VARIABLE:
	    return _("Variable");
	    break;
	case MATCH_HOST:
	    return _("Host");
	    break;
    }
    return "WTF";
}
    
static void
msg_show_match(widgets_t *widgets_p, gint match_type, const gchar *match){
    if (!widgets_p) return;
    if (!match) {
	const gchar *option_type = get_match_type_text(match_type);
	rfm_diagnostics (widgets_p, "xffm/stock_dialog-error",NULL);
	match = _("Found no match");
	rfm_diagnostics (widgets_p, "xffm_tag/red", "(", 
		option_type, ")", NULL);
    } 
    rfm_diagnostics (widgets_p, "xffm_tag/blue", " ", match, "\n", NULL);
    
}

static void
msg_help_text(widgets_t *widgets_p){
    if (!widgets_p) return;
    rfm_show_text(widgets_p);

    rfm_diagnostics (widgets_p, "xffm/stock_dialog-info", NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/green",
	    _("Completion mode:"), 
	    " bash ", _("command"), "/", _("file"), " --> ", NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/red", "TAB.",  NULL);



    rfm_diagnostics (widgets_p, "xffm_tag/green", "   ", 
	    _("Completion mode:")," ", _("Command history")," --> ", NULL); 
    rfm_diagnostics (widgets_p, "xffm_tag/red", "CTRL+TAB.\n",  NULL);
}

static void
msg_result_text(widgets_t *widgets_p, gint match_type){
    if (!widgets_p) return;
    rfm_show_text(widgets_p);
    rfm_diagnostics (widgets_p, "xffm/stock_dialog-info", NULL);
    const gchar *option_type = get_match_type_text(match_type);

    rfm_diagnostics (widgets_p, "xffm_tag/green", _("Options >>"), NULL);
    rfm_diagnostics (widgets_p, "xffm_tag/red", "(", option_type, ")\n", NULL);
}

static gint
ya_strcmp (
    gconstpointer a,
    gconstpointer b
) {
    return strcmp ((char *) a, (char *) b);
}

static const gchar *
get_workdir(widgets_t *widgets_p){
    if (!widgets_p) return g_get_home_dir();
    view_t *view_p=widgets_p->view_p;
    if (!view_p) return g_get_home_dir();
    record_entry_t *en=view_p->en;
    if (!en || !en->path) return g_get_home_dir();
    if (!rfm_g_file_test(en->path, G_FILE_TEST_IS_DIR)) return g_get_home_dir();
    return en->path;
}

static int
length_equal_string(const gchar *a, const gchar *b){
    int length=0;
    int i;
    for (i = 0; i < strlen(a) && i < strlen(b); i++){
	if (strncmp(a,b,i+1)) {
	    length=i;
	    break;
	} else {
	    length=i+1;
	}
    }
     NOOP(stderr, "%s --- %s differ at length =%d\n", a,b,length);
   return length;
}

static gchar *
list_matches (widgets_t * widgets_p, GSList **matches_p, gint match_type){
    *matches_p = g_slist_sort (*matches_p, ya_strcmp);
    GSList *a;
    GSList *b;
    int length;
    GSList *p = *matches_p;
    char *suggest = g_strdup ((gchar *) (p->data));
    int equal_length=strlen(suggest);
    for(a = *matches_p; a && a->data; a = a->next) {
	NOOP(stderr, "comparing a %s\n", (gchar *)a->data);
	for(b = a->next; b && b->data; b = b->next) {
	    NOOP(stderr, "comparing b %s\n", (gchar *)b->data);
	    length=length_equal_string(
		    (const gchar *)(a->data), (const gchar *)(b->data));
	    if(length < equal_length) {
		equal_length = length;
	    }
	    NOOP(stderr, "comparing %s to %s: length=%d\n", (gchar *)a->data, (gchar *)b->data, length);
	}
    }
    suggest[equal_length]=0;
    NOOP(stderr, "string=%s, equal_length=%d, suggest=%s\n",
	    (gchar *) (p->data), equal_length, suggest);


    int i;
    for(i = 1, p = *matches_p; p && p->data; p = p->next, i++) {
	msg_show_match(widgets_p, match_type, (const gchar *)p->data);
    }
    return suggest;
}

static gchar *
complete_it(widgets_t * widgets_p, GSList **matches_p, gint match_type){
        gchar *suggest = NULL;
        if(*matches_p) {
            NOOP( "COMPLETE: matches %d\n", g_slist_length (*matches_p));
            if(g_slist_length (*matches_p) == 1) {
                suggest = g_strdup ((*matches_p)->data);
            } else {
		if (widgets_p) rfm_show_text(widgets_p);
		msg_result_text(widgets_p, match_type);
		suggest=list_matches(widgets_p, matches_p, match_type);
            }
        }
	return suggest;
}
static gchar *
bash_file_completion(widgets_t *widgets_p,  
	const char *in_file_token, gint *match_count_p){
    if (!in_file_token) return NULL;
    if (strlen(in_file_token) == 0) return NULL;

    gchar *file_token=NULL;
    if (*in_file_token == '~' && strchr(in_file_token, '/')){
	if (strncmp(in_file_token, "~/", strlen("~/"))==0){
	    file_token = g_strconcat(g_get_home_dir(), in_file_token+1, NULL);
	} else {
	    gchar *dir = rfm_get_tilde_dir(in_file_token);
	    if (dir) file_token = g_strconcat(dir, strchr(in_file_token, '/')+1, NULL);
	    g_free(dir);
	}
    } 
    if (!file_token) {
	file_token = g_strdup(in_file_token);
    }
    
  
    GSList *matches=NULL;


    gchar *directory;
    gchar *relative_directory=NULL;
    if (g_path_is_absolute(file_token)) {
	directory = g_strdup_printf("%s*", file_token);

    } else {
	if (widgets_p) {
	    directory = g_strdup_printf("%s/%s*", get_workdir(widgets_p), file_token);
	} else {
	    gchar *d = g_get_current_dir();
	    directory = g_strdup_printf("%s/%s*", d, file_token);
	    g_free(d);
	}
	relative_directory = g_path_get_dirname(file_token);
	if (strcmp(relative_directory, ".")==0 && 
		strncmp(file_token, "./", strlen("./")) != 0) {
	    g_free(relative_directory);
	    relative_directory=NULL;
	}
    }
    
    NOOP(stderr, "file_token=%s\ndirectory=%s\n", file_token, directory);

    
    
    glob_t stack_glob_v;
    gint flags=GLOB_NOESCAPE;

    glob(directory, flags, NULL, &stack_glob_v);
    g_free(directory);

    if (stack_glob_v.gl_pathc == 0){
	NOOP(stderr, "NO MATCHES\n");
	    msg_show_match(widgets_p, MATCH_FILE, NULL);
    } else if (stack_glob_v.gl_pathc <= BASH_COMPLETION_OPTIONS){
	gint i;
	for (i=0; i<stack_glob_v.gl_pathc; i++){
	    gboolean isdir=rfm_g_file_test(stack_glob_v.gl_pathv[i], G_FILE_TEST_IS_DIR);
	    gchar *base;
	    if (g_path_is_absolute(file_token)) {
		base = g_strdup(stack_glob_v.gl_pathv[i]);
	    } else {
		base = g_path_get_basename(stack_glob_v.gl_pathv[i]);
		if (relative_directory) {
		    gchar *b=g_build_filename(relative_directory, base, NULL);
		    g_free(base);
		    base=b;
		}
	    }
	    if (isdir) {
		gchar *d=g_strconcat(base, "/", NULL);
		g_free(base);
		base = d;
	    }

	    //rfm_diagnostics (widgets_p, "xffm_tag/green", "file option = ", base, "\n", NULL);
	    matches = g_slist_append (matches, base);
	}
    } else {
	msg_too_many_matches(widgets_p);
    }

	    
    g_free(relative_directory);
    globfree(&stack_glob_v);
    gchar *suggest = complete_it(widgets_p, &matches, MATCH_FILE);
    NOOP(stderr, "match = \"%s\"\n",suggest? suggest : "no match!");


    if (suggest) {
	*match_count_p = g_slist_length(matches);
    } else *match_count_p = 0;

    GSList *p=matches;
    for (;p && p->data; p=p->next){
	g_free(p->data);
    }
    g_slist_free (matches);

    g_free(file_token);
    return suggest;
}


static gchar *
bash_exec_completion(widgets_t *widgets_p, const char *in_token, gint *match_count_p){
    GSList *matches=NULL;

    gchar *token=NULL;
    if (*in_token == '~' && strchr(in_token, '/')){
	if (strncmp(in_token, "~/", strlen("~/"))==0){
	    token = g_strconcat(g_get_home_dir(), in_token+1, NULL);
	} else {
	    gchar *dir = rfm_get_tilde_dir(in_token);
	    if (dir) token = g_strconcat(dir, strchr(in_token, '/')+1, NULL);
	    g_free(dir);
	}
    } 
    if (!token) {
	token = g_strdup(in_token);
    }
	
    glob_t stack_glob_v;
    gboolean straight_path = g_path_is_absolute(token) ||
	strncmp(token, "./", strlen("./"))==0 ||
	strncmp(token, "../", strlen("../"))==0;

    if (straight_path) {
	gchar *d;
	if (widgets_p) d = g_strdup(get_workdir(widgets_p));
	else d = g_get_current_dir();
	if (chdir(d) < 0){ 
	    DBG("chdir %s\n",d);
	}
	g_free(d);
	
	gchar *directory = g_strdup_printf("%s*", token);
	glob(directory, 0, NULL, &stack_glob_v);
	g_free(directory);

    }     
    else if (getenv("PATH") && strlen(getenv("PATH"))){
	gchar *path_v = g_strdup(getenv("PATH"));
	//gchar **path_pp = rfm_split(path_v,':');
	gchar **path_pp = g_strsplit(path_v, ":", -1);
	gchar **pp = path_pp;
	    
	for (; pp && *pp; pp++){
	    if (strlen(*pp)==0) continue;

	    gint flags;
	    gchar *directory=g_strdup_printf("%s/%s*", *pp, token);
	    if (pp==path_pp) flags = 0;
	    else flags =  GLOB_APPEND;
	    //gint glob_result = 
	    glob(directory, flags, NULL, &stack_glob_v);
	    g_free(directory);
	    if (stack_glob_v.gl_pathc > BASH_COMPLETION_OPTIONS) break;
	}
	g_strfreev(path_pp);
	g_free(path_v);
    }

    if (stack_glob_v.gl_pathc == 0){
	    msg_show_match(widgets_p, MATCH_COMMAND, NULL);
    } else if (stack_glob_v.gl_pathc <= BASH_COMPLETION_OPTIONS){
	struct stat st;
	gint i;
	for (i=0; i<stack_glob_v.gl_pathc; i++){
            // stack_glob_v.gl_pathv is initialized in the glob() call.
            // coverity[uninit_use : FALSE]
	    if (stat (stack_glob_v.gl_pathv[i], &st)==0 && (S_IXOTH & st.st_mode)){
		gchar *base;
		if (straight_path) {
		    base = g_strdup(stack_glob_v.gl_pathv[i]);
		} else {
		    base = g_path_get_basename(stack_glob_v.gl_pathv[i]);
		}
		//rfm_diagnostics (widgets_p, "xffm_tag/red", "exec option = ", base, "\n", NULL);
		matches = g_slist_append (matches, base);
	    }
	}
    } else {
	msg_too_many_matches(widgets_p);
    }

    if (chdir(g_get_home_dir()) < 0){ DBG("chdir %s\n",g_get_home_dir());}

    globfree(&stack_glob_v);


    gchar *suggest = complete_it(widgets_p, &matches, MATCH_COMMAND);
    NOOP( "complete_it: MATCH_COMMAND=%d suggest=%s, matches=%d\n",
	    MATCH_COMMAND, suggest, g_slist_length(matches));
    if (!suggest) {
        suggest = bash_file_completion(widgets_p, token, match_count_p);
	if (suggest) {

	    gchar *d;
	    if (widgets_p) d = g_strdup(get_workdir(widgets_p));
	    else d = g_get_current_dir();
	    gchar *absolute_suggest = g_build_filename(d, suggest, NULL);
	    g_free(d);
	    g_strchomp (absolute_suggest);
	    NOOP(stderr, "file absolute_suggest=%s (%s)\n", absolute_suggest, suggest);
	    if (access(absolute_suggest, X_OK) != 0){
		g_free(suggest);
		suggest=NULL;
		NOOP(stderr, "access \"%s\": %s\n", absolute_suggest, strerror(errno));
	    } 
	    g_free(absolute_suggest);
	}
    }
    NOOP(stderr, "suggest=%s\n", suggest);
    *match_count_p = g_slist_length(matches);
   if (g_slist_length(matches)==1 && rfm_g_file_test(suggest, G_FILE_TEST_IS_DIR)){
	gchar *s=g_strconcat(suggest, "/", NULL);
	g_free(suggest);
	suggest = s;
    }

    GSList *p=matches;
    for (;p && p->data; p=p->next){
	g_free(p->data);
    }
    g_slist_free (matches);

    g_free(token);
    return suggest;

}


static gchar *
variable_complete(widgets_t *widgets_p, const gchar *token, gint *match_p){
    // variable, starts with a $ (pending)
    extern char **environ;
    gchar **env = environ;
    GSList *envlist = NULL;
    for (; env && *env; env++){
	gchar *e = g_strdup_printf("$%s", *env);
	if (strchr(e, '=')) {
	    *(strchr(e, '=')) = 0;
	    envlist = g_slist_prepend (envlist, e);
	}
    }
    GSList *matches = NULL;
    GSList *list;
    for (list=envlist; list && list->data; list = list->next){
	if (strncmp(token, (gchar *)list->data, strlen(token))==0){
	    matches = g_slist_prepend (matches, g_strdup((gchar *)list->data));
	} 
    }
    if (matches){
        *match_p = g_slist_length(matches);
    } else {
	*match_p = 0;
    }
    for (list=envlist; list && list->data; list = list->next){ g_free(list->data);}
    g_slist_free(envlist);
    gchar *suggest = complete_it(widgets_p, &matches, MATCH_VARIABLE);
    for (list=matches; list && list->data; list = list->next){ g_free(list->data);}
    g_slist_free(matches);
    return suggest;
}


static gchar *
userdir_complete(widgets_t *widgets_p, const gchar *token, gint *match_p){
    // username, starts with a ~ (pending)
    // Get username from passwd. If user has a shell, append a /
    // otherwise, append a space.
    // Function: 
    struct passwd *pw;
    GSList *pwlist = NULL;
    while((pw = getpwent ()) != NULL) {
	gboolean shell_ok = strcmp(pw->pw_shell, "/bin/false") && 
	    g_file_test(pw->pw_shell, G_FILE_TEST_IS_EXECUTABLE);
        pwlist = g_slist_prepend (pwlist, g_strdup_printf ("~%s%s", 
		pw->pw_name,
		(shell_ok)?"/":""));
// bug: space will be escaped on 2+ completion:	(shell_ok)?"/":" "));
    }
    endpwent ();
    GSList *matches = NULL;
    GSList *list;
    for (list=pwlist; list && list->data; list = list->next){
	if (strncmp(token, (gchar *)list->data, strlen(token))==0){
	    matches = g_slist_prepend (matches, g_strdup((gchar *)list->data));
	} 
    }
    if (matches){
        *match_p = g_slist_length(matches);
    } else {
	*match_p = 0;
    }
    for (list=pwlist; list && list->data; list = list->next){ g_free(list->data);}
    g_slist_free(pwlist);
    gchar *suggest = complete_it(widgets_p, &matches, MATCH_USER);
    for (list=matches; list && list->data; list = list->next){ g_free(list->data);}
    g_slist_free(matches);
    return suggest;
}
static gchar *
hostname_complete(widgets_t *widgets_p, const gchar *token, gint *match_p){
    // hostname, starts with a @ (pending)
    // Get host from /etc/hosts
    GSList *hostlist = NULL;
    FILE *hosts=fopen("/etc/hosts", "r");
    if (hosts){
        gchar buffer[1024];
        memset (buffer, 0, 1024);
        while(fgets(buffer, 1023, hosts) && !feof(hosts)){
            g_strchug(buffer);
            if (*buffer == '#') continue;
            gchar *t[2];
            t[0] = strchr(buffer, '\t');
            t[1] = strchr(buffer, ' ');
            gchar *p=NULL;
            if (t[0] && t[1]) {
                if (t[0] < t[1]) p = t[0];
                else p = t[1];
            } else if (t[0]) p = t[0];
            else p = t[1];
            // chop
            if (p) {
                if (strchr(p, '\n')) *strchr(p, '\n')=0;
                g_strstrip(p);
                hostlist = g_slist_prepend(hostlist, g_strdup_printf("@%s", p));
            }
        }
        fclose(hosts);
    }
    GSList *matches = NULL;
    GSList *list;
    for (list=hostlist; list && list->data; list = list->next){
	if (strncmp(token, (gchar *)list->data, strlen(token))==0){
	    matches = g_slist_prepend (matches, g_strdup((gchar *)list->data));
	} 
    }
    if (matches){
        *match_p = g_slist_length(matches);
    } else {
	*match_p = 0;
    }
    for (list=hostlist; list && list->data; list = list->next){ g_free(list->data);}
    g_slist_free(hostlist);
    gchar *suggest = complete_it(widgets_p, &matches, MATCH_HOST);
    for (list=matches; list && list->data; list = list->next){ g_free(list->data);}
    g_slist_free(matches);
    return suggest;

    return NULL;
}

static gchar *
extra_completion(widgets_t *widgets_p,  const gchar *token, gint *matches_p){
   if (*token == '$' || *token == '@' || *token == '~'){
	gchar *suggest = NULL;
	switch (*token){
	    case '$':
		suggest = variable_complete(widgets_p, token, matches_p);
		break;
	    case '~':
		suggest = userdir_complete(widgets_p, token, matches_p);
		break;
	    case '@':
		suggest = hostname_complete(widgets_p, token, matches_p);
		break;
	}
	return suggest;
    }
   return NULL;
}

static gchar *extra_space(gchar *suggest, gint *matches_p){
    gint loc = strlen(suggest) -1;
    if (loc < 0) loc = 0;
    if (*matches_p == 1 && suggest[loc] != '/'){
	gchar *g = g_strconcat(suggest, " ", NULL);
	g_free(suggest);
	suggest = g;
    }
    return suggest;
}

#define ALT_CHAR 13
static gchar *
bash_complete(widgets_t *widgets_p,  const gchar *token, gint *matches_p){
    //
    // If we are dealing with file completion, then we have a head.

    gchar *active_token = g_strdup(token);
    g_strchug(active_token);
    gchar *file_token = NULL;
    gchar *command_token = NULL;
    const gchar *head = NULL;

    gchar *suggest = extra_completion(widgets_p, active_token, matches_p);
    if (suggest) {
	suggest = extra_space(suggest, matches_p);
	if (strcmp(suggest, active_token)==0){
	    g_free(suggest);
	    suggest = rfm_get_tilde_dir(active_token);
	}
	g_free(active_token);
	return suggest;
    }
 
    // Obtain the file token separation, if any.
    gboolean esc_space;
    // Test for use of escaped spaces.
    if (strstr(token, "\\ ")){
	esc_space=TRUE;
	gchar *tmp_token = g_strdup(token);
	gint i,j; for(i=0,j=0; j<strlen(token); i++,j++){
	    if (strncmp(token+j, "\\ ", 2) == 0){
		tmp_token[i] = ALT_CHAR;
		j++;
	    } else {
		tmp_token[i] = token[j];
	    }
	}
	tmp_token[i] = 0;

	g_free(active_token);
	active_token = tmp_token;
    }



    {
	gchar *t[5];
	gchar *token_char = {"=\t ><"};
	gint i;
	gboolean file_completion=FALSE;
	for (i=0; i < 5; i++) {
	    t[i] = strrchr(active_token, token_char[i]);
	    if (t[i]) file_completion = TRUE;
	}
	if (file_completion){
	    if (t[0] > t[1] && t[0] > t[2] && t[0] > t[3] && t[0] > t[4]) i = 0;
	    else if (t[1] > t[0] && t[1] > t[2] && t[1] > t[3] && t[1] > t[4]) i = 1;
	    else if (t[2] > t[0] && t[2] > t[1] && t[2] > t[3] && t[2] > t[4]) i = 2;
	    else if (t[3] > t[0] && t[3] > t[1] && t[3] > t[2] && t[3] > t[4]) i = 3;
	    else if (t[4] > t[0] && t[4] > t[1] && t[4] > t[2] && t[4] > t[3]) i = 4;
	    else g_error("should never happen");
	    file_token = g_strdup(t[i]+1);
	    *(t[i]+1) = 0;
	    head = active_token;
	}
    } 

    if (!file_token) command_token = g_strdup(active_token);
    
    
    // Now that we have our token defined, we need to check
    // if the user is using quotes or not.
    gchar *quote=NULL;
    const gchar *r = (file_token)?file_token:command_token;
    if (*r == '\'') quote="\'";
    if (*r == '\"') quote="\"";
    if (quote) {
	gchar *g = g_strdup(r+1);
	if (file_token) {
	    g_free(file_token);
	    file_token=g;
	} else {
	    g_free(command_token);
	    command_token=g;
	}
    }

    if (esc_space) {
	gchar *s = (file_token)?file_token:command_token;
	gint j; for(j=0; s && j<strlen(s); j++){
	    if (s[j] == ALT_CHAR){
		s[j] = ' ';
	    } 
	}
    }
    if (head && esc_space) {
	gchar *g = (gchar *) malloc(2*strlen(head) + 1);
	if (!g) g_error("malloc: %s", strerror(errno));
	memset(g, 0, 2*strlen(head) + 1);
	gint i;
	gchar *p = active_token;
	for (i=0; p && *p; p++,i++){
	    if (*p == ALT_CHAR) {
		g[i++] = '\\';
		g[i] = ' ';
	    } else g[i] = *p;
	}
	g_free(active_token);
	active_token = g_strdup(g);
	g_free(g);
	head = active_token;
    }

    gboolean extra_completed = FALSE;
    if (command_token) {
	suggest = extra_completion(widgets_p, command_token, matches_p);
	NOOP( "extra_completion ...%s -> %s\n", command_token, suggest);
	if (suggest && strcmp(suggest, command_token)==0){
	    g_free(suggest);
	    suggest = rfm_get_tilde_dir(command_token);
	    NOOP( "extra_completion ...%s -> %s\n", command_token, suggest);
	}
	if (!suggest) {
	    suggest = bash_exec_completion(widgets_p, command_token, matches_p);	
	    NOOP( "bash_exec_completion ...%s -> %s\n", command_token, suggest);
	} else extra_completed = TRUE;
    }
    else if (file_token) {
	NOOP( "bash_file_completion ...%s\n", file_token);
	suggest = extra_completion(widgets_p, file_token, matches_p);
	if (suggest && strcmp(suggest, file_token)==0){
	    g_free(suggest);
	    suggest = rfm_get_tilde_dir(file_token);
	}
	if (!suggest) {
	    suggest = bash_file_completion(widgets_p, file_token, matches_p);
	}  else extra_completed = TRUE;
    }

    if (suggest) {
      if (quote) {
	gchar *s;
	/*if (*matches_p==1 && suggest[strlen(suggest)-1] != '/') {
	    s = g_strconcat (quote, suggest, quote, " ", NULL);
	} else */
	{
	    s = g_strconcat (quote, suggest, NULL);
	}
	g_free(suggest);
	suggest=s;

      } else {
	if (strchr(suggest, ' ')){
	    gchar **split = g_strsplit (suggest, " ", -1);
	    gchar *s = g_strjoinv ("\\ ", split);
	    g_free(suggest);
	    suggest=s;
	    g_strfreev(split);
	}
      }
    }
    // add final space for $, @ and ~ completion.
    if (extra_completed){
	suggest = extra_space(suggest, matches_p);
    }
    if (head && suggest) {
	gchar *g = g_strconcat(head, suggest, NULL);
	g_free(suggest);
	suggest = g;
    }
    g_free(active_token);
    g_free(command_token);
    g_free(file_token);

    return suggest;
}


static gchar *
bash_complete_with_head(widgets_t *widgets_p,  const gchar *in_token, gint *matches_p){
    // Do we have a command separator?
    gboolean multiple_command = (strrchr(in_token, ';') || strrchr(in_token, '&') ||  strrchr(in_token, '|'));

    if (!multiple_command){
	return bash_complete(widgets_p, in_token, matches_p);
    }
    gchar *token = g_strdup(in_token);
    gchar *t[3];
    gchar *token_char={";&|"};
    gint i;
    for (i=0; i < 3; i++) t[i] = strrchr(token, token_char[i]);
    if (t[0] > t[1] && t[0] > t[2]) i = 0;
    else if (t[1] > t[0] && t[1] > t[2]) i = 1;
    else if (t[2] > t[0] && t[2] > t[1]) i = 2;
    else g_error("should never happen");
    gchar *p = g_strdup(t[i]+1);
    *(t[i]+1) = 0;
    gchar *suggest = bash_complete(widgets_p, g_strchug(p), matches_p);
    g_free(p);
    if (suggest) {
	gchar *g = g_strconcat(token, suggest, NULL);
	g_free(suggest);
	suggest=g;
    }
    g_free(token);
    return suggest;
}

static gboolean 
valid_token(widgets_t *widgets_p,  const gchar *token){
    if (!token) {
	msg_help_text(widgets_p);
	return FALSE;
    }
    gchar *t = g_strdup(token);
    g_strchug (t);
    if (strlen(t) == 0) {
	g_free(t);
	msg_help_text(widgets_p);
	return FALSE;
    }
    g_free(t);
    return TRUE;
}


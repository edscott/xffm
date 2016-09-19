#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
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

/////////////////////////////  CPY  ////////////////////////////////////
//
//




//#define MAX_LINE_SIZE (_POSIX_PATH_MAX*3)
//#define CHILD_FILE_LENGTH 64


static gboolean
make_overwrite_dialog (const gchar * target, const gchar * src, const gchar *command) {
    struct stat st;
    if (lstat (target, &st)<0){
        fprintf (stderr, "make_overwrite_dialog(): cannot lstat %s\n", target);
    }
    gchar *mess;
    gchar *ss1 = rfm_time_to_string (st.st_mtime);
    gchar *ss2 = rfm_sizetag ((off_t) st.st_size, -1);
    if(src) {
        struct stat s_st;
        if(lstat (src, &s_st) < 0) {
            fprintf (stderr, "make_overwrite_dialog(): cannot lstat %s\n", src);
        }
        gchar *s1 = rfm_time_to_string (s_st.st_mtime);
        gchar *s2 = rfm_sizetag ((off_t) s_st.st_size, -1);
        gchar *target_utf = rfm_utf_string (target);
        gchar *source_utf = rfm_utf_string (src); 

	gchar *standard_text = g_strdup_printf(_("%s: overwrite %s? "), 
		command, target_utf);
	gchar *dates = g_strdup_printf("%s %s %s\n%s %s %s",
		_("Target location: "), s1, s2,
		_("Source Location:"), ss1, ss2);

        mess = g_strdup_printf ("<b>%s</b>\n<i>%s</i>",
		standard_text, dates);
	g_free(standard_text);
	g_free(dates);
	
        g_free (s1); g_free (s2);
        g_free (source_utf); g_free (target_utf);
    } else {
	// This is deprecated... for remove/shred...
        mess = g_strdup_printf ("%s\n(%s %s)", target, ss1, ss2);
    }

    g_free (ss1);
    g_free (ss2);
    TRACE( "confirm %s\n", mess);
    widgets_t *widgets_p = rfm_get_widget ("widgets_p");
    gint result = rfm_confirm ( widgets_p, GTK_MESSAGE_WARNING,  
	mess, 
        _("Cancel"), // if NULL, button not shown
	NULL   // if NULL, "Ok" button shown
	);
    g_free(mess);

    return result;
}

static gchar *
mktgpath (const gchar * target_dir, char *source) {
    gchar *name = g_path_get_basename (source);
    gchar *target = g_build_filename (target_dir, name, NULL);
    g_free (name);
    return target;
}

static gboolean
warn_target_exists (const gchar * target, const gchar * src, gboolean force_override, const gchar *command) {
    TRACE( "force_override=%d\n", force_override);
    gint result = TRUE;
    if(!force_override) {
	result = make_overwrite_dialog (target, src, command);
    }
    return result;
}

static gboolean
ok_input (char *target, const gchar *path, gboolean force_override, const gchar *command) {
	TRACE( "%s at ok_input\n", target);
    if(!rfm_g_file_test (target, G_FILE_TEST_EXISTS)) {
	TRACE( "%s does not exist\n", target);
        return (TRUE);
    }
    return warn_target_exists (target, path, force_override, command);
}

static void
verify_list (widgets_t * widgets_p, GSList **list_p, const gchar * target_path, gboolean interactive, const gchar *command) {
    gboolean force_override = !interactive;



    GSList *list;
    GSList *out_list=NULL;
    for(list = *list_p; list && list->data; list = list->next) {
	// Duplicate path for verified list
	gchar *path=g_strdup((gchar *)list->data);
        //record_entry_t *s_en = rfm_stat_entry (url, 0);

        gchar *target = (rfm_g_file_test(target_path, G_FILE_TEST_IS_DIR))?
	    mktgpath (target_path, path) : g_strdup(target_path);

        NOOP (stderr, "verify_list: path=\"%s\" target=%s\n", path, target_path);

        if (ok_input (target, path, force_override, command)) {
	    out_list = g_slist_append (out_list, path);
        } else {
            rfm_threaded_diagnostics (widgets_p, "xffm_tag/green", 
                    g_strconcat(_("Cancel"), " :", path, "\n", NULL));
            g_free(path);
        }

        g_free (target);
    }
    g_slist_free(*list_p);
    *list_p = out_list;
    return;
}

static void *
thread_cp (void *arg_p){
    void **arg = (void **)arg_p;
    int mode = GPOINTER_TO_INT(arg[0]);
    GSList **verified_list_p=arg[1];
    gchar *target_path = arg[2];
    widgets_t *widgets_p = arg[3];
    g_free(arg_p);

    view_t *view_p=widgets_p->view_p;
    gchar *sources[MAX_COMMAND_ARGS];
    int i = 0;
    
// Choose the GNU command to execute:
    switch (mode) {
    case (TR_MOVE_REMOTE):
	mode = TR_MOVE;
        // case falls through. This is intentional
        // coverity[unterminated_case : FALSE]
    case (TR_MOVE):
        sources[i++] = g_strdup("mv");
        break;
    case (TR_LINK_REMOTE):
	mode = TR_LINK;
        // case falls through. This is intentional
        // coverity[unterminated_case : FALSE]
    case (TR_LINK):
        sources[i++] = g_strdup("ln");
        break;
    case (TR_COPY_REMOTE):
	mode = TR_COPY;
        // case falls through. This is intentional
        // coverity[unterminated_case : FALSE]
    case (TR_COPY):
        sources[i++] = g_strdup("cp");
        break;
    default:
	DBG("cp mode not specified\n");
	return NULL;
    }
    
// Set the options for each command. These options are listed as environment
    RfmProgramOptions *options_p=NULL;
    gint64 flag;
    const gchar *cflag="0x0";
    switch (mode) {
	case (TR_COPY):
	    options_p = get_cp_options();
	    cflag = getenv("RFM_CP_FLAGS");
	    break;
	case (TR_MOVE):
	    options_p = get_mv_options();
	    cflag = getenv("RFM_MV_FLAGS");
	    break;
	case (TR_LINK):
	    options_p = get_ln_options();
	    cflag = getenv("RFM_LN_FLAGS");
	    break;
    }
    errno=0;
    flag = strtoll(cflag, NULL, 16);
    if (errno){
	DBG("cp.i: strtollfailed \n");
    }

    gboolean interactive = FALSE; //cp, mv, but not ln (yet)
    // flag j==0 is the always ask flag, which is not a command flag
    gint j = 0;
    for (; options_p && options_p->option; options_p++, j++){
	if (j==0) continue;
	if (!(options_p->sensitive)) continue;
	if (!(flag & (ONE64<<j))) continue;
	if (!options_p->choice_id){
	    if (strcmp(options_p->option, "-i")==0) interactive = TRUE;
	    else {
		sources[i++] = g_strdup(options_p->option);
		if (strcmp(options_p->option, "-n")==0) interactive = FALSE;
	    }
	} 
#if GNU_CP
	else {
	    const gchar *item=NULL;
	    switch(options_p->choice_id){
		case RFM_CP_backup:
		    item = getenv("RFM_CP_backup");
		    break;
		case RFM_CP_preserve:
		    item = getenv("RFM_CP_preserve");
		    break;
		case RFM_CP_no_preserve:
		    item = getenv("RFM_CP_no_preserve");
		    break;
		case RFM_CP_reflink:
		    item = getenv("RFM_CP_reflink");
		    break;
		case RFM_CP_sparse:
		    item = getenv("RFM_CP_sparse");
		    break;
		case RFM_CP_suffix:
		    item = getenv("RFM_CP_suffix");
		    break;
		case RFM_MV_backup:
		    item = getenv("RFM_MV_backup");
		    break;
		case RFM_MV_suffix:
		    item = getenv("RFM_MV_suffix");
		    break;
		case RFM_LN_backup:
		    item = getenv("RFM_LN_backup");
		    break;
		case RFM_LN_suffix:
		    item = getenv("RFM_LN_suffix");
		    break;
	    }
	    sources[i++] = g_strconcat(options_p->option,item,NULL);
	}
#endif
    }

    // Options are now set. What follows is listing the parameters to
    // the respective GNU command.
    //
    // Interactive processing (if applicable) for cp/mv
    verify_list (widgets_p, verified_list_p, target_path, interactive, sources[0]);
    if(g_slist_length(*verified_list_p) == 0 || *verified_list_p == NULL) {
        NOOP ("CPY: verified_list_p == NULL\n");
	g_slist_free(*verified_list_p);
	g_free(verified_list_p);
	g_free(target_path);
	gint  k = 0; for(; k<i; k++) g_free(sources[k]);
        return GINT_TO_POINTER(FALSE);
    }
    // If we are in link mode, we make all targets relative to 
    // avoid absolute paths for symlinks.
    GSList *list;
    GSList *ln_list=NULL;
    gchar *tgtdir = NULL;
    gboolean ok = TRUE;
    gboolean readOK = TRUE;
    if (mode==TR_LINK) {
	const gchar *ln_path=(*verified_list_p)->data;
	
	tgtdir = g_strdup (target_path);
        gchar *base_dir = g_path_get_dirname (ln_path);
        int up = 0;
        while(base_dir && tgtdir && !strstr (base_dir, tgtdir)
              && strchr (tgtdir, '/')) {
            up++;
            *(strrchr (tgtdir, '/')) = 0;
            NOOP ("CPY: tgtdir=%s\n", tgtdir);
        }
        g_free (base_dir);

	for(list = *verified_list_p; list && list->data; list = list->next) {
	    const gchar *orig_path=list->data;
            NOOP(stderr, "Adding source: %s\n", orig_path);
	    const gchar *source_substring = orig_path + strlen (tgtdir) + 1;
	    int newsource_length = up * strlen ("../") + strlen (source_substring) + 1;
	    gchar *newsource = (gchar *)malloc (newsource_length);
	    if (!newsource) g_error("malloc: %s", strerror(errno));
	    memset (newsource, 0, newsource_length);
	    int k;
	    for(k = 0; k < up; k++) {
		strcat (newsource, "../");
	    }
	    strcat (newsource, source_substring);
	    // relative links may fail if the path we are building
	    // from contains a symlink itself...
	    gchar *target;
	    if(rfm_g_file_test (target_path, G_FILE_TEST_IS_DIR)) {
		target = g_strdup (target_path);
	    } else {
		target = g_path_get_dirname (target_path);
	    }
	    gchar *relative_path=g_build_filename(target, newsource, NULL);
	    g_free(target);
	    if (!rfm_g_file_test(relative_path, G_FILE_TEST_EXISTS)){
		// in this case, fall back to absolute path
		NOOP("%s does not exist! Fallback= %s\n",
			newsource, orig_path);
		g_free(newsource);
		newsource=g_strdup(orig_path);
	    } else {
		NOOP("%s is ok\n", newsource);
	    }
	    g_free(relative_path);
	    sources[i++] = g_strdup(newsource);
	    ln_list=g_slist_prepend(ln_list, newsource);
	    if(i == MAX_COMMAND_ARGS - 3) {
		rfm_diagnostics (widgets_p, "xffm/stock_dialog-warning", NULL);
                gchar *max=g_strdup_printf("%d",MAX_COMMAND_ARGS);
                rfm_diagnostics (widgets_p, "xffm_tag/stderr", sources[0], ": ", strerror(E2BIG)," (> ",max,")","\n", NULL);
                g_free(max);
		ok = FALSE;
		break;
	    }
	}
	g_free (tgtdir);

	gboolean exists=g_file_test (target_path, G_FILE_TEST_IS_DIR);

        if(exists) {
            tgtdir = g_strdup (target_path);
        } else {
            tgtdir = g_path_get_dirname (target_path);
        }
        sources[i++] = g_strdup(tgtdir);
        if(strcmp(tgtdir, ".") && !g_file_test(tgtdir, G_FILE_TEST_IS_DIR)) {
            DBG ("Cannot chdir to %s\n", tgtdir);
            g_free (tgtdir);
	    tgtdir=NULL;
	    ok = FALSE;
        } else {
            g_free (widgets_p->workdir);
            widgets_p->workdir = g_strdup(tgtdir);
        }
        // End TR_LINK...
    } else {
	for(list = *verified_list_p; list && list->data; list = list->next) {
	    sources[i++] = g_strdup((gchar *)list->data);
	    if (!rfm_read_ok_path((gchar *)list->data)){
		NOOP("readOK=false %s\n", (gchar *) list->data);
		readOK=FALSE;
	    }
	    if(i == MAX_COMMAND_ARGS - 3) {
		ok = FALSE;
		break;
	    }
	}	    
        sources[i++] = g_strdup((gchar *) target_path);
    }

    sources[i++] = NULL;

    TRACE("cplnmv: ok=%d\n", ok);
    // note: with run_argv we do not need to escape paths.
    if(ok) {
	if (view_p->flags.type != DESKVIEW_TYPE) {
	    rfm_threaded_show_text(widgets_p);
	}
	// Not for dummies. On cp/mv/ln we will check for permissions on target
	// directory, and if permission do not allow the operation,
	// then we will try to do it with sudo.
        gboolean need_sudo = FALSE;
	gchar *src_dir=NULL;
	if (mode==TR_MOVE) {
	    src_dir=g_path_get_dirname(sources[i-3]);
            NOOP(stderr, "target path=%s src_dir=%s readOK=%d\n", target_path, src_dir, readOK);
            need_sudo = !rfm_write_ok_path(target_path) ||
	        (src_dir && !rfm_write_ok_path(src_dir)) ||
	        !readOK;
	} else if (mode==TR_COPY) {
	    src_dir=g_path_get_dirname(sources[i-3]);
            need_sudo = !rfm_write_ok_path(target_path) || !readOK;
	} else if (mode==TR_LINK) {
	    gchar *ttt = g_path_get_dirname(sources[i-2]);
            gchar *t = realpath(ttt, NULL);
            TRACE("cplnmv: realpath=%s \n", t);
            need_sudo = !rfm_write_ok_path(t);
            g_free(t);
            g_free(ttt);
        }
        TRACE("cplnmv: need sudo=%d \n", need_sudo);
	

	if (!need_sudo) {
	    rfm_thread_run_argv (widgets_p, sources, FALSE);

	} else {

	    gchar *failed=NULL;
	    gchar *tgt=NULL;
	    const gchar *operation;
	    if (rfm_g_file_test (target_path, G_FILE_TEST_IS_DIR)){
		tgt = g_strdup (target_path);
	    } else {
		tgt = g_path_get_dirname (target_path);
	    }
	    switch (mode){
		case TR_COPY:
		    failed=g_strdup_printf("%s.", _("Failed to copy file"));
		    operation="cp";
		    break;
		case TR_MOVE:
		    failed = g_strdup_printf("%s: %s", _( "Move files"), 
				strerror(EACCES));

		    if (src_dir && !rfm_write_ok_path(src_dir)) {
			g_free(tgt);
			tgt=g_strdup(src_dir);
		    }
		    operation="mv";
		    break;
		case TR_LINK:
		    failed=g_strdup_printf(_("Failed to link %s to %s"), 
			    _("File"), _("Destination"));
		    operation="ln";
		    break;
		default:
		    DBG("non valid condition in thread_cp()\n");
	    }
	    if (confirm_sudo(widgets_p, tgt, failed, (void *)operation)){
                RFM_TRY_SUDO (widgets_p, sources, FALSE);
	    }
	    g_free(failed);
	    g_free(tgt);
	}  
	gchar **s = sources;
	for (;s && *s; s++) g_free(*s);
	g_free(src_dir);
    } else {
        NOOP ("PASTE CP: ok == FALSE\n");
    }
    // We should do partial cleanup here...
    // Clean up stuff generated in link mode:
    g_free(tgtdir);
    for(list = ln_list; list && list->data; list = list->next) {
        g_free(list->data);
    }
    g_slist_free (ln_list);
    // 1. Clean up input selection_list
    for(list = *verified_list_p; list && list->data; list = list->next) {
        g_free(list->data);
    }
    g_slist_free (*verified_list_p);
    g_free (verified_list_p);
    g_free (target_path);
    return NULL;
}

// plain_cp is an exception: it is a main thread action, the threaded action
// is the thread_cp() function.
static void *
plain_cp (widgets_t *widgets_p, gint mode, GList *in_list, const gchar *target_path, gboolean new_thread) {

    const gchar *flag_id=NULL;
    switch (mode){
	case (TR_MOVE_REMOTE):
	case (TR_MOVE):
	    flag_id = "RFM_MV_FLAGS"; break;
	case (TR_LINK_REMOTE):
	case (TR_LINK):
	    flag_id = "RFM_LN_FLAGS"; break;
	case (TR_COPY_REMOTE):
	case (TR_COPY):
	    flag_id = "RFM_CP_FLAGS"; break;
    }
    if (!flag_id){
        DBG("plain_cp(): flag_id is NULL\n");
        return NULL;
    }

    if (!rfm_rational(RFM_MODULE_DIR, "settings", widgets_p, (void *)flag_id, "options_dialog")){
	return NULL;
    } 

    GSList **verified_list_p = (GSList **)malloc(sizeof(GSList *));
    if (!verified_list_p) g_error("malloc: %s\n", strerror(errno));

    *verified_list_p = NULL;
    
    
    GList *list;
    for(list = in_list; list && list->data; list = list->next) {
	// Duplicate path for thread list
	gchar *path=g_strdup((gchar *)list->data);
        *verified_list_p = g_slist_prepend (*verified_list_p, path);
        NOOP(stderr, "adding %s to the operation list\n", path);
    }
    *verified_list_p = g_slist_reverse (*verified_list_p);
    

    void **arg = (void **)malloc(4*sizeof(void *));
    if (!arg) g_error("malloc: %s\n", strerror(errno));
    arg[0] = GINT_TO_POINTER(mode);
    arg[1] = verified_list_p;
    arg[2] = g_strdup(target_path);
    arg[3] = widgets_p;
    if (new_thread){
	rfm_view_thread_create(widgets_p->view_p, thread_cp, (void *)arg,
		"callbacks: thread_cp");
    } else {
	thread_cp((void *)arg);
    }
    return GINT_TO_POINTER(TRUE);
}


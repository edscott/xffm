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

static void *
target_dialog(void *data){
    gchar *f = g_build_filename (LS_DBH_FILE, NULL);
    return get_response_history("Target for ls","Target:", "",
	    f, NULL, data,
	    NULL, NULL,
	    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            "/",
	    MATCH_FILE);
}

static void 
ls(widgets_t *widgets_p){
    if (!rfm_rational(RFM_MODULE_DIR, "settings", widgets_p, "RFM_LS_FLAGS", "options_dialog")){
	return ;
    } 
    view_t *view_p = widgets_p->view_p;
    GSList *s_list = view_p->selection_list;
    if (g_slist_length(s_list) > MAX_COMMAND_ARGS - 25){
	DBG("Argument list too long (%d > %d)\n", 
	    g_slist_length(s_list), MAX_COMMAND_ARGS - 25);
	return;
    }
    gchar *argv[MAX_COMMAND_ARGS];
    argv[0] = "ls";
    RfmProgramOptions *options_p = get_ls_options();
    const gchar *cflag="0x0";

    if (getenv("RFM_LS_FLAGS"))cflag = getenv("RFM_LS_FLAGS");
    errno=0;
    gint64 flag = strtoll(cflag, NULL, 16);
    if (errno){
	DBG("ls.i: strtollfailed \n");
    }


    gint i=1;
    // flag j==0 is the always ask flag, which is not a command flag
    gint j = 0;
    GSList *list=NULL;

    for (; options_p && options_p->option; options_p++, j++){
	if (j==0) continue;
	if (!(options_p->sensitive)) continue;
	if (!(flag & (ONE64<<j))) continue;
	if (!options_p->choice_id){
	    argv[i++] = (gchar *)options_p->option;
  	} 
#ifdef GNU_LS
	else {
	    const gchar *item=NULL;
	    switch(options_p->choice_id){
		case RFM_LS_ignore:
		    item = getenv("RFM_LS_ignore");
		    break;
		case RFM_LS_tabsize:
		    item = getenv("RFM_LS_tabsize");
		    break;
		case RFM_LS_blocksize:
		    item = getenv("RFM_LS_blocksize");
		    break;
		case RFM_LS_format:
		    item = getenv("RFM_LS_format");
		    break;
		case RFM_LS_hide:
		    item = getenv("RFM_LS_hide");
		    break;
		case RFM_LS_istyle:
		    item = getenv("RFM_LS_istyle");
		    break;
		case RFM_LS_qstyle:
		    item = getenv("RFM_LS_qstyle");
		    break;
		case RFM_LS_sort:
		    item = getenv("RFM_LS_sort");
		    break;
		case RFM_LS_time:
		    item = getenv("RFM_LS_time");
		    break;
		case RFM_LS_tstyle:
		    item = getenv("RFM_LS_tstyle");
		    break;
		case RFM_LS_width:
		    item = getenv("RFM_LS_width");
		    break;
	    }
	    if (g_str_has_prefix(options_p->option, "--")){
		argv[i++] = g_strconcat(options_p->option,"=",item,NULL);
		list = g_slist_prepend(list, argv[i]);
		i++;
	    } else {
		argv[i++] = (gchar *)options_p->option;
		argv[i++] = (gchar *)item;
	    }
	}
#endif
    }

    if (view_p->flags.preferences & __SHOW_HIDDEN) {
	    argv[i++] = "-a";
    }
    if (!(view_p->flags.preferences & __SHOWS_BACKUP)) {
	    argv[i++] = "-I";
	    argv[i++] = "*~";
    }


    gboolean item_added = FALSE;
    gchar *target = NULL;
#if 0
    // ASK_TARGET
    gboolean ask_target = FALSE; //TRUE;  
    // FIXME:
    // slist requieres a lock on view_p->selection_list.
    // A background refresh can break s_list and cause a crash here.
    //
    // Until this is fixed, target will be asked.
/*
    if (s_list && s_list->data && g_slist_length(s_list->data)) {
	gboolean anyone_not_path = FALSE;
	for (s_list = view_p->selection_list; 
		s_list && s_list->data; s_list = s_list->next){
	    record_entry_t *en = s_list->data;
	    if (!g_file_test(en->path, G_FILE_TEST_EXISTS)) {
		anyone_not_path = TRUE;
		break;
	    }
	}
	if (!anyone_not_path) ask_target = FALSE;
    }
*/
    if (view_p->en && g_file_test(view_p->en->path,G_FILE_TEST_IS_DIR))
        target = target_dialog(view_p->en->path);
    else target = NULL;
    argv[i++] = target;

    if (target && g_file_test(target, G_FILE_TEST_IS_DIR))item_added = TRUE;
#else
    if (view_p->en) {
        g_free(widgets_p->workdir);
        widgets_p->workdir = g_strdup(view_p->en->path);
    }
    if (g_slist_length(view_p->selection_list) > 0 ) {
        for (s_list = view_p->selection_list; s_list && s_list->data; s_list = s_list->next){
            record_entry_t *en = s_list->data;
            //if (g_path_is_absolute(en->path)) {
            if (g_file_test(en->path, G_FILE_TEST_EXISTS)) {
                if (view_p->en && g_file_test(view_p->en->path,G_FILE_TEST_IS_DIR)) {
                    gchar *basename = g_path_get_basename(en->path);
                    argv[i++] = basename; 
                    list = g_slist_prepend(list, basename);
                } else argv[i++] = en->path; 
                item_added = TRUE;
            }
        }
    } else {
        argv[i++] = (view_p->en && g_file_test(view_p->en->path,G_FILE_TEST_IS_DIR))?view_p->en->path:"/";
        item_added = TRUE;
    }
#endif
    if (!item_added) return;
    argv[i] = NULL;
    rfm_threaded_show_text(widgets_p);
    rfm_thread_run_argv(widgets_p, argv, FALSE);
    g_free(target);
    for (s_list = list; s_list && s_list->data; s_list = s_list->next){
	g_free(s_list->data);
    }
    g_slist_free(list);

    return;
}



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

#include "rodent.h"
#include "rfm_modules.h"

/* this should be first 2 lines after headers: */
G_MODULE_EXPORT LIBRFM_MODULE 


#include "run.i"

// rfm_thread_run:
// This modality will execute the command via "sh -c" and allow pipes and 
// redirection. and will be saved in the lpterm history file

G_MODULE_EXPORT
void *
rfm_thread_run (widgets_t * widgets_p, const gchar * command, void *interm) {

    pid_t controller;
    gchar *exec_command;
    if(interm) {
        const gchar *term = rfm_what_term ();
        const char *exec_option = rfm_term_exec_option(term);
        exec_command = g_strconcat (term, " ", exec_option, " ", command, NULL);
	//fprintf(stderr, "exec command= %s\n", exec_command);
    } else
        exec_command = g_strdup (command);
    gchar *save_command = g_strdup (exec_command);
    g_strstrip (exec_command);
    if(strncmp (exec_command, "sudo", strlen ("sudo")) == 0 && strncmp (exec_command, "sudo -A", strlen ("sudo -A")) != 0) {
        check_sudo ();
        gchar *nc = g_strdup_printf ("sudo -A %s", exec_command + strlen ("sudo"));
        g_free (exec_command);
        exec_command = nc;
    }

    gchar *argv[5];
    gint i=0;

    gchar *shell = rfm_shell();
    if (!shell){
	DBG("No valid shell found\n");
    }
    argv[i++] = shell;
    argv[i++] = "-c";
    argv[i++] = exec_command;
    argv[i++] = NULL;

    controller = thread_run (widgets_p, argv, NULL, NULL, NULL, NULL);
    gboolean visible;
    rfm_global_t *rfm_global_p = rfm_global();
    if (rfm_global_p ) {
	visible = rfm_threaded_diagnostics_is_visible (widgets_p);
    } else {
	visible = rfm_diagnostics_is_visible (widgets_p);
    }
    if(visible) {
        gchar *g = rfm_diagnostics_start_string(exec_command, controller, TRUE);
        if (rfm_global_p ) {
            rfm_diagnostics (widgets_p, "xffm/emblem_greenball", g, NULL);
            g_free(g);
        } else {
            rfm_threaded_diagnostics (widgets_p, "xffm/emblem_greenball", g);
        }
    }
     if(controller > 0) {
	gchar *run_button_text = g_strdup_printf("%s -c \"%s\"", shell, exec_command);
        setup_run_button_thread (widgets_p, run_button_text, controller);
	g_free(run_button_text);
    }
    // instead of disposing of exec_command, we will save command 
    // in the sh_command history 
    rfm_save_sh_command_history (widgets_p->view_p, save_command);
    // do not do this: g_free(save_command); 
    // because save_command gets put into the history GList.
    g_free(shell);
    g_free (exec_command);
    return  GINT_TO_POINTER(controller);
}


// Use rfm_thread_run_argv or rfm_thread_run?
// rfm_thread_run_argv will execute directly, not via a shell, and will
// not be saved in the lpterm history

// rfm_thread_run_argv:
// This modality will execute the command without shell
G_MODULE_EXPORT
void *
rfm_thread_run2argv (widgets_t * widgets_p, const gchar * in_command, void *interm) {
    // do the call with argv so that command not saved in lpterm history
    gint argcp;
    gchar **argvp;
    pid_t child;
    gchar *command = g_strdup(in_command);
    g_strstrip (command);
    if(strncmp (command, "sudo", strlen ("sudo")) == 0 && strncmp (command, "sudo -A", strlen ("sudo -A")) != 0) {
	gchar *nc = NULL;
        check_sudo ();
        nc = g_strdup_printf ("sudo -A %s", command + strlen ("sudo"));
	g_free(command);
	command = nc;
    }
    if(g_shell_parse_argv (command, &argcp, &argvp, NULL)) {
        NOOP ("OPEN: rfm_thread_run_argv\n");
        // this should always work
        child = rfm_thread_run_argv (widgets_p, argvp, GPOINTER_TO_INT(interm));
        g_strfreev (argvp);
    } else {
        DBG ("failed to parse command with g_shell_parse_argv() at run.c\n");
        child = GPOINTER_TO_INT(rfm_thread_run (widgets_p, command, interm));
    }
    g_free(command);
    return GINT_TO_POINTER(child);
}


G_MODULE_EXPORT
void *
rfm_try_sudo (widgets_t * widgets_p, gchar ** argv, void *interm) {
    
    gint i;
    gint j=0;
    gchar *exec_argv[MAX_COMMAND_ARGS];

    check_sudo ();
    exec_argv[j++] = "sudo";
    exec_argv[j++] = "-A";
    for(i = 0; argv[i] != NULL && j < MAX_COMMAND_ARGS - 2; i++) {
        NOOP (" %s", argv[i]);
        exec_argv[j++] = argv[i];
    }



    rfm_threaded_show_text(widgets_p);
    if (j==MAX_COMMAND_ARGS - 1) {
    	rfm_threaded_diagnostics(widgets_p,"xffm/stock_dialog-warning",NULL);
        gchar *max=g_strdup_printf("%d",MAX_COMMAND_ARGS);
        rfm_threaded_diagnostics (widgets_p, "xffm_tag/stderr", g_strconcat(strerror(E2BIG)," (> ",max,")","\n", NULL));
        g_free(max);
    }
    

    exec_argv[j++] = NULL;
    NOOP (" \n");

    pid_t child =
	private_rfm_thread_run_argv(widgets_p, exec_argv, 
		GPOINTER_TO_INT(interm), 
		NULL, NULL, NULL, NULL);
    return GINT_TO_POINTER(child);
}

///   vector function... 
G_MODULE_EXPORT
void *
m_thread_run_argv (void *p) {
    void **arg = p;
    widgets_t * widgets_p = arg[0]; 
    //view_t *view_p = widgets_p->view_p;
    gchar ** argv = arg[1];
    gboolean interm = GPOINTER_TO_INT(arg[2]);
    gint *stdin_fd = arg[3];
    void (*stdout_f) (void *stdout_data,
		  void *stream,
		  int childFD) = arg[4];
    void (*stderr_f) (void *stdout_data,
		  void *stream,
		  int childFD) = arg[5];
    void (*tubo_done_f) (void *data) = arg[6];

    NOOP( "At m_thread_run_argv()\n");
    if (widgets_p->workdir
	    &&
	!rfm_g_file_test_with_wait(widgets_p->workdir, G_FILE_TEST_IS_DIR)){
	 gchar *workdir = g_strconcat("workdir = ", 
		 (widgets_p->workdir)?widgets_p->workdir: "NULL", NULL); 
	 rfm_time_out(widgets_p, workdir);
	 g_free(workdir);
	 return NULL; 
     }

    if (widgets_p->workdir && access(widgets_p->workdir, R_OK|X_OK) != 0){
	rfm_threaded_show_text(widgets_p);
	rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-error", NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/stderr",
		g_strconcat(strerror(EACCES), ": '", widgets_p->workdir, "'\n", NULL));
	 return NULL; 
    }

	

    pid_t child =
	private_rfm_thread_run_argv(widgets_p, argv, interm, stdin_fd, stdout_f, stderr_f, tubo_done_f);

    g_free(widgets_p->workdir);
    widgets_p->workdir = g_strdup(g_get_home_dir());

    g_free(p);
    return GINT_TO_POINTER(child);
}



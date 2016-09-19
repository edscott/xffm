/*  rfm_settings_dialog.c
 *
 *  Copyright (C) 2003-2012 edscott wilson garcia <edscott@users.sf.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; 
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define SETTINGS_MODULE_C

#include "rodent.h"

#include "settings-module.h"

#ifndef RFM_MCS_PLUGIN
# define RFM_MCS_PLUGIN
#endif

// Change the channel when you change the Module directory...
#define CHANNEL  RFM_MODULE_DIR

#define BORDER 6
#define SKIP BORDER

static gboolean running=FALSE;

typedef struct {
    gchar *variable;
    gchar *value;
    gboolean editable;
} Item;

enum {
    COLUMN_VARIABLE,
    COLUMN_VALUE,
    COLUMN_EDITABLE,
    NUM_COLUMNS
};

typedef struct settings_t{
    widgets_t *widgets_p;
    GtkWidget *desktop_margin_spinbutton[4];
    GtkWidget *desktopcolor_button;
    GtkWidget *iconviewcolor_button;
    GtkWidget *desktopimage_button;
    GtkWidget *desktopdir_button;
    GtkWidget *desktopdir_entry;
    GtkWidget *combo_box;
    GtkWidget *preview_size_spinbutton;
    GtkWidget *fontsize_box;
    GtkWidget *vfontsize_box;
    GtkWidget *fontfamily_box;
    GtkWidget *vfontfamily_box;
    GtkWidget *iconsize_box;
    GtkWidget *terminal_box;
    GtkWidget *editor_box;
    GtkListStore *model;
    GtkWidget *dialog;
    gboolean disable_options;
    gboolean stable_deskdir; // XXX gtk+ bug workaround: see file_set()
}settings_t;
    
static GtkWidget *settings_dialog=NULL;
static gint shm_settings_serial = 0;
static gint settings_timer = 0;

LIBRFM_MODULE

/********************************************************/



static McsManager *mcs_manager = NULL;
static McsPlugin *mp = NULL;

void *localhost_check(void);
void *run_rfm_settings_dialog (void);
#include "settings-module.i"
//  
//

 
G_MODULE_EXPORT void *
module_active (void){
    return GINT_TO_POINTER(1);
}


G_MODULE_EXPORT
void *
localhost_check(void){
    gboolean auto_valid=FALSE;
    const gchar *display_env = getenv("DISPLAY");
    if (display_env){
	if (strncmp(display_env, ":0",strlen(":0"))==0) auto_valid=TRUE;
	else if (strncmp(display_env, "127.0.0.1:", strlen("127.0.0.1:"))==0)
	    auto_valid=TRUE;
	else if (strncmp(display_env, "localhost:", strlen("localhost:"))==0) 
	    auto_valid=TRUE;
	else {
	    gchar *g=g_strconcat(g_get_host_name(), ":", NULL);
	    if (strncmp(display_env, g, strlen(g))==0) auto_valid=TRUE;
	    g_free(g);
	}
    }
    NOOP("localhost_check(): %d DISPLAY=%s\n", 
	    auto_valid, display_env);
    return GINT_TO_POINTER(auto_valid);
}

G_MODULE_EXPORT 
void *
run_rfm_settings_dialog (void){
    // only one should be allowed at a time (for the time being)
    if (settings_dialog) {
	gtk_window_deiconify (GTK_WINDOW(settings_dialog));
	gtk_window_stick (GTK_WINDOW(settings_dialog));     
	gtk_window_set_keep_above  (GTK_WINDOW(settings_dialog), TRUE);
	return NULL;
    }
    settings_t *settings_p=(settings_t *)malloc(sizeof(settings_t));
    if (!settings_p) g_error("malloc: %s", strerror(errno));
    memset(settings_p, 0, sizeof(settings_t));
    rfm_context_function(context_run_rfm_settings_dialog, settings_p);
    //rfm_view_thread_create(NULL, thread_run_rfm_settings_dialog, settings_p, "thread_run_rfm_settings_dialog");
    return GINT_TO_POINTER(1);
}

G_MODULE_EXPORT 
void *
mcs_set_var (const gchar * setting_name, const gchar * setting_value) {
    if (!running) return NULL;
    NOOP ("mcs_set_var(): %s -> %s\n", setting_name, setting_value);
    mcs_manager_set_string (mcs_manager, setting_name, CHANNEL, setting_value);
    mcs_manager_notify (mcs_manager, CHANNEL);
    return GINT_TO_POINTER (1);
}

G_MODULE_EXPORT 
void *
mcs_shm_stop (void) {
    if (!running) return NULL;
    NOOP ("mcs_shm_stop().\n");
    if(mp) {
	g_free(mp->manager);
	g_free(mp->plugin_name);
	g_free(mp->caption);
	g_free(mp->shm_settings_file);
        munmap (mp->m, sizeof (mcs_shm_t));
	g_free(mp);
    }
    gint i; for(i = 0; i < RFM_OPTIONS; i++) {
	g_free (rfm_options[i].value);
    }
    shm_settings_serial = -1;
    return GINT_TO_POINTER (1);
}

G_MODULE_EXPORT 
void *
mcs_shm_start (void) {
    TRACE("mcs_shm_start().\n");
    if(!mp) {
        gint i;
        COMMENT(stderr, "starting instance of mcs-shm\n");
        mp = (McsPlugin *) malloc (sizeof (McsPlugin));
	if (!mp) g_error("malloc: %s", strerror(errno));
        memset (mp, 0, sizeof (McsPlugin));
        mp->manager = (McsManager *) malloc (sizeof (McsManager));
	if (!mp->manager) g_error("malloc: %s", strerror(errno));
        memset (mp->manager, 0, sizeof (McsManager));
        mcs_manager = mp->manager;
	mp->shm_settings_file =  g_strdup_printf ("/%d-%s", 
		(gint)getuid (), MCS_FILE_NAME);
        mp->plugin_name = g_strdup (CHANNEL);
        mp->caption = g_strdup_printf ("<b><big>%s\nRodent %s</big>\n(<i>librfm-%s</i>)</b>",
		 _("Personal settings"), TAG, VERSION);
        //mp->run_dialog = run_rfm_settings_dialog;
	//
	// This will start the icon module, if present.
	//

        COMMENT(stderr, "shm_open %s\n", mp->shm_settings_file);
        mp->shm = shm_open (mp->shm_settings_file, O_RDWR, 0700);
        if(mp->shm < 0) {
            COMMENT(stderr, "shm_open failed, now creating...\n");
            mcs_shm_t *mcs_shm_p;
            mp->shm = shm_open (mp->shm_settings_file, O_CREAT | O_RDWR, 0700);
            if(mp->shm < 0) {
                g_free (mp->manager);
		g_free(mp->shm_settings_file);
		g_free(mp->plugin_name);
		g_free(mp->caption);
                g_free (mp);
                mp = NULL;
                return NULL;
            }
            if(ftruncate (mp->shm, sizeof (mcs_shm_t)) < 0)
                DBG ("ftruncate: %s\n", strerror (errno));

            mp->m = mmap (0, sizeof (mcs_shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, mp->shm, 0);
            memset (mp->m, 0, sizeof (mcs_shm_t));
            mcs_shm_p = (mcs_shm_t *) mp->m;

            mcs_shm_p->serial = 1;
            if(mcs_shm_fileread ()) {
                mcs_shm_bringforth ();
            } else {
                mcs_shm_init ();
            }

            msync (mp->m, sizeof (mcs_shm_t), MS_SYNC);
            COMMENT(stderr, "creating new shm block\n");
        } else {
            COMMENT(stderr, "using preexisting shm block\n");
            mp->m = mmap (0, sizeof (mcs_shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, mp->shm, 0);
            mcs_shm_bringforth ();
        }

        /* read environment... */
        NOOP(stderr, "+++ read environment\n");
        for(i = 0; i < RFM_OPTIONS; i++) {
	    NOOP(stderr, "environment %s -> %s\n", 
		    rfm_options[i].name, getenv(rfm_options[i].name));
	    mcs_shm_t *mcs_shm_p = (mcs_shm_t *) mp->m;
	    g_free (rfm_options[i].value);

            // false positive. We are not comparing the array, we are comparing
            // the preset value within the array of structures, which may or may
            // not be null.
            // coverity[array_null : FALSE]
	    if(mcs_shm_p->data[i].value) {
		NOOP("mcs_shm_start(): setting %s from hardcode to %s\n",
		    rfm_options[i].name, mcs_shm_p->data[i].value);
		rfm_options[i].value = 
		    g_strdup (mcs_shm_p->data[i].value);
	    } else {
		rfm_options[i].value = g_strdup("");
	    }
	    rfm_setenv (rfm_options[i].name, rfm_options[i].value, TRUE);
	    NOOP(stderr, "mcs_shm_start:  %s->%s\n", rfm_options[i].name, rfm_options[i].value);
        }

    }
    //
    // Here we add a main thread timeout event for settings_monitor()
    //
    mcs_shm_t *mcs_shm_p = NULL;
    if(mp) {
        mcs_shm_p = (mcs_shm_t *) mp->m;
        shm_settings_serial = mcs_shm_p->serial;
        settings_timer = g_timeout_add_seconds (1, settings_monitor, NULL);
    }
    NOOP ("mcs_shm_start(%s): mcs_shm_p->serial=%d\n", 
	    mp->shm_settings_file,
	    (mcs_shm_p)?mcs_shm_p->serial:0);

    running=TRUE;
    return mp;
}

G_MODULE_EXPORT 
const gchar *
g_module_check_init (GModule * module) {
    NOOP (stderr, "g_module_check_init(settings module): setting environment...\n");
    int i;
    environ_t *environ_v = rfm_get_environ();
    for(i = 0; i < RFM_OPTIONS; i++) {
	NOOP(stderr, "g_module_check_init():%d/%d  %s <-> %s <-> %s\n", i+1, RFM_OPTIONS, environ_v[i].env_var, environ_v[i].env_string, environ_v[i].env_text);

	rfm_options[i].name = environ_v[i].env_var;
	if(environ_v[i].env_string){
	    rfm_options[i].value = g_strdup (environ_v[i].env_string);
	} else {
	    if (i==RFM_DESKTOP_DIR) {
		rfm_options[i].value = NULL;
		//rfm_options[i].value = g_build_filename (DEFAULT_DESKTOP_DIR, NULL);
	    } else {	
		rfm_options[i].value = g_strdup("");
	    }
	}
	COMMENT(stderr, "g_module_check_init:  %s->%s\n", rfm_options[i].name, rfm_options[i].value);
	// Done elsewhere:
	// rfm_setenv(rfm_options[i].name, rfm_options[i].value, FALSE);
    }
    mcs_shm_start ();
    return NULL;
}
G_MODULE_EXPORT void 
g_module_unload(GModule * module){
    mcs_shm_stop();
}
G_MODULE_EXPORT void *
module_load(void){
    return GINT_TO_POINTER(1);
}
    
G_MODULE_EXPORT
void *
options_dialog(widgets_t *widgets_p,  const gchar *flag_id){
    if (!flag_id) return NULL;
    const gchar *command;
    if (strcmp(flag_id, "RFM_CP_FLAGS")==0) command = "cp";
    else if (strcmp(flag_id, "RFM_MV_FLAGS")==0) command = "mv";
    else if (strcmp(flag_id, "RFM_LN_FLAGS")==0) command = "ln";
    else if (strcmp(flag_id, "RFM_RM_FLAGS")==0) command = "rm";
    else if (strcmp(flag_id, "RFM_LS_FLAGS")==0) command = "ls";
    else if (strcmp(flag_id, "RFM_SHRED_FLAGS")==0) command = "shred";
    else {
	DBG("options_dialog(): flag %s not recognized\n", flag_id);
	return NULL;
    }
    const gchar *sflag = getenv(flag_id);
    if (sflag && strlen(sflag)){
	guint64 flag;
	errno = 0;
	flag = strtoll(sflag, NULL, 0);
	if (errno) {
	    DBG("options_dialog(): %s\n", strerror(errno));
	    return NULL;
	}
	if (!(0x01 & flag)) return GINT_TO_POINTER(TRUE);
    } else {
	DBG("options_dialog(): cannot get %s from the environment\n", flag_id);
	return NULL;
    }    

    void **arg[]={(void *)widgets_p, (void *)command};
    void *retval = rfm_context_function(options_dialog_f, (void *)arg);
    return  retval; 
}

    
/////////////////////////////////////////////////////////////////////////


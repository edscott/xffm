#ifndef XFFM_PLUS_X
# define XFFM_PLUS_X
# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include <gtk/gtk.h>
# include "intl.h"
# include "debug.h"
#define USER_RFM_CACHE_DIR      g_get_user_cache_dir(),"rfm"

typedef struct pixbuf_t {
    time_t mtime; // stat mtime info for thumbnails
    gint   size;  // pixbuf icon size
    off_t  st_size; // stat st_size for thumbnails
    ino_t  st_ino; // stat st_ino for thumbnails
    GdkPixbuf *pixbuf;
    union {
        gchar *mime_id;
        gchar *path;
    };
} pixbuf_t;

// These defines are set by configuration script when librfm is installed.
#define GNU_LS 1
#define GNU_CP 1
#define GNU_MV 1
#define GNU_LN 1
#define GNU_RM 1
#define GNU_SHRED 1

typedef struct environ_t {
    gchar *env_var;
    gchar *env_string;
    gchar *env_text;
    gchar **env_options;
} environ_t;

/* increment version number for MCS_SHM_PLUGIN_FILE on configuration changes
 * (i.e. adding or removing configuration environment variables) */
// MCS_SHM_PLUGIN_FILE depends on CONFIG_BUILD, so that CONFIG_BUILD id 
// should be incremented with each modification to rfm environment variables.
// For rodent Gamma: build is 5190 (see files.h)
// For rodent Delta-unstable: build is 5858 (see files.h)
// For rodent Core-unstable: build is 6203 (see files.h)
// For rodent Core-rc1: build is 6214a (see files.h)
// For rodent Core-rc3: build is 6335i (see files.h)
//
//
enum {
    RFM_DOUBLE_CLICK_NAVIGATION,/* toggle */
    RFM_VERTICAL_TOOLBAR,       // boolean
    RFM_USE_GTK_ICON_THEME,     // fallback to default system gtk icon theme 
    RFM_DRAG_DOES_MOVE,         /* toggle */
    RFM_CONTENT_FOLDER_ICONS,
    RFM_ENABLE_TIPS,           /* toggle */
    RFM_ENABLE_LABEL_TIPS,           /* toggle */
    RFM_PREVIEW_IMAGE_SIZE,     // integer
    RFM_FIXED_FONT_SIZE,      /* size selector */
    RFM_VARIABLE_FONT_SIZE,      /* size selector */
    RFM_FIXED_FONT_FAMILY,      /* family selector */
    RFM_VARIABLE_FONT_FAMILY,      /* family selector */
    RFM_DEFAULT_ICON_SIZE,      /* size selector */
    TERMINAL_CMD,               /* path selector */
    EDITOR,                 /* path selector */
    RFM_MAXIMUM_COMPLETION_OPTIONS, 
    RFM_MAXIMUM_DIAGNOSTIC_LINES, 
    RFM_LOAD_TIMEOUT, 
    /* desktop:: */
    RFM_ENABLE_DESKTOP,         /* toggle */
    RFM_ENABLE_DESKTOP_DIAGNOSTICS,    /* toggle */
    RFM_NAVIGATE_DESKTOP,       /* toggle */
    RFM_DESKTOP_TOP_MARGIN,     /* spin button*/
    RFM_DESKTOP_BOTTOM_MARGIN,  /* spin button*/
    RFM_DESKTOP_RIGHT_MARGIN,   /* spin button*/
    RFM_DESKTOP_LEFT_MARGIN,    /* spin button*/
    RFM_DESKTOP_DIR,            /* path selector */
    RFM_DESKTOP_IMAGE,          /* path selector */
    RFM_DESKTOP_COLOR,          /* color selector desktop */
    RFM_ICONVIEW_COLOR,         /* color selector iconview */
    RFM_TRANSPARENCY,           /* opacity selector, iconview */

    RFM_PLUGIN_FLAGS,                   /*  */
    RFM_MODULE_FLAGS,                   /*  */

    RFM_TOOLBAR,
    RFM_PASTEBOARD_SERIAL,
    RFM_BOOKMARK_SERIAL,

    ///// core options ///////////////////////////////////////////////////////
    RFM_SHRED_FLAGS,                   /*  */
    RFM_LS_FLAGS,                   /*  */
    RFM_CP_FLAGS,                   /*  */
    RFM_MV_FLAGS,                   /*  */
    RFM_LN_FLAGS,                   /*  */
    RFM_RM_FLAGS,                   /*  */

    // core option parameters
    RFM_SHRED_iterations,	// combo type              
    RFM_SHRED_size,         	// combo type	        

#ifdef GNU_LS
    RFM_LS_ignore,  		// entry type
    RFM_LS_tabsize,  		// entry type 
    RFM_LS_blocksize,  		// entry type
    RFM_LS_hide,  		// entry type
    RFM_LS_width,  		// entry type
    RFM_LS_format, 		// combo type
    RFM_LS_istyle,		// combo type
    RFM_LS_qstyle,		// combo type
    RFM_LS_sort,		// combo type
    RFM_LS_time,		// combo type
    RFM_LS_tstyle,		// combo type
#endif
#ifdef GNU_CP
    RFM_CP_backup, 		// combo type      
    RFM_CP_suffix,		// combo type  
    RFM_CP_preserve,		// combo type        
    RFM_CP_no_preserve,		// combo type            
    RFM_CP_reflink, 		// combo type              
    RFM_CP_sparse,		// combo type           
#endif
#ifdef GNU_MV
    RFM_MV_backup,  		// combo type                  
    RFM_MV_suffix,  		// combo type                  
#endif
#ifdef GNU_LN
    RFM_LN_backup, 		// combo type                    
    RFM_LN_suffix, 		// combo type                  
#endif
#ifdef GNU_RM
    RFM_RM_interactive,  	// combo type                 
#endif

    SMB_USER,                   /* input text */
    SUDO_ASKPASS,
    SSH_ASKPASS,
    
    VERSION_CONTROL,		
    PWD,                        
    RFM_OPTIONS
};

#endif


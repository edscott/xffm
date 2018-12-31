#ifndef XFFM_HH
#define XFFM_HH
#define URIFILE "file://"

enum
{
    ROOTVIEW_TYPE,
    LOCALVIEW_TYPE,
    FSTAB_TYPE,
    NFS_TYPE,
    SSHFS_TYPE,
    ECRYPTFS_TYPE,
    CIFS_TYPE,
    PKG_TYPE
};

enum
{
  FLAGS,
  TREEVIEW_PIXBUF,
  DISPLAY_PIXBUF,
  NORMAL_PIXBUF,
  HIGHLIGHT_PIXBUF,
  TOOLTIP_PIXBUF,
  DISPLAY_NAME,
  ACTUAL_NAME,
  PATH,
  SIZE,
  DATE,
  TOOLTIP_TEXT,
  ICON_NAME,
  TYPE,
  MIMETYPE, 
  PREVIEW_PATH,
  PREVIEW_TIME,
  PREVIEW_PIXBUF,
  NUM_COLS
};

namespace xf {
    static gboolean isTreeView;
    static GList *localMonitorList = NULL;
    static GtkMenu *fstabPopUp=NULL;
    static GtkMenu *fstabItemPopUp=NULL;
    static GtkMenu *localPopUp=NULL;
    static GtkMenu *localItemPopUp=NULL;
    static GtkMenu *rootPopUp=NULL;
    static GtkMenu *rootItemPopUp=NULL;
}
#include "types.h"

#include "common/tubo.hh"
#include "common/util.hh"
#include "common/pixbuf.hh"
#include "common/print.hh"
#include "common/settings.hh"
#include "common/mime.hh"
#include "common/run.hh"
#include "common/gio.hh"
#include "common/dnd.hh"
#include "common/clipboard.hh"
#include "common/gtk.hh"
#include "common/tooltip.hh"
#include "common/icons.hh"

#include "response/passwdresponse.hh"
#include "response/comboresponse.hh"
#include "response/commandresponse.hh"

#include "find/fgr.hh"
#include "find/find.hh"
#include "find/signals.hh"


#include "base/model.hh"
#include "view.hh"

#include "dialog/dialog.hh"

namespace xf {
template <class Type> 
class Fm {
    
public:
    Fm(int argc, char *argv[]){
#ifdef ENABLE_NLS
	/* this binds rfm domain: */
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);

	bindtextdomain ("librfm", PACKAGE_LOCALE_DIR);
	bindtextdomain ("rodent", PACKAGE_LOCALE_DIR);
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset ("librfm", "UTF-8");
	bind_textdomain_codeset ("rodent", "UTF-8");
# endif
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	TRACE ("binding %s, at %s", GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
# endif
#endif
	/* ignore hangups? */
	(void)signal (SIGHUP, SIG_IGN);
#define CORE 1
#ifdef CORE
	struct rlimit rlim;
	if (!strstr(argv[0], "getpass")) {
	    fprintf(stderr, "Enabling core dumps...\n");
	}
	rlim.rlim_cur = RLIM_INFINITY;
	rlim.rlim_max = RLIM_INFINITY;
	setrlimit (RLIMIT_CORE, &rlim);
#endif

	TRACE ("call to setlocale");
	setlocale (LC_ALL, "");
	if (argv[1] && strcmp(argv[1],"--fgr")==0){
	    xf::Fgr<double> *fgr = new(xf::Fgr<double>);
	    fgr->main(argc, argv);
	    return;
	}


	
	TRACE ("call to gtk_init");
	gtk_init (&argc, &argv);
	if (argv[1] && strcmp(argv[1],"--find")==0){
	    xf::Find<xf::findSignals<double>> dialog((const gchar *)argv[2]);
	    //g_idle_add(set_up_dialog, path);
	    gtk_main();
	    return;
	}

	// In order for SSH_ASKPASS to work (for sshfs or for executing
	// ssh commands at the lpterminal) we must detach the tty. 
	// This interferes with stepwise debugging with gdb.
	// Get password for ssh or sudo
	if (strstr(argv[0], "xfgetpass")){
	    xf::Response<double>::sendPassword(argv+1);
	    exit(1);
	} else {
#define FORK
#ifdef FORK
	    if(fork ()){
		sleep(2);
		_exit (123);
	    }
	    setsid(); // detach main process from tty
#endif
	}

	gchar *path = NULL;
	if (argv[1]){
	    if (g_file_test(argv[1], G_FILE_TEST_IS_DIR)){
		path = (gchar *)calloc(1,PATH_MAX);
		realpath(argv[1], path);
	    }
	    else path = g_strdup(argv[1]);
	} else path = g_strdup("xffm:root");

	// FIXME: set these environment variables *only* if they are not already set
	//        in the environment (allow user override)

	// XXX xterm is mandatory...
	
	setenv("TERMINAL", "xterm -rv", 1);
	const gchar *term_cmd = getenv("TERMINAL_CMD");
	if (!term_cmd || !strlen(term_cmd)) term_cmd = "xterm -e";

	gchar *getpass = g_find_program_in_path("xfgetpass");
	if (!getpass) {
	    std::cerr<<"*** Warning: Xffm not correctly installed. Cannot find xfgetpass in path\n";
	} else {
	    TRACE("get pass at %s\n", getpass);
	    setenv("SUDO_ASKPASS", getpass, 1);
	    setenv("SSH_ASKPASS", getpass, 1);
	}
	
	gchar *e = NULL;
	if (getenv("EDITOR")) e = g_find_program_in_path(getenv("EDITOR"));
	if (e) {
	    gchar *g = g_path_get_basename(e);
	    g_free(e);
	    if (strrchr(g, ' ')) *(strrchr(g, ' ')) = 0;
	    if (strcmp(g, "nano")==0) g = g_strdup_printf("%s nano", term_cmd); 
	    if (strcmp(g, "vi")==0) g = g_strdup_printf("%s vi", term_cmd); 
	    if (strcmp(g, "vim")==0) g = g_strdup_printf("%s vim", term_cmd); 
	    //if (strcmp(g, "emacs")==0) g = g_strdup_printf("%s emacs", term_cmd); 
	    if (strcmp(g, "gvim")==0) g = g_strdup_printf("gvim -f", term_cmd); 
	    e=g;
	} else {
	    e = g_find_program_in_path("gvim");
	    if (e) {
		g_free(e);
		e = g_strdup("gvim -f");
	    } else {
		e = g_find_program_in_path("nano");
		if(!e){
		    // nano is mandatory
		    std::cerr<<"*** Warning: No suitable EDITOR found (tried gvim, nano)\n";
		} 
		g_free(e);
		e = g_strdup_printf("%s nano", term_cmd);
	    }
	}
	setenv("EDITOR", e, 1);
	auto xffm = new(xf::Dialog<double>)(path);
	g_object_set_data(G_OBJECT(mainWindow), "xffm", xffm);
	//xffm->setDialogTitle("Fm");
	xffm->setDialogIcon("system-file-manager");
	xf::ClipBoard<double>::startClipBoard();  
    }
    ~Fm(void){
	ClipBoard<double>::stopClipBoard();  
    }
};
}
#endif

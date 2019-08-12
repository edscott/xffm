#ifndef XFFM_HH
#define XFFM_HH


namespace xf {
    static GtkMenu *fstabPopUp=NULL;
    static GtkMenu *fstabItemPopUp=NULL;
    static GtkMenu *localPopUp=NULL;
    static GtkMenu *localItemPopUp=NULL;
    static GtkMenu *rootPopUp=NULL;
    static GtkMenu *rootItemPopUp=NULL;
    static GtkMenu *pkgPopUp=NULL;
    static GtkMenu *pkgItemPopUp=NULL;

    GtkMenu *popUpArray[]={
	fstabPopUp,
	fstabItemPopUp,
	localPopUp,
	localItemPopUp,
	rootPopUp,
	rootItemPopUp,
	pkgPopUp,
	pkgItemPopUp,
	NULL
    };
}

#include "view/view.hh"
#include "dialog/dialog.hh"

namespace xf {
template <class Type> 
class Fm {
    
public:
    ~Fm(void){
	ClipBoard<double>::stopClipBoard();  
    }
    Fm(int argc, char *argv[]){
	/* ignore hangups? */
	(void)signal (SIGHUP, SIG_IGN);
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


	XInitThreads();
	TRACE ("call to gtk_init");
	gtk_init (&argc, &argv);
        auto gtksettings = gtk_settings_get_default();
        // this is to avoid crashes on remote x connection which want to use audible bell.
        g_object_set(G_OBJECT(gtksettings), "gtk-error-bell", FALSE, NULL);
        
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
	    xf::Response<double>::sendPassword(argv);
	    exit(1);
	} else {
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


	setEditor(setTerminal());
	setPasswordPrompt();

	
	auto xffm = new(xf::Dialog<double>)(path);
	g_object_set_data(G_OBJECT(mainWindow), "xffm", xffm);
	//xffm->setDialogTitle("Fm");
	xffm->setDialogIcon("system-file-manager");
	xf::ClipBoard<double>::startClipBoard();  
    }

    static const gchar *getCurrentWorkdir(){
        return getCurrentPage()->workDir();
	// also: FIXME: clean up :
        // return getCurrentNotebook()->workdir();

    }

    static GtkTextView *getCurrentTextview(){
	return getCurrentPage()->output();
    }

    static View<Type> *getCurrentView(){
	return getCurrentPage()->view();
    }

    static Page<Type> *getCurrentPage(){
	return getCurrentNotebook()->currentPageObject();
    }
 
    static Notebook<Type> *getCurrentNotebook(){
	return (Notebook<Type> *)g_object_get_data(G_OBJECT(mainWindow), "dialogObject");
    }
   
    static const gchar *getCurrentDirectory(GtkEntry *entry=NULL){
	if (mainWindow){
	    TRACE("dialogObject = %p\n", object);
	    const gchar *wd = getCurrentPage()->workDir();
	    if (!wd) wd = g_get_home_dir();
	    return wd;
	}
	if (entry){
	    static gchar *currentFolder=NULL;
	    g_free(currentFolder);
	    currentFolder=NULL;
	    auto workdir = (const gchar *)g_object_get_data(G_OBJECT(entry), "workdir");
	    auto entryValue = gtk_entry_get_text(entry);
	    if (entryValue && g_file_test(entryValue, G_FILE_TEST_IS_DIR)) {
		currentFolder = g_strdup(entryValue);
	    } else if (g_file_test(workdir, G_FILE_TEST_IS_DIR)) {
		currentFolder = g_strdup(workdir);
	    } 
	    if (!currentFolder || !g_file_test(currentFolder, G_FILE_TEST_IS_DIR)) {
		g_free(currentFolder);
		currentFolder = g_get_current_dir();
	    }
	    return currentFolder;
	}
	return g_get_home_dir();
    }

private:
    
    static const gchar *setTerminalCmd(const gchar *terminal){
	auto userSetTerminalCmd = getenv("TERMINAL_EXEC");
	if (userSetTerminalCmd && !strlen(userSetTerminalCmd)) 
	    userSetTerminalCmd = NULL;
	if (userSetTerminalCmd) return userSetTerminalCmd;
	auto terminalCmd = g_strconcat(terminal, " -e", NULL);
	DBG("TERMINAL_EXEC not defined, assuming %s\n", terminalCmd);
	setenv("TERMINAL_EXEC", terminalCmd, 1);
	return terminalCmd;
    }

    static const gchar *setTerminal(void){
	auto userSetTerminal = getenv("TERMINAL");
	if (userSetTerminal && !strlen(userSetTerminal)) 
	    userSetTerminal = NULL;
	if (userSetTerminal){
	    WARN("User set terminal = %s\n", userSetTerminal);
	    return setTerminalCmd(userSetTerminal);
	} 
	auto terminal = g_find_program_in_path("uxterm");
	if (terminal){
	    setenv("TERMINAL", "uxterm -rv -vb", 1);
	    WARN("Using terminal = %s\n", getenv("TERMINAL"));
	    return setTerminalCmd(terminal);
	}
	terminal = g_find_program_in_path("xterm");
	if (terminal){
	    setenv("TERMINAL", "xterm -rv -vb", 1);
	    WARN("Using terminal = %s\n", getenv("TERMINAL"));
	    return setTerminalCmd(terminal);
	}
	ERROR("No terminal command found. Please define environment variable \"TERMINAL\"\n");
	return "xterm -e";
    }

    static void setEditor(const gchar *terminalCmd){
	const gchar *e = getenv("EDITOR");
        if (e && strlen(e)==0) e = NULL;
        gchar *f = NULL;
	if (e) {
            // remove options
            f = g_strdup(e);
	    if (strrchr(f, ' ')) *(strrchr(f, ' ')) = 0;
        }

        if (f && g_file_test(f, G_FILE_TEST_EXISTS)){
	    gchar *g = g_path_get_basename(e);
	    if (strcmp(g, "nano")==0 || strcmp(g, "vi")==0 
                || strcmp(g, "vim")==0 || strcmp(g, "emacs")==0){                  
                g_free(f);
                f = g_strdup_printf("%s %s", terminalCmd, e); 
            }

        } else {
            g_free(f);
	    f = g_find_program_in_path("gvim");
	    if (f) {
		g_free(f);
		f = g_strdup("gvim -f");
	    } else {
		f = g_find_program_in_path("vi");
                if (f){
		    g_free(f);
		    f = g_strdup_printf("%s vi", terminalCmd);
	        } else {
		    f = g_find_program_in_path("nano");
		    if(!f){
		        // nano is mandatory
		        std::cerr<<
                            "*** Warning: No suitable EDITOR found"
                           <<" (tried gvim, vi, nano)\n";
                    }
		    g_free(f);
		    f = g_strdup_printf("%s nano", terminalCmd);
		} 
	    }

        }
        if (f) {
            DBG("editor is %s\n", f);
            setenv("EDITOR", f, 1);
        }
    }

    static void setPasswordPrompt(void){
	gchar *getpass = g_find_program_in_path("xfgetpass");
	if (!getpass) {
	    ERROR(" Xffm not correctly installed. Cannot find xfgetpass in path\n");
	} else {
	    TRACE("get pass at %s\n", getpass);
	    setenv("SUDO_ASKPASS", getpass, 1);
	    setenv("SSH_ASKPASS", getpass, 1);
	}
    }

};
}
#endif

#ifndef XFFM_HH
#define XFFM_HH


namespace xf {
    static GtkMenu *fstabPopUp=NULL;
    static GtkMenu *fstabItemPopUp=NULL;
    static GtkMenu *localPopUp=NULL;
    static GtkMenu *localItemPopUp=NULL;
    static GtkMenu *rootPopUp=NULL;
    static GtkMenu *rootItemPopUp=NULL;

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
        // Construct app hash
        MimeApplication<Type>::constructAppHash();
        /* ignore hangups? */
        (void)signal (SIGHUP, SIG_IGN);
#ifdef CORE
        struct rlimit rlim;
        if (!strstr(argv[0], "getpass")) {
            WARN("Enabling core dumps...\n");
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
            //gtk_main();
            return;
        }

        // In order for SSH_ASKPASS to work (for sshfs or for executing
        // ssh commands at the lpterminal) we must detach the tty. 
        // This interferes with stepwise debugging with gdb.
        // Get password for ssh or sudo
        if (strstr(argv[0], "xfgetpass")){
            xf::PasswordResponse<double>::sendPassword(argv);
            exit(1);
        }

        // Detach if "-f" argument not given.
        if (!argv[1] || strcmp(argv[1],"-f")) {
            if(fork ()){
                sleep(2);
                _exit (123);
            }
            setsid(); // detach main process from tty
        } else {
          DBG("Xffm running in foreground.\n")
        }
        if (argv[1] && strcmp(argv[1],"-f")==0) {
            argv[1] = argv[2];
        }

        gchar *path = NULL;
        if (argv[1]){
            if (g_file_test(argv[1], G_FILE_TEST_IS_DIR)){
                path = (gchar *)calloc(1,PATH_MAX);
                if (!realpath(argv[1], path)){
                  DBG("realpath(%s): %s\n", path, strerror(errno));
                }
            }
            else path = g_strdup("xffm:root");
        } else path = g_strdup("xffm:root");

        
        auto xffm = new(xf::Dialog<double>)(path);
        Util<Type>::getEditor();
        Util<Type>::getTerminal();
        Util<Type>::getTerminalCmd();
        Util<Type>::getPasswordPrompt();
        g_object_set_data(G_OBJECT(mainWindow), "xffm", xffm);
        xffm->setDialogIcon(XFFM_ICON);
//        xffm->setDialogIcon(FILE_MANAGER);
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

public:

    static void // Print<Type>::printInfo will free string.
    printInfo(const gchar *icon, gchar *string){
        auto page = getCurrentPage();
        Print<Type>::showTextSmall(page->output());
        Print<Type>::printInfo(page->output(), icon, string);     
    }

    static void // Print<Type>::printInfo will free string.
    printInfo(gchar *string){
        auto page = getCurrentPage();
        Print<Type>::showTextSmall(page->output());
        Print<Type>::printInfo(page->output(), string);     
    }

    static void // Print<Type>::printDbg will free string.
    printDbg(gchar *string){
        auto page = getCurrentPage();
        Print<Type>::showTextSmall(page->output());
        Print<Type>::printDbg(page->output(), string);     
    }

    static void // Print<Type>::printError will free string.
    printError(gchar *string){
        auto page = getCurrentPage();
        Print<Type>::showTextSmall(page->output());
        Print<Type>::printError(page->output(), string);     
    }
    

private:
};
}
#endif

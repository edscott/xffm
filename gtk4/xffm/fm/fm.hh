#ifndef FM_HH
#define FM_HH

#ifdef USE_GTK4
#include "window/window.hh"
#else

#include "view/view.hh"
#include "dialog/dialog.hh"
#endif

namespace xf {
template <class Type> 
class Fm{
private:
  void coreSetup(int argc, char *argv[]){
        (void)signal (SIGHUP, SIG_IGN);
#ifdef CORE
        struct rlimit rlim;
        if (!strstr(argv[0], "getpass")) {
            WARN("Fm:: coreSetup(getpass): Enabling core dumps...\n");
        }
        rlim.rlim_cur = RLIM_INFINITY;
        rlim.rlim_max = RLIM_INFINITY;
        setrlimit (RLIMIT_CORE, &rlim);
#endif
  }

public:

    ~Fm(void){
        //ClipBoard<double>::stopClipBoard();  
    }

    Fm(int argc, char *argv[]){
        // Construct app hash
        //MimeApplication<Type>::constructAppHash();
        coreSetup(argc, argv);

        TRACE ("call to setlocale");
        setlocale (LC_ALL, "");
        XInitThreads();
        TRACE ("call to gtk_init");

        gtk_init ();
        auto gtksettings = gtk_settings_get_default();
        // this is to avoid crashes on remote x connection which want to use audible bell.
        g_object_set(G_OBJECT(gtksettings), "gtk-error-bell", FALSE, NULL);

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
        auto xffm = new(xf::window<double>)(path);
    }

private:
};
}
#endif

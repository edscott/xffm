/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
 * license: GPL v.3
 */
//  to open inspector:
//  GTK_DEBUG=interactive xffm/xffm -f
#include <librsvg/rsvg.h>

#define DEFAULT_OUTPUT_BG "#bbbbbb"

// Debugging macros:
#define ENABLE_GRIDVIEW
//#undef ENABLE_GRIDVIEW

#define ENABLE_MENU_CLASS
#undef ENABLE_MENU_CLASS

#define ENABLE_THREAD_POOL
//#undef ENABLE_THREAD_POOL

#include "xffm.h"
#include "templates.h"




static void setupBindText(void){
#ifdef ENABLE_NLS
    /* this binds domain: */
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    TRACE ("binding %s, at %s\n", GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
# endif
#endif
    TRACE ("call to setlocale");
    setlocale (LC_ALL, "");
}

static void coreSetup(int argc, char *argv[]){
    (void) signal (SIGHUP, SIG_IGN);
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

static bool detachProcess(const char *argv1){
  // Detach if "-f" argument not given.
  if (!argv1 || strcmp(argv1,"-f")) {
      if(fork ()){
          sleep(2);
          _exit (123);
      }
      setsid(); // detach main process from tty
      return true;
  } 
  // If xffm is running in foreground,
  // then the tty is not detached.
  // So we force the X11 askpass:
  setenv("SSH_ASKPASS_REQUIRE", "force", 1);
  TRACE("Xffm running in foreground.\n");
  return false;
}

static  gchar *getPath(const char *argv1){
    if (!argv1) return g_strdup("xffm:root");
    if (!g_file_test(argv1, G_FILE_TEST_EXISTS)){
      DBG("\"%s\" does not exist.\n", argv1);
      return g_strdup(g_get_home_dir());
    }
    gchar *path = realpath(argv1, NULL);
    if (g_file_test(path, G_FILE_TEST_IS_DIR)){
      if (chdir(path) < 0) {
        DBG("xffm.cc::Cannot chdir to %s (%s)\n", argv1, strerror(errno));
        return g_strdup(g_get_home_dir());
      }
      return path;
    } 
    DBG("\"%s\" is not a directory.\n", path);
    return g_path_get_basename(path);
}


int
main (int argc, char *argv[]) {
  TRACE("parent=%d, self=%d\n",getppid(), getpid()); 
  coreSetup(argc, argv);
  xffindProgram = argv[0];
  xffmProgram = argv[0];
  //foo bar
      
  XInitThreads();
  if (strstr(argv[0], "xfgetpass")){
      xf::PasswordResponse::sendPassword(argv);
      exit(1);
  }

  // Run in foreground if "-f"  given:
  auto foreground = !detachProcess(argv[1]);
  if (foreground) argv[1] = argv[2];
  /*if (chdir(g_get_home_dir()) < 0) {
      DBG("xffm.cc::Cannot chdir to %s (%s)\n", g_get_home_dir(), strerror(errno));
  }*/
 
  pthread_t threadLeader;
  pthread_create(&threadLeader, NULL, xf::Thread:: threadPoolRun, NULL);
  pthread_detach(threadLeader);

  gchar *path = getPath(argv[1]);
  TRACE("path is %s (%s)\n", path, argv[1]); 
  auto fm = new(xf::Fm)(path);

  auto c = new xf::ClipBoard;
  g_object_set_data(G_OBJECT(MainWidget), "ClipBoard", c);
  
  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);

  delete c;
  TRACE("exit xffm main loop\n");
  return 0;
}

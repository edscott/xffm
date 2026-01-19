/*
 * Copyright 2005-2024 Edscott Wilson Garcia 
 * license: GPL v.3
 */
//  to open inspector:
//  GTK_DEBUG=interactive xffm/xffm -f
#include <librsvg/rsvg.h>

//#define DEFAULT_OUTPUT_BG "#bbbbbb"
#define DEFAULT_OUTPUT_BG "#000000"
#define DEFAULT_INPUT_BG "#dddddd"

// Debugging macros:
#define ENABLE_GRIDVIEW
//#undef ENABLE_GRIDVIEW

#define ENABLE_MENU_CLASS
#undef ENABLE_MENU_CLASS

#define ENABLE_THREAD_POOL
//#undef ENABLE_THREAD_POOL

const char **environment = NULL;
static const char *rfm_root_id="RFM_ROOT";
static const char *Bookmarks_id="Bookmarks";
const char *Xname_ = NULL;
static char *lastWS=NULL;

static void *bookmarksObject = NULL;

#include "config.h"
#include "xffm.h"


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

static void coreSetup(int argc, const char *argv[]){
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
  // Detach if "-b" argument given.
  if (argv1 && strcmp(argv1,"-b")==0) {
      if(fork ()){
          sleep(2);
          _exit (123);
      }
      setsid(); // detach main process from tty
      setenv("SSH_ASKPASS_REQUIRE", "force", 1);
      return false;
  } else {
    // If xffm is running in foreground,
    // then the tty is not detached.
    // So we force the X11 askpass:
    setsid(); // detach main process from tty
    setenv("SSH_ASKPASS_REQUIRE", "force", 1);
  }
  TRACE("Xffm running in foreground.\n");
  return true;
}

static char *getPath(const char *argv1){
    if (!argv1) return g_strdup(Bookmarks_id);
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
main (int argc, const char *argv[], const char *envp[]) {
  environment = envp; // not used anymore, methinks.
  Xname_ = (const char *)g_strdup_printf("xffm-%ld", time(NULL));
    
  mkdir(g_get_user_data_dir(), 0700);
  mkdir(g_get_user_cache_dir(), 0700);
  mkdir(g_get_user_config_dir(), 0700);

  // Settings changed from version .001 to .002:
  xf::Settings::removeGroup("flags");
  // Setup initial xffm4 environment from user file.
  auto basicEnv = xf::EnvDialog::basicEnv();
  for (auto p=basicEnv; p && *p; p++){
    auto s = xf::Settings::getString("ENVIRONMENT",*p);
    if (s) {
      setenv(*p, s, 1);
    }
    g_free(s);
  }

  char *cacheDir = g_strconcat(USER_CACHE_DIR, NULL);
  if (!g_file_test(cacheDir, G_FILE_TEST_IS_DIR)){
    if (mkdir(cacheDir, 0700) != 0){
      ERROR_("%s: %s\n", cacheDir, strerror(errno));
      exit(1);
    }
  }
  g_free(cacheDir);
  
  if (argv[1] && strcmp(argv[1], "--fgr") == 0){
      xf::Fgr *fgr = new(xf::Fgr);
      fgr->main(argc, argv);
      return 0;
  }

  TRACE("parent=%d, self=%d\n",getppid(), getpid()); 
  coreSetup(argc, argv);
  xffindProgram = "xffm4";
  xffmProgram = argv[0];
#ifdef ENABLE_NLS
    /* this binds domain: */
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    DBG("bindtextdomain (%s, %s)\n", GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    DBG ("binding %s, at %s with codeset UTF-8\n", GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
# endif
#else
#warning "Translations not enabled."
#endif


  XInitThreads();
  if (strstr(argv[0], "xfgetpass")){
      xf::PasswordResponse::sendPassword(argv);
      exit(1);
  }

  bool doFind = false;
  if (argv[1] && strcmp(argv[1], "--find") == 0){
    doFind = true;
    if (!argv[2]) argv[1] = g_get_home_dir();
    else argv[1] = argv[2];
  }

  // Run in foreground if "-f"  given:
  auto foreground = !detachProcess(argv[1]);
  if (foreground) argv[1] = argv[2];
  /*if (chdir(g_get_home_dir()) < 0) {
      DBG("xffm.cc::Cannot chdir to %s (%s)\n", g_get_home_dir(), strerror(errno));
  }*/
 

  threadPoolObject = (void *)new xf::ThreadPool;
  bookmarksObject = (void *)new xf::Bookmarks(false); // True for .config/gtk-4.0 bookmarks
                                                      // otherwise .config/gtk-3.0 bookmarks
  
  auto path = getPath(argv[1]);
  TRACE("path is %s (%s) --> %s\n", path, argv[1], _(path)); 
  auto fm = new(xf::Fm)(path, doFind); // 
  g_free(path);
  // Constructors c and d will use global variable Child::mainWidget(). 
  auto c = new xf::ClipBoard<xf::LocalDir>;
  xf::clipBoardObject = (void *)c;
  auto d = new xf::Dnd<xf::LocalDir>;

  
  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);

  delete c;
  delete d;
  delete fm;
  TRACE("exit xffm main loop\n");
  return 0;
}

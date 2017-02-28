
#include "environment_c.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

environment_c::environment_c(void){
        gtk_thread=NULL;
}
    

void
environment_c::rfm_init_env (void) {
    gint i;
    static gsize initialized = 0;
    if (g_once_init_enter (&initialized)){
      environ_t *environ_v = get_environ();
      // XXX: This initialization may now be obsolete:
      //      must check.
      for(i = 0; environ_v[i].env_var; i++) {
        // copy default values from static memory to heap
        if(strcmp (environ_v[i].env_var, "SUDO_ASKPASS") == 0 ||
	    strcmp(environ_v[i].env_var, "SSH_ASKPASS") == 0 ) {
            environ_v[i].env_string = g_find_program_in_path ("rodent-getpass");
	} 
	else if(environ_v[i].env_string) {
            environ_v[i].env_string = g_strdup (environ_v[i].env_string);
            NOOP ("ENV: %s %s\n", environ_v[i].env_var, environ_v[i].env_string);
        }
      }
      g_once_init_leave (&initialized, 1);
    }
    return;
}



void
environment_c::rfm_setenv (const gchar *name, gchar *value, gboolean verbose) {
    rfm_init_env();
    NOOP(stderr, "setting %s to %s\n", name, value);
    gint which;
    gboolean go_ahead = FALSE;
    environ_t *environ_v = get_environ();
    for(which = 0; environ_v[which].env_var; which++){
        if(strcmp (name, environ_v[which].env_var) == 0){
            break;
	}
    }
    if(!environ_v[which].env_var) return;
    if(!value || !strlen (value)) {
        g_free (environ_v[which].env_string);
#ifdef HAVE_UNSETENV
        environ_v[which].env_string = NULL;
        unsetenv (name);
#else
        environ_v[which].env_string = g_strconcat (name, "=", NULL);
        putenv (environ_v[which].env_string);
#endif
        /*if(verbose) {
            if(strcmp (name, "SMB_USER") == 0) {
                TRACE("Mcs manager changed rfm environment: %s\n", name);
            } else {
                TRACE("Mcs manager changed rfm environment: %s=%s\n", name, ((value) ? value : " "));
            }
        }*/
        return;
    }
    if(strcmp (name, "RFM_MAX_PREVIEW_SIZE") == 0) {
        if(is_number (value))
            go_ahead = TRUE;
    } else if(strcmp (name, "TERMCMD") == 0) {
        if(value && strlen (value)) {
            gchar *c;
            gchar *t = g_strdup (value);
            t = g_strstrip (t);
            if(strchr (t, ' '))
                t = strtok (t, " ");
            c = g_find_program_in_path (t);
            if(c && access (c, X_OK) == 0) {
                go_ahead = TRUE;
            }
            g_free (c);
            c = NULL;
            g_free (t);
            t = NULL;
        }
    } else
        go_ahead = TRUE;
    if(go_ahead) {
        g_free (environ_v[which].env_string);
	gchar *getpass = NULL;
	gchar *editor = NULL;
	if (strcmp (name, "EDITOR") == 0){
	    editor = get_text_editor_envar(value);
	    NOOP(stderr, "Setting text editor to %s\n", editor);
	}
	if (editor){
	    value = editor;
	} else {
	    if (strcmp (name, "SUDO_ASKPASS") == 0 ||
		strcmp(name, "SSH_ASKPASS") == 0 ){
		if (!g_file_test(value, G_FILE_TEST_EXISTS)){
		    getpass = g_find_program_in_path ("rodent-getpass");
		}
	    }
	}
	if (getpass) value = getpass;


	NOOP(stderr, "rfm_setenv(): setting %s -> %s \n", name, value);
#ifdef HAVE_SETENV
        environ_v[which].env_string = g_strdup (value);
        setenv (name, environ_v[which].env_string, 1);
#else
        environ_v[which].env_string = g_strconcat (name, "=", value, NULL);
        putenv (environ_v[which].env_string);
#endif
	g_free(editor);
    } else {                    /* not go_ahead */
        DBG ("failed to change rfm environment: %s\n", name);
    }
    return;
}

// This function gets static information. Better would be to get the
// number of currently available cores for processing.
gint
environment_c::get_max_threads(void){
    gint cores=MAX_PREVIEW_THREADS;
    gchar *nproc=g_find_program_in_path("nproc");
    if (nproc){
        // LINUX
        FILE *pipe = popen (nproc, "r");
	if(pipe) {
            gchar buf[256];
	    memset(buf, 0, 256);
            if (fgets (buf, 255, pipe) && !feof(pipe)){
	        errno = 0;
                long lcore = strtol(buf, NULL, 10);
                if (!errno && lcore > 0) cores = lcore;
            }
            pclose(pipe);
	}
	g_free (nproc);
        return cores;
    }
    // No nproc?
    gchar *sysctl=g_find_program_in_path("sysctl");
    if (sysctl){
        // BSD
        gchar *cmd = g_strdup_printf("%s -a", sysctl);
        g_free(sysctl);
        FILE *pipe = popen (cmd, "r");
	if(pipe) {
            gchar buf[256];
	    memset(buf, 0, 256);
            while (fgets (buf, 255, pipe) && !feof(pipe)){
                if (strstr(buf,"hw.ncpu:")){
	            errno = 0;
                    long lcore = strtol(buf+strlen("hw.ncpu:"), NULL, 10);
                    // overflow not going to happen.
                    // coverity[overflow_assign : FALSE]
                    if (!errno && lcore > 0) cores = lcore;
                    break;
                }
            }
            pclose(pipe);
	}
	g_free (cmd);
    }

    return cores;
}

gboolean
environment_c::is_number (char *p) {
    char *c = p;
    if(!c || !strlen (c))
        return FALSE;
    for(; *c; c++) {
        if(*c < '0' || *c > '9')
            return FALSE;
    }
    return TRUE;
}
static const gchar *terminals_v[] = {
	"roxterm", 
	"sakura",
	"gnome-terminal", 
	"Eterm", 
	"konsole", 
	"Terminal", 
	"aterm", 
	"xterm", 
	"kterm", 
	"wterm", 
	"multi-aterm", 
	"evilvte",
	"mlterm",
	"xvt",
	"rxvt",
	"urxvt",
	"mrxvt",
	"tilda",
	NULL
};

static const gchar *editors_v[] = {
	"gvim -f",  
	"mousepad", 
	"gedit", 
	"kate", 
	"xemacs", 
	"nano",
	"vi",
	NULL
};
 
const gchar * 
environment_c::term_exec_option(const gchar *terminal) {
    const gchar *exec_option = "-e";
    gchar *t = g_path_get_basename (terminal);
    if(strcmp (t, "gnome-terminal") == 0 || strcmp (t, "Terminal") == 0)
            exec_option = "-x";
    g_free(t);
    return exec_option;
}

gchar * 
environment_c::get_text_editor_envar(const gchar *value){
    if(!value) return NULL;
    
    gchar *editor=g_path_get_basename(value);
    // if nano or vi, then use terminal emulator
    if (editor && 
	    (strncmp(editor, "vi",strlen("vi"))==0 
	     || 
	     strncmp(editor, "nano",strlen("nano"))==0)){
	const gchar *t=getenv("TERMINAL_CMD");
	gchar *term = g_find_program_in_path(t);
	if (term) g_free(term);
	else {
	    t=NULL;
	    gint i;
	    for (i=0; terminals_v[i]; i++){
		// sakura is broken... 
		if (strstr(terminals_v[i], "sakura")) continue;
		term = g_find_program_in_path(terminals_v[i]);
		if (term){
		    t=terminals_v[i];
		    g_free(term);
		    break;
		}
	    }
	}
	if (t && strlen(t)) {
	    gchar *b=g_strdup_printf("%s %s %s",
		    t, term_exec_option(t), editor);
	    g_free(editor);
	    editor = b;
	}
    } else {
	g_free(editor);
	editor = g_strdup(value);
    }
    return (editor);
}


void 
environment_c::set_gtk_thread(GThread *data){
    gtk_thread = data;
}

GThread *
environment_c::get_gtk_thread(void){
    return gtk_thread;
}

void 
environment_c::my_mkdir(gchar *data){
    g_mkdir_with_parents (data, 0700);
    g_free(data);
}

void 
environment_c::rfm_init(void){
#ifdef ENABLE_NLS
    /* this binds rfm domain: */
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
# ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
# endif
#endif
    set_gtk_thread(g_thread_self());
    my_mkdir(g_build_filename (DEFAULT_DESKTOP_DIR, NULL));
    my_mkdir(g_build_filename (USER_PIXMAPS, NULL));
    my_mkdir(g_build_filename (USER_RFM_DIR, NULL));
    my_mkdir(g_build_filename (USER_RFM_CACHE_DIR, NULL));
    my_mkdir(g_build_filename (USER_DBH_DIR, NULL));
    my_mkdir(g_build_filename (USER_DBH_CACHE_DIR, NULL));
    my_mkdir(g_build_filename (RFM_THUMBNAIL_DIR, NULL));
}

// FIXME: use vector template down here and below


static const gchar *shred_iterations[]={"1","3","5","10","20",NULL};
static const gchar *shred_size[]={"10K","100K","1M","10M","100M","1G",NULL};
static const gchar *ls_format[] = {"long","across","commas","horizontal","single-column","verbose","vertical",NULL};
static const gchar *ls_istyle[] = {"none","slash","file-type","classify",NULL};
static const gchar *ls_qstyle[] = {"literal","locale","shell","shell-always","c","escape",NULL};
static const gchar *ls_sort[] = {"none","extension","size","time","version",NULL};
static const gchar *ls_time[] = {"status","access",NULL};
static const gchar *ls_tstyle[] = {"locale","full-iso","long-iso","iso",NULL};
static const gchar *cp_v_control[] = {"existing","none","numbered","simple",NULL};
static const gchar *cp_attributes[] = {"all","mode","ownership","timestamps","context","links","xattr",NULL};
static const gchar *cp_when[] = {"always","auto","never",NULL};
static const gchar *cp_suffix[] = {"~",".bak",NULL};
static const gchar *rm_interactive[]={"always","never","once",NULL};

static RfmProgramOptions ls_options[]={ 
    // 0x01
    {"",N_("Ask the user to get additional parameters"),TRUE,NULL},
    // 0x02
    {"-A",N_("-A, --almost-all           do not list implied . and .."),TRUE,NULL},
#ifdef GNU_LS
    // 0x04
    {"-C",N_("-C                         list entries by columns"),TRUE,NULL},
    // 0x08
    {"-D",N_("-D, --dired                generate output designed for Emacs' dired mode"),TRUE,NULL},
    // 0x010
    {"-F",N_("-F, --classify             append indicator (one of */=>@|) to entries"),TRUE,NULL},
    // 0x020
    {"-G",N_("-G, --no-group             in a long listing, don't print group names"),TRUE,NULL},
    // 0x040
    {"-H",N_("-H, --dereference-command-line\nfollow symbolic links listed on the command line"),TRUE,NULL},
    // 0x080
    {"-I",N_("-I, --ignore=PATTERN       do not list implied entries matching shell PATTERN"),TRUE,GINT_TO_POINTER(-1),RFM_LS_ignore},
    // 0x0100
    {"-L",N_("-L, --dereference          when showing file information for a symbolic\nlink, show information for the file the link\nreferences rather than for the link itself"),TRUE,NULL},
    // 0x0200
    {"-N",N_("-N, --literal              print raw entry names (don't treat e.g. control\ncharacters specially)"),TRUE,NULL},
    // 0x0400
    {"-Q",N_("-Q, --quote-name           enclose entry names in double quotes"),TRUE,NULL},
    // 0x0800
    {"-R",N_("-R, --recursive            list subdirectories recursively"),TRUE,NULL},
    // 0x01000
    {"-S",N_("-S                         sort by file size"),TRUE,NULL},
    // 0x02000
    {"-T",N_("-T, --tabsize=COLS         assume tab stops at each COLS instead of 8"),TRUE,GINT_TO_POINTER(-1),RFM_LS_tabsize},
    // 0x04000
    {"-U",N_("-U                         do not sort; list entries in directory order"),TRUE,NULL},
    // 0x08000
    {"-X",N_("-X                         sort alphabetically by entry extension"),TRUE,NULL},
    // 0x010000
    {"-Z",N_("-Z, --context              print any SELinux security context of each file"),TRUE,NULL},
    // 0x020000
    {"-a",N_("-a, --all                  do not ignore entries starting with ."),TRUE,NULL},
    // 0x040000
    {"--author",N_("--author               with -l, print the author of each file"),TRUE,NULL},
    // 0x080000
    {"-b",N_("-b, --escape               print C-style escapes for nongraphic characters"),TRUE,NULL},
    // 0x0
    {"--block-size","--block-size=SIZE      scale sizes by SIZE before printing them.  E.g.,\n'--block-size=M' prints sizes in units of\n1,048,576 bytes.  See SIZE format below.",TRUE,GINT_TO_POINTER(-1),RFM_LS_blocksize},
    // 0x0100000
    {"-c",N_("-c                         with -lt: sort by, and show, ctime (time of last\nmodification of file status information)\nwith -l: show ctime and sort by name\notherwise: sort by ctime, newest first"),TRUE,NULL},
    // 0x0200000
    {"--color",N_("--color[=WHEN]         colorize the output.  WHEN defaults to 'always'\nor can be 'never' or 'auto'.  More info below"),TRUE,NULL},
    // 0x0400000
    {"-d",N_("-d, --directory            list directory entries instead of contents,\nand do not dereference symbolic links"),TRUE,NULL},
    // 0x0800000
    {"-f",N_("-f                         do not sort, enable -aU, disable -ls --color"),TRUE,NULL},
    // 0x01000000
    {"--file-type",N_("--file-type            likewise, except do not append '*'"),TRUE,NULL},
    // 0x02000000
    {"--format",N_("--format=WORD          across -x, commas -m, horizontal -x, long -l,\nsingle-column -1, verbose -l, vertical -C"),TRUE,ls_format,RFM_LS_format},
    // 0x04000000
    {"--full-time",N_("--full-time            like -l --time-style=full-iso"),TRUE,NULL},
    // 0x08000000
    {"-g",N_("-g     like -l, but do not list owner"),TRUE,NULL},
    // 0x010000000
    {"--group-directories-first",N_("group directories before files."),TRUE,NULL},
    // 0x020000000
    {"-h",N_("-h, --human-readable       with -l, print sizes in human readable format\n(e.g., 1K 234M 2G)"),TRUE,NULL},
    // 0x040000000
    {"--si",N_("--si                   likewise, but use powers of 1000 not 1024"),TRUE,NULL},
    // 0x080000000
    {"--dereference-command-line-symlink-to-dir",N_("--dereference-command-line-symlink-to-dir\nfollow each command line symbolic link\nthat points to a directory"),TRUE,NULL},
    // 0x0100000000
    {"--hide",N_("--hide=PATTERN         do not list implied entries matching shell PATTERN\n(overridden by -a or -A)"),TRUE, GINT_TO_POINTER(-1),RFM_LS_hide},
    // 0x0200000000
    {"--indicator-style",N_("--indicator-style=WORD  append indicator with style WORD to entry names:\nnone (default), slash (-p),\nfile-type (--file-type), classify (-F)"),TRUE,ls_istyle,RFM_LS_istyle},
    // 0x0400000000
    {"-i",N_("-i, --inode                print the index number of each file"),TRUE,NULL},
    // 0x0800000000
    {"-k",N_("-k, --kibibytes            use 1024-byte blocks"),TRUE,NULL},
    // 0x01000000000
    {"-l",N_("-l                         use a long listing format"),TRUE,NULL},
    // 0x02000000000
    {"-m",N_("-m                         fill width with a comma separated list of entries"),TRUE,NULL},
    // 0x04000000000
    {"-n",N_("-n, --numeric-uid-gid      like -l, but list numeric user and group IDs"),TRUE,NULL},
    // 0x08000000000
    {"-o",N_("-o                         like -l, but do not list group information"),TRUE,NULL},
    // 0x010000000000
    {"-p",N_("-p, --indicator-style=slash\nappend / indicator to directories"),TRUE,NULL},
    // 0x020000000000
    {"-q",N_("-q, --hide-control-chars   print ? instead of non graphic characters"),TRUE,NULL},
    // 0x040000000000
    {"--show-control-chars",N_("--show-control-chars   show non graphic characters as-is (default\nunless program is 'ls' and output is a terminal)"),TRUE,NULL},
    // 0x080000000000
    {"--quoting-style",N_("--quoting-style=WORD   use quoting style WORD for entry names:\nliteral, locale, shell, shell-always, c, escape"),TRUE,ls_qstyle,RFM_LS_qstyle},
    // 0x0100000000000
    {"-r",N_("-r, --reverse              reverse order while sorting"),TRUE,NULL},
    // 0x0200000000000
    {"-s",N_("-s, --size                 print the allocated size of each file, in blocks"),TRUE,NULL},
    // 0x0400000000000
    {"--sort",N_("--sort=WORD            sort by WORD instead of name: none -U,\nextension -X, size -S, time -t, version -v"),TRUE,ls_sort,RFM_LS_sort},
    // 0x0800000000000
    {"--time",N_("--time=WORD            with -l, show time as WORD instead of modification\ntime: atime -u, access -u, use -u, ctime -c,\nor status -c; use specified time as sort key\nif --sort=time--time=WORD            with -l, show time as WORD instead of modification\ntime: atime -u, access -u, use -u, ctime -c,\nor status -c; use specified time as sort key\nif --sort=time--time=WORD            with -l, show time as WORD instead of modification\ntime: atime -u, access -u, use -u, ctime -c,\nor status -c; use specified time as sort key\nif --sort=time"),TRUE,ls_time,RFM_LS_time},
    // 0x01000000000000
    {"--time-style",N_("--time-style=STYLE  show times using style STYLE:\nfull-iso, long-iso, iso, +FORMAT\nFORMAT is interpreted like 'date'"),TRUE,ls_tstyle,RFM_LS_tstyle},
    // 0x02000000000000
    {"-t",N_("-t                         sort by modification time, newest first"),TRUE,NULL},
    // 0x04000000000000
    {"-u",N_("-u                         with -lt: sort by, and show, access time\nwith -l: show access time and sort by name\notherwise: sort by access time"),TRUE,NULL},
    // 0x08000000000000
    {"-v",N_("-v                         natural sort of (version) numbers within text"),TRUE,NULL},
    // 0x010000000000000
    {"-w",N_("-w, --width=COLS           assume screen width instead of current value"),TRUE,GINT_TO_POINTER(-1),RFM_LS_width},
    // 0x020000000000000
    {"-x",N_("-x                         list entries by lines instead of by columns"),TRUE,NULL},
    // 0x040000000000000
    {"-1",N_("-1                         list one file per line"),TRUE,NULL},
    // 0x080000000000000
    {"--help",NULL,FALSE,NULL},
    // 0x0100000000000000
    {"--version",NULL,FALSE,NULL},
#else
    // 0x02
    {"-B",N_("Force printing of non-printable characters"),TRUE,NULL},
    // 0x04
    {"-C",N_("Columns"),TRUE,NULL},
    // 0x08
    {"-D","-D format",FALSE,NULL},
    // 0x010
    {"-F",N_("append one of */=>@|"),TRUE,NULL},
    // 0x020
    {"-G",N_("Colorize"),TRUE,NULL},
    // 0x040
    {"-H",N_("Follow links"),TRUE,NULL},
    // 0x080
    {"-I",N_("prevent -A"),TRUE,NULL},
    // 0x0100
    {"-L",N_("Always follow links"),TRUE,NULL},
    // 0x0200
    {"-P",N_("Don't follow symbolic links"),TRUE,NULL},
    // 0x0400
    {"-R",N_("recursive"),TRUE,NULL},
    // 0x0800
    {"-S",N_("Sort by Size"),TRUE,NULL},
    // 0x01000
    {"-T",N_("With -l, display complete time"),TRUE,NULL},
    // 0x02000
    {"-U",N_("Sort by Date"),TRUE,NULL},
    // 0x04000
    {"-W",N_("Display whiteouts"),TRUE,NULL},
    // 0x08000
    {"-Z",N_("Display maclabel"),TRUE,NULL},
    // 0x010000
    {"-a","-a, --all",FALSE,NULL},
    // 0x020000
    {"-b","-b, --escape",TRUE,NULL},
    // 0x040000
    {"-c",N_("Sort by ctime"),TRUE,NULL},
    // 0x080000
    {"-d",N_("list directory entries"),TRUE,NULL},
    // 0x0100000
    {"-f",N_("Do Not Sort"),TRUE,NULL},
    // 0x0200000
    {"-g",N_("like -l, but do not list owner"),TRUE,NULL},
    // 0x0400000
    {"-h",N_("human-readable"),TRUE,NULL},
    // 0x0800000
    {"-i",N_("Inode"),TRUE,NULL},
    // 0x01000000
    {"-k",N_("block-size=1K"),TRUE,NULL},
    // 0x02000000
    {"-l",N_("Details"),TRUE,NULL},
    // 0x04000000
    {"-m",N_("Comma separated values (CSV)"),TRUE,NULL},
    // 0x08000000
    {"-n",N_("numeric-uid-gid"),TRUE,NULL},
    // 0x010000000
    {"-o",N_("include file flags"),TRUE,NULL},
    // 0x020000000
    {"-p",N_("indicator-style=slash"),TRUE,NULL},
    // 0x040000000
    {"-q",N_("hide control characters"),TRUE,NULL},
    // 0x080000000
    {"-r",N_("reverse"),TRUE,NULL},
    // 0x0100000000
    {"-s",N_("show size"),TRUE,NULL},
    // 0x0200000000
    {"-t",N_("Sort by Date"),TRUE,NULL},
    // 0x0400000000
    {"-u",N_("Sort by atime"),TRUE,NULL},
    // 0x0800000000
    {"-w",N_("Force raw printing"),TRUE,NULL},
    // 0x01000000000
    {"-x",N_("Sort across"),TRUE,NULL},
    // 0x02000000000
    {"-1",N_("1 file per line"),TRUE,NULL},
#endif
    {NULL,NULL,FALSE,NULL}
 };

static RfmProgramOptions cp_options[]={
    // 0x01
    {"",N_("Ask the user to get additional parameters"),TRUE,NULL},
#ifdef GNU_CP
    // 0x02
    {"-H",N_("-H, --dereference-command-line\nfollow symbolic links listed on the command line"),TRUE,NULL},
    // 0x04
    {"-L",N_("-L, --dereference            always follow symbolic links in SOURCE"),TRUE,NULL},
    // 0x08
    {"-P",N_("-P, --no-dereference         never follow symbolic links in SOURCE"),TRUE,NULL},
    // 0x010
    {"-R",N_("-R, -r, --recursive          copy directories recursively"),TRUE,NULL},
    // 0x020
    {"-a",N_("-a, --archive                same as -dR --preserve=all"),TRUE,NULL},
    // 0x040
    {"--attributes-only",N_("--attributes-only        don't copy the file data, just the attributes"),TRUE,NULL},
    // 0x080
    {"-b",N_("-b                  like --backup but does not accept an argument"),TRUE,NULL},
    // 0x0100
    {"--backup=",N_("--backup[=CONTROL]  make a backup of each existing destination file"),TRUE,cp_v_control,RFM_CP_backup},
    // 0x0200
    {"--copy-contents",N_("--copy-contents          copy contents of special files when recursive"),TRUE,NULL},
    // 0x0400
    {"-d",N_("-d                           same as --no-dereference --preserve=links"),TRUE,NULL},
    // 0x0800
    {"-f",N_("-f, --force                 remove existing destination files"),TRUE,NULL},
    // 0x01000
    {"-i",N_("Interactive"),TRUE,NULL},
    // 0x02000
    {"-l",N_("-l, --link                   hard link files instead of copying"),TRUE,NULL},
    // 0x04000
    {"-n",N_("n, --no-clobber             do not overwrite an existing file (overrides\n"
"a previous -i option)"),TRUE,NULL},
    // 0x08000
    {"-p",N_("-p                           same as --preserve=mode,ownership,timestamps"),TRUE,NULL},
    // 0x010000
    {"--preserve=",N_("--preserve[=ATTR_LIST]   preserve the specified attributes (default:\nmode,ownership,timestamps), if possible\nadditional attributes: context, links, xattr,\nall"),TRUE,cp_attributes,RFM_CP_preserve},
    // 0x020000
    {"--no-preserve=",N_("--no-preserve=ATTR_LIST  don't preserve the specified attributes"),TRUE,cp_attributes,RFM_CP_no_preserve},
    // 0x040000
    {"--parents",N_("--parents                use full source file name under DIRECTORY"),TRUE,NULL},
    // 0x080000
    {"--reflink=",N_("--reflink[=WHEN]         control clone/CoW copies. See below"),TRUE,cp_when,RFM_CP_reflink},
    // 0x0100000
    {"--remove-destination",N_("--remove-destination     remove each existing destination file before\nattempting to open it (contrast with --force)"),TRUE,NULL},
    // 0x0200000
    {"--sparse=",N_("--sparse=WHEN            control creation of sparse files. See below"),TRUE,cp_when,RFM_CP_sparse},
    // 0x0400000
    {"--strip-trailing-slashes",N_("--strip-trailing-slashes  remove any trailing slashes from each SOURCE\nargument"),TRUE,NULL},
    // 0x0800000
    {"-s",N_("-s, --symbolic-link          make symbolic links instead of copying"),TRUE,NULL},
    // 0x01000000
    {"--suffix=",N_("-S, --suffix=SUFFIX          override the usual backup suffix"),TRUE,cp_suffix, RFM_CP_suffix},
    // 0x02000000
    {"-T",N_("-t, --target-directory=DIRECTORY  copy all SOURCE arguments into DIRECTORY"),FALSE,NULL},
    // 0x04000000
    {"-u",N_("-u, --update                 copy only when the SOURCE file is newer\nthan the destination file or when the\ndestination file is missing"),TRUE,NULL},
    // 0x08000000
    {"-v",N_("-v, --verbose                explain what is being done"),TRUE,NULL},
    // 0x010000000
    {"-x",N_("-x, --one-file-system        stay on this file system"),TRUE,NULL},
    // 0x020000000
    {"--help",N_("Help"),FALSE,NULL},
    // 0x040000000
    {"--version",N_("Version"),FALSE,NULL},
#else
    // 0x02
    {"-H",N_("Follow links"),TRUE,NULL},
    // 0x04
    {"-L",N_("Always follow links"),TRUE,NULL},
    // 0x08
    {"-P",N_("Don't follow symbolic links"),TRUE,NULL},
    // 0x010
    {"-R",N_("recursive"),TRUE,NULL},
    // 0x020
    {"-a",N_("archive"),TRUE,NULL},
    // 0x040
    {"-f",N_("force"),TRUE,NULL},
    // 0x080
    {"-i",N_("-i                    prompt before every removal"),TRUE,NULL},
    // 0x0100
    {"-l",N_("hard link files instead of copying"),TRUE,NULL},
    // 0x0200
    {"-n",N_("Do not overwrite any file"),TRUE,NULL},
    // 0x0400
    {"-p",N_("Preserve all attributes"),TRUE,NULL},
    // 0x0800
    {"-v",N_("Verbose"),TRUE,NULL},
    // 0x01000
    {"-x",N_("Stay on single filesystem"),TRUE,NULL},
#endif

    {NULL,NULL,FALSE,NULL}
};

static RfmProgramOptions mv_options[]={
    // 0x01
    {"",N_("Ask the user to get additional parameters"),TRUE,NULL},
#ifdef GNU_MV
    // 0x02
    {"-b",N_("-b                  like --backup but does not accept an argument"),TRUE,NULL},
    // 0x04
    {"--backup=",N_("--backup[=CONTROL]  make a backup of each existing destination file"),TRUE,cp_v_control,RFM_MV_backup},
    // 0x08
    {"-f",N_("-f, --force                 remove existing destination files"),TRUE,NULL},
    // 0x010
    {"-i",N_("-i                    prompt before every removal"),TRUE,NULL},
    // 0x020
    {"--strip-trailing-slashes",N_("--strip-trailing-slashes  remove any trailing slashes from each SOURCE\nargument"),TRUE,NULL},
    // 0x040
    {"--suffix=",N_("-S, --suffix=SUFFIX          override the usual backup suffix"),TRUE,cp_suffix,RFM_MV_suffix},
    // 0x080
    {"-t",N_("-t, --target-directory=DIRECTORY  move all SOURCE arguments into DIRECTORY"),FALSE,NULL},
    // 0x0100
    {"-T",N_("-T, --no-target-directory    treat DEST as a normal file"),FALSE,NULL},
    // 0x0200
    {"-u",N_("-u, --update                 move only when the SOURCE file is newer\nthan the destination file or when the\ndestination file is missing"),TRUE,NULL},
    // 0x0400
    {"-v",N_("-v, --verbose                explain what is being done"),TRUE,NULL},
    // 0x0800
    {"--help",N_("Help"),FALSE,NULL},
    // 0x01000
    {"--version",N_("Version"),TRUE,NULL},
#else
    // 0x02
    {"-f",N_("force"),TRUE,NULL},
    // 0x04
    {"-i",N_("Interactive"),TRUE,NULL},
    // 0x08
    {"-n",N_("Do not overwrite any file"),TRUE,NULL},
    // 0x010
    {"-v",N_("Verbose"),TRUE,NULL},

#endif
    {NULL,NULL,FALSE,NULL}
};

static RfmProgramOptions ln_options[]={
    // 0x01
    {"",N_("Ask the user to get additional parameters"),TRUE,NULL},
#ifdef GNU_LN
    // 0x02
    {"-b",N_("-b                  like --backup but does not accept an argument"),TRUE,NULL},
    // 0x04
    {"--backup=",N_("--backup[=CONTROL]  make a backup of each existing destination file"),TRUE,cp_v_control,RFM_LN_backup},
    // 0x08
    {"-d","-d, -F, --directory",FALSE,NULL},
    // 0x010
    {"-f",N_("-f, --force                 remove existing destination files"),TRUE,NULL},
    // 0x020
    {"-L",N_("-L, --logical               dereference TARGETs that are symbolic links"),TRUE,NULL},
    // 0x040
    {"-n",N_("-n, --no-dereference        treat LINK_NAME as a normal file if\nit is a symbolic link to a directory"),TRUE,NULL},
    // 0x080
    {"-P",N_("-P, --physical              make hard links directly to symbolic links"),TRUE,NULL},
    // 0x0100
    {"-s",N_("-s, --symbolic              make symbolic links instead of hard links"),TRUE,NULL},
    // 0x0200
    {"--suffix=",N_("-S, --suffix=SUFFIX         override the usual backup suffix"),TRUE,cp_suffix,RFM_LN_suffix},
    // 0x0400
    {"-t",N_("-t, --target-directory=DIRECTORY  specify the DIRECTORY in which to create\nthe links"),FALSE,NULL},
    // 0x0800
    {"-T",N_("-T, --no-target-directory   treat LINK_NAME as a normal file always"),FALSE,NULL},
    // 0x01000
    {"-v",N_("-v, --verbose                explain what is being done"),TRUE,NULL},
    // 0x02000
    {"--help",N_("Help"),FALSE,NULL},
    // 0x04000
    {"--version",N_("Version"),FALSE,NULL},
#else
    // 0x02
    {"-F",N_("force"),TRUE,NULL},
    // 0x04
    {"-L",N_("make hard links to symbolic link references"),TRUE,NULL},
    // 0x08
    {"-P",N_("make hard links directly to symbolic links"),TRUE,NULL},
    // 0x010
    {"-f",N_("force"),TRUE,NULL},
    // 0x020
    {"-h",N_("Don't follow symbolic links"),TRUE,NULL},
    // 0x040
    {"-i",N_("Interactive"),TRUE,NULL},
    // 0x080
    {"-n",N_("Don't follow symbolic links"),TRUE,NULL},
    // 0x0100
    {"-s",N_("Create Symbolic Link"),TRUE,NULL},
    // 0x0200
    {"-v",N_("Verbose"),TRUE,NULL},
    // 0x0400
    {"-w",N_("Warn if error in documents"),TRUE,NULL},
#endif
    {NULL,NULL,FALSE,NULL}
};

static RfmProgramOptions rm_options[]={
    // 0x01
    {"",N_("Ask the user to get additional parameters"),TRUE,NULL},
#ifdef GNU_RM
    // 0x02
    {"-f",N_("-f, --force           ignore nonexistent files and arguments, never prompt"),TRUE,NULL},
    // 0x04
    {"-i",N_("-i                    prompt before every removal"),TRUE,NULL},
    // 0x08
    {"-I",N_("-I                    prompt once before removing more than three files, or\nwhen removing recursively.  Less intrusive than -i,\nwhile still giving protection against most mistakes"),TRUE,NULL},
    // 0x010
    {"--interactive=",N_("--interactive[=WHEN]  prompt according to WHEN: never, once (-I), or\nalways (-i).  Without WHEN, prompt always"),TRUE,rm_interactive,RFM_RM_interactive},
    // 0x020
    {"--one-file-system",N_("Stay on single filesystem"),TRUE,NULL},
    // 0x040
    {"--no-preserve-root",N_("--no-preserve-root  do not treat '/' specially"),TRUE,NULL},
    // 0x080
    {"--preserve-root",N_("--preserve-root   do not remove '/' (default)"),TRUE,NULL},
    // 0x0100
    {"-R",N_("-r, -R, --recursive   remove directories and their contents recursively"),TRUE,NULL},
    // 0x0200
    {"-v",N_("-v, --verbose         explain what is being done"),TRUE,NULL},
    // 0x0400
    {"--help",N_("Help"),FALSE,NULL},
    // 0x0800
    {"--version",N_("Version"),FALSE,NULL},
#else
    // 0x02
    {"-d",N_("Remove directory"),TRUE,NULL},
    // 0x04
    {"-f",N_("force"),TRUE,NULL},
    // 0x08
    {"-i",N_("Interactive"),TRUE,NULL},
    // 0x010
    {"-I",N_("Interactive"),TRUE,NULL},
    // 0x020
    {"-P",N_("Overwrite all files"),TRUE,NULL},
    // 0x040
    {"-R",N_("recursive"),TRUE,NULL},
    // 0x080
    {"-r",N_("recursive"),FALSE,NULL},
    // 0x0100
    {"-v",N_("Verbose"),TRUE,NULL},
    // 0x0200
    {"-W",N_("Undelete"),FALSE,NULL},
#endif
    {NULL,NULL,FALSE,NULL}
};

static RfmProgramOptions shred_options[]={
    // 0x01
    {"",N_("Ask the user to get additional parameters"),TRUE,NULL},
    // 0x02
    {"-f",N_("force"),TRUE,NULL},
    // this could use a spin button
    // 0x04
    {"--iterations=",N_("-n, --iterations=N  overwrite N times instead of the default (%d)"), TRUE,shred_iterations,RFM_SHRED_iterations},
    // 0x08
    {"--random-source=",N_("--random-source=FILE  get random bytes from FILE"),FALSE,NULL},// this needs a fileselector button.
    // 0x010
    {"--size=",N_("-s, --size=N   shred this many bytes (suffixes like K, M, G accepted)"),TRUE,shred_size,RFM_SHRED_size},// this could use a spinbutton

    // 0x020
    {"-u",N_("-u, --remove   truncate and remove file after overwriting"),TRUE,NULL},
    // 0x040
    {"-v",N_("-v, --verbose  show progress"),TRUE,NULL},
    // 0x080
    {"-x",N_("-x, --exact    do not round file sizes up to the next full block;\nthis is the default for non-regular files"),TRUE,NULL},
    // 0x0100
    {"-z",N_("-z, --zero     add a final overwrite with zeros to hide shredding"),TRUE,NULL},
    // 0x0200
    {"--help",N_("Help"),FALSE,NULL},
    // 0x0400
    {"--version",N_("Version"),FALSE,NULL},
    {NULL,NULL,FALSE,NULL}
};

// only those with default values (later to be reset by user if so desired)
static environ_default_t
environ_default_v[]={
    { "RFM_DRAG_DOES_MOVE", "Yes"}, 
    { "RFM_ENABLE_TIPS", N_("Yes")},
    { "RFM_ENABLE_LABEL_TIPS", N_("Yes")},
    { "RFM_FIXED_FONT_SIZE", "9"},
    { "RFM_VARIABLE_FONT_SIZE", "9"},
    { "RFM_FIXED_FONT_FAMILY", "monospace",},
    { "RFM_VARIABLE_FONT_FAMILY", "serif"},
    { "RFM_DEFAULT_ICON_SIZE", "Normal"},
    { "TERMINAL_CMD", "xterm"}, 
    { "EDITOR", "gvim -f"},
    { "RFM_MAXIMUM_COMPLETION_OPTIONS", "104"},
    { "RFM_LOAD_TIMEOUT", "5"},
    { "RFM_MAXIMUM_DIAGNOSTIC_LINES", "1000"},
    { "RFM_ENABLE_DESKTOP_DIAGNOSTICS", N_("Yes")},

    { "RFM_DESKTOP_TOP_MARGIN", "20"},
    { "RFM_DESKTOP_BOTTOM_MARGIN", "40"},
    { "RFM_DESKTOP_RIGHT_MARGIN", "25"},
    { "RFM_DESKTOP_LEFT_MARGIN", "50"},
    
    { "RFM_DESKTOP_IMAGE", PREFIX"/share/images/roa153b.jpg"}, 
    { "RFM_DESKTOP_COLOR", "#4C1E0C"}, 
    { "RFM_ICONVIEW_COLOR", "#383C3F"},
   
     { "RFM_PLUGIN_FLAGS", "0xffffffff"}, 
    { "RFM_MODULE_FLAGS", "0xffffffff"}, 
     {"RFM_PASTEBOARD_SERIAL", "0"},
    {"RFM_BOOKMARK_SERIAL", "0"},

     { "RFM_SHRED_FLAGS", "0x163"}, 
#ifdef GNU_CP
    { "RFM_LS_FLAGS", "0x2040400011"}, 
    { "RFM_CP_FLAGS", "0x8000891"}, 
    { "RFM_MV_FLAGS", "0x40b"}, 
    { "RFM_LN_FLAGS", "0x1113"}, 
    { "RFM_RM_FLAGS", "0x3aa"}, 
#else
    { "RFM_LS_FLAGS", "0x820080"}, 
    { "RFM_CP_FLAGS", "0xc91"}, 
    { "RFM_MV_FLAGS", "0x15"}, 
    { "RFM_LN_FLAGS", "0x791"}, 
    { "RFM_RM_FLAGS", "0x3ea"}, 
#endif

    // core option parameters
    { "RFM_SHRED_iterations", "3"}, 
    { "RFM_SHRED_size", "100K"}, 

    {"SUDO_ASKPASS", PREFIX"/bin/rodent-getpass"}, 
    {"SSH_ASKPASS", PREFIX"/bin/rodent-getpass"}, 

    
    { "VERSION_CONTROL", "existing"},
 
    {NULL, NULL}
};

static environ_t environ_v[RFM_OPTIONS + 1] = {        
    // +1 because zero does not count in enum...
    /* general:: */
    { "RFM_DOUBLE_CLICK_NAVIGATION",    NULL, N_("Activate items with a double click"), NULL}, 
    { "RFM_VERTICAL_TOOLBAR",           NULL,N_("Use vertical toolbar"),NULL},
    { "RFM_USE_GTK_ICON_THEME",         NULL, N_("Icon Theme Specification"), NULL}, 
    { "RFM_DRAG_DOES_MOVE",             NULL, N_("Drag: move"), NULL}, 
    { "RFM_CONTENT_FOLDER_ICONS",       NULL, N_("Emblems"), NULL},
    { "RFM_ENABLE_TIPS",                NULL, N_("Enable tooltips"), NULL},
    { "RFM_ENABLE_LABEL_TIPS",          NULL, N_("Enable tooltips"), NULL},
    {"RFM_PREVIEW_IMAGE_SIZE",          NULL,N_("Preview image size in pixels"),NULL},
    { "RFM_FIXED_FONT_SIZE",            NULL, N_("The font size"), NULL},
    { "RFM_VARIABLE_FONT_SIZE",         NULL, N_("The font size"), NULL},
    { "RFM_FIXED_FONT_FAMILY",          NULL, N_("The font family"), NULL},
    { "RFM_VARIABLE_FONT_FAMILY",       NULL, N_("The font family"), NULL},
    { "RFM_DEFAULT_ICON_SIZE",          NULL, N_("Icon size"), NULL},
    { "TERMINAL_CMD",                   NULL, N_("Terminal Emulator"), NULL}, 
    { "EDITOR",                         NULL, N_("Text Editor"), NULL},
    { "RFM_MAXIMUM_COMPLETION_OPTIONS", NULL, N_("Maximum completion options displayed"), NULL},
    { "RFM_LOAD_TIMEOUT",               NULL, N_("Maximum time (seconds) to wait for a load directory"), NULL},
    { "RFM_MAXIMUM_DIAGNOSTIC_LINES",   NULL, N_("Maximum lines in lp terminal buffer"), NULL},
    /* desktop:: */
    { "RFM_ENABLE_DESKTOP",             NULL, N_("Show Desktop Grid"), NULL}, 
    { "RFM_ENABLE_DESKTOP_DIAGNOSTICS", NULL, N_("Console Message Viewer"), NULL},
    { "RFM_NAVIGATE_DESKTOP",           NULL, N_("Navigation Window"), NULL}, 

    { "RFM_DESKTOP_TOP_MARGIN",         NULL, N_("Top Margin"), NULL},
    { "RFM_DESKTOP_BOTTOM_MARGIN",      NULL, N_("Bottom Margin"), NULL},
    { "RFM_DESKTOP_RIGHT_MARGIN",       NULL, N_("Right margin"), NULL},
    { "RFM_DESKTOP_LEFT_MARGIN",        NULL, N_("Left Margin"), NULL},
   
    { "RFM_DESKTOP_DIR",                NULL, N_("Desktop path:"), NULL},
    { "RFM_DESKTOP_IMAGE",              NULL, N_("Background image"), NULL}, 
    { "RFM_DESKTOP_COLOR",              NULL, N_("Background color"), NULL}, 
    { "RFM_ICONVIEW_COLOR",             NULL, N_("Background color"), NULL},
    { "RFM_TRANSPARENCY",               NULL, N_("Background transparency:"), NULL},

    { "RFM_PLUGIN_FLAGS",               NULL, "plugin bitflags (lite)", NULL}, 
    { "RFM_MODULE_FLAGS",               NULL, "module bitflags (lite)", NULL}, 

    {"RFM_TOOLBAR",                     NULL, N_("Toolbar configuration")},
    {"RFM_PASTEBOARD_SERIAL",           NULL, N_("Pasteboard serial control"), NULL},
    {"RFM_BOOKMARK_SERIAL",             NULL, N_("Bookmark serial control"), NULL},

    ///// core options ///////////////////////////////////////////////////////
    { "RFM_SHRED_FLAGS", NULL, NULL, NULL}, 
#ifdef GNU_CP
    { "RFM_LS_FLAGS", NULL, NULL, NULL}, 
    { "RFM_CP_FLAGS", NULL, NULL, NULL}, 
    { "RFM_MV_FLAGS", NULL, NULL, NULL}, 
    { "RFM_LN_FLAGS", NULL, NULL, NULL}, 
    { "RFM_RM_FLAGS", NULL, NULL, NULL}, 
#else
    { "RFM_LS_FLAGS", NULL, NULL, NULL}, 
    { "RFM_CP_FLAGS", NULL, NULL, NULL}, 
    { "RFM_MV_FLAGS", NULL, NULL, NULL}, 
    { "RFM_LN_FLAGS", NULL, NULL, NULL}, 
    { "RFM_RM_FLAGS", NULL, NULL, NULL}, 
#endif

    // core option parameters
    { "RFM_SHRED_iterations",   NULL, NULL, shred_iterations}, 
    { "RFM_SHRED_size",         NULL, NULL, shred_size}, 

#ifdef GNU_LS
    { "RFM_LS_ignore",          NULL, NULL, NULL},
    { "RFM_LS_tabsize",         NULL, NULL, NULL},
    { "RFM_LS_blocksize",       NULL, NULL, NULL},
    { "RFM_LS_hide",            NULL, NULL, NULL},
    { "RFM_LS_width",           NULL, NULL, NULL},
    { "RFM_LS_format",          NULL, NULL, ls_format},
    { "RFM_LS_istyle",          NULL, NULL, ls_istyle},
    { "RFM_LS_qstyle",          NULL, NULL, ls_qstyle},
    { "RFM_LS_sort",            NULL, NULL, ls_sort},
    { "RFM_LS_time",            NULL, NULL, ls_time},
    { "RFM_LS_tstyle",          NULL, NULL, ls_tstyle},
#endif

#ifdef GNU_CP
    { "RFM_CP_backup",          NULL, NULL, cp_v_control}, 
    { "RFM_CP_suffix",          NULL, NULL, cp_suffix}, 
    { "RFM_CP_preserve",        NULL, NULL, cp_attributes}, 
    { "RFM_CP_no_preserve",     NULL, NULL, cp_attributes}, 
    { "RFM_CP_reflink",         NULL, NULL, cp_when}, 
    { "RFM_CP_sparse",          NULL, NULL, cp_when}, 
#endif

#ifdef GNU_MV
    { "RFM_MV_backup",          NULL, NULL, cp_v_control}, 
    { "RFM_MV_suffix",          NULL, NULL, cp_suffix}, 
#endif

#ifdef GNU_LN
    { "RFM_LN_backup",          NULL, NULL, cp_v_control}, 
    { "RFM_LN_suffix",          NULL, NULL, cp_suffix}, 
#endif

#ifdef GNU_RM
    { "RFM_RM_interactive",     NULL, NULL, rm_interactive}, 

#endif


    {"SMB_USER",        NULL, N_("Samba default remote user"), NULL}, 
    {"SUDO_ASKPASS",    NULL, N_("Sudo ask password program"), NULL}, 
    {"SSH_ASKPASS",     NULL, N_("Ssh ask passphrase program"), NULL}, 

    
    { "VERSION_CONTROL", NULL, NULL, cp_v_control},
    { "PWD",             NULL, "third party stuff", NULL}, 

    { NULL, NULL, NULL}

};

environ_t  *get_environ(void){return environ_v;}


void *get_ls_options(void){return (void *)ls_options;}
void *get_cp_options(void){return (void *)cp_options;}
void *get_mv_options(void){return (void *)mv_options;}
void *get_ln_options(void){return (void *)ln_options;}
void *get_rm_options(void){return (void *)rm_options;}
void *get_shred_options(void){return (void *)shred_options;}


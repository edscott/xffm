
#ifndef FGR_HH
#define FGR_HH

// For now, only GNU_GREP.
#define GNU_GREP 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include  <locale.h>

#define GLOBBER_MASK             0xffff

#define GLOBBER_RECURSIVE                    0x01
#define GLOBBER_RECURSIVE_NO_HIDDEN            0x02
#define GLOBBER_VERBOSE                      0x04
#define GLOBBER_ADD_DOT_FILTER                0x08

/* things that require a stat: */
#define GLOBBER_MTIME        0x040
#define GLOBBER_ATIME        0x080
#define GLOBBER_CTIME        0x100
#define GLOBBER_TIME         (GLOBBER_MTIME|GLOBBER_ATIME|GLOBBER_CTIME)
#define GLOBBER_XDEV         0x200
#define GLOBBER_SIZE         0x400
#define GLOBBER_PERM         0x800
#define GLOBBER_TYPE         0x1000
#define GLOBBER_USER         0x2000
#define GLOBBER_GROUP        0x4000
#define GLOBBER_STAT             (GLOBBER_XDEV|GLOBBER_SIZE|GLOBBER_TIME|GLOBBER_PERM|GLOBBER_TYPE|GLOBBER_USER|GLOBBER_GROUP)
#define GLOBBER_RESULT_LIMIT        0x8000

#ifndef S_IFMT
# define S_IFMT     0170000
#endif
#ifndef S_IFSOCK
# define        S_IFSOCK   0140000
#endif
#ifndef S_IFLNK
# define       S_IFLNK    0120000
#endif
#ifndef S_IFREG
# define       S_IFREG    0100000
#endif
#ifndef S_IFBLK
# define      S_IFBLK    0060000
#endif
#ifndef S_IFDIR
# define       S_IFDIR    0040000
#endif
#ifndef S_IFCHR
# define       S_IFCHR    0020000
#endif
#ifndef S_IFIFO
# define       S_IFIFO    0010000
#endif
#ifndef S_ISVTX
# define      S_ISVTX    0001000
#endif

#define GLOBRUN_PID             0x10000
#define GLOBRUN_COUNT           0x20000
#define GLOBRUN_FILTERED           0x40000
#define GLOBRUN_IGNORE_CASE        0x80000
#define GLOBRUN_REG_EXP          0x100000
#define GLOBRUN_INVERT               0x200000
#define GLOBRUN_WORDS_ONLY           0x400000
#define GLOBRUN_LINES_ONLY           0x800000
#define GLOBRUN_ZERO_BYTE            0x1000000
#define GLOBRUN_NOBINARIES           0x2000000

#ifndef GLOB_TILDE
# define GLOB_TILDE 0x0
#endif
#ifndef GLOB_ONLYDIR
# define GLOB_ONLYDIR 0x0
#endif

#define MONTH_T 2628000
#define DAY_T 86400
#define HOUR_T 3600
#define MIN_T 60

#define MAX_ARG 25
#ifndef COPYRIGHT
# define COPYRIGHT "Copyright (c) 2025 Edscott Wilson Garcia. GPL v.3 distribution licence."
#endif

#include "types.h"

typedef struct globber_t {
    int options;
    int type;
    int user;
    int group;
    off_t sizeG;
    off_t sizeL;
    long unsigned month_t;
    long unsigned day_t;
    long unsigned hour_t;
    long unsigned min_t;
/* private variables, not to be duplicated on recursion: */
    struct stat st;
    int pass;
    time_t tiempo;
    time_t actual;
    int dostat;
} globber_t;

static const char *fgrMessage[] = {
        " ",
        N_("Options:"), " ", "[-vVPrMACaiIyLcwxZ] [-fpotkhsmudgeE]] [" N_("Path"),"]", "\n", " \n",
        "fgr: ", N_("Locate documents and folders on this computer by name or content"), "\n", " \n",
        N_("Filter:"), N_("Search for files"),"\n",
        //N_("-k | -h | -d | -m --->  -M | -C | -A"), "\n",
        N_("Contents:"), N_("File's contents filtered as plain text."), "\n", " \n",

        "-v", "\t\t",
        N_("Verbose output debugging mode."), "\n",
        "-V", "\t\t",
        N_("Show version information"), "\n"," \n",

        N_("Filter Details"),"\n"," \n",

        "-a", "\t\t",
        N_("Stay on single filesystem"),"\n",
        "-P", "\t\t",
        N_("Print...") ," ", N_("The unique Process ID that identifies this process."),"\n",
        "-D", "\t\t",
        N_("Find hidden files and directories"), "\n",
        "-f ", N_("Filter"),"\t"
        N_("To display files which pass the filter (stars and regexps will be processed)"),"\n",
        "-r", "\t\t",
        N_("Recursive"),"\n",
        "-s ", "+",N_("KByte"), "\t",
        N_("is greater than or equal to"), " (KB): ", N_("KByte"),"\n",
        "-s ", "-", N_("KByte"), "\t",
        N_("is less than or equal to"), " (KB): ", N_("KByte"),"\n",
        "-p ", "suid | exe", "\t",
        N_("Set UID"), "(suid) | ", N_("Executable Text Files"), " (exe)\n",
        "-o ", "octal", "\t",
        N_("The permissions of the file, in octal notation."),"\n",
        "-t ", N_("Type"), " \t",
        " any | reg | dir | sym | sock | blk | chr | fifo","\n",
        "\t\t",N_("Any")," (any) ", N_("Default type"),"\n",
        "\t\t",N_("Regular file")," (reg)\n",
        "\t\t",N_("Directory")," (dir)\n",
        "\t\t",N_("Symbolic Link")," (sym)\n",
        "\t\t",N_("UNIX Socket")," (sock)\n",
        "\t\t",N_("Block device")," (blk)\n",
        "\t\t",N_("Character device")," (chr)\n",
        "\t\t",N_("FIFO")," (fifo)\n",

        "-k ", N_("Minutes"),"\t",
        N_("Details"),": ", N_("TIME"),
          " (", N_("Previous"), "): " N_("Minutes"), " [--> -M | -C | -A]", "\n",
        "-h ", N_("Hours"), "\t",
        N_("Details"),": ", N_("TIME"),
          " (", N_("Previous"), "): " N_("Hours"), " [--> -M | -C | -A]", "\n",
        "-d ", N_("Days"), " \t",
        N_("Details"),": ", N_("TIME"),
          " (", N_("Previous"), "): " N_("Days"), " [--> -M | -C | -A]", "\n",
        "-m ", N_("Month"), "\t", 
        N_("Details"),": ", N_("TIME"),
          " (", N_("Previous"), "): " N_("Month(s)"), " [--> -M | -C | -A]", "\n",

        "-M ", N_("TIME"), "\t",
        N_("Modification Time :"),  N_("TIME"), " (", 
            "mknod, truncate, utime, write)", "\n", 

        "-A ", N_("TIME"), "\t",
        N_("Access Time :"),  N_("TIME"), " (",
            "exec, mknod, pipe, utime, read)", "\n", 

        "-C ", N_("TIME"), "\t",
        N_("Creation Time :"),  N_("TIME"), " (",
            "chown, chgrp, chmod, ln)", "\n", 

        "-u ", N_("uid"), "  \t",
        N_("User:"), " ", N_("uid"),"\n",

        "-g ", " ", N_("gid"), "  \t",
        N_("Group:"), " ",  N_("gid"),"\n",

        "-Z", "\t\t",
        N_("Output  a  zero  byte  (the  ASCII  NULL  character) instead of the character that  normally  follows  a file  name"),"\n \n",
        
        N_("Content View"), ": --------\n \n", 
        "-e ", N_("STRING"), "\t"
        N_("Regular expression"), ": ", N_("STRING"), "\n",
        "-E ", N_("STRING"), "\t",
        N_("Regular expression"), "(", N_("Extended"), "): ", N_("STRING"), "\n",
        "-i", "\t\t",
        N_("Ignore case"), "\n",
        "-I", "\t\t",
        N_("Text Files"), "\n", 
        "-L", "\t\t",
        N_("No match"), "\n", 
        "-c", "\t\t",
        N_("Line Count"), "\n",
        "-w", "\t\t",
        N_("Words"), "\n",
        "-x", "\t\t",
        N_("Lines"), "\n \n ",
        "xffind-", VERSION, " ", COPYRIGHT, "\n",
        NULL
    };

static int terminated = 0;
static int initial;
static int options = 0;
static char *token;
static void *object = NULL;

namespace xf {

template <class Type>
FgrData {

    GtkBox *mainBox_ = NULL;
    pid_t pid_ = 0;
    gint resultLimit_ = 0;
    gint resultLimitCounter_ = 0;
    GSList *findList_ = NULL;
    gchar **argument_ = NULL;
    gboolean done_ = false;

    char *path_ = NULL;
  public:
    FgrData(GtkBox *xffmMainBox){
      mainBox_ = xffmMainBox;
    }

    GtkBox *mainBox(void){return mainBox_;}

    void showArguments(void){
#if 1
//#ifdef FULL_DEBUG
        fprintf(stderr, "DBG> arguments: \'");
        for (auto p=argument_; p && *p; p++){
          fprintf(stderr, "%s ", *p);
        }
        fprintf(stderr, "\n");
#endif
    }

    void
    set_arguments(gchar *path){
        path_ = g_strdup(path);
        int i=0;
        auto mainBox = mainBox();

        /* limit */
        resultLimit_ = gtk_spin_button_get_value_as_int (
                GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "upper_limit_spin")));
        resultLimitCounter_ = 0; // fgr ends with "fgr search complete"

        // FIXME: MAX_COMMAND_ARGS is way too much overkill...
        argument_ = (char **)calloc(MAX_COMMAND_ARGS, sizeof(gchar *));
        if (!argument_){
            std::cerr<<"calloc error at get_arguments()\n";
            exit(1);
        }
        
        /* the rest */
        argument_[i++] = g_strdup(xffindProgram);
        argument_[i++] = g_strdup("--fgr");
        argument_[i++] = g_strdup("-v"); // (verbose output) 
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("Recursive")))))
        {
            if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                        g_object_get_data(G_OBJECT(mainBox), _("Find hidden files and directories"))))) 
            {
                argument_[i++] = g_strdup("-r");
            } else {
                argument_[i++] = g_strdup("-R");
            }
        } 

        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( //?? check this XXX
                    g_object_get_data(G_OBJECT(mainBox), _("Stay on single filesystem")))))
        {
            argument_[i++] = g_strdup("-a");
        } 
        
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("File size"))))) {
          getSizeOptions(Data, &i);
        }


        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("Specific Time"))))){
          getTimeOptions(Data, &i);
        }

        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("File properties"))))){
          getFileOptions(Data, &i);
        }


        //  grep options
        getGrepOptions(Data, &i);

        /* select list */

        auto dd = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(mainBox), "file_type_om")); 
        auto item = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(dd));
        auto ftype = gtk_string_object_get_string(item);
        //const char *ftype = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(
                //g_object_get_data(G_OBJECT(mainBox), "file_type_om")));
        for(int j = 0; ftypes[j] != NULL; j++) {
            if(ftype && strcmp (ftype, _(ftypes[j])) == 0) {
                argument_[i++] = g_strdup("-t");
                argument_[i++] = g_strdup(ft[j]);
                break;
            }
        }

        auto filterEntry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "filter_entry"));
        auto filterBuffer = gtk_entry_get_buffer(filterEntry);
        const char *filter = gtk_entry_buffer_get_text(filterBuffer);
        //const char *filter = gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "filter_entry")));
        /* apply wildcard filter if not specified (with dotfiles option) */
        argument_[i++] = g_strdup("-f");
        if(filter && strlen (filter)) {
            argument_[i++] = g_strdup(filter);
        } else {
            argument_[i++] = g_strdup("*");
            argument_[i++] = g_strdup("-D");
        }

        /* last argument_ is the path */
        //argument_[i++] = g_strdup_printf("\"%s\"", path); g_free(path);
        argument_[i++] = path;
        argument_[i] = (char *)0;
    }
  private:

        static void getSizeOptions(fgrData_t *Data, int *i){
          GtkBox *mainBox = Data->mainBox;
          if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "size_greater")))) 
          {
               int size_greater = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "size_greater_spin")));            
               argument_[(*i)++] = g_strdup("-s");
               argument_[(*i)++] = g_strdup_printf("+%d", size_greater);
          }
          if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "size_smaller")))) 
          {
               gint size_smaller = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "size_smaller_spin")));            
               argument_[(*i)++] = g_strdup("-s");
               argument_[(*i)++] = g_strdup_printf("-%d", size_smaller);
          }
        }

        static void getTimeOptions(fgrData_t *Data, int *i){
          GtkBox *mainBox = Data->mainBox;
          if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "last_months")))) 
          {
              argument_[(*i)++] = g_strdup((gchar *)get_time_type(mainBox));
              gint last_months = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_months_spin")));            
              argument_[(*i)++] = g_strdup("-m");
              argument_[(*i)++] = g_strdup_printf("%d", last_months);
          }
          else if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                      g_object_get_data(G_OBJECT(mainBox), "last_days"))))
          {
              argument_[(*i)++] = (gchar *)get_time_type(mainBox);
              gint last_days = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_days_spin")));            
              argument_[(*i)++] = g_strdup("-d");
              argument_[(*i)++] = g_strdup_printf("%d", last_days);
          }
          else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "last_hours"))))
          {
              argument_[(*i)++] = (gchar *)get_time_type(mainBox);
              gint last_hours = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_hours_spin")));            
              argument_[(*i)++] = g_strdup("-h");
              argument_[(*i)++] = g_strdup_printf("%d", last_hours);
          }
          else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "last_minutes"))))
          {
              argument_[(*i)++] = g_strdup((gchar *)get_time_type(mainBox));
              gint last_minutes = gtk_spin_button_get_value_as_int (
                  GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_minutes_spin")));            
              argument_[(*i)++] = g_strdup("-k");
              argument_[(*i)++] = g_strdup_printf("%d", last_minutes);
          }
        }

      static void getFileOptions(fgrData_t *Data, int *i){
          GtkBox *mainBox = Data->mainBox;
        // SUID/EXE
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("Executable"))))
            ||
            gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                        g_object_get_data(G_OBJECT(mainBox), _("SUID"))))){
              argument_[(*i)++] = g_strdup("-p");
              if (gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("Executable"))))) {
                argument_[(*i)++] = g_strdup("exe");
              } else {
                argument_[(*i)++] = g_strdup("suid");
              }
              return;
        }
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), _("Octal Permissions")))))
        {
            argument_[(*i)++] = g_strdup("-o");
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "permissions_entry"));
            auto buffer = gtk_entry_get_buffer(entry);
            const char *c = gtk_entry_buffer_get_text (buffer);
            // FIXME: check for valid octal number <= 0777
            if (c && strlen(c)) argument_[(*i)++] = g_strdup(c);
            else argument_[(*i)++] = g_strdup("0666");
            return;
        } 
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), "uid"))))
        {
            auto dd = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(mainBox), "uid_combo")); 
            auto item = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(dd));
            auto selected = gtk_string_object_get_string(item);
          
            if(selected && strlen(selected)) {
                argument_[(*i)++] = g_strdup("-u");
                struct passwd *pw = getpwnam (selected);
                if(pw) {
                    argument_[(*i)++] = g_strdup_printf("%d", pw->pw_uid);
                } else {
                    argument_[(*i)++] = g_strdup_printf("%d",atoi(selected));
                }

            }
            return;
        }
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), "gid"))))
        {
            auto dd = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(mainBox), "uid_combo")); 
            auto item = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(dd));
            auto selected = gtk_string_object_get_string(item);
          
            if(selected && strlen(selected)) {
                argument_[(*i)++] = g_strdup("-g");
                struct group *gr = getgrnam (selected);
                if(gr) {
                    argument_[(*i)++] = g_strdup_printf("%d", gr->gr_gid);
                } else {
                    argument_[(*i)++] = g_strdup_printf("%d",atoi(selected));
                }

            }
            return;
        }

        

      }

      static void getGrepOptions(fgrData_t *Data, int *i){
          GtkBox *mainBox = Data->mainBox;
        if(!gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                    g_object_get_data(G_OBJECT(mainBox), "case_sensitive"))))
        {
            argument_[(*i)++] = g_strdup("-i");
        } 
        if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                    g_object_get_data(G_OBJECT(mainBox), "line_count"))))
        {
            argument_[(*i)++] = g_strdup("-c");
        } 


        auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "grep_entry"));
        auto buffer = gtk_entry_get_buffer(entry);
        const char *token = gtk_entry_buffer_get_text(buffer);  
  
        //const gchar *token = gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "grep_entry")));
        if(token && strlen(token)) {
              if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                        g_object_get_data(G_OBJECT(mainBox), "ext_regexp"))))
            {
                argument_[(*i)++] = g_strdup("-E");
            } else {
                argument_[(*i)++] = g_strdup("-e");
            }
            argument_[(*i)++] = g_strdup(token);

            /* options for grep: ***** */
            if(!gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                        g_object_get_data(G_OBJECT(mainBox), "look_in_binaries"))))
            {
                argument_[(*i)++] = g_strdup("-I");
            }
            if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                        g_object_get_data(G_OBJECT(mainBox), "match_words"))))
            {
                argument_[(*i)++] = g_strdup("-w");
            }
            else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                        g_object_get_data(G_OBJECT(mainBox), "match_lines"))))
            {
                argument_[(*i)++] = g_strdup("-x");
            }
            else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                        g_object_get_data(G_OBJECT(mainBox), "match_no_match"))))
            {
                argument_[(*i)++] = g_strdup("-L");
            }
        }
      }

      
};

template <class Type>
class Fgr {
    Type *fgrData_;
public:

    Fgr (Type *fgrData){
      fgrData_ = fgrData;
      DBG("*** Now running fgr at path =\"%s\"\n", fgrData_->path());
 /*       gchar *base = g_path_get_basename(argv[0]);
        if (strcmp(base, "fgr")) fgrMain(argc, argv);
        else  fgrMain(argc-1, ++argv);
        g_free(base);
        return 0;*/
    }
    ~Fgr(void){
      // FIXME: cleanup huevones!
      DBG("*** FIXME: cleanup huevones!\n");
    }

private:
    int type = 0;
    long size = 0;
    long month_t = 0;
    long unsigned day_t = 0;
    long unsigned hour_t = 0;
    long unsigned min_t = 0;

    static int
    display (char *input) {
        if(terminated)
            return terminated;      /* die quietly and quickly */
        printf ("%s\n", input);
        if(time (NULL) - initial > 3) {
            fflush (NULL);
            initial = time (NULL);
        }
        return terminated;
    }

    static int
    grep (char *file) {
        static const char *arguments[MAX_ARG] ;
        int status = 0;
        if(terminated)
            return terminated;      /* die quietly and quickly */

        arguments[status++] = "grep";
#ifdef GNU_GREP
        //arguments[status++] = "-d";
        //arguments[status++] = "skip";
        //arguments[status++] = "-H";
        if(options & GLOBRUN_NOBINARIES)
            arguments[status++] = "-I";
        if(options & GLOBRUN_ZERO_BYTE)
            arguments[status++] = "-Z";
        if(options & GLOBRUN_INVERT) {
            arguments[status++] = "-L";
        } else {
            arguments[status++] = "-l";
        }
#else
        if(options & GLOBRUN_INVERT) {
            arguments[status++] = "-v";
        } else {
            arguments[status++] = "-l";
        }
#endif
        if(options & GLOBRUN_IGNORE_CASE)
            arguments[status++] = "-i";
        if(options & GLOBRUN_WORDS_ONLY)
            arguments[status++] = "-w";
        if(options & GLOBRUN_LINES_ONLY)
            arguments[status++] = "-x";

        if((options & GLOBRUN_COUNT)) {
            arguments[status++] = "-c";
        }
#ifdef GNU_GREP
        if(options & GLOBRUN_REG_EXP)
            arguments[status++] = "-E";
        else
#endif
            arguments[status++] = "-e";
        
        arguments[status++] = token;

        arguments[status++] = file;
        arguments[status++] = (char *)0;
#if 0
      //  if (options & GLOBBER_VERBOSE)
      {
        int i;
        for (i = 0; arguments[i] != NULL; i++)
          fprintf(stderr,"%s ", arguments[i]);
        fprintf(stderr,"\n");
      }
#endif
        if(fork () == 0) {
            execvp ("grep",(char* const*) arguments);
            fprintf(stderr, "%s: %s\n", strerror (ENOENT), "grep");
            exit (1);
        }
        wait (&status);

        return terminated;
    }


    static void
    finish (int sig) {
        /*printf("\n****\nglob terminated by signal\n****\n"); */
        terminated = 1;
        fflush (NULL);
    }

    static void
    halt (int sig) {
        fflush (NULL);
        globber_destroy (object);
        exit (1);
    }

#define CHECK_ARG if (argc <= i) goto error;
    int
    fgrMain (int argc, char **argv) {
        int i,
          timetype = 0;
        const char *filter = "*";
        int doGrep=0;
        //int (*operate) (char *) = display;
        const char *path = ".";
        initial = time (NULL);

        /* initializations  */
        signal (SIGHUP, halt);
        signal (SIGSEGV, finish);
        signal (SIGKILL, finish);
        signal (SIGTERM, finish);

        if(argc < 2) {
          error:
            fprintf (stdout, _("Usage : %s [OPTIONS]\n"), argv[0]);
            i = 0;
            while(fgrMessage[i])
                fprintf (stdout, "%s", _(fgrMessage[i++]));
            exit (1);
        }
        object = globber_create ();
        for(i = 1; i < argc; i++) {
            if(argv[i][0] == '-' && strcmp(argv[i],"--fgr")) {
                /* options for the globber : **************** */
                if(strstr (argv[i], "v") != NULL) {
                    glob_set_options (object, GLOBBER_VERBOSE);
                    options |= GLOBBER_VERBOSE;
                    continue;
                }
                if(strstr (argv[i], "M") != NULL) {
                    timetype = 1;
                    glob_set_options (object, GLOBBER_MTIME);
                    continue;
                }
                if(strstr (argv[i], "A") != NULL) {
                    timetype = 1;
                    glob_set_options (object, GLOBBER_ATIME);
                    continue;
                }
                if(strstr (argv[i], "C") != NULL) {
                    timetype = 1;
                    glob_set_options (object, GLOBBER_CTIME);
                    continue;
                }
                if(strstr (argv[i], "a") != NULL) {
                    glob_set_options (object, GLOBBER_XDEV);
                    options |= GLOBBER_XDEV;
                    continue;
                }
                if(strstr (argv[i], "v") != NULL) {
                    glob_set_options (object, GLOBBER_VERBOSE);
                    options |= GLOBBER_VERBOSE;
                    continue;
                }
                if(strstr (argv[i], "r") != NULL) {
                    glob_set_options (object, GLOBBER_RECURSIVE);
                    options |= GLOBBER_RECURSIVE;
                    continue;
                }
                if(strstr (argv[i], "R") != NULL) {
                    glob_set_options (object, GLOBBER_RECURSIVE);
                    glob_set_options (object, GLOBBER_RECURSIVE_NO_HIDDEN);
                    options |= GLOBBER_RECURSIVE;
                    options |= GLOBBER_RECURSIVE_NO_HIDDEN;
                    continue;
                }
                if(strstr (argv[i], "u") != NULL) {
                    i++;
                    CHECK_ARG;
                    glob_set_user (object, atol (argv[i]));
                    continue;
                }
                if(strstr (argv[i], "g") != NULL) {
                    i++;
                    CHECK_ARG;
                    glob_set_group (object, atol (argv[i]));
                    continue;
                }

                if(strstr (argv[i], "t") != NULL) {
                    i++;
                    type &= 07777;
                    CHECK_ARG;
                    /*if (strcmp (argv[i], "any") == 0) type &= 07777; */
                    if(strcmp (argv[i], "reg") == 0)
                        type |= S_IFREG;
                    if(strcmp (argv[i], "dir") == 0)
                        type |= S_IFDIR;
                    if(strcmp (argv[i], "sym") == 0)
                        type |= S_IFLNK;
                    if(strcmp (argv[i], "sock") == 0)
                        type |= S_IFSOCK;
                    if(strcmp (argv[i], "blk") == 0)
                        type |= S_IFBLK;
                    if(strcmp (argv[i], "chr") == 0)
                        type |= S_IFCHR;
                    if(strcmp (argv[i], "fifo") == 0)
                        type |= S_IFIFO;
                    if(strcmp (argv[i], "any") != 0) {
                        glob_set_options (object, GLOBBER_TYPE);
                        glob_set_type (object, type);
                    }
                    continue;
                }
                if(strstr (argv[i], "p") != NULL) {
                    i++;
                    /*type &= S_IFMT; */
                    CHECK_ARG;
                    if(strcmp (argv[i], "suid") == 0)
                        type |= S_ISUID;
                    if(strcmp (argv[i], "exe") == 0)
                        type |= S_IXUSR;
                    glob_set_options (object, GLOBBER_PERM);
                    glob_set_type (object, type);
                    continue;
                }
                if(strstr (argv[i], "o") != NULL) {
                    int valor;
                    i++;
                    type &= S_IFMT;
                    CHECK_ARG;
                    sscanf (argv[i], "%o", &valor);
                    type |= (07777 & valor);
                    glob_set_options (object, GLOBBER_PERM);
                    glob_set_type (object, type);
                    continue;
                }

                if(strstr (argv[i], "s") != NULL) {
                    i++;
                    CHECK_ARG;
                    size = atol (argv[i]);
                    if(size < 0){
                        glob_set_sizeL (object, (-1) * size * 1000);
                    } else {
                        glob_set_sizeG (object, size * 1000);
                    }
                    continue;
                }

                if(strstr (argv[i], "D") != NULL) {
                    glob_set_options (object, GLOBBER_ADD_DOT_FILTER);
                    continue;
                }
                // time
                // minutes
                if(strstr (argv[i], "k") != NULL) {
                    i++;
                    CHECK_ARG;
                    glob_set_options (object, GLOBBER_TIME);
                    min_t = atol (argv[i]);
                    glob_set_minutes (object, min_t);
                    continue;
                }
                // hours
                if(strstr (argv[i], "h") != NULL) {
                    i++;
                    CHECK_ARG;
                    glob_set_options (object, GLOBBER_TIME);
                    hour_t = atol (argv[i]);
                    glob_set_hours (object, hour_t);
                    continue;
                }
                // days
                if(strstr (argv[i], "d") != NULL) {
                    i++;
                    CHECK_ARG;
                    glob_set_options (object, GLOBBER_TIME);
                    day_t = atol (argv[i]);
                    glob_set_days (object, day_t);
                    continue;
                }
                // months
                if(strstr (argv[i], "m") != NULL) {
                    i++;
                    CHECK_ARG;
                    glob_set_options (object, GLOBBER_TIME);
                    month_t = atol (argv[i]);
                    glob_set_months (object, month_t);
                    continue;
                }

                if(strstr (argv[i], "f") != NULL) {
                    options |= GLOBRUN_FILTERED;
                    i++;
                    CHECK_ARG;
                    filter = argv[i];
                    /*if (options & GLOBBER_VERBOSE) 
                     *  fprintf(stderr, "filtering %s\n", filter);*/
                    continue;
                }
                /* options for grep : ****************** */
                if(strstr (argv[i], "I") != NULL) {
                    options |= GLOBRUN_NOBINARIES;
                    continue;
                }
                if((strstr (argv[i], "i") != NULL) || (strstr (argv[i], "y") != NULL)) {
                    options |= GLOBRUN_IGNORE_CASE;
                    continue;
                }
                if(strstr (argv[i], "L") != NULL) {
                    options |= GLOBRUN_INVERT;
                    continue;
                }
                if(strstr (argv[i], "c") != NULL) {
                    options |= GLOBRUN_COUNT;
                    continue;
                }
                if(strstr (argv[i], "w") != NULL) {
                    options |= GLOBRUN_WORDS_ONLY;
                    continue;
                }
                if(strstr (argv[i], "x") != NULL) {
                    options |= GLOBRUN_LINES_ONLY;
                    continue;
                }
                if(strstr (argv[i], "Z") != NULL) {
                    options |= GLOBRUN_ZERO_BYTE;
                    continue;
                }
                if(strstr (argv[i], "P") != NULL) {
                    options |= GLOBRUN_PID;
                    printf ("PID=%d\n", (int)getpid ());
                    fflush (NULL);
                    continue;
                }
                if(strstr (argv[i], "E") != NULL) {
                    doGrep=1;
                    i++;
                    CHECK_ARG;
                    token = argv[i];
                    //operate = grep;
                    options |= GLOBRUN_REG_EXP;
                    continue;
                }
                if(strstr (argv[i], "e") != NULL) {
                    doGrep=1;
                    i++;
                    CHECK_ARG;
                    token = argv[i];
                    //operate = grep;
                    options |= GLOBRUN_REG_EXP;
                    options ^= GLOBRUN_REG_EXP;     /* turn off extended regexp */
                    continue;
                }

                if(strstr (argv[i], "V") != NULL) {
                    printf ("This is xffm+ %s\nForward any bug reports to http://sf.net/projects/xffm\n%s\n", 
                            VERSION, COPYRIGHT);
                    return 0;
                }
                fprintf (stdout, "Oops... %s: %s\n", strerror (EINVAL), argv[i]);
                exit (1);
            }
            if(((min_t) || (hour_t) || (day_t) || (month_t)) && !timetype)
                glob_set_options (object, GLOBBER_MTIME);
            if(argv[i] != NULL)
                path = argv[i];
        }                           /* end of argument processing */

        if (doGrep) {
            terminated = globber (object, path, grep, filter);
        } else {
            terminated = globber (object, path, display, filter);
        }

        //terminated = globber (object, path, operate, filter);
    /*        if (terminated) printf("glob run was terminated.\n");*/
        if(!terminated) {           /* die quietly and quickly */
            if(options & GLOBRUN_PID)
                printf ("GLOB DONE=%d\n", (int)getpid ());
        }
        if(options & GLOBBER_VERBOSE) {
            fprintf (stdout, "fgr search complete!\n");
        }
        fflush (NULL);
        globber_destroy (object);
        //sleep(1);
        exit (0);
    }    
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  globber  /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
    
    /* int
    display (char *input) {
        printf ("%s\n", input);     //fflush(stdout); 
        return 0;
    }*/

    /* public */
    int
    glob_clear_options (void *address) {
        if (!address) return 1;
        globber_t *objeto = (globber_t *)address;
        memset(objeto, 0, sizeof(globber_t));
        objeto->user = -1;
        objeto->group = -1;
        objeto->sizeL = -1;
        objeto->sizeG = -1;
        objeto->min_t = -1;
        objeto->hour_t = -1;
        objeto->day_t = -1;
        objeto->month_t = -1;
        return 1;
    }

    void *
    globber_create (void) {
        globber_t *objeto;
        objeto = (globber_t *) malloc (sizeof (globber_t));
        if (!objeto) {
            fprintf(stderr, "Unable to malloc basic structure.\n");
            exit(1);
        }
        glob_clear_options ((void *)objeto);
        return (void *)objeto;
    }

    static void *
    globber_destroy (void *address) {
        if(address)
            free (address);
        return NULL;
    }

    int
    glob_set_options (void *address, int options) {
        if (!address){
            fprintf(stderr, "broken call to glob_set_options()\n");
            exit(1);
        }
        globber_t *objeto = (globber_t *)address;
        objeto->options |= options;
        return 1;
    }

    int
    glob_set_type (void *address, int type) {
        if (!address){
            fprintf(stderr, "broken call to glob_set_type()\n");
            exit(1);
        }
        globber_t *objeto = (globber_t *)address;
        objeto->type = type;
        return 1;
    }

    int
    glob_set_sizeG (void *address, off_t size) {
        if (!address){
            fprintf(stderr, "broken call to glob_set_sizeG()\n");
            exit(1);
        }
        globber_t *objeto = (globber_t *)address;
        glob_set_options (objeto, GLOBBER_SIZE);
        objeto->sizeG = size;
        return 1;
    }

    int
    glob_set_sizeL (void *address, off_t size) {
        if (!address){
            fprintf(stderr, "broken call to glob_set_sizeL()\n");
            exit(1);
        }
        globber_t *objeto = (globber_t *)address;
        glob_set_options (objeto, GLOBBER_SIZE);
        objeto->sizeL = size;
        return 1;
    }

    int
    glob_set_user (void *address, int user) {
        if (!address){
            fprintf(stderr, "broken call to glob_set_user()\n");
            exit(1);
        }
        globber_t *objeto = (globber_t *)address;
        glob_set_options (objeto, GLOBBER_USER);
        objeto->user = user;
        return 1;
    }

    int
    glob_set_group (void *address, int group) {
        if (!address){
            fprintf(stderr, "broken call to glob_set_group()\n");
            exit(1);
        }
        globber_t *objeto = (globber_t *)address;
        ;
        glob_set_options (objeto, GLOBBER_GROUP);
        objeto->group = group;
        return 1;
    }

    int
    glob_set_minutes (void *address, long unsigned min_t) {
        if (!address){
            fprintf(stderr, "broken call to glob_set_time()\n");
            exit(1);
        }
        globber_t *objeto = (globber_t *)address;
        objeto->min_t = min_t;
        return 1;
    }

    int
    glob_set_hours (void *address, long unsigned hour_t) {
        if (!address){
            fprintf(stderr, "broken call to glob_set_time()\n");
            exit(1);
        }
        globber_t *objeto = (globber_t *)address;
        objeto->hour_t = hour_t;
        return 1;
    }

    int
    glob_set_days (void *address,long unsigned day_t) {
        if (!address){
            fprintf(stderr, "broken call to glob_set_time()\n");
            exit(1);
        }
        globber_t *objeto = (globber_t *)address;
        objeto->day_t = day_t;
        return 1;
    }

    int
    glob_set_months (void *address, long unsigned month_t) {
        if (!address){
            fprintf(stderr, "broken call to glob_set_time()\n");
            exit(1);
        }
        globber_t *objeto = (globber_t *)address;
        objeto->month_t = month_t;
        return 1;
    }

    /* if the user defined "operate" function returns TRUE, Globber will exit 
     * and return to calling module with the same return value  */

    int
    globber (void *address, const char *path, int (*operate) (char *), const char *filter) {
        /* these variables must be kept on the heap */
        glob_t dirlist;
        int i;
        char *globstring;
        char *dot_filter = NULL;
        const char *actual_filter = NULL;
        globber_t *object;
        struct stat path_st;
        errno=0;
      /*  if (!path || !strlen(path)) {
            fprintf(stderr, "invalid path: %s\n", path?"\"\"":"NULL");
            return -1;
        }*/
        if(!address)
            object = (globber_t *) globber_create ();
        else
            object = (globber_t *) address;

    // this is debug rather than verbose...
    // if (object->options & GLOBBER_VERBOSE) fprintf(stderr, "---> %s\n", path);
        if(object->options & GLOBBER_TIME) {
            if(object->options & GLOBBER_MTIME)
                object->options &= ((GLOBBER_CTIME | GLOBBER_ATIME) ^ 0xffffffff);
            else if(object->options & GLOBBER_CTIME)
                object->options &= (GLOBBER_ATIME ^ 0xffffffff);
        }

        dirlist.gl_offs = 2;
        if(!operate)
            operate = display;
        actual_filter = filter;
      filter_repeat:

        if(actual_filter) {
            globstring = (char *)malloc (strlen (path) + strlen (actual_filter) + 2);
            if (!globstring){fprintf(stderr,"malloc: %s", strerror(errno)); exit(1);}
            strcpy (globstring, path);
            if(path[strlen (path) - 1] != '/')
                strcat (globstring, "/");
            strcat (globstring, actual_filter);
        } else {
            globstring = (char *)malloc (strlen (path) + 2);
            strcpy (globstring, path);
            //globstring = path;
        }

        // If file cannot be stat(), assume it is not there (whatever).
        // coverity[fs_check_call : FALSE]
        if(stat (path, &path_st) < 0) {
            if (errno){
                DBG("fgr.hh::globber(): stat %s (%s)\n",
                    path, strerror(errno));
                errno=0;
            }
            int pass = object->pass;
            if (!address) free(object);
            free(globstring);
            return (pass);
        }
        if(glob (globstring, GLOB_ERR | GLOB_TILDE, NULL, &dirlist) != 0) {
    // this is debug rather than verbose...
    //    if (object->options & GLOBBER_VERBOSE)
    //      fprintf(stderr, "%s: %s\n", globstring,strerror(ENOENT));
        } else {
            for(i = 0; i < dirlist.gl_pathc; i++) {
                if((object->options & GLOBBER_STAT) && lstat (dirlist.gl_pathv[i], &(object->st))>=0) {
                    if(object->options & GLOBBER_USER) {
                        if(object->user != object->st.st_uid)
                            continue;
                    }
                    if(object->options & GLOBBER_GROUP) {
                        if(object->group != object->st.st_gid)
                            continue;
                    }
                    if(object->options & GLOBBER_TIME) {
                        object->actual = time (NULL);
                        if(object->options & GLOBBER_MTIME)
                            object->tiempo = object->st.st_mtime;
                        if(object->options & GLOBBER_ATIME)
                            object->tiempo = object->st.st_atime;
                        if(object->options & GLOBBER_CTIME)
                            object->tiempo = object->st.st_ctime;

                        if((object->min_t > 0) && ((object->actual - object->tiempo) / MIN_T > object->min_t))
                            continue;
                        if((object->hour_t > 0) && ((object->actual - object->tiempo) / HOUR_T > object->hour_t))
                            continue;
                        if((object->day_t > 0) && ((object->actual - object->tiempo) / DAY_T > object->day_t))
                            continue;

                        if((object->month_t > 0) && ((object->actual - object->tiempo) / MONTH_T > object->month_t))
                            continue;


                    }
                    if(object->options & GLOBBER_SIZE) {
                        if((object->sizeL >= 0) && (object->st.st_size > object->sizeL))
                            continue;
                        if((object->sizeG > 0) && object->st.st_size < object->sizeG)
                            continue;
                    }
                    if(object->options & GLOBBER_PERM) {
                        if((object->st.st_mode & 07777) & (object->type & 07777)) ;
                        else {
                            if((object->st.st_mode & 07777) == (object->type & 07777)) ;
                            else
                                continue;
                        }
                    }

                    if(object->options & GLOBBER_TYPE) {
                        if((object->st.st_mode & S_IFMT) != (object->type & S_IFMT))
                            continue;
                    }
                }
                /* done lstat'ing */
                if((object->pass = (*(operate)) (dirlist.gl_pathv[i])) != 0)
                    break;
            }
        }                           /* initial glob is done */
        free (globstring);
        globfree (&dirlist);
        if(!dot_filter && object->options & GLOBBER_ADD_DOT_FILTER) {
            dot_filter = (char *)malloc (strlen (filter) + 2);
            if (!dot_filter){fprintf(stderr,"malloc: %s", strerror(errno)); exit(1);}
            strcpy (dot_filter, ".");
            strcat (dot_filter, filter);
            actual_filter = dot_filter;
            goto filter_repeat;
        }
        if(dot_filter) {
            free (dot_filter);
            dot_filter = NULL;
            actual_filter = NULL;
        }
        if(object->pass) {
            int pass = object->pass;
            if (!address) free(object);
            return (pass);  /* error returned from function */
        }
#if 10
        if(object->options & GLOBBER_RECURSIVE) {
            DIR *directory;
            struct dirent *d;
            directory = opendir (path);
            if(directory)
                while((d = readdir (directory)) != NULL) {
                    char *fullpath;
                    if(strcmp (d->d_name, ".") == 0 || strcmp (d->d_name, "..") == 0)
                        continue;
                    fullpath = (char *)malloc (strlen (path) + strlen (d->d_name) + 2);
            if (!fullpath){fprintf(stderr,"malloc: %s", strerror(errno)); exit(1);}
                    sprintf (fullpath, "%s/%s", path, d->d_name);

                    if(lstat (fullpath, &(object->st)) < 0) {
                        free (fullpath);
                        continue;
                    }

                    if(!S_ISDIR (object->st.st_mode) || (object->st.st_mode & S_IFMT) == S_IFLNK) {
                        free (fullpath);
                        continue;   /* dont follow symlinks */
                    }
                    if(S_ISDIR (object->st.st_mode) && *(d->d_name) == '.' && (object->options & GLOBBER_RECURSIVE_NO_HIDDEN)) {
                        /*printf("object->options=0x%x\n",(unsigned)object->options); */
                        free (fullpath);
                        continue;   /* dont recurse into hidden directories */
                    }
                    if(object->options & GLOBBER_XDEV && object->st.st_dev != path_st.st_dev) {
                        free (fullpath);
                        continue;   /* dont leave device */
                    }
    // this is debug rather than verbose...
    //      if (object->options & GLOBBER_VERBOSE) fprintf(stderr, "%s: --->\n",fullpath);

                    object->pass = globber (address, fullpath, operate, filter);
                    if(object->pass) {
                        free (fullpath);
                        break;
                    }
                    free (fullpath);
                }
            closedir (directory);

        }
#endif
        int pass = object->pass;
        if (!address) free(object);
        return (pass);
    }
};
} // namespace xf


#endif

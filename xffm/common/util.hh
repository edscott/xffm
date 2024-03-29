#ifndef XFUTIL_HH
#define XFUTIL_HH
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef HAVE_ZIP_H
# include <zip.h>
#endif


#ifndef PARAMS
# if defined PROTOTYPES || (defined __STDC__ && __STDC__)
#  define PARAMS(Args) Args
# else
#  define PARAMS(Args) ()
# endif
#endif


#if ! defined TIMESPEC_H
# define TIMESPEC_H

# if TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
# else
#  if HAVE_SYS_TIME_H
#   include <sys/time.h>
#  else
#   include <time.h>
#  endif
# endif

# ifdef ST_MTIM_NSEC
#  define TIMESPEC_NS(timespec) ((timespec).ST_MTIM_NSEC)
# else
#  define TIMESPEC_NS(timespec) 0
# endif

#endif


/* Use this to suppress gcc's `...may be used before initialized' warnings. */
#ifdef lint
# define IF_LINT(Code) Code
#else
# define IF_LINT(Code)          /* empty */
#endif

enum {
    time_ctime,
    time_mtime,
    time_atime
};



namespace xf
{
template <class Type> class Settings;
template <class Type>
class Util {
private:
    static gint
    compareDown (const void *a, const void *b) {
        auto aa = (const gchar *)a;
        auto bb = (const gchar *)b;
        if (strchr(aa,'/')) aa = strchr(aa,'/')+1;
        if (strchr(bb,'/')) bb = strchr(bb,'/')+1;
        return -strcasecmp((const gchar *)aa, (const gchar *)bb);
    }
    
    static gint
    compareUp (const void *a, const void *b) {
        auto aa = (const gchar *)a;
        auto bb = (const gchar *)b;
        if (strchr(aa,'/')) aa = strchr(aa,'/')+1;
        if (strchr(bb,'/')) bb = strchr(bb,'/')+1;
        return strcasecmp((const gchar *)aa, (const gchar *)bb);
    }

    /* Return a character indicating the type of file described by
       file mode BITS:
       'd' for directories
       'D' for doors
       'b' for block special files
       'c' for character special files
       'n' for network special files
       'm' for multiplexor files
       'M' for an off-line (regular) file
       'l' for symbolic links
       's' for sockets
       'p' for fifos
       'C' for contigous data files
       '-' for regular files
       '?' for any other file type.  */

    static char
    ftypelet (mode_t bits) {
#ifdef S_ISBLK
        if(S_ISBLK (bits))
            return 'b';
#endif
        if(S_ISCHR (bits))
            return 'c';
        if(S_ISDIR (bits))
            return 'd';
        if(S_ISREG (bits))
            return '-';
#ifdef S_ISFIFO
        if(S_ISFIFO (bits))
            return 'p';
#endif
#ifdef S_ISLNK
        if(S_ISLNK (bits))
            return 'l';
#endif
#ifdef S_ISSOCK
        if(S_ISSOCK (bits))
            return 's';
#endif
#ifdef S_ISMPC
        if(S_ISMPC (bits))
            return 'm';
#endif
#ifdef S_ISNWK
        if(S_ISNWK (bits))
            return 'n';
#endif
#ifdef S_ISDOOR
        if(S_ISDOOR (bits))
            return 'D';
#endif
#ifdef S_ISCTG
        if(S_ISCTG (bits))
            return 'C';
#endif

        /* The following two tests are for Cray DMF (Data Migration
           Facility), which is a HSM file system.  A migrated file has a
           `st_dm_mode' that is different from the normal `st_mode', so any
           tests for migrated files should use the former.  */

#ifdef S_ISOFD
        if(S_ISOFD (bits))
            /* off line, with data  */
            return 'M';
#endif
#ifdef S_ISOFL
        /* off line, with no data  */
        if(S_ISOFL (bits))
            return 'M';
#endif
        return '?';
    }
    
public:
  /* Compute mode string.  On most systems, it's based on st_mode.
     On systems with migration (via the stat.st_dm_mode field), use
     the file's migrated status.  */

    static
    gchar *
    modeString (mode_t mode) {
        gchar *str=(gchar *)calloc(1,13);
        if (!str) g_error("calloc: %s", strerror(errno));
        str[0] = ftypelet (mode);
        str[1] = mode & S_IRUSR ? 'r' : '-';
        str[2] = mode & S_IWUSR ? 'w' : '-';
        str[3] = mode & S_IXUSR ? 'x' : '-';
        str[4] = mode & S_IRGRP ? 'r' : '-';
        str[5] = mode & S_IWGRP ? 'w' : '-';
        str[6] = mode & S_IXGRP ? 'x' : '-';
        str[7] = mode & S_IROTH ? 'r' : '-';
        str[8] = mode & S_IWOTH ? 'w' : '-';
        str[9] = mode & S_IXOTH ? 'x' : '-';
        if(mode & S_ISUID)
            str[3] = mode & S_IXUSR ? 's' : 'S';
        if(mode & S_ISGID)
            str[6] = mode & S_IXGRP ? 's' : 'S';
        if(mode & S_ISVTX)
            str[9] = mode & S_IXOTH ? 't' : 'T';
        str[10] = 0;
        return (str);
    }

    static void
    resetObjectData(GObject *object, const gchar *id, gchar *newString){
        auto oldString = g_object_get_data(object, id);
        g_free(oldString);
        g_object_set_data(object, id, (void *) newString);
    }

    static GList *
    sortList(GList *list){
        // Default sort order:
        if (Settings<Type>::getInteger("LocalView", "Descending") <= 0) {
            return g_list_sort (list,compareDown);
        } else {
            return g_list_sort (list,compareUp);
        }
    }

    static gchar *md5sum(const gchar *file){
        gchar *md5sum = g_find_program_in_path("md5sum");
        if (!md5sum){
            ERROR("cannot find md5sum program\n");
            return NULL;
        }
        g_free(md5sum);
        gchar *command = g_strdup_printf("md5sum %s", file);
        FILE *pipe = popen(command, "r");
        if (!pipe){
            ERROR("cannot pipe to %s\n", command);
            g_free(command);
            return NULL;
        }
        gchar buffer[1024];
        memset (buffer, 0, 1024);
        if (!fgets(buffer, 1023, pipe)){
           DBG("fgets(%s): %s\n", command, "no characters read.");
        }
        g_free(command);
        pclose(pipe);
        if (strlen(buffer)) return g_strdup(buffer);
        return NULL;
    }

    static void 
    lineBreaker(gchar *inputLine, gint lineLength){
        if (strlen(inputLine) > lineLength){
            gchar *remainder;
            gchar *p = inputLine+lineLength;
            do{
                if (*p ==' ' || *p =='_') {
                    *p = '\n';
                    remainder = p+1;
                    break;
                }
                p++;
            } while (*p);
            if (*p != 0) lineBreaker(remainder, lineLength);
        }
    }

    static gchar *fileInfo(const gchar *path){
        gchar *file = g_find_program_in_path("file");
        if (!file) return g_strdup("\"file\" command not in path!");
        gchar *result = NULL; 
        gchar *command = g_strdup_printf("%s \'%s\'", file, path);
        result = pipeCommand(command);
        g_free(command);
        g_free(file);

        if (result){
            gchar *retval;
            if (strchr(result, ':')) retval=g_strdup(strchr(result, ':')+1);
            else retval = g_strdup(result);
            g_free(result);
            gchar *p=retval;
            for(;p && *p; p++) {
                if (*p == '<') *p='[';
                else if (*p == '>') *p=']';
            }
            lineBreaker(retval, 40);
            return retval;
        }
        return NULL;
    }

    static gchar *statInfo(const struct stat *st){
        if (!st){
            DBG("util::statinfo: st is null\n");
            return NULL;
        }
        auto mode = modeString(st->st_mode);
        struct passwd *pw = getpwuid (st->st_uid);
        struct group *gr = getgrgid(st->st_gid);
        auto links = st->st_nlink;

        auto user = pw ? g_strdup(pw->pw_name) : g_strdup_printf("%d", st->st_uid);
        auto group = gr ? g_strdup(gr->gr_name) : g_strdup_printf("%d", st->st_gid);

        auto info = g_strdup_printf("%s %lu %s %s", mode, (unsigned long)links, user, group);
        g_free(user);
        g_free(group);
        g_free(mode);
        return info;
    }

    static gchar *argv2command(const gchar **arg){
        // Build command line
        gchar *command = g_strdup(arg[0]);
        for (auto p=arg+1; p && *p; p++){
            gchar *g;
            if (strchr(*p, ' ')) {
                g = g_strconcat(command, " \"",*p,"\"", NULL);
            } else {
                g = g_strconcat(command, " ",*p, NULL);
            }
            g_free(command);
            command = g;
        }
        return command;
    }
#define PAGE_LINE 256

    static gchar *pipeCommand(const gchar *command){
        FILE *pipe = popen (command, "r");
        if(pipe) {
            gchar line[PAGE_LINE];
            line[PAGE_LINE - 1] = 0;
            if (!fgets (line, PAGE_LINE - 1, pipe)){
                  DBG("fgets(%s): %s\n", command, "no characters read.");
            } else {
              if (strchr(line, '\n'))*(strchr(line, '\n'))=0;
            }
            pclose (pipe);
            return g_strdup(line);
        } 
        return NULL;
    }

    static gchar *pipeCommandFull(const gchar *command){
        FILE *pipe = popen (command, "r");
        if(pipe) {
            gchar *result=g_strdup("");
            gchar line[PAGE_LINE];
            while (fgets (line, PAGE_LINE - 1, pipe) && !feof(pipe)){
                line[PAGE_LINE - 1] = 0;
                auto g = g_strconcat(result, line, NULL);
                g_free(result);
                result = g;
            }
            pclose (pipe);
            return result;
        } 
        return NULL;
    }


    static void
    threadwait (void) {
        struct timespec thread_wait = {
            0, 100000000
        };
        nanosleep (&thread_wait, NULL);
    }

    static gint
    length_equal_string(const gchar *a, const gchar *b){
        int length=0;
        int i;
        for (i = 0; i < strlen(a) && i < strlen(b); i++){
            if (strncmp(a,b,i+1)) {
                length=i;
                break;
            } else {
                length=i+1;
            }
        }
         TRACE("%s --- %s differ at length =%d\n", a,b,length);
       return length;
    }

    static gchar *
    get_tilde_dir(const gchar *token){
        struct passwd *pw;
        gchar *tilde_dir = NULL;
        while((pw = getpwent ()) != NULL) {
            gchar *id = g_strdup_printf("~%s/", pw->pw_name);
            if (strncmp(token, id, strlen(id))==0){
                tilde_dir = g_strdup_printf("%s/", pw->pw_dir);
                g_free(id);
                break;
            }
            g_free(id);
        }
        endpwent ();
        return tilde_dir;
    }


    static gint
    translate_key (gint x) {
        switch (x) {
        case GDK_KEY_KP_Divide:
            return GDK_KEY_slash;
        case GDK_KEY_KP_Subtract:
            return GDK_KEY_minus;
        case GDK_KEY_KP_Multiply:
            return GDK_KEY_asterisk;
        case GDK_KEY_KP_Add:
            return GDK_KEY_plus;
        case GDK_KEY_KP_Space:
            return GDK_KEY_space;
        case GDK_KEY_KP_0:
            return GDK_KEY_0;
        case GDK_KEY_KP_1:
            return GDK_KEY_1;
        case GDK_KEY_KP_2:
            return GDK_KEY_2;
        case GDK_KEY_KP_3:
            return GDK_KEY_3;
        case GDK_KEY_KP_4:
            return GDK_KEY_4;
        case GDK_KEY_KP_5:
            return GDK_KEY_5;
        case GDK_KEY_KP_6:
            return GDK_KEY_6;
        case GDK_KEY_KP_7:
            return GDK_KEY_7;
        case GDK_KEY_KP_8:
            return GDK_KEY_8;
        case GDK_KEY_KP_9:
            return GDK_KEY_9;
        }
        return x;
    }

    static gint
    compose_key (gint key, gint dead_key) {
        switch (dead_key) {
        case GDK_KEY_dead_grave:
            switch (key) {
            case GDK_KEY_A:
                return GDK_KEY_Agrave;
            case GDK_KEY_a:
                return GDK_KEY_agrave;
            case GDK_KEY_E:
                return GDK_KEY_Egrave;
            case GDK_KEY_e:
                return GDK_KEY_egrave;
            case GDK_KEY_I:
                return GDK_KEY_Igrave;
            case GDK_KEY_i:
                return GDK_KEY_igrave;
            case GDK_KEY_O:
                return GDK_KEY_Ograve;
            case GDK_KEY_o:
                return GDK_KEY_ograve;
            case GDK_KEY_U:
                return GDK_KEY_Ugrave;
            case GDK_KEY_u:
                return GDK_KEY_ugrave;
            }
            break;
        case GDK_KEY_dead_acute:
            TRACE ("dead key=0x%x composing %c\n", (unsigned)dead_key, (char)key);
            switch (key) {
            case GDK_KEY_A:
                return GDK_KEY_Aacute;
            case GDK_KEY_a:
                return GDK_KEY_aacute;
            case GDK_KEY_E:
                return GDK_KEY_Eacute;
            case GDK_KEY_e:
                return GDK_KEY_eacute;
            case GDK_KEY_I:
                return GDK_KEY_Iacute;
            case GDK_KEY_i:
                return GDK_KEY_iacute;
            case GDK_KEY_O:
                return GDK_KEY_Oacute;
            case GDK_KEY_o:
                return GDK_KEY_oacute;
            case GDK_KEY_U:
                return GDK_KEY_Uacute;
            case GDK_KEY_u:
                return GDK_KEY_uacute;
            case GDK_KEY_Y:
                return GDK_KEY_Yacute;
            case GDK_KEY_y:
                return GDK_KEY_yacute;
            case GDK_KEY_S:
                return GDK_KEY_Sacute;
            case GDK_KEY_Z:
                return GDK_KEY_Zacute;
            case GDK_KEY_s:
                return GDK_KEY_sacute;
            case GDK_KEY_z:
                return GDK_KEY_zacute;
            case GDK_KEY_R:
                return GDK_KEY_Racute;
            case GDK_KEY_r:
                return GDK_KEY_racute;
            case GDK_KEY_L:
                return GDK_KEY_Lacute;
            case GDK_KEY_l:
                return GDK_KEY_lacute;
            case GDK_KEY_C:
                return GDK_KEY_Cacute;
            case GDK_KEY_c:
                return GDK_KEY_cacute;
            case GDK_KEY_N:
                return GDK_KEY_Nacute;
            case GDK_KEY_n:
                return GDK_KEY_nacute;
            }
            break;
        case GDK_KEY_dead_diaeresis:
            switch (key) {
            case GDK_KEY_A:
                return GDK_KEY_Adiaeresis;
            case GDK_KEY_a:
                return GDK_KEY_adiaeresis;
            case GDK_KEY_E:
                return GDK_KEY_Ediaeresis;
            case GDK_KEY_e:
                return GDK_KEY_ediaeresis;
            case GDK_KEY_I:
                return GDK_KEY_Idiaeresis;
            case GDK_KEY_i:
                return GDK_KEY_idiaeresis;
            case GDK_KEY_O:
                return GDK_KEY_Odiaeresis;
            case GDK_KEY_o:
                return GDK_KEY_odiaeresis;
            case GDK_KEY_U:
                return GDK_KEY_Udiaeresis;
            case GDK_KEY_u:
                return GDK_KEY_udiaeresis;
            case GDK_KEY_Y:
                return GDK_KEY_Ydiaeresis;
            case GDK_KEY_y:
                return GDK_KEY_ydiaeresis;
            }
            break;
        case GDK_KEY_dead_cedilla:
            switch (key) {
            case GDK_KEY_C:
                return GDK_KEY_Ccedilla;
            case GDK_KEY_c:
                return GDK_KEY_ccedilla;
            case GDK_KEY_S:
                return GDK_KEY_Scedilla;
            case GDK_KEY_s:
                return GDK_KEY_scedilla;
            case GDK_KEY_T:
                return GDK_KEY_Tcedilla;
            case GDK_KEY_t:
                return GDK_KEY_tcedilla;
            case GDK_KEY_R:
                return GDK_KEY_Rcedilla;
            case GDK_KEY_r:
                return GDK_KEY_rcedilla;
            case GDK_KEY_L:
                return GDK_KEY_Lcedilla;
            case GDK_KEY_l:
                return GDK_KEY_lcedilla;
            case GDK_KEY_G:
                return GDK_KEY_Gcedilla;
            case GDK_KEY_g:
                return GDK_KEY_gcedilla;
            case GDK_KEY_N:
                return GDK_KEY_Ncedilla;
            case GDK_KEY_n:
                return GDK_KEY_ncedilla;
            case GDK_KEY_K:
                return GDK_KEY_Kcedilla;
            case GDK_KEY_k:
                return GDK_KEY_kcedilla;
            }
            break;
        case GDK_KEY_dead_circumflex:
            switch (key) {
            case GDK_KEY_A:
                return GDK_KEY_Acircumflex;
            case GDK_KEY_a:
                return GDK_KEY_acircumflex;
            case GDK_KEY_E:
                return GDK_KEY_Ecircumflex;
            case GDK_KEY_e:
                return GDK_KEY_ecircumflex;
            case GDK_KEY_I:
                return GDK_KEY_Icircumflex;
            case GDK_KEY_i:
                return GDK_KEY_icircumflex;
            case GDK_KEY_O:
                return GDK_KEY_Ocircumflex;
            case GDK_KEY_o:
                return GDK_KEY_ocircumflex;
            case GDK_KEY_U:
                return GDK_KEY_Ucircumflex;
            case GDK_KEY_u:
                return GDK_KEY_ucircumflex;
            case GDK_KEY_H:
                return GDK_KEY_Hcircumflex;
            case GDK_KEY_h:
                return GDK_KEY_hcircumflex;
            case GDK_KEY_J:
                return GDK_KEY_Jcircumflex;
            case GDK_KEY_j:
                return GDK_KEY_jcircumflex;
            case GDK_KEY_C:
                return GDK_KEY_Ccircumflex;
            case GDK_KEY_c:
                return GDK_KEY_ccircumflex;
            case GDK_KEY_G:
                return GDK_KEY_Gcircumflex;
            case GDK_KEY_g:
                return GDK_KEY_gcircumflex;
            case GDK_KEY_S:
                return GDK_KEY_Scircumflex;
            case GDK_KEY_s:
                return GDK_KEY_scircumflex;
            }
            break;
        }
        return key;
    }

    static gint
    deadkey (int key) {
        /* support for deadkeys */
        switch (key) {
            /* spanish */
        case GDK_KEY_dead_acute:
        case GDK_KEY_dead_diaeresis:
            return key;
            /* french */
        case GDK_KEY_dead_cedilla:
        case GDK_KEY_dead_grave:
        case GDK_KEY_dead_circumflex:
            return key;
            /* others (if you want any of these, submit a request) */
        case GDK_KEY_dead_tilde:
        case GDK_KEY_dead_macron:
        case GDK_KEY_dead_breve:
        case GDK_KEY_dead_abovedot:
        case GDK_KEY_dead_abovering:
        case GDK_KEY_dead_doubleacute:
        case GDK_KEY_dead_caron:
        case GDK_KEY_dead_ogonek:
        case GDK_KEY_dead_iota:
        case GDK_KEY_dead_voiced_sound:
        case GDK_KEY_dead_semivoiced_sound:
        case GDK_KEY_dead_belowdot:
    /* these two are > gtk-2.2: */
    /*     case GDK_KEY_dead_hook:*/
    /*     case GDK_KEY_dead_horn:*/
            return 0;
        default:
            return 0;
        }
    }


    static gchar *
    valid_utf_pathstring (const gchar * string) {
        gchar *utf_string = NULL;
        utf_string = recursive_utf_string (string);
        TRACE ("valid_utf_pathstring: string=%s utf_string=%s\n", string, utf_string);
        return utf_string;
    }


    static gchar *
    recursive_utf_string (const gchar * path) {
        gchar *dir,
         *base,
         *valid,
         *utf_base,
         *utf_dir;
        if(!path)
            return NULL;
        if(g_utf8_validate (path, -1, NULL))
            return g_strdup (path);
        dir = g_path_get_dirname (path);
        TRACE ("dir=%s\n", dir);
        if(!dir || !strlen (dir) || strcmp (dir, "./") == 0 || strcmp (dir, ".") == 0) {
            /* short circuit non-paths */
            g_free (dir);
            return utf_string (path);
        }
        /* otherwise asume a mixed utf/locale string */
        base = g_path_get_basename (path);
        utf_dir = recursive_utf_string (dir);
        if(!g_utf8_validate (base, -1, NULL)) {
            utf_base = utf_string (base);
            g_free (base);
        } else {
            utf_base = base;
        }

        valid = g_strconcat (utf_dir, G_DIR_SEPARATOR_S, utf_base, NULL);

        TRACE("dir=%s base=%s valide=%s\n",dir, base, valid); 
        g_free (utf_base);
        g_free (utf_dir);
        g_free (dir);
        return valid;
    }


    static void
    set_store_data_from_list(GtkListStore *list_store, GSList **list){
        GtkTreeIter iter;
        gtk_list_store_clear (list_store);
        GSList *p = *list;
        for (; p && p->data; p=p->next) {
            gtk_list_store_append (list_store, &iter);
            gtk_list_store_set (list_store, &iter,
                              0, (gchar *)p->data,
                              -1);
          /* Note: The store will keep a copy of the string internally, 
           * so the list may be deallocated */
        }
    }


    static void getPasswordPrompt(void){
        gchar *getpass = g_find_program_in_path("xfgetpass");
        if (!getpass) {
            static const gchar *
                buildGetPass = g_build_path(G_DIR_SEPARATOR_S,buildDir,"xfgetpass",NULL);
            setenv("SUDO_ASKPASS", buildGetPass, 1);
            setenv("SSH_ASKPASS", buildGetPass, 1);
            TRACE("get pass at %s\n", buildGetPass);
        } else {
            TRACE("get pass at %s\n", getpass);
            setenv("SUDO_ASKPASS", getpass, 1);
            setenv("SSH_ASKPASS", getpass, 1);
        }
    }

    static const gchar *getEditor(){
        setEditor();
        return getenv("EDITOR");        
    }

    static const gchar *getTerminal(){
        setTerminal();
        return getenv("TERMINAL");
    }

    static const gchar *getTerminalCmd(){
        setTerminal();
        return  getenv("TERMINAL_CMD");
    }

private:

    static const gchar *fixTerminalEditor(const gchar *e){
        // Terminal based editors...
        for (auto p=getTerminalEditors(); p && *p; p++){
            if (strncmp(e, *p,strlen(*p))==0){
                auto terminalCmd = getTerminalCmd();
                // A terminal based editor.
                static gchar *f = g_strdup_printf("%s %s", terminalCmd, e); 
                setenv("EDITOR", f, 1);
                return f;
            }
        }
        return e;
    }

    static const gchar *fixGvim(const gchar *e){
        // Do not fork gvim, so that git commit works...
        if (e && strcmp(e, "gvim")==0) return "gvim -f";
        return e;
    }

    static void setEditor(void){
        static gboolean done = FALSE;
        if (done) return;

        // Environment variable EDITOR was defined previously.
        const gchar *e = getenv("EDITOR");
        if (e && strlen(e)==0) e = NULL;

        else if (e) { // Predefined value.
            e = fixGvim(e);
            e = fixTerminalEditor(e);
            setenv("EDITOR", e, 1);
            done = TRUE;
            return;
        }

        // Environment variable EDITOR was not defined.
        // Look for one.
        auto editors = getEditors();
        for (auto p=editors; p && *p; p++){
            auto s = g_strdup(*p);
            if (strchr(s, ' ')) *strchr(s, ' ') = 0;
            auto t = g_find_program_in_path (s);
            g_free(s);
            if (t) {
                e=*p;
                g_free(t);
                break;  
            }  
        }

        if (!e){
            DBG("No suitable EDITOR found, defaulting to gvim. Please install or define EDITOR environment variable.\n");
            e="vi";
        } else {
            INFO("Found EDITOR %s\n", e);

        }
        e = fixGvim(e);
        e = fixTerminalEditor(e);
        setenv("EDITOR", e, 1);
        done = TRUE;
        return;
    }

    static void setTerminal(void){
        static gboolean done = FALSE;
        if (done) return;
        const gchar *terminal = getenv("TERMINAL");
        if (terminal && strlen(terminal)) {
            INFO("User set terminal = %s\n", terminal);
            setTerminalCmd(terminal);
            done = TRUE;
            return;
        } 
        DBG("setTerminal()... TERMINAL not defined in environment.\n");
        // TERMINAL not defined. Look for one.
        const gchar **p=getTerminals();
        const gchar *foundTerm = NULL;
        for (;p && *p; p++){
            auto s = g_strdup(*p);
            if (strchr(s, ' ')) *strchr(s, ' ') = 0;
            auto t = g_find_program_in_path (s);
            g_free(s);
            if (t) {
                INFO("Found terminal: %s\n", t);
                terminal=*p;
                g_free(t);
                setenv("TERMINAL", *p, 1);
                setTerminalCmd(*p);
                done = TRUE;
                return;
            }  
        }
        if (!terminal){
            DBG("No terminal command found. Please install or define TERMINAL environment variable.\n");
            // Fallback...
            setenv("TERMINAL", "xterm", 1);
            setTerminalCmd("xterm");
        }
        done = TRUE;
        return ;
    }

    static void
    setTerminalCmd (const gchar *t) {
        static gboolean done = FALSE;
        if (done) return;
        const gchar *exec_option = "-e";
        if(strncmp (t, "gnome-terminal", strlen("gnome-terminal")) == 0 ||
           strncmp (t, "Terminal", strlen("Terminal")) == 0) {
            exec_option = "-x";
        }
        static const gchar *terminalCommand = g_strconcat(t, " ", exec_option, NULL);
        setenv("TERMINAL_CMD", terminalCommand, 1);
        return;
    }
     
    static const gchar **
    getTerminals(void) {
        static const gchar *terminals_v[] = {
            "xterm -vb -rv", 
            "uxterm -vb -rv", 
            "konsole", 
            "gnome-terminal", 
            "sakura",
            "Eterm", 
            "Terminal", 
            "aterm", 
            "kterm", 
            "wterm", 
            NULL
        };
        return terminals_v;
    }

    static const gchar **
    getEditors(void) {
        static const gchar *editors_v[] = {
            "gvim -f",  
            "gedit", 
            "kate", 
            "xemacs", 
            "nano",
            "vi",
            NULL
        }; 
        return editors_v;
    }
    static const gchar **
    getTerminalEditors(void) {
        static const gchar *editors_v[] = {
            "emacs", 
            "nano",
            "vi",
            "vim",
            NULL
        }; 
        return editors_v;
    }
    
    
public:
    
       
    // FIXME: valgrind does not like this function. It has something
    //        which is broken.
    /*
    gchar *
    wrap_utf_string(const gchar *data, gint length){
        gchar *u;
        if (!g_utf8_validate (data, -1, NULL)){
            u = utf_string(data);
        } else {
            u = g_strdup(data);
        }
        gchar *uu = NULL;
        gint i;for (i=0; i<g_utf8_strlen(u, -1); i += length){
            gchar *xu = g_utf8_substring (u, i, i+length);
            if (!uu) uu = g_strdup(xu);
            else { 
                gchar *g = g_strconcat(uu,"\n",xu,NULL);
                g_free(xu);
                g_free(uu);
                uu = g;
            }
        }
        g_free(u);
        return uu;
    }
    */

    static gboolean
    context_function_f(gpointer data){
        void **arg = (void **)data;
        void * (*function)(gpointer) = (void* (*)(void*))arg[0];
        gpointer function_data = arg[1];
        pthread_mutex_t *mutex = (pthread_mutex_t *)arg[2];
        pthread_cond_t *signal = (pthread_cond_t *)arg[3];
        void **result_p = (void **)arg[4];
        void *result = (*function)(function_data);
        pthread_mutex_lock(mutex);
        *result_p = result;
        pthread_cond_signal(signal);
        pthread_mutex_unlock(mutex);
        return FALSE;
    }

    static void *
    context_function(void * (*function)(gpointer), void * function_data){
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t signal = PTHREAD_COND_INITIALIZER; 
        void *result=GINT_TO_POINTER(-1);
        void *arg[] = {
            (void *)function,
            (void *)function_data,
            (void *)&mutex,
            (void *)&signal,
            (void *)&result
        };
        gboolean owner = g_main_context_is_owner(g_main_context_default());
        if (owner){
            context_function_f(arg);
        } else {
            g_main_context_invoke(NULL, context_function_f, arg);
            pthread_mutex_lock(&mutex);
            if (result == GINT_TO_POINTER(-1)) pthread_cond_wait(&signal, &mutex);
            pthread_mutex_unlock(&mutex);
        }
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&signal);
        return result;
    }

    static gchar *
    utf_string (const gchar * t) {
        if(!t) { return g_strdup (""); }
        if(g_utf8_validate (t, -1, NULL)) { return g_strdup (t); }
        /* so we got a non-UTF-8 */
        /* but is it a valid locale string? */
        gchar *actual_tag;
        actual_tag = g_locale_to_utf8 (t, -1, NULL, NULL, NULL);
        if(actual_tag)
            return actual_tag;
        /* So it is not even a valid locale string... 
         * Let us get valid utf-8 caracters then: */
        const gchar *p;
        actual_tag = g_strdup ("");
        for(p = t; *p; p++) {
            // short circuit end of string:
            gchar *r = g_locale_to_utf8 (p, -1, NULL, NULL, NULL);
            if(g_utf8_validate (p, -1, NULL)) {
                gchar *qq = g_strconcat (actual_tag, p, NULL);
                g_free (actual_tag);
                actual_tag = qq;
                break;
            } else if(r) {
                gchar *qq = g_strconcat (actual_tag, r, NULL);
                g_free (r);
                g_free (actual_tag);
                actual_tag = qq;
                break;
            }
            // convert caracter to utf-8 valid.
            gunichar gu = g_utf8_get_char_validated (p, 2);
            if(gu == (gunichar) - 1) {
                gu = g_utf8_get_char_validated ("?", -1);
            }
            gchar outbuf[8];
            memset (outbuf, 0, 8);
            gint outbuf_len = g_unichar_to_utf8 (gu, outbuf);
            if(outbuf_len < 0) {
                ERROR ("utf_string: unichar=%d char =%c outbuf_len=%d\n", gu, p[0], outbuf_len);
            }
            gchar *qq = g_strconcat (actual_tag, outbuf, NULL);
            g_free (actual_tag);
            actual_tag = qq;
        }
        return actual_tag;
    }

#define MAX_PATH_LABEL 40
#define MAX_PATH_START_LABEL 18
    static const gchar *
    chop_excess (gchar * b) {
        // chop excess length...

        const gchar *home = g_get_home_dir();
        gchar *bb;
        if (strncmp(home, b, strlen(home))==0){
            if (strlen(home) == strlen(b)) return b;
            bb = g_strconcat("~/", b + strlen(home)+1, NULL);
        } else {
            bb = g_strdup(b);
        }
        
        int len = strlen (bb);

        if(len < MAX_PATH_LABEL) {
            strcpy(b, bb);
            g_free(bb);
            return (b);
        }
            
        bb[MAX_PATH_START_LABEL - 3] = 0;

        gchar *g = g_strconcat(bb, "...", b + (len - MAX_PATH_LABEL + MAX_PATH_START_LABEL), NULL);
        strcpy (b, g);
        g_free(bb);
        g_free(g);

        return b;
    }

#define MAX_NAME_LENGTH 13
    static gboolean
    chopBeginning (gchar * b) {
        gint len = strlen (b);

        if(len <= MAX_NAME_LENGTH) {
            return FALSE;
        }
        gint newStart = len - MAX_NAME_LENGTH;
        memmove(b, b+newStart, MAX_NAME_LENGTH+1);
        return TRUE;
    }



    static gchar *
    compact_line(const gchar *line){
        //1. Remove leading and trailing whitespace
        //2. Compact intermediate whitespace

        gchar *newline= g_strdup(line); 
        g_strstrip(newline);
        gchar *p = newline;
        for(;p && *p; p++){
            if (*p ==' ') g_strchug(p+1);
        }
        return newline;
    }

    static GList *
    find_in_string_list(GList *list, const gchar *string){
        GList *tmp=g_list_first(list);
        for (;tmp && tmp->data; tmp=tmp->next){
            if (strcmp((gchar *)tmp->data, string)==0) {
                return tmp;
            }
        }
        return NULL;
    }

    static gboolean
    program_in_path(const gchar *program){
        gchar *s = g_find_program_in_path (program);
        if (!s) return FALSE;
        g_free(s);
        return TRUE;
    }

    static const gchar *
    default_shell(void){
        const gchar *shells[]={"bash","tcsh","csh","dash","zsh","ksh","sash","ash","sh",NULL};
        const gchar **shell;
        for (shell = shells; shell && *shell; shell++){
            if (program_in_path(*shell)) return *shell;
        }
        g_warning("unable to find a valid shell\n");
        return "/bin/sh";
    }

    static const gchar *
    u_shell(void){
        if(getenv ("SHELL") && strlen (getenv ("SHELL"))) {
            if (program_in_path(getenv ("SHELL"))) return getenv ("SHELL");
        }

        if(getenv ("XTERM_SHELL") && strlen (getenv ("XTERM_SHELL"))) {
            if (program_in_path(getenv ("XTERM_SHELL"))) return getenv ("XTERM_SHELL");
        }
        return default_shell();
    }


    static gchar *
    esc_string (const gchar * string) {
        gint i, j, k;
        const gchar *charset = "\\\"\' ()|<>";

        for(j = 0, i = 0; i < strlen (string); i++) {
            for(k = 0; k < strlen (charset); k++) {
                if(string[i] == charset[k])
                    j++;
            }
        }
        gchar *estring = (gchar *) calloc (1, strlen (string) + j + 1);
        for(j = 0, i = 0; i < strlen (string); i++, j++) {
            for(k = 0; k < strlen (charset); k++) {
                if(string[i] == charset[k])
                    estring[j++] = '\\';
            }
            estring[j] = string[i];
        }
        TRACE ("ESC:estring=%s\n", estring);
        return estring;
    }

    // TreeModel histories
    static GtkTreeModel *
    loadHistory (const gchar *history) {
        GtkTreeStore *store = gtk_tree_store_new (1, G_TYPE_STRING);
        FILE *historyFile = fopen (history, "r");
        if(!historyFile) {
            TRACE("util_c::loadHistory(): creating new history: \"%s\"\n", history);
            return GTK_TREE_MODEL(store);
        }
        gchar line[2048];
        memset (line, 0, 2048);
        GtkTreeIter iter;
        while(fgets (line, 2047, historyFile) && !feof (historyFile)) {
            if(strchr (line, '\n')) *strchr (line, '\n') = 0;
            gchar *newline = compact_line(line);
            //if(strlen (line) == 0) continue;
            gtk_tree_store_append (store, &iter, NULL);
            gtk_tree_store_set (store, &iter, 0, newline, -1);
            TRACE("setting tree store value: %s\n" , newline);
            g_free(newline);
        }
        fclose (historyFile);
        return GTK_TREE_MODEL(store);
    }

    static void 
    saveHistory (const gchar *history, GtkTreeModel *model, const gchar *data) {
        gchar *historyDir = g_path_get_dirname(history);
        TRACE("history dir = %s\n", historyDir);
        if (!g_file_test(historyDir,G_FILE_TEST_IS_DIR)){
            g_mkdir_with_parents (historyDir, 0770);
        }
        g_free(historyDir);
        GtkTreeStore *store =GTK_TREE_STORE(model);
        GtkTreeIter iter;
        GtkTreeIter *iter_p;
        gboolean next = TRUE;
        gboolean found = FALSE;
        next = gtk_tree_model_get_iter_first (model, &iter);
        while (next){
            gchar *value;
            gtk_tree_model_get (model, &iter, 0, &value, -1);
            if (strcmp(data, value)==0) {
                g_free(value); // yeah...
                found = TRUE;
                break;
            }
            g_free(value);
            next = gtk_tree_model_iter_next(model, &iter);
        }

        // if item is already in history, bring it to the front
        // else, prepend item.
        if (found) {
            gtk_tree_store_move_after (store, &iter, NULL);
        } else {
            gtk_tree_store_prepend (store, &iter, NULL);
            gtk_tree_store_set (store, &iter, 0, data, -1);
            TRACE("prepending tree store value: %s\n", data);
        }
        // rewrite history file
        FILE *historyFile = fopen (history, "w");
        if(!historyFile) {
            ERROR("util_c::saveHistory(): unable to write to file: \"%s\"\n", history);
            return;
        }

        next = gtk_tree_model_get_iter_first (model, &iter);
        while (next){
            gchar *value;
            gtk_tree_model_get (model, &iter, 0, &value, -1);
                
            fprintf (historyFile, "%s\n", (gchar *)value);

            g_free(value);
            next = gtk_tree_model_iter_next(model, &iter);
        }

        fclose (historyFile);
        return;
    }
    
    // List histories
    static gpointer
    loadHistory (const gchar *history, GList **history_list_p) {
        //gchar *history = g_build_filename (CSH_HISTORY, NULL);
        GList *p;
        // clean out history list before loading a new one.
        for(p = *history_list_p; p; p = p->next) {
            g_free (p->data);
        }
        g_list_free (*history_list_p);
        *history_list_p = NULL;

        FILE *historyFile = fopen (history, "r");
        if(historyFile) {
            gchar line[2048];
            memset (line, 0, 2048);
            while(fgets (line, 2047, historyFile) && !feof (historyFile)) {
                if(strchr (line, '\n')) *strchr (line, '\n') = 0;
                if(strlen (line) == 0) continue;
                gchar *newline = compact_line(line);
                GList *element = find_in_string_list(*history_list_p, newline);

                if (element) { 
                    // remove old element
                    gchar *data=(gchar *)element->data;
                    *history_list_p = g_list_remove(*history_list_p, data);
                    g_free(data);
                }
                // put element at top of the pile
                *history_list_p = g_list_prepend(*history_list_p, newline);
            }
            fclose (historyFile);
        }
            
        return NULL;
    }


    static void 
    saveHistory (const gchar *history, GList **history_list_p, const gchar * data) {
        gchar *historyDir = g_path_get_dirname(history);
        TRACE("history dir = %s\n", historyDir);
        if (!g_file_test(historyDir,G_FILE_TEST_IS_DIR)){
            g_mkdir_with_parents (historyDir, 0660);
        }
        g_free(historyDir);
        GList *p;
        gchar *item = g_strdup(data);
        g_strstrip (item);
        // Get last registered item
        void *last = g_list_nth_data (*history_list_p, 0);
        if (last && strcmp((gchar *)last, item) == 0) {
            g_free(item);
            // repeat of last item. nothing to do here.
            return;
        }

        // if item is already in history, bring it to the front
        GList *itemData = find_in_string_list(*history_list_p, item);
        if (itemData){
            void *data = itemData->data;
            // remove old position
            *history_list_p = g_list_remove(*history_list_p, data);
            // insert at top of list (item 0 is empty string)
            *history_list_p = g_list_insert(*history_list_p, data, 0);
        } else {
            // so the item was not found. proceed to prepend
            TRACE("prepend: %s\n", item);
            *history_list_p = g_list_prepend(*history_list_p, item);
        }

        // rewrite history file
        //gchar *history = g_build_filename (CSH_HISTORY, NULL);
        // read it first to synchronize with other xffm+ instances
        GList *disk_history = NULL;       
        FILE *historyFile = fopen (history, "r");
        if(historyFile) {
            char line[2048];
            memset (line, 0, 2048);
            while(fgets (line, 2047, historyFile) && !feof (historyFile)) {
                if(strchr (line, '\n')) *strchr (line, '\n') = 0;
                if(strcmp (line, item) != 0) {
                    disk_history = g_list_prepend (disk_history, g_strdup (line));
                }
            }
            fclose (historyFile);
        }
        disk_history = g_list_prepend (disk_history, g_strdup (item));
        disk_history = g_list_reverse(disk_history);

        // write it out
        historyFile = fopen (history, "w");
        if(historyFile) {
            GList *p;
            for(p = g_list_first (disk_history); p && p->data; p = p->next) {
                fprintf (historyFile, "%s\n", (gchar *)p->data);
                g_free (p->data);
            }
            fclose (historyFile);
        }
        // cleanout old history list
        for(p = g_list_first (*history_list_p); p && p->data; p = p->next) {
            g_free(p->data);
        }
        g_list_free (*history_list_p);
        // assign new history list
        *history_list_p = disk_history;
        return;
    }
    

};

#if 0
// Used by execute()
static gchar *
get_command_fmt(record_entry_t *en) {
    gchar *command_fmt = MIME_command (en->mimetype);
    if(!command_fmt) {
        command_fmt = MIME_command (en->mimemagic);
    }
    gboolean is_text = ((en->mimetype && strstr (en->mimetype, "text/")) ||
                        (en->mimemagic && strstr (en->mimemagic, "text/")) || 
                        (en->filetype && strstr (en->filetype, "text")));
    if(!command_fmt && is_text && getenv ("EDITOR_CMD") && strlen (getenv ("EDITOR_CMD"))) {
        command_fmt = g_strdup (getenv ("EDITOR_CMD"));
    }
    return command_fmt;
}

// Used by execute()
static gchar *
strip_path(gchar *command_fmt, const gchar *path){
    if (!path) return command_fmt;
    TRACE("stipping %s\n", command_fmt);
    if (strstr(command_fmt, path)){
        gchar *end = strstr(command_fmt,path) + strlen(path);
        *strstr(command_fmt, path) = 0;
        gchar *g = g_strconcat(command_fmt, "%","s",end, NULL);
        g_free(command_fmt);
        command_fmt=g;
        TRACE("stipped %s\n", command_fmt);
        return command_fmt;
    }
    // What if the path is quoted or escaped or both
    gchar *esc_path = rfm_esc_string (path);
    if (strstr(command_fmt, esc_path)){
        command_fmt = strip_path(command_fmt, esc_path);
    }
    g_free(esc_path);
    return command_fmt;
}

typedef struct execute_t{
    widgets_t *widgets_p;
    GSList *list;
} execute_t;

#endif



#if 0

// See dialog.i
//
// we need history dbh, in term flag dbh, completion on bash match command, and
// should work from a selection list from the iconview.
void 
open_x(widgets_t *widgets_p){
    view_t *view_p = widgets_p->view_p;
    GSList *list=NULL;
    GSList *s_list = view_p->selection_list;
    for (; s_list && s_list->data; s_list = s_list->next){
        record_entry_t *in_en = s_list->data;
        record_entry_t *out_en = rfm_copy_entry(in_en);
        list = g_slist_append(list, out_en); 
    }
    execute(widgets_p, list);
}

static void
time_out_message(widgets_t *widgets_p, const gchar *path){
    rfm_threaded_show_text(widgets_p);
    rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-error",  g_strconcat(path, ": ", NULL));
    rfm_threaded_diagnostics(widgets_p, "xffm_tag/stderr", g_strconcat(_(strerror(ETIMEDOUT)), "\n", NULL));
    rfm_global_t *rfm_global_p = rfm_global();
    rfm_threaded_cursor_reset(rfm_global_p->window);
    return;
}


// This here is the test for non mimetype associated open command:

void
//open_with (widgets_t *widgets_p, record_entry_t * en) {
open_with (const gchar *path) {
    /* open with */
    gchar *command=NULL;
    gchar *command_fmt=NULL;
    TRACE ("open_with()... \n");

    if(!path) {
        return;
    }

    gchar *wd = g_path_get_dirname (en->path);
    if (!g_file_test_with_wait (wd, G_FILE_TEST_EXISTS)){
         time_out_message(widgets_p, wd);
         g_free(wd);
         wd = g_strdup(g_get_home_dir());
     }
        
    g_free (widgets_p->workdir);
    widgets_p->workdir = wd;

    // Here we take special consideration for shell scripts.
    // Shell scripts will be editable files, therefore will
    // have an associated mime_command to open the editor.
    // tests 

    if (!en->mimetype) en->mimetype=MIME_type (en->path, en->st);
    if(!en->mimemagic){
        if (IS_LOCAL_TYPE(en->type) && !en->mimemagic) {
            en->mimemagic = rfm_rational(RFM_MODULE_DIR, "mime", en, "mime_magic", "mime_function");
            if (!en->mimemagic) en->mimemagic = g_strdup(_("unknown"));
        }
        else en->mimemagic = g_strdup(_("unknown"));
    }

    if(!en->filetype) {
        if (IS_LOCAL_TYPE(en->type)) {
            en->filetype = rfm_rational(RFM_MODULE_DIR, "mime", en, "mime_file", "mime_function"); 
            if (!en->filetype) en->filetype = g_strdup(_("unknown"));
        }
        else en->filetype = g_strdup(_("unknown"));
    }

    command_fmt = MIME_command (en->mimetype);
    TRACE ("OPEN: command_fmt(%s) = %s\n", en->mimetype, command_fmt);
    if(!command_fmt) {
        command_fmt = MIME_command (en->mimemagic);
    }

    gboolean is_script= ((en->mimetype && strstr (en->mimetype, "/x-sh")) ||
                         (en->mimemagic && strstr (en->mimemagic, "/x-sh")) ||
                 (en->mimetype && strstr (en->mimetype, "/x-shellscript")) ||   
                 (en->mimemagic && strstr (en->mimemagic, "/x-shellscript")) ||
                         (en->mimetype && strstr (en->mimetype, "/x-csh")) ||   
                         (en->mimemagic && strstr (en->mimemagic, "/x-csh")) ||
                         (en->mimetype && strstr (en->mimetype, "/x-perl")) ||   
                         (en->mimemagic && strstr (en->mimemagic, "/x-perl"))                          );
    if (is_script && !IS_EXE_TYPE(en->type)){
        g_free(command_fmt);
        command_fmt = NULL;
    }

    // for default editor...
    gchar *text_editor = NULL;
    if(!command_fmt) {
        text_editor = rodent_get_text_editor(en);
        TRACE ("OPEN: text_editor = %s\n", text_editor);
        if(text_editor) {
            /* OK to apply an editor */
            command_fmt = g_strconcat(text_editor, " ", NULL);
        }
    }

    //command_fmt=get_command_fmt(en);
    if (is_script) {
        rfm_threaded_show_text(widgets_p);
        if (!IS_EXE_TYPE(en->type)){
            rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-warning", 
                    g_strconcat(en->path, "\n", NULL));
            rfm_threaded_diagnostics(widgets_p, "xffm_tag/stderr",
                    g_strdup_printf(_("Scriptfile \"%s\" is not executable. Please set the executable-attribute on that file.", en->path));
            rfm_threaded_diagnostics(widgets_p, "xffm/stock_dialog-info", NULL);
            gchar *text=g_strdup_printf (_("Open with %s"), _("Text Editor"));
            gchar *base=g_path_get_basename(en->path);
            rfm_threaded_diagnostics(widgets_p, "xffm_tag/green", g_strconcat(text, ": ", base, "\n", NULL));
            g_free(base);
            g_free(text);
        }
    }

    TRACE ("open_with(): magic=%s, mime=%s, command_fmt=%s, editor=%s\n",
            en->mimemagic, en->mimetype, command_fmt, text_editor);
    g_free(text_editor);

    if(command_fmt) {
        command = MIME_mk_command_line (command_fmt, en->path);
        TRACE( "OPEN: command = %s\n", command);

        RFM_THREAD_RUN2ARGV (widgets_p, command, FALSE);
        g_free (command);
        g_free (command_fmt);
    } else {
        open_x(widgets_p);
        //rodent_open_with_activate (NULL, (gpointer) widgets_p);
    }
    return;
}
  


{
    gchar *f = g_build_filename (RUN_DBH_FILE, NULL);
    gchar *ff = g_build_filename (RUN_FLAG_FILE, NULL);
    TRACE (stderr, "RUN_DBH_FILE=%s RUN_FLAG_FILE=%s\n", f, ff);
    
    gchar *title;
    if (selection_list) {
        title=g_strdup_printf(_("Open with %s"),"");
    } else {
        title=g_strdup(_("Execute Shell Command"));
    }
    g = get_response_history (title, 
            files_txt,
            _("Console: quickly run single commands -- write a command here and press enter."),
            f, 
            first_path,
            command_fmt, //NULL, // entry text
            ff,
            _("Run in Terminal"),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            "/usr/bin",
            MATCH_COMMAND); 
        g_free (f);
        g_free (ff);
        TRACE(stderr, "got: \"%s\"\n", g);
    }


#endif

} // namespace xf

#endif



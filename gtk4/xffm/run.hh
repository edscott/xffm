#ifndef XFRUN_HH
#define XFRUN_HH


#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <unistd.h>
#include <gtk/gtk.h>
        

static pthread_mutex_t fork_mutex_=PTHREAD_MUTEX_INITIALIZER;       
static pthread_mutex_t string_hash_mutex=PTHREAD_MUTEX_INITIALIZER;       
static GHashTable *stringHash = NULL;
namespace xf 
{
//template <class Type> 
class Run {

    static void
    push_hash(pid_t controller, gchar *string){
        pthread_mutex_lock(&string_hash_mutex);
        if (!stringHash) {
            stringHash = 
                g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, g_free);      
        }
        g_hash_table_replace(stringHash, GINT_TO_POINTER(controller), string);
        pthread_mutex_unlock(&string_hash_mutex);
    }

    static gchar *
    pop_hash(pid_t controller){
        pthread_mutex_lock(&string_hash_mutex);
        if (!stringHash) {
            pthread_mutex_unlock(&string_hash_mutex);
            return NULL;
        }

        auto string = (gchar *)g_hash_table_lookup (stringHash, GINT_TO_POINTER(controller));
        if (!string){
            //WARN("controller %d not found in hashtable (process has completed)\n", controller);
            string = g_strdup_printf("%d\n", controller);
        } else {
            g_hash_table_steal(stringHash, GINT_TO_POINTER(controller));
        }
        pthread_mutex_unlock(&string_hash_mutex);

        return string;
    }

    static gchar *
    exit_string(gchar *tubo_string){
        gchar *string = NULL;
        if(strchr (tubo_string, '\n')) *strchr (tubo_string, '\n') = 0;
        auto s = strchr (tubo_string, '(');
        int pid = -1;
        long id = 0;
        if (s) {
            s++;
            if(strchr (s, ')')) *strchr (s, ')') = 0;
            errno = 0;
            id = strtol(s, NULL, 10);
            if (!errno){
                pid = Tubo::getChild((pid_t) id);
            }
        }
        gchar *c_string = pop_hash((pid_t)pid);
        if (!c_string) return g_strdup("\n");
        g_strstrip(c_string);
        string = g_strconcat(c_string, "\n", NULL);
        g_free(c_string);
        return string;
    }


public:
    static pid_t 
    thread_runReap(gpointer data, const gchar **arguments, 
            void (*stdout_f)(void *, void *, int),
            void (*stderr_f)(void *, void *, int),
            void (*finish_f)(void *)){
        return thread_run(TUBO_EXIT_TEXT|TUBO_VALID_ANSI|TUBO_CONTROLLER_PID|TUBO_REAP_CHILD,
                data, arguments,stdout_f, stderr_f, finish_f);
    }
    
    static pid_t 
    thread_run(gpointer data, const gchar **arguments, 
            void (*stdout_f)(void *, void *, int),
            void (*stderr_f)(void *, void *, int),
            void (*finish_f)(void *)){
        return thread_run(TUBO_EXIT_TEXT|TUBO_VALID_ANSI|TUBO_CONTROLLER_PID,
                data, arguments,stdout_f, stderr_f, finish_f);
    }

private:
    static pid_t 
    thread_run(gint flags, gpointer data, const gchar **arguments, 
            void (*stdout_f)(void *, void *, int),
            void (*stderr_f)(void *, void *, int),
            void (*finish_f)(void *)){
        auto command = g_strdup("");
        auto p = arguments;
        for (;p && *p; p++){
            auto g =  g_strdup_printf("%s %s", command, *p);
            g_free(command);
            command = g;
        }
        TRACE("thread_run command = %s\n", command);

        pid_t pid = Tubo::Fork (fork_function,(gchar **)arguments,
                                    NULL, // stdin
                                    stdout_f,
                                    stderr_f,
                                    finish_f,
                                    data, // XXX view_v,
                                    flags);
        if (pid < 0) {
            g_free(command);
            return 0;
        }
        pid_t grandchild=Tubo::getChild (pid);
        // Reference to command now belongs to the hashtable.
        g_strstrip(command);
        push_hash(grandchild, g_strdup(command));
        TRACE("push hash: \"%s\"\n", command);
        g_free(command);
        return pid;
    }
public:
    static bool
    run_in_shell(const gchar *command){
        const gchar *special = "\'*?<>|&";
        if (strchr(command, '`')) return TRUE;
        if (strchr(command, '?')) return TRUE;
        if (strchr(command, '*')) return TRUE;
        if (strchr(command, '<')) return TRUE; 
        if (strchr(command, '>')) return TRUE; 
        if (strchr(command, '|')) return TRUE; 
        if (strchr(command, '&')) return TRUE; 
        if (strchr(command, '\'')) return TRUE;
        if (strchr(command, '"')) return TRUE;
        // Are we defining an environment variable or something else?
        gint count;
        gchar **g;
        bool retval = FALSE;
        if (!g_shell_parse_argv (command, &count, &g, NULL)) return TRUE;
        if (!g || !g[0]) {
            retval = TRUE;
        }
        else {
            auto p = g_find_program_in_path(g[0]);
            if (!p) retval = TRUE;
            g_free(p);
        }
        g_strfreev(g);
        return retval;
    }
    
    static pid_t 
    thread_run(GtkTextView *textview, const gchar **arguments, gboolean scrollUp){
        auto command = g_strdup("");
        auto p = arguments;
        for (;p && *p; p++){
            auto g =  g_strdup_printf("%s %s", command, *p);
            g_free(command);
            command = g;
        }
        int flags = TUBO_EXIT_TEXT|TUBO_VALID_ANSI|TUBO_CONTROLLER_PID;

        pid_t pid = Tubo::Fork (fork_function,(gchar **)arguments,
                                    NULL, // stdin
                                    run_operate_stdout, //stdout_f,
                                    run_operate_stderr, //stderr_f
                                    (scrollUp)?scrollToTop:fork_finished_function,
                                    textview, // XXX view_v,
                                    flags);
        pid_t grandchild=Tubo::getChild (pid);
        if (textview) Util::printInfo(textview, "emblem-greenball", g_strdup_printf("%d:%s\n", grandchild, command));
        g_strstrip(command);
        push_hash(grandchild, g_strdup(command));
        TRACE("push hash: \"%s\"\n", command);
        g_free(command);
        return pid;
    }

    static pid_t thread_run(GtkTextView *textview, const gchar *command, gboolean scrollUp){
        GError *error = NULL;
        gint argc;
        gchar **argv;


        gchar *ncommand;
        if (run_in_shell(command)){
            ncommand = g_strdup_printf("%s -c \"%s\"", Util::u_shell(), command);
        } else {
            ncommand = g_strdup(command);
        }
        if(!g_shell_parse_argv (ncommand, &argc, &argv, &error)) {
            auto msg = g_strcompress (error->message);
            if (textview) Util::printError(textview, g_strdup_printf("%s: %s\n", msg, ncommand));
            else TRACE("%s: %s\n", msg, ncommand);
            g_free(ncommand);
            g_error_free (error);
            g_free (msg);
            return 0;
        }
        g_free(ncommand);
        auto full_path = g_find_program_in_path(argv[0]);
        if (full_path){
            g_free(argv[0]);
            argv[0] = full_path;
        }
        pid_t pid = thread_run(textview, (const gchar **)argv, scrollUp);
        
        g_strfreev(argv);
        return pid;
    }

    static void *thread_f(void *data){
        return NULL;
    }
    static void *wait_f(void *data){
        return NULL;
    }

    static void
    run_operate_stdout (void *data, void *stream, int childFD) {
        GtkTextView *textview;
        if (!data){
          DBG("run_operate_stdout: invalid argument\n");
          //  textview = Fm<Type>::getCurrentTextview();
        } else {
           if (!Util::isValidTextView(data)) return;
           textview = GTK_TEXT_VIEW(data);
        }
        if (!gtk_widget_is_visible(GTK_WIDGET(textview))) return;
        /*
        view_c *view_p = (view_c *)data;
        window_c *window_p = (window_c *)(view_p->get_window_v());
        if (!window_p->is_view_in_list(data)) return;
        */
        char *line;
        line = (char *)stream;
        if(line[0] == '\n') return;
        //TRACE("* %s", line);
        const gchar *exit_token = "Tubo-id exit:";
        gchar *recur = NULL;
        if (strstr(line, exit_token) && strstr(line, exit_token) != line){
            recur = g_strdup(strstr(line, exit_token));
            strcpy(strstr(line, exit_token), "\n");
        }

        //int bell=0x07; // bell
        int bs=0x08;   // backspace
        //int ht=0x09;   // horizontal tab
        //int lf=0x0a;   // linefeed
        //int vt=0x0b;   // vertical tab
        //int ff=0x0c;   // formfeed
        //int cr=0x0d;   // carriage return

        // apply all ^H (bs) found (as in rar output)
        int i, j;
        gchar *outline = g_strdup (line);

        for(i = 0, j = 0; line[i]; i++) {
            if(line[i] == bs && j > 0) j--;
            else {
                outline[j] = line[i];
                j++;
            }
        }
        outline[j] = 0;

        if(strncmp (line, exit_token, strlen (exit_token)) == 0) {
            gchar *string = exit_string(line);
            Util::printInfo(textview, "emblem-redball", g_strdup_printf("%s", string));
            g_free(string);
        } else {
            Util::print(textview, g_strdup(outline));
        }
        g_free(outline);


        // With this, this thread will not do a DOS attack
        // on the gtk event loop.
        static gint count = 1;
        if (count % 20 == 0){
            usleep(10000);
        } 
        if (recur) {
            run_operate_stdout (data, recur, childFD);
            g_free(recur);
        }
        return;
    }
    static void
    run_operate_stderr (void *data, void *stream, int childFD) {
        GtkTextView *textview;
        if (!data){
          DBG("run_operate_stderr: invalid argument\n");
           // textview = Fm<Type>::getCurrentTextview();
            
        } else {
           if (!Util::isValidTextView(data)) return;
           textview = GTK_TEXT_VIEW(data);
        }  

        //Util::show_text(textview);      
        
        char *line;
        line = (char *)stream;
        if(line[0] != '\n') {
            if (strstr(line, "error")||strstr(line,_("error"))) {
                Util::print(textview, "Cyan", g_strdup(line));
            } else if (strstr(line, "***")) {
                Util::print(textview, "red/white_bg", g_strdup(line));
            } else if (strstr(line, "warning")||strstr(line, _("warning"))) {
                Util::print(textview, "yellow", g_strdup(line));
            } else {                
                Util::printStdErr(textview, g_strdup(line));
            }
        }

        // With this, this thread will not do a DOS attack
        // on the gtk event loop.
        static gint count = 1;
        if (count % 20 == 0){
            usleep(10000);
        } 
       return;
    }

    static void
    operate_stdout (void *data, void *stream, gint childFD) {
        auto textview = GTK_TEXT_VIEW(data);
        auto line = (gchar *)stream;

        Util::print(textview, g_strdup(line));
        // This is a bit hacky, to keep runaway output from hogging
        // up the gtk event loop.
        static gint count = 1;
        if (count % 20 == 0){
            usleep(100000);
        } else {
            usleep(1000);
        }
        // reap child here.
       return;
    }
    static void
    operate_stderr (void *data, void *stream, gint childFD) {
        auto textview = GTK_TEXT_VIEW(data);
        auto line = (gchar *)stream;

        Util::printStdErr(textview, g_strdup(line));
        // This is a bit hacky, to keep runaway output from hogging
        // up the gtk event loop.
        static gint count = 1;
        if (count % 20 == 0){
            usleep(100000);
        } else {
            usleep(1000);
        }
       return;
    }

    static gboolean scrollToTop_f(void *data) {
        //view_c *view_p = (view_c *)data;
        //view_p->get_lpterm_p()->print("bold", g_strdup_printf("%s\n", "run complete."));
        auto textview = GTK_TEXT_VIEW(data);
        Util::showText(textview);
        Util::scroll_to_top(textview);
        return FALSE;
    }

    static void
    scrollToTop (void *data) {
        g_timeout_add(5, scrollToTop_f, data);                                                
    }

    static gboolean done_f(void *data) {
        //view_c *view_p = (view_c *)data;
        //view_p->get_lpterm_p()->print("bold", g_strdup_printf("%s\n", "run complete."));
        return FALSE;
    }

    static void
    fork_finished_function (void *data) {
        g_timeout_add(5, done_f, data);                                                
    }

    static void
    threadwait (void) {
        struct timespec thread_wait = {
            0, 100000000
        };
        nanosleep (&thread_wait, NULL);
    }

#define MAX_COMMAND_ARGS 2048


    static void
    fork_function (void *data) {
        auto argv = (gchar **)data;

        gint i = 0;
        pthread_mutex_lock(&fork_mutex_);
        gchar *passwordCommand = g_path_get_basename(argv[0]);
        if (strchr(passwordCommand, ' ')) *strchr(passwordCommand, ' ')=0;

        if (i>=MAX_COMMAND_ARGS - 1) {
            TRACE("run.hh::%s: (> %d)\n", strerror(E2BIG), MAX_COMMAND_ARGS);
            argv[MAX_COMMAND_ARGS - 1]=NULL;
        }
        setenv("RFM_ASKPASS_COMMAND", passwordCommand, 1);
        g_free(passwordCommand);

        pthread_mutex_unlock(&fork_mutex_);
        execvp (argv[0], argv);
        g_warning ("CHILD could not execvp: this should not happen\n");
        g_warning ("Do you have %s in your path?\n", argv[0]);
        threadwait ();
        _exit (123);
    }

    static gchar *
    sudo_fix(const gchar *command){
        if (!strstr(command, "sudo ")) return NULL; 
        gchar *new_command = NULL;
        if (strncmp(strstr(command, "sudo "), "sudo -A ", strlen("sudo -A "))!=0)
        {
            auto original_head=g_strdup(command);
            auto pos = strstr(original_head, "sudo ");
            if (pos){
                *pos = 0;
                auto tail=g_strdup(strstr(command, "sudo ")+strlen("sudo "));
                new_command = g_strconcat(original_head, "sudo -A -p \\\"",_("Enter password"), ": \\\" ", tail, NULL);
                g_free(tail);
            }
            g_free(original_head);
        }
        return new_command;
    }

    static pid_t 
    shell_command(GtkTextView *textview, const gchar *c, gboolean scrollUp, gboolean showTextPane){
        // Make sure any sudo command has the "-A" option
        auto command = sudo_fix(c);
        TRACE("shell_command = %s\n", c);
        if (showTextPane) Util::showText(textview);
        auto currentDir = g_get_current_dir();
        auto child = GTK_WIDGET(g_object_get_data(G_OBJECT(textview), "child"));
        auto wd = Child::getWorkdir(child);
#ifdef WINDOWS_COMPILE
        g_chdir(wd);
#else
        chdir(wd);
#endif
        pid_t pid = thread_run(textview, command?command:c, scrollUp);
        g_free (command);
#ifdef WINDOWS_COMPILE
        g_chdir(currentDir);
#else
        chdir(currentDir);
#endif

        g_free(currentDir);
        if (!pid) return 0;
        return pid;
    }

    static gboolean
    runInTerminal(const gchar *commandFmt){
        if (fixedInTerminal(commandFmt)) return TRUE;
        gchar *a = baseCommand(commandFmt);
        gboolean retval = FALSE;
        if (Settings::keyFileHasGroupKey("Terminal",  a) &&
                Settings::getInteger("Terminal", a))retval = TRUE;
        g_free(a);
        return retval;
    }
 
    static gchar *
    baseIcon(const gchar *iconFmt){
        if (!iconFmt) return NULL;
        gchar *a = g_strdup(iconFmt);
        g_strstrip(a);
        if (strchr(a, ' ')) *(strchr(a, ' ')) = 0;
        gchar *g = g_path_get_basename(a);
        g_free(a);
        a=g;
        return a;
    }
   
    static gchar *
    baseCommand(const gchar *commandFmt){
        if (!commandFmt) return NULL;
        gchar *a = g_strdup(commandFmt);
        g_strstrip(a);
        if (strchr(a, ' ')) *(strchr(a, ' ')) = 0;
        return a;
    }
    static gboolean
    fixedInTerminal(const gchar *app){
        gchar *a = baseCommand(app);
        gchar *b = strrchr(a, G_DIR_SEPARATOR);
        if (!b) b=a; else b++;
        gchar const *exceptions[] = {"vi", "vim", "vimdiff", "vimtutor", "nano", NULL};
        gchar const **q;
        gboolean retval = FALSE;
        for (q=exceptions; q && *q; q++){
            if (strcmp(a, *q) == 0){
                retval=TRUE;
                break;
            }
        }
        g_free(a);
        return retval;
    }
    
    static gchar *
    mkCommandLine (const gchar *command_fmt, const gchar *path) {
        TRACE("mime_mk_command_line()...\n");

        TRACE ("MIME: mime_mk_command_line(%s)\n", path);
        gchar *command_line = NULL;
        gchar *fmt = NULL;

        if(!command_fmt)
            return NULL;
        if(!path)
            path = "";

        TRACE ("MIME: command_fmt=%s\n", command_fmt);

        /* this is to send path as an argument */

        if(strstr (command_fmt, "%s")) {
            fmt = g_strdup (command_fmt);
        } else {
            fmt = g_strconcat (command_fmt, " %s", NULL);
        }
        TRACE ("MIME: command_fmt fmt=%s\n", fmt);

        TRACE ("MIME: path=%s\n", path);
        gchar *esc_path = Util::esc_string (path);
        command_line = g_strdup_printf (fmt, esc_path);
        g_free (esc_path);
        TRACE ("MIME2: command_line=%s\n", command_line);

        g_free (fmt);
        return command_line;
    }
 
    static gchar *
    mkTerminalLine (const gchar *command, const gchar *path) {
        TRACE("mime_mk_terminal_line()...\n");
        TRACE ("MIME: mime_mk_command_line(%s)\n", command);
        gchar *command_line = NULL;

        if(!command) return NULL;
        gchar *a = mkCommandLine(command, path);

        auto term = Util::getTerminalCmd();
        command_line = g_strdup_printf ("%s %s", term, a);
        g_free(a);
        return command_line;
    }
#if 0
    static gboolean isValidCommand (const char *cmd_fmt) {
        //return GINT_TO_POINTER(TRUE);
        TRACE ("MIME: mime_is_valid_command(%s)\n", cmd_fmt);
        GError *error = NULL;
        int argc;
        gchar *path;
        gchar **argv;
        if(!cmd_fmt)
            return  (FALSE);
        if(!g_shell_parse_argv (cmd_fmt, &argc, &argv, &error)) {
            gchar *msg = g_strcompress (error->message);
            ERROR ("%s: %s\n", msg, cmd_fmt);
            g_error_free (error);
            g_free (msg);
            return  (FALSE);
        }
        gchar **ap = argv;
        if (*ap==NULL) {
            errno = ENOENT;
            return  (FALSE);
        }

        // assume command is correct if environment is being set
        if (strchr(*ap, '=')){
            g_strfreev (argv);
            return  (TRUE);
        }

        path = g_find_program_in_path (*ap);
        if(!path) {
            gboolean direct_path = g_file_test (argv[0], G_FILE_TEST_EXISTS) ||
                strncmp (argv[0], "./", strlen ("./")) == 0 || strncmp (argv[0], "../", strlen ("../")) == 0;
            TRACE("argv[0]=%s\n",argv[0]);
            if(direct_path) {
                path = g_strdup (argv[0]);
            }
        }
        TRACE ("mime_is_valid_command(): g_find_program_in_path(%s)=%s\n", argv[0], path);

        //if (!path || access(path, X_OK) != 0) {
        if(!path) {
            g_strfreev (argv);
            errno = ENOENT;
            return  (FALSE);
        }
        // here we test for execution within sudo
        // XXX we could also check for commands executed in a terminal, but not today...
        gboolean retval=(TRUE);
        if (strcmp(argv[0],"sudo")==0) {
            int i=1;
            if (strcmp(argv[i],"-A")==0) i++;
            retval=isValidCommand(argv[i]);
        }

        g_strfreev (argv);
        g_free (path);
        return retval;
    }

    
    static gchar *getRunCommand(GtkWindow *parent, const gchar *path){
        TRACE("runWith: path = %s\n", path);
        auto displayPath = Util::valid_utf_pathstring(path);
        auto markup = 
            g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>\n<span color=\"red\">(%s)</span>", displayPath, 
                _("Executable"));  
        g_free(displayPath);
        
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(parent), _("Run Executable..."), "system-run");
        entryResponse->setResponseLabel(markup);
        g_free(markup);

        entryResponse->setCheckButton(_("Run in Terminal"));
        entryResponse->setCheckButton(Run<Type>::runInTerminal(path));

        entryResponse->setEntryLabel(_("Arguments for the Command"));
        // get last used arguments...
        entryResponse->setEntryDefault("");
        
        entryResponse->setCheckButtonEntryCallback((void *)toggleTerminalRun, (void *)path); 
        auto response = entryResponse->runResponse();


        if (!response) return NULL;
        // Is the terminal flag set?
        gchar *command ;
        if (runInTerminal(path)){
            command = mkTerminalLine(path, response);
        } else {
            command = mkCommandLine(path, response);
        }
        g_free(response);
        return command;
    }

    static gchar *getOpenWithCommand(GtkWindow *parent, GList *pathList, const gchar *wd){
        if (!pathList || g_list_length(pathList)==0) return NULL;
        gboolean multiple = (g_list_length(pathList) > 1);
        const gchar *path ;
        const gchar *pathExt ;
        gchar *mimetype;
        const gchar **apps;
        gchar *defaultApp;
        gchar *textApp = NULL;

       
        for (auto l= pathList; l && l->data; l=l->next){
            path  = (const gchar *)l->data;
            auto fileInfo = Util::fileInfo(path);        
            textApp = defaultTextApp(fileInfo);
            if (textApp && strlen(textApp)) break;
        }

        for (auto l= pathList; l && l->data; l=l->next){
            pathExt  = (const gchar *)l->data;
            defaultApp = defaultExtApp(pathExt);
            if (defaultApp && strlen(defaultApp)) break;
        }

        for (auto l= pathList; l && l->data; l=l->next){
            path  = (const gchar *)l->data;
            mimetype = Mime<Type>::mimeType(path);
            apps = MimeApplication<Type>::locate_apps(mimetype);
            if (apps) break;
        }
        

        gchar *mpath = getMpath(pathList);
        gchar *responseLabel = getResponseLabel(mpath, multiple?NULL:mimetype);
        gchar *response = NULL;
        gchar *command = NULL;

        auto appCount = 0;
        if (apps && apps[0]) {
            appCount++;
            if (apps[1]) appCount++;
        }
        if (defaultApp) appCount++;
        if (textApp) appCount++;
        TRACE("appcount=%d\n",appCount);

        if (appCount <= 1) {
            auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(parent), _("Open with"), RUN);
            entryResponse->setResponseLabel(responseLabel);

            entryResponse->setCheckButton(_("Run in Terminal"));
            if (!multiple) entryResponse->setCheckButton(defaultApp && Run<Type>::runInTerminal(defaultApp));

            entryResponse->setEntryLabel(_("Open with"));
            if (apps && apps[0]) entryResponse->setEntryDefault(apps[0]);
            if (textApp) entryResponse->setEntryDefault(textApp);
            if (defaultApp) entryResponse->setEntryDefault(defaultApp);
            entryResponse->setEntryBashCompletion(wd);
            
            entryResponse->setCheckButtonEntryCallback((void *)toggleTerminal); 
            entryResponse->setEntryCallback((void *)entryKeyRelease); 
            response = entryResponse->runResponse();
        } else {
            auto comboResponse = new(ComboResponse<Type>)(GTK_WINDOW(parent), _("Open with"), RUN);
            comboResponse->setResponseLabel(responseLabel);

            comboResponse->setCheckButton(_("Run in Terminal"));
            if (!multiple) comboResponse->setCheckButton(defaultApp && Run<Type>::runInTerminal(defaultApp));

            comboResponse->setComboLabel(_("Open with"));
            if (apps && apps[0]) comboResponse->setComboOptions(apps);
            if (textApp) comboResponse->setComboDefault(textApp);
            if (defaultApp) comboResponse->setComboDefault(defaultApp);
            comboResponse->setComboBashCompletion(wd);

            comboResponse->setCheckButtonComboCallback((void *)toggleTerminal); 
            comboResponse->setComboCallback((void *)comboChanged); 
        
            response = comboResponse->runResponse();
        }

        if (not response) goto done;
        if (strrchr(response,'\n')) *(strrchr(response,'\n')) = 0;
        if (strlen(response)==0) goto done;

        // Check whether application is valid.
        if (!Run<Type>::isValidCommand(response)){
            gchar *message = g_strdup_printf("\n<span color=\"#990000\"><b>%s</b></span>:\n <b>%s</b>\n", _("Invalid entry"), response); 
            Dialogs<Type>::quickHelp (GTK_WINDOW(parent), message);
            g_free(message);
            goto done;
        }


        // save value as default for mimetype extension
        setMimetypeDefault(path, mimetype, response);
        TRACE("setting mimetype default to: %s\n", response);
        MimeApplication<Type>::add2ApplicationHash(mimetype, response, TRUE);

        // get command line
        if (!multiple) {
        // Is the terminal flag set?
            if (runInTerminal(response)){
                command = mkTerminalLine(response, mpath);
            } else {
                command = mkCommandLine(response, mpath);
            }
        } else { 
               if (runInTerminal(response)){
                command = mkTerminalLine(response, "");
            } else {
                command = mkCommandLine(response, "");
            }
            auto g = g_strconcat(command, " ", mpath, NULL);
            g_free(command);
            command = g;
        }
done:
        g_free(mimetype);
         g_free(defaultApp);
        g_free(mpath);
        g_free(responseLabel);
        g_free(response);

        return command;
   }


    static gchar *
    defaultExtApp(const gchar *path){
        auto ext = strrchr(path, '.');
        if (!ext || strlen(ext)<2) return NULL;
        gchar *defaultApp = Settings::getString("MimeTypeApplications", ext+1);
        TRACE("*** defaultExtApp (%s) --> %s --> %s\n", path, ext+1, defaultApp);
        return defaultApp;
    }

    static gchar *
    defaultTextApp(const gchar *fileInfo){
        gchar *defaultApp = NULL;
        gboolean textFiletype =(fileInfo && 
                (strstr(fileInfo, "text")||strstr(fileInfo,"empty")));
        if (textFiletype) {
            auto editor = Util::getEditor();
            defaultApp =g_strdup_printf("%s %%s", editor);
        }
        return defaultApp;
    }

    static gchar *
    defaultMimeTypeApp(const gchar *mimetype){
        gchar *defaultApp = Settings::getString("MimeTypeApplications", mimetype);
        if (!defaultApp) {
            const gchar **apps = MimeApplication<Type>::locate_apps(mimetype);
            if (apps && *apps) defaultApp = g_strdup(*apps);
        }

        if (!defaultApp)  {
            gboolean textMimetype = (mimetype && strncmp(mimetype, "text/", strlen("text/")) == 0);
            if (textMimetype) {
                auto editor = Util::getEditor();
                defaultApp =g_strdup_printf("%s %%s", editor);
            }
        }
        return defaultApp;
    }
private:

    static void 
    setMimetypeDefault(const gchar *path, const gchar *mimetype, const gchar *response){
        
        if (strchr(path, '.') && strlen(strchr(path, '.'))>1){
            auto ext = strrchr(path,'.') + 1; 
            Settings::setString("MimeTypeApplications", ext, response);
            TRACE("*** saving %s --> response\n", ext, response);
        } else {
            Settings::setString("MimeTypeApplications", mimetype, response);
            TRACE("*** saving %s --> response\n", mimetype, response);
        }
    }

    static gchar *
    getMpath(GList *pathList){
        gboolean multiple = (g_list_length(pathList) > 1);
        auto path =(const gchar *)pathList->data;
        gchar *mpath = multiple?g_strdup(""):g_strdup(path);
        if (multiple) {
            for(auto l = pathList; l && l->data; l = l->next) {
                auto path  = (const gchar *)l->data;
                TRACE("multiple path: %s\n", path);
                auto g = g_strconcat(mpath, " ", path, NULL);
                g_free(mpath);
                mpath = g;
            }
            TRACE("composite path: %s\n", mpath);
        } 
        return mpath;
    }
    static gchar *
    getResponseLabel(const gchar *mpath, const gchar *mimetype){
        return g_strdup_printf("<b><span size=\"larger\" color=\"blue\">%s</span></b>\n<span color=\"#880000\">(%s)</span>", 
                !mimetype?"":mpath, 
                !mimetype?_("You have selected multiple files or folders"):mimetype);
    }
    
    static void 
    toggleTerminalRun (GtkToggleButton *togglebutton, gpointer data){
        if (!data) {
            ERROR("toggleTerminalRun: data not set to path\n");
            return;
        }
        auto path = (const gchar *)data;
        TRACE("runPath = %s\n", path);
        gint value;
        if (gtk_toggle_button_get_active(togglebutton)) value = 1; else value = 0;
        gchar *a = Run<Type>::baseCommand(path);
        Settings::setInteger("Terminal", a, value);
        g_free(a);
    }

    static void 
    toggleTerminal (GtkToggleButton *togglebutton, gpointer data){
        if (!data) return;
        const gchar *app = gtk_entry_get_text(GTK_ENTRY(data));
        // Hard coded exceptions:
        if (Run<Type>::fixedInTerminal(app)) {
            gtk_toggle_button_set_active(togglebutton, TRUE);
            return;
        }
        
        // if not valid command, do nothing 
        if (!Run<Type>::isValidCommand(app)) return;
        // Valid command, continue. Get basename 
        gint value;
        if (gtk_toggle_button_get_active(togglebutton)) value = 1; else value = 0;
        gchar *a = Run<Type>::baseCommand(app);
        Settings::setInteger("Terminal", a, value);
        g_free(a);
    }
   
    static void
    comboChanged (GtkComboBox *combo, gpointer data){
        auto comboResponse = (ComboResponse<Type> *)data;
        auto checkButton = GTK_TOGGLE_BUTTON(comboResponse->checkButton());
        auto entry = comboResponse->comboEntry();
        const gchar *text = gtk_entry_get_text(entry);
        gtk_toggle_button_set_active(checkButton, Run<Type>::runInTerminal(text));
    }

    static void
    entryKeyRelease (GtkWidget *widget, GdkEvent  *event, gpointer data){
        auto entryResponse = (EntryResponse<Type> *)data;
        auto checkButton = GTK_TOGGLE_BUTTON(entryResponse->checkButton());
        auto entry = GTK_ENTRY(widget);
        const gchar *text = gtk_entry_get_text(entry);
        gtk_toggle_button_set_active(checkButton, Run<Type>::runInTerminal(text));
    }
#endif  


};
}

#endif

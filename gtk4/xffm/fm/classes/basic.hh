#ifndef BASIC_HH
#define BASIC_HH
namespace xf {
  GList *dialogStack = NULL;
  class Basic {
    public:
      static int getMaxNameLen(GListModel *store){
        int max = 0;
        auto items = g_list_model_get_n_items (store);
        for (guint i=0; i<items; i++){
          auto info = G_FILE_INFO(g_list_model_get_item(store, i)); // GFileInfo
          auto name = g_file_info_get_name(info);
          if (strlen(name) > max) max = strlen(name);
        }
        return max;
      }

      static void pushDialog(GtkWindow *dialog){
        dialogStack = g_list_prepend(dialogStack, dialog);
      }
      static void popDialog(GtkWindow *dialog){
        dialogStack = g_list_remove(dialogStack, dialog);
      }
      static GtkWindow *topDialog(void){
        if (!dialogStack) return NULL;
        return GTK_WINDOW(g_list_first(dialogStack)->data);
      }

      static GFile *getGfile(GFileInfo *info){
        return G_FILE(g_file_info_get_attribute_object(info, "standard::file"));
      }

      static void freeSelectionList(GList *selectionList){
        if (!selectionList) return;
        for (auto l=selectionList; l && l->data; l= l->next){
          auto info = G_OBJECT(l->data);
          g_object_unref(info);
        }
        g_list_free(selectionList);
      }
    private:
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
     
public:
    static GList *getChildren(GtkWidget *widget){
      GList *list = NULL;
      auto child = gtk_widget_get_first_child(widget);
      if (child) {
        list = g_list_append(list, child);
        GtkWidget *next=NULL;
        while ((next = gtk_widget_get_next_sibling(child)) != NULL){
          child = next;
          list = g_list_append(list, child);
        }
      }
      return list;
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

    static char *getPath(GFileInfo *info){
        auto file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
        return g_file_get_path(file);
    }
      
    static void setDialog(GtkWindow *dialog){
    }
    static void unsetDialog(GtkWindow *dialog){
    }

    static void *hide_f(void *data){
      auto w = GTK_WIDGET(data);
      gtk_widget_set_visible(w, false);
      return NULL;
    }
    static void contextHide(GtkWidget *w){
      context_function(hide_f, (void *)w);
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

    static const gchar *getEditor(){
        setEditor();
        return getenv("EDITOR");        
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

    static void *destroy_f(void *window){
      gtk_window_destroy(GTK_WINDOW(window));
      return NULL;
    }

    static void *present_f(void *window){
      if (GTK_IS_WINDOW(window)) gtk_window_present(GTK_WINDOW(window));
      return NULL;
    }
public:
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
    static const gchar *getTerminal(){
        setTerminal();
        return  getenv("TERMINAL");
    }

    static const gchar *getTerminalCmd(){
        setTerminal();
        return  getenv("TERMINAL_CMD");
    }

    static void sensitive(GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data){ gtk_widget_set_sensitive(GTK_WIDGET(data), true);}
    static void insensitive(GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data){ gtk_widget_set_sensitive(GTK_WIDGET(data), false);}

    static void present(GtkWindow *window){
      context_function(present_f, (void *)window);
    }

    static void destroy(GtkWindow *window){
      context_function(destroy_f, (void *)window);
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

    static void
    concat(gchar **fullString, const gchar* addOn){
        if (!(*fullString)) {
          *fullString = g_strdup(addOn);
        }
        auto newString = g_strconcat(*fullString, addOn, NULL);
        g_free(*fullString);
        *fullString = newString;
    }
    static GtkBox *
    mkEndBox(){ // not a hack, according to gnome people...
      auto endBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_widget_set_hexpand(GTK_WIDGET(endBox), false);
      auto spacer = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_widget_set_hexpand(GTK_WIDGET(spacer), true);
      boxPack0(endBox,GTK_WIDGET(spacer), TRUE, TRUE, 0);
      return endBox;
    }
    static 
    void boxPack0(  
        GtkBox* box,
        GtkWidget* child,
        gboolean expand,
        gboolean fill,
        guint padding)
    {
      GtkOrientation orientation = gtk_orientable_get_orientation(GTK_ORIENTABLE(box));
      gtk_box_append(box, child);
      gtk_widget_set_halign (GTK_WIDGET(child),fill?GTK_ALIGN_FILL: GTK_ALIGN_START);
      // other options: GTK_ALIGN_START, GTK_ALIGN_END, GTK_ALIGN_BASELINE
      if (orientation == GTK_ORIENTATION_HORIZONTAL){
        gtk_widget_set_hexpand(GTK_WIDGET(child), expand);
        gtk_widget_set_margin_start(GTK_WIDGET(child), padding);
        gtk_widget_set_margin_end(GTK_WIDGET(child), padding);
      } else if (orientation == GTK_ORIENTATION_VERTICAL){
        gtk_widget_set_vexpand(GTK_WIDGET(child), expand);
        gtk_widget_set_margin_top(GTK_WIDGET(child), padding);
        gtk_widget_set_margin_bottom(GTK_WIDGET(child), padding);
      } else {
        fprintf(stderr, "boxPack0(): programming error. Exit(2)\n");
        exit(2);
      }
    }
    static void
    flushGTK(void){
      // This may introduce race conditions
      //while (g_main_context_pending(NULL)) g_main_context_iteration(NULL, TRUE);
    }
    static gchar *
    utf_string (const gchar * t) {
      if(!t) { return g_strdup (""); }
      if(g_utf8_validate (t, -1, NULL)) {
        return g_strdup (t); 
      }
      /* so we got a non-UTF-8 */
      /* but is it a valid locale string? */
      gchar *actual_tag;
      actual_tag = g_locale_to_utf8 (t, -1, NULL, NULL, NULL);
      if(actual_tag){
          return actual_tag;
      }
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
    
    static void *context_function(void * (*function)(gpointer), void * function_data){
      pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
      pthread_cond_t signal = PTHREAD_COND_INITIALIZER; 
      auto result=GINT_TO_POINTER(-1);
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
          g_main_context_invoke(NULL, CONTEXT_CALLBACK(context_function_f), arg);
          pthread_mutex_lock(&mutex);
          if (result == GINT_TO_POINTER(-1)) pthread_cond_wait(&signal, &mutex);
          pthread_mutex_unlock(&mutex);
      }
      pthread_mutex_destroy(&mutex);
      pthread_cond_destroy(&signal);
      return result;
    }
    
    static GtkButton *mkButton(const char *iconName, const char *markup){
      auto button = gtk_button_new();
      auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      if (iconName){
        auto image = gtk_image_new_from_icon_name(iconName);
        gtk_box_append (box, GTK_WIDGET(image));
      }
      if (markup){
        auto label = gtk_label_new("");
        auto g = g_strdup_printf("  %s", markup);
        gtk_label_set_markup(GTK_LABEL(label), g);
        g_free(g);
        gtk_box_append (box, GTK_WIDGET(label));
      }
      gtk_button_set_child(GTK_BUTTON(button), GTK_WIDGET(box));
      return GTK_BUTTON(button);
    }
    
    static void setAsDialog(GtkWidget *widget, const char *Xname, const char *Xclass){
        bool OK = false;
 #ifdef GDK_WINDOWING_X11
        GdkDisplay *displayGdk = gdk_display_get_default();
        if (GDK_IS_X11_DISPLAY (displayGdk)) {
          OK = true;
          Display *display = gdk_x11_display_get_xdisplay(displayGdk);
          XClassHint *wm_class = (XClassHint *)calloc(1, sizeof(XClassHint));
          wm_class->res_name = g_strdup(Xname);
          wm_class->res_class = g_strdup(Xclass);

          GtkNative *native = gtk_widget_get_native(widget);
          GdkSurface *surface = gtk_native_get_surface(native);
          Window w = gdk_x11_surface_get_xid (surface);
          XSetClassHint(display, w, wm_class);

          Atom atom = gdk_x11_get_xatom_by_name_for_display (displayGdk, "_NET_WM_WINDOW_TYPE_DIALOG");
          Atom atom0 = gdk_x11_get_xatom_by_name_for_display (displayGdk, "_NET_WM_WINDOW_TYPE");
          XChangeProperty (display, w,
            atom0, XA_ATOM, 
            32, PropModeReplace,
            (guchar *)&atom, 1);
        }
#endif
#ifdef GDK_WINDOWING_WAYLAND
//#warning "Compiling for Wayland (unstable)"
        OK = true;
#endif        
#ifdef GDK_WINDOWING_WIN32
#warning "Compiling for Windows (unstable)"
        OK = true;
#endif
        if (!OK) {
          g_error ("Unsupported GDK backend");
          exit(1);
        }
   }
    static bool
    alwaysTerminal(const char *command){
      char *key = g_strdup(command);
      if (strchr(key, ' ')) *strrchr(key, ' ') = 0;
      const char *always[] = {"vi", "vim", "nano", "emacs", NULL};
      for (auto p=always; p && *p; p++){
        if (strcmp(key, *p) == 0) {
          g_free(key);
          return true;
        }
      }
      g_free(key);
      return false;
    }
    static GList *getChildren(GtkBox *box){
      GList *list = NULL;
      GtkWidget *w = gtk_widget_get_first_child(GTK_WIDGET(box));
      if (w) {
        list = g_list_append(list, w);
        while ((w=gtk_widget_get_next_sibling(w)) != NULL) list = g_list_append(list, w);
      }
      return list;
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
        auto date = dateString(st->st_mtime);
        auto size = sizeString(st->st_size);

        auto info = g_strdup_printf("%s %3luL %s %s %s:%s ", 
            mode, (unsigned long)links, size, date, user, group);
        //auto info = g_strdup_printf("%s %s:%3lu %s %s:%s ", mode, _("Links"), (unsigned long)links, date, user, group);
        g_free(size);
        g_free(date);
        g_free(user);
        g_free(group);
        g_free(mode);
        return info;
    }
    static void
    setTooltip(GtkWidget *w, const gchar *text){
      //auto t =g_strconcat("<span color=\"yellow\" bgcolor=\"blue\"><i>", text, "</i></span>", NULL);
      auto t =g_strconcat("<span color=\"yellow\"><i>", text, "</i></span>", NULL);
      gtk_widget_set_tooltip_markup (w,t);
      //gtk_widget_add_css_class (w, "tooltip" );
      g_free(t);
      return;
    }
    static 
    GtkButton *newButton(const gchar *icon, const gchar *tooltipText){
      auto button = GTK_BUTTON(gtk_button_new_from_icon_name(icon));
      setTooltip(GTK_WIDGET(button), tooltipText);

      gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
      gtk_button_set_has_frame(button, FALSE);
      return button;
    }
    static 
    GtkMenuButton *newMenuButton(const gchar *icon, const gchar *tooltipText){
      if (!tooltipText && !icon) {
        fprintf(stderr, "Util::newMenuButton(): programing error.\n");
        exit(1);
      }
      auto button = GTK_MENU_BUTTON(gtk_menu_button_new());
      if (tooltipText) {
        if (!icon) gtk_menu_button_set_label(button, tooltipText);
        else setTooltip(GTK_WIDGET(button), tooltipText);
      }
      if (icon) gtk_menu_button_set_icon_name(button, icon);

      gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
      gtk_menu_button_set_has_frame(button, FALSE);
      return button;
    }

private:
    static gboolean
    context_function_f(gpointer data){
        void **arg = (void **)data;
        void * (*function)(gpointer) = (void* (*)(void*))arg[0];
        gpointer function_data = arg[1];
        pthread_mutex_t *mutex = (pthread_mutex_t *)arg[2];
        pthread_cond_t *signal = (pthread_cond_t *)arg[3];
        auto result_p = (void **)arg[4];
        void *result = (*function)(function_data);
        pthread_mutex_lock(mutex);
        *result_p = result;
        pthread_cond_signal(signal);
        pthread_mutex_unlock(mutex);
        return FALSE;
    }
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
    static gchar *
    modeString (mode_t mode) {
        gchar *str=(gchar *)calloc(1, 15);
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

    static gchar *
    sizeString (size_t size) {
        if (size > 1024 * 1024 * 1024){
            return g_strdup_printf(" %3ld GB", size/(1024 * 1024 * 1024));
        }
        if (size > 1024 * 1024){
            return g_strdup_printf(" %3ld MB", size/(1024 * 1024));
        }
        if (size > 1024){
            return g_strdup_printf(" %3ld KB", size/(1024));
        }
        return g_strdup_printf(" %3ld B ", size);
    }
    static gchar *
    dateString (time_t the_time) {
        //pthread_mutex_lock(&dateStringMutex);

#ifdef HAVE_LOCALTIME_R
            struct tm t_r;
#endif
            struct tm *t;

#ifdef HAVE_LOCALTIME_R
            t = localtime_r (&the_time, &t_r);
#else
            t = localtime (&the_time);
#endif
            gchar *date_string=
                g_strdup_printf (" %04d/%02d/%02d  %02d:%02d", t->tm_year + 1900,
                     t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
       // pthread_mutex_unlock(&dateStringMutex);

        return date_string;
    }
public:
      static
      void addMotionController(GtkWidget  *widget){
        auto controller = gtk_event_controller_motion_new();
        gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), controller);
        g_signal_connect (G_OBJECT (controller), "enter", 
            G_CALLBACK (negative), widget);
        g_signal_connect (G_OBJECT (controller), "leave", 
            G_CALLBACK (positive), widget);
    }
     // FIXME: if gridview Settings color for background is
      //        too close to #acaaa5, use a different css class color
      //
      //        Also: add highlight color for text box and change 
      //              text from label to entry, to allow inline
      //              renaming on longpress.
private:
    static gboolean
    negative ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_add_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        //Basic::flushGTK(); // this will cause race condition crash...
        return FALSE;
    }
    static gboolean
    positive ( GtkEventControllerMotion* self,
                    gdouble x,
                    gdouble y,
                    gpointer data) 
    {
        auto eventBox = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
        gtk_widget_remove_css_class (GTK_WIDGET(eventBox), "pathbarboxNegative" );
        //Basic::flushGTK();
        return FALSE;
    }

    
    
  };

}
#endif

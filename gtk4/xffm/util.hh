#ifndef XF_UTIL_HH
#define XF_UTIL_HH
#define MAX_LINES_IN_BUFFER 10000    

namespace xf {
  class Util {
    private:
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

    static     gchar *
    get_terminal_name (void) {
      auto path = getWorkdir();
        gchar *iconname;
        if(!path) {
            iconname = utf_string (g_get_host_name());
        } else if(g_path_is_absolute(path) &&
                g_file_test (path, G_FILE_TEST_EXISTS)) {
            gchar *basename = g_path_get_basename (path);
            gchar *pathname = g_strdup (path);
            gchar *b = utf_string (basename);   // non chopped
            chop_excess (pathname);
            gchar *q = utf_string (pathname);   // non chopped

            g_free (basename);
            g_free (pathname);
            //iconname = g_strconcat (display_host, ":  ", b, " (", q, ")", NULL);
            iconname = g_strconcat (b, " (", q, ")", NULL);
            g_free (q);
            g_free (b);
        } else {
            iconname = utf_string (path);
            chop_excess (iconname);
        }

        return (iconname);
    }


    public:


    static GtkPopover *mkMenu(const char **text, GHashTable **mHash){
      auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
      auto titleBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
      gtk_box_append (vbox, GTK_WIDGET(titleBox));

      GtkPopover *menu = GTK_POPOVER(gtk_popover_new ());
      g_object_set_data(G_OBJECT(menu), "titleBox", titleBox);
      gtk_popover_set_autohide(GTK_POPOVER(menu), TRUE);
      gtk_popover_set_has_arrow(GTK_POPOVER(menu), FALSE);
      gtk_widget_add_css_class (GTK_WIDGET(menu), "inquire" );


      for (const char **p=text; p && *p; p++){
        GtkButton *button = GTK_BUTTON(gtk_button_new());
        auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto label = GTK_LABEL(gtk_label_new(""));
        gtk_label_set_markup(label, *p);
        auto icon = (const char *) g_hash_table_lookup(mHash[0], *p);
        if (icon){
          auto image = gtk_image_new_from_icon_name(icon);
          Util::boxPack0(hbox, GTK_WIDGET(image),  FALSE, FALSE, 0);
        }
        Util::boxPack0(hbox, GTK_WIDGET(label),  FALSE, FALSE, 5);
        gtk_button_set_child(button, GTK_WIDGET(hbox));
        Util::boxPack0(vbox, GTK_WIDGET(button),  FALSE, FALSE, 0);
      
        
        g_object_set_data(G_OBJECT(menu), *p, button);

        gtk_button_set_has_frame(button, FALSE);
        gtk_widget_set_visible(GTK_WIDGET(button), TRUE);
        
        auto callback = g_hash_table_lookup(mHash[1], *p);
        TRACE("mkMenu() callback=%p, icon=%s\n", callback, icon);
        if (callback) {
          auto data = g_hash_table_lookup(mHash[2], *p);
          g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK(callback), data);
        }
        g_object_set_data(G_OBJECT(button), "menu", menu);
      }
          
      gtk_popover_set_child (menu, GTK_WIDGET(vbox));
      return menu;
    }

    static
    void setWindowTitle(void){
        gchar *gg = get_terminal_name();
        auto user = g_get_user_name();
        auto host = g_strdup(g_get_host_name());
        if (strchr(host, '.')) *strchr(host, '.')=0;
        gchar *g = g_strconcat(user,"@",host,":",gg, NULL);
        g_free(host);
        g_free(gg); 
        gtk_window_set_title(GTK_WINDOW(MainWidget), g);
        g_free(g);
        auto basename = g_path_get_basename(getWorkdir());
        auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(MainWidget), "notebook"));
        auto child = getCurrentChild();
        auto tabWidget = gtk_notebook_get_tab_label(notebook, child);
        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(tabWidget), "label"));
        gtk_label_set_markup(label, basename);
        g_free(basename);
    }
    
    static char *inputText(GtkTextView *input){
        auto buffer = gtk_text_view_get_buffer(input);
        GtkTextIter  start, end;
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        auto text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        g_strstrip(text);   
        if (text[0] == '$') { // Eliminate any preceeding $
          text[0] = ' ';
          g_strstrip(text);   
        }
        return text;
    }
    static bool
    cd (const gchar **v) {   
        if (v[1] == NULL){
          return setWorkdir(g_get_home_dir());
        }
        // tilde and $HOME
        if (strncmp(v[1], "~", strlen("~")) == 0){
          const char *part2 = v[1] + strlen("~");
          char *dir = g_strconcat(g_get_home_dir(), part2, NULL);
          auto retval = setWorkdir(dir);
          g_free(dir);
          return retval;
        }
        if (strncmp(v[1], "$HOME", strlen("$HOME")) == 0){
          const char *part2 = v[1] + strlen("$HOME");
          char *dir = g_strconcat(g_get_home_dir(),  part2, NULL);
          auto retval = setWorkdir(dir);
          g_free(dir);
          return retval;
        }

        // must allow relative paths too.
        if (!g_path_is_absolute(v[1])){
          //const gchar *wd = getWorkdir();
          //bool isRoot = (strcmp(wd, G_DIR_SEPARATOR_S)==0);
          char *dir =  g_strconcat(getWorkdir(), G_DIR_SEPARATOR_S, v[1], NULL);
          gchar *rpath = realpath(dir, NULL);
          g_free(dir);
          if (!rpath) return false;

          if(!g_file_test (rpath, G_FILE_TEST_IS_DIR)) {
            g_free(rpath);
            return false; 
          }
          auto retval = setWorkdir(rpath);
          g_free(rpath);
          return retval;
        }
        // absolute path

        gchar *rpath = realpath(v[1], NULL);
        if (!rpath) return false;
        

        auto retval = setWorkdir(rpath);
        g_free(rpath);

        return retval;
    }

    static char **getVector(const char *text, const char *token){
      auto string =g_strdup(text);
      g_strstrip(string);     
      //DBG( "getVector():string=%s\n", string);
      char **vector;
      if (!strstr(string, token)){
        vector = (char **)calloc(2,sizeof(char *));
        vector[0] = g_strdup(string);
      } else {
        vector = g_strsplit(string,token,-1);
      }
      g_free(string);
      //for (char **p=vector; p && *p && p->id p++)  DBG( "getVector():p=%s\n",*p);
      return vector;
    }

    static GtkBox *vButtonBox(void){
      return GTK_BOX(g_object_get_data(G_OBJECT(MainWidget), "buttonBox"));
    }
    static GtkTextView *getCurrentTextView(void){
      auto child = getCurrentChild();
      return GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(child), "output"));
    }
    static GtkPaned *getCurrentPane(void){
      auto child = getCurrentChild();
      auto vpane = GTK_PANED(g_object_get_data(G_OBJECT(child), "vpane"));
      return vpane;
    }

    static GtkBox *getCurrentButtonSpace(void){
      auto child = getCurrentChild();
      return GTK_BOX(g_object_get_data(G_OBJECT(child), "buttonSpace"));
    }
    static GtkWidget *getCurrentChild(void){
      //DBG("getCurrentChild...\n");
      if (!MainWidget) return NULL;
      auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(MainWidget), "notebook"));
      int num = gtk_notebook_get_current_page(notebook);
      GtkWidget *child = gtk_notebook_get_nth_page (notebook, num);
      return child;
    }
    static const gchar *getWorkdir(void){
      //DBG("getWorkdir...\n");
      if (!MainWidget) return NULL;
      auto child = getCurrentChild();
      return (const gchar *)g_object_get_data(G_OBJECT(child), "path");
    }
    static bool setWorkdir(const gchar *path){
      //DBG("setWorkdir...\n");
      if (!MainWidget) return false;
      auto child = getCurrentChild();
      auto wd = (gchar *)g_object_get_data(G_OBJECT(child), "path");
      g_free(wd);
      g_object_set_data(G_OBJECT(child), "path", g_strdup(path));
      setWindowTitle();
      return true;
    }
      
    static void
    flushGTK(void){
      while (g_main_context_pending(NULL))
        g_main_context_iteration(NULL, TRUE);
    }
    static void
    concat(gchar **fullString, const gchar* addOn){
        if (!(*fullString)) {
          *fullString = g_strdup(addOn);
          return;
        }
        auto newString = g_strconcat(*fullString, addOn, NULL);
        g_free(*fullString);
        *fullString = newString;
    }
    static 
    void packEnd(GtkBox *box, GtkWidget *widget){
        GtkBox *vbox =    GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        Util::boxPack0 (box, GTK_WIDGET(vbox), FALSE, FALSE, 0);
        Util::boxPack0 (vbox, GTK_WIDGET(widget), FALSE, FALSE, 0);
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
    static 
    GtkTextView *newTextView(void){
        auto output = GTK_TEXT_VIEW(gtk_text_view_new ());
        gtk_text_view_set_monospace (output, TRUE);
        gtk_widget_set_can_focus(GTK_WIDGET(output), FALSE);
        gtk_text_view_set_wrap_mode (output, GTK_WRAP_WORD);
        gtk_text_view_set_cursor_visible (output, FALSE);
        gtk_widget_add_css_class (GTK_WIDGET(output), "output" );
        //gtk_container_set_border_width (GTK_CONTAINER (output), 2);
        return output;
    }
    static 
    GtkScale *newSizeScale(const gchar *tooltipText){
        auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 6.0, 24.0, 6.0));
        // Load saved value fron xffm+/settings.ini file (if any)
        gint size = Settings::getInteger("xfterm", "fontSize");
        if (size < 0) size = DEFAULT_FIXED_FONT_SIZE;
        gtk_range_set_value(GTK_RANGE(size_scale), size);
        gtk_range_set_increments (GTK_RANGE(size_scale), 2.0, 6.0);
        gtk_widget_set_size_request (GTK_WIDGET(size_scale),-1,75);
        gtk_scale_set_value_pos (size_scale,GTK_POS_BOTTOM);
        gtk_adjustment_set_upper (gtk_range_get_adjustment(GTK_RANGE(size_scale)), 24.0);
        setTooltip (GTK_WIDGET(size_scale),tooltipText);        
        return size_scale;
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
    private:
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
    static const gchar *getTerminal(){
        setTerminal();
        return  getenv("TERMINAL");
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
    /////   print  //////

    static void *
    scroll_to_top(GtkTextView *textview){
        if (!textview) return NULL;
        // make sure all text is written before attempting scroll
        flushGTK();
        GtkTextIter start, end;
        auto buffer = gtk_text_view_get_buffer (textview);
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        gtk_text_view_scroll_to_iter (textview,
                              &start,
                              0.0,
                              FALSE,
                              0.0, 0.0);        
        flushGTK();
        return NULL;
    }

    static void *
    scroll_to_bottom(GtkTextView *textview){
        if (!textview) return NULL;
        // make sure all text is written before attempting scroll
        flushGTK();
        GtkTextIter start, end;
        auto buffer = gtk_text_view_get_buffer (textview);
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        auto mark = gtk_text_buffer_create_mark (buffer, "scrolldown", &end, FALSE);
        gtk_text_view_scroll_to_mark (textview, mark, 0.2,    /*gdouble within_margin, */
                                      TRUE, 1.0, 1.0);
        //gtk_text_view_scroll_mark_onscreen (textview, mark);
        flushGTK();
        return NULL;
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


  static void clear_text(GtkTextView *textview){
      if (!textview) return;
      void *arg[]={(void *)textview, NULL};
      context_function(clear_text_buffer_f, arg);
  }
  static void print(GtkTextView *textview, const gchar *tag, gchar *string){
      if (!textview) return;
      void *arg[]={(void *)textview, (void *)tag, (void *)string};
      context_function(print_f, arg);
      g_free(string);
  }
  static void print(GtkTextView *textview, gchar *string){
      print(textview, NULL, string);
  }


    static void print_error(GtkTextView *textview, gchar *string){
        if (!textview) return;
        print_icon(textview, "dialog-error", "bold", string);
    }

    static void print_icon(GtkTextView *textview, 
                              const gchar *iconname, 
                              const gchar *tag, 
                              gchar *string){
        print(textview, string);
    }
    static void // print_icon will free string.
    printInfo(GtkTextView *textview, const gchar *icon, gchar *string){
        print(textview, string);
    }
    static void // print_icon will free string.
    printError(GtkTextView *textview, gchar *string){
        print(textview, string);
    }
    static void // print_icon will free string.
    printStdErr(GtkTextView *textview, gchar *string){
        print(textview, string);
    }

    static void showText(GtkTextView *textview){
        if (!textview) return;
        auto vpane = GTK_PANED(g_object_get_data(G_OBJECT(textview), "vpane"));
        void *arg[]={(void *)vpane, NULL, NULL, NULL, NULL};
        context_function(show_text_buffer_f, arg);
    }

/*

    static void // print_icon will free string.
    printError(GtkTextView *textview, gchar *string){
      TRACE("printError\n");
	      showTextSmallErr(textview);
        auto tag = Settings<Type>::getString("window.errorColor");
        if (tag) print(textview, "edit-delete", tag, string);
        else print(textview, "edit-delete", "red", string);
    }
 
    static void // print_icon will free string.
    printStdErr(GtkTextView *textview, gchar *string){
	      showTextSmallErr(textview);
        auto tag = Settings<Type>::getString("window.errorColor");
        if (tag) print(textview, tag, string);
        else print(textview, "red", string);
    }

    static void // print_icon will free string.
    printInfo(GtkTextView *textview, gchar *string){
        auto tag = Settings<Type>::getString("window.infoColor");
        if (tag) print(textview, tag, string);
        else print(textview, "green", string);
    }
    
    static void // print_icon will free string.
    printInfo(GtkTextView *textview, const gchar *icon, gchar *string){
        auto tag = Settings<Type>::getString("window.infoColor");
        if (tag) print(textview, icon, tag, string);
        else print(textview, icon, "green", string);
     
    }

  static void print_icon(GtkTextView *textview, const gchar *iconname, gchar *string)
  {
      if (!textview) return;
      auto pixbuf = pixbuf_c::getPixbuf(iconname, -16);
      void *arg[]={(void *)pixbuf, (void *)textview, NULL, (void *)string};
      context_function(print_i, arg);
      g_free(string);
  }

  static void print(GtkTextView *textview, 
                              const gchar *iconname, 
                              const gchar *tag, 
                              gchar *string)
  {
      print_icon(textview, iconname, tag, string);
  }

  static void print_icon(GtkTextView *textview, 
                              const gchar *iconname, 
                              const gchar *tag, 
                              gchar *string)
  {
      if (!textview) return;
      auto pixbuf = pixbuf_c::getPixbuf(iconname, -16);
      void *arg[]={(void *)pixbuf, (void *)textview, (void *)tag, (void *)string};
      context_function(print_i, arg);
      g_free(string);
  }

 */

  private:
  static void *
  clear_text_buffer_f(void *data){
      if (!data) return GINT_TO_POINTER(-1);
      auto arg=(void **)data;
      auto buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (arg[0]));
      GtkTextIter start, end;
      gtk_text_buffer_get_bounds (buffer, &start, &end);
      gtk_text_buffer_delete (buffer, &start, &end);
      return NULL;
  }
  static void * 
  print_f(void *data){
      if (!data) return GINT_TO_POINTER(-1);
      void **arg = (void **)data;
      auto textview = GTK_TEXT_VIEW(arg[0]);
      if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
      auto tag = (const gchar *)arg[1];
      auto string = (const gchar *)arg[2];
      // FIXME: if fontsize change, we need to process a new global CSS
      //        each textview will have its own class.
      // set_font_size (GTK_WIDGET(textview));

      auto buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
      if (trim_output(buffer)){
          // textview is at line limit.
      }

      auto tags = resolve_tags(buffer, tag);
      if(string && strlen (string)) {
          insert_string (buffer, string, tags);
      }
      g_free(tags);
      scroll_to_bottom(textview);
      return NULL;
  }
 /* static void *
  print_i(void *data){
      if (!data) return GINT_TO_POINTER(-1);
      void **arg=(void **)data;
      GtkTextView *textview = GTK_TEXT_VIEW(arg[1]);
      GdkPixbuf *pixbuf = (GdkPixbuf *)arg[0];
      if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
      GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
      GtkTextIter start, end;
      gtk_text_buffer_get_bounds (buffer, &start, &end);
      gtk_text_buffer_insert_pixbuf (buffer, &end, pixbuf);
      return print_f(arg+1);
  }*/

  static gboolean trim_output(GtkTextBuffer *buffer) {
  //  This is just to prevent a memory overrun (intentional or unintentional).
      auto line_count = gtk_text_buffer_get_line_count (buffer);
      auto max_lines_in_buffer = MAX_LINES_IN_BUFFER;
      if (line_count > max_lines_in_buffer) {
          gtk_text_buffer_set_text(buffer, "", -1);
          return TRUE;
      }
      return FALSE;
  }
  static void insert_string (GtkTextBuffer * buffer, const gchar * s, GtkTextTag **tags) {
      if(!s) return;
      GtkTextIter start, end, real_end;
      auto line = gtk_text_buffer_get_line_count (buffer);
      gchar *a = NULL;
     

      gchar esc[]={0x1B, '[', 0};
      gboolean head_section = strncmp(s, esc, strlen(esc));
      const char *esc_location = strstr (s, esc);
      if(esc_location) {      // escape sequence: <ESC>[

          // do a split
              TRACE( "0.splitting %s\n", s);
          gchar **split = g_strsplit(s, esc, -1);
          insert_string (buffer, split[0], NULL);
          gchar *fullTag=NULL;
          gchar **pp=split+1;
          GtkTextTag **textviewTags = NULL;
          // single
          for (;pp && *pp; pp++){
              gchar *code = *pp;

              if (*code == 'K'){
                   // erase from cursor to eol
                   // insert string with whatever tag you have.
                  insert_string (buffer, code+1, textviewTags);
                  continue;
              }
              /* if (strncmp(code, "0K", 2)==0 ||strncmp(code, "1K", 2)==0 || strncmp(code, "2K", 2)==0){
                  // erase from cursor to eol, from bol to cursor, erase line
                  concat(&fullString, code+2);
                  continue;
              }*/
               // split on m
              if (*code == 'm'){
                  g_free(fullTag); fullTag = NULL;
                  g_free(textviewTags); textviewTags = NULL;
                  insert_string (buffer, code+1, NULL);
                  continue;
              }
              gchar **ss = g_strsplit(code, "m", 2);
              gchar **codes;
              // Check for multiple tags
              if (strchr(code, ';')){
                  codes = g_strsplit(ss[0], ";", -1);
              } else {
                  codes = (gchar **)calloc(2, sizeof(gchar*));
                  codes[0] = g_strdup(ss[0]);
              }
              TRACE( "1.splitting %s --> %s, %s: codes[0]=%s\n", *pp, ss[0], ss[1],codes[0]);
              // construct xffm tag
              gchar **t;
              gchar *thisTag=NULL;
              for (t=codes; t && *t; t++){
                  const gchar *ansiTag = get_ansi_tag(*t);
                  if (!ansiTag){
                      if (strcmp(*t, "0")) {
                          g_free(fullTag); fullTag = NULL;
                          g_free(textviewTags); textviewTags = NULL;
                          insert_string (buffer, ss[1], NULL);
                          g_strfreev(ss);
                          goto endloop;
                      } else {
                          TRACE("no ansiTag for \"%s\"\n", *t);
                      }
                      continue;
                  } else {
                      TRACE("ansiTag=%s\n", ansiTag);
                       concat(&fullTag, "/");
                       concat(&fullTag, ansiTag);
                      TRACE("fullTag= %s\n", fullTag); 
                  }
              }
              // Insert string
              g_free(textviewTags);
              if (fullTag) textviewTags = resolve_tags(buffer, fullTag);
              else textviewTags = NULL;
              insert_string (buffer, ss[1], textviewTags);
              g_strfreev(ss);
endloop:;
          }
          g_free(fullTag);
          g_free(textviewTags);
          g_strfreev(split);
          // done
          return;
      }

      auto mark = gtk_text_buffer_get_mark (buffer, "rfm-ow");
      gchar *cr = (gchar *)strchr (s, 0x0D);
      if(cr && cr[1] == 0x0A) *cr = ' ';       //CR-LF
      if(strchr (s, 0x0D)) {      //CR
          auto aa = g_strdup (s);
          *strchr (aa, 0x0D) = 0;
          insert_string (buffer, aa, tags);
          g_free (aa);
          const char *caa = strchr (s, 0x0D) + 1;
          if(mark) {
              gtk_text_buffer_get_iter_at_line (buffer, &start, line);
              gtk_text_buffer_move_mark (buffer, mark, &start);
          }
          insert_string (buffer, caa, tags);
          g_free (a);
          // we're done
          return;
      }

      gchar *q = utf_string (s);
      /// this should be mutex protected since this function is being called
      //  from threads all over the place.
      static pthread_mutex_t insert_mutex =  PTHREAD_MUTEX_INITIALIZER;

      pthread_mutex_lock(&insert_mutex);
      if(mark == NULL) {
          gtk_text_buffer_get_end_iter (buffer, &end);
          gtk_text_buffer_create_mark (buffer, "rfm-ow", &end, FALSE);
      } else {
          gtk_text_buffer_get_iter_at_mark (buffer, &end, mark);
      }

      gtk_text_buffer_get_iter_at_line (buffer, &start, line);
      gtk_text_buffer_get_end_iter (buffer, &real_end);

      // overwrite section
      auto pretext = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
      auto posttext = gtk_text_buffer_get_text (buffer, &end, &real_end, TRUE);
      long prechars = g_utf8_strlen (pretext, -1);
      long postchars = g_utf8_strlen (posttext, -1);
      long qchars = g_utf8_strlen (q, -1);
      g_free (pretext);
      g_free (posttext);
      if(qchars < postchars) {
          gtk_text_buffer_get_iter_at_line_offset (buffer, &real_end, line, prechars + qchars);
      }
      /////
      gtk_text_buffer_delete (buffer, &end, &real_end);
      if(tags) {
          gint tag_count = 0;
          GtkTextTag **tt = tags;
          for (;tt && *tt; tt++)tag_count++;
          switch (tag_count) { // This is hacky
              case 1:
                  gtk_text_buffer_insert_with_tags (buffer, &end, q, -1, 
                          tags[0], NULL);
                  break;
              case 2:
                  gtk_text_buffer_insert_with_tags (buffer, &end, q, -1,
                          tags[0],tags[1], NULL);
                  break;
              default: // Max. 3 tags...
                  gtk_text_buffer_insert_with_tags (buffer, &end, q, -1,
                          tags[0],tags[1],tags[2], NULL);
                  break;
          }
                  
      } else {
          TRACE( "gtk_text_buffer_insert %s\n", q);
          gtk_text_buffer_insert (buffer, &end, q, -1);
      }
      pthread_mutex_unlock(&insert_mutex);
      g_free (q);
      g_free (a);
      return;
  }
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
 public:
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
 private:
  static
  GtkTextTag *
  resolve_tag (GtkTextBuffer * buffer, const gchar * id) {
      TRACE("Print::resolve_tag(%s)\n", id);
      if (!id) return NULL;

      GtkTextTag *tag = NULL;
      lpterm_colors_t lpterm_colors_v[] = {

          {"command", {101, 0x5858, 0x3434, 0xcfcf}},
          {"stderr", {102, 0xcccc, 0, 0}},
          {"command_id", {103, 0x0000, 0x0000, 0xffff}},
          {"grey", {110, 0x8888, 0x8888, 0x8888}},
// xterm colors
          // normal
          {"black", {201, 0x0000, 0x0000, 0x0000}},
          {"red", {202, 0xcdcd, 0x0, 0x0}},
          {"green", {203, 0x0, 0xcdcd, 0x0}},
          {"yellow", {204, 0xcdcd, 0xcdcd, 0x0}},
          {"blue", {205, 0x0, 0x0, 0xeeee}},
          {"magenta", {206, 0xcdcd, 0x0, 0xcdcd}},
          {"cyan", {207, 0x0, 0xcdcd, 0xcdcd}},
          {"white", {208, 0xe5e5, 0xe5e5, 0xe5e5}}, //gray
          // bright
          {"Black", {211, 0x7f7f, 0x7f7f, 0x7f7f}},
          {"Red", {212, 0xffff, 0x0, 0x0}},
          {"Green", {213, 0x0, 0xffff, 0x0}},
          {"Yellow", {214, 0xffff, 0xffff, 0x0}},
          {"Blue", {215, 0x0, 0x0, 0xffff}},
          {"Magenta", {216, 0xffff, 0x0, 0xffff}},
          {"Cyan", {217, 0x0, 0xffff, 0xffff}},
          {"White", {218, 0xffff, 0xffff, 0xffff}}, 

          {NULL, {0, 0, 0, 0}}

      };
      void *initialized = g_object_get_data(G_OBJECT(buffer), "text_tag_initialized");
      if (!initialized) {
          lpterm_colors_t *p = lpterm_colors_v;
          for(; p && p->id; p++) {
              gtk_text_buffer_create_tag (buffer, p->id, 
                      "foreground_gdk", &(p->color), 
                      NULL);
              TRACE("*** gtk_text_tag_table_lookup(%s) -> %p\n",p->id, tag);
              gchar *bg_id = g_strconcat(p->id, "_bg", NULL);
              gtk_text_buffer_create_tag (buffer, bg_id, 
                      "background_gdk", &(p->color), 
                      NULL);
              g_free(bg_id);
         }
          gtk_text_buffer_create_tag (buffer, 
                  "bold", "weight", PANGO_WEIGHT_BOLD, "foreground_gdk", NULL, NULL);
          gtk_text_buffer_create_tag (buffer, 
                  "italic", "style", PANGO_STYLE_ITALIC, "foreground_gdk", NULL, NULL);
          g_object_set_data(G_OBJECT(buffer), "text_tag_initialized", GINT_TO_POINTER(1));
      } 

      tag = gtk_text_tag_table_lookup (gtk_text_buffer_get_tag_table (buffer), id);
      TRACE("*** gtk_text_tag_table_lookup(%s) -> %p\n",id, tag);

      if (!tag && id[0] == '#'){
          // create a new tag for color id.
          TRACE("Print::resolve_tag(%s): *** creating new tag.\n", id);
          auto color = (GdkRGBA *)calloc(1, sizeof(GdkRGBA));
          if (!color){
              ERROR("Print::resolve_tag: calloc: %s\n", strerror(errno));
              exit(1);
          }
          if (gdk_rgba_parse (color, id)) {
              auto c = (GdkColor *)calloc(1, sizeof(GdkColor));
              if (!c){
                  ERROR("Print::resolve_tag: calloc: %s\n", strerror(errno));
                  exit(1);
              }
              c->red = color->red * 0xffff;
              c->green = color->green * 0xffff;
              c->blue = color->blue * 0xffff;
              TRACE("***tag %s is %lf,%lf,%lf\n", id, color->red,color->green,color->blue); 
              TRACE("***tag %s is 0x%0x,0x%0x,0x%0x\n", id, c->red,c->green,c->blue); 
              tag = gtk_text_buffer_create_tag(buffer, id, "foreground_gdk", c, NULL);
          } 
          g_free(color);            
      }

      if (!tag) fprintf(stderr,"***Error:: resolve_tag(): No GtkTextTag for %s", id);
      return tag;
  }

  static GtkTextTag **
  resolve_tags(GtkTextBuffer * buffer, const gchar *tag){
      TRACE("Print::resolve_tags(%s)\n", tag);
      if (!tag) return NULL;
      gchar **userTags;
      if (strchr(tag, '/')){
          // multiple tags
          userTags = g_strsplit(tag, "/", -1);
      } else {
          userTags = (gchar **)calloc(2, sizeof(GtkTextTag *));
          userTags[0] = g_strdup(tag);
      }
      gchar **t;
      gint tag_count = 0;
      for (t = userTags;t && *t; t++){
          tag_count++;
      }
      auto tags = (GtkTextTag **)calloc(tag_count+1, sizeof(GtkTextTag *));
      int i=0;
      for (t=userTags; t && *t;t++){
          tags[i] = resolve_tag (buffer, *t);
          if (tags[i] == NULL) {
              TRACE("*** could not resolve tag: \"%s\"\n", *t);
          } else i++;
      }
      g_strfreev(userTags);
      return tags;
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

  static const gchar *
  get_ansi_tag(const gchar *code){
      static sequence_t sequence_v[] = {
              {"black", "30"},
              {"black_bg", "40"},
              {"red", "31"},
              {"red_bg", "41"},
              {"green", "32"},
              {"green_bg", "42"},
              {"yellow", "33"},
              {"yellow_bg", "43"},
              {"blue", "34"},
              {"blue_bg", "44"},
              {"magenta", "35"},
              {"magenta_bg", "45"},
              {"cyan", "36"},
              {"cyan_bg", "46"},
              {"white", "37"},
              {"white_bg", "47"},
            
              {"Black_bg", "100"},
              {"Black", "90"},
              {"Red_bg", "101"},
              {"Red", "91"},
              {"Green_bg", "102"},
              {"Green", "92"},
              {"Yellow_bg", "103"},
              {"Yellow", "93"},
              {"Blue_bg", "104"},
              {"Blue", "94"},
              {"Magenta_bg", "105"},
              {"Magenta", "95"},
              {"Cyan_bg", "106"},
              {"Cyan", "96"},
              {"White_bg", "107"},
              {"White", "97"},

              {"bold", "1"},
              {"bold", "01"},
              {"italic", "4"},
              {"italic", "04"},
              {"blink", "5"},
              {"blink", "05"},
              {NULL, ""},
              {NULL, "0"},
              {NULL, "00"},
              {NULL, "22"},
              {NULL, "24"},
              {NULL, NULL}            // this marks the end of sequences.
      };
      sequence_t *p;
      for(p = sequence_v; p && p->sequence; p++) {
          if(strcmp (code, p->sequence) == 0) {
              return p->id;
          }
      }
      return NULL;

  }
    static void *
    show_text_buffer_f (void *data) {
        if (!data) return GINT_TO_POINTER(-1);
        auto arg=(void **)data;
        auto vpane = GTK_PANED(arg[0]);
        auto fullview =arg[1]; 
        auto small = arg[2];
        auto err = arg[3];
        if(!vpane) {
            ERROR("vpane is NULL\n");
            return NULL;
        }
        TRACE("show_text_buffer_f:: err=%p\n", err);
  /*      gint min, max;
        g_object_get(G_OBJECT(vpane), "min-position", &min, NULL);
        g_object_get(G_OBJECT(vpane), "max-position", &max, NULL);
        if (fullview) {
            TRACE("show_text_buffer_f()::fullview:setting vpane position to %d\n", min);
            gtk_paned_set_position (vpane, min);
            g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(min));
            while (gtk_events_pending()) gtk_main_iteration();
            TRACE("vpane position set to =%d\n", gtk_paned_get_position(vpane));
            return NULL;
        }*/

        graphene_rect_t grect;
        if (!gtk_widget_compute_bounds (GTK_WIDGET(vpane), MainWidget, &grect)){
          fprintf(stderr, "show_text_buffer_f:: should not happen.\n");
        }

        //GtkAllocation allocation;
        //gtk_widget_get_allocation(GTK_WIDGET(vpane), &allocation);
        float vheight = grect.size.height;
        gint height ;
        if (small) height = 8*vheight/10;
        else height = 2*vheight/3;
        TRACE("vheight = %d, position = %d\n", vheight, gtk_paned_get_position(vpane));
        if (gtk_paned_get_position(vpane) > height) {
            TRACE("show_text_buffer_f()::setting vpane position to %d\n", height);
            gtk_paned_set_position (vpane, height);
            //g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(height));
        } else TRACE("not setting vpane position to %d\n", height);
        

        return NULL;
    }
 public:
    static gchar *
    get_current_text (GtkTextView *textview) {
        // get current text
        GtkTextIter start, end;
        auto buffer = gtk_text_view_get_buffer (textview);

        gtk_text_buffer_get_bounds (buffer, &start, &end);
        auto t = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
        g_strchug(t);
        return t;
    }
    static gboolean isValidTextView(void *textView){
        gboolean retval = FALSE;
        void *p = g_list_find(textviewList, textView);
        if (p) retval = TRUE;
        return retval;
    }

    static void *reference_textview(GtkTextView *textView){
        TRACE("reference_run_button(%p)\n", (void *)textView);
        textviewList = g_list_prepend(textviewList, (void *)textView);
        return NULL;
    }

    static void
    unreference_textview(GtkTextView *textView){
        TRACE("unreference_run_button(%p)\n", (void *)textView);
        void *p = g_list_find(textviewList, (void *)textView);
        if (p){
            textviewList = g_list_remove(textviewList, (void *)textView);
        }
    }


  };
}
#endif

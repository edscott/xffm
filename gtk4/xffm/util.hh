#ifndef XF_UTIL_HH
#define XF_UTIL_HH
#define MAX_LINES_IN_BUFFER 10000    
namespace xf {
  class Util : public UtilBasic, public Print, public Workdir {
  public:
      
  static
  GtkCssProvider * setCSSprovider(void){
    GtkCssProvider *css_provider = gtk_css_provider_new();

    char *outputBg = Settings::getString("xfterm", "outputBg");
    if (!outputBg) outputBg = g_strdup("#bbbbbb");

    char *outputFg = Settings::getString("xfterm", "outputFg");
    if (!outputFg) outputFg = g_strdup("#111111");

    char *inputBg = Settings::getString("xfterm", "inputBg");
    if (!inputBg) inputBg = g_strdup("#dddddd");

    char *inputFg = Settings::getString("xfterm", "inputFg");
    if (!inputFg) inputFg = g_strdup("#000000");

    char *iconsFg = Settings::getString("xfterm", "iconsFg");
    if (!iconsFg) iconsFg = g_strdup("#000000");

    char *iconsBg = Settings::getString("xfterm", "iconsBg");
    if (!iconsBg) iconsBg = g_strdup("#ffffff");


    char *data = g_strdup_printf(
    "\
    .vbox {\
      background-color: #888888;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    .output {\
      background-color: %s;\
      color: %s;\
      font-family: monospace;\
    }\
    .outputview text selection {\
      background-color: blue;\
      color: yellow;\
    }\
    .input {\
      background-color: %s;\
      color: %s;\
      font-family: monospace;\
    }\
    .inputview text selection  {\
      background-color: #abc2df;\
      color: black;\
      font-family: monospace;\
    }\
    .gridviewColors {\
      background-color: %s;\
      color: %s;\
      font-family: monospace;\
    }\
    .gridviewBox {\
      margin-bottom: 0;\
      margin-left: 0;\
      margin-right: 0;\
      margin-top: 0;\
    }\
    .font1 {\
      font-size: xx-small;\
    }\
    .font2 {\
      font-size: x-small;\
    }\
    .font3 {\
      font-size: small;\
    }\
    .font4 {\
      font-size: medium;\
    }\
    .font5 {\
      font-size: large;\
    }\
    .font6 {\
      font-size: x-large;\
    }\
    .font7 {\
      font-size: xx-large;\
    }\
    .prompt {\
      background-color: #333333;\
      color: #00ff00;\
    }\
    tooltip {\
      color: black;\
      background-color: #acaaa5;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    .pathbarbox * {\
      background-color: #dcdad5;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    .location * {\
      color: blue;\
      background-color: #dcdad5;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    .pathbarboxNegative * {\
      background-color: #acaaa5;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    ",
      outputBg, outputFg, inputBg, inputFg, iconsBg, iconsFg);
    g_free(outputBg);
    g_free(outputFg);
    g_free(inputBg);
    g_free(inputFg);
    g_free(iconsFg);
    g_free(iconsBg);
    gtk_css_provider_load_from_string (css_provider, data);
    g_free(data);
    return css_provider;
  }
  static void
  defaultColors(GtkButton *button, void *data){
    auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
    gtk_popover_popdown(menu);
    const char *x[]={"Bg", "Fg", NULL};
    for (const char **p = x; p && *p; p++){
      auto key = g_strconcat((const char *)data, *p, NULL);
      Settings::removeKey("xfterm", key);
      g_free(key);
    }
    auto css = Util::setCSSprovider();
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_USER); 
    
    context_function(reloadAll_f, NULL);
    
    return;
  }

    static void
    setColor(GObject* source_object, GAsyncResult* result, gpointer data){
      auto dialog =(GtkColorDialog*)source_object;
      GError *error_=NULL;
      auto item = (const char *)data;

      GdkRGBA *color = gtk_color_dialog_choose_rgba_finish (dialog, result, &error_);

      if (color) {
        TRACE("setColor: r=%f, g=%f, b=%f, a=%f\n",
          color->red, color->green, color->blue, color->alpha);
        short int red = color->red * 255;
        short int green = color->green * 255;
        short int blue = color->blue * 255;
        TRACE("color #%02x%02x%02x for %s\n", red, green, blue, item);
        char buffer[32];
        snprintf(buffer, 32, "#%02x%02x%02x", red, green, blue);
        Settings::setString("xfterm", item, buffer);
        auto css = Util::setCSSprovider();
        gtk_style_context_add_provider_for_display(gdk_display_get_default(),
            GTK_STYLE_PROVIDER(css),
            GTK_STYLE_PROVIDER_PRIORITY_USER); 
        
        // Now we change the CSS class.
        // 1. Save color to Settings
        // 2. Reload modified CSS class
        //    if this does the trick END
        //    if not, then 
        //       1. remove css class from all outputs
        //       2. add modified css to all class.
        // Also, we could use a user class.
        // with user class:
        // 1. Save color to Settings
        // 2. create modified user css class
        // 3. remove default color css class
        // 4. add user defined color css.
        // On startup, use user color css instead if
        // Settings has the information.
        // 5. But because of hidden mask transparency, we must reload 
        //    all gridviews 
        // reload all pages. This is done by sending the changed workdir signal.
        // This must be done in main context
        context_function(reloadAll_f, NULL);
      } else {
        TRACE("No color selected.\n");
      }
      g_free(color);
    }

    
    static void *reloadAll_f(void *data){
        auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(MainWidget), "notebook"));
        auto n = gtk_notebook_get_n_pages(notebook);
        for (int i=0; i<n; i++){
          auto child = gtk_notebook_get_nth_page(notebook, i);
          auto path = Child::getWorkdir(child);
          setWorkdir(path, child);
        }
        return NULL;
    }
    
    static void addMenu(GtkPopover *menu, GtkWidget *parent){
      g_object_set_data(G_OBJECT(parent), "menu", menu);
      // Important: must use both of the following instructions:
      gtk_popover_set_default_widget(menu, parent);
      gtk_widget_set_parent(GTK_WIDGET(menu), parent);

      auto gesture = gtk_gesture_click_new();
      gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),3);
      g_signal_connect (G_OBJECT(gesture) , "pressed", EVENT_CALLBACK (openMenu), (void *)menu);
      gtk_widget_add_controller(GTK_WIDGET(parent), GTK_EVENT_CONTROLLER(gesture));
      gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
          GTK_PHASE_CAPTURE);
      return;  
    }
    

  private:
    static void
    clear (GtkButton *button, void *data){
     auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
     gtk_popover_popdown(menu);
     auto childWidget =Util::getCurrentChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      clear_text(output);
      return ;
    }
    static    
    GtkScale *newSizeScale(const gchar *tooltipText, const char *which ){
        double value;
        auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1.0, 7.0, 1.0));
        // Load saved value fron xffm+/settings.ini file (if any)
        char key[64];
        snprintf(key, 64, "%sSize", which);
        char *size = Settings::getString("xfterm", key);
        if (!size) value = 4;
        else if (strcmp(size, "font1")==0) value=1;
        else if (strcmp(size, "font2")==0) value=2;
        else if (strcmp(size, "font3")==0) value=3;
        else if (strcmp(size, "font4")==0) value=4;
        else if (strcmp(size, "font5")==0) value=5;
        else if (strcmp(size, "font6")==0) value=6;
        else if (strcmp(size, "font7")==0) value=7;

        gtk_range_set_value(GTK_RANGE(size_scale), value);

        gtk_range_set_increments (GTK_RANGE(size_scale), 1.0, 1.0);
        gtk_widget_set_size_request (GTK_WIDGET(size_scale),75,-1);

        gtk_scale_set_value_pos (size_scale,GTK_POS_BOTTOM);
        //gtk_adjustment_set_upper (gtk_range_get_adjustment(GTK_RANGE(size_scale)), 24.0);
        Util::setTooltip (GTK_WIDGET(size_scale),tooltipText);   
        if (strcmp(which, "output")==0) g_signal_connect(G_OBJECT(size_scale), "value-changed", G_CALLBACK(changeSize1), NULL);
        else  g_signal_connect(G_OBJECT(size_scale), "value-changed", G_CALLBACK(changeSize2), NULL);
        return size_scale;
    }
    static void
    changeSize (GtkRange* self, gpointer data){
      auto textviewName = (const char *) data;
      auto value = gtk_range_get_value(self);
      int v = value;
      auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(MainWidget), "notebook"));
      auto n = gtk_notebook_get_n_pages(notebook);
      char buf[32];

     // const char *textviews[]={"output", "input", "dollar", NULL};
     // for (const char **p=textviews; p && *p; p++)
     for (int i=0; i<n; i++){
        auto child = gtk_notebook_get_nth_page(notebook, i);
        auto textview = GTK_WIDGET(g_object_get_data(G_OBJECT(child), textviewName));
        for (int j=1; j<=7; j++){
          snprintf(buf, 32, "font%d", j);
          gtk_widget_remove_css_class (textview, buf);
        }
        snprintf(buf, 32, "font%d", v);
        gtk_widget_add_css_class (textview, buf);
      }
      snprintf(buf, 32, "font%d", v);
      char key[64];
      snprintf(key, 64, "%sSize", textviewName);
      if (strcmp(textviewName, "dollar")) Settings::setString("xfterm", key, buf);
    }
    static void
    changeSize1 (GtkRange* self, gpointer user_data){
      changeSize (self, (void *)"output");
    }
    static void
    changeSize2 (GtkRange* self, gpointer user_data){
      changeSize (self, (void *)"dollar");
      changeSize (self, (void *)"input");
    }

  public:
    static void
    terminalColors(GtkButton *button, void *data){

      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu"));
      gtk_popover_popdown(menu);
      
      auto dialog = gtk_color_dialog_new();
      //gtk_widget_set_parent(GTK_WIDGET(dialog), MainWidget);
      gtk_color_dialog_set_modal (dialog, TRUE);
      gtk_color_dialog_set_title (dialog, "Color dialog");
      gtk_color_dialog_choose_rgba (dialog, GTK_WINDOW(MainWidget), 
          NULL, NULL, Util::setColor, data);
    }

    static void
    popup(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(data);      
      gtk_popover_popup(menu);
    }

    private:
    static gboolean openMenu(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
      auto menu = GTK_POPOVER(data);
      // position is relative to the parent/default widget.
      TRACE("position %lf,%lf\n", x, y);
      gtk_popover_popup(menu);
      return TRUE;
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


    public:


    
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
    cd (const gchar **v, GtkWidget *child) {   
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
          //const gchar *wd = Child::getWorkdir(child);
          //bool isRoot = (strcmp(wd, G_DIR_SEPARATOR_S)==0);
          char *dir =  g_strconcat(Child::getWorkdir(child), G_DIR_SEPARATOR_S, v[1], NULL);
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
      //TRACE( "getVector():string=%s\n", string);
      char **vector;
      if (!strstr(string, token)){
        vector = (char **)calloc(2,sizeof(char *));
        vector[0] = g_strdup(string);
      } else {
        vector = g_strsplit(string,token,-1);
      }
      g_free(string);
      //for (char **p=vector; p && *p && p->id p++)  TRACE( "getVector():p=%s\n",*p);
      return vector;
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
        gtk_widget_add_css_class (GTK_WIDGET(output), "outputview" );
        char *size = Settings::getString("xfterm", "size");
        if (!size) size = g_strdup("font4"); // medium
        gtk_widget_add_css_class (GTK_WIDGET(output), size );
        return output;
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

  };
}
#endif

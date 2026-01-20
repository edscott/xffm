#ifndef XF_PAGE_CHILD
#define XF_PAGE_CHILD
#include "runbutton.hh"
#include "signals.hh"
#include "base.hh"

namespace xf{
template <class Type> class Dialog;

template <class Type>
class Page : public PageBase<double>
{
    // fmDialog elements are not yet defined,
    // so you cannot use a static Type, like double.
    using dialog_c = Dialog<Type>;

    using baseview_c = View<Type>;
    using gtk_c = Gtk<Type>;
    using util_c = Util<Type>;
    using run_c = Run<Type>;
    using runbutton_c = RunButton<Type>;
    using print_c = Print<Type>;
    using pagesignals_c = PageSignals<Type>;
    using settings_c = Settings<Type>;
    using completion_c = Completion<Type>;

    dialog_c *parent_;
    baseview_c *view_;
    
    GtkBox *pageChild_;
    GtkLabel *pageLabel_;
    GtkBox *pageLabelBox_;
    GtkSpinner *pageLabelSpinner_;
    GtkBox *pageLabelSpinnerBox_;
    GtkBox *pageLabelIconBox_;
    GtkButton *pageLabelButton_;

    gboolean terminalMode_;
    gboolean iconviewIsDefault_;
    GtkBox *hViewBox_;
public:

    View<Type> *view(void){ return view_;}
    void setView(View<Type> *view){view_ = view;}
    dialog_c *parent(void){return parent_;}
    GtkBox *pageChild(void){ return pageChild_;}
    GtkLabel *pageLabel(void){ return pageLabel_;}
    GtkBox *pageLabelBox(void){ return pageLabelBox_;}
    GtkSpinner *pageLabelSpinner(void){ return pageLabelSpinner_;}
    GtkBox *pageLabelSpinnerBox(void){ return pageLabelSpinnerBox_;}
    GtkBox *pageLabelIconBox(void){ return pageLabelIconBox_;}
    GtkButton *pageLabelButton(void){ return pageLabelButton_;}
    GtkBox *hViewBox(void){return hViewBox_;}

    Page(dialog_c *parent, const gchar *workdir){
        parent_ = parent;
        Notebook<Type>::reference_textview(this->output());
        pageChild_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(pageChild_), TRUE);

        pageLabel_ = GTK_LABEL(gtk_label_new (_("loading...")));
        pageLabelBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(pageLabelBox_), FALSE);

        pageLabelSpinner_ = GTK_SPINNER(gtk_spinner_new());
        pageLabelSpinnerBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(pageLabelSpinnerBox_), FALSE);

        pageLabelIconBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(pageLabelIconBox_), FALSE);
        pageLabelButton_ = GTK_BUTTON(gtk_button_new ());

        compat<bool>::boxPack0 (pageLabelBox_, GTK_WIDGET(pageLabelSpinnerBox_), TRUE, TRUE, 0);
        compat<bool>::boxPack0 (pageLabelSpinnerBox_, GTK_WIDGET(pageLabelSpinner_), TRUE, TRUE, 0);
        compat<bool>::boxPack0 (pageLabelBox_, GTK_WIDGET(pageLabelIconBox_), TRUE, TRUE, 0);
        compat<bool>::boxPack0 (pageLabelBox_, GTK_WIDGET(pageLabel_), TRUE, TRUE, 2);
        compat<bool>::boxPack1 (pageLabelBox_, GTK_WIDGET(pageLabelButton_), TRUE, TRUE, 0);
        gtk_c::setup_image_button(pageLabelButton_, CLOSE_TAB, _("Close"));
    
        //set_spinner(pageSpinner_, TRUE, pageLabelIconBox_);
        set_spinner(FALSE);

        compat<bool>::boxPack0 (pageChild_, this->pathbar(), FALSE, TRUE, 0);
//        compat<bool>::boxPack0 (pageChild_, this->pathbar(), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(pageChild_), "pathbar", this->pathbar());


        hViewBox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        //gtk_widget_set_hexpand(GTK_WIDGET(hViewBox_), TRUE);

        compat<bool>::boxPack0 (hViewBox_, GTK_WIDGET(this->vpane()), TRUE, TRUE, 0);
        compat<bool>::boxPack0 (pageChild_, GTK_WIDGET(hViewBox_), TRUE, TRUE, 0);
        compat<bool>::boxPack0 (pageChild_, GTK_WIDGET(this->hButtonBox()), FALSE, TRUE, 0);
        g_signal_connect(G_OBJECT(this->toggleToIconview()), "clicked", 
                BUTTON_CALLBACK(pagesignals_c::toggleToIconview), (void *)this);
        g_signal_connect(G_OBJECT(this->toggleToIconviewErr()), "clicked", 
                BUTTON_CALLBACK(pagesignals_c::toggleToIconview), (void *)this);
        g_signal_connect(G_OBJECT(this->toggleToTerminal()), "clicked", 
                BUTTON_CALLBACK(pagesignals_c::toggleToTerminal), (void *)this);
        g_signal_connect(G_OBJECT(this->sizeScale()), "change-value", 
                RANGE_CALLBACK(pagesignals_c::rangeChangeValue), (void *)this);
        g_signal_connect(G_OBJECT(this->clearButton()), "clicked", 
                BUTTON_CALLBACK(pagesignals_c::clearText), (void *)this);
        g_signal_connect(G_OBJECT(this->scriptButton()), "clicked", 
                BUTTON_CALLBACK(pagesignals_c::scriptRun), (void *)this);

        g_signal_connect(G_OBJECT(this->sizeScale()), "button-release-event", 
                EVENT_CALLBACK(pagesignals_c::rangeOff), (void *)this);

        
        g_signal_connect (this->topScrolledWindow(), "leave-notify-event", 
                EVENT_CALLBACK (pagesignals_c::leave_notify_event),
                (void *)this);

        g_signal_connect(G_OBJECT(this->vpane()), "button-release-event", 
                EVENT_CALLBACK(pagesignals_c::paneRelease), (void *)this);


        print_c::setColor(GTK_WIDGET(this->output()));
        // Data for lpterm
        this->setCompletionOutput(this->output());
        this->setCompletionInput(this->input());
        this->setLPTermPage(this);
        
        // This is for the hacky stderr output in print.hh
        g_object_set_data(G_OBJECT(this->vpane()), "toggleToIconview", this->toggleToIconview());
        g_object_set_data(G_OBJECT(this->vpane()), "toggleToIconviewErr", this->toggleToIconviewErr());
        g_object_set_data(G_OBJECT(this->vpane()), "fmButtonBox", this->fmButtonBox());
        g_object_set_data(G_OBJECT(this->vpane()), "termButtonBox", this->termButtonBox());

        //gtk_widget_realize(GTK_WIDGET(pageChild_));
        gtk_widget_show_all(GTK_WIDGET(pageLabelBox_));
        gtk_widget_show_all(GTK_WIDGET(pageChild_));
        return;
    }
    
    ~Page(void){
        Notebook<Type>::unreference_textview(this->output());
        auto view = (View<Type> *)
            g_object_get_data(G_OBJECT(this->topScrolledWindow()), "view");
            if (view){
            TRACE("now deleting View...\n");
            delete view;
        } else {
            DBG("not deleting View object\n");
        }
    }

    void updateStatusLabel(const gchar *text){
        this->setStatusLabel(text);
    }
      

    pid_t command(const gchar *command){
        return (this->run_lp_command(this->output(), this->workDir(), command, TRUE, FALSE));
    }

    pid_t command(const gchar *command, const gchar *workdir){
        return (this->run_lp_command(this->output(), workdir, command, TRUE, FALSE));
    }    
    
    void scriptRun(void){
            gchar *command = print_c::get_current_text(this->input());
            gchar *response=NULL;
            if (!command || !strlen(command)){
                g_free(command);
                command = g_strdup("ls --color -Flah");
                response = g_strdup("/dev/null");
            } else {
                //Dialog for script target
                auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Confirm Replace"), "video-x-generic");
                auto markup = 
                    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>script: %s</b></span>", _("Destination File"));  
                
                entryResponse->setResponseLabel(markup);
                g_free(markup);  
                entryResponse->setEntryDefault("/dev/null");
                response = entryResponse->runResponse();
                TRACE("response=%s\n", response); 
                if (!response){
                    g_free(command);
                    return;
                }
                if (strcmp(response,"/dev/null") && g_file_test(response, G_FILE_TEST_EXISTS)){
                    auto warning = g_strdup_printf(_("The output file %s already exists, do you want to overwrite it?"), response);
                    auto confirmResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Confirm Replace"), "dialog-warning");
                    confirmResponse->setResponseLabel(warning);
                    auto confirm = confirmResponse->runResponse();
                    delete confirmResponse;
                    if (!confirm) {
                        g_free(command);
                        g_free(response);
                        return;
                    }
                }
            }

            gchar *g = g_strdup_printf("script -f -c \"%s\" %s", command, response);
            g_free(command);
            command = g;
            this->csh_clean_start();
            this->run_lp_command(this->output(), this->workDir(), command, TRUE, FALSE);
            this->csh_save_history(command);
            print_c::clear_text(this->input());
            g_free(command);
            g_free(response);
    }

    void 
    newRunButton(const gchar * command, pid_t child){
        TRACE("page->newRunButton\n");
        gboolean shellIcon = run_c::run_in_shell(command);
        auto runButton = (runbutton_c *)new(runbutton_c);
        runButton->setup((void *)this, command, child, shellIcon);
        //runButton->make_run_data_button(runButton);
        //auto runButton = (runbutton_c *)new(runbutton_c(this, command, child, shellIcon));
        parent()->reference_run_button((void *)runButton);
        // final creation will occur with context function.
    }
    
    const gchar *pageWorkdir(void){return (const gchar *)this->workDir();}
    
    static     gchar *
    get_tab_name (const gchar *path) {
        gchar *name;
        if(!path) {
            name = util_c::utf_string (g_get_host_name());
        } else if(g_path_is_absolute(path) &&
                g_file_test (path, G_FILE_TEST_EXISTS)) {
            gchar *basename = g_path_get_basename (path);
            gchar *b = util_c::utf_string (basename);   // non chopped

            //util_c::chop_excess (pathname);
            gchar *dirname= g_path_get_dirname(path);
            basename=g_path_get_basename(dirname);
            gchar *updirname = g_path_get_dirname(dirname);
            gchar *upbasename=g_path_get_basename(updirname);
            gchar *choppedName;
            if (strcmp(upbasename, G_DIR_SEPARATOR_S)!=0){
                choppedName = g_strconcat(upbasename, G_DIR_SEPARATOR_S, NULL);
            } else {
                choppedName = g_strdup(G_DIR_SEPARATOR_S);
            }
            if (strcmp(basename, G_DIR_SEPARATOR_S)!=0){
                gchar *g = g_strconcat(choppedName, basename, G_DIR_SEPARATOR_S, NULL);
                g_free(choppedName);
                choppedName = g;
            }
            if (util_c::chopBeginning(choppedName)) {
                gchar *g = g_strconcat("...", choppedName, NULL);
                g_free(choppedName);
                choppedName = g;

            }
            gchar *q = util_c::utf_string (choppedName);   // non chopped

            g_free (basename);
            g_free (upbasename);
            g_free (dirname);
            g_free (updirname);
            g_free (choppedName);
            //name = g_strconcat (b, " (", q, ")", NULL);
            name = g_strconcat (b, NULL);
            g_free (q);
            g_free (b);
        } else {
            name = util_c::utf_string (path);
            util_c::chop_excess (name);
        }

        return (name);
    }
    void setDialogTitle(void){
        gchar *gg = completion_c::get_terminal_name(this->workDir());
        auto user = g_get_user_name();
        auto host = g_strdup(g_get_host_name());
        if (strchr(host, '.')) *strchr(host, '.')=0;
        gchar *g = g_strconcat(user,"@",host,":",gg, NULL);
        g_free(host);
        g_free(gg); 
        auto dialog = (dialog_c *)parent_;
        dialog->setDialogTitle(g);
        g_free(g);
    }
    void setPageWorkdir(const gchar *dir){
        // Avoid multiple resets...
        static gboolean first = TRUE;
        gboolean reset = (g_file_test(dir, G_FILE_TEST_IS_DIR) &&
            (this->workDir() && strcmp(dir, this->workDir())));
        if (!reset && !first) return;
        first = FALSE;
        //if (this->workDir() && strcmp(dir, this->workDir())==0) return;
        TRACE("setPageWorkdir: %s\n", dir);
        this->setWorkDir(dir);
        TRACE("update_pathbar: %s\n", dir);
        this->update_pathbar(dir);
        //INFO_("cd %s\n", dir);
        gchar *g = get_tab_name(this->workDir());
        setPageLabel(g);
        g_free(g);
        setDialogTitle();
    }

    void setTabIcon(const gchar *icon){
         gtk_c::set_container_image(GTK_CONTAINER(pageLabelIconBox_), icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
   }
     void setPageLabel(const gchar *text){
         gtk_label_set_markup(pageLabel_, text);
         //gtk_notebook_set_tab_label(notebook_, child, GTK_WIDGET(label));
   }
    
    void setVpanePosition(gint position){
        gtk_paned_set_position (this->vpane(), position);
        gint max;
        g_object_get(G_OBJECT(this->vpane()), "max-position", &max, NULL);
         g_object_set_data(G_OBJECT(this->vpane()), "oldCurrent", GINT_TO_POINTER(position));
        g_object_set_data(G_OBJECT(this->vpane()), "oldMax", GINT_TO_POINTER(max));   
    }
   
    void set_spinner(gboolean state)
    {
        if (!state){
            gtk_spinner_stop (pageLabelSpinner_);
            gtk_widget_show(GTK_WIDGET(pageLabelIconBox_));
            return;
        }
        gtk_widget_hide(GTK_WIDGET(pageLabelIconBox_));  
        gtk_spinner_start (pageLabelSpinner_);
    }

    
    void setDefaultIconview(gboolean state){iconviewIsDefault_ = state;}
    gboolean iconviewIsDefault(void){return iconviewIsDefault_;}
    
    void showFmButtonBox(gboolean err){
      if (!err) {
        this->showFmBox();
        terminalMode_ = FALSE;
      } else {
         this->showTermBox(err);
        terminalMode_ = FALSE;
      }
   }
    
    void showIconview(gint state){
        if (!gtk_widget_is_visible(GTK_WIDGET(this->pageChild_))){
                TRACE("page2.hh:: showIconview() call with invisible parent\n");
                return;
        }

        if (state > 0) {
            showFmButtonBox(FALSE);
            print_c::hide_text(this->output());
        } else  {
            this->showTermBox(FALSE);
            while (gtk_events_pending())gtk_main_iteration();
            if (state == 0) setVpanePosition(0);
            else {
                // Get vpane position...
                auto currentPosition = gtk_paned_get_position (this->vpane());
                TRACE("current vpane position = %d\n", currentPosition);
                GtkAllocation allocation;
                gtk_widget_get_allocation(GTK_WIDGET(this->hViewBox()),&allocation);
                TRACE("current hViewBox height = %d\n", allocation.height);
                if (allocation.height - currentPosition < 5){
                    gint position = settings_c::getInteger("window", "height");
                    if (position < 0) position = 200;
                    else position /= 2;
                    setVpanePosition(position);
                }
            }
            terminalMode_ = TRUE;
        } 

    }
public:
    gint
    keyboardEvent( GdkEventKey * event) {


        TRACE("page keyboard_event\n");

        // <ESC> Toggles to iconview mode
        // <TAB> Toggles to terminal mode (partial output)
        if (terminalMode_){
            if (event->keyval == GDK_KEY_Escape) {
                showIconview(1);
                return TRUE;
            }
        } else { // in iconview mode: toggle to terminal
            if (event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_Escape) {
                showIconview(0);
                return TRUE;
            }
            // These navigation keys belong to iconview.
            /*gint navigationKeys[]={
                GDK_KEY_Page_Up, GDK_KEY_KP_Page_Up,
                GDK_KEY_Page_Down, GDK_KEY_KP_Page_Down,
                GDK_KEY_Right, GDK_KEY_KP_Right,
                GDK_KEY_Left, GDK_KEY_KP_Left,
                GDK_KEY_Up, GDK_KEY_KP_Up,
                GDK_KEY_Down, GDK_KEY_KP_Down,
                GDK_KEY_Return, GDK_KEY_KP_Enter,
                -1};*/
            gint navigationKeys[]={
                GDK_KEY_Page_Up, GDK_KEY_KP_Page_Up,
                GDK_KEY_Page_Down, GDK_KEY_KP_Page_Down,
                GDK_KEY_Right, GDK_KEY_KP_Right,
                GDK_KEY_Left, GDK_KEY_KP_Left,
                GDK_KEY_Return, GDK_KEY_KP_Enter,
                -1};
            for (int i=0; navigationKeys[i] > 0; i++){
                if (event->keyval == navigationKeys[i]) {
                    TRACE("navigation key\n");
                    return view()->keyboardEvent(event);
                }
            }
            // Any other key activates terminal (partial output).
            showIconview(-1);
            // Send key to completion.
        }
        return this->completionKeyboardEvent(event);
    }

    
    void setSizeScale(gint size){
        gtk_range_set_value(GTK_RANGE(this->sizeScale()), size);
    }

    gint fontSize(void){
        auto range = GTK_RANGE(this->sizeScale());
        gdouble value = gtk_range_get_value (range);
        gint round = value + 0.5;
        return round;
    }
        
    static void
    set_font_family (GtkWidget * widget, const gchar *in_family, gboolean fixed) {
        if (!in_family) g_error("in_family cannot be NULL\n");
        if (!GTK_IS_WIDGET(widget)) return;
        auto family = (gchar *) g_object_get_data(G_OBJECT(widget), "font-family");
        auto fontsize = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),        "fontsize"));

        gint newsize=8; // default font size.
        const gchar *p;
        if (fixed) p = getenv ("RFM_FIXED_FONT_SIZE");
        else p = getenv ("RFM_VARIABLE_FONT_SIZE");
        if(p && strlen (p)) {
            errno=0;
            long value = strtol(p, NULL, 0);
            if (errno == 0){
                newsize = value;
            }
        }

        if(newsize != fontsize || !family || strcmp(family, in_family)) {
            //TRACE(stderr, "XXX setting %s fontsize  %d -> %d \n", in_family, fontsize, newsize);
            if (!family || strcmp(family, in_family)) {
                g_free(family);
                family = g_strdup(in_family);
                g_object_set_data(G_OBJECT(widget), "font-family", family);
            }
            fontsize = newsize;
            g_object_set_data(G_OBJECT(widget), 
                    "fontsize", GINT_TO_POINTER(fontsize));

            GtkStyleContext *style_context = gtk_widget_get_style_context (widget);
            gtk_style_context_add_class(style_context, GTK_STYLE_CLASS_VIEW );
            GtkCssProvider *css_provider = gtk_css_provider_new();
            GError *error=NULL;
            gchar *data = g_strdup_printf("* {\
    font-family: %s;\
    font-size: %dpx;\
    }", family, fontsize);
            gtk_css_provider_load_from_data (css_provider, data, -1, &error);
            g_free(data);
            if (error){
                ERROR("fm/dialog/notebook/page/page.hh:: set_font_family(%s, %d): %s\n", family, fontsize, error->message);
                g_error_free(error);
            } else {
                gtk_style_context_add_provider (style_context, GTK_STYLE_PROVIDER(css_provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            }
             


        }
    }
};

}



#endif

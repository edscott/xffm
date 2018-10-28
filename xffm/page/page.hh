#ifndef XF_PAGE_CHILD
#define XF_PAGE_CHILD
#include "common/run.hh"
#include "common/util.hh"
#include "completion/completion.hh"
#include "vbuttonbox.hh"
#include "hbuttonbox.hh"
#include "vpane.hh"
#include "threadcontrol.hh"
#include "runbutton.hh"
#include "pathbar.hh"
#include "pagebase.hh"
#include "signals/page.hh"

namespace xf{

template <class Type>
class Page :
    public Completion<Type>,
    public ThreadControl<Type>,
    public Pathbar<Type>,
    public PageBase<Type>,
    protected Vpane<Type>,
    protected HButtonBox<Type>,
    protected VButtonBox<Type>
{
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
    using run_c = Run<double>;
    using print_c = Print<double>;
private:
    GList *run_button_list;
    pthread_mutex_t *rbl_mutex;
    Dialog<Type> *parent_;
#ifdef XFFM_CC	
    BaseView<Type> *baseView_;
#endif
    

public:

    Page(Dialog<Type> *parent, const gchar *workdir):Pathbar<Type>(workdir){
	parent_ = parent;
	pageChild_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
	pageLabel_ = GTK_LABEL(gtk_label_new ("foobar"));
	pageLabelBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	pageLabelSpinner_ = GTK_SPINNER(gtk_spinner_new());
	pageLabelSpinnerBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	pageLabelIconBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	pageLabelButton_ = GTK_BUTTON(gtk_button_new ());

	gtk_box_pack_start (pageLabelBox_, GTK_WIDGET(pageLabelSpinnerBox_), TRUE, TRUE, 0);
	gtk_box_pack_start (pageLabelSpinnerBox_, GTK_WIDGET(pageLabelSpinner_), TRUE, TRUE, 0);
	gtk_box_pack_start (pageLabelBox_, GTK_WIDGET(pageLabelIconBox_), TRUE, TRUE, 0);
	gtk_box_pack_start (pageLabelBox_, GTK_WIDGET(pageLabel_), TRUE, TRUE, 2);
	gtk_box_pack_end (pageLabelBox_, GTK_WIDGET(pageLabelButton_), TRUE, TRUE, 0);
        gtk_c::setup_image_button(pageLabelButton_, "window-close-symbolic", _("Close"));
    
	//set_spinner(pageSpinner_, TRUE, pageLabelIconBox_);
	set_spinner(FALSE);

	gtk_box_pack_start (pageChild_, this->pathbar(), FALSE, FALSE, 0);

	//gtk_widget_show_all(GTK_WIDGET(pageLabelBox_));

        GtkBox *hViewBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

	gtk_box_pack_start (hViewBox, GTK_WIDGET(this->vpane_), TRUE, TRUE, 0);
	//gtk_box_pack_start (hViewBox, GTK_WIDGET(vButtonBox_), FALSE, FALSE, 0);
	gtk_box_pack_start (pageChild_, GTK_WIDGET(hViewBox), TRUE, TRUE, 0);
	gtk_box_pack_start (pageChild_, GTK_WIDGET(this->hButtonBox_), FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(this->toggleToIconview_), "clicked", 
                BUTTON_CALLBACK(PageSignals<Type>::toggleToIconview), (void *)this);
        g_signal_connect(G_OBJECT(this->toggleToTerminal_), "clicked", 
                BUTTON_CALLBACK(PageSignals<Type>::toggleToTerminal), (void *)this);
        g_signal_connect(G_OBJECT(this->sizeScale_), "change-value", 
                RANGE_CALLBACK(PageSignals<Type>::rangeChangeValue), (void *)this);
        g_signal_connect(G_OBJECT(this->clearButton_), "clicked", 
                BUTTON_CALLBACK(PageSignals<Type>::clearText), (void *)this);
        g_signal_connect(G_OBJECT(this->scriptButton_), "clicked", 
                BUTTON_CALLBACK(PageSignals<Type>::scriptRun), (void *)this);

        g_signal_connect(G_OBJECT(this->sizeScale_), "button-release-event", 
                EVENT_CALLBACK(PageSignals<Type>::rangeOff), (void *)this);

#ifdef XFFM_CC
        g_signal_connect (this->top_scrolled_window_, "leave-notify-event", 
                EVENT_CALLBACK (PageSignals<Type>::leave_notify_event),
		(void *)this);
#endif


	// Data for lpterm
        this->setOutput(this->output_);
        this->setInput(this->input_);
        this->setPage(this);

	pthread_mutexattr_t r_attr;
	pthread_mutexattr_init(&r_attr);
	pthread_mutexattr_settype(&r_attr, PTHREAD_MUTEX_RECURSIVE);
	rbl_mutex = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
	pthread_mutex_init(rbl_mutex, &r_attr);
	run_button_list = NULL;


	print_c::setColor(GTK_WIDGET(this->output()));
	//gtk_widget_realize(GTK_WIDGET(pageChild_));
	gtk_widget_show_all(GTK_WIDGET(pageLabelBox_));
	gtk_widget_show_all(GTK_WIDGET(pageChild_));
	return;
    }
    
    ~Page(void){
	GList *l = run_button_list;
	pthread_mutex_lock(rbl_mutex);
	for (; l && l->data; l=l->next){
	    //run_button_c *rb_p = (run_button_c *)l->data;
	    unreference_run_button(l->data);
	}
	g_list_free(run_button_list);
	run_button_list=NULL;
	pthread_mutex_unlock(rbl_mutex);
	pthread_mutex_destroy(rbl_mutex);
	g_free(rbl_mutex);
#ifdef XFFM_CC
	auto baseView = (BaseView<Type> *)
	    g_object_get_data(G_OBJECT(top_scrolled_window()), "baseView");
    	if (baseView){
            WARN("now deleting BaseView...\n");
            delete baseView;
        } else {
            ERROR("not deleting BaseView object\n");
        }


#endif
    }

#ifdef XFFM_CC
    BaseView<Type> *baseView(void){ return baseView_;}
#endif
    Dialog<Type> *parent(void){return parent_;}

    void command(const gchar *command){
        this->run_lp_command(this->output(), this->workDir(), command);
    }
    
    void scriptRun(void){
	    gchar *command = print_c::get_current_text(this->input());
            if (!command || !strlen(command)){
                g_free(command);
                command = g_strdup("ls --color -Flah");
            }
            gchar *g = g_strdup_printf("script -f -c \"%s\" /dev/null", command);
            g_free(command);
            command = g;
            this->csh_clean_start();
	    this->run_lp_command(this->output(), this->workDir(), command);
	    this->csh_save_history(command);
	    print_c::clear_text(this->input());
	    g_free(command);
    }

//    void reference_run_button(run_button_c *rb_p){
    void *reference_run_button(void *rb_p){
	DBG("reference_run_button(%p)\n", rb_p);
	pthread_mutex_lock(rbl_mutex);
	run_button_list = g_list_prepend(run_button_list, rb_p);
	pthread_mutex_unlock(rbl_mutex);
	return NULL;
    }

    void
    unreference_run_button(void *rb_p){
	DBG("unreference_run_button(%p)\n", rb_p);
	pthread_mutex_lock(rbl_mutex);
	void *p = g_list_find(run_button_list, rb_p);
	if (p){
	    run_button_list = g_list_remove(run_button_list, rb_p);
	    delete ((RunButton<Type> *)rb_p);
	}
	pthread_mutex_unlock(rbl_mutex);
    }

    void 
    newRunButton(const gchar * command, pid_t child){
	TRACE("page->newRunButton\n");
	gboolean shellIcon = run_c::run_in_shell(command);
	auto runButton = (RunButton<Type> *)new(RunButton<Type>);
	runButton->setup((void *)this, command, child, shellIcon);
	//runButton->make_run_data_button(runButton);
	//auto runButton = (RunButton<Type> *)new(RunButton<Type>(this, command, child, shellIcon));
	reference_run_button((void *)runButton);
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
            //iconname = g_strconcat (display_host, ":  ", b, " (", q, ")", NULL);
            name = g_strconcat (b, " (", q, ")", NULL);
            g_free (q);
            g_free (b);
        } else {
            name = util_c::utf_string (path);
            util_c::chop_excess (name);
        }

        return (name);
    }
    void setDialogTitle(void){
        gchar *gg = Completion<Type>::get_terminal_name(this->workDir());
        gchar *g = g_strconcat("xffm: ", gg, NULL);
        g_free(gg); 
        auto dialog = (Dialog<Type> *)parent_;
        dialog->setDialogTitle(g);
	g_free(g);
    }
    void setPageWorkdir(const gchar *dir){
	TRACE("setPageWorkdir: %s\n", dir);
	this->setWorkDir(dir);
	TRACE("update_pathbar: %s\n", dir);
	this->update_pathbar(dir);
        if (g_file_test(dir, G_FILE_TEST_IS_DIR)){
	    print_c::print(this->output(), "green", g_strdup_printf("cd %s\n", dir));
        }
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
	gtk_paned_set_position (this->vpane_, position);
        gint max;
	g_object_get(G_OBJECT(this->vpane_), "max-position", &max, NULL);
 	g_object_set_data(G_OBJECT(this->vpane_), "oldCurrent", GINT_TO_POINTER(position));
	g_object_set_data(G_OBJECT(this->vpane_), "oldMax", GINT_TO_POINTER(max));   
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
       
    void showIconview(gint state){
	if (!gtk_widget_is_visible(GTK_WIDGET(this->pageChild_))){
		TRACE("page2.hh:: showIconview() call with invisible parent\n");
		return;
	}

        if (state > 0) {
            gtk_widget_hide(GTK_WIDGET(this->toggleToIconview_));
            gtk_widget_hide(GTK_WIDGET(this->input_));
            gtk_widget_hide(GTK_WIDGET(this->termButtonBox_));

            gtk_widget_show(GTK_WIDGET(this->toggleToTerminal_));
            gtk_widget_show(GTK_WIDGET(this->statusButton_));
            print_c::hide_text(this->output_);
            terminalMode_ = FALSE;
        } else  {
            gtk_widget_hide(GTK_WIDGET(this->toggleToTerminal_));
            gtk_widget_hide(GTK_WIDGET(this->statusButton_));

            gtk_widget_show(GTK_WIDGET(this->toggleToIconview_));
            gtk_widget_show(GTK_WIDGET(this->input_));
            gtk_widget_show(GTK_WIDGET(this->termButtonBox_));
            while (gtk_events_pending())gtk_main_iteration();
            if (state == 0) setVpanePosition(0);
            else {
                gint position = parent_->getSettingInteger("window", "height");
                if (position < 0) position = 200;
                else position /= 2;
                setVpanePosition(position);
            }// print_c::show_text(this->output_);
            terminalMode_ = TRUE;
        } 

    }
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
                    WARN("navigation key\n");
#ifdef XFFM_CC
                    return baseView()->keyboardEvent(event);
#else
                    return FALSE;
#endif
                }
            }
            // Any other key activates terminal (partial output).
            showIconview(-1);
            // Send key to completion.
        }
        return this->completionKeyboardEvent(event);
    }

    
    void setSizeScale(gint size){
        gtk_range_set_value(GTK_RANGE(this->sizeScale_), size);
    }

    GtkBox *pageChild(void){ return pageChild_;}
    GtkLabel *pageLabel(void){ return pageLabel_;}
    GtkBox *pageLabelBox(void){ return pageLabelBox_;}
    GtkSpinner *pageLabelSpinner(void){ return pageLabelSpinner_;}
    GtkBox *pageLabelSpinnerBox(void){ return pageLabelSpinnerBox_;}
    GtkBox *pageLabelIconBox(void){ return pageLabelIconBox_;}
    GtkButton *pageLabelButton(void){ return pageLabelButton_;}
    GtkScrolledWindow *top_scrolled_window(void){ return this->top_scrolled_window_;};
    
    GtkPaned *vpane(void){return this->vpane_;}
    GtkTextView *input(void){ return this->input_;}
    GtkTextView *output(void){ return this->output_;}
    GtkButton *toggleToTerminal(void){return this->toggleToTerminal_;} 
    GtkButton *toggleToIconview(void){return this->toggleToIconview_;} 

    GtkBox *hButtonBox(void){return this->hButtonBox_;}   
    GtkBox *vButtonBox(void){return this->vButtonBox_;}   

private:
    GtkBox *pageChild_;
    GtkLabel *pageLabel_;
    GtkBox *pageLabelBox_;
    GtkSpinner *pageLabelSpinner_;
    GtkBox *pageLabelSpinnerBox_;
    GtkBox *pageLabelIconBox_;
    GtkButton *pageLabelButton_;

    gboolean terminalMode_;
    gboolean iconviewIsDefault_;

public:

    gint fontSize(void){
        auto range = GTK_RANGE(this->sizeScale_);
        gdouble value = gtk_range_get_value (range);
        gint round = value + 0.5;
        return round;
    }
        
    static void
    set_font_family (GtkWidget * widget, const gchar *in_family, gboolean fixed) {
	if (!in_family) g_error("in_family cannot be NULL\n");
	if (!GTK_IS_WIDGET(widget)) return;
	auto family = (gchar *) g_object_get_data(G_OBJECT(widget), "font-family");
	auto fontsize = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),	"fontsize"));

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
	    TRACE(stderr, "XXX setting %s fontsize  %d -> %d \n", in_family, fontsize, newsize);
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
		fprintf(stderr, "gerror: %s\n", error->message);
		g_error_free(error);
	    }
	    gtk_style_context_add_provider (style_context, GTK_STYLE_PROVIDER(css_provider),
				    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	     


	}
    }
};

}



#endif

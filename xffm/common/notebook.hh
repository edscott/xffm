#ifndef XF_NOTEBOOK
#define XF_NOTEBOOK
#include "menupopover.hh"
#include "page/page2.hh"

namespace xf {

template <class Type>
class notebookSignals {
public:
    /////////////  notebook specific signal bindings: /////////////////////////
#define NOTEBOOK_CALLBACK(X)  G_CALLBACK((void (*)(GtkNotebook *,GtkWidget *, guint, gpointer)) X)

    static void
    switch_page (GtkNotebook *notebook,
                   GtkWidget   *page,
                   guint        new_page,
                   gpointer     data)
    {
        TRACE("switch_page: new page=%d\n", new_page);


        //FIXME: what else?
        /*
        // This callback may occur after view has been destroyed.
        window_c *window_p = (window_c *)g_object_get_data(G_OBJECT(notebook), "window_p");
        if (!window_p->is_view_in_list(data)) {
            ERROR("switch_page:: view_p %p no longer exists.\n", data);
            return;
        }

        view_c *view_p = (view_c *)data;
        gint current_page = gtk_notebook_get_current_page (notebook);
        TRACE("switch_page, page_num=%d current_page=%d xfdir_p=%p\n" ,new_page, current_page, view_p->get_xfdir_p());
        if (!view_p->get_xfdir_p()) return;
        view_p->set_window_title(new_page);
        view_p->set_application_icon(new_page);
        */
    }

    static void
    page_added (GtkNotebook *notebook,
                   GtkWidget   *child,
                   guint        page_num,
                   gpointer     data)
    {
        TRACE("page_added\n");
    }

    static void
    page_removed (GtkNotebook *notebook,
                   GtkWidget   *child,
                   guint        page_num,
                   gpointer     data)
    {
        TRACE("page_removed\n");
    }

    static void
    page_reordered (GtkNotebook *notebook,
                   GtkWidget   *child,
                   guint        page_num,
                   gpointer     data)
    {
        TRACE("page_reordered\n");
    }
#define NOTEBOOK_1_CALLBACK(X)  G_CALLBACK((void (*)(GtkNotebook *,GtkDirectionType *,gpointer)) X)

    static void
    move_focus_out (GtkNotebook     *notebook,
                   GtkDirectionType arg1,
                   gpointer         data)
    {
        TRACE("move_focus_out\n");
    }

#define NOTEBOOK_2_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkNotebook *,GtkDirectionType *, gboolean, gpointer)) X)

    static gboolean
    reorder_tab (GtkNotebook     *notebook,
                   GtkDirectionType arg1,
                   gboolean         arg2,
                   gpointer         data)
    {
        TRACE("reorder_tab\n");
        return FALSE;
    }

#define NOTEBOOK_3_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkNotebook *,GtkNotebookTab *,gpointer)) X)
    static gboolean
    focus_tab (GtkNotebook   *notebook,
                   GtkNotebookTab arg1,
                   gpointer       data)
    {
        TRACE("focus_tab\n");
        return FALSE;
    }
        
#define NOTEBOOK_4_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkNotebook *, gint, gpointer)) X)
    static gboolean
    change_current_page (GtkNotebook *notebook,
                   gint         arg1,
                   gpointer     data)
    {
        TRACE("change_current_page\n");
        return FALSE;
    }

#define NOTEBOOK_5_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkNotebook *, GtkWidget   *, gint, gint, gpointer)) X)
    static GtkNotebook*
    create_window (GtkNotebook *notebook,
                   GtkWidget   *page,
                   gint         x,
                   gint         y,
                   gpointer     data)
    {
        TRACE("create_window\n");
        return NULL;
    }

#define NOTEBOOK_6_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkNotebook *, gboolean, gpointer)) X)
    static gboolean
    select_page (GtkNotebook *notebook,
                   gboolean     arg1,
                   gpointer     data)
    {
        TRACE("select_page\n");
        return FALSE;
    }

    static void
    on_new_page(GtkButton *button, void *data){
        auto notebook = (Notebook<Type> *)data;
        const gchar *workdir = notebook->workdir();
        TRACE("on_new_page this: %p (%s)\n", data, workdir);
#ifdef XFFM_CC
        auto dialog = (fmDialog<Type> *) data;
        dialog->addPage(notebook->workdir());
#else
        notebook->addPage(notebook->workdir());
#endif
    }

    static void
    on_remove_page(GtkButton *button, void *data){
        TRACE("on_remove_page this: %p\n", data);
        auto page = (Page<Type> *)data;
        auto notebook = (Notebook<Type> *)(g_object_get_data(G_OBJECT(page->pageChild()), "Notebook"));
        notebook->removePage(GTK_WIDGET(page->pageChild()));
    }


};

template <class Type>
class Notebook : public MenuPopover<Type>{
    using gtk_c = Gtk<double>;
    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
    using print_c = Print<double>;
public:
    Notebook(void){
        notebook_ = GTK_NOTEBOOK(gtk_notebook_new());
        g_object_set_data(G_OBJECT(this->menuButton_), "notebook_p", (void *)this);
        pageHash_ =g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
        gtk_notebook_set_scrollable (notebook_, TRUE);

        auto new_tab_button = GTK_BUTTON(gtk_button_new ());
        gtk_c::setup_image_button(new_tab_button, "tab-new-symbolic", _("Open a new tab (Ctrl+T)"));
//        gtk_c::setup_image_button(new_tab_button, "list-add", _("Open a new tab (Ctrl+T)"));
        gtk_widget_show(GTK_WIDGET(new_tab_button));

        auto button_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_box_pack_start(button_box, GTK_WIDGET(new_tab_button),  FALSE, FALSE, 0);
        gtk_box_pack_start(button_box, GTK_WIDGET(this->menuButton()),  FALSE, FALSE, 0);
        gtk_widget_show(GTK_WIDGET(button_box));
        gtk_notebook_set_action_widget (notebook_, GTK_WIDGET(button_box), GTK_PACK_END);
    
        g_signal_connect(G_OBJECT(new_tab_button), "clicked", 
                BUTTON_CALLBACK(notebookSignals<Type>::on_new_page), (void *)this); 
        //g_signal_connect (G_OBJECT(window), "destroy", G_CALLBACK (destroy), 
        //        NULL);
        signalSetup();
        gtk_widget_show (GTK_WIDGET(notebook_));
        return;

    }
    
    Page<double> *addPage(const gchar *workdir){
	gint oldPosition = -1;
	gboolean terminalMode;
	if (gtk_notebook_get_current_page (notebook_) >= 0){
	    oldPosition = gtk_paned_get_position(vpane());
	    auto w = currentPageObject()->toggleToIconview();
	    terminalMode = gtk_widget_is_visible(GTK_WIDGET(w));
	}
        auto page = new(Page<double>);
        g_object_set_data(G_OBJECT(page->pageChild()), "Notebook", (void *)this);

        // This will (and should) be set by the corresponding
	// page class template 
	// page->setPageLabel(g); 
        auto pageNumber = gtk_notebook_append_page (notebook_,
                          GTK_WIDGET(page->pageChild()),
                          GTK_WIDGET(page->pageLabelBox()));
        gtk_notebook_set_tab_reorderable (notebook_,GTK_WIDGET(page->pageChild()), TRUE);
        TRACE("******* added page number %d: child=%p\n", pageNumber, (void *)page->pageChild());
        g_hash_table_replace(pageHash_, (void *)page->pageChild(), (void *)page);
        gtk_notebook_set_current_page (notebook_,pageNumber);
	// This will set the workdir for completion
        page->setPageWorkdir(workdir);  
	
        g_signal_connect(G_OBJECT(page->pageLabelButton()), "clicked", 
                BUTTON_CALLBACK(notebookSignals<Type>::on_remove_page), (void *)page); 
	// If current page exists, use the same vpane position, otherwise
	// use default value 
	
	if (oldPosition < 0) {
	    page->showIconview(page->iconviewIsDefault(), TRUE);
	} else {
	    page->showIconview(!terminalMode, FALSE);
	    setVpanePosition(oldPosition);
	}
        return page;
    }

    void removePage(GtkWidget *child){
        auto pageNumber = gtk_notebook_page_num (notebook_, child);
        auto currentPage = gtk_notebook_get_current_page (notebook_);
        if (pageNumber < 0){
            ERROR("child %p is not in notebook\n", (void *)child);
            exit(1);
            return;
        }
        TRACE("disconnect page %d\n", pageNumber);
       //gtk_widget_hide(child);
        auto page = (Page<Type> *)g_hash_table_lookup(pageHash_, (void *)child);
        if (currentPage == pageNumber) {
            gtk_notebook_set_current_page (notebook_, pageNumber-1);
        } 

        gtk_widget_hide(child);
        g_hash_table_remove(pageHash_, (void *)child);
        while (gtk_events_pending()) gtk_main_iteration();
        if (g_hash_table_size(pageHash_) == 0){
            gtk_widget_hide(gtk_widget_get_toplevel(child));
        }
        while (gtk_events_pending()) gtk_main_iteration();
        if (g_hash_table_size(pageHash_) == 0){
            gtk_main_quit();
            exit(1);
        }

        delete(page);
        gtk_notebook_remove_page (notebook_, pageNumber);

        TRACE("******** deleted page with child %p\n", (void *)child);
    }


    void setTabIcon(GtkWidget *child, const gchar *icon){
        Page<Type> *page = (Page<Type> *)g_hash_table_lookup(pageHash_, (void *)child);
        if (!page){
            ERROR("setVpanePosition:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
            return;
        }
        page->setTabIcon(icon);
    }

    void setTabIcon(const gchar *icon){
        setTabIcon(currentPageChild(), icon);
    }

    void setVpanePosition(GtkWidget *child, gint position){
        Page<Type> *page = (Page<Type> *)g_hash_table_lookup(pageHash_, (void *)child);
        if (!page){
            ERROR("setVpanePosition:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
            return;
        }
        page->setVpanePosition(position);
    }

    void setVpanePosition(gint position){
        setVpanePosition(currentPageChild(), position);
    }

    const gchar *workdir(GtkWidget *child){
        Page<Type> *page = (Page<Type> *)g_hash_table_lookup(pageHash_, (void *)child);

        if (!page){
            ERROR("setVpanePosition:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
            return NULL;
        }
        return page->workDir();
    }

    gboolean setWorkdir(const gchar *dir){
        return  setWorkdir(currentPageChild(dir));
    }
    gboolean setWorkdir(GtkWidget *child, const gchar *dir){
        Page<Type> *page = (Page<Type> *)g_hash_table_lookup(pageHash_, (void *)child);

        if (!page){
            ERROR("setVpanePosition:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
            return NULL;
        }
        return page->setWorkDir(dir);
    }

    const gchar * workdir(void){
        return  workdir(currentPageChild());
    }
    GtkTextView *diagnostics(GtkWidget *child){
        Page<Type> *page = (Page<Type> *)g_hash_table_lookup(pageHash_, (void *)child);

        if (!page){
            ERROR("setVpanePosition:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
            return NULL;
        }
        return page->diagnostics();
    }

    GtkTextView * diagnostics(void){
        return  diagnostics(currentPageChild());
    }

    GtkPaned *vpane(GtkWidget *child){
        Page<Type> *page = (Page<Type> *)g_hash_table_lookup(pageHash_, (void *)child);

        if (!page){
            ERROR("setVpanePosition:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
            return NULL;
        }
        return page->vpane();
    }

    GtkPaned *vpane(void){
        return  vpane(currentPageChild());
    }

    gint currentPage(void){
        return gtk_notebook_get_current_page(notebook_);
    }
    Page<Type> *currentPageObject(void){
        auto child = currentPageChild();
        auto page = g_hash_table_lookup(pageHash_, (void *)child);
        return (Page<Type> *)page;
    }
    GtkWidget *currentPageChild(void){
        return gtk_notebook_get_nth_page (notebook_,currentPage());
    }

    void setPageLabel(const gchar *text){
        setPageLabel(currentPageChild(), text);
    }
    void setPageLabel(GtkWidget *child, const gchar *text){
         Page<Type> *page = (Page<Type> *)
             g_hash_table_lookup(pageHash_, (void *)child);
         page->setPageLabel(text);
         
         /*auto label = GTK_LABEL(gtk_label_new(""));
	 GtkWidget *child = gtk_notebook_get_nth_page (notebook_, pageNumber);
         //auto label = GTK_LABEL(g_object_get_data(G_OBJECT(child), "page_label"));
         gtk_label_set_markup(label, text);
         gtk_notebook_set_tab_label(notebook_, child, GTK_WIDGET(label));*/
    }
    GtkNotebook *notebook(void){ return notebook_;}
    void insertNotebook(GtkWindow *window){
        if (!window) return;
        g_object_set_data(G_OBJECT(window), "notebook", (void *)notebook_);
        gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET(notebook_));
    }
private:
    GtkNotebook *notebook_;
    GHashTable *pageHash_;
    void signalSetup(void){
        // notebook specific signal bindings:
        g_signal_connect (notebook_, "change-current-page", 
                NOTEBOOK_4_CALLBACK (notebookSignals<double>::change_current_page), NULL);
        g_signal_connect (notebook_, "create-window", 
                NOTEBOOK_5_CALLBACK (notebookSignals<double>::create_window), NULL);
        g_signal_connect (notebook_, "focus-tab", 
                NOTEBOOK_3_CALLBACK (notebookSignals<double>::focus_tab), NULL);
        g_signal_connect (notebook_, "move-focus-out", 
                NOTEBOOK_1_CALLBACK (notebookSignals<double>::move_focus_out), NULL);
        g_signal_connect (notebook_, "page-added", 
                NOTEBOOK_CALLBACK (notebookSignals<double>::page_added), NULL);
        g_signal_connect (notebook_, "page-removed", 
                NOTEBOOK_CALLBACK (notebookSignals<double>::page_removed), NULL);
        g_signal_connect (notebook_, "page-reordered", 
                NOTEBOOK_CALLBACK (notebookSignals<double>::page_reordered), NULL);
        g_signal_connect (notebook_, "reorder-tab", 
                NOTEBOOK_2_CALLBACK (notebookSignals<double>::reorder_tab), NULL);
        g_signal_connect (notebook_, "select-page", 
                NOTEBOOK_6_CALLBACK (notebookSignals<double>::select_page), NULL);
        g_signal_connect (notebook_, "switch-page", 
                NOTEBOOK_CALLBACK (notebookSignals<double>::switch_page), NULL);

    }
#if 0    
    void
    view_c::remove_page(void){
        gint page_num = gtk_notebook_page_num (notebook_, get_page_child());
        gint current_page = gtk_notebook_get_current_page (get_notebook());

        gtk_notebook_remove_page (get_notebook(), page_num);
        if (current_page == page_num) {
            gtk_notebook_set_current_page (get_notebook(), page_num-1);
        }    
    }

void
view_c::set_page_label(void){
    gchar *tab_label = g_path_get_basename(xfdir_p->get_label());
    gtk_label_set_markup(GTK_LABEL(get_page_label()), tab_label);
    g_free(tab_label);
}

void
view_c::set_view_details(void){
    set_page_label();
    set_window_title();
    set_application_icon();
    update_tab_label_icon();
    while (gtk_events_pending()) gtk_main_iteration();
    update_pathbar(xfdir_p->get_path());
}

void
view_c::update_tab_label_icon(void){
    GList *children = 
	gtk_container_get_children (GTK_CONTAINER(get_page_label_icon_box()));
    GList *l = children;
    for (;l && l->data; l=l->next){
	 gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);
    const gchar *icon_name = xfdir_p->get_xfdir_iconname();
    GdkPixbuf *pixbuf = 
            pixbuf_c::get_pixbuf(icon_name, GTK_ICON_SIZE_BUTTON);
    if (pixbuf){
	GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
	gtk_container_add (GTK_CONTAINER (get_page_label_icon_box()), image);
	gtk_widget_show(image);
    }
}

void
view_c::set_application_icon (void) {
    const gchar *iconname = xfdir_p->get_xfdir_iconname();
    GdkPixbuf *icon_pixbuf = pixbuf_c::get_pixbuf (iconname, GTK_ICON_SIZE_DIALOG);
    if(icon_pixbuf) {
	GtkWindow *window = ((window_c *)get_window_v())->get_window();
        gtk_window_set_icon (window, icon_pixbuf);
    }
    // FIXME add to tab label (not here...)
}

void
view_c::set_application_icon (gint page_num) {
    GtkWidget *child_box = gtk_notebook_get_nth_page(get_notebook(), page_num);
    view_c *view_p = (view_c *)g_object_get_data(G_OBJECT(child_box), "view_p");
    if (!view_p->get_xfdir_p()) return;
    
    const gchar *iconname = view_p->get_xfdir_p()->get_xfdir_iconname();
    GdkPixbuf *icon_pixbuf = pixbuf_c::get_pixbuf (iconname, GTK_ICON_SIZE_DIALOG);
    if(icon_pixbuf) {
	GtkWindow *window = ((window_c *)get_window_v())->get_window();
        gtk_window_set_icon (window, icon_pixbuf);
    }
    // FIXME add to tab label (not here...)
}

void
view_c::set_window_title(void){
    gchar *window_title = xfdir_p->get_window_name();
    GtkWindow *window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(get_notebook())));
    gtk_window_set_title (window, window_title);
    g_free (window_title);

}

void
view_c::set_window_title(gint page_num){
    GtkWidget *child_box = gtk_notebook_get_nth_page(get_notebook(), page_num);
    view_c *view_p = (view_c *)g_object_get_data(G_OBJECT(child_box), "view_p");
    if (!view_p->get_xfdir_p()) {
        fprintf(stderr, "view_c::set_window_title(gint page_num): no xfdir_p\n");
        return;
    }

    gchar *window_title = view_p->get_xfdir_p()->get_window_name();
    GtkWindow *window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(get_notebook())));
    gtk_window_set_title (window, window_title);
    g_free (window_title);

}

static void 
on_remove_page_button(GtkWidget *b, gpointer data){
    view_c *view_p = (view_c *)data;
    view_p->remove_page();       
    // delete object: (remove from view_list)
    window_c *window_p = (window_c *)view_p->get_window_v();
    window_p->remove_view_from_list(data); // this calls view_c destructor
}

#endif

};

}

#endif

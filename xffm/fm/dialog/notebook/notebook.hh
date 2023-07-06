#ifndef XF_NOTEBOOK
#define XF_NOTEBOOK
#include "signals.hh"
#include "menupopover.hh"
#include "vbuttonbox.hh"
#include "page/page.hh"

    
static GList *textview_list=NULL;

namespace xf {
template <class Type> class Dialog;

template <class Type>
class Notebook : 
    public VButtonBox<double>,
    public MenuPopover<Type>
{
    using gtk_c = Gtk<double>;
    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
    using print_c = Print<double>;
    GList *run_button_list=NULL;
    pthread_mutex_t *rbl_mutex;
public:

    ~Notebook(void){
        GList *l = run_button_list;
        pthread_mutex_lock(rbl_mutex);
        for (; l && l->data; l=l->next){
            unreference_run_button(l->data);
        }
        g_list_free(run_button_list);
        run_button_list=NULL;
        pthread_mutex_unlock(rbl_mutex);
        pthread_mutex_destroy(rbl_mutex);
        g_free(rbl_mutex);
    }

    Notebook(void){
        pthread_mutexattr_t r_attr;
        pthread_mutexattr_init(&r_attr);
        pthread_mutexattr_settype(&r_attr, PTHREAD_MUTEX_RECURSIVE);
        rbl_mutex = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(rbl_mutex, &r_attr);
        run_button_list = NULL;

        notebook_ = GTK_NOTEBOOK(gtk_notebook_new());
        g_object_set_data(G_OBJECT(this->menuButton_), "notebook_p", (void *)this);
        pageHash_ =g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
        gtk_notebook_set_scrollable (notebook_, TRUE);

        //auto pb = Pixbuf<Type>::getPixbuf ("format-justify-fill", TINY_BUTTON);
        //auto popupImage = gtk_image_new_from_pixbuf (pb);
        popupImage = gtk_label_new("");
        auto text = g_strdup_printf("<span color=\"red\">%s</span>",_("Long press time"));
        gtk_label_set_markup(GTK_LABEL(popupImage),text);
        g_free(text);


/*
        auto newTabPage = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_show(newTabPage);
        auto newTabButton = gtk_c::newButton("list-add-symbolic", _("New Tab"));
        

        gtk_widget_show(GTK_WIDGET(newTabButton));
        auto pageNumber = gtk_notebook_append_page (notebook_,
                          GTK_WIDGET(newTabPage),
                          GTK_WIDGET(newTabButton));
        gtk_notebook_set_tab_reorderable (notebook_,GTK_WIDGET(newTabPage), FALSE);        
*/
        auto actionWidget = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        auto tabButtonBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        auto newTabButton = gtk_c::newButton(NEW_TAB, _("New Tab"));
        g_signal_connect(G_OBJECT(newTabButton), "clicked", 
                BUTTON_CALLBACK(notebookSignals<Type>::on_new_page), (void *)this);    
        
        gtk_box_pack_start(actionWidget, GTK_WIDGET(tabButtonBox),  TRUE, FALSE, 0);
        gtk_box_pack_start(tabButtonBox, GTK_WIDGET(popupImage),  TRUE, FALSE, 0);
        gtk_box_pack_start(tabButtonBox, GTK_WIDGET(newTabButton),  TRUE, FALSE, 0);
        gtk_box_pack_start(tabButtonBox, GTK_WIDGET(this->menuButton()),  TRUE, FALSE, 0);

        gtk_widget_show_all(GTK_WIDGET(actionWidget));
        gtk_notebook_set_action_widget (notebook_, GTK_WIDGET(actionWidget), GTK_PACK_END);
    

        //g_signal_connect (G_OBJECT(window), "destroy", G_CALLBACK (destroy), 
        //        NULL);
        signalSetup();
        gtk_widget_show (GTK_WIDGET(notebook_));
        gtk_widget_hide(popupImage);
        // does not work:
        // g_object_set_data(G_OBJECT(mainWindow), "notebook_p", this);
        return;

    }

    View<Type> *loadIconview(Page<Type> *page, const gchar *path){
        if (!path) path = "xffm:root";
        TRACE("loadIconview::adding path: %s\n", path);
        // Create View object.
        auto view =  new View<Type>(page);
        g_object_set_data(G_OBJECT(page->topScrolledWindow()), "view", view);
        // Add the iconview into the scrolled window.
        
        gtk_container_add (GTK_CONTAINER (page->topScrolledWindow()),
                GTK_WIDGET(view->iconView()));
        gtk_widget_show (GTK_WIDGET(view->iconView()));

        gtk_container_add (GTK_CONTAINER (page->treeScrolledWindow()),
                GTK_WIDGET(view->treeView()));
        gtk_widget_show (GTK_WIDGET(view->treeView()));

        // Load contents, depending on what path specifies.
        view->loadModel(path);
        while (gtk_events_pending())gtk_main_iteration();
        return view;
    }

//    void reference_run_button(run_button_c *rb_p){
    void *reference_run_button(void *rb_p){
        TRACE("reference_run_button(%p)\n", rb_p);
        pthread_mutex_lock(rbl_mutex);
        run_button_list = g_list_prepend(run_button_list, rb_p);
        pthread_mutex_unlock(rbl_mutex);
        return NULL;
    }

    void
    unreference_run_button(void *rb_p){
        TRACE("unreference_run_button(%p)\n", rb_p);
        pthread_mutex_lock(rbl_mutex);
        void *p = g_list_find(run_button_list, rb_p);
        if (p){
            run_button_list = g_list_remove(run_button_list, rb_p);
            delete ((RunButton<Type> *)rb_p);
        }
        pthread_mutex_unlock(rbl_mutex);
    }

    static gboolean isValidTextView(void *textView){
        gboolean retval = FALSE;
        void *p = g_list_find(textview_list, textView);
        if (p) retval = TRUE;
        return retval;
    }

    static void *reference_textview(GtkTextView *textView){
        TRACE("reference_run_button(%p)\n", (void *)textView);
        textview_list = g_list_prepend(textview_list, (void *)textView);
        return NULL;
    }

    static void
    unreference_textview(GtkTextView *textView){
        TRACE("unreference_run_button(%p)\n", (void *)textView);
        void *p = g_list_find(textview_list, (void *)textView);
        if (p){
            textview_list = g_list_remove(textview_list, (void *)textView);
        }
    }

    Page<double> *addPage(const gchar *path){
        gchar *workdir;
        if (!path) {
            workdir = g_get_current_dir();
        } else {
            if (g_file_test(path, G_FILE_TEST_IS_DIR)){
                workdir = g_strdup(path);
            } else {
                workdir = g_strdup(g_get_home_dir());
            }
        }
        
        auto page = new(Page<Type>)((Dialog<Type> *)this, workdir);
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
        TRACE("Notebook::   addPage(%s)\n", workdir);      
        page->showIconview(1);
        
        g_signal_connect(G_OBJECT(page->pageLabelButton()), "clicked", 
                BUTTON_CALLBACK(notebookSignals<Type>::on_remove_page), (void *)page); 
        auto view = loadIconview(page, path);
        page->setView(view);
        page->setPageWorkdir(workdir); 
        g_free(workdir);

        auto size = Settings<Type>::getInteger("xfterm", "fontSize");
        print_c::set_font_size(GTK_WIDGET(page->output()), size);
        print_c::set_font_size(GTK_WIDGET(page->input()), size);
    
        /*if (!g_file_test(path, G_FILE_TEST_IS_DIR) 
                    || strcmp(path, "xffm:local")!=0) {
            page->setPageLabel(view->path());
        }*/
        return page;
    }

    void removePage(GtkWidget *child){
        auto pageNumber = gtk_notebook_page_num (notebook_, child);
        auto currentPage = gtk_notebook_get_current_page (notebook_);
        if (pageNumber < 0){
            ERROR("fm/dialog/notebook/notebook.hh::child %p is not in notebook\n", (void *)child);
            //exit(1);
            return;
        }
        TRACE("disconnect page %d\n", pageNumber);
       //gtk_widget_hide(child);
        auto page = (Page<Type> *)g_hash_table_lookup(pageHash_, (void *)child);
        auto view = page->view();
        view->disableMonitor();
        
        
        if (currentPage == pageNumber) {
            auto pages = gtk_notebook_get_n_pages (notebook_);
            gint nextPage = currentPage+1;
            if (nextPage > pages-1){
                nextPage = pages-1;
            } 
            gtk_notebook_set_current_page (notebook_, nextPage);
        } 

        gtk_widget_hide(child);
        g_hash_table_remove(pageHash_, (void *)child);
        while (gtk_events_pending()) gtk_main_iteration();
        if (g_hash_table_size(pageHash_) == 0){
            gtk_widget_hide(gtk_widget_get_toplevel(child));
        }
        while (gtk_events_pending()) gtk_main_iteration();
        if (g_hash_table_size(pageHash_) == 0){
            DialogSignals<Type>::delete_event(GTK_WIDGET(mainWindow), NULL, NULL);
            //gtk_main_quit();
            //exit(1);
        }
        
        delete(page);
        gtk_notebook_remove_page (notebook_, pageNumber);

        TRACE("******** deleted page with child %p\n", (void *)child);
    }


    void setTabIcon(GtkWidget *child, const gchar *icon){
        Page<Type> *page = (Page<Type> *)g_hash_table_lookup(pageHash_, (void *)child);
        if (!page){
            ERROR("fm/dialog/notebook/notebook.hh::setTabIcon:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
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
            ERROR("fm/dialog/notebook/notebook.hh::setVpanePosition:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
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
            ERROR("fm/dialog/notebook/notebook.hh::workdir:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
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
            ERROR("fm/dialog/notebook/notebook.hh::setWorkdir:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
            return FALSE;
        }
        return page->setWorkDir(dir);
    }

    const gchar * workdir(void){
        return  workdir(currentPageChild());
    }
    GtkTextView *diagnostics(GtkWidget *child){
        Page<Type> *page = (Page<Type> *)g_hash_table_lookup(pageHash_, (void *)child);

        if (!page){
            ERROR("fm/dialog/notebook/notebook.hh::diagnostics:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
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
            ERROR("fm/dialog/notebook/notebook.hh::vpane:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
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
     Page<Type> *currentPageObject(gint pageNum){
        auto child = gtk_notebook_get_nth_page (notebook_,pageNum);
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
        auto box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET(box));
        gtk_box_pack_start (box, GTK_WIDGET(notebook_), TRUE, TRUE, 0);
        gtk_box_pack_end (box, GTK_WIDGET(this->vButtonBox()), FALSE, FALSE, 0);
        gtk_widget_show(GTK_WIDGET(box));
        
    }
    
private:
    GtkNotebook *notebook_;
    GHashTable *pageHash_;
    void signalSetup(void){
        // notebook specific signal bindings:
        g_signal_connect (notebook_, "change-current-page", 
                NOTEBOOK_4_CALLBACK (notebookSignals<double>::change_current_page), (void *)this);
        g_signal_connect (notebook_, "create-window", 
                NOTEBOOK_5_CALLBACK (notebookSignals<double>::create_window), (void *)this);
        g_signal_connect (notebook_, "focus-tab", 
                NOTEBOOK_3_CALLBACK (notebookSignals<double>::focus_tab), (void *)this);
        g_signal_connect (notebook_, "move-focus-out", 
                NOTEBOOK_1_CALLBACK (notebookSignals<double>::move_focus_out), (void *)this);
        g_signal_connect (notebook_, "page-added", 
                NOTEBOOK_CALLBACK (notebookSignals<double>::page_added), (void *)this);
        g_signal_connect (notebook_, "page-removed", 
                NOTEBOOK_CALLBACK (notebookSignals<double>::page_removed), (void *)this);
        g_signal_connect (notebook_, "page-reordered", 
                NOTEBOOK_CALLBACK (notebookSignals<double>::page_reordered), (void *)this);
        g_signal_connect (notebook_, "reorder-tab", 
                NOTEBOOK_2_CALLBACK (notebookSignals<double>::reorder_tab), (void *)this);
        g_signal_connect (notebook_, "select-page", 
                NOTEBOOK_6_CALLBACK (notebookSignals<double>::select_page), (void *)this);
        g_signal_connect (notebook_, "switch-page", 
                NOTEBOOK_CALLBACK (notebookSignals<double>::switch_page), (void *)this);

    }
#if 0    

void
view_c::set_application_icon (void) {
    const gchar *iconname = xfdir_p->get_xfdir_iconname();
    GdkPixbuf *icon_pixbuf = pixbuf_c::getPixbuf (iconname, GTK_ICON_SIZE_DIALOG);
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
    GdkPixbuf *icon_pixbuf = pixbuf_c::getPixbuf (iconname, GTK_ICON_SIZE_DIALOG);
    if(icon_pixbuf) {
        GtkWindow *window = ((window_c *)get_window_v())->get_window();
        gtk_window_set_icon (window, icon_pixbuf);
    }
    // FIXME add to tab label (not here...)
}

#endif

};

}

#endif

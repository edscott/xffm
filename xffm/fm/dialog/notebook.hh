#ifndef XF_NOTEBOOK
#define XF_NOTEBOOK
#include "fm/dialog/menupopover.hh"
#include "fm/page/page.hh"
#include "fm/dialog/notebooksignals.hh"

namespace xf {
template <class Type> class Dialog;

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

    BaseView<Type> *loadIconview(Page<Type> *page, const gchar *path){
	if (!path) path = "xffm:root";
	WARN("loadIconview::adding path: %s\n", path);
	// Create BaseView object.
        auto baseView =  new BaseView<Type>(page);
        g_object_set_data(G_OBJECT(page->topScrolledWindow()), "baseView", baseView);
        // Add the iconview into the scrolled window.
	gtk_container_add (GTK_CONTAINER (page->topScrolledWindow()),
		GTK_WIDGET(baseView->iconView()));
	gtk_widget_show (GTK_WIDGET(baseView->iconView()));

        // Load contents, depending on what path specifies.
	baseView->loadModel(path);
	while (gtk_events_pending())gtk_main_iteration();
        return baseView;
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
	auto baseView = loadIconview(page, path);
	page->setBaseView(baseView);
        page->setPageWorkdir(workdir); 
	g_free(workdir);
	/*if (!g_file_test(path, G_FILE_TEST_IS_DIR) 
		    || strcmp(path, "xffm:local")!=0) {
	    page->setPageLabel(baseView->path());
	}*/
        return page;
    }

    void removePage(GtkWidget *child){
        auto pageNumber = gtk_notebook_page_num (notebook_, child);
        auto currentPage = gtk_notebook_get_current_page (notebook_);
        if (pageNumber < 0){
            ERROR("child %p is not in notebook\n", (void *)child);
            //exit(1);
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
            ERROR("setTabIcon:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
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
            ERROR("workdir:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
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
            ERROR("setWorkdir:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
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
            ERROR("diagnostics:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
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
            ERROR("vpane:: no hash entry for page number %d\n", gtk_notebook_page_num (notebook_, child));
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
                NOTEBOOK_CALLBACK (notebookSignals<double>::switch_page), (void *)this);

    }
#if 0    

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

#endif

};

}

#endif
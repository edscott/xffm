#ifndef XF_NOTEBOOK_SIGNALS
#define XF_NOTEBOOK_SIGNALS
static gint lastPage=-1;

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
	static gboolean startup = TRUE;
        DBG("switch_page: new page=%d last page=%d\n", new_page, lastPage);
        auto notebook_p = (Notebook<Type> *)data;
        auto page_p = (Page<Type> *)notebook_p->currentPageObject(new_page);
        if (!page_p){
            if (!startup) ERROR("page_p is null\n");
            return;
        }
	startup = FALSE;
        page_p->setDialogTitle();
        gtk_widget_set_sensitive(GTK_WIDGET(page_p->pageLabelButton()), TRUE);
        auto pages = gtk_notebook_get_n_pages(notebook);

        if (lastPage >= 0 && lastPage != new_page && lastPage < pages){
            page_p = (Page<Type> *)notebook_p->currentPageObject(lastPage);
            gtk_widget_set_sensitive(GTK_WIDGET(page_p->pageLabelButton()), FALSE);
        }
        lastPage = new_page;


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
        DBG("page_added\n");
        if (lastPage < 0)lastPage=0;
        /*auto notebook_p = (Notebook<Type> *)data;
        if (lastPage >= 0){
            auto page_p = (Page<Type> *)notebook_p->currentPageObject(lastPage);
            gtk_widget_set_sensitive(GTK_WIDGET(page_p->pageLabelButton()), FALSE);
        }
        lastPage=page_num;*/
    }

    static void
    page_removed (GtkNotebook *notebook,
                   GtkWidget   *child,
                   guint        page_num,
                   gpointer     data)
    {
        DBG("page_removed\n");
    }

    static void
    page_reordered (GtkNotebook *notebook,
                   GtkWidget   *child,
                   guint        page_num,
                   gpointer     data)
    {
        DBG("page_reordered page is %d, lastPage=%d\n", page_num, lastPage);
        lastPage = page_num;
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
	auto page = notebook->currentPageObject();
	auto view = page->view();
	
        //const gchar *workdir = notebook->workdir();
        TRACE("on_new_page this: %p (%s)\n", data, workdir);
        //auto dialog = (Dialog<Type> *) data;
        //dialog->addPage(notebook->workdir());
        notebook->addPage(view->path());
    }

    static void
    on_remove_page(GtkButton *button, void *data){
        TRACE("on_remove_page this: %p\n", data);
        auto page = (Page<Type> *)data;
        auto notebook = (Notebook<Type> *)(g_object_get_data(G_OBJECT(page->pageChild()), "Notebook"));
        notebook->removePage(GTK_WIDGET(page->pageChild()));
        while(gtk_events_pending()) gtk_main_iteration();
    }


};


}

#endif

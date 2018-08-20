#ifndef XF_NOTEBOOK
#define XF_NOTEBOOK
#include "pagechild.hh"

namespace xf {

template <class Type>
class Notebook;

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
        DBG("switch_page\n");
        //FIXME:
        /*
        // This callback may occur after view has been destroyed.
        window_c *window_p = (window_c *)g_object_get_data(G_OBJECT(notebook), "window_p");
        if (!window_p->is_view_in_list(data)) {
            DBG("switch_page:: view_p %p no longer exists.\n", data);
            return;
        }

        view_c *view_p = (view_c *)data;
        gint current_page = gtk_notebook_get_current_page (notebook);
        DBG("switch_page, page_num=%d current_page=%d xfdir_p=%p\n" ,new_page, current_page, view_p->get_xfdir_p());
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
        DBG("page_reordered\n");
    }
#define NOTEBOOK_1_CALLBACK(X)  G_CALLBACK((void (*)(GtkNotebook *,GtkDirectionType *,gpointer)) X)

    static void
    move_focus_out (GtkNotebook     *notebook,
                   GtkDirectionType arg1,
                   gpointer         data)
    {
        DBG("move_focus_out\n");
    }

#define NOTEBOOK_2_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkNotebook *,GtkDirectionType *, gboolean, gpointer)) X)

    static gboolean
    reorder_tab (GtkNotebook     *notebook,
                   GtkDirectionType arg1,
                   gboolean         arg2,
                   gpointer         data)
    {
        DBG("reorder_tab\n");
        return FALSE;
    }

#define NOTEBOOK_3_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkNotebook *,GtkNotebookTab *,gpointer)) X)
    static gboolean
    focus_tab (GtkNotebook   *notebook,
                   GtkNotebookTab arg1,
                   gpointer       data)
    {
        DBG("focus_tab\n");
        return FALSE;
    }
        
#define NOTEBOOK_4_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkNotebook *, gint, gpointer)) X)
    static gboolean
    change_current_page (GtkNotebook *notebook,
                   gint         arg1,
                   gpointer     data)
    {
        DBG("change_current_page\n");
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
        DBG("create_window\n");
        return NULL;
    }

#define NOTEBOOK_6_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkNotebook *, gboolean, gpointer)) X)
    static gboolean
    select_page (GtkNotebook *notebook,
                   gboolean     arg1,
                   gpointer     data)
    {
        DBG("select_page\n");
        return FALSE;
    }

    static void
    on_new_page(GtkButton *button, void *data){
        DBG("on_new_page this: %p\n", data);
        Notebook<Type> *notebook = (Notebook<Type> *)data;
        notebook->addPage();
    }


};

template <class Type>
class Notebook {
    using gtk_c = Gtk<double>;
    using pixbuf_c = Pixbuf<double>;
public:
    Notebook(void){
        notebook_ = GTK_NOTEBOOK(gtk_notebook_new());
        pageHash_ =g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
        gtk_notebook_set_scrollable (notebook_, TRUE);
        auto menu_button = GTK_MENU_BUTTON(gtk_menu_button_new());
        // FIXME:
        //auto popover = GTK_POPOVER(gtk_popover_new_from_model (menu_button, signal_menu_model));
        //gtk_menu_button_set_popover (menu_button, popover);
        gtk_widget_show(GTK_WIDGET(menu_button));

        auto new_tab_button = gtk_button_new ();
        gtk_c::setup_image_button(new_tab_button, "list-add", _("Open a new tab (Ctrl+T)"));
        gtk_widget_show(new_tab_button);

        auto button_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_box_pack_start(button_box, GTK_WIDGET(new_tab_button),  FALSE, FALSE, 0);
        gtk_box_pack_start(button_box, GTK_WIDGET(menu_button),  FALSE, FALSE, 0);
        gtk_widget_show(GTK_WIDGET(button_box));
        gtk_notebook_set_action_widget (notebook_, GTK_WIDGET(button_box), GTK_PACK_END);
    
        // FIXME:
        void *p = (void *)this;
        DBG("notebook this=%p/%p\n", (void *)this, p);

        g_signal_connect(G_OBJECT(new_tab_button), "clicked", 
//                BUTTON_CALLBACK(on_new_page), p); 
                BUTTON_CALLBACK(notebookSignals<Type>::on_new_page), (void *)this); 
        //g_signal_connect (G_OBJECT(window), "destroy", G_CALLBACK (destroy), 
        //        NULL);
        signalSetup();
        //addPage();
        gtk_widget_show (GTK_WIDGET(notebook_));
        return;

    }
    
    gint addPage(void){
        auto page = new(PageChild<Type>);
        gint pageNumber = gtk_notebook_append_page (notebook_,
                          GTK_WIDGET(page->pageChild()),
                          GTK_WIDGET(page->pageLabelBox()));
        DBG("added page number %d: %p\n", pageNumber, (void *)page);
        g_hash_table_replace(pageHash_, GINT_TO_POINTER(pageNumber+1), (void *)page);

    }

    void setVpanePosition(gint pageNumber, gint position){
        PageChild<Type> *page = (PageChild<Type> *)g_hash_table_lookup(pageHash_, GINT_TO_POINTER(pageNumber+1));
        if (!page){
            DBG("setVpanePosition:: no hash entry for page number %d\n", pageNumber);
            return;
        }
        page->setVpanePosition(position);
    }

    void setVpanePosition(gint position){
        gint pageNumber = gtk_notebook_get_current_page(notebook_);
        setVpanePosition(pageNumber, position);
    }

    GtkTextView *diagnostics(gint pageNumber){
        PageChild<Type> *page = (PageChild<Type> *)g_hash_table_lookup(pageHash_, GINT_TO_POINTER(pageNumber+1));
        if (!page){
            DBG("setVpanePosition:: no hash entry for page number %d\n", pageNumber);
            return NULL;
        }
        return page->diagnostics();
    }

    GtkTextView * diagnostics(void){
        gint pageNumber = gtk_notebook_get_current_page(notebook_);
        return  diagnostics(pageNumber);
    }

    GtkPaned *vpane(gint pageNumber){
        PageChild<Type> *page = (PageChild<Type> *)g_hash_table_lookup(pageHash_, GINT_TO_POINTER(pageNumber+1));
        if (!page){
            DBG("setVpanePosition:: no hash entry for page number %d\n", pageNumber);
            return NULL;
        }
        return page->vpane();
    }

    GtkPaned *vpane(void){
        gint pageNumber = gtk_notebook_get_current_page(notebook_);
        return  vpane(pageNumber);
    }

    gint currentPage(void){
        return gtk_notebook_get_current_page(notebook_);
    }
    void setPageLabel(gint pageNumber, const gchar *text){
         auto label = GTK_LABEL(gtk_label_new(""));
	 GtkWidget *child = gtk_notebook_get_nth_page (notebook_, pageNumber);
         //auto label = GTK_LABEL(g_object_get_data(G_OBJECT(child), "page_label"));
         gtk_label_set_markup(label, text);
         gtk_notebook_set_tab_label(notebook_, child, GTK_WIDGET(label));
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

};

}

#endif

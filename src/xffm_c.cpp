#include "xffm_c.hpp"

void
on_new_page(GtkWidget *widget, gpointer data){
    xffm_c *xffm_p = (xffm_c *)g_object_get_data(G_OBJECT(widget), "object");
    view_c *view_p = new view_c(xffm_p->window_p);
    xffm_p->add_view_to_list(view_p);
}

xffm_c::xffm_c(void){
    signals_p = new signals_c();
    window_p = new window_c();
    GtkWidget *new_button = window_p->get_new_button();

    signals_p->setup_callback((void *)this, new_button, "clicked", (void *)on_new_page,NULL); 
    view_c *view_p = new view_c(window_p);
    view_list = g_list_prepend(NULL, (void *)view_p);

}
xffm_c::~xffm_c(void){
    GList *l;
    for (l=view_list; l && l->data; l=l->next){
        view_c *view_p = (view_c *)l->data;
        delete view_p;
    }
    delete window_p;
    delete signals_p;
}

void
xffm_c::add_view_to_list(view_c *view_p) {
    view_list = g_list_prepend(view_list, (void *)view_p);
}




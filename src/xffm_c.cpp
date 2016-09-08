#include "xffm_c.hpp"
#include "window_c.hpp"


xffm_c::xffm_c(GtkApplication *data, const gchar *dir){
    app = (GtkApplication *)data;
    window_p = new window_c();
    // initial view...
    if (!dir || !g_file_test(dir,G_FILE_TEST_IS_DIR))
        window_p->create_new_page(g_get_home_dir());
    else 
        window_p->create_new_page(dir);
    gtk_application_add_window (app, get_window());
}


xffm_c::xffm_c(GtkApplication *data){
    app = (GtkApplication *)data;
    window_p = new window_c();
    // initial view...
    window_p->create_new_page(g_get_home_dir());
    gtk_application_add_window (app, get_window());
}

GtkWindow *
xffm_c::get_window(void){ return GTK_WINDOW(window_p->get_window());}

void 
xffm_c::create_new_page(const gchar *data){
    window_p->create_new_page(data);
}
    
/*
 
xffm_c::xffm_c(void){
    window_p = new window_c();
    // initial view...
    view_c *view_p = new view_c((void *)window_p, window_p->get_notebook());
    window_p->add_view_to_list(view_p);
    view_p->load(g_get_home_dir());
}

   */
xffm_c::~xffm_c(void){
    delete window_p;
}  



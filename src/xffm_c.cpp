#include "xffm_c.hpp"
#include "view_c.hpp"

#include "xfdir_c.hpp"


xffm_c::xffm_c(void){
    window_p = new window_c();
    // initial view...
    window_p->create_new_page(g_get_home_dir());
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




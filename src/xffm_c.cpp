#include "xffm_c.hpp"
#include "view_c.hpp"

#include "xfdir_c.hpp"


xffm_c::xffm_c(void){
    window_p = new window_c();
    // initial view...
    view_c *view_p = new view_c((void *)window_p, window_p->get_notebook(), window_p->get_new_tab_child());
    window_p->add_view_to_list(view_p);
    xfdir_c *xfdir_p = new xfdir_c();
    view_p->set_treemodel(xfdir_p->get_tree_model("/"));

}
xffm_c::~xffm_c(void){
    delete window_p;
}  




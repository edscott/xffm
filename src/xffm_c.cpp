#include "xffm_c.hpp"
#include "view_c.hpp"


xffm_c::xffm_c(void){
    window_p = new window_c();
    view_c *view_p = new view_c(window_p->get_notebook(), window_p->get_new_tab_child());
    window_p->add_view_to_list(view_p);

}
xffm_c::~xffm_c(void){
    delete window_p;
}                                                   



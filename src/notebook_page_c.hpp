#ifndef NOTEBOOK_PAGE_C_HPP
#define NOTEBOOK_PAGE_C_HPP
#include "widgets_c.hpp"
#include "utility_c.hpp"
#include "signals_c.hpp"

// This class arranges and shows all necessary widgets for class view_c
// and binds gtk callbacks.

class notebook_page_c:public widgets_c, protected signals_c {
    public:
        notebook_page_c(GtkWidget *);
        ~notebook_page_c(void);
        void clear_diagnostics(void);
    protected:
        void set_treemodel(GtkTreeModel *);
    private:
        GtkListStore *list_store;

        GtkWidget *notebook;
        GtkWidget *icon_view;           // drawing area

        void pack();
        void signals();

};
#endif

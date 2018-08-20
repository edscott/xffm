#ifndef XF_TERM_DIALOG
#define XF_TERM_DIALOG


#include "common/types.h"
#include "common/icons.hh"
#include "common/pixbuf.hh"
#include "common/gtk.hh"
#include "common/tooltip.hh"
#include "common/dialog.hh"
#include "common/util.hh"

#include "common/print.hh"

namespace xf 
{
    
template <class Type>
class termDialog : public Dialog<Type> {
    using print_c = Print<double>;
 /*   using util_c = Util<double>;
    using gtk_c = Gtk<double>;
    using pixbuf_c = Pixbuf<double>;
    using pixbuf_icons_c = Icons<double>;
    using tooltip_c = Tooltip<double>;*/
 //   using dialog_c = xfDialog<double>;
 //
private:

 

    ///
public:
    //termDialog(const gchar *title, const gchar *icon):Dialog<Type>(title, icon){
    termDialog(void){
	//DBG("%s %s\n", title, icon);
        this->setDialogSize(600,400);
        this->setVpanePosition(0);
        print_c::print(this->diagnostics(), "tag/red", g_strdup("Hello world!\n"));
        //DBG("current page = %d\n", this->currentPage());
        this->setPageLabel(this->currentPage(), "foo");
    }
    void createDialog(const gchar *path){
        gchar *default_path=NULL;
        if (path) default_path = g_strdup(path);
DBG("1\n");
	GtkWindow *dialog = this->dialog();
    }


#if 0
	//GtkWindow *dialog = this->mkddDialog("Term","utilities-terminal" );
DBG("162\n");
	auto horizontalButtonSpace = createHorizontalButtonSpace(); 
DBG("17\n");
	gtk_box_pack_start (page_child, GTK_WIDGET(horizontalButtonSpace), FALSE, FALSE, 0);
DBG("171\n");

DBG("172\n");
	

	gtk_widget_show_all(GTK_WIDGET(dialog));
	
    }

#endif
private:


};
}



#endif

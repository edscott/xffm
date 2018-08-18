#ifndef XF_TERM_DIALOG
#define XF_TERM_DIALOG


#include "types.h"
#include "xficons.hh"
#include "xfpixbuf.hh"
#include "xfgtk.hh"
#include "xftooltip.hh"
#include "xfutil.hh"
#include "xfdialog.hh"

namespace xf {
namespace term{
template <class Type>
class Dialog : public xf::Dialog<Type>{
protected:
    void createDialog(const gchar *path){
        dialog_ = createXFDialog_(path);
	gtk_widget_show(GTK_WIDGET(dialog_));
	
    }

private:
    GtkWindow dialog_;

};
}

}


#endif

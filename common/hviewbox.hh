#ifndef XF_HVIEWBOX
#define  XF_HVIEWBOX
#include "vpane.hh"
#include "vbuttonbox.hh"
namespace xf {

template <class Type>
class HViewBox: public Vpane<Type>, VButtonBox<Type> {
public:
    HViewBox(void){
	hViewBox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        this->insertVPane(hViewBox_);
        this->insertVButtonBox(hViewBox_);
	//gtk_box_pack_start (hview_box, GTK_WIDGET(vpane_), TRUE, TRUE, 0);
    }
    void insertHViewBox(GtkBox *pageChild){
	gtk_box_pack_start (pageChild, GTK_WIDGET(hViewBox_), TRUE, TRUE, 0);
    }

    GtkBox *hViewBox(void){ return hViewBox_;}

private:
    GtkBox *hViewBox_;
};
}




#endif

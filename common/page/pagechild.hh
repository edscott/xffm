#ifndef XF_PAGE_CHILD
#define XF_PAGE_CHILD
#include "vbuttonbox.hh"
#include "hbuttonbox.hh"
#include "vpane.hh"
#include "completion/csh.hh"

namespace xf{

template <class Type>
class PageChild: public Vpane<Type>, VButtonBox<Type>, HButtonBox<Type>, CshCompletion<Type> {
    using gtk_c = Gtk<double>;
public:

    PageChild(void){
	pageChild_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
	pageLabel_ = GTK_LABEL(gtk_label_new ("foobar"));
	pageLabelBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	pageLabelSpinner_ = GTK_SPINNER(gtk_spinner_new());
	pageLabelSpinnerBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	pageLabelIconBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	pageLabelButton_ = GTK_BUTTON(gtk_button_new ());

        // FIXME: this goes in term::dialog:

	gtk_box_pack_start (pageLabelBox_, GTK_WIDGET(pageLabelSpinnerBox_), TRUE, TRUE, 0);
	gtk_box_pack_start (pageLabelSpinnerBox_, GTK_WIDGET(pageLabelSpinner_), TRUE, TRUE, 0);
	gtk_box_pack_start (pageLabelBox_, GTK_WIDGET(pageLabelIconBox_), TRUE, TRUE, 0);
	gtk_box_pack_start (pageLabelBox_, GTK_WIDGET(pageLabel_), TRUE, TRUE, 2);
	gtk_box_pack_end (pageLabelBox_, GTK_WIDGET(pageLabelButton_), TRUE, TRUE, 0);
        gtk_c::setup_image_button(GTK_WIDGET(pageLabelButton_), "window-close", _("Close"));
    
	//set_spinner(pageSpinner_, TRUE, pageLabelIconBox_);
	set_spinner(FALSE);

	// pathbar should be created with pathbar_c object.
	// FIXME: add pathbar
	// gtk_box_pack_start (pageChild, pathbar_p->get_pathbar(), FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(pageLabelBox_));

        GtkBox *hViewBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        GtkBox *hButtonBox = this->hButtonBox();
        GtkBox *vButtonBox = this->vButtonBox();
        GtkPaned *vpane = this->vpane();
	gtk_box_pack_start (hViewBox, GTK_WIDGET(vpane), TRUE, TRUE, 0);
	gtk_box_pack_start (hViewBox, GTK_WIDGET(vButtonBox), FALSE, FALSE, 0);
	gtk_box_pack_start (pageChild_, GTK_WIDGET(hViewBox), TRUE, TRUE, 0);
	gtk_box_pack_start (pageChild_, GTK_WIDGET(hButtonBox), FALSE, FALSE, 0);

        this->setCompletionTextView(this->diagnostics());
        this->setCompletionInput(this->status());
	gtk_widget_show_all(GTK_WIDGET(pageChild_));

	return;

    }
    void setTabIcon(const gchar *icon){
         gtk_c::set_container_image(GTK_CONTAINER(pageLabelIconBox_), icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
   }
     void setPageLabel(const gchar *text){
         gtk_label_set_markup(pageLabel_, text);
         //gtk_notebook_set_tab_label(notebook_, child, GTK_WIDGET(label));
   }
    
   
    void set_spinner(gboolean state)
    {
	if (!state){
	    gtk_spinner_stop (pageLabelSpinner_);
	    gtk_widget_show(GTK_WIDGET(pageLabelIconBox_));
	    return;
	}
	gtk_widget_hide(GTK_WIDGET(pageLabelIconBox_));  
	gtk_spinner_start (pageLabelSpinner_);
    }
    GtkBox *pageChild(void){ return pageChild_;}
    GtkLabel *pageLabel(void){ return pageLabel_;}
    GtkBox *pageLabelBox(void){ return pageLabelBox_;}
    GtkSpinner *pageLabelSpinner(void){ return pageLabelSpinner_;}
    GtkBox *pageLabelSpinnerBox(void){ return pageLabelSpinnerBox_;}
    GtkBox *pageLabelIconBox(void){ return pageLabelIconBox_;}
    GtkButton *pageLabelButton(void){ return pageLabelButton_;}
private:
    GtkBox *pageChild_;
    GtkLabel *pageLabel_;
    GtkBox *pageLabelBox_;
    GtkSpinner *pageLabelSpinner_;
    GtkBox *pageLabelSpinnerBox_;
    GtkBox *pageLabelIconBox_;
    GtkButton *pageLabelButton_;

};




}



#endif

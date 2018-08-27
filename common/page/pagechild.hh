#ifndef XF_PAGE_CHILD
#define XF_PAGE_CHILD
#include "vbuttonbox.hh"
#include "hbuttonbox.hh"
#include "vpane.hh"
#include "common/completion/completion.hh"
#include "threadcontrol.hh"
#include "runbutton.hh"

namespace xf{
template <class Type> class RunButton;

template <class Type>
class PageChild : public Completion<Type>, ThreadControl<Type>{
    using gtk_c = Gtk<double>;
private:
    GList *run_button_list;
    pthread_mutex_t *rbl_mutex;

public:

    PageChild(void){
	pageWorkdir_ = NULL;
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
        hButtonBox_=HButtonBox<Type>::newBox();
        vButtonBox_ = VButtonBox<Type>::newBox();
        vpane_ = Vpane<Type>::newVpane();

	gtk_box_pack_start (hViewBox, GTK_WIDGET(vpane_), TRUE, TRUE, 0);
	gtk_box_pack_start (hViewBox, GTK_WIDGET(vButtonBox_), FALSE, FALSE, 0);
	gtk_box_pack_start (pageChild_, GTK_WIDGET(hViewBox), TRUE, TRUE, 0);
	gtk_box_pack_start (pageChild_, GTK_WIDGET(hButtonBox_), FALSE, FALSE, 0);

	// Data for lpterm
        this->setOutput(this->diagnostics());
        this->setInput(this->status());
        this->setPage(this);

	pthread_mutexattr_t r_attr;
	pthread_mutexattr_init(&r_attr);
	pthread_mutexattr_settype(&r_attr, PTHREAD_MUTEX_RECURSIVE);
	rbl_mutex = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
	pthread_mutex_init(rbl_mutex, &r_attr);
	run_button_list = NULL;



	gtk_widget_show_all(GTK_WIDGET(pageChild_));

	return;
    }
    
    ~PageChild(void){
	GList *l = run_button_list;
	pthread_mutex_lock(rbl_mutex);
	for (; l && l->data; l=l->next){
	    //run_button_c *rb_p = (run_button_c *)l->data;
	    unreference_run_button(l->data);
	}
	g_list_free(run_button_list);
	run_button_list=NULL;
	pthread_mutex_unlock(rbl_mutex);
	pthread_mutex_destroy(rbl_mutex);
	g_free(rbl_mutex);
    }

//    void reference_run_button(run_button_c *rb_p){
    void *reference_run_button(void *rb_p){
	DBG("reference_run_button(%p)\n", rb_p);
	pthread_mutex_lock(rbl_mutex);
	run_button_list = g_list_prepend(run_button_list, rb_p);
	pthread_mutex_unlock(rbl_mutex);
    }

    void
    unreference_run_button(void *rb_p){
	DBG("unreference_run_button(%p)\n", rb_p);
	pthread_mutex_lock(rbl_mutex);
	void *p = g_list_find(run_button_list, rb_p);
	if (p){
	    run_button_list = g_list_remove(run_button_list, rb_p);
	    // FIXME delete ((RunButton<Type> *)rb_p);
	}
	pthread_mutex_unlock(rbl_mutex);
    }
    
    void setPageWorkdir(const gchar *dir){
	DBG("setPageWorkdir: %s\n", dir);
	//g_free(pageWorkdir_);
	pageWorkdir_ = g_strdup(dir);
	gchar *g = Completion<Type>::get_terminal_name(pageWorkdir_);
	setPageLabel(g);
	g_free(g);
	// and for bash completion
	this->setWorkdir(dir);
    }

    void setTabIcon(const gchar *icon){
         gtk_c::set_container_image(GTK_CONTAINER(pageLabelIconBox_), icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
   }
     void setPageLabel(const gchar *text){
         gtk_label_set_markup(pageLabel_, text);
         //gtk_notebook_set_tab_label(notebook_, child, GTK_WIDGET(label));
   }
    
    void setVpanePosition(gint position){
	gtk_paned_set_position (vpane_, position);
        gint max;
	g_object_get(G_OBJECT(vpane_), "max-position", &max, NULL);
 	g_object_set_data(G_OBJECT(vpane_), "oldCurrent", GINT_TO_POINTER(position));
	g_object_set_data(G_OBJECT(vpane_), "oldMax", GINT_TO_POINTER(max));   
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
    
    GtkBox *hButtonBox(void){return hButtonBox_;}
    GtkImage *status_icon(void){ return GTK_IMAGE(g_object_get_data(G_OBJECT(hButtonBox_), "status"));}
    GtkImage *iconview_icon(void){ return GTK_IMAGE(g_object_get_data(G_OBJECT(hButtonBox_), "status"));}
    GtkTextView *status(void){ return GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(hButtonBox_), "status"));}
    GtkBox *statusBox(void){ return GTK_BOX(g_object_get_data(G_OBJECT(hButtonBox_), "status"));}
    GtkButton *status_button(void){ return GTK_BUTTON(g_object_get_data(G_OBJECT(hButtonBox_), "status"));}
    GtkLabel *status_label(void){ return GTK_LABEL(g_object_get_data(G_OBJECT(hButtonBox_), "status"));}

    GtkBox *vButtonBox(void){return vButtonBox_;}
    GtkButton *clear_button(void){ return GTK_BUTTON(g_object_get_data(G_OBJECT(vButtonBox_), "clear_button"));}
    GtkScale *size_scale(void){ return GTK_SCALE(g_object_get_data(G_OBJECT(vButtonBox_), "size_scale"));}
    
    GtkPaned *vpane(void){return vpane_;}
    GtkTextView *diagnostics(void){ return GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(vpane_), "diagnostics"));}

private:
    GtkBox *pageChild_;
    GtkLabel *pageLabel_;
    GtkBox *pageLabelBox_;
    GtkSpinner *pageLabelSpinner_;
    GtkBox *pageLabelSpinnerBox_;
    GtkBox *pageLabelIconBox_;
    GtkButton *pageLabelButton_;

    GtkBox *hButtonBox_;
    GtkBox *vButtonBox_;
    GtkPaned *vpane_;

    gchar *pageWorkdir_;


};




}



#endif

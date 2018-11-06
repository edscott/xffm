#ifndef XF_COMBORESPONSE_HH
# define XF_COMBORESPONSE_HH
#include "entryresponse.hh"

namespace xf
{
template <class Type>
class ComboResponse: public EntryResponse<Type> {
    
    using gtk_c = Gtk<Type>;
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;
    using pixbuf_icons_c = Icons<Type>;
    using page_c = Page<Type>;

protected:
    GtkComboBoxText *combo_;

public:

        ComboResponse (GtkWindow *parent, const gchar *windowTitle): EntryResponse<Type>(parent, windowTitle) {
	
        combo_ = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new_with_entry());
        gtk_box_pack_start (GTK_BOX (this->hbox_), GTK_WIDGET(combo_), TRUE, TRUE, 0);
        g_object_set_data(G_OBJECT(combo_),"response", this->response_);
        

        return;
    }

    void setComboLabel(const gchar *value){
        this->setLabel(this->comboLabel(), value);
        gtk_widget_show(GTK_WIDGET(combo_));
    }

    GtkEntry *comboEntry(void){
        return GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_)));
    }

    void setComboOptions(const gchar **comboOptions){        
        if (!comboOptions || *comboOptions==NULL){
            ERROR("setComboOptions() with comboOptions = NULL is not valid.\n");
            return;
        }

        for (const gchar **p=comboOptions; p && *p; p++){
            gtk_combo_box_text_append_text (combo_,*p);
            DBG("setting combo value: %s\n" , *p);
        }
        gtk_combo_box_set_active (GTK_COMBO_BOX(combo_),0);
        gtk_widget_show(GTK_WIDGET(combo_));
    }

    void setComboDefault(const gchar *value){
        if (!value) return;
        gtk_combo_box_text_prepend_text (combo_, value);
        gtk_combo_box_set_active (GTK_COMBO_BOX(combo_),0);
        gtk_widget_show(GTK_WIDGET(combo_));
    }
    void setComboBashCompletion(const gchar *wd){
        auto entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo_)));
        this->connectBashCompletion(wd, entry);
    }


    void setCheckButtonComboCallback(void *clickAction){
        // Set the toggle action.
        g_signal_connect (G_OBJECT (this->checkButton()), "clicked", 
		    BUTTON_CALLBACK(clickAction), 
                    (void *)gtk_bin_get_child(GTK_BIN(combo_)));
        gtk_widget_show(GTK_WIDGET(this->checkButton()));
    }

    void setComboCallback(void *changeAction){
        // Set the toggle action.
        g_signal_connect (G_OBJECT (combo_), "changed", 
		    BUTTON_CALLBACK(changeAction), this);
        gtk_widget_show(GTK_WIDGET(this->checkButton()));
    }


    gchar *getResponse(void){
        return g_strdup (gtk_combo_box_text_get_active_text (combo_));
    }
 
};
}
#endif


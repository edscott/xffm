#ifndef XF_LOCALRM__HH
# define XF_LOCALRM__HH
#include "common/gio.hh"

# define RM_YES                 1
# define SHRED_YES              2
# define TRASH_YES              3
# define RM_CANCEL              4
namespace xf
{
template <class Type>
class LocalRm {


public:
    static void
    rm(GtkMenuItem *menuItem, gpointer data){
        
        TRACE("LocalRm:: rm\n");
        auto view =  (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        //  single or multiple item selected?
        GList *selectionList;
        if (isTreeView){
            auto treeModel = view->treeModel();
            auto selection = gtk_tree_view_get_selection (view->treeView());
            selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        } else {
            selectionList = gtk_icon_view_get_selected_items (view->iconView());
        }
        if (!selectionList){
            ERROR("local/rm.hh::rm(): nothing selected\n");
        }
        GList *list = NULL;
        for (auto tmp=selectionList; tmp && tmp->data; tmp = tmp->next){
            gchar *path;
            GtkTreeIter iter;
            auto tpath = (GtkTreePath *)tmp->data;
            gtk_tree_model_get_iter(view->treeModel(), &iter, tpath);
            gtk_tree_model_get(view->treeModel(), &iter, PATH, &path, -1);
            list = g_list_append(list, g_strdup(path));
        }
#if 1
        rmQuery(view, list);
#else
        while (list && g_list_length(list)>0) {
            list = rmQuery(view, list);
        }
#endif
    }

private:
    static void
    rmQuery(View<Type> *view, GList *list){
        TRACE("rmQuery: rm\n");
        if (!list || !g_list_length(list)) {
            g_list_free(list);
            return;
        }
        auto text = g_strdup_printf(_("Delete %s"), (gchar *)list->data);
        auto message = g_list_length(list) > 1 ? 
        g_strdup_printf("<span color=\"red\" size=\"larger\">%s\n%s (%d)</span>",_("Warning: "), _("Multiple selections"), g_list_length(list)):
        "";
        auto rmDialog = createRemove(view, text, message, FALSE, g_list_length(list)>1);
        g_object_set_data(G_OBJECT(rmDialog), "list", list);

        gtk_widget_show (GTK_WIDGET(rmDialog));
        ResponseClass<Type>::runDialog(rmDialog, (void *) applyResponse, (void *)list);
        return;
   }

    static void *
    applyResponse(void *data){
      void **arg = (void **)data;
      auto dialog = GTK_WINDOW(arg[0]);
      //auto response_f = ((*)(void *))arg[1];
      auto list = (GList *)(arg[2]);

      gint response = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialog), "response"));
      gtk_widget_hide(GTK_WIDGET(dialog));
      MainDialog = NULL;
      switch (response) {
          case TRASH_YES: // Trash operation
              if (!Gio<Type>::execute(list, MODE_TRASH)){
                  ERROR("local/rm.hh::Cannot trash\n");
              }
              break;
          case RM_YES:// rm operation
          {
              if (!Gio<Type>::execute(list, MODE_RM)){
                  ERROR("local/rm.hh::Cannot delete\n");
              }
              break;
          }
          case SHRED_YES: // Shred operation
              if (!Gio<Type>::execute(list, MODE_SHRED)){
                  ERROR("local/rm.hh::Cannot shred\n");
              }
             break;
          case RM_CANCEL:
              TRACE( "remove cancelled\n");
              break;
          default:
             TRACE( "**default : cancel remove all\n");
             break;
      }
      for (auto l=list; l && l->data; l=l->next) g_free(l->data);         
      g_list_free(list);
      g_free(arg);
      return GINT_TO_POINTER(response);
    }
    
    static GtkWindow *
    createRemove (View<Type> *view, const gchar *text, 
                  const gchar *message, gboolean always, 
                  gboolean multiple) 
    {
        TRACE("createRemove: rm\n");
        auto rmDialog = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
        //auto rmDialog = GTK_WINDOW(gtk_dialog_new ());
        gtk_window_set_type_hint(GTK_WINDOW(rmDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
        
        // title
        gchar *g=g_strdup_printf("Xffm+ %s", _("Remove"));
        gtk_window_set_title (GTK_WINDOW(rmDialog), g);
        // icon
        auto pb = Pixbuf<Type>::getPixbuf("edit-delete", SIZE_ICON);
        gtk_window_set_icon (GTK_WINDOW(rmDialog), pb);
        //gtk_window_set_modal (GTK_WINDOW(rmDialog), TRUE);
        gtk_window_set_transient_for(GTK_WINDOW(rmDialog), GTK_WINDOW(mainWindow));

        auto vbox2 = Gtk<Type>::vboxNew (FALSE, 0);
        gtk_widget_show (GTK_WIDGET(vbox2));

        gtk_container_add (GTK_CONTAINER (rmDialog), GTK_WIDGET(vbox2));

        if (multiple){
            pb = Pixbuf<Type>::getPixbuf ("dialog-warning", -96);
            auto q = gtk_image_new_from_pixbuf (pb);
            gtk_widget_show (GTK_WIDGET(q));
            compat<bool>::boxPackStart (vbox2, GTK_WIDGET(q), TRUE, TRUE, 5);
        } else {
            pb = Pixbuf<Type>::getPixbuf ("edit-delete", -48);
            auto q = gtk_image_new_from_pixbuf (pb);
            gtk_widget_show (GTK_WIDGET(q));
            compat<bool>::boxPackStart (vbox2, GTK_WIDGET(q), TRUE, TRUE, 5);
        }

        auto hbox26 = Gtk<Type>::hboxNew (FALSE, 0);
        gtk_widget_show (GTK_WIDGET(hbox26));
        compat<bool>::boxPackStart (vbox2, GTK_WIDGET(hbox26), TRUE, TRUE, 0);


        auto vbox12 = Gtk<Type>::vboxNew (FALSE, 0);
        gtk_widget_show (GTK_WIDGET(vbox12));
        compat<bool>::boxPackStart (hbox26, GTK_WIDGET(vbox12), TRUE, TRUE, 0);

        auto label16 = GTK_LABEL(gtk_label_new (""));
        auto markup = g_strdup_printf("<span size=\"larger\"><b>  %s  </b></span>", text);
        gtk_label_set_markup(label16, markup);
        g_free(markup);
        gtk_widget_show (GTK_WIDGET(label16));
        compat<bool>::boxPackStart (vbox12, GTK_WIDGET(label16), FALSE, FALSE, 0);

        auto label20 = GTK_LABEL(gtk_label_new (message));
        gtk_label_set_markup(label20, message);
        gtk_widget_show (GTK_WIDGET(label20));
        compat<bool>::boxPackStart (vbox12, GTK_WIDGET(label20), FALSE, FALSE, 0);

        auto hbox9 = Gtk<Type>::hboxNew (FALSE, 0);
        gtk_widget_show (GTK_WIDGET(hbox9));
        compat<bool>::boxPackStart (vbox12, GTK_WIDGET(hbox9), TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (hbox9), 5);

     /* always apply to all! 
      *   auto togglebutton = gtk_check_button_new_with_mnemonic (_("Apply to all"));
        gtk_widget_show (GTK_WIDGET(togglebutton));
        compat<bool>::boxPackStart (hbox9, GTK_WIDGET(togglebutton), FALSE, FALSE, 0);

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), !always);
        
        g_object_set_data(G_OBJECT(rmDialog),"togglebutton", togglebutton);*/

        auto buttonbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_widget_show (GTK_WIDGET(buttonbox));
        compat<bool>::boxPackStart (GTK_BOX (vbox12), GTK_WIDGET(buttonbox), TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (buttonbox), 5);
        gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonbox), GTK_BUTTONBOX_END);
        gtk_box_set_spacing (GTK_BOX (buttonbox), 5);

        // Cancel:
        auto button = Gtk<Type>::dialog_button ("window-close", _("Cancel"));
        g_object_set_data(G_OBJECT(button), "dialog", rmDialog);
        gtk_container_add (GTK_CONTAINER (buttonbox),GTK_WIDGET(button));

        g_signal_connect (G_OBJECT (button), "clicked", 
                G_CALLBACK (ResponseClass<Type>::responseAction), GINT_TO_POINTER(RM_CANCEL));
        
        // Shred: (available to BSD with rm -P option. FIXME whenever BSD available.)
        button = Gtk<Type>::dialog_button ("edit-delete/NE/edit-delete-symbolic/2.0/150", _("Shred"));
        g_object_set_data(G_OBJECT(button), "dialog", rmDialog);
        gtk_container_add (GTK_CONTAINER (buttonbox), GTK_WIDGET(button));

        g_signal_connect (G_OBJECT (button), "clicked", 
                G_CALLBACK (ResponseClass<Type>::responseAction), GINT_TO_POINTER(SHRED_YES));
        gchar *shred = g_find_program_in_path("shred");
        gtk_widget_set_sensitive(GTK_WIDGET(button), shred != NULL);
        g_free(shred);

        // Delete: 
        button = Gtk<Type>::dialog_button ("edit-delete", _("Delete"));
        g_object_set_data(G_OBJECT(button), "dialog", rmDialog);
        gtk_container_add (GTK_CONTAINER (buttonbox), GTK_WIDGET(button));

        g_signal_connect (G_OBJECT (button), "clicked", 
                G_CALLBACK (ResponseClass<Type>::responseAction), GINT_TO_POINTER(RM_YES));

        // Trash:
        button = Gtk<Type>::dialog_button ("user-trash", _("Trash"));
        g_object_set_data(G_OBJECT(button), "dialog", rmDialog);
        gtk_container_add (GTK_CONTAINER (buttonbox), GTK_WIDGET(button));

        g_signal_connect (G_OBJECT (button), "clicked", 
                G_CALLBACK (ResponseClass<Type>::responseAction), GINT_TO_POINTER(TRASH_YES));

        g_signal_connect (rmDialog, "delete-event", 
            G_CALLBACK (ResponseClass<Type>::on_destroy_event), GINT_TO_POINTER(RM_CANCEL));
        g_signal_connect (rmDialog, "destroy-event", 
            G_CALLBACK (ResponseClass<Type>::on_destroy_event), GINT_TO_POINTER(RM_CANCEL));


        gtk_widget_realize (GTK_WIDGET(rmDialog));

        gtk_widget_grab_focus (GTK_WIDGET(button));

        gtk_window_set_position(GTK_WINDOW(rmDialog), GTK_WIN_POS_CENTER_ON_PARENT);
     
        MainDialog = GTK_WINDOW(rmDialog);
        return rmDialog;
    }

private:
    
};
}

#endif

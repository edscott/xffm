#ifndef XF_LOCALDND__HH
# define XF_LOCALDND__HH

namespace xf
{
//template <class Type> class View;
template <class Type>
class Dnd {

public:
#if 0
    static gchar *
    sendDndData(View<Type> *view){
        return ClipBoard::getSelectionData(view, NULL);
    }
    static GdkDragAction queryAction(View<Type> *view, const gchar *target, gchar **files){
      int number=0;
      for (gchar **p=files; p && *p; p++) number++;
      GtkWidget *dialog;
      GdkDragAction action = (GdkDragAction)0;
      int flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
      dialog = gtk_dialog_new_with_buttons (_("Multiple selections"),
                                   GTK_WINDOW(mainWindow),
                                   (GtkDialogFlags)flags,
                                   _("Accept"),
                                   1,
                                   _("Cancel"),
                                   0,
                                   NULL);
      MainDialog = GTK_WINDOW(dialog);

    
      GtkWidget *b= gtk_dialog_get_content_area(GTK_DIALOG(dialog));

      gchar *s = g_strdup_printf("%s: <b>%s</b>\n(%s:%d)+\n", _("Target"), target? target: view->path(), _("Files"), number);
      int count=1;
      for (char **p=files; p && *p; p++, count++){
        char buffer[256];
        snprintf(buffer, 256, "<b>%d:</b> %s\n", count, *p);
        Basic::concat(&s, buffer);
        if (count >= 3){
          Basic::concat(&s, "<i>");
          Basic::concat(&s, _("More…"));
          Basic::concat(&s, "</i>");
          Basic::concat(&s, "\n");
          break;
        }
      }
      GtkWidget *w = gtk_label_new("");
      gtk_label_set_markup(GTK_LABEL(w), s);
      g_free(s);
      gtk_box_pack_end(GTK_BOX(b), w, TRUE, TRUE, 5);
      GtkBox *rBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL,3));
      gtk_box_pack_end(GTK_BOX(b), GTK_WIDGET(rBox), FALSE, FALSE, 5);

      GtkWidget *r;
      const char *opts[]={_("Copy"), _("Move"), _("Link"), NULL};
      int optAction[]={2,4,8};
      int k=0;
      GtkRadioButton *group=NULL;
      for (const char **p=opts; p && *p; p++, k++){
        r = gtk_radio_button_new_with_label_from_widget (group, *p);
        if (k==0) group = GTK_RADIO_BUTTON(r);
        compat<bool>::boxPackStart(GTK_BOX(rBox), r, TRUE, TRUE, 5);
        g_object_set_data(G_OBJECT(dialog), *p, r);
        g_object_set_data(G_OBJECT(r), "action", GINT_TO_POINTER(optAction[k]));
        if (action == optAction[k]) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(r), TRUE);
      }

      gtk_widget_show_all(GTK_WIDGET(dialog));
      int response = gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_hide(dialog);

      if (!response) return action;
      k=0;
      for (const char **p=opts; p && *p; p++, k++){
        r = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), *p));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(r))) 
          action = (GdkDragAction) GPOINTER_TO_INT(g_object_get_data(G_OBJECT(r), "action"));
      }
      MainDialog = NULL;
      gtk_widget_destroy(dialog);

      fprintf(stderr, "response=%d, action=%d\n", response, action);
      return action;
    }
#endif               
    static gboolean
    receiveDndData(
            //View<Type> *view, // gridview
            const gchar *target, 
            const GtkSelectionData *selection_data, 
            GdkDragAction action)
    {
        TRACE("View::receiveDndData\n");
        if (!selection_data) {
            ERROR("!selection_data\n");
            return FALSE;
        }
        auto dndData = (const char *)gtk_selection_data_get_data (selection_data);
        gchar **files = g_strsplit(dndData, "\n", -1);
        int number=0;
        for (gchar **p=files; p && *p; p++) number++;
        if (!number) return FALSE;

        const char *tgt = target? target: view->path();
        char *source=files[0];
        if (strstr(source, "file://")) source += strlen("file://");
        char *sourceDir = g_path_get_dirname(source);
        if (!sourceDir) return FALSE;
        if (strcmp(sourceDir, tgt) == 0) {
          g_free(sourceDir);
          return FALSE;
        }
        g_free(sourceDir);
        //DBG("View::receiveDndData number=%d\n", number);


        // FIXME: if (number > 1) action = queryAction(view, tgt, files);

        //DBG("View::receiveDndData action=%d\n", action);
        const gchar *command;
        const gchar *message;
        gint mode;
        switch (action){
            case GDK_ACTION_DEFAULT: 
            case GDK_ACTION_MOVE:
                message = _("Moving files");
                command = "mv -b -f";
                mode = MODE_MOVE;
                break;
            case GDK_ACTION_COPY:
                message = _("Copying files locally");
                command = "cp -R -b -f";
                mode = MODE_COPY;
                break;
            case GDK_ACTION_LINK:
                message = _("Create Link");
                command = "ln -s -b -f";
                mode = MODE_LINK;
                break;
            case GDK_ACTION_PRIVATE:
            case GDK_ACTION_ASK:
                ERROR("Not supported GDK_ACTION_PRIVATE || GDK_ACTION_ASK\n");
                return FALSE;

        }

        auto more = (files[1] != NULL && strstr(files[1], "file://"))?
                g_strdup_printf("[+ %s]", _("more")):
                g_strdup("");
        TRACE("%s %s %s ---> %s\n", message, files[0], more, tgt);

            
        Print<Type>::print(view->page()->output(), "green", 
                    g_strdup_printf("%s %s %s ---> %s\n", 
                    message, files[0], more, tgt)
                );

        g_free(more);
        // FIXME: auto result = Gio<Type>::executeURL(files, tgt, mode);
        if (files) g_strfreev(files);
        return result;
    }

};
}
#endif



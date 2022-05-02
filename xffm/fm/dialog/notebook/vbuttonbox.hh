#ifndef XF_VBUTTONBOX
#define XF_VBUTTONBOX

#ifdef ENABLE_CUSTOM_RESPONSE
#include "createini.hh"
#endif

namespace xf {
    

template <class Type>
class VButtonBox {
    using gtk_c = Gtk<double>;
#ifdef ENABLE_CUSTOM_RESPONSE
    static void removeCustomButton(GtkButton *button, void *data){
        auto comboResponse = new(ComboResponse<Type>)(mainWindow, _("Remove Button Contents"), "list-remove");
        auto actualFiles = Settings<Type>::getString("custombuttons", "files");
        if (!actualFiles) return;

        auto paths = g_strsplit(actualFiles, ":", -1);
        auto combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
        auto hbox = Gtk<Type>::hboxNew (FALSE, 6);
        for (auto p=paths; p && *p; p++){
            if (g_file_test(*p, G_FILE_TEST_IS_REGULAR) ) gtk_combo_box_text_append_text (combo,*p);
        }
        g_strfreev(paths);
        gtk_combo_box_set_active (GTK_COMBO_BOX(combo),0);
        gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(combo), TRUE, TRUE, 0);
        gtk_widget_show_all(GTK_WIDGET(hbox));
        gtk_box_pack_start (GTK_BOX (comboResponse->vbox2()), GTK_WIDGET(hbox), TRUE, TRUE, 0);

        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
        auto response = comboResponse->runResponse(0);
        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);
        
        if (response) {
            g_free(response);
            response = gtk_combo_box_text_get_active_text(combo);
            paths = g_strsplit(actualFiles, ":", -1);
            gchar *newPaths=g_strdup("");
            
            for (auto p=paths; p && *p; p++){
                if (strcmp(*p, response)){
                    auto g = g_strconcat(newPaths, (strlen(newPaths))?":":"", *p, NULL);
                    g_free(newPaths);
                    newPaths = g;
                }
            }
            if (!newPaths || !strlen(newPaths)) gtk_widget_hide(GTK_WIDGET(button));
            g_strfreev(paths);
            // save setting
            Settings<Type>::setString("custombuttons", "files", newPaths);
            g_free(newPaths);

            // hide button with data response
            auto list = gtk_container_get_children (GTK_CONTAINER(data));
            for (auto p=list; p && p->data; p=p->next){
                auto file = (gchar *)g_object_get_data(G_OBJECT(p->data), "file");
                if (file && strcmp(file, response)==0){
                    gtk_widget_hide(GTK_WIDGET(p->data));
                    break;
                }
            }

            DBG("response=%s\n", response);
        }
        g_free(response);
    }
    static void newCustomButton(GtkButton *button, void *data){
        auto creator = new (xf::iniCreator<Type>)(20);
        gtk_main();
    }

    static void addCustomButton(GtkButton *button, void *data){
        auto entryResponse = new(EntryFileResponse<Type>)(mainWindow, _("Add custom content"), "list-add");
        entryResponse->setEntryLabel(_("Select file to load"));
        
        auto entry = entryResponse->entry();
        auto chooserButton = entryResponse->chooserButton();
        auto wd = Fm<Type>::getCurrentDirectory();

        entryResponse->setEntryBashFileCompletion(wd);
        entryResponse->setInLineCompletion(1);
        auto view = Fm<Type>::getCurrentView();

        gchar *path = NULL;
        GList *selectionList;
        if (isTreeView){
            auto treeModel = view->treeModel();
            auto selection = gtk_tree_view_get_selection (view->treeView());
            selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        } else {
            selectionList = gtk_icon_view_get_selected_items (view->iconView());
        }
        view->setSelectionList(selectionList);
        if (selectionList) {
            GtkTreeIter iter;
            if (gtk_tree_model_get_iter (view->treeModel(), &iter, (GtkTreePath *)selectionList->data)){
                gtk_tree_model_get (view->treeModel(), &iter, PATH, &path, -1);                
                if (path){
                    auto g = g_path_get_basename(path);
                    g_free(path);
                    path = g;
                }
            }
        }

        entryResponse->setEntryDefault(path?path:"");
        g_free(path);



        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
        

        auto response = entryResponse->runResponse(0);

        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);

        if (!response) return;
        if (!g_path_is_absolute(response)){
            auto g = g_strconcat(wd, "/", response, NULL);
            g_free(response);
            response = g;
        }
        TRACE("response = %s\n", response);
        
        auto keyFile = g_key_file_new();
        GError *error=NULL;
        g_key_file_load_from_file(keyFile, response,(GKeyFileFlags) (0), &error);
        if (error){
            auto g = g_strdup_printf(_("Cannot load file %s. Reason: %s"), response, error->message);
            Dialogs<Type>::quickHelp(mainWindow, g, "dialog-error");
            g_free(response);
            g_free(g);
            g_key_file_free(keyFile);
            return;
        }
        if (!g_key_file_has_key(keyFile, "custombutton", "exec", &error) && error){
            auto g = g_strdup_printf("%s: %s", response, error->message);
            Dialogs<Type>::quickHelp(mainWindow, g, "dialog-error");
            g_free(response);
            g_free(g);
            g_key_file_free(keyFile);
            return;
           
        }

        

        auto files = Settings<Type>::getString("custombuttons", "files");
        if (!files) {
            Settings<Type>::setString("custombuttons", "files", response);
        } else { 
            auto g = g_strconcat(files, ":", response, NULL);
            Settings<Type>::setString("custombuttons", "files", g);
            g_free(g);
        }

        g_free(response);
        Dialogs<Type>::quickHelp(mainWindow, _("You need to restart the application"), "dialog-warning");
    }
#endif
public:
    GtkBox *vButtonBox(void){return vButtonBox_;}
    VButtonBox(void){
        vButtonBox_ = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

        GError *error=NULL;
        GtkStyleContext *style_context = gtk_widget_get_style_context (GTK_WIDGET(vButtonBox_));
        gtk_style_context_add_class(style_context, GTK_STYLE_CLASS_BUTTON );
        GtkCssProvider *css_provider = gtk_css_provider_new();
        gtk_css_provider_load_from_data (css_provider, 
 //     background-color: #dcdad5;
    "\
    box * {\
      background-color: #888888;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    ", 
            -1, &error);
        if (error){
            ERROR("vbuttonbox.hh::VButtonBox(): %s\n", error->message);
            g_error_free(error);
        } else {
            gtk_style_context_add_provider (style_context, GTK_STYLE_PROVIDER(css_provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
#ifdef ENABLE_PKG_MODULE
#ifdef HAVE_PKG
        auto pkg = 
            gtk_c::newButton("emblem-bsd", _("Software Updater"));
#else
# ifdef HAVE_PACMAN
        auto pkg = 
            gtk_c::newButton("emblem-archlinux", _("Software Updater"));
# else
#  ifdef HAVE_EMERGE
        auto pkg = 
            gtk_c::newButton("emblem-gentoo", _("Software Updater"));
#  else
        auto pkg = 
            gtk_c::newButton("help-about-symbolic", _("Software Updater"));
#  endif
# endif
#endif
        gtk_box_pack_end (vButtonBox_, GTK_WIDGET(pkg), FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(pkg), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::pkg), NULL);
#endif


#ifdef ENABLE_CUSTOM_RESPONSE
        auto newCustom = 
            gtk_c::newButton("document-new", _("FIXME New custom content"));
        g_signal_connect(G_OBJECT(newCustom), "clicked", G_CALLBACK(newCustomButton), NULL);
        gtk_box_pack_end (vButtonBox_, GTK_WIDGET(newCustom), FALSE, FALSE, 0); 
        
        auto addCustom = 
            gtk_c::newButton("list-add", _("Add custom content"));
        g_signal_connect(G_OBJECT(addCustom), "clicked", G_CALLBACK(addCustomButton), NULL);
        gtk_box_pack_end (vButtonBox_, GTK_WIDGET(addCustom), FALSE, FALSE, 0);

        auto removeCustom = 
            gtk_c::newButton("list-remove", _("Remove Button Contents"));
        g_signal_connect(G_OBJECT(removeCustom), "clicked", G_CALLBACK(removeCustomButton), vButtonBox_);
        gtk_box_pack_end (vButtonBox_, GTK_WIDGET(removeCustom), FALSE, FALSE, 0);
        auto customButtonFiles = Settings<Type>::getString("custombuttons", "files");
        if (customButtonFiles){
            gchar **files;
            if (strchr(customButtonFiles ,':')) {
                files = g_strsplit(customButtonFiles, ":",-1);
            } else {
               files = (gchar **)calloc(2, sizeof(gchar *)); 
               files[0] = g_strdup(customButtonFiles);
            }
            for (auto p=files; p && *p; p++){
                if (g_file_test(*p, G_FILE_TEST_EXISTS)){
                    auto custombutton = CustomResponse<Type>::custombutton(*p);
                    g_object_set_data(G_OBJECT(custombutton), "file", g_strdup(*p));
                    if (custombutton) gtk_box_pack_end (vButtonBox_, GTK_WIDGET(custombutton), FALSE, FALSE, 0);
                }
            }
            g_strfreev(files);
        }
#endif
        auto search = gtk_c::newButton(SEARCH, _("Search"));
        gtk_box_pack_start (vButtonBox_, GTK_WIDGET(search), FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(search), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::search), NULL);

        auto terminal = gtk_c::newButton(UTILITIES_TERMINAL, _("Open terminal"));
        gtk_box_pack_start (vButtonBox_, GTK_WIDGET(terminal), FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(terminal), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::terminal), NULL);

        auto newWindow = gtk_c::newButton(WINDOW_NEW, _("Open a New Window"));
        gtk_box_pack_start (vButtonBox_, GTK_WIDGET(newWindow), FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(newWindow), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::open), 
                (void *)xffmProgram);

#ifdef ENABLE_DIFF_MODULE
        auto diffApp = g_find_program_in_path("rodent-diff");
        if (diffApp) {
            auto differences = gtk_c::newButton("differences", _("Differences"));
            gtk_box_pack_start (vButtonBox_, GTK_WIDGET(differences), FALSE, FALSE, 0);
            g_signal_connect(G_OBJECT(differences), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::open), 
                (void *)"rodent-diff");
        }
#endif

        auto home = gtk_c::newButton(GO_HOME, _("Home Directory"));
        gtk_box_pack_start (vButtonBox_, GTK_WIDGET(home), FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(home), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::home), NULL);

        // Increase image preview size button
        auto imageUpButton = gtk_c::newButton(IMAGE_X_GENERIC "/SE/list-add/1.5/220", _("Reset image size"));
        gtk_box_pack_start (vButtonBox_, GTK_WIDGET(imageUpButton), FALSE, FALSE, 0);
        g_signal_connect(imageUpButton, "clicked", G_CALLBACK(upImage), NULL);
        // Decrease image preview size button
        auto imageDownButton = gtk_c::newButton(IMAGE_X_GENERIC "/SE/list-remove/1.5/220", _("Reset image size"));
        gtk_box_pack_start (vButtonBox_, GTK_WIDGET(imageDownButton), FALSE, FALSE, 0);
        g_signal_connect(imageDownButton, "clicked", G_CALLBACK(downImage), NULL);



#if 0
#ifdef ENABLE_FSTAB_MODULE
        auto fstab = gtk_c::newButton("media-eject", _("Disk Image Mounter"));
        gtk_box_pack_start (vButtonBox_, GTK_WIDGET(fstab), FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(fstab), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::fstab), NULL);
#endif

        auto trash = gtk_c::newButton("user-trash", _("Trash bin"));
        gtk_box_pack_start (vButtonBox_, GTK_WIDGET(trash), FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(trash), "clicked", G_CALLBACK(MenuPopoverSignals<Type>::trash), NULL);
#endif

        gtk_widget_show_all(GTK_WIDGET(vButtonBox_));
        
#ifdef ENABLE_CUSTOM_RESPONSE
        if (!customButtonFiles || !strlen(customButtonFiles)) 
            gtk_widget_hide(GTK_WIDGET(removeCustom));
        g_free(customButtonFiles);
#endif
        return ;
    }
protected:
private:
    GtkBox *vButtonBox_;
    static GtkScale *newSizeScale(void){
        auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 6.0, 24.0, 6.0));
        gtk_range_set_value(GTK_RANGE(size_scale), 12);
        gtk_range_set_increments (GTK_RANGE(size_scale), 2.0, 6.0);
        gtk_widget_set_size_request (GTK_WIDGET(size_scale),-1,75);
        return size_scale;
    }
    static void removeButton(GtkButton *button, void *data){
        auto entry = GTK_ENTRY(data);
        auto file = gtk_entry_get_text(entry);
        DBG("remove %s\n", file);
    }
    
    static gboolean
    downImage (GtkWidget *eventBox,
               GdkEvent  *event,
               gpointer   data) {
        auto page = Fm<Type>::getCurrentPage();
        auto view = page->view();
        auto pixels = Settings<Type>::getInteger("ImageSize", page->workDir());
        page->setImageSize(pixels/2);
        view->reloadModel();
        return FALSE;
    }

    static gboolean
    upImage (GtkWidget *eventBox,
               GdkEvent  *event,
               gpointer   data) {
        auto page = Fm<Type>::getCurrentPage();
        auto view = page->view();

        auto pixels = Settings<Type>::getInteger("ImageSize", page->workDir());
        if (pixels > 0) pixels *=2;
        else pixels = 48;
        page->setImageSize(pixels);
        if (pixels <= MAX_PIXBUF_SIZE) view->reloadModel();
        return FALSE;
    }

};
   



}



#endif

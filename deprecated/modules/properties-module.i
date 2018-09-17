#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 * Edscott Wilson Garcia Copyright 2001-2011
 *
 * Copyright (C) 1999 Rasca, Berlin
 * EMail: thron@gmx.de
 *
 * Olivier Fourdan (fourdan@xfce.org)
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  */

typedef struct {
    GtkWidget *top;
    GtkWidget *user;
    GtkWidget *group;
    struct stat *st;
    int result;
    int type;
} dlg;

typedef struct row_t {
    GtkWidget *w[5];
    gboolean flag;
} row_t;

typedef struct properties_t {
    dlg dl;
    mode_t new_u_m;
    mode_t new_g_m;
    mode_t new_o_m;
    uid_t new_owner;
    uid_t new_group;
    row_t u_row;
    row_t g_row;
    row_t o_row;
    row_t row_chown;
    row_t row_chgrp;
    record_entry_t *en;
    struct stat st;
    GSList *select_list;
    GSList *g_user;
    GSList *g_group;
    GdkPixbuf *preview_pixbuf;
    gchar *module_txt;
    GtkWidget *ok;
}properties_t;
    


static void
clean_up(properties_t *properties_p){
    GSList *tmp = properties_p->select_list;
    for(; tmp; tmp = tmp->next) {
	rfm_destroy_entry((record_entry_t *) tmp->data);
    }
    g_slist_free (properties_p->select_list);
    /* free the lists */
    for(tmp = properties_p->g_user;tmp; tmp = tmp->next) {
        g_free (tmp->data);
    }
    g_slist_free (properties_p->g_user);
    for(tmp = properties_p->g_group;tmp; tmp = tmp->next) {
        g_free (tmp->data);
    }
    g_slist_free (properties_p->g_group);
    if (properties_p->preview_pixbuf){
	g_object_unref(properties_p->preview_pixbuf);
    }
    if (properties_p->preview_pixbuf){
	g_object_unref(properties_p->preview_pixbuf);
    }
    g_free(properties_p->module_txt);
    g_free(properties_p);
}

static GtkWidget *
label_new (const char *text, GtkJustification j_type) {
    GtkWidget *label;
    label = gtk_label_new ("");
    gtk_label_set_markup(GTK_LABEL(label), text);
    gtk_label_set_justify (GTK_LABEL (label), j_type);
    /*if(j_type == GTK_JUSTIFY_RIGHT) {
        gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    } else {
        gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    }*/
    return (label);
}

static void
set_sensitive (GtkWidget * widget, void *data) {
    gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
}

static void
on_off (GtkWidget * toggle, void *data) {
    int i;
    row_t *row = (row_t *) data;
    if(row->flag)
        row->flag = FALSE;
    else
        row->flag = TRUE;
    for(i = 0; i < 5; i++)
        gtk_widget_set_sensitive (row->w[i], row->flag);
}

static void
on_off2 (GtkWidget * toggle, void *data) {
    int i;
    row_t *row = (row_t *) data;
    if(row->flag)
        row->flag = FALSE;
    else
        row->flag = TRUE;
    for(i = 0; i < 2; i++)
        gtk_widget_set_sensitive (row->w[i], row->flag);
}

static void
on_cancel (GtkWidget * btn, gpointer data) {
    properties_t *properties_p=data;
    gtk_widget_hide(properties_p->dl.top);
    gtk_widget_destroy(properties_p->dl.top);
}

static void
on_destroy (GtkWidget * btn, gpointer data) {
    properties_t *properties_p=data;
    clean_up(properties_p);
    //gtk_main_quit();
}

static void
on_ok (GtkWidget * ok, gpointer data) {
    properties_t *properties_p=data;
    char *val;
    struct passwd *pw;
    struct group *gr;

    val = (char *)gtk_entry_get_text (GTK_ENTRY (properties_p->dl.user));
    if(val) {
        pw = getpwnam (val);
        if(pw) {
            if(properties_p->dl.st)
                properties_p->new_owner = properties_p->dl.st->st_uid = pw->pw_uid;
            else
                properties_p->new_owner = pw->pw_uid;
        }
    }
    val = (char *)gtk_entry_get_text (GTK_ENTRY (properties_p->dl.group));
    if(val) {
        gr = getgrnam (val);
        if(gr) {
            if(properties_p->dl.st)
                properties_p->new_group = properties_p->dl.st->st_gid = gr->gr_gid;
            else
                properties_p->new_group = gr->gr_gid;
        }
    }

    gchar *argv[5];
    char argument_s[64];

        if(properties_p->en) {
	    // single item
            /*en->st->st_mtime=0; */
            /* argument variable must be thread safe */
            if(properties_p->en->st->st_mode != properties_p->st.st_mode &&
		    !IS_BROKEN_LNK (properties_p->en->type)) {
                /* chmod() on a symlink itself isn't possible */
                if(chmod (properties_p->en->path, properties_p->st.st_mode) == -1) {
                    sprintf (argument_s, "0%o", properties_p->st.st_mode & 0777);
                    argv[0] = "chmod";
                    argv[1] = argument_s;
                    argv[2] = properties_p->en->path;
                    argv[3] = NULL;
		    widgets_t *widgets_p = rfm_get_widget("widgets_p");
                    RFM_TRY_SUDO (widgets_p, argv, FALSE);
                } else {
                    properties_p->en->st->st_mode = properties_p->st.st_mode;
                }
            }
            if((properties_p->en->st->st_uid != properties_p->st.st_uid || 
		    properties_p->en->st->st_gid != properties_p->st.st_gid) &&
		    !IS_BROKEN_LNK (properties_p->en->type)) {

                TRACE( "PROP: chown now\n");
                if(chown (properties_p->en->path, properties_p->new_owner, properties_p->new_group) == -1) {
                    sprintf (argument_s, "%d:%d", properties_p->st.st_uid, properties_p->st.st_gid);
		    gint i=0;
                    argv[i++] = "chown";
 		    if (IS_SDIR(properties_p->en->type)){
			argv[i++] = "-R";
		    } 
		    argv[i++] = argument_s;
                    argv[i++] = properties_p->en->path;
                    argv[i] = NULL;
                    TRACE( "PROP: trysudo, %s %s %s\n", argv[0], argv[1], argv[2]);
		    widgets_t *widgets_p = rfm_get_widget("widgets_p");
                    RFM_TRY_SUDO (widgets_p, argv, FALSE);
                } else {
                    TRACE( "PROP: chown OK!\n");
                    properties_p->en->st->st_uid = properties_p->st.st_uid;
                    properties_p->en->st->st_gid = properties_p->st.st_gid;
                }
            }
        } else {
	    // multiple items
            struct stat st;
	    GSList *tmp = properties_p->select_list;
            for(; tmp; tmp = tmp->next) {
                record_entry_t *en = (record_entry_t *) tmp->data;
                argv[1] = argument_s;
                argv[2] = en->path;
                argv[3] = NULL;

                /*en->st->st_mtime=0; */
                if(properties_p->row_chown.flag) {
                    if(chown (en->path, properties_p->new_owner, -1) == -1) {
                        sprintf (argument_s, "%d", properties_p->new_owner);
                        argv[0] = "chown";
			widgets_t *widgets_p = rfm_get_widget("widgets_p");
                        RFM_TRY_SUDO (widgets_p, argv, FALSE);
                    }
                }
                if(properties_p->row_chgrp.flag) {
                    if(chown (en->path, -1, properties_p->new_group) == -1) {
                        sprintf (argument_s, "%d", properties_p->new_group);
                        argv[0] = "chgrp";
			widgets_t *widgets_p = rfm_get_widget("widgets_p");
                        RFM_TRY_SUDO (widgets_p, argv, FALSE);
                    }
                }
                if(properties_p->u_row.flag && stat (en->path, &st) >= 0) {
                    st.st_mode &= (S_IRWXO | S_IRWXG);
                    st.st_mode |= properties_p->new_u_m;
                    if(chmod (en->path, st.st_mode) == -1) {
                        argv[0] = "chmod";
                        sprintf (argument_s, "0%o", properties_p->st.st_mode & 0777);
			widgets_t *widgets_p = rfm_get_widget("widgets_p");
                        RFM_TRY_SUDO (widgets_p, argv, FALSE);
                    }
                }
                if(properties_p->g_row.flag && stat (en->path, &st) >= 0) {
                    st.st_mode &= (S_IRWXU | S_IRWXO);
                    st.st_mode |= properties_p->new_g_m;
                    if(chmod (en->path, properties_p->st.st_mode) == -1) {
                        argv[0] = "chmod";
                        sprintf (argument_s, "0%o", st.st_mode & 0777);
			widgets_t *widgets_p = rfm_get_widget("widgets_p");
                        RFM_TRY_SUDO (widgets_p, argv, FALSE);
                    }
                }
                if(properties_p->o_row.flag && stat (en->path, &st) >= 0) {
                    st.st_mode &= (S_IRWXU | S_IRWXG);
                    st.st_mode |= properties_p->new_o_m;
                    if(chmod (en->path, st.st_mode) == -1) {
                        argv[0] = "chmod";
                        sprintf (argument_s, "0%o", properties_p->st.st_mode & 0777);
			widgets_t *widgets_p = rfm_get_widget("widgets_p");
                        RFM_TRY_SUDO (widgets_p, argv, FALSE);
                    }
                }
            }
        }
    gtk_widget_hide(properties_p->dl.top);
    gtk_widget_destroy(properties_p->dl.top);
	

}

static void
perm (GtkWidget * toggle, properties_t *properties_p, int bit) {
    gtk_widget_set_sensitive(properties_p->ok, TRUE);
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (toggle))) {
        if(properties_p->dl.st)
            properties_p->dl.st->st_mode |= (mode_t) bit;
        if((mode_t) bit & S_IRWXU)
            properties_p->new_u_m |= (mode_t) bit;
        if((mode_t) bit & S_IRWXG)
            properties_p->new_g_m |= (mode_t) bit;
        if((mode_t) bit & S_IRWXO)
            properties_p->new_o_m |= (mode_t) bit;
        if((mode_t) bit & 07000) {
            properties_p->new_u_m |= (mode_t) bit;
            properties_p->new_g_m |= (mode_t) bit;
            properties_p->new_o_m |= (mode_t) bit;
        }
    } else {
        if(properties_p->dl.st)
            properties_p->dl.st->st_mode &= (mode_t) ~ bit;
        if((mode_t) bit & S_IRWXU)
            properties_p->new_u_m &= (mode_t) ~ bit;
        if((mode_t) bit & S_IRWXG)
            properties_p->new_g_m &= (mode_t) ~ bit;
        if((mode_t) bit & S_IRWXO)
            properties_p->new_o_m &= (mode_t) ~ bit;
        if((mode_t) bit & 07000) {
            properties_p->new_u_m &= (mode_t) ~ bit;
            properties_p->new_g_m &= (mode_t) ~ bit;
            properties_p->new_o_m &= (mode_t) ~ bit;
        }
    }

}

static void
cb_perm_S_IRUSR (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_IRUSR);
}

static void
cb_perm_S_IWUSR (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_IWUSR);
}

static void
cb_perm_S_IXUSR (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_IXUSR);
}

static void
cb_perm_S_ISUID (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_ISUID);
}

static void
cb_perm_S_IRGRP (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_IRGRP);
}

static void
cb_perm_S_IWGRP (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_IWGRP);
}

static void
cb_perm_S_IXGRP (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_IXGRP);
}

static void
cb_perm_S_ISGID (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_ISGID);
}

static void
cb_perm_S_IROTH (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_IROTH);
}

static void
cb_perm_S_IWOTH (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_IWOTH);
}

static void
cb_perm_S_IXOTH (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_IXOTH);
}

static void
cb_perm_S_ISVTX (GtkWidget * toggle, void *data) {
    perm (toggle, (properties_t *)data, S_ISVTX);
}

static gint
on_key_press (GtkWidget * w, GdkEventKey * event, void *data) {
    properties_t *properties_p=data;
    if(event->keyval == GDK_KEY_Escape) {
	gtk_widget_hide(properties_p->dl.top);
	gtk_widget_destroy(properties_p->dl.top);
        return (TRUE);
    }
    return (FALSE);
}

static void *
dlg_prop (void *data) {
    properties_t *properties_p = data;
    GtkWidget *ok = NULL,
        *cancel = NULL,
        *label,
        *notebook,
        *owner[4],
        *perm[20],
        *vbox,
        *hbox;
     GtkGrid *table;
    if(properties_p->en){
	population_t *private_population_p=
		(population_t *)malloc(sizeof(population_t));
	if (private_population_p){
            private_population_p->en=rfm_copy_entry(properties_p->en);
            properties_p->preview_pixbuf = rfm_natural(RFM_MODULE_DIR, "mime",  private_population_p, "mime_preview");  
            // since properties module is now detached from view_p,
            // we must have a separate reference for the preview_pixbuf
            // this call does not shortcircuit with possibly generated pixbuf
            // on mouse over icon (just avoid the race condition)
            if (GDK_IS_PIXBUF(properties_p->preview_pixbuf)){
                g_object_ref(properties_p->preview_pixbuf);
            }
	    rfm_destroy_entry(private_population_p->en);
	    g_free(private_population_p);
        }
    }


#ifdef HAVE_LOCALTIME_R
    //struct tm t_r;
#endif
    //struct tm *t;
    struct passwd *pw;
    struct group *gr;
    gchar buf[1024];
    properties_p->g_user = NULL;
    properties_p->g_group = NULL;
    gchar *path;
    gchar *title;
#ifndef LINE_MAX
# define LINE_MAX  1024
#endif
    gint o = 0;

    if(properties_p->en)
        path = properties_p->en->path;
    else
        path = NULL;
    //dl.result = 0;
    //u_row.flag = g_row.flag = o_row.flag = row_chown.flag = row_chgrp.flag = FALSE;
    properties_p->dl.st = &properties_p->st;

    properties_p->dl.top = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(properties_p->dl.top), GDK_WINDOW_TYPE_HINT_DIALOG);

    if(path) {
        char *b = g_path_get_basename (path);
        gchar *q = rfm_utf_string (rfm_chop_excess (b));
        title = g_strdup_printf ("%s --> %s", _("Properties"), q);
        g_free (q);
        g_free (b);
    } else {
        char *b = g_strdup_printf(ngettext ("%'u item","%'u items",
		    g_slist_length (properties_p->select_list)), 
		g_slist_length (properties_p->select_list));
        title = g_strconcat (_("Properties"), " ---> ", b, NULL);
        g_free (b);
    }
    gtk_window_set_title (GTK_WINDOW (properties_p->dl.top), title);
    //  set properties icon
    GdkPixbuf * pb = rfm_get_pixbuf("xffm/stock_properties", SIZE_ICON);
    gtk_window_set_icon (GTK_WINDOW (properties_p->dl.top), pb);
    g_object_unref(pb);
    
    
    g_free (title);
    g_signal_connect (G_OBJECT (properties_p->dl.top), "destroy", G_CALLBACK (on_destroy), properties_p);



    vbox = rfm_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (properties_p->dl.top), vbox);
    notebook = gtk_notebook_new ();
    gtk_box_pack_start (GTK_BOX (vbox), notebook, FALSE, TRUE, 0);
    
    //hack...
    hbox = rfm_hbox_new (FALSE, 0);
    rfm_set_box_gradient(hbox);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
    gtk_widget_show (hbox);
    
    hbox = rfm_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (notebook);
    gtk_widget_show (vbox);
    //rfm_set_box_gradient(hbox);
    gtk_widget_show (hbox);

    /* ok and cancel buttons */
    properties_p->ok = ok = rfm_dialog_button ("xffm/stock_apply", _("Apply"));
    gtk_widget_set_sensitive(properties_p->ok, FALSE);

    cancel = rfm_dialog_button ("xffm/stock_cancel", _("Cancel"));

    gtk_widget_set_can_default(ok, TRUE);
    gtk_widget_set_can_default(cancel, TRUE);

    gtk_box_pack_start (GTK_BOX (hbox), cancel, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), ok, TRUE, FALSE, 0);



    g_signal_connect (G_OBJECT (ok), "clicked", G_CALLBACK (on_ok),  properties_p);
    g_signal_connect (G_OBJECT (cancel), "clicked", G_CALLBACK (on_cancel), properties_p);
    gtk_widget_grab_default (cancel);
    gtk_widget_show (cancel);
    gtk_widget_show (ok);

    /* date and size page */
    if(properties_p->en) {
        /*gchar *filename=g_path_get_basename(en->path); */

	// Add the icon (or preview)

	if (!properties_p->en->mimetype) {
	    properties_p->en->mimetype = MIME_type(properties_p->en->path, NULL);
	} 
	if (!properties_p->en->mimemagic){
	   if (IS_LOCAL_TYPE(properties_p->en->type)) {
		properties_p->en->mimemagic = rfm_rational(RFM_MODULE_DIR, "mime", properties_p->en, "mime_magic", "mime_function");
		if (!properties_p->en->mimemagic) properties_p->en->mimemagic = g_strdup(_("unknown"));
	   }
	   else properties_p->en->mimemagic = g_strdup(_("unknown"));
	}

	GdkPixbuf *icon_pixbuf=properties_p->preview_pixbuf;
	if (!icon_pixbuf) {
	    gchar *id=rfm_get_entry_icon_id (NULL, properties_p->en, TRUE);
            icon_pixbuf=rfm_get_pixbuf(id, BIG_ICON_SIZE);
	    g_free(id);
        }

        gchar *b = g_path_get_basename (properties_p->en->path);
	gchar *module_text = NULL;
	widgets_t *widgets_p = rfm_get_widget("widgets_p");

	gchar *markup = rodent_get_tip_text (widgets_p->view_p, NULL, properties_p->en, module_text);
	GtkWidget *frame = rfm_create_tooltip_window(NULL, NULL, icon_pixbuf, markup, b);
	g_object_unref(icon_pixbuf);
        g_free (b);
	gtk_widget_show(frame);
        label = gtk_label_new (_("Information"));
       
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

        //gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
	

    }
    /* permissions page */
    /* if (!(flags & IS_STALE_LINK)) */
    {
        label = gtk_label_new (_("Permissions"));

        GtkWidget *permissions_box = rfm_vbox_new(FALSE, 0);
        rfm_set_box_gradient(GTK_WIDGET(permissions_box));
        gtk_widget_show(permissions_box);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), GTK_WIDGET(permissions_box), label);

#if GTK_MAJOR_VERSION==3
        table = GTK_GRID(gtk_grid_new());
#else
        if (properties_p->en) table = GTK_TABLE(gtk_table_new(3,5,FALSE));
        else table = GTK_TABLE(gtk_table_new(3,6,FALSE));
#endif
        gtk_box_pack_start (GTK_BOX (permissions_box), GTK_WIDGET(table), TRUE, FALSE, 0);

        if(!properties_p->en) {
            o = 1;
            perm[15] = gtk_check_button_new_with_label (_("Apply changes"));
            perm[16] = gtk_check_button_new_with_label (_("Apply changes"));
            perm[17] = gtk_check_button_new_with_label (_("Apply changes"));
        } else {
            perm[15] = perm[16] = perm[17] = NULL;
        }
        /*} else if (en && IS_SDIR(en->type)){
           perm[15] = gtk_check_button_new_with_label(_("Recursive"));
           }else perm[15]=NULL; */
        properties_p->u_row.w[0] = perm[0] = gtk_label_new (_("Owner:"));
        properties_p->u_row.w[1] = perm[1] = gtk_check_button_new_with_label (_("Read"));
        if(properties_p->st.st_mode & S_IRUSR)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[1]), 1);
        g_signal_connect (G_OBJECT (perm[1]), "clicked", G_CALLBACK (cb_perm_S_IRUSR), properties_p);
        properties_p->u_row.w[2] = perm[2] = gtk_check_button_new_with_label (_("Write"));
        if(properties_p->st.st_mode & S_IWUSR)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[2]), 1);
        g_signal_connect (G_OBJECT (perm[2]), "clicked", G_CALLBACK (cb_perm_S_IWUSR), properties_p);
        properties_p->u_row.w[3] = perm[3] = gtk_check_button_new_with_label (_("Execute"));
        if(properties_p->st.st_mode & S_IXUSR)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[3]), 1);
        g_signal_connect (G_OBJECT (perm[3]), "clicked", G_CALLBACK (cb_perm_S_IXUSR), properties_p);
        properties_p->u_row.w[4] = perm[4] = gtk_check_button_new_with_label (_("Set UID"));
        if(properties_p->st.st_mode & S_ISUID)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[4]), 1);
        g_signal_connect (G_OBJECT (perm[4]), "clicked", G_CALLBACK (cb_perm_S_ISUID), properties_p);

        gtk_grid_attach (table, perm[0], 0 + o, 0, 1, 1);
        gtk_grid_attach (table, perm[1], 1 + o, 0, 1, 1);
        gtk_grid_attach (table, perm[2], 2 + o, 0, 1, 1);
        gtk_grid_attach (table, perm[3], 3 + o, 0, 1, 1);
        gtk_grid_attach (table, perm[4], 4 + o, 0, 1, 1);

        properties_p->g_row.w[0] = perm[5] = gtk_label_new (_("Group:"));
        properties_p->g_row.w[1] = perm[6] = gtk_check_button_new_with_label (_("Read"));
        if(properties_p->st.st_mode & S_IRGRP)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[6]), 1);
        g_signal_connect (G_OBJECT (perm[6]), "clicked", G_CALLBACK (cb_perm_S_IRGRP), properties_p);
        properties_p->g_row.w[2] = perm[7] = gtk_check_button_new_with_label (_("Write"));
        if(properties_p->st.st_mode & S_IWGRP)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[7]), 1);
        g_signal_connect (G_OBJECT (perm[7]), "clicked", G_CALLBACK (cb_perm_S_IWGRP), properties_p);
        properties_p->g_row.w[3] = perm[8] = gtk_check_button_new_with_label (_("Execute"));
        if(properties_p->st.st_mode & S_IXGRP)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[8]), 1);
        g_signal_connect (G_OBJECT (perm[8]), "clicked", G_CALLBACK (cb_perm_S_IXGRP), properties_p);
        properties_p->g_row.w[4] = perm[9] = gtk_check_button_new_with_label (_("Set GID"));
        if(properties_p->st.st_mode & S_ISGID)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[9]), 1);
        g_signal_connect (G_OBJECT (perm[9]), "clicked", G_CALLBACK (cb_perm_S_ISGID), properties_p);
        gtk_grid_attach (table, perm[5], 0 + o, 1, 1, 1);
        gtk_grid_attach (table, perm[6], 1 + o, 1, 1, 1);
        gtk_grid_attach (table, perm[7], 2 + o, 1, 1, 1);
        gtk_grid_attach (table, perm[8], 3 + o, 1, 1, 1);
        gtk_grid_attach (table, perm[9], 4 + o, 1, 1, 1);

        gchar *tt=g_strdup_printf("%s ",_("Others:"));
        properties_p->o_row.w[0] = perm[10] = gtk_label_new (tt);
        g_free(tt);
        properties_p->o_row.w[1] = perm[11] = gtk_check_button_new_with_label (_("Read"));
        if(properties_p->st.st_mode & S_IROTH)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[11]), 1);
        g_signal_connect (G_OBJECT (perm[11]), "clicked", G_CALLBACK (cb_perm_S_IROTH), properties_p);
        properties_p->o_row.w[2] = perm[12] = gtk_check_button_new_with_label (_("Write"));
        if(properties_p->st.st_mode & S_IWOTH)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[12]), 1);
        g_signal_connect (G_OBJECT (perm[12]), "clicked", G_CALLBACK (cb_perm_S_IWOTH), properties_p);
        properties_p->o_row.w[3] = perm[13] = gtk_check_button_new_with_label (_("Execute"));
        if(properties_p->st.st_mode & S_IXOTH)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[13]), 1);
        g_signal_connect (G_OBJECT (perm[13]), "clicked", G_CALLBACK (cb_perm_S_IXOTH), properties_p);
        properties_p->o_row.w[4] = perm[14] = gtk_check_button_new_with_label (_("Sticky"));
        if(properties_p->st.st_mode & S_ISVTX)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (perm[14]), 1);
        g_signal_connect (G_OBJECT (perm[14]), "clicked", G_CALLBACK (cb_perm_S_ISVTX), properties_p);
 
	if(perm[15]) {
	    gtk_grid_attach (table, perm[15], 0, 0, 1, 1);
            g_signal_connect (G_OBJECT (perm[15]), "clicked", G_CALLBACK (on_off), (gpointer) (&properties_p->u_row));
            g_signal_connect (G_OBJECT (perm[15]), "clicked", G_CALLBACK (set_sensitive), (gpointer) (properties_p->ok));
        }
        if(perm[16]) {
	    gtk_grid_attach (table, perm[16], 0, 1, 1, 1);
            //gtk_table_attach (GTK_TABLE (table), perm[16], 0, 1, 1, 2, 0, 0, X_PAD, 0);
            g_signal_connect (G_OBJECT (perm[16]), "clicked", G_CALLBACK (on_off), (gpointer) (&properties_p->g_row));
            g_signal_connect (G_OBJECT (perm[16]), "clicked", G_CALLBACK (set_sensitive), (gpointer) (properties_p->ok));
        }
        if(perm[17]) {
	    gtk_grid_attach (table, perm[17], 0, 2, 1, 1);
            //gtk_table_attach (GTK_TABLE (table), perm[17], 0, 1, 2, 3, 0, 0, X_PAD, 0);
            g_signal_connect (G_OBJECT (perm[17]), "clicked", G_CALLBACK (on_off), (gpointer) (&properties_p->o_row));
            g_signal_connect (G_OBJECT (perm[17]), "clicked", G_CALLBACK (set_sensitive), (gpointer) (properties_p->ok));
        }

        gtk_grid_attach (table, perm[10], 0 + o, 2, 1, 1);
        gtk_grid_attach (table, perm[11], 1 + o, 2, 1, 1);
        gtk_grid_attach (table, perm[12], 2 + o, 2, 1, 1);
        gtk_grid_attach (table, perm[13], 3 + o, 2, 1, 1);
        gtk_grid_attach (table, perm[14], 4 + o, 2, 1, 1);

        if(perm[15]) {
            for(o = 0; o < 15; o++)
                gtk_widget_set_sensitive (perm[o], FALSE);
        }
        int i = 0,
            j = 2;
	gchar *ku=g_strdup_printf("<b>%s:</b>",
		(g_slist_length(properties_p->select_list) > 1)?
		_("Paths"): _("Path"));
        label = label_new (ku, GTK_JUSTIFY_RIGHT);
	g_free(ku);
	gtk_grid_attach (table, label, 0, 4, 1, 1);
        i = 5;
        GSList *tmp = properties_p->select_list;
        for(; tmp; tmp = tmp->next, i++) {
            record_entry_t *en = (record_entry_t *) tmp->data;
            char *b;
            //b = g_path_get_basename (en->path);
            b = g_strdup (en->path);
            gchar *q = rfm_utf_string (rfm_chop_excess (b));
            label = label_new (q, GTK_JUSTIFY_LEFT);   
            g_free (b);
            g_free (q);
	    gtk_grid_attach (table, label, 1, i, 4, 1);
            j=0;
            if(i >= 15) {
		label = label_new (" ", GTK_JUSTIFY_LEFT);
		gtk_grid_attach (table, label, 1, i + 1, 1, 1);
		gchar *z = g_strdup_printf("<i>%s</i>", _("More..."));
                label = label_new (z, GTK_JUSTIFY_LEFT);
		g_free(z);
		gtk_grid_attach (table, label, 1, i + 2, 1, 1);
		j=2;
                break;
            }

        }
	if (!j){
		label = label_new (" ", GTK_JUSTIFY_LEFT);
		gtk_grid_attach (table, label, 1, i + 1, 1, 1);
		j=1;
	}
	gchar *plural = g_strdup_printf (ngettext("%'u item", "%'u items",  
			g_slist_length (properties_p->select_list)), 
		    g_slist_length (properties_p->select_list));
	gchar *b = g_strdup_printf ("<i>(%s)</i>",plural);
	label = label_new (b, GTK_JUSTIFY_LEFT);
	g_free (b);
	gtk_grid_attach (table, label, 1, i + j + 1, 1, 1);
	gtk_widget_show_all(GTK_WIDGET(table));
        /* end permissions page */
    }

    /* owner/group page */
    if(properties_p->en)  o = 0;  else  o = 1;
    while((pw = getpwent ()) != NULL) {
        properties_p->g_user = g_slist_append (properties_p->g_user, g_strdup (pw->pw_name));
    }
    properties_p->g_user = g_slist_sort (properties_p->g_user, (GCompareFunc) strcmp);
    endpwent ();

    while((gr = getgrent ()) != NULL) {
        properties_p->g_group = g_slist_append (properties_p->g_group, g_strdup (gr->gr_name));
    }
    endgrent ();
    properties_p->g_group = g_slist_sort (properties_p->g_group, (GCompareFunc) strcmp);

    label = gtk_label_new (_("Owner"));
    GtkWidget *owner_box = rfm_vbox_new(FALSE, 0);
    rfm_set_box_gradient(GTK_WIDGET(owner_box));
    gtk_widget_show(owner_box);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), GTK_WIDGET(owner_box), label);

#if GTK_MAJOR_VERSION==3
    table = GTK_GRID(gtk_grid_new());
#else
    if (properties_p->en) table = GTK_TABLE(gtk_table_new(3,2,FALSE));
    else table = GTK_TABLE(gtk_table_new(3,3,FALSE));
#endif
    gtk_box_pack_start (GTK_BOX (owner_box), GTK_WIDGET(table), TRUE, FALSE, 0);

    if(!properties_p->en) {
        perm[15] = gtk_check_button_new_with_label (_("Apply"));
        perm[16] = gtk_check_button_new_with_label (_("Apply"));
    }

    if(properties_p->en)
        pw = getpwuid (properties_p->st.st_uid);
    else
        pw = getpwuid (geteuid ());
    sprintf (buf, "%s", pw ? pw->pw_name : _("unknown"));
    properties_p->g_user = g_slist_prepend (properties_p->g_user, g_strdup (buf));
    

    GtkListStore *usermodel=gtk_list_store_new (1, G_TYPE_STRING);
    rfm_set_store_data_from_list (usermodel, &properties_p->g_user);

#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    properties_p->row_chown.w[1] = owner[1] = 
	gtk_combo_box_entry_new_with_model((GtkTreeModel *)usermodel, 0);
#else
    properties_p->row_chown.w[1] = owner[1] = gtk_combo_box_new_with_entry();
    gtk_combo_box_set_model (GTK_COMBO_BOX(owner[1]),(GtkTreeModel *)usermodel);
    gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX(owner[1]),0);
#endif
    
    properties_p->dl.user = gtk_bin_get_child(GTK_BIN(owner[1]));

    gtk_entry_set_text (GTK_ENTRY (properties_p->dl.user), buf);

    properties_p->row_chown.w[0] = owner[0] = label_new (_("Owner:"), GTK_JUSTIFY_RIGHT);
    gtk_grid_attach (table, owner[0], 0 + o, 0, 1, 1);
    gtk_grid_attach (table, owner[1], 1 + o, 0, 1, 1);
                                                         /*gint left,
                                                         gint top,
                                                         gint width,
                                                         gint height*/

    if(properties_p->en && !rfm_write_ok_path(properties_p->en->path)) {
        gchar *g = g_find_program_in_path ("sudo");
        if(!g)
            gtk_widget_set_sensitive (owner[1], FALSE);
        g_free (g);
    }

    if(properties_p->en)
        gr = getgrgid (properties_p->st.st_gid);
    else
        gr = getgrgid (geteuid ());
    sprintf (buf, "%s", gr ? gr->gr_name : _("unknown"));
    properties_p->g_group = g_slist_prepend (properties_p->g_group, g_strdup (buf));

    GtkListStore *groupmodel=gtk_list_store_new (1, G_TYPE_STRING);
    rfm_set_store_data_from_list (groupmodel, &properties_p->g_group);
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    properties_p->row_chgrp.w[1] = owner[3] = 
	gtk_combo_box_entry_new_with_model((GtkTreeModel *)groupmodel, 0);
#else
    properties_p->row_chgrp.w[1] = owner[3] = gtk_combo_box_new_with_entry();
    gtk_combo_box_set_model (GTK_COMBO_BOX(owner[3]),(GtkTreeModel *)groupmodel);
    gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX(owner[3]),0);
    GtkWidget *entry = gtk_bin_get_child(GTK_BIN(owner[1]));
    rfm_set_box_gradient(GTK_WIDGET(entry));
    entry = gtk_bin_get_child(GTK_BIN(owner[3]));
    rfm_set_box_gradient(GTK_WIDGET(entry));
                                                        
#endif
    properties_p->dl.group = gtk_bin_get_child (GTK_BIN(owner[3]));
    gtk_entry_set_text (GTK_ENTRY (properties_p->dl.group), buf);

    properties_p->row_chgrp.w[0] = owner[2] = label_new (_("Group:"), GTK_JUSTIFY_RIGHT);
    gtk_grid_attach (table, owner[2], 0 + o, 1, 1, 1);
    gtk_grid_attach (table, owner[3], 1 + o, 1, 1, 1);

    if(properties_p->en && !rfm_write_ok_path(properties_p->en->path)) {
        gchar *g = g_find_program_in_path ("sudo");
        if(!g)
            gtk_widget_set_sensitive (owner[3], FALSE);
        g_free (g);
    }

    g_signal_connect (G_OBJECT (properties_p->row_chown.w[1]), "changed", 
	    G_CALLBACK (set_sensitive), (gpointer) (properties_p->ok));
    g_signal_connect (G_OBJECT (properties_p->row_chgrp.w[1]), "changed", 
	    G_CALLBACK (set_sensitive), (gpointer) (properties_p->ok));

    if(!properties_p->en) {
        for(o = 0; o < 4; o++)
            gtk_widget_set_sensitive (owner[o], FALSE);
        g_signal_connect (G_OBJECT (perm[15]), "clicked", G_CALLBACK (on_off2), (gpointer) (&properties_p->row_chown));
        g_signal_connect (G_OBJECT (perm[15]), "clicked", G_CALLBACK (set_sensitive), (gpointer) (properties_p->ok));
        g_signal_connect (G_OBJECT (perm[16]), "clicked", G_CALLBACK (on_off2), (gpointer) (&properties_p->row_chgrp));
        g_signal_connect (G_OBJECT (perm[16]), "clicked", G_CALLBACK (set_sensitive), (gpointer) (properties_p->ok));
	gtk_grid_attach (table, perm[15], 0, 0, 1, 1);
	gtk_grid_attach (table, perm[16], 0, 1, 1, 1);
    }
    {
        int i = 0,
            j = 2;
	gchar *ku=g_strdup_printf("<b>%s:</b>",
		(g_slist_length(properties_p->select_list) > 1)?
		_("Paths"): _("Path"));
	label = label_new (ku, GTK_JUSTIFY_RIGHT);
	g_free(ku);
	gtk_grid_attach (table, label, 0, 2, 1, 1);
        i = 1;
        GSList *tmp = properties_p->select_list;
        for(; tmp; tmp = tmp->next, i++) {
            record_entry_t *en = (record_entry_t *) tmp->data;
            char *b;
            //b = g_path_get_basename (en->path);
            b = g_strdup (en->path);
            gchar *q = rfm_utf_string (rfm_chop_excess (b));
            label = label_new (q, GTK_JUSTIFY_LEFT);   
            g_free (b);
            g_free (q);
	    gtk_grid_attach (table, label, 1, i + 2, 2, 1);
	    j=0;
            if(i >= 10) {
		label = label_new (" ", GTK_JUSTIFY_LEFT);
		gtk_grid_attach (table, label, 1, i + 3, 1, 1);
		gchar *z = g_strdup_printf("<i>%s</i>", _("More..."));
                label = label_new (z, GTK_JUSTIFY_LEFT);
		g_free(z);
		gtk_grid_attach (table, label, 1, i + 4, 1, 1);
		j=2;
                break;
            }
        }
	if (!j){
		label = label_new (" ", GTK_JUSTIFY_LEFT);
		gtk_grid_attach (table, label, 1, i + 3, 1, 1);
		j=1;
	}
	gchar *plural = g_strdup_printf (ngettext("%'u item", "%'u items",  
			g_slist_length (properties_p->select_list)), 
		    g_slist_length (properties_p->select_list));
	gchar *b = g_strdup_printf ("<i>(%s)</i>",plural);
	label = label_new (b, GTK_JUSTIFY_LEFT);
	g_free (b);
	gtk_grid_attach (table, label, 1, i + j + 4, 1, 1);

	gtk_widget_show_all(GTK_WIDGET(table));
    }

    g_signal_connect (G_OBJECT (properties_p->dl.top), "key_press_event", G_CALLBACK (on_key_press), properties_p);
    gtk_window_set_position (GTK_WINDOW (properties_p->dl.top), GTK_WIN_POS_MOUSE);
    //gdk_window_raise(gtk_widget_get_window (properties_p->dl.top));
    //gtk_widget_realize (properties_p->dl.top);

    gtk_widget_show (properties_p->dl.top);
    //gtk_widget_show_all (properties_p->dl.top);
    


    return NULL;
}

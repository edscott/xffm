#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif
/*
 * Copyright (C) 2002-2012 Edscott Wilson Garcia
 * EMail: edscott@users.sf.net
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
 * along with this program; */

static void *
done_with_rename (gpointer data) {
    widgets_t *widgets_p = data;
    if (rfm_get_gtk_thread() != g_thread_self()){
	g_warning("done_with_rename() is a main thread function\n");
        return NULL;
    }
    view_t *view_p = widgets_p->view_p;
    gchar *path;
    NOOP ("rodent_done_with_rename\n");
    if(!view_p->widgets.rename) {
        NOOP ("!view_p->widgets.rename\n");
        return NULL;
    }
    path = g_object_get_data (G_OBJECT (view_p->widgets.rename), "path");
    g_free (path);


    NOOP ("gtk_widget_destroy(GTK_WIDGET(view_p->widgets.rename));\n");
    gtk_widget_destroy (GTK_WIDGET (view_p->widgets.rename));

    view_p->widgets.rename = NULL;
    gtk_main_quit();
    rodent_unselect_all_pixbuf (view_p);
    return NULL;
}


/////////////////////////////   ENTRY rename/copy/simlink  /////////////////
//

static int
entry_rename (widgets_t * widgets_p, const gchar * nfile, const gchar * ofile) {
    fprintf (stderr,"entry_rename: %s --> %s\n", ofile, nfile);//OK
    if(!widgets_p || !nfile || !ofile || !strlen (nfile) || !strlen (ofile))
        return FALSE;

    GList *in_list = NULL;
    gchar *src = g_strdup(ofile);
    in_list = g_list_append (in_list, src);

    plain_cp (widgets_p, TR_MOVE, in_list, nfile, FALSE);
    g_list_free (in_list);
    g_free(src);
    view_t *view_p = widgets_p->view_p;
    if (!xfdir_monitor_control_greenlight(widgets_p)){
	rodent_trigger_reload(view_p);
    }


    return TRUE;
}

static int
entry_duplicate (widgets_t * widgets_p, const gchar * nfile, const gchar * ofile) {
    if(!widgets_p || !nfile || !ofile || !strlen (nfile) || !strlen (ofile))
        return FALSE;

    GList *in_list = NULL;
    gchar *src = g_strdup(ofile);
    in_list = g_list_append (in_list, src);
    plain_cp (widgets_p, TR_COPY, in_list, nfile, FALSE);
    g_list_free (in_list);
    g_free(src);
    view_t *view_p = widgets_p->view_p;
    if (!xfdir_monitor_control_greenlight(widgets_p)){
	rodent_trigger_reload(view_p);
    }



    return TRUE;
}

static int
entry_symlink (widgets_t * widgets_p, const gchar * nfile, const gchar * ofile) {

    if(!widgets_p || !nfile || !ofile || !strlen (nfile) || !strlen (ofile))
        return FALSE;

#if 1
    g_free(widgets_p->workdir);
    // Relative links, always...
    widgets_p->workdir=g_path_get_dirname(ofile);
    gboolean need_sudo = !rfm_write_ok_path(widgets_p->workdir);
    
    gchar *b_ofile=g_path_get_basename(ofile);
    gchar *b_nfile=g_path_get_basename(nfile);
    gchar *sources[]={"ln", "-s", b_ofile, b_nfile, NULL};
    if (need_sudo){
	gchar *failed=g_strdup_printf(_("Failed to link %s to %s"), 
			    _("File"), _("Destination"));
        if (confirm_sudo(widgets_p, widgets_p->workdir, failed, "ln")){
            RFM_TRY_SUDO (widgets_p, sources, FALSE);
        }
        g_free (failed);
    } else {
        rfm_thread_run_argv (widgets_p, sources, FALSE);
    }
    g_free(b_ofile);
    g_free(b_nfile);

#else
    // oldway (no sudo)
    g_free(widgets_p->workdir);
    // Relative links, always...
    widgets_p->workdir=g_path_get_dirname(ofile);
    gchar *b_ofile=g_path_get_basename(ofile);
    gchar *b_nfile=g_path_get_basename(nfile);
    gchar *command = g_strdup_printf ("ln -s \"%s\" \"%s\"", b_ofile, b_nfile);
    RFM_THREAD_RUN2ARGV (widgets_p, command, FALSE);
    g_free (command);
    g_free(b_ofile);
    g_free(b_nfile);
#endif


    view_t *view_p = widgets_p->view_p;
    if (!xfdir_monitor_control_greenlight(widgets_p)){
	rodent_trigger_reload(view_p);
    }



    return TRUE;
}

static void
entry_activate (GtkEntry * entry, view_t * view_p, int caso) {
    gchar *actual_tag;
    gchar *tag;
    gchar *b;
    int result = FALSE;
    gchar *path;
    widgets_t *widgets_p=&(view_p->widgets);
    
    if (view_p->widgets.rename) gtk_widget_hide (GTK_WIDGET (view_p->widgets.rename));

    path = g_object_get_data (G_OBJECT (view_p->widgets.rename), "path");
    if(!path) return;

    actual_tag = gtk_editable_get_chars ((GtkEditable *) entry, 0, -1);
    g_strstrip(actual_tag);
    tag = g_locale_from_utf8 (actual_tag, -1, NULL, NULL, NULL);
    g_free (actual_tag);
    actual_tag = tag;
    b = g_path_get_basename (path);

    gchar *d = g_path_get_dirname (path);
    gchar *p = g_build_filename (d, actual_tag, NULL);
    g_free (d);
    switch (caso) {
    case RENAME_CASO:
        result = entry_rename (widgets_p, p, path);
        if(result) {
            NOOP ("renaming %s to %s\n", path, p);
        }
        break;
    case DUPLICATE_CASO:
        result = entry_duplicate (widgets_p, p, path);
        if(result) {
            NOOP ("duplication %s to %s\n", path, p);
        }
        break;
    case SYMLINK_CASO:
        result = entry_symlink (widgets_p, p, path);
        if(result) {
            NOOP ("symlinking %s to %s\n", path, p);
        }
        break;
    }
    g_free (p);

    g_free (b);
    g_free (actual_tag);
    done_with_rename (widgets_p);
}

static void
entry_activate_rename (GtkEntry * entry, gpointer data) {
    entry_activate (entry, (view_t *) data, 0);
}

static void
entry_activate_duplicate (GtkEntry * entry, gpointer data) {
    entry_activate (entry, (view_t *) data, 1);
}

static void
entry_activate_symlink (GtkEntry * entry, gpointer data) {
    entry_activate (entry, (view_t *) data, 2);
}

static void
destroy_dialog (GtkWidget * widget, gpointer data) {
    widgets_t *widgets_p = data;
    done_with_rename (widgets_p);
}

static gint
on_key_press (GtkWidget * entry, GdkEventKey * event, gpointer data) {
    if(event->keyval == GDK_KEY_Escape) {
	widgets_t *widgets_p = data;
        done_with_rename (widgets_p);
        //rfm_status (&(view_p->widgets), NULL, _("Omitting"), NULL);
        return TRUE;
    }
    return FALSE;
}

static gboolean
grab_focus (GtkWidget * widget, GdkEventCrossing * event, gpointer data) {
    rfm_global_t *rfm_global_p = rfm_global();
    XSetInputFocus (rfm_global_p->Xdisplay, 
	    GDK_WINDOW_XID (gtk_widget_get_parent_window (widget)), RevertToParent, CurrentTime);
    XUngrabPointer (rfm_global_p->Xdisplay, CurrentTime);
    return TRUE;
}

static void
entry_aid(view_t *view_p, gint caso, gint labelY){
    GtkWidget *vpane = g_object_get_data(G_OBJECT(view_p->widgets.paper), "vpane");
    if (vpane==NULL) return;

    GtkAllocation allocation;
    gtk_widget_get_allocation (vpane, &allocation);
    
    NOOP( "ENTRY population_p->labelY = %d, position=%lf\n",
	    labelY, allocation.height * 0.75);
    if (labelY < allocation.height * 0.70){
	rfm_show_text(&(view_p->widgets));
    } else return;

    switch (caso) {
	case 0:
	    rfm_diagnostics(&(view_p->widgets),"xffm/stock_dialog-info",NULL);

	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/green",
	    _("Rename the selected file"), "--> ", NULL);
	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/red",
	    _("Click"), "\n", NULL);

	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",
		    _("Duplicate this path"), "--> ", NULL);
	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",_
		    ("Control"),"+",_("Click"), "\n", NULL);

	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",
		    _("Create Symlink"), "--> ", NULL);
	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",
		    _("Shift"),"+",_("Control"),"+", _("Click"), "\n", NULL);
	    break;
	case 1:
	    rfm_diagnostics(&(view_p->widgets),"xffm/stock_dialog-info",NULL); 
	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/green",
		    _("Duplicate this path"), "--> ", NULL);
	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/red",_
		    ("Control"),"+",_("Click"), "\n", NULL);

	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",
	    _("Rename the selected file"), "--> ", NULL);
	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",
	    _("Click"), "\n", NULL);

	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",
		    _("Create Symlink"), "--> ", NULL);
	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",
		    _("Shift"),"+",_("Control"),"+", _("Click"), "\n", NULL);
	    break;
	case 2:
	    rfm_diagnostics(&(view_p->widgets),"xffm/stock_dialog-info",NULL); 
	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/green",
		    _("Create Symlink"), "--> ", NULL);
	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/red",
		    _("Shift"),"+",_("Control"),"+", _("Click"), "\n", NULL);

	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",
	    _("Rename the selected file"), "--> ", NULL);
	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",
	    _("Click"), "\n", NULL);

	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",
		    _("Duplicate this path"), "--> ", NULL);
	    rfm_diagnostics(&(view_p->widgets),"xffm_tag/grey",_
		    ("Control"),"+",_("Click"), "\n", NULL);

	    break;
    }
}

/* entry function: */
static void *
mk_rename_entry (gpointer data) {
    rfm_global_t *rfm_global_p = rfm_global();
    void **arg = data;
    view_t *view_p = arg[0];
    const population_t *population_p = arg[1];
    gint caso = GPOINTER_TO_INT(arg[2]);
    g_free(arg);
    GdkRectangle label_rect;
    if (!rfm_get_population_label_rect_full(view_p, population_p, &label_rect)) return NULL;

    GtkWidget *entry;
    GtkWidget *hbox;

    //widgets_t *widgets_p=&(view_p->widgets);
    
    GdkRectangle frame_extent;
    GdkRectangle window_extent;

    gint delta_y;
    gint delta_h;
    gint y_offset;
    gdk_window_get_position (gtk_widget_get_window(rfm_global_p->window), 
    	&window_extent.x, &window_extent.y);
    Drawable drawable = 
        GDK_WINDOW_XID(gtk_widget_get_window(rfm_global_p->window)); 
    rfm_get_drawable_geometry(drawable, NULL, NULL, &window_extent.width, &window_extent.height, NULL);
    NOOP("window_geometry:  x=%d,  y=%d, w=%d, h=%d", window_extent.x, window_extent.y, window_extent.width, window_extent.height); 

    gdk_window_get_frame_extents (gtk_widget_get_window(rfm_global_p->window),&frame_extent);
    NOOP("window_frame_extents: x=%d, y=%d, w=%d, h=%d\n", 
    	frame_extent.x, frame_extent.y, frame_extent.width, frame_extent.height); 


    delta_y = window_extent.y - frame_extent.y;
    delta_h = frame_extent.height - window_extent.height;
    y_offset = delta_y + delta_h;
    NOOP("window: delta_y=%d, delta_h=%d, y_offset=%d\n", 
    	delta_y, delta_h, y_offset); 


    NOOP ("rodent_mk_text_entry...\n");
    if(!population_p || !population_p->en || !population_p->en->path){
	NOOP(stderr, "rodent_mk_text_entry: invalid population\n");
        return NULL;
    }

    /* caso=0, rename; caso=1, duplicate; caso=2, symlink */

    entry = gtk_entry_new ();
    hbox = rfm_hbox_new (FALSE, 0);
    view_p->widgets.rename = gtk_window_new (GTK_WINDOW_POPUP);

    gint x_coordinate;
    gint y_coordinate;
    /* change relative coordinates to absolute coordinates and place entry window */
    {
        double sh=0;
	GtkScrolledWindow *scrolled_window = g_object_get_data(G_OBJECT(view_p->widgets.paper), "scrolled_window");
        if(scrolled_window && GTK_IS_SCROLLED_WINDOW (scrolled_window)) {
            sh = gtk_adjustment_get_value (gtk_scrolled_window_get_vadjustment (scrolled_window));
        }
        gtk_window_get_position ((GtkWindow *) rfm_global_p->window, &x_coordinate, &y_coordinate);
        x_coordinate += ((frame_extent.width - window_extent.width)/2);
        y_coordinate = label_rect.y + frame_extent.y - sh 
                    + y_offset + TEXTSPACING;
        gtk_window_move ((GtkWindow *) view_p->widgets.rename,
                         label_rect.x + x_coordinate, 
			 y_coordinate);
	
    }

    gtk_window_set_resizable (GTK_WINDOW (view_p->widgets.rename), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (view_p->widgets.rename), 0);
    gtk_window_set_modal (GTK_WINDOW (view_p->widgets.rename), FALSE);

    gchar *g = NULL;
    gchar *b = g_path_get_basename (population_p->en->path);
    gchar *path = g_strdup (population_p->en->path);
    g_object_set_data (G_OBJECT (view_p->widgets.rename), "path", path);
    g_object_set_data (G_OBJECT (view_p->widgets.rename), "caso", GINT_TO_POINTER(caso));

    if(caso == 0) {
        g = g_strdup (b);
        rfm_status (&(view_p->widgets), "xffm/stock_dialog-warning", b, ": ", _("Rename"), "...", NULL);
        g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (entry_activate_rename), view_p);
	NOOP(stderr, "Rename...\n");
    } else if(caso == 1) {
        gchar *dir = g_path_get_dirname (population_p->en->path);
        g = g_strdup_printf (_("Copy of %s"), b);
        g_free (dir);
        rfm_status (&(view_p->widgets), "xffm/stock_dialog-warning", b, ": ", _("Duplicate"), "...", NULL);
	NOOP(stderr, "Duplicate...\n");
        g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (entry_activate_duplicate), view_p);
    } else if(caso == 2) {
        g = g_strdup_printf (_("Link to %s"), b);
        rfm_status (&(view_p->widgets), "xffm/stock_dialog-warning", b, ": ", _("Create Symbolic Link"), "...", NULL);
	NOOP(stderr, "Symlink...\n");
        g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (entry_activate_symlink), view_p);
    }
    gchar *q = rfm_utf_string (g);
    gchar *p=q;
    double extra=0;

    while (p && *p) {  //hack: gtk bug workaround
    gunichar up=g_utf8_get_char_validated (p, -1);
    if (up > 0) {
    if (g_unichar_isupper(up)) extra += 0.3;
    //if (g_unichar_islower(up)) extra -= 0.1;
    if (g_unichar_iswide(up)) extra += 1.0;
    if (g_unichar_ispunct(up)) extra -= 0.2;
    if (g_unichar_iszerowidth(up)) extra -= 1.0;
    }
    p++;
    }
    extra += 0.5;
    int chars = extra + g_utf8_strlen (q, -1);
    if (chars < 7) chars=7;

    gtk_entry_set_width_chars ((GtkEntry *)entry, chars);
    gtk_entry_set_text ((GtkEntry *) entry, q);
    g_free (q);
    g_free (b);
    g_free (g);

    gtk_editable_set_editable ((GtkEditable *) entry, TRUE);

    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_container_add (GTK_CONTAINER (view_p->widgets.rename), hbox);

    gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, FALSE, 0);

    g_signal_connect (G_OBJECT (view_p->widgets.rename), "destroy-event", G_CALLBACK (destroy_dialog), &(view_p->widgets));
    g_signal_connect (G_OBJECT (view_p->widgets.rename), "key_press_event", G_CALLBACK (on_key_press), &(view_p->widgets));
    g_signal_connect (G_OBJECT (view_p->widgets.rename), "delete-event", G_CALLBACK (destroy_dialog), &(view_p->widgets));


    g_signal_connect (G_OBJECT (entry), "enter-notify-event", G_CALLBACK (grab_focus), view_p);
    entry_aid(view_p, caso, label_rect.y);
    NOOP ("now showing view_p->widgets.rename \n");
    gtk_widget_show_all (view_p->widgets.rename);
        gint endpos;
        g = gtk_editable_get_chars ((GtkEditable *) entry, 0, -1);
        if(strchr (g, '.')) {
            gtk_editable_select_region ((GtkEditable *) entry, 0, 0);
            for(endpos = strlen (g) - 1; g[endpos] >= 0; endpos--) {
                if(g[endpos] == '.')
                    break;
            }
            NOOP ("entry box: endpos=%d\n", endpos);
            gtk_editable_select_region ((GtkEditable *) entry, 0, endpos);
        }
        g_free (g);    

    /* this sucks: gtk_widget_grab_focus (view_p->widgets.rename); */
    XSetInputFocus (rfm_global_p->Xdisplay, 
	    GDK_WINDOW_XID (gtk_widget_get_parent_window (entry)), RevertToParent, CurrentTime);

    gtk_window_set_transient_for (GTK_WINDOW (view_p->widgets.rename), GTK_WINDOW (rfm_global_p->window));
    
    rfm_get_drawable_geometry(rfm_global_p->root_Xwindow, NULL, NULL, &window_extent.width, &window_extent.height, NULL);


    //test 1, is the box larger than the root window?
    GtkAllocation allocation;
    gtk_widget_get_allocation (view_p->widgets.rename, &allocation);
    
    if (allocation.width > window_extent.width) {
        gtk_widget_set_size_request(entry, window_extent.width-6, -1);
        gtk_widget_set_size_request(view_p->widgets.rename, window_extent.width, -1);
        x_coordinate=0;
        // this does not do what I think it should do (show start of truncated text):
        //gtk_entry_set_alignment((GtkEntry *)entry, 0);
    } else { // it fits, try to place it.
        int center = x_coordinate + label_rect.x;
        // overspill?
        if (center + allocation.width > window_extent.width) x_coordinate = window_extent.width - allocation.width;
        // center is OK
        else x_coordinate=center;
    }


    gtk_window_move ((GtkWindow *) view_p->widgets.rename,
                x_coordinate, //     
		y_coordinate);
    XGrabPointer (rfm_global_p->Xdisplay,
                  GDK_WINDOW_XID (gtk_widget_get_parent_window (entry)),
                  TRUE, 0, GrabModeSync, GrabModeAsync,
                  GDK_WINDOW_XID (gtk_widget_get_parent_window (entry)),
                  None, CurrentTime);
    XUngrabPointer(rfm_global_p->Xdisplay,CurrentTime);
    gtk_main();
    return NULL;
}



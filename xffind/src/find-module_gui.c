/*
 * Copyright (C) 2003-2012 Edscott Wilson Garcia
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "rodent.h"
#include "rfm_modules.h"

#include "find-module_gui.h"

// default values:
gint result_limit=256;
gint size_greater=16;
gint size_smaller=1024;
gint last_minutes=60;
gint last_hours=2;
gint last_days=7;
gint last_months=2;
gboolean default_recursive=TRUE;
gboolean default_recursiveH=FALSE;
gboolean default_xdev=TRUE;
gboolean default_case_sensitive=FALSE;
gboolean default_ext_regexp=FALSE;
gboolean default_look_in_binaries=FALSE;
gboolean default_line_count=FALSE;
gint default_type_index=0;
gboolean default_anywhere=TRUE;
gboolean default_match_words=FALSE;
gboolean default_match_lines=FALSE;
gboolean default_match_no_match=FALSE;
GSList *find_list = NULL;
gchar  *last_workdir = NULL;
    
gboolean have_grep = FALSE;
gboolean have_gnu_grep = FALSE;

static const gchar *
filter_text_help=
	        N_("Basic rules:\n" "\n"
                          "*  Will match any character zero or more times.\n"
                          "?  Will match any character exactly one time\n"
                          "[] Match any character within the [] \n"
                          "^  Match at beginning of string\n" 
			  "$  Match at end of string \n");
static const gchar *
grep_text_help=
		N_("Reserved characters for extended regexp are\n"
                          ". ^ $ [ ] ? * + { } | \\ ( ) : \n"
                          "In  basic regular expressions the metacharacters\n"
                          "?, +, {, |, (, and ) lose their special meaning.\n"
                          "\n"
                          "The  period  .   matches  any  single  character.\n"
                          "The caret ^ matches at the start of line.\n"
                          "The dollar $ matches at the end of line.\n" "\n"
                          "Characters within [ ] matches any single \n"
                          "       character in the list.\n"
                          "Characters within [^ ] matches any single\n"
                          "       character *not* in the list.\n"
                          "Characters inside [ - ] matches a range of\n"
                          "       characters (ie [0-9] or [a-z]).\n" "\n"
                          "A regular expression may be followed by one\n"
                          "       of several repetition operators:\n"
                          "?      The preceding item is optional and matched\n"
                          "       at most once.\n"
                          "*      The preceding item will be matched zero\n"
                          "       or more times.\n"
                          "+      The preceding item will be matched one or\n"
                          "       more times.\n"
                          "{n}    The preceding item is matched exactly n times.\n"
                          "{n,}   The preceding item is matched n or more times.\n"
                          "{n,m}  The preceding item is matched at least n times,\n"
                          "       but not more than m times.\n" "\n"
                          "To match any reserved character, precede it with \\. \n"
                          "\n"
                          "Two regular expressions may be joined by the logical or\n"
                          "       operator |.\n"
                          "Two regular expressions may be concatenated.\n" "\n"
                          "More information is available by typing \"man grep\"\n"
			  );

static void
on_help_filter (GtkToggleButton * button, gpointer data) {
    GtkWidget *dialog=data;
    widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
    if (gtk_toggle_button_get_active (button)) {
	rfm_clear_text(widgets_p);
	rfm_show_text (widgets_p); 
	rfm_diagnostics(widgets_p,"xffm_tag/blue", _(filter_text_help), NULL);
	rfm_scroll_to_top(widgets_p);
    } else {
	rfm_clear_text(widgets_p);
    }
}

static void
on_help_grep (GtkToggleButton * button, gpointer data) {
    GtkWidget *dialog=data;
    widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
    if (gtk_toggle_button_get_active (button)) {
	rfm_clear_text(widgets_p);
	rfm_show_text (widgets_p); 
	rfm_diagnostics(widgets_p,"xffm_tag/blue", _(grep_text_help), NULL);
	rfm_scroll_to_top(widgets_p);
    } else {
	rfm_clear_text(widgets_p);
    }
}

GtkWidget *
fill_string_option_menu (GtkComboBox *om, GSList * strings) {
    if (!om || ! strings){
	DBG("create_string_option_menu: GtkComboBox==NULL || GList==NULL\n");
	return NULL;
    }
    GSList *p;
    for (p=strings; p && p->data; p=p->next) {
#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    // this is deprecated...
	gtk_combo_box_append_text (om, (const gchar *)p->data);
#else
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(om), (const gchar *)p->data);
#endif
      /* Note: The model will keep a copy of the string internally, 
       * so the list may be freed */
    }
    gtk_combo_box_set_active(om, default_type_index);
    return (GtkWidget *)om;
}

static void
edit_command (GtkWidget * button, gpointer data) {
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");

    GSList *list = find_list;
    rfm_show_text (widgets_p); 
    if (!list || g_slist_length(list) < 1) {
	rfm_diagnostics(widgets_p, "xffm/stock_dialog-warning",NULL);
	rfm_diagnostics(widgets_p, "xffm_tag/stderr", _("Search returned no results"), "\n", NULL);
	return;
    }

    const gchar *editor = getenv("EDITOR");
    if (!editor || strlen(editor)==0){
	rfm_diagnostics(widgets_p, "xffm/stock_dialog-warning",NULL);
	rfm_diagnostics(widgets_p, "xffm_tag/stderr", _("No editor component found."), "\n", NULL);
	return;
    }

    gchar *command = g_strdup(editor);
  

    for (; list && list->data; list=list->next){
	gchar *g = g_strconcat(command, " \"", (gchar *)list->data, "\"", NULL);
	g_free(command);
	command = g;
    }

    rfm_diagnostics(widgets_p, "xffm/stock_execute",NULL);
    rfm_diagnostics(widgets_p, "xffm_tag/blue", " ", command, "\n", NULL);

    g_free(widgets_p->workdir);
    widgets_p->workdir = g_strdup(last_workdir);

    // Hack: for nano or vi, run in terminal
    gboolean in_terminal = FALSE;
    if (strstr(command, "nano") || 
	    (strstr(command, "vi") && !strstr(command, "gvim")))
    {
	in_terminal = TRUE;
    }

    RFM_THREAD_RUN2ARGV(widgets_p, command, in_terminal);
    
    g_free(widgets_p->workdir);
    widgets_p->workdir = g_strdup(g_get_home_dir());
}


static void
command_help (GtkWidget * button, gpointer data) {
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
    gchar *argv[]={data, "--help", NULL};
    rfm_clear_text (widgets_p); 
    rfm_show_text (widgets_p); 
    rfm_thread_run_argv_full (widgets_p, argv, FALSE, NULL, rfm_markup_stdout_f, rfm_dump_output, rfm_null_function);
}

static void
clear_text (GtkWidget * button, gpointer data) {
    GtkWidget *dialog=g_object_get_data(G_OBJECT(button), "dialog");
    widgets_t *widgets_p=g_object_get_data(G_OBJECT(dialog), "widgets_p");
    rfm_clear_text (widgets_p);
}

static GSList *
get_user_slist(void){
    GSList *g_user = NULL;
    struct passwd *pw;
    while((pw = getpwent ()) != NULL) {
        g_user = g_slist_append (g_user, g_strdup (pw->pw_name));
    }
    g_user = g_slist_sort (g_user, (GCompareFunc) strcmp);
    endpwent ();
    pw = getpwuid (geteuid ());
    gchar *buf = g_strdup_printf ("%s", pw ? pw->pw_name : _("unknown"));
    g_user = g_slist_prepend (g_user, buf);
    return g_user;
}

static GSList *
get_group_slist(void){
    GSList *g_group=NULL;
    struct group *gr;
    while((gr = getgrent ()) != NULL) {
       g_group = g_slist_append (g_group, g_strdup (gr->gr_name));
    }
    endgrent ();
    g_group = g_slist_sort (g_group, (GCompareFunc) strcmp);
    gr = getgrgid (geteuid ());
    gchar *buf = g_strdup_printf ("%s", gr ? gr->gr_name : _("unknown"));
    g_group = g_slist_prepend (g_group, buf);
    return g_group;
}

static void 
sensitivize (GtkToggleButton *togglebutton, gpointer data){
    GtkWidget *hbox = data;
    gtk_widget_set_sensitive(hbox, gtk_toggle_button_get_active(togglebutton));
}

typedef struct radio_t {
    GtkWidget *box;
    GtkToggleButton *toggle[5];
} radio_t;

static void 
sensitivize_radio (GtkToggleButton *togglebutton, gpointer data){
    if (!data) return;
    radio_t *radio_p = data;
    gtk_widget_set_sensitive(radio_p->box, FALSE);
    GtkToggleButton **tb_p = radio_p->toggle;
    for (; tb_p && *tb_p; tb_p++){
	if (gtk_toggle_button_get_active(*tb_p)){
	    gtk_widget_set_sensitive(radio_p->box, TRUE);
	}
    }
}

static radio_t *
create_radios(GtkWidget *dialog, GtkWidget *options_vbox){
    radio_t *radio_p = (radio_t *)malloc(sizeof(radio_t));
    if (!radio_p) g_error("malloc: %s", strerror(errno));
    g_object_set_data(G_OBJECT(dialog), "radio_p", radio_p );
    memset(radio_p, 0, sizeof(radio_t));

    GtkWidget *radio1 = gtk_radio_button_new_with_label (NULL, "mtime");
    rfm_add_custom_tooltip(radio1, NULL, _("Modified"));
    GtkWidget *radio2 = gtk_radio_button_new_with_label_from_widget (
	    GTK_RADIO_BUTTON (radio1), "ctime");
    rfm_add_custom_tooltip(radio2, NULL, _("Created"));
    GtkWidget *radio3 = gtk_radio_button_new_with_label_from_widget (
	    GTK_RADIO_BUTTON (radio1), "atime");
    rfm_add_custom_tooltip(radio3, NULL, _("Accessed"));

    g_object_set_data(G_OBJECT(dialog), "radio1", radio1 );
    g_object_set_data(G_OBJECT(dialog), "radio2", radio2 );
    g_object_set_data(G_OBJECT(dialog), "radio3", radio3 );
    gtk_widget_show (radio1);
    gtk_widget_show (radio2);
    gtk_widget_show (radio3);

    GtkWidget *radio_box=rfm_vbox_new (FALSE, 0);
    g_object_set_data(G_OBJECT(dialog), "radio_box", radio_box );
    gtk_widget_show (radio_box);
    gtk_box_pack_start (GTK_BOX (options_vbox), radio_box, TRUE, FALSE, 0);
    gtk_widget_set_sensitive(radio_box, FALSE);
    radio_p->box = radio_box;


    /*GtkWidget *label = gtk_label_new(_("modified"));
    gtk_box_pack_start (GTK_BOX (radio_box), label, TRUE, FALSE, 0);
    gtk_widget_show (label);*/
    GtkWidget *box=rfm_vbox_new (FALSE, 0);
    gtk_widget_show (box);
    gtk_box_pack_start (GTK_BOX (radio_box), box, TRUE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (box), radio1, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (box), radio2, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (box), radio3, TRUE, FALSE, 0);

    /*label = gtk_label_new(_("within the last"));
    gtk_box_pack_start (GTK_BOX (radio_box), label, TRUE, FALSE, 0);
    gtk_widget_show (label);*/
    return radio_p;
}


static  GtkToggleButton *
add_option_entry(GtkWidget *dialog,
	GtkWidget *options_vbox, 
	const gchar *check_name,
	const gchar *entry_name,
	const gchar *text,
	const gchar *default_value)
{
    if ((!entry_name && !check_name)|| !options_vbox || !dialog) {
	DBG("add_option_entry(): incorrect function call\n");
	return NULL;
    }
    GtkWidget *hbox = rfm_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (options_vbox), hbox, TRUE, FALSE, 0);

    GtkWidget *size_hbox = rfm_hbox_new (FALSE, 0);
    GtkWidget *check = NULL;
    if (check_name) {
	check = gtk_check_button_new();
	gtk_widget_show (check);
	g_object_set_data(G_OBJECT(dialog), check_name, check);
	gtk_box_pack_start (GTK_BOX (hbox), check, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (check), "toggled", G_CALLBACK (sensitivize),
		size_hbox);
	gtk_widget_set_sensitive(size_hbox, FALSE);
    }
    gtk_widget_show (size_hbox);
    gtk_box_pack_start (GTK_BOX (hbox), size_hbox, FALSE, FALSE, 0);

    if (text) {
	GtkWidget *label = gtk_label_new (text);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (size_hbox), label, TRUE, FALSE, 0);
    }
    
    if (entry_name) {
	GtkWidget *label = gtk_label_new (": ");
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (size_hbox), label, FALSE, FALSE, 0);
	
	GtkWidget *entry = gtk_entry_new();
	gtk_widget_show (entry);
	gtk_box_pack_start (GTK_BOX (size_hbox), entry, TRUE, TRUE, 0);
	g_object_set_data(G_OBJECT(dialog), entry_name, entry);
	gtk_entry_set_text (GTK_ENTRY(entry), default_value);                          
    }

    if (check) return GTK_TOGGLE_BUTTON(check);
    return NULL;
}


static  GtkToggleButton *
add_option_radio2(GtkWidget *dialog,
	GtkWidget *options_vbox, 
	const gchar *check_name,
	const gchar *radio1_name,
	const gchar *radio2_name,
	const gchar *text1,
	const gchar *text2)
{
    if ((!radio1_name  && !check_name)|| !options_vbox || !dialog) {
	DBG("add_option_radio2(): incorrect function call\n");
	return NULL;
    }
    GtkWidget *hbox = rfm_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (options_vbox), hbox, TRUE, FALSE, 0);

    GtkWidget *size_hbox = rfm_hbox_new (FALSE, 0);
    GtkWidget *check = NULL;
    if (check_name) {
	check = gtk_check_button_new();
	gtk_widget_show (check);
	g_object_set_data(G_OBJECT(dialog), check_name, check);
	gtk_box_pack_start (GTK_BOX (hbox), check, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (check), "toggled", G_CALLBACK (sensitivize),
		size_hbox);
	gtk_widget_set_sensitive(size_hbox, FALSE);
    }
    gtk_widget_show (size_hbox);
    gtk_box_pack_start (GTK_BOX (hbox), size_hbox, FALSE, FALSE, 0);

    if (text1 && radio1_name) {
	GtkWidget *radio1 = gtk_radio_button_new_with_label (NULL, text1);
	gtk_widget_show (radio1);
	gtk_box_pack_start (GTK_BOX (size_hbox), radio1, TRUE, TRUE, 0);
	g_object_set_data(G_OBJECT(dialog), radio1_name, radio1);
        if (text2 && radio2_name) {
	    GtkWidget *radio2 = gtk_radio_button_new_with_label_from_widget (
	    GTK_RADIO_BUTTON (radio1), text2);
	    gtk_widget_show (radio2);
	    gtk_box_pack_start (GTK_BOX (size_hbox), radio2, TRUE, TRUE, 0);
	    g_object_set_data(G_OBJECT(dialog), radio2_name, radio2);
	}
    }

    if (check) return GTK_TOGGLE_BUTTON(check);
    return NULL;
}

static  GtkToggleButton *
add_option_spin(GtkWidget *dialog,
	GtkWidget *options_vbox, 
	const gchar *check_name,
	const gchar *spin_name,
	const gchar *text,
	gint default_value)
{
    /*GtkIconInfo *icon_info =
        gtk_icon_theme_lookup_icon (gtk_icon_theme_get_default(),
                            "list-add-symbolic",
                            GTK_ICON_SIZE_MENU,
                            0);*/
    // gtk bug workaround. if no icotheme in use, plus/minus icons cannot be loaded.
    //if (icon_info) {
/*    if (!getenv("RFM_USE_GTK_ICON_THEME") || !strlen(getenv("RFM_USE_GTK_ICON_THEME"))) {
        gchar *def_val = g_strdup_printf("%d", default_value);
        GtkToggleButton *t = add_option_entry(dialog, options_vbox, check_name, spin_name, text, def_val);
        g_free(def_val);
        return t;
    }*/
    //if (icon_info) g_object_unref(icon_info);

    if ((!spin_name && !check_name)|| !options_vbox || !dialog) {
	DBG("add_option_spin(): incorrect function call\n");
	return NULL;
    }
    GtkWidget *hbox = rfm_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (options_vbox), hbox, TRUE, FALSE, 0);

    GtkWidget *size_hbox = rfm_hbox_new (FALSE, 0);
    GtkWidget *check = NULL;
    if (check_name) {
	check = gtk_check_button_new();
	gtk_widget_show (check);
	g_object_set_data(G_OBJECT(dialog), check_name, check);
	gtk_box_pack_start (GTK_BOX (hbox), check, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (check), "toggled", G_CALLBACK (sensitivize),
		size_hbox);
	gtk_widget_set_sensitive(size_hbox, FALSE);
    }
    gtk_widget_show (size_hbox);
    gtk_box_pack_start (GTK_BOX (hbox), size_hbox, FALSE, FALSE, 0);

    if (text) {
	GtkWidget *label = gtk_label_new (text);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (size_hbox), label, TRUE, FALSE, 0);
    }
    
    if (spin_name) {
	GtkWidget *label = gtk_label_new (": ");
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (size_hbox), label, FALSE, FALSE, 0);
	GtkAdjustment *spinbutton_adj = GTK_ADJUSTMENT(gtk_adjustment_new (default_value, 0, 4096*4096, 1, 64, 0));
	GtkWidget *spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 0, 0);
	gtk_widget_show (spinbutton);
	gtk_box_pack_start (GTK_BOX (size_hbox), spinbutton, TRUE, TRUE, 0);
	gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (spinbutton), GTK_UPDATE_IF_VALID);
	g_object_set_data(G_OBJECT(dialog), spin_name, spinbutton);
	gtk_widget_set_size_request (spinbutton, 75, -1);
    }

    if (check) return GTK_TOGGLE_BUTTON(check);
    return NULL;
}



static  GtkToggleButton *
add_option_combo(GtkWidget *dialog,
	GtkWidget *options_vbox, 
	const gchar *check_name,
	const gchar *combo_name,
	const gchar *text,
	GSList *list)
{
    if ((!combo_name && !check_name)|| !options_vbox || !dialog) {
	DBG("add_option_spin(): incorrect function call\n");
	return NULL;
    }
    GtkWidget *hbox = rfm_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (options_vbox), hbox, TRUE, FALSE, 0);

    GtkWidget *size_hbox = rfm_hbox_new (FALSE, 0);
    GtkWidget *check = NULL;
    if (check_name) {
	check = gtk_check_button_new();
	gtk_widget_show (check);
	g_object_set_data(G_OBJECT(dialog), check_name, check);
	gtk_box_pack_start (GTK_BOX (hbox), check, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (check), "toggled", G_CALLBACK (sensitivize),
		size_hbox);
	gtk_widget_set_sensitive(size_hbox, FALSE);
    }
    gtk_widget_show (size_hbox);
    gtk_box_pack_start (GTK_BOX (hbox), size_hbox, FALSE, FALSE, 0);

    if (text) {
	GtkWidget *label = gtk_label_new (text);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (size_hbox), label, TRUE, FALSE, 0);
 	label = gtk_label_new (": ");
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (size_hbox), label, FALSE, FALSE, 0);
   }

    if (combo_name) {
	GtkListStore *usermodel=gtk_list_store_new (1, G_TYPE_STRING);
	rfm_set_store_data_from_list (usermodel, &list);

#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
	GtkWidget *combo = 
	    gtk_combo_box_entry_new_with_model((GtkTreeModel *)usermodel, 0);
#else
	 GtkWidget *combo = gtk_combo_box_new_with_entry();
	gtk_combo_box_set_model (GTK_COMBO_BOX(combo),(GtkTreeModel *)usermodel);
	gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX(combo),0);
							    
#endif
	
	GtkWidget *entry  = gtk_bin_get_child(GTK_BIN(combo));
	gtk_entry_set_text (GTK_ENTRY (entry), (gchar *)list->data);
	gtk_widget_show(combo);
	gtk_box_pack_start (GTK_BOX (size_hbox), combo, TRUE, TRUE, 0);
	g_object_set_data(G_OBJECT(dialog), combo_name, combo);
	gtk_widget_set_size_request (combo, 120, -1);
    }
    if (check) return GTK_TOGGLE_BUTTON(check);
    return NULL;
}

static GSList *
free_string_slist(GSList *slist){
    GSList *tmp;
    for (tmp=slist; tmp && tmp->data; tmp=tmp->next){
        g_free(tmp->data);
    }
    g_slist_free(slist);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
#define MAX_WIDTH 600

GtkWidget *
create_find_dialog () {



    gchar *grep = g_find_program_in_path ("grep");
    if(grep) {
        FILE *pipe;
        const gchar *cmd = "grep --version";
        have_grep = TRUE;
        pipe = popen (cmd, "r");
        if(pipe) {
            gchar line[256];
            memset (line, 0, 256);
            if(fgets (line, 255, pipe) == NULL)
                DBG ("fgets: %s\n", strerror (errno));
            pclose (pipe);
            if(strstr (line, "GNU"))
                have_gnu_grep = TRUE;
        }
    }
    g_free (grep);

    GtkAccelGroup *accel_group = gtk_accel_group_new ();

    GtkWidget *dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    GdkGeometry geometry;
    rfm_get_drawable_geometry (gdk_x11_get_default_root_xwindow (), 
	    NULL, NULL, &(geometry.max_width), &(geometry.max_height), NULL);
    geometry.max_width -= 25;
    geometry.max_height -= 25;
    gtk_window_set_geometry_hints (GTK_WINDOW(dialog), dialog, 
	    &geometry, GDK_HINT_MAX_SIZE);

    widgets_t *widgets_p =(widgets_t *)malloc(sizeof(widgets_t));
    memset(widgets_p, 0, sizeof(widgets_t));
    g_object_set_data(G_OBJECT(dialog), "widgets_p", widgets_p);
	
    g_object_set_data(G_OBJECT(dialog), "window", dialog);


    gtk_window_set_title (GTK_WINDOW (dialog), _("Find"));
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);

    GdkPixbuf *pixbuf = rfm_get_pixbuf("xffm/stock_find", SIZE_ICON);
    gtk_window_set_icon ((GtkWindow *) dialog, pixbuf);
    g_object_unref(pixbuf);
	    
    GtkWidget *main_vbox = rfm_vbox_new (FALSE, 0);
    gtk_widget_show (main_vbox);
    gtk_container_add (GTK_CONTAINER (dialog), main_vbox);

    GtkWidget *vpane = rfm_vpaned_new ();
    g_object_set_data(G_OBJECT(dialog), "vpane", vpane);
    // hack:
    widgets_p->paper = dialog;

    gtk_widget_show (vpane);
    gtk_box_pack_start (GTK_BOX (main_vbox), vpane, TRUE, TRUE, 0);
    GtkWidget *top_pane_vbox = rfm_vbox_new (FALSE, 6);
    GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_paned_pack1 (GTK_PANED (vpane), 
	    GTK_WIDGET (sw), FALSE, TRUE);
#if GTK_MAJOR_VERSION==3 && GTK_MINOR_VERSION>=8
    gtk_container_add(GTK_CONTAINER(sw), top_pane_vbox);

#else
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), top_pane_vbox);
#endif
    gtk_widget_show (sw);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(sw), 
	    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    widgets_p->diagnostics = gtk_text_view_new ();
    GtkWidget *scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_widget_show ((widgets_p->diagnostics));
    gtk_paned_pack2 (GTK_PANED (vpane), scrolledwindow, TRUE, TRUE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), 
	    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_container_add (GTK_CONTAINER (scrolledwindow), (widgets_p->diagnostics));
    gtk_container_set_border_width (GTK_CONTAINER ((widgets_p->diagnostics)), 2);
    gtk_widget_set_can_focus((widgets_p->diagnostics), FALSE);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW ((widgets_p->diagnostics)), GTK_WRAP_WORD);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW ((widgets_p->diagnostics)), FALSE);


    
    gtk_container_set_border_width (GTK_CONTAINER (top_pane_vbox), 5);


    GtkWidget *hbox= rfm_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (top_pane_vbox), hbox, FALSE, FALSE, 0);

    gchar *t=g_strdup_printf("<b>%s <i>(fgr)</i></b>  ", _("Find"));
    GtkWidget *label42 = gtk_label_new (t);
    g_free(t);
    gtk_widget_show (label42);
    gtk_box_pack_start (GTK_BOX (hbox), label42, FALSE, FALSE, 0);
    gtk_label_set_use_markup (GTK_LABEL (label42), TRUE);

    GtkWidget *button;

    button = rfm_dialog_button ("xffm/stock_dialog-question", NULL);
    rfm_add_custom_tooltip(button, NULL, "fgr --help");
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (command_help), "fgr");
    g_object_set_data(G_OBJECT(button), "dialog", dialog);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    gtk_widget_show(button);

   

    GtkWidget *vbox7a = rfm_vbox_new (FALSE, 5);
    gtk_widget_show (vbox7a);
    gtk_box_pack_start (GTK_BOX (top_pane_vbox), vbox7a, TRUE, TRUE, 0);

    GtkWidget *path_box = rfm_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox7a), path_box, FALSE, FALSE, 0);

    t=g_strdup_printf("%s:", _("Path"));
    GtkWidget *path_label = gtk_label_new (t);
    g_free(t);
    gtk_widget_show (path_label);

#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    // this is deprecated...
    GtkWidget *path_combo = gtk_combo_box_entry_new ();
#else
    GtkWidget *path_combo = gtk_combo_box_new_with_entry ();
#endif

    g_object_set_data (G_OBJECT (path_combo), "GladeParentKey", path_combo);
    g_object_set_data(G_OBJECT(dialog), "path_combo", path_combo);


    button = rfm_dialog_button ("xffm/stock_directory", NULL);
    GtkWidget *vbox = rfm_vbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (path_box), path_label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (path_box), path_combo, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (path_box), vbox, FALSE, FALSE, 0);
    gtk_widget_show (vbox);
    gtk_widget_show (path_combo);
    gtk_widget_show (button);
    g_object_set_data(G_OBJECT(dialog), "fileselector", button);

    GtkWidget *filter_box = rfm_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox7a), filter_box, TRUE, FALSE, 5);

    gchar *text=g_strdup_printf("%s ", _("Filter:"));
    GtkWidget *filter_label = gtk_label_new (text);
    g_free(text);
    gtk_widget_show (filter_label);
    gtk_box_pack_start (GTK_BOX (filter_box), filter_label, FALSE, FALSE, 0);

#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    // this is deprecated...
    GtkWidget *filter_combo = gtk_combo_box_entry_new ();
#else
    GtkWidget *filter_combo = gtk_combo_box_new_with_entry ();
#endif
    g_object_set_data (G_OBJECT (filter_combo), "GladeParentKey", filter_combo);
    gtk_widget_show (filter_combo);
    gtk_box_pack_start (GTK_BOX (filter_box), filter_combo, FALSE, TRUE, 0);
    g_object_set_data(G_OBJECT(dialog), "filter_combo", filter_combo);
    

    GtkWidget *togglebutton2 = rfm_toggle_button("xffm/stock_dialog-question", "");
    gtk_box_pack_start (GTK_BOX (filter_box), togglebutton2, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(dialog), "togglebutton2", togglebutton2);
    rfm_add_custom_tooltip(togglebutton2, NULL,  _(filter_text_help));
    g_signal_connect (togglebutton2,
                      "toggled", G_CALLBACK (on_help_filter), 
		      (gpointer) dialog);

    GtkWidget *hbox17 = rfm_hbox_new (FALSE, 0);
    gtk_widget_show (hbox17);
    gtk_box_pack_start (GTK_BOX (vbox7a), hbox17, TRUE, FALSE, 0);

    GtkWidget *left_options_vbox = rfm_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox17), left_options_vbox, FALSE, FALSE, 0);
    GtkWidget *center_options_vbox = rfm_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox17), center_options_vbox, FALSE, FALSE, 0);
    GtkWidget *right_options_vbox = rfm_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox17), right_options_vbox, FALSE, FALSE, 0);

    /// option -r "recursive"
    gtk_toggle_button_set_active(add_option_spin(dialog, left_options_vbox, "recursive", NULL, _("Recursive"), 0), default_recursive);

    /// option -D "recursiveH"
    gtk_toggle_button_set_active(add_option_spin(dialog, left_options_vbox, "recursiveH", NULL, _("Find hidden files and directories"), 0), default_recursiveH);

    /// option -a "xdev"
    gtk_toggle_button_set_active(add_option_spin(dialog, left_options_vbox, "xdev", NULL, _("Stay on single filesystem"), 0), default_xdev);

    /// option "upper_limit_spin" (only in gtk dialog)
    text = g_strdup_printf("%s (%s)", _("Results"), _("Upper limit"));
    add_option_spin(dialog, left_options_vbox, NULL, "upper_limit_spin", text, result_limit);
    g_free(text);

    // option -s +KByte "size_greater", "size_greater_spin"
    text = g_strdup_printf("%s (%s)", _("At Least"), _("kBytes"));
    add_option_spin(dialog, center_options_vbox, "size_greater", "size_greater_spin", text, size_greater);
    g_free(text);
    
    // option -s -KByte "size_smaller", "size_smaller_spin"
    text = g_strdup_printf("%s (%s)", _("At Most"), _("kBytes"));
    add_option_spin(dialog, center_options_vbox, "size_smaller", "size_smaller_spin", text, size_smaller);
    g_free(text);

    GSList *slist;
    // option -u uid "uid" "uid_combo"
    slist = get_user_slist();
    add_option_combo(dialog, center_options_vbox, "uid", "uid_combo", _("User"), slist);
    slist = free_string_slist(slist);

    // option -g gid "gid" "gid_combo"
    slist = get_group_slist();
    add_option_combo(dialog, center_options_vbox, "gid", "gid_combo", _("Group"), slist);
    slist = free_string_slist(slist);
    
    // option -o octal "octal_p" "permissions_entry"
    add_option_entry(dialog, center_options_vbox, "octal_p", "permissions_entry", _("Octal Permissions"), "0666");
    GtkWidget *entry = g_object_get_data(G_OBJECT(dialog), "permissions_entry");
    gtk_widget_set_size_request (entry, 75, -1);
    
    // option -p suid | exe 
    add_option_radio2(dialog, center_options_vbox, "suidexe", "suid_radio", "exe_radio", _("SUID"), _("Executable"));

    
    // option -M -A -C
    radio_t *radio_p = create_radios(dialog, right_options_vbox);
    // radio_p freed on destroy event for dialog.
   
    // option -k minutes "last_minutes", "last_minutes_spin"
    radio_p->toggle[0] = add_option_spin(dialog, right_options_vbox, "last_minutes", "last_minutes_spin", _("Minutes"), last_minutes);
    g_signal_connect (G_OBJECT (radio_p->toggle[0]), "toggled", G_CALLBACK (sensitivize_radio), radio_p);
    
   // option -h hours "last_hours", "last_hours_spin"
    radio_p->toggle[1] = add_option_spin(dialog, right_options_vbox, "last_hours", "last_hours_spin", _("Hours"), last_hours);
    g_signal_connect (G_OBJECT (radio_p->toggle[1]), "toggled", G_CALLBACK (sensitivize_radio), radio_p);
    
    // option -d days "last_days", "last_days_spin"
    radio_p->toggle[2] = add_option_spin(dialog, right_options_vbox, "last_days", "last_days_spin", _("Days"), last_days);
    g_signal_connect (G_OBJECT (radio_p->toggle[2]), "toggled", G_CALLBACK (sensitivize_radio), radio_p);
    
    // option -m months "last_months", "last_months_spin"
    radio_p->toggle[3] = add_option_spin(dialog, right_options_vbox, "last_months", "last_months_spin", _("Months"), last_months);
    g_signal_connect (G_OBJECT (radio_p->toggle[3]), "toggled", G_CALLBACK (sensitivize_radio), radio_p);
    
     ///////////

 
    GtkWidget *hbox21 = rfm_hbox_new (FALSE, 0);
    gtk_widget_show (hbox21);
    gtk_box_pack_start (GTK_BOX (left_options_vbox), hbox21, TRUE, FALSE, 0);

    GtkWidget *label37 = gtk_label_new (_("File type : "));
    gtk_widget_show (label37);
    gtk_box_pack_start (GTK_BOX (hbox21), label37, FALSE, FALSE, 0);

#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    // this is deprecated...
    GtkWidget *file_type_om =  gtk_combo_box_new_text();
#else
    GtkWidget *file_type_om =  gtk_combo_box_text_new();
#endif
    gtk_widget_show (file_type_om);
    gtk_box_pack_start (GTK_BOX (hbox21), file_type_om, TRUE, TRUE, 0);
    g_object_set_data(G_OBJECT(dialog), "file_type_om", file_type_om);

    ////////////////  grep options.... /////////////////////////
    
    t=g_strdup_printf("<b>%s</b>", _("Contains"));
    GtkWidget *contains_label = gtk_label_new (t);
    g_free(t);
    gtk_widget_show (contains_label);
    gtk_box_pack_start (GTK_BOX (top_pane_vbox), contains_label, FALSE, FALSE, 1);
    gtk_label_set_use_markup (GTK_LABEL (contains_label), TRUE);

    GtkWidget *hbox26 = rfm_hbox_new (FALSE, 0);
    gtk_widget_show (hbox26);
    gtk_box_pack_start (GTK_BOX (top_pane_vbox), hbox26, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox26), 2);

    GtkWidget *vbox8 = rfm_vbox_new (FALSE, 5);
    gtk_widget_show (vbox8);
    gtk_box_pack_start (GTK_BOX (hbox26), vbox8, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox8), 5);

    GtkWidget *grep_box = rfm_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox8), grep_box, FALSE, FALSE, 0);

    t=g_strdup_printf("%s: ",_("Contains the text"));
    GtkWidget *grep_label = gtk_label_new (t);
    g_free(t);
    gtk_widget_show (grep_label);
    gtk_box_pack_start (GTK_BOX (grep_box), grep_label, FALSE, FALSE, 0);

#if GTK_MAJOR_VERSION==2 && GTK_MINOR_VERSION<24
    GtkWidget *grep_combo = gtk_combo_box_entry_new ();
#else
    GtkWidget *grep_combo = gtk_combo_box_new_with_entry ();
#endif
    g_object_set_data (G_OBJECT (grep_combo), "GladeParentKey", grep_combo);
    gtk_widget_show (grep_combo);
    gtk_box_pack_start (GTK_BOX (grep_box), grep_combo, FALSE, FALSE, 5);
    gtk_widget_set_sensitive (grep_combo, have_grep);
    g_object_set_data(G_OBJECT(dialog), "grep_combo", grep_combo);

    GtkWidget *togglebutton3 = rfm_toggle_button ("xffm/stock_dialog-question", "");
    gtk_box_pack_start (GTK_BOX (grep_box), togglebutton3, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(dialog), "togglebutton3", togglebutton3);
    rfm_add_custom_tooltip(togglebutton3, NULL, _(grep_text_help));
    g_signal_connect (togglebutton3,
                      "toggled", G_CALLBACK (on_help_grep), 
		      (gpointer) dialog);

    GtkWidget *hbox20 = rfm_hbox_new (FALSE, 0);
    gtk_widget_show (hbox20);
    gtk_box_pack_start (GTK_BOX (vbox8), hbox20, FALSE, FALSE, 0);

    GtkWidget *vbox13 = rfm_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox20), vbox13, TRUE, TRUE, 0);

    GtkWidget *case_sensitive = gtk_check_button_new_with_mnemonic (_("Case Sensitive"));
    gtk_widget_show (case_sensitive);
    gtk_box_pack_start (GTK_BOX (vbox13), case_sensitive, FALSE, FALSE, 0);
    gtk_widget_set_sensitive (case_sensitive, FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (case_sensitive), default_case_sensitive);
    g_object_set_data(G_OBJECT(dialog), "case_sensitive", case_sensitive);

    GtkWidget *ext_regexp = gtk_check_button_new_with_mnemonic (_("Extended regexp"));
    gtk_widget_show (ext_regexp);
    gtk_box_pack_start (GTK_BOX (vbox13), ext_regexp, FALSE, FALSE, 0);
    gtk_widget_set_sensitive (ext_regexp, FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ext_regexp), default_ext_regexp);
    g_object_set_data(G_OBJECT(dialog), "ext_regexp", ext_regexp);

    GtkWidget *look_in_binaries = gtk_check_button_new_with_mnemonic (_("Include binary files"));
    gtk_widget_show (look_in_binaries);
    gtk_box_pack_start (GTK_BOX (vbox13), look_in_binaries, FALSE, FALSE, 0);
    gtk_widget_set_sensitive (look_in_binaries, FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (look_in_binaries), default_look_in_binaries);
    g_object_set_data(G_OBJECT(dialog), "look_in_binaries", look_in_binaries);

    GtkWidget *line_count = gtk_check_button_new_with_mnemonic (_("Line Count"));
    // XXX: (FIXME) this option (-c) does not work in fgr...
    //    gtk_widget_show (line_count);
    gtk_box_pack_start (GTK_BOX (vbox13), line_count, FALSE, FALSE, 0);
    gtk_widget_set_sensitive (line_count, FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (line_count), default_line_count);
    g_object_set_data(G_OBJECT(dialog), "line_count", line_count);
    
   
    GtkWidget *hbox28 = rfm_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (top_pane_vbox), hbox28, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox28), 2);

    GtkWidget *vbox11 = rfm_vbox_new (FALSE, 0);
    gtk_widget_show (vbox11);
    gtk_box_pack_start (GTK_BOX (hbox28), vbox11, TRUE, TRUE, 0);



    GtkWidget *hbox24 = rfm_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox11), hbox24, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox24), 5);
    
    GtkWidget *label40 = gtk_label_new ("");
    t=g_strdup_printf("<b>%s</b>: ", _("Match"));
    gtk_label_set_markup (GTK_LABEL(label40), t);
    g_free(t);
    gtk_widget_show (label40);
    gtk_box_pack_start (GTK_BOX (hbox24), label40, FALSE, FALSE, 0);
    gtk_widget_set_sensitive (label40, FALSE);
    g_object_set_data(G_OBJECT(dialog), "label40", label40);

    GSList *anywhere_group = NULL;
    GtkWidget *anywhere = gtk_radio_button_new_with_mnemonic (NULL, _("Anywhere"));
    gtk_widget_show (anywhere);
    gtk_box_pack_start (GTK_BOX (hbox24), anywhere, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (anywhere), anywhere_group);
    g_object_set_data(G_OBJECT(dialog), "anywhere", anywhere);
    anywhere_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (anywhere));
    gtk_widget_set_sensitive (anywhere, FALSE);
    if (default_anywhere) {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (anywhere), default_anywhere);
    }
     

    GtkWidget *match_words = gtk_radio_button_new_with_mnemonic (NULL, _("Whole words only"));
    gtk_widget_show (match_words);
    gtk_box_pack_start (GTK_BOX (hbox24), match_words, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (match_words), anywhere_group);
    g_object_set_data(G_OBJECT(dialog), "match_words", match_words);
    anywhere_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (match_words));
    gtk_widget_set_sensitive (match_words, FALSE);
    if (default_match_words) {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (match_words), default_match_words);
    }


    GtkWidget *match_lines = gtk_radio_button_new_with_mnemonic (NULL, _("lines"));
    gtk_widget_show (match_lines);
    gtk_box_pack_start (GTK_BOX (hbox24), match_lines, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (match_lines), anywhere_group);
    g_object_set_data(G_OBJECT(dialog), "match_lines", match_lines);
    anywhere_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (match_lines));
    gtk_widget_set_sensitive (match_lines, FALSE);
    if (default_match_lines) {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (match_lines), default_match_lines);
    }

    GtkWidget *match_no_match = gtk_radio_button_new_with_mnemonic (NULL, _("No match"));
    gtk_widget_show (match_no_match);
    gtk_box_pack_start (GTK_BOX (hbox24), match_no_match, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (match_no_match), anywhere_group);
    g_object_set_data(G_OBJECT(dialog), "match_no_match", match_no_match);
    anywhere_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (match_no_match));
    gtk_widget_set_sensitive (match_no_match, FALSE);
    if (default_match_no_match) {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (match_no_match), default_match_no_match);
    }

    gtk_widget_show(hbox);
    gtk_widget_show (path_box);
    gtk_widget_show (filter_box);
    gtk_widget_show (right_options_vbox);
    gtk_widget_show (center_options_vbox);
    gtk_widget_show (left_options_vbox);
    gtk_widget_show (grep_box);
    gtk_widget_show (vbox13);
    gtk_widget_show (hbox28);
    gtk_widget_show (hbox24);
    gtk_widget_show(top_pane_vbox);


    GtkWidget *hbuttonbox2 = rfm_hbutton_box_new ();
    gtk_widget_show (hbuttonbox2);
    gtk_box_pack_start (GTK_BOX (main_vbox), hbuttonbox2, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox2), 3);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox2), GTK_BUTTONBOX_EDGE);

    GtkWidget *button_image;
    gchar *g=g_strdup_printf("%s/%s", _("Find"), _("Close"));
    GtkWidget *find_button =  
        rfm_dialog_button("xffm/stock_find", g);
    g_free(g);
    g_object_set_data(G_OBJECT(dialog), "find_button", find_button);
    g_object_set_data(G_OBJECT(find_button), "dialog", dialog);
    
    GtkWidget *apply_button =  
        rfm_dialog_button("xffm/stock_find", _("Find"));
    g_object_set_data(G_OBJECT(dialog), "apply_button", apply_button);
    g_object_set_data(G_OBJECT(apply_button), "dialog", dialog);

    GtkWidget *cancel_button =  
        rfm_dialog_button("xffm/stock_cancel", _("Cancel"));
    g_object_set_data(G_OBJECT(dialog), "cancel_button", cancel_button);
    g_object_set_data(G_OBJECT(cancel_button), "dialog", dialog);
    gtk_widget_set_sensitive(cancel_button, FALSE);
    

    GtkWidget *clear_button =  
        rfm_dialog_button("xffm/stock_clear", _("Clear"));
    g_object_set_data(G_OBJECT(dialog), "clear_button", clear_button);
    g_object_set_data(G_OBJECT(clear_button), "dialog", dialog);
    g_signal_connect (G_OBJECT (clear_button), "clicked",
	    G_CALLBACK (clear_text), NULL);

    GtkWidget *close_button =  
        rfm_dialog_button("xffm/stock_close", _("Close"));
    g_object_set_data(G_OBJECT(dialog), "close_button", close_button);
    g_object_set_data(G_OBJECT(close_button), "dialog", dialog);


    gtk_widget_show (find_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox2), find_button);
    gtk_widget_set_can_default(find_button, TRUE);

    gtk_widget_show (apply_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox2), apply_button);
    gtk_widget_set_can_default(apply_button, TRUE);

    gtk_widget_show (clear_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox2), clear_button);

    gtk_widget_show (cancel_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox2), cancel_button);

    const gchar *editor = getenv("EDITOR");
    if (editor && strlen(editor)){
	gchar *basename = g_strdup(editor);
	if (strchr(basename, ' ')) *strchr(basename, ' ') = 0;
	gchar *editor_path = g_find_program_in_path(basename);
	if (editor_path){
	    gchar *iconpath=g_strdup(basename);
	    GdkPixbuf *pix = rfm_get_pixbuf (iconpath, SMALL_ICON_SIZE); //refs
	    if (!pix) iconpath = g_strdup("xffm/stock_edit");
            else g_object_unref(pix);

	    GtkWidget *edit_button = 
                rfm_dialog_button(iconpath, _("Edit"));
 	    g_free(iconpath);
	    g_object_set_data(G_OBJECT(dialog), "edit_button", edit_button);
	    g_object_set_data(G_OBJECT(edit_button), "dialog", dialog);
	    g_signal_connect (G_OBJECT (edit_button), "clicked", G_CALLBACK (edit_command), NULL);
	    gtk_widget_show(edit_button);
	    gtk_container_add (GTK_CONTAINER (hbuttonbox2), edit_button);
	    g_free(editor_path);
	} 
	g_free(basename);
    } else {
	NOOP("getenv(\"EDITOR\") = %s\n", editor);
    }


    gtk_widget_show (close_button);
    gtk_container_add (GTK_CONTAINER (hbuttonbox2), close_button);
    gtk_widget_add_accelerator (close_button, "clicked", accel_group, GDK_KEY_Escape, 0, GTK_ACCEL_VISIBLE);

    gtk_widget_grab_default (find_button);
    gtk_window_add_accel_group (GTK_WINDOW (dialog), accel_group);

    gtk_widget_realize(dialog);
 
    GtkAllocation label_allocation;
    GtkAllocation allocation;
    gtk_widget_get_allocation(top_pane_vbox, &allocation);
    gtk_widget_get_allocation(path_label, &label_allocation);
    // width will be the smaller of the two.
    gint width = (geometry.max_width < allocation.width)?
	geometry.max_width: allocation.width;
    NOOP(stderr, "max w= %d, top_pane w= %d, label w=%d\n", 
	    geometry.max_width,
	     allocation.width, 
	    label_allocation.width);
    gint combo_width;
    combo_width = width - label_allocation.width - 120;
    if (combo_width > 100){
	gtk_widget_set_size_request (path_combo, combo_width, -1);
    }
    gtk_widget_get_allocation(filter_label, &label_allocation);
    combo_width = width - label_allocation.width - 120;
    if (combo_width > 100){
	gtk_widget_set_size_request (filter_combo, combo_width, -1);
    }
    gtk_widget_get_allocation(grep_label, &label_allocation);
    combo_width = width - label_allocation.width - 120;
    if (combo_width > 100){
	gtk_widget_set_size_request (grep_combo, combo_width, -1);
    }

    // This will be constrained to the maximum width set by geometry hints:

#if GTK_MAJOR_VERSION==2
    gtk_window_set_default_size (GTK_WINDOW (dialog), allocation.width+50,  allocation.height+75);
#else
    gtk_window_set_default_geometry(GTK_WINDOW (dialog),  allocation.width+50,  allocation.height+75);
#endif
    
    
    gtk_paned_set_position (GTK_PANED (vpane), 1000);

    
    return dialog;
}

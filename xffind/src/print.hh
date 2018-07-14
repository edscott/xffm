#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <pwd.h>
#include <gtk/gtk.h>

//#include "util_c.hh"
//#include "view_c.hpp"
#define MAX_LINES_IN_BUFFER 10000    
#include "pthreadCalls.h"
////////////////////////////////////////////////////////////////////////////
//                       print class methods                            //
///////////////////////////////////////////////////////////////////////////
namespace xf
{
template <class Type>
class print<Type>: public util<Type> {
public:
    void context_function(void * (*function)(gpointer), void * function_data){
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t signal = PTHREAD_COND_INITIALIZER; 
	void *result=GINT_TO_POINTER(-1);
	void *arg[] = {
	    (void *)function,
	    (void *)function_data,
	    (void *)&mutex,
	    (void *)&signal,
	    (void *)&result
	};
	gboolean owner = g_main_context_is_owner(g_main_context_default());
	if (owner){
	    context_function_f(arg);
	} else {
	    g_main_context_invoke(NULL, context_function_f, arg);
	    pthread_mutex_lock(&mutex);
	    if (result == GINT_TO_POINTER(-1)) pthread_cond_wait(&signal, &mutex);
	    pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&signal);
	return result;
    }

    static void print_error(gchar *string){
	void *arg[]={(void *)this, (void *)"tag/bold", (void *)string};
	context_function(print_e, arg);
	g_free(string);

    }

    void static print_debug(gchar *string){
	void *arg[]={(void *)this, (void *)"tag/italic", (void *)string};
	context_function(print_d, arg);
	g_free(string);

    }


    void static print(gchar *string){
	void *arg[]={(void *)this, NULL, (void *)string};
	context_function(print_f, arg);
	g_free(string);
    }

    void static print_tag(const gchar *tag, gchar *string){
	void *arg[]={(void *)this, (void *)tag, (void *)string};
	context_function(print_f, arg);
	g_free(string);
    }

    void static print_icon(const gchar *iconname, gchar *string)
    {
	GdkPixbuf *pixbuf = pixbuf_c::get_pixbuf(iconname, -16);
	void *arg[]={(void *)pixbuf, (void *)this, NULL, (void *)string};
	context_function(print_i, arg);
	g_free(string);
    }


    void static print_icon_tag(const gchar *iconname, 
				 const gchar *tag, 
				 gchar *string)
    {
	GdkPixbuf *pixbuf = pixbuf_c::get_pixbuf(iconname, -16);
	void *arg[]={(void *)pixbuf, (void *)this, (void *)tag, (void *)string};
	context_function(print_i, arg);
	g_free(string);
    }

    void static print_status(gchar *string){
	void *arg[]={(void *)this, (void *)string};
	context_function(print_s, arg);
	g_free(string);
    }


    void static print_status_label(gchar *string){
	void *arg[]={(void *)status_label, (void *)string};
	context_function(print_sl, arg);
	g_free(string);
    }


    void static show_text(void){
	view_c *view_p = (view_c *)view_v;
	view_p->show_diagnostics();
    }

    void static clear_text(void){
	view_c *view_p = (view_c *)view_v;
	view_p->clear_diagnostics();
    }

    void static clear_status(void){
	view_c *view_p = (view_c *)view_v;
	view_p->clear_status();
    }


    void *
    static scroll_to_top(void){
	// make sure all text is written before attempting scroll
	while (gtk_events_pending()) gtk_main_iteration();
	GtkTextMark *mark;
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer (diagnostics);
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	mark = gtk_text_buffer_create_mark (buffer, "scrollmark", &start, FALSE);
	gtk_text_view_scroll_to_mark (diagnostics, mark, 0.2,    /*gdouble within_margin, */
				      TRUE, 0.0, 0.0);
	gtk_text_buffer_delete_mark (buffer, mark);
	return NULL;
    }

    void *
    static scroll_to_bottom(void){
	// make sure all text is written before attempting scroll
	while (gtk_events_pending()) gtk_main_iteration();
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer (diagnostics);
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	GtkTextMark *mark = gtk_text_buffer_create_mark (buffer, "scrolldown", &end, FALSE);
	gtk_text_view_scroll_to_mark (diagnostics, mark, 0.2,    /*gdouble within_margin, */
				      TRUE, 1.0, 1.0);
	//gtk_text_view_scroll_mark_onscreen (diagnostics, mark);
	gtk_text_buffer_delete_mark(buffer, mark);
	return NULL;
    }

    gchar *
    static get_current_text (void) {
	// get current text
	GtkTextIter start, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer ((GtkTextView *) status);

	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gchar *t = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
	g_strchug(t);
	return t;
    }

    gchar *
    static get_text_to_cursor (void) {
	// get current text
	GtkTextIter start, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(status));
	gint cursor_position;
	// cursor_position is a GtkTextBuffer internal property (read only)
	g_object_get (G_OBJECT (buffer), "cursor-position", &cursor_position, NULL);
	
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 0);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, cursor_position);
	gchar *t = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
	g_strchug(t);
	TRACE ("lpterm_c::get_text_to_cursor: to cursor position=%d %s\n", cursor_position, t);
	return t;
    }


    const gchar *
    static get_workdir(void){
	view_c *view_p = (view_c *)view_v;
	return view_p->get_path();
    }


//////////////////////////////////////////////////////////////////////
//                             print.i                              //
//////////////////////////////////////////////////////////////////////



typedef struct sequence_t {
    const gchar *id;
    const gchar *sequence;
} sequence_t;


static sequence_t sequence_v[] = {
        {"tag/black", "30"},
        {"tag/black_bg", "40"},
        {"tag/red", "31"},
        {"tag/red_bg", "41"},
        {"tag/green", "32"},
        {"tag/green_bg", "42"},
        {"tag/yellow", "33"},
        {"tag/yellow_bg", "43"},
        {"tag/blue", "34"},
        {"tag/blue_bg", "44"},
        {"tag/magenta", "35"},
        {"tag/magenta_bg", "45"},
        {"tag/cyan", "36"},
        {"tag/cyan_bg", "46"},
        {"tag/white", "37"},
        {"tag/white_bg", "47"},

        {"tag/bold", "1"},
        {"tag/bold", "01"},
        {"tag/italic", "4"},
        {"tag/italic", "04"},
        {"tag/blink", "5"},
        {"tag/blink", "05"},
        {NULL, ""},
        {NULL, "0"},
        {NULL, "00"},
        {NULL, "22"},
        {NULL, "24"},
        {NULL, NULL}            // this marks the end of sequences.
};


static const gchar *
get_ansi_tag(const gchar *code){
    sequence_t *p;
    for(p = sequence_v; p && p->sequence; p++) {
	if(strcmp (code, p->sequence) == 0) {
	    return p->id;
	}
    }
    return NULL;

}


typedef struct lpterm_colors_t {
    const gchar *id;
    GdkColor color;
} lpterm_colors_t;


static
GtkTextTag *
resolve_tag (GtkTextBuffer * buffer, const gchar * id) {
    if (!id) return NULL;

    GtkTextTag *tag = NULL;
    lpterm_colors_t lpterm_colors_v[] = {
        {"tag/command", {101, 0x5858, 0x3434, 0xcfcf}},
        {"tag/stderr", {102, 0xcccc, 0, 0}},
        {"tag/command_id", {103, 0x0000, 0x0000, 0xffff}},
        {"tag/green", {104, 0x0000, 0xaaaa, 0x0000}},
        {"tag/red", {105, 0xcdcd, 0x0000, 0x0000}},
        {"tag/blue", {106, 0x0000, 0x0000, 0xcdcd}},
        {"tag/yellow", {107, 0xcdcd, 0xcdcd, 0x0000}},
        {"tag/magenta", {108, 0xcdcd, 0x0000, 0xcdcd}},
        {"tag/cyan", {109, 0x0000, 0xcdcd, 0xcdcd}},
        {"tag/black", {1, 0x0000, 0x0000, 0x0000}},
        {"tag/white", {0, 0xffff, 0xffff, 0xffff}},
        {"tag/grey", {110, 0x8888, 0x8888, 0x8888}},
        {NULL, {0, 0, 0, 0}}

    };
    void *initialized = g_object_get_data(G_OBJECT(buffer), "text_tag_initialized");
    if (!initialized) {
	lpterm_colors_t *p = lpterm_colors_v;
        for(; p && p->id; p++) {
	    gtk_text_buffer_create_tag (buffer, p->id, 
		    "foreground_gdk", &(p->color), 
		    NULL);
	    gchar *bg_id = g_strconcat(p->id, "_bg", NULL);
 	    gtk_text_buffer_create_tag (buffer, bg_id, 
		    "background_gdk", &(p->color), 
		    NULL);
	    g_free(bg_id);
       }
        gtk_text_buffer_create_tag (buffer, 
                "tag/bold", "weight", PANGO_WEIGHT_BOLD, "foreground_gdk", NULL, NULL);
        gtk_text_buffer_create_tag (buffer, 
                "tag/italic", "style", PANGO_STYLE_ITALIC, "foreground_gdk", NULL, NULL);
	g_object_set_data(G_OBJECT(buffer), "text_tag_initialized", GINT_TO_POINTER(1));
    } 

    tag = gtk_text_tag_table_lookup (gtk_text_buffer_get_tag_table (buffer), id);
    if (!tag) fprintf(stderr, "No GtkTextTag for %s\n", id);
    return tag;
}



GtkTextTag **
static resolve_tags(GtkTextBuffer * buffer, const gchar *tag){
    GtkTextTag **tags = NULL;
    if(tag) {
        if(strncmp (tag, "tag/", strlen ("tag/")) == 0) {
	    tags = (GtkTextTag **)malloc(2*sizeof(GtkTextTag *));
	    if (!tags) g_error("malloc\n");
	    tags[0] = resolve_tag (buffer, tag);
	    tags[1] = NULL;
        } else if(strncmp (tag, "icon/", strlen ("icon/")) == 0){
            //icon = rfm_get_pixbuf (id, SIZE_BUTTON);
        }
    }
    return tags;
}

void
static insert_string (GtkTextBuffer * buffer, const gchar * s, GtkTextTag **tags) {
    if(!s) return;
    GtkTextIter start, end, real_end;
    gint line = gtk_text_buffer_get_line_count (buffer);
    gchar *a = NULL;
   

    gchar esc[]={0x1B, '[', 0};
    gboolean head_section = strncmp(s, esc, strlen(esc));
    const char *esc_location = strstr (s, esc);
    if(esc_location) {      //vt escape sequence
	// do a split
	    NOOP(stderr, "0.splitting %s\n", s);
	gchar **split = g_strsplit(s, esc, -1);
	if (!split) {
	    DBG("insert_string(): split_esc barfed\n");
	    return;
	}

	gchar **pp=split;
	gint count = 0;
	for (;pp && *pp; pp++, count++){
	    if (strlen(*pp) == 0) continue;
	    NOOP(stderr, "split %d: %s\n", count, *pp);
	    if (count==0 && head_section){
		insert_string (buffer, *pp, NULL);
		continue;
	    }
	    gchar *code = *pp;
	    if (*code == 'K'){
		insert_string (buffer, "\n", NULL);
		continue;
	    }
	    NOOP(stderr, "1.splitting %s\n", *pp);
	    
	    gchar **ss = g_strsplit(*pp, "m", 2);

	    // Insert tags
	    gchar **tags = NULL;
	    if (strchr(ss[0],';')) {
		NOOP(stderr, "2.splitting %s\n", ss[0]);
		tags = g_strsplit(ss[0], ";", -1);
	    } else {
		tags = (gchar **)malloc(sizeof(gchar *) * 2 );
		if (!tags) g_error("malloc: %s\n", "no memory");
		tags[0] = g_strdup(ss[0]);
		tags[1] = 0;
	    }
	    gchar **t = tags;
	    gint tag_count = 0;
	    for (;t && *t; t++)tag_count++;
	    GtkTextTag **gtags = (GtkTextTag **)malloc((tag_count+1)*sizeof(GtkTextTag *));
	    if (!gtags) g_error("malloc: %s\n", "no memory");
	    memset(gtags, 0, (tag_count+1)*sizeof(GtkTextTag *));


	    gint i;
	    for (i=0,t = tags;t && *t; t++) {
		if (!strcmp(*t,"01") || !strcmp(*t,"1")
		    || !strcmp(*t,"05") || !strcmp(*t,"5"))
		{
		    gtags[i++] = resolve_tag(buffer, "tag/bold");
		    continue;
		}
		const gchar *tag = get_ansi_tag(*t);
		if (!tag) {
		    NOOP(stderr, "no ansi tag for %s\n", *t);
		    continue;
		}
		gtags[i++] = resolve_tag(buffer, tag);
		NOOP(stderr, "ansi_tag=%s\n", tag);
	    }


	    // Insert string
	    insert_string (buffer, ss[1], gtags);
	    g_free(gtags);

	    g_strfreev(ss);
	    
	}
	g_strfreev(split);
	// done
	return;
    }

    GtkTextMark *mark = gtk_text_buffer_get_mark (buffer, "rfm-ow");
    gchar *cr = (gchar *)strchr (s, 0x0D);
    if(cr && cr[1] == 0x0A) *cr = ' ';       //CR-LF
    if(strchr (s, 0x0D)) {      //CR
        gchar *aa = g_strdup (s);
        *strchr (aa, 0x0D) = 0;
        insert_string (buffer, aa, tags);
        g_free (aa);
        const char *caa = strchr (s, 0x0D) + 1;
        if(mark) {
            gtk_text_buffer_get_iter_at_line (buffer, &start, line);
            gtk_text_buffer_move_mark (buffer, mark, &start);
        }
        insert_string (buffer, caa, tags);
        g_free (a);
        // we're done
        return;
    }

    gchar *q = utf_string (s);
    /// this should be mutex protected since this function is being called
    //  from threads all over the place.
    static pthread_mutex_t insert_mutex =  PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&insert_mutex);
    if(mark == NULL) {
        gtk_text_buffer_get_end_iter (buffer, &end);
        gtk_text_buffer_create_mark (buffer, "rfm-ow", &end, FALSE);
    } else {
        gtk_text_buffer_get_iter_at_mark (buffer, &end, mark);
    }

    gtk_text_buffer_get_iter_at_line (buffer, &start, line);
    gtk_text_buffer_get_end_iter (buffer, &real_end);

    // overwrite section
    gchar *pretext = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
    gchar *posttext = gtk_text_buffer_get_text (buffer, &end, &real_end, TRUE);
    long prechars = g_utf8_strlen (pretext, -1);
    long postchars = g_utf8_strlen (posttext, -1);
    long qchars = g_utf8_strlen (q, -1);
    g_free (pretext);
    g_free (posttext);
    if(qchars < postchars) {
        gtk_text_buffer_get_iter_at_line_offset (buffer, &real_end, line, prechars + qchars);
    }
    /////
    gtk_text_buffer_delete (buffer, &end, &real_end);
    if(tags) {
	gint tag_count = 0;
	GtkTextTag **tt = tags;
	for (;tt && *tt; tt++)tag_count++;
	switch (tag_count) { // This is hacky
	    case 1:
		gtk_text_buffer_insert_with_tags (buffer, &end, q, -1, 
			tags[0], NULL);
		break;
	    case 2:
		gtk_text_buffer_insert_with_tags (buffer, &end, q, -1,
			tags[0],tags[1], NULL);
		break;
	    default: // Max. 3 tags...
		gtk_text_buffer_insert_with_tags (buffer, &end, q, -1,
			tags[0],tags[1],tags[1], NULL);
		break;
	}
		
    } else {
	NOOP(stderr, "gtk_text_buffer_insert %s\n", q);
        gtk_text_buffer_insert (buffer, &end, q, -1);
    }
    pthread_mutex_unlock(&insert_mutex);
    g_free (q);
    g_free (a);
    return;
}

 
static gboolean trim_diagnostics(GtkTextBuffer *buffer) const {
//  This is just to prevent a memory overrun (intentional or unintentional).
    gint line_count = gtk_text_buffer_get_line_count (buffer);
    glong max_lines_in_buffer = MAX_LINES_IN_BUFFER;
    if (line_count > max_lines_in_buffer) {
        gtk_text_buffer_set_text(buffer, "", -1);
        return TRUE;
    }
    return FALSE;
}

static void
set_font_size (GtkWidget * widget) {
    if (!GTK_IS_WIDGET(widget)) return;
    gint fontsize = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),	"fontsize"));

    gint newsize=8; // default font size.
    const gchar *p = getenv ("RFM_FIXED_FONT_SIZE");
    if(p && strlen (p)) {
	errno=0;
	long value = strtol(p, NULL, 0);
	if (errno == 0){
	    newsize = value;
	}
    }

    if(newsize != fontsize) {
	fontsize = newsize;
	g_object_set_data(G_OBJECT(widget), 
		"fontsize", GINT_TO_POINTER(fontsize));

        GtkStyleContext *style_context = gtk_widget_get_style_context (widget);
        gtk_style_context_add_class(style_context, GTK_STYLE_CLASS_VIEW );
        GtkCssProvider *css_provider = gtk_css_provider_new();
        GError *error=NULL;
        gchar *data = g_strdup_printf("* {\
font-size: %dpx;\
}", fontsize);
        gtk_css_provider_load_from_data (css_provider, data, -1, &error);
        g_free(data);
        if (error){
            fprintf(stderr, "gtk_css_provider_load_from_data: %s\n", error->message);
            g_error_free(error);
            return;
        }
        gtk_style_context_add_provider (style_context, 
                GTK_STYLE_PROVIDER(css_provider),
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        
    }
}



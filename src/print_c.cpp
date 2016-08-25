#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <unistd.h>
#include <gtk/gtk.h>

#include "print_c.hpp"
#include "view_c.hpp"


/////////////////////////////////////////////////////////////////////////////
//              static thread functions                                    //
////////////////////////////////////////////////////////////////////////////
static void *print_f(void *);
static void *print_i(void *);
static void *print_d(void *);
static void *print_e(void *);
static void *print_s(void *);
static void *clear_text_buffer_f(void *);

////////////////////////////////////////////////////////////////////////////
//                       print_c class methods                            //
///////////////////////////////////////////////////////////////////////////

print_c::print_c(void *data){
    view_v = data;
    view_c *view_p = (view_c *)data;
    gtk_p = view_p->get_gtk_p();
    diagnostics = GTK_TEXT_VIEW(view_p->get_diagnostics());
    status_label = GTK_LABEL(view_p->get_status_label());
}

GtkTextView *
print_c::get_diagnostics(void){ return diagnostics;}
GtkLabel *
print_c::get_status_label(void){ return status_label;} 


void print_c::print_error( const gchar *format, ...){
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)this, (void *)"tag/bold", (void *)string};
    context_function(print_e, arg);
    g_free(string);

}

void print_c::print_debug(const gchar *format, ...){
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)this, (void *)"tag/italic", (void *)string};
    context_function(print_d, arg);
    g_free(string);

}


void print_c::print(const gchar *format, ...){
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)this, NULL, (void *)string};
    context_function(print_f, arg);
    g_free(string);
}

void print_c::print_tag(const gchar *tag, const gchar *format, ...){
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)this, (void *)tag, (void *)string};
    context_function(print_f, arg);
    g_free(string);
}

void print_c::print_icon(const gchar *iconname, 
			     const gchar *format, ...)
{
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    GdkPixbuf *pixbuf = gtk_p->get_pixbuf(iconname, -16);
    void *arg[]={(void *)pixbuf, (void *)this, NULL, (void *)string};
    context_function(print_i, arg);
    g_free(string);
}


void print_c::print_icon_tag(const gchar *iconname, 
	                     const gchar *tag, 
			     const gchar *format, ...)
{
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    GdkPixbuf *pixbuf = gtk_p->get_pixbuf(iconname, -16);
    void *arg[]={(void *)pixbuf, (void *)this, (void *)tag, (void *)string};
    context_function(print_i, arg);
    g_free(string);
}

void print_c::print_status(const gchar *format, ...){
    if (!status_label) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);
    if (strstr(string, "\n")) *(strstr(string, "\n")) = 0;

    void *arg[]={(void *)status_label, (void *)string};
    context_function(print_s, arg);
    g_free(string);
}



void print_c::show_text(void){
    view_c *view_p = (view_c *)view_v;
    view_p->show_diagnostics();
}

void print_c::clear_text(void){
    view_c *view_p = (view_c *)view_v;
    view_p->clear_diagnostics();
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



static GtkTextTag **
resolve_tags(GtkTextBuffer * buffer, const gchar *tag){
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

static void
insert_string (print_c *print_p, GtkTextBuffer * buffer, const gchar * s, GtkTextTag **tags) {
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
		insert_string (print_p, buffer, *pp, NULL);
		continue;
	    }
	    gchar *code = *pp;
	    if (*code == 'K'){
		insert_string (print_p, buffer, "\n", NULL);
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
	    insert_string (print_p, buffer, ss[1], gtags);
	    g_free(gtags);

	    g_strfreev(ss);
	    
	}
	g_strfreev(split);
	// done
	return;
    }

    GtkTextMark *mark = gtk_text_buffer_get_mark (buffer, "rfm-ow");
    if(strchr (s, 0x0D)) {      //CR
        gchar *aa = g_strdup (s);
        *strchr (aa, 0x0D) = 0;
        insert_string (print_p, buffer, aa, tags);
        g_free (aa);
        const char *caa = strchr (s, 0x0D) + 1;
        if(mark) {
            gtk_text_buffer_get_iter_at_line (buffer, &start, line);
            gtk_text_buffer_move_mark (buffer, mark, &start);
        }
        insert_string (print_p, buffer, caa, tags);
        g_free (a);
        // we're done
        return;
    }

    gchar *q = print_p->utf_string (s);
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

#define MAX_LINES_IN_BUFFER 1000    
static void
scroll_to_mark(GtkTextView *textview){
    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    GtkTextMark *mark = gtk_text_buffer_get_mark (buffer, "scrollmark");
    if (mark == NULL){
	mark = gtk_text_buffer_create_mark (buffer, "scrollmark", &end, FALSE);
    } else {
	gtk_text_buffer_move_mark   (buffer,  mark  ,&end);
    }
    gtk_text_view_scroll_mark_onscreen (textview, mark);
    gint line_count = gtk_text_buffer_get_line_count (buffer);
    if (line_count > MAX_LINES_IN_BUFFER) {
	gtk_text_buffer_get_iter_at_line (buffer, &start, 0);
	gtk_text_buffer_get_iter_at_line (buffer, &end, line_count - MAX_LINES_IN_BUFFER);
	gtk_text_buffer_delete (buffer, &start, &end);
    }
    return;
}


static void *
print_f(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg = (void **)data;
    print_c *print_p = (print_c *) arg[0];
    GtkTextView *textview = print_p->get_diagnostics();
    if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
    const gchar *tag = (const gchar *)arg[1];
    const gchar *string = (const gchar *)arg[2];

    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    GtkTextTag **tags = resolve_tags(buffer, tag);
    if(string && strlen (string)) {
        insert_string (print_p, buffer, string, tags);
    }
    g_free(tags);
    scroll_to_mark(textview);
    return NULL;
}

#if 0   
static void *
print_op(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    void *arg1[]={arg[0],(void *)"tag/green", arg[1]};
    print_f((void *)arg1);
    void *arg2[]={arg[0], (void *)"tag/bold", arg[2]};
    return print_f((void *)arg2);
}
#endif

static void *
print_i(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    GdkPixbuf *pixbuf = (GdkPixbuf *)arg[0];
    print_c *print_p = (print_c *) arg[1];
    GtkTextView *textview = print_p->get_diagnostics();
    if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_insert_pixbuf (buffer, &end, pixbuf);
    return print_f(arg+1);
}
    

static void *
print_d(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    void *d_arg[]={arg[0],(void *)"tag/italic", (void *)"DBG> "};
    print_f((void *)d_arg);
    return print_f(data);
}
    
    
static void *
print_e(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    void *e_arg1[]={arg[0],(void *)"tag/red", (void *)"*** "};
    print_f((void *)e_arg1);
    return print_f(data);
}


static void *
print_s(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg = (void **)data;
    GtkLabel *status_label = (GtkLabel *)arg[0];
    const gchar *text = (const gchar *)arg[1];
    if (!GTK_IS_LABEL(status_label)) return GINT_TO_POINTER(-1);
    gtk_label_set_markup(status_label, text);
    return NULL;
}

static void *
clear_text_buffer_f(void *data){
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (data));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
    return NULL;
}



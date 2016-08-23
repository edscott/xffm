#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#ifdef HAVE_WINDOWS_H
# include <windows.h>
// build libtubo with:
// ./configure --host=x86_64-w64-mingw32 --prefix=/usr/x86_64-w64-mingw32
#else
#endif

# include <tubo.h>

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <unistd.h>
#include <gtk/gtk.h>

#include "run_c.hpp"


#define DEFAULT_DEBUG TRUE;
//#define DEFAULT_DEBUG FALSE;


////////////////////////////////////////////////////////////////////////////////////
//              static internal functions (context_function and print)            //
////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//                             print.i                              //
//////////////////////////////////////////////////////////////////////


gchar *
utf_string (const gchar * t) {
    if(!t) {
	NOOP("utf_string(): string == NULL!\n");
        return g_strdup ("");
    }

    if(g_utf8_validate (t, -1, NULL)) {
        return g_strdup (t);
    }
    /* so we got a non-UTF-8 */
    /* but is it a valid locale string? */
    gchar *actual_tag;
    actual_tag = g_locale_to_utf8 (t, -1, NULL, NULL, NULL);
    if(actual_tag)
        return actual_tag;
    /* So it is not even a valid locale string... 
     * Let us get valid utf-8 caracters then: */
    const gchar *p;
    actual_tag = g_strdup ("");
    for(p = t; *p; p++) {
        // short circuit end of string:
        gchar *r = g_locale_to_utf8 (p, -1, NULL, NULL, NULL);
        if(g_utf8_validate (p, -1, NULL)) {
            gchar *qq = g_strconcat (actual_tag, p, NULL);
            g_free (actual_tag);
            actual_tag = qq;
            break;
        } else if(r) {
            gchar *qq = g_strconcat (actual_tag, r, NULL);
            g_free (r);
            g_free (actual_tag);
            actual_tag = qq;
            break;
        }
        // convert caracter to utf-8 valid.
        gunichar gu = g_utf8_get_char_validated (p, 2);
        if(gu == (gunichar) - 1) {
            //DBG("gu=%d\n",(int)gu);
            gu = g_utf8_get_char_validated ("?", -1);
        }
        gchar outbuf[8];
        memset (outbuf, 0, 8);
        gint outbuf_len = g_unichar_to_utf8 (gu, outbuf);
        if(outbuf_len < 0) {
            DBG ("unichar=%d char =%c outbuf_len=%d\n", gu, p[0], outbuf_len);
        }
        gchar *qq = g_strconcat (actual_tag, outbuf, NULL);
        g_free (actual_tag);
        actual_tag = qq;
    }
    return actual_tag;
}


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

static void *
clear_text_buffer_f(void *data){
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (data));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
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
insert_string (GtkTextBuffer * buffer, const gchar * s, GtkTextTag **tags) {
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
    GtkTextView *textview = (GtkTextView *)arg[0];
    if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
    const gchar *tag = (const gchar *)arg[1];
    const gchar *string = (const gchar *)arg[2];

    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    GtkTextTag **tags = resolve_tags(buffer, tag);
    if(string && strlen (string)) {
        insert_string (buffer, string, tags);
    }
    g_free(tags);
    scroll_to_mark(textview);
    return NULL;
}
    
static void *
print_d(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    void *out_arg[]={arg[0],(void *)"tag/italic", (void *)"DBG> "};
    print_f((void *)out_arg);
    return print_f(data);
}
    
static void *
print_op(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    void *arg1[]={arg[0],(void *)"tag/green", arg[1]};
    print_f((void *)arg1);
    void *arg2[]={arg[0], (void *)"tag/bold", arg[2]};
    return print_f((void *)arg2);
}
    
static void *
print_e(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    void *arg1[]={arg[0],(void *)"tag/red", (void *)"*** "};
    print_f((void *)arg1);
    void *arg2[]={arg[0], arg[1], arg[2]};
    return print_f((void *)arg2);
}

static void *
print_s(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg = (void **)data;
    GtkTextView *textview = (GtkTextView *)arg[0];
    if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    gtk_text_buffer_set_text (buffer, " ", -1);
    return print_f(data);
}


////////////////////////////////////////////////////////////////////////////////////
//                             run_c class methods                                //
////////////////////////////////////////////////////////////////////////////////////


void *run_c::gtk_context(void * (*function)(gpointer), void * function_data){
    return context_function(function, function_data);
}


void run_c::print(GtkWidget *textview, const gchar *tag, const gchar *string){
    if (!textview) return;
    void *arg[]={(void *)textview, (void *)tag, (void *)string};
    context_function(print_f, arg);
}

void run_c::print_error( const gchar *format, ...){
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)diagnostics, (void *)"tag/bold", (void *)string};
    context_function(print_e, arg);
    g_free(string);

}

void run_c::print_operation(gpointer thread, const gchar *format, ...){
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    gchar *p = (thread != NULL)?
        g_strdup_printf("%p> ", thread):
        g_strdup("metimp> ");
    void *arg[]={(void *)diagnostics, (void *)p, (void *)string};
    context_function(print_op, arg);
    g_free(p);
    g_free(string);

}

void run_c::print_debug(const gchar *format, ...){
    if (!debug || !diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)diagnostics, (void *)"tag/italic", (void *)string};
    context_function(print_d, arg);
    g_free(string);

}

void run_c::print_diagnostics(const gchar *tag, const gchar *format, ...){
    if (!diagnostics) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);

    void *arg[]={(void *)diagnostics, (void *)tag, (void *)string};
    context_function(print_f, arg);
    g_free(string);
}

void run_c::print_status(const gchar *tag, const gchar *format, ...){
    if (!status) return;
    va_list var_args;
    va_start (var_args, format);
    gchar *string = g_strdup_vprintf(format, var_args);
    va_end (var_args);
    if (strstr(string, "\n")) *(strstr(string, "\n")) = 0;

    void *arg[]={(void *)status, (void *)tag, (void *)string};
    context_function(print_s, arg);
    g_free(string);
}

void run_c::clear_text_buffer(GtkWidget *data){
    context_function(clear_text_buffer_f, (void *)data);
}

static void *thread_f(void *data){
}
static void *wait_f(void *data){
}

static void
operate_stdout (void *data, void *stream, gint childFD) {
    run_c *run_p = (run_c *)data;
    gchar *line = (gchar *)stream;

    run_p->print_diagnostics("tag/green", "%s", line);
    // This is a bit hacky, to keep runaway output from hogging
    // up the gtk event loop.
    static gint count = 1;
    if (count % 20 == 0){
        usleep(100000);
    } else {
        usleep(1000);
    }
   return;
}
static void
operate_stderr (void *data, void *stream, gint childFD) {
    run_c *run_p = (run_c *)data;
    gchar *line = (gchar *)stream;

    run_p->print_diagnostics("tag/red", "%s", line);
    // This is a bit hacky, to keep runaway output from hogging
    // up the gtk event loop.
    static gint count = 1;
    if (count % 20 == 0){
        usleep(100000);
    } else {
        usleep(1000);
    }
   return;
}

gboolean done_f(void *data) {
    run_c *run_p =(run_c *)data;
    run_p->print_diagnostics("tag/bold", "%s\n", "run complete.");
    return FALSE;
}

#ifdef HAVE_WINDOWS_H
static void *
windows_exec(void *data){
    void **args = (void **)data;
    run_c *run_p = (run_c *)args[0];
    gint so = GPOINTER_TO_INT(args[1]);
    gint se = GPOINTER_TO_INT(args[2]); 
    GPid pid = (GPid)(args[3]); 
    const gchar *tag = se? "tag/red": "tag/grey";
    FILE* fp = fdopen(so?so:se, "r");
    gchar buffer[4096];
    
    if (!se) run_p->print_diagnostics("tag/blue", " > %s", "Hello Windows subprocess\n");

    while (fgets(buffer, 4096, fp) && !feof(fp)){
        //fprintf(stderr, "got: %s", buffer);
        run_p->print_diagnostics(tag, " > %s", buffer);
    }
    if (!se) run_p->print_diagnostics("tag/blue", " > %s", "Goodbye Windows subprocess\n");
    fclose(fp);
    close(so?so:se);

    g_spawn_close_pid(pid);

}
#endif

static
void
fork_finished_function (void *data) {
    g_timeout_add(1, done_f, data);                                                
}


pid_t run_c::thread_run(gchar **arguments){
    gchar *command = g_strdup("");
    gchar **p = arguments;
    for (;p && *p; p++){
        gchar *g =  g_strdup_printf("%s %s", command, *p);
        g_free(command);
        command = g;
    }
    print_operation(NULL, "%s\n", command);
    g_free(command);
#ifdef HAVE_WINDOWS_H
    print_diagnostics("tag/bold", "run_c::thread_run() currently not available in Windows.\nTrying g_exec\n");
    GPid pid;
    GError *error=NULL;

    gint fd, fe;

    
    g_spawn_async_with_pipes (NULL, //const gchar *working_directory,
                          arguments, //gchar **argv,
                          NULL, //gchar **envp,
                          G_SPAWN_DEFAULT, //G_SPAWN_DO_NOT_REAP_CHILD, //GSpawnFlags flags,
                          NULL, // ignored by W. GSpawnChildSetupFunc child_setup,
                          NULL, // ignored by W. gpointer user_data,
                          &pid, //GPid *child_pid,
                          NULL, //gint *standard_input,
                          &fd,
                          &fe, //&standard_error,
                          &error); //GError **error);
    

    if (error){
        print_error("g_spawn error: %s\n", error->message);
        g_error_free(error);
    } else {
        print_diagnostics("tag/bold", "Started process with handle:%d\n", pid);
        pthread_t thread;
        void *arg1[] = {(void *)this, GINT_TO_POINTER(fd), NULL, GINT_TO_POINTER(pid)};
        void *arg2[] = {(void *)this, NULL, GINT_TO_POINTER(fe), GINT_TO_POINTER(pid)};
        pthread_create(&thread, NULL, windows_exec, arg1);
        pthread_create(&thread, NULL, windows_exec, arg2);
    }
#else
    int flags = TUBO_REAP_CHILD|TUBO_VALID_ANSI;
    pid_t pid = Tubo_exec (arguments,
                                NULL, // stdin
                                operate_stdout, //stdout_f,
                                operate_stderr, //stderr_f
                                fork_finished_function,
                                this,
                                flags);
    print_diagnostics("tag/bold", "Started process with pid:%d\n", pid);
#endif
}

void
run_c::set_debug(gboolean state){
    debug=state;
}

run_c::run_c(void){
    diagnostics = NULL;
    status = NULL;
    debug = DEFAULT_DEBUG;
}

run_c::run_c(GtkWidget *data1){
    diagnostics = GTK_TEXT_VIEW(data1);
    status = NULL;
    debug = DEFAULT_DEBUG;
}

run_c::run_c(GtkWidget *data1, GtkWidget *data2){
    diagnostics = GTK_TEXT_VIEW(data1);
    status = GTK_TEXT_VIEW(data2);
    debug = DEFAULT_DEBUG;
}


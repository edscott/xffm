#ifndef XFPRINT_HH
#define XFPRINT_HH

#define DEFAULT_FIXED_FONT_SIZE 13

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <pwd.h>
#include <gtk/gtk.h>


static gint defaultFixedFontSize=12;
#define MAX_LINES_IN_BUFFER 10000    
namespace xf
{


template <class Type>
class Print {
    using util_c = Util<double>;
    using pixbuf_c = Pixbuf<double>;
public:

    static void print(GtkTextView *textview, const gchar *tag, gchar *string){
        if (!textview) return;
	void *arg[]={(void *)textview, (void *)tag, (void *)string};
	context_function(print_f, arg);
	g_free(string);
    }
   
    static void print(GtkTextView *textview, gchar *string){
        print(textview, NULL, string);
    }


    static void print_debug(GtkTextView *textview, gchar *string){
        if (!textview) return;
	void *arg[]={(void *)textview, (void *)"italic", (void *)string};
	context_function(print_d, arg);
	g_free(string);

    }
        
    static void print_icon(GtkTextView *textview, const gchar *iconname, gchar *string)
    {
        if (!textview) return;
	auto pixbuf = pixbuf_c::get_pixbuf(iconname, -16);
	void *arg[]={(void *)pixbuf, (void *)textview, NULL, (void *)string};
	context_function(print_i, arg);
	g_free(string);
    }

    static void print_icon(GtkTextView *textview, 
                                const gchar *iconname, 
				const gchar *tag, 
				gchar *string)
    {
        if (!textview) return;
	auto pixbuf = pixbuf_c::get_pixbuf(iconname, -16);
	void *arg[]={(void *)pixbuf, (void *)textview, (void *)tag, (void *)string};
	context_function(print_i, arg);
	g_free(string);
    }

    static void print_status(GtkTextView *textview, gchar *string){
        if (!textview) return;
	void *arg[]={(void *)textview, (void *)string};
	context_function(print_s, arg);
	g_free(string);
    }

    static void print_status_label(GtkLabel *label, gchar *string){
        if (!label || !string) return;
	void *arg[]={(void *)label, (void *)string};
	context_function(print_sl, arg);
	g_free(string);
    }

    static void print_error(GtkTextView *textview, gchar *string){
        if (!textview) return;
	void *arg[]={(void *)textview, (void *)"bold", (void *)string};
	context_function(print_e, arg);
	g_free(string);

    }


    static void clear_text(GtkTextView *textview){
        if (!textview) return;
	void *arg[]={(void *)textview, NULL};
        context_function(clear_text_buffer_f, arg);
    }


    static void show_text(GtkTextView *textview){
        if (!textview) return;
        auto vpane = GTK_PANED(g_object_get_data(G_OBJECT(textview), "vpane"));
	void *arg[]={(void *)vpane, NULL};
        context_function(show_text_buffer_f, arg);
    }

    static void show_textFull(GtkTextView *textview){
        if (!textview) return;
        auto vpane = GTK_PANED(g_object_get_data(G_OBJECT(textview), "vpane"));
	void *arg[]={(void *)vpane, GINT_TO_POINTER(1),NULL};
        context_function(show_text_buffer_f, arg);
    }
 /*   static gboolean show_textFull(void *data){
	show_textFull(GTK_TEXT_VIEW(data));
	return FALSE;
    }
    static gboolean hide_text(void *data){
	hide_text(GTK_TEXT_VIEW(data));
	return FALSE;
    } */


    static void hide_text(GtkTextView *textview){
        if (!textview) return;
        auto vpane = GTK_PANED(g_object_get_data(G_OBJECT(textview), "vpane"));
	void *arg[]={(void *)vpane, NULL};
        context_function(hide_text_buffer_f, arg);
    }


    static void *
    scroll_to_top(GtkTextView *textview){
        if (!textview) return NULL;
	// make sure all text is written before attempting scroll
	while (gtk_events_pending()) gtk_main_iteration();
	GtkTextIter start, end;
	auto buffer = gtk_text_view_get_buffer (textview);
	gtk_text_buffer_get_bounds (buffer, &start, &end);
        gtk_text_view_scroll_to_iter (textview,
                              &start,
                              0.0,
                              FALSE,
                              0.0, 0.0);        
	while (gtk_events_pending()) gtk_main_iteration();
	return NULL;
    }

    static void *
    scroll_to_bottom(GtkTextView *textview){
        if (!textview) return NULL;
	// make sure all text is written before attempting scroll
	while (gtk_events_pending()) gtk_main_iteration();
	GtkTextIter start, end;
	auto buffer = gtk_text_view_get_buffer (textview);
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	auto mark = gtk_text_buffer_create_mark (buffer, "scrolldown", &end, FALSE);
	gtk_text_view_scroll_to_mark (textview, mark, 0.2,    /*gdouble within_margin, */
				      TRUE, 1.0, 1.0);
	//gtk_text_view_scroll_mark_onscreen (textview, mark);
	gtk_text_buffer_delete_mark(buffer, mark);
	return NULL;
    }


private:
    static gboolean
    context_function_f(gpointer data){
        void **arg = (void **)data;
        void * (*function)(gpointer) = (void* (*)(void*))arg[0];
        gpointer function_data = arg[1];
        pthread_mutex_t *mutex = (pthread_mutex_t *)arg[2];
        pthread_cond_t *signal = (pthread_cond_t *)arg[3];
        auto result_p = (void **)arg[4];
        void *result = (*function)(function_data);
        pthread_mutex_lock(mutex);
        *result_p = result;
        pthread_cond_signal(signal);
        pthread_mutex_unlock(mutex);
        return FALSE;
    }

    static void *context_function(void * (*function)(gpointer), void * function_data){
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t signal = PTHREAD_COND_INITIALIZER; 
	auto result=GINT_TO_POINTER(-1);
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
	    g_main_context_invoke(NULL, CONTEXT_CALLBACK(context_function_f), arg);
	    pthread_mutex_lock(&mutex);
	    if (result == GINT_TO_POINTER(-1)) pthread_cond_wait(&signal, &mutex);
	    pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&signal);
	return result;
    }

    static void * 
    print_f(void *data){
        if (!data) return GINT_TO_POINTER(-1);
        void **arg = (void **)data;
        auto textview = GTK_TEXT_VIEW(arg[0]);
        if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
        auto tag = (const gchar *)arg[1];
        auto string = (const gchar *)arg[2];
        set_font_size (GTK_WIDGET(textview));

        auto buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
        if (trim_diagnostics(buffer)){
            // textview is at line limit.
        }

        auto tags = resolve_tags(buffer, tag);
        if(string && strlen (string)) {
            insert_string (buffer, string, tags);
        }
        g_free(tags);
        scroll_to_bottom(textview);
        return NULL;
    }

    static void *
    print_e(void *data){
        if (!data) return GINT_TO_POINTER(-1);
        void **arg=(void **)data;
        void *e_arg1[]={arg[0],(void *)"red", (void *)"*** "};
        print_f((void *)e_arg1);
        return print_f(data);
    }
    static void *
    print_d(void *data){
        if (!data) return GINT_TO_POINTER(-1);
        void **arg=(void **)data;
        void *d_arg[]={arg[0],(void *)"italic", (void *)"DBG> "};
        print_f((void *)d_arg);
        return print_f(data);
    }

    static void *
    print_i(void *data){
        if (!data) return GINT_TO_POINTER(-1);
        void **arg=(void **)data;
        GtkTextView *textview = GTK_TEXT_VIEW(arg[1]);
        GdkPixbuf *pixbuf = (GdkPixbuf *)arg[0];
        if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        gtk_text_buffer_insert_pixbuf (buffer, &end, pixbuf);
        return print_f(arg+1);
    }
    static void *
    print_s(void *data){
        if (!data) return GINT_TO_POINTER(-1);
        auto arg = (void **)data;
        auto textview = GTK_TEXT_VIEW(arg[0]);
 
        if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
        auto string = (const gchar *)arg[1];

        set_font_size (GTK_WIDGET(textview));
        auto buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        gtk_text_buffer_delete (buffer, &start, &end);
        if(string && strlen (string)) {
            insert_string (buffer, string, NULL);
        }
        return NULL;
    }
    static void *
    print_sl(void *data){
        if (!data) return GINT_TO_POINTER(-1);
        auto arg = (void **)data;
        auto status_label = GTK_LABEL(arg[0]);
        auto text = (const gchar *)arg[1];
        if (!GTK_IS_LABEL(status_label)) return GINT_TO_POINTER(-1);
        gtk_label_set_markup(status_label, text);
        return NULL;
    }


public:
    static gchar *
    get_current_text (GtkTextView *textview) {
	// get current text
	GtkTextIter start, end;
	auto buffer = gtk_text_view_get_buffer (textview);

	gtk_text_buffer_get_bounds (buffer, &start, &end);
	auto t = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
	g_strchug(t);
	return t;
    }

    static gchar *
    get_text_to_cursor (GtkTextView *textview) {
	// get current text
	GtkTextIter start, end;
	auto buffer = gtk_text_view_get_buffer (textview);
	gint cursor_position;
	// cursor_position is a GtkTextBuffer internal property (read only)
	g_object_get (G_OBJECT (buffer), "cursor-position", &cursor_position, NULL);
	
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 0);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, cursor_position);
	auto t = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
	g_strchug(t);
	TRACE ("lpterm_c::get_text_to_cursor: to cursor position=%d %s\n", cursor_position, t);
	return t;
    }
private:

    static void *
    clear_text_buffer_f(void *data){
        if (!data) return GINT_TO_POINTER(-1);
        auto arg=(void **)data;
        auto buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (arg[0]));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        gtk_text_buffer_delete (buffer, &start, &end);
        return NULL;
    }

    static void *
    hide_text_buffer_f (void *data) {
        if (!data) return GINT_TO_POINTER(-1);
        auto arg=(void **)data;
        auto vpane = GTK_PANED(arg[0]);
        if(!vpane) {
            ERROR("vpane is NULL\n");
            return NULL;
        }
	gint max;
	GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(vpane));
	GtkAllocation allocation;
	gtk_widget_get_allocation(window, &allocation);
	gint height = allocation.height;

	TRACE("setting vpane position to %d\n", height);
        gtk_paned_set_position (vpane, height);
	g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(height));
        return NULL;
    }

    static void *
    show_text_buffer_f (void *data) {
        if (!data) return GINT_TO_POINTER(-1);
        auto arg=(void **)data;
        auto vpane = GTK_PANED(arg[0]);
	auto fullview =arg[1]; 
        if(!vpane) {
            ERROR("vpane is NULL\n");
            return NULL;
        }
        gint min, max;
	g_object_get(G_OBJECT(vpane), "min-position", &min, NULL);
	g_object_get(G_OBJECT(vpane), "max-position", &max, NULL);
	if (fullview) {
	    TRACE("show_text_buffer_f:setting vpane position to %d\n", min);
	    gtk_paned_set_position (vpane, min);
	    g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(min));
            while (gtk_events_pending()) gtk_main_iteration();
            TRACE("vpane position set to =%d\n", gtk_paned_get_position(vpane));
	    return NULL;
	}
	//GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(vpane));
	GtkAllocation allocation;
	//gtk_widget_get_allocation(window, &allocation);
	//gint height = allocation.height;
	gtk_widget_get_allocation(GTK_WIDGET(vpane), &allocation);
	gint vheight = allocation.height;
        gint height = 2*vheight/4;

        TRACE("vheight = %d, position = %d\n", vheight, gtk_paned_get_position(vpane));
	if (gtk_paned_get_position(vpane) > height) {
	    TRACE("setting vpane position to %d\n", height);
	    gtk_paned_set_position (vpane, height);
	    g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(height));
	} else TRACE("not setting vpane position to %d\n", height);
        return NULL;
    }

/*
     static const gchar *
    get_workdir(void){
	view_c *view_p = (view_c *)view_v;
	return view_p->get_path();
    }
*/







    static const gchar *
    get_ansi_tag(const gchar *code){
        static sequence_t sequence_v[] = {
                {"black", "30"},
                {"black_bg", "40"},
                {"red", "31"},
                {"red_bg", "41"},
                {"green", "32"},
                {"green_bg", "42"},
                {"yellow", "33"},
                {"yellow_bg", "43"},
                {"blue", "34"},
                {"blue_bg", "44"},
                {"magenta", "35"},
                {"magenta_bg", "45"},
                {"cyan", "36"},
                {"cyan_bg", "46"},
                {"white", "37"},
                {"white_bg", "47"},
              
                {"Black_bg", "100"},
                {"Black", "90"},
                {"Red_bg", "101"},
                {"Red", "91"},
                {"Green_bg", "102"},
                {"Green", "92"},
                {"Yellow_bg", "103"},
                {"Yellow", "93"},
                {"Blue_bg", "104"},
                {"Blue", "94"},
                {"Magenta_bg", "105"},
                {"Magenta", "95"},
                {"Cyan_bg", "106"},
                {"Cyan", "96"},
                {"White_bg", "107"},
                {"White", "97"},

                {"bold", "1"},
                {"bold", "01"},
                {"italic", "4"},
                {"italic", "04"},
                {"blink", "5"},
                {"blink", "05"},
                {NULL, ""},
                {NULL, "0"},
                {NULL, "00"},
                {NULL, "22"},
                {NULL, "24"},
                {NULL, NULL}            // this marks the end of sequences.
        };
        sequence_t *p;
        for(p = sequence_v; p && p->sequence; p++) {
            if(strcmp (code, p->sequence) == 0) {
                return p->id;
            }
        }
        return NULL;

    }


    static
    GtkTextTag *
    resolve_tag (GtkTextBuffer * buffer, const gchar * id) {
        if (!id) return NULL;

        GtkTextTag *tag = NULL;
        lpterm_colors_t lpterm_colors_v[] = {

            {"command", {101, 0x5858, 0x3434, 0xcfcf}},
            {"stderr", {102, 0xcccc, 0, 0}},
            {"command_id", {103, 0x0000, 0x0000, 0xffff}},
            {"grey", {110, 0x8888, 0x8888, 0x8888}},
// xterm colors
            // normal
            {"black", {201, 0x0000, 0x0000, 0x0000}},
            {"red", {202, 0xcdcd, 0x0, 0x0}},
            {"green", {203, 0x0, 0xcdcd, 0x0}},
            {"yellow", {204, 0xcdcd, 0xcdcd, 0x0}},
            {"blue", {205, 0x0, 0x0, 0xeeee}},
            {"magenta", {206, 0xcdcd, 0x0, 0xcdcd}},
            {"cyan", {207, 0x0, 0xcdcd, 0xcdcd}},
            {"white", {208, 0xe5e5, 0xe5e5, 0xe5e5}}, //gray
            // bright
            {"Black", {211, 0x7f7f, 0x7f7f, 0x7f7f}},
            {"Red", {212, 0xffff, 0x0, 0x0}},
            {"Green", {213, 0x0, 0xffff, 0x0}},
            {"Yellow", {214, 0xffff, 0xffff, 0x0}},
            {"Blue", {215, 0x0, 0x0, 0xffff}},
            {"Magenta", {216, 0xffff, 0x0, 0xffff}},
            {"Cyan", {217, 0x0, 0xffff, 0xffff}},
            {"White", {218, 0xffff, 0xffff, 0xffff}}, 

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
                    "bold", "weight", PANGO_WEIGHT_BOLD, "foreground_gdk", NULL, NULL);
            gtk_text_buffer_create_tag (buffer, 
                    "italic", "style", PANGO_STYLE_ITALIC, "foreground_gdk", NULL, NULL);
            g_object_set_data(G_OBJECT(buffer), "text_tag_initialized", GINT_TO_POINTER(1));
        } 

        tag = gtk_text_tag_table_lookup (gtk_text_buffer_get_tag_table (buffer), id);
        // if (!tag) ERROR("No GtkTextTag for %s\n", id);
        return tag;
    }

    static GtkTextTag **
    resolve_tags(GtkTextBuffer * buffer, const gchar *tag){
        if (!tag) return NULL;
        gchar **userTags;
        if (strchr(tag, '/')){
            // multiple tags
            userTags = g_strsplit(tag, "/", -1);
        } else {
            userTags = (gchar **)calloc(2, sizeof(GtkTextTag *));
            userTags[0] = g_strdup(tag);
        }
        gchar **t;
        gint tag_count = 0;
        for (t = userTags;t && *t; t++){
            tag_count++;
        }
        auto tags = (GtkTextTag **)calloc(tag_count+1, sizeof(GtkTextTag *));
        int i=0;
        for (t=userTags; t && *t;t++){
            tags[i] = resolve_tag (buffer, *t);
            if (tags[i] == NULL) {
                ERROR("*** print_c::invalid tag: \"%s\"\n", *t);
            } else i++;
        }
        g_strfreev(userTags);
        return tags;
    }

    static gchar *
    concat(gchar **fullString, const gchar *separator, const gchar* addOn){
        gchar *newString;
        if (!(*fullString)) newString=g_strdup(addOn);
        else {
            newString = g_strconcat(*fullString, separator, addOn, NULL);
            g_free(*fullString);
        }
        return newString;
    }

    static void insert_string (GtkTextBuffer * buffer, const gchar * s, GtkTextTag **tags) {
        if(!s) return;
        GtkTextIter start, end, real_end;
        auto line = gtk_text_buffer_get_line_count (buffer);
        gchar *a = NULL;
       

        gchar esc[]={0x1B, '[', 0};
        gboolean head_section = strncmp(s, esc, strlen(esc));
        const char *esc_location = strstr (s, esc);
        if(esc_location) {      // escape sequence: <ESC>[

            // do a split
                TRACE( "0.splitting %s\n", s);
            gchar **split = g_strsplit(s, esc, -1);
            insert_string (buffer, split[0], NULL);
            gchar *fullTag=NULL;
            gchar **pp=split+1;
            GtkTextTag **textviewTags = NULL;
            // single
            for (;pp && *pp; pp++){
                gchar *code = *pp;
               
                // this was for mpg123... FIXME no eol
                // mpg123 is not providing new lines
                /* if (*code == 'K'){
                    insert_string (buffer, "\n", NULL);
                    continue;
                }*/
                if (*code == 'K'){
                     // erase from cursor to eol
                     // insert string with whatever tag you have.
                    insert_string (buffer, code+1, textviewTags);
                    continue;
                }
                /* if (strncmp(code, "0K", 2)==0 ||strncmp(code, "1K", 2)==0 || strncmp(code, "2K", 2)==0){
                    // erase from cursor to eol, from bol to cursor, erase line
                    fullString = concat(&fullString, "", code+2);
                    continue;
                }*/
                 // split on m
                if (*code == 'm'){
                    g_free(fullTag); fullTag = NULL;
                    g_free(textviewTags); textviewTags = NULL;
                    insert_string (buffer, code+1, NULL);
                    continue;
                }
                gchar **ss = g_strsplit(code, "m", 2);
                gchar **codes;
                // Check for multiple tags
                if (strchr(code, ';')){
                    codes = g_strsplit(ss[0], ";", -1);
                } else {
                    codes = (gchar **)calloc(2, sizeof(gchar*));
                    codes[0] = g_strdup(ss[0]);
                }
                TRACE( "1.splitting %s --> %s, %s: codes[0]=%s\n", *pp, ss[0], ss[1],codes[0]);
                // construct xffm tag
                gchar **t;
                gchar *thisTag=NULL;
                for (t=codes; t && *t; t++){
                    const gchar *ansiTag = get_ansi_tag(*t);
                    if (!ansiTag){
                        if (strcmp(*t, "0")) {
                            g_free(fullTag); fullTag = NULL;
                            g_free(textviewTags); textviewTags = NULL;
                            insert_string (buffer, ss[1], NULL);
                            g_strfreev(ss);
                            goto endloop;
                        } else {
			    WARN("no ansiTag for \"%s\"\n", *t);
			}
                        continue;
                    } else {
                        TRACE("ansiTag=%s\n", ansiTag);
                        fullTag = concat(&fullTag, "/", ansiTag);
                        TRACE("fullTag= %s\n", fullTag); 
                    }
                }
                // Insert string
                g_free(textviewTags);
                if (fullTag) textviewTags = resolve_tags(buffer, fullTag);
                else textviewTags = NULL;
                insert_string (buffer, ss[1], textviewTags);
                g_strfreev(ss);
endloop:;
            }
            g_free(fullTag);
            g_free(textviewTags);
            g_strfreev(split);
            // done
            return;
        }

        auto mark = gtk_text_buffer_get_mark (buffer, "rfm-ow");
        gchar *cr = (gchar *)strchr (s, 0x0D);
        if(cr && cr[1] == 0x0A) *cr = ' ';       //CR-LF
        if(strchr (s, 0x0D)) {      //CR
            auto aa = g_strdup (s);
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

        gchar *q = util_c::utf_string (s);
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
        auto pretext = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
        auto posttext = gtk_text_buffer_get_text (buffer, &end, &real_end, TRUE);
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
                            tags[0],tags[1],tags[2], NULL);
                    break;
            }
                    
        } else {
            TRACE( "gtk_text_buffer_insert %s\n", q);
            gtk_text_buffer_insert (buffer, &end, q, -1);
        }
        pthread_mutex_unlock(&insert_mutex);
        g_free (q);
        g_free (a);
        return;
    }

     
    static gboolean trim_diagnostics(GtkTextBuffer *buffer) {
    //  This is just to prevent a memory overrun (intentional or unintentional).
        auto line_count = gtk_text_buffer_get_line_count (buffer);
        auto max_lines_in_buffer = MAX_LINES_IN_BUFFER;
        if (line_count > max_lines_in_buffer) {
            gtk_text_buffer_set_text(buffer, "", -1);
            return TRUE;
        }
        return FALSE;
    }
public:
    static void
    setColor(GtkWidget * widget) {
        auto style_context = gtk_widget_get_style_context (widget);
        gtk_style_context_add_class(style_context, GTK_STYLE_CLASS_VIEW );
        auto css_provider = gtk_css_provider_new();
        GError *error=NULL;
        auto data = g_strdup_printf("\
textview text {\
   background-color: black;\
   color: white;\
  }\
.view text selection {\
  background-color: blue;\
  color: yellow;\
}\
");
        gtk_css_provider_load_from_data (css_provider, data, -1, &error);
        g_free(data);
        if (error){
            ERROR("gtk_css_provider_load_from_data: %s\n", error->message);
            g_error_free(error);
            return;
        }
        gtk_style_context_add_provider (style_context, 
                GTK_STYLE_PROVIDER(css_provider),
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    }
    static void
    set_font_size (GtkWidget * widget, gint fontsize) {
        if (!GTK_IS_WIDGET(widget)) return;
        auto oldfontsize = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),	"fontsize"));

        if (oldfontsize == fontsize) return;


        TRACE("fontsize %d --> %d\n", oldfontsize, fontsize);
        g_object_set_data(G_OBJECT(widget), "fontsize", GINT_TO_POINTER(fontsize));

        auto style_context = gtk_widget_get_style_context (widget);
        gtk_style_context_add_class(style_context, GTK_STYLE_CLASS_VIEW );
        auto css_provider = gtk_css_provider_new();
        GError *error=NULL;
        auto data = g_strdup_printf("* {\
font-size: %dpx;\
}", fontsize);
        gtk_css_provider_load_from_data (css_provider, data, -1, &error);
        g_free(data);
        if (error){
            ERROR("gtk_css_provider_load_from_data: %s\n", error->message);
            g_error_free(error);
            return;
        }
        gtk_style_context_add_provider (style_context, 
                GTK_STYLE_PROVIDER(css_provider),
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
private:
    static void
    set_font_size (GtkWidget * widget) {
        if (!GTK_IS_WIDGET(widget)) return;
        auto fontsize = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),"fontsize"));
        if (!fontsize){
            set_font_size(widget, DEFAULT_FIXED_FONT_SIZE);
        }
    }

};
}

#endif


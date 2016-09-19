#include "mime_magic_c_hpp"


#ifdef HAVE_LIBMAGIC
static GMutex *magic_mutex = NULL;
static magic_t cookie;
static gsize initialized = 0;
#endif
 
G_MODULE_EXPORT void *
module_active (void){
    return GINT_TO_POINTER(1);
}

#ifdef HAVE_LIBMAGIC

// Lib magic is available...
//
// not thread safe: put in a mutex.
// This function may obtain a basic or alias mimetype, but will always
// return a basic mimetype.
static gchar *
lib_magic (const gchar * file, int flags) {
    gchar *type=NULL;
    g_mutex_lock (magic_mutex);    
    magic_setflags (cookie, flags);
    const char *ctype = magic_file (cookie, file);
    if (ctype) type = g_strdup(ctype);
    g_mutex_unlock (magic_mutex);    
    return type;
}

// see "man libmagic" for explantion of flags
// Since MAGIC_NO_CHECK_ENCODING is not in file 4.x, we take care
// of that here.
#ifndef MAGIC_MIME_TYPE
#define MAGIC_MIME_TYPE  0
#endif
#ifndef MAGIC_NO_CHECK_APPTYPE
#define MAGIC_NO_CHECK_APPTYPE  0
#endif
#ifndef MAGIC_NO_CHECK_ENCODING
#define MAGIC_NO_CHECK_ENCODING  0
#endif
#ifndef MAGIC_SYMLINK
#define MAGIC_SYMLINK  0
#endif
#ifndef MAGIC_NO_CHECK_COMPRESS
#define MAGIC_NO_CHECK_COMPRESS  0
#endif
#ifndef MAGIC_NO_CHECK_TAR
#define MAGIC_NO_CHECK_TAR  0
#endif
#ifndef MAGIC_PRESERVE_ATIME
#define  MAGIC_PRESERVE_ATIME 0
#endif

//#define DISABLE_MAGIC

G_MODULE_EXPORT 
void *
mime_magic (void *p) {
    const gchar *file = p;
    NOOP(stderr, "mime_magic(%s)...\n", 
	    file);
    // Does the user even have read permission?
    if (access(file, R_OK) < 0){
	const gchar *h_type =
	    _("No Read Permission");
        return (void *)g_strdup(h_type);
    }
    
    gint flags = MAGIC_MIME_TYPE | MAGIC_SYMLINK | MAGIC_PRESERVE_ATIME;
    gchar *mimemagic = lib_magic (file, flags);
    NOOP(stderr, "mime_magic(%s)...%s\n", file, mimemagic);
    gchar *old_type = mimemagic; 
    mimemagic = rfm_natural(RFM_MODULE_DIR, "mime", mimemagic, "mime_get_alias_type");
    g_free(old_type);
    return (void *)mimemagic;
}

G_MODULE_EXPORT 
void *
mime_encoding (void *p) {
    const gchar *file = p;
    NOOP(stderr, "mime_encoding(%s)...\n", file);
    // Does the user even have read permission?
    if (access(file, R_OK) < 0){
	const gchar *h_type =
	    _("No Read Permission");
        return (void *)g_strdup(h_type);
    }
    //int flags = MAGIC_MIME_ENCODING;
    int flags = MAGIC_MIME_ENCODING | MAGIC_PRESERVE_ATIME | MAGIC_SYMLINK;
    gchar *encoding = lib_magic (file, flags);
    if (encoding) {
	NOOP(stderr, "%s --> %s\n", file, encoding);
	return (void *)encoding;
    }
    return NULL;
}

G_MODULE_EXPORT 
void *
mime_file (void *p) {
    const gchar *file = p;
    NOOP(stderr, "mime_file(%s)...\n", file);
    gint flags =  MAGIC_PRESERVE_ATIME;
    gchar *f = lib_magic (file, flags);
    NOOP(stderr, "mime_file(%s)...%s\n", file, f);
    if (!f) {
	return NULL;
    }
    if (rfm_g_file_test(file, G_FILE_TEST_IS_SYMLINK)){
	flags |= MAGIC_SYMLINK;
	gchar *ff = f;
	f = lib_magic (file, flags);
	gchar *gf = g_strconcat(ff, "\n", f, NULL);
	g_free(f);
	g_free(ff);
	return gf;

    }
    return (void *)f;
}

#endif
G_MODULE_EXPORT
const gchar *
g_module_check_init (GModule * module) {
    NOOP("***************g_module_check_init\n");
#ifdef HAVE_LIBMAGIC
    if (g_once_init_enter(&initialized)){
        rfm_mutex_init(magic_mutex);
        cookie = magic_open (MAGIC_NONE);
        magic_load (cookie, NULL);
	g_once_init_leave(&initialized, 1);
    }
#endif
    return NULL;
}

G_MODULE_EXPORT
void
g_module_unload (GModule * module) {
#ifdef HAVE_LIBMAGIC
    initialized=0;
    rfm_mutex_free(magic_mutex);
#endif
}

//////////////////////////////////////////////////////////////////////////////




static
GdkPixbuf *
fix_pixbuf_scale(GdkPixbuf *in_pixbuf){
    if (!in_pixbuf || !GDK_IS_PIXBUF(in_pixbuf)) return NULL;
    GdkPixbuf *out_pixbuf=NULL;
    gint height = gdk_pixbuf_get_height (in_pixbuf);
    gint width = gdk_pixbuf_get_width (in_pixbuf);

    // this is to fix paper size previews for text files and pdfs
    gint size = rfm_get_preview_image_size();
    if((width < height && height != size) || 
	(width >= height && width != size)) {
	//out_pixbuf = rfm_pixbuf_scale_simple (in_pixbuf,  size, GDK_INTERP_HYPER);
	out_pixbuf = rfm_pixbuf_scale_stretch (in_pixbuf,  
		5*size/7,size,
		 GDK_INTERP_HYPER);
	g_object_ref(out_pixbuf);
        g_object_unref(in_pixbuf);
	return out_pixbuf;
    } 
    return in_pixbuf;
}


static GdkPixbuf *
load_preview_pixbuf_from_disk (const gchar * thumbnail) {
    GdkPixbuf *pixbuf = NULL;
    if (rfm_g_file_test(thumbnail, G_FILE_TEST_EXISTS)) {
	pixbuf = rfm_pixbuf_new_from_file (thumbnail, -1, -1);
    }
#if 10
    if (!GDK_IS_PIXBUF(pixbuf)) {
	NOOP ("!GDK_IS_PIXBUF(pixbuf)\n");
	return NULL;
    }
    GdkPixbuf *original=pixbuf;
    pixbuf = fix_pixbuf_scale(original); // this unrefs old and refs new.
    if (original != pixbuf) {
	rfm_pixbuf_save(pixbuf, thumbnail);
    }
#endif
    return pixbuf;
}



//static
//void show_tip (const population_t * population_p);

/////////////////////////////////////////////////////////////////
// forked version of libmagicwand... No need to mutex thread unsafe library here...

void * mime_function (void *p, void *q);
void * mime_file (void *p);
void * mime_encoding (void *p);
void * mime_magic (void *p);

G_MODULE_EXPORT
const gchar *
want_imagemagick_preview (record_entry_t * en) {
    NOOP (stderr, "want_imagemagick_preview\n");
    if(!en) return NULL;


    if(!en->filetype) {
	//en->filetype = mime_file ((void *)(en->path));
	en->filetype = mime_function (en, "mime_file");
    }
    if(!en->mimemagic){
	//en->mimemagic = mime_magic ((void *)(en->path));
	en->mimemagic = mime_function (en, "mime_magic");
	if(!en->mimemagic) en->mimemagic =g_strdup(_("unknown"));
    }
    gchar *mimetype = g_strconcat ( en->mimetype, "/",en->mimemagic, 
	    (en->mimemagic)?"/":NULL, en->filetype, NULL);
    const gchar *convert_type = NULL;

    if(mimetype && strstr (mimetype, "text") && !(strstr (mimetype, "opendocument"))) {     // decode delegate is ghostscript
	if(!en->encoding) {
	    //en->encoding = mime_encoding ((void *)(en->path));
	    en->encoding = mime_function (en, "mime_encoding");
	    if(!en->encoding) en->encoding=g_strdup(_("unknown"));
	}
        NOOP ("mime_encoding= %s\n", en->encoding);
        if (strcmp(en->encoding,"binary")==0) {
            return NULL;
        } 
        convert_type = "TXT";
    } else if(mimetype && strstr (mimetype, "pdf")) {       // decode delegate is ghostscript
        convert_type = "PDF";
    } else if(mimetype && (strstr (mimetype, "postscript") || strstr (mimetype, "eps")) ){
        // decode delegate is ghostscript
        convert_type = "PS";
    }

    g_free (mimetype);

    if(!convert_type)
        return NULL;
    NOOP ("converttype=%s\n", convert_type);

    static gboolean warned = FALSE;

    gboolean gs_warn = strcmp (convert_type, "PS") == 0 || strcmp (convert_type, "PDF") == 0;
    if(gs_warn) {
        gchar *ghostscript = g_find_program_in_path ("gs");
        if(!ghostscript) {
            if(!warned) {
                g_warning
                    ("\n*** Please install ghostscript for ps and pdf previews\n*** Make sure ghostscript fonts are installed too!\n*** You have been warned.\n");
                fflush (NULL);
                warned = TRUE;
            }
            return NULL;
        }
        g_free (ghostscript);
    }
    return convert_type;
}

static void *
gs_wait_f(void *data){
    void **arg = data;
    //GMutex *wait_mutex = arg[0];
    GCond *wait_signal = arg[1];
    pid_t pid = GPOINTER_TO_INT(arg[2]);
    g_free(arg);
    gint status;
    waitpid (pid, &status, WUNTRACED);
    g_cond_signal(wait_signal);
    g_thread_yield();
    rfm_cond_free(wait_signal);
    return NULL;
}

// this function should take a private population_p
static GdkPixbuf *
image_magic_preview_forked (const population_t * population_p, gchar * thumbnail, const gchar * convert_type) {

    gchar *ghostscript = g_find_program_in_path ("gs");
    //gchar *convert = g_find_program_in_path ("convert");
    if(!ghostscript) {
        g_error ("cannot find \"%s\" program in path at rodent_magick.i", ghostscript);
        return NULL;
    }
    int fd = open (population_p->en->path, O_RDONLY);
    if(fd < 0) {
        return 0;
    }
    close (fd);

    population_p = population_p;


// options for pdf/ps and txt previews:
// imagemagick: this depends on ghostscript and gsfonts 
// 
// plain ghostscript is muuch faster than imagemagick: (for ps/pdf)
// gs -dSAFER -dBATCH -dNOPAUSE -sDEVICE=png256 -dFirstPage=1 -dLastPage=1 -dPDFFitPage -r100 -sOutputFile=out.png input.pdf
//
//
// text files...must first convert to ps:
//    groff  -Tps  file  >file.ps
// only problem is the encoding now...
// this works, but how to determine the iso code?
//      iconv -f utf-8 -t iso8859-1 -o file.iso8859 file-utf8
//
// solution: use paps
// paps file >  file.ps
//
// real solution, source paps code to produce preview with cairo/pango

    char *src, *tgt;

    char *arg[13];
    int i = 0;

    //pdf and ps ghostscript conversion
    src = g_strdup (population_p->en->path);
    tgt = g_strdup_printf ("-sOutputFile=%s", thumbnail);
    arg[i++] = ghostscript;
    arg[i++] = "-dSAFER";
    arg[i++] = "-dBATCH";
    arg[i++] = "-dNOPAUSE";
    arg[i++] = "-sPAPERSIZE=letter";    // or a4...
    arg[i++] = "-sDEVICE=png256";
    arg[i++] = "-dFirstPage=1";
    arg[i++] = "-dLastPage=1";
    arg[i++] = "-dPDFFitPage";
    arg[i++] = "-r100";
    arg[i++] = tgt;
    arg[i++] = src;
    arg[i++] = NULL;

    NOOP ("SHOW_TIPx: %s(%s)\n", arg[0], arg[3]);
    GdkPixbuf * retval=NULL;
    // this fork is ok from thread, I guess.
    TRACE( "--> creating thumbnail %s\n", thumbnail);
    pid_t pid = fork ();
    if(!pid) {
	TRACE( "--> child is creating thumbnail %s\n", thumbnail);
        execv (arg[0], arg);
        _exit (123);
    } else {
	// Create wait thread.
	void **arg = (void **)malloc(3*sizeof(void *));
	if (!arg) g_error("malloc: %s\n", strerror(errno));
	GCond *wait_signal;
	GMutex *wait_mutex;
	rfm_mutex_init(wait_mutex);
	rfm_cond_init(wait_signal);
	arg[0] = wait_mutex;
	arg[1] = wait_signal;
	arg[2] = GINT_TO_POINTER(pid);

	g_mutex_lock(wait_mutex);
	rfm_thread_create("ghostscript wait thread", gs_wait_f, arg, FALSE);
	if (!rfm_cond_timed_wait(wait_signal, wait_mutex, 4)){
	    DBG("Aborting runaway ghostscript preview for %s (pid %d)\n",
		    src, (int)pid);
	    kill(pid, SIGKILL);
	} else {
	    // this function refs retval
	    retval = load_preview_pixbuf_from_disk (thumbnail); // refs
	}
	g_mutex_unlock(wait_mutex);
	rfm_mutex_free(wait_mutex);
        NOOP ("SHOW_TIPx: preview created by convert\n");
    }
    g_free (ghostscript);       //arg[0]
    //g_free (convert);           //arg[0]
    g_free (src);
    g_free (tgt);
    return retval; 
}

///////  end forked stuff /////
//////////////////////////////////////////////////
//
// These routines are heavily modified from the "paps" source
// GPL by Dov Grobgeld <dov.grobgeld@gmail.com>

#define BUFSIZE (4096)
#define DEFAULT_FONT_FAMILY	"Sans"
//#define DEFAULT_FONT_FAMILY   "Arial"
#define DEFAULT_FONT_SIZE	"12"
#define MAKE_FONT_NAME(f,s)	f " " s

#include <locale.h>
#include <pango/pango.h>

typedef struct {
    cairo_t *cr;
    cairo_surface_t *surface;
    PangoContext *context;
    int column_width;
    int column_height;
    int top_margin;
    int bottom_margin;
    int left_margin;
    int right_margin;
    int page_width;
    int page_height;
    PangoDirection pango_dir;
} page_layout_t;

typedef enum {
    PAPER_TYPE_A4 = 0,
    PAPER_TYPE_US_LETTER = 1,
    PAPER_TYPE_US_LEGAL = 2
} paper_type_t;

typedef struct {
    int width;
    int height;
} paper_size_t;

static const paper_size_t paper_sizes[] = {
    {595, 842},                 /* A4 */
    {612, 792},                 /* US letter */
    {612, 1008}                 /* US legal */
};

typedef struct linelink_t {
    PangoLayoutLine *pango_line;
    PangoRectangle logical_rect;
    PangoRectangle ink_rect;
    int formfeed;
} linelink_t;

/* Structure representing a paragraph
 */
typedef struct paragraph_t {
    char *text;
    int length;
    int height;                 /* Height, in pixels */
    int formfeed;
    PangoLayout *layout;
} paragraph_t;


static PangoLanguage *
get_language (void) {
    PangoLanguage *retval;
    gchar *lang = g_strdup (setlocale (LC_CTYPE, NULL));
    gchar *p;

    p = strchr (lang, '.');
    if(p)
        *p = 0;
    p = strchr (lang, '@');
    if(p)
        *p = 0;

    retval = pango_language_from_string (lang);
    g_free (lang);

    return retval;
}

static PangoDirection
get_direction (const gchar * str) {
    PangoDirection direction;
    const gchar *p;
    if(g_utf8_validate (str, -1, NULL)) {
        for(p = str; p != NULL && *p; p = g_utf8_find_next_char (p, NULL)) {
            gunichar ch = g_utf8_get_char (p);
            direction = pango_unichar_direction (ch);
            if(direction == PANGO_DIRECTION_NEUTRAL)
                continue;
            NOOP ("direction found\n");
            return direction;
        }
    }
    // default:
    NOOP ("PANGO_DIRECTION_LTR\n");
    return PANGO_DIRECTION_LTR;
}

static gint
x_strcmp(gconstpointer a, gconstpointer b){
    return strcmp((gchar *)a, (gchar *)b);
}

static gchar *
directory_text(const gchar *path){
    gint count=0;
    DIR *directory = opendir(path);
    if (!directory) {
	NOOP("directory_text(): Cannot open %s\n", path);
	return g_strdup_printf("%s: %s\n", path, strerror(errno));
    }
// http://womble.decadent.org.uk/readdir_r-advisory.html

#if defined(HAVE_FPATHCONF) && defined(HAVE_DIRFD)
    size_t size = offsetof(struct dirent, d_name) + 
	fpathconf(dirfd(directory), _PC_NAME_MAX) + 1;
#else
    size_t size = offsetof(struct dirent, d_name) +
	pathconf(path, _PC_NAME_MAX) + 1;
#endif
    struct dirent *d;
    gchar *utf = rfm_utf_string(path);
    gchar *dir_text = g_strdup_printf("%s:\n", utf);
    g_free(utf);
    struct dirent *buffer = (struct dirent *)malloc(size);
    if (!buffer) g_error("malloc: %s\n", strerror(errno));

    GSList *list = NULL;
    gint error;
    while ((error = readdir_r(directory, buffer, &d)) == 0 && d != NULL){
	utf = rfm_utf_string(d->d_name);
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	const gchar *string=_("unknown");
	switch (d->d_type){
	    case DT_BLK:
		string = _("Block device");
		break;
	    case DT_CHR:
		string = _("Character device");
		break;
	    case DT_DIR:
		string = _("Directory");
		break;
	    case DT_FIFO:
		string = _("FIFO");
		break;
	    case DT_LNK:
		string = _("Symbolic Link");
		break;
	    case DT_REG:
		string = _("Regular file");
		break;
	    case DT_SOCK:
		string = _("Socket");
		break;
	    default :
		break;
	}
	gchar *line = g_strdup_printf("[%s]\t%s", string, utf);
#else
	gchar *line = g_strdup_printf("%s", d->d_name);
#endif
	g_free(utf);
	list = g_slist_prepend(list, line);
	if (count++ >= 100) break;
    }
    closedir (directory);
    g_free(buffer);
    if (error) {
	utf = rfm_utf_string(strerror(error));
	gchar *g = g_strdup_printf("%s\t%s\n", dir_text, utf);
	g_free(utf);
	g_free(dir_text);
	dir_text = g;
    } else {
	list = g_slist_sort(list, x_strcmp);
	GSList *tmp = list;
	for (;tmp && tmp->data; tmp = tmp->next){
	    gchar *g = g_strdup_printf("%s\t%s\n", dir_text, (gchar *)tmp->data);
	    g_free(dir_text);
	    dir_text = g;
	    g_free(tmp->data);
	}
    }
    g_slist_free(list);

    return dir_text;
}

static gchar *
read_file (record_entry_t *en) {
    gchar *path = g_strdup(en->path);
    gchar *encoding;

    if (!IS_LOCAL_TYPE(en->type) && 
	    !rfm_g_file_test_with_wait(path, G_FILE_TEST_EXISTS)){
        return NULL;
    }
    
    gchar *buffer = (gchar *)malloc (BUFSIZE);
    if (!buffer) g_error("malloc: %s", strerror(errno));
    memset (buffer, 0, BUFSIZE);

    if (g_file_test(path, G_FILE_TEST_IS_DIR)){
	gchar *txt = directory_text(path);
	strncpy(buffer, txt, BUFSIZE-1);
	g_free(txt);
	encoding = g_strdup("UTF-8");
	goto finishup;
    }
    gint fd = open (path, O_RDONLY);
    if(fd < 0) {
        DBG ("open(%s): %s\n", path, strerror (errno));
	g_free(path);
        g_free (buffer);
        return NULL;
    }
    // string terminated with memset() above.
    // coverity[string_null_argument : FALSE]
    gint bytes = read (fd, buffer, BUFSIZE - 1);
    close (fd);
    if(bytes < 0) {
        DBG ("%s: Error reading file %s (%s).\n", g_get_prgname (), path, strerror (errno));
	g_free(path);
        g_free (buffer);
        return NULL;
    }
    gint i;
    for (i=0; i<BUFSIZE-2; i++) {
	if (buffer[i] < 32 ) {
	    if (buffer[i] == 9) continue;
	    if (buffer[i] == 10) continue;
	    if (buffer[i] == 0) break;
	    else buffer[i] = '.';
	}
    }
    encoding = MIME_encoding ((void *)path);

    if(strrchr (buffer, '\n')) {
        *(strrchr (buffer, '\n') + 1) = 0;
    } else {
	buffer[BUFSIZE - 1] = 0;
    }

finishup:;

    gsize bytes_read,
      bytes_written;
    GError *error = NULL;
    if(encoding && (!strstr (encoding, "utf-8") || !strstr (encoding, "UTF-8"))) {
        gchar *obuffer = g_convert_with_fallback (buffer, -1, "UTF-8", encoding,
                                                 NULL, &bytes_read, &bytes_written, &error);
        if(error) {
            NOOP ("g_convert_with_fallback(%s): %s\n", path, error->message);
            g_error_free (error);
	    error=NULL;
	    obuffer = g_convert_with_fallback (buffer, -1, "UTF-8", "iso8859-15",
                                                 NULL, &bytes_read, &bytes_written, &error);
	    if (error) {
		NOOP ("b.g_convert_with_fallback(%s): %s\n", path, error->message);
		g_error_free (error);
	    }
        }
        NOOP ("convert successful from %s to UTF-8\n", encoding);
        g_free (buffer);
        buffer = obuffer;
    }

    g_free(path);
    g_free (encoding);
    return buffer;
}

int
output_page (GList * pango_lines, page_layout_t * page_layout) {
    int column_pos = 0;
    int page_idx = 1;

    int pango_column_height = page_layout->page_height - page_layout->top_margin - page_layout->bottom_margin;

    while(pango_lines && pango_lines->data) {
        linelink_t *line_link = pango_lines->data;
        PangoLayoutLine *line = line_link->pango_line;
	if (!line) continue;
        PangoRectangle logical_rect;
        PangoRectangle ink_rect;
        pango_layout_line_get_extents (line, &ink_rect, &logical_rect);
        /* Assume square aspect ratio for now */
        column_pos += (line_link->logical_rect.height / PANGO_SCALE);
        double y_pos = column_pos + page_layout->top_margin;
        if(y_pos > pango_column_height)
            break;              //just first page
        double x_pos = page_layout->left_margin;
        /* Do RTL column layout for RTL direction */
        if(page_layout->pango_dir == PANGO_DIRECTION_RTL) {
            x_pos = page_layout->page_width - page_layout->right_margin;
        }
        NOOP ("gdk_draw_layout_line: %lf, %lf\n", x_pos, y_pos);
        cairo_move_to (page_layout->cr, x_pos, y_pos);
        pango_cairo_show_layout_line (page_layout->cr, line);
        pango_lines = pango_lines->next;
    }
    return page_idx;
}

/* Split a list of paragraphs into a list of lines.
 */
GList *
split_paragraphs_into_lines (page_layout_t * page_layout, GList * paragraphs) {
    GList *line_list = NULL;
    int max_height = 0;
    /* Read the file */

    /* Now split all the paragraphs into lines */
    GList *par_list;

    par_list = paragraphs;
    while(par_list && par_list->data) {
        int para_num_lines;
        linelink_t *line_link;
        paragraph_t *para = par_list->data;

	para_num_lines = pango_layout_get_line_count (para->layout);
        NOOP ("para_num_lines = %d\n", para_num_lines);

        int i;
        for(i = 0; i < para_num_lines; i++) {
            PangoRectangle logical_rect;
            PangoRectangle ink_rect;
            PangoLayoutLine *pango_line = pango_layout_get_line_readonly (para->layout, i);
	    if (!pango_line) break;

            line_link = g_new (linelink_t, 1);
            line_link->formfeed = 0;
            line_link->pango_line = pango_line;
            pango_layout_line_get_extents (line_link->pango_line, &ink_rect, &logical_rect);
            line_link->logical_rect = logical_rect;
            if(para->formfeed && i == (para_num_lines - 1))
                line_link->formfeed = 1;
            line_link->ink_rect = ink_rect;
            line_list = g_list_prepend (line_list, line_link);
            if(logical_rect.height > max_height)
                max_height = logical_rect.height;
        }

        par_list = par_list->next;
    }

    return g_list_reverse (line_list);

}

/* Take a UTF8 string and break it into paragraphs on \n characters
 */
static GList *
split_text_into_paragraphs (page_layout_t * page_layout, char *text) {
    char *p = text;
    char *next;
    gunichar wc;
    GList *result = NULL;
    char *last_para = text;
    int width = page_layout->page_width - page_layout->right_margin - page_layout->left_margin;

    while(p != NULL && *p) {
        wc = g_utf8_get_char (p);
        next = g_utf8_next_char (p);
        if(wc == (gunichar) - 1) {
            DBG ("%s: Invalid character in input\n", g_get_prgname ());
            wc = 0;
        }
        if(!*p || !wc || wc == '\n' || wc == '\f') {
            paragraph_t *para = g_new (paragraph_t, 1);
            para->text = last_para;
            para->length = p - last_para;
            para->layout = pango_layout_new (page_layout->context);
            pango_layout_set_text (para->layout, para->text, para->length);
            pango_layout_set_justify (para->layout, FALSE);
            pango_layout_set_alignment (para->layout,
                                        page_layout->pango_dir == PANGO_DIRECTION_LTR ? PANGO_ALIGN_LEFT : PANGO_ALIGN_RIGHT);
            pango_layout_set_wrap (para->layout, PANGO_WRAP_WORD_CHAR);

            pango_layout_set_width (para->layout, width * PANGO_SCALE);
            para->height = 0;

            last_para = next;

            if(wc == '\f')
                para->formfeed = 1;
            else
                para->formfeed = 0;

            result = g_list_append (result, para);
        }
        if(!wc)                 /* incomplete character at end */
            break;
        p = next;
    }

    return (result);
}


typedef struct tarball_t {
    const gchar *cmd;
    const gchar *options;
    const gchar *mimetype;
    gboolean status;
} tarball_t;

static tarball_t tarball_v[] = {
    {"tar", "-tzf", "application/x-compressed-tar", FALSE},
    {"tar", "-tjf", "application/x-bzip-compressed-tar", FALSE},
    {"tar", "--use-compress-program -tf", "application/x-lzma-compressed-tar", FALSE},
    {"tar", "-tJf", "application/x-tarz", FALSE},
    {"tar", "-tf", "application/x-tar", FALSE},
    {"rpm", "-qip", "application/x-rpm", FALSE},
    {"rpm", "-qip", "application/x-redhat-package-manager", FALSE},
    {"dpkg", "--info", "application/x-deb", FALSE},
    {"dpkg", "--info", "application/x-debian-package", FALSE},
    {NULL, NULL, NULL}
};

gboolean is_tarball(record_entry_t *en){
    if (en->st && en->st->st_size > 5000000) return -1;
    const gchar *mimetype = en->mimetype;
    if (!mimetype) return -1;
    static gsize initialized = 0;
    tarball_t *tarball_p;
    if (g_once_init_enter (&initialized)) {
	tarball_p = tarball_v;
	for (;tarball_p && tarball_p->cmd; tarball_p++){
	    gchar *g = g_find_program_in_path(tarball_p->cmd);
	    if (g) {
		tarball_p->status = TRUE;
		g_free(g);
	    }
	}
	g_once_init_leave (&initialized, 1);
    }

    tarball_p = tarball_v;
    for (;tarball_p && tarball_p->cmd; tarball_p++){
	if (!tarball_p->status) continue;
	if (strcmp(mimetype, tarball_p->mimetype)==0) return TRUE;
    }
    return FALSE;
}

static gchar *
tarball_text(const gchar *path, const gchar *mimetype){
    gchar *cmd=NULL;
    tarball_t *tarball_p = tarball_v;
    for (;tarball_p && tarball_p->cmd; tarball_p++){
	if (!tarball_p->status) continue;
	if (strcmp(mimetype, tarball_p->mimetype)==0) {
	    cmd = g_strdup_printf("%s %s \"%s\"",
		    tarball_p->cmd, tarball_p->options, path);
	    NOOP(stderr, "CMD = %s\n", cmd);
	    break;
	}
    }
    if (!cmd) return g_strdup(_("File format not recognized"));

    gint count = 0;
    gchar *t = g_strdup_printf(_("Contents of %s"), path);
    gchar *text = g_strconcat(t, "\n", NULL);
    g_free(t);

    FILE *p = popen (cmd, "r");
    if(p) {
	gchar line[1024];
	memset(line, 0, 1024);

	while(fgets (line, 1023, p) && ! feof(p)) {
	    if (count++ >= 50) break;
	    gchar *g = g_strdup_printf("%s\t%s", text, line);
	    g_free(text);
	    text = g;
	}
	pclose (p);
    }
    g_free(cmd);
    return text;
}


// this function take a private population_p
static void *
text_preview_f (void *data){
    void **arg = data;
    gchar * text = arg[0];
    gchar * thumbnail = arg[1];
    page_layout_t page_layout;

    NOOP ("PAP: file read complete:\n%s\n", "");
    NOOP ("PAP: file read complete:\n%s\n", text);

    /* Page layout, either a4 or letter or legal. */
    page_layout.page_width = paper_sizes[PAPER_TYPE_US_LETTER].width;
    page_layout.page_height = paper_sizes[PAPER_TYPE_US_LETTER].height;
    page_layout.top_margin = 36;
    page_layout.bottom_margin = 36;
    page_layout.right_margin = 36;
    page_layout.left_margin = 36;

    // determine pango_dir 
    page_layout.pango_dir = get_direction (text);
    page_layout.column_height = page_layout.page_height - page_layout.top_margin - page_layout.bottom_margin;
    page_layout.column_width = page_layout.page_width - page_layout.left_margin - page_layout.right_margin;

    // create cairo drawable surface and context  
    page_layout.surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, page_layout.page_width, page_layout.page_height);
    //CAIRO_FORMAT_A1 CAIRO_FORMAT_A8
    page_layout.cr = cairo_create (page_layout.surface);
    if(cairo_surface_status (page_layout.surface) != CAIRO_STATUS_SUCCESS) {
        g_error ("cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS");
    }
    // clear cairo surface
    //
    cairo_set_source_rgb (page_layout.cr, 1.0, 1.0, 1.0);
    cairo_paint (page_layout.cr);
    // create pango context
    page_layout.context = pango_cairo_create_context (page_layout.cr);
    // Setup pango context
    pango_cairo_context_set_resolution (page_layout.context, -1);
    pango_context_set_language (page_layout.context, get_language ());
    pango_context_set_base_dir (page_layout.context, page_layout.pango_dir);

    // font description 
    gchar *font = MAKE_FONT_NAME (DEFAULT_FONT_FAMILY, DEFAULT_FONT_SIZE);
    PangoFontDescription *font_description = pango_font_description_from_string (font);
    if((pango_font_description_get_set_fields (font_description)
        & PANGO_FONT_MASK_FAMILY) == 0) {
        pango_font_description_set_family (font_description, DEFAULT_FONT_FAMILY);
    }
    if((pango_font_description_get_set_fields (font_description)
        & PANGO_FONT_MASK_SIZE) == 0) {
        pango_font_description_set_size (font_description, atoi (DEFAULT_FONT_SIZE) * PANGO_SCALE);
    }
    pango_context_set_font_description (page_layout.context, font_description);
    pango_font_description_free (font_description);

    NOOP ("pango setup complete\n");
    GList *paragraphs = split_text_into_paragraphs (&page_layout, text);

    NOOP ("pango split_text_into_paragraphs complete--> %d \n", g_list_length (paragraphs));
    GList *pango_lines = split_paragraphs_into_lines (&page_layout, paragraphs);
    NOOP ("pango split_paragraphs_into_lines complete--> %d\n", g_list_length (pango_lines));

    // cairo setup
    cairo_new_path (page_layout.cr);
    cairo_set_line_width (page_layout.cr, 0.5);
    cairo_set_source_rgb (page_layout.cr, 0.0, 0.0, 0.0);

    //output layouts to page ()
    output_page (pango_lines, &page_layout);
    // tmp: write png of pixbuf
    NOOP("// numpages=%d\n", num_pages);
    cairo_destroy (page_layout.cr);
    NOOP ("// write thumbnail\n");
    if(cairo_surface_write_to_png (page_layout.surface, thumbnail) != CAIRO_STATUS_SUCCESS) {
        DBG ("cairo_surface_write_to_png(surface,) != CAIRO_STATUS_SUCCESS");
    }
    cairo_surface_destroy (page_layout.surface);
    // destroy GLists
    GList *tmp;
    for(tmp = pango_lines; tmp && tmp->data; tmp = tmp->next) {
        linelink_t *line = tmp->data;
        if(G_IS_OBJECT (line->pango_line)) g_object_unref (line->pango_line);
        g_free (line);
    }
    for(tmp = paragraphs; tmp && tmp->data; tmp = tmp->next) {
        paragraph_t *para = tmp->data;
        if(G_IS_OBJECT (para->layout)) {
            g_object_unref (para->layout);
        }
        g_free (para);
    }
    if(G_IS_OBJECT (page_layout.context)) g_object_unref (page_layout.context);

    g_list_free (paragraphs);
    g_list_free (pango_lines);
 
    // pixbuf generated and reffed in routine:
    NOOP("load_preview_pixbuf_from_disk %s\n", thumbnail);
    GdkPixbuf *pixbuf = load_preview_pixbuf_from_disk (thumbnail); // refs
    return pixbuf;
}

static GdkPixbuf *
text_preview (const population_t * population_p, gchar * thumbnail, view_t * view_p) {
    if(!population_p || !population_p->en || !population_p->en->path)
        return NULL;

    if (population_p->en->mimetype && strstr(population_p->en->mimetype,"text")){
	// OK
    } 

    // Read a cache page worth of text and convert to utf-8 
    // Tarballs...
    
    gint Tarballs = is_tarball(population_p->en);
    if (Tarballs < 0) return NULL;

    gchar *text;
    if (population_p->en->st && population_p->en->st->st_size == 0){
        TRACE( "oooooo   size for %s == 0\n", population_p->en->path);
	text = g_strdup_printf("*****  %s  *****", _("Empty file"));
    } else  if (Tarballs) {
	text =tarball_text(population_p->en->path, population_p->en->mimetype);
    } else {
	text = read_file (population_p->en);
    }
    if(!text) return NULL;
    if (!strchr(text, '\n')){
	gchar *t = g_strconcat(text,"\n",NULL);
	g_free(text);
	text = t;
    }
    void *arg[]={text, thumbnail};
    GdkPixbuf *pixbuf = rfm_context_function(text_preview_f, arg);
    g_free(text);
    return pixbuf;
}

static
void *
mime_preview_at_size(const population_t * population_p) {
    gint preview_size = rfm_get_preview_image_size();
    TRACE( "oooooo   mime_preview\n");
    if(!population_p->en || !population_p->en->st) {
        NOOP ("SHOW_TIPx: !population_p->en || !population_p->en->st\n");
        return NULL;
    }

    // Check if in pixbuf hash. If so, return with the hashed pixbuf.
    // Note that if the thumbnail is out of date, Null will be returned 
    // from the pixbuf hash.
    GdkPixbuf *pixbuf = (GdkPixbuf *) rfm_find_in_pixbuf_hash(population_p->en->path, preview_size); // refs
    if(pixbuf) {
        TRACE( "oooooo   pixbuf located in hash table.\n");
        return pixbuf;
    }

    gchar *thumbnail = rfm_get_thumbnail_path (population_p->en->path, preview_size);
        TRACE( "oooooo   thumbnail path=%s\n", thumbnail);
    // Empty files hack
    if(population_p->en->st->st_size == 0) {
	pixbuf = text_preview (population_p, thumbnail, population_p->view_p); //refs
	// Replace newly created pixbuf in pixbuf hash.
	// Reference will now belong to the hash table.
#if 0
	g_object_unref(pixbuf);
#else
	rfm_put_in_pixbuf_hash(population_p->en->path, preview_size, pixbuf);
#endif
	g_free(thumbnail);
        return pixbuf;
    }
    // So it is not already loaded (rodent_preview_if_loaded()
    // should be called beforehand.
    // Is the pixbuf in the thumbnail cache?

    // Thumbnail should always resolve to a local and absolute path.
    if(thumbnail && g_file_test (thumbnail, G_FILE_TEST_EXISTS)) {
        TRACE( "oooooo   thumbnail %s exists!\n", thumbnail);
        struct stat st;
        if(stat (thumbnail, &st) < 0){
            DBG("stat(%s): %s", thumbnail, strerror (errno));
            return NULL;
        }
        if(st.st_mtime >= population_p->en->st->st_mtime) {
            TRACE( "oooooo: %s: thumbnail %s is up to date\n",
                    population_p->en->path, thumbnail);
            // pixbuf generated and reffed in routine:
            pixbuf = load_preview_pixbuf_from_disk (thumbnail); // refs
            if(pixbuf) {
                g_free (thumbnail);
		if (!GDK_IS_PIXBUF(pixbuf)) return NULL;
		
		// pixbuf has ref==1 
		// Replace newly created pixbuf in pixbuf hash.
		// Reference will now belong to the hash table.
#if 0
	g_object_unref(pixbuf);
#else
		rfm_put_in_pixbuf_hash(population_p->en->path, preview_size, pixbuf);
#endif
                NOOP ("SHOW_TIPx: preview loaded from thumbnail file\n");
                return pixbuf;
            }
        }
    }


    // So it is not in thumbnail cache. So it will have to be regenerated
    // from scratch.
    
    TRACE( "oooooo: %s: thumbnail %s must be generated\n",
                    population_p->en->path, thumbnail);
    

    // First we shall try internal gtk image manipulation:
    if(rfm_entry_is_image (population_p->en)) {
	// This function will automatically put the pixbuf into the pixbuf hash:
        pixbuf = rfm_get_pixbuf (population_p->en->path, preview_size); //refs
        NOOP ("SHOW_TIPx: preview created by gtk\n");
        g_free (thumbnail);
        if(pixbuf)  return pixbuf;
        else return NULL;
    }

    // So it is not an image type
    // Do we have zip previews plugin?
    if (rfm_void(RFM_MODULE_DIR, "mimezip", "module_active")){
	NOOP(stderr, "zip test\n");

	if(!population_p->en->filetype) {
	    population_p->en->filetype = mime_function ((void *)(population_p->en), 
		    "mime_file");
	    //population_p->en->filetype = mime_file ((void *)(population_p->en->path));
	}
	gboolean OpenDocument = (population_p->en->filetype && strstr (population_p->en->filetype, "OpenDocument") != NULL);
	gboolean plainzip = (population_p->en->filetype && strstr (population_p->en->filetype, "Zip archive") != NULL);
	gboolean plainrar = (population_p->en->filetype && strstr (population_p->en->filetype, "RAR archive") != NULL);
	if(OpenDocument || plainzip || plainrar) {
	    const gchar *function=NULL;
	    if (OpenDocument) function = "get_zip_preview";
	    else if (plainzip) function = "get_zip_image";
	    else if (plainrar) function = "get_rar_image";
	    else g_error("bummer at mime_preview()\n");
	    
	    pixbuf = rfm_natural(RFM_MODULE_DIR, "mimezip", population_p->en->path,	function); //refs
	    if (pixbuf && GDK_IS_PIXBUF(pixbuf)) {
		// Mimezip function will ref to keep things standarized.
		// fix_pixbuf_scale unrefs and refs as needed.
		GdkPixbuf *old_pixbuf = pixbuf;
		pixbuf = fix_pixbuf_scale(old_pixbuf);
	       	// This may or may not be the same pixbuf.
		if (pixbuf != old_pixbuf) {
		    rfm_pixbuf_save(pixbuf, thumbnail);
		}
		// Replace newly created pixbuf in pixbuf hash.
		// Reference will now belong to the hash table.
		rfm_put_in_pixbuf_hash(population_p->en->path, preview_size, pixbuf);
	    } else {
		DBG ("Could not retrieve thumbnail from zipped %s\n", 
			population_p->en->path);
	    }
	    g_free (thumbnail);
	    if (!GDK_IS_PIXBUF(pixbuf)) return NULL;
	    
	    return pixbuf;
	}
    
    } else {
	NOOP(stderr, "mimezip not active\n");
    }


    // Ok, that didn't work either. Is it ghostscript (ps or pdf) or text?
    // this will construct the thumbnail in disk to load on next mousemove.
    const gchar *convert_type = want_imagemagick_preview (population_p->en);

    if(!convert_type) {
        NOOP ("SHOW_TIPx: convert type=%s\n",convert_type);
	convert_type = "TXT";
        //g_free (thumbnail);
        //return NULL;
    }

    // pdf forks to ghostscript to create thumbnail file
    pixbuf = NULL;
    if(strcmp (convert_type, "PDF") == 0 || strcmp (convert_type, "PDF") == 0) {
            // pixbuf generated and reffed in routine:
        pixbuf = image_magic_preview_forked (population_p, thumbnail, convert_type);// refs
    }
    // text uses pango cairo to create thumbnail file
    // default to text preview (even of binaries...)
    else // if(strcmp (convert_type, "TXT") == 0) 
    {
        view_t *view_p = population_p->view_p;
            // pixbuf generated and reffed in routine:
        pixbuf = text_preview (population_p, thumbnail, view_p); // refs
    }
    g_free (thumbnail);
    if (!pixbuf || !GDK_IS_PIXBUF(pixbuf)) return NULL;
    // Replace newly created pixbuf in pixbuf hash.
    // Reference will now belong to the hash table.
#if 0
	g_object_unref(pixbuf);
#else
    rfm_put_in_pixbuf_hash(population_p->en->path, preview_size, pixbuf);
#endif
    return pixbuf;
}


G_MODULE_EXPORT
GdkPixbuf *
mime_preview (const population_t * population_p) {
    GdkPixbuf *pixbuf = mime_preview_at_size(population_p);
    return pixbuf;
}


//////////////////////////////////////////////////

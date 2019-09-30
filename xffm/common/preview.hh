#ifndef PREVIEW_HH
#define PREVIEW_HH

#define PREVIEW_IMAGE_SIZE 400
#define BUFSIZE (4096)
#define DEFAULT_FONT_FAMILY	"Sans"
//#define DEFAULT_FONT_FAMILY   "Arial"
#define DEFAULT_FONT_SIZE	"12"
#define MAKE_FONT_NAME(f,s)	f " " s

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


namespace xf {

template <class Type>
class Preview {

public:
    static GdkPixbuf *
    previewDefault(const gchar *filePath, const gchar *mimetype, struct stat *st_p)
    {
	return previewDefault(filePath, mimetype, st_p, 3*PREVIEW_IMAGE_SIZE/4);
    }
    static GdkPixbuf *
    previewDefault(const gchar *filePath, 
	    const gchar *mimetype, struct stat *st_p,
	    gint size)
    {
	
	if(!filePath || !st_p) {
	    ERROR ("previewAtSize: !file_path || !st_p\n");
	    return NULL;
	}
	// First of all, are we dealing with an empty file?
	if(st_p->st_size == 0) {
	    // Yeah, so we use the common empty file name.
	    filePath = "empty-file";
	}

	auto pixbuf = loadFromHash(filePath);
	if (pixbuf) {
	    TRACE("%s loaded from hash...\n", filePath);
	    return pixbuf;
	}

	pixbuf = loadFromThumbnails(filePath, st_p);
	if (pixbuf) {
	    TRACE("%s loaded from thumbnails...\n", filePath);
	    return pixbuf;
	}

	// So it is not in thumbnail cache. So it will have to be regenerated
	// from scratch.
	// text file preview creations
	if (!mimetype){
	    ERROR("previewDefault(): Mimetype should be defined beforehand\n");
	    return NULL;
	}
	gboolean textType =(
		strstr(mimetype, "text")
		|| strstr(mimetype, "shellscript")
		|| strcmp(filePath, "empty-file")==0
		|| g_file_test(filePath, G_FILE_TEST_IS_DIR)
		);

	if (textType){
	    pixbuf = textPreview (filePath); 
	    if (pixbuf) {
		PixbufHash<Type>::put_in_pixbuf_hash(filePath, size, pixbuf);
		// FIXME: save in thumbnails
		
		return pixbuf;
	    }
	    ERROR("previewAtSize: text preview of some sort should be available here\n");
	    return Pixbuf<Type>::get_pixbuf("text-x-generic", -size);
	}
	    
	// pdf previews...
	gboolean useGhostScript = (strstr (mimetype, "pdf")
		    || strstr (mimetype, "postscript") 
		    || strstr (mimetype, "eps")
		    );
        if(useGhostScript) {  
	    // decode delegate is ghostscript
	    pixbuf = gsPreview (filePath);// refs
	    if (pixbuf) return pixbuf;
	    else Pixbuf<Type>::get_pixbuf("image-x-generic-template", -size);
	}
	// image previews...
	if (strstr (mimetype, "image")) {   
	    pixbuf = Icons<Type>::pixbuf_new_from_file(filePath, size, -1);
	    if (pixbuf) return pixbuf; 
	    else return Pixbuf<Type>::get_pixbuf("image-x-generic", -size);

	}
	if (strstr (mimetype, "video")) 
		return Pixbuf<Type>::get_pixbuf("video-x-generic", -size);
	if (strstr (mimetype, "audio")) 
		return Pixbuf<Type>::get_pixbuf("audio-x-generic", -size);

	return Pixbuf<Type>::get_pixbuf("preferences-system", -size);
	//return Pixbuf<Type>::get_pixbuf("image-missing", PREVIEW_IMAGE_SIZE);
    }


private:
static GdkPixbuf *
gsPreview (const gchar *path) {
    gchar *ghostscript = g_find_program_in_path ("gs");
    static gboolean warned = FALSE;
    if(!ghostscript) {
	if(!warned) {
	    g_warning
		("\n*** Please install ghostscript for ps and pdf previews\n*** Make sure ghostscript fonts are installed too!\n*** You have been warned.\n");
	    fflush (NULL);
	    warned = TRUE;
	}
	return NULL;
    }

    gint fd = open (path, O_RDONLY);
    if(fd < 0) {
        return NULL;
    }
    close (fd);

// options for pdf/ps previews:
// 
// plain ghostscript is muuch faster than imagemagick: (for ps/pdf)
// gs -dSAFER -dBATCH -dNOPAUSE -sDEVICE=png256 -dFirstPage=1 -dLastPage=1 -dPDFFitPage -r100 -sOutputFile=out.png input.pdf
//
//

    gchar *src, *tgt;
    gchar *arg[13];
    gint i = 0;
    auto thumbnail = PixbufHash<Type>::get_thumbnail_path (path, PREVIEW_IMAGE_SIZE);

    //pdf and ps ghostscript conversion
    src = g_strdup (path);
    tgt = g_strdup_printf ("-sOutputFile=%s", thumbnail);
    arg[i++] = ghostscript;
    arg[i++] = (gchar *)"-dSAFER";
    arg[i++] = (gchar *)"-dBATCH";
    arg[i++] = (gchar *)"-dNOPAUSE";
    arg[i++] = (gchar *)"-sPAPERSIZE=letter";    // or a4...
    arg[i++] = (gchar *)"-sDEVICE=png256";
    arg[i++] = (gchar *)"-dFirstPage=1";
    arg[i++] = (gchar *)"-dLastPage=1";
    arg[i++] = (gchar *)"-dPDFFitPage";
    arg[i++] = (gchar *)"-r100";
    arg[i++] = tgt;
    arg[i++] = src;
    arg[i++] = NULL;

    TRACE ("SHOW_TIPx: %s(%s)\n", arg[0], arg[3]);
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
	void **arg = (void **)calloc(3, sizeof(void *));
	if (!arg) g_error("gsPreview(): malloc: %s\n", strerror(errno));
	pthread_mutex_t waitMutex = PTHREAD_MUTEX_INITIALIZER;
	//GMutex *wait_mutex;

	//GCond *wait_signal;
	pthread_cond_t waitSignal = PTHREAD_COND_INITIALIZER;


	//rfm_mutex_init(wait_mutex);
	//rfm_cond_init(wait_signal);
	
	arg[0] = &waitMutex;
	arg[1] = &waitSignal;
	arg[2] = GINT_TO_POINTER(pid);

	pthread_mutex_lock(&waitMutex);
	//g_mutex_lock(wait_mutex);
	pthread_t thread;
	pthread_create(&thread, NULL, gs_wait_f, arg);
	const struct timespec abstime={
	    time(NULL) + 4, 0
	};
	//if (!pthread_cond_timedwait(&waitSignal, &waitMutex, &abstime)){
	if (pthread_cond_wait(&waitSignal, &waitMutex) != 0){
	//if (!rfm_cond_timed_wait(wait_signal, wait_mutex, 4)){
	    ERROR("Aborting runaway ghostscript preview for %s (pid %d)\n",
		    src, (int)pid);
	    kill(pid, SIGKILL);
	} else {
	    TRACE("condition wait complete for file %s\n", thumbnail);
	    // this function refs retval
	    retval = loadFromThumbnails(path, NULL);
	    //retval = Pixbuf<Type>::pixbuf_from_file (thumbnail, 3*PREVIEW_IMAGE_SIZE/4, PREVIEW_IMAGE_SIZE);
	    //retval = load_preview_pixbuf_from_disk (thumbnail); // refs
	}
	pthread_mutex_unlock(&waitMutex);
	//g_mutex_unlock(wait_mutex);
	pthread_mutex_destroy(&waitMutex);
	//rfm_mutex_free(wait_mutex);
        TRACE ("SHOW_TIPx: preview created by convert\n");
    }
    g_free (ghostscript);       //arg[0]
    g_free (src);
    g_free (tgt);
    return retval; 
}


static void *
gs_wait_f(void *data){
    auto arg = (void **)data;
    auto waitMutex = (pthread_mutex_t *)arg[0];
    auto waitSignal = (pthread_cond_t *)arg[1];
    pid_t pid = GPOINTER_TO_INT(arg[2]);
    g_free(arg);
    gint status;
    TRACE("waiting for %d\n", pid);
    waitpid (pid, &status, WUNTRACED);
    TRACE("wait for %d complete\n", pid);
    //pthread_mutex_unlock(waitMutex); 
    pthread_cond_signal(waitSignal); 
    return NULL;
}


   
    static gint
    x_strcmp(gconstpointer a, gconstpointer b){
	return strcmp((gchar *)a, (gchar *)b);
    }
 
    static gint
    output_page (GList * pango_lines, page_layout_t * page_layout) {
	int column_pos = 0;
	int page_idx = 1;

	int pango_column_height = page_layout->page_height - page_layout->top_margin - page_layout->bottom_margin;

	while(pango_lines && pango_lines->data) {
	    auto line_link = (linelink_t *)pango_lines->data;
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
	    TRACE ("gdk_draw_layout_line: %lf, %lf\n", x_pos, y_pos);
	    cairo_move_to (page_layout->cr, x_pos, y_pos);
	    pango_cairo_show_layout_line (page_layout->cr, line);
	    pango_lines = pango_lines->next;
	}
	return page_idx;
    }


    static PangoLanguage *
    get_language (void) {
	PangoLanguage *retval;
	gchar *lang = g_strdup (setlocale (LC_CTYPE, NULL));
	gchar *p;

	p = strchr (lang, '.');
	if(p) *p = 0;
	p = strchr (lang, '@');
	if(p) *p = 0;

	retval = pango_language_from_string (lang);
	g_free (lang);

	return retval;
    }


    /* Split a list of paragraphs into a list of lines.
     */
    static GList *
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
	    auto para = (paragraph_t *)par_list->data;

	    para_num_lines = pango_layout_get_line_count (para->layout);
	    TRACE ("para_num_lines = %d\n", para_num_lines);

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
		ERROR("%s: Invalid character in input\n", g_get_prgname ());
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

    static gchar *
    directoryText(const gchar *path){
	gint count=0;
	DIR *directory = opendir(path);
	if (!directory) {
	    ERROR("directoryText(): Cannot open %s\n", path);
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
	
	gchar *utf = Util<Type>::utf_string(path);
	gchar *dir_text = g_strdup_printf("%s:\n", utf);
	g_free(utf);

	GSList *list = NULL;
        struct dirent *d; // static pointer
        while ((d = readdir(directory))  != NULL){
	    utf = Util<Type>::utf_string(d->d_name);
	    auto xd_p = LocalView<Type>::get_xd_p(path, d, TRUE);
	    const gchar *string=_("unknown");
	    switch (xd_p->d_type){
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

	    g_free(utf);
	    list = g_slist_prepend(list, line);
	    if (count++ >= 100) break;
	}
	closedir (directory);
	list = g_slist_sort(list, x_strcmp);
	GSList *tmp = list;
	for (;tmp && tmp->data; tmp = tmp->next){
	    gchar *g = g_strdup_printf("%s\t%s\n", dir_text, (gchar *)tmp->data);
	    g_free(dir_text);
	    dir_text = g;
	    g_free(tmp->data);
	}
	g_slist_free(list);

	return dir_text;
    }


    static GdkPixbuf *
    loadFromHash(const gchar *filePath){
	// Check if in pixbuf hash. If so, return with the hashed pixbuf.
	// Note that if the thumbnail is out of date, the thumbnail should
	// be marked invalid. This will happen if filePath is absolute.
	auto pixbuf = PixbufHash<Type>::find_in_pixbuf_hash(filePath, PREVIEW_IMAGE_SIZE);
	
	if(pixbuf) {
	    TRACE( "previewAtSize(): pixbuf %s located in hash table.\n",
		    filePath);
	    return pixbuf;
	}
	return NULL;
    }

    static GdkPixbuf *
    loadFromThumbnails(const gchar *filePath, struct stat *st_p){
	return loadFromThumbnails(filePath, st_p, 3*PREVIEW_IMAGE_SIZE/4, PREVIEW_IMAGE_SIZE);
    }
public:
    static GdkPixbuf *
    loadFromThumbnails(const gchar *filePath, struct stat *st_p, 
	    gint width, gint height){
	// Look into thumbnail cache directory...
	auto thumbnailPath = PixbufHash<Type>::get_thumbnail_path (filePath, height);
	if (g_file_test(thumbnailPath,G_FILE_TEST_EXISTS)){
	    errno=0;
	    struct stat st;
	    if (stat(thumbnailPath, &st)<0){
		DBG("preview.hh::loadFromThumbnails(): stat %s (%s)\n",
		    thumbnailPath, strerror(errno));
		errno=0;
		g_free(thumbnailPath);
		return NULL;
	    }

	    if (st_p && (st.st_mtime < st_p->st_mtime)){
		unlink(thumbnailPath);
	    } else {
		GError *error=NULL;
		TRACE("Now loading pixbuf from %s\n",  thumbnailPath);
		auto pixbuf = gdk_pixbuf_new_from_file (thumbnailPath, &error);
		if (error){
		    ERROR("previewAtSize: %s (%s)\n", thumbnailPath, error->message);
		    g_error_free(error);
		} else {
		    g_free(thumbnailPath);
		    //resize
		    gint h = gdk_pixbuf_get_height(pixbuf);
		    if (h != height){
			auto newPixbuf = 
			    gdk_pixbuf_scale_simple (pixbuf, 
				width,
				height, GDK_INTERP_HYPER);
			g_object_unref(pixbuf);
			pixbuf = newPixbuf; 
		    }
		    return pixbuf;
		}
	    }
	}
	else TRACE("%s does not exist\n", thumbnailPath);
	g_free(thumbnailPath);
	return NULL;
    }
private:
    static GdkPixbuf *
    textPreview (const gchar *path) {

	// Read a cache page worth of text and convert to utf-8 

	gchar *text;
	if (strcmp(path, "empty-file")==0){
	    TRACE( "oooooo   size for %s == 0\n", path);
	    text = g_strdup_printf("*****  %s  *****", _("Empty file"));
	} else {
	    text = readFileHead(path);
	}
	if(!text) return NULL;
	if (!strchr(text, '\n')){
	    gchar *t = g_strconcat(text,"\n",NULL);
	    g_free(text);
	    text = t;
	}
	void *arg[]={
	    (void *)text,
	    (void *)path
	};
	auto pixbuf = (GdkPixbuf *)Util<Type>::context_function(text_preview_f, (void *)arg);
	g_free(text);
	return pixbuf;
    }

    static gchar *
    convertToUtf8(gchar *inputText, gchar *encoding){
	if(encoding && (!strstr (encoding, "utf-8") || !strstr (encoding, "UTF-8"))) {
	    gsize bytes_read;
	    gsize bytes_written;
	    GError *error = NULL;
	    gchar *outputText = 
		g_convert_with_fallback (inputText, -1, "UTF-8", encoding,
					 NULL, &bytes_read, &bytes_written, &error);
	    gboolean success = TRUE;
	    if(error) {
		success = FALSE;
		ERROR("g_convert_with_fallback() to UTF8: %s\n", error->message);
		g_error_free (error);
		error=NULL;
		outputText = 
		    g_convert_with_fallback (inputText, -1, "UTF-8", "iso8859-15",
					     NULL, &bytes_read, &bytes_written, &error);
		if (error) {
		    ERROR("b.g_convert_with_fallback() to iso8859-15: %s\n", error->message);
		    g_error_free (error);
		} else success = TRUE;

	    }
	    if (success){
		TRACE ("encoding OK from %s (to UTF8 or iso8859-15)\n", encoding);
		g_free (inputText);
		return outputText;
	    }
	} 
	g_free(encoding);
	return inputText;
    }

    static gchar *
    errorText(const gchar *path){
	gchar *g = g_strdup_printf(_("Cannot read from %s"), path);
	gchar *head = g_strdup_printf("%s\nreadFileHead(): calloc outputText: %s\n",
		g, strerror(errno));
	g_free(g);
	return head;
    }

    static gchar *
    readFileHead(const gchar *path) {
	auto encoding = Mime<Type>::encoding (path);
	TRACE("readFileHead: %s\n", path);
	if (g_file_test(path, G_FILE_TEST_IS_DIR)){
	    gchar *head = directoryText(path);
	    if (strlen(head) >= BUFSIZE) head[BUFSIZE-1] = 0;
	    return convertToUtf8(head, encoding);
	}
	gint fd = open (path, O_RDONLY);
	if(fd < 0) {
	    ERROR("readFileHead(): open(%s): %s\n", path, strerror (errno));
	    return errorText(path);
	}
	gchar buffer[BUFSIZE];
	memset(buffer,0, BUFSIZE);
	gint bytes = read (fd, buffer, BUFSIZE - 1);
	buffer[BUFSIZE - 1] = 0;
	if(strrchr (buffer, '\n')) {
	    *(strrchr (buffer, '\n') + 1) = 0;
	}
	close (fd);

	if(bytes < 0) {
	    ERROR("readFileHead():read %s (%s).\n", path, strerror (errno));
	    return errorText(path);
	}
	gint i;
	// Switch non printable characters to '.'
	for (i=0; i<BUFSIZE-2; i++) {
	    if (buffer[i] < 32 ) {
		if (buffer[i] == 9) continue;
		if (buffer[i] == 10) continue;
		if (buffer[i] == 0) break;
		else buffer[i] = '.';
	    }
	}

	auto outputText = g_strdup(buffer);

	return convertToUtf8(outputText, encoding);

    }



    // this function take a private population_p
    static void *
    text_preview_f (void *data){
	auto arg = (void **)data;
	auto text = (gchar *)arg[0];
	auto filePath = (gchar *)arg[1];
	auto thumbnail = PixbufHash<Type>::get_thumbnail_path (filePath, PREVIEW_IMAGE_SIZE);
	
	page_layout_t page_layout;

	TRACE ("PAP: file read complete:\n%s\n", "");
	TRACE ("PAP: file read complete:\n%s\n", text);

	/* Page layout, either a4 or letter or legal. */
	page_layout.page_width = paper_sizes[PAPER_TYPE_US_LETTER].width;
	page_layout.page_height = paper_sizes[PAPER_TYPE_US_LETTER].height;
	page_layout.top_margin = 36;
	page_layout.bottom_margin = 36;
	page_layout.right_margin = 36;
	page_layout.left_margin = 36;

	// determine pango_dir  (XXX deprecated, pango should do this automatically...)
	//page_layout.pango_dir = pango_find_base_dir (text, -1);
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
	const gchar *font = MAKE_FONT_NAME (DEFAULT_FONT_FAMILY, DEFAULT_FONT_SIZE);
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

	TRACE ("pango setup complete\n");
	GList *paragraphs = split_text_into_paragraphs (&page_layout, text);

	TRACE ("pango split_text_into_paragraphs complete--> %d \n", g_list_length (paragraphs));
	GList *pango_lines = split_paragraphs_into_lines (&page_layout, paragraphs);
	TRACE ("pango split_paragraphs_into_lines complete--> %d\n", g_list_length (pango_lines));

	// cairo setup
	cairo_new_path (page_layout.cr);
	cairo_set_line_width (page_layout.cr, 0.5);
	cairo_set_source_rgb (page_layout.cr, 0.0, 0.0, 0.0);

	//output layouts to page ()
	output_page (pango_lines, &page_layout);
	// tmp: write png of pixbuf
	TRACE("// numpages=%d\n", num_pages);
	cairo_destroy (page_layout.cr);
	TRACE ("// write thumbnail\n");
	if(cairo_surface_write_to_png (page_layout.surface, thumbnail) != CAIRO_STATUS_SUCCESS) {
	    ERROR("cairo_surface_write_to_png(surface,) != CAIRO_STATUS_SUCCESS");
	}
	cairo_surface_destroy (page_layout.surface);
	// destroy GLists
	GList *tmp;
	for(tmp = pango_lines; tmp && tmp->data; tmp = tmp->next) {
	    auto line = (linelink_t *)tmp->data;
	    if(G_IS_OBJECT (line->pango_line)) g_object_unref (line->pango_line);
	    g_free (line);
	}
	for(tmp = paragraphs; tmp && tmp->data; tmp = tmp->next) {
	    auto para = (paragraph_t *)tmp->data;
	    if(G_IS_OBJECT (para->layout)) {
		g_object_unref (para->layout);
	    }
	    g_free (para);
	}
	if(G_IS_OBJECT (page_layout.context)) g_object_unref (page_layout.context);

	g_list_free (paragraphs);
	g_list_free (pango_lines);
     
	// pixbuf generated and reffed in routine:
	TRACE("load_preview_pixbuf_from_disk %s\n", thumbnail);
	auto pixbuf = Pixbuf<Type>::pixbuf_from_file (thumbnail, -1, -1);
	GdkPixbuf *original=pixbuf;
	pixbuf = Pixbuf<Type>::fix_pixbuf_scale(original, PREVIEW_IMAGE_SIZE); // this unrefs old and refs new.
	if (original != pixbuf) {
	    Pixbuf<Type>::pixbuf_save(pixbuf, thumbnail);
	}
	return pixbuf;
    }

};

}

#endif

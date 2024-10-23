#ifndef PREVIEW_HH
#define PREVIEW_HH
#include <sys/types.h>
#include <sys/wait.h>

#define PREVIEW_IMAGE_SIZE  400
#define BUFSIZE (4096)
#define DEFAULT_FONT_FAMILY    "Sans"
//#define DEFAULT_FONT_FAMILY   "Arial"
#define DEFAULT_FONT_SIZE    "12"
#define MAKE_FONT_NAME(f,s)    f " " s

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

template <class Type> class Texture;
template <class Type>
class Preview {
  public:
    static GdkPaintable *
    getPaintableWithThumb(const char *path)
    {
        GdkPaintable *paintable = NULL;
        if (!path) {
            TRACE("getPaintableWithThumb(path==NULL)\n");
            return NULL;
        }

        if (thumbnailOK(path)){
            // Read thumbnail.
            paintable = readThumbnail(path);
            if (paintable){
               TRACE("getPaintableWithThumb(): Loaded %s from thumbnail at height %d.\n", path, height);
               return paintable;
            }
        } 
        // Thumbnail not OK.
        paintable = buildImagePaintable(path);
        if (!paintable) {
            TRACE("buildImagePaintable(%s)\n", path);
            return NULL;
        }
        saveThumbnail(path, paintable);
        return paintable;
    }
    
    static GdkPaintable *
    textPreview (const gchar *path, gint pixels) {
      GdkPaintable *paintable = NULL;
      if (thumbnailOK(path)){
          // Read thumbnail.
          paintable = readThumbnail(path);
          if (paintable){
             TRACE("getPaintableWithThumb(): Loaded %s from thumbnail at height %d.\n", path, height);
             return paintable;
          }
      } 

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

      paintable = processTextPixbuf(text, path, pixels);
      g_free(text);
      return paintable;        
    }
       
    static GdkPaintable *
    gsPreview (const gchar *path, gint pixels) {
      GdkPaintable *paintable = NULL;
      if (thumbnailOK(path)){
          // Read thumbnail.
          paintable = readThumbnail(path);
          if (paintable){
             TRACE("getPaintableWithThumb(): Loaded %s from thumbnail at height %d.\n", path, height);
             return paintable;
          }
      } 
      return ghostscript(path, pixels);
    }
  private:


    typedef struct paintable_t {
      GdkPaintable *paintable;
      time_t mtime;
    } paintable_t;
    static void freePaintable(void *data){
      auto paintableX = (paintable_t *)data;
      g_object_unref(paintableX->paintable);
      g_free(data);
    }

    static GHashTable *hash(void){
      static GHashTable *paintableHash = NULL;
      if (!paintableHash) paintableHash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, freePaintable);
      return paintableHash;
    }

    static void
    zapThumbnail(const char *path){
        //Eliminate from thumbnail cache:
      auto item = g_hash_table_lookup(hash(), path);
      if (item){
        g_hash_table_remove(hash(), path);
      }
    }
    static void
    saveThumbnail(const char *path, GdkPaintable *paintable){
        if (!path || !paintable) {
            ERROR("saveThumbnail(%s): !name \n", path);
            return ;
        }
        zapThumbnail(path);
        auto paintableX = (paintable_t *)calloc(1, sizeof(paintable_t));
        g_object_ref(G_OBJECT(paintable));
        paintableX->paintable = paintable;
        paintableX->mtime = time(NULL);

        g_hash_table_insert(hash(), g_strdup(path), paintableX);
    }
    static GdkPaintable *
    readThumbnail(const char *path){
      auto item = g_hash_table_lookup(hash(), path);
      if (!item) return NULL;
      auto paintableX = (paintable_t *)item;
      return GDK_PAINTABLE(paintableX->paintable);
    }
    static bool thumbnailOK(const char *path){
        bool retval = true;
        auto item = g_hash_table_lookup(hash(), path);
        if (!item) return false;
        auto paintableX = (paintable_t *)item;
        
        struct stat stPath;
        if (stat(path, &stPath)<0) {
            DBG("thumbnailOK(): stat %s (%s)\n", path, strerror(errno));
            errno=0;
            retval = false;
        } 
        if (paintableX->mtime < stPath.st_mtime){
            TRACE("thumbnailOK(%s): out of date. Removing %s.\n", name, thumbnailPath);
            zapThumbnail(path);
            retval = false;
        }     
        return true;
    }
    static
    GdkPaintable *
    zipThumbnail(const char *path){
        GdkPaintable *paintable = NULL;
#ifdef HAVE_ZIP_H
        TRACE("creating zip preview for %s\n",path);
        int errorp;
        auto zf = zip_open(path, ZIP_RDONLY, &errorp);
        if (!zf) {
            return NULL;
        }
        struct zip_stat sb;
        if(zip_stat (zf, "Thumbnails/thumbnail.png", 0, &sb) != 0) {
            zip_close (zf);
            return NULL;
        }
        void *ptr = calloc(1,sb.size);
        if (!ptr) g_error("calloc: %s", strerror(errno));
        struct zip_file *f = zip_fopen (zf, "Thumbnails/thumbnail.png", 0);
       
        zip_fread (f, ptr, sb.size);
        zip_fclose (f);
        zip_close (zf);
        gchar *base = g_path_get_basename(path);
        gchar *fname = g_strdup_printf("%s/%d-%s.png",
                g_get_tmp_dir(), getuid(), base);
        g_free(base);
        gint fd=creat(fname, S_IRUSR | S_IWUSR);
        if (fd >= 0){
            if (write(fd, ptr, sb.size) < 0){
                DBG("could not write to %s\n", fname);
            }
            close(fd);
        }
        if (g_file_test(fname, G_FILE_TEST_EXISTS)){
            paintable = GDK_PAINTABLE(gdk_texture_new_from_filename (fname, NULL));
            unlink(fname);
        }    
        g_free(fname);
        g_free(ptr);
#endif
        return paintable;
    }
    static bool
    isZipThumbnailed(const char *path){
#ifdef HAVE_ZIP_H
        int errorp;
        auto zf = zip_open(path, ZIP_RDONLY, &errorp);
        if (!zf) {
            return FALSE;
        }
        auto f = zip_fopen(zf, "Thumbnails/thumbnail.png", 0);
        bool retval;
        if (f){
            retval = TRUE;
            zip_fclose(f);
        } else retval = FALSE;
        zip_close(zf);
        return retval;
#else
 #warning "isZipThumbnailed(): zip.h not configured."
#endif
        return FALSE;
    }
    static GdkPaintable *
    buildImagePaintable(const char *path){
        GError *error_ = NULL;
        GdkPaintable *paintable = NULL;
        if (isZipThumbnailed(path)) {
            paintable = zipThumbnail(path);
        }
        if (!paintable) {
            // If file disappears before this completes, pixbuf will be
            // NULL and GError undefined. So just ignore GError to avoid
            // crash while trying to get the error message.
            paintable = GDK_PAINTABLE(gdk_texture_new_from_filename (path, &error_));
            if (error_){
              DBG("** Error::buildImagePaintable():%s\n", error_->message);
              g_error_free(error_);
              return NULL;
            }
        }
        return paintable;
     }
    static int
    x_strcmp(gconstpointer a, gconstpointer b){
        return strcmp((char *)a, (char *)b);
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
        
        gchar *utf = Basic::utf_string(path);
        gchar *dir_text = g_strdup_printf("%s:\n", utf);
        g_free(utf);

        GSList *list = NULL;
            struct dirent *d; // static pointer
            while ((d = readdir(directory))  != NULL){
            utf = Basic::utf_string(d->d_name);
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
    static gchar *
    errorText(const gchar *path){
        gchar *g = g_strdup_printf(_("Cannot read from %s"), path);
        gchar *head = g_strdup_printf("%s\nreadFileHead(): calloc outputText: %s\n",
            g, strerror(errno));
        g_free(g);
        return head;
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
    readFileHead(const gchar *path) {
        auto encoding = MimeMagic::encoding (path);
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
    static GdkPaintable *
    processTextPixbuf(const gchar *text, const gchar *path, gint pixels){
        if(!text) return NULL;
        void *arg[]={
            (void *)text,
            (void *)path,
            GINT_TO_POINTER(pixels)
        };
        //auto paintable = (GdkPaintable *)Basic::context_function(text_preview_f, (void *)arg);
        auto paintable = (GdkPaintable *)text_preview_f(arg);
        return paintable;
    }


    
    static void *
    text_preview_f (void *data){
        auto arg = (void **)data;
        auto text = (gchar *)arg[0];
        auto path = (gchar *)arg[1];
        gint pixels = GPOINTER_TO_INT(arg[2]);
        
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
        // determine pango_dir  (XXX deprecated, pango should do this automatically...)
        //pango_context_set_base_dir (page_layout.context, page_layout.pango_dir);

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
        // tmp: write png of paintable
        //TRACE("// numpages=%d\n", num_pages);
        cairo_destroy (page_layout.cr);
        TRACE ("// write previewPixbuf\n");

        auto paintable = GDK_PAINTABLE(Texture<Type>::gdk_texture_new_for_surface(page_layout.surface));

        //if(cairo_surface_write_to_png (page_layout.surface, previewPixbuf) != CAIRO_STATUS_SUCCESS) {
          //  ERROR("cairo_surface_write_to_png(surface,) != CAIRO_STATUS_SUCCESS");
        //}
        cairo_surface_destroy (page_layout.surface);


        // cleanups
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
/*         
        // paintable generated and reffed in routine:
        TRACE("load_preview_paintable_from_disk %s\n", previewPixbuf);
        GError *error = NULL;
        auto paintable = GDK_PAINTABLE(gdk_texture_new_from_filename (previewPixbuf, &error));
        if(error) {
            ERROR ("text_preview_f() %s:%s\n", error->message, previewPixbuf);
            g_error_free (error);
            return NULL;
        }
*/
        //GdkPaintable *original=paintable;
        // this does the antialiasing...
        //paintable = fixPixbufScale(original, pixels); // this unrefs old and refs new.
        return paintable;
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
//////////////////////////   pdf /////////////////////////////////////

    static GdkPaintable *
    ghostscript (const gchar *path, gint pixels) {
      GdkPaintable * retval=NULL;
      char *ghostscript = g_find_program_in_path ("gs");
      static gboolean warned = FALSE;
      if(!ghostscript) {
        if(!warned) {
            g_warning
            ("\n*** Please install ghostscript for ps and pdf previews\n*** Make sure ghostscript fonts are installed too!\n");
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

      // The preview will be generated at size: PREVIEW_IMAGE_SIZE
      auto previewPath = get_thumbnail_path (path, PREVIEW_IMAGE_SIZE);
      //auto thumbnail = get_thumbnail_path (path, pixels);

      //pdf and ps ghostscript conversion
      src = g_strdup (path);
      tgt = g_strdup_printf ("-sOutputFile=%s", previewPath);
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
      // this fork is ok from thread, I guess.
      TRACE( "gsPreview(): preview for %s\n", src);
      pid_t pid = fork ();
      if(!pid) {
        TRACE( "--> child is creating preview %s\n", preview);
            auto fd = fopen ("/dev/null", "wb");
            dup2(fileno(fd), fileno(stdout));
            execv (arg[0], arg);
            _exit (123);
      } else {
        // Create wait thread. Detached.
        void **arg = (void **)calloc(3, sizeof(void *));
        if (!arg) g_error("gsPreview(): malloc: %s\n", strerror(errno));
        pthread_mutex_t waitMutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t waitSignal = PTHREAD_COND_INITIALIZER;
        
        arg[0] = &waitMutex;
        arg[1] = &waitSignal;
        arg[2] = GINT_TO_POINTER(pid);

        pthread_mutex_lock(&waitMutex);
        
        auto dbgText = g_strdup_printf("Preview::gsPreview(%s)", path);
        auto thread_p = new Thread(dbgText, gs_wait_f, arg);
        g_free(dbgText);

        const struct timespec abstime={
            time(NULL) + 4, 0
        };
        if (pthread_cond_wait(&waitSignal, &waitMutex) != 0){
            ERROR("Aborting runaway ghostscript preview for %s (pid %d)\n",
                src, (int)pid);
            kill(pid, SIGKILL);
        } else {
            TRACE("condition wait complete for file %s\n", previewPath);
            // this function refs retval
            // Now we correct for requested size
            //retval = correctPixbufSize(path, pixels);
            //retval = loadFromThumbnails(path, NULL, pixels);
            GError *error_ = NULL;
            retval = GDK_PAINTABLE(gdk_texture_new_from_filename (previewPath, &error_));
            if (error_){
              DBG("** Error::gsPreview(): %s\n", error_->message);
              g_error_free(error_);
              retval = NULL;
            }
        }
        pthread_mutex_unlock(&waitMutex);
        pthread_mutex_destroy(&waitMutex);
            TRACE ("SHOW_TIPx: preview created by convert\n");
      }
      g_free (ghostscript);       //arg[0]
      g_free (src);
      g_free (tgt);
      g_free (previewPath);
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
    static gchar *
    get_thumbnail_path (const gchar * file, gint size) {
      gchar *cache_dir;
      gchar *thumbnail_path = NULL;
      GString *gs;
      gchar key[11];

      cache_dir = g_build_filename (XFTHUMBNAIL_DIR, NULL);
      if(g_mkdir_with_parents (cache_dir, 0700) < 0) {
          g_free (cache_dir);
          return NULL;
      }

      /* thumbnails are not subject to thumbnailization: */
      gchar *dirname = g_path_get_dirname (file);
      if(strncmp (cache_dir, dirname, strlen (cache_dir)) == 0) {
          TRACE ("thumbnails cannot be thumbnailed:%s\n", file);
          g_free (cache_dir);
          g_free (dirname);
          return NULL;
      }

      gs = g_string_new (dirname);
      auto uintKey = g_string_hash (gs);
      sprintf (key, "%10u", uintKey);
      g_strstrip (key);
      g_string_free (gs, TRUE);
      g_free (dirname);

      gchar *thumbnail_dir = g_build_filename (cache_dir, key, NULL);
      if(g_mkdir_with_parents (thumbnail_dir, 0700) < 0) {
          g_free (thumbnail_dir);
          return NULL;
      }

      gchar *filename = g_path_get_basename (file);

      gs = g_string_new (file);
      sprintf (key, "%10u", g_string_hash (gs));
      g_strstrip (key);
      g_string_free (gs, TRUE);
      g_free (filename);

      filename = g_strdup_printf ("%s-%d.png", key, size);
      thumbnail_path = g_build_filename (thumbnail_dir, filename, NULL);
      g_free (filename);
      g_free (cache_dir);
      g_free (thumbnail_dir);
      TRACE ("thread: %s ->thumbnail_path=%s\n", file, thumbnail_path);

      return thumbnail_path;
    }

};

}

#endif

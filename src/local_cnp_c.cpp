#include "local_cnp_c.hpp"
#include "view_c.hpp"


local_cnp_c::local_cnp_c(void *data):key_file(NULL){
    view_v = data;
}

void
local_cnp_c::clear_paste_buffer(void){
    shm_unlink (PASTE_SHM_NAME);
}

void
local_cnp_c::store_paste_buffer(gchar *buffer, gint len){ 

    // Remove old MIT-shm  pasteboard.
    shm_unlink (PASTE_SHM_NAME);
    
    gint fd = shm_open (PASTE_SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(fd < 0){
        g_error ("rfm_store_paste_buffer(): shm_open(%s): %s", PASTE_SHM_NAME, strerror (errno));
    }

    // Truncate to necessary memory size to allocate.
    if(ftruncate (fd, sizeof(gint)+strlen(buffer)+1) < 0) {
        g_error ("rfm_store_paste_buffer(): ftruncate(%s): %s", PASTE_SHM_NAME, strerror (errno));
    }

    // Get a shared memory pointer.
    void *p = mmap (NULL, sizeof(gint)+strlen(buffer)+1, 
	    PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   // Close file descriptor
    close(fd);
    // Save size as first sizeof(gint) bytes.
    gint *len_p = p;
    *len_p=sizeof(gint)+strlen(buffer)+1;
    // Save text buffer now.
    gchar *buffer_p = p + sizeof(gint);
    strcpy(buffer_p, buffer);
    // Put in shared memory.
    if(msync (p, sizeof(gint)+strlen(buffer)+1, MS_SYNC) < 0){
        DBG ("rfm_store_paste_buffer(): msync(%s): %s\n", PASTE_SHM_NAME, strerror (errno));
    }
    // Release map so other processes may shm_unlink.
    munmap (p, sizeof(gint)+strlen(buffer)+1);
}

 
gint local_cnp_c::get_paste_length(){
    gint fd = shm_open (PASTE_SHM_NAME, O_RDONLY, S_IRUSR | S_IWUSR);
    if(fd < 0){
	return 0;
    }
    // Figure out the size.
    void *p = mmap (NULL, sizeof(gint), 
    //void *p = mmap (NULL, 133, 
	    PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    gint len = *((gint *)p);
    if(msync (p, sizeof(gint), MS_SYNC) < 0){
        DBG ("msync(%s): %s\n", PASTE_SHM_NAME, strerror (errno));
    }
    munmap (p, sizeof(gint));
    return len;
}


gchar *
local_cnp_c::get_paste_buffer (void ) {
    gint len=get_paste_length();
    if (len==0) {
	return NULL;
    }
    int fd = shm_open (PASTE_SHM_NAME, O_RDONLY, S_IRUSR | S_IWUSR);
    if(fd < 0){
	return NULL;
    }


    void *pp = mmap (NULL, len, PROT_READ, MAP_SHARED, fd, 0);
    gchar *buffer_p = pp + sizeof(gint);
    gchar *buffer = g_strdup(buffer_p);
    munmap (pp, len);
    close(fd);
    return buffer;
}


int
local_cnp_c::pasteboard_status (void) {
    // This is cool with client side pasteboard because
    // nobody is going to mess with pasteboard.
    // With server side, this is not so, because somebody
    // might mess it up and rodent will still think the pasteboard
    // is valid...

    rfm_update_pasteboard ();
    view_c *view_p = (view_c *)view_v;
    gchar *b = view_p->xbuffer;
    if(!b || !strlen (b)) return 0;
    
    const gchar *cut = "#xfvalid_buffer:cut";
    const gchar *copy = "#xfvalid_buffer:copy";
    if(strncmp (b, copy, strlen (copy)) == 0)
        return 1;
    if(strncmp (b, cut, strlen (cut)) == 0)
        return 2;
    return 0;
}

gchar **
local_cnp_c::pasteboard_v(void){
    if (!rfm_pasteboard_status ()) return NULL;
    // this is to skip the valid buffer line:
    view_c *view_p = (view_c *)view_v;
    gchar *search = strchr (view_p->xbuffer, '\n');
    if(!search)return NULL;
    search++;
    gchar **v = g_strsplit(search, "\n", -1);
    return v;
}


/* returns 0 if not in pasteboard, 1 if in copy pasteboard or 2 if
 * in cut pasteboard */
int
local_cnp_c::in_pasteboard (record_entry_t * en) {
    if(!en || !en->path) return FALSE;
    if(IS_ROOT_TYPE (en->type) && !IS_SDIR(en->type)) return FALSE;

    gchar **pasteboard_v = rfm_pasteboard_v(view_p);
    gchar **p = pasteboard_v;
    gint retval=0;
    gint status = rfm_pasteboard_status (view_p);
    for(; p && *p; p++){
        if(strcmp (*p, en->path) == 0) {
            retval = status;
            break;
        }
    }
    g_strfreev(pasteboard_v);
    return retval;
}

gboolean
local_cnp_c::update_pasteboard (view_t * view_p) {
    if(!view_p->xbuffer) view_p->xbuffer = rfm_get_paste_buffer ();
    gchar *current_xbuffer = rfm_get_paste_buffer ();
    if (!current_xbuffer && !view_p->xbuffer) return FALSE;
    if(!view_p->xbuffer && current_xbuffer){
        view_p->xbuffer = current_xbuffer;
        return TRUE;
    }
    if(view_p->xbuffer && !current_xbuffer){
        g_free (view_p->xbuffer);
        view_p->xbuffer = NULL;
        return TRUE;
    }
    // here both pointers are valid
    if(strcmp (current_xbuffer, view_p->xbuffer)) {
        NOOP ("XBUFFER: xbuffer has changed! %s\n", current_xbuffer);
        g_free (view_p->xbuffer);
        view_p->xbuffer = current_xbuffer;
        return TRUE;
    } else {
        NOOP ("XBUFFER: xbuffer OK!\n");
        g_free (current_xbuffer);
        return FALSE;
    }

}


static gint
gui_pasteboard_list (GList ** list_p) {

    // client side pasteboard, via MIT-shm
    gchar *b=rfm_get_paste_buffer();
    if((!b) || (!strlen (b))) {
      no_pasteboard:
	g_free(b);
	return 0;
    }
    gboolean cut;
    gchar *word;
    if((word = strtok (b, ":")) == NULL)
        goto no_pasteboard;
    if(!strstr (word, "#xfvalid_buffer"))
        goto no_pasteboard;
    if((word = strtok (NULL, ":")) == NULL)
        goto no_pasteboard;
    if(strstr (word, "cut"))
        cut = TRUE;
    else
        cut = FALSE;
    if((word = strtok (NULL, ":")) == NULL)
        goto no_pasteboard;
    //src_host = g_strdup(word);

    word = word + strlen (word) + 1;
    if(word[0] == '\n') {
        word++;
        if(word[0] == 0)
            goto no_pasteboard;
    } else {
        if((word = strtok (NULL, "\n")) == NULL)
            goto no_pasteboard;
        word = word + strlen (word) + 1;
    }

    /* create list to send to CreateTmpList */
    gint retval = rfm_uri_parse_list (word, list_p);
 
    g_free (b);
    if (retval) {
	if(cut) retval=1;
	else retval= 2;
    }
    return retval;
}

static gint
gui_pasteboard_transfer (widgets_t * widgets_p,
                         record_entry_t * t_en, 
			 GList * list, 
			 gboolean cut,
			 gboolean symlink) {
    if (!list){
        g_warning("gui_pasteboard_transfer() list is null\n");
        return 0;
    }
    gchar *url = (gchar *) list->data;
    if(!url){
	DBG("gui_pasteboard_transfer: !url\n");
        return 0;
    }
 
    if(t_en->module) {
        NOOP ("PASTE: en->module=%s\n", t_en->module);
        if(rfm_natural (PLUGIN_DIR, t_en->module, t_en, "valid_drop_site")) {
            NOOP ("module: valid_drop_site for %s\n", t_en->module);
            rfm_natural (PLUGIN_DIR, t_en->module, t_en, "set_drop_entry");
            if(rfm_rational (PLUGIN_DIR, t_en->module, list, widgets_p, "process_drop")) {
                NOOP ("module: process_drop ok\n");
                /*result=TRUE; */
            }
            rfm_void (PLUGIN_DIR, t_en->module, "clear_drop_entry");
            return 1;
        }
    }
    NOOP ("PASTE: must be local then\n");
    int mode;
    if(cut) {
        mode = TR_MOVE;
    } else {
        mode = TR_COPY;
    }
    if(symlink) {
        mode = TR_LINK;
    }
       
    // Only the uploading notice works here...
    gchar *text=NULL;
    const gchar *icon=NULL;
    //gint type=0;
    gboolean local_target = TRUE;
    //gboolean local_source = TRUE;
    //if (!IS_LOCAL_TYPE(type))local_source = FALSE;
    if (!IS_LOCAL_TYPE(t_en->type))local_target = FALSE;
    if (!local_target){
      switch (mode){
        case TR_COPY:
	case TR_MOVE:
        case TR_COPY_REMOTE:
	case TR_MOVE_REMOTE:
	    icon = "xffm/emblem_network/compositeSE/stock_go-forward";
	    text = g_strdup_printf(_("Uploading file %s"), "...");
	    break;
	default:
	    break;
      }
    } else {
	// Local target. Test first item of list for remote host.
	const gchar *path = list->data;
	record_entry_t *en = rfm_stat_entry(path, 0);
	if (!IS_LOCAL_TYPE(en->type)) {
	    icon = "xffm/emblem_network/compositeSE/stock_go-back";
	    text = g_strdup_printf(_("Downloading file %s..."), "");
	}
	rfm_destroy_entry(en);
   }


    
    if (text) {
	rfm_threaded_diagnostics(widgets_p, "xffm/emblem_network/compositeSE/stock_go-forward", NULL);
	rfm_threaded_diagnostics(widgets_p, icon, NULL);
	rfm_threaded_diagnostics(widgets_p, "xffm_tag/red", g_strconcat(text, "\n", NULL));
	g_free(text);
    }
    // Here we are already in a threaded environment, so plain cp
    // would create an unnecessary extra thread.
    // cp (mode, list, t_en->path);
    plain_cp(widgets_p, mode, list, t_en->path, FALSE);
    
    return 1;
}


static void
private_paste (widgets_t * widgets_p, view_t * view_p, gboolean symlink) {
    gboolean cut;
    GList *list = NULL;
    record_entry_t *t_en = NULL;
    int i;


    
    if((t_en = view_p->en) == NULL){
	DBG("(t_en = view_p->en) == NULL\n");
        return;
    }


    if((i = gui_pasteboard_list (&list)) == 0){
	DBG("(i = gui_pasteboard_list (&list)) == 0\n");
        return;
    }
    if(i == 1)
        cut = TRUE;
    else
        cut = FALSE;
    i = gui_pasteboard_transfer (widgets_p, t_en, list, cut, symlink);

    list = rfm_uri_free_list (list);

    if(i) {
        if(cut){
	    rfm_clear_paste_buffer();
	}	    
    }
    return;
}

static void
gui_pasteboard_copy_cut (widgets_t * widgets_p, gboolean cut, GSList ** paste_list) {
    gint len;
    gchar *buffer;
    //gchar *files;
    view_t *view_p = widgets_p->view_p;
    // clear any previous icon emblems
    rodent_clear_cut_icons (view_p);
    
    if(*paste_list == NULL){
	DBG("*paste_list == NULL\n");
        return;
    }

    if(cut)
        rfm_threaded_status (widgets_p, "xffm/stock_dialog-info", g_strdup(_("Cut")));
    else
        rfm_threaded_status (widgets_p, "xffm/stock_dialog-info", g_strdup(_("Copy")));

    rfm_clear_paste_buffer();

    len = 1 + strlen ("#xfvalid_buffer:copy:%%:\n");
    len += strlen (g_get_host_name ());
    GSList *tmp = *paste_list;
    for(; tmp; tmp = tmp->next) {
        gint addlen;
        record_entry_t *en = (record_entry_t *) tmp->data;
        addlen = 0;
        len += (1 + strlen (en->path) + addlen);
    }
    buffer = (gchar *)malloc (len * sizeof (char) + 1);
    if(!buffer) {
        DBG ("rfm: unable to allocate paste buffer\n");
        return;
    }
    sprintf (buffer, "#xfvalid_buffer:%s:%s:\n", (cut) ? "cut" : "copy", g_get_host_name ());
    //files = buffer + strlen (buffer);

    for(tmp = *paste_list; tmp; tmp = tmp->next) {
        record_entry_t *en = (record_entry_t *) tmp->data;
        {
            strcat (buffer, en->path);
            strcat (buffer, "\n");
        }
    }
    NOOP("dbg:len=%d,strlen=%d,data=%s\n",len,strlen(buffer),buffer); 
    rfm_store_paste_buffer(buffer, len);
    g_free (buffer);
    buffer = NULL;

    if(cut)
        rfm_threaded_status (widgets_p, "xffm/stock_dialog-info", g_strconcat(_("Cut"), NULL));
    else
        rfm_threaded_status (widgets_p, "xffm/stock_dialog-info", g_strconcat(_("Copy"), NULL));
    //view_p->flags.pasteboard_serial++;
    NOOP ("PASTE: view_p->flags.pasteboard_serial=%d\n", view_p->flags.pasteboard_serial);
    gchar *value = g_strdup_printf ("%d", view_p->flags.pasteboard_serial + 1);
    if (rfm_rational (RFM_MODULE_DIR, "settings", (void *)"RFM_PASTEBOARD_SERIAL", (void *)value, "mcs_set_var") == NULL){
        rfm_setenv ("RFM_PASTEBOARD_SERIAL", value, TRUE);
        NOOP("cannot set RFM_PASTEBOARD_SERIAL");
    }
    g_free (value);
    rodent_update_cut_icons(view_p);
    TRACE("copy-cut calling rodent_expose_all()\n");
    //rodent_expose_all (view_p);
    rodent_redraw_items (view_p);
}

static void
copy_cut_callback (widgets_t *widgets_p, gboolean cut) {

    view_t *view_p = widgets_p->view_p;
    if (!rfm_entry_available(widgets_p, view_p->en)) return; 
    if(!view_p->selection_list) return;

    gui_pasteboard_copy_cut (widgets_p, cut, &(view_p->selection_list));
    // unselect stuff if it is cut or copied.
    GSList *tmp=view_p->selection_list;
    for (;tmp && tmp->data; tmp=tmp->next){
	record_entry_t *en=tmp->data;
	rfm_destroy_entry(en);
    }
    g_slist_free (view_p->selection_list);
    view_p->selection_list = NULL;

    // Mark all population items as unselected.
    if(rfm_population_read_lock (view_p, "copy_cut_callback")) {
        population_t **pp;
        for(pp = view_p->population_pp; pp && *pp; pp++) {
            population_t *population_p = *pp;
            population_p->flags  &= (POPULATION_SELECTED ^ 0xffffffff);
        }
        rfm_population_read_unlock (view_p, "copy_cut_callback");
    }

}
static void
paste_callback (widgets_t *widgets_p, gboolean symlink) {
    view_t *view_p = widgets_p->view_p;
    if(!view_p->en)
        return;
    if (!rfm_entry_available(widgets_p, view_p->en)) return; 
    //rfm_details->pastepath = view_p->en->path;
    private_paste (widgets_p, view_p, symlink);
}



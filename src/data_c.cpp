#include <iostream>
#include "data_c.hpp"

using namespace std;

static void 
free_pixbuf_t(void *data){
    pixbuf_t *pixbuf_p = (pixbuf_t *) data;
    if (!pixbuf_p) return ;
    NOOP(stderr, "destroying pixbuf_t for %s size %d\n", pixbuf_p->path, pixbuf_p->size);
    if (pixbuf_p->pixbuf && !G_IS_OBJECT (pixbuf_p->pixbuf)) {
	cerr << "This should not happen: pixbuf_p->mime_id, not a pixbuf:"
	    << pixbuf_p->mime_id << "\n";
    } else {
	g_object_unref (pixbuf_p->pixbuf);
    }
    g_free(pixbuf_p->path);
    g_free(pixbuf_p);
    return;
}

static void
free_apps(void *data){
    if (!data) return;
    gchar **apps = (gchar **)data;
    g_strfreev(apps);
}

void 
data_c::free_pixbuf_tt(void *data){
    free_pixbuf_t(data);
}

data_c::data_c(void){
    pthread_mutex_init(&readdir_mutex, NULL);
    pthread_mutex_init(&cache_mutex, NULL);
    pthread_mutex_init(&mimetype_hash_mutex, NULL);
    pthread_mutex_init(&alias_hash_mutex, NULL);
    pthread_mutex_init(&application_hash_mutex, NULL);

    mimetype_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    alias_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    application_hash_type = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_apps);

    // Read only hashes:
    application_hash_sfx = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    application_hash_icon = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    application_hash_text = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    application_hash_text2 = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    application_hash_output = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    application_hash_output_ext = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    generic_icon_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    
    iconname_hash = g_hash_table_new(g_str_hash, g_str_equal);
    populate_iconname_hash();
    pixbuf_hash = 
	g_hash_table_new_full (g_str_hash, g_str_equal,g_free, free_pixbuf_t);

    
    highlight_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    tooltip_text_hash = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);

}

data_c::~data_c(void){
    pthread_mutex_destroy(&readdir_mutex);
    g_hash_table_destroy(iconname_hash);
    g_hash_table_destroy(mimetype_hash);
    g_hash_table_destroy(alias_hash);
    g_hash_table_destroy(application_hash_type);

    g_hash_table_destroy(application_hash_sfx);
    g_hash_table_destroy(application_hash_icon);
    g_hash_table_destroy(application_hash_text);
    g_hash_table_destroy(application_hash_text2);
    g_hash_table_destroy(application_hash_output);
    g_hash_table_destroy(application_hash_output_ext);
    g_hash_table_destroy(generic_icon_hash);
    
    pthread_mutex_destroy(&cache_mutex);
    pthread_mutex_destroy(&mimetype_hash_mutex);
    pthread_mutex_destroy(&alias_hash_mutex);
    pthread_mutex_destroy(&application_hash_mutex);
    
    g_hash_table_destroy (pixbuf_hash);
   
    g_hash_table_destroy(highlight_hash);
    g_hash_table_destroy(tooltip_text_hash);

}

void
data_c::populate_iconname_hash(void){
    g_hash_table_insert(iconname_hash, _("Open terminal"), (void *)"terminal");
    g_hash_table_insert(iconname_hash, _("Execute Shell Command"), (void *)"execute");
    g_hash_table_insert(iconname_hash, _("Paste"), (void *)"edit-paste");
    g_hash_table_insert(iconname_hash, _("Add bookmark"), (void *)"list-add");
    g_hash_table_insert(iconname_hash, _("Remove bookmark"), (void *)"list-remove");
    g_hash_table_insert(iconname_hash, _("Search"), (void *)"find");
    g_hash_table_insert(iconname_hash, _("Close"), (void *)"cancel");
    g_hash_table_insert(iconname_hash, _("Exit"), (void *)"window-close");
    g_hash_table_insert(iconname_hash, _("Open with"), (void *)"execute");
    g_hash_table_insert(iconname_hash, _("Cut"), (void *)"cut");
    g_hash_table_insert(iconname_hash, _("Copy"), (void *)"copy");
    g_hash_table_insert(iconname_hash, _("Delete"), (void *)"delete");
    g_hash_table_insert(iconname_hash, _("Shred"), (void *)"dialog-warning");
    g_hash_table_insert(iconname_hash, _("bcrypt"), (void *)"emblem-keyhole");
    g_hash_table_insert(iconname_hash, _("Open in New Tab"), (void *)"open");
}

pthread_mutex_t *
data_c::get_readdir_mutex(void){ 
    return &readdir_mutex;
}

GtkApplication *
data_c::get_app(void){
    return app; 
}

void 
data_c::set_app(GtkApplication *data){
    app = data;
} 


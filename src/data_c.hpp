#ifndef DATA_C_HPP
#define DATA_C_HPP
#include "xffm+.h"

class data_c {
    public:
        data_c(void);
        ~data_c(void);
	GtkApplication *get_app(void); 
	void set_app(GtkApplication *); 
	pthread_mutex_t *get_readdir_mutex(void);

        GHashTable *pixbuf_hash;
	void free_pixbuf_tt(void *);
	

        pthread_mutex_t cache_mutex;
        pthread_mutex_t mimetype_hash_mutex;
        pthread_mutex_t alias_hash_mutex;
        pthread_mutex_t application_hash_mutex;


        GHashTable *mimetype_hash;
        GHashTable *alias_hash;
        GHashTable *application_hash_type;
        GHashTable *application_hash_sfx;
        GHashTable *application_hash_icon;
        GHashTable *application_hash_text;
        GHashTable *application_hash_text2;
        GHashTable *application_hash_output;
        GHashTable *application_hash_output_ext;
        GHashTable *generic_icon_hash;

        
        GHashTable *highlight_hash;
	
    private:
	GtkApplication *app; 
	pthread_mutex_t readdir_mutex;
};

#endif


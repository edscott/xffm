#ifndef LITE_C_HPP
#define LITE_C_HPP
#include "xffm+.h"

class lite_c {
    public:
        lite_c(void);
        ~lite_c(void);
        const gchar *get_lite_emblem(const gchar *);
        gboolean get_lite_colors(const gchar *, guchar *, guchar *, guchar *);

    private:
        GHashTable *lite_type_hash;
        GHashTable *lite_key_hash;
        pthread_mutex_t lite_hash_mutex;



};


#endif

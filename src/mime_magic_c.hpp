#ifndef MIME_MAGIC_C_HPP
#define MIME_MAGIC_C_HPP
#include "xffm+.h"
#include <pthread.h>
#ifdef HAVE_LIBMAGIC
#include <magic.h>
#else
#error "libmagic not found during configure!"
#endif

class mime_magic_c {
    public:
        mime_magic_c(void);
        gchar * mime_magic_unalias (const gchar *);
        gchar * mime_encoding (const gchar *);
        const gchar * mime_file (const gchar *);
    private:
        gchar *lib_magic (const gchar *, gint);
        pthread_mutex_t magic_mutex;
        magic_t cookie;



};

#endif


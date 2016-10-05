#ifndef PREVIEW_C_HPP
#define PREVIEW_C_HPP
#include "xffm+.h"


class preview_c {
    public:
	preview_c(gtk_c *);
	const gchar *want_imagemagick_preview (record_entry_t * en);
	GdkPixbuf *mime_preview (const population_t * population_p);
    private:
	gtk_c *gtk_p;

};


#endif

#ifndef PREVIEW_C_HPP
#define PREVIEW_C_HPP
#include "xffm+.h"

#define PREVIEW_IMAGE_SIZE 400

class preview_c {
    public:
	preview_c(xfdir_c *);
	const gchar *want_imagemagick_preview (record_entry_t * en);
	GdkPixbuf *mime_preview (const population_t * population_p);
    private:
	xfdir_c *xfdir_p;

};


#endif

#ifndef INTL_H
#define INTL_H

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(String) dgettext(GETTEXT_PACKAGE,String)
# define N_(String)  String

#else
# define _(String) String
# define N_(String) String
# define ngettext(Format1,Format2,N) Format1
# define textdomain(String) 
# define bindtextdomain(Domain,Directory)
#endif

#endif


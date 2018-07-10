#ifndef __MY_INTL_H__
# define __MY_INTL_H__

# ifdef ENABLE_NLS
#  include <libintl.h>
#  define _(String) gettext(String)
#  define N_(String)  String

# else
#  define _(String) String
#  define N_(String) String
#  define ngettext(Format1,Format2,N) Format1
#  define textdomain(String)
#  define bindtextdomain(Domain,Directory)
# endif
#endif

#if 0

/* NLS is disabled */

# define _(String) String
# define N_(String) String
# define textdomain(String) String
# define gettext(String) String
# define dgettext(Domain,String) String
# define dcgettext(Domain,String,Type) String
# define bindtextdomain(Domain,Directory) (Domain)

#endif

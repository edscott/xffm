#ifndef DEBUG_H
#define DEBUG_H
#include <stdio.h>
#define COMMENT(...) G_STMT_START{ (void)0; }G_STMT_END
#define NOOP(...) G_STMT_START{ (void)0; }G_STMT_END

#ifdef DEBUG

#include <sys/resource.h>
# define DBG(...)   \
    G_STMT_START{ \
	fprintf(stderr, "db>(%p):", g_thread_self());\
	fprintf(stderr, __VA_ARGS__); \
	fflush(stderr);\
    }G_STMT_END
# define ALERT(...) \
    G_STMT_START{ \
	gchar *text=g_strdup_printf (__VA_ARGS__); \
	g_warning(text); \
	g_free(text); \
    }G_STMT_END
#else
# define DBG(...)   G_STMT_START{ (void)0; }G_STMT_END
# define ALERT(...) G_STMT_START{\
    fprintf(stderr, "++ Warning: ");\
    fprintf(stderr, __VA_ARGS__); \
}G_STMT_END
#endif



#ifdef DEBUG_TRACE
#include <sys/resource.h>
# define THREAD_CREATE(X,Y,Z) \
    TRACE("THREAD CREATED %s (%p)\n", \
	    Z, \
	    rfm_thread_create(Z, X, Y, FALSE))
# define TRACE(...)   \
    G_STMT_START{ \
	fprintf(stderr, "tr>(%p):", g_thread_self());\
	fprintf(stderr, __VA_ARGS__); \
	fflush(stderr);\
    }G_STMT_END
#else
# define THREAD_CREATE(X,Y,Z) rfm_thread_create(Z, X, Y, FALSE)
# define TRACE(...)   G_STMT_START{ (void)0; }G_STMT_END
#endif

#endif


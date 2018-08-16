#ifndef DEBUG_H
#define DEBUG_H

#define NOOP(...) { (void)0; }

#ifdef DEBUG
# define DBG(...)   \
    { \
	fprintf(stderr, "DBG <tubo>:");\
	fprintf(stderr, __VA_ARGS__); \
	fflush(stderr);\
    }
#else
# define DBG(...)   { (void)0; }
#endif


#ifdef DEBUG_TRACE
# define TRACE(...)   \
    { \
	fprintf(stderr, "TRACE <tubo>:");\
	fprintf(stderr, __VA_ARGS__); \
	fflush(stderr);\
   }
#else
# define TRACE(...)   { (void)0; }
#endif

#endif

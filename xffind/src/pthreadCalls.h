/////////////////////////////////////////////////////////////////////////////
//              static thread functions                                    //
#ifndef PTHREADCALLS_H
#define PTHREADCALLS_H
////////////////////////////////////////////////////////////////////////////
void *print_f(void *);
void *print_i(void *);
void *print_d(void *);
void *print_e(void *);
void *print_s(void *);
void *print_sl(void *);
void *clear_text_buffer_f(void *);
gboolean context_function_f(gpointer data);
#endif

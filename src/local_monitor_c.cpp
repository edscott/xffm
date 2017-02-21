#include <string.h>
#include <gio/gio.h>

#include "local_monitor_c.hpp"
gchar *
decode (GFileMonitorEvent ev)
{
    gchar *fmt = (gchar *)g_malloc0 (1024);
    gint caret = 0;
#define dc(x) \
    case G_FILE_MONITOR_EVENT_##x: \
        strcat(fmt, #x); caret += strlen(#x); fmt[caret] = ':';  \
    break;

    switch (ev) {
        dc(CHANGED);
        dc(CHANGES_DONE_HINT);
        dc(DELETED);
        dc(CREATED);
        dc(ATTRIBUTE_CHANGED);
        dc(PRE_UNMOUNT);
        dc(UNMOUNTED);
        dc(MOVED);
    }
#undef dc
    return fmt;
}

void
monitor_f (GFileMonitor      *mon,
          GFile             *first,
          GFile             *second,
          GFileMonitorEvent  event,
          gpointer           data)
{
    char *msg = decode (event);
   
#define fn(x) ((x) ? g_file_get_basename (x) : "--")
    fprintf (stderr, "Received event %s (code %d), first file \"%s\", second file \"%s\"\n",
              msg, event, fn(first), fn(second));
#undef fn

    g_free (msg);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

local_monitor_c::local_monitor_c(){
    fprintf(stderr, "local_monitor_c::local_monitor_c()\n");
    cancellable = g_cancellable_new ();
    gfile = NULL;
    monitor = NULL;
}

local_monitor_c::~local_monitor_c(void){
    fprintf(stderr, "local_monitor_c::~local_monitor_c()\n");
    stop_monitor();
    //g_cancellable_cancel (cancellable);
    //g_object_unref(cancellable);
    if (gfile) g_object_unref(gfile);
    if (monitor) g_object_unref(monitor);
}

void
local_monitor_c::start_monitor(const gchar *data){
    fprintf(stderr, "local_monitor_c::start_monitor...\n");
    if (gfile) g_object_unref(gfile);
    gfile = g_file_new_for_path (data);
    error=NULL;
    if (monitor) g_object_unref(monitor);
    monitor = g_file_monitor_directory (gfile, G_FILE_MONITOR_WATCH_MOVES, cancellable,&error);
    if (error){
        fprintf(stderr, "g_file_monitor_directory(%s) failed: %s\n",
                data, error->message);
        g_object_unref(gfile);
        gfile=NULL;
        return;
    }
    g_signal_connect (monitor, "changed", 
            G_CALLBACK (monitor_f), (void *)this);
}

void 
local_monitor_c::stop_monitor(void){
    fprintf(stderr, "* stop_monitor...\n");
    g_file_monitor_cancel(monitor);
}

#ifndef LOCAL_MONITOR_C_HPP
#define LOCAL_MONITOR_C_HPP


#include "xffm+.h"
class local_monitor_c {
    public:
        local_monitor_c(void);
        ~local_monitor_c(void);
    protected:
        void start_monitor(const gchar *);
        void stop_monitor(void);
    private:
        GCancellable *cancellable;
        GFile *gfile;
        GFileMonitor *monitor;
        GError *error;
};

#endif

#ifndef ROOTMONITOR_HH
#define ROOTMONITOR_HH
namespace xf {

  template <class Type> RootMonitor {

    char *sum_;
    public:
      RootMonitor(GridView<LocalDir> *gridview){
        checkSumMnt(&sum_);
        pthread_t thread;
        pthread_create(&thread, NULL, threadF1, (void *)mountArg_);
        pthread_detach(thread);

      }


  };

}
#endif

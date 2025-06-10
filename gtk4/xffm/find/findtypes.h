#ifndef FINDTYPES_HH
#define FINDTYPES_HH
#define FILTER_HISTORY g_get_user_data_dir(),"xffm4","xffind.filter",NULL
#define GREP_HISTORY g_get_user_data_dir(),"xffm4","xffind.grep",NULL
#define PATH_HISTORY g_get_user_data_dir(),"xffm4","xffind.path",NULL

typedef struct radio_t {
    GtkBox *box;
    GtkCheckButton *toggle[5];
} radio_t;

typedef struct fgrData_t{
    GtkBox *mainBox;
    pid_t pid;
    gint resultLimit;
    gint resultLimitCounter;
    GSList *findList;
    gchar **argument;
    gboolean done;
    void *object;
}fgrData_t;

typedef struct opt_t{
  const char *text;
  const char *id;
  gboolean defaultValue;
} opt_t;


#endif

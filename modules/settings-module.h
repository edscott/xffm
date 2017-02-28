#ifndef SETTINGS_MODULE_H
# define SETTINGS_MODULE_H

# ifdef SETTINGS_MODULE_C

#  define McsPluginInitResult int
#  define MCS_PLUGIN_INIT_OK 1

#  define SHM_STRING_LENGTH 32
#  define SHM_VALUE_LENGTH 255

typedef struct McsManager {
    gint shm;
    gchar shm_name[64];
} McsManager;

typedef struct McsPlugin {
    McsManager *manager;
    gchar *plugin_name;
    gchar *caption;
    gchar *shm_settings_file;
    void *(*run_dialog) (struct McsPlugin * mp);
    gint shm;
    void *m;
} McsPlugin;

typedef struct mcs_data_t {
    gchar *v_string;
} mcs_data_t;

typedef struct McsSetting {
    mcs_data_t data;
} McsSetting;

typedef struct mcs_shm_data_t {
    char name[SHM_STRING_LENGTH];
    char value[SHM_VALUE_LENGTH];
} mcs_shm_data_t;

typedef struct mcs_shm_t {
    int serial;
    mcs_shm_data_t data[RFM_OPTIONS];
} mcs_shm_t;

static
void mcs_manager_notify (McsManager *, const gchar *);

static
void mcs_manager_set_string (McsManager *, const gchar *, const gchar *, const gchar *);

typedef struct {
    gchar *name;
    gchar *value;
} Options;

static Options rfm_options[RFM_OPTIONS];

#  define MCS_PLUGIN_CHECK_INIT
# endif
#endif

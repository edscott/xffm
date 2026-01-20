#ifndef ECRYPTFS_I
#define ECRYPTFS_I

#define EFS_KEY_FILE                g_get_user_config_dir(),"xffm+","efs.ini"

#define EFS_AUTHORIZATION _("Encryption Options")
#define EFS_TIP _("Encrypt Files")
#define ECRYPTFS_SIG _("Mount ecrypt signature")
#define ECRYPTFS_FNEK_SIG _("Filename ecrypt signature")
#define EFS_INFO1 _("Ecrypt Filesystem (EFS)")
#define EFS_INFO2 _("New EFS Link")
#define EFS_REMOTE_PATH _("Encrypted directory")
#define EPS_ENABLE_FILENAME_CRYPTO _("Encrypt filenames")
#define EPS_REQUIRES_SIGNATURE _("Requires ecryptfs signature")
#define EPS_PASSTHROUGH _("Plaintext passthrough")

#define FUSE_BROADBAND  _("local filesystem")
#define FUSE_COMPUTER _("Computer Name:")
#define FUSE_REMOTE_PATH  _("Remote Path")
#define FUSE_MOUNT_POINT  _("Mount point:")
#define FUSE_URL  _("URL")
#define FUSE_MONITOR  _("Enable file monitoring")
#define FUSE_CAUTION  _("Caution")
#define FUSE_LOGIN  _("Username:")
#define FUSE_SECURE_SHELL_KEY  _("Secure Shell Key")
#define FUSE_ALLOW_EMPTY_PASSPHRASE  _("Allow empty private key passphrase")
#define FUSE_PASSPHRASE  _("Enter your Secure Shell passphrase:")


#define FLAG_KEY(x)  g_strdup_printf("FLAG_%d", x)
namespace xf{
//#define GroupOptions group_options_t
typedef struct group_options_t {
    gint sensitive;
    const gchar *flag;
    const gchar *id;
    const gchar *text;
    const gchar *entry;
    const gchar *tip;
}group_options_t;

static gchar **mount_option_keys = NULL;
static  group_options_t mount_options[]={
//linux options:
    {
        .sensitive = 1, .flag = "-r", .id = N_("Mount file system readonly."), 
        .text = NULL, 
        .entry = NULL, .tip = NULL
    },
    {
        .sensitive = 1, .flag = "-v", .id = N_("Be verbose."), 
        .text = NULL, 
        .entry = NULL, .tip = NULL
    },
    {
        .sensitive = 1, .flag = "-V", .id = N_("Print version."), 
        .text = NULL, 
        .entry = NULL, .tip = NULL
    },
    {
        .sensitive = 2, .flag = "-w", .id = N_("Mount file system read-write."), 
        .text = NULL, 
        .entry = NULL, .tip = NULL
    },
//#ifdef THIS_IS_LINUX
    {
        .sensitive = 1, .flag = "-f", .id = N_("Fake mount."), 
        .text = NULL,  
        .entry = NULL, 
        .tip = N_("Don't actually call the mount system call.")
    },
    {
        .sensitive = 1, .flag = "-n", .id = N_("Do  not  update  /etc/mtab."), 
        .text = NULL, 
        .entry = NULL,
        .tip = 
N_("By default, an entry is created in/etc/mtab for every mounted file system.\n Use this option to skip making an entry.")
    },
    {
        .sensitive = 1, .flag = "-s", .id = N_("Tolerate sloppy options."), 
        .text = NULL,
        .entry = NULL,
        .tip = N_("Tolerate sloppy mount options rather than fail.") 
    },
//#endif
    {
        .sensitive = 1, .flag = "-h", .id = N_("Print help message."), 
        .text = NULL, 
        .entry = NULL, .tip = NULL
    },
    
    {.sensitive = 1, NULL, NULL, NULL, NULL, NULL}
};

static gchar **efs_option_keys = NULL;
static  group_options_t efs_options[]={
    {
        .sensitive = 2, .flag = "", .id = "ecryptfs_cipher=", 
        .text = "aes | blowfish | des3_ede | cast6 | cast5",
        .entry = "aes",
        .tip = N_("Specify the symmetric cipher to be used on a per file basis. ")
    },
    
    {
        .sensitive = 2, .flag = "", .id = "ecryptfs_key_bytes=", 
        .text = NULL, 
        .entry = "16",
        .tip = N_("Specify  the keysize to be used with the selected cipher. If the cipher only has one keysize the keysize  does  not  need  to  be specified.\n\n\taes: blocksize = 16; min keysize = 16; max keysize = 32\n\tblowfish: blocksize = 16; min keysize = 16; max keysize = 56\n\tdes3_ede: blocksize = 8; min keysize = 24; max keysize = 24\n\tcast6: blocksize = 16; min keysize = 16; max keysize = 32\n\tcast5: blocksize = 8; min keysize = 5; max keysize = 16\n\t")
    },
    {
        .sensitive = 3, .flag = "", .id = "ecryptfs_passthrough=", 
        .text = NULL, 
        .entry = "no",
        .tip = N_("Allows for non-eCryptfs files to be read and written from within an eCryptfs mount. This option is turned off by default.")
    },
        
    {
        .sensitive = 3, .flag = "", .id = "ecryptfs_enable_filename_crypto=", 
        .text = NULL, 
        .entry = "no",
        .tip = N_("Specify whether filename encryption should be enabled.  If  not, the  mount  helper  will  not  prompt  the user for the filename encryption key signature.\nIf you enabled filename encryption then pass an additional mount option: ecryptfs_fnek_sig=XY, where XY is the same signature you provide with the ecryptfs_sig option.")
    },
    {
        .sensitive = 1, .flag = "", .id = "ecryptfs_sig=", 
        .text = N_("fekek_sig"), 
        .entry = "",
        .tip = N_("Specify  the  signature  of the mount wide authentication token. The authentication token must be in the  kernel  keyring  before the  mount  is performed. \necryptfs-manager or the eCryptfs mount helper can be used to construct the authentication token and add it to the keyring prior to mounting.") 
    },
    
    {
        .sensitive = 1, .flag = "", .id = "ecryptfs_fnek_sig=", 
        .text = N_("fnek_sig"), 
        .entry ="",
        .tip = N_("Specify  the  signature  of  the mount wide authentication token used for filename crypto. The authentication must be in the kernel keyring before mounting.")
    },
    
    
    
    {
        .sensitive = -1, .flag = "", .id = "no_sig_cache", 
        .text = NULL, 
        .entry = NULL,
        .tip = N_("Do not check the mount key signature against the values  in  the user's  ~/.ecryptfs/sig-cache.txt  file. This is useful for such things as non-interactive  setup  scripts,  so  that  the  mount helper  does  not stop and prompt the user in the event that the key sig is not in the cache.")
    },
    
    {
        .sensitive = 1, .flag = "", .id = "ecryptfs_encrypted_view", 
        .text = NULL, 
        .entry = NULL,
        .tip = N_("This option provides a unified  encrypted  file  format  of  the eCryptfs  files in the lower mount point.  Currently, it is only useful if the lower mount point contains files with the metadata stored in the extended attribute.  Upon a file read in the upper mount point, the encrypted version of the file will be presented with  the  metadata  in  the  file  header instead of the xattr.  Files cannot be opened for writing when this option is enabled.")
    },
    
    {
        .sensitive = 1, .flag = "", .id = "ecryptfs_xattr", 
        .text = NULL, 
        .entry = NULL,
        .tip = N_("Store the metadata in the extended attribute of the lower  files rather than the header region of the lower files.")
    },
    
    {
        .sensitive = 1, .flag = "", .id = "verbose", 
        .text = NULL, 
        .entry = NULL,
        .tip = N_("Log  ecryptfs  information  to  /var/log/messages.   Do  not run eCryptfs in verbose-mode unless you are doing so  for  the  sole purpose  of development, since secret values will be written out to the system log in that case.")
    },

    
    {
        .sensitive = -1, .flag = "", .id = "verbosity=", 
        .text = NULL, 
        .entry = "1",
        .tip = N_("If verbosity=1, the mount helper will ask you for missing values (default).  Otherwise, if verbosity=0, it will not ask for missing values and will fail if required values are omitted.  ")
    },
    
    {
        .sensitive = 0, .flag = "", .id = "passphrase_passwd=", 
        .text = N_("passphrase"), 
        .entry = "",
        .tip = N_("The actual password is passphrase. Since the password is visible to utilities (like ps under Unix) this form should only be  used where security is not important.")
    },
    
    {
        .sensitive = 1, .flag = "", .id = "passphrase_passwd_file=", 
        .text = N_("filename"), 
        .entry = "",
        .tip = N_("The    password   should   be   specified   in   a   file   with passwd=(passphrase). It is highly reccomended that the  file  be stored on a secure medium such as a personal usb key.")
    },
    
    {
        .sensitive = 0, .flag = "", .id = "passphrase_passwd_fd=", 
        .text = N_(""), 
        .entry = "",
        .tip = N_("The password is specified through the specified file descriptor.")
    },
    
    {
        .sensitive = 1, .flag = "", .id = "passphrase_salt=", 
        .text = "hex value", 
        .entry = "",
        .tip = N_("The salt should be specified as a 16 digit hex value.  ")
    },
    
    {
        .sensitive = 1, .flag = "", .id = "openssl_keyfile=", 
        .text = N_("filename"), 
        .entry = "",
        .tip = N_("The   password   should   be   specified   in   a   file    with openssl_passwd=(openssl-password). It is highly reccomended that the file be stored on a secure medium such  as  a  personal  usb key.")
    },
    
    
    
    {
        .sensitive = 1, .flag = "", .id = "openssl_passwd_file=", 
        .text = N_("filename"), 
        .entry = "",
        .tip = N_("The   password   should   be   specified   in   a   file    with openssl_passwd=(openssl-password). It is highly reccomended that the file be stored on a secure medium such  as  a  personal  usb key.")
    },
    {
        .sensitive = 0, .flag = "", .id = "openssl_passwd_fd=", 
        .text = N_("file descriptor"), 
        .entry = "",
        .tip = N_("The password is specified through the specified file descriptor.")
    },
    {
        .sensitive = 0, .flag = "", .id = "openssl_passwd=", 
        .text = N_("password"), 
        .entry = "",
        .tip = N_("The  password  can  be  specified on the command line. Since the password is visible in the process list,  it  is  highly  recommended to use this option only for testing purposes.")
    },
    
    {.sensitive = 1, NULL, NULL, NULL, NULL, NULL}
};




}
#endif

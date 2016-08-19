
#ifndef __MIME_H__
# define __MIME_H__

# define MIME_hashtype(x) ((gchar *)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "find_mimetype_in_hash"))

# define MIME_file(x) ((gchar *)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_file"))

# define MIME_command_icon(x) ((const gchar *)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_command_icon"))

# define MIME_command_text(x) ((const gchar *)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_command_text"))
# define MIME_command_text2(x) ((const gchar *)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_command_text2"))
# define MIME_command_output(x) (GPOINTER_TO_INT(rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_command_output")))
# define MIME_command_output_ext(x) ((const gchar *)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_command_output_ext"))

# define MIME_command(x) ((gchar *)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_command"))
# define MIME_apps(x) ((gchar **)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_apps"))
# define MIME_magic(x) ((gchar *)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_magic"))
# define MIME_encoding(x) ((gchar *)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_encoding"))
//# define MIME_typeinfo(x) ((gchar *)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_typeinfo"))
# define MIME_is_valid_command(x) (GPOINTER_TO_INT(rfm_natural(RFM_MODULE_DIR, "mime", (void *)x, "mime_is_valid_command")))
# define MIME_mk_terminal_line(x) ((gchar *)rfm_natural(RFM_MODULE_DIR, "mime", (void *)(x), "mime_mk_terminal_line"))

# define MIME_type(x, y) ((gchar *)rfm_rational(RFM_MODULE_DIR, "mime", (void *)(x), (void *)(y), "mime_type"))
# define MIME_add(x, y) (rfm_rational(RFM_MODULE_DIR, "mime", (void *)(x), (void *)(y), "mime_add"))
# define MIME_mk_command_line(x, y) ((gchar *)rfm_rational(RFM_MODULE_DIR, "mime", (void *)(x), (void *)(y), "mime_mk_command_line"))

/****************************************************************/
/**
 * mime_get_type:
 * @file: NULL terminated string with full path of file to query.
 * @flags: bitwise TRY_STAT and/or TRY_MAGIC or zero.
 *
 * Returns: string with the mime type, or "undetermined type"
 * if no mime type is associated.
 *
 *
G_MODULE_EXPORT
gchar *
mime_type(const gchar *file, struct stat *st_p);
*/

/****************************************************************/
/**
 * mime_command:
 * @type: Mimetype to query for associated application
 *
 * As there  may be more than one application associated,
 * use mime_apps() if you want all of them. This function
 * returns the default application, which is the last associated
 * application entered into the association hash.
 * 
 * Returns: string with the default application for @file, or
 * NULL if there is no application associated. * 
 *
G_MODULE_EXPORT gchar *mime_command (gchar * type);

****************************************************************/
/**
 * mime_apps:
 * @file: Path of file to query for associated applications.
 *
 * Returns: array of strings with associated applications. The last
 * element of the array is a NULL pointer. If no applications are
 * associated, returns NULL.
 *
 *
G_MODULE_EXPORT gchar **mime_apps (gchar * type);

****************************************************************/
/**
 * mk_command_line:
 * @command_fmt: Command string to execute.
 * @path: Path to file to send with command.
 * @interm: TRUE if to execute within a terminal window.
 * @hold: TRUE if to hold the output of a terminal window on 
 * completion of command.
 * 
 * A command string may have %s to indicate the position where the
 * path is to be inserted. Otherwise the path is appended.
 *
 * Returns: A string with the command line constructed, suitable for
 * a g_spawn_async() or similar.
 * 
 **/

/****************************************************************/
/**
 * mime_typeinfo:
 * @type: the mimetype for which information is being requested
 *
 * Returns: A string with mimetype information. 
 * 
 **/

/****************************************************************/
/**
 * mime_add:
 * @file: File to associate command with.
 * @command: Command to associate.
 *
 * This associates a command with a file type, determined by the file
 * extension. When this command is used, the associated command becomes
 * the default command for the file type while the module remains 
 * loaded. 
 *
 *
 **/

/****************************************************************/
/**
 * is_valid_command:
 * @cmd_fmt: command to check for.
 *
 * Checks whether @cmd_fmt is in $PATH and whether it can be executed.
 *
 *
 * Returns: TRUE if @cmd_fmt can be executed, FALSE otherwise.
 **/

#endif

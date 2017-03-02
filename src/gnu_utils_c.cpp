#include <string.h>
#include "gnu_utils_c.hpp"
/**
 * NAME
 *      cp - copy files and directories
 *
 * SYNOPSIS
 *       cp [OPTION]... [-T] SOURCE DEST        (1st form)
 *       cp [OPTION]... SOURCE... DIRECTORY     (2nd form)
 *       cp [OPTION]... -t DIRECTORY SOURCE...  (3rd form)
 * 
 *       Form 1:
 *           I don't quite understand how this one works, if
 *           it actually works different.
 *       Form 2:
 *          This form duplicates form 3, which is the preferred
 *          method for xffm+.
 *       Form 3:
 *          This is preferred..
 *
 *
 *           
 *
 *
**/
gboolean 
gnu_utils_c::cp(GList *src_list, const gchar *tgt, const gchar *options){
    gchar *command = get_command("cp", src_list, tgt, options);
    if (!command) return FALSE;
    execute_command(command); //async, reference is passed to thread function.
    return TRUE;
}
                            /* end cp */

/**
 * NAME
 *      mv - move (rename) files
 *
 *SYNOPSIS
 *      mv [OPTION]... [-T] SOURCE DEST
 *      mv [OPTION]... SOURCE... DIRECTORY
 *      mv [OPTION]... -t DIRECTORY SOURCE...
 *
 *       Form 1:
 *           I don't quite understand how this one works, if
 *           it actually works different.
 *       Form 2:
 *          This form duplicates form 3, which is the preferred
 *          method for xffm+.
 *       Form 3:
 *          This is preferred when we have a single target 
 *          drop or paste event.
 *          
 *
**/ 
gboolean 
gnu_utils_c::mv(GList *src_list, const gchar *tgt, const gchar *options){
    gchar *command = get_command("mv", src_list, tgt, options);
    if (!command) return FALSE;
    execute_command(command); //async, reference is passed to thread function.
}
                            /* end mv */

/**
 * NAME
 *      ln - make links between files
 *
 * SYNOPSIS
 *      ln [OPTION]... [-T] TARGET LINK_NAME   (1st form)
 *      ln [OPTION]... TARGET                  (2nd form)
 *      ln [OPTION]... TARGET... DIRECTORY     (3rd form)
 *      ln [OPTION]... -t DIRECTORY TARGET...  (4th form)
 *
 *      Form 1:
 *          This is preferred when we have a single target
 *          with the link to be created in the same directory
 *          and in this case we must create a default
 *          LINK_NAME which the user can modify at will.
 *      Form 2:
 *          Name will be automatically set
 *          by gnu-ln behavior.When TARGET already exists, fall
 *          back to form 1.
 *      Form 3:
 *          This form duplicates form 4, which is the preferred
 *          method for xffm+.
 *      Form 4:
 *          This is the preferred method for drop or paste
 *          with multiple targets. .
 *
 *
 **/
gboolean 
gnu_utils_c::ln(GList *src_list, const gchar *tgt, const gchar *options){
    if (!src_list) return FALSE;
    gchar *command = get_command("ln", src_list, tgt, options);
    if (!command) return FALSE;
    execute_command(command); //async, reference is passed to thread function.
}
                            /* end ln */

/**
 * NAME
 *      rm - remove files or directories
 * 
 * SYNOPSIS
 *      rm [OPTION]... [FILE]...
 *
**/      
gboolean 
gnu_utils_c::rm(GList *src_list, const gchar *options){
    gchar *command = get_command("rm", src_list, NULL, options);
    if (!command) return FALSE;
    execute_command(command); //async, reference is passed to thread function.
}
                            /* end rm */
/**
 * NAME
 *      shred - overwrite a file to hide its contents, and optionally delete it.
 *      
 * SYNOPSIS
 *      shred [OPTION]... FILE...
 *
**/ 

gboolean 
gnu_utils_c::shred(GList *src_list, const gchar *options){
    gchar *command = get_command("shred", src_list, NULL, options);
    if (!command) return FALSE;
    execute_command(command); //async, reference is passed to thread function.
}
                            /*emd shred */

gchar * 
gnu_utils_c::get_command(const gchar *c, GList *src_list, const gchar *tgt, const gchar *options){
    fprintf(stderr, "%s tgt= %s \n",c, tgt);
    if (!src_list ) return NULL;
    gchar *command;
    if (strstr(options, " -t ")){
        // We allow user to specify a different target directory.
        command = g_strconcat(c, " ", options, NULL);
    } else if (tgt) {
        command = g_strconcat(c, " ", options, " -t ", tgt, NULL);
    } else {
        command = g_strconcat(c, " ", options, NULL);
    }
    GList *l = src_list;
    for (;l && l->data; l=l->next)
    {
        fprintf(stderr, "file: %s\n", (gchar *)l->data);
        gchar *g = g_strconcat(command, " ", (const gchar *)l->data, NULL);
        g_free(command);
        command=g;
    }
    return command;
}

gchar * 
gnu_utils_c::execute_command(gchar *command){
    fprintf(stderr, "Now is time to execute:\n\'%s\'\n", command);
    // thread wait function will return with status of command.
}

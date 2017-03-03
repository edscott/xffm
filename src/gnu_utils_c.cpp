#include "gnu_utils_c.hpp"
#include "view_c.hpp"

gnu_utils_c::gnu_utils_c(void ){}

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
gnu_utils_c::cp(void *view_p, GList *src_list, const gchar *tgt, const gchar *options){
#if 0
    gchar *command = get_command("cp", src_list, tgt, options);
    if (!command) return FALSE;
    execute_command(view_p, command);  
    g_free(command);
#else
    gchar **arguments = get_command_argv("cp", src_list, tgt, options);
    if (!arguments) return FALSE;
    execute_command(view_p, arguments);  
    // When to g_strfreev ?
    

#endif

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
gnu_utils_c::mv(void *view_p, GList *src_list, const gchar *tgt, const gchar *options){
    gchar *command = get_command("mv", src_list, tgt, options);
    if (!command) return FALSE;
    execute_command(view_p, command);  
    g_free(command);

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
gnu_utils_c::ln(void *view_p, GList *src_list, const gchar *tgt, const gchar *options){
    if (!src_list) return FALSE;
    gchar *command = get_command("ln", src_list, tgt, options);
    if (!command) return FALSE;
    execute_command(view_p, command);  
    g_free(command);
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
gnu_utils_c::rm(void *view_p, GList *src_list, const gchar *options){
    gchar *command = get_command("rm", src_list, NULL, options);
    if (!command) return FALSE;
    execute_command(view_p, command); 
    g_free(command);
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
gnu_utils_c::shred(void *view_p, GList *src_list, const gchar *options){
    gchar *command = get_command("shred", src_list, NULL, options);
    if (!command) return FALSE;
    execute_command(view_p, command); 
    g_free(command);

}
                            /*end shred */

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


gchar **
gnu_utils_c::get_command_argv(const gchar *c, GList *src_list, const gchar *tgt, const gchar *options){
    // FIXME: we need a non null strsplit here
    gchar **option_v = g_strsplit(options, " ", -1);
    // count options
    // vector size = options_count + 1(cp) + 1(-t) +1(tgt) + g_list_length(src_list) +1;
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
gnu_utils_c::execute_command(void *data,  gchar *command){
    fprintf(stderr, "Now is time to execute:\n\'%s\'\n", command);
    // execute in lpterm associated to view_p
    view_c *view_p =(view_c *)data;
    // thread wait function will return with status of command.
    view_p->get_lpterm_p()->thread_run(command);
}

// This would be preferred.
gchar * 
gnu_utils_c::execute_command(void *data,  gchar **arguments){
    fprintf(stderr, "Now is time to execute:\n\'%s\'\n", command);
    // execute in lpterm associated to view_p
    view_c *view_p =(view_c *)data;
    // thread wait function will return with status of command.
    gchar *arguments[]={NULL};
    view_p->get_lpterm_p()->thread_run(arguments);
}

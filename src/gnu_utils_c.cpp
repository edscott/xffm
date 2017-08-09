#include "gnu_utils_c.hpp"
#include "view_c.hpp"


gchar **
gnu_utils_c::non_empty_strsplit(const gchar *input, const gchar *token){
    if (!input || !token || !strstr(input, token)) return NULL;        
    gchar *c = g_strdup(input);
    if (strchr(c, '\n')) *strchr(c, '\n') = 0;
    gchar **src = g_strsplit(c, token, -1);
    g_free(c);
    if (!src) return NULL;
    gchar **r= (gchar **)malloc((g_strv_length(src)+1)*sizeof(gchar *)); 
    if (!r) return NULL;
    memset (r, 0, (g_strv_length(src)+1)*sizeof(gchar *));
    gchar **p, **tgt;
    for (p=src, tgt=r; p && *p; p++) {
        if (strlen(*p)){
            *tgt = g_strdup(*p);
            tgt++;
        }
    }
    g_strfreev(src);
    return r;   
}

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
    gchar **arguments = get_command_argv("cp", src_list, tgt, options);
    if (!arguments) return FALSE;
    execute_command(view_p, (const gchar **)arguments);  
    g_strfreev(arguments);
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
    gchar **arguments = get_command_argv("mv", src_list, tgt, options);
    if (!arguments) return FALSE;
    execute_command(view_p, (const gchar **)arguments);  
    g_strfreev(arguments);
    return TRUE;
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
    gchar **arguments = get_command_argv("ln", src_list, tgt, options);
    if (!arguments) return FALSE;
    execute_command(view_p, (const gchar **)arguments);  
    g_strfreev(arguments);
    return TRUE;
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
    gchar **arguments = get_command_argv("rm", src_list, NULL, options);
    if (!arguments) return FALSE;
    execute_command(view_p, (const gchar **)arguments);  
    g_strfreev(arguments);
    return TRUE;
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
    gchar **arguments = get_command_argv("shred", src_list, NULL, options);
    if (!arguments) return FALSE;
    execute_command(view_p, (const gchar **)arguments);  
    g_strfreev(arguments);
    return TRUE;
}
                            /*end shred */



gchar **
gnu_utils_c::get_command_argv(const gchar *cmd, GList *src_list, const gchar *tgt, const gchar *options){
    gint vsize = g_list_length(src_list);
    gchar **option_v = non_empty_strsplit(options, " ");
    gchar **p=option_v;
    while (p && *p) {
        vsize++;
        p++;
    }
    // 4 = cmd -t target NULL
    vsize += 5; 
    gchar **argv = (gchar **)calloc(vsize, sizeof(gchar *));
    gint i = 0;
    argv[i++] = g_strdup(cmd);
    if (tgt){
        argv[i++] = g_strdup("-t");
        argv[i++] = g_strdup(tgt);
    }
    gchar **q = argv+i;
    p=option_v;
    while (p && *p) {
        *q = g_strdup(*p);
        p++;
        q++;
    }
    g_strfreev(option_v);
    GList *l = src_list;
    while (l && l->data){
        *q = g_strdup((gchar *)l->data);
        q++;
        l = l->next;
    }
    return argv;
}


// This would be preferred.
pid_t 
gnu_utils_c::execute_command(void *data,  const gchar **arguments){
    fprintf(stderr, "Now is time to execute:\n\'%s\'\n", arguments[0]);
    // execute in lpterm associated to view_p
    view_c *view_p =(view_c *)data;
    // thread wait function will return with status of command.
    return view_p->get_lpterm_p()->thread_run(arguments);
}

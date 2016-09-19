/*
 * Copyright (C) 2002-2012 Edscott Wilson Garcia
 * EMail: edscott@users.sf.net
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "rodent.h"

#include "rfm_modules.h"
/* this should be first 2 lines after headers: */
G_MODULE_EXPORT LIBRFM_MODULE 

#define RENAME_CASO 	0
#define DUPLICATE_CASO 	1
#define SYMLINK_CASO 	2

#include "primary-options.h"
#include "dialogs.i"
#include "touch.i"
#include "ls.i"
#include "rm.i"
#include "cp.i"
#include "rename_entry.i"
#include "callbacks.i"
/* Public:
 *    All these functions are processed by main thread. */

void *
get_menu_callback(void *p){
    if (!p)return (void *)menu_callback_v;
    enum menu_enum_t menu_enum = GPOINTER_TO_INT(p);
    return get_menu_callback_p(menu_enum);
}

void *
set_menu_callback_keybind(void *p, void *q, void *r){
    enum menu_enum_t menu_enum = GPOINTER_TO_INT(p);
    RodentCallback *menu_callback_p = (RodentCallback *)
	get_menu_callback(GINT_TO_POINTER(menu_enum));
    if (!menu_callback_p) {
        DBG("set_menu_callback_keybind(): !menu_callback_p\n");
        return NULL;
    }
    errno=0;
    if (q) {
	long l = strtol((gchar *)q, NULL, 0);
	if (errno){
	    DBG("strtol(): %s\n", strerror(errno));
	    menu_callback_p->key = 0;
	    errno=0;
	} else menu_callback_p->key = (gint) l;
    } else menu_callback_p->key = 0;
    if (r) {
	long l = strtol((gchar *)r, NULL, 0);
	if (errno){
	    DBG("strtol(): %s\n", strerror(errno));
	    menu_callback_p->mask = 0;
	} else menu_callback_p->mask = (gint) l;
    } else menu_callback_p->mask = 0;
    return GINT_TO_POINTER(1);
}

static gboolean 
toggle_test (view_t *view_p, enum menu_enum_t menu_enum, GtkWidget *menuitem){
    if (!IS_TOGGLE(menu_enum)) return TRUE;
#ifdef DEBUG_TRACE
    RodentCallback *menu_callback_p = get_menu_callback(GINT_TO_POINTER(menu_enum));
    TRACE( "main popupmenu activate %s (%d)\n", menu_callback_p->string, menu_enum);
#endif
    // If popup menu not mapped, ignore.
    // We should not ignore keybindings, but keybindings do not
    // enter the callback with this function.
    rfm_global_t *rfm_global_p = rfm_global();
    if (g_object_get_data(G_OBJECT(rfm_global_p->window), "popup_mapped")){
	TRACE(  "***** main popupmenu activate %d ignored (popup_mapped).\n",
		menu_enum);
	    return FALSE;
    }
    TRACE(  "***** main popupmenu activate %d ok (not popup_mapped).\n",
		menu_enum);
	
    NOOP( "toggle test for %d (%s)\n", menu_enum, menu_callback_p->string);
    if (IS_RADIO(menu_enum)){
	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem))) {
	    NOOP(stderr, "IS_RADIO and is not active...\n");
	    return FALSE;
	}
	NOOP(stderr, "IS_RADIO and is active...\n");
    } else {
	gboolean is_active = FALSE;
	gboolean active = 
	    gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	// Plain check button.
	switch (menu_enum){
	    case PREVIEW_TOGGLED:
		is_active =	(view_p->flags.preferences & __SHOW_IMAGES);
		break;
	    case HIDDEN_TOGGLED:
		is_active =	(view_p->flags.preferences & __SHOW_HIDDEN);
		break;
	    case BACKUP_TOGGLED:
		is_active =	(view_p->flags.preferences & __SHOWS_BACKUP);
		break;
	    case CASESORT_TOGGLED:
		is_active =	(view_p->flags.preferences & __CASE_INSENSITIVE);
		break;
	    default:
		DBG("Toggle not considered in case, callbacks.c:callback()\n");
	}
	if (active && is_active) return FALSE;
	else if (!active && !is_active) return FALSE;
	if (menu_enum == PREVIEW_TOGGLED) {
	    view_p->flags.preferences |= __INVALIDATE_ICONS;
	}
    }
    return TRUE;
}

static void
exec_callback(view_t *view_p, enum menu_enum_t menu_enum, GtkWidget *menuitem){
    TRACE("** exec_callback\n");
    void **arg = (void **)malloc(3*sizeof(void *));
    arg[0] = GINT_TO_POINTER(menu_enum);
    arg[1] = menuitem;
    arg[2] = view_p;
    rfm_view_thread_create(view_p, threaded_callback, arg, "threaded_callback");
}

void *
callback(enum menu_enum_t menu_enum, GtkWidget *menuitem){
    widgets_t *widgets_p = rfm_get_widget ("widgets_p");
    view_t *view_p = widgets_p->view_p;
    NOOP(stderr, "IS_TOGGLE(%d)=%d\n", menu_enum, IS_TOGGLE(menu_enum));

    if (menuitem && !toggle_test(view_p, menu_enum, menuitem)) return NULL;
    TRACE("callback: exec_callback\n");
    exec_callback(view_p, menu_enum, menuitem);
    
    return GINT_TO_POINTER(1);
}



/////////////////////////////////////////////////////////////////////////////

void *
key_callback(void *p1, void *p2){
    guint keyval = GPOINTER_TO_UINT(p1);
    // Reduce mask to usable state (no caps/num lock or other stuff)
    guint state = GPOINTER_TO_UINT(p2) & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_MOD1_MASK|GDK_MOD5_MASK);
    RodentCallback *p = menu_callback_v;
    for (; p && p->function_id >= 0; p++){
	if (keyval != p->key) continue;
	gboolean state_ok=FALSE;
	if (p->mask==0 && state==0) state_ok = TRUE;
	if (state & GDK_SHIFT_MASK){
	    guint s = state & (GDK_SHIFT_MASK ^ 0xffffffff);
            if (s & p->mask) state_ok = TRUE;
	} else {
	    if (state & p->mask) state_ok = TRUE;
	}
	if (state_ok){
	    NOOP( "state & p->mask = %d\n", state & p->mask);
	    NOOP( "keyval = %d p->key = %d p->mask=%d  state = %d (non-reduced: 0x%x), logic=%d\n", 
		    keyval, p->key, p->mask, state, GPOINTER_TO_INT(p2),
		(p->mask==0 || (state & p->mask)));

	    widgets_t *widgets_p = rfm_get_widget ("widgets_p");
	    view_t *view_p = widgets_p->view_p;
	    if (p->function_id == MENU_ACTIVATE){
		// This does not create a new thread
		GdkEventButton  event;
		event.type = GDK_BUTTON_PRESS;
		event.time = gtk_get_current_event_time();
		event.button = 3;
		rodent_pop_menu ("main_popup_menu", &event);
	    } else {
		TRACE("keycallback: exec_callback\n");
		// This creates a thread
                // Mouse event is not valid here, so we set
                // selected_p to null.
                view_p->mouse_event.selected_p = NULL;
		exec_callback(view_p, p->function_id, NULL);
		//callback(p->function_id, NULL);
	    }
	    return GINT_TO_POINTER(TRUE);
	}
    }
    NOOP( "0x%x mask 0x%x not found (ctrl= 0x%x shift=0x%x mod1=0x%x mod2=0x%x  mod3=0x%x mod4=0x%x mod5=0x%x)\n",
	keyval, state, GDK_CONTROL_MASK, GDK_SHIFT_MASK, GDK_MOD1_MASK, GDK_MOD2_MASK, GDK_MOD3_MASK, GDK_MOD4_MASK, GDK_MOD5_MASK);
    return GINT_TO_POINTER(FALSE);
}

// This is used by dnd action
void *
cp (void *mode_p, GList *in_list, gchar *target_path) {
    widgets_t *widgets_p = rfm_get_widget("widgets_p");
    plain_cp(widgets_p, GPOINTER_TO_INT(mode_p), in_list, target_path, TRUE);
    return GINT_TO_POINTER(TRUE);
}



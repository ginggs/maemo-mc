/* Dialog box features module for the Midnight Commander
   Copyright (C) 1994, 1995, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2007, 2009, 2010 Free Software Foundation

   Authors: 1994, 1995 Radek Doulik, Miguel de Icaza
            2009, 2010 Andrew Borodin

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/** \file dialog.h
 *  \brief Header: dialog box features module
 */

#ifndef MC_DIALOG_H
#define MC_DIALOG_H

#include <sys/types.h>          /* size_t */

#include "lib/global.h"
#include "lib/tty/mouse.h"
#include "lib/util.h"           /* Hook */

/* Common return values */
#define B_EXIT          0
#define B_CANCEL        1
#define B_ENTER         2
#define B_HELP          3
#define B_USER          100

typedef struct Widget Widget;

/* Widget messages */
typedef enum
{
    WIDGET_INIT,                /* Initialize widget */
    WIDGET_FOCUS,               /* Draw widget in focused state */
    WIDGET_UNFOCUS,             /* Draw widget in unfocused state */
    WIDGET_DRAW,                /* Sent to widget to draw themselves */
    WIDGET_KEY,                 /* Sent to widgets on key press */
    WIDGET_HOTKEY,              /* Sent to widget to catch preprocess key */
    WIDGET_COMMAND,             /* Send to widget to handle command */
    WIDGET_DESTROY,             /* Sent to widget at destruction time */
    WIDGET_CURSOR,              /* Sent to widget to position the cursor */
    WIDGET_IDLE,                /* Sent to widgets with options & W_WANT_IDLE */
    WIDGET_RESIZED              /* Sent after a widget has been resized */
} widget_msg_t;

typedef enum
{
    MSG_NOT_HANDLED = 0,
    MSG_HANDLED = 1
} cb_ret_t;

/* Widgets are expected to answer to the following messages:

   WIDGET_FOCUS:   1 if the accept the focus, 0 if they do not.
   WIDGET_UNFOCUS: 1 if they accept to release the focus, 0 if they don't.
   WIDGET_KEY:     1 if they actually used the key, 0 if not.
   WIDGET_HOTKEY:  1 if they actually used the key, 0 if not.
 */

/* Dialog messages */
typedef enum
{
    DLG_INIT = 0,               /* Initialize dialog */
    DLG_IDLE = 1,               /* The idle state is active */
    DLG_DRAW = 2,               /* Draw dialog on screen */
    DLG_FOCUS = 3,              /* A widget has got focus */
    DLG_UNFOCUS = 4,            /* A widget has been unfocused */
    DLG_RESIZE = 5,             /* Window size has changed */
    DLG_KEY = 6,                /* Key before sending to widget */
    DLG_HOTKEY_HANDLED = 7,     /* A widget has got the hotkey */
    DLG_POST_KEY = 8,           /* The key has been handled */
    DLG_UNHANDLED_KEY = 9,      /* Key that no widget handled */
    DLG_ACTION = 10,            /* State of check- and radioboxes has changed
                                 * and listbox current entry has changed */
    DLG_VALIDATE = 11,          /* Dialog is to be closed */
    DLG_END = 12                /* Shut down dialog */
} dlg_msg_t;

/* Flags for create_dlg */
typedef enum
{
    DLG_REVERSE = (1 << 5),     /* Tab order is opposite to the add order */
    DLG_WANT_TAB = (1 << 4),    /* Should the tab key be sent to the dialog? */
    DLG_WANT_IDLE = (1 << 3),   /* Dialog wants idle events */
    DLG_COMPACT = (1 << 2),     /* Suppress spaces around the frame */
    DLG_TRYUP = (1 << 1),       /* Try to move two lines up the dialog */
    DLG_CENTER = (1 << 0),      /* Center the dialog */
    DLG_NONE = 0                /* No options */
} dlg_flags_t;

/* Dialog state */
typedef enum
{
    DLG_ACTIVE = 0,             /* Dialog is visible and active */
    DLG_SUSPENDED = 1,          /* Dialog is suspended */
    DLG_CLOSED = 2              /* Dialog is closed */
} dlg_state_t;

/* Dialog callback */
typedef struct Dlg_head Dlg_head;
typedef cb_ret_t (*dlg_cb_fn) (Dlg_head * h, Widget * sender,
                               dlg_msg_t msg, int parm, void *data);

/* menu command execution */
typedef cb_ret_t (*menu_exec_fn) (int command);

/* get string representation of shortcut assigned  with command */
/* as menu is a widget of dialog, ask dialog about shortcut string */
typedef char *(*dlg_shortcut_str) (unsigned long command);

/* get dialog name to show in dialog list */
typedef char *(*dlg_title_str) (const Dlg_head * h, size_t len);

/* Dialog color constants */
#define DLG_COLOR_NUM       4
#define DLG_NORMALC(h)      ((h)->color[0])
#define DLG_FOCUSC(h)       ((h)->color[1])
#define DLG_HOT_NORMALC(h)  ((h)->color[2])
#define DLG_HOT_FOCUSC(h)   ((h)->color[3])

struct Dlg_head
{
    /* Set by the user */
    gboolean modal;             /* type of dialog: modal or not */
    dlg_flags_t flags;          /* User flags */
    const char *help_ctx;       /* Name of the help entry */
    int *color;                 /* Color set. Unused in viewer and editor */
    char *title;                /* Title of the dialog */

    /* Set and received by the user */
    int ret_value;              /* Result of run_dlg() */

    /* Geometry */
    int x, y;                   /* Position relative to screen origin */
    int cols, lines;            /* Width and height of the window */

    /* Internal flags */
    dlg_state_t state;
    gboolean fullscreen;        /* Parents dialogs don't need refresh */
    gboolean winch_pending;     /* SIGWINCH signal has been got. Resize dialog after rise */
    int mouse_status;           /* For the autorepeat status of the mouse */

    /* Internal variables */
    GList *widgets;             /* widgets list */
    GList *current;             /* Curently active widget */
    void *data;                 /* Data can be passed to dialog */

    dlg_cb_fn callback;
    dlg_shortcut_str get_shortcut;      /* Shortcut string */
    dlg_title_str get_title;    /* useless for modal dialogs */

    struct Dlg_head *parent;    /* Parent dialog */
};

/* Color styles for normal and error dialogs */
extern int dialog_colors[4];
extern int alarm_colors[4];

/* Widget callback */
typedef cb_ret_t (*callback_fn) (Widget * widget, widget_msg_t msg, int parm);

/* widget options */
typedef enum
{
    W_WANT_HOTKEY = (1 << 1),
    W_WANT_CURSOR = (1 << 2),
    W_WANT_IDLE = (1 << 3),
    W_IS_INPUT = (1 << 4),
    W_DISABLED = (1 << 5)       /* Widget cannot be selected */
} widget_options_t;

/* Flags for widget repositioning on dialog resize */
typedef enum
{
    WPOS_KEEP_LEFT = (1 << 0),  /* keep widget distance to left border of dialog */
    WPOS_KEEP_RIGHT = (1 << 1), /* keep widget distance to right border of dialog */
    WPOS_KEEP_TOP = (1 << 2),   /* keep widget distance to top border of dialog */
    WPOS_KEEP_BOTTOM = (1 << 3),        /* keep widget distance to bottom border of dialog */
    WPOS_KEEP_HORZ = WPOS_KEEP_LEFT | WPOS_KEEP_RIGHT,
    WPOS_KEEP_VERT = WPOS_KEEP_TOP | WPOS_KEEP_BOTTOM,
    WPOS_KEEP_ALL = WPOS_KEEP_HORZ | WPOS_KEEP_VERT
} widget_pos_flags_t;

/* Every Widget must have this as its first element */
struct Widget
{
    int x, y;
    int cols, lines;
    widget_options_t options;
    widget_pos_flags_t pos_flags;       /* repositioning flags */
    unsigned int id;                    /* Number of the widget, starting with 0 */
    callback_fn callback;
    mouse_h mouse;
    struct Dlg_head *owner;
};

/* draw box in window */
void draw_box (Dlg_head * h, int y, int x, int ys, int xs, gboolean single);

/* Creates a dialog head  */
Dlg_head *create_dlg (gboolean modal, int y1, int x1, int lines, int cols,
                      const int *colors, dlg_cb_fn callback,
                      const char *help_ctx, const char *title, dlg_flags_t flags);

void dlg_set_default_colors (void);

int add_widget_autopos (Dlg_head * dest, void *w, widget_pos_flags_t pos_flags);
int add_widget (Dlg_head * dest, void *w);

/* sets size of dialog, leaving positioning to automatic mehtods
   according to dialog flags */
void dlg_set_size (Dlg_head * h, int lines, int cols);
/* this function allows to set dialog position */
void dlg_set_position (Dlg_head * h, int y1, int x1, int y2, int x2);

void init_dlg (Dlg_head * h);
int run_dlg (Dlg_head * d);
void destroy_dlg (Dlg_head * h);

void dlg_run_done (Dlg_head * h);
void dlg_process_event (Dlg_head * h, int key, Gpm_Event * event);

char *dlg_get_title (const Dlg_head *h, size_t len);

/* To activate/deactivate the idle message generation */
void set_idle_proc (Dlg_head * d, int enable);

void dlg_redraw (Dlg_head * h);

void widget_set_size (Widget * widget, int y, int x, int lines, int cols);

void dlg_broadcast_msg (Dlg_head * h, widget_msg_t message, gboolean reverse);

void init_widget (Widget * w, int y, int x, int lines, int cols,
                  callback_fn callback, mouse_h mouse_handler);

/* Default callback for dialogs */
cb_ret_t default_dlg_callback (Dlg_head * h, Widget * sender, dlg_msg_t msg, int parm, void *data);

/* Default paint routine for dialogs */
void common_dialog_repaint (struct Dlg_head *h);

#define widget_move(w, _y, _x) tty_gotoyx (((Widget *)(w))->y + _y, ((Widget *)(w))->x + _x)
#define dlg_move(h, _y, _x) tty_gotoyx (((Dlg_head *)(h))->y + _y, ((Dlg_head *)(h))->x + _x)

extern GList *top_dlg;

/* A hook list for idle events */
extern Hook *idle_hook;

static inline cb_ret_t
send_message (Widget * w, widget_msg_t msg, int parm)
{
    return w->callback (w, msg, parm);
}

/* Return 1 if the widget is active, 0 otherwise */
static inline int
dlg_widget_active (void *w)
{
    Widget *w1 = (Widget *) w;
    return ((Widget *) w1->owner->current->data == w1);
}

static inline unsigned int
dlg_get_current_widget_id (const Dlg_head * h)
{
    return ((Widget *) h->current->data)->id;
}

void dlg_replace_widget (Widget * old, Widget * new);
int dlg_overlap (Widget * a, Widget * b);
void widget_erase (Widget *);
void dlg_erase (Dlg_head * h);
void dlg_stop (Dlg_head * h);

/* Widget selection */
void dlg_select_widget (void *widget);
void dlg_one_up (Dlg_head * h);
void dlg_one_down (Dlg_head * h);
int dlg_focus (Dlg_head * h);
Widget *find_widget_type (const Dlg_head * h, callback_fn callback);
Widget *dlg_find_by_id (const Dlg_head * h, unsigned int id);
void dlg_select_by_id (const Dlg_head * h, unsigned int id);

/* Redraw all dialogs */
void do_refresh (void);

/* Sets/clear the specified flag in the options field */
#define widget_option(w,f,i) \
    w.options = ((i) ? ((w).options | (f)) : ((w).options & (~(f))))

#define widget_want_cursor(w,i) widget_option((w), W_WANT_CURSOR, (i))
#define widget_want_hotkey(w,i) widget_option((w), W_WANT_HOTKEY, (i))
#define widget_disable(w,i) widget_option((w), W_DISABLED, (i))

/* Used in load_prompt() */
void update_cursor (Dlg_head * h);

#endif /* MC_DIALOG_H */

/*
   Widgets for the Midnight Commander

   Copyright (C) 1994-2015
   Free Software Foundation, Inc.

   Authors:
   Radek Doulik, 1994, 1995
   Miguel de Icaza, 1994, 1995
   Jakub Jelinek, 1995
   Andrej Borsenkow, 1996
   Norbert Warmuth, 1997
   Andrew Borodin <aborodin@vmail.ru>, 2009, 2010, 2013

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file listbox.c
 *  \brief Source: WListbox widget
 */

#include <config.h>

#include <stdlib.h>

#include "lib/global.h"

#include "lib/tty/tty.h"
#include "lib/tty/mouse.h"
#include "lib/skin.h"
#include "lib/strutil.h"
#include "lib/util.h"           /* Q_() */
#include "lib/keybind.h"        /* global_keymap_t */
#include "lib/widget.h"

/*** global variables ****************************************************************************/

const global_keymap_t *listbox_map = NULL;

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/

static int
listbox_entry_cmp (const void *a, const void *b, void *user_data)
{
    const WLEntry *ea = (const WLEntry *) a;
    const WLEntry *eb = (const WLEntry *) b;

    (void) user_data;

    return strcmp (ea->text, eb->text);
}

/* --------------------------------------------------------------------------------------------- */

static void
listbox_entry_free (void *data)
{
    WLEntry *e = data;
    g_free (e->text);
    g_free (e);
}

/* --------------------------------------------------------------------------------------------- */

static void
listbox_drawscroll (WListbox * l)
{
    Widget *w = WIDGET (l);
    int max_line = w->lines - 1;
    int line = 0;
    int i;
    int length;

    /* Are we at the top? */
    widget_move (w, 0, w->cols);
    if (l->top == 0)
        tty_print_one_vline (TRUE);
    else
        tty_print_char ('^');

    length = g_queue_get_length (l->list);

    /* Are we at the bottom? */
    widget_move (w, max_line, w->cols);
    if (l->top + w->lines == length || w->lines >= length)
        tty_print_one_vline (TRUE);
    else
        tty_print_char ('v');

    /* Now draw the nice relative pointer */
    if (!g_queue_is_empty (l->list))
        line = 1 + ((l->pos * (w->lines - 2)) / length);

    for (i = 1; i < max_line; i++)
    {
        widget_move (w, i, w->cols);
        if (i != line)
            tty_print_one_vline (TRUE);
        else
            tty_print_char ('*');
    }
}

/* --------------------------------------------------------------------------------------------- */

static void
listbox_draw (WListbox * l, gboolean focused)
{
    Widget *w = WIDGET (l);
    const WDialog *h = w->owner;
    const gboolean disabled = (w->options & W_DISABLED) != 0;
    const int normalc = disabled ? DISABLED_COLOR : h->color[DLG_COLOR_NORMAL];
    /* *INDENT-OFF* */
    int selc = disabled
        ? DISABLED_COLOR
        : focused
            ? h->color[DLG_COLOR_HOT_FOCUS] 
            : h->color[DLG_COLOR_FOCUS];
    /* *INDENT-ON* */

    int length = 0;
    GList *le = NULL;
    int pos;
    int i;
    int sel_line = -1;

    if (l->list != NULL)
    {
        length = g_queue_get_length (l->list);
        le = g_queue_peek_nth_link (l->list, (guint) l->top);
    }

    /*    pos = (le == NULL) ? 0 : g_list_position (l->list, le); */
    pos = (le == NULL) ? 0 : l->top;

    for (i = 0; i < w->lines; i++)
    {
        const char *text = "";

        /* Display the entry */
        if (pos == l->pos && sel_line == -1)
        {
            sel_line = i;
            tty_setcolor (selc);
        }
        else
            tty_setcolor (normalc);

        widget_move (l, i, 1);

        if (l->list != NULL && le != NULL && (i == 0 || pos < length))
        {
            WLEntry *e = LENTRY (le->data);

            text = e->text;
            le = g_list_next (le);
            pos++;
        }

        tty_print_string (str_fit_to_term (text, w->cols - 2, J_LEFT_FIT));
    }

    l->cursor_y = sel_line;

    if (l->scrollbar && length > w->lines)
    {
        tty_setcolor (normalc);
        listbox_drawscroll (l);
    }
}

/* --------------------------------------------------------------------------------------------- */

static int
listbox_check_hotkey (WListbox * l, int key)
{
    if (!listbox_is_empty (l))
    {
        int i;
        GList *le;

        for (i = 0, le = g_queue_peek_head_link (l->list); le != NULL; i++, le = g_list_next (le))
        {
            WLEntry *e = LENTRY (le->data);

            if (e->hotkey == key)
                return i;
        }
    }

    return (-1);
}

/* --------------------------------------------------------------------------------------------- */

/* Selects from base the pos element */
static int
listbox_select_pos (WListbox * l, int base, int pos)
{
    int last = 0;

    base += pos;

    if (!listbox_is_empty (l))
        last = g_queue_get_length (l->list) - 1;

    base = min (base, last);

    return base;
}

/* --------------------------------------------------------------------------------------------- */

static void
listbox_fwd (WListbox * l)
{
    if ((guint) l->pos + 1 >= g_queue_get_length (l->list))
        listbox_select_first (l);
    else
        listbox_select_entry (l, l->pos + 1);
}

/* --------------------------------------------------------------------------------------------- */

static void
listbox_back (WListbox * l)
{
    if (l->pos <= 0)
        listbox_select_last (l);
    else
        listbox_select_entry (l, l->pos - 1);
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
listbox_execute_cmd (WListbox * l, unsigned long command)
{
    cb_ret_t ret = MSG_HANDLED;
    int i;
    Widget *w = WIDGET (l);
    int length;

    if (l->list == NULL || g_queue_is_empty (l->list))
        return MSG_NOT_HANDLED;

    switch (command)
    {
    case CK_Up:
        listbox_back (l);
        break;
    case CK_Down:
        listbox_fwd (l);
        break;
    case CK_Top:
        listbox_select_first (l);
        break;
    case CK_Bottom:
        listbox_select_last (l);
        break;
    case CK_PageUp:
        for (i = 0; (i < w->lines - 1) && (l->pos > 0); i++)
            listbox_back (l);
        break;
    case CK_PageDown:
        length = g_queue_get_length (l->list);
        for (i = 0; i < w->lines - 1 && l->pos < length - 1; i++)
            listbox_fwd (l);
        break;
    case CK_Delete:
        if (l->deletable)
        {
            gboolean is_last, is_more;

            length = g_queue_get_length (l->list);

            is_last = (l->pos + 1 >= length);
            is_more = (l->top + w->lines >= length);

            listbox_remove_current (l);
            if ((l->top > 0) && (is_last || is_more))
                l->top--;
        }
        break;
    case CK_Clear:
        if (l->deletable && mc_global.widget.confirm_history_cleanup
            /* TRANSLATORS: no need to translate 'DialogTitle', it's just a context prefix */
            && (query_dialog (Q_ ("DialogTitle|History cleanup"),
                              _("Do you want clean this history?"),
                              D_ERROR, 2, _("&Yes"), _("&No")) == 0))
            listbox_remove_list (l);
        break;
    default:
        ret = MSG_NOT_HANDLED;
    }

    return ret;
}

/* --------------------------------------------------------------------------------------------- */

/* Return MSG_HANDLED if we want a redraw */
static cb_ret_t
listbox_key (WListbox * l, int key)
{
    unsigned long command;

    if (l->list == NULL)
        return MSG_NOT_HANDLED;

    /* focus on listbox item N by '0'..'9' keys */
    if (key >= '0' && key <= '9')
    {
        int oldpos = l->pos;
        listbox_select_entry (l, key - '0');

        /* need scroll to item? */
        if (abs (oldpos - l->pos) > WIDGET (l)->lines)
            l->top = l->pos;

        return MSG_HANDLED;
    }

    command = keybind_lookup_keymap_command (listbox_map, key);
    if (command == CK_IgnoreKey)
        return MSG_NOT_HANDLED;
    return listbox_execute_cmd (l, command);
}

/* --------------------------------------------------------------------------------------------- */

/* Listbox item adding function */
static inline void
listbox_append_item (WListbox * l, WLEntry * e, listbox_append_t pos)
{
    if (l->list == NULL)
    {
        l->list = g_queue_new ();
        pos = LISTBOX_APPEND_AT_END;
    }

    switch (pos)
    {
    case LISTBOX_APPEND_AT_END:
        g_queue_push_tail (l->list, e);
        break;

    case LISTBOX_APPEND_BEFORE:
        g_queue_insert_before (l->list, g_queue_peek_nth_link (l->list, (guint) l->pos), e);
        break;

    case LISTBOX_APPEND_AFTER:
        g_queue_insert_after (l->list, g_queue_peek_nth_link (l->list, (guint) l->pos), e);
        break;

    case LISTBOX_APPEND_SORTED:
        g_queue_insert_sorted (l->list, e, (GCompareDataFunc) listbox_entry_cmp, NULL);
        break;

    default:
        break;
    }
}

/* --------------------------------------------------------------------------------------------- */

static inline void
listbox_destroy (WListbox * l)
{
    listbox_remove_list (l);
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
listbox_callback (Widget * w, Widget * sender, widget_msg_t msg, int parm, void *data)
{
    WListbox *l = LISTBOX (w);
    WDialog *h = w->owner;
    cb_ret_t ret_code;

    switch (msg)
    {
    case MSG_INIT:
        return MSG_HANDLED;

    case MSG_HOTKEY:
        {
            int pos, action;

            pos = listbox_check_hotkey (l, parm);
            if (pos < 0)
                return MSG_NOT_HANDLED;

            listbox_select_entry (l, pos);
            send_message (h, w, MSG_ACTION, l->pos, NULL);

            if (l->callback != NULL)
                action = l->callback (l);
            else
                action = LISTBOX_DONE;

            if (action == LISTBOX_DONE)
            {
                h->ret_value = B_ENTER;
                dlg_stop (h);
            }

            return MSG_HANDLED;
        }

    case MSG_KEY:
        ret_code = listbox_key (l, parm);
        if (ret_code != MSG_NOT_HANDLED)
        {
            listbox_draw (l, TRUE);
            send_message (h, w, MSG_ACTION, l->pos, NULL);
        }
        return ret_code;

    case MSG_ACTION:
        return listbox_execute_cmd (l, parm);

    case MSG_CURSOR:
        widget_move (l, l->cursor_y, 0);
        send_message (h, w, MSG_ACTION, l->pos, NULL);
        return MSG_HANDLED;

    case MSG_FOCUS:
    case MSG_UNFOCUS:
    case MSG_DRAW:
        listbox_draw (l, msg != MSG_UNFOCUS);
        return MSG_HANDLED;

    case MSG_DESTROY:
        listbox_destroy (l);
        return MSG_HANDLED;

    case MSG_RESIZE:
        return MSG_HANDLED;

    default:
        return widget_default_callback (w, sender, msg, parm, data);
    }
}

/* --------------------------------------------------------------------------------------------- */

static int
listbox_event (Gpm_Event * event, void *data)
{
    WListbox *l = LISTBOX (data);
    Widget *w = WIDGET (data);

    if (!mouse_global_in_widget (event, w))
        return MOU_UNHANDLED;

    /* Single click */
    if ((event->type & GPM_DOWN) != 0)
        dlg_select_widget (l);

    if (listbox_is_empty (l))
        return MOU_NORMAL;

    if ((event->type & (GPM_DOWN | GPM_DRAG)) != 0)
    {
        int ret = MOU_REPEAT;
        Gpm_Event local;
        int i;

        local = mouse_get_local (event, w);
        if (local.y < 1)
            for (i = -local.y; i >= 0; i--)
                listbox_back (l);
        else if (local.y > w->lines)
            for (i = local.y - w->lines; i > 0; i--)
                listbox_fwd (l);
        else if ((local.buttons & GPM_B_UP) != 0)
        {
            listbox_back (l);
            ret = MOU_NORMAL;
        }
        else if ((local.buttons & GPM_B_DOWN) != 0)
        {
            listbox_fwd (l);
            ret = MOU_NORMAL;
        }
        else
            listbox_select_entry (l, listbox_select_pos (l, l->top, local.y - 1));

        /* We need to refresh ourselves since the dialog manager doesn't */
        /* know about this event */
        listbox_draw (l, TRUE);
        return ret;
    }

    /* Double click */
    if ((event->type & (GPM_DOUBLE | GPM_UP)) == (GPM_UP | GPM_DOUBLE))
    {
        Gpm_Event local;
        int action;

        local = mouse_get_local (event, w);
        dlg_select_widget (l);
        listbox_select_entry (l, listbox_select_pos (l, l->top, local.y - 1));

        if (l->callback != NULL)
            action = l->callback (l);
        else
            action = LISTBOX_DONE;

        if (action == LISTBOX_DONE)
        {
            w->owner->ret_value = B_ENTER;
            dlg_stop (w->owner);
        }
    }

    return MOU_NORMAL;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

WListbox *
listbox_new (int y, int x, int height, int width, gboolean deletable, lcback_fn callback)
{
    WListbox *l;
    Widget *w;

    if (height <= 0)
        height = 1;

    l = g_new (WListbox, 1);
    w = WIDGET (l);
    widget_init (w, y, x, height, width, listbox_callback, listbox_event);

    l->list = NULL;
    l->top = l->pos = 0;
    l->deletable = deletable;
    l->callback = callback;
    l->allow_duplicates = TRUE;
    l->scrollbar = !mc_global.tty.slow_terminal;
    widget_want_hotkey (w, TRUE);
    widget_want_cursor (w, FALSE);

    return l;
}

/* --------------------------------------------------------------------------------------------- */

int
listbox_search_text (WListbox * l, const char *text)
{
    if (!listbox_is_empty (l))
    {
        int i;
        GList *le;

        for (i = 0, le = g_queue_peek_head_link (l->list); le != NULL; i++, le = g_list_next (le))
        {
            WLEntry *e = LENTRY (le->data);

            if (strcmp (e->text, text) == 0)
                return i;
        }
    }

    return (-1);
}

/* --------------------------------------------------------------------------------------------- */

/* Selects the first entry and scrolls the list to the top */
void
listbox_select_first (WListbox * l)
{
    l->pos = l->top = 0;
}

/* --------------------------------------------------------------------------------------------- */

/* Selects the last entry and scrolls the list to the bottom */
void
listbox_select_last (WListbox * l)
{
    int lines = WIDGET (l)->lines;
    int length = 0;

    if (!listbox_is_empty (l))
        length = g_queue_get_length (l->list);

    l->pos = length > 0 ? length - 1 : 0;
    l->top = length > lines ? length - lines : 0;
}

/* --------------------------------------------------------------------------------------------- */

void
listbox_select_entry (WListbox * l, int dest)
{
    GList *le;
    int pos;
    gboolean top_seen = FALSE;

    if (listbox_is_empty (l) || dest < 0)
        return;

    /* Special case */
    for (pos = 0, le = g_queue_peek_head_link (l->list); le != NULL; pos++, le = g_list_next (le))
    {
        if (pos == l->top)
            top_seen = TRUE;

        if (pos == dest)
        {
            l->pos = dest;
            if (!top_seen)
                l->top = l->pos;
            else
            {
                int lines = WIDGET (l)->lines;

                if (l->pos - l->top >= lines)
                    l->top = l->pos - lines + 1;
            }
            return;
        }
    }

    /* If we are unable to find it, set decent values */
    l->pos = l->top = 0;
}

/* --------------------------------------------------------------------------------------------- */

/* Returns the current string text as well as the associated extra data */
void
listbox_get_current (WListbox * l, char **string, void **extra)
{
    WLEntry *e = NULL;
    gboolean ok;

    if (l != NULL)
        e = listbox_get_nth_item (l, l->pos);

    ok = (e != NULL);

    if (string != NULL)
        *string = ok ? e->text : NULL;

    if (extra != NULL)
        *extra = ok ? e->data : NULL;
}

/* --------------------------------------------------------------------------------------------- */

WLEntry *
listbox_get_nth_item (const WListbox * l, int pos)
{
    if (!listbox_is_empty (l) && pos >= 0)
    {
        GList *item;

        item = g_queue_peek_nth_link (l->list, (guint) pos);
        if (item != NULL)
            return LENTRY (item->data);
    }

    return NULL;
}

/* --------------------------------------------------------------------------------------------- */

GList *
listbox_get_first_link (const WListbox * l)
{
    return (l == NULL || l->list == NULL) ? NULL : g_queue_peek_head_link (l->list);
}

/* --------------------------------------------------------------------------------------------- */

void
listbox_remove_current (WListbox * l)
{
    if (!listbox_is_empty (l))
    {
        GList *current;
        int length;

        current = g_queue_peek_nth_link (l->list, (guint) l->pos);
        listbox_entry_free (LENTRY (current->data));
        g_queue_delete_link (l->list, current);

        length = g_queue_get_length (l->list);

        if (length == 0)
            l->top = l->pos = 0;
        else if (l->pos >= length)
            l->pos = length - 1;
    }
}

/* --------------------------------------------------------------------------------------------- */

gboolean
listbox_is_empty (const WListbox * l)
{
    return (l == NULL || l->list == NULL || g_queue_is_empty (l->list));
}

/* --------------------------------------------------------------------------------------------- */

void
listbox_set_list (WListbox * l, GList * list)
{
    listbox_remove_list (l);

    if (l != NULL)
    {
        GList *ll;

        l->list = g_queue_new ();

        for (ll = list; ll != NULL; ll = g_list_next (ll))
            g_queue_push_tail (l->list, ll->data);

        g_list_free (list);
    }
}

/* --------------------------------------------------------------------------------------------- */

void
listbox_remove_list (WListbox * l)
{
    if (l != NULL)
    {
        if (l->list != NULL)
        {
            g_queue_foreach (l->list, (GFunc) listbox_entry_free, NULL);
            g_queue_free (l->list);
        }

        l->list = NULL;
        l->pos = l->top = 0;
    }
}

/* --------------------------------------------------------------------------------------------- */

char *
listbox_add_item (WListbox * l, listbox_append_t pos, int hotkey, const char *text, void *data)
{
    WLEntry *entry;

    if (l == NULL)
        return NULL;

    if (!l->allow_duplicates && (listbox_search_text (l, text) >= 0))
        return NULL;

    entry = g_new (WLEntry, 1);
    entry->text = g_strdup (text);
    entry->data = data;
    entry->hotkey = hotkey;

    listbox_append_item (l, entry, pos);

    return entry->text;
}

/* --------------------------------------------------------------------------------------------- */
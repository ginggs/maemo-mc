/** \file wtools.h
 *  \brief Header: widget based utility functions
 */

#ifndef MC__WTOOLS_H
#define MC__WTOOLS_H

#include "lib/timer.h"

/*** typedefs(not structures) and defined constants **********************************************/

/* Pass this as def_text to request a password */
#define INPUT_PASSWORD ((char *) -1)

/* Use this as header for message() - it expands to "Error" */
#define MSG_ERROR ((char *) -1)

typedef struct status_msg_t status_msg_t;
#define STATUS_MSG(x) ((status_msg_t *)(x))

typedef struct simple_status_msg_t simple_status_msg_t;
#define SIMPLE_STATUS_MSG(x) ((simple_status_msg_t *)(x))

typedef void (*status_msg_cb) (status_msg_t * sm);
typedef int (*status_msg_update_cb) (status_msg_t * sm);

/*** enums ***************************************************************************************/

/* flags for message() and query_dialog() */
enum
{
    D_NORMAL = 0,
    D_ERROR = (1 << 0),
    D_CENTER = (1 << 1)
} /* dialog options */ ;

/*** structures declarations (and typedefs of structures)*****************************************/

/* Base class for status message of long-time operations.
   Useful to show progress of long-time operaions and interrupt it. */

struct status_msg_t
{
    WDialog *dlg;               /* pointer to status message dialog */
    guint64 delay;              /* delay before raise the 'dlg' in microseconds */
    mc_timer_t *timer;          /* timer to measure 'delay' */
    gboolean block;             /* how to get event using tty_get_event() */

    status_msg_cb init;         /* callback to init derived classes */
    status_msg_update_cb update;        /* callback to update dlg */
    status_msg_cb deinit;       /* callback to deinit deribed clesses */
};

/* Simple status message with label and 'Abort' button */
struct simple_status_msg_t
{
    status_msg_t status_msg;    /* base class */

    WLabel *label;
};

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/* The input dialogs */
char *input_dialog (const char *header, const char *text,
                    const char *history_name, const char *def_text,
                    input_complete_t completion_flags);
char *input_dialog_help (const char *header, const char *text, const char *help,
                         const char *history_name, const char *def_text, gboolean strip_password,
                         input_complete_t completion_flags);
char *input_expand_dialog (const char *header, const char *text, const char *history_name,
                           const char *def_text, input_complete_t completion_flags);

int query_dialog (const char *header, const char *text, int flags, int count, ...);
void query_set_sel (int new_sel);

/* Create message box but don't dismiss it yet, not background safe */
struct WDialog *create_message (int flags, const char *title,
                                const char *text, ...) __attribute__ ((format (__printf__, 3, 4)));

/* Show message box, background safe */
void message (int flags, const char *title, const char *text, ...)
    __attribute__ ((format (__printf__, 3, 4)));

gboolean mc_error_message (GError ** mcerror);

status_msg_t *status_msg_create (const char *title, double delay, status_msg_cb init_cb,
                                 status_msg_update_cb update_cb, status_msg_cb deinit_cb);
void status_msg_destroy (status_msg_t * sm);
void status_msg_init (status_msg_t * sm, const char *title, double delay, status_msg_cb init_cb,
                      status_msg_update_cb update_cb, status_msg_cb deinit_cb);
void status_msg_deinit (status_msg_t * sm);
int status_msg_common_update (status_msg_t * sm);

void simple_status_msg_init_cb (status_msg_t * sm);

/*** inline functions ****************************************************************************/

#endif /* MC__WTOOLS_H */

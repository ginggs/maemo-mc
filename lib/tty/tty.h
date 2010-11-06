
/** \file tty.h
 *  \brief Header: %interface to the terminal controlling library
 *
 *  This file is the %interface to the terminal controlling library:
 *  slang or ncurses. It provides an additional layer of abstraction
 *  above the "real" libraries to keep the number of ifdefs in the other
 *  files small.
 */

#ifndef MC_TTY_H
#define MC_TTY_H

#include "lib/global.h"         /* include <glib.h> */

#ifdef HAVE_SLANG
#   include "tty-slang.h"
#else
#   include "tty-ncurses.h"
#endif

/* {{{ Input }}} */

extern int reset_hp_softkeys;

extern void tty_init (gboolean slow, gboolean ugly_lines);
extern void tty_shutdown (void);

extern gboolean tty_is_slow (void);

extern void tty_start_interrupt_key (void);
extern void tty_enable_interrupt_key (void);
extern void tty_disable_interrupt_key (void);
extern gboolean tty_got_interrupt (void);

extern void tty_reset_prog_mode (void);
extern void tty_reset_shell_mode (void);

extern void tty_raw_mode (void);
extern void tty_noraw_mode (void);

extern void tty_noecho (void);
extern int tty_flush_input (void);

extern void tty_keypad (gboolean set);
extern void tty_nodelay (gboolean set);
extern int tty_baudrate (void);

extern int tty_lowlevel_getch (void);

/* {{{ Output }}} */

/*
   The output functions do not check themselves for screen overflows,
   so make sure that you never write more than what fits on the screen.
   While SLang provides such a feature, ncurses does not.
 */

extern int tty_reset_screen (void);
extern void tty_touch_screen (void);

extern void tty_gotoyx (int y, int x);
extern void tty_getyx (int *py, int *px);

extern void tty_set_alt_charset (gboolean alt_charset);

extern void tty_display_8bit (gboolean what);
extern void tty_print_char (int c);
extern void tty_print_alt_char (int c, gboolean single);
extern void tty_print_anychar (int c);
extern void tty_print_string (const char *s);
extern void tty_printf (const char *s, ...);

extern void tty_print_one_vline (gboolean single);
extern void tty_print_one_hline (gboolean single);
extern void tty_draw_hline (int y, int x, int ch, int len);
extern void tty_draw_vline (int y, int x, int ch, int len);
extern void tty_draw_box (int y, int x, int rows, int cols, gboolean single);
extern void tty_fill_region (int y, int x, int rows, int cols, unsigned char ch);

extern int mc_tty_frm[];

typedef enum
{
    /* single lines */
    MC_TTY_FRM_VERT,
    MC_TTY_FRM_HORIZ,
    MC_TTY_FRM_LEFTTOP,
    MC_TTY_FRM_RIGHTTOP,
    MC_TTY_FRM_LEFTBOTTOM,
    MC_TTY_FRM_RIGHTBOTTOM,
    MC_TTY_FRM_TOPMIDDLE,
    MC_TTY_FRM_BOTTOMMIDDLE,
    MC_TTY_FRM_LEFTMIDDLE,
    MC_TTY_FRM_RIGHTMIDDLE,
    MC_TTY_FRM_CROSS,

    /* double lines */
    MC_TTY_FRM_DVERT,
    MC_TTY_FRM_DHORIZ,
    MC_TTY_FRM_DLEFTTOP,
    MC_TTY_FRM_DRIGHTTOP,
    MC_TTY_FRM_DLEFTBOTTOM,
    MC_TTY_FRM_DRIGHTBOTTOM,
    MC_TTY_FRM_DTOPMIDDLE,
    MC_TTY_FRM_DBOTTOMMIDDLE,
    MC_TTY_FRM_DLEFTMIDDLE,
    MC_TTY_FRM_DRIGHTMIDDLE,

    MC_TTY_FRM_MAX
} mc_tty_frm_t;

extern char *tty_tgetstr (const char *name);

extern void tty_beep (void);

#define KEY_KP_ADD	4001
#define KEY_KP_SUBTRACT	4002
#define KEY_KP_MULTIPLY	4003

extern void tty_refresh (void);
extern void tty_setup_sigwinch (void (*handler) (int));

extern int mc_tty_normalize_lines_char (const char *);

#endif /* MC_TTY_H */

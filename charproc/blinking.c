#include "blinking.h"
#include "charproc/includes.h"


#if OPT_BLINK_CURS || OPT_BLINK_TEXT
static void
StartBlinking(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);

    if (screen->blink_timer == 0) {
	unsigned long interval = (unsigned long) ((screen->cursor_state == ON)
						  ? screen->blink_on
						  : screen->blink_off);
	if (interval == 0)	/* wow! */
	    interval = 1;	/* let's humor him anyway */
	screen->blink_timer = XtAppAddTimeOut(app_con,
					      interval,
					      HandleBlinking,
					      xw);
    }
}

static void
StopBlinking(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);

    if (screen->blink_timer) {
	XtRemoveTimeOut(screen->blink_timer);
	screen->blink_timer = 0;
	reallyStopBlinking(xw);
    } else {
	screen->blink_timer = 0;
    }
}

#if OPT_BLINK_TEXT
Bool
LineHasBlinking(TScreen *screen, CLineData *ld)
{
    Bool result = False;
    if (ld != NULL) {
	int col;

	for (col = 0; col < MaxCols(screen); ++col) {
	    if (ld->attribs[col] & BLINK) {
		result = True;
		break;
	    }
	}
    }
    return result;
}
#endif

/*
 * Blink the cursor by alternately showing/hiding cursor.  We leave the timer
 * running all the time (even though that's a little inefficient) to make the
 * logic simple.
 */
static void
HandleBlinking(XtPointer closure, XtIntervalId * id GCC_UNUSED)
{
    XtermWidget xw = (XtermWidget) closure;
    TScreen *screen = TScreenOf(xw);
    Bool resume = False;

    screen->blink_timer = 0;
    screen->blink_state = !screen->blink_state;

#if OPT_BLINK_CURS
    if (DoStartBlinking(screen)) {
	if (screen->cursor_state == ON) {
	    if (screen->select || screen->always_highlight) {
		HideCursor(xw);
		if (screen->cursor_state == OFF)
		    screen->cursor_state = BLINKED_OFF;
	    }
	} else if (screen->cursor_state == BLINKED_OFF) {
	    screen->cursor_state = OFF;
	    ShowCursor(xw);
	    if (screen->cursor_state == OFF)
		screen->cursor_state = BLINKED_OFF;
	}
	resume = True;
    }
#endif

#if OPT_BLINK_TEXT
    /*
     * Inspect the lines on the current screen to see if any have the BLINK flag
     * associated with them.  Prune off any that have had the corresponding
     * cells reset.  If any are left, repaint those lines with ScrnRefresh().
     */
    if (!(screen->blink_as_bold)) {
	int row;
	int start_row = LastRowNumber(screen);
	int first_row = start_row;
	int last_row = -1;

	for (row = start_row; row >= 0; row--) {
	    LineData *ld = getLineData(screen, ROW2INX(screen, row));

	    if (ld != NULL && LineTstBlinked(ld)) {
		if (LineHasBlinking(screen, ld)) {
		    resume = True;
		    if (row > last_row)
			last_row = row;
		    if (row < first_row)
			first_row = row;
		} else {
		    LineClrBlinked(ld);
		}
	    }
	}
	/*
	 * FIXME: this could be a little more efficient, e.g,. by limiting the
	 * columns which are updated.
	 */
	if (first_row <= last_row) {
	    ScrnRefresh(xw,
			first_row,
			0,
			last_row + 1 - first_row,
			MaxCols(screen),
			True);
	}
    }
#endif

    /*
     * If either the cursor or text is blinking, restart the timer.
     */
    if (resume)
	StartBlinking(xw);
}
#endif /* OPT_BLINK_CURS || OPT_BLINK_TEXT */

#if OPT_BLINK_CURS
int
DoStartBlinking(TScreen *screen)
{
    int actual = ((screen->cursor_blink == cbTrue ||
		   screen->cursor_blink == cbAlways)
		  ? 1
		  : 0);
    int wanted = screen->cursor_blink_esc ? 1 : 0;
    int result;
    if (screen->cursor_blink_xor) {
	result = actual ^ wanted;
    } else {
	result = actual | wanted;
    }
    return result;
}

void
SetCursorBlink(XtermWidget xw, BlinkOps enable)
{
    TScreen *screen = TScreenOf(xw);

    if (SettableCursorBlink(screen)) {
	screen->cursor_blink = enable;
    }
    if (DoStartBlinking(screen)) {
	StartBlinking(xw);
    } else {
	/* EMPTY */
#if OPT_BLINK_TEXT
	reallyStopBlinking(xw);
#else
	StopBlinking(xw);
#endif
    }
    update_cursorblink();
}

void
ToggleCursorBlink(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);

    if (screen->cursor_blink == cbTrue) {
	SetCursorBlink(xw, cbFalse);
    } else if (screen->cursor_blink == cbFalse) {
	SetCursorBlink(xw, cbTrue);
    }
}
#endif


void
RestartBlinking(XtermWidget xw)
{
#if OPT_BLINK_CURS || OPT_BLINK_TEXT
    TScreen *screen = TScreenOf(xw);

    if (screen->blink_timer == 0) {
	Bool resume = False;

#if OPT_BLINK_CURS
	if (DoStartBlinking(screen)) {
	    resume = True;
	}
#endif
#if OPT_BLINK_TEXT
	if (!resume) {
	    int row;

	    for (row = screen->max_row; row >= 0; row--) {
		CLineData *ld = getLineData(screen, ROW2INX(screen, row));

		if (ld != NULL && LineTstBlinked(ld)) {
		    if (LineHasBlinking(screen, ld)) {
			resume = True;
			break;
		    }
		}
	    }
	}
#endif
	if (resume)
	    StartBlinking(xw);
    }
#else
    (void) xw;
#endif
}

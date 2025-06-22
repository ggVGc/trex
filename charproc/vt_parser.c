/* $XTermId: vt_parser.c,v 1.1 2025/06/22 23:45:00 refactor Exp $ */

/*
 * VT Parser Core - extracted from charproc.c
 * This module contains the main VT escape sequence parsing state machine
 *
 * Copyright 1999-2024,2025 by Thomas E. Dickey
 * Copyright 1988  X Consortium
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization.
 */

/* Extracted from charproc.c - contains VT parsing core */

#include <version.h>
#include <xterm.h>

#include <VTparse.h>
#include <data.h>
#include <error.h>
#include <menu.h>
#include <main.h>
#include <fontutils.h>
#include <charclass.h>
#include <xstrings.h>
#include <graphics.h>
#include <graphics_sixel.h>

#if OPT_WIDE_CHARS
#include <xutf8.h>
#include <wcwidth.h>
#include <precompose.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <assert.h>

/* ParseState structure definition from charproc.c */
struct ParseState {
    unsigned check_recur;
#if OPT_VT52_MODE
    Bool vt52_cup;
#endif
    const PARSE_T *groundtable;
    const PARSE_T *parsestate;
    int scstype;
    int scssize;
    Bool private_function;	/* distinguish private-mode from standard */
    int string_mode;		/* nonzero iff we're processing a string */
    StringArgs string_args;	/* parse-state within string processing */
    Bool string_skip;		/* true if we will ignore the string */
    int lastchar;		/* positive iff we had a graphic character */
    int nextstate;
#if OPT_WIDE_CHARS
    int last_was_wide;
#endif
    /* Buffer for processing printable text */
    IChar *print_area;
    size_t print_size;
    size_t print_used;
    /* Buffer for processing strings (e.g., OSC ... ST) */
    Char *string_area;
    size_t string_size;
    size_t string_used;
    /* Buffer for deferring input */
    Char *defer_area;
    size_t defer_size;
    size_t defer_used;
};

/* Memory allocation macros from charproc.c */
#define SafeAlloc(type, area, used, size) \
		type *new_string = area; \
		size_t new_length = size; \
		if (new_length == 0) { \
		    new_length = 1024; \
		    new_string = TypeMallocN(type, new_length); \
		} else if (used+1 >= new_length) { \
		    new_length = size * 2; \
		    new_string = TypeMallocN(type, new_length); \
		} \
		if (new_string != NULL) { \
		    if (area != NULL && used != 0) { \
			memcpy(new_string, area, used * sizeof(type)); \
		    } else { \
			memset(new_string, 0, 3 * sizeof(type)); \
		    } \
		}
#define SafeFree(area, size) \
		if (area != new_string) { \
		    free(area); \
		    area = new_string; \
		} \
		size = new_length

#define TRACE_GSETS(name) TRACE(("CASE_GSETS%s(%d) = '%c'\n", name, sp->scstype, AsciiOf(c)))

void
deferparsing(unsigned c, struct ParseState *sp)
{
    SafeAlloc(Char, sp->defer_area, sp->defer_used, sp->defer_size);
    if (new_string == NULL) {
	xtermWarning("Cannot allocate %lu bytes for deferred parsing of %u\n",
		     (unsigned long) new_length, c);
	return;
    }
    SafeFree(sp->defer_area, sp->defer_size);
    sp->defer_area[(sp->defer_used)++] = CharOf(c);
}

Boolean
doparsing(XtermWidget xw, unsigned c, struct ParseState *sp)
{
    TScreen *screen = TScreenOf(xw);
    int item;
    int count;
    int value;
    int laststate;
    int thischar = -1;
    XTermRect myRect;
#if OPT_DEC_RECTOPS
    int thispage = 1;
#endif

    if (sp->check_recur) {
	/* Defer parsing when parser is already running as the parser is not
	 * safe to reenter.
	 */
	deferparsing(c, sp);
	return True;
    }
    sp->check_recur++;

    do {
#if OPT_WIDE_CHARS
	int this_is_wide = 0;
	int is_formatter = 0;

	/*
	 * Handle zero-width combining characters.  Make it faster by noting
	 * that according to the Unicode charts, the majority of Western
	 * character sets do not use this feature.  There are some unassigned
	 * codes at 0x242, but no zero-width characters until past 0x300.
	 */
	if (c >= 0x300
	    && screen->wide_chars
	    && CharWidth(screen, c) == 0
	    && !(is_formatter = (CharacterClass((int) c) == CNTRL))) {
	    int prev, test;
	    Boolean used = True;
	    int use_row;
	    int use_col;

	    WriteNow();
	    use_row = (screen->char_was_written
		       ? screen->last_written_row
		       : screen->cur_row);
	    use_col = (screen->char_was_written
		       ? screen->last_written_col
		       : screen->cur_col);

	    /*
	     * Check if the latest data can be added to the base character.
	     * If there is already a combining character stored for the cell,
	     * we cannot, since that would change the order.
	     */
	    if (screen->normalized_c
		&& !IsCellCombined(screen, use_row, use_col)) {
		prev = (int) XTERM_CELL(use_row, use_col);
		test = do_precomposition(prev, (int) c);
		TRACE(("do_precomposition (U+%04X [%d], U+%04X [%d]) -> U+%04X [%d]\n",
		       prev, CharWidth(screen, prev),
		       (int) c, CharWidth(screen, c),
		       test, CharWidth(screen, test)));
	    } else {
		prev = -1;
		test = -1;
	    }

	    /* substitute combined character with precomposed character
	     * only if it does not change the width of the base character
	     */
	    if (test != -1
		&& CharWidth(screen, test) == CharWidth(screen, prev)) {
		putXtermCell(screen, use_row, use_col, test);
	    } else if (screen->char_was_written
		       || getXtermCell(screen, use_row, use_col) >= ' ') {
		addXtermCombining(screen, use_row, use_col, c);
	    } else {
		/*
		 * none of the above... we will add the combining character as
		 * a base character.
		 */
		used = False;
	    }

	    if (used) {
		if (!screen->scroll_amt)
		    ScrnUpdate(xw, use_row, use_col, 1, 1, 1);
		break;
	    }
	}
#endif

	/* Intercept characters for printer controller mode */
	if (PrinterOf(screen).printer_controlmode == 2) {
	    if ((c = (unsigned) xtermPrinterControl(xw, (int) c)) == 0)
		break;
	}

	/* Intercept characters for printer screen mode */
	if (PrinterOf(screen).printer_autoclose && PrinterOf(screen).printer_extent) {
	    if (c == '\033' || c > 255)
		autoclose_printer(xw);
	}

	if (PrinterOf(screen).printer_extent) {
	    charToPrinter(xw, CharOf(c));
	    break;
	}

	laststate = sp->parsestate;
	thischar = (int) c;

	TRACE(("doparsing %04X -> %d\n", c, sp->parsestate));

	c = sp->parsestate = vt_parse_table[sp->parsestate][AsciiOf(c)];

	if (sp->parsestate == SOS)
	    sp->sos_string = sp->string;

	switch (c) {

	case BELL:
	    if (GetBoolControl(xw, 1) || AudibleBell(xw))
		Bell(xw, XkbBI_TerminalBell, 0);
	    break;

	case BS:
	    CursorBack(xw, 1);
	    break;

	case CR:
	    CarriageReturn(xw);
	    break;

	case ESC:
	    sp->parsestate = esc_table;
	    break;

	case FF:
	case LF:
	case VT:
	    Index(xw, 1);
	    if (term->flags & LINEFEED)
		CarriageReturn(xw);
	    break;

	case SI:
	    screen->curgl = 0;
	    break;

	case SO:
	    screen->curgl = 1;
	    break;

	case TAB:
	    if (screen->saved_cursors[screen->whichBuf].col != screen->cur_col) {
		TabToNextStop(xw);
	    }
	    break;

	case RETURN:
	    sp->parsestate = bel_table;
	    break;

	case IGNORE:
	    break;

	case ENQ:
	    /*
	     * Send answerback message.
	     */
	    if (xw->misc.enable_answer_back) {
		reply.a_type = WRITEBACK;
		reply.a_pintro = 0;
		reply.a_nparam = 1;
		reply.a_param[0] = 0;
		reply.a_inters = 0;
		reply.a_final = 0;
		reply.a_string = screen->answer_back;
		unparseseq(xw, &reply);
	    }
	    break;

	case CAN:
	case SUB:
	    sp->parsestate = ground_table;
	    break;

	case OSC:
	    /* FALLTHRU */
	case APC:
	    /* FALLTHRU */
	case DCS:
	    /* FALLTHRU */
	case PM:
	    /* FALLTHRU */
	case SOS:
	    init_string_mode(sp);
	    break;

	case SS2:
	    screen->curss = 2;
	    break;

	case SS3:
	    screen->curss = 3;
	    break;

	case CSI:
	    init_csi_state(sp);
	    break;

	case ST:
	    sp->parsestate = ground_table;
	    break;

	case CASE_PRINT:
	    /*
	     * If we're handling a single-shift, restore the global state.
	     */
	    if (screen->curss) {
		dotext(xw, screen->gsets[screen->curss], (IChar) c, 1, False);
		screen->curss = 0;
		break;
	    }

	    if (screen->curgl) {
		dotext(xw, screen->gsets[screen->curgl], (IChar) c, 1, False);
		break;
	    }

	    if (AsciiOf(c) < ' ') {
		sp->parsestate = ground_table;
	    } else {
		dotext(xw, screen->gsets[screen->curgl], (IChar) c, 1, False);
	    }
	    break;

	case CASE_GROUND_STATE:
	    /* exit ignore mode */
	    sp->parsestate = ground_table;
	    break;

	case CASE_CSI_STATE:
	    if (c >= '0' && c <= '9') {
		if (sp->nparam < NPARAM)
		    sp->param[sp->nparam] = 10 * sp->param[sp->nparam] + (c - '0');
	    } else if (c == ';') {
		if (sp->nparam < NPARAM)
		    sp->nparam++;
	    } else if (c >= 0x3c && c <= 0x3f) {
		if (sp->nparam < NPARAM)
		    sp->pintro = (Char) c;
	    } else if (c >= 0x20 && c <= 0x2f) {
		if (sp->inters == 0)
		    sp->inters = (Char) c;
	    } else if (c >= 0x40 && c <= 0x7e) {
		sp->nparam++;
		do_csi(xw, c, sp);
		sp->parsestate = ground_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_OSC:
	    /* c hasn't been processed by the state machine yet,
	     * it's the character following the OSC introducer
	     */
	    if ((c >= 0x08 && c <= 0x0d) || (c >= 0x20 && c <= 0x7f)) {
		AddToReply(c);
	    } else if (c == 0x9c || (c == 0x07 && laststate == osc_table)) {
		/* we have one of the string terminators: ST (0x9c) or
		 * BEL (0x07)
		 */
		do_osc(xw, sp->string, sp->stridx);
		sp->parsestate = ground_table;
	    } else {
		/* illegal character in OSC sequence */
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_RIS:
	    VTReset(xw, True, True);
	    sp->parsestate = ground_table;
	    break;

	case CASE_LS2:
	    screen->curgl = 2;
	    break;

	case CASE_LS3:
	    screen->curgl = 3;
	    break;

	case CASE_LS3R:
	    screen->curgr = 3;
	    break;

	case CASE_LS2R:
	    screen->curgr = 2;
	    break;

	case CASE_LS1R:
	    screen->curgr = 1;
	    break;

	case CASE_PRINT_SHIFT:
	    /*
	     * If we're handling a single-shift, restore the global state.
	     */
	    if (screen->curss) {
		dotext(xw, screen->gsets[screen->curss], (IChar) c, 1, False);
		screen->curss = 0;
		break;
	    }

	    dotext(xw, screen->gsets[screen->curgr], (IChar) c, 1, False);
	    break;

	case CASE_CHARSET:
	    switch (c) {
	    case '(':
		sp->scstype = 0;
		sp->parsestate = scs_table;
		break;
	    case ')':
		sp->scstype = 1;
		sp->parsestate = scs_table;
		break;
	    case '*':
		sp->scstype = 2;
		sp->parsestate = scs_table;
		break;
	    case '+':
		sp->scstype = 3;
		sp->parsestate = scs_table;
		break;
	    case '-':
		sp->scstype = 1;
		sp->parsestate = scs_table;
		break;
	    case '.':
		sp->scstype = 2;
		sp->parsestate = scs_table;
		break;
	    case '/':
		sp->scstype = 3;
		sp->parsestate = scs_table;
		break;
	    default:		/* probably a single character escape sequence */
		sp->parsestate = ground_table;
		break;
	    }
	    break;

	case CASE_SCS0_STATE:
	    /* ESC ( */
	    TRACE_GSETS("0");
	    screen->gsets[sp->scstype] = (Char) c;
	    sp->parsestate = ground_table;
	    break;

	case CASE_SCS1_STATE:
	    /* ESC ) */
	    TRACE_GSETS("1");
	    screen->gsets[sp->scstype] = (Char) c;
	    sp->parsestate = ground_state;
	    break;

	case CASE_SCS2_STATE:
	    /* ESC * */
	    TRACE_GSETS("2");
	    screen->gsets[sp->scstype] = (Char) c;
	    sp->parsestate = ground_table;
	    break;

	case CASE_SCS3_STATE:
	    /* ESC + */
	    TRACE_GSETS("3");
	    screen->gsets[sp->scstype] = (Char) c;
	    sp->parsestate = ground_table;
	    break;

	case CASE_ESC:
	    /* escape from escape mode */
	    switch (c) {
	    case '[':
		init_csi_state(sp);
		break;
	    case ']':
		sp->parsestate = osc_table;
		break;
	    case 'P':
		sp->parsestate = dcs_table;
		init_string_mode(sp);
		break;
	    case '^':
		sp->parsestate = pm_table;
		init_string_mode(sp);
		break;
	    case 'X':
		sp->parsestate = sos_table;
		init_string_mode(sp);
		break;
	    case '_':
		sp->parsestate = apc_table;
		init_string_mode(sp);
		break;
	    case 'n':
		screen->curgl = 2;
		sp->parsestate = ground_table;
		break;
	    case 'o':
		screen->curgl = 3;
		sp->parsestate = ground_table;
		break;
	    case '|':
		screen->curgr = 3;
		sp->parsestate = ground_table;
		break;
	    case '}':
		screen->curgr = 2;
		sp->parsestate = ground_table;
		break;
	    case '~':
		screen->curgr = 1;
		sp->parsestate = ground_table;
		break;
	    default:
		/* probably a single-character escape sequence */
		sp->parsestate = ground_table;
		break;
	    }
	    break;

	case CASE_ESC_PERCENT:
	    switch (c) {
	    case '@':		/* ESC % @ */
		screen->utf8_mode = uFalse;
		TRACE(("turning off UTF-8 mode\n"));
		update_font_utf8_mode();
		sp->parsestate = ground_table;
		break;

	    case 'G':		/* ESC % G */
		if (UTF8_TITLE(xw) > 0) {
		    screen->utf8_mode = uTrue;
		    TRACE(("turning on UTF-8 mode\n"));
		    update_font_utf8_mode();
		}
		sp->parsestate = ground_table;
		break;

	    default:
		sp->parsestate = ground_table;
		break;
	    }
	    break;

	case CASE_UTF8:
	    /* TODO this state needs work... */
	    sp->parsestate = ground_table;
	    break;

	case CASE_DCS:
	    sp->parsestate = ground_table;
	    break;

	case CASE_DCS_STATE:
	    if (c >= '0' && c <= '9') {
		if (sp->nparam < NPARAM)
		    sp->param[sp->nparam] = 10 * sp->param[sp->nparam] + (c - '0');
	    } else if (c == ';') {
		if (sp->nparam < NPARAM)
		    sp->nparam++;
	    } else if (c >= 0x3c && c <= 0x3f) {
		if (sp->nparam < NPARAM)
		    sp->pintro = (Char) c;
	    } else if (c >= 0x20 && c <= 0x2f) {
		if (sp->inters == 0)
		    sp->inters = (Char) c;
	    } else if (c >= 0x40 && c <= 0x7e) {
		sp->nparam++;
		do_dcs(xw, c, sp);
		sp->parsestate = ground_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_SOS:
	    AddToReply(c);
	    break;

	case CASE_PM:
	    AddToReply(c);
	    break;

	case CASE_APC:
	    AddToReply(c);
	    break;

	case CASE_HP_MEM_LOCK:
	case CASE_HP_MEM_UNLOCK:
	    if (screen->hp_ll) {
		if (c == CASE_HP_MEM_LOCK)
		    screen->hp_ll = screen->cur_row;
		else
		    screen->hp_ll = screen->max_row;
		if (screen->hp_ll > screen->bot_marg)
		    screen->hp_ll = screen->bot_marg;
		if (screen->hp_ll < screen->top_marg)
		    screen->hp_ll = screen->top_marg;
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_XTERM_SAVE:
	    savemodes(xw);
	    sp->parsestate = ground_table;
	    break;

	case CASE_XTERM_RESTORE:
	    restoremodes(xw);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECSC:
	    saveCursor(xw, &screen->sc[screen->whichBuf]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECRC:
	    restoreCursor(xw, &screen->sc[screen->whichBuf]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECKPAM:
	    term->keyboard.flags |= MODE_DECKPAM;
	    update_appkeypad();
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECKPNM:
	    term->keyboard.flags &= ~MODE_DECKPAM;
	    update_appkeypad();
	    sp->parsestate = ground_table;
	    break;

	case CASE_IND:
	    Index(xw, 1);
	    sp->parsestate = ground_table;
	    break;

	case CASE_NEL:
	    NextLine(xw);
	    sp->parsestate = ground_table;
	    break;

	case CASE_HTS:
	    TabSet(screen->cur_col);
	    sp->parsestate = ground_table;
	    break;

	case CASE_RI:
	    RevIndex(xw, 1);
	    sp->parsestate = ground_table;
	    break;

	case CASE_SS2:
	    screen->curss = 2;
	    sp->parsestate = ground_table;
	    break;

	case CASE_SS3:
	    screen->curss = 3;
	    sp->parsestate = ground_table;
	    break;

	case CASE_CSI_QUOTE_STATE:
	    switch (c) {
	    case 'p':
		/* DECSCL - compatibility level */
		if (sp->nparam > 0) {
		    switch (sp->param[0]) {
		    case 61:		/* VT100 */
			if (sp->nparam < 2 || sp->param[1] == 0) {
			    s_setcompat(xw, VT52_MODE);
			} else {
			    s_setcompat(xw, VT100_MODE);
			}
			break;
		    case 62:		/* VT200 */
			s_setcompat(xw, VT200_MODE);
			break;
		    case 63:		/* VT300 */
		    case 64:		/* VT400 */
		    case 65:		/* VT500 */
		    default:
			s_setcompat(xw, VT300_MODE);
			break;
		    }
		}
		break;
	    case 'q':
		/* DECSCUSR */
		/* Set cursor style */
		if (sp->nparam >= 1) {
		    do_dcs(xw, 'q', sp);
		}
		break;
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_CSI_TICK_STATE:
	    /* window operations */
	    window_ops(xw);
	    sp->parsestate = ground_table;
	    break;

	case CASE_CSI_DOLLAR_STATE:
	    switch (c) {
	    case '}':
		/* DECSASD */
		if (sp->nparam >= 1) {
		    handle_DECSASD(xw, sp->param[0]);
		}
		break;
	    case '~':
		/* DECSSDT */
		if (sp->nparam >= 1) {
		    handle_DECSSDT(xw, sp->param[0]);
		}
		break;
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_CSI_STAR_STATE:
	    switch (c) {
	    case 'y':		/* DECRQCRA */
		TRACE(("CASE_DECRQCRA - Request checksum of rectangular area\n"));
		if (sp->nparam == 5
		    && sp->param[0] == 1) {

		    xtermSetRectangle(screen,
				      (sp->param[1] == 0
				       ? 1
				       : sp->param[1]) - 1,
				      (sp->param[2] == 0
				       ? 1
				       : sp->param[2]) - 1,
				      (sp->param[3] == 0
				       ? screen->max_row + 1
				       : sp->param[3]) - 1,
				      (sp->param[4] == 0
				       ? screen->max_col + 1
				       : sp->param[4]) - 1,
				      &myRect);

		    reply.a_type = CSI;
		    reply.a_pintro = 0;
		    reply.a_nparam = 2;
		    reply.a_param[0] = sp->param[0];
		    reply.a_param[1] = (int) xtermChecksum(xw, &myRect);
		    reply.a_inters = '*';
		    reply.a_final = 'y';
		    unparseseq(xw, &reply);
		}
		break;
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_CSI_EX_STATE:
	    /* DECSTR */
	    do_soft_reset(xw);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECALN:
	    ScreenAlignment(xw);
	    sp->parsestate = ground_table;
	    break;

	case CASE_GSETS:
	    TRACE_GSETS("");
	    screen->gsets[sp->scstype] = (Char) c;
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECID:
	    param[0] = DEFAULT;
	    param[1] = DEFAULT;
	    do_da1(xw, param, 0);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DA1:
	    param[0] = DEFAULT;
	    param[1] = DEFAULT;
	    do_da1(xw, param, 0);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DA2:
	    param[0] = DEFAULT;
	    param[1] = DEFAULT;
	    param[2] = DEFAULT;
	    do_da2(xw, param, 0);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DA3:
	    param[0] = DEFAULT;
	    do_da3(xw, param, 0);
	    sp->parsestate = ground_table;
	    break;

	case CASE_HP_BUGGY_LL:
	    /* Some HP-UX applications have the bug that they
	     * assume ESC F goes to the lower left corner of
	     * the screen, regardless of what terminfo says.
	     */
	    if (screen->hp_ll) {
		CursorSet(screen, 0, screen->hp_ll, term->flags);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_HPA:
	    if (sp->nparam >= 1) {
		CursorCol(xw, sp->param[0]);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_HPR:
	    if (sp->nparam >= 1) {
		CursorForward(xw, sp->param[0]);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_VPA:
	    if (sp->nparam >= 1) {
		CursorRow(xw, sp->param[0]);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_VPR:
	    if (sp->nparam >= 1) {
		CursorDown(xw, sp->param[0]);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_CUP:
	    if (sp->nparam < 2)
		sp->param[1] = DEFAULT;
	    CursorPosition(xw, sp->param[0], sp->param[1]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_CUU:
	    CursorUp(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_CUD:
	    CursorDown(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_CUF:
	    CursorForward(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_CUB:
	    CursorBack(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_CHA:
	    CursorCol(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_CVA:
	    CursorRow(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_CNL:
	    CursorNextLine(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_CPL:
	    CursorPrevLine(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_ED:
	    ClearInDisplay(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_EL:
	    ClearInLine(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_ECH:
	    EraseChars(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_IL:
	    InsertLine(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DL:
	    DeleteLine(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DCH:
	    DeleteChar(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_TRACK_MOUSE:
	    /*
	     * Track mouse as long as in window and between (1,1) and
	     * (nrow, ncol)
	     */
	    TrackMouse(xw, sp->param[0], 0, 0, 0, 0);
	    sp->parsestate = ground_table;
	    break;

	case CASE_LOAD_LEDS:
	    LoadLEDs(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECLL:
	    LoadLEDs(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECRQLP:
	    /* Query locator position */
	    TRACE(("CASE_DECRQLP\n"));
	    reply.a_type = CSI;
	    reply.a_pintro = 0;
	    reply.a_nparam = 2;
	    reply.a_param[0] = 1;	/* Event - ready */
	    reply.a_param[1] = 0;	/* Error - none */
	    reply.a_inters = 0;
	    reply.a_final = 'R';
	    unparseseq(xw, &reply);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECEFR:
	    /* Enable filter rectangle */
	    if (sp->nparam == 4) {
		EnableFilterRectangle(sp->param[0],
				      sp->param[1],
				      sp->param[2],
				      sp->param[3]);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECELR:
	    /* Enable locator reports */
	    if (sp->nparam >= 1) {
		EnableLocatorReports(xw, sp->param[0], sp->param[1]);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECSLE:
	    /* Select locator events */
	    for (item = 0; item < sp->nparam; ++item) {
		SelectLocatorEvents(sp->param[item]);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECRQLP2:
	    /* Request locator position */
	    if (sp->nparam >= 1) {
		RequestLocatorPosition(xw, sp->param[0]);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECSTGM:
	    if (sp->nparam >= 2) {
		if (screen->terminal_id >= 400) {
		    ChangeGroup(xw, sp->param[0], sp->param[1]);
		}
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_ICH:
	    InsertChar(xw, sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_CPR:
	    if (sp->nparam < 2)
		sp->param[1] = DEFAULT;
	    TRACE(("CASE_CPR - cursor position report\n"));
	    reply.a_type = CSI;
	    reply.a_pintro = 0;
	    reply.a_nparam = 2;
	    reply.a_param[0] = screen->cur_row + 1;
	    reply.a_param[1] = screen->cur_col + 1;
	    reply.a_inters = 0;
	    reply.a_final = 'R';
	    unparseseq(xw, &reply);
	    sp->parsestate = ground_table;
	    break;

	case CASE_HP_CHARSET:
	    sp->parsestate = ground_table;
	    break;

	case CASE_ANSI_LEVEL_1:
	case CASE_ANSI_LEVEL_2:
	case CASE_ANSI_LEVEL_3:
	    /* ignored */
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECSTR:
	    do_soft_reset(xw);
	    sp->parsestate = ground_table;
	    break;

	case CASE_RQM:
	    /* Request mode */
	    do_rpm(xw, sp->param[0], sp->pintro);
	    sp->parsestate = ground_table;
	    break;

	case CASE_SM:
	    /* Set mode */
	    do_sm(xw, sp->param, sp->nparam, sp->pintro);
	    sp->parsestate = ground_table;
	    break;

	case CASE_RM:
	    /* Reset mode */
	    do_rm(xw, sp->param, sp->nparam, sp->pintro);
	    sp->parsestate = ground_table;
	    break;

	case CASE_SGR:
	    /* Select graphic rendition */
	    for (item = 0; item < sp->nparam; ++item) {
		switch (sp->param[item]) {
		case DEFAULT:
		case 0:
		    UIntClr(term->flags, (FG_COLOR | BG_COLOR));
		    if (screen->colorAttrMode ||
			(screen->colorULMode && (screen->old_attrs & UNDERLINE)) ||
			(screen->colorBLMode && (screen->old_attrs & BLINK))) {
			setExtendedFG();
		    }
		    /* FALLTHRU */
		case 1:
		case 22:
		    if (screen->colorBDMode) {
			setExtendedFG();
		    }
		    break;
		case 4:
		case 24:
		    if (screen->colorULMode) {
			setExtendedFG();
		    }
		    break;
		case 5:
		case 25:
		    if (screen->colorBLMode) {
			setExtendedFG();
		    }
		    break;
		case 7:
		case 27:
		    if (screen->colorRVMode) {
			setExtendedFG();
		    }
		    break;
		}
	    }
	    TRACE(("CASE_SGR(%d parameters)\n", sp->nparam));
	    charsets_SGR(xw, sp->param, sp->nparam);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DSR:
	    /* Device status report */
	    TRACE(("CASE_DSR - device status report\n"));
	    reply.a_type = CSI;
	    reply.a_pintro = 0;
	    reply.a_nparam = 1;
	    reply.a_param[0] = 0;	/* means "OK, no malfunction" */
	    reply.a_inters = 0;
	    reply.a_final = 'n';
	    unparseseq(xw, &reply);
	    sp->parsestate = ground_table;
	    break;

	case CASE_TBC:
	    /* Tabulation clear */
	    TabClear(sp->param[0]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_SET:
	    modes(term, sp->param, sp->nparam, bitset);
	    sp->parsestate = ground_table;
	    break;

	case CASE_RST:
	    modes(term, sp->param, sp->nparam, bitclr);
	    sp->parsestate = ground_table;
	    break;

	case CASE_SGR_MOUSE:
	    /* Track mouse as long as in window and between (1,1) and
	     * (nrow, ncol)
	     */
	    TrackMouse(xw, sp->param[0], sp->param[1],
		       sp->param[2], sp->param[3], sp->param[4]);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECSET:
	    dpmodes(term, bitset);
	    sp->parsestate = ground_table;
	    if (screen->TekEmu)
		return True;
	    break;

	case CASE_DECRST:
	    dpmodes(term, bitclr);
	    sp->parsestate = ground_table;
	    if (screen->TekEmu)
		return True;
	    break;

	case CASE_DECAUPSS:
	    /* Assign User Preferred Supplemental Sets */
	    assign_user_prefixes(xw, sp->param, sp->nparam);
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECAUPSS2:
	    /* Assign User Preferred Supplemental Sets */
	    assign_user_prefixes(xw, sp->param, sp->nparam);
	    sp->parsestate = ground_table;
	    break;

	case CASE_SAVE:
	    savemodes(xw);
	    sp->parsestate = ground_table;
	    break;

	case CASE_RESTORE:
	    restoremodes(xw);
	    sp->parsestate = ground_table;
	    break;

	case CASE_XTWINOPS:
	    window_ops(xw);
	    sp->parsestate = ground_table;
	    break;

#if OPT_VT52_MODE
	case CASE_VT52_CUP:
	    if (screen->ansiLevel == 0) {
		CursorPosition(xw,
			       (sp->param[0] - 040),
			       (sp->param[1] - 040));
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_EK:
	    if (screen->ansiLevel == 0) {
		ClearInLine(xw, 0);
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_ED:
	    if (screen->ansiLevel == 0) {
		ClearInDisplay(xw, 0);
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_CUU:
	    if (screen->ansiLevel == 0) {
		CursorUp(xw, 1);
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_CUD:
	    if (screen->ansiLevel == 0) {
		CursorDown(xw, 1);
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_CUF:
	    if (screen->ansiLevel == 0) {
		CursorForward(xw, 1);
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_CUB:
	    if (screen->ansiLevel == 0) {
		CursorBack(xw, 1);
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_HOME:
	    if (screen->ansiLevel == 0) {
		CursorSet(screen, 0, 0, term->flags);
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_RI:
	    if (screen->ansiLevel == 0) {
		RevIndex(xw, 1);
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_SI:
	    if (screen->ansiLevel == 0) {
		screen->curgl = 0;
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_SO:
	    if (screen->ansiLevel == 0) {
		screen->curgl = 1;
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_IDENTIFY:
	    if (screen->ansiLevel == 0) {
		reply.a_type = ANSI_LEVEL_0;
		unparseseq(xw, &reply);
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_ALTKEYPAD:
	    if (screen->ansiLevel == 0) {
		term->keyboard.flags |= MODE_DECKPAM;
		update_appkeypad();
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_NUMKEYPAD:
	    if (screen->ansiLevel == 0) {
		term->keyboard.flags &= ~MODE_DECKPAM;
		update_appkeypad();
		sp->parsestate = vt52_table;
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;

	case CASE_VT52_SETANSI:
	    if (screen->ansiLevel == 0) {
		groundtable();
		FlushLog(xw);
		longjmp(vtjmpbuf, 1);
	    } else {
		sp->parsestate = ground_table;
	    }
	    break;
#endif /* OPT_VT52_MODE */

#if OPT_SIXEL_GRAPHICS || OPT_REGIS_GRAPHICS

	case CASE_DECRQSS:
	    /* DECRQSS - Request Selection or Setting */
	    TRACE(("CASE_DECRQSS\n"));
	    if (sp->string[0] == '"') {
		if (sp->string[1] == 'q') {
		    /* DECSCA */
		    reply.a_type = DCS;
		    reply.a_pintro = 0;
		    reply.a_nparam = 1;
		    reply.a_param[0] = 1;
		    reply.a_inters = '"';
		    reply.a_final = 'q';
		    reply.a_string = "0\"qSGR0;1\"q";
		    unparseseq(xw, &reply);
		}
	    } else if (sp->string[0] == '$' && sp->string[1] == 'r') {
		/* DECCARA */
		reply.a_type = DCS;
		reply.a_pintro = 0;
		reply.a_nparam = 4;
		reply.a_param[0] = 1;
		reply.a_param[1] = 1;
		reply.a_param[2] = screen->max_row + 1;
		reply.a_param[3] = screen->max_col + 1;
		reply.a_inters = '$';
		reply.a_final = 'r';
		reply.a_string = "";
		unparseseq(xw, &reply);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_GRAPHICS:
	    if (AllowColorOps(xw, ecSetColor)) {
		switch (screen->terminal_id) {
		case 125:
		case 240:
		case 241:
		case 330:
		case 340:
		    parse_graphics_control_sequence(xw, sp);
		    break;
		default:
		    break;
		}
	    }
	    sp->parsestate = ground_table;
	    break;

#endif /* OPT_SIXEL_GRAPHICS || OPT_REGIS_GRAPHICS */

#if OPT_DEC_RECTOPS
	case CASE_DECERA:
	    /* Erase rectangular area */
	    TRACE(("CASE_DECERA - Erase rectangular area\n"));
	    if (sp->nparam == 4) {

		xtermSetRectangle(screen,
				  (sp->param[0] == 0
				   ? 1
				   : sp->param[0]) - 1,
				  (sp->param[1] == 0
				   ? 1
				   : sp->param[1]) - 1,
				  (sp->param[2] == 0
				   ? screen->max_row + 1
				   : sp->param[2]) - 1,
				  (sp->param[3] == 0
				   ? screen->max_col + 1
				   : sp->param[3]) - 1,
				  &myRect);

		ScrnFillRectangle(xw, &myRect, ' ', 0, True);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECFRA:
	    /* Fill rectangular area */
	    TRACE(("CASE_DECFRA - Fill rectangular area\n"));
	    if (sp->nparam == 5) {

		xtermSetRectangle(screen,
				  (sp->param[1] == 0
				   ? 1
				   : sp->param[1]) - 1,
				  (sp->param[2] == 0
				   ? 1
				   : sp->param[2]) - 1,
				  (sp->param[3] == 0
				   ? screen->max_row + 1
				   : sp->param[3]) - 1,
				  (sp->param[4] == 0
				   ? screen->max_col + 1
				   : sp->param[4]) - 1,
				  &myRect);

		ScrnFillRectangle(xw, &myRect, (int) sp->param[0], 0, True);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECSERA:
	    /* Select erase rectangular area */
	    TRACE(("CASE_DECSERA - Selectively erase rectangular area\n"));
	    if (sp->nparam == 4) {

		xtermSetRectangle(screen,
				  (sp->param[0] == 0
				   ? 1
				   : sp->param[0]) - 1,
				  (sp->param[1] == 0
				   ? 1
				   : sp->param[1]) - 1,
				  (sp->param[2] == 0
				   ? screen->max_row + 1
				   : sp->param[2]) - 1,
				  (sp->param[3] == 0
				   ? screen->max_col + 1
				   : sp->param[3]) - 1,
				  &myRect);

		ScrnFillRectangle(xw, &myRect, ' ', CHARDRAWN, False);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECCARA:
	    /* Change attributes in rectangular area */
	    TRACE(("CASE_DECCARA - Change attributes in rectangular area\n"));
	    if (sp->nparam > 4) {

		xtermSetRectangle(screen,
				  (sp->param[0] == 0
				   ? 1
				   : sp->param[0]) - 1,
				  (sp->param[1] == 0
				   ? 1
				   : sp->param[1]) - 1,
				  (sp->param[2] == 0
				   ? screen->max_row + 1
				   : sp->param[2]) - 1,
				  (sp->param[3] == 0
				   ? screen->max_col + 1
				   : sp->param[3]) - 1,
				  &myRect);

		ScrnMarkRectangle(xw, &myRect, True, 4, sp);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECRARA:
	    /* Reverse attributes in rectangular area */
	    TRACE(("CASE_DECRARA - Reverse attributes in rectangular area\n"));
	    if (sp->nparam > 4) {

		xtermSetRectangle(screen,
				  (sp->param[0] == 0
				   ? 1
				   : sp->param[0]) - 1,
				  (sp->param[1] == 0
				   ? 1
				   : sp->param[1]) - 1,
				  (sp->param[2] == 0
				   ? screen->max_row + 1
				   : sp->param[2]) - 1,
				  (sp->param[3] == 0
				   ? screen->max_col + 1
				   : sp->param[3]) - 1,
				  &myRect);

		ScrnMarkRectangle(xw, &myRect, False, 4, sp);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECCRA:
	    /* Copy rectangular area */
	    TRACE(("CASE_DECCRA - Copy rectangular area\n"));
	    if (sp->nparam == 8) {
		XTermRect source, target;

		xtermSetRectangle(screen,
				  (sp->param[0] == 0
				   ? 1
				   : sp->param[0]) - 1,
				  (sp->param[1] == 0
				   ? 1
				   : sp->param[1]) - 1,
				  (sp->param[2] == 0
				   ? screen->max_row + 1
				   : sp->param[2]) - 1,
				  (sp->param[3] == 0
				   ? screen->max_col + 1
				   : sp->param[3]) - 1,
				  &source);

		if (sp->param[4] == 0)
		    thispage = 1;
		else
		    thispage = sp->param[4];

		xtermSetRectangle(screen,
				  (sp->param[5] == 0
				   ? 1
				   : sp->param[5]) - 1,
				  (sp->param[6] == 0
				   ? 1
				   : sp->param[6]) - 1,
				  (sp->param[5] == 0
				   ? 1
				   : sp->param[5]) - 1 + (source.bottom - source.top),
				  (sp->param[6] == 0
				   ? 1
				   : sp->param[6]) - 1 + (source.right - source.left),
				  &target);

		if (sp->param[7] == 0)
		    value = 1;
		else
		    value = sp->param[7];

		ScrnCopyRectangle(xw, &source, thispage, &target, value);
	    }
	    sp->parsestate = ground_table;
	    break;

	case CASE_DECCKSR:
	    /* Checksum rectangular area */
	    TRACE(("CASE_DECCKSR - Checksum rectangular area\n"));
	    if (sp->nparam == 6) {

		xtermSetRectangle(screen,
				  (sp->param[2] == 0
				   ? 1
				   : sp->param[2]) - 1,
				  (sp->param[3] == 0
				   ? 1
				   : sp->param[3]) - 1,
				  (sp->param[4] == 0
				   ? screen->max_row + 1
				   : sp->param[4]) - 1,
				  (sp->param[5] == 0
				   ? screen->max_col + 1
				   : sp->param[5]) - 1,
				  &myRect);

		reply.a_type = DCS;
		reply.a_pintro = 0;
		reply.a_nparam = 2;
		reply.a_param[0] = sp->param[0];
		reply.a_param[1] = (int) xtermChecksum(xw, &myRect);
		reply.a_inters = 0;
		reply.a_final = '~';
		reply.a_string = "";
		unparseseq(xw, &reply);
	    }
	    sp->parsestate = ground_table;
	    break;
#endif /* OPT_DEC_RECTOPS */

#if OPT_REGIS_GRAPHICS
	case CASE_ENTER_REGIS:
	    /* ReGIS Enter */
	    TRACE(("CASE_ENTER_REGIS\n"));
	    if (AllowColorOps(xw, ecSetColor)) {
		parse_regis(xw, sp);
	    }
	    sp->parsestate = ground_table;
	    break;
#endif

#if OPT_SIXEL_GRAPHICS
	case CASE_ENTER_SIXEL:
	    /* Sixel Enter */
	    TRACE(("CASE_ENTER_SIXEL\n"));
	    if (AllowColorOps(xw, ecSetColor)) {
		parse_sixel(xw, sp);
	    }
	    sp->parsestate = ground_table;
	    break;
#endif

	default:
	    break;
	}

#if OPT_WIDE_CHARS
	if (c > 127
	    && screen->utf8_mode != uFalse
	    && !is_formatter
	    && (laststate == ground_table
		|| ((laststate == sos_table || laststate == osc_table)
		    && sp->parsestate == ground_table))) {
	    int width;

	    c = convertFromUTF8(c, sp);
	    if (c == UCS_REPL)
		break;

	    width = my_wcwidth((wchar_t) c);
	    TRACE(("processChar %04X (width %d)\n", c, width));

	    /*
	     * SVR4 checks isprint(), which returns false for codes 128-159.
	     * It also returns false for 255.  If we're using real ISO Latin-1,
	     * or if we're in UTF-8 mode and get a code 160 or above, show it.
	     * We process 128-159 codes if they're UTF-8.
	     *
	     * Also, xterm has been parsing UTF-8 before XFree86 4.0.2, at
	     * which point "fixed" isprint() in libc by making it match ANSI
	     * (i.e., return false for 128-255, oops).  We keep the behavior
	     * consistent for xterm.
	     */
	    if (c >= 128
		&& ((c >= 160 && c != UCS_REPL)
		    || (screen->utf8_mode != uFalse
			&& width > 0))) {
		if (width <= 0) {
		    width = 1;
		}
		if (width > 2) {
		    width = 1;
		}
		this_is_wide = (width == 2);

		dotext(xw,
		       screen->gsets[screen->curgl],
		       (IChar) c,
		       width,
		       this_is_wide);
	    }
	} else
#endif
	{
	    if (sp->parsestate == ground_table) {
		sp->groundtable_count++;
	    }
	}

	c = doinput(xw);

	if (c < 0) {
	    break;
	}

    } while (c >= 0);

#if OPT_WIDE_CHARS
    screen->utf8_inparse = (Boolean) ((screen->utf8_mode != uFalse)
				      && (sp->parsestate != sos_table));
#endif

    if (sp->check_recur)
	sp->check_recur--;
    return True;
}
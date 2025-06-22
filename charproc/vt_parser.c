#include "vt_parser.h"

typedef enum {
    sa_INIT
    ,sa_LAST
    ,sa_REGIS
    ,sa_SIXEL
} StringArgs;

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

static struct ParseState myState;

static void
init_groundtable(TScreen *screen, struct ParseState *sp)
{
    (void) screen;

#if OPT_VT52_MODE
    if (!(screen->vtXX_level)) {
	sp->groundtable = vt52_table;
    } else if (screen->terminal_id >= 100)
#endif
    {
	sp->groundtable = ansi_table;
    }
}

static void
illegal_parse(XtermWidget xw, unsigned c, struct ParseState *sp)
{
    ResetState(sp);
    sp->nextstate = sp->parsestate[c];
    Bell(xw, XkbBI_MinorError, 0);
}

static void
begin_sixel(XtermWidget xw, struct ParseState *sp)
{
    TScreen *screen = TScreenOf(xw);

    sp->string_args = sa_LAST;
    if (optSixelGraphics(screen)) {
#if OPT_SIXEL_GRAPHICS
	ANSI params;
	const char *cp;

	cp = (const char *) sp->string_area;
	sp->string_area[sp->string_used] = '\0';
	parse_ansi_params(&params, &cp);
	parse_sixel_init(xw, &params);
	sp->string_args = sa_SIXEL;
	sp->string_used = 0;
#else
	(void) screen;
	TRACE(("ignoring sixel graphic (compilation flag not enabled)\n"));
#endif
    }
}

/*
 * Color palette changes using the OSC controls require a repaint of the
 * screen - but not immediately.  Do the repaint as soon as we detect a
 * state which will not lead to another color palette change.
 */
static void
repaintWhenPaletteChanged(XtermWidget xw, struct ParseState *sp)
{
    Boolean ignore = False;

    switch (sp->nextstate) {
    case CASE_ESC:
	ignore = ((sp->parsestate == ansi_table) ||
		  (sp->parsestate == sos_table));
#if USE_DOUBLE_BUFFER
	if (resource.buffered && TScreenOf(xw)->needSwap) {
	    ignore = False;
	}
#endif
	break;
    case CASE_OSC:
	ignore = ((sp->parsestate == ansi_table) ||
		  (sp->parsestate == esc_table));
	break;
    case CASE_IGNORE:
	ignore = (sp->parsestate == sos_table);
	break;
    case CASE_ST:
	ignore = ((sp->parsestate == esc_table) ||
		  (sp->parsestate == sos_table));
	break;
    case CASE_ESC_DIGIT:
	ignore = (sp->parsestate == csi_table);
	break;
    case CASE_ESC_SEMI:
	ignore = (sp->parsestate == csi2_table);
	break;
    }

    if (!ignore) {
	TRACE(("repaintWhenPaletteChanged\n"));
	xw->work.palette_changed = False;
	xtermRepaint(xw);
	xtermFlushDbe(xw);
    }
}

static void
init_parser(XtermWidget xw, struct ParseState *sp)
{
    TScreen *screen = TScreenOf(xw);

    free(sp->defer_area);
    free(sp->print_area);
    free(sp->string_area);
    memset(sp, 0, sizeof(*sp));
    sp->scssize = 94;		/* number of printable/nonspace ASCII */
    sp->lastchar = -1;		/* not a legal IChar */
    sp->nextstate = -1;		/* not a legal state */

    init_groundtable(screen, sp);
    ResetState(sp);
}

static void
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

static void
select_charset(struct ParseState *sp, int type, int size)
{
    TRACE(("select_charset G%d size %d -> G%d size %d\n",
	   sp->scstype, sp->scssize,
	   type, size));

    sp->scstype = type;
    sp->scssize = size;
    if (size == 94) {
	sp->parsestate = scstable;
    } else {
	sp->parsestate = scs96table;
    }
}

static Boolean
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

	/*
	 * VT52 is a little ugly in the one place it has a parameterized
	 * control sequence, since the parameter falls after the character
	 * that denotes the type of sequence.
	 */
#if OPT_VT52_MODE
	if (sp->vt52_cup) {
	    int row, col;
	    if (nparam < NPARAM - 1) {
		SetParam(nparam++, (int) AsciiOf(c) - 32);
		parms.is_sub[nparam] = 0;
	    }
	    if (nparam < 2)
		break;
	    sp->vt52_cup = False;
	    /*
	     * According to EK-VT5X-OP-001 DECscope User's Guide, if the row
	     * is out of range, no vertical movement occurs, while if the
	     * column is out of range, it is set to the rightmost column.
	     *
	     * However, DEC 070 (VSRM - VT52 Emulation EL-00070-0A, page A-28)
	     * differs from that, updating the column only when the parameter
	     * is in range, i.e., not mentioning the rightmost column.
	     */
	    if ((row = GetParam(0)) > screen->max_row)
		row = screen->cur_row;
	    if ((col = GetParam(1)) > screen->max_col)
		col = ((screen->terminal_id < 100)
		       ? screen->max_col	/* real VT52 */
		       : screen->cur_col);	/* emulated VT52 */
	    CursorSet(screen, row, col, xw->flags);
	    sp->parsestate = vt52_table;
	    SetParam(0, 0);
	    SetParam(1, 0);
	    break;
	}
#endif

	laststate = sp->nextstate;
	if (c == ANSI_DEL
	    && sp->parsestate == sp->groundtable
	    && sp->scssize == 96
	    && sp->scstype != 0) {
	    /*
	     * Handle special case of shifts for 96-character sets by checking
	     * if we have a DEL.  The other special case for SPACE will always
	     * be printable.
	     */
	    sp->nextstate = CASE_PRINT;
	} else
#if OPT_WIDE_CHARS
	if (c > 255) {
	    /*
	     * The parsing tables all have 256 entries.  If we're supporting
	     * wide characters, we handle them by treating them the same as
	     * printing characters.
	     */
	    if (sp->parsestate == sp->groundtable) {
		sp->nextstate = is_formatter ? CASE_IGNORE : CASE_PRINT;
	    } else if (sp->parsestate == sos_table) {
		if ((c & WIDEST_ICHAR) > 255) {
		    TRACE(("Found code > 255 while in SOS state: %04X\n", c));
		    c = BAD_ASCII;
		}
	    } else {
		sp->nextstate = CASE_GROUND_STATE;
	    }
	} else
#endif
	    sp->nextstate = sp->parsestate[c];

#if OPT_BROKEN_OSC
	/*
	 * Linux console palette escape sequences start with an OSC, but do
	 * not terminate correctly.  Some scripts do not check before writing
	 * them, making xterm appear to hang (it's awaiting a valid string
	 * terminator).  Just ignore these if we see them - there's no point
	 * in emulating bad code.
	 */
	if (screen->brokenLinuxOSC
	    && sp->parsestate == sos_table) {
	    if (sp->string_used && sp->string_area) {
		switch (sp->string_area[0]) {
		case 'P':
		    if (sp->string_used <= 7)
			break;
		    /* FALLTHRU */
		case 'R':
		    illegal_parse(xw, c, sp);
		    TRACE(("Reset to ground state (brokenLinuxOSC)\n"));
		    break;
		}
	    }
	}
#endif

#if OPT_BROKEN_ST
	/*
	 * Before patch #171, carriage control embedded within an OSC string
	 * would terminate it.  Some (buggy, of course) applications rely on
	 * this behavior.  Accommodate them by allowing one to compile xterm
	 * and emulate the old behavior.
	 */
	if (screen->brokenStringTerm
	    && sp->parsestate == sos_table
	    && c < 32) {
	    switch (c) {
	    case ANSI_EOT:	/* FALLTHRU */
	    case ANSI_BS:	/* FALLTHRU */
	    case ANSI_HT:	/* FALLTHRU */
	    case ANSI_LF:	/* FALLTHRU */
	    case ANSI_VT:	/* FALLTHRU */
	    case ANSI_FF:	/* FALLTHRU */
	    case ANSI_CR:	/* FALLTHRU */
	    case ANSI_SO:	/* FALLTHRU */
	    case ANSI_SI:	/* FALLTHRU */
	    case ANSI_XON:	/* FALLTHRU */
	    case ANSI_CAN:
		illegal_parse(xw, c, sp);
		TRACE(("Reset to ground state (brokenStringTerm)\n"));
		break;
	    }
	}
#endif

#if OPT_C1_PRINT
	/*
	 * This is not completely foolproof, but will allow an application
	 * with values in the C1 range to use them as printable characters,
	 * provided that they are not intermixed with an escape sequence.
	 */
#if OPT_WIDE_CHARS
	if (!screen->wide_chars)
#endif
	    if (screen->c1_printable
		&& (c >= 128 && c < 256)) {
		sp->nextstate = (sp->parsestate == esc_table
				 ? CASE_ESC_IGNORE
				 : sp->parsestate[160]);
		TRACE(("allowC1Printable %04X %s ->%s\n",
		       c, which_table(sp->parsestate),
		       visibleVTparse(sp->nextstate)));
	    }
#endif

#if OPT_WIDE_CHARS
	/*
	 * If we have a C1 code and the c1_printable flag is not set, simply
	 * ignore it when it was translated from UTF-8, unless the parse-state
	 * tells us that a C1 would be legal.
	 */
	if (screen->wide_chars
	    && (c >= 128 && c < 160)) {
#if OPT_C1_PRINT
	    if (screen->c1_printable) {
		sp->nextstate = CASE_PRINT;
		TRACE(("allowC1Printable %04X %s ->%s\n",
		       c, which_table(sp->parsestate),
		       visibleVTparse(sp->nextstate)));
	    } else
#endif
	    if (sp->parsestate != ansi_table)
		sp->nextstate = CASE_IGNORE;
	}

	/*
	 * If this character is a different width than the last one, put the
	 * previous text into the buffer and draw it now.
	 */
	this_is_wide = isWide((int) c);
	if (this_is_wide != sp->last_was_wide) {
	    WriteNow();
	}
#endif

	/*
	 * Accumulate string for printable text.  This may be 8/16-bit
	 * characters.
	 */
	if (sp->nextstate == CASE_PRINT) {
	    SafeAlloc(IChar, sp->print_area, sp->print_used, sp->print_size);
	    if (new_string == NULL) {
		xtermWarning("Cannot allocate %lu bytes for printable text\n",
			     (unsigned long) new_length);
		break;
	    }
	    SafeFree(sp->print_area, sp->print_size);
#if OPT_VT52_MODE
	    /*
	     * Strip output text to 7-bits for VT52.  We should do this for
	     * VT100 also (which is a 7-bit device), but xterm has been
	     * doing this for so long we shouldn't change this behavior.
	     */
	    if (screen->vtXX_level < 1)
		c = AsciiOf(c);
#endif
	    sp->print_area[sp->print_used++] = (IChar) c;
	    sp->lastchar = thischar = (int) c;
#if OPT_WIDE_CHARS
	    sp->last_was_wide = this_is_wide;
#endif
	    if (morePtyData(screen, VTbuffer)) {
		break;
	    }
	}

	if (sp->nextstate == CASE_PRINT
	    || (laststate == CASE_PRINT && sp->print_used)) {
	    WriteNow();
	}

	/*
	 * Accumulate string for DCS, OSC controls
	 * The string content should always be 8-bit characters.
	 *
	 * APC, PM and SOS are ignored; xterm currently does not use those.
	 */
	if (sp->parsestate == sos_table) {
#if OPT_WIDE_CHARS
	    /*
	     * We cannot display codes above 255, but let's try to
	     * accommodate the application a little by not aborting the
	     * string.
	     */
	    if ((c & WIDEST_ICHAR) > 255) {
		sp->nextstate = CASE_PRINT;
		c = BAD_ASCII;
	    }
#endif
	    if (sp->string_mode == ANSI_APC ||
		sp->string_mode == ANSI_PM ||
		sp->string_mode == ANSI_SOS) {
		/* EMPTY */
	    }
#if OPT_SIXEL_GRAPHICS
	    else if (sp->string_args == sa_SIXEL) {
		/* avoid adding the string-terminator */
		if (sos_table[CharOf(c)] == CASE_IGNORE)
		    parse_sixel_char(AsciiOf(c));
	    }
#endif
	    else if (sp->string_skip) {
		sp->string_used++;
	    } else if (sp->string_used >= screen->strings_max) {
		sp->string_skip = True;
		sp->string_used++;
		FreeAndNull(sp->string_area);
		sp->string_size = 0;
	    } else {
		Boolean utf8Title;

		SafeAlloc(Char, sp->string_area, sp->string_used, sp->string_size);
		if (new_string == NULL) {
		    xtermWarning("Cannot allocate %lu bytes for string mode %#02x\n",
				 (unsigned long) new_length, sp->string_mode);
		    break;
		}
		SafeFree(sp->string_area, sp->string_size);

		/*
		 * Provide for special case where xterm allows an OSC string to
		 * contain 8-bit data.  Otherwise, ECMA-48 section 9 recommends
		 * parsing controls with a 7-bit table, precluding the use of
		 * 8-bit data.
		 */
#if OPT_WIDE_CHARS
		utf8Title = (sp->string_mode == ANSI_OSC
			     && IsSetUtf8Title(xw)
			     && (sp->string_used >= 2)
			     && (sp->string_area[0] == '0'
				 || sp->string_area[0] == '2')
			     && sp->string_area[1] == ';');
#else
		utf8Title = False;
#endif

		/*
		 * ReGIS and SIXEL data can be detected by skipping over (only)
		 * parameters to the first non-parameter character and
		 * inspecting it.  Since both are DCS, we can also ignore OSC.
		 */
		sp->string_area[(sp->string_used)++] = (utf8Title
							? CharOf(c)
							: AsciiOf(c));
		if (sp->string_args < sa_LAST) {
		    switch (c) {
		    case ':':
		    case ';':
		    case '0':
		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '5':
		    case '6':
		    case '7':
		    case '8':
		    case '9':
			break;
		    case 'p':
			sp->string_args = sa_REGIS;
			break;
		    case 'q':
			begin_sixel(xw, sp);
			break;
		    default:
			sp->string_args = sa_LAST;
			break;
		    }
		}
	    }
	} else if (sp->parsestate != esc_table) {
	    /* if we were accumulating, we're not any more */
	    sp->string_mode = 0;
	    sp->string_used = 0;
	}

	DumpParams();
	TRACE(("parse %04X -> %s %s (used=%lu)\n",
	       c, visibleVTparse(sp->nextstate),
	       which_table(sp->parsestate),
	       (unsigned long) sp->string_used));

	/*
	 * If the parameter list has subparameters (tokens separated by ":")
	 * reject any controls that do not accept subparameters.
	 */
	if (parms.has_subparams) {
	    switch (sp->nextstate) {
	    case CASE_GROUND_STATE:
	    case CASE_CSI_IGNORE:
	    case CASE_CAN:
	    case CASE_SUB:
		/* FALLTHRU */

	    case CASE_ESC_DIGIT:
	    case CASE_ESC_SEMI:
	    case CASE_ESC_COLON:
		/* these states are required to parse parameter lists */
		break;

#if OPT_MOD_FKEYS
	    case CASE_SET_MOD_FKEYS:
#endif
	    case CASE_SGR:
		TRACE(("...possible subparam usage\n"));
		break;

	    case CASE_CSI_DEC_DOLLAR_STATE:
	    case CASE_CSI_DOLLAR_STATE:
	    case CASE_CSI_HASH_STATE:
	    case CASE_CSI_EX_STATE:
	    case CASE_CSI_QUOTE_STATE:
	    case CASE_CSI_SPACE_STATE:
	    case CASE_CSI_STAR_STATE:
	    case CASE_CSI_TICK_STATE:
	    case CASE_DEC2_STATE:
	    case CASE_DEC3_STATE:
	    case CASE_DEC_STATE:
		/* use this branch when we do not yet have the final character */
		TRACE(("...unexpected subparam usage\n"));
		InitParams();
		sp->nextstate = CASE_CSI_IGNORE;
		break;

	    default:
		/* use this branch for cases where we have the final character
		 * in the table that processed the parameter list.
		 */
		TRACE(("...unexpected subparam usage\n"));
		ResetState(sp);
		break;
	    }
	}

	if (xw->work.palette_changed) {
	    repaintWhenPaletteChanged(xw, sp);
	}
#if OPT_STATUS_LINE
	/*
	 * If we are currently writing to the status-line, ignore controls that
	 * apply only to the full screen, or which use features which we will
	 * not support in the status-line.
	 */
	if (IsStatusShown(screen) && (screen)->status_active) {
	    switch (sp->nextstate) {
	    case CASE_DECDHL:
	    case CASE_DECSWL:
	    case CASE_DECDWL:
	    case CASE_CUU:
	    case CASE_CUD:
	    case CASE_VPA:
	    case CASE_VPR:
	    case CASE_ED:
	    case CASE_TRACK_MOUSE:
	    case CASE_DECSTBM:
	    case CASE_DECALN:
	    case CASE_GRAPHICS_ATTRIBUTES:
	    case CASE_CAN:
	    case CASE_SUB:
	    case CASE_SPA:
	    case CASE_EPA:
	    case CASE_SU:
	    case CASE_IND:
	    case CASE_CPL:
	    case CASE_CNL:
	    case CASE_NEL:
	    case CASE_RI:
#if OPT_DEC_LOCATOR
	    case CASE_DECEFR:
	    case CASE_DECELR:
	    case CASE_DECSLE:
	    case CASE_DECRQLP:
#endif
#if OPT_DEC_RECTOPS
	    case CASE_DECRQCRA:
	    case CASE_DECCRA:
	    case CASE_DECERA:
	    case CASE_DECFRA:
	    case CASE_DECSERA:
	    case CASE_DECSACE:
	    case CASE_DECCARA:
	    case CASE_DECRARA:
#endif
		ResetState(sp);
		sp->nextstate = -1;	/* not a legal state */
		break;
	    }
	}
#endif

	switch (sp->nextstate) {
	case CASE_PRINT:
	    TRACE(("CASE_PRINT - printable characters\n"));
	    break;

	case CASE_GROUND_STATE:
	    TRACE(("CASE_GROUND_STATE - exit ignore mode\n"));
	    ResetState(sp);
	    break;

	case CASE_IGNORE:
	    TRACE(("CASE_IGNORE - Ignore character %02X\n", c));
	    break;

	case CASE_CAN:
	    /*
	     * ECMA-48 5th edition (June 1991) documents CAN thusly:
	     *
	     *> CAN is used to indicate that the data preceding it in the data
	     *> stream is in error.  As a result, this data shall be ignored.
	     *> The specific meaning of this control function shall be defined
	     *> for each application and/or between sender and recipient.
	     *
	     * The scope of "preceding" is vague.  DEC 070 clarifies it by
	     * saying that the current control sequence is cancelled, and also
	     * page 3-30, 3.5.4.4 Termination Conditions
	     */
	    if (screen->terminal_id >= 100 && screen->terminal_id < 200) {
		IChar effect = (
#if OPT_WIDE_CHARS
				   0x2592
#else
				   2
#endif
		);
		dotext(xw,
		       screen->gsets[(int) (screen->curgl)],
		       &effect, 1);
	    }
	    ResetState(sp);
	    break;

	case CASE_SUB:
	    TRACE(("CASE_SUB - substitute/show error\n"));
	    /*
	     * ECMA-48 5th edition (June 1991) documents SUB without describing
	     * its effect.  Earlier editions do not mention it.
	     *
	     * DEC's VT100 user guide documents SUB as having the same effect
	     * as CAN (cancel), which displays the error character (see page 39
	     * for a note under "checkerboard characters", and page 42).
	     *
	     * The VT220 reference improves the visible effect (display as a
	     * reverse "?"), as well as mentioning that device control
	     * sequences also are cancelled.
	     *
	     * DEC 070 comments that a "half-tone blotch" is used with VT100,
	     * etc.
	     *
	     * None of that applies to VT52.
	     */
	    if (screen->terminal_id >= 100) {
		IChar effect = (
#if OPT_WIDE_CHARS
				   (screen->terminal_id > 200) ? 0x2426 : 0x2592
#else
				   2
#endif
		);
		dotext(xw,
		       screen->gsets[(int) (screen->curgl)],
		       &effect, 1);
	    }
	    ResetState(sp);
	    break;

	case CASE_ENQ:
	    TRACE(("CASE_ENQ - answerback\n"));
	    if (((xw->keyboard.flags & MODE_SRM) == 0)
		? (sp->check_recur == 0)
		: (sp->check_recur <= 1)) {
		for (count = 0; screen->answer_back[count] != 0; count++)
		    unparseputc(xw, screen->answer_back[count]);
		unparse_end(xw);
	    }
	    break;

	case CASE_BELL:
	    TRACE(("CASE_BELL - bell\n"));
	    if (sp->string_mode == ANSI_OSC) {
		if (sp->string_area) {
		    if (sp->string_used)
			sp->string_area[--(sp->string_used)] = '\0';
		    if (sp->check_recur <= 1)
			do_osc(xw, sp->string_area, sp->string_used, (int) c);
		}
		ResetState(sp);
	    } else {
		/* bell */
		Bell(xw, XkbBI_TerminalBell, 0);
	    }
	    break;

	case CASE_BS:
	    TRACE(("CASE_BS - backspace\n"));
	    CursorBack(xw, 1);
	    break;

	case CASE_CR:
	    TRACE(("CASE_CR\n"));
	    CarriageReturn(xw);
	    break;

	case CASE_ESC:
	    if_OPT_VT52_MODE(screen, {
		sp->parsestate = vt52_esc_table;
		break;
	    });
	    sp->parsestate = esc_table;
	    break;

#if OPT_VT52_MODE
	case CASE_VT52_CUP:
	    TRACE(("CASE_VT52_CUP - VT52 cursor addressing\n"));
	    sp->vt52_cup = True;
	    ResetState(sp);
	    break;

	case CASE_VT52_IGNORE:
	    TRACE(("CASE_VT52_IGNORE - VT52 ignore-character\n"));
	    sp->parsestate = vt52_ignore_table;
	    break;
#endif

	case CASE_VMOT:
	    TRACE(("CASE_VMOT\n"));
	    /*
	     * form feed, line feed, vertical tab
	     */
	    xtermAutoPrint(xw, c);
	    xtermIndex(xw, 1);
	    if (xw->flags & LINEFEED)
		CarriageReturn(xw);
	    else if (screen->jumpscroll && !screen->fastscroll)
		do_xevents(xw);
	    break;

	case CASE_CBT:
	    TRACE(("CASE_CBT\n"));
	    /* cursor backward tabulation */
	    count = one_if_default(0);
	    while ((count-- > 0)
		   && (TabToPrevStop(xw))) ;
	    ResetState(sp);
	    break;

	case CASE_CHT:
	    TRACE(("CASE_CHT\n"));
	    /* cursor forward tabulation */
	    count = one_if_default(0);
	    while ((count-- > 0)
		   && (TabToNextStop(xw))) ;
	    ResetState(sp);
	    break;

	case CASE_TAB:
	    /* tab */
	    TabToNextStop(xw);
	    break;

	case CASE_SI:
	    screen->curgl = 0;
	    if_OPT_VT52_MODE(screen, {
		ResetState(sp);
	    });
	    break;

	case CASE_SO:
	    screen->curgl = 1;
	    if_OPT_VT52_MODE(screen, {
		ResetState(sp);
	    });
	    break;

	case CASE_DECDHL:
	    TRACE(("CASE_DECDHL - double-height line: %s\n",
		   (AsciiOf(c) == '3') ? "top" : "bottom"));
	    xterm_DECDHL(xw, AsciiOf(c) == '3');
	    ResetState(sp);
	    break;

	case CASE_DECSWL:
	    TRACE(("CASE_DECSWL - single-width line\n"));
	    xterm_DECSWL(xw);
	    ResetState(sp);
	    break;

	case CASE_DECDWL:
	    TRACE(("CASE_DECDWL - double-width line\n"));
	    xterm_DECDWL(xw);
	    ResetState(sp);
	    break;

	case CASE_SCR_STATE:
	    /* enter scr state */
	    sp->parsestate = scrtable;
	    break;

	case CASE_SCS0_STATE:
	    /* enter scs state 0 */
	    select_charset(sp, 0, 94);
	    break;

	case CASE_SCS1_STATE:
	    /* enter scs state 1 */
	    select_charset(sp, 1, 94);
	    break;

	case CASE_SCS2_STATE:
	    /* enter scs state 2 */
	    select_charset(sp, 2, 94);
	    break;

	case CASE_SCS3_STATE:
	    /* enter scs state 3 */
	    select_charset(sp, 3, 94);
	    break;

	case CASE_SCS1A_STATE:
	    /* enter scs state 1 */
	    select_charset(sp, 1, 96);
	    break;

	case CASE_SCS2A_STATE:
	    /* enter scs state 2 */
	    select_charset(sp, 2, 96);
	    break;

	case CASE_SCS3A_STATE:
	    /* enter scs state 3 */
	    select_charset(sp, 3, 96);
	    break;

	case CASE_ESC_IGNORE:
	    /* unknown escape sequence */
	    sp->parsestate = eigtable;
	    break;

	case CASE_ESC_DIGIT:
	    /* digit in csi or dec mode */
	    if (nparam > 0) {
		value = zero_if_default(nparam - 1);
		SetParam(nparam - 1, (10 * value) + (int) (AsciiOf(c) - '0'));
		if (GetParam(nparam - 1) > MAX_I_PARAM)
		    SetParam(nparam - 1, MAX_I_PARAM);
		if (sp->parsestate == csi_table)
		    sp->parsestate = csi2_table;
	    }
	    break;

	case CASE_ESC_SEMI:
	    /* semicolon in csi or dec mode */
	    if (nparam < NPARAM) {
		parms.is_sub[nparam] = 0;
		SetParam(nparam++, DEFAULT);
	    }
	    if (sp->parsestate == csi_table)
		sp->parsestate = csi2_table;
	    break;

	    /*
	     * A _few_ commands accept colon-separated subparameters.
	     * Mark the parameter list so that we can exclude (most) bogus
	     * commands with simple/fast checks.
	     */
	case CASE_ESC_COLON:
	    if (nparam < NPARAM) {
		parms.has_subparams = 1;
		if (nparam == 0) {
		    parms.is_sub[nparam] = 1;
		    SetParam(nparam++, DEFAULT);
		} else if (parms.is_sub[nparam - 1] == 0) {
		    parms.is_sub[nparam - 1] = 1;
		    parms.is_sub[nparam] = 2;
		    parms.params[nparam] = 0;
		    ++nparam;
		} else {
		    parms.is_sub[nparam] = 1 + parms.is_sub[nparam - 1];
		    parms.params[nparam] = 0;
		    ++nparam;
		}
	    }
	    break;

	case CASE_DEC_STATE:
	    /* enter dec mode */
	    sp->parsestate = dec_table;
	    break;

	case CASE_DEC2_STATE:
	    /* enter dec2 mode */
	    sp->parsestate = dec2_table;
	    break;

	case CASE_DEC3_STATE:
	    /* enter dec3 mode */
	    sp->parsestate = dec3_table;
	    break;

	case CASE_ICH:
	    TRACE(("CASE_ICH - insert char\n"));
	    InsertChar(xw, (unsigned) one_if_default(0));
	    ResetState(sp);
	    break;

	case CASE_CUU:
	    TRACE(("CASE_CUU - cursor up\n"));
	    CursorUp(screen, one_if_default(0));
	    ResetState(sp);
	    break;

	case CASE_CUD:
	    TRACE(("CASE_CUD - cursor down\n"));
	    CursorDown(screen, one_if_default(0));
	    ResetState(sp);
	    break;

	case CASE_CUF:
	    TRACE(("CASE_CUF - cursor forward\n"));
	    CursorForward(xw, one_if_default(0));
	    ResetState(sp);
	    break;

	case CASE_CUB:
	    TRACE(("CASE_CUB - cursor backward\n"));
	    CursorBack(xw, one_if_default(0));
	    ResetState(sp);
	    break;

	case CASE_CUP:
	    TRACE(("CASE_CUP - cursor position\n"));
	    if_OPT_XMC_GLITCH(screen, {
		Jump_XMC(xw);
	    });
	    CursorSet(screen, one_if_default(0) - 1, one_if_default(1) - 1, xw->flags);
	    ResetState(sp);
	    break;

	case CASE_VPA:
	    TRACE(("CASE_VPA - vertical position absolute\n"));
	    CursorSet(screen, one_if_default(0) - 1, CursorCol(xw), xw->flags);
	    ResetState(sp);
	    break;

	case CASE_HPA:
	    TRACE(("CASE_HPA - horizontal position absolute\n"));
	    CursorSet(screen, CursorRow(xw), one_if_default(0) - 1, xw->flags);
	    ResetState(sp);
	    break;

	case CASE_VPR:
	    TRACE(("CASE_VPR - vertical position relative\n"));
	    CursorSet(screen,
		      CursorRow(xw) + one_if_default(0),
		      CursorCol(xw),
		      xw->flags);
	    ResetState(sp);
	    break;

	case CASE_HPR:
	    TRACE(("CASE_HPR - horizontal position relative\n"));
	    CursorSet(screen,
		      CursorRow(xw),
		      CursorCol(xw) + one_if_default(0),
		      xw->flags);
	    ResetState(sp);
	    break;

	case CASE_HP_BUGGY_LL:
	    TRACE(("CASE_HP_BUGGY_LL\n"));
	    /* Some HP-UX applications have the bug that they
	       assume ESC F goes to the lower left corner of
	       the screen, regardless of what terminfo says. */
	    if (screen->hp_ll_bc)
		CursorSet(screen, screen->max_row, 0, xw->flags);
	    ResetState(sp);
	    break;

	case CASE_ED:
	    TRACE(("CASE_ED - erase display\n"));
	    do_cd_xtra_scroll(xw, zero_if_default(0));
	    do_erase_display(xw, zero_if_default(0), OFF_PROTECT);
	    ResetState(sp);
	    break;

	case CASE_EL:
	    TRACE(("CASE_EL - erase line\n"));
	    do_erase_line(xw, zero_if_default(0), OFF_PROTECT);
	    ResetState(sp);
	    break;

	case CASE_ECH:
	    TRACE(("CASE_ECH - erase char\n"));
	    /* ECH */
	    do_erase_char(xw, one_if_default(0), OFF_PROTECT);
	    ResetState(sp);
	    break;

	case CASE_IL:
	    TRACE(("CASE_IL - insert line\n"));
	    InsertLine(xw, one_if_default(0));
	    ResetState(sp);
	    break;

	case CASE_DL:
	    TRACE(("CASE_DL - delete line\n"));
	    DeleteLine(xw, one_if_default(0), True);
	    ResetState(sp);
	    break;

	case CASE_DCH:
	    TRACE(("CASE_DCH - delete char\n"));
	    DeleteChar(xw, (unsigned) one_if_default(0));
	    ResetState(sp);
	    break;

	case CASE_TRACK_MOUSE:
	    /*
	     * A single parameter other than zero is always scroll-down.
	     * A zero-parameter is used to reset the mouse mode, and is
	     * not useful for scrolling anyway.
	     */
	    if (nparam > 1 || GetParam(0) == 0) {
		CELL start;

		TRACE(("CASE_TRACK_MOUSE\n"));
		/* Track mouse as long as in window and between
		 * specified rows
		 */
		start.row = one_if_default(2) - 1;
		start.col = GetParam(1) - 1;
		TrackMouse(xw,
			   GetParam(0),
			   &start,
			   GetParam(3) - 1, GetParam(4) - 2);
	    } else {
		TRACE(("CASE_SD - scroll down\n"));
		/* SD */
		RevScroll(xw, one_if_default(0));
		do_xevents(xw);
	    }
	    ResetState(sp);
	    break;

	case CASE_SD:
	    /*
	     * Cater to ECMA-48's typographical error...
	     */
	    TRACE(("CASE_SD - scroll down\n"));
	    RevScroll(xw, one_if_default(0));
	    do_xevents(xw);
	    ResetState(sp);
	    break;

	case CASE_DECID:
	    TRACE(("CASE_DECID\n"));
	    if_OPT_VT52_MODE(screen, {
		/*
		 * If xterm's started in VT52 mode, it's not emulating VT52
		 * within VT100, etc., so the terminal identifies differently.
		 */
		switch (screen->terminal_id) {
		case 50:
		    value = 'A';
		    break;
		case 52:
		    value = 'K';
		    break;
		case 55:
		    value = 'C';
		    break;
		default:
		    value = 'Z';
		    break;
		}
		unparseputc(xw, ANSI_ESC);
		unparseputc(xw, '/');
		unparseputc(xw, value);
		unparse_end(xw);
		ResetState(sp);
		break;
	    });
	    SetParam(0, DEFAULT);	/* Default ID parameter */
	    /* FALLTHRU */
	case CASE_DA1:
	    TRACE(("CASE_DA1\n"));
	    if (GetParam(0) <= 0) {	/* less than means DEFAULT */
		count = 0;
		init_reply(ANSI_CSI);
		reply.a_pintro = '?';

		/*
		 * The first parameter corresponds to the highest operating
		 * level (i.e., service level) of the emulation.  A DEC
		 * terminal can be setup to respond with a different DA
		 * response, but there's no control sequence that modifies
		 * this.  We set it via a resource.
		 */
		if (screen->display_da1 < 200) {
		    switch (screen->display_da1) {
		    case 132:
			reply.a_param[count++] = 4;	/* VT132 */
#if OPT_REGIS_GRAPHICS
			reply.a_param[count++] = 6;	/* no STP, AVO, GPO (ReGIS) */
#else
			reply.a_param[count++] = 2;	/* no STP, AVO, no GPO (ReGIS) */
#endif
			break;
		    case 131:
			reply.a_param[count++] = 7;	/* VT131 */
			break;
		    case 125:
			reply.a_param[count++] = 12;	/* VT125 */
#if OPT_REGIS_GRAPHICS
			reply.a_param[count++] = 0 | 2 | 1;	/* no STP, AVO, GPO (ReGIS) */
#else
			reply.a_param[count++] = 0 | 2 | 0;	/* no STP, AVO, no GPO (ReGIS) */
#endif
			reply.a_param[count++] = 0;	/* no printer */
			reply.a_param[count++] = XTERM_PATCH;	/* ROM version */
			break;
		    case 102:
			reply.a_param[count++] = 6;	/* VT102 */
			break;
		    case 101:
			reply.a_param[count++] = 1;	/* VT101 */
			reply.a_param[count++] = 0;	/* no options */
			break;
		    default:	/* VT100 */
			reply.a_param[count++] = 1;	/* VT100 */
			reply.a_param[count++] = 2;	/* no STP, AVO, no GPO (ReGIS) */
			break;
		    }
		} else {
		    reply.a_param[count++] = (ParmType) (60
							 + screen->display_da1
							 / 100);
		    reply.a_param[count++] = 1;		/* 132-columns */
		    reply.a_param[count++] = 2;		/* printer */
#if OPT_REGIS_GRAPHICS
		    if (optRegisGraphics(screen)) {
			reply.a_param[count++] = 3;	/* ReGIS graphics */
		    }
#endif
#if OPT_SIXEL_GRAPHICS
		    if (optSixelGraphics(screen)) {
			reply.a_param[count++] = 4;	/* sixel graphics */
		    }
#endif
		    reply.a_param[count++] = 6;		/* selective-erase */
#if OPT_SUNPC_KBD
		    if (xw->keyboard.type == keyboardIsVT220)
#endif
			reply.a_param[count++] = 8;	/* user-defined-keys */
		    reply.a_param[count++] = 9;		/* national replacement charsets */
		    reply.a_param[count++] = 15;	/* technical characters */
		    reply.a_param[count++] = 16;	/* locator port */
		    if (screen->display_da1 >= 400) {
			reply.a_param[count++] = 17;	/* terminal state interrogation */
			reply.a_param[count++] = 18;	/* windowing extension */
			reply.a_param[count++] = 21;	/* horizontal scrolling */
		    }
		    if_OPT_ISO_COLORS(screen, {
			reply.a_param[count++] = 22;	/* ANSI color, VT525 */
		    });
		    reply.a_param[count++] = 28;	/* rectangular editing */
#if OPT_DEC_LOCATOR
		    reply.a_param[count++] = 29;	/* ANSI text locator */
#endif
		}
		reply.a_nparam = (ParmType) count;
		reply.a_final = 'c';
		unparseseq(xw, &reply);
	    }
	    ResetState(sp);
	    break;

	case CASE_DA2:
	    TRACE(("CASE_DA2\n"));
	    if (GetParam(0) <= 0) {	/* less than means DEFAULT */
		count = 0;
		init_reply(ANSI_CSI);
		reply.a_pintro = '>';

		if (screen->terminal_id >= 200) {
		    switch (screen->terminal_id) {
		    case 220:
		    default:
			reply.a_param[count++] = 1;	/* VT220 */
			break;
		    case 240:
		    case 241:
			/* http://www.decuslib.com/DECUS/vax87a/gendyn/vt200_kind.lis */
			reply.a_param[count++] = 2;	/* VT240 */
			break;
		    case 320:
			/* http://www.vt100.net/docs/vt320-uu/appendixe.html */
			reply.a_param[count++] = 24;	/* VT320 */
			break;
		    case 330:
			reply.a_param[count++] = 18;	/* VT330 */
			break;
		    case 340:
			reply.a_param[count++] = 19;	/* VT340 */
			break;
		    case 382:
			reply.a_param[count++] = 32;	/* VT382 */
			break;
		    case 420:
			reply.a_param[count++] = 41;	/* VT420 */
			break;
		    case 510:
			/* http://www.vt100.net/docs/vt510-rm/DA2 */
			reply.a_param[count++] = 61;	/* VT510 */
			break;
		    case 520:
			reply.a_param[count++] = 64;	/* VT520 */
			break;
		    case 525:
			reply.a_param[count++] = 65;	/* VT525 */
			break;
		    }
		} else {
		    reply.a_param[count++] = 0;		/* VT100 (nonstandard) */
		}
		reply.a_param[count++] = XTERM_PATCH;	/* Version */
		reply.a_param[count++] = 0;	/* options (none) */
		reply.a_nparam = (ParmType) count;
		reply.a_final = 'c';
		unparseseq(xw, &reply);
	    }
	    ResetState(sp);
	    break;

	case CASE_DECRPTUI:
	    TRACE(("CASE_DECRPTUI\n"));
	    if ((screen->vtXX_level >= 4)
		&& (GetParam(0) <= 0)) {	/* less than means DEFAULT */
		unparseputc1(xw, ANSI_DCS);
		unparseputc(xw, '!');
		unparseputc(xw, '|');
		/* report the "terminal unit id" as 4 pairs of hexadecimal
		 * digits -- meaningless for a terminal emulator, but some
		 * host may care about the format.
		 */
		for (count = 0; count < 8; ++count) {
		    unparseputc(xw, '0');
		}
		unparseputc1(xw, ANSI_ST);
		unparse_end(xw);
	    }
	    ResetState(sp);
	    break;

	case CASE_TBC:
	    TRACE(("CASE_TBC - tab clear\n"));
	    if ((value = GetParam(0)) <= 0)	/* less than means default */
		TabClear(xw->tabs, screen->cur_col);
	    else if (value == 3)
		TabZonk(xw->tabs);
	    ResetState(sp);
	    break;

	case CASE_SET:
	    TRACE(("CASE_SET - set mode\n"));
	    ansi_modes(xw, bitset);
	    ResetState(sp);
	    break;

	case CASE_RST:
	    TRACE(("CASE_RST - reset mode\n"));
	    ansi_modes(xw, bitclr);
	    ResetState(sp);
	    break;

	case CASE_SGR:
	    for (item = 0; item < nparam; ++item) {
		int op = GetParam(item);
		int skip;

		if_OPT_XMC_GLITCH(screen, {
		    Mark_XMC(xw, op);
		});
		TRACE(("CASE_SGR %d\n", op));

		/*
		 * Only SGR 38/48 accept subparameters, and in those cases
		 * the values will not be seen at this point.
		 */
		if ((skip = param_has_subparams(item)) != 0) {
		    switch (op) {
		    case 38:
			/* FALLTHRU */
		    case 48:
			if_OPT_ISO_COLORS(screen, {
			    break;
			});
			/* FALLTHRU */
		    default:
			TRACE(("...unexpected subparameter in SGR\n"));
			item += skip;	/* ignore this */
			op = 9999;	/* will never use this, anyway */
			break;
		    }
		}

		switch (op) {
		case DEFAULT:
		    /* FALLTHRU */
		case 0:
		    resetRendition(xw);
		    if_OPT_ISO_COLORS(screen, {
			reset_SGR_Colors(xw);
		    });
		    break;
		case 1:	/* Bold                 */
		    UIntSet(xw->flags, BOLD);
		    if_OPT_ISO_COLORS(screen, {
			setExtendedFG(xw);
		    });
		    break;
#if OPT_WIDE_ATTRS
		case 2:	/* faint, decreased intensity or second colour */
		    UIntSet(xw->flags, ATR_FAINT);
		    if_OPT_ISO_COLORS(screen, {
			setExtendedFG(xw);
		    });
		    break;
		case 3:	/* italicized */
		    setItalicFont(xw, UseItalicFont(screen));
		    UIntSet(xw->flags, ATR_ITALIC);
		    if_OPT_ISO_COLORS(screen, {
			setExtendedFG(xw);
		    });
		    break;
#endif
		case 4:	/* Underscore           */
		    UIntSet(xw->flags, UNDERLINE);
		    if_OPT_ISO_COLORS(screen, {
			setExtendedFG(xw);
		    });
		    break;
		case 5:	/* Blink (less than 150 per minute) */
		    /* FALLTHRU */
		case 6:	/* Blink (150 per minute, or more) */
		    UIntSet(xw->flags, BLINK);
		    StartBlinking(xw);
		    if_OPT_ISO_COLORS(screen, {
			setExtendedFG(xw);
		    });
		    break;
		case 7:
		    UIntSet(xw->flags, INVERSE);
		    if_OPT_ISO_COLORS(screen, {
			setExtendedBG(xw);
		    });
		    break;
		case 8:
		    UIntSet(xw->flags, INVISIBLE);
		    break;
#if OPT_WIDE_ATTRS
		case 9:	/* crossed-out characters */
		    UIntSet(xw->flags, ATR_STRIKEOUT);
		    break;
#endif
#if OPT_WIDE_ATTRS
		case 21:	/* doubly-underlined */
		    UIntSet(xw->flags, ATR_DBL_UNDER);
		    break;
#endif
		case 22:	/* reset 'bold' */
		    UIntClr(xw->flags, BOLD);
#if OPT_WIDE_ATTRS
		    UIntClr(xw->flags, ATR_FAINT);
#endif
		    if_OPT_ISO_COLORS(screen, {
			setExtendedFG(xw);
		    });
		    break;
#if OPT_WIDE_ATTRS
		case 23:	/* not italicized */
		    ResetItalics(xw);
		    if_OPT_ISO_COLORS(screen, {
			setExtendedFG(xw);
		    });
		    break;
#endif
		case 24:
		    UIntClr(xw->flags, UNDERLINE);
#if OPT_WIDE_ATTRS
		    UIntClr(xw->flags, ATR_DBL_UNDER);
#endif
		    if_OPT_ISO_COLORS(screen, {
			setExtendedFG(xw);
		    });
		    break;
		case 25:	/* reset 'blink' */
		    UIntClr(xw->flags, BLINK);
		    if_OPT_ISO_COLORS(screen, {
			setExtendedFG(xw);
		    });
		    break;
		case 27:
		    UIntClr(xw->flags, INVERSE);
		    if_OPT_ISO_COLORS(screen, {
			setExtendedBG(xw);
		    });
		    break;
		case 28:
		    UIntClr(xw->flags, INVISIBLE);
		    break;
#if OPT_WIDE_ATTRS
		case 29:	/* not crossed out */
		    UIntClr(xw->flags, ATR_STRIKEOUT);
		    break;
#endif
		case 30:
		    /* FALLTHRU */
		case 31:
		    /* FALLTHRU */
		case 32:
		    /* FALLTHRU */
		case 33:
		    /* FALLTHRU */
		case 34:
		    /* FALLTHRU */
		case 35:
		    /* FALLTHRU */
		case 36:
		    /* FALLTHRU */
		case 37:
		    if_OPT_ISO_COLORS(screen, {
			xw->sgr_foreground = (op - 30);
			xw->sgr_38_xcolors = False;
			clrDirectFG(xw->flags);
			setExtendedFG(xw);
		    });
		    break;
		case 38:
		    /* This is more complicated than I'd like, but it should
		     * properly eat all the parameters for unsupported modes.
		     */
		    if_OPT_ISO_COLORS(screen, {
			Boolean extended;
			if (parse_extended_colors(xw, &value, &item,
						  &extended)) {
			    xw->sgr_foreground = value;
			    xw->sgr_38_xcolors = True;
			    setDirectFG(xw->flags, extended);
			    setExtendedFG(xw);
			}
		    });
		    break;
		case 39:
		    if_OPT_ISO_COLORS(screen, {
			reset_SGR_Foreground(xw);
		    });
		    break;
		case 40:
		    /* FALLTHRU */
		case 41:
		    /* FALLTHRU */
		case 42:
		    /* FALLTHRU */
		case 43:
		    /* FALLTHRU */
		case 44:
		    /* FALLTHRU */
		case 45:
		    /* FALLTHRU */
		case 46:
		    /* FALLTHRU */
		case 47:
		    if_OPT_ISO_COLORS(screen, {
			xw->sgr_background = (op - 40);
			clrDirectBG(xw->flags);
			setExtendedBG(xw);
		    });
		    break;
		case 48:
		    if_OPT_ISO_COLORS(screen, {
			Boolean extended;
			if (parse_extended_colors(xw, &value, &item,
						  &extended)) {
			    xw->sgr_background = value;
			    setDirectBG(xw->flags, extended);
			    setExtendedBG(xw);
			}
		    });
		    break;
		case 49:
		    if_OPT_ISO_COLORS(screen, {
			reset_SGR_Background(xw);
		    });
		    break;
		case 90:
		    /* FALLTHRU */
		case 91:
		    /* FALLTHRU */
		case 92:
		    /* FALLTHRU */
		case 93:
		    /* FALLTHRU */
		case 94:
		    /* FALLTHRU */
		case 95:
		    /* FALLTHRU */
		case 96:
		    /* FALLTHRU */
		case 97:
		    if_OPT_AIX_COLORS(screen, {
			xw->sgr_foreground = (op - 90 + 8);
			clrDirectFG(xw->flags);
			setExtendedFG(xw);
		    });
		    break;
		case 100:
#if !OPT_AIX_COLORS
		    if_OPT_ISO_COLORS(screen, {
			reset_SGR_Foreground(xw);
			reset_SGR_Background(xw);
		    });
		    break;
#endif
		case 101:
		    /* FALLTHRU */
		case 102:
		    /* FALLTHRU */
		case 103:
		    /* FALLTHRU */
		case 104:
		    /* FALLTHRU */
		case 105:
		    /* FALLTHRU */
		case 106:
		    /* FALLTHRU */
		case 107:
		    if_OPT_AIX_COLORS(screen, {
			xw->sgr_background = (op - 100 + 8);
			clrDirectBG(xw->flags);
			setExtendedBG(xw);
		    });
		    break;
		default:
		    /* later: skip += NPARAM; */
		    break;
		}
	    }
	    ResetState(sp);
	    break;

	    /* DSR (except for the '?') is a superset of CPR */
	case CASE_DSR:
	    sp->private_function = True;

	    /* FALLTHRU */
	case CASE_CPR:
	    TRACE(("CASE_DSR - device status report\n"));
	    count = 0;
	    init_reply(ANSI_CSI);
	    reply.a_pintro = CharOf(sp->private_function ? '?' : 0);
	    reply.a_final = 'n';

	    switch (GetParam(0)) {
	    case 5:
		TRACE(("...request operating status\n"));
		/* operating status */
		reply.a_param[count++] = 0;	/* (no malfunction ;-) */
		break;
	    case 6:
		TRACE(("...request %s\n",
		       (sp->private_function
			? "DECXCPR"
			: "CPR")));
		/* CPR */
		/* DECXCPR (with page=1) */
		value = screen->cur_row;
		if ((xw->flags & ORIGIN) != 0) {
		    value -= screen->top_marg;
		}
		if_STATUS_LINE(screen, {
		    if ((value -= LastRowNumber(screen)) < 0)
			value = 0;
		});
		reply.a_param[count++] = (ParmType) (value + 1);

		value = (screen->cur_col + 1);
		if ((xw->flags & ORIGIN) != 0) {
		    value -= screen->lft_marg;
		}
		reply.a_param[count++] = (ParmType) value;

		if (sp->private_function &&
		    (screen->vtXX_level >= 4 ||
		     (screen->terminal_id >= 330 &&
		      screen->vtXX_level >= 3))) {
		    /* VT330 (not VT320) and VT420 */
		    reply.a_param[count++] = 1;
		}
		reply.a_final = 'R';
		break;
	    case 15:
		TRACE(("...request printer status\n"));
		if (sp->private_function
		    && screen->vtXX_level >= 2) {	/* VT220 */
		    reply.a_param[count++] = 13;	/* no printer detected */
		}
		break;
	    case 25:
		TRACE(("...request UDK status\n"));
		if (sp->private_function
		    && screen->vtXX_level >= 2) {	/* VT220 */
		    reply.a_param[count++] = 20;	/* UDK always unlocked */
		}
		break;
	    case 26:
		TRACE(("...request keyboard status\n"));
		if (sp->private_function
		    && screen->vtXX_level >= 2) {	/* VT220 */
		    reply.a_param[count++] = 27;
		    reply.a_param[count++] = 1;		/* North American */
		    if (screen->vtXX_level >= 3) {	/* VT320 */
			reply.a_param[count++] = 0;	/* ready */
		    }
		    if (screen->vtXX_level >= 4) {	/* VT420 */
			reply.a_param[count++] = 0;	/* LK201 */
		    }
		}
		break;
	    case 55:		/* according to the VT330/VT340 Text Programming Manual */
		TRACE(("...request locator status\n"));
		if (sp->private_function
		    && screen->vtXX_level >= 3) {	/* VT330 */
#if OPT_DEC_LOCATOR
		    reply.a_param[count++] = 50;	/* locator ready */
#else
		    reply.a_param[count++] = 53;	/* no locator */
#endif
		}
		break;
	    case 56:
		TRACE(("...request locator type\n"));
		if (sp->private_function
		    && screen->vtXX_level >= 3) {	/* VT330 */
		    reply.a_param[count++] = 57;
#if OPT_DEC_LOCATOR
		    reply.a_param[count++] = 1;		/* mouse */
#else
		    reply.a_param[count++] = 0;		/* unknown */
#endif
		}
		break;
	    case 62:
		TRACE(("...request DECMSR - macro space\n"));
		if (sp->private_function
		    && screen->vtXX_level >= 4) {	/* VT420 */
		    reply.a_pintro = 0;
		    reply.a_radix[count] = 16;	/* no data */
		    reply.a_param[count++] = 0;		/* no space for macros */
		    reply.a_inters = '*';
		    reply.a_final = L_CURL;
		}
		break;
	    case 63:
		TRACE(("...request DECCKSR - memory checksum\n"));
		/* DECCKSR - Memory checksum */
		if (sp->private_function
		    && screen->vtXX_level >= 4) {	/* VT420 */
		    init_reply(ANSI_DCS);
		    reply.a_param[count++] = (ParmType) GetParam(1);	/* PID */
		    reply.a_delim = "!~";	/* delimiter */
		    reply.a_radix[count] = 16;	/* use hex */
		    reply.a_param[count++] = 0;		/* no data */
		}
		break;
	    case 75:
		TRACE(("...request data integrity\n"));
		if (sp->private_function
		    && screen->vtXX_level >= 4) {	/* VT420 */
		    reply.a_param[count++] = 70;	/* no errors */
		}
		break;
	    case 85:
		TRACE(("...request multi-session configuration\n"));
		if (sp->private_function
		    && screen->vtXX_level >= 4) {	/* VT420 */
		    reply.a_param[count++] = 83;	/* not configured */
		}
		break;
	    default:
		break;
	    }

	    if ((reply.a_nparam = (ParmType) count) != 0)
		unparseseq(xw, &reply);

	    ResetState(sp);
	    sp->private_function = False;
	    break;

	case CASE_MC:
	    TRACE(("CASE_MC - media control\n"));
	    xtermMediaControl(xw, GetParam(0), False);
	    ResetState(sp);
	    break;

	case CASE_DEC_MC:
	    TRACE(("CASE_DEC_MC - DEC media control\n"));
	    xtermMediaControl(xw, GetParam(0), True);
	    ResetState(sp);
	    break;

	case CASE_HP_MEM_LOCK:
	    /* FALLTHRU */
	case CASE_HP_MEM_UNLOCK:
	    TRACE(("%s\n", ((sp->parsestate[c] == CASE_HP_MEM_LOCK)
			    ? "CASE_HP_MEM_LOCK"
			    : "CASE_HP_MEM_UNLOCK")));
	    if (screen->scroll_amt)
		FlushScroll(xw);
	    if (sp->parsestate[c] == CASE_HP_MEM_LOCK)
		set_tb_margins(screen, screen->cur_row, screen->bot_marg);
	    else
		set_tb_margins(screen, 0, screen->bot_marg);
	    ResetState(sp);
	    break;

	case CASE_DECSTBM:
	    TRACE(("CASE_DECSTBM - set scrolling region\n"));
	    {
		int top;
		int bot;
		top = one_if_default(0);
		if (nparam < 2 || (bot = GetParam(1)) == DEFAULT
		    || bot > MaxRows(screen)
		    || bot == 0)
		    bot = MaxRows(screen);
		if (bot > top) {
		    if (screen->scroll_amt)
			FlushScroll(xw);
		    set_tb_margins(screen, top - 1, bot - 1);
		    CursorSet(screen, 0, 0, xw->flags);
		}
		ResetState(sp);
	    }
	    break;

	case CASE_DECREQTPARM:
	    TRACE(("CASE_DECREQTPARM\n"));
	    if (screen->terminal_id < 200) {	/* VT102 */
		value = zero_if_default(0);
		if (value == 0 || value == 1) {
		    init_reply(ANSI_CSI);
		    reply.a_nparam = 7;
		    reply.a_param[0] = (ParmType) (value + 2);
		    reply.a_param[1] = 1;	/* no parity */
		    reply.a_param[2] = 1;	/* eight bits */
		    reply.a_param[3] = 128;	/* transmit 38.4k baud */
		    reply.a_param[4] = 128;	/* receive 38.4k baud */
		    reply.a_param[5] = 1;	/* clock multiplier ? */
		    reply.a_param[6] = 0;	/* STP flags ? */
		    reply.a_final = 'x';
		    unparseseq(xw, &reply);
		}
	    }
	    ResetState(sp);
	    break;

	case CASE_DECSET:
	    /* DECSET */
#if OPT_VT52_MODE
	    if (screen->vtXX_level != 0)
#endif
		dpmodes(xw, bitset);
	    ResetState(sp);
#if OPT_TEK4014
	    if (TEK4014_ACTIVE(xw)) {
		TRACE(("Tek4014 is now active...\n"));
		if (sp->check_recur)
		    sp->check_recur--;
		return False;
	    }
#endif
	    break;

	case CASE_DECRST:
	    /* DECRST */
	    dpmodes(xw, bitclr);
	    init_groundtable(screen, sp);
	    ResetState(sp);
	    break;

	case CASE_DECALN:
	    TRACE(("CASE_DECALN - alignment test\n"));
	    if (screen->cursor_state)
		HideCursor(xw);
	    /*
	     * DEC STD 070 (see pages D-19 to D-20) does not mention left/right
	     * margins.  The section is dated March 1985, not updated for the
	     * VT420 (introduced in 1990).
	     */
	    UIntClr(xw->flags, ORIGIN);
	    screen->do_wrap = False;
	    resetRendition(xw);
	    resetMargins(xw);
	    xterm_ResetDouble(xw);
	    CursorSet(screen, 0, 0, xw->flags);
	    xtermParseRect(xw, 0, NULL, &myRect);
	    ScrnFillRectangle(xw, &myRect, 'E', nrc_ASCII, 0, False);
	    ResetState(sp);
	    break;

	case CASE_GSETS5:
	    if (screen->vtXX_level >= 5) {
		TRACE_GSETS("5");
		xtermDecodeSCS(xw, sp->scstype, 5, 0, (int) c);
	    }
	    ResetState(sp);
	    break;

	case CASE_GSETS3:
	    if (screen->vtXX_level >= 3) {
		TRACE_GSETS("3");
		xtermDecodeSCS(xw, sp->scstype, 3, 0, (int) c);
	    }
	    ResetState(sp);
	    break;

	case CASE_GSETS:
	    if (strchr("012AB", AsciiOf(c)) != NULL) {
		TRACE_GSETS("");
		xtermDecodeSCS(xw, sp->scstype, 1, 0, (int) c);
	    } else if (screen->vtXX_level >= 2) {
		TRACE_GSETS("");
		xtermDecodeSCS(xw, sp->scstype, 2, 0, (int) c);
	    }
	    ResetState(sp);
	    break;

	case CASE_ANSI_SC:
	    if (IsLeftRightMode(xw)) {
		int left;
		int right;

		TRACE(("CASE_DECSLRM - set left and right margin\n"));
		left = one_if_default(0);
		if (nparam < 2 || (right = GetParam(1)) == DEFAULT
		    || right > MaxCols(screen)
		    || right == 0)
		    right = MaxCols(screen);
		if (right > left) {
		    set_lr_margins(screen, left - 1, right - 1);
		    CursorSet(screen, 0, 0, xw->flags);
		}
	    } else if (only_default()) {
		TRACE(("CASE_ANSI_SC - save cursor\n"));
		CursorSave(xw);
	    }
	    ResetState(sp);
	    break;

	case CASE_DECSC:
	    TRACE(("CASE_DECSC - save cursor\n"));
	    CursorSave(xw);
	    ResetState(sp);
	    break;

	case CASE_ANSI_RC:
	    if (!only_default())
		break;
	    /* FALLTHRU */
	case CASE_DECRC:
	    TRACE(("CASE_%sRC - restore cursor\n",
		   (sp->nextstate == CASE_DECRC) ? "DEC" : "ANSI_"));
	    CursorRestore(xw);
	    if_OPT_ISO_COLORS(screen, {
		setExtendedFG(xw);
	    });
	    ResetState(sp);
	    break;

	case CASE_DECKPAM:
	    TRACE(("CASE_DECKPAM\n"));
	    xw->keyboard.flags |= MODE_DECKPAM;
	    update_appkeypad();
	    ResetState(sp);
	    break;

	case CASE_DECKPNM:
	    TRACE(("CASE_DECKPNM\n"));
	    UIntClr(xw->keyboard.flags, MODE_DECKPAM);
	    update_appkeypad();
	    ResetState(sp);
	    break;

	case CASE_CSI_QUOTE_STATE:
	    sp->parsestate = csi_quo_table;
	    break;

#if OPT_BLINK_CURS
	case CASE_CSI_SPACE_STATE:
	    sp->parsestate = csi_sp_table;
	    break;

	case CASE_DECSCUSR:
	    TRACE(("CASE_DECSCUSR\n"));
	    {
		Boolean change;
		int blinks = screen->cursor_blink_esc;
		XtCursorShape shapes = screen->cursor_shape;

		HideCursor(xw);

		switch (GetParam(0)) {
		case DEFAULT:
		    /* FALLTHRU */
		case DEFAULT_STYLE:
		    /* FALLTHRU */
		case BLINK_BLOCK:
		    blinks = True;
		    screen->cursor_shape = CURSOR_BLOCK;
		    break;
		case STEADY_BLOCK:
		    blinks = False;
		    screen->cursor_shape = CURSOR_BLOCK;
		    break;
		case BLINK_UNDERLINE:
		    blinks = True;
		    screen->cursor_shape = CURSOR_UNDERLINE;
		    break;
		case STEADY_UNDERLINE:
		    blinks = False;
		    screen->cursor_shape = CURSOR_UNDERLINE;
		    break;
		case BLINK_BAR:
		    blinks = True;
		    screen->cursor_shape = CURSOR_BAR;
		    break;
		case STEADY_BAR:
		    blinks = False;
		    screen->cursor_shape = CURSOR_BAR;
		    break;
		}
		change = (blinks != screen->cursor_blink_esc ||
			  shapes != screen->cursor_shape);
		TRACE(("cursor_shape:%d blinks:%d%s\n",
		       screen->cursor_shape, blinks,
		       change ? " (changed)" : ""));
		if (change) {
		    xtermSetCursorBox(screen);
		    if (SettableCursorBlink(screen)) {
			screen->cursor_blink_esc = blinks;
			UpdateCursorBlink(xw);
		    }
		}
	    }
	    ResetState(sp);
	    break;
#endif

#if OPT_SCROLL_LOCK
	case CASE_DECLL:
	    TRACE(("CASE_DECLL\n"));
	    if (nparam > 0) {
		for (count = 0; count < nparam; ++count) {
		    int op = zero_if_default(count);
		    switch (op) {
		    case 0:
		    case DEFAULT:
			xtermClearLEDs(screen);
			break;
		    case 1:
			/* FALLTHRU */
		    case 2:
			/* FALLTHRU */
		    case 3:
			xtermShowLED(screen,
				     (Cardinal) op,
				     True);
			break;
		    case 21:
			/* FALLTHRU */
		    case 22:
			/* FALLTHRU */
		    case 23:
			xtermShowLED(screen,
				     (Cardinal) (op - 20),
				     True);
			break;
		    }
		}
	    } else {
		xtermClearLEDs(screen);
	    }
	    ResetState(sp);
	    break;
#endif

#if OPT_VT52_MODE
	case CASE_VT52_FINISH:
	    TRACE(("CASE_VT52_FINISH terminal_id %d, vtXX_level %d\n",
		   screen->terminal_id,
		   screen->vtXX_level));
	    if (screen->terminal_id >= 100
		&& screen->vtXX_level == 0) {
		sp->groundtable =
		    sp->parsestate = ansi_table;
		/*
		 * On restore, the terminal does not recognize DECRQSS for
		 * DECSCL (per vttest).
		 */
		set_vtXX_level(screen, 1);
		xw->flags = screen->vt52_save_flags;
		screen->curgl = screen->vt52_save_curgl;
		screen->curgr = screen->vt52_save_curgr;
		screen->curss = screen->vt52_save_curss;
		restoreCharsets(screen, screen->vt52_save_gsets);
		update_vt52_vt100_settings();
	    }
	    break;
#endif

	case CASE_ANSI_LEVEL_1:
	    TRACE(("CASE_ANSI_LEVEL_1\n"));
	    set_ansi_conformance(screen, 1);
	    ResetState(sp);
	    break;

	case CASE_ANSI_LEVEL_2:
	    TRACE(("CASE_ANSI_LEVEL_2\n"));
	    set_ansi_conformance(screen, 2);
	    ResetState(sp);
	    break;

	case CASE_ANSI_LEVEL_3:
	    TRACE(("CASE_ANSI_LEVEL_3\n"));
	    set_ansi_conformance(screen, 3);
	    ResetState(sp);
	    break;

	case CASE_DECSCL:
	    TRACE(("CASE_DECSCL(%d,%d)\n", GetParam(0), GetParam(1)));
	    /*
	     * This changes the emulation level, and is not recognized by
	     * VT100s.  However, a VT220 or above can be set to conformance
	     * level 1 to act like a VT100.
	     */
	    if (screen->terminal_id >= 200) {
		/*
		 * Disallow unrecognized parameters, as well as attempts to set
		 * the operating level higher than the given terminal-id.
		 */
		if (GetParam(0) >= 61
		    && GetParam(0) <= 60 + (screen->terminal_id / 100)) {
		    int new_vtXX_level = GetParam(0) - 60;
		    int case_value = zero_if_default(1);
		    /*
		     * Note:
		     *
		     * The VT300, VT420, VT520 manuals claim that DECSCL does a
		     * hard reset (RIS).
		     *
		     * Both the VT220 manual and DEC STD 070 (which documents
		     * levels 1-4 in detail) state that it is a soft reset.
		     *
		     * Perhaps both sets of manuals are right (unlikely).
		     * Kermit says it's soft.
		     */
		    ReallyReset(xw, False, False);
		    init_parser(xw, sp);
		    set_vtXX_level(screen, new_vtXX_level);
		    if (new_vtXX_level > 1) {
			switch (case_value) {
			case 1:
			    show_8bit_control(False);
			    break;
			case 0:
			case 2:
			    show_8bit_control(True);
			    break;
			}
		    }
		}
	    }
	    ResetState(sp);
	    break;

	case CASE_DECSCA:
	    TRACE(("CASE_DECSCA\n"));
	    screen->protected_mode = DEC_PROTECT;
	    if (GetParam(0) <= 0 || GetParam(0) == 2) {
		UIntClr(xw->flags, PROTECTED);
		TRACE(("...clear PROTECTED\n"));
	    } else if (GetParam(0) == 1) {
		xw->flags |= PROTECTED;
		TRACE(("...set PROTECTED\n"));
	    }
	    ResetState(sp);
	    break;

	case CASE_DECSED:
	    TRACE(("CASE_DECSED\n"));
	    do_erase_display(xw, zero_if_default(0), DEC_PROTECT);
	    ResetState(sp);
	    break;

	case CASE_DECSEL:
	    TRACE(("CASE_DECSEL\n"));
	    do_erase_line(xw, zero_if_default(0), DEC_PROTECT);
	    ResetState(sp);
	    break;

	case CASE_GRAPHICS_ATTRIBUTES:
#if OPT_GRAPHICS
	    TRACE(("CASE_GRAPHICS_ATTRIBUTES\n"));
	    {
		/* request: item, action, value */
		/* reply: item, status, value */
		if (nparam != 3) {
		    TRACE(("DATA_ERROR: malformed CASE_GRAPHICS_ATTRIBUTES request with %d parameters\n", nparam));
		} else {
		    int status = 3;	/* assume failure */
		    int result = 0;
		    int result2 = 0;

		    TRACE(("CASE_GRAPHICS_ATTRIBUTES request: %d, %d, %d\n",
			   GetParam(0), GetParam(1), GetParam(2)));
		    switch (GetParam(0)) {
		    case 1:	/* color register count */
			switch (GetParam(1)) {
			case 1:	/* read */
			    status = 0;		/* success */
			    result = (int) get_color_register_count(screen);
			    break;
			case 2:	/* reset */
			    screen->numcolorregisters = 0;
			    status = 0;		/* success */
			    result = (int) get_color_register_count(screen);
			    break;
			case 3:	/* set */
			    if (GetParam(2) > 1 &&
				(unsigned) GetParam(2) <= MAX_COLOR_REGISTERS) {
				screen->numcolorregisters = GetParam(2);
				status = 0;	/* success */
				result = (int) get_color_register_count(screen);
			    }
			    break;
			case 4:	/* read maximum */
			    status = 0;		/* success */
			    result = MAX_COLOR_REGISTERS;
			    break;
			default:
			    TRACE(("DATA_ERROR: CASE_GRAPHICS_ATTRIBUTES color register count request with unknown action parameter: %d\n",
				   GetParam(1)));
			    status = 2;		/* error in Pa */
			    break;
			}
			if (status == 0 && !(optSixelGraphics(screen)
					     || optRegisGraphics(screen)))
			    status = 3;
			break;
# if OPT_SIXEL_GRAPHICS
		    case 2:	/* graphics geometry */
			switch (GetParam(1)) {
			case 1:	/* read */
			    TRACE(("Get sixel graphics geometry\n"));
			    status = 0;		/* success */
			    result = Min(Width(screen), (int) screen->graphics_max_wide);
			    result2 = Min(Height(screen), (int) screen->graphics_max_high);
			    break;
			case 2:	/* reset */
			    /* FALLTHRU */
			case 3:	/* set */
			    break;
			case 4:	/* read maximum */
			    status = 0;		/* success */
			    result = screen->graphics_max_wide;
			    result2 = screen->graphics_max_high;
			    break;
			default:
			    TRACE(("DATA_ERROR: CASE_GRAPHICS_ATTRIBUTES graphics geometry request with unknown action parameter: %d\n",
				   GetParam(1)));
			    status = 2;		/* error in Pa */
			    break;
			}
			if (status == 0 && !optSixelGraphics(screen))
			    status = 3;
			break;
#endif
# if OPT_REGIS_GRAPHICS
		    case 3:	/* ReGIS geometry */
			switch (GetParam(1)) {
			case 1:	/* read */
			    status = 0;		/* success */
			    result = screen->graphics_regis_def_wide;
			    result2 = screen->graphics_regis_def_high;
			    break;
			case 2:	/* reset */
			    /* FALLTHRU */
			case 3:	/* set */
			    /* FALLTHRU */
			case 4:	/* read maximum */
			    /* not implemented */
			    break;
			default:
			    TRACE(("DATA_ERROR: CASE_GRAPHICS_ATTRIBUTES ReGIS geometry request with unknown action parameter: %d\n",
				   GetParam(1)));
			    status = 2;		/* error in Pa */
			    break;
			}
			if (status == 0 && !optRegisGraphics(screen))
			    status = 3;
			break;
#endif
		    default:
			TRACE(("DATA_ERROR: CASE_GRAPHICS_ATTRIBUTES request with unknown item parameter: %d\n",
			       GetParam(0)));
			status = 1;
			break;
		    }

		    init_reply(ANSI_CSI);
		    reply.a_pintro = '?';
		    count = 0;
		    reply.a_param[count++] = (ParmType) GetParam(0);
		    reply.a_param[count++] = (ParmType) status;
		    if (status == 0) {
			reply.a_param[count++] = (ParmType) result;
			if (GetParam(0) >= 2)
			    reply.a_param[count++] = (ParmType) result2;
		    }
		    reply.a_nparam = (ParmType) count;
		    reply.a_final = 'S';
		    unparseseq(xw, &reply);
		}
	    }
#endif
	    ResetState(sp);
	    break;

	case CASE_ST:
	    TRACE(("CASE_ST: End of String (%lu bytes) (mode=%d)\n",
		   (unsigned long) sp->string_used,
		   sp->string_mode));
	    ResetState(sp);
	    if (!sp->string_used && !sp->string_args)
		break;
	    if (sp->string_skip) {
		xtermWarning("Ignoring too-long string (%lu) for mode %#02x\n",
			     (unsigned long) sp->string_used,
			     sp->string_mode);
		sp->string_skip = False;
		sp->string_used = 0;
	    } else {
		if (sp->string_used)
		    sp->string_area[--(sp->string_used)] = '\0';
		if (sp->check_recur <= 1) {
		    switch (sp->string_mode) {
		    case ANSI_APC:
			/* ignored */
			break;
		    case ANSI_DCS:
#if OPT_SIXEL_GRAPHICS
			if (sp->string_args == sa_SIXEL) {
			    parse_sixel_finished();
			    TRACE(("DONE parsed sixel data\n"));
			} else
#endif
			    do_dcs(xw, sp->string_area, sp->string_used);
			break;
		    case ANSI_OSC:
			do_osc(xw, sp->string_area, sp->string_used, ANSI_ST);
			break;
		    case ANSI_PM:
			/* ignored */
			break;
		    case ANSI_SOS:
			/* ignored */
			break;
		    default:
			TRACE(("unknown mode\n"));
			break;
		    }
		}
	    }
	    break;

	case CASE_SOS:
	    TRACE(("CASE_SOS: Start of String\n"));
	    if (ParseSOS(screen)) {
		BeginString(ANSI_SOS);
	    } else {
		illegal_parse(xw, c, sp);
	    }
	    break;

	case CASE_PM:
	    TRACE(("CASE_PM: Privacy Message\n"));
	    if (ParseSOS(screen)) {
		BeginString(ANSI_PM);
	    } else {
		illegal_parse(xw, c, sp);
	    }
	    break;

	case CASE_DCS:
	    TRACE(("CASE_DCS: Device Control String\n"));
	    BeginString2(ANSI_DCS);
	    break;

	case CASE_APC:
	    TRACE(("CASE_APC: Application Program Command\n"));
	    if (ParseSOS(screen)) {
		BeginString(ANSI_APC);
	    } else {
		illegal_parse(xw, c, sp);
	    }
	    break;

	case CASE_SPA:
	    TRACE(("CASE_SPA - start protected area\n"));
	    screen->protected_mode = ISO_PROTECT;
	    xw->flags |= PROTECTED;
	    ResetState(sp);
	    break;

	case CASE_EPA:
	    TRACE(("CASE_EPA - end protected area\n"));
	    UIntClr(xw->flags, PROTECTED);
	    ResetState(sp);
	    break;

	case CASE_SU:
	    TRACE(("CASE_SU - scroll up\n"));
	    xtermScroll(xw, one_if_default(0));
	    ResetState(sp);
	    break;

	case CASE_SL:		/* ISO 6429, non-DEC */
	    TRACE(("CASE_SL - scroll left\n"));
	    xtermScrollLR(xw, one_if_default(0), True);
	    ResetState(sp);
	    break;

	case CASE_SR:		/* ISO 6429, non-DEC */
	    TRACE(("CASE_SR - scroll right\n"));
	    xtermScrollLR(xw, one_if_default(0), False);
	    ResetState(sp);
	    break;

	case CASE_DECDC:
	    TRACE(("CASE_DC - delete column\n"));
	    if (screen->vtXX_level >= 4) {
		xtermColScroll(xw, one_if_default(0), True, screen->cur_col);
	    }
	    ResetState(sp);
	    break;

	case CASE_DECIC:
	    TRACE(("CASE_IC - insert column\n"));
	    if (screen->vtXX_level >= 4) {
		xtermColScroll(xw, one_if_default(0), False, screen->cur_col);
	    }
	    ResetState(sp);
	    break;

	case CASE_DECBI:
	    TRACE(("CASE_BI - back index\n"));
	    if (screen->vtXX_level >= 4) {
		xtermColIndex(xw, True);
	    }
	    ResetState(sp);
	    break;

	case CASE_DECFI:
	    TRACE(("CASE_FI - forward index\n"));
	    if (screen->vtXX_level >= 4) {
		xtermColIndex(xw, False);
	    }
	    ResetState(sp);
	    break;

	case CASE_IND:
	    TRACE(("CASE_IND - index\n"));
	    xtermIndex(xw, 1);
	    do_xevents(xw);
	    ResetState(sp);
	    break;

	case CASE_CPL:
	    TRACE(("CASE_CPL - cursor prev line\n"));
	    CursorPrevLine(xw, one_if_default(0));
	    ResetState(sp);
	    break;

	case CASE_CNL:
	    TRACE(("CASE_CNL - cursor next line\n"));
	    CursorNextLine(xw, one_if_default(0));
	    ResetState(sp);
	    break;

	case CASE_NEL:
	    TRACE(("CASE_NEL\n"));
	    xtermIndex(xw, 1);
	    CarriageReturn(xw);
	    ResetState(sp);
	    break;

	case CASE_HTS:
	    TRACE(("CASE_HTS - horizontal tab set\n"));
	    TabSet(xw->tabs, screen->cur_col);
	    ResetState(sp);
	    break;

	case CASE_REPORT_VERSION:
	    TRACE(("CASE_REPORT_VERSION - report terminal version\n"));
	    if (GetParam(0) <= 0) {
		unparseputc1(xw, ANSI_DCS);
		unparseputc(xw, '>');
		unparseputc(xw, '|');
		unparseputs(xw, xtermVersion());
		unparseputc1(xw, ANSI_ST);
		unparse_end(xw);
	    }
	    ResetState(sp);
	    break;

	case CASE_RI:
	    TRACE(("CASE_RI - reverse index\n"));
	    RevIndex(xw, 1);
	    ResetState(sp);
	    break;

	case CASE_SS2:
	    TRACE(("CASE_SS2\n"));
	    if (screen->vtXX_level > 1)
		screen->curss = 2;
	    ResetState(sp);
	    break;

	case CASE_SS3:
	    TRACE(("CASE_SS3\n"));
	    if (screen->vtXX_level > 1)
		screen->curss = 3;
	    ResetState(sp);
	    break;

	case CASE_CSI_STATE:
	    /* enter csi state */
	    InitParams();
	    SetParam(nparam++, DEFAULT);
	    sp->parsestate = csi_table;
	    break;

	case CASE_ESC_SP_STATE:
	    /* esc space */
	    sp->parsestate = esc_sp_table;
	    break;

	case CASE_CSI_EX_STATE:
	    /* csi exclamation */
	    sp->parsestate = csi_ex_table;
	    break;

	case CASE_CSI_TICK_STATE:
	    /* csi tick (') */
	    sp->parsestate = csi_tick_table;
	    break;

#if OPT_DEC_LOCATOR
	case CASE_DECEFR:
	    TRACE(("CASE_DECEFR - Enable Filter Rectangle\n"));
	    if (okSendMousePos(xw) == DEC_LOCATOR) {
		MotionOff(screen, xw);
		if ((screen->loc_filter_top = GetParam(0)) < 1)
		    screen->loc_filter_top = LOC_FILTER_POS;
		if (nparam < 2
		    || (screen->loc_filter_left = GetParam(1)) < 1)
		    screen->loc_filter_left = LOC_FILTER_POS;
		if (nparam < 3
		    || (screen->loc_filter_bottom = GetParam(2)) < 1)
		    screen->loc_filter_bottom = LOC_FILTER_POS;
		if (nparam < 4
		    || (screen->loc_filter_right = GetParam(3)) < 1)
		    screen->loc_filter_right = LOC_FILTER_POS;
		InitLocatorFilter(xw);
	    }
	    ResetState(sp);
	    break;

	case CASE_DECELR:
	    MotionOff(screen, xw);
	    if (GetParam(0) <= 0 || GetParam(0) > 2) {
		screen->send_mouse_pos = MOUSE_OFF;
		TRACE(("DECELR - Disable Locator Reports\n"));
	    } else {
		TRACE(("DECELR - Enable Locator Reports\n"));
		screen->send_mouse_pos = DEC_LOCATOR;
		xtermShowPointer(xw, True);
		if (GetParam(0) == 2) {
		    screen->locator_reset = True;
		} else {
		    screen->locator_reset = False;
		}
		if (nparam < 2 || GetParam(1) != 1) {
		    screen->locator_pixels = False;
		} else {
		    screen->locator_pixels = True;
		}
		screen->loc_filter = False;
	    }
	    ResetState(sp);
	    break;

	case CASE_DECSLE:
	    TRACE(("DECSLE - Select Locator Events\n"));
	    for (count = 0; count < nparam; ++count) {
		switch (zero_if_default(count)) {
		case 0:
		    MotionOff(screen, xw);
		    screen->loc_filter = False;
		    screen->locator_events = 0;
		    break;
		case 1:
		    screen->locator_events |= LOC_BTNS_DN;
		    break;
		case 2:
		    UIntClr(screen->locator_events, LOC_BTNS_DN);
		    break;
		case 3:
		    screen->locator_events |= LOC_BTNS_UP;
		    break;
		case 4:
		    UIntClr(screen->locator_events, LOC_BTNS_UP);
		    break;
		}
	    }
	    ResetState(sp);
	    break;

	case CASE_DECRQLP:
	    TRACE(("DECRQLP - Request Locator Position\n"));
	    if (GetParam(0) < 2) {
		/* Issue DECLRP Locator Position Report */
		GetLocatorPosition(xw);
	    }
	    ResetState(sp);
	    break;
#endif /* OPT_DEC_LOCATOR */

	case CASE_CSI_AMP_STATE:
	    TRACE(("CASE_CSI_AMP_STATE\n"));
	    /* csi ampersand (&) */
	    if (screen->vtXX_level >= 3)
		sp->parsestate = csi_amp_table;
	    else
		sp->parsestate = eigtable;
	    break;

#if OPT_DEC_RECTOPS
	case CASE_CSI_DOLLAR_STATE:
	    TRACE(("CASE_CSI_DOLLAR_STATE\n"));
	    /* csi dollar ($) */
	    if (screen->vtXX_level >= 3)
		sp->parsestate = csi_dollar_table;
	    else
		sp->parsestate = eigtable;
	    break;

	case CASE_CSI_STAR_STATE:
	    TRACE(("CASE_CSI_STAR_STATE\n"));
	    /* csi star (*) */
	    if (screen->vtXX_level >= 4)
		sp->parsestate = csi_star_table;
	    else
		sp->parsestate = eigtable;
	    break;

	case CASE_DECRQCRA:
	    if (screen->vtXX_level >= 4 && AllowWindowOps(xw, ewGetChecksum)) {
		int checksum;
		int pid;

		TRACE(("CASE_DECRQCRA - Request checksum of rectangular area\n"));
		xtermCheckRect(xw, ParamPair(0), &checksum);
		init_reply(ANSI_DCS);
		count = 0;
		checksum &= 0xffff;
		pid = GetParam(0);
		reply.a_param[count++] = (ParmType) pid;
		reply.a_delim = "!~";	/* delimiter */
		reply.a_radix[count] = 16;
		reply.a_param[count++] = (ParmType) checksum;
		reply.a_nparam = (ParmType) count;
		TRACE(("...checksum(%d) = %04X\n", pid, checksum));
		unparseseq(xw, &reply);
	    }
	    ResetState(sp);
	    break;

	case CASE_DECCRA:
	    if (screen->vtXX_level >= 4) {
		TRACE(("CASE_DECCRA - Copy rectangular area\n"));
		xtermParseRect(xw, ParamPair(0), &myRect);
		ScrnCopyRectangle(xw, &myRect, ParamPair(5));
	    }
	    ResetState(sp);
	    break;

	case CASE_DECERA:
	    if (screen->vtXX_level >= 4) {
		TRACE(("CASE_DECERA - Erase rectangular area\n"));
		xtermParseRect(xw, ParamPair(0), &myRect);
		ScrnFillRectangle(xw, &myRect, ' ', nrc_ASCII, xw->flags, True);
	    }
	    ResetState(sp);
	    break;

	case CASE_DECFRA:
	    if (screen->vtXX_level >= 4) {
		value = use_default_value(0, ' ');

		TRACE(("CASE_DECFRA - Fill rectangular area\n"));
		/* DEC 070, page 5-170 says the fill-character is either
		 * ASCII or Latin1; xterm allows printable Unicode values.
		 */
		if (nparam > 0
		    && ((value >= 256 && CharWidth(screen, value) > 0)
			|| IsLatin1(value))) {
		    xtermParseRect(xw, ParamPair(1), &myRect);
		    ScrnFillRectangle(xw, &myRect,
				      value, current_charset(screen, value),
				      xw->flags, True);
		}
	    }
	    ResetState(sp);
	    break;

	case CASE_DECSERA:
	    if (screen->vtXX_level >= 4) {
		TRACE(("CASE_DECSERA - Selective erase rectangular area\n"));
		xtermParseRect(xw, ParamPair(0), &myRect);
		ScrnWipeRectangle(xw, &myRect);
	    }
	    ResetState(sp);
	    break;

	case CASE_DECSACE:
	    TRACE(("CASE_DECSACE - Select attribute change extent\n"));
	    screen->cur_decsace = zero_if_default(0);
	    ResetState(sp);
	    break;

	case CASE_DECCARA:
	    if (screen->vtXX_level >= 4) {
		TRACE(("CASE_DECCARA - Change attributes in rectangular area\n"));
		xtermParseRect(xw, ParamPair(0), &myRect);
		ScrnMarkRectangle(xw, &myRect, False, ParamPair(4));
	    }
	    ResetState(sp);
	    break;

	case CASE_DECRARA:
	    if (screen->vtXX_level >= 4) {
		TRACE(("CASE_DECRARA - Reverse attributes in rectangular area\n"));
		xtermParseRect(xw, ParamPair(0), &myRect);
		ScrnMarkRectangle(xw, &myRect, True, ParamPair(4));
	    }
	    ResetState(sp);
	    break;

	case CASE_DECSCPP:
	    if (screen->vtXX_level >= 3) {
		TRACE(("CASE_DECSCPP\n"));
		/* default and 0 are "80", with "132" as the other legal choice */
		switch (zero_if_default(0)) {
		case 0:
		case 80:
		    value = 80;
		    break;
		case 132:
		    value = 132;
		    break;
		default:
		    value = -1;
		    break;
		}
		if (value > 0) {
		    if (screen->cur_col + 1 > value)
			CursorSet(screen, screen->cur_row, value - 1, xw->flags);
		    UIntClr(xw->flags, IN132COLUMNS);
		    if (value == 132)
			UIntSet(xw->flags, IN132COLUMNS);
		    RequestResize(xw, -1, value, True);
		}
	    }
	    ResetState(sp);
	    break;

	case CASE_DECSNLS:
	    if (screen->vtXX_level >= 4 && AllowWindowOps(xw, ewSetWinLines)) {
		TRACE(("CASE_DECSNLS\n"));
		value = zero_if_default(0);
		if (value >= 1 && value <= 255) {
		    RequestResize(xw, value, -1, True);
		}
	    }
	    ResetState(sp);
	    break;

	case CASE_DECRQDE:
	    if (screen->vtXX_level >= 3) {
		init_reply(ANSI_CSI);
		count = 0;
		reply.a_param[count++] = (ParmType) MaxRows(screen);	/* number of lines */
		reply.a_param[count++] = (ParmType) MaxCols(screen);	/* number of columns */
		reply.a_param[count++] = 1;	/* current page column */
		reply.a_param[count++] = 1;	/* current page line */
		reply.a_param[count++] = 1;	/* current page */
		reply.a_inters = '"';
		reply.a_final = 'w';
		reply.a_nparam = (ParmType) count;
		unparseseq(xw, &reply);
	    }
	    ResetState(sp);
	    break;

	case CASE_DECRQPSR:
#define reply_char(n,c) do { reply.a_radix[(n)] = 1; reply.a_param[(n)++] = (ParmType)(c); } while (0)
#define reply_bit(n,c) ((n) ? (c) : 0)
	    if (screen->vtXX_level >= 3) {
		TRACE(("CASE_DECRQPSR\n"));
		switch (GetParam(0)) {
		case 1:
		    TRACE(("...DECCIR\n"));
		    init_reply(ANSI_DCS);
		    count = 0;
		    reply_char(count, '1');
		    reply_char(count, '$');
		    reply_char(count, 'u');
		    reply.a_param[count++] = (ParmType) (screen->cur_row + 1);
		    reply.a_param[count++] = (ParmType) (screen->cur_col + 1);
		    reply.a_param[count++] = (ParmType) thispage;
		    reply_char(count, ';');
		    reply_char(count, (0x40
				       | reply_bit(xw->flags & INVERSE, 8)
				       | reply_bit(xw->flags & BLINK, 4)
				       | reply_bit(xw->flags & UNDERLINE, 2)
				       | reply_bit(xw->flags & BOLD, 1)
			       ));
		    reply_char(count, ';');
		    reply_char(count, 0x40 |
			       reply_bit(screen->protected_mode &
					 DEC_PROTECT, 1)
			);
		    reply_char(count, ';');
		    reply_char(count, (0x40
				       | reply_bit(screen->do_wrap, 8)
				       | reply_bit((screen->curss == 3), 4)
				       | reply_bit((screen->curss == 2), 2)
				       | reply_bit(xw->flags & ORIGIN, 1)
			       ));
		    reply_char(count, ';');
		    reply.a_param[count++] = screen->curgl;
		    reply.a_param[count++] = screen->curgr;
		    reply_char(count, ';');
		    value = 0x40;
		    for (item = 0; item < NUM_GSETS; ++item) {
			value |= (is_96charset(screen->gsets[item]) << item);
		    }
		    reply_char(count, value);	/* encoded charset sizes */
		    reply_char(count, ';');
		    for (item = 0; item < NUM_GSETS; ++item) {
			int ps;
			char *temp = encode_scs(screen->gsets[item], &ps);
			while (*temp != '\0') {
			    reply_char(count, *temp++);
			}
		    }
		    reply.a_nparam = (ParmType) count;
		    unparseseq(xw, &reply);
		    break;
		case 2:
		    TRACE(("...DECTABSR\n"));
		    init_reply(ANSI_DCS);
		    reply.a_delim = "/";
		    count = 0;
		    reply_char(count, '2');
		    reply_char(count, '$');
		    reply_char(count, 'u');
		    for (item = 0; item < MAX_TABS; ++item) {
			if (count + 1 >= NPARAM)
			    break;
			if (item > screen->max_col)
			    break;
			if (TabIsSet(xw->tabs, item))
			    reply.a_param[count++] = (ParmType) (item + 1);
		    }
		    reply.a_nparam = (ParmType) count;
		    unparseseq(xw, &reply);
		    break;
		}
	    }
	    ResetState(sp);
	    break;

	case CASE_DECRQUPSS:
	    TRACE(("CASE_DECRQUPSS\n"));
	    if (screen->vtXX_level >= 3) {
		int psize = 0;
		char *encoded = encode_scs(screen->gsets_upss, &psize);
		init_reply(ANSI_DCS);
		count = 0;
		reply_char(count, psize ? '1' : '0');
		reply_char(count, '!');
		reply_char(count, 'u');
		reply_char(count, *encoded++);
		if (*encoded)
		    reply_char(count, *encoded);
		reply.a_nparam = (ParmType) count;
		unparseseq(xw, &reply);
	    }
	    break;

	case CASE_RQM:
	    TRACE(("CASE_RQM\n"));
	    do_ansi_rqm(xw, ParamPair(0));
	    ResetState(sp);
	    break;

	case CASE_DECRQM:
	    TRACE(("CASE_DECRQM\n"));
	    do_dec_rqm(xw, ParamPair(0));
	    ResetState(sp);
	    break;

	case CASE_CSI_DEC_DOLLAR_STATE:
	    TRACE(("CASE_CSI_DEC_DOLLAR_STATE\n"));
	    /* csi ? dollar ($) */
	    sp->parsestate = csi_dec_dollar_table;
	    break;

	case CASE_DECST8C:
	    TRACE(("CASE_DECST8C\n"));
	    if (screen->vtXX_level >= 5
		&& (GetParam(0) == 5 || GetParam(0) <= 0)) {
		TabZonk(xw->tabs);
		for (count = 0; count < MAX_TABS; ++count) {
		    item = (count + 1) * 8;
		    if (item > screen->max_col)
			break;
		    TabSet(xw->tabs, item);
		}
	    }
	    ResetState(sp);
	    break;
#else
	case CASE_CSI_DOLLAR_STATE:
	    /* csi dollar ($) */
	    sp->parsestate = eigtable;
	    break;

	case CASE_CSI_STAR_STATE:
	    /* csi dollar (*) */
	    sp->parsestate = eigtable;
	    break;

	case CASE_CSI_DEC_DOLLAR_STATE:
	    /* csi ? dollar ($) */
	    sp->parsestate = eigtable;
	    break;

	case CASE_DECST8C:
	    /* csi ? 5 W */
	    ResetState(sp);
	    break;
#endif /* OPT_DEC_RECTOPS */

#if OPT_VT525_COLORS
	case CASE_CSI_COMMA_STATE:
	    TRACE(("CASE_CSI_COMMA_STATE\n"));
	    /* csi comma (,) */
	    if (screen->vtXX_level >= 5)
		sp->parsestate = csi_comma_table;
	    else
		sp->parsestate = eigtable;
	    break;

	case CASE_DECAC:
	    TRACE(("CASE_DECAC\n"));
#if OPT_ISO_COLORS
	    if (screen->terminal_id >= 525) {
		int fg, bg;

		switch (GetParam(0)) {
		case 1:
		    fg = GetParam(1);
		    bg = GetParam(2);
		    if (fg >= 0 && fg < 16 && bg >= 0 && bg < 16) {
			Boolean repaint = False;

			if (AssignFgColor(xw,
					  GET_COLOR_RES(xw, screen->Acolors[fg])))
			    repaint = True;
			if (AssignBgColor(xw,
					  GET_COLOR_RES(xw, screen->Acolors[bg])))
			    repaint = True;
			if (repaint)
			    xtermRepaint(xw);
			screen->assigned_fg = fg;
			screen->assigned_bg = bg;
		    }
		    break;
		case 2:
		    /* window frames: not implemented */
		    break;
		}
	    }
#endif
	    ResetState(sp);
	    break;

	case CASE_DECATC:
#if OPT_ISO_COLORS
	    TRACE(("CASE_DECATC\n"));
	    if (screen->terminal_id >= 525) {
		int ps = GetParam(0);
		int fg = GetParam(1);
		int bg = GetParam(2);
		if (ps >= 0 && ps < 16
		    && fg >= 0 && fg < 16
		    && bg >= 0 && bg < 16) {
		    screen->alt_colors[ps].fg = fg;
		    screen->alt_colors[ps].bg = bg;
		}
	    }
#endif
	    ResetState(sp);
	    break;

	case CASE_DECTID:
	    TRACE(("CASE_DECTID\n"));
	    switch (GetParam(0)) {
	    case 0:
		screen->display_da1 = 100;
		break;
	    case 1:
		screen->display_da1 = 101;
		break;
	    case 2:
		screen->display_da1 = 102;
		break;
	    case 5:
		screen->display_da1 = 220;
		break;
	    case 7:
		screen->display_da1 = 320;
		break;
	    case 9:
		screen->display_da1 = 420;
		break;
	    case 10:
		screen->display_da1 = 520;
		break;
	    }
	    ResetState(sp);
	    break;
#else
	case CASE_CSI_COMMA_STATE:
	    sp->parsestate = eigtable;
	    break;
#endif

	case CASE_DECSASD:
#if OPT_STATUS_LINE
	    if (screen->vtXX_level >= 2) {
		handle_DECSASD(xw, zero_if_default(0));
	    }
#endif
	    ResetState(sp);
	    break;

	case CASE_DECSSDT:
#if OPT_STATUS_LINE
	    if (screen->vtXX_level >= 2) {
		handle_DECSSDT(xw, zero_if_default(0));
	    }
#endif
	    ResetState(sp);
	    break;

#if OPT_XTERM_SGR		/* most are related, all use csi_hash_table[] */
	case CASE_CSI_HASH_STATE:
	    TRACE(("CASE_CSI_HASH_STATE\n"));
	    /* csi hash (#) */
	    sp->parsestate = csi_hash_table;
	    break;

	case CASE_XTERM_CHECKSUM:
#if OPT_DEC_RECTOPS
	    if (screen->vtXX_level >= 4 && AllowWindowOps(xw, ewSetChecksum)) {
		TRACE(("CASE_XTERM_CHECKSUM\n"));
		screen->checksum_ext = zero_if_default(0);
	    }
#endif
	    ResetState(sp);
	    break;

	case CASE_XTERM_PUSH_SGR:
	    TRACE(("CASE_XTERM_PUSH_SGR\n"));
	    value = 0;
	    if (nparam == 0 || (nparam == 1 && GetParam(0) == DEFAULT)) {
		value = DEFAULT;
	    } else if (nparam > 0) {
		for (count = 0; count < nparam; ++count) {
		    item = zero_if_default(count);
		    /* deprecated - for compatibility */
#if OPT_ISO_COLORS
		    if (item == psFG_COLOR_obs) {
			item = psFG_COLOR;
		    } else if (item == psBG_COLOR_obs) {
			item = psBG_COLOR;
		    }
#endif
		    if (item > 0 && item < MAX_PUSH_SGR) {
			value |= (1 << (item - 1));
		    }
		}
	    }
	    xtermPushSGR(xw, value);
	    ResetState(sp);
	    break;

	case CASE_XTERM_REPORT_SGR:
	    TRACE(("CASE_XTERM_REPORT_SGR\n"));
	    xtermParseRect(xw, ParamPair(0), &myRect);
	    xtermReportSGR(xw, &myRect);
	    ResetState(sp);
	    break;

	case CASE_XTERM_POP_SGR:
	    TRACE(("CASE_XTERM_POP_SGR\n"));
	    xtermPopSGR(xw);
	    ResetState(sp);
	    break;

	case CASE_XTERM_PUSH_COLORS:
	    TRACE(("CASE_XTERM_PUSH_COLORS\n"));
	    if (nparam == 0) {
		xtermPushColors(xw, DEFAULT);
	    } else {
		for (count = 0; count < nparam; ++count) {
		    xtermPushColors(xw, GetParam(count));
		}
	    }
	    ResetState(sp);
	    break;

	case CASE_XTERM_POP_COLORS:
	    TRACE(("CASE_XTERM_POP_COLORS\n"));
	    if (nparam == 0) {
		xtermPopColors(xw, DEFAULT);
	    } else {
		for (count = 0; count < nparam; ++count) {
		    xtermPopColors(xw, GetParam(count));
		}
	    }
	    ResetState(sp);
	    break;

	case CASE_XTERM_REPORT_COLORS:
	    TRACE(("CASE_XTERM_REPORT_COLORS\n"));
	    xtermReportColors(xw);
	    ResetState(sp);
	    break;

	case CASE_XTERM_TITLE_STACK:
	    xtermReportTitleStack(xw);
	    ResetState(sp);
	    break;
#endif

	case CASE_S7C1T:
	    TRACE(("CASE_S7C1T\n"));
	    if (screen->vtXX_level >= 2) {
		show_8bit_control(False);
		ResetState(sp);
	    }
	    break;

	case CASE_S8C1T:
	    TRACE(("CASE_S8C1T\n"));
	    if (screen->vtXX_level >= 2) {
		show_8bit_control(True);
		ResetState(sp);
	    }
	    break;

	case CASE_OSC:
	    TRACE(("CASE_OSC: Operating System Command\n"));
	    BeginString(ANSI_OSC);
	    break;

	case CASE_RIS:
	    TRACE(("CASE_RIS\n"));
	    VTReset(xw, True, True);
	    /* NOTREACHED */

	case CASE_DECSTR:
	    TRACE(("CASE_DECSTR\n"));
	    VTReset(xw, False, False);
	    /* NOTREACHED */

	case CASE_REP:
	    TRACE(("CASE_REP\n"));
	    if (CharWidth(screen, sp->lastchar) > 0) {
		IChar repeated[2];
		count = one_if_default(0);
		repeated[0] = (IChar) sp->lastchar;
		while (count-- > 0) {
		    dotext(xw,
			   screen->gsets[(int) (screen->curgl)],
			   repeated, 1);
		}
	    }
	    ResetState(sp);
	    break;

	case CASE_LS2:
	    TRACE(("CASE_LS2\n"));
	    if (screen->ansi_level > 2)
		screen->curgl = 2;
	    ResetState(sp);
	    break;

	case CASE_LS3:
	    TRACE(("CASE_LS3\n"));
	    if (screen->ansi_level > 2)
		screen->curgl = 3;
	    ResetState(sp);
	    break;

	case CASE_LS3R:
	    TRACE(("CASE_LS3R\n"));
	    if (screen->ansi_level > 2)
		screen->curgr = 3;
	    ResetState(sp);
	    break;

	case CASE_LS2R:
	    TRACE(("CASE_LS2R\n"));
	    if (screen->ansi_level > 2)
		screen->curgr = 2;
	    ResetState(sp);
	    break;

	case CASE_LS1R:
	    TRACE(("CASE_LS1R\n"));
	    if (screen->ansi_level > 2)
		screen->curgr = 1;
	    ResetState(sp);
	    break;

	case CASE_XTERM_SAVE:
	    savemodes(xw);
	    ResetState(sp);
	    break;

	case CASE_XTERM_RESTORE:
	    restoremodes(xw);
	    ResetState(sp);
	    break;

	case CASE_XTERM_WINOPS:
	    TRACE(("CASE_XTERM_WINOPS\n"));
	    window_ops(xw);
	    ResetState(sp);
	    break;
#if OPT_WIDE_CHARS
	case CASE_ESC_PERCENT:
	    TRACE(("CASE_ESC_PERCENT\n"));
	    sp->parsestate = esc_pct_table;
	    break;

	case CASE_UTF8:
	    /* If we did not set UTF-8 mode from resource or the
	     * command-line, allow it to be enabled/disabled by
	     * control sequence.
	     */
	    TRACE(("CASE_UTF8 wide:%d, utf8:%d, req:%s\n",
		   screen->wide_chars,
		   screen->utf8_mode,
		   BtoS(AsciiOf(c) == 'G')));
	    if ((!screen->wide_chars) && (AsciiOf(c) == 'G')) {
		WriteNow();
		ChangeToWide(xw);
	    }
	    if (screen->wide_chars
		&& !screen->utf8_always) {
		switchPtyData(screen, AsciiOf(c) == 'G');
		TRACE(("UTF8 mode %s\n",
		       BtoS(screen->utf8_mode)));
	    } else {
		TRACE(("UTF8 mode NOT turned %s (%s)\n",
		       BtoS(AsciiOf(c) == 'G'),
		       (screen->utf8_mode == uAlways)
		       ? "UTF-8 mode set from command-line"
		       : "wideChars resource was not set"));
	    }
	    ResetState(sp);
	    break;

	case CASE_SCS_DQUOTE:
	    TRACE(("CASE_SCS_DQUOTE\n"));
	    sp->parsestate = scs_2qt_table;
	    break;

	case CASE_GSETS_DQUOTE:
	    if (screen->vtXX_level >= 5) {
		TRACE_GSETS("_DQUOTE");
		xtermDecodeSCS(xw, sp->scstype, 5, '"', (int) c);
	    }
	    ResetState(sp);
	    break;

	case CASE_SCS_AMPRSND:
	    TRACE(("CASE_SCS_AMPRSND\n"));
	    sp->parsestate = scs_amp_table;
	    break;

	case CASE_GSETS_AMPRSND:
	    if (screen->vtXX_level >= 5) {
		TRACE_GSETS("_AMPRSND");
		xtermDecodeSCS(xw, sp->scstype, 5, '&', (int) c);
	    }
	    ResetState(sp);
	    break;

	case CASE_SCS_PERCENT:
	    TRACE(("CASE_SCS_PERCENT\n"));
	    sp->parsestate = scs_pct_table;
	    break;

	case CASE_GSETS_PERCENT:
	    if (screen->vtXX_level >= 3) {
		TRACE_GSETS("_PERCENT");
		switch (AsciiOf(c)) {
		case '0':	/* DEC Turkish */
		case '2':	/* Turkish */
		case '=':	/* Hebrew */
		    value = 5;
		    break;
		case '5':	/* DEC Supplemental Graphics */
		case '6':	/* Portuguese */
		default:
		    value = 3;
		    break;
		}
		xtermDecodeSCS(xw, sp->scstype, value, '%', (int) c);
	    }
	    ResetState(sp);
	    break;
#endif
	case CASE_XTERM_SHIFT_ESCAPE:
	    TRACE(("CASE_XTERM_SHIFT_ESCAPE\n"));
	    value = ((nparam == 0)
		     ? 0
		     : one_if_default(0));
	    if (value >= 0 && value <= 1)
		xw->keyboard.shift_escape = value;
	    ResetState(sp);
	    break;

#if OPT_MOD_FKEYS
	case CASE_SET_FMT_KEYS:
	    TRACE(("CASE_SET_FMT_KEYS\n"));
	    if (nparam >= 1) {
		int first = GetParam(0);
		int second = ((nparam > 1)
			      ? GetParam(1)
			      : DEFAULT);
		set_fmt_fkeys(xw, first, second, True);
	    } else {
		for (value = 1; value <= 5; ++value)
		    set_fmt_fkeys(xw, value, DEFAULT, True);
	    }
	    ResetState(sp);
	    break;

	case CASE_XTERM_REPORT_FMT_KEYS:
	    TRACE(("CASE_XTERM_REPORT_FMT_KEYS\n"));
	    for (value = 0; value < nparam; ++value) {
		report_fmt_fkeys(xw, GetParam(value));
	    }
	    ResetState(sp);
	    break;

	case CASE_SET_MOD_FKEYS:
	    TRACE(("CASE_SET_MOD_FKEYS\n"));
	    if (nparam >= 1) {
		int first = GetParam(0);
		int second = ((nparam > 1)
			      ? GetParam(1)
			      : DEFAULT);
		if (parms.has_subparams) {
		    if (subparam_index(0, 0) == 0 &&
			subparam_index(1, 0) == 1 &&
			subparam_index(1, 1) == 2) {
			set_mod_fkeys(xw, first, second, True, parms.params[2]);
		    }
		} else {
		    set_mod_fkeys(xw, first, second, True, 0);
		}
	    } else {
		for (value = 1; value <= 5; ++value)
		    set_mod_fkeys(xw, value, DEFAULT, True, 0);
	    }
	    ResetState(sp);
	    break;

	case CASE_SET_MOD_FKEYS0:
	    TRACE(("CASE_SET_MOD_FKEYS0\n"));
	    if (nparam >= 1 && GetParam(0) != DEFAULT) {
		set_mod_fkeys(xw, GetParam(0), DEFAULT, False, 0);
	    } else {
		xw->keyboard.modify_now.function_keys = DEFAULT;
	    }
	    ResetState(sp);
	    break;

	case CASE_XTERM_REPORT_MOD_FKEYS:
	    TRACE(("CASE_XTERM_REPORT_MOD_FKEYS\n"));
	    for (value = 0; value < nparam; ++value) {
		report_mod_fkeys(xw, GetParam(value));
	    }
	    ResetState(sp);
	    break;
#endif
	case CASE_HIDE_POINTER:
	    TRACE(("CASE_HIDE_POINTER\n"));
	    if (nparam >= 1 && GetParam(0) != DEFAULT) {
		screen->pointer_mode = GetParam(0);
	    } else {
		screen->pointer_mode = DEF_POINTER_MODE;
	    }
	    ResetState(sp);
	    break;

	case CASE_XTERM_SM_TITLE:
	    TRACE(("CASE_XTERM_SM_TITLE\n"));
	    if (nparam >= 1) {
		for (count = 0; count < nparam; ++count) {
		    value = GetParam(count);
		    if (ValidTitleMode(value))
			screen->title_modes |= xBIT(value);
		}
	    } else {
		screen->title_modes = DEF_TITLE_MODES;
	    }
	    TRACE(("...title_modes %#x\n", screen->title_modes));
	    ResetState(sp);
	    break;

	case CASE_XTERM_RM_TITLE:
	    TRACE(("CASE_XTERM_RM_TITLE\n"));
	    if (nparam >= 1) {
		for (count = 0; count < nparam; ++count) {
		    value = GetParam(count);
		    if (ValidTitleMode(value))
			screen->title_modes &= ~xBIT(value);
		}
	    } else {
		screen->title_modes = DEF_TITLE_MODES;
	    }
	    TRACE(("...title_modes %#x\n", screen->title_modes));
	    ResetState(sp);
	    break;

	case CASE_CSI_IGNORE:
	    sp->parsestate = cigtable;
	    break;

	case CASE_DECSWBV:
	    TRACE(("CASE_DECSWBV\n"));
	    switch (zero_if_default(0)) {
	    case 2:
		/* FALLTHRU */
	    case 3:
		/* FALLTHRU */
	    case 4:
		screen->warningVolume = bvLow;
		break;
	    case 5:
		/* FALLTHRU */
	    case 6:
		/* FALLTHRU */
	    case 7:
		/* FALLTHRU */
	    case 8:
		screen->warningVolume = bvHigh;
		break;
	    default:
		screen->warningVolume = bvOff;
		break;
	    }
	    TRACE(("...warningVolume %d\n", screen->warningVolume));
	    ResetState(sp);
	    break;

	case CASE_DECSMBV:
	    TRACE(("CASE_DECSMBV\n"));
	    switch (zero_if_default(0)) {
	    case 2:
		/* FALLTHRU */
	    case 3:
		/* FALLTHRU */
	    case 4:
		screen->marginVolume = bvLow;
		break;
	    case 0:
		/* FALLTHRU */
	    case 5:
		/* FALLTHRU */
	    case 6:
		/* FALLTHRU */
	    case 7:
		/* FALLTHRU */
	    case 8:
		screen->marginVolume = bvHigh;
		break;
	    default:
		screen->marginVolume = bvOff;
		break;
	    }
	    TRACE(("...marginVolume %d\n", screen->marginVolume));
	    ResetState(sp);
	    break;
	}
	if (sp->parsestate == sp->groundtable)
	    sp->lastchar = thischar;
    } while (0);

#if OPT_WIDE_CHARS
    screen->utf8_inparse = (Boolean) ((screen->utf8_mode != uFalse)
				      && (sp->parsestate != sos_table));
#endif

    if (sp->check_recur)
	sp->check_recur--;
    return True;
}

void
unparseputc(XtermWidget xw, int c)
{
    TScreen *screen = TScreenOf(xw);
    IChar *buf = screen->unparse_bfr;
    unsigned len;

    if ((screen->unparse_len + 2) >= screen->unparse_max)
	unparse_end(xw);

    len = screen->unparse_len;

#if OPT_TCAP_QUERY
    /*
     * If we're returning a termcap string, it has to be translated since
     * a DCS must not contain any characters except for the normal 7-bit
     * printable ASCII (counting tab, carriage return, etc).  For now,
     * just use hexadecimal for the whole thing.
     */
    if (screen->tc_query_code >= 0) {
	char tmp[3];
	sprintf(tmp, "%02X", (unsigned) (c & 0xFF));
	buf[len++] = CharOf(tmp[0]);
	buf[len++] = CharOf(tmp[1]);
    } else
#endif
    if ((buf[len++] = (IChar) c) == '\r' && (xw->flags & LINEFEED)) {
	buf[len++] = '\n';
    }

    screen->unparse_len = len;

    /* If send/receive mode is reset, we echo characters locally */
    if ((xw->keyboard.flags & MODE_SRM) == 0) {
	doparsing(xw, (unsigned) c, &myState);
    }
}

static void
VTparse(XtermWidget xw)
{
    Boolean keep_running;

    /* We longjmp back to this point in VTReset() */
    (void) setjmp(vtjmpbuf);
    init_parser(xw, &myState);

    do {
	keep_running = doparsing(xw, doinput(xw), &myState);
	if (myState.check_recur == 0 && myState.defer_used != 0) {
	    while (myState.defer_used) {
		Char *deferred = myState.defer_area;
		size_t len = myState.defer_used;
		size_t i;
		myState.defer_area = NULL;
		myState.defer_size = 0;
		myState.defer_used = 0;
		for (i = 0; i < len; i++) {
		    (void) doparsing(xw, deferred[i], &myState);
		}
		free(deferred);
	    }
	} else {
	    free(myState.defer_area);
	}
	myState.defer_area = NULL;
	myState.defer_size = 0;
	myState.defer_used = 0;
    } while (keep_running);
}

void
VTRun(XtermWidget xw)
{
    TScreen *screen = TScreenOf(xw);

    TRACE(("VTRun ...\n"));

    if (!screen->Vshow && !resource.notMapped) {
	set_vt_visibility(True);
    }
    update_vttekmode();
    update_vtshow();
    update_tekshow();
    set_vthide_sensitivity();

    ScrnAllocBuf(xw);

    screen->cursor_state = OFF;
    screen->cursor_set = ON;
#if OPT_BLINK_CURS
    if (DoStartBlinking(screen))
	StartBlinking(xw);
#endif

#if OPT_TEK4014
    if (Tpushb > Tpushback) {
	fillPtyData(xw, VTbuffer, (char *) Tpushback, (size_t) (Tpushb - Tpushback));
	Tpushb = Tpushback;
    }
#endif
    screen->is_running = True;
    if (screen->embed_high && screen->embed_wide) {
	ScreenResize(xw, screen->embed_wide, screen->embed_high, &(xw->flags));
    }
#if OPT_MAXIMIZE
    else if (resource.fullscreen == esTrue || resource.fullscreen == esAlways)
	FullScreen(xw, True);
#endif
    if (!setjmp(VTend))
	VTparse(xw);
    StopBlinking(xw);
    HideCursor(xw);
    screen->cursor_set = OFF;
    TRACE(("... VTRun\n"));
}

void
VTReset(XtermWidget xw, Bool full, Bool saved)
{
    ReallyReset(xw, full, saved);

    FreeAndNull(myState.string_area);
    FreeAndNull(myState.print_area);

    longjmp(vtjmpbuf, 1);	/* force ground state in parser */
}

static void
VTDestroy(Widget w GCC_UNUSED)
{
#ifdef NO_LEAKS
    XtermWidget xw = (XtermWidget) w;
    TScreen *screen = TScreenOf(xw);
    Display *dpy = screen->display;
    Cardinal n, k;

    StopBlinking(xw);

    if (screen->scrollWidget) {
	XtUninstallTranslations(screen->scrollWidget);
	XtDestroyWidget(screen->scrollWidget);
    }

    while (screen->saved_fifo > 0) {
	deleteScrollback(screen);
    }

    for (n = 0; n < MAX_SAVED_TITLES; ++n)
	xtermFreeTitle(&screen->saved_titles.data[n]);

#if OPT_STATUS_LINE
    free(screen->status_fmt);
#endif
#ifndef NO_ACTIVE_ICON
    TRACE_FREE_LEAK(xw->misc.active_icon_s);
#endif
#if OPT_ISO_COLORS
    TRACE_FREE_LEAK(screen->cmap_data);
    for (n = 0; n < MAXCOLORS; n++) {
	TRACE_FREE_LEAK(screen->Acolors[n].resource);
    }
    for (n = 0; n < MAX_SAVED_SGR; n++) {
	TRACE_FREE_LEAK(xw->saved_colors.palettes[n]);
    }
#endif
    for (n = 0; n < NCOLORS; n++) {
	switch (n) {
#if OPT_TEK4014
	case TEK_BG:
	    /* FALLTHRU */
	case TEK_FG:
	    /* FALLTHRU */
	case TEK_CURSOR:
	    break;
#endif
	default:
	    TRACE_FREE_LEAK(screen->Tcolors[n].resource);
	    break;
	}
    }
    FreeMarkGCs(xw);
    TRACE_FREE_LEAK(screen->unparse_bfr);
    TRACE_FREE_LEAK(screen->save_ptr);
    TRACE_FREE_LEAK(screen->saveBuf_data);
    TRACE_FREE_LEAK(screen->saveBuf_index);
    for (n = 0; n < 2; ++n) {
	TRACE_FREE_LEAK(screen->editBuf_data[n]);
	TRACE_FREE_LEAK(screen->editBuf_index[n]);
    }
    TRACE_FREE_LEAK(screen->keyboard_dialect);
    TRACE_FREE_LEAK(screen->cursor_font_name);
    TRACE_FREE_LEAK(screen->pointer_shape);
    TRACE_FREE_LEAK(screen->term_id);
#if OPT_WIDE_CHARS
    TRACE_FREE_LEAK(screen->utf8_mode_s);
    TRACE_FREE_LEAK(screen->utf8_fonts_s);
    TRACE_FREE_LEAK(screen->utf8_title_s);
#if OPT_LUIT_PROG
    TRACE_FREE_LEAK(xw->misc.locale_str);
    TRACE_FREE_LEAK(xw->misc.localefilter);
#endif
#endif
    TRACE_FREE_LEAK(xw->misc.T_geometry);
    TRACE_FREE_LEAK(xw->misc.geo_metry);
#if OPT_INPUT_METHOD
    cleanupInputMethod(xw);
    TRACE_FREE_LEAK(xw->misc.f_x);
    TRACE_FREE_LEAK(xw->misc.input_method);
    TRACE_FREE_LEAK(xw->misc.preedit_type);
#endif
    releaseCursorGCs(xw);
    releaseWindowGCs(xw, &(screen->fullVwin));
#ifndef NO_ACTIVE_ICON
    XFreeFont(screen->display, getIconicFont(screen)->fs);
    releaseWindowGCs(xw, &(screen->iconVwin));
#endif
    XtUninstallTranslations((Widget) xw);
#if OPT_TOOLBAR
    XtUninstallTranslations((Widget) XtParent(xw));
#endif
    XtUninstallTranslations((Widget) SHELL_OF(xw));

    if (screen->hidden_cursor)
	XFreeCursor(screen->display, screen->hidden_cursor);

    xtermCloseFonts(xw, screen->fnts);
#if OPT_WIDE_ATTRS
    xtermCloseFonts(xw, screen->ifnts);
#endif
    noleaks_cachedCgs(xw);
    free_termcap(xw);

    FREE_VT_WIN(fullVwin);
#ifndef NO_ACTIVE_ICON
    FREE_VT_WIN(iconVwin);
#endif /* NO_ACTIVE_ICON */

    TRACE_FREE_LEAK(screen->selection_targets_8bit);
#if OPT_SELECT_REGEX
    for (n = 0; n < NSELECTUNITS; ++n) {
	if (screen->selectMap[n] == Select_REGEX) {
	    TRACE_FREE_LEAK(screen->selectExpr[n]);
	}
    }
#endif

#if OPT_RENDERFONT
    for (n = 0; n < NMENUFONTS; ++n) {
	int e;
	for (e = 0; e < fMAX; ++e) {
	    xtermCloseXft(screen, getMyXftFont(xw, e, (int) n));
	}
    }
    discardRenderDraw(screen);
    {
	ListXftFonts *p;
	while ((p = screen->list_xft_fonts) != NULL) {
	    screen->list_xft_fonts = p->next;
	    free(p);
	}
    }
#endif

    /* free things allocated via init_Sres or Init_Sres2 */
#ifndef NO_ACTIVE_ICON
    TRACE_FREE_LEAK(screen->icon_fontname);
#endif
#ifdef ALLOWLOGGING
    TRACE_FREE_LEAK(screen->logfile);
#endif
    TRACE_FREE_LEAK(screen->eight_bit_meta_s);
    TRACE_FREE_LEAK(screen->charClass);
    TRACE_FREE_LEAK(screen->answer_back);
    TRACE_FREE_LEAK(screen->printer_state.printer_command);
    TRACE_FREE_LEAK(screen->disallowedColorOps);
    TRACE_FREE_LEAK(screen->disallowedFontOps);
    TRACE_FREE_LEAK(screen->disallowedMouseOps);
    TRACE_FREE_LEAK(screen->disallowedPasteOps);
    TRACE_FREE_LEAK(screen->disallowedTcapOps);
    TRACE_FREE_LEAK(screen->disallowedWinOps);
    TRACE_FREE_LEAK(screen->default_string);
    TRACE_FREE_LEAK(screen->eightbit_select_types);

#if OPT_WIDE_CHARS
    TRACE_FREE_LEAK(screen->utf8_select_types);
#endif

#if 0
    for (n = fontMenu_font1; n <= fontMenu_lastBuiltin; n++) {
	TRACE_FREE_LEAK(screen->MenuFontName(n));
    }
#endif

    TRACE_FREE_LEAK(screen->initial_font);

#if OPT_LUIT_PROG
    TRACE_FREE_LEAK(xw->misc.locale_str);
    TRACE_FREE_LEAK(xw->misc.localefilter);
#endif

    TRACE_FREE_LEAK(xw->misc.cdXtraScroll_s);
    TRACE_FREE_LEAK(xw->misc.tiXtraScroll_s);

#if OPT_RENDERFONT
    TRACE_FREE_LEAK(xw->misc.default_xft.f_n);
#if OPT_WIDE_CHARS
    TRACE_FREE_LEAK(xw->misc.default_xft.f_w);
#endif
    TRACE_FREE_LEAK(xw->misc.render_font_s);
#endif

    TRACE_FREE_LEAK(xw->misc.default_font.f_n);
    TRACE_FREE_LEAK(xw->misc.default_font.f_b);

#if OPT_WIDE_CHARS
    TRACE_FREE_LEAK(xw->misc.default_font.f_w);
    TRACE_FREE_LEAK(xw->misc.default_font.f_wb);
#endif

    TRACE_FREE_LEAK(xw->work.wm_name);
    freeFontLists(&(xw->work.fonts.x11));
#if OPT_RENDERFONT
    freeFontLists(&(xw->work.fonts.xft));
#endif

    xtermFontName(NULL);
#if OPT_LOAD_VTFONTS || OPT_WIDE_CHARS
    TRACE_FREE_LEAK(screen->cacheVTFonts.default_font.f_n);
    TRACE_FREE_LEAK(screen->cacheVTFonts.default_font.f_b);
#if OPT_WIDE_CHARS
    TRACE_FREE_LEAK(screen->cacheVTFonts.default_font.f_w);
    TRACE_FREE_LEAK(screen->cacheVTFonts.default_font.f_wb);
#endif
    freeFontLists(&(screen->cacheVTFonts.fonts.x11));
    for (n = 0; n < NMENUFONTS; ++n) {
	for (k = 0; k < fMAX; ++k) {
	    if (screen->menu_font_names[n][k] !=
		screen->cacheVTFonts.menu_font_names[n][k]) {
		if (screen->menu_font_names[n][k] != _Font_Selected_) {
		    TRACE_FREE_LEAK(screen->menu_font_names[n][k]);
		}
		TRACE_FREE_LEAK(screen->cacheVTFonts.menu_font_names[n][k]);
	    } else {
		TRACE_FREE_LEAK(screen->menu_font_names[n][k]);
	    }
	}
    }
#endif

#if OPT_BLINK_CURS
    TRACE_FREE_LEAK(screen->cursor_blink_s);
#endif

#if OPT_REGIS_GRAPHICS
    TRACE_FREE_LEAK(screen->graphics_regis_default_font);
    TRACE_FREE_LEAK(screen->graphics_regis_screensize);
#endif
#if OPT_GRAPHICS
    TRACE_FREE_LEAK(screen->graph_termid);
    TRACE_FREE_LEAK(screen->graphics_max_size);
#endif

    for (n = 0; n < NSELECTUNITS; ++n) {
#if OPT_SELECT_REGEX
	TRACE_FREE_LEAK(screen->selectExpr[n]);
#endif
#if OPT_XRES_QUERY
	TRACE_FREE_LEAK(screen->onClick[n]);
#endif
    }

    XtFree((void *) (screen->selection_atoms));

    for (n = 0; n < MAX_SELECTIONS; ++n) {
	free(screen->selected_cells[n].data_buffer);
    }

    if (defaultTranslations != xtermClassRec.core_class.tm_table) {
	TRACE_FREE_LEAK(defaultTranslations);
    }
    TRACE_FREE_LEAK(xtermClassRec.core_class.tm_table);
    TRACE_FREE_LEAK(xw->keyboard.shift_escape_s);
    TRACE_FREE_LEAK(xw->keyboard.extra_translations);
    TRACE_FREE_LEAK(xw->keyboard.shell_translations);
    TRACE_FREE_LEAK(xw->keyboard.xterm_translations);
    TRACE_FREE_LEAK(xw->keyboard.print_translations);
    UnmapSelections(xw);

    XtFree((void *) (xw->visInfo));

#if OPT_WIDE_CHARS
    FreeTypedBuffer(IChar);
    FreeTypedBuffer(XChar2b);
    FreeTypedBuffer(Char);
#endif
#if OPT_RENDERFONT
#if OPT_RENDERWIDE
    FreeTypedBuffer(XftCharSpec);
#else
    FreeTypedBuffer(XftChar8);
#endif
#endif

    TRACE_FREE_LEAK(myState.print_area);
    TRACE_FREE_LEAK(myState.string_area);
    memset(&myState, 0, sizeof(myState));

#endif /* defined(NO_LEAKS) */
}

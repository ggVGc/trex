#ifndef BLINKING_H_B8ALIMAN
#define BLINKING_H_B8ALIMAN

void RestartBlinking(XtermWidget xw);

#if OPT_BLINK_CURS || OPT_BLINK_TEXT
#define SettableCursorBlink(screen) \
	(((screen)->cursor_blink != cbAlways) && \
	 ((screen)->cursor_blink != cbNever))
#define UpdateCursorBlink(xw) \
	 SetCursorBlink(xw, TScreenOf(xw)->cursor_blink)
void SetCursorBlink(XtermWidget /* xw */ ,
			   BlinkOps /* enable */ );
void HandleBlinking(XtPointer /* closure */ ,
			   XtIntervalId * /* id */ );
void StartBlinking(XtermWidget /* xw */ );
void StopBlinking(XtermWidget /* xw */ );
#else
#define StartBlinking(xw)	/* nothing */
#define StopBlinking(xw)	/* nothing */
#endif


#endif /* end of include guard: BLINKING_H_B8ALIMAN */

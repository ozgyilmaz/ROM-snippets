(c) 1996 Erwin S. Andreasen <erwin@andreasen.org> - see the License file

Those 3 functions are IMHO very handy: you can avoid having to create a
temporary buffer, sprintf() to it, then finally send_to_char.

Example usage:

Instead:

{
	char buf[MSL];
	sprintf (buf, "Bla bla has %d and %s", num, string);
	send_to_char (buf, ch);
}

just:

printf_to_char (ch, "Bla bla has %d and %s", num, string);

Similarly, with bugf/logf:

logf ("%s got %d gold", ch->name, amount);
bugf ("Undefined %s at %d:%d", some_string, line, col);




In merc.h insert:


void bugf (char * fmt, ...) __attribute__ ((format(printf,1,2)));
void logf (char * fmt, ...) __attribute__ ((format(printf,1,2)));
void    printf_to_char  args( ( CHAR_DATA *ch, char *fmt, ...) ) __attribute__ ((format(printf, 2,3)));


The __attribute__ stuff makes gcc (if you are using -Wall, which you should!)
check that you are passing the right amount of parameters.


Here is the source for the functions. Using vsnprintf() ensure that that the
buffers are not overflowed.

What is MSL? Simply #define MSL MAX_STRING_LENGTH , it is much easier to
type :)


/* source: EOD, by John Booth <???> */
void printf_to_char (CHAR_DATA *ch, char *fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	va_list args;
	va_start (args, fmt);
	vsnprintf (buf, MSL, fmt, args);
	va_end (args);
	
	send_to_char (buf, ch);
}

void bugf (char * fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	va_list args;
	va_start (args, fmt);
	vsnprintf (buf, MSL, fmt, args);
	va_end (args);
	
	bug (buf, 0);
}

void logf (char * fmt, ...)
{
	char buf [2*MSL];
	va_list args;
	va_start (args, fmt);
	vsnprintf (buf, 2*MSL, fmt, args);
	va_end (args);
	
	log_string (buf);
}

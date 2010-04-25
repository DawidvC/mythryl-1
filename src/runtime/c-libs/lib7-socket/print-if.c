/* print-if.c
 *
 * See overview comments in
 *
 *     src/runtime/c-libs/lib7-socket/print-if.h
 *
 * This routine is not really socket-specific, so it
 * probably belongs in some more general directory,
 * but initially at least I'm using it to debug
 * socket stuff, so this location will do for now. -- 2010-02-21 CrT
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "print-if.h"

int print_if_fd = 0;	/* Zero value means no trace logging. (We'd never log to stdin anyhow! :-)	*/

#define MAX_BUF 4096

/* If print_if_fd is nonzero, fprintf given
 * message to it, preceded by a seconds.microseconds timestamp.
 * A typical line looks like
 *
 *    1266769503.421967:  foo.c:  The 23 zots are barred.
 */
void   print_if   (const char * fmt, ...) {

    if (!print_if_fd) {

       return;

    } else {

        int  len;
        int  seconds;
        int  microseconds;

        extern int   _lib7_time_gettimeofday   (int* microseconds);	/* From		src/runtime/c-libs/lib7-time/timeofday.c	*/

	char buf[ MAX_BUF ];

	va_list va;

	/* Start by writing the timestamp into buf[].
	 *
	 * We match the timestamp format in fun print_if in
         * 
         *     src/lib/src/lib/thread-kit/src/lib/tracing.pkg
	 *
	 * Making the gettimeofday() system call here
	 * is a little bit risky in that the system
         * call might change the behavior being debugged,
         * but I think the tracelog timestamps are valuable
         * enough to justify the risk:
         */
	seconds = _lib7_time_gettimeofday (&microseconds);
	sprintf(buf,"%10d.%06d:             ", seconds, microseconds);

	/* Now write the message proper into buf[],
         * right after the timestamp:
	 */
        len = strlen( buf );

	va_start(va, fmt);
	vsnprintf(buf+len, MAX_BUF-len, fmt, va); 
	va_end(va);


	/* Finish up by writing buf[]
         * contents to print_if_fd.
	 *
	 * write() is a low-level unbuffered
	 * system call, so we do not need to
	 * do a flush( print_if_fd ) -- there
	 * is no such call at this level.
	 *
	 * Note that usually we need strlen(buf)+1
	 * when dealing with null-terminated strings
	 * but here we do not want to write the final
	 * null, so strlen(buf) is in fact correct:
	 */
	write( print_if_fd, buf, strlen(buf) );
    }
}

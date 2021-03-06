/* unix-timers.c
 *
 * A UNIX specific interface to the system timers.
 */

#include "../config.h"

#include "runtime-unixdep.h"
#include "runtime-base.h"
#include "runtime-state.h"
#include "runtime-timer.h"

#ifdef HAS_GETRUSAGE

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <sys/resource.h>
typedef struct rusage time_struct_t;
#define GET_TIME(t)		getrusage(RUSAGE_SELF, &(t))
#define SYS_TIME(t)		((t).ru_stime)
#define USR_TIME(t)		((t).ru_utime)
#define SET_TIME(tp, t)		{ *((struct timeval *)(tp)) = (t); }

#else /* !HAS_GETRUSAGE */

#include <sys/times.h>
static long	ClksPerSec = 0;
typedef struct tms time_struct_t;
#define GET_TIME(t)		times(&(t))
#define SYS_TIME(t)		((t).tms_stime)
#define USR_TIME(t)		((t).tms_utime)
#define SET_TIME(tp, t)		{ 					\
	Time_t		*__tp = (tp);					\
	clock_t		__t = (t);					\
	__tp->uSeconds = ((__t % ClksPerSec) * 1000000) / ClksPerSec;	\
	__tp->seconds  = (__t / ClksPerSec);				\
    }

#endif /* HAS_GETRUSAGE */



/* Several versions of Unix seem to allow time values to decrease on successive
 * calls to getrusage.  To avoid problems in the Lib7 code, which assumes that
 * time is monotonically increasing, we latch the time values.
 *
 * NOTE: this should probably be move to the VProc package to avoid problems
 * on MP machines.
 */
static Time_t		lastU, lastS;


/* set_up_timers:
 *
 * Do any system specific timer initialization.
 */
void set_up_timers ()
{
    lastU.seconds = lastU.uSeconds = 0;
    lastS.seconds = lastS.uSeconds = 0;

#ifndef HAS_GETRUSAGE
    if (ClksPerSec == 0)
	ClksPerSec = sysconf(_SC_CLK_TCK);
#endif

} /* end of set_up_timers */


/* get_cpu_time:
 *
 * Get the user and/or system cpu times in a system independent way.
 */
void get_cpu_time (Time_t *usrT, Time_t *sysT)
{
    time_struct_t   ts;

    GET_TIME(ts);

    if (usrT != NULL) {
	SET_TIME(usrT, USR_TIME(ts));
	if (usrT->seconds < lastU.seconds)
	    usrT->seconds = lastU.seconds;
	if ((usrT->seconds == lastU.seconds) && (usrT->uSeconds < lastU.uSeconds))
	    usrT->uSeconds = lastU.uSeconds;
	lastU = *usrT;
    }

    if (sysT != NULL) {
	SET_TIME(sysT, SYS_TIME(ts));
	if (sysT->seconds < lastS.seconds)
	    sysT->seconds = lastS.seconds;
	if ((sysT->seconds == lastS.seconds) && (sysT->uSeconds < lastS.uSeconds))
	    sysT->uSeconds = lastS.uSeconds;
	lastS = *sysT;
    }

} /* end of get_cpu_time. */


/* COPYRIGHT (c) 1995 by AT&T Bell Laboratories.
 * Subsequent changes by Jeff Prothero Copyright (c) 2010,
 * released under Gnu Public Licence version 3.
 */


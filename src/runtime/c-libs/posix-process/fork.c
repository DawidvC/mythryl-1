/* fork.c
 *
 */

#include "../../config.h"

#include "runtime-base.h"
#include "runtime-values.h"
#include "runtime-heap.h"
#include "lib7-c.h"
#include "cfun-proto-list.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

/* _lib7_P_Process_fork : Void -> Int
 *
 * Fork a new process.
 */
lib7_val_t _lib7_P_Process_fork (lib7_state_t *lib7_state, lib7_val_t arg)
{
    int      status;

    status = fork();

    CHECK_RETURN (lib7_state, status)

} /* end of _lib7_P_Process_fork */


/* COPYRIGHT (c) 1995 by AT&T Bell Laboratories.
 * Subsequent changes by Jeff Prothero Copyright (c) 2010,
 * released under Gnu Public Licence version 3.
 */

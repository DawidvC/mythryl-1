/* runtime-fp.h
 *
 * NOTE: changes to this file must be tracked in runtime-fp.c.
 */


#ifdef TARGET_X86

extern void Save_C_FPState();
extern void Restore_C_FPState();

extern void Save_LIB7_FPState();
extern void Restore_LIB7_FPState();

#else

#define Save_C_FPState()
#define Restore_C_FPState()
#define Save_LIB7_FPState()
#define Restore_LIB7_FPState()

#endif

/* end of runtime-fp.h */



/* COPYRIGHT (c) 1995 by AT&T Bell Laboratories.
 * Subsequent changes by Jeff Prothero Copyright (c) 2010,
 * released under Gnu Public Licence version 3.
 */


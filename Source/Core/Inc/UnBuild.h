/*=============================================================================
	UnBuild.h: Unreal build settings.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Major build options.
-----------------------------------------------------------------------------*/

// Whether to perform slow validation checks.
#ifndef CHECK_ALL
#define	CHECK_ALL 0
#endif

// Whether to turn off all checks.
#ifndef CHECK_NONE
#define CHECK_NONE 0
#endif

// Whether to gather performance statistics.
#ifndef STATS
#define STATS 1
#endif

// Whether to check for memory leaks.
#ifndef CHECK_ALLOCS
#define CHECK_ALLOCS 0
#endif

// Whether to perform CPU-intensive timing of critical loops.
#ifndef DO_SLOW_CLOCK
#define DO_SLOW_CLOCK 0
#endif

// Whether to track call-stack errors.
#ifndef DO_GUARD
#define DO_GUARD 1
#endif

// Whether to track call-stack errors in performance critical routines.
#ifndef DO_SLOW_GUARD
#define DO_SLOW_GUARD 0
#endif

// Whether to use Intel assembler code.
#ifndef ASM
#define ASM 1
#endif

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

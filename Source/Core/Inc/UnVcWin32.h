/*=============================================================================
	UnVcWin32.h: Unreal definitions for Visual C++ SP2 running under Win32.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

/*----------------------------------------------------------------------------
	Platform compiler definitions.
----------------------------------------------------------------------------*/

#define __WIN32__	1
#define __INTEL__	1
#include <stdarg.h>
/*----------------------------------------------------------------------------
	Platform specifics types and defines.
----------------------------------------------------------------------------*/

// Undo any Windows defines.
#undef BYTE
#undef CHAR
#undef WORD
#undef DWORD
#undef INT
#undef FLOAT
#undef MAXBYTE
#undef MAXWORD
#undef MAXDWORD
#undef MAXINT
#undef VOID
#undef CDECL

// Make sure HANDLE is defined.
#ifndef _WINDOWS_
	#define HANDLE DWORD
	#define HINSTANCE DWORD
#endif

// Sizes.
enum {DEFAULT_ALIGNMENT = 8 }; // Default boundary to align memory allocations on.
enum {CACHE_LINE_SIZE   = 32}; // Cache line size.

// Optimization macros (preceeded by #pragma).
#define DISABLE_OPTIMIZATION optimize("",off)
#define ENABLE_OPTIMIZATION  optimize("",on)

// Function type macros.
#define DLL_IMPORT	__declspec(dllimport)	/* Import function from DLL */
#define DLL_EXPORT  __declspec(dllexport)	/* Export function to DLL */
#define VARARGS     					/* Functions with variable arguments */
#define CDECL	    __cdecl					/* Standard C function */
#define STDCALL		__stdcall				/* Standard calling convention */

// Variable arguments.
#define GET_VARARGS(msg,fmt) {va_list va_;va_start(va_,fmt); appGetVarArgs(msg,fmt,va_);va_end(va_);}
#define GET_VARARGSR(msg,fmt,result) {va_list va_;va_start(va_,fmt);result =  appGetVarArgs(msg,fmt,va_);va_end(va_);}
// Compiler name.
#ifdef _DEBUG
	#define COMPILER "Compiled with Visual C++ Debug"
#else
	#define COMPILER "Compiled with Visual C++"
#endif

// Unsigned base types.
typedef unsigned char		BYTE;		// 8-bit  unsigned.
typedef unsigned short		_WORD;		// 16-bit unsigned.
typedef unsigned long		DWORD;		// 32-bit unsigned.
typedef unsigned __int64	QWORD;		// 64-bit unsigned.

// Signed base types.
typedef	char				CHAR;		// 8-bit  signed.
typedef signed short		SWORD;		// 16-bit signed.
typedef signed int  		INT;		// 32-bit signed.
typedef signed __int64		SQWORD;		// 64-bit signed.

// Other base types.
using UBOOL =  int			;		// Boolean 0 (false) or 1 (true).
typedef float				FLOAT;		// 32-bit IEEE floating point.
typedef double				DOUBLE;		// 64-bit IEEE double.

// Unwanted VC++ level 4 warnings to disable.
#pragma warning(disable : 4244) /* conversion to float, possible loss of data							*/
#pragma warning(disable : 4699) /* creating precompiled header											*/
#pragma warning(disable : 4200) /* Zero-length array item at end of structure, a VC-specific extension	*/
#pragma warning(disable : 4100) /* unreferenced formal parameter										*/
#pragma warning(disable : 4514) /* unreferenced inline function has been removed						*/
#pragma warning(disable : 4201) /* nonstandard extension used : nameless struct/union					*/
#pragma warning(disable : 4710) /* inline function not expanded											*/
#pragma warning(disable : 4702) /* unreachable code in inline expanded function							*/
#pragma warning(disable : 4711) /* function selected for autmatic inlining								*/
#pragma warning(disable : 4725) /* Pentium fdiv bug														*/
#pragma warning(disable : 4127) /* Conditional expression is constant									*/
#pragma warning(disable : 4512) /* assignment operator could not be generated                           */
#pragma warning(disable : 4530) /* C++ exception handler used, but unwind semantics are not enabled     */
#pragma warning(disable : 4245) /* conversion from 'enum ' to 'unsigned long', signed/unsigned mismatch */
#pragma warning(disable : 4305) /* truncation from 'const double' to 'float'                            */
#pragma warning(disable : 4238) /* nonstandard extension used : class rvalue used as lvalue             */
#pragma warning(disable : 4251) /* needs to have dll-interface to be used by clients of class 'ULinker' */
#pragma warning(disable : 4275) /* non dll-interface class used as base for dll-interface class         */
#pragma warning(disable : 4511) /* copy constructor could not be generated                              */
#pragma warning(disable : 4284) /* return type is not a UDT or reference to a UDT                       */
#pragma warning(disable : 4355) /* this used in base initializer list                                   */
#pragma warning(disable : 4097) /* typedef-name '' used as synonym for class-name ''                    */

// If C++ exception handling is disabled, force guarding to be off.
#ifndef _CPPUNWIND
	#undef  DO_GUARD
	#undef  DO_SLOW_GUARD
	#define DO_GUARD 0
	#define DO_SLOW_GUARD 0
#endif

// Make sure characters are unsigned.
#ifdef _CHAR_UNSIGNED
	#error "Bad VC++ option: Characters must be signed"
#endif

// No asm if not compiling for x86.
#ifndef _M_IX86
	#undef ASM
	#define ASM 0
#endif

// If no asm, redefine __asm to cause compile-time error.
#if !ASM
	#define __asm ERROR_ASM_NOT_ALLOWED
#endif

// FILE forward declaration.
typedef struct _iobuf FILE;
#define USEEK_CUR 1
#define USEEK_END 2
#define USEEK_SET 0

// NULL.
#define NULL 0

// Package implementation.
#define IMPLEMENT_PACKAGE_PLATFORM(pkgname) \
	extern "C" {HINSTANCE hInstance;} \
	INT __declspec(dllexport) __stdcall DllMain( HINSTANCE hInInstance, DWORD Reason, void* Reserved ) \
	{ hInstance = hInInstance; return 1; }

/*----------------------------------------------------------------------------
	Functions.
----------------------------------------------------------------------------*/

//
// Round a floating point number to an integer.
// Note that (int+.5) is rounded to (int+1).
//
#if ASM
#define DEFINED_appRound 1
inline INT appRound( FLOAT F )
{
	INT I;
	__asm fld [F]
	__asm fistp [I]
	return I;
}
#endif

//
// Converts to integer equal to or less than.
//
#if ASM
#define DEFINED_appFloor 1
inline INT appFloor( FLOAT F )
{
	static FLOAT Half=0.5;
	INT I;
	__asm fld [F]
	__asm fsub [Half]
	__asm fistp [I]
	return I;
}
#endif

//
// CPU cycles, related to GSecondsPerCycle.
//
#if ASM
#define DEFINED_appCycles 1
#pragma warning (disable : 4035)
inline DWORD appCycles()
{
	__asm
	{
		xor   eax,eax	// Required so that VC++ realizes EAX is modified.
		_emit 0x0F		// RDTSC  -  Pentium+ time stamp register to EDX:EAX.
		_emit 0x31		// Use only 32 bits in EAX - even a Ghz cpu would have a 4+ sec period.
		xor   edx,edx	// Required so that VC++ realizes EDX is modified.
	}
}
#pragma warning (default : 4035)
#endif

//
// Seconds, arbitrarily based.
//
#if ASM
#define DEFINED_appSeconds 1
#pragma warning (disable : 4035)
extern CORE_API DOUBLE GSecondsPerCycle;
inline DOUBLE appSeconds()
{
	DWORD L,H;
	__asm
	{
		xor   eax,eax	// Required so that VC++ realizes EAX is modified.
		xor   edx,edx	// Required so that VC++ realizes EDX is modified.
		_emit 0x0F		// RDTSC  -  Pentium+ time stamp register to EDX:EAX.
		_emit 0x31		// Use only 32 bits in EAX - even a Ghz cpu would have a 4+ sec period.
		mov   [L],eax   // Save low value.
		mov   [H],edx   // Save high value.
	}
	return ((DOUBLE)L +  4294967296.0 * (DOUBLE)H) * GSecondsPerCycle;
}
#pragma warning (default : 4035)
#endif

/*----------------------------------------------------------------------------
	Globals.
----------------------------------------------------------------------------*/

// System identification.
extern "C"
{
	extern HINSTANCE      hInstance;
	extern CORE_API UBOOL GIsMMX;
	extern CORE_API UBOOL GIsPentiumPro;
	extern CORE_API UBOOL GIsKatmai;
	extern CORE_API UBOOL GIsK6;
	extern CORE_API UBOOL GIsK63D;
}

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/

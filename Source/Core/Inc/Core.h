/*=============================================================================
	Core.h: Unreal core public header file.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_CORE
#define _INC_CORE

/*----------------------------------------------------------------------------
	Low level includes.
----------------------------------------------------------------------------*/

#ifndef CORE_API
#ifdef CORE_EXPORTS
#define CORE_API DLL_EXPORT
#else
#define CORE_API DLL_IMPORT
#endif
#endif

// Build options.
#include "UnBuild.h"

// Compiler specific include.
#if _MSC_VER
#include "UnVcWin32.h"
#else
#error Unknown Compiler
#endif
// API definition.

/*----------------------------------------------------------------------------
	Forward declarations.
----------------------------------------------------------------------------*/

// Objects.
class	UObject;
class		UExporter;
class		UFactory;
class		UField;
class			UConst;
class			UEnum;
class			UProperty;
class				UByteProperty;
class				UIntProperty;
class				UBoolProperty;
class				UFloatProperty;
class				UObjectProperty;
class					UClassProperty;
class				UNameProperty;
class				UStringProperty;
class				UStructProperty;
class			UStruct;
class				UFunction;
class				UState;
class					UClass;
class		ULinker;
class			ULinkerLoad;
class			ULinkerSave;
class		UPackage;
class		USubsystem;
class			USystem;
class		UTextBuffer;

// Structs.
class FName;
class FArchive;
class FCompactIndex;
class FExec;
class FGlobalPlatform;
class FGuid;
class FMemCache;
class FMemStack;
class FObjectManager;
class FOutputDevice;
class FPackageInfo;
class FTransactionTracker;
class FUnknown;
class FRepLink;

/*----------------------------------------------------------------------------
	Global variables.
----------------------------------------------------------------------------*/

// Core globals.
CORE_API extern FObjectManager			GObj;
CORE_API extern FGlobalPlatform*		GSystem;
CORE_API extern FTransactionTracker*	GUndo;
CORE_API extern FMemCache				GCache;
CORE_API extern FMemStack				GMem;
CORE_API extern FOutputDevice*			GLogHook;
CORE_API extern FExec*					GExecHook;
CORE_API extern USystem*				GSys;
CORE_API extern UProperty*				GProperty;
CORE_API extern DWORD*					GBoolAddr;
CORE_API extern char				    GErrorHist[4096];
CORE_API extern char                    GComputerName[32];
CORE_API extern	DOUBLE					GSecondsPerCycle;
CORE_API extern SQWORD					GTicks;
CORE_API extern INT                     GScriptCycles;
CORE_API extern DWORD					GPageSize;
CORE_API extern DWORD					GProcessorCount;
CORE_API extern DWORD					GPhysicalMemory;
CORE_API extern UBOOL					GIsEditor;
CORE_API extern UBOOL					GIsClient;
CORE_API extern UBOOL					GIsServer;
CORE_API extern UBOOL					GIsCriticalError;
CORE_API extern UBOOL					GIsStarted;
CORE_API extern UBOOL					GIsRunning;
CORE_API extern UBOOL					GIsSlowTask;
CORE_API extern UBOOL					GIsGuarded;
CORE_API extern UBOOL					GIsRequestingExit;
CORE_API extern UBOOL					GIsStrict;
CORE_API extern UBOOL                   GScriptEntryTag;
CORE_API extern UBOOL                   GNoAutoReplace;

// Per module globals.
extern "C" DLL_EXPORT char GPackage[];

// Normal includes.
#include "UnFile.h"			// Low level utility code.
#include "UnObjVer.h"		// Object version info.
#include "UnArc.h"			// Archive class.
#include "UnTemplate.h"     // Dynamic arrays.
#include "UnName.h"			// Global name subsystem.
#include "UnNames.h"		// Hardcoded names.
#include "UnPlatfm.h"		// Platform dependent subsystem definition.
#include "UnStack.h"		// Script stack definition.
#include "UnObjBas.h"		// Object base class.
#include "UnCorObj.h"		// Core object class definitions.
#include "UnClass.h"		// Class definition.
#include "UnType.h"			// Base property type.
#include "UnScript.h"		// Script class.
#include "UnCache.h"		// Cache based memory management.
#include "UnMem.h"			// Stack based memory management.
#include "UnCID.h"          // Cache ID's.

/*-----------------------------------------------------------------------------
	Core templates.
-----------------------------------------------------------------------------*/

//
// Dynamic array inlines.
//
template< class T > 
inline FArchive& operator<<( FArchive& Ar, TArray<T>& A )
{
	guard(TArray<<);
	if ( sizeof(T)==1 )
	{
		// Serialize simple bytes which require no construction or destruction.
		Ar << AR_INDEX(A.ArrayNum);
		if( Ar.IsLoading() )
		{
			A.ArrayMax = A.ArrayNum;
			A.Realloc( sizeof(T) );
		}
		Ar.Serialize( &A(0), A.Num() );
	}
	else if( Ar.IsLoading() )
	{
		// Load array.
		A.Empty();
		Ar << AR_INDEX(A.ArrayMax);
		A.Realloc( sizeof(T) );
		for( INT i=0; i<A.ArrayMax; i++ )
			Ar << *new(A)T;
		check(A.ArrayNum==A.ArrayMax);
	}
	else
	{
		// Save array.
		Ar << AR_INDEX(A.ArrayNum);
		for( INT i=0; i<A.ArrayNum; i++ )
			Ar << A( i );
	}
	return Ar;
	unguard;
}
template< class T >
inline void TArray<T>::Remove( INT Index, INT Count )
{
	guard(TArray::Remove);
	check(Index>=0);
	check(Index<=ArrayNum);
	check(Index+Count<=ArrayNum);

	// Do destruction.
	for( INT i=Index; i<Index+Count; i++ )
		(&(*this)(i))->~T();

	// Move binary data around.
	FArray::Remove( Index, Count, sizeof(T) );

	unguard;
}

/*----------------------------------------------------------------------------
	Outputable string.
----------------------------------------------------------------------------*/

class FStringOut : public FString, public FOutputDevice
{
	void WriteBinary( const void* Data, INT Length, EName MsgType=NAME_None )
	{
		*this += (char*)Data;
	}
};

/*-----------------------------------------------------------------------------
	Functions.
-----------------------------------------------------------------------------*/

CORE_API TArray<FString> appFindFiles( const char* Spec );

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
#endif

/*=============================================================================
	Core.cpp: Unreal core.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

#include "CorePrivate.h"

/*-----------------------------------------------------------------------------
	Global variables.
-----------------------------------------------------------------------------*/

// Global subsystems in the core.
CORE_API FObjectManager GObj;
CORE_API FMemCache GCache;
CORE_API FMemStack GMem;

// Global subsystems outside the core.
CORE_API USystem* GSys=NULL;
CORE_API FGlobalPlatform* GSystem=NULL;
CORE_API FTransactionTracker* GUndo=NULL;
CORE_API FOutputDevice* GLogHook=NULL;
CORE_API FExec* GExecHook=NULL;
CORE_API UProperty* GProperty;
CORE_API DWORD* GBoolAddr;
CORE_API DOUBLE GSecondsPerCycle=1.0;
CORE_API SQWORD GTicks=1;
CORE_API char GErrorHist[4096]="";
CORE_API char GComputerName[32]="";
CORE_API INT GScriptCycles;
CORE_API DWORD GPageSize=4096;
CORE_API DWORD GProcessorCount=1;
CORE_API DWORD GPhysicalMemory=16384*1024;
CORE_API UBOOL GIsEditor=0;
CORE_API UBOOL GIsClient=0;
CORE_API UBOOL GIsServer=0;
CORE_API UBOOL GIsCriticalError=0;
CORE_API UBOOL GIsStarted=0;
CORE_API UBOOL GIsRunning=0;
CORE_API UBOOL GIsSlowTask=0;
CORE_API UBOOL GIsGuarded=0;
CORE_API UBOOL GIsRequestingExit=0;
CORE_API UBOOL GIsStrict=0;
CORE_API UBOOL GScriptEntryTag=0;
CORE_API UBOOL GNoAutoReplace=0;

// System identification.
#if __INTEL__
CORE_API UBOOL GIsMMX=0;
CORE_API UBOOL GIsPentiumPro=0;
CORE_API UBOOL GIsKatmai=0;
CORE_API UBOOL GIsK6=0;
CORE_API UBOOL GIsK63D=0;
#endif

/*-----------------------------------------------------------------------------
	Explicit instantiation.
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
	Package implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_PACKAGE(Core);

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

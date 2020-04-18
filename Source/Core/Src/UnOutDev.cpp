/*=============================================================================
	UnOutDev.cpp: Unreal FOutputDevice implementation
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "CorePrivate.h"

/*-----------------------------------------------------------------------------
	FOutputDevice implementation.
-----------------------------------------------------------------------------*/

//
// Print a message on the output device using NAME_Log type.
//
void FOutputDevice::Log( const char* Text )
{
	guard(FOutputDevice::Log);
	WriteBinary( Text, appStrlen(Text), NAME_Log );
	unguard;
};

//
// Print a message on the output device using NAME_Log type.
//
void FOutputDevice::Log( EName Event, const char* Text )
{
	guardSlow(FOutputDevice::Log);
	if( !(FName(Event).GetFlags() & RF_Suppress) )
		WriteBinary( Text, appStrlen(Text), Event );
	unguardSlow;
};

//
// Print a message on the output device, variable parameters.
//
void VARARGS FOutputDevice::Logf( EName Event, const char* Fmt, ... )
{
	guardSlow(FOutputDevice::Logf);
	if( !(FName(Event).GetFlags() & RF_Suppress) )
	{
		char TempStr[4096];
		GET_VARARGS( TempStr, Fmt );
		WriteBinary( TempStr, appStrlen(TempStr), Event );
	}
	unguardSlow;
}

//
// Print a message on the output device, variable parameters.
//
void VARARGS FOutputDevice::Logf( const char* Fmt, ... )
{
	char TempStr[4096];
	GET_VARARGS( TempStr, Fmt );
	WriteBinary( TempStr, appStrlen(TempStr), NAME_Log );
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

/*=============================================================================
	IpDrvPrivate.h: Unreal TCP/IP driver.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

#pragma warning( disable : 4201 )
#include <windows.h>
#include <winsock.h>
#include "Engine.h"
#include "UnNet.h"

/*-----------------------------------------------------------------------------
	Definitions.
-----------------------------------------------------------------------------*/

struct FIpAddr
{
	DWORD Addr;
	DWORD Port;
};

char* wsaError( INT Code=-1 );
UBOOL wsaInit( char* Error256 );
extern UBOOL GInitialized;

/*-----------------------------------------------------------------------------
	Public includes.
-----------------------------------------------------------------------------*/

#include "IpDrvClasses.h"

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

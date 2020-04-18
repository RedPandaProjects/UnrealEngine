/*=============================================================================
	UnNet.h: Unreal networking.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Forward declarations.
-----------------------------------------------------------------------------*/

class FChannel;
class FBunchHeader;
class FInBunch;
class FOutBunch;

/*-----------------------------------------------------------------------------
	High level.
-----------------------------------------------------------------------------*/

enum {NET_REVISION=1};

/*-----------------------------------------------------------------------------
	Includes.
-----------------------------------------------------------------------------*/

#include "UnNetDrv.h"   // Network driver class.
#include "UnConn.h"     // Connection class.
#include "UnBunch.h"    // Bunch class.
#include "UnChan.h"     // Channel class.

/*-----------------------------------------------------------------------------
	UPendingLevel.
-----------------------------------------------------------------------------*/

//
// Class controlling a pending game level.
//
class UPendingLevel : public ULevelBase
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UPendingLevel,UObject,CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(UPendingLevel)

	// Variables.
	UBOOL		Success;
	UBOOL		SentJoin;
	UBOOL		LonePlayer;
	INT			FilesNeeded;
	char		Error256[256];

	// Constructors.
	UPendingLevel( UEngine* InEngine, const FURL& InURL );

	// FNetworkNotify interface.
	EAcceptConnection NotifyAcceptingConnection();
	void NotifyAcceptedConnection( class UNetConnection* Connection );
	UBOOL NotifyAcceptingChannel( class FChannel* Channel );
	ULevel* NotifyGetLevel();
	void NotifyReceivedText( UNetConnection* Connection, const char* Text );
	void NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const char* Error );
	UBOOL NotifySendingFile( UNetConnection* Connection, FGuid Guid );

	// UPendingLevel interface.
	void Tick( FLOAT DeltaTime );
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

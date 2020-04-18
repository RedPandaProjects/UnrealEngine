/*=============================================================================
	UnPlayer.h: Unreal player class.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	UPlayer.
-----------------------------------------------------------------------------*/

//
// A player, the base class of UViewport (local players) and UNetConnection (remote players).
//
class ENGINE_API UPlayer : public UObject, public FOutputDevice
{
	DECLARE_ABSTRACT_CLASS(UPlayer,UObject,CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(UPlayer)

	// Objects.
	APlayerPawn* Actor;
	UConsole* Console;
	FString	TravelItems;

	// Constructor.
	UPlayer( ULevel* InLevel );

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();

	// UPlayer interface.
	virtual void ReadInput( FLOAT DeltaSeconds )=0;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

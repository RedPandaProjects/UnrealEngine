/*=============================================================================
	UnNetDrv.h: Unreal network driver base class.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	UNetDriver.
-----------------------------------------------------------------------------*/

//
// Base class of a network driver attached to an active or pending level.
//
class ENGINE_API UNetDriver : public USubsystem
{
	DECLARE_ABSTRACT_CLASS(UNetDriver,USubsystem,CLASS_Transient|CLASS_Config)

	// Variables.
	TArray<UNetConnection*>	Connections;
	UNetConnection*			ServerConnection;
	FNetworkNotify*			Notify;
	FPackageMap				Map;
	DOUBLE					Time;
	FLOAT					ConnectionTimeout;
	FLOAT					InitialConnectTimeout;
	FLOAT					AckTimeout;
	FLOAT					KeepAliveTime;
	FLOAT					DumbProxyTimeout;
	FLOAT					SimulatedProxyTimeout;
	FLOAT					SpawnPrioritySeconds;
	FLOAT					ServerTravelPause;
	INT						DefaultByteLimit;
	INT						MaxClientByteLimit;
	INT						MaxTicksPerSecond;
	UBOOL					DuplicateClientMoves;

	// Constructors.
	UNetDriver();
	static void InternalClassInitializer( UClass* Class );

	// UObject interface.
	void Destroy();
	void Serialize( FArchive& Ar );

	// FNetworkDriver interface.
	virtual UBOOL Init( UBOOL Connect, FNetworkNotify* InNotify, FURL& URL, char* Error256 );
	virtual void Tick()=0;
	virtual UBOOL Exec( const char* Cmd, FOutputDevice* Out=GSystem )=0;
	virtual UBOOL IsInternet()=0;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

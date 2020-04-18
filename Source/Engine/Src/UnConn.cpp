/*=============================================================================
	UnConn.h: Unreal connection base class.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"

/*-----------------------------------------------------------------------------
	AInfo object implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(AInfo);

/*-----------------------------------------------------------------------------
	UNetDriver implementation.
-----------------------------------------------------------------------------*/

UNetDriver::UNetDriver()
:	Connections				()
,	Time					( appSeconds() )
,	ConnectionTimeout		( 15.0  )
,	InitialConnectTimeout	( 120.0 )
,	AckTimeout				( 1.0   )
,	KeepAliveTime			( 1.0   )
,	DefaultByteLimit		( 22000 )
,	MaxClientByteLimit		( 22000 )
,	DumbProxyTimeout		( 1.0   )
,	SimulatedProxyTimeout	( 10.0  )
,	SpawnPrioritySeconds    ( 1.0   )
,	ServerTravelPause		( 5.0   )
,	DuplicateClientMoves	( 1 )
,	MaxTicksPerSecond		( 30 )
{}

void UNetDriver::InternalClassInitializer( UClass* Class )
{
	guard(UNetDriver::InternalClassInitializer);
	if( appStricmp( Class->GetName(), "NetDriver" )==0 )
	{
		new(Class,"ConnectionTimeout",    RF_Public)UFloatProperty(CPP_PROPERTY(ConnectionTimeout    ), "Client", CPF_Config );
		new(Class,"InitialConnectTimeout",RF_Public)UFloatProperty(CPP_PROPERTY(InitialConnectTimeout), "Client", CPF_Config );
		new(Class,"AckTimeout",           RF_Public)UFloatProperty(CPP_PROPERTY(AckTimeout           ), "Client", CPF_Config );
		new(Class,"KeepAliveTime",        RF_Public)UFloatProperty(CPP_PROPERTY(KeepAliveTime        ), "Client", CPF_Config );
		new(Class,"DumbProxyTimeout",     RF_Public)UFloatProperty(CPP_PROPERTY(DumbProxyTimeout     ), "Client", CPF_Config );
		new(Class,"SimulatedProxyTimeout",RF_Public)UFloatProperty(CPP_PROPERTY(SimulatedProxyTimeout), "Client", CPF_Config );
		new(Class,"SpawnPrioritySeconds", RF_Public)UFloatProperty(CPP_PROPERTY(SpawnPrioritySeconds ), "Client", CPF_Config );
		new(Class,"ServerTravelPause",    RF_Public)UFloatProperty(CPP_PROPERTY(ServerTravelPause    ), "Client", CPF_Config );
		new(Class,"DefaultByteLimit",     RF_Public)UIntProperty  (CPP_PROPERTY(DefaultByteLimit     ), "Client", CPF_Config );
		new(Class,"MaxClientByteLimit",   RF_Public)UIntProperty  (CPP_PROPERTY(MaxClientByteLimit   ), "Client", CPF_Config );
		new(Class,"MaxTicksPerSecond",    RF_Public)UIntProperty  (CPP_PROPERTY(MaxTicksPerSecond    ), "Client", CPF_Config );
		new(Class,"DuplicateClientMoves", RF_Public)UBoolProperty (CPP_PROPERTY(DuplicateClientMoves ), "Client", CPF_Config );
	}
	unguard;
}
UBOOL UNetDriver::Init( UBOOL Connect, FNetworkNotify* InNotify, FURL& URL, char* Error256 )
{
	guard(UNetDriver::Init);

	// Save notify.
	Notify = InNotify;

	return 1;
	unguard;
}
void UNetDriver::Serialize( FArchive& Ar )
{
	guard(UNetDriver::Serialize);

	UObject::Serialize( Ar );
	Ar << Connections << ServerConnection << Map;

	unguard;
}
void UNetDriver::Destroy()
{
	guard(UNetDriver::Destroy);
	Super::Destroy();
	unguard;
}
IMPLEMENT_CLASS(UNetDriver);

/*-----------------------------------------------------------------------------
	UPendingLevel implementation.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
UPendingLevel::UPendingLevel( UEngine* InEngine, const FURL& InURL )
:	ULevelBase( InEngine, InURL )
{
	guard(UPendingLevel::UPendingLevel);

	// Init.
	URL        = InURL;
	*Error256  = 0;
	NetDriver  = NULL;

	// Try to create network driver.
	UClass* NetDriverClass = GObj.LoadClass( UNetDriver::StaticClass, NULL, "ini:Engine.Engine.NetworkDevice", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
	NetDriver = ConstructClassObject<UNetDriver>( NetDriverClass );
	if( NetDriver->Init( 1, this, URL, Error256) )
	{
		// Send initial message.
		NetDriver->ServerConnection->Logf( "HELLO REVISION=1", NET_REVISION );
		NetDriver->ServerConnection->FlushNet();
	}
	else
	{
		delete NetDriver;
		NetDriver=NULL;
	}
	unguard;
}

//
// FNetworkNotify interface.
//
EAcceptConnection UPendingLevel::NotifyAcceptingConnection()
{
	guard(UPendingLevel::NotifyAcceptingConnection);
	return ACCEPTC_Reject;
	unguard;
}
void UPendingLevel::NotifyAcceptedConnection( class UNetConnection* Connection )
{
	guard(UPendingLevel::NotifyAcceptedConnection);
	unguard;
}
UBOOL UPendingLevel::NotifyAcceptingChannel( class FChannel* Channel )
{
	guard(UPendingLevel::NotifyAcceptingChannel);
	return 0;
	unguard;
}
ULevel* UPendingLevel::NotifyGetLevel()
{
	guard(UPendingLevel::NotifyGetLevel);
	return NULL;
	unguard;
}
void UPendingLevel::NotifyReceivedText( UNetConnection* Connection, const char* Text )
{
	guard(UPendingLevel::NotifyReceivedText);
	check(Connection==NetDriver->ServerConnection);
	debugf( NAME_DevNet, "PendingLevel received: %s", Text );

	// This client got a response from the server.
	if( ParseCommand(&Text,"UPGRADE") )
	{
		// Report mismatch.
		INT Revision=0;
		Parse( Text, "REVISION=", Revision );
		Engine->SetProgress( "", "", -1.0 );
	}
	else if( ParseCommand(&Text,"FAILURE") )
	{
		// Report problem to user.
		Engine->SetProgress( "Rejected By Server", Text, 10.0 );
	}
	else if( ParseCommand( &Text, "USES" ) )
	{
		// Dependency information.
		FPackageInfo& Info = Connection->Driver->Map( Connection->Driver->Map.AddItem( FPackageInfo(NULL) ) );
		char PackageName[NAME_SIZE];
		Parse( Text, "GUID=", Info.Guid );
		Parse( Text, "SIZE=", Info.FileSize );
		Parse( Text, "FLAGS=", Info.PackageFlags );
		Parse( Text, "PACKAGE=", PackageName, ARRAY_COUNT(PackageName) );
		Info.Parent = GObj.CreatePackage(NULL,PackageName);
	}
	else if( ParseCommand( &Text, "CHALLENGE" ) )
	{
		// Challenged by server.
		Parse( Text,"CHALLENGE=", Connection->Challenge );
		FString Str;
		URL.String( Str );
		NetDriver->ServerConnection->Logf( "LOGIN RESPONSE=%i URL=%s", Engine->ChallengeResponse(Connection->Challenge), *Str );
		NetDriver->ServerConnection->FlushNet();
	}
	else if( ParseCommand( &Text, "WELCOME" ) )
	{
		// Server accepted connection.
		debugf( NAME_DevNet, "Welcomed by server: %s", Text );

		// Parse welcome message.
		URL.Map.Parse( Text, "LEVEL=" );
		ParseUBOOL( Text, "LONE=", LonePlayer );
		Parse( Text, "CHALLENGE=", Connection->Challenge );

		// Make sure all packages we need are downloadable.
		for( INT i=0; i<Connection->Driver->Map.Num(); i++ )
		{
			char Filename[256];
			FPackageInfo& Info = Connection->Driver->Map(i);
			if( !appFindPackageFile( Info.Parent->GetName(), &Info.Guid, Filename ) )
			{
				appSprintf( Filename, "%s.dll", Info.Parent->GetName() );
				if( appFSize(Filename) <= 0 )
				{
					// We need to download this package.
					FilesNeeded++;
					Info.PackageFlags |= PKG_Need;
					if( !(Info.PackageFlags & PKG_AllowDownload) )
					{
						char Msg[256];
						appSprintf( Msg, "Downloading '%s' not allowed", Info.Parent->GetName() );
						Engine->SetProgress( "Unable To Enter This Server", Msg, 10.0 );
						NetDriver->ServerConnection->State = USOCK_Closed;
						return;
					}
				}
			}
		}

		// Send first download request.
		for( i=0; i<Connection->Driver->Map.Num(); i++ )
			if( Connection->Driver->Map(i).PackageFlags & PKG_Need )
				{Connection->ReceiveFile( i ); break;}

		// We have successfully connected.
		Success = 1;
	}
	else
	{
		// Other command.
	}
	unguard;
}
void UPendingLevel::NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const char* Error )
{
	guard(UPendingLevel::NotifyReceivedFile);
	check(NetDriver->Map.IsValidIndex(PackageIndex));

	// Map pack to package.
	FPackageInfo& Info = NetDriver->Map(PackageIndex);
	if( Error[0] )
	{
		// If transfer failed, so propagate error.
		if( !Error256[0] )
			appSprintf( Error256, LocalizeError("DownloadFailed"), Info.Parent->GetName(), Error );
	}
	else
	{
		// Now that a file has been successfully received, mark its package as downloaded.
		check(Connection==NetDriver->ServerConnection);
		check(Info.PackageFlags&PKG_Need);
		Info.PackageFlags &= ~PKG_Need;
		FilesNeeded--;

		// Send next download request.
		for( INT i=0; i<Connection->Driver->Map.Num(); i++ )
			if( Connection->Driver->Map(i).PackageFlags & PKG_Need )
				{Connection->ReceiveFile( i ); break;}
	}
	unguard;
}
UBOOL UPendingLevel::NotifySendingFile( UNetConnection* Connection, FGuid Guid )
{
	guard(UPendingLevel::NotifySendingFile);

	// Server has requested a file from this client.
	debugf( NAME_DevNet, LocalizeError("RequestDenied") );
	return 0;

	unguard;
}

//
// Update the pending level's status.
//
void UPendingLevel::Tick( FLOAT DeltaTime )
{
	guard(UPendingLevel::Tick);
	check(NetDriver);
	check(NetDriver->ServerConnection);

	// Handle timed out or failed connection.
	if( NetDriver->ServerConnection->State==USOCK_Closed && !Error256[0] )
	{
		appSprintf( Error256, LocalizeError("ConnectionFailed") );
		return;
	}

	// Update network driver.
	NetDriver->Tick();

	unguard;
}
IMPLEMENT_CLASS(UPendingLevel);

/*-----------------------------------------------------------------------------
	UNetConnection object implementation.
-----------------------------------------------------------------------------*/

void UNetConnection::Serialize( FArchive &Ar )
{
	guard(UNetConnection::Serialize);

	// Serialize parent class.
	UPlayer::Serialize( Ar );

	unguard;
}
void UNetConnection::Destroy()
{
	guard(UNetConnection::Destroy);

	// Remove from driver.
	if( Driver->ServerConnection )
	{
		check(Driver->ServerConnection==this);
		Driver->ServerConnection=NULL;
	}
	else
	{
		INT Index;
		check(Driver->ServerConnection==NULL);
		check(Driver->Connections.FindItem( this, Index ));
		Driver->Connections.RemoveItem( this );
	}

	// Set to closed so the channels don't try to send data.
	State = USOCK_Closed;

	// Kill all channels.
	for( FChannelIterator It(this); It; ++It )
		delete *It;

	Super::Destroy();
	unguard;
}
IMPLEMENT_CLASS(UNetConnection);

/*-----------------------------------------------------------------------------
	Base class init and exit.
-----------------------------------------------------------------------------*/

//
// Init this connection's base properties.
//
UNetConnection::UNetConnection( UNetDriver* InDriver )
:	Driver				( InDriver )
,	State				( USOCK_Invalid )
,	ProtocolVersion		( MIN_PROTOCOL_VERSION )
,	MaxPacket			( 0 )
,	ByteLimit			( InDriver->DefaultByteLimit )
,	SimLatency			( 0 )
,	SimPacketLoss		( 0 )
,	LastReceiveTime		( Driver->Time )
,	LastSendTime		( Driver->Time )
,	LastTickTime		( Driver->Time )
,	QueuedBytes			( 0 )
,	OutNum				( 0 )
,	URL					()
{
	guard(UNetConnection::UNetConnection);

	// Init the list of channels.
	for( INT i=0; i<MAX_CHANNELS; i++ )
	{
		Channels     [i] = NULL;
		OutReliable  [i] = SEQ_None;
		OutSequence  [i] = SEQ_Initial;
		LastInRetired[i] = SEQ_None;
		LastInRcvd   [i] = SEQ_None;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Validation.
-----------------------------------------------------------------------------*/

//
// Make sure this connection is in a reasonable state.
//
void UNetConnection::AssertValid()
{
	guard(UNetConnection::AssertValid);

	check(ProtocolVersion>=MIN_PROTOCOL_VERSION);
	check(ProtocolVersion<=MAX_PROTOCOL_VERSION);
	check(State==USOCK_Closed || State==USOCK_Pending || State==USOCK_Open);
	check(MaxPacket<=MAX_PACKET_SIZE);

	unguard;
}

/*-----------------------------------------------------------------------------
	Flush.
-----------------------------------------------------------------------------*/

void UNetConnection::FlushNet( UBOOL Duplicate )
{
	guard(UNetConnection::FlushNet);
	LastBunchEnd = 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	FOutputDevice interface.
-----------------------------------------------------------------------------*/

void UNetConnection::WriteBinary( const void* Data, int Length, EName MsgType )
{
	guard(UNetConnection::WriteBinary);

	if(Channels[0] && Channels[0]->State==UCHAN_Open )
		((FControlChannel*)Channels[0])->WriteBinary( Data, Length, MsgType );

	unguard;
}

/*-----------------------------------------------------------------------------
	Bandwidth limiting.
-----------------------------------------------------------------------------*/

//
// This should be overridden in subclasses.
//
UBOOL UNetConnection::IsNetReady()
{
	guard(UNetConnection::IsReady);
	return QueuedBytes <= 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	Packet reception.
-----------------------------------------------------------------------------*/

//
// Handle a packet we just received.
//
void UNetConnection::ReceivedPacket( BYTE* Data, INT Size )
{
	guard(UNetConnection::ReceivedPacket);
	AssertValid();

	// Disassemble and dispatch all bunches in the packet.
	while( Size > 0 )
	{
		// Get pointers.
		FBunchHeader* BunchHeader = (FBunchHeader*)Data;
		FBunch*       Bunch       = (FBunch*)Data;
		BYTE*         BunchData   = &Data[sizeof(FBunch)];

		// Process the bunch.
		if( Size < sizeof(FBunchHeader) )
		{
			// Too small to be anything valid.
			debugfSlow( NAME_DevNetTraffic, "Connection received unknown bunch (%i/%i)", Size, sizeof(FBunch) );
			return;
		}
		else if( BunchHeader->ChIndex & CHF_Ack )
		{
			// This is an acknowledgement or negative acknowledgement.
			INT ChannelIndex  = Bunch->ChIndex & CHF_Mask;
			Data             += sizeof(FBunchHeader);
			Size             -= sizeof(FBunchHeader);

			// Forward the ack to the channel.
			debugfSlow( NAME_DevNetTraffic, "   Received ack %i: %i!", ChannelIndex, BunchHeader->Sequence );
			if( Channels[ChannelIndex] )
			{
				if( Bunch->ChIndex & CHF_AckNak )
					Channels[ChannelIndex]->ReceivedNak( BunchHeader->Sequence );
				else
					Channels[ChannelIndex]->ReceivedAck( BunchHeader->Sequence );
			}
			else debugfSlow( NAME_DevNetTraffic, "Received ack on closed channel %i", ChannelIndex );
		}
		else if( Size < sizeof(FBunch) )
		{
			// Too small to be a valid data bunch.
			debugfSlow( NAME_DevNetTraffic, "Connection received unknown bunch (%i/%i)", Size, sizeof(FBunch) );
			return;
		}
		else if( Bunch->GetTotalSize() > Size )
		{
			// Bunch claims it's larger than the enclosing packet.
			debugfSlow( NAME_DevNetTraffic, "Connection bunch overrun (%i/%i)", Bunch->GetTotalSize(), Size );
			return;
		}
		else
		{
			// This is a valid data bunch.
			Data += Bunch->GetTotalSize();
			Size -= Bunch->GetTotalSize();
			INT ChIndex = Bunch->ChIndex & CHF_Mask;
			if( Channels[ChIndex] )
			{
				// Verify channel type.
				debugfSlow( NAME_DevNetTraffic, "   Bunch %i.%i: ChType %i, Size %i -> %i", ChIndex, Bunch->Sequence, Bunch->_ChType, Bunch->GetTotalSize(), Bunch->DataSize );
			}
			else if( Bunch->ChIndex & CHF_Close )
			{
				// If sender is resending his close-request on a channel that's already closed, just re-ack it.
				debugfSlow( NAME_DevNetTraffic, "   Close %i: Size %i", ChIndex, Bunch->_ChType, Bunch->GetTotalSize() );
				if( Bunch->Sequence == LastInRetired[Bunch->ChIndex&CHF_Mask] )
					SendAck( Bunch->ChIndex&CHF_Mask, Bunch->Sequence );
				else
					debugfSlow( NAME_DevNetTraffic, "Got close request on unknown channel %i/%i", Bunch->Sequence, LastInRetired[Bunch->ChIndex&CHF_Mask] );
				continue;
			}
			else
			{
				if( !GIsKnownChannelType(Bunch->_ChType) )
				{
					debugfSlow( NAME_DevNetTraffic, "Connection unknown bunch type (%i)", Bunch->_ChType );
					continue;
				}
				debugfSlow( NAME_DevNetTraffic, "   Bunch Create %i: ChType %i, Size %i -> %i", ChIndex, Bunch->_ChType, Bunch->GetTotalSize(), Bunch->DataSize );
				if( Channels[0] || (ChIndex==0 && Bunch->_ChType==CHTYPE_Control) )
				{
					// Create new channel.
					FChannel* Channel = CreateChannel( (EChannelType)Bunch->_ChType, 0, ChIndex );

					// Notify the server of the new channel.
					if( !Driver->Notify->NotifyAcceptingChannel( Channel ) )
					{
						// Channel refused, so close it, flush it, and delete it.
						Channel->SendClose();
						FlushNet();
						delete Channel;
						if( ChIndex==0 )
						{
							debugfSlow( NAME_DevNetTraffic, "Channel 0 create failed" );
							delete this;
							return;
						}
						continue;
					}
				}
				else
				{
					debugfSlow( NAME_DevNetTraffic, "Received bunch channel %i before connected", ChIndex );
					continue;
				}
			}

			// Dispatch the raw, unsequenced bunch to the channel.
			FInBunch LocalBunch( this, Bunch, BunchData );
			Channels[ChIndex]->ReceivedRawBunch( LocalBunch, 1 );
		}
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Special packet sending.
-----------------------------------------------------------------------------*/

//
// Send an acknowledgement.
//
void UNetConnection::SendAck( _WORD ChIndex, _WORD Sequence )
{
	guard(UNetConnection::SendAck);

	// Send the ack.
	FBunchHeader Ack( ChIndex | CHF_Ack, Sequence );

	// Flush the current packet if there's no room.
	if( OutNum + (INT)sizeof(Ack) > MaxPacket )
		FlushNet();

	// Append onto the outgoing packet.
	appMemcpy( &OutData[OutNum], &Ack, sizeof(Ack) );
	OutNum += sizeof(Ack);

	unguard;
}

//
// Send a negative acknowledgement.
//
void UNetConnection::SendNak( _WORD ChIndex, _WORD Sequence )
{
	guard(UNetConnection::SendNak);

	// Send the nak.
	FBunchHeader Nak( ChIndex | CHF_Ack | CHF_AckNak, Sequence );

	// Flush the current packet if there's no room.
	if( OutNum + (INT)sizeof(Nak) > MaxPacket )
		FlushNet();

	// Append onto the outgoing packet.
	appMemcpy( &OutData[OutNum], &Nak, sizeof(Nak) );
	OutNum += sizeof(Nak);

	unguard;
}

/*-----------------------------------------------------------------------------
	Channel creation.
-----------------------------------------------------------------------------*/

//
// Create a channel.
//
FChannel* UNetConnection::CreateChannel( EChannelType ChType, UBOOL bOpenedLocally, _WORD ChIndex )
{
	guard(UNetConnection::CreateChannel);
	check(GIsKnownChannelType(ChType));
	AssertValid();
	
	// If no channel index was specified, find the first available.
	if( ChIndex == MAXWORD )
	{
		for( ChIndex=0; ChIndex<MAX_CHANNELS; ChIndex++ )
			if( !Channels[ChIndex] )
				break;
		if( ChIndex == MAX_CHANNELS )
			return NULL;
	}

	// Make sure channel is valid.
	check(ChIndex<MAX_CHANNELS);
	check(Channels[ChIndex]==NULL);

	// Create channel.
	Channels[ChIndex] = GChannelConstructors[ChType]( this, ChIndex, bOpenedLocally );
	//debugf( "Created channel %i of type %i", ChIndex, ChType);

	return Channels[ChIndex];
	unguard;
}

/*-----------------------------------------------------------------------------
	Connection polling.
-----------------------------------------------------------------------------*/

//
// Poll the connection.
// If it is timed out, close it.
//
void UNetConnection::Tick()
{
	guard(UNetConnection::Tick);
	AssertValid();

	// Update queued byte count.
	FLOAT AllowedLatency = -ByteLimit/20.0;
	QueuedBytes -= (Driver->Time - LastTickTime) * ByteLimit;
	if( QueuedBytes < AllowedLatency )
		QueuedBytes = AllowedLatency;
	LastTickTime = Driver->Time;

	// Handle timeouts.
	FLOAT Timeout = (State==USOCK_Pending || !Actor) ? Driver->InitialConnectTimeout : Driver->ConnectionTimeout;
	if( Driver->Time - LastReceiveTime > Timeout )
	{
		// Timeout.
		if( State != USOCK_Closed )
			debugf( NAME_DevNet, "Connection timed out afer %f seconds (%f)", Timeout, Driver->Time - LastReceiveTime );
		State = USOCK_Closed;
	}
	else
	{
		// Tick the channels.
		for( FChannelIterator It(this); It; ++It )
			It->Tick();

		// If channel 0 has closed, mark the conection as closed.
		if( Channels[0]==NULL && (OutReliable[0]!=SEQ_None || LastInRetired[0]!=SEQ_None) )
			State = USOCK_Closed;
	}

	// Flush.
	FlushNet();

	unguard;
}

/*-----------------------------------------------------------------------------
	Actor channel.
-----------------------------------------------------------------------------*/

//
// Return the FActorChannel corresponding to a specified actor, or NULL
// if no such channel is open.
//
FActorChannel* UNetConnection::GetActorChannel( AActor* Actor )
{
	guardSlow(UNetConnection::GetActorChannel);
	//!!optimize with hash.
	for( FTypedChannelIterator<FActorChannel> It(this); It; ++It )
		if( It->State==UCHAN_Open && It->Actor==Actor )
			return *It;
	return NULL;
	unguardSlow;
}

/*---------------------------------------------------------------------------------------
	File transfer.
---------------------------------------------------------------------------------------*/

//
// Initiate downloading a file to the cache directory.
// The transfer will eventually succeed or fail, and the
// NotifyReceivedFile will be called with the results.
//
void UNetConnection::ReceiveFile( INT PackageIndex )
{
	guard(UNetConnection::ReceiveFile);
	check(Driver->Map.IsValidIndex(PackageIndex));
	FPackageInfo& Info = Driver->Map( PackageIndex );

	// Create channel.
	FFileChannel* Ch = (FFileChannel *)CreateChannel( CHTYPE_File, 1 );
	if( !Ch )
	{
		Driver->Notify->NotifyReceivedFile( this, PackageIndex, LocalizeError("ChAllocate") );
		return;
	}

	// Set channel properties.
	Ch->PackageIndex = PackageIndex;
	appStrcpy( Ch->PrettyName, Driver->Map(PackageIndex).Parent->GetName() );

	// Send file request.
	FOutBunch Bunch( Ch );
	Bunch << Info.Guid;
	Bunch.Header.ChIndex |= CHF_Reliable;
	Ch->SendBunch( Bunch, 0 );

	unguard;
}

/*---------------------------------------------------------------------------------------
	UNetConnection UPlayer interface.
---------------------------------------------------------------------------------------*/

//
// Read input from this network player.
//
void UNetConnection::ReadInput( APlayerPawn* Pawn, FLOAT DeltaSeconds )
{}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

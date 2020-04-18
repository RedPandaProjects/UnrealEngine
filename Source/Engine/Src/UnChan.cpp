/*=============================================================================
	UnChan.cpp: Unreal datachannel implementation.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"

/*-----------------------------------------------------------------------------
	FChannel implementation.
-----------------------------------------------------------------------------*/

//
// Initialize the base channel.
//
FChannel::FChannel( INT InChType, UNetConnection* InConnection, INT InChIndex, INT InOpenedLocally )
:	Connection		( InConnection )
,	ChIndex			( InChIndex )
,	OpenedLocally	( InOpenedLocally )
,	State			( UCHAN_Open )
,	ChType			( (EChannelType)InChType )
{
	guard(FChannel::FChannel);
	check(CHF_Mask>=UNetConnection::MAX_CHANNELS);

	// Init incoming and outgoing buffers.
	for( INT i=0; i<RELIABLE_BUFFER; i++ )
	{
		OutRec[i] = NULL;
		InRec [i] = NULL;
	}
	unguard;
}

//
// Close the base channel.
//
void FChannel::Close()
{
	guard(FChannel::Close);
	check(ChIndex==0 || OpenedLocally);
	check(State==UCHAN_Open);
	check(Connection->Channels[ChIndex]==this);
	if( (Connection->State==USOCK_Open || Connection->State==USOCK_Pending) && State==UCHAN_Open )
	{
		// Send a close notify, and wait for ack.
		SendClose();
		State     = UCHAN_Closing;
		CloseTime = Connection->Driver->Time;
	}
	unguard;
}

//
// Base channel destructor.
//
FChannel::~FChannel()
{
	guard(FChannel::~FChannel);
	check(Connection);
	check(Connection->Channels[ChIndex]==this);

	// Free any pending incoming and outgoing bunches.
	for( INT i=0; i<RELIABLE_BUFFER; i++ )
	{
		if( OutRec[i] ) FreePooledBunch( OutRec[i] );
		if( InRec [i] ) FreePooledBunch( InRec [i] );
	}

	// Remove from connection's channel table.
	Connection->Channels[ChIndex] = NULL;
	Connection                    = NULL;

	unguard;
}

//
// Handle an acknowledgement on this channel.
//
void FChannel::ReceivedAck( _WORD Sequence )
{
	guard(FChannel::ReceivedAck);
	check(Connection->Channels[ChIndex]==this);

	for( INT i=0; i<RELIABLE_BUFFER; i++ )
		if( OutRec[i] && OutRec[i]->Header.Sequence==Sequence )
			break;
	if( i<RELIABLE_BUFFER )
	{
		UBOOL DoClose = (OutRec[i]->Header.ChIndex & CHF_Close);

		// Delete the acknowledged outgoing record.
		FreePooledBunch( OutRec[i] );
		OutRec[i] = NULL;

		// If the outgoing packet was a close message, destroy this channel.
		if( DoClose )
			delete this;
	}
	else
	{
		// Unrecognized acknowledgement.
		debugfSlow( NAME_DevNetTraffic, "   Unexpected ack (%i/%i)", ChIndex, Sequence, Connection->LastInRetired[ChIndex] );
	}
	unguard;
}

//
// Received a negative acknowledgement.
//
void FChannel::ReceivedNak( _WORD Sequence )
{
	guard(FBunch::ReceivedNak);
	check(Connection->Channels[ChIndex]==this);
	for( INT i=0; i<RELIABLE_BUFFER; i++ )
		if( OutRec[i] && OutRec[i]->Header.Sequence==Sequence )
			break;
	if( i < RELIABLE_BUFFER )
	{
		// Resend the requested record.
		SendRawBunch( *OutRec[i] );
		OutTime[i] = Connection->Driver->Time;
	}
	else
	{
		// Unrecognized acknowledgement.
		debugfSlow( NAME_DevNetTraffic, "Channel %i received unexpected nak (%i/%i)", ChIndex, Sequence, Connection->LastInRetired[ChIndex] );
	}
	unguard;
}

//
// Return the maximum amount of data that can be sent in this
// bunch without overflow.
//
INT FChannel::MaxSend()
{
	guard(FChannel::MaxSend);
	return Max( 0, Connection->MaxPacket - Connection->OutNum - (INT)sizeof(FBunch) );
	unguard;
}

//
// Handle time passing on this channel.
//
void FChannel::Tick()
{
	guard(FChannel::Tick);
	check(Connection->Channels[ChIndex]==this);

	// Resend any pending packets if we didn't get the appropriate acks.
	for( INT i=0; i<RELIABLE_BUFFER; i++ )
	{
		if( OutRec[i] && Connection->Driver->Time-OutTime[i] > Connection->Driver->AckTimeout )
		{
			debugfSlow( NAME_DevNetTraffic, "Channel %i ack timeout; resending %i...", ChIndex, OutRec[i]->Header.Sequence );
			check(OutRec[i]->Header.ChIndex&CHF_Reliable);
			SendRawBunch( *OutRec[i] );
			OutTime[i] = Connection->Driver->Time;
		}
	}

	// If channel is trying to close, and we've timed out waiting for an ack, 
	// we assume the ack was lost the first time it was sent, and forcefully close the channel.
	if( State==UCHAN_Closing && Connection->Driver->Time - CloseTime > Connection->Driver->ConnectionTimeout )
	{
		debugfSlow( NAME_DevNetTraffic, "Tired of waiting, closing unacked channel %i", ChIndex );
		delete this;
	}
	unguard;
}

//
// Make sure the incoming buffer is in sequence and there are no duplicates.
//
void FChannel::AssertInSequenced()
{
	guard(FChannel::AssertInSequenced);

	// Verify that buffer contains no missing entries.
	for( int i=0; i<RELIABLE_BUFFER; i++ )
		if( InRec[i]==NULL )
			break;
	while( ++i < RELIABLE_BUFFER )
		check(InRec[i]==NULL);

	// Verify that buffer is in order with no duplicates.
	for( i=0; i<RELIABLE_BUFFER-1 && InRec[i] && InRec[i+1]; i++ )
	{
		check((SWORD)(InRec[i+1]->Header.Sequence - InRec[i]->Header.Sequence)!=0);
		check((SWORD)(InRec[i+1]->Header.Sequence - InRec[i]->Header.Sequence)>0);
	}
	unguard;
}

//
// Process a raw, possibly out-of-sequence bunch: either queue it or dispatch it.
//
void FChannel::ReceivedRawBunch( FInBunch& Bunch, UBOOL bFirstTime )
{
	guard(FChannel::ReceivedRawBunch);
	check(Connection->Channels[ChIndex]==this);

	// If this is a duplicate of an existing bunch, or out-of-order, skip it.
	if( (SWORD)(Bunch.Header.Sequence-Connection->LastInRcvd[ChIndex]) <= 0)
	{
		// Ack it to shut up the sender.
		if( (Bunch.Header.ChIndex & CHF_Reliable) && bFirstTime )
			Connection->SendAck( ChIndex, Bunch.Header.Sequence );
		if( Bunch.Header.Sequence+1==Connection->LastInRcvd[ChIndex] )
			debugfSlow( NAME_DevNetTraffic, "      Received duplicate bunch %i/%i/%i", Bunch.Header.Sequence, Connection->LastInRcvd[ChIndex], Connection->LastInRetired[ChIndex] );
		else
			debugfSlow( NAME_DevNetTraffic, "      Received out-of-order bunch %i/%i/%i", Bunch.Header.Sequence, Connection->LastInRcvd[ChIndex], Connection->LastInRetired[ChIndex] );
		return;
	}

	// If this bunch has a dependency on a previous, reliable sequence that 
	// hasn't been received, buffer it until we retire the dependency bunch.
	UBOOL Process = 1;
	if
	(	Bunch.Header.PrevSequence!=SEQ_None
	&&	Bunch.Header.PrevSequence!=Connection->LastInRetired[ChIndex] )
	{
		// If required sequence isn't queued up, speculatively nak it 
		// to encourage the sender to retransmit it quickly.
		for( INT i=0; i<RELIABLE_BUFFER; i++ )
			if( InRec[i] && InRec[i]->Header.Sequence==Bunch.Header.PrevSequence )
				break;
		if( i==RELIABLE_BUFFER )
			Connection->SendNak( ChIndex, Bunch.Header.PrevSequence );

		// Find or make a place for this.
		if( InRec[RELIABLE_BUFFER-1] != NULL )
		{
			debugfSlow( NAME_DevNetTraffic, "      Queue full; discarding %s packet", (Bunch.Header.ChIndex&CHF_Reliable) ? "reliable" : "unreliable" );
			return;
		}
		debugfSlow( NAME_DevNetTraffic, "      Queuing bunch with unreceived dependency" );

		// Find the place for this item, sorted in sequence.
		for( i=0; i<RELIABLE_BUFFER; i++ )
		{
			if( !InRec[i] )
			{
				break;
			}
			else if( Bunch.Header.Sequence == InRec[i]->Header.Sequence)
			{
				Process = 0;
				break;
			}
			else if( (SWORD)(Bunch.Header.Sequence - InRec[i]->Header.Sequence) < 0 )
			{
				break;
			}
		}

		// There was space, so queue it.
		if( Process )
		{
			for( INT j=RELIABLE_BUFFER-1; j>i; j-- )
				InRec[j] = InRec[j-1];
			InRec[i] = Bunch.DuplicateBunch();
			AssertInSequenced();
		}
		Process = 0;
	}

	// If reliable, or we're a client, ack it.
	if( bFirstTime )
		if( (Bunch.Header.ChIndex & CHF_Reliable) || Connection->Driver->ServerConnection )
			Connection->SendAck( ChIndex, Bunch.Header.Sequence );

	// Process incoming.
	if( Process )
	{
		// If reliable, note this packet's retirement.
		Connection->LastInRcvd[ChIndex] = Bunch.Header.Sequence;
		if( Bunch.Header.ChIndex & CHF_Reliable )
			Connection->LastInRetired[ChIndex] = Bunch.Header.Sequence;

		// We have fully received the bunch, so process it.
		if( Bunch.Header.ChIndex & CHF_Close )
		{
			// Handle a close-notify.
			if( !OpenedLocally || ChIndex==0 )
			{
				debugfSlow( NAME_DevNetTraffic, "      Channel %i got close-notify", ChIndex );
				delete this;
			}
			else debugf( NAME_DevNet, "      Owned channel %i got unexpected close-notify", ChIndex );
		}
		else
		{
			// Handle a regular bunch.
			if( State == UCHAN_Open )
				ReceivedBunch( Bunch );

			// If there are any queued-up dependencies ready to release, unleash them in-order now.
			if( InRec[0] && InRec[0]->Header.PrevSequence==Connection->LastInRetired[ChIndex] )
			{
				debugfSlow( NAME_DevNetTraffic, "      Unleashing queued bunches" );
				FInBunch* Bunch = InRec[0];
				for( INT i=1; i<RELIABLE_BUFFER; i++ )
					InRec[i-1] = InRec[i];
				InRec[RELIABLE_BUFFER-1] = NULL;
				AssertInSequenced();
				ReceivedRawBunch( *Bunch, 0 );
				FreePooledBunch( Bunch );
			}
		}
	}
	unguard;
}

//
// Send a close notification.
//
void FChannel::SendClose()
{
	guard(FChannel::SendClose);

	FOutBunch CloseBunch( this, 1 );
	check(!CloseBunch.Overflowed);
	CloseBunch.Header.ChIndex |= (CHF_Reliable | CHF_Close);
	SendBunch( CloseBunch, 0 );

	unguard;
}

//
// Send a raw bunch.
//
void FChannel::SendRawBunch( FOutBunch& Bunch )
{
	guard(FChannel::SendRawBunch);
	check(Connection->Channels[ChIndex]==this);

	// Handle overflow if this data doesn't fit in the connection's packet.
	if( Connection->OutNum + Bunch.Header.GetTotalSize() > Connection->MaxPacket )
	{
		Connection->FlushNet();
		if( Connection->OutNum + Bunch.Header.GetTotalSize() > Connection->MaxPacket )
			appErrorf( "Bunch size overflowed: %i+%i>%i", Bunch.Header.GetTotalSize(), Connection->OutNum, Connection->MaxPacket );
	}

	// Remember position.
	Connection->SentBunchStart = Connection->OutNum;

	// Copy header.
	appMemcpy( Connection->OutData+Connection->OutNum, &Bunch.Header, sizeof(FBunch) );
	Connection->OutNum += sizeof(FBunch);

	// Copy data.
	appMemcpy( Connection->OutData+Connection->OutNum, Bunch.Data, Bunch.Header.DataSize);
	Connection->OutNum += Bunch.Header.DataSize;

	// If absolutely filled now, flush so that MaxSend() never returns zero.
	check(Connection->OutNum<=Connection->MaxPacket);
	if( Connection->OutNum == Connection->MaxPacket )
		Connection->FlushNet();

	unguard;
}

//
// Send a bunch if it's not overflowed, and queue it if it's reliable.
//
UBOOL FChannel::SendBunch( FOutBunch& Bunch, UBOOL Merge )
{
	guard(FChannel::SendBunch);
	check(State==UCHAN_Open);
	check(Connection->Channels[ChIndex]==this);
	//debugf( "Channel %i: Sending bunch", ChIndex );

	// If overflowed, skip.
	if( Bunch.Overflowed )
	{
		// big problem if reliable!!
		debugf( NAME_DevNet, "Discarding overflowed bunch" );
		return 0;
	}

	// Contemplate merging.
	INT BunchIndex = INDEX_NONE;
	if
	(	Merge
	&&	Connection->LastBunchEnd
	&&	Connection->LastBunchEnd==Connection->OutNum 
	&&	Connection->OutNum+Bunch.Header.DataSize<=UNetConnection::MAX_PACKET_SIZE )
	{
		FBunch* OldHeader = (FBunch*)(Connection->OutData+Connection->LastBunchStart);
		if( (OldHeader->ChIndex&CHF_Mask)==(ChIndex&CHF_Mask) )
		{
			// Remove previously sent data from queue and add it into new bunch.
			appMemmove( Bunch.Data + OldHeader->DataSize, Bunch.Data, Bunch.Header.DataSize );
			appMemcpy( Bunch.Data, OldHeader+1, OldHeader->DataSize );
			Bunch.Header.Sequence     = OldHeader->Sequence;
			Bunch.Header.PrevSequence = OldHeader->PrevSequence;
			Bunch.Header.DataSize    += OldHeader->DataSize;
			if( OldHeader->ChIndex & CHF_Reliable )
			{
				Bunch.Header.ChIndex |= CHF_Reliable;
				BunchIndex = Connection->LastOutIndex;
				check(OutRec[BunchIndex]);
			}
			check(Connection->OutNum>Connection->LastBunchStart);
			Connection->OutNum = Connection->LastBunchStart;
		}
		else Merge=0;
	}
	else Merge=0;

	// Find outgoing bunch index.
	if( Bunch.Header.ChIndex & CHF_Reliable )
	{
		// Find spot, which was guaranteed available by FOutBunch constructor.
		if( BunchIndex==INDEX_NONE )
			BunchIndex = ReserveOutgoingIndex( (Bunch.Header.ChIndex&CHF_Close)!=0 );
		check(BunchIndex!=INDEX_NONE);

		// Save a copy of the reliable bunch in case it must be resent.
		OutTime[BunchIndex] = Connection->Driver->Time;
		INT Size = sizeof(FOutBunch) + Bunch.Header.DataSize - sizeof(Bunch.Data);
		check(Size<=sizeof(FOutBunch));
		if( !OutRec[BunchIndex] )
			OutRec[BunchIndex] = (FOutBunch*)AllocPooledBunch();
		appMemcpy( OutRec[BunchIndex], &Bunch, Size );
	}

	// Send the raw bunch.
	SendRawBunch( Bunch );

	// Update channel sequence count.
	if( Bunch.Header.ChIndex & CHF_Reliable )
		Connection->OutReliable[ChIndex] = Bunch.Header.Sequence;
	if( !Merge )
		if( ++Connection->OutSequence[ChIndex] < SEQ_First )
			Connection->OutSequence[ChIndex] = SEQ_First;
	Connection->LastBunchStart = Connection->SentBunchStart;
	Connection->LastBunchEnd   = Connection->OutNum;
	Connection->LastOutIndex   = BunchIndex;

	return 1;
	unguard;
}

//
// Describe the channel.
//
char* FChannel::Describe( char* String256 )
{
	guard(FChannel::Describe);
	appSprintf
	(
		String256,
		"State=%s",
		State==UCHAN_Open ? "open" : State==UCHAN_Closing ? "closing" : "closed"
	);
	return String256;
	unguard;
}

//
// Reserve an outgoing index on this channel.
//
INT FChannel::ReserveOutgoingIndex( UBOOL bClose )
{
	guardSlow(FChannel::ReserveOutgoingIndex);

	// Always save one bunch for the close-notification.
	INT MaxBunch = bClose ? RELIABLE_BUFFER : RELIABLE_BUFFER-1;

	// If bunch buffer is full, can't allocate a bunch.
	for( INT BunchIndex=0; BunchIndex<MaxBunch; BunchIndex++ )
		if( OutRec[BunchIndex] == NULL )
			return BunchIndex;

	return INDEX_NONE;
	unguardSlow;
}

//
// Return whether this channel is ready for sending.
//
UBOOL FChannel::IsNetReady( UBOOL Saturate )
{
	guard(FChannel::IsNetReady);

	// If saturation allowed, ignore queued byte count.
	if( Saturate )
		Connection->QueuedBytes = 0;

	// If connection is saturated and we don't want saturation, we're not ready.
	if( !Connection->IsNetReady() )
		return 0;

	// Ready if there's space available.
	for( INT BunchIndex=0; BunchIndex<RELIABLE_BUFFER-1; BunchIndex++ )
		if( OutRec[BunchIndex] == NULL )
			return 1;

	// Not ready.
	return 0;

	unguard;
}

/*-----------------------------------------------------------------------------
	FControlChannel implementation.
-----------------------------------------------------------------------------*/

//
// Initialize the text channel.
//
FControlChannel::FControlChannel( UNetConnection* InConnection, INT InChannelIndex, INT InOpenedLocally )
:	FChannel( ChannelType, InConnection, InChannelIndex, InOpenedLocally )
{
	guard(FControlChannel::FControlChannel);
	unguard;
}

//
// FControlChannel destructor. 
//
FControlChannel::~FControlChannel()
{
	guard(FControlChannel::~FControlChannel);
	check(Connection);
	check(Connection->Channels[ChIndex]==this);

	unguard;
}

//
// Handle an incoming bunch.
//
void FControlChannel::ReceivedBunch( FInBunch& Bunch )
{
	guard(FControlChannel::ReceivedBunch);
	check(State==UCHAN_Open);
	for( ; ; )
	{
		char Text[UNetConnection::MAX_PACKET_SIZE];
		Bunch.String( Text, sizeof(Text) );
		if( Bunch.Overflowed )
			break;
		Connection->Driver->Notify->NotifyReceivedText( Connection, Text );
	}
	unguard;
}

//
// Text channel FOutputDevice interface.
//
void FControlChannel::WriteBinary( const void* Data, int Length, EName MsgType )
{
	guard(FControlChannel::WriteBinary);

	// Delivery is not guaranteed because NewBunch may fail.!!
	FOutBunch Bunch( this );
	Bunch.Header.ChIndex |= CHF_Reliable;
	if( !Bunch.Overflowed )
	{
		Bunch.String( (char*)Data, UNetConnection::MAX_PACKET_SIZE );
		SendBunch( Bunch, 1 );
	}
	else debugf( NAME_DevNet, "Control channel bunch overflowed" );//!!
	unguard;
}

//
// Describe the text channel.
//
char* FControlChannel::Describe( char* String256 )
{
	guard(FControlChannel::Describe);
	char Extra256[256];
	appSprintf
	(
		String256,
		"Text %s",
		FChannel::Describe(Extra256)
	);
	return String256;
	unguard;
}

IMPLEMENT_CHTYPE(FControlChannel);

/*-----------------------------------------------------------------------------
	FActorChannel.
-----------------------------------------------------------------------------*/

//
// Initialize this actor channel.
//
FActorChannel::FActorChannel( UNetConnection* InConnection, INT InChannelIndex, INT InOpenedLocally )
:	FChannel		( ChannelType, InConnection, InChannelIndex, InOpenedLocally )
,	Level			( Connection->Driver->Notify->NotifyGetLevel() )
,	Actor			( NULL )
,	Recent			( NULL )
,	RelevantTime	( Connection->Driver->Time )
,	LastUpdateTime	( Connection->Driver->Time - Connection->Driver->SpawnPrioritySeconds )
{
	guard(FActorChannel::FActorChannel);
	unguard;
}

//
// Close it.
//
void FActorChannel::Close()
{
	guard(FActorChannel::Close);
	Actor = NULL;
	FChannel::Close();
	unguard;
}

//
// Actor channel destructor.
//
FActorChannel::~FActorChannel()
{
	guard(FActorChannel::~FActorChannel);
	check(Connection);
	check(Connection->Channels[ChIndex]==this);

	// Free the cached version of the actor.
	guard(FreeRecent);
	if( Recent )
		appFree( Recent );
	unguard;

	// If we're the client, destroy this actor.
	guard(DestroyChannelActor);
	if( Connection->Driver->ServerConnection )
	{
		check(!Actor || Actor->IsValid());
		check(Level);
		check(Level->IsValid());
		check(Connection);
		check(Connection->IsValid());
		check(Connection->Driver);
		check(Connection->Driver->IsValid());
		if( Actor )
			Actor->XLevel->DestroyActor( Actor, 1 );
	}
	unguard;

	unguard;
}

//
// Handle receiving a bunch of data on this actor channel.
//
void FActorChannel::ReceivedBunch( FInBunch& Bunch )
{
	guard(FActorChannel::ReceivedBunch);
	check(State==UCHAN_Open);

	// Initialize client if first time through.
	if( Actor == NULL )
	{
		guard(InitialClientActor);

		// Read class.
		UObject* Object;
		Bunch << Object;
		Actor = Cast<AActor>( Object );
		if( Actor==NULL )
		{
			// Transient actor.
			UClass* ActorClass = Cast<UClass>( Object );
#if CHECK_ALL
			check(ActorClass);
			check(ActorClass->IsChildOf(AActor::StaticClass));
#endif
			FVector Location;
			Bunch << Location;
			Actor = Level->SpawnActor( ActorClass, NAME_None, NULL, NULL, Location, FRotator(0,0,0), NULL, 1, 1, 1 );
#if CHECK_ALL
			check(Actor);
			check(Actor->Role<ROLE_Authority);
			check(Actor->RemoteRole==ROLE_Authority);
#endif
		}
		debugfSlow( NAME_DevNetTraffic, "      Net spawn %s:", Actor->GetFullName() );
		unguard;
	}
	debugfSlow( NAME_DevNetTraffic, "      Actor %s:", Actor->GetFullName() );

	// Owned by connection's player?
	guard(SetNetMode);
	Actor->bNetOwner = 0;
	APlayerPawn* Top = Cast<APlayerPawn>( Actor->GetTopOwner() );
	UPlayer* Player = Top ? Top->Player : NULL;

	// Set quickie replication variables.
	if( Level->NetDriver->ServerConnection )
	{
		// We are the client.
		if( Player && Player->IsA( UViewport::StaticClass ) )
			Actor->bNetOwner = 1;
	}
	else
	{
		// We are the server.
		if( Player==Connection )
			Actor->bNetOwner = 1;
	}
	unguard;

	// Handle the data stream.
	guard(HandleStream);
	FName PropertyName;
	Bunch << PropertyName;
	while( !Bunch.Overflowed )
	{
		// Save key properties.
		FVector   SavedLocation = Actor->Location;
		FRotator  SavedRotation = Actor->Rotation;
		AActor*   SavedBase     = Actor->Base;
		DWORD     OldCollision  = Actor->bCollideActors;
		FLOAT	  OldRadius		= Actor->CollisionRadius;
		FLOAT     OldHeight     = Actor->CollisionHeight;
		FPlane    OldSimAnim    = Actor->SimAnim;

		// Receive properties.
		guard(Properties);
		while( !Bunch.Overflowed )
		{
			if( PropertyName == NAME_Role )
				PropertyName = NAME_RemoteRole;
			else if( PropertyName == NAME_RemoteRole )
				PropertyName = NAME_Role;
			FRepLink* Link=NULL;
			for( UClass* RepClass=Actor->GetClass(); RepClass && !Link; RepClass=RepClass->GetSuperClass() )
				for( Link=RepClass->Reps; Link; Link=Link->Next )
					if( Link->Property->GetFName() == PropertyName )
						break;
			if( !Link )
				break;
			UProperty* It = Link->Property;
			if( !Level->NetDriver->ServerConnection && It->RepOffset!=MAXWORD )
			{
				// See if UnrealScript replication condition is met.
				guard(EvalPropertyCondition);
				Exchange(Actor->Role,Actor->RemoteRole);
				FFrame EvalStack( Actor, It->GetOwnerClass(), It->RepOffset, NULL );
				BYTE Buffer[MAX_CONST_SIZE], *Val=Buffer;
				EvalStack.Step( Actor, Val );
				Exchange(Actor->Role,Actor->RemoteRole);

				// Skip if no replication is desired.
				if( !*(DWORD*)Val )
				{
					debugf( NAME_DevNet, "Received unwanted property value %s in %s", *PropertyName, Actor->GetFullName() );
					Bunch.Overflowed = 1;
					break;
				}
				unguard;
			}

			// Receive the property value.
			if( !Bunch.ReceiveProperty( It, (BYTE*)Actor, Recent ) )
			{
				debugf( NAME_DevNet, "Received invalid property value %s", *PropertyName );
				Bunch.Overflowed = 1;
				break;
			}

			// For debugging.
			debugfSlow( NAME_DevNetTraffic, "         %s", *PropertyName );

			// Get next.
			Bunch << PropertyName;
		}
		unguard;

		// Handle changed properties.
		guard(AssimilateChanges);
		#define ExchangeB(A,B) {UBOOL T=A; A=B; B=T;}
		ExchangeB( Actor->bCollideActors, OldCollision );
		Exchange( Actor->CollisionRadius, OldRadius );
		Exchange( Actor->CollisionHeight, OldHeight );
		Exchange( Actor->Location, SavedLocation );
		Exchange( Actor->Rotation, SavedRotation );
		if( Actor->SimAnim != OldSimAnim )
		{
			Actor->AnimFrame = Actor->SimAnim.X * 0.0001;
			Actor->AnimRate = Actor->SimAnim.Y * 0.0001;
			Actor->TweenRate = Actor->SimAnim.Z * 0.001;
			Actor->AnimLast = Actor->SimAnim.W * 0.0001;
			//if ( Actor->IsA(APawn::StaticClass) )
			//	debugf("%s SimAnim %f %f %f %f", Actor->GetName(), Actor->SimAnim.X, Actor->SimAnim.Y, Actor->SimAnim.Z, Actor->SimAnim.W);
			if ( Actor->AnimLast < 0 )
			{
				Actor->AnimLast *= -1;
				Actor->bAnimLoop = 1;
				if ( Actor->IsA(APawn::StaticClass) && (Actor->AnimMinRate < 0.5) )
					Actor->AnimMinRate = 0.5;
			}
			else 
				Actor->bAnimLoop = 0;
		}
		if( Actor->bCollideActors!= OldCollision )
		{
			Actor->SetCollision( OldCollision, Actor->bBlockActors, Actor->bBlockPlayers );
		}
		if( Actor->CollisionRadius!=OldRadius || Actor->CollisionHeight!=OldHeight )
		{
			Actor->SetCollisionSize( OldRadius, OldHeight );
		}
		if( Actor->Location!=SavedLocation )
		{
			//!! FIXME - teleport causes based actors to be lost. OK for network?
			Level->FarMoveActor( Actor, SavedLocation, 0, 1 );
		}
		if( Actor->Rotation!=SavedRotation )
		{
			FCheckResult Hit;
			//!!teleport
			Level->MoveActor( Actor, FVector(0,0,0), SavedRotation, Hit, 0, 0, 0, 1 );
		}
		if( Actor->Base!=SavedBase )
		{
			// Base changed.
			Exchange( Actor->Base, SavedBase );
			Actor->eventBump( SavedBase );
			if( SavedBase )
				SavedBase->eventBump( Actor );
			Actor->SetBase( SavedBase );
		}
		Actor->bJustTeleported=0;
		unguard;

		// Handle procedure calls.
		if( PropertyName!=NAME_None && !Bunch.Overflowed )
		{
			// A remote procedure call.
			guard(RemoteCall);
			FName Message = PropertyName;
			UFunction* Function = Actor->FindFunction( Message );
			if( !Function || !(Function->FunctionFlags & FUNC_Net) )
			{
#if CHECK_ALL
				appErrorf( "No replicated field %s in %s", *PropertyName, Actor->GetFullName() );
#endif
				debugf( NAME_DevNet, "Replicared field not found in %s: %s", Actor->GetClassName(), *Message );
				return;
			}
			UBOOL Ignore=0;
			BYTE ParmMask=0, ParmBit=1;
			Bunch << ParmMask;
			if( !Level->NetDriver->ServerConnection && Function->RepOffset!=MAXWORD )
			{
				// See if UnrealScript replication condition is met.
				guard(EvalRPCCondition);
				Exchange(Actor->Role,Actor->RemoteRole);
				for( UFunction* Test=Function; Test->GetSuperFunction(); Test=Test->GetSuperFunction() );
				FFrame EvalStack( Actor, Test->GetOwnerClass(), Test->RepOffset, NULL );
				BYTE Buffer[MAX_CONST_SIZE], *Val=Buffer;
				EvalStack.Step( Actor, Val );
				Exchange(Actor->Role,Actor->RemoteRole);
				if( !*(DWORD*)Val )
				{
					debugf( NAME_DevNet, "Received unwanted function %s in ", *Message, Actor->GetFullName() );
					Ignore = 1;
				}
				unguard;
			}
			debugfSlow( NAME_DevNetTraffic, "      Received RPC: %s", *Message );
			FMemMark Mark(GMem);
			BYTE* Parms = new(GMem,MEM_Zeroed,Function->ParmsSize)BYTE;
			for( TFieldIterator<UProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It,ParmBit=ParmBit<<1 )
			{
				if( (ParmMask&ParmBit) || !ParmBit )
				{
					if( !Bunch.ReceiveProperty( *It, Parms, NULL ) )
					{
						debugf( NAME_DevNet, "Received invalid property value %s", It->GetName() );
						return;
					}
				}
			}
			if( !Ignore )
				Actor->ProcessEvent( Function, Parms );
			Mark.Pop();
			Bunch << PropertyName;
			unguard;
		}
	}
	unguard;

	// If this is the player's channel and the connection was pending, mark it open.
	if
	(	Actor->XLevel->NetDriver->ServerConnection
	&&	Actor->XLevel->NetDriver->ServerConnection->State==USOCK_Pending
	&&	Actor->bNetOwner
	&&	Actor->IsA(APlayerPawn::StaticClass) )
	{
		// Hook up the Viewport to the new player actor.
		UViewport* Viewport=NULL;
		guard(HandleClientPlayer);
		check(Actor->XLevel->Engine->Client);
		check(Actor->XLevel->Engine->Client->Viewports.Num());
		Viewport = Actor->XLevel->Engine->Client->Viewports(0);
		Viewport->Actor->Player = NULL;
		unguard;

		// Init the new playerpawn.
		guard(InitialPlayerPawn);
		APlayerPawn* Pawn = (APlayerPawn*)Actor;
		Pawn->Role        = ROLE_AutonomousProxy;
		Pawn->ShowFlags   = SHOW_Backdrop | SHOW_Actors | SHOW_Menu | SHOW_PlayerCtrl | SHOW_RealTime;
		Pawn->RendMap	  = REN_DynLight;
		Pawn->SetPlayer( Viewport );
		Actor->XLevel->Engine->Client->Viewports(0)->Input->ResetInput();
		Actor->Level->LevelAction = LEVACT_None;
		unguard;

		// Mark this connection as open.
		guard(OpenConnection);
		check(Level->NetDriver->ServerConnection->State==USOCK_Pending);
		Level->NetDriver->ServerConnection->State = USOCK_Open;
		unguard;
	}

	unguardf(( "(Actor %s)", Actor ? Actor->GetName() : "null"));
}

//
// Replicate this channel's actor differences.
//
void FActorChannel::ReplicateActor( UBOOL FullReplication )
{
	guard(FActorChannel::ReplicateActor);
	check(Actor!=NULL);
	check(State==UCHAN_Open);
	Actor->OtherTag++;

	// Make sure previous version of the actor is around.
	if( Recent )
	{
		// Not sending initial packet.
		Actor->bNetInitial = 0;
		if( !FullReplication )
			return;
	}
	else
	{
		// Start with default version of the actor.
		INT Size = Actor->GetClass()->Defaults.Num();
		Recent   = (BYTE*)appMalloc( Size, "FActorChannelRecent" );
		appMemcpy( Recent, &Actor->GetClass()->Defaults(0), Size );
		Actor->bNetInitial = 1;
	}

	// Create an outgoing bunch, and skip this actor if the channel is saturated.
	FOutBunch Bunch( this );
	if( Bunch.Overflowed )
	{
		check(!Actor->bNetInitial);
		return;
	}
	if( Actor->bNetInitial )
	{
		Bunch.Header.ChIndex |= CHF_Reliable;
	}

	// Owned by connection's player?
	Actor->bNetOwner = 0;
	for( AActor* Top=Actor; Top->Owner; Top=Top->Owner );
	UPlayer* Player = Top->IsA(APlayerPawn::StaticClass) ? ((APlayerPawn*)Top)->Player : NULL;

	// Set quickie replication variables.
	if( Level->NetDriver->ServerConnection )
	{
		// We are the client.
		if( Player && Player->IsA( UViewport::StaticClass ) )
			Actor->bNetOwner = 1;
	}
	else
	{
		// We are the server.
		if( Player && Player->IsA( UNetConnection::StaticClass ) && ((UNetConnection*)Player)==Connection )
			Actor->bNetOwner = 1;
	}

	// If initial, send init data.
	if( Actor->bNetInitial && OpenedLocally )
	{
		// Send class and name.
		if( Actor->bStatic || Actor->bNoDelete )
		{
			// Persitent actor.
			Bunch << Actor;
		}
		else
		{
			// Transient actor.
			UClass* ActorClass = Actor->GetClass();
			Bunch << ActorClass << Actor->Location;
			((AActor*)Recent)->Location = Actor->Location;
		}
	}

	// Save out the actor's RemoteRole, and downgrade it if necessary.
	BYTE ActualRemoteRole=Actor->RemoteRole;
	if( Actor->RemoteRole==ROLE_AutonomousProxy && !Actor->bNetOwner )
		Actor->RemoteRole=ROLE_SimulatedProxy;
	Actor->bSimulatedPawn = Actor->IsA(APawn::StaticClass) && (Actor->RemoteRole == ROLE_SimulatedProxy);

	// Replicate all applicable properties.
	for( UClass* RepClass=Actor->GetClass(); RepClass; RepClass=RepClass->GetSuperClass() )
	{
		for( FRepLink* Link=RepClass->Reps; Link; Link=Link->Next )
		{
			FRepLink*  Condition = Link->Condition;
			UProperty* It        = Link->Property;
			INT        Index     = 0;
			if
			(	Condition->LastObject != Actor
			||	Condition->LastStamp  != Actor->OtherTag
			||	Condition->LastResult )
			{
				for( Index=0; Index<It->ArrayDim; Index++ )
				{
					UBOOL RandomForce=0;
					if
					(	!It->Matches(Actor,Recent,Index)
					||	(It->PropertyFlags & CPF_NetAlways)
					||	(RandomForce=(appFrand()<1.0/1000.0))!=0 )
					{
						if
						(	Condition->LastObject!=Actor
						||	Condition->LastStamp!=Actor->OtherTag )
						{
							// Evaluate replication condition.
							FFrame EvalStack( Actor, It->GetOwnerClass(), It->RepOffset, NULL );
							BYTE Buffer[MAX_CONST_SIZE], *Val=Buffer;
							EvalStack.Step( Actor, Val );
							Condition->LastObject = Actor;
							Condition->LastStamp  = Actor->OtherTag;
							Condition->LastResult = *(DWORD*)Val!=0;
						}
						if( Condition->LastResult )
						{
							if( (It->PropertyFlags & CPF_NetReliable) && !RandomForce )
							{
								Bunch.Header.ChIndex |= CHF_Reliable;
							}
							if( Bunch.SendProperty( It, Index, (BYTE*)Actor, Recent, 1 ) )
								goto FilledUp;
							Actor->XLevel->NumReps++;
						}
					}
				}
			}
		}
	}
	FilledUp:
	check(!Bunch.Overflowed);

	// If not overflowed, send and mark as updated.
	if( !Bunch.Overflowed )
	{
		if( Bunch.Header.DataSize )
			if( SendBunch( Bunch, 1 ) )
				LastUpdateTime = Connection->Driver->Time;
	}
	else check(!Actor->bNetInitial);

	// Reset temporary net info.
	Actor->bNetOwner  = 0;
	Actor->RemoteRole = ActualRemoteRole;

	unguard;
}

//
// Describe the actor channel.
//
char* FActorChannel::Describe( char* String256 )
{
	guard(FActorChannel::Describe);
	char Extra256[256];
	if( State != UCHAN_Open ) appSprintf
	(
		String256,
		"Actor=NotOpen %s",
		FChannel::Describe(Extra256)
	);
	if( !Actor ) appSprintf
	(
		String256,
		"Actor=None %s ",
		FChannel::Describe(Extra256)
	);
	else appSprintf
	(
		String256,
		"Actor=%s (Role=%i RemoteRole=%i) %s",
		Actor->GetFullName(),
		Actor->Role,
		Actor->RemoteRole,
		FChannel::Describe(Extra256)
	);
	return String256;
	unguard;
}

IMPLEMENT_CHTYPE(FActorChannel);

/*-----------------------------------------------------------------------------
	FFileChannel implementation.
-----------------------------------------------------------------------------*/

FFileChannel::FFileChannel( UNetConnection* InConnection, INT InChannelIndex, INT InOpenedLocally )
:	FChannel		( ChannelType, InConnection, InChannelIndex, InOpenedLocally )
,	File			( NULL )
,	Transfered		( 0 )
,	PackageIndex	( INDEX_NONE )
{
	guard(FFileChannel::FFileChannel);

	// Strings.
	appStrcpy( Filename, "" );
	appStrcpy( PrettyName, "" );
	appStrcpy( Error, LocalizeError("Unknown","Core") );

	unguard;
}
void FFileChannel::ReceivedBunch( FInBunch& Bunch )
{
	guard(FFileChannel::ReceivedBunch);
	check(State==UCHAN_Open);
	if( OpenedLocally )
	{
		// Receiving a file sent from the other side.
		FPackageInfo& Info = Connection->Driver->Map( PackageIndex );
		if( Bunch.Header.DataSize>0 )
		{
			// Receiving spooled file data.
			if( Transfered==0 )
			{
				// Open temporary file initially.
				debugf( NAME_DevNet, "Receiving package '%s'", Info.Parent->GetName() );
				appMkdir( GSys->CachePath );
				appCreateTempFilename( GSys->CachePath, Filename );
				File = appFopen( Filename, "wb" );
			}

			// Receive.
			if( !File )
			{
				// Opening file failed.
				appSprintf( Error, LocalizeError("NetOpen") );
				Close();
			}
			else if( appFwrite( Bunch.Data, 1, Bunch.Header.DataSize, File ) != Bunch.Header.DataSize )
			{
				// Write failed.
				appSprintf( Error, LocalizeError("NetWrite"), Filename );
				Close();
			}
			else
			{
				// Successful.
				Transfered += Bunch.Header.DataSize;
				char Msg1[256], Msg2[256];
				appSprintf( Msg1, "Receiving '%s' (F10 Cancels)", PrettyName );
				appSprintf( Msg2, "Size %iK, Complete %3.1f%%", Info.FileSize/1024, 100.f*Transfered/Info.FileSize );
				Connection->Driver->Notify->NotifyProgress( Msg1, Msg2, 4.0 );
			}
		}
		else
		{
			// Finished transfer.
			char Dest[256], GuidString[64];
			appSprintf( Dest, "%s\\%s.uxx", GSys->CachePath, Info.Guid.String(GuidString) );
			if( Transfered==0 )
			{
				appSprintf( Error, LocalizeError("NetRefused"), Info.Parent->GetName() );
			}
			else if( appFclose(File)!=0 )
			{
				appSprintf( Error, LocalizeError("NetClose") );
			}
			else if( appFSize(Filename)!=Info.FileSize )
			{
				appSprintf( Error, LocalizeError("NetSize") );
			}
			else if( !appMoveFile( Filename, Dest ) )
			{
				appSprintf( Error, LocalizeError("NetMove") );
			}
			else
			{
				// Success.
				*Error = 0;
				char Msg[256];
				appSprintf( Msg, "Received '%s'", PrettyName );
				Connection->Driver->Notify->NotifyProgress( "Success", Msg, 4.0 );
				Connection->Driver->Notify->NotifyReceivedFile( Connection, PackageIndex, Error );
			}
			Close();
		}
	}
	else
	{
		// Request to send a file.
		FGuid Guid;
		Bunch << Guid;
		if( !Bunch.Overflowed )
		{
			for( int i=0; i<Connection->Driver->Map.Num(); i++ )
			{
				FPackageInfo& Info = Connection->Driver->Map(i);
				if( Info.Guid==Guid && Info.URL[0] )
				{
					if( Connection->Driver->Notify->NotifySendingFile( Connection, Guid ) )
					{
						check(Info.Linker);
						File = appFopen( Info.URL, "rb" );
						if( File )
						{
							// Accepted! Now initiate file sending.
							debugf( NAME_DevNet, LocalizeProgress("NetSend"), Info.Parent->GetName() );
							PackageIndex = i;
							return;
						}
					}
				}
			}
		}

		// Illegal request; refuse it and send a zero-length bunch.
		debugf( NAME_DevNet, LocalizeError("NetInvalid") );
		FOutBunch Bunch( this );
		Bunch.Header.ChIndex |= CHF_Reliable;
		SendBunch( Bunch, 0 );
	}
	unguard;
}
void FFileChannel::Tick()
{
	guard(FFileChannel::Tick);
	FChannel::Tick();
	FPackageInfo& Info = Connection->Driver->Map( PackageIndex );
	while( File && !OpenedLocally && IsNetReady(1) )
	{
		// Sending.
		//debugf( NAME_DevNet, LocalizeProgress("NetSending"), Info.Parent->GetName(), Transfered, Info.FileSize );
		FOutBunch Bunch( this );
		BYTE Buffer[UNetConnection::MAX_PACKET_SIZE];
		INT Size = MaxSend();
		INT Read = appFread( Buffer, 1, Size, File );
		Bunch.Serialize( Buffer, Read );
		Bunch.Header.ChIndex |= CHF_Reliable;
		SendBunch( Bunch, 0 );
		check(!Bunch.Overflowed);
		Transfered += Read;
		if( Read == 0 )
		{
			// Finished.
			appFclose( File );
			File = NULL;
		}
	}
	unguard;
}
FFileChannel::~FFileChannel()
{
	guard(FFileChannel::~FFileChannel);
	check(Connection);
	check(Connection->Channels[ChIndex]==this);
	check(Connection->Driver->Map.IsValidIndex(PackageIndex));

	// Cleanup the file.
	if( File )
	{
		appFclose( File );
		if( OpenedLocally && Error[0] )
			appUnlink( Filename );
	}

	// Notify that the receive is complete.
	if( OpenedLocally && *Error && State!=USOCK_Closed )
		Connection->Driver->Notify->NotifyReceivedFile( Connection, PackageIndex, Error );

	unguard;
}
char* FFileChannel::Describe( char* String256 )
{
	guard(FFileChannel::Describe);
	FPackageInfo& Info = Connection->Driver->Map( PackageIndex );
	char Extra256[256];
	appSprintf
	(
		String256,
		"File='%s', %s=%i/%i %s",
		Filename,
		OpenedLocally ? "Received" : "Sent",
		Transfered,
		Info.FileSize,
		FChannel::Describe( Extra256 )
	);
	return String256;
	unguard;
}

IMPLEMENT_CHTYPE(FFileChannel);

/*-----------------------------------------------------------------------------
	Channel type registry.
-----------------------------------------------------------------------------*/

// Channel tables.
ENGINE_API FChannel* (*GChannelConstructors[CHTYPE_MAX])( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally );

//
// Global channel registrar.
//
BYTE GAutoRegisterChannel( INT ChType, FChannel* (*Constructor)( UNetConnection* InConnection, INT InChIndex, UBOOL InOpenedLocally ) )
{
	static int Initialized = 0;
	if( !Initialized )
	{
		Initialized = 1;
		for( int i=0; i<CHTYPE_MAX; i++ )	
			GChannelConstructors[i] = NULL;
	}
	GChannelConstructors[ChType] = Constructor;
	return 0;
}

//
// Return whether a channel type is recognized.
//
UBOOL GIsKnownChannelType( INT Type )
{
	guard(IsKnownChannelType);
	return Type>=0 && Type<CHTYPE_MAX && GChannelConstructors[Type];
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

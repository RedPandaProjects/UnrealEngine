/*=============================================================================
	UnLevTic.cpp: Level timer tick function
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"

/*-----------------------------------------------------------------------------
	Helper classes.
-----------------------------------------------------------------------------*/

//
// Priority sortable list for appSort.
//
struct FActorPriority
{
	AActor*			Actor;		// Actor.
	FActorChannel*	Channel;	// Actor channel.
	FLOAT			Priority;	// Update priority, higher = more important.
	FActorPriority()
	{}
	FActorPriority( UNetConnection* InConnection, AActor* InActor )
	{
		Actor   = InActor;
		Channel = InConnection->GetActorChannel( Actor );
		if( Channel )
		{
			// Priority of updating an existing actor.
			Priority = Actor->NetPriority * (InConnection->Driver->Time - Channel->LastUpdateTime);
		}
		else
		{
			// Priority of spawning a new actor = high.
			Priority = Actor->NetPriority * InConnection->Driver->SpawnPrioritySeconds;
		}
		if( InActor->bNetOptional )
		{
			// Update after all other actors.
			Priority -= 100000.0;
		}
	}
	friend inline INT Compare( const FActorPriority& A, const FActorPriority& B )
	{
		return B.Priority - A.Priority;
	}
};

/*-----------------------------------------------------------------------------
	Tick a single actor.
-----------------------------------------------------------------------------*/

UBOOL AActor::Tick( FLOAT DeltaSeconds, ELevelTick TickType )
{
	guard(AActor::Tick);

	// Ignore actors in stasis
	if ( bStasis 
		&& (bForceStasis || (Physics==PHYS_None) || (Physics == PHYS_Rotating))
		&& (XLevel->TimeSeconds - XLevel->Model->Nodes->Zones[Region.ZoneNumber].LastRenderTime > 5)
		&& (Level->NetMode == NM_Standalone) )
		return 1;

	// Handle owner-first updating.
	if( Owner && (INT)Owner->bTicked!=XLevel->Ticked )
	{
		XLevel->NewlySpawned = new(GDynMem)FActorLink(this,XLevel->NewlySpawned);
		return 0;
	}
	bTicked = XLevel->Ticked;

	APawn* Pawn = NULL;
	if( bIsPawn )
		Pawn = Cast<APawn>(this);

	INT bSimulatedPawn = ( Pawn && (Role == ROLE_SimulatedProxy) );

	// Update all animation, including multiple passes if necessary.
	INT Iterations = 0;
	FLOAT Seconds = DeltaSeconds;
	//if ( bSimulatedPawn )
	//	debugf("Animation %s frame %f rate %f tween %f",*AnimSequence,AnimFrame, AnimRate, TweenRate);
	while
	(	IsAnimating()
	//&&	(Role>=ROLE_SimulatedProxy)
	&&	(Seconds>0.0)
	&&	(++Iterations <= 4) )
	{

		// Remember the old frame.
		FLOAT OldAnimFrame = AnimFrame;

		// Update animation, and possibly overflow it.
		if( AnimFrame >= 0.0 )
		{
			// Update regular or velocity-scaled animation.
			if( AnimRate >= 0.0 )
				AnimFrame += AnimRate * Seconds;
			else
				AnimFrame += ::Max( AnimMinRate, Velocity.Size() * -AnimRate ) * Seconds;

			// Handle all animation sequence notifys.
			if( bAnimNotify && Mesh )
			{
				const FMeshAnimSeq* Seq = Mesh->GetAnimSeq( AnimSequence );
				if( Seq )
				{
					FLOAT BestElapsedFrames = 100000.0;
					const FMeshAnimNotify* BestNotify = NULL;
					for( INT i=0; i<Seq->Notifys.Num(); i++ )
					{
						const FMeshAnimNotify& Notify = Seq->Notifys(i);
						if( OldAnimFrame<Notify.Time && AnimFrame>=Notify.Time )
						{
							FLOAT ElapsedFrames = Notify.Time - OldAnimFrame;
							if( BestNotify==NULL || ElapsedFrames<BestElapsedFrames )
							{
								BestElapsedFrames = ElapsedFrames;
								BestNotify        = &Notify;
							}
						}
					}
					if( BestNotify )
					{
						Seconds   = Seconds * (AnimFrame - BestNotify->Time) / (AnimFrame - OldAnimFrame);
						AnimFrame = BestNotify->Time;
						UFunction* Function = FindFunction( BestNotify->Function );
						if( Function )
							ProcessEvent( Function, NULL );
						continue;
					}
				}
			}

			// Handle end of animation sequence.
			if( AnimFrame<AnimLast )
			{
				// We have finished the animation updating for this tick.
				break;
			}
			else if( bAnimLoop )
			{
				if( AnimFrame < 1.0 )
				{
					// Still looping.
					Seconds = 0.0;
				}
				else
				{
					// Just passed end, so loop it.
					Seconds = Seconds * (AnimFrame - 1.0) / (AnimFrame - OldAnimFrame);
					AnimFrame = 0.0;
				}
				if( OldAnimFrame < AnimLast )
				{
					if( GetMainFrame()->LatentAction == EPOLL_FinishAnim )
						bAnimFinished = 1;
					if ( !bSimulatedPawn )
						eventAnimEnd();
				}
			}
			else 
			{
				// Just passed end-minus-one frame.
				Seconds = Seconds * (AnimFrame - AnimLast) / (AnimFrame - OldAnimFrame);
				AnimFrame	 = AnimLast;
				bAnimFinished = 1;
				AnimRate      = 0.0;
				if ( !bSimulatedPawn )
					eventAnimEnd();
				
				if ( (RemoteRole < ROLE_SimulatedProxy) && !IsA(AWeapon::StaticClass) )
				{
					SimAnim.X = 10000 * AnimFrame;
					SimAnim.Y = 10000 * AnimRate;
				}
			}
		}
		else
		{
			// Update tweening.
			AnimFrame += TweenRate * Seconds;
			if( AnimFrame >= 0.0 )
			{
				// Finished tweening.
				Seconds          = Seconds * (AnimFrame-0) / (AnimFrame - OldAnimFrame);
				AnimFrame = 0.0;
				if( AnimRate == 0.0 )
				{
					bAnimFinished = 1;
					if ( !bSimulatedPawn )
						eventAnimEnd();
				}
			}
			else
			{
				// Finished tweening.
				break;
			}
		}
	}

	// This actor is tickable.
	if ( bSimulatedPawn )
		//simulated pawns just predict location, no script execution
		moveSmooth(Velocity * DeltaSeconds);
	else if ( RemoteRole == ROLE_AutonomousProxy ) 
	{
		if ( Role == ROLE_Authority )
		{
			// server handles timers for autonomous proxy
			if( (TimerRate>0.0) && (TimerCounter+=DeltaSeconds)>=TimerRate )
			{
				// Normalize the timer count.
				INT TimerTicksPassed = 1;
				if( TimerRate > 0.0 )
				{
					TimerTicksPassed     = (int)(TimerCounter/TimerRate);
					TimerCounter -= TimerRate * TimerTicksPassed;
					if( TimerTicksPassed && !bTimerLoop )
					{
						// Only want a one-shot timer message.
						TimerTicksPassed = 1;
						TimerRate = 0.0;
					}
				}

				// Call timer routine with count of timer events that have passed.
				eventTimer();
			}
		}
	}
	else if( Role>=ROLE_SimulatedProxy )
	{
		APlayerPawn* PlayerPawn = NULL;
		if ( Pawn )
			PlayerPawn = Cast<APlayerPawn>(this);
		if( !PlayerPawn || !PlayerPawn->Player )
		{
			// Non-player update.
			if( TickType==LEVELTICK_ViewportsOnly )
				return 1;

			// Tick the nonplayer.
			if ( IsProbing(NAME_Tick) )
				eventTick(DeltaSeconds);
		}
		else
		{
			// Player update.
			if( PlayerPawn->IsA(ACamera::StaticClass) && !(PlayerPawn->ShowFlags & SHOW_PlayerCtrl) )
				return 1;

			// Process PlayerTick with input.
			PlayerPawn->Player->ReadInput( DeltaSeconds );
			PlayerPawn->eventPlayerInput( DeltaSeconds );
			PlayerPawn->eventPlayerTick( DeltaSeconds );
			PlayerPawn->Player->ReadInput( 0.0 );
		}

		// Update the actor's script state code.
		ProcessState( DeltaSeconds );

		// Update timers.
		if( TimerRate>0.0 && (TimerCounter+=DeltaSeconds)>=TimerRate )
		{
			// Normalize the timer count.
			INT TimerTicksPassed = 1;
			if( TimerRate > 0.0 )
			{
				TimerTicksPassed     = (int)(TimerCounter/TimerRate);
				TimerCounter -= TimerRate * TimerTicksPassed;
				if( TimerTicksPassed && !bTimerLoop )
				{
					// Only want a one-shot timer message.
					TimerTicksPassed = 1;
					TimerRate = 0.0;
				}
			}

			// Call timer routine with count of timer events that have passed.
			eventTimer();
		}

		// Update LifeSpan.
		if( LifeSpan!=0.f )
		{
			LifeSpan -= DeltaSeconds;
			if( LifeSpan <= 0.0001 )
			{
				// Actor's LifeSpan expired.
				eventExpired();
				XLevel->DestroyActor( this );
				return 1;
			}
		}

		// Perform physics.
		if( (Physics!=PHYS_None) && (Role!=ROLE_AutonomousProxy) )
			performPhysics( DeltaSeconds );

		if ( (Role == ROLE_AutonomousProxy) 
			&& Base && Base->IsA(AMover::StaticClass)
			&& (Physics == PHYS_Walking) )
		{
			AActor* OldBase = Base;
			XLevel->FarMoveActor( Base, Base->Location + Base->Velocity * DeltaSeconds, 0, 1 );
			moveSmooth(OldBase->Velocity * DeltaSeconds);
			SetBase(OldBase);
		}
	}

	// Update eyeheight and send visibility updates
	// with PVS, monsters look for other monsters, rather than sending msgs
	// Also sends PainTimer messages if PainTime
	if( Pawn )
	{
		if ( Pawn->bIsPlayer && (Role >= ROLE_AutonomousProxy) )
			Pawn->eventUpdateEyeHeight(DeltaSeconds);

		if ( (Role == ROLE_Authority) && (TickType==LEVELTICK_All) )
		{
			if( Pawn->SightCounter < 0.0 )
				Pawn->SightCounter += 0.2;

			Pawn->SightCounter = Pawn->SightCounter - DeltaSeconds; 
			if( Pawn->bIsPlayer && !Pawn->bHidden )
				Pawn->ShowSelf();

			if( (Pawn->SightCounter < 0.0) && Pawn->IsProbing(NAME_EnemyNotVisible) )
			{
				Pawn->CheckEnemyVisible();
				Pawn->SightCounter = 0.1;
			}

			if( Pawn->PainTime > 0.0 )
			{
				Pawn->PainTime -= DeltaSeconds;
				if (Pawn->PainTime < 0.001)
				{
					Pawn->PainTime = 0.0;
					Pawn->eventPainTimer();
				}
			}

			if( Pawn->SpeechTime > 0.0 )
			{
				Pawn->SpeechTime -= DeltaSeconds;
				if (Pawn->SpeechTime < 0.001)
				{
					Pawn->SpeechTime = 0.0;
					Pawn->eventSpeechTimer();
				}
			}
		}
	}
	return 1;
	unguard;
}

/*-----------------------------------------------------------------------------
	Network client tick.
-----------------------------------------------------------------------------*/

void ULevel::TickNetClient( FLOAT DeltaSeconds )
{
	guard(ULevel::TickNetClient);
	clock(NetTickCycles);
	if( NetDriver->ServerConnection->State==USOCK_Open )
	{
		for( FTypedChannelIterator<FActorChannel> It(NetDriver->ServerConnection); It; ++It )
		{
			guard(UpdateLocalActors);
			check(*It);
			check(It.GetIndex()>=0);
			check(It.GetIndex()<UNetConnection::MAX_CHANNELS);
			check(It->ChType==CHTYPE_Actor);
			if( It->State==UCHAN_Open && It->Actor )
				check(GetActorIndex(It->Actor)!=INDEX_NONE);
			if( It->State==UCHAN_Open && It->Actor && It->Actor->IsA(APlayerPawn::StaticClass) )
			{
				guard(CheckPawn);
				FActorChannel* ActorChannel = *It;
				APlayerPawn* Pawn = (APlayerPawn*)ActorChannel->Actor;
				if( Pawn->Player )
					ActorChannel->ReplicateActor( 1 );
				unguard;
			}
			unguard;
		}
		NetDriver->ServerConnection->FlushNet( NetDriver->DuplicateClientMoves );
	}
	else if( NetDriver->ServerConnection->State==USOCK_Closed )
	{
		// Server disconnected.
		Engine->SetClientTravel(NULL,"?failed",1,0,TRAVEL_Absolute);
	}
	unclock(NetTickCycles);
	unguard;
}

/*-----------------------------------------------------------------------------
	Network server ticking individual client.
-----------------------------------------------------------------------------*/

INT ULevel::ServerTickClient( UNetConnection* Connection, FLOAT DeltaSeconds )
{
	guard(ULevel::ServerTickClient);
	check(Connection->State==USOCK_Pending || Connection->State==USOCK_Open || Connection->State==USOCK_Closed);
	INT Updated=0;

	// Handle closed channel.
	if( Connection->State==USOCK_Closed )
	{
		debugf( NAME_DevNet, "Destroying %s because connection closed", Connection->GetName() );
		delete Connection;
		return 0;
	}

	// Handle not ready channel.
	if
	(	!Connection->Actor
	||	!Connection->IsNetReady()
	||	Connection->State!=USOCK_Open )
		return 0;

	// Get list of visible/relevant actors.
	AActor* Relevant[256];
	INT NumRelevant = GetRelevantActors( Connection->Actor, Relevant, ARRAY_COUNT(Relevant) );

	// If an actor's relevence has timed out, delete its channel; otherwise
	// treat it as relevant for now.
	for( FTypedChannelIterator<FActorChannel> It(Connection); It; ++It )
	{
		AActor* Actor=It->Actor;
		if( It->State==UCHAN_Open && Actor )
		{
			if( Actor->NetTag==NetTag )
			{
				// This actor is relevant, so update the channel.
				It->RelevantTime = NetDriver->Time;
			}
			else if
			(	(Actor->Role==ROLE_SimulatedProxy)
			?	(NetDriver->Time-It->RelevantTime<NetDriver->SimulatedProxyTimeout)
			:	(NetDriver->Time-It->RelevantTime<NetDriver->DumbProxyTimeout) )
			{
				// This actor's relevence hasn't timed out yet.
				if( NumRelevant<ARRAY_COUNT(Relevant) )
					Relevant[NumRelevant++] = Actor;
			}
			else
			{
				// Relevence has timed out, so destroy the channel.
				check(It->OpenedLocally);
				check(Actor!=Connection->Actor);
				debugfSlow( NAME_DevNetTraffic, "Irrelevant %s", Actor->GetFullName() );
				It->Close();
			}
		}
	}

	// Make priority-sorted list.
	FMemMark Mark(GMem);
	FActorPriority* PriorityActors = new(GMem,NumRelevant)FActorPriority;
	for( INT j=0; j<NumRelevant; j++ )
		PriorityActors[j] = FActorPriority( Connection, Relevant[j] );
	appSort( PriorityActors, NumRelevant );

	// Update all relevant actors in sorted order.
	for( j=0; j<NumRelevant && Connection->IsNetReady(); j++ )
	{
		// Find or create the channel for this actor.
		//debugf("%i...%f...%f",j,PriorityActors[j].Priority,PriorityActors[j].Channel ? PriorityActors[j].Channel->LastUpdateTime:0);
		FActorChannel* Channel = PriorityActors[j].Channel;
		if( !Channel && NetDriver->Map.ObjectToIndex(PriorityActors[j].Actor->GetClass())!=INDEX_NONE )
		{
			// Create a new channel for this actor.
			Channel = (FActorChannel *)Connection->CreateChannel( CHTYPE_Actor, 1 );
			Channel->Actor = PriorityActors[j].Actor;
		}

		// Send updates to the remote player.
		if( Channel )
		{
			check(Channel->State==UCHAN_Open);
			if( Channel->IsNetReady(0) )
			{
				Channel->ReplicateActor( 1 );
				if( Connection->OutNum > UNetConnection::IDEAL_PACKET_SIZE )
					Connection->FlushNet();
				Updated++;
			}
		}
	}
	Mark.Pop();

	return Updated;
	unguard;
}

/*-----------------------------------------------------------------------------
	Network server tick.
-----------------------------------------------------------------------------*/

void ULevel::TickNetServer( FLOAT DeltaSeconds )
{
	guard(ULevel::TickNetServer);

	// Update all clients.
	clock(NetTickCycles);
	INT Updated=0;
	for( INT i=0; i<NetDriver->Connections.Num(); i++ )
		Updated += ServerTickClient( NetDriver->Connections(i), DeltaSeconds );
	unclock(NetTickCycles);

	// Stats.
	if( Updated ) for( i=0; i<NetDriver->Connections.Num(); i++ )
	{
		UNetConnection* Connection = NetDriver->Connections(i);
		if
		(	Connection->Actor
		&&	Connection->State==USOCK_Open
		&&	Connection->Actor->bExtra0 )
		{
			// Send stats.
			char Stats[256];
			INT NumActors=0;
			for( INT i=0; i<Num(); i++ )
				NumActors += Actors(i)!=NULL;
			appSprintf
			(
				Stats,
				"cli=%i act=%03.1f (%i) see=%03.1f net=%03.1f pv/c=%i rep/c=%i",
				NetDriver->Connections.Num(),
				GSecondsPerCycle*1000 * ActorTickCycles,
				NumActors,
				GSecondsPerCycle*1000 * GetRelevantCycles,
				GSecondsPerCycle*1000 * (NetTickCycles - GetRelevantCycles),
				NumPV/NetDriver->Connections.Num(),
				NumReps/NetDriver->Connections.Num()
			);
			Connection->Actor->eventClientMessage(Stats);
		}
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Main level timer tick handler.
-----------------------------------------------------------------------------*/

//
// Update the level after a variable amount of time, DeltaSeconds, has passed.
// All child actors are ticked after their owners have been ticked.
//
void ULevel::Tick( ELevelTick TickType, FLOAT DeltaSeconds )
{
	guard(ULevel::Tick);
	InitStats();
	FMemMark Mark(GMem);
	FMemMark DynMark(GDynMem);
	GInitRunaway();
	InTick=1;

	// Update the net code and fetch all incoming packets.
	if( NetDriver )
	{
		NetDriver->Tick();
		if( NetDriver->ServerConnection )
			TickNetClient( DeltaSeconds );
	}

	// Update collision.
	if( Hash )
		Hash->Tick();

	// Update time.
	ALevelInfo* Info = GetLevelInfo();
	DeltaSeconds *= Info->TimeDilation;
	TimeSeconds += DeltaSeconds;
	Info->TimeSeconds = TimeSeconds;
	appSystemTime( Info->Year, Info->Month, Info->DayOfWeek, Info->Day, Info->Hour, Info->Minute, Info->Second, Info->Millisecond );
	if( Info->bPlayersOnly )
		TickType = LEVELTICK_ViewportsOnly;

	// Clamp time between 200 fps and 2.5 fps.
	DeltaSeconds = Clamp(DeltaSeconds,0.005f,0.40f);

	// If caller wants time update only, or we are paused, skip the rest.
	if
	(	(TickType!=LEVELTICK_TimeOnly)
	&&	(!Info->Pauser[0])
	&&	(!NetDriver || !NetDriver->ServerConnection || NetDriver->ServerConnection->State==USOCK_Open) )
	{
		// Tick all actors, owners before owned.
		clock(ActorTickCycles);
		NewlySpawned=NULL;
		INT Updated=0;
		for( INT iActor=iFirstDynamicActor; iActor<Num(); iActor++ )
			if( Actors(iActor) )
				Updated += Actors(iActor)->Tick(DeltaSeconds,TickType);
		while( NewlySpawned && Updated )
		{
			FActorLink* Link=NewlySpawned;
			NewlySpawned=NULL;
			Updated=0;
			for( Link; Link; Link=Link->Next )
				Updated += Link->Actor->Tick( DeltaSeconds, TickType );
		}
	}
	else if( Info->Pauser[0] )
	{
		// Absorb input if paused.
		for( INT iActor=iFirstDynamicActor; iActor<Num(); iActor++ )
		{
			APlayerPawn* PlayerPawn=Cast<APlayerPawn>(Actors(iActor));
			if( PlayerPawn && PlayerPawn->Player )
			{
				PlayerPawn->Player->ReadInput( DeltaSeconds );
				PlayerPawn->eventPlayerInput( DeltaSeconds );
				for( TFieldIterator<UFloatProperty> It(PlayerPawn->GetClass()); It; ++It )
					if( It->PropertyFlags & CPF_Input )
						*(FLOAT*)((BYTE*)PlayerPawn + It->Offset) = 0.f;
			}
			else if( Actors(iActor) && Actors(iActor)->bAlwaysTick )
				Actors(iActor)->Tick(DeltaSeconds,TickType);
		}
	}
	unclock(ActorTickCycles);

	// Update net server.
	if( NetDriver && !NetDriver->ServerConnection )
		TickNetServer( DeltaSeconds );

	// Finish up.
	Ticked = !Ticked;
	InTick = 0;
	Mark.Pop();
	DynMark.Pop();
	CleanupDestroyed( 0 );
	unguardf(( "(NetMode=%i)", GetLevelInfo()->NetMode ));
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

/*=============================================================================
	UnActor.cpp: AActor implementation
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"

/*-----------------------------------------------------------------------------
	AActor object implementations.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(AActor);
IMPLEMENT_CLASS(AWeapon);
IMPLEMENT_CLASS(ALevelInfo);
IMPLEMENT_CLASS(AGameInfo);
IMPLEMENT_CLASS(ACamera);
IMPLEMENT_CLASS(AZoneInfo);
IMPLEMENT_CLASS(ASkyZoneInfo);
IMPLEMENT_CLASS(APathNode);
IMPLEMENT_CLASS(ANavigationPoint);
IMPLEMENT_CLASS(AScout);
IMPLEMENT_CLASS(AInterpolationPoint);
IMPLEMENT_CLASS(ADecoration);
IMPLEMENT_CLASS(AProjectile);
IMPLEMENT_CLASS(AWarpZoneInfo);
IMPLEMENT_CLASS(ATeleporter);
IMPLEMENT_CLASS(APlayerStart);
IMPLEMENT_CLASS(AKeypoint);
IMPLEMENT_CLASS(AInventory);
IMPLEMENT_CLASS(AInventorySpot);
IMPLEMENT_CLASS(ATriggers);
IMPLEMENT_CLASS(ATrigger);
IMPLEMENT_CLASS(ATriggerMarker);
IMPLEMENT_CLASS(AButtonMarker);
IMPLEMENT_CLASS(AWarpZoneMarker);
IMPLEMENT_CLASS(AHUD);
IMPLEMENT_CLASS(AMenu);
IMPLEMENT_CLASS(ASavedMove);
IMPLEMENT_CLASS(ACarcass);
IMPLEMENT_CLASS(ALiftCenter);
IMPLEMENT_CLASS(ALiftExit);

/*-----------------------------------------------------------------------------
	APlayerPawn implementation.
-----------------------------------------------------------------------------*/

//
// Set the player.
//
void APlayerPawn::SetPlayer( UPlayer* InPlayer )
{
	guard(APlayerPawn::SetPlayer);
	check(InPlayer!=NULL);

	// Detach old player.
	if( InPlayer->Actor )
	{
		InPlayer->Actor->Player = NULL;
		InPlayer->Actor = NULL;
	}

	// Set the viewport.
	Player = InPlayer;
	InPlayer->Actor = this;

	// Send possess message to script.
	eventPossess();

	// Debug message.
	debugf( NAME_Log, "Possessed PlayerPawn: %s", GetFullName() );

	unguard;
}

/*-----------------------------------------------------------------------------
	ALevelInfo.
-----------------------------------------------------------------------------*/

void ALevelInfo::execGetAddressURL( FFrame& Stack, BYTE*& Result )
{
	guardSlow(ALevelInfo::execGetAddressURL);

	P_FINISH;

	FString Str;
	XLevel->URL.String(Str);
	appStrncpy( (char*)Result, *Str, 240 );
	char* Tmp = appStrchr( (char*)Result, '?' );
	if( Tmp )
		*Tmp = 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( ALevelInfo, INDEX_NONE, execGetAddressURL );

/*-----------------------------------------------------------------------------
	AZoneInfo.
-----------------------------------------------------------------------------*/

void AZoneInfo::PostEditChange()
{
	guard(AZoneInfo::PostEditChange);
	Super::PostEditChange();
	if( GIsEditor )
		GCache.Flush();
	unguard;
}

/*-----------------------------------------------------------------------------
	AActor.
-----------------------------------------------------------------------------*/

void AActor::ProcessEvent( UFunction* Function, void* Parms )
{
	guardSlow(AActor::ProcessEvent);
	if( Level->bBegunPlay )
		Super::ProcessEvent( Function, Parms );
	unguardSlow;
}

void AActor::PostEditChange()
{
	guard(AActor::PostEditChange);
	Super::PostEditChange();
	if( GIsEditor )
		bLightChanged = 1;
	unguard;
}

//
// Set the actor's collision properties.
//
void AActor::SetCollision
(
	UBOOL NewCollideActors,
	UBOOL NewBlockActors,
	UBOOL NewBlockPlayers
)
{
	guard(AActor::SetCollision);

	// Untouch this actor.
	if( bCollideActors && GetLevel()->Hash )
		GetLevel()->Hash->RemoveActor( this );

	// Set properties.
	bCollideActors = NewCollideActors;
	bBlockActors   = NewBlockActors;
	bBlockPlayers  = NewBlockPlayers;

	// Touch this actor.
	if( bCollideActors && GetLevel()->Hash )
		GetLevel()->Hash->AddActor( this );

	unguard;
}

//
// Set collision size.
//
void AActor::SetCollisionSize( FLOAT NewRadius, FLOAT NewHeight )
{
	guard(AActor::SetCollisionSize);

	// Untouch this actor.
	if( bCollideActors && GetLevel()->Hash )
		GetLevel()->Hash->RemoveActor( this );

	// Set properties.
	CollisionRadius = NewRadius;
	CollisionHeight = NewHeight;

	// Touch this actor.
	if( bCollideActors && GetLevel()->Hash )
		GetLevel()->Hash->AddActor( this );

	unguard;
}

//
// Return whether this actor overlaps another.
//
UBOOL AActor::IsOverlapping( const AActor* Other ) const
{
	guardSlow(AActor::IsOverlapping);
	debug(Other!=NULL);

	if( !IsBrush() && !Other->IsBrush() && Other!=Level )
	{
		// See if cylinder actors are overlapping.
		return
			Square(Location.X      - Other->Location.X)
		+	Square(Location.Y      - Other->Location.Y)
		<	Square(CollisionRadius + Other->CollisionRadius) 
		&&	Square(Location.Z      - Other->Location.Z)
		<	Square(CollisionHeight + Other->CollisionHeight);
	}
	else
	{
		// We cannot detect whether these actors are overlapping so we say they aren't.
		return 0;
	}
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	Actor touch minions.
-----------------------------------------------------------------------------*/

static UBOOL TouchTo( AActor* Actor, AActor* Other )
{
	guard(TouchTo);
	check(Actor);
	check(Other);
	check(Actor!=Other);

	INT Available=-1;
	for( INT i=0; i<ARRAY_COUNT(Actor->Touching); i++ )
	{
		if( Actor->Touching[i] == NULL )
		{
			// Found an available slot.
			Available = i;
		}
		else if( Actor->Touching[i] == Other )
		{
			// Already touching.
			return 1;
		}
	}
	if( Available == -1 )
	{
		// Try to prune touches.
		for( i=0; i<ARRAY_COUNT(Actor->Touching); i++ )
		{
			check(Actor->Touching[i]->IsValid());
			if( Actor->Touching[i]->Physics == PHYS_None )
			{
				Actor->EndTouch( Actor->Touching[i], 0 );
				Available = i;
			}
		}
		if ( (Available == -1) && Other->IsA(APawn::StaticClass) )
		{
			// try to prune in favor of 1. players, 2. other pawns
			for( i=0; i<ARRAY_COUNT(Actor->Touching); i++ )
			{
				if( !Actor->Touching[i]->IsA(APawn::StaticClass) )
				{
					Actor->EndTouch( Actor->Touching[i], 0 );
					Available = i;
					break;
				}
			}
			if ( (Available == -1) && ((APawn *)Other)->bIsPlayer )
				for( i=0; i<ARRAY_COUNT(Actor->Touching); i++ )
				{
					if( !Actor->Touching[i]->IsA(APawn::StaticClass) || !((APawn *)Actor->Touching[i])->bIsPlayer )
					{
						Actor->EndTouch( Actor->Touching[i], 0 );
						Available = i;
						break;
					}
				}
		}
	}

	if( Available >= 0 )
	{
		// Make Actor touch TouchActor.
		Actor->Touching[Available] = Other;
		Actor->eventTouch( Other );

		// See if first actor did something that caused an UnTouch.
		if( Actor->Touching[Available] != Other )
			return 0;
	}

	return 1;
	unguard;
}

//
// Note that TouchActor has begun touching Actor.
//
// If an actor's touch list overflows, neither actor receives the
// touch messages, as if they are not touching.
//
// This routine is reflexive.
//
// Handles the case of the first-notified actor changing its touch status.
//
void AActor::BeginTouch( AActor* Other )
{
	guard(AActor::BeginTouch);

	// Perform reflective touch.
	if( TouchTo( this, Other ) )
		TouchTo( Other, this );

	unguard;
}

//
// Note that TouchActor is no longer touching Actor.
//
// If NoNotifyActor is specified, Actor is not notified but
// TouchActor is (this happens during actor destruction).
//
void AActor::EndTouch( AActor* Other, UBOOL NoNotifySelf )
{
	guard(AActor::EndTouch);
	check(Other!=this);

	// Notify Actor.
	for( int i=0; i<ARRAY_COUNT(Touching); i++ )
	{
		if( Touching[i] == Other )
		{
			Touching[i] = NULL;
			if( !NoNotifySelf )
				eventUnTouch( Other );
			break;
		}
	}

	// Notify TouchActor.
	for( i=0; i<ARRAY_COUNT(Other->Touching); i++ )
	{
		if( Other->Touching[i] == this )
		{
			Other->Touching[i] = NULL;
			Other->eventUnTouch( this );
			break;
		}
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	AActor member functions.
-----------------------------------------------------------------------------*/

//
// Destroy the actor.
//
void AActor::Serialize( FArchive& Ar )
{
	guard(AActor::Serialize);
	Super::Serialize( Ar );
	if( Ar.Ver()<57 )//oldver
		InitialState = GObj.GetTempState();
	if( Ar.Ver()<58 )
		Group = GObj.GetTempGroup();
	unguard;
}

/*-----------------------------------------------------------------------------
	Relations.
-----------------------------------------------------------------------------*/

//
// Change the actor's owner.
//
void AActor::SetOwner( AActor *NewOwner )
{
	guard(AActor::SetOwner);

	// Sets this actor's parent to the specified actor.
	if( Owner != NULL )
		Owner->eventLostChild( this );

	Owner = NewOwner;

	if( Owner != NULL )
		Owner->eventGainedChild( this );

	unguard;
}

//
// Change the actor's base.
//
void AActor::SetBase( AActor* NewBase, int bNotifyActor )
{
	guard(AActor::SetBase);
	//debugf("SetBase %s -> %s",GetName(),NewBase ? NewBase->GetName() : "NULL");

	// Verify no recursion.
	for( AActor* Loop=NewBase; Loop!=NULL; Loop=Loop->Base )
		if ( Loop == this ) 
			return;

	if( NewBase != Base )
	{
		// Notify old base, unless it's the level.
		if( Base && Base!=Level )
		{
			Base->StandingCount--;
			Base->eventDetach( this );
		}

		// Set base.
		Base = NewBase;

		// Notify new base, unless it's the level.
		if( Base && Base!=Level )
		{
			Base->StandingCount++;
			Base->eventAttach( this );
		}

		// Notify this actor of his new floor.
		if ( bNotifyActor )
			eventBaseChange();
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	The end.
-----------------------------------------------------------------------------*/

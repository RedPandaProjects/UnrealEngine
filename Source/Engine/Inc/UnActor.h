/*=============================================================================
	UnActor.h: AActor class inlines.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
        * Aug 30, 1996: Mark added PLevel
		* Oct 19, 1996: Tim redesigned to eliminate redundency
=============================================================================*/

/*-----------------------------------------------------------------------------
	FActorLink.
-----------------------------------------------------------------------------*/

//
// Linked list of actors.
//
struct FActorLink
{
	// Variables.
	AActor*     Actor;
	FActorLink* Next;

	// Functions.
	FActorLink( AActor* InActor, FActorLink* InNext )
	: Actor(InActor), Next(InNext)
	{}
};

/*-----------------------------------------------------------------------------
	AActor inlines.
-----------------------------------------------------------------------------*/

//
// Brush checking.
//
inline UBOOL AActor::IsBrush()       const {return Brush!=NULL && IsA(ABrush::StaticClass);}
inline UBOOL AActor::IsStaticBrush() const {return Brush!=NULL && IsA(ABrush::StaticClass) &&  bStatic;}
inline UBOOL AActor::IsMovingBrush() const {return Brush!=NULL && IsA(ABrush::StaticClass) && !bStatic;}

//
// See if this actor is owned by TestOwner.
//
inline UBOOL AActor::IsOwnedBy( const AActor* TestOwner ) const
{
	guardSlow(AActor::IsOwnedBy);
	for( const AActor* Arg=this; Arg; Arg=Arg->Owner )
	{
		if( Arg == TestOwner )
			return 1;
	}
	return 0;
	unguardSlow;
}

//
// Get the top-level owner of an actor.
//
inline AActor* AActor::GetTopOwner()
{
	for( AActor* Top=this; Top->Owner; Top=Top->Owner );
	return Top;
}

//
// See if this actor is in the specified zone.
//
inline UBOOL AActor::IsInZone( const AZoneInfo* TestZone ) const
{
	return Region.Zone!=Level ? Region.Zone==TestZone : 1;
}

//
// If this actor is a player, return it as a Pawn.
// Otherwise return NULL.
//
inline APlayerPawn* AActor::GetPlayerPawn() const
{
	guardSlow(AActor::GetPlayerPawn);

	// Only descendents of the pawn class may be players.
	if( !IsA(APlayerPawn::StaticClass) )
		return NULL;

	// Players must have player objects.
	if( ((APlayerPawn*)this)->Player == NULL )
		return NULL;

	// This is a player.
	return (APlayerPawn*)this;

	unguardSlow;
}

//
// Return whether this actor looks like a player.
//
inline UBOOL AActor::IsPlayer() const
{
	guardSlow(AActor::IsPlayer);

	// Only descendents of the pawn class may be players.
	if( !IsA(APawn::StaticClass) )
		return 0;

	// Players must have viewports.
	return ((APawn*)this)->bIsPlayer;

	unguardSlow;
}

//
// Determine if BlockingActor should block actors of the given class.
// This routine needs to be reflexive or else it will create funky
// results, i.e. A->IsBlockedBy(B) <-> B->IsBlockedBy(A).
//
inline UBOOL AActor::IsBlockedBy( const AActor* Other ) const
{
	guardSlow(AActor::IsBlockedBy);
	debug(this!=NULL);
	debug(Other!=NULL);

	if( Other==Level )
		return bCollideWorld;
	else if( Other->IsBrush() )
		return bCollideWorld && (GetPlayerPawn() ? Other->bBlockPlayers : Other->bBlockActors);
	else if ( IsBrush() )
		return Other->bCollideWorld && (Other->GetPlayerPawn() ? bBlockPlayers : bBlockActors);
	else
		return ( (GetPlayerPawn() || IsA(AProjectile::StaticClass)) ? Other->bBlockPlayers : Other->bBlockActors)
		&&	   ( (Other->GetPlayerPawn() || Other->IsA(AProjectile::StaticClass)) ? bBlockPlayers : bBlockActors);

	unguardSlow;
}

//
// Return whether this actor's movement is based on another actor.
//
inline UBOOL AActor::IsBasedOn( const AActor* Other ) const
{
	guard(AActor::IsBasedOn);
	for( const AActor* Test=this; Test!=NULL; Test=Test->Base )
		if( Test == Other )
			return 1;
	return 0;
	unguard;
}

//
// Return the level of an actor.
//
inline class ULevel* AActor::GetLevel() const
{
	return XLevel;
}

//
// Return the actor's view rotation.
//
inline FRotator AActor::GetViewRotation()
{
	guardSlow(AActor::GetViewRotation);
	return IsA(APawn::StaticClass) ? ((APawn*)this)->ViewRotation : Rotation;
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	AActor audio.
-----------------------------------------------------------------------------*/

//
// Get the actor's primitive.
//
inline UPrimitive* AActor::GetPrimitive() const
{
	guardSlow(AActor::GetPrimitive);
	if     ( Brush  ) return Brush;
	else if( Mesh   ) return Mesh;
	else              return XLevel->Engine->Cylinder;
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

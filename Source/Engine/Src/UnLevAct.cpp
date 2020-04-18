/*=============================================================================
	UnLevAct.cpp: Level actor functions
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"

/*-----------------------------------------------------------------------------
	Level actor management.
-----------------------------------------------------------------------------*/

//
// Create a new actor. Returns the new actor, or NULL if failure.
//
AActor* ULevel::SpawnActor
(
	UClass*			Class,
	FName			InName,
	AActor*			Owner,
	class APawn*	Instigator,
	FVector			Location,
	FRotator		Rotation,
	AActor*			Template,
	UBOOL			bIsPlayer,
	UBOOL			bNoCollisionFail,
	UBOOL			bRemoteOwned
)
{
	guard(ULevel::SpawnActor);

	// Make sure this class is spawnable.
	if( !Class )
	{
		debugf( NAME_Warning, "SpawnActor failed because no class was specified" );
		return NULL;
	}
	if( Class->ClassFlags & CLASS_Abstract )
	{
		debugf( NAME_Warning, "SpawnActor failed because class %s is abstract", Class->GetName() );
		return NULL;
	}
	else if( !Class->IsChildOf(AActor::StaticClass) )
	{
		debugf( NAME_Warning, "SpawnActor failed because %s is not an actor class", Class->GetName() );
		return NULL;
	}
	else if( !GIsEditor && (Class->GetDefaultActor()->bStatic || Class->GetDefaultActor()->bNoDelete) )
	{
		debugf( NAME_Warning, "SpawnActor failed because class %s has bStatic or bNoDelete", Class->GetName() );
		return NULL;		
	}

	// Use class's default actor as a template.
	if( !Template )
		Template = Class->GetDefaultActor();
	check(Template!=NULL);

	// Make sure actor will fit at desired location, and adjust location if necessary.
	if( (Template->bCollideWorld || Template->bCollideWhenPlacing) && !bNoCollisionFail )
	{
		if( !FindSpot( Template->GetCylinderExtent(), Location, 0, 1 ) )
		{
			//debugf( "SpawnActor %s failed because FindSpot didn't fit", Class->GetName() );
			return NULL;
		}
	}

	// Add at end of list.
	INT iActor = Add();
	ModifyItem( iActor );
    AActor* Actor = Actors(iActor) = (AActor*)GObj.ConstructObject( Class, GetParent(), InName, 0, Template );
	Actor->SetFlags( RF_Transactional );

	// Set base actor properties.
	Actor->Tag		= Class->GetFName();
	Actor->Region	= FPointRegion( GetLevelInfo() );
	Actor->Level	= GetLevelInfo();
	Actor->bTicked  = !Ticked;
	Actor->XLevel	= this;
	if( Class->IsChildOf(APawn::StaticClass) )
		((APawn*)Actor)->bIsPlayer = bIsPlayer;

	// Set network role.
	check(Actor->Role==ROLE_Authority);
	if( bRemoteOwned )
		Exchange( Actor->Role, Actor->RemoteRole );

	// Remove the actor's brush, if it has one, because moving brushes are not duplicatable.
	if( Actor->Brush )
		Actor->Brush = NULL;

	// Set the actor's location and rotation.
	Actor->Location = Location;
	Actor->OldLocation = Location;
	Actor->Rotation = Rotation;
	if( Actor->bCollideActors && Hash  )
		Hash->AddActor( Actor );

	// Init the actor's zone.
	Actor->Region = FPointRegion(GetLevelInfo());
	if( Actor->IsA(APawn::StaticClass) )
		((APawn*)Actor)->FootRegion = ((APawn*)Actor)->HeadRegion = FPointRegion(GetLevelInfo());

	// Set owner.
	Actor->SetOwner( Owner );

	// Set instigator
	Actor->Instigator = Instigator;

	// Send messages.
	Actor->InitExecution();
	Actor->Spawned();
	Actor->eventSpawned();
	Actor->eventPreBeginPlay();
	Actor->eventBeginPlay();
	if( Actor->bDeleteMe )
		return NULL;

	// Set the actor's zone.
	SetActorZone( Actor, iActor==0, 1 );

	// Send PostBeginPlay.
	Actor->eventPostBeginPlay();

	// Check for encroachment.
	if( !bNoCollisionFail && CheckEncroachment( Actor, Actor->Location, Actor->Rotation, 0 ) )
	{
		DestroyActor( Actor );
		return NULL;
	}

	// Init scripting.
	Actor->eventSetInitialState();

	// Find Base
	if( !Actor->Base && Actor->bCollideWorld
		 && (Actor->IsA(ADecoration::StaticClass) || Actor->IsA(AInventory::StaticClass) || Actor->IsA(APawn::StaticClass)) 
		 && ((Actor->Physics == PHYS_None) || (Actor->Physics == PHYS_Rotating)) )
		Actor->FindBase();

	// Success: Return the actor.
	if( InTick )
		NewlySpawned = new(GDynMem)FActorLink(Actor,NewlySpawned);

	return Actor;
	unguardf(( "(%s)", Class->GetName() ));
}

//
// Spawn a brush.
//
ABrush* ULevel::SpawnBrush()
{
	guard(ULevel::SpawnBrush);

	ABrush* Result = (ABrush*)SpawnActor( ABrush::StaticClass );
	check(Result);

	return Result;
	unguard;
}

//
// Destroy an actor.
// Returns 1 if destroyed, 0 if it couldn't be destroyed.
//
// What this routine does:
// * Remove the actor from the actor list.
// * Generally cleans up the engine's internal state.
//
// What this routine does not do, but is done in ULevel::Tick instead:
// * Removing references to this actor from all other actors.
// * Killing the actor resource.
//
// This routine is set up so that no problems occur even if the actor
// being destroyed inside its recursion stack.
//
UBOOL ULevel::DestroyActor( AActor* ThisActor, UBOOL bNetForce )
{
	guard(ULevel::DestroyActor);
	check(ThisActor);
	check(ThisActor->IsValid());
	//debugf( NAME_Log, "Destroy %s", ThisActor->GetClassName() );

	// In-game deletion rules.
	guard(DestroyRules);
	if( !GIsEditor )
	{
		// Can't kill bStatic and bNoDelete actors during play.
		if( ThisActor->bStatic || ThisActor->bNoDelete )
			return 0;

		// Can't kill if wrong role.
		if( ThisActor->Role!=ROLE_Authority && !bNetForce )
			return 0;

		// If already on list to be deleted, pretend the call was successful.
		if( ThisActor->bDeleteMe )
			return 1;
	}
	unguard;

	// Get index.
	INT iActor = GetActorIndex( ThisActor );
	guard(ModifyActor);
	ModifyItem( iActor );
	ThisActor->Modify();
	unguard;

	// Send EndState notification.
	guard(EndState);
	if( ThisActor->GetMainFrame() && ThisActor->GetMainFrame()->StateNode && ThisActor->IsProbing(NAME_EndState) )
	{
		ThisActor->EndState();
		if( ThisActor->bDeleteMe )
			return 1;
	}
	unguard;

	// Remove from base.
	guard(Debase);
	if( ThisActor->Base )
	{
		ThisActor->SetBase( NULL );
		if( ThisActor->bDeleteMe )
			return 1;
	}
	if( ThisActor->StandingCount > 0 )
		for( INT i=0; i<Num(); i++ )
			if( Actors(i) && Actors(i)->Base == ThisActor ) 
				Actors(i)->SetBase( NULL );
	unguard;

	// Remove from world collision hash.
	guard(Unhash);
	if( Hash )
	{
		if( ThisActor->bCollideActors )
			Hash->RemoveActor( ThisActor );
		Hash->CheckActorNotReferenced( ThisActor );
	}
	unguard;

	// Tell this actor it's about to be destroyed.
	guard(ProcessDestroyed);
	ThisActor->eventDestroyed();
	if( ThisActor->bDeleteMe )
		return 1;
	unguard;

	// Clean up all owned and touching actors.
	guard(CleanupStandardRefs);
	for( INT iActor=0; iActor<Num(); iActor++ )
	{
		AActor* Other = Actors(iActor);
		if( Other )
		{
			if( Other->Owner==ThisActor )
			{
				Other->SetOwner( NULL );
				if( ThisActor->bDeleteMe )
					return 1;
			}
			else
			{
				for( INT j=0; j<ARRAY_COUNT(Other->Touching); j++ )
				{
					if( Other->Touching[j]==ThisActor )
					{
						ThisActor->EndTouch( Other, 1 );
						if( ThisActor->bDeleteMe )
							return 1;
					}
				}
			}
		}
	}
	unguard;

	// If this actor has an owner, notify it that it has lost a child.
	guard(Disown);
	if( ThisActor->Owner )
	{
		ThisActor->Owner->eventLostChild( ThisActor );
		if( ThisActor->bDeleteMe )
			return 1;
	}
	unguard;

	// Notify net players that this guy has been destroyed.
	guard(NotifyNetPlayers);
	if( NetDriver )
	{
		for( INT i=0; i<NetDriver->Connections.Num(); i++ )
		{
			UNetConnection* Connection = NetDriver->Connections(i);
			FActorChannel*  Channel    = Connection->GetActorChannel( ThisActor );
			if( Channel )
			{
				check(Channel->OpenedLocally);
				Channel->Close();
			}
		}
	}
	unguard;

	// Remove the actor from the actor list.
	guard(Unlist);
	check(Actors(iActor)==ThisActor);
	Actors(iActor) = NULL;
	ThisActor->bDeleteMe = 1;
	unguard;

	// Do object destroy.
	guard(ShutupSound);
	if( Engine->Audio )
		Engine->Audio->NoteDestroy( ThisActor );
	ThisActor->ConditionalDestroy();
	unguard;

	// Cleanup.
	guard(Cleanup);
	if( !GIsEditor )
	{
		// During play, just add to delete-list list and destroy when level is unlocked.
		ThisActor->Deleted = FirstDeleted;
		FirstDeleted       = ThisActor;
	}
	else
	{
		// Destroy them now.
		CleanupDestroyed( 1 );
	}
	unguard;

	// Return success.
	return 1;
	unguardf(( "(%s)", ThisActor->GetFullName() ));
}

//
// Compact the actor list.
//
void ULevel::CompactActors()
{
	guard(ULevel::CompactActors);
	INT c = iFirstDynamicActor;
	for( INT i=iFirstDynamicActor; i<Num(); i++ )
	{
		if( Actors(i) )
		{
			if( !Actors(i)->bDeleteMe )
				Actors(c++) = Actors(i);
			else
				debugf( "Undeleted %s", Actors(i)->GetFullName() );
		}
	}
	if( c != Num() )
		Remove( c, Num()-c );
	unguard;
}

//
// Cleanup destroyed actors.
// During gameplay, called in ULevel::Unlock.
// During editing, called after each actor is deleted.
//
void ULevel::CleanupDestroyed( UBOOL bForce )
{
	guard(ULevel::CleanupDestroyed);

	// Pack actor list.
	if( !GIsEditor && !bForce )
		CompactActors();

	// If nothing deleted, exit.
	if( !FirstDeleted )
		return;

	// Don't do anything unless a bunch of actors are in line to be destroyed.
	guard(CheckDeleted);
	INT c=0;
	for( AActor* A=FirstDeleted; A; A=A->Deleted )
		c++;
	if( c<128 && !bForce )
		return;
	unguard;

	// Remove all references to actors tagged for deletion.
	guard(CleanupRefs);
	for( INT iActor=0; iActor<Num(); iActor++ )
	{
		AActor* Actor = Actors(iActor);
		if( Actor )
		{
			// Would be nice to say if(!Actor->bStatic), but we can't count on it.
			//!!checkSlow(!Actor->bDeleteMe);
			Actor->GetClass()->CleanupDestroyed( (BYTE*)Actor );
		}
	}
	unguard;

	// If editor, let garbage collector destroy objects.
	if( GIsEditor )
		return;

	guard(FinishDestroyedActors);
	while( FirstDeleted!=NULL )
	{
		// Physically destroy the actor-to-delete.
		check(FirstDeleted->bDeleteMe);
		AActor* ActorToKill = FirstDeleted;
		FirstDeleted        = FirstDeleted->Deleted;
		check(ActorToKill->bDeleteMe);

		// Destroy the actor.
		delete ActorToKill;
	}
	unguard;

	unguard;
}

/*-----------------------------------------------------------------------------
	Player spawning.
-----------------------------------------------------------------------------*/

//
// Find an available camera actor in the level and return it, or spawn a new
// one if none are available.  Returns actor number or NULL if none are
// available.
//
void ULevel::SpawnViewActor( UViewport* Viewport )
{
	guard(ULevel::SpawnViewActor);
	check(Engine->Client);
	check(Viewport->Actor==NULL);

	// Find an existing camera actor.
	guard(FindExisting);
	for( INT iActor=0; iActor<Num(); iActor++ )
	{
		ACamera* TestActor = Cast<ACamera>( Actors(iActor) );
		if( TestActor && !TestActor->Player && (Viewport->GetFName()==TestActor->Tag) ) 
		{
			Viewport->Actor = TestActor;
            break;
		}
    }
	unguard;

	guard(SpawnNew);
    if( !Viewport->Actor )
	{
		// None found, spawn a new one and set default position.
		Viewport->Actor = (ACamera*)SpawnActor( ACamera::StaticClass, NAME_None, NULL, NULL, FVector(-500,-300,+300), FRotator(0,0,0), NULL, 1, 1 );
		check(Viewport->Actor);
		Viewport->Actor->Tag = Viewport->GetFName();
	}
	unguard;

	// Set the new actor's properties.
	guard(SetProperties);
	Viewport->Actor->SetFlags( RF_NotForClient | RF_NotForServer );
	Viewport->Actor->ClearFlags( RF_Transactional );
	Viewport->Actor->Player		= Viewport;
	Viewport->Actor->ShowFlags	= SHOW_Frame | SHOW_MovingBrushes | SHOW_Actors | SHOW_Brush | SHOW_Menu;
	Viewport->Actor->RendMap    = REN_DynLight;
	unguard;

	// Set the zone.
	SetActorZone( Viewport->Actor, 0, 1 );

	unguard;
}

//
// Spawn an actor for gameplay.
//
struct FAcceptInfo
{
	AActor* Actor;
	FString Name;
	TArray<FString> Parms;
	FAcceptInfo( AActor* InActor, const char* InName )
	: Actor( InActor ), Name( InName ), Parms()
	{}
};
APlayerPawn* ULevel::SpawnPlayActor( UPlayer* Player, ENetRole RemoteRole, const FURL& URL, FString Items, char* Error256 )
{
	guard(ULevel::SpawnPlayActor);
	check(Error256);
	Error256[0]=0;

	// Get PlayerClass.
	UClass* PlayerClass=NULL;
	const char* Str = URL.GetOption( "CLASS=", NULL );
	if( Str )
		PlayerClass = LoadClass<APlayerPawn>( NULL, Str, NULL, LOAD_NoWarn | LOAD_KeepImports, GetSandbox() );
	if( !PlayerClass )
		PlayerClass = LoadClass<APlayerPawn>( NULL, "ini:DefaultPlayer.Class", NULL, LOAD_NoFail | LOAD_KeepImports, GetSandbox() );

	// Make the option string.
	char Options[1024]="";
	for( INT i=0; i<URL.Op.Num(); i++ )
	{
		appStrcat( Options, "?" );
		appStrcat( Options, *URL.Op(i) );
	}

	// Tell UnrealScript to log in.
	APlayerPawn* Actor = GetLevelInfo()->Game->eventLogin( *URL.Portal, Options, Error256, PlayerClass );
	if( !Actor )
		return NULL;

	// Possess the newly-spawned player.
	Actor->SetPlayer( Player );
	Actor->Role       = ROLE_Authority;
	Actor->RemoteRole = RemoteRole;
	Actor->ShowFlags  = SHOW_Backdrop | SHOW_Actors | SHOW_Menu | SHOW_PlayerCtrl | SHOW_RealTime;
	Actor->RendMap	  = REN_DynLight;
	if( ParseParam(appCmdLine(),"alladmin") )
		Actor->bAdmin = 1;
	Actor->eventTravelPreAccept();

	// Any saved items?
	const char* PlayerName = URL.GetOption( "NAME=", *FURL::DefaultName );
	if( PlayerName )
	{
		for( INT i=0; i<TravelNames.Num(); i++ )
		{
			if( appStricmp(*TravelNames(i),PlayerName)==0 )
			{
				Items = TravelItems(i);
				TravelNames.Remove(i);
				TravelItems.Remove(i);
				break;
			}
		}
	}

	// Handle inventory items.
	Str = *Items;
	char ClassName[256], ActorName[256];
	TArray<FAcceptInfo> Accepted;
	while( Parse(Str,"CLASS=",ClassName,ARRAY_COUNT(ClassName)) && Parse(Str,"NAME=",ActorName,ARRAY_COUNT(ActorName)) )
	{
		// Load class.
		FAcceptInfo* Accept=NULL;
		AActor* Spawned=NULL;
		UClass* Class=LoadClass<AActor>( NULL, ClassName, NULL, LOAD_NoWarn|LOAD_KeepImports|LOAD_AllowDll, GetSandbox() );
		if( !Class )
		{
			debugf( NAME_Log, "SpawnPlayActor: Cannot accept travelling class '%s'", ClassName );
		}
		else if( Class->IsChildOf(APlayerPawn::StaticClass) )
		{
			Accept = new(Accepted)FAcceptInfo(Actor,ActorName);
		}
		else if( (Spawned=SpawnActor( Class, NAME_None, Actor, NULL, Actor->Location, Actor->Rotation, NULL, 0, 1 ))==NULL )
		{
			debugf( NAME_Log, "SpawnPlayActor: Failed to spawn travelling class '%s'", ClassName );
		}
		else
		{
			debugf( NAME_Log, "SpawnPlayActor: Spawned travelling actor" );
			Accept = new(Accepted)FAcceptInfo(Spawned,ActorName);
		}

		// Save properties.
		char Buffer[256];
		ParseLine(&Str,Buffer,ARRAY_COUNT(Buffer),1);
		ParseLine(&Str,Buffer,ARRAY_COUNT(Buffer),1);
		while( ParseLine(&Str,Buffer,ARRAY_COUNT(Buffer),1) && appStrcmp(Buffer,"}")!=0 )
			if( Accept )
				new(Accept->Parms)FString(Buffer);
	}

	// Import properties.
	for( i=0; i<Accepted.Num(); i++ )
	{
		// Parse all properties.
		for( INT j=0; j<Accepted(i).Parms.Num(); j++ )
		{
			const char* Ptr = *Accepted(i).Parms(j);
			while( *Ptr==' ' )
				Ptr++;
			char VarName[256], *VarEnd=VarName;
			while( appIsAlnum(*Ptr) )
				*VarEnd++ = *Ptr++;
			*VarEnd=0;
			INT Element=0;
			if( *Ptr=='[' )
			{
				Element=appAtoi(++Ptr);
				while( appIsDigit(*Ptr) )
					Ptr++;
				if( *Ptr++!=']' )
					continue;
			}
			if( *Ptr++!='=' )
				continue;
			for( TFieldIterator<UProperty> It(Accepted(i).Actor->GetClass()); It; ++It )
			{
				if
				(	(It->PropertyFlags & CPF_Travel)
				&&	appStricmp( It->GetName(), VarName )==0 
				&&	Element<It->ArrayDim )
				{
					// Import the property.
					BYTE* Data = (BYTE*)Accepted(i).Actor + It->Offset + Element*It->ArrayDim;
					UObjectProperty* Ref = Cast<UObjectProperty>( *It );
					if( Ref && Ref->PropertyClass->IsChildOf(AActor::StaticClass) )
					{
						for( INT k=0; k<Accepted.Num(); k++ )
						{
							if( Accepted(k).Name==Ptr )
							{
								*(UObject**)Data = Accepted(k).Actor;
								break;
							}
						}
					}
					else It->ImportText( Ptr, Data, 0 );
				}
			}
		}
	}

	// Call travel-acceptance functions in reverse order to avoid inventory flipping.
	for( i=Accepted.Num()-1; i>=0; i-- )
		Accepted(i).Actor->eventTravelPreAccept();
	GetLevelInfo()->Game->eventAcceptInventory( Actor );
	for( i=Accepted.Num()-1; i>=0; i-- )
		Accepted(i).Actor->eventTravelPostAccept();
	Actor->eventTravelPostAccept();

	// Remember travelling items for restarting level.
	Actor->CarryInfo = NULL;
	if( Items != "" )
	{
		Actor->CarryInfo = new(GetParent())UTextBuffer;
		Actor->CarryInfo->Text = Items;
	}

	return Actor;
	unguard;
}

/*-----------------------------------------------------------------------------
	Level actor moving/placing.
-----------------------------------------------------------------------------*/

//
// Find a suitable nearby location to place a collision box.
// No suitable location will ever be found if Location is not a valid point inside the level ( and not in 
// a wall)

// AdjustSpot used by FindSpot
void ULevel::AdjustSpot( FVector& Adjusted, FVector TraceDest, FLOAT TraceLen, FCheckResult& Hit )
{
	SingleLineCheck( Hit, NULL, TraceDest, Adjusted, TRACE_VisBlocking );
	if( Hit.Time < 1.0 )
		Adjusted = Adjusted + Hit.Normal * (1.05 - Hit.Time) * TraceLen;
}

UBOOL ULevel::FindSpot
(
	FVector  Extent,
	FVector& Location,
	UBOOL	 bCheckActors,
	UBOOL	 bAssumeFit
)
{
	guard(ULevel::FindSpot);

	// trace to all corners to find interpenetrating walls
	FCheckResult Hit(1.0);
	if( Extent==FVector(0,0,0) )
		return SinglePointCheck( Hit, Location, Extent, 0, GetLevelInfo(), bCheckActors )==1;

	if( bAssumeFit && SinglePointCheck( Hit,Location, Extent, 0, GetLevelInfo(), bCheckActors )==1 )
		return 1;
	FVector Adjusted = Location;
	FLOAT TraceLen = Extent.Size() + 2.0;

	for (int i=-1;i<2;i+=2)
	{
		AdjustSpot(Adjusted, Adjusted + FVector(i * Extent.X,0,0), Extent.X, Hit); 
		AdjustSpot(Adjusted, Adjusted + FVector(0,i * Extent.Y,0), Extent.Y, Hit); 
		AdjustSpot(Adjusted, Adjusted + FVector(0,0,i * Extent.Z), Extent.Z, Hit); 
	}
	if( SinglePointCheck( Hit, Adjusted, Extent, 0, GetLevelInfo(), bCheckActors )==1 )
	{
		Location = Adjusted;
		return 1;
	}

	for (i=-1;i<2;i+=2)
		for (int j=-1;j<2;j+=2)
			for (int k=-1;k<2;k+=2)
				AdjustSpot(Adjusted, Adjusted + FVector(i * Extent.X, j * Extent.Y, k * Extent.Z), TraceLen, Hit); 

	if( (Adjusted - Location).SizeSquared() > 1.5 * Extent.SizeSquared() )
		return 0;

	if( SinglePointCheck( Hit, Adjusted, Extent, 0, GetLevelInfo(), bCheckActors )==1 )
	{
		Location = Adjusted;
		return 1;
	}
	return 0;
	unguard;
}

//
// Try to place an actor that has moved a long way.  This is for
// moving actors through teleporters, adding them to levels, and
// starting them out in levels.  The results of this function is independent
// of the actor's current location and rotation.
//
// If the actor doesn't fit exactly in the location specified, tries
// to slightly move it out of walls and such.
//
// Returns 1 if the actor has been successfully moved, or 0 if it couldn't fit.
//
// Updates the actor's Zone and sends ZoneChange if it changes.
//
UBOOL ULevel::FarMoveActor( AActor* Actor, FVector DestLocation,  UBOOL test, UBOOL bNoCheck )
{
	guard(ULevel::FarMoveActor);
	check(Actor!=NULL);
	if( (Actor->bStatic || !Actor->bMovable) && !GIsEditor )
		return 0;

	if( Actor->bCollideActors && Hash ) //&& !test
		Hash->RemoveActor( Actor );

	FVector newLocation = DestLocation;
	int result = 1;
	if (!bNoCheck && Actor->bCollideWorld || Actor->bCollideWhenPlacing) 
		result = FindSpot( Actor->GetCylinderExtent(), newLocation, 0, 0 );

	if (result && !test && !bNoCheck)
		result = !CheckEncroachment( Actor, newLocation, Actor->Rotation, 1);
	
	if( result )
	{
		if( !test )
		{
			if( Actor->StandingCount > 0 )
				for( INT i=0; i<Num(); i++ )
					if( Actors(i) && Actors(i)->Base == Actor ) 
						Actors(i)->SetBase( NULL );
			Actor->bJustTeleported = true;
		}
		Actor->Location = newLocation;
		Actor->OldLocation = newLocation; //to zero velocity
	}

	if( Actor->bCollideActors && Hash ) //&& !test
		Hash->AddActor( Actor );

	// Set the zone after moving, so that if a ZoneChange or ActorEntered/ActorEntered message
	// tries to move the actor, the hashing will be correct.
	if( result )
		SetActorZone( Actor, test );

	return result;
	unguard;
}

//
// Place the actor on the floor below.  May move the actor a long way down.
// Updates the actor's Zone and sends ZoneChange if it changes.
//
//

UBOOL ULevel::DropToFloor( AActor *Actor)
{
	guard(ULevel::DropToFloor);
	check(Actor!=NULL);

	// Try moving down a long way and see if we hit the floor.

	FCheckResult Hit(1.0);
	MoveActor( Actor, FVector( 0, 0, -1000 ), Actor->Rotation, Hit );
	return (Hit.Time < 1.0);

	unguard;
}

//
// Tries to move the actor by a movement vector.  If no collision occurs, this function 
// just does a Location+=Move.
//
// Assumes that the actor's Location is valid and that the actor
// does fit in its current Location. Assumes that the level's 
// Dynamics member is locked, which will always be the case during
// a call to ULevel::Tick; if not locked, no actor-actor collision
// checking is performed.
//
// If bCollideWorld, checks collision with the world.
//
// For every actor-actor collision pair:
//
// If both have bCollideActors and bBlocksActors, performs collision
//    rebound, and dispatches Touch messages to touched-and-rebounded 
//    actors.  
//
// If both have bCollideActors but either one doesn't have bBlocksActors,
//    checks collision with other actors (but lets this actor 
//    interpenetrate), and dispatches Touch and UnTouch messages.
//
// Returns 1 if some movement occured, 0 if no movement occured.
//
// Updates actor's Zone and sends ZoneChange if it changes.
//
// If Test = 1 (default 0), do not send notifications.
//
UBOOL ULevel::MoveActor
(
	AActor*			Actor,
	FVector			Delta,
	FRotator		NewRotation,
	FCheckResult&	Hit,
	UBOOL			bTest,
	UBOOL			bIgnorePawns,
	UBOOL			bIgnoreBases,
	UBOOL			bNoFail
)
{
	guard(ULevel::MoveActor);
	check(Actor!=NULL);
	if( (Actor->bStatic || !Actor->bMovable) && !GIsEditor )
		return 0;

	// Skip if no vector.
	if( Delta.IsNearlyZero() )
	{
		if( NewRotation==Actor->Rotation )
		{
			return 1;
		}
		else if( !Actor->StandingCount && !Actor->IsMovingBrush() )
		{
			Actor->Rotation  = NewRotation;
			return 1;
		}
	}

	// Set up.
	Hit = FCheckResult(1.0);
	NumMoves++;
	clock(MoveCycles);
	FMemMark Mark(GMem);
	FLOAT DeltaSize;
	FVector DeltaDir;
	if( Delta.IsNearlyZero() )
	{
		DeltaSize = 0;
		DeltaDir = Delta;
	}
	else
	{
		DeltaSize = Delta.Size();
		DeltaDir       = Delta/DeltaSize;
	}
	FLOAT TestAdjust	   = 2.0;
	FVector TestDelta      = Delta + TestAdjust * DeltaDir;
	INT     MaybeTouched   = 0;
	INT     NumHits        = 0;
	FCheckResult* FirstHit = NULL;

	// Perform movement collision checking if needed for this actor.
	if( (Actor->bCollideActors || Actor->bCollideWorld) && !Actor->IsMovingBrush() && Delta!=FVector(0,0,0) )
	{
		// Check collision along the line.
		FirstHit = MultiLineCheck
		(
			GMem,
			Actor->Location + TestDelta,
			Actor->Location,
			Actor->GetCylinderExtent(),
			(Actor->bCollideActors && !Actor->IsMovingBrush()) ? 1              : 0,
			(Actor->bCollideWorld  && !Actor->IsMovingBrush()) ? GetLevelInfo() : NULL,
			0
		);

		// Handle first blocking actor.
		if( Actor->bCollideWorld || Actor->bBlockActors || Actor->bBlockPlayers )
		{
			for( FCheckResult* Test=FirstHit; Test; Test=Test->GetNext() )
			{
				if
				(	(!bIgnorePawns || Test->Actor->bStatic || (!Test->Actor->IsA(APawn::StaticClass) && !Test->Actor->IsA(ADecoration::StaticClass)))
				&&	(!bIgnoreBases || !Actor->IsBasedOn(Test->Actor))
				&&	(!Test->Actor->IsBasedOn(Actor)               ) )
				{
					MaybeTouched = 1;
					if( Actor->IsBlockedBy(Test->Actor) )
					{
						Hit = *Test;
						break;
					}
				}
			}
		}
	}

	// Attenuate movement.
	FVector FinalDelta = Delta;
	if( Hit.Time < 1.0 && !bNoFail )
	{
		// Fix up delta, given that TestDelta = Delta + TestAdjust.
		FLOAT FinalDeltaSize = (DeltaSize + TestAdjust) * Hit.Time;
		if ( FinalDeltaSize <= TestAdjust)
		{
			FinalDelta = FVector(0,0,0);
			Hit.Time = 0;
		}
		else 
		{
			FinalDelta = TestDelta * Hit.Time - TestAdjust * DeltaDir;
			Hit.Time   = (FinalDeltaSize - TestAdjust) / DeltaSize;
		}
	}

	// Move the based actors (before encroachment checking).
	if( Actor->StandingCount && !bTest )
	{
		for( int i=0; i<Num(); i++ )
		{
			AActor* Other = Actors(i);
			if( Other && Other->Base==Actor )
			{
				// Move base.
				FVector   RotMotion( 0, 0, 0 );
				FRotator DeltaRot ( 0, NewRotation.Yaw - Actor->Rotation.Yaw, 0 );
				if( NewRotation != Actor->Rotation )
				{
					// Handle rotation-induced motion.
					FRotator ReducedRotation = FRotator( 0, ReduceAngle(NewRotation.Yaw) - ReduceAngle(Actor->Rotation.Yaw), 0 );
					FVector   Pointer         = Actor->Location - Other->Location;
					RotMotion                 = Pointer - Pointer.TransformVectorBy( GMath.UnitCoords * ReducedRotation );
				}
				FCheckResult Hit(1.0);
				MoveActor( Other, FinalDelta + RotMotion, Other->Rotation + DeltaRot, Hit, 0, 0, 1 );

				// Update pawn view.
				if( Other->IsA(APawn::StaticClass) )
					((APawn*)Other)->ViewRotation += DeltaRot;
			}
		}
	}

	// Abort if encroachment declined.
	if( !bTest && !bNoFail && !Actor->IsA(APawn::StaticClass) && CheckEncroachment( Actor, Actor->Location + FinalDelta, NewRotation, 0 ) )
	{
		unclock(MoveCycles);
		return 0;
	}

	// Update the location.
	if( Actor->bCollideActors && Hash )
		Hash->RemoveActor( Actor );
	Actor->Location += FinalDelta;
	Actor->Rotation  = NewRotation;
	if( Actor->bCollideActors && Hash )
		Hash->AddActor( Actor );

	// Handle bump and touch notifications.
	if( !bTest )
	{
		// Notify first bumped actor unless it's the level or the actor's base.
		if( Hit.Actor && Hit.Actor!=GetLevelInfo() && !Actor->IsBasedOn(Hit.Actor) )
		{
			// Notify both actors of the bump.
			Hit.Actor->eventBump(Actor);
			Actor->eventBump(Hit.Actor);
		}

		// Handle Touch notifications.
		if( MaybeTouched || !Actor->bBlockActors || !Actor->bBlockPlayers )
			for( FCheckResult* Test=FirstHit; Test && Test->Time<Hit.Time; Test=Test->GetNext() )
				if
				(	(!Test->Actor->IsBasedOn(Actor))
				&&	(!bIgnoreBases || !Actor->IsBasedOn(Test->Actor))
				&&	(!Actor->IsBlockedBy(Test->Actor)) )
					Actor->BeginTouch( Test->Actor );

		// UnTouch notifications.
		for( int i=0; i<ARRAY_COUNT(Actor->Touching); i++ )
			if( Actor->Touching[i] && !Actor->IsOverlapping(Actor->Touching[i]) )
				Actor->EndTouch( Actor->Touching[i], 0 );
	}

	// Set actor zone.
	SetActorZone( Actor, bTest );
	Mark.Pop();

	// Return whether we moved at all.
	unclock(MoveCycles);
	return Hit.Time>0.0;
	unguard;
}

/*-----------------------------------------------------------------------------
	Encroachment.
-----------------------------------------------------------------------------*/

//
// Check whether Actor is encroaching other actors after a move, and return
// 0 to ok the move, or 1 to abort it.
//
UBOOL ULevel::CheckEncroachment
(
	AActor*		Actor,
	FVector		TestLocation,
	FRotator	TestRotation,
	UBOOL		bTouchNotify
)
{
	guard(ULevel::CheckEncroachment);
	check(Actor);

	// If this actor doesn't need encroachment checking, allow the move.
	if( !Actor->bCollideActors && !Actor->bBlockActors && !Actor->bBlockPlayers && !Actor->IsMovingBrush() )
		return 0;

	// Query the mover about what he wants to do with the actors he is encroaching.
	FMemMark Mark(GMem);
	FCheckResult* FirstHit = Hash ? Hash->ActorEncroachmentCheck( GMem, Actor, TestLocation, TestRotation, 0 ) : NULL;	
	for( FCheckResult* Test = FirstHit; Test!=NULL; Test=Test->GetNext() )
	{
		int noProcess = 0;
		if
		(	Test->Actor!=Actor
		&&	Test->Actor!=GetLevelInfo()
		&&	Actor->IsBlockedBy( Test->Actor ) )
		{
			if ( Actor->IsMovingBrush() && !Test->Actor->IsMovingBrush() ) 
			{
				// check if mover can safely push encroached actor
				//Move test actor away from mover
				FVector MoveDir = TestLocation - Actor->Location;
				FVector OldLoc = Test->Actor->Location;
				Test->Actor->moveSmooth(MoveDir);
				// see if mover still encroaches test actor
				FCheckResult* RecheckHit = Hash->ActorEncroachmentCheck( GMem, Actor, TestLocation, TestRotation, 0 );
				noProcess = 1;
				for ( FCheckResult* Recheck = RecheckHit; Recheck!=NULL; Recheck=Recheck->GetNext() )
					if ( Recheck->Actor == Test->Actor )
					{
						noProcess = 0;
						break;
					}
				if ( !noProcess ) //push test actor back toward brush
				{
					FVector realLoc = Actor->Location;
					Actor->Location = TestLocation;
					Test->Actor->moveSmooth(-1 * MoveDir);
					Actor->Location = realLoc;
				}
			}
			if ( !noProcess && Actor->eventEncroachingOn(Test->Actor) )
			{
				Mark.Pop();
				return 1;
			}
		}
	}

	// If bTouchNotify, send Touch and UnTouch notifies.
	if( bTouchNotify )
	{
		// UnTouch notifications.
		for( int i=0; i<ARRAY_COUNT(Actor->Touching); i++ )
			if( Actor->Touching[i] && !Actor->IsOverlapping(Actor->Touching[i]) )
				Actor->EndTouch( Actor->Touching[i], 0 );
	}

	// Notify the encroached actors but not the level.
	for( Test = FirstHit; Test; Test=Test->GetNext() )
		if
		(	Test->Actor!=Actor
		&&	Test->Actor!=GetLevelInfo() )
		{ 
			if( Actor->IsBlockedBy(Test->Actor) ) 
				Test->Actor->eventEncroachedBy(Actor);
			else if( bTouchNotify )
				Actor->BeginTouch( Test->Actor );
		}
							
	Mark.Pop();


	// Ok the move.
	return 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	SinglePointCheck.
-----------------------------------------------------------------------------*/

//
// Check for nearest hit.
// Return 1 if no hit, 0 if hit.
//
UBOOL ULevel::SinglePointCheck
(
	FCheckResult&	Hit,
	FVector			Location,
	FVector			Extent,
	DWORD			ExtraNodeFlags,
	ALevelInfo*		Level,
	UBOOL			bActors
)
{
	guard(ULevel::SinglePointCheck);
	FMemMark Mark(GMem);
	FCheckResult* Hits = MultiPointCheck( GMem, Location, Extent, ExtraNodeFlags, Level, bActors );
	if( !Hits )
	{
		Mark.Pop();
		return 1;
	}
	Hit = *Hits;
	for( Hits = Hits->GetNext(); Hits!=NULL; Hits = Hits->GetNext() )
		if( (Hits->Location-Location).SizeSquared() < (Hit.Location-Location).SizeSquared() )
			Hit = *Hits;
	Mark.Pop();
	return 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	MultiPointCheck.
-----------------------------------------------------------------------------*/

FCheckResult* ULevel::MultiPointCheck( FMemStack& Mem, FVector Location, FVector Extent, DWORD ExtraNodeFlags, ALevelInfo* Level, UBOOL bActors )
{
	guard(ULevel::MultiPointCheck);
	FCheckResult* Result=NULL;

	// Check with actors.
	if( bActors && Hash )
		Result = Hash->ActorPointCheck( Mem, Location, Extent, ExtraNodeFlags );

	// Check with level.
	if( Level )
	{
		FCheckResult TestHit(1.0);
		if( Level->XLevel->Model->PointCheck( TestHit, NULL, Location, Extent, 0 )==0 )
		{
			// Hit.
			TestHit.GetNext() = Result;
			Result            = new(GMem)FCheckResult(TestHit);
			Result->Actor     = Level;
		}
	}
	return Result;
	unguard;
}

/*-----------------------------------------------------------------------------
	SingleLineCheck.
-----------------------------------------------------------------------------*/

//
// Trace a line and return the first hit actor (LevelInfo means hit the world geomtry).
//
UBOOL ULevel::SingleLineCheck
(
	FCheckResult&	Hit,
	AActor*			SourceActor,
	const FVector&	End,
	const FVector&	Start,
	DWORD           TraceFlags,
	FVector			Extent,
	BYTE			ExtraNodeFlags
)
{
	guard(ULevel::Trace);

	// Get list of hit actors.
	FMemMark Mark(GMem);
	FCheckResult* FirstHit = MultiLineCheck
	(
		GMem,
		End,
		Start,
		Extent,
		(TraceFlags & TRACE_AllColliding) ? 1 : 0,
		(TraceFlags & TRACE_Level       ) ? GetLevelInfo() : NULL,
		ExtraNodeFlags
	);

	// Skip owned actors and return the one nearest actor.
	for( FCheckResult* Check = FirstHit; Check!=NULL; Check=Check->GetNext() )
	{
		if( !SourceActor || !SourceActor->IsOwnedBy( Check->Actor ) )
		{
			if( Check->Actor->IsA(ALevelInfo::StaticClass) )
			{
				if( TraceFlags & TRACE_Level )
					break;
			}
			else if( Check->Actor->IsA(APawn::StaticClass) )
			{
				if( TraceFlags & TRACE_Pawns )
					break;
			}
			else if( Check->Actor->IsA(AMover::StaticClass) )
			{
				if( TraceFlags & TRACE_Movers )
					break;
			}
			else if( Check->Actor->IsA(AZoneInfo::StaticClass) )
			{
				if( TraceFlags & TRACE_ZoneChanges )
					break;
			}
			else
			{
				if( TraceFlags & TRACE_Others )
				{
					if( TraceFlags & TRACE_OnlyProjActor )
					{
						if( Check->Actor->bProjTarget || (Check->Actor->bBlockActors && Check->Actor->bBlockPlayers) )
							break;
					}
					else break;
				}
			}
		}
	}
	if( Check )
	{
		Hit = *Check;
	}
	else
	{
		Hit.Time = 1.0;
		Hit.Actor = NULL;
	}

	Mark.Pop();
	return Check==NULL;
	unguard;
}

/*-----------------------------------------------------------------------------
	MultiLineCheck.
-----------------------------------------------------------------------------*/

FCheckResult* ULevel::MultiLineCheck
(
	FMemStack&		Mem,
	FVector			End,
	FVector			Start,
	FVector			Extent,
	UBOOL			bCheckActors,
	ALevelInfo*		LevelInfo,
	BYTE			ExtraNodeFlags
)
{
	guard(ULevel::MultiLineCheck);
	INT NumHits=0;
	FCheckResult Hits[64];

	// Check for collision with the level, and cull by the end point for speed.
	FLOAT Dilation = 1.0;
	INT bOnlyCheckForMovers = 0;
	INT bHitWorld = 0;

	guard(CheckWithLevel);
	if( LevelInfo && LevelInfo->XLevel->Model->LineCheck( Hits[NumHits], NULL, End, Start, Extent, ExtraNodeFlags )==0 )
	{
		bHitWorld = 1;
		Hits[NumHits].Actor = LevelInfo;
		FLOAT Dist = (Hits[NumHits].Location - Start).Size();
		Dilation = ::Min(1.f, Hits[NumHits].Time * (Dist + 5)/Dist);
		End = Start + (End - Start) * Dilation;
		if( Hits[NumHits].Time < 0.01 )
			bOnlyCheckForMovers = 1;
		NumHits++;
	}
	unguard;

	// Check with actors.
	guard(CheckWithActors);
	if( bCheckActors && Hash )
	{
		for( FCheckResult* Link=Hash->ActorLineCheck( Mem, End, Start, Extent, ExtraNodeFlags ); Link && NumHits<ARRAY_COUNT(Hits); Link=Link->GetNext() )
		{
			if ( !bOnlyCheckForMovers || Link->Actor->IsA(AMover::StaticClass) )
			{
				if ( bHitWorld && Link->Actor->IsA(AMover::StaticClass) 
					&& (Link->Normal == Hits[0].Normal)
					&& ((Link->Location - Hits[0].Location).SizeSquared() < 4) ) // make sure it wins compared to world
				{
					FVector TraceDir = End - Start;
					FLOAT TraceDist = TraceDir.Size();
					TraceDir = TraceDir/TraceDist;
					Link->Location = Hits[0].Location - 2 * TraceDir;
					Link->Time = (Link->Location - Start).Size();
					Link->Time = Link->Time/TraceDist;
				}
				Link->Time *= Dilation;
				Hits[NumHits++] = *Link;
			}
		}
	}
	unguard;

	// Sort the list.
	FCheckResult* Result = NULL;
	if( NumHits )
	{
		appSort( Hits, NumHits );
		Result = new(Mem,NumHits)FCheckResult;
		for( int i=0; i<NumHits; i++ )
		{
			Result[i]      = Hits[i];
			Result[i].Next = (i+1<NumHits) ? &Result[i+1] : NULL;
		}
	}
	return Result;
	unguard;
}

/*-----------------------------------------------------------------------------
	ULevel zone functions.
-----------------------------------------------------------------------------*/

//
// Figure out which zone an actor is in, update the actor's iZone,
// and notify the actor of the zone change.  Skips the zone notification
// if the zone hasn't changed.
//
void ULevel::SetActorZone( AActor* Actor, UBOOL bTest, UBOOL bForceRefresh )
{
	guard(ULevel::SetActorZone);
	check(Actor);
	if( Actor->bDeleteMe )
		return;

	// If LevelInfo actor, handle specially.
	if( Actor == GetLevelInfo() )
	{
		Actor->Region = FPointRegion( GetLevelInfo() );
		return;
	}

	// See if this is a pawn.
	APawn* Pawn = Actor->IsA(APawn::StaticClass) ? (APawn*)Actor : NULL;

	// If refreshing, init the actor's current zone.
	if( bForceRefresh )
	{
		// Init the actor's zone.
		Actor->Region = FPointRegion(GetLevelInfo());
		if( Pawn )
			Pawn->FootRegion = Pawn->HeadRegion = FPointRegion(GetLevelInfo());
	}

	// Find zone based on actor's location and see if it has changed.
	FPointRegion NewRegion = Model->PointRegion( Num() ? GetLevelInfo() : (ALevelInfo*)Actor, Actor->Location );
	if( NewRegion.Zone!=Actor->Region.Zone )
	{
		// Notify old zone info of player leaving.
		if( !bTest )
		{
			Actor->Region.Zone->eventActorLeaving(Actor);
			Actor->eventZoneChange( NewRegion.Zone );
		}
		Actor->Region = NewRegion;
		if( !bTest )
		{
			Actor->Region.Zone->eventActorEntered(Actor);
		}
	}
	else Actor->Region = NewRegion;
	debug(Actor->Region.Zone!=NULL);

	if( Pawn )
	{
		// Update foot region.
		FPointRegion NewFootRegion = Model->PointRegion( GetLevelInfo(), Pawn->Location - FVector(0,0,Pawn->CollisionHeight) );
		if( NewFootRegion.Zone!=Pawn->FootRegion.Zone && !bTest )
			Pawn->eventFootZoneChange(NewFootRegion.Zone);
		Pawn->FootRegion = NewFootRegion;

		// Update head region.
		FPointRegion NewHeadRegion = Model->PointRegion( GetLevelInfo(), Pawn->Location + FVector(0,0,Pawn->EyeHeight) );
		if( NewHeadRegion.Zone!=Pawn->HeadRegion.Zone && !bTest )
			Pawn->eventHeadZoneChange(NewHeadRegion.Zone);
		Pawn->HeadRegion = NewHeadRegion;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

/*=============================================================================
	UnLevel.cpp: Level-related functions
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"

/*-----------------------------------------------------------------------------
	ULevelBase implementation.
-----------------------------------------------------------------------------*/

ULevelBase::ULevelBase( UEngine* InEngine, const FURL& InURL )
:	URL( InURL )
,	Engine( InEngine )
{}
void ULevelBase::Serialize( FArchive& Ar )
{
	guard(ULevelBase::Serialize);
	Super::Serialize(Ar);

	if( Ar.Ver()>=59 )
		Ar << URL;

	if( !Ar.IsLoading() && !Ar.IsSaving() )
		Ar << NetDriver;

	unguard;
}
void ULevelBase::Destroy()
{
	guard(ULevelBase::Destroy);
	if( NetDriver )
	{
		delete NetDriver;
		NetDriver = NULL;
	}
	UObject::Destroy();
	unguard;
}
void ULevelBase::NotifyProgress( const char* Str1, const char* Str2, FLOAT Seconds )
{
	guard(ULevelBase::NotifyProgress);
	Engine->SetProgress( Str1, Str2, Seconds );
	unguard;
}
IMPLEMENT_DB_CLASS(ULevelBase);

/*-----------------------------------------------------------------------------
	Level creation & emptying.
-----------------------------------------------------------------------------*/

//
//	Create a new level and allocate all objects needed for it.
//	Call with Editor=1 to allocate editor structures for it, also.
//
ULevel::ULevel( UEngine* InEngine, UBOOL InRootOutside )
:	ULevelBase( InEngine )
{
	guard(ULevel::ULevel);

	// Allocate subobjects.
	SetFlags( RF_Transactional );
	Model = new( GetParent() )UModel( NULL, InRootOutside );
	Model->SetFlags( RF_Transactional );

	// Spawn the level info.
	SpawnActor( ALevelInfo::StaticClass );
	check(GetLevelInfo());

	// Spawn the default brush.
	ABrush* Temp = SpawnBrush();
	check(Temp==Actors(1));
	Temp->Brush = new( GetParent(), "Brush" )UModel( Temp, 1 );
	Temp->SetFlags( RF_NotForClient | RF_NotForServer | RF_Transactional );
	Temp->Brush->SetFlags( RF_NotForClient | RF_NotForServer | RF_Transactional );

	unguard;
}
void ULevel::ShrinkLevel()
{
	guard(ULevel::Shrink);

	Model->ShrinkModel();
	ReachSpecs.Shrink();

	unguard;
}
void ULevel::DetailChange( UBOOL NewDetail )
{
	guard(ULevel::DetailChange);
	GetLevelInfo()->bHighDetailMode = NewDetail;
	if( GetLevelInfo()->Game )
		GetLevelInfo()->Game->eventDetailChange();
	unguard;
}

/*-----------------------------------------------------------------------------
	Level locking and unlocking.
-----------------------------------------------------------------------------*/

//
// Modify this level.
//
void ULevel::Modify()
{
	guard(ULevel::Modify);
	UObject::Modify();
	Model->Modify();
	unguard;
}

void ULevel::SetActorCollision( UBOOL bCollision )
{
	guard(ULevel::SetActorCollision);

	// Init collision if first time through.
	if( bCollision && !Hash )
	{
		// Init hash.
		guard(StartCollision);
		Hash = GNewCollisionHash();
		for( INT i=0; i<Num(); i++ )
			if( Actors(i) && Actors(i)->bCollideActors )
				Hash->AddActor( Actors(i) );
		unguard;
	}
	else if( Hash && !bCollision )
	{
		// Destroy hash.
		guard(EndCollision);
		for( INT i=0; i<Num(); i++ )
			if( Actors(i) && Actors(i)->bCollideActors )
				Hash->RemoveActor( Actors(i) );
		delete Hash;
		Hash = NULL;
		unguard;
	}

	unguard;
}

/*-----------------------------------------------------------------------------
	Level object implementation.
-----------------------------------------------------------------------------*/

void ULevel::Serialize( FArchive& Ar )
{
	guard(ULevel::Serialize);
	Super::Serialize(Ar);

	// ULevel.
	Ar << Model;
	Ar << ReachSpecs;
	Ar << TimeSeconds;
	Ar << FirstDeleted;
	for( INT i=0; i<NUM_LEVEL_TEXT_BLOCKS; i++ )
		Ar << TextBlocks[i];

	if( Ar.Ver() >= 61 )//oldver
		Ar << TravelNames << TravelItems;

	unguard;
}
void ULevel::Export( FOutputDevice& Out, const char* FileType, int Indent )
{
	guard(ULevel::Export);

	Out.Logf( "%sBegin Map Name=%s\r\n", appSpc(Indent), GetName() );
	UBOOL AllSelected = appStricmp(FileType,"copy")!=0;
	for( INT iActor=0; iActor<Num(); iActor++ )
	{
		AActor* Actor = Actors(iActor);
		if( Actor && !Actor->IsA(ACamera::StaticClass) && (AllSelected ||Actor->bSelected) )
		{
			Out.Logf( "%sBegin Actor Class=%s Name=%s\r\n", appSpc(Indent), Actor->GetClassName(), Actor->GetName() );
			GObj.ExportProperties( Actor->GetClass(), (BYTE*)Actor, &Out, Indent+3, Actor->GetClass(), &Actor->GetClass()->Defaults(0) );
			Out.Logf("%sEnd Actor\r\n",appSpc(Indent));
		}
	}
	Out.Logf( "%sEnd Map\r\n", appSpc(Indent) );

	unguard;
}
void ULevel::ModifyAllItems()
{
	guard(ULevel::ModifyAllItems);

	// Modify all actors that exist.
	for( int i=0; i<Num(); i++ )
	{
		ModifyItem( i );
		if( Actors(i) )
			Actors(i)->Modify();
	}
	unguard;
}
void ULevel::Destroy()
{
	guard(ULevel::Destroy);

	// Free the cached collision info.
	if( Hash )
	{
		delete Hash;
		Hash = NULL; /* Required because actors may try to unhash themselves */
	}

	if( BrushTracker )
	{
		delete BrushTracker;
		BrushTracker = NULL; /* Required because brushes may clean themselves up */
	}

	ULevelBase::Destroy();
	unguard;
}
IMPLEMENT_DB_CLASS(ULevel);

/*-----------------------------------------------------------------------------
	Reconcile actors and Viewports after loading or creating a new level.

	These functions provide the basic mechanism by which UnrealEd associates
	Viewports and actors together, even when new maps are loaded which contain
	an entirely different set of actors which must be mapped onto the existing 
	Viewports.
-----------------------------------------------------------------------------*/

//
// Remember actors.
//
void ULevel::RememberActors()
{
	guard(ULevel::RememberActors);

	if( Engine->Client )
	{
		for( int i=0; i<Engine->Client->Viewports.Num(); i++ )
		{
			UViewport* Viewport			= Engine->Client->Viewports(i);
			Viewport->SavedOrthoZoom	= Viewport->Actor->OrthoZoom;
			Viewport->SavedFovAngle		= Viewport->Actor->FovAngle;
			Viewport->SavedShowFlags	= Viewport->Actor->ShowFlags;
			Viewport->SavedRendMap		= Viewport->Actor->RendMap;
			Viewport->SavedMisc1		= Viewport->Actor->Misc1;
			Viewport->SavedMisc2		= Viewport->Actor->Misc2;
			Viewport->Actor				= NULL;
		}
	}
	unguard;
}

//
// Reconcile actors.  This is called after loading a level.
// It attempts to match each existing Viewport to an actor in the newly-loaded
// level.  If no decent match can be found, creates a new actor for the Viewport.
//
void ULevel::ReconcileActors()
{
	guard(ULevel::ReconcileActors);
	check(GIsEditor);

	// Dissociate all actor Viewports and remember their view properties.
	for( int i=0; i<Num(); i++ )
		if( Actors(i) && Actors(i)->IsA(APlayerPawn::StaticClass) )
			if( ((APlayerPawn*)Actors(i))->Player )
				((APlayerPawn*)Actors(i))->Player = NULL;

	// Match Viewports and Viewport-actors with identical names.
	guard(MatchIdentical);
	for( int i=0; i<Engine->Client->Viewports.Num(); i++ )
	{
		UViewport* Viewport = Engine->Client->Viewports(i);
		check(Viewport->Actor==NULL);
		for( INT j=0; j<Num(); j++ )
		{
			AActor* Actor = Actors(j);
			if( Actor && Actor->IsA(ACamera::StaticClass) && appStricmp(*Actor->Tag,Viewport->GetName())==0 )
			{
				debugf( NAME_Log, "Matched Viewport %s", Viewport->GetName() );
				Viewport->Actor         = (APlayerPawn *)Actor;
				Viewport->Actor->Player = Viewport;
				break;
			}
		}
	}
	unguard;

	// Match up all remaining Viewports to actors.
	guard(MatchEditorOther);
	for( i=0; i<Engine->Client->Viewports.Num(); i++ )
	{
		// Hook Viewport up to an existing actor or createa a new one.
		UViewport* Viewport = Engine->Client->Viewports(i);
		if( !Viewport->Actor )
			SpawnViewActor( Viewport );
	}
	unguard;

	// Handle remaining unassociated view actors.
	guard(KillViews);
	for( i=0; i<Num(); i++ )
	{
		ACamera* View = Cast<ACamera>(Actors(i));
		if( View )
		{
			UViewport* Viewport = Cast<UViewport>(View->Player);
			if( Viewport )
			{
				UViewport* Viewport	= (UViewport*)View->Player;
				View->ClearFlags( RF_Transactional );
				View->OrthoZoom		= Viewport->SavedOrthoZoom;	
				View->FovAngle		= Viewport->SavedFovAngle;
				View->ShowFlags		= Viewport->SavedShowFlags;
				View->RendMap		= Viewport->SavedRendMap;
				View->Misc1			= Viewport->SavedMisc1;
				View->Misc2			= Viewport->SavedMisc2;
			}
			else DestroyActor( View );
		}
	}
	unguard;

	unguard;
}

/*-----------------------------------------------------------------------------
	ULevel command-line.
-----------------------------------------------------------------------------*/

UBOOL ULevel::Exec( const char* Cmd, FOutputDevice* Out )
{
	guard(ULevel::Exec);
	const char* Str = Cmd;
	if( NetDriver && NetDriver->Exec( Cmd, Out ) ) return 1;
	else return 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	ULevel networking related functions.
-----------------------------------------------------------------------------*/

//
// Return the level's sandbox, if any.
//
FPackageMap* ULevel::GetSandbox()
{
	guardSlow(ULevel::GetSandbox);
	return NetDriver ? &NetDriver->Map : NULL;
	unguardSlow;
}

//
// Start listening for connections.
//
UBOOL ULevel::Listen( char* Error256 )
{
	guard(ULevel::Listen);
	if( NetDriver )
	{
		appSprintf( Error256, LocalizeError("NetAlready") );
		return 0;
	}
	else if( !GetLinker() )
	{
		appSprintf( Error256, LocalizeError("NetListen") );
		return 0;
	}
	else
	{
		UClass* NetDriverClass = GObj.LoadClass( UNetDriver::StaticClass, NULL, "ini:Engine.Engine.NetworkDevice", NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
		NetDriver = (UNetDriver*)GObj.ConstructObject( NetDriverClass );
		if( NetDriver->Init( 0, this, URL, Error256) )
		{
			// Init LinkerMap.
			NetDriver->Map.Init( this );
			UGameEngine* GameEngine = CastChecked<UGameEngine>( Engine );

			// Load server required packages.
			for( INT i=0; i<ARRAY_COUNT(GameEngine->ServerPackages); i++ )
			{
				if( *GameEngine->ServerPackages[i] )
				{
					debugf( "Loading: %s", GameEngine->ServerPackages[i] );
					ULinkerLoad* Linker = GObj.GetPackageLinker( NULL, GameEngine->ServerPackages[i], LOAD_NoFail|LOAD_KeepImports, NULL, NULL );
					NetDriver->Map.AddLinker( Linker );
				}
			}

			// Spawn network server support.
			for( i=0; i<ARRAY_COUNT(GameEngine->ServerActors); i++ )
			{
				if( *GameEngine->ServerActors[i] )
				{
					debugf( "Spawning: %s", GameEngine->ServerActors[i] );
					UClass* HelperClass = GObj.LoadClass( AActor::StaticClass, NULL, GameEngine->ServerActors[i], NULL, LOAD_NoFail | LOAD_KeepImports, NULL );
					SpawnActor( HelperClass );
				}
			}

			// Add GameInfo's package to map.
			check(GetLevelInfo());
			check(GetLevelInfo()->Game);
			check(GetLevelInfo()->Game->GetClass()->GetLinker());
			NetDriver->Map.AddLinker( GetLevelInfo()->Game->GetClass()->GetLinker() );

			// Precompute linker info.
			NetDriver->Map.Compute();

			// Set LevelInfo properties.
			GetLevelInfo()->NetMode   = Engine->Client ? NM_ListenServer : NM_DedicatedServer;
			GetLevelInfo()->bInternet = NetDriver->IsInternet();
			GetLevelInfo()->NextSwitchCountdown = NetDriver->ServerTravelPause;
			return 1;
		}
		else
		{
			delete NetDriver;
			NetDriver=NULL;
			return 0;
		}
	}
	unguard;
}

//
// Return whether this level is a server.
//
UBOOL ULevel::IsServer()
{
	guardSlow(ULevel::IsServer);
	return !NetDriver || !NetDriver->ServerConnection;
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	ULevel network notifys.
-----------------------------------------------------------------------------*/

//
// The network driver is about to accept a new connection attempt by a
// connectee, and we can accept it or refuse it.
//
EAcceptConnection ULevel::NotifyAcceptingConnection()
{
	guard(ULevel::NotifyAcceptingConnection);
	check(NetDriver!=NULL);

	if( NetDriver->ServerConnection )
	{
		// We are a client and we don't welcome incoming connections.
		debugf( NAME_DevNet, "NotifyAcceptingConnection client %s: Refused", GetName() );
		return ACCEPTC_Reject;
	}
	else
	{
		// We are a server and incoming connections should be mapped onto players.
		debugf( NAME_DevNet, "NotifyAcceptingConnection server %s: Accepted", GetName() );
		return *GetLevelInfo()->NextURL ? ACCEPTC_Ignore : ACCEPTC_Accept;
	}
	unguard;
}

//
// This server has accepted a connection.
//
void ULevel::NotifyAcceptedConnection( UNetConnection* Connection )
{
	guard(ULevel::NotifyAcceptedConnection);
	check(NetDriver!=NULL);
	check(NetDriver->ServerConnection==NULL);

	unguard;
}

//
// The network interface is notifying this level of a new channel-open
// attempt by a connectee, and we can accept or refuse it.
//
UBOOL ULevel::NotifyAcceptingChannel( FChannel* Channel )
{
	guard(ULevel::NotifyAcceptingChannel);
	check(NetDriver!=NULL);

	if( NetDriver->ServerConnection )
	{
		// We are a client and the server has just opened up a new channel.
		//debugf( "NotifyAcceptingChannel %i/%i client %s", Channel->ChIndex, Channel->ChType, GetName() );
		if( Channel->ChType==CHTYPE_Actor )
		{
			// Actor channel.
			//debugf( "Client accepting actor channel" );
			return 1;
		}
		else
		{
			// Unwanted channel type.
			debugf( NAME_DevNet, "Client refusing unwanted channel of type %i", Channel->ChType );
			return 0;
		}
	}
	else
	{
		// We are the server.
		if( Channel->ChIndex==0 && Channel->ChType==CHTYPE_Control )
		{
			// The client has opened initial channel.
			debugf( NAME_DevNet, "NotifyAcceptingChannel Control %i server %s: Accepted", Channel->ChIndex, GetFullName() );
			return 1;
		}
		else if( Channel->ChType==CHTYPE_File )
		{
			// The client is going to request a file.
			debugf( NAME_DevNet, "NotifyAcceptingChannel File %i server %s: Accepted", Channel->ChIndex, GetFullName() );
			return 1;
		}
		else
		{
			// Client can't open any other kinds of channels.
			debugf( NAME_DevNet, "NotifyAcceptingChannel %i %i server %s: Refused", Channel->ChType, Channel->ChIndex, GetFullName() );
			return 0;
		}
	}
	unguard;
}

//
// Received text on the control channel.
//
void ULevel::NotifyReceivedText( UNetConnection* Connection, const char* Text )
{
	guard(ULevel::NotifyReceivedText);
	if( NetDriver->ServerConnection )
	{
		// We are the client.
		debugf( NAME_DevNet, "Level client received: %s", Text );
		if( ParseCommand(&Text,"FAILURE") )
		{
			// Return to entry.
			Engine->SetClientTravel( NULL, "?failed", 1, 0, TRAVEL_Absolute );
		}
	}
	else
	{
		// We are the server.
		debugf( NAME_DevNet, "Level server received: %s", Text );
		if( ParseCommand(&Text,"HELLO") )
		{
			// Handle revision.
			INT Revision=0;
			Parse( Text, "REVISION=", Revision );
			if( Revision < NET_REVISION )
			{
				Connection->Logf( "UPGRADE REVISION=%i", NET_REVISION );
				Connection->FlushNet();
				delete Connection;
				return;
			}

			// Get byte limit.
			Connection->ByteLimit = NetDriver->DefaultByteLimit;
			Connection->Challenge = appCycles();
			Connection->Logf( "CHALLENGE CHALLENGE=%i", Connection->Challenge );
			Connection->FlushNet();
		}
		else if( ParseCommand(&Text,"LOGIN") )
		{
			// Admit or deny the player here.
			INT Response=0;
			if
			(	!Parse(Text,"RESPONSE=",Response)
			||	!Engine->ChallengeResponse(Connection->Challenge)==Response )
			{
				Connection->Logf( "FAILURE CHALLENGE" );
				Connection->FlushNet();
				delete Connection;
				return;
			}
			char Str[1024]="", Error256[256]="";
			Parse( Text, "URL=", Str, ARRAY_COUNT(Str) );
			Connection->RequestURL = Str;
			debugf( NAME_DevNet, "Login request: %s", *Connection->RequestURL );
			GetLevelInfo()->Game->eventPreLogin( Str, Error256 );
			if( *Error256 )
			{
				debugf( NAME_DevNet, "PreLogin failure: %s", Error256 );
				Connection->Logf( "FAILURE %s", Error256 );
				Connection->FlushNet();
				delete Connection;
				return;
			}
			INT RequestedByteLimit;
			if( Parse( Str, "RATE=", RequestedByteLimit ) )
				Connection->ByteLimit = Clamp( RequestedByteLimit, 500, NetDriver->MaxClientByteLimit );
			debugf( "Client rate is %i", Connection->ByteLimit );
			for( INT i=0; i<Connection->Driver->Map.Num(); i++ )
			{
				// Send information about the package.
				char GuidStr[64];
				FPackageInfo& Info = Connection->Driver->Map(i);
				Connection->Logf
				(
					"USES GUID=%s PACKAGE=%s FLAGS=%i SIZE=%i",
					Info.Guid.String(GuidStr),
					Info.Parent->GetName(),
					Info.PackageFlags,
					Info.FileSize
				);
			}
			Connection->Logf
			(
				"WELCOME LEVEL=%s LONE=%i",
				GetParent()->GetName(),
				GetLevelInfo()->bLonePlayer
			);
			Connection->FlushNet();
		}
		else if( ParseCommand(&Text,"JOIN") && !Connection->Actor )
		{
			// Spawn the player-actor for this network player.
			char Error256[256]="";
			debugf( NAME_DevNet, "Join request: %s", *Connection->RequestURL );
			if( !SpawnPlayActor( Connection, ROLE_AutonomousProxy, FURL(NULL,*Connection->RequestURL,TRAVEL_Absolute), "", Error256 ) )
			{
				// Failed to connect.
				debugf( NAME_DevNet, "Join failure: %s", Error256 );
				Connection->Logf( "FAILURE %s", Error256 );
				Connection->FlushNet();
				delete Connection;
			}
			else
			{
				// Successfully in game.
				debugf( NAME_DevNet, "Join succeeded", Connection->Actor->PlayerName );
			}
		}
	}
	unguard;
}

//
// Called when a file receive is about to begin.
//
void ULevel::NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const char* Error )
{
	guard(ULevel::NotifyReceivingFile);
	appErrorf( "Level received unexpected file" );
	unguard;
}

//
// Called when other side requests a file.
//
UBOOL ULevel::NotifySendingFile( UNetConnection* Connection, FGuid Guid )
{
	guard(ULevel::NotifySendingFile);
	if( NetDriver->ServerConnection )
	{
		// We are the client.
		debugf( NAME_DevNet, "Server requested file: Refused" );
		return 0;
	}
	else
	{
		// We are the server.
		debugf( NAME_DevNet, "Client requested file: Allowed" );
		return 1;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Stats.
-----------------------------------------------------------------------------*/

void ULevel::InitStats()
{
	guard(ULevel::InitStats);
	NetTickCycles = ActorTickCycles = AudioTickCycles = FindPathCycles
	= MoveCycles = NumMoves = NumReps = NumPV = GetRelevantCycles = NumRPC = SeePlayer
	= Spawning = Unused = 0;
	GScriptEntryTag = GScriptCycles = 0;
	unguard;
}
void ULevel::GetStats( char* Result )
{
	guard(ULevel::GetStats);
	appSprintf
	(
		Result,
		"Script=%05.1f Actor=%04.1f Path=%04.1f See=%04.1f Spawn=%04.1f Audio=%04.1f Un=%04.1f Move=%04.1f (%i) Net=%04.1f",
		GSecondsPerCycle*1000 * GScriptCycles,
		GSecondsPerCycle*1000 * ActorTickCycles,
		GSecondsPerCycle*1000 * FindPathCycles,
		GSecondsPerCycle*1000 * SeePlayer,
		GSecondsPerCycle*1000 * Spawning,
		GSecondsPerCycle*1000 * AudioTickCycles,
		GSecondsPerCycle*1000 * Unused,
		GSecondsPerCycle*1000 * MoveCycles,
		NumMoves,
		GSecondsPerCycle*1000 * NetTickCycles
	);
	unguard;
}

/*-----------------------------------------------------------------------------
	Actors relevant to a viewer.
-----------------------------------------------------------------------------*/

//
// Check visibility.
//
static UBOOL CanSee
(
	AActor*		Viewer,
	FVector		Location,
	AActor*		Target,
	FVector		Ahead
)
{
	guardSlow(CanSee);
	if( Target->IsOwnedBy( Viewer ) )
		return 1;
	if( Target->Owner && Target->Owner->IsA(APawn::StaticClass) && ((APawn*)Target->Owner)->Weapon==Target )
		return CanSee( Viewer, Location, Target->Owner, Ahead );
	if( Target->IsA(AZoneInfo::StaticClass) )
		return 1;
	if( Target->bHidden && !Target->bBlockPlayers && !Target->AmbientSound )
		return 0;
	FCheckResult Hit(1.0);

	// Moving brushes would need volume visibility checking which is impractical here.
	if( Target->Brush )
		return 1;

	// Trace from current location.
	if( Viewer->XLevel->Model->LineCheck(Hit,NULL,Location,Target->Location,FVector(0,0,0),NF_NotVisBlocking) )
		return 1;

	// Trace from predicted future location.
	if( Viewer->XLevel->Model->LineCheck(Hit,NULL,Ahead,Target->Location,FVector(0,0,0),NF_NotVisBlocking) )
		return 1;

	// If near, pick random location in bounding box, which will average out with relevence timer.
	if( (Target->Location-Location).SizeSquared() < Square(64*Target->CollisionHeight) )
	{
		// Near, so trace from current location to box vertices.
		FBox Box = Target->GetPrimitive()->GetRenderBoundingBox( Target, 0 );
		FVector V
		(
			Box.Min.X + appFrand()*(Box.Max.X-Box.Min.X),
			Box.Min.Y + appFrand()*(Box.Max.Y-Box.Min.Y),
			Box.Min.Z + appFrand()*(Box.Max.Z-Box.Min.Z)
		);
		if( Viewer->XLevel->Model->LineCheck(Hit,NULL,V,Location,FVector(0,0,0),NF_NotVisBlocking) )
			return 1;
	}

	return 0;
	unguardSlow;
}

//
// Get a list of actors that are relevant to a given network player pawn.
// These actors are replicated over the net.
//
INT ULevel::GetRelevantActors( APlayerPawn* InViewer, AActor** List, INT Max )
{
	guard(ULevel::GetRelevantActors);
	clock(GetRelevantCycles);
	debug(Max>0);
	NetTag++;

	// Get viewer coordinates.
	FVector  Location  = InViewer->Location;
	FRotator Rotation  = InViewer->ViewRotation;
	AActor*  Viewer    = InViewer;
	InViewer->eventPlayerCalcView( Viewer, Location, Rotation );
	check(Viewer);

	// Compute ahead-vectors for prediction.
	FVector Ahead;
	if( Viewer->Velocity.Size() < 4.0 )
	{
		// Moving slowly, so predict ahead based on rotation.
		Ahead = Rotation.Vector() * 128.0;
	}
	else
	{
		// Moving quickly, so predict ahead based on velocity.
		FLOAT PredictSeconds = 1.0;
		Ahead = PredictSeconds * Viewer->Velocity;
	}
	FLOAT PredictAheadSeconds = 1.0;
	FCheckResult Hit(1.0);
	Hit.Location = Location + Ahead;
	Viewer->XLevel->Model->LineCheck(Hit,NULL,Hit.Location,Location,FVector(0,0,0),NF_NotVisBlocking);

	// Slow version which doesn't use any precomputed visibility.
	INT Count=0;
	for( INT i=iFirstDynamicActor; i<Num(); i++ )
	{
		if
		(	Actors(i)
		&&	Actors(i)->RemoteRole!=ROLE_None
		&&	(Actors(i)==InViewer || CanSee(Viewer,Location,Actors(i),Hit.Location)) )
		{
			Actors(i)->NetTag = NetTag;
			List[Count++] = Actors(i);
			if( Count == Max )
				break;
		}
	}
	NumPV += Count;
	unclock(GetRelevantCycles);
	return Count;
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

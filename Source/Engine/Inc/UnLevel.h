/*=============================================================================
	UnLevel.h: ULevel definition.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Network notification sink.
-----------------------------------------------------------------------------*/

//
// Accepting connection responses.
//
enum EAcceptConnection
{
	ACCEPTC_Reject,	// Reject the connection.
	ACCEPTC_Accept, // Accept the connection.
	ACCEPTC_Ignore, // Ignore it, sending no reply, while server travelling.
};

//
// The net code uses this to send notifications.
//
class ENGINE_API FNetworkNotify
{
public:
	virtual EAcceptConnection NotifyAcceptingConnection()=0;
	virtual void NotifyAcceptedConnection( class UNetConnection* Connection )=0;
	virtual UBOOL NotifyAcceptingChannel( class FChannel* Channel )=0;
	virtual ULevel* NotifyGetLevel()=0;
	virtual void NotifyReceivedText( UNetConnection* Connection, const char* Text )=0;
	virtual UBOOL NotifySendingFile( UNetConnection* Connection, FGuid GUID )=0;
	virtual void NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const char* Error )=0;
	virtual void NotifyProgress( const char* Str1, const char* Str2, FLOAT Seconds )=0;
};

/*-----------------------------------------------------------------------------
	FCollisionHashBase.
-----------------------------------------------------------------------------*/

class FCollisionHashBase
{
public:
	// FCollisionHashBase interface.
	virtual ~FCollisionHashBase() {};
	virtual void Tick()=0;
	virtual void AddActor( AActor *Actor )=0;
	virtual void RemoveActor( AActor *Actor )=0;
	virtual FCheckResult* ActorLineCheck( FMemStack& Mem, FVector End, FVector Start, FVector Extent, BYTE ExtraNodeFlags )=0;
	virtual FCheckResult* ActorPointCheck( FMemStack& Mem, FVector Location, FVector Extent, DWORD ExtraNodeFlags )=0;
	virtual FCheckResult* ActorRadiusCheck( FMemStack& Mem, FVector Location, FLOAT Radius, DWORD ExtraNodeFlags )=0;
	virtual FCheckResult* ActorEncroachmentCheck( FMemStack& Mem, AActor* Actor, FVector Location, FRotator Rotation, DWORD ExtraNodeFlags )=0;
	virtual void CheckActorNotReferenced( AActor* Actor )=0;
};

ENGINE_API FCollisionHashBase* GNewCollisionHash();

/*-----------------------------------------------------------------------------
	ULevel base.
-----------------------------------------------------------------------------*/

//
// A game level.
//
class ENGINE_API ULevelBase : public UDatabase, public FNetworkNotify
{
	DECLARE_ABSTRACT_CLASS(ULevelBase,UDatabase,0)
	DECLARE_DB_CLASS(ULevelBase,UDatabase,AActor*)
	NO_DEFAULT_CONSTRUCTOR(ULevelBase)

	// Variables.
	class UNetDriver*	NetDriver;
	class UEngine*		Engine;
	FURL				URL;

	// Constructors.
	ULevelBase( UEngine* InOwner, const FURL& InURL=FURL(NULL) );

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();

	// FNetworkNotify interface.
	void NotifyProgress( const char* Str1, const char* Str2, FLOAT Seconds );
};

/*-----------------------------------------------------------------------------
	ULevel class.
-----------------------------------------------------------------------------*/

//
// Trace actor options.
//
enum ETraceActorFlags
{
	// Bitflags.
	TRACE_Pawns         = 0x01, // Check collision with pawns.
	TRACE_Movers        = 0x02, // Check collision with movers.
	TRACE_Level         = 0x04, // Check collision with level geometry.
	TRACE_ZoneChanges   = 0x08, // Check collision with soft zone boundaries.
	TRACE_Others        = 0x10, // Check collision with all other kinds of actors.
	TRACE_OnlyProjActor = 0x20, // Check collision with other actors only if they are projectile targets

	// Combinations.
	TRACE_VisBlocking   = TRACE_Level | TRACE_Movers,
	TRACE_AllColliding  = TRACE_Pawns | TRACE_Movers | TRACE_Level | TRACE_Others,
	TRACE_AllEverything = TRACE_Pawns | TRACE_Movers | TRACE_Level | TRACE_ZoneChanges | TRACE_Others,
	TRACE_ProjTargets	= TRACE_OnlyProjActor | TRACE_AllColliding,
};

//
// Level updating.
//
enum ELevelTick
{
	LEVELTICK_TimeOnly		= 0,	// Update the level time only.
	LEVELTICK_ViewportsOnly	= 1,	// Update time and viewports.
	LEVELTICK_All			= 2,	// Update all.
};

//
// The level object.  Contains the level's actor list, Bsp information, and brush list.
//
class ENGINE_API ULevel : public ULevelBase
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ULevel,ULevelBase,0)
	DECLARE_DB_CLASS(ULevel,ULevelBase,AActor*)
	NO_DEFAULT_CONSTRUCTOR(ULevel)

	// Number of blocks of descriptive text to allocate with levels.
	enum {NUM_LEVEL_TEXT_BLOCKS=16};

	// Main variables, always valid.
	TArray<FReachSpec>		ReachSpecs;
	UModel*					Model;
	UTextBuffer*			TextBlocks[NUM_LEVEL_TEXT_BLOCKS];
	FLOAT                   TimeSeconds;
	TArray<FString>			TravelNames;
	TArray<FString>			TravelItems;

	// Only valid in memory.
	FCollisionHashBase* Hash;
	class FMovingBrushTrackerBase* BrushTracker;
	AActor* FirstDeleted;
	struct FActorLink* NewlySpawned;
	UBOOL InTick, Ticked;
	INT iFirstDynamicActor, NetTag;
	BYTE ZoneDist[64][64];

	// Temporary stats.
	INT NetTickCycles, ActorTickCycles, AudioTickCycles, FindPathCycles, MoveCycles, NumMoves, NumReps, NumPV, GetRelevantCycles, NumRPC, SeePlayer, Spawning, Unused;

	// Constructor.
	ULevel( UEngine* InEngine, UBOOL RootOutside );

	// UObject interface.
	void Export( FOutputDevice& Out, const char* FileType, int Indent );
	void Serialize( FArchive &Ar );
	void Destroy();

	// ULevel interface.
	virtual void Modify();
	virtual void SetActorCollision( UBOOL bCollision );
	virtual void Tick( ELevelTick TickType, FLOAT DeltaSeconds );
	virtual void TickNetClient( FLOAT DeltaSeconds );
	virtual void TickNetServer( FLOAT DeltaSeconds );
	virtual INT ServerTickClient( UNetConnection* Conn, FLOAT DeltaSeconds );
	virtual void ReconcileActors();
	virtual void RememberActors();
	virtual UBOOL Exec( const char* Cmd, FOutputDevice* Out=GSystem );
	virtual void ShrinkLevel();
	virtual void ModifyAllItems();
	virtual INT GetRelevantActors( APlayerPawn* Pawn, AActor** List, INT Max );
	virtual void CompactActors();
	virtual UBOOL Listen( char* Error256 );
	virtual UBOOL IsServer();
	virtual UBOOL MoveActor( AActor *Actor, FVector Delta, FRotator NewRotation, FCheckResult &Hit, UBOOL Test=0, UBOOL IgnorePawns=0, UBOOL bIgnoreBases=0, UBOOL bNoFail=0 );
	virtual UBOOL FarMoveActor( AActor* Actor, FVector DestLocation, UBOOL Test=0, UBOOL bNoCheck=0 );
	virtual UBOOL DropToFloor( AActor* Actor );
	virtual UBOOL DestroyActor( AActor* Actor, UBOOL bNetForce=0 );
	virtual void CleanupDestroyed( UBOOL bForce );
	virtual AActor* SpawnActor( UClass* Class, FName InName=NAME_None, AActor* Owner=NULL, class APawn* Instigator=NULL, FVector Location=FVector(0,0,0), FRotator Rotation=FRotator(0,0,0), AActor* Template=NULL, UBOOL bIsPlayer=0, UBOOL bNoCollisionFail=0, UBOOL bRemoteOwned=0 );
	virtual ABrush*	SpawnBrush();
	virtual void SpawnViewActor( UViewport* Viewport );
	virtual APlayerPawn* SpawnPlayActor( UPlayer* Viewport, ENetRole RemoteRole, const FURL& URL, FString Items, char* Error256 );
	virtual void SetActorZone( AActor* Actor, UBOOL bTest=0, UBOOL bForceRefresh=0 );
	virtual UBOOL FindSpot( FVector Extent, FVector& Location, UBOOL bCheckActors, UBOOL bAssumeFit );
	virtual void AdjustSpot( FVector &Adjusted, FVector TraceDest, FLOAT TraceLen, FCheckResult &Hit );
	virtual UBOOL CheckEncroachment( AActor* Actor, FVector TestLocation, FRotator TestRotation, UBOOL bTouchNotify );
	virtual FPackageMap* GetSandbox();
	virtual UBOOL SinglePointCheck( FCheckResult& Hit, FVector Location, FVector Extent, DWORD ExtraNodeFlags, ALevelInfo* Level, UBOOL bActors );
	virtual UBOOL SingleLineCheck( FCheckResult& Hit, AActor* SourceActor, const FVector& End, const FVector& Start, DWORD TraceFlags, FVector Extent=FVector(0,0,0), BYTE NodeFlags=0 );
	virtual FCheckResult* MultiPointCheck( FMemStack& Mem, FVector Location, FVector Extent, DWORD ExtraNodeFlags, ALevelInfo* Level, UBOOL bActors );
	virtual FCheckResult* MultiLineCheck( FMemStack& Mem, FVector End, FVector Start, FVector Size, UBOOL bCheckActors, ALevelInfo* LevelInfo, BYTE ExtraNodeFlags );
	virtual void InitStats();
	virtual void GetStats( char* Result );
	virtual void DetailChange( UBOOL NewDetail );

	// FNetworkNotify interface.
	EAcceptConnection NotifyAcceptingConnection();
	void NotifyAcceptedConnection( class UNetConnection* Connection );
	UBOOL NotifyAcceptingChannel( class FChannel* Channel );
	ULevel* NotifyGetLevel() {return this;}
	void NotifyReceivedText( UNetConnection* Connection, const char* Text );
	void NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const char* Error );
	UBOOL NotifySendingFile( UNetConnection* Connection, FGuid GUID );

	// Accessors.
	ABrush* Brush()
	{
		guardSlow(ULevel::Brush);
		check(Num()>=2);
		check(Actors(1)!=NULL);
		check(Actors(1)->Brush!=NULL);
		return (ABrush*)Actors(1);
		unguardSlow;
	}
	INT GetActorIndex( AActor* Actor )
	{
		guard(ULevel::GetActorIndex);
		for( int i=0; i<Num(); i++ )
			if( Actors(i) == Actor )
				return i;
		appErrorf( "Actor not found: %s", Actor->GetFullName() );
		return INDEX_NONE;
		unguard;
	}
	ALevelInfo* GetLevelInfo()
	{
		guardSlow(ULevel::GetLevelInfo);
		check(Actors(0));
		check(Actors(0)->IsA(ALevelInfo::StaticClass));
		return (ALevelInfo*)Actors(0);
		unguardSlow;
	}
	AZoneInfo* GetZoneActor( INT iZone )
	{
		guardSlow(ULevel::GetZoneActor);
		return Model->Nodes->Zones[iZone].ZoneActor ? Model->Nodes->Zones[iZone].ZoneActor : GetLevelInfo();
		unguardSlow;
	}
	AActor*& Actors( int i )
	{
		return Element(i);
	}
};

/*-----------------------------------------------------------------------------
	Iterators.
-----------------------------------------------------------------------------*/

//
// Iterate through all static brushes in a level.
//
class FStaticBrushIterator
{
public:
	// Public constructor.
	FStaticBrushIterator( ULevel *InLevel )
	:	Level   ( InLevel   )
	,	Index   ( -1        )
	{
		debug(Level!=NULL);
		++*this;
	}
	void operator++()
	{
		do
		{
			if( ++Index >= Level->Num() )
			{
				Level = NULL;
				break;
			}
		} while
		(	!Level->Actors(Index)
		||	!Level->Actors(Index)->IsStaticBrush() );
	}
	ABrush* operator* ()
	{
		debug(Level);
		debug(Index<Level->Num());
		debug(Level->Actors(Index));
		debug(Level->Actors(Index)->IsStaticBrush());
		return (ABrush*)Level->Actors(Index);
	}
	ABrush* operator-> ()
	{
		debug(Level);
		debug(Index<Level->Num());
		debug(Level->Actors(Index));
		debug(Level->Actors(Index)->IsStaticBrush());
		return (ABrush*)Level->Actors(Index);
	}
	operator UBOOL()
	{
		return Level != NULL;
	}
protected:
	ULevel*		Level;
	INT		    Index;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

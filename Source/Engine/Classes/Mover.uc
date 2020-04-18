//=============================================================================
// The moving brush class.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Mover expands Brush
	intrinsic;

// How the mover should react when it encroaches an actor.
var() enum EMoverEncroachType
{
	ME_StopWhenEncroach,	// Stop when we hit an actor.
	ME_ReturnWhenEncroach,	// Return to previous position when we hit an actor.
   	ME_CrushWhenEncroach,   // Crush the poor helpless actor.
   	ME_IgnoreWhenEncroach,  // Ignore encroached actors.
} MoverEncroachType;

// How the mover moves from one position to another.
var() enum EMoverGlideType
{
	MV_MoveByTime,			// Move linearly.
	MV_GlideByTime,			// Move with smooth acceleration.
} MoverGlideType;

// What classes can bump trigger this mover
var() enum EBumpType
{
	BT_PlayerBump,		// Can only be bumped by player.
	BT_PawnBump,		// Can be bumped by any pawn
	BT_AnyBump,			// Cany be bumped by any solid actor
} BumpType;

//-----------------------------------------------------------------------------
// Keyframe numbers.
var() byte       KeyNum;           // Current or destination keyframe.
var byte         PrevKeyNum;       // Previous keyframe.
var() const byte NumKeys;          // Number of keyframes in total (0-3).
var() const byte WorldRaytraceKey; // Raytrace the world with the brush here.
var() const byte BrushRaytraceKey; // Raytrace the brush here.

//-----------------------------------------------------------------------------
// Movement parameters.
var() float      MoveTime;         // Time to spend moving between keyframes.
var() float      StayOpenTime;     // How long to remain open before closing.
var() float      OtherTime;        // TriggerPound stay-open time.
var() int        EncroachDamage;   // How much to damage encroached actors.

//-----------------------------------------------------------------------------
// Mover state.
var() bool       bTriggerOnceOnly; // Go dormant after first trigger.
var() bool       bSlave;           // This brush is a slave.
var() bool		 bUseTriggered;		// Triggered by player grab
var() bool		 bDamageTriggered;	// Triggered by taking damage
var() bool       bDynamicLightMover; // Apply dynamic lighting to mover.
var() name       PlayerBumpEvent;  // Optional event to cause when the player bumps the mover.
var() name       BumpEvent;			// Optional event to cause when any valid bumper bumps the mover.
var   actor      SavedTrigger;      // Who we were triggered by.
var() float		 DamageThreshold;	// minimum damage to trigger
var	  int		 numTriggerEvents;	// number of times triggered ( count down to untrigger )
var	  Mover		 Leader;			// for having multiple movers return together
var	  Mover		 Follower;
var() name		 ReturnGroup;		// if none, same as tag

//-----------------------------------------------------------------------------
// Audio.
var(MoverSounds) sound      OpeningSound;     // When start opening.
var(MoverSounds) sound      OpenedSound;      // When finished opening.
var(MoverSounds) sound      ClosingSound;     // When start closing.
var(MoverSounds) sound      ClosedSound;      // When finish closing.
var(MoverSounds) sound      MoveAmbientSound; // Optional ambient sound when moving.

//-----------------------------------------------------------------------------
// Internal.
var vector       KeyPos[8];
var rotator      KeyRot[8];
var vector       BasePos, OldPos, OldPrePivot, SavedPos;
var rotator      BaseRot, OldRot, SavedRot;

// AI related
var       NavigationPoint  myMarker;
var		  Actor			TriggerActor;
var		  Actor         TriggerActor2;
var		  Pawn			WaitingPawn;
var		  bool			bOpening;
var		  bool			bPlayerOnly;

function FindTriggerActor()
{
	local Actor A;

	TriggerActor = None;
	TriggerActor2 = None;
	ForEach AllActors(class 'Actor', A)
		if ( A.Event == Tag )
		{
			if ( A.IsA('Counter') || A.IsA('Pawn') )
			{
				bPlayerOnly = true;
				return; //FIXME - handle counters
			}
			if (TriggerActor == None)
				TriggerActor = A;
			else
			{
				TriggerActor2 = A;
				break;
			}
		}

	if ( TriggerActor == None )
	{
		bPlayerOnly = (BumpType == BT_PlayerBump);
		return;
	}

	bPlayerOnly = ( TriggerActor.IsA('Trigger') && (Trigger(TriggerActor).TriggerType == TT_PlayerProximity) );
	if ( bPlayerOnly && ( TriggerActor2 != None) )
	{
		bPlayerOnly = ( TriggerActor2.IsA('Trigger') && (Trigger(TriggerActor).TriggerType == TT_PlayerProximity) );
		if ( !bPlayerOnly )
		{
			A = TriggerActor;
			TriggerActor = TriggerActor2;
			TriggerActor2 = A;
		}
	}
}

/* set specialgoal/movetarget or special pause if necessary
if mover can't be affected by this pawn, return false
Each mover state should implement the appropriate version
*/
function bool HandleDoor(pawn Other)
{
	return false;
}

function Actor SpecialHandling(Pawn Other)
{
	if ( bDamageTriggered )	
	{
		if ( !Other.bCanDoSpecial || (Other.Weapon == None) )
			return None;

		Other.Target = self;
		Other.bShootSpecial = true;
		Other.FireWeapon();
		Other.bFire = 0;
		Other.bAltFire = 0;
		return self;
	}

	if ( BumpType == BT_PlayerBump && !Other.bIsPlayer )
		return None;

	return self;
}

//-----------------------------------------------------------------------------
// Movement functions.

// Interpolate to keyframe KeyNum in Seconds time.
final function InterpolateTo( byte NewKeyNum, float Seconds )
{
	NewKeyNum = Clamp( NewKeyNum, 0, ArrayCount(KeyPos) );
	if( NewKeyNum==PrevKeyNum && KeyNum!=PrevKeyNum )
	{
		// Reverse the movement smoothly.
		PhysAlpha = 1.0 - PhysAlpha;
		OldPos    = BasePos + KeyPos[KeyNum];
		OldRot    = BaseRot + KeyRot[KeyNum];
	}
	else
	{
		// Start a new movement.
		OldPos    = Location;
		OldRot    = Rotation;
		PhysAlpha = 0.0;
	}

	// Setup physics.
	SetPhysics(PHYS_MovingBrush);
	bInterpolating   = true;
	PrevKeyNum       = KeyNum;
	KeyNum			 = NewKeyNum;
	PhysRate         = 1.0 / Seconds;
}

// Set the specified keyframe.
final function SetKeyframe( byte NewKeyNum, vector NewLocation, rotator NewRotation )
{
	KeyNum         = Clamp( NewKeyNum, 0, ArrayCount(KeyPos) );
	KeyPos[KeyNum] = NewLocation;
	KeyRot[KeyNum] = NewRotation;
}

// Interpolation ended.
function InterpolateEnd( actor Other )
{
	local byte OldKeyNum;

	OldKeyNum  = PrevKeyNum;
	PrevKeyNum = KeyNum;
	PhysAlpha  = 0;

	// If more than two keyframes, chain them.
	if( KeyNum>0 && KeyNum<OldKeyNum )
	{
		// Chain to previous.
		InterpolateTo(KeyNum-1,MoveTime);
	}
	else if( KeyNum<NumKeys-1 && KeyNum>OldKeyNum )
	{
		// Chain to next.
		InterpolateTo(KeyNum+1,MoveTime);
	}
	else
	{
		// Finished interpolating.
		AmbientSound = None;
	}
}

//-----------------------------------------------------------------------------
// Mover functions.

// Handle when the mover finishes closing.
function FinishedClosing()
{
	// Update sound effects.
	PlaySound( ClosedSound, SLOT_None );

	// Notify our triggering actor that we have completed.
	if( SavedTrigger != None )
		SavedTrigger.EndEvent();
	SavedTrigger = None;
	Instigator = None;
}

// Handle when the mover finishes opening.
function FinishedOpening()
{
	local actor A;

	// Update sound effects.
	PlaySound( OpenedSound, SLOT_None );
	
	// Trigger any chained movers.
	if( Event != '' )
		foreach AllActors( class 'Actor', A, Event )
			A.Trigger( Self, Instigator );

	if ( WaitingPawn != None )
	{
		WaitingPawn.StopWaiting();
		if ( (WaitingPawn.SpecialGoal == self) || (WaitingPawn.SpecialGoal == myMarker) )
			WaitingPawn.SpecialGoal = None; 
		WaitingPawn = None;
	}
}

// Open the mover.
function DoOpen()
{
	bOpening = true;
	InterpolateTo( 1, MoveTime );
	PlaySound( OpeningSound, SLOT_None );
	AmbientSound = MoveAmbientSound;
}

// Close the mover.
function DoClose()
{
	local actor A;

	bOpening = false;
	InterpolateTo( Max(0,KeyNum-1), MoveTime );
	PlaySound( ClosingSound, SLOT_None );
	if( Event != '' )
		foreach AllActors( class 'Actor', A, Event )
			A.UnTrigger( Self, Instigator );
	AmbientSound = MoveAmbientSound;
}

//-----------------------------------------------------------------------------
// Engine notifications.

// When mover enters gameplay.
function BeginPlay()
{
	local rotator R;

	// Init key info.
	Super.BeginPlay();
	KeyNum         = Clamp( KeyNum, 0, ArrayCount(KeyPos) );
	PhysAlpha      = 0.0;

	// Set initial location.
	Move( BasePos + KeyPos[KeyNum] - Location );

	// Initial rotation.
	SetRotation( BaseRot + KeyRot[KeyNum] );

	// find movers in same group
	if ( ReturnGroup == '' )
		ReturnGroup = tag;
}

// Immediately after mover enters gameplay.
function PostBeginPlay()
{
	local mover M;

	//brushes can't be deleted, so if not relevant, make it invisible and non-colliding
	if ( !Level.Game.IsRelevant(self) )
	{
		bHidden = true;
		DrawType = DT_None;
		SetCollision(false, false, false);
		SetLocation(Location + vect(0,0,20000)); // temp since still in bsp
	}
	else
	{
		FindTriggerActor();
		// Initialize all slaves.
		if( !bSlave )
		{
			foreach AllActors( class 'Mover', M, Tag )
			{
				if( M.bSlave )
				{
					M.GotoState('');
					M.SetBase( Self );
				}
			}
		}
		if ( Leader == None )
		{	
			Leader = self;
			ForEach AllActors( class'Mover', M )
				if ( (M != self) && (M.ReturnGroup == ReturnGroup) )
				{
					M.Leader = self;
					M.Follower = Follower;
					Follower = M;
				}
		}
	}
}

function MakeGroupStop()
{
	// Stop moving immediately.
	bInterpolating = false;
	AmbientSound = None;
	GotoState( , '' );

	if ( Follower != None )
		Follower.MakeGroupStop();
}

function MakeGroupReturn()
{
	// Abort move and reverse course.
	bInterpolating = false;
	AmbientSound = None;
	if( KeyNum<PrevKeyNum )
		GotoState( , 'Open' );
	else
		GotoState( , 'Close' );

	if ( Follower != None )
		Follower.MakeGroupReturn();
}
		
// Return true to abort, false to continue.
function bool EncroachingOn( actor Other )
{
	if ( Other.IsA('Carcass') || Other.IsA('Decoration') )
	{
		Other.TakeDamage(10000, None, Other.Location, vect(0,0,0), 'Crushed');
		return false;
	}
	if ( Other.IsA('Fragment') )
	{
		Other.Destroy();
		return false;
	}

	// Damage the encroached actor.
	if( EncroachDamage != 0 )
		Other.TakeDamage( EncroachDamage, Instigator, Other.Location, vect(0,0,0), 'Crushed' );

	// If we have a bump-player event, and Other is a pawn, do the bump thing.
	if( Pawn(Other)!=None && Pawn(Other).bIsPlayer && PlayerBumpEvent!='' )
		Bump( Other );

	// Stop, return, or whatever.
	if( MoverEncroachType == ME_StopWhenEncroach )
	{
		Leader.MakeGroupStop();
		return true;
	}
	else if( MoverEncroachType == ME_ReturnWhenEncroach )
	{
		Leader.MakeGroupReturn();
		if ( Other.IsA('Pawn') )
		{
			if ( Pawn(Other).bIsPlayer )
				Pawn(Other).PlaySound(Pawn(Other).Land, SLOT_Talk);
			else
				Pawn(Other).PlaySound(Pawn(Other).HitSound1, SLOT_Talk);
		}	
		return true;
	}
	else if( MoverEncroachType == ME_CrushWhenEncroach )
	{
		// Kill it.
		Other.KilledBy( Instigator );
		return false;
	}
	else if( MoverEncroachType == ME_IgnoreWhenEncroach )
	{
		// Ignore it.
		return false;
	}
}

// When bumped by player.
function Bump( actor Other )
{
	local actor A;
	local pawn  P;

	P = Pawn(Other);
	if ( (BumpType != BT_AnyBump) && (P == None) )
		return;
	if ( (BumpType == BT_PlayerBump) && !P.bIsPlayer )
		return;
	if ( (BumpType == BT_PawnBump) && (Other.Mass < 10) )
		return;
	if( BumpEvent!='' )
		foreach AllActors( class 'Actor', A, BumpEvent )
			A.Trigger( Self, P );

	if( P!=None && P.bIsPlayer && PlayerBumpEvent!='' )
		foreach AllActors( class 'Actor', A, PlayerBumpEvent )
			A.Trigger( Self, P );
	if ( Other.IsA('Pawn') && (Pawn(Other).SpecialGoal == self) )
		Pawn(Other).SpecialGoal = None;
}

// When damaged
function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
{
	if ( bDamageTriggered && (Damage >= DamageThreshold) )
		self.Trigger(self, instigatedBy);
}

//-----------------------------------------------------------------------------
// Trigger states.

// When triggered, open, wait, then close.
state() TriggerOpenTimed
{
	function bool HandleDoor(pawn Other)
	{
		if ( bOpening )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			return true;
		}
		if ( bUseTriggered )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			Trigger(Other, Other);
			return true;
		}
		if ( bPlayerOnly && !Other.bIsPlayer )
			return false;
		if ( (BumpEvent == tag) || (Other.bIsPlayer && (PlayerBumpEvent == tag)) )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			return true;
		}
		if ( bDamageTriggered )
		{
			WaitingPawn = Other;
			Other.SpecialGoal = self;
			if ( !Other.bCanDoSpecial || (Other.Weapon == None) )
				return false;

			Other.Target = self;
			Other.bShootSpecial = true;
			Other.FireWeapon();
			Other.bFire = 0;
			Other.bAltFire = 0;
			return true;
		}

		if ( (TriggerActor != None) && (!TriggerActor.IsA('Trigger') || Trigger(TriggerActor).IsRelevant(Other)) )
		{
			if ( (TriggerActor2 != None) && (!TriggerActor2.IsA('Trigger') || Trigger(TriggerActor2).IsRelevant(Other)) 
				&& (VSize(TriggerActor2.Location - Other.Location) < VSize(TriggerActor.Location - Other.Location)) )
			{
				Other.SpecialGoal = TriggerActor2;
				Other.MoveTarget = TriggerActor2;
				return True;
			}
			Other.SpecialGoal = TriggerActor;
			Other.MoveTarget = TriggerActor;
			return True;
		}			
		return false;
	}

	function Trigger( actor Other, pawn EventInstigator )
	{
		SavedTrigger = Other;
		Instigator = EventInstigator;
		if ( SavedTrigger != None )
			SavedTrigger.BeginEvent();
		GotoState( 'TriggerOpenTimed', 'Open' );
	}

	function BeginState()
	{
		bOpening = false;
	}

Open:
	Disable( 'Trigger' );
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	Sleep( StayOpenTime );
	if( bTriggerOnceOnly )
		GotoState('');
Close:
	DoClose();
	FinishInterpolation();
	FinishedClosing();
	Enable( 'Trigger' );
}

// Toggle when triggered.
state() TriggerToggle
{
	function bool HandleDoor(pawn Other)
	{
		if ( bOpening )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			return true;
		}
		if ( bUseTriggered )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			Trigger(Other, Other);
			return true;
		}
		if ( bPlayerOnly && !Other.bIsPlayer )
			return false;
		if ( (BumpEvent == tag) || (Other.bIsPlayer && (PlayerBumpEvent == tag)) )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			return true;
		}
		if ( bDamageTriggered )
		{
			WaitingPawn = Other;
			Other.SpecialGoal = self;
			if ( !Other.bCanDoSpecial || (Other.Weapon == None) )
				return false;

			Other.Target = self;
			Other.bShootSpecial = true;
			Other.FireWeapon();
			Other.bFire = 0;
			Other.bAltFire = 0;
			return true;
		}
		if ( (TriggerActor != None) && (!TriggerActor.IsA('Trigger') || Trigger(TriggerActor).IsRelevant(Other)) )
		{
			if ( (TriggerActor2 != None) && (!TriggerActor2.IsA('Trigger') || Trigger(TriggerActor2).IsRelevant(Other)) 
				&& (VSize(TriggerActor2.Location - Other.Location) < VSize(TriggerActor.Location - Other.Location)) )
			{
				Other.SpecialGoal = TriggerActor2;
				Other.MoveTarget = TriggerActor2;
				return True;
			}
			Other.SpecialGoal = TriggerActor;
			Other.MoveTarget = TriggerActor;
			return True;
		}			
		return false;
	}
	
	function Trigger( actor Other, pawn EventInstigator )
	{
		SavedTrigger = Other;
		Instigator = EventInstigator;
		if ( SavedTrigger != None )
			SavedTrigger.BeginEvent();
		if( KeyNum==0 || KeyNum<PrevKeyNum )
			GotoState( 'TriggerToggle', 'Open' );
		else
			GotoState( 'TriggerToggle', 'Close' );
	}
Open:
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	SavedTrigger.EndEvent();
	Stop;
Close:		
	DoClose();
	FinishInterpolation();
	FinishedClosing();
}

// Open when triggered, close when get untriggered.
state() TriggerControl
{
	function bool HandleDoor(pawn Other)
	{
		if ( bOpening )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			return true;
		}
		if ( bUseTriggered )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			Trigger(Other, Other);
			return true;
		}
		if ( bPlayerOnly && !Other.bIsPlayer )
			return false;
		if ( (BumpEvent == tag) || (Other.bIsPlayer && (PlayerBumpEvent == tag)) )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			return true;
		}
		if ( bDamageTriggered )
		{
			WaitingPawn = Other;
			Other.SpecialGoal = self;
			if ( !Other.bCanDoSpecial || (Other.Weapon == None) )
				return false;

			Other.Target = self;
			Other.bShootSpecial = true;
			Other.FireWeapon();
			Other.bFire = 0;
			Other.bAltFire = 0;
			return true;
		}
		if ( (TriggerActor != None) && (!TriggerActor.IsA('Trigger') || Trigger(TriggerActor).IsRelevant(Other)) )
		{
			if ( (TriggerActor2 != None) && (!TriggerActor2.IsA('Trigger') || Trigger(TriggerActor2).IsRelevant(Other)) 
				&& (VSize(TriggerActor2.Location - Other.Location) < VSize(TriggerActor.Location - Other.Location)) )
			{
				Other.SpecialGoal = TriggerActor2;
				Other.MoveTarget = TriggerActor2;
				return True;
			}
			Other.SpecialGoal = TriggerActor;
			Other.MoveTarget = TriggerActor;
			return True;
		}			
		return false;
	}

	function Trigger( actor Other, pawn EventInstigator )
	{
		numTriggerEvents++;
		SavedTrigger = Other;
		Instigator = EventInstigator;
		if ( SavedTrigger != None )
			SavedTrigger.BeginEvent();
		GotoState( 'TriggerControl', 'Open' );
	}
	function UnTrigger( actor Other, pawn EventInstigator )
	{
		numTriggerEvents--;
		if ( numTriggerEvents <=0 )
		{
			numTriggerEvents = 0;
			SavedTrigger = Other;
			Instigator = EventInstigator;
			SavedTrigger.BeginEvent();
			GotoState( 'TriggerControl', 'Close' );
		}
	}

	function BeginState()
	{
		numTriggerEvents = 0;
	}

Open:
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	SavedTrigger.EndEvent();
	if( bTriggerOnceOnly )
		GotoState('');
	Stop;
Close:		
	DoClose();
	FinishInterpolation();
	FinishedClosing();
}

// Start pounding when triggered.
state() TriggerPound
{
	function bool HandleDoor(pawn Other)
	{
		if ( bOpening )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			return true;
		}
		if ( bUseTriggered )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			Trigger(Other, Other);
			return true;
		}
		if ( bPlayerOnly && !Other.bIsPlayer )
			return false;
		if ( (BumpEvent == tag) || (Other.bIsPlayer && (PlayerBumpEvent == tag)) )
		{
			WaitingPawn = Other;
			Other.SpecialPause = 2.5;
			return true;
		}
		if ( bDamageTriggered )
		{
			WaitingPawn = Other;
			Other.SpecialGoal = self;
			if ( !Other.bCanDoSpecial || (Other.Weapon == None) )
				return false;

			Other.Target = self;
			Other.bShootSpecial = true;
			Other.FireWeapon();
			Other.bFire = 0;
			Other.bAltFire = 0;
			return true;
		}
		if ( (TriggerActor != None) && (!TriggerActor.IsA('Trigger') || Trigger(TriggerActor).IsRelevant(Other)) )
		{
			if ( (TriggerActor2 != None) && (!TriggerActor2.IsA('Trigger') || Trigger(TriggerActor2).IsRelevant(Other)) 
				&& (VSize(TriggerActor2.Location - Other.Location) < VSize(TriggerActor.Location - Other.Location)) )
			{
				Other.SpecialGoal = TriggerActor2;
				Other.MoveTarget = TriggerActor2;
				return True;
			}
			Other.SpecialGoal = TriggerActor;
			Other.MoveTarget = TriggerActor;
			return True;
		}			
		return false;
	}

	function Trigger( actor Other, pawn EventInstigator )
	{
		numTriggerEvents++;
		SavedTrigger = Other;
		Instigator = EventInstigator;
		GotoState( 'TriggerPound', 'Open' );
	}
	function UnTrigger( actor Other, pawn EventInstigator )
	{
		numTriggerEvents--;
		if ( numTriggerEvents <= 0 )
		{
			numTriggerEvents = 0;
			SavedTrigger = None;
			Instigator = None;
			GotoState( 'TriggerPound', 'Close' );
		}
	}
	function BeginState()
	{
		numTriggerEvents = 0;
	}

Open:
	DoOpen();
	FinishInterpolation();
	Sleep(OtherTime);
Close:
	DoClose();
	FinishInterpolation();
	Sleep(StayOpenTime);
	if( bTriggerOnceOnly )
		GotoState('');
	if( SavedTrigger != None )
		goto 'Open';
}

//-----------------------------------------------------------------------------
// Bump states.

// Open when bumped, wait, then close.
state() BumpOpenTimed
{
	function bool HandleDoor(pawn Other)
	{
		if ( (BumpType == BT_PlayerBump) && !Other.bIsPlayer )
			return false;

		Bump(Other);
		WaitingPawn = Other;
		Other.SpecialPause = 2.5;
		return true;
	}

	function Bump( actor Other )
	{
		if ( (BumpType != BT_AnyBump) && (Pawn(Other) == None) )
			return;
		if ( (BumpType == BT_PlayerBump) && !Pawn(Other).bIsPlayer )
			return;
		if ( (BumpType == BT_PawnBump) && (Other.Mass < 10) )
			return;
		Global.Bump( Other );
		SavedTrigger = None;
		Instigator = Pawn(Other);
		GotoState( 'BumpOpenTimed', 'Open' );
	}
Open:
	Disable( 'Bump' );
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	Sleep( StayOpenTime );
	if( bTriggerOnceOnly )
		GotoState('');
Close:
	DoClose();
	FinishInterpolation();
	FinishedClosing();
	Enable( 'Bump' );
}

// Open when bumped, close when reset.
state() BumpButton
{
	function bool HandleDoor(pawn Other)
	{
		if ( (BumpType == BT_PlayerBump) && !Other.bIsPlayer )
			return false;

		Bump(Other);
		return false; //let pawn try to move around this button
	}

	function Bump( actor Other )
	{
		if ( (BumpType != BT_AnyBump) && (Pawn(Other) == None) )
			return;
		if ( (BumpType == BT_PlayerBump) && !Pawn(Other).bIsPlayer )
			return;
		if ( (BumpType == BT_PawnBump) && (Other.Mass < 10) )
			return;
		Global.Bump( Other );
		SavedTrigger = Other;
		Instigator = Pawn( Other );
		GotoState( 'BumpButton', 'Open' );
	}
	function BeginEvent()
	{
		bSlave=true;
	}
	function EndEvent()
	{
		bSlave     = false;
		Instigator = None;
		GotoState( 'BumpButton', 'Close' );
	}
Open:
	Disable( 'Bump' );
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	if( bTriggerOnceOnly )
		GotoState('');
	if( bSlave )
		Stop;
Close:
	DoClose();
	FinishInterpolation();
	FinishedClosing();
	Enable( 'Bump' );
}

//-----------------------------------------------------------------------------
// Stand states.

// Open when stood on, wait, then close.
state() StandOpenTimed
{
	function bool HandleDoor(pawn Other)
	{
		if ( bPlayerOnly && !Other.bIsPlayer )
			return false;
		Other.SpecialPause = 2.5;
		WaitingPawn = Other;
		if ( Other.Base == self )
			Attach(Other);
		return true;
	}

	function Attach( actor Other )
	{
		local pawn  P;

		P = Pawn(Other);
		if ( (BumpType != BT_AnyBump) && (P == None) )
			return;
		if ( (BumpType == BT_PlayerBump) && !P.bIsPlayer )
			return;
		if ( (BumpType == BT_PawnBump) && (Other.Mass < 10) )
			return;
		SavedTrigger = None;
		GotoState( 'StandOpenTimed', 'Open' );
	}
Open:
	Disable( 'Attach' );
	DoOpen();
	FinishInterpolation();
	FinishedOpening();
	Sleep( StayOpenTime );
	if( bTriggerOnceOnly )
		GotoState('');
Close:
	DoClose();
	FinishInterpolation();
	FinishedClosing();
	Enable( 'Attach' );
}

defaultproperties
{
     MoverEncroachType=ME_ReturnWhenEncroach
     MoverGlideType=MV_GlideByTime
     NumKeys=2
	 BrushRaytraceKey=0
     MoveTime=+00001.000000
     StayOpenTime=+00004.000000
     DefaultEdCategory=Mover
     bStatic=False
	 bDynamicLightMover=False
	 bIsMover=True
     CollisionRadius=+00160.000000
     CollisionHeight=+00160.000000
     bCollideActors=True
     bBlockActors=True
     bBlockPlayers=True
     Physics=PHYS_MovingBrush
     State=BumpOpenTimed
     InitialState=BumpOpenTimed
	 TransientSoundVolume=+00003.000000
     SoundVolume=228
	 NetPriority=7
}

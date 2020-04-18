//=============================================================================
// Trigger: senses things happening in its proximity and generates 
// sends Trigger/UnTrigger to actors whose names match 'EventName'.
//=============================================================================
class Trigger expands Triggers
	localized
	intrinsic;

#exec Texture Import File=Textures\Trigger.pcx Name=S_Trigger Mips=Off Flags=2

//-----------------------------------------------------------------------------
// Trigger variables.

// Trigger type.
var() enum ETriggerType
{
	TT_PlayerProximity,	// Trigger is activated by player proximity.
	TT_PawnProximity,	// Trigger is activated by any pawn's proximity
	TT_ClassProximity,	// Trigger is activated by actor of that class only
	TT_AnyProximity,    // Trigger is activated by any actor in proximity.
	TT_Shoot,		    // Trigger is activated by player shooting it.
} TriggerType;

// Human readable triggering message.
var() localized string[80] Message;

// Only trigger once and then go dormant.
var() bool bTriggerOnceOnly;

// For triggers that are activated/deactivated by other triggers.
var() bool bInitiallyActive;

var() class<actor> ClassProximityType;

var() float	RepeatTriggerTime; //if > 0, repeat trigger message at this interval is still touching other
var() float ReTriggerDelay; //minimum time before trigger can be triggered again
var	  float TriggerTime;
var() float DamageThreshold; //minimum damage to trigger if TT_Shoot

// AI vars
var	actor TriggerActor;	// actor that triggers this trigger
var actor TriggerActor2;

//=============================================================================
// AI related functions

function PostBeginPlay()
{
	if ( !bInitiallyActive )
		FindTriggerActor();
	if ( TriggerType == TT_Shoot )
	{
		bHidden = false;
		bProjTarget = true;
		DrawType = DT_None;
	}
		
	Super.PostBeginPlay();
}

function FindTriggerActor()
{
	local Actor A;

	TriggerActor = None;
	TriggerActor2 = None;
	ForEach AllActors(class 'Actor', A)
		if ( A.Event == Tag)
		{
			if ( Counter(A) != None )
				return; //FIXME - handle counters
			if (TriggerActor == None)
				TriggerActor = A;
			else
			{
				TriggerActor2 = A;
				return;
			}
		}
}

function Actor SpecialHandling(Pawn Other)
{
	local int i;

	if ( bTriggerOnceOnly && !bCollideActors )
		return None;

	if ( (TriggerType == TT_PlayerProximity) && !Other.bIsPlayer )
		return None;

	if ( !bInitiallyActive )
	{
		if ( TriggerActor == None )
			FindTriggerActor();
		if ( TriggerActor == None )
			return None;
		if ( (TriggerActor2 != None) 
			&& (VSize(TriggerActor2.Location - Other.Location) < VSize(TriggerActor.Location - Other.Location)) )
			return TriggerActor2;
		else
			return TriggerActor;
	}

	// is this a shootable trigger?
	if ( TriggerType == TT_Shoot )
	{
		if ( !Other.bCanDoSpecial || (Other.Weapon == None) )
			return None;

		Other.Target = self;
		Other.bShootSpecial = true;
		Other.FireWeapon();
		Other.bFire = 0;
		Other.bAltFire = 0;
		return Other;
	}

	// can other trigger it right away?
	if ( IsRelevant(Other) )
	{
		for (i=0;i<4;i++)
			if (Touching[i] == Other)
				Touch(Other);
		return self;
	}

	return self;
}

// when trigger gets turned on, check its touch list

function CheckTouchList()
{
	local int i;

	for (i=0;i<4;i++)
		if ( Touching[i] != None )
			Touch(Touching[i]);
}

//=============================================================================
// Trigger states.

// Trigger is always active.
state() NormalTrigger
{
}

// Other trigger toggles this trigger's activity.
state() OtherTriggerToggles
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		bInitiallyActive = !bInitiallyActive;
		if ( bInitiallyActive )
			CheckTouchList();
	}
}

// Other trigger turns this on.
state() OtherTriggerTurnsOn
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		local bool bWasActive;

		bWasActive = bInitiallyActive;
		bInitiallyActive = true;
		if ( !bWasActive )
			CheckTouchList();
	}
}

// Other trigger turns this off.
state() OtherTriggerTurnsOff
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		bInitiallyActive = false;
	}
}

//=============================================================================
// Trigger logic.

//
// See whether the other actor is relevant to this trigger.
//
final function bool IsRelevant( actor Other )
{
	if( !bInitiallyActive )
		return false;
	switch( TriggerType )
	{
		case TT_PlayerProximity:
			return Pawn(Other)!=None && Pawn(Other).bIsPlayer;
		case TT_PawnProximity:
			return Pawn(Other)!=None && ( Pawn(Other).Intelligence > BRAINS_None );
		case TT_ClassProximity:
			return ClassIsChildOf(Other.Class, ClassProximityType);
		case TT_AnyProximity:
			return true;
		case TT_Shoot:
			return ( (Projectile(Other) != None) && (Projectile(Other).Damage >= DamageThreshold) );
	}
}
//
// Called when something touches the trigger.
//
function Touch( actor Other )
{
	local actor A;
	if( IsRelevant( Other ) )
	{
		if ( ReTriggerDelay > 0 )
		{
			if ( Level.TimeSeconds - TriggerTime < ReTriggerDelay )
				return;
			TriggerTime = Level.TimeSeconds;
		}
		// Broadcast the Trigger message to all matching actors.
		if( Event != '' )
			foreach AllActors( class 'Actor', A, Event )
				A.Trigger( Other, Other.Instigator );

		if ( Other.IsA('Pawn') && (Pawn(Other).SpecialGoal == self) )
			Pawn(Other).SpecialGoal = None;
				
		if( Message != "" )
			// Send a string message to the toucher.
			Other.Instigator.ClientMessage( Message );

		if( bTriggerOnceOnly )
			// Ignore future touches.
			SetCollision(False);
		else if ( RepeatTriggerTime > 0 )
			SetTimer(RepeatTriggerTime, false);
	}
}

function Timer()
{
	local bool bKeepTiming;
	local int i;

	bKeepTiming = false;

	for (i=0;i<4;i++)
		if ( (Touching[i] != None) && IsRelevant(Touching[i]) )
		{
			bKeepTiming = true;
			Touch(Touching[i]);
		}

	if ( bKeepTiming )
		SetTimer(RepeatTriggerTime, false);
}

function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
{
	local actor A;

	if ( bInitiallyActive && (TriggerType == TT_Shoot) && (Damage >= DamageThreshold) && (instigatedBy != None) )
	{
		if ( ReTriggerDelay > 0 )
		{
			if ( Level.TimeSeconds - TriggerTime < ReTriggerDelay )
				return;
			TriggerTime = Level.TimeSeconds;
		}
		// Broadcast the Trigger message to all matching actors.
		if( Event != '' )
			foreach AllActors( class 'Actor', A, Event )
				A.Trigger( instigatedBy, instigatedBy );

		if( Message != "" )
			// Send a string message to the toucher.
			instigatedBy.Instigator.ClientMessage( Message );

		if( bTriggerOnceOnly )
			// Ignore future touches.
			SetCollision(False);
	}
}

//
// When something untouches the trigger.
//
function UnTouch( actor Other )
{
	local actor A;
	if( IsRelevant( Other ) )
	{
		// Untrigger all matching actors.
		if( Event != '' )
			foreach AllActors( class 'Actor', A, Event )
				A.UnTrigger( Other, Other.Instigator );
	}
}

defaultproperties
{
	 Texture=S_Trigger
     bInitiallyActive=True
     State=NormalTrigger
     InitialState=NormalTrigger
}

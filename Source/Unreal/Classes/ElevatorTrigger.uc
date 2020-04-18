//=============================================================================
// ElevatorTrigger.
//=============================================================================
class ElevatorTrigger expands Triggers;

// A special trigger devised for the ElevatorMover class, since
// detecting one trigger message is not enough to determine 2 or more
// different commands (like up/down).  When an actor is within its'
// radius, it sends a message to the ElevatorMover with the desired
// keyframe change and moving time interval.

var() int 	GotoKeyframe;
var() float	MoveTime;
var() bool   bTriggerOnceOnly;
var() class<actor> ClassProximityType;

// Trigger type.
var() enum ETriggerType
{
	TT_PlayerProximity,	// Trigger is activated by player proximity.
	TT_PawnProximity,	// Trigger is activated by any pawn's proximity
	TT_ClassProximity,	// Trigger is activated by actor of that class only
	TT_AnyProximity,    // Trigger is activated by any actor in proximity.
	TT_Shoot,		    // Trigger is activated by player shooting it.
} TriggerType;

//
// See whether the other actor is relevant to this trigger.
//
final function bool IsRelevant( actor Other )
{
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
			return ( Projectile(Other) != None );
	}
}
//
// Called when something touches the trigger.
//
function Touch( actor Other )
{
	local ElevatorMover EM;
	if( IsRelevant( Other ) )
	{
		// Call the ElevatorMover's Move function
		if( Event != '' )
			foreach AllActors( class 'ElevatorMover', EM, Event )
				EM.MoveKeyframe( GotoKeyFrame, MoveTime );

		if( bTriggerOnceOnly )
			// Ignore future touches.
			SetCollision(False);
	}
}

defaultproperties
{
}

//=============================================================================
// Jumper.
// Creatures will jump on hitting this trigger in direction specified
//=============================================================================
class Jumper expands Triggers;

var() bool bOnceOnly;
var() class<scriptedPawn> LimitedToClass;
var ScriptedPawn Pending;
var() float JumpZ;

function Timer()
{
	Pending.SetPhysics(PHYS_Falling);
	Pending.Velocity = Pending.GroundSpeed * Vector(Rotation);
	if ( JumpZ != 0 )
		Pending.Velocity.Z = JumpZ;
	else
		Pending.Velocity.Z = FMax(100, Pending.JumpZ);
	Pending.DesiredRotation = Rotation;
	Pending.bJumpOffPawn = true;
	Pending.SetFall();
}

function Touch( actor Other )
{
	if ( Other.IsA('ScriptedPawn') 
			&& ((LimitedToClass == None) || (Other.Class == LimitedToClass)) )
	{
		Pending = ScriptedPawn(Other);
		SetTimer(0.01, false);
		if ( bOnceOnly )
			Disable('Touch');
	}
}

defaultproperties
{
     bDirectional=True
}
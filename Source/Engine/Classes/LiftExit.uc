//=============================================================================
// LiftExit.
//=============================================================================
class LiftExit expands NavigationPoint
	intrinsic;

var() name LiftTag;
var	 mover MyLift;

function PostBeginPlay()
{
	if ( LiftTag != '' )
		ForEach AllActors(class'Mover', MyLift, LiftTag )
			break;
	Super.PostBeginPlay();
}

/* SpecialHandling is called by the navigation code when the next path has been found.  
It gives that path an opportunity to modify the result based on any special considerations
*/

function Actor SpecialHandling(Pawn Other)
{
	if ( Other.Base == MyLift )
	{
		if ( (self.Location.Z < Other.Location.Z + Other.CollisionHeight)
			 && Other.LineOfSightTo(self) )
			return self;
		Other.SpecialGoal = None;
		MyLift.HandleDoor(Other);
		if ( (Other.SpecialGoal == MyLift) || (Other.SpecialGoal == None) )
			Other.SpecialGoal = MyLift.myMarker;
		return Other.SpecialGoal;
	}
	return self;
}

defaultproperties
{
}

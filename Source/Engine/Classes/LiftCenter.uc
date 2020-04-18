//=============================================================================
// LiftCenter.
//=============================================================================
class LiftCenter expands NavigationPoint
	intrinsic;

var() name LiftTag;
var	 mover MyLift;

function PostBeginPlay()
{
	if ( LiftTag != '' )
		ForEach AllActors(class'Mover', MyLift, LiftTag )
		{
			MyLift.myMarker = self;
			SetBase(MyLift);
			break;
		}
	Super.PostBeginPlay();
}

/* SpecialHandling is called by the navigation code when the next path has been found.  
It gives that path an opportunity to modify the result based on any special considerations
*/

function Actor SpecialHandling(Pawn Other)
{
	if ( (self.Location.Z < Other.Location.Z + Other.CollisionHeight)
		&& (VSize(Location - Other.Location) < 300)
		&& Other.LineOfSightTo(self) ) 
		return self;
	if ( MyLift.BumpType == BT_PlayerBump && !Other.bIsPlayer )
		return None;
	Other.SpecialGoal = None;
	MyLift.HandleDoor(Other);
	if ( (Other.SpecialGoal == MyLift) || (Other.SpecialGoal == None) )
		Other.SpecialGoal = self;
	return Other.SpecialGoal;
}

defaultproperties
{
	bStatic=false
}
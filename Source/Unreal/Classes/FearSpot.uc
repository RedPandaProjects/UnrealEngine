//=============================================================================
// FearSpot.
// Creatures will tend to back away when entering this spot
// To be effective, there should also not be any paths going through the area
//=============================================================================
class FearSpot expands Triggers;

var() bool bInitiallyActive;

function Touch( actor Other )
{
	if ( bInitiallyActive && Other.IsA('ScriptedPawn') )
		ScriptedPawn(Other).FearThisSpot(self);
}

function Trigger( actor Other, pawn EventInstigator )
{
	bInitiallyActive = !bInitiallyActive;
}

defaultproperties
{
}
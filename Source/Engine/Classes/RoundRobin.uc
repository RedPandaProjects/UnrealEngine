//=============================================================================
// RoundRobin: Each time it's triggered, it advances through a list of
// outgoing events.
//=============================================================================
class RoundRobin expands Triggers;

var() name OutEvents[16]; // Events to generate.
var() bool bLoop;         // Whether to loop when get to end.
var int i;                // Internal counter.

//
// When RoundRobin is triggered...
//
function Trigger( actor Other, pawn EventInstigator )
{
	local actor A;
	if( OutEvents[i] != '' )
	{
		foreach AllActors( class 'Actor', A, OutEvents[i] )		
		{
			A.Trigger( Self, EventInstigator );
		}
		if( ++i>=ArrayCount(OutEvents) || OutEvents[i]=='' )
		{
			if( bLoop ) i=0;
			else
				SetCollision(false,false,false);
		}
	}
}

defaultproperties
{
}

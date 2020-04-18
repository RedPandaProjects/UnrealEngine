//=============================================================================
// MakeNaliFriendly
//  makes all fearful Nali friendly again.
// Use this when player "helps" nali, to make up for earlier killing one
// "accidentally"
//=============================================================================
class MakeNaliFriendly expands Keypoint;

function Trigger(actor Other, pawn EventInstigator)
{
	local Pawn aPawn;

	if ( EventInstigator.bIsPlayer ) 
	{
		aPawn = Level.PawnList;
		while ( aPawn != None )
		{
			if ( aPawn.IsA('Nali') )
				aPawn.AttitudeToPlayer = ATTITUDE_Friendly;
			aPawn = aPawn.NextPawn;
		}
	}
}

defaultproperties
{
	bCollideActors=true
}
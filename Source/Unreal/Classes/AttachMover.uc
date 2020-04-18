//=============================================================================
// AttachMover.
//=============================================================================
class AttachMover expands Mover;

// Allows attachment of actors to this mover, so that they will move
// as the mover moves, keeping their relative position to the mover.
// The relative positions are determined by the positions of the actors
// during the first keyframe (0) of the mover.
// The Tag of the actors and the AttachTag of this mover must be the same
// in order for actors to become attached.

var() name AttachTag;

// Immediately after mover enters gameplay.
function PostBeginPlay()
{
	local Actor Act;
	local Mover Mov;

	// Initialize all slaves.
	if ( AttachTag != '' )
		foreach AllActors( class 'Actor', Act, AttachTag )
		{
			Mov = Mover(Act);
			if (Mov == None) {

				Act.SetBase( Self );
			}
			else if (Mov.bSlave) {
			
				Mov.GotoState('');
				Mov.SetBase( Self );
			}
		}
}

defaultproperties
{
}

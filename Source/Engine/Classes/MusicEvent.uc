//=============================================================================
// MusicEvent.
//=============================================================================
class MusicEvent expands Triggers;

// Variables.
var() music            Song;
var() byte             SongSection;
var() byte             CdTrack;
var() EMusicTransition Transition;
var() bool             bSilence;
var() bool             bOnceOnly;
var() bool             bAffectAllPlayers;

// When gameplay starts.
function BeginPlay()
{
	if( Song==None )
	{
		Song = Level.Song;
	}
	if( bSilence )
	{
		SongSection = 255;
		CdTrack     = 255;
	}
}

// When triggered.
function Trigger( actor Other, pawn EventInstigator )
{
	local PlayerPawn P;
	local Pawn A;

	if( bAffectAllPlayers )
	{
		A = Level.PawnList;
		While ( A != None )
		{
			if ( A.IsA('PlayerPawn') )
				PlayerPawn(A).ClientSetMusic( Song, SongSection, CdTrack, Transition );
			A = A.nextPawn;
		}
	}
	else
	{
		// Only affect the one player.
		P = PlayerPawn(EventInstigator);
		if( P==None )
			return;
			
		// Go to music.
		P.ClientSetMusic( Song, SongSection, CdTrack, Transition );
	}	

	// Turn off if once-only.
	if( bOnceOnly )
	{
		SetCollision(false,false,false);
		disable( 'Trigger' );
	}
}

defaultproperties
{
     CdTrack=255
     Transition=MTRAN_Fade
	 bAffectAllPlayers=True
}

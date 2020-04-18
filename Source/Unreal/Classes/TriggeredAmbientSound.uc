//=============================================================================
// TriggeredAmbientSound.
//=============================================================================
class TriggeredAmbientSound expands Keypoint;

// Re-plays a sound effect if triggered.  If in state TriggerToggled,
// the sound is turned off if triggered, and turned on, etc..
// when triggered again.  In the OnWhileTriggered state, the instigator
// must stay within the trigger's radius for the sound effect to continue
// playing.  bInitiallyOn determines if the sound is playing from the 
// start, and does not apply in the OnWhileTriggered state.  A variance
// can also be set for the timing, so that the replay time can seem
// more 'real'.

var() bool  bInitiallyOn;
var() sound AmbSound;
var() float rePlayTime;
var() float rePlayVariance;
var() bool  bPlayOnceOnly;

var	  bool  bIsOn;
var	  bool  bPlayedOnce;

function BeginPlay () {
	
	local float timeDelta;
	
	if (bInitiallyOn) {
		PlaySound (AmbSound, , Float(SoundVolume)/128, , Float(SoundRadius)*25);
		bIsOn = true;
	}

	timeDelta = (rePlayVariance*2)*FRand() - rePlayVariance + rePlayTime;
	SetTimer(timeDelta, False);
}


function Timer () {

	local float timeDelta;
	
	if ( (bIsOn && !bPlayOnceOnly) || (bIsOn && bPlayOnceOnly && !bPlayedOnce) ) {
	
		// Replay the sound
		// Which sound should be played?
		PlaySound (AmbSound, , Float(SoundVolume)/128, , Float(SoundRadius)*25);
		bPlayedOnce = true;
	}

	timeDelta = (rePlayVariance*2)*FRand() - rePlayVariance + rePlayTime;
	SetTimer(timeDelta, False);
}


state() TriggerToggled
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		bIsOn = !bIsOn;
		bPlayedOnce = false;
	}
}


state() OnWhileTriggered
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		bIsOn = true;
		bPlayedOnce = false;
	}

	function UnTrigger( actor Other, pawn EventInstigator )
	{
		bIsOn = false;
	}

Begin:
	bIsOn = false;
}

defaultproperties
{
}

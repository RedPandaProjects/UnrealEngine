//=============================================================================
// DynamicAmbientSound.
//=============================================================================
class DynamicAmbientSound expands Keypoint;

var() bool       bInitiallyOn;   	// Initial state
var() sound      Sounds[6];      	// What to play (must be at least one sound)
var() float      playProbability;	// The chance of the sound effect playing
var() float      minReCheckTime;   // Try to restart the sound after (min amount)
var() float      maxReCheckTime;   // Try to restart the sound after (max amount)
var() bool       bDontRepeat;		// Never play two of the same sound in a row
var   bool       soundPlaying;   	// Is it currently playing it?
var   float      rePlayTime;
var   int        numSounds;		// The number of sounds available
var   int        lastSound;		// Which sound was played most recently?

function BeginPlay () {
	
	local int i;
	
	// Calculate how many sounds the user specified
	numSounds=6;
	for (i=0; i<6; i++) {
		if (Sounds[i] == None) {
			numSounds=i;
			break;
		}
	}

	lastSound=-1;
	if (bInitiallyOn) {
		// Which sound should be played?
		i = Rand(numSounds);
		PlaySound (Sounds[i], , Float(SoundVolume)/128, , Float(SoundRadius)*25);
		lastSound = i;
	}

	rePlayTime = (maxReCheckTime-minReCheckTime)*FRand() + minReCheckTime;
	SetTimer(rePlayTime, False);
}


function Timer () {

	local int i;
	
	if (FRand() <= playProbability) {
	
		// Play the sound
		// Which sound should be played?
		i = Rand(numSounds);
		while( i == lastSound && bDontRepeat && numSounds > 1 )
			i = Rand(numSounds);
		
		PlaySound (Sounds[i], , Float(SoundVolume)/128, , Float(SoundRadius)*25);
		lastSound = i;
	}

	rePlayTime = (maxReCheckTime-minReCheckTime)*FRand() + minReCheckTime;
	SetTimer(rePlayTime, False);
}

defaultproperties
{
     playProbability=+00000.600000
     minReCheckTime=+00005.000000
     maxReCheckTime=+00010.000000
     bStatic=False
     SoundVolume=42
}

//=============================================================================
// SmokeHose.
//=============================================================================
class SmokeHose expands Effects;

// Shoots out a stream of RisingSpriteSmokePuffs.  The smoke sprites are shot
// in the direction of the SmokeHoseDest actor with the same Tag as this actors
// Event.  The speed, speedvariance, spawning delay and variance, size and
// size variance, as well as the acceleration of the smoke puffs are adjustable.
// The effect can be initially active all the time, or enabled when the player
// is within a trigger's radius.

var() bool   bInitiallyActive;
var() float  SmokeSpeed;
var() Vector SmokeAccel;
var() float  SmokeDelay;
var() float  SmokeDelayVariance;
var() float  SpeedVariance;
var() float  BasePuffSize;
var() float  SizeVariance;
var() int    TotalNumPuffs;

var Actor    SmokeDestObj;
var int	    NumPuffsSpawned;

function BeginPlay()
{
	local SmokeHoseDest SDest;

	// Find the SmokeHoseDest object which has the same tag as
	// the event of this SmokeHose.
	foreach AllActors( class 'SmokeHoseDest', SDest, Event )
	{
		SmokeDestObj = SDest;
		break;
	}
	NumPuffsSpawned = 0;
	if( bInitiallyActive )
	{
		Enable('Timer');
		SetTimer( SmokeDelay+FRand()*SmokeDelayVariance, False );
	}
	else
		Enable('Trigger');
}

function Timer()
{
	// Spawn another smoke puff sprite
	local RisingSpriteSmokePuff sp;
	
	sp = Spawn(class'RisingSpriteSmokePuff');
	sp.DrawScale = BasePuffSize+FRand()*SizeVariance;
	sp.Velocity  = (SmokeSpeed + FRand()*SpeedVariance) * 
					Normal(SmokeDestObj.Location - Location);
	sp.Acceleration = SmokeAccel;
	NumPuffsSpawned++;
	if( NumPuffsSpawned>TotalNumPuffs ) Destroy();
	SetTimer( SmokeDelay+FRand()*SmokeDelayVariance, False );
}

function Trigger( actor Other, pawn EventInstigator )
{
	// Actor entered the triggers radius
	Enable('Timer');
	SetTimer( SmokeDelay+FRand()*SmokeDelayVariance, False );
}

function UnTrigger( actor Other, pawn EventInstigator )
{
	// Actor left the triggers radius
	Disable('Timer');
}

defaultproperties
{
     bInitiallyActive=True
     SmokeSpeed=+00100.000000
     SmokeAccel=(X=0.000000,Y=0.000000,Z=90.000000)
     SmokeDelay=+00000.100000
     SmokeDelayVariance=+00000.150000
     SpeedVariance=+00020.000000
     BasePuffSize=+00001.000000
     SizeVariance=+00000.200000
     TotalNumPuffs=10000
     bHidden=True
     DrawType=DT_Sprite
     Physics=PHYS_None
}

//=============================================================================
// FatnessTrigger
// When triggered, alters the fatness of an actor from one state to another,
// over a set time interval.
//=============================================================================
class FatnessTrigger expands Triggers;

var() Name  FatTag;		// Tag of object to recieve Fattening/UnFattening
var() int   StartFatness;	// The initial fatness of the object
var() int   EndFatness;		// The final fatness of the object
var() float ChangeTime;		// How long does the change take?

var Actor FatActor;
var float TimePassed;

function BeginPlay()
{
	FatActor = None;
	TimePassed = 0;
}

function Trigger( Actor Other, Pawn EventInstigator )
{
	// Find the object that should be changed
	if ( FatTag != '' )
		foreach AllActors( class 'Actor', FatActor, FatTag )
		{
		}

	if( FatActor != None )
	{
		Enable('Tick');
	}
}

function Tick( float DeltaTime )
{
	local float CurFat, TimeRatio;

	if( FatActor != None )
	{
		// Check the timing
		TimePassed += DeltaTime;
		if( TimePassed >= ChangeTime )
		{
			TimeRatio = 1;
			Disable('Tick');
		}
		else TimeRatio = TimePassed/ChangeTime;

		// Continue the fattening process
		CurFat = (Float(EndFatness) - Float(StartFatness))*TimeRatio + Float(StartFatness);
		FatActor.fatness = Int( CurFat );
	}
	else Disable('Tick');
}

defaultproperties
{
}

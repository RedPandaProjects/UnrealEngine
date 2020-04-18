//=============================================================================
// StochasticTrigger.
//=============================================================================
class StochasticTrigger expands Triggers;

// Works like a DynamicAmbientSound, only events are called instead of
// sounds. M

var() name   Events[6];      		// What events to call (must be at least one event)
var() float  triggerProbability;	// The chance of the event occuring effect playing
var() float  minReCheckTime;   	// Try to re-trigger the event after (min amount)
var() float  	maxReCheckTime;   	// Try to re-trigger the event after (max amount)
var   bool	bIsActive;			// This trigger dispacher is activated/deactivated
var   float  	reTriggerTime;
var   int    	numEvents;			// The number of events available
var   actor  triggerInstigator;   	// Who enabled this actor to dispach?

function BeginPlay () 
{
	local int i;
	
	// Calculate how many events the user specified
	numEvents=6;
	for (i=0; i<6; i++) {
		if (Events[i] == '') {
			numEvents=i;
			break;
		}
	}

	reTriggerTime = (maxReCheckTime-minReCheckTime)*FRand() + minReCheckTime;
	SetTimer(reTriggerTime, False);
}

state() TriggeredActive
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		// StochasticTrigger is active
		if ( triggerInstigator == None )
			triggerInstigator = EventInstigator;
		else
			triggerInstigator = Other;
		Instigator = EventInstigator;
		bIsActive = true;
	}

	function UnTrigger( actor Other, pawn EventInstigator )
	{
		// StochasticTrigger is inactive
		if ( triggerInstigator == None )
			triggerInstigator = EventInstigator;
		else
			triggerInstigator = Other;
		Instigator = EventInstigator;
		bIsActive = false;
	}
Begin:
	bIsActive = false; 		// initially the trigger dispacher is inactive
}

state() AlwaysActive
{
Begin:
	bIsActive = true;
}

function Timer () 
{
	local int 	i;
	local actor 	A;

	if (FRand() <= triggerProbability && bIsActive == true) 
	{
		// Trigger an event
		// Which event should be initiated?
		i = Rand(numEvents);

		// Broadcast the Trigger message to all matching actors.
		if( Events[i] != '' )
			foreach AllActors( class 'Actor', A, Events[i] )
				A.Trigger( triggerInstigator, Instigator );
	}

	reTriggerTime = (maxReCheckTime-minReCheckTime)*FRand() + minReCheckTime;
	SetTimer(reTriggerTime, False);
}

defaultproperties
{
}

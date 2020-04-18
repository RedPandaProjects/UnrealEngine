//=============================================================================
// Dispatcher: receives one trigger (corresponding to its name) as input, 
// then triggers a set of specifid events with optional delays.
//=============================================================================
class Dispatcher expands Triggers;

#exec Texture Import File=Textures\Dispatch.pcx Name=S_Dispatcher Mips=Off Flags=2

//-----------------------------------------------------------------------------
// Dispatcher variables.

var() name  OutEvents[8]; // Events to generate.
var() float OutDelays[8]; // Relative delays before generating events.
var int i;                // Internal counter.

//=============================================================================
// Dispatcher logic.

//
// When dispatcher is triggered...
//
function Trigger( actor Other, pawn EventInstigator )
{
	Instigator = EventInstigator;
	gotostate('Dispatch');
}

//
// Dispatch events.
//
state Dispatch
{
Begin:
	disable('Trigger');
	for( i=0; i<ArrayCount(OutEvents); i++ )
	{
		if( OutEvents[i] != '' )
		{
			Sleep( OutDelays[i] );
			foreach AllActors( class 'Actor', Target, OutEvents[i] )
				Target.Trigger( Self, Instigator );
		}
	}
	enable('Trigger');
}

defaultproperties
{
	 Texture=S_Dispatcher
}

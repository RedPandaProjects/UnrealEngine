//=============================================================================
// CodeTrigger.
//=============================================================================
class CodeTrigger expands Triggers;

// Used along with an ElevatorMover (which is its visual cue), it sends
// a code to the CodeMaster object which tries to match up a pattern
// of a set of CodeTrigger activations, whereupon the CodeMaster executes
// a special event.
// If the order of triggerings is invalid, the set of ElevatorMovers are
// reset to the initial state. M

var() int 	Code;			// What is the label code of this trigger/mover with respect to the sequence
var() name   CodeMasterTag;	// The Tag of the codemaster object to use

var   CodeMaster 		cdMaster;
var   ElevatorMover  	elMover;

function BeginPlay()
{
	local CodeMaster cm;
	local ElevatorMover em;

	// Find the CodeMaster who's Tag matches the CodeMasterTag
	if( CodeMasterTag != '' )
		foreach AllActors( class 'CodeMaster', cm, CodeMasterTag )
			cdMaster = cm;
	
	// Find the ElevatorMover who's Tag matches this triggers Event
	if( Event != '' )
		foreach AllActors( class 'ElevatorMover', em, Event )
			elMover = em;

	if( cdMaster == None ) log("No CodeMaster object found.");
	if( elMover  == None ) log("No ElevatorMover object found.");
}

function Touch( actor Other )
{
	// Set the Elevator Mover to Keyframe 1
	elMover.MoveKeyframe( 1, elMover.MoveTime );

	// Notify the CodeMaster that it has been triggered
	cdMaster.NotifyTriggered( Code );
}

defaultproperties
{
}

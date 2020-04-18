//=============================================================================
// MixMover.
//=============================================================================
class MixMover expands Mover;

var() float OpenTimes [6];
var() float CloseTimes[6];
var() name  Tags      [6];
var() name  Events    [6];
var() name  AttachTag;

var int  LastKeyNum;
var int  NextKeyNum;
var int  MoveDirection;
var bool bMoveKey;

function BeginPlay() {

	KeyNum = 0;
	bMoveKey = true;
	Super.BeginPlay();
}


// Immediately after mover enters gameplay.
function PostBeginPlay()
{
	local Actor Act;
	local Mover Mov;

	// Initialize all slaves.
	if( !bSlave && (AttachTag != '') )
	{
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
}

function DoOpen() {

	// Open through to the next keyframe.
	//
	LastKeyNum = KeyNum;
	InterpolateTo (KeyNum+1, OpenTimes[Keynum]);
	PlaySound (OpeningSound);
	AmbientSound = MoveAmbientSound;
}

function DoClose() {

	// Close through to the next keyframe.
	//
	LastKeyNum = KeyNum;
	InterpolateTo (KeyNum-1, CloseTimes[Keynum-1]);
	PlaySound (ClosingSound);
	AmbientSound = MoveAmbientSound;
}


function MoveKeyframe( int newKeyNum )
{		
	if( !bMoveKey ) return;

	NextKeyNum = newKeyNum;
	if( NextKeyNum < KeyNum )
	{
		MoveDirection = -1;
		GotoState('ElevatorTriggerGradual','ChangeFrame');
	}
	
	if( NextKeyNum > KeyNum )
	{
		MoveDirection = 1;
		GotoState('ElevatorTriggerGradual','ChangeFrame');
	}
}


state() ElevatorTriggerGradual {

	function InterpolateEnd(actor Other) {	
	}

ChangeFrame:
	bMoveKey = false;

	// Move the mover
	//
	if( MoveDirection > 0	){
		DoOpen();
		FinishInterpolation();
		FinishedClosing();
	}
	else {
		DoClose();
		FinishInterpolation();
		FinishedOpening();
	}

	// Check if there are more frames to go
	//
	if( KeyNum != NextKeyNum )
	{
		GotoState('ElevatorTriggerGradual','ChangeFrame');
	}

	bMoveKey = true;
	Stop;
}


//=======================================================================
// The various states

// When triggered, open, wait, then close.
//
state() GradualTriggerOpenTimed 
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		SavedTrigger = Other;
		Instigator = EventInstigator;
		SavedTrigger.BeginEvent();
		GotoState( 'GradualTriggerOpenTimed', 'Open' );
	}

	function InterpolateEnd(actor Other) {	
	}

Begin:
	// Set Tag/Event to the first set in the Tags[] and
	// Events[] arrays.
	Tag   = Tags[0];
	Event = Events[0];
	Enable('Trigger');
	Stop;

Open:
	Disable ('Trigger');
	DoOpen();
	FinishInterpolation();
	FinishedOpening();

	// Check if this is the fully opened position,
	// for which a delay is necessary.
	//
	if (KeyNum == NumKeys) {		// Note: NumKeys=0 means one key frame
		Sleep (StayOpenTime);
		if( bTriggerOnceOnly )
			// Stays in this position forever
			GotoState ('');
		else 
			// The closing sequence must begin
			GotoState ('GradualTriggerOpenTimed', 'Close');
	}
	
	// Check if the next Tag is the same as the current,
	// which would continue interpolating to the next
	// key-frame.
	//
	if (Tags[KeyNum] == Tags[LastKeyNum]) {
		GotoState ('GradualTriggerOpenTimed', 'Open');
	}
	Tag   = Tags[KeyNum];		// Change the next open conditions
	Event = Events[KeyNum];
	Enable ('Trigger');
	Stop;
	
Close:
	Disable ('Trigger');
	DoClose();
	FinishInterpolation();
	FinishedClosing();	// throw the current Event, if exists

	if (KeyNum > 0) 		// Still more key-frames to go through
		GotoState ('GradualTriggerOpenTimed', 'Close');

	Tag   = Tags[0];		// Reset the initial state
	Event = Events[0];
	Enable ('Trigger');
}


// Start pounding when triggered.
//
state() GradualTriggerPound
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		SavedTrigger = Other;
		Instigator = EventInstigator;
		GotoState( 'GradualTriggerPound', 'Open' );
	}

	function UnTrigger( actor Other, pawn EventInstigator )
	{
		SavedTrigger = None;
		Instigator = None;
		GotoState( 'GradualTriggerPound', 'Close' );
	}

Begin:
	// Set Tag/Event to the first set in the Tags[] and
	// Events[] arrays.
	Tag   = Tags[0];
	Event = Events[0];
	Stop;

Open:
	Disable ('Trigger');
	DoOpen();
	FinishInterpolation();
	FinishedOpening();

	// If the next key frame is not the last, then
	// keep playing back the frames.
	//
	if (Keynum < NumKeys) {
		GotoState ('GradualTriggerOpenTimed', 'Open');
	}
Close:
	Disable ('Trigger');
	DoClose();
	FinishInterpolation();
	FinishedClosing();	// throw the current Event, if exists

	if (KeyNum > 0) 		// Still more key-frames to go through
		GotoState ('GradualTriggerOpenTimed', 'Close');

	Sleep(StayOpenTime);
	if( bTriggerOnceOnly )
		GotoState('');
	if( SavedTrigger != None )
		goto 'Open';
}

defaultproperties
{
}


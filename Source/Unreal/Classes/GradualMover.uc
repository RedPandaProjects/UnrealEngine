//=============================================================================
// GradualMover.
//=============================================================================
class GradualMover expands Mover;

var(GradualProperties) float OpenTimes [6];
var(GradualProperties) float CloseTimes[6];
var(GradualProperties) name  Tags      [6];
var(GradualProperties) name  Events    [6];

var int  LastKeyNum;
var bool bIsFullyOpen;

// The strategy is to dynamically modify the Tag and Event 
// data members, which, I assume, are automatically checked 
// when making Event->Tag matches, using the Tags[] and 
// Events[] arrays.
// When the Mover has finished its' opening stage, the
// open state lasts for the duration specified by the
// StayOpenTime data member, whereupon the closing sequence
// begins, unless bTriggerOnceOnly is set True.

function BeginPlay() {

	// Set Tag/Event to the first set in the Tags[] and
	// Events[] arrays.
	Tag   = Tags[0];
	Event = Events[0];
	KeyNum = 0;
	bIsFullyOpen = false;
	Super.BeginPlay();
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

//=======================================================================
// The various states

// When triggered, open, wait, then close.
//
state() GradualTriggerOpenTimed {

	function Trigger( actor Other, pawn EventInstigator )
	{
		SavedTrigger = Other;
		Instigator = EventInstigator;
		SavedTrigger.BeginEvent();
		GotoState( 'GradualTriggerOpenTimed', 'Open' );
	}

	function InterpolateEnd(actor Other) {	
	}

Open:
	Disable ('Trigger');
	DoOpen();
	FinishInterpolation();
	FinishedOpening();

	// Check if this is the fully opened position,
	// for which a delay is necessary.
	//
	if (KeyNum == NumKeys-1) {		// Note: NumKeys=0 means one key frame
		Sleep (StayOpenTime);
		AmbientSound = None;
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
	AmbientSound = None;
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
Open:
	Disable ('Trigger');
	DoOpen();
	FinishInterpolation();
	FinishedOpening();

	// If the next key frame is not the last, then
	// keep playing back the frames.
	//
	if (Keynum < NumKeys-1) {
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
	{
		AmbientSound = None;
		GotoState('');
	}
	if( SavedTrigger != None )
		goto 'Open';
}

state() GradualTriggerToggle
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		SavedTrigger = Other;
		Instigator = EventInstigator;
		SavedTrigger.BeginEvent();
		if( bIsFullyOpen )
			GotoState( 'GradualTriggerToggle', 'Close' );
		else
			GotoState( 'GradualTriggerToggle', 'Open' );
	}

	function InterpolateEnd(actor Other) {	
	}

Open:
	Disable ('Trigger');
	DoOpen();
	FinishInterpolation();
	FinishedOpening();

	// Check if this is the fully opened position,
	// which we wait in until another triggering event occurs.
	//
	if (KeyNum == NumKeys-1) {		// Note: NumKeys=0 means one key frame
		bIsFullyOpen = true;
		AmbientSound = None;
		Enable ('Trigger');
		Stop;
	}
	
	GotoState ('GradualTriggerToggle', 'Open');
	
Close:
	Disable ('Trigger');
	DoClose();
	FinishInterpolation();
	FinishedClosing();	// throw the current Event, if exists

	if (KeyNum > 0) 		// Still more key-frames to go through
		GotoState ('GradualTriggerToggle', 'Close');

	bIsFullyOpen = false;
	AmbientSound = None;
	Enable ('Trigger');
}

defaultproperties
{
}

//=============================================================================
// ElevatorMover.
//=============================================================================
class ElevatorMover expands Mover;

// Allows this mover to go from any key frame to any other key frame,
// depending on the trigger settings.  This produces elevator-like control. M

var int   LastKeyNum;
var int   NextKeyNum;
var int   MoveDirection;
var float MoveTimeInterval;
var bool  bMoveKey;

function BeginPlay() 
{
	Super.BeginPlay();
	bMoveKey = true;
}

function MoveKeyframe( int newKeyNum, float newMoveTime )
{
	if( !bMoveKey ) return;

	NextKeyNum = newKeyNum;
	if( NextKeyNum < KeyNum )
	{
		MoveDirection = -1;
		MoveTimeInterval = newMoveTime/(KeyNum-NextKeyNum);
		GotoState('ElevatorTriggerGradual','ChangeFrame');
	}
	
	if( NextKeyNum > KeyNum )
	{
		MoveDirection = 1;
		MoveTimeInterval = newMoveTime/(NextKeyNum-KeyNum);
		GotoState('ElevatorTriggerGradual','ChangeFrame');
	}
}

function DoOpen() 
{
	// Open through to the next keyframe.
	//
	LastKeyNum = KeyNum;
	InterpolateTo (KeyNum+1, MoveTime);
	PlaySound (OpeningSound);
	AmbientSound = MoveAmbientSound;
}

function DoClose() {

	// Close through to the next keyframe.
	//
	LastKeyNum = KeyNum;
	InterpolateTo (KeyNum-1, MoveTime);
	PlaySound (ClosingSound);
	AmbientSound = MoveAmbientSound;
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

defaultproperties
{
}

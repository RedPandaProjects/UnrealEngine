//=============================================================================
// RotatingMover.
//=============================================================================
class RotatingMover expands Mover;

var() rotator RotateRate;

function BeginPlay()
{
	Disable( 'Tick' );
}

function Tick( float DeltaTime )
{
	SetRotation( Rotation + (RotateRate*DeltaTime) );
}

function Trigger( Actor other, Pawn EventInstigator )
{
	Enable('Tick');
}

function UnTrigger( Actor other, Pawn EventInstigator )
{
	Disable('Tick');
}

defaultproperties
{
}

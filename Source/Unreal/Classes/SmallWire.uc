//=============================================================================
// SmallWire.
//=============================================================================
class SmallWire expands Wire;

Auto State Animate
{

Begin:
	if (WireType == E_WireWiggle) LoopAnim('Wiggle',1.0);
	else LoopAnim('Still',FRand()*0.03+0.02);
}
defaultproperties
{
     DrawScale=+00000.600000
     CollisionRadius=+00003.000000
     CollisionHeight=+00040.000000
}

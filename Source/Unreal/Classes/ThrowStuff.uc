//=============================================================================
// ThrowStuff.
// throw pawns, inventory (non-item), and decorations around
//=============================================================================
class ThrowStuff expands Keypoint;

var() vector throwVect;
var() bool	bRandomize;
var() float	interval;
var() byte	Numthrows;
var byte remainingThrows;
var float	baseSize;

	function PreBeginPlay()
	{
		if (bRandomize)
			baseSize = VSize(throwVect);
		Super.PreBeginPlay();
	}

	function Trigger(actor Other, pawn EventInstigator)
	{
		remainingThrows = Numthrows;
		Throwing();
	}

	function Throwing()
	{
		local actor A;
		local float throwSize, oldZ;
		if ( event != '' )
			ForEach AllActors(class 'Actor', A, event)
			{
				A.SetPhysics(PHYS_Falling);		
				if ( Pawn(A) != None )
					Pawn(A).AddVelocity(throwVect);
				else if ( ((Decoration(A) != None) && Decoration(A).bPushable)
					|| ((Inventory(A) != None) && !A.bHidden) )
					A.Velocity = throwVect;
				if (bRandomize)
				{
					oldZ = throwVect.Z;
					throwSize = VSize(throwVect);
					if (throwSize > 1.5 * baseSize)
						throwSize = baseSize;
					else if (throwSize < 0.5 * baseSize)
						throwSize = baseSize;
					throwVect = throwSize * (Normal(throwVect) + 0.5 * VRand());
					if ( (oldZ > 0) != (throwVect.Z > 0) )
						throwVect.Z *= -1;
				}
			}		
		
		if (Remainingthrows > 1)
		{
			Remainingthrows--;
			SetTimer(interval, false);
		}
	}
	
	function Timer()
	{
		Throwing();	
	}

defaultproperties
{
     Numthrows=1
     bStatic=False
}

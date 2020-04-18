//=============================================================================
// Earthquake.
// note - this just shakes the players.  Trigger other effects directly
//=============================================================================
class Earthquake expands Keypoint;

var() float magnitude;
var() float duration;
var() float radius;
var() bool bThrowPlayer;
var   float remainingTime;

	function Trigger(actor Other, pawn EventInstigator)
	{
		local Pawn P;
		local vector throwVect;
		if (bThrowPlayer)
		{
			throwVect = 0.18 * Magnitude * VRand();
			throwVect.Z = FMax(Abs(ThrowVect.Z), 120);
		} 
		P = Level.PawnList;
		while ( P != None )
		{
			if ( (PlayerPawn(P) != None) && (VSize(Location - P.Location) < radius) )
			{
				if (bThrowPlayer && (P.Physics != PHYS_Falling) )
					P.AddVelocity(throwVect);
				PlayerPawn(P).ShakeView(duration, magnitude, 0.015 * magnitude);
			}
			P = P.nextPawn;
		}
		if ( bThrowPlayer && (duration > 0.5) )
		{
			remainingTime = duration;
			SetTimer(0.5, false);
		}
	}

	function Timer()
	{
		local vector throwVect;
		local Pawn P;
		
		remainingTime -= 0.5;
		throwVect = 0.15 * Magnitude * VRand();
		throwVect.Z = FMax(Abs(ThrowVect.Z), 120);

		P = Level.PawnList;
		while ( P != None )
		{
			if ( (PlayerPawn(P) != None) && (VSize(Location - P.Location) < radius) )
			{
				if ( P.Physics != PHYS_Falling )
					P.AddVelocity(ThrowVect);
				P.BaseEyeHeight = FMin(P.Default.BaseEyeHeight, P.BaseEyeHeight * (0.5 + FRand()));
				PlayerPawn(P).ShakeView(remainingTime, magnitude, 0.015 * magnitude);
			}
			P = P.nextPawn;
		}
			
		if ( remainingTime > 0.5 )
			SetTimer(0.5, false);
	}	

defaultproperties
{
     Magnitude=+02000.000000
     duration=+00005.000000
     Radius=+00300.000000
	 bThrowPlayer=True
     bStatic=False
}

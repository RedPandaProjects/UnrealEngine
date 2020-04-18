//=============================================================================
// LesserBrute.
//=============================================================================
class LesserBrute expands Brute;

#exec TEXTURE IMPORT NAME=Brute3 FILE=Models\Brute3.PCX GROUP="Skins"

function PlayRunning()
{
	if (Focus == Destination)
	{
		LoopAnim('Walk', -1.3/GroundSpeed,,0.4);
		return;
	}	

	LoopAnim('Walk', StrafeAdjust(),,0.3);
}

function PlayWalking()
{
	LoopAnim('Walk', -1.3/GroundSpeed,,0.4);
}

function PlayMovingAttack()
{
	PlayAnim('WalkFire', 1.3);
}

function GoBerserk()
{
	bLongBerserk = false;
	if ( bBerserk || ((Health < 0.75 * Default.Health) && (FRand() < 0.7)) )
		bBerserk = true;
	else 
		bBerserk = false;
	if ( bBerserk )
	{
		AccelRate = 4 * AccelRate;
		GroundSpeed = (2.1 + 0.2 * skill) * Default.GroundSpeed;
	}
}

state TacticalMove
{
ignores SeePlayer, HearNoise;

	function AnimEnd()
	{
		If ( bBerserk )
			LoopAnim('Charge', -1.1/GroundSpeed,,0.5);
		else
			PlayCombatMove();
	}
			
	function BeginState()
	{
		GoBerserk();
		Super.BeginState();
	}

	function EndState()
	{
		if ( bBerserk )
		{
			GroundSpeed = Default.GroundSpeed;
			AccelRate = Default.AccelRate;
		}
		Super.EndState();
	}
}


defaultproperties
{
	 ReducedDamageType=None
	 ReducedDamagePct=+00000.000000
     WhipDamage=14
	 Skin=Brute3
     Health=180
     CarcassType=LesserBruteCarcass
	 Fatness=110
     GroundSpeed=+00130.000000
     AccelRate=+00400.000000
     ReFireRate=+00000.200000
     Mass=+00250.000000
     Buoyancy=+00240.000000
	 DrawScale=+00000.800000
     CollisionRadius=+00042.000000
     CollisionHeight=+00042.000000
}
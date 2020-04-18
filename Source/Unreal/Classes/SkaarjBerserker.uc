//=============================================================================
// SkaarjBerserker.
//=============================================================================
class SkaarjBerserker expands SkaarjWarrior;

#exec TEXTURE IMPORT NAME=Skaarjw2 FILE=MODELS\Skar1.PCX GROUP=Skins 

function WhatToDoNext(name LikelyState, name LikelyLabel)
{
	local Pawn aPawn;

	aPawn = Level.PawnList;
	while ( aPawn != None )
	{
		if ( (aPawn.IsA('PlayerPawn') || aPawn.IsA('ScriptedPawn'))
			&& (VSize(Location - aPawn.Location) < 500)
			&& CanSee(aPawn) )
		{
			if ( SetEnemy(aPawn) )
			{
				GotoState('Attacking');
				return;
			}
		}
		aPawn = aPawn.nextPawn;
	}
	
	Super.WhatToDoNext(LikelyState, LikelyLabel);
}	

function eAttitude AttitudeToCreature(Pawn Other)
{
	if ( Other.IsA('ScriptedPawn') && !Other.IsA('Pupae') )
		return ATTITUDE_Hate;
	else
		return ATTITUDE_Ignore;
}

defaultproperties
{
     voicePitch=+00000.300000
     LungeDamage=40
     SpinDamage=40
     ClawDamage=20
	 CombatStyle=+00001.000000
     Aggressiveness=+00000.800000
     Health=320
     Skill=+00001.000000
	 DrawScale=+00001.200000
	 Fatness=150
     Skin=Skaarjw2
     CollisionHeight=+00056.000000
     Mass=+00180.000000
     Buoyancy=+00180.000000
     RotationRate=(Yaw=50000)
}

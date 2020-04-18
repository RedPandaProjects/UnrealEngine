//=============================================================================
// Scout used for path generation.
//=============================================================================
class Scout expands Pawn
	intrinsic;

function PreBeginPlay()
{
	Destroy(); //scouts shouldn't exist during play
}

defaultproperties
{
     SightRadius=+04100.000000
     AccelRate=+00001.000000
     CombatStyle=+4363467783093056800000000.000000
     CollisionRadius=+00052.000000
     CollisionHeight=+00050.000000
     bCollideActors=False
     bCollideWorld=False
     bBlockActors=False
     bBlockPlayers=False
     bProjTarget=False
}

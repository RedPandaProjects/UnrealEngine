//=============================================================================
// KrallElite.
//=============================================================================
class KrallElite expands Krall;

#exec TEXTURE IMPORT NAME=ekrall FILE=MODELS\ekrall.PCX GROUP=Skins 

function PreBeginPlay()
{
	Super.PreBeginPlay();
	bCanDuck = true;
}
function PlayMeleeAttack()
{
	local float decision;

	decision = FRand();
	if (!bSpearToss)
		decision *= 0.65;
	if (decision < 0.22)
		PlayAnim('Strike1'); 
 	else if (decision < 0.44)
   		PlayAnim('Strike2');
 	else if (decision < 0.65)
 		PlayAnim('Strike3');
 	else
 		PlayAnim('Throw'); 
}

defaultproperties
{
     ProjectileSpeed=+00880.000000
     StrikeDamage=28
     ThrowDamage=38
     PoundDamage=28
     MinDuckTime=5.000000
     RangedProjectile=Class'Unreal.EliteKrallBolt'
     Health=200
     UnderWaterTime=-1.000000
     bCanStrafe=True
	 bLeadTarget=True
     Skill=1.000000
     Skin=Texture'Unreal.ekrall'
}

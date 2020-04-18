//=============================================================================
// DeadChairMale.
//=============================================================================
class DeadChairMale expands MaleBodyTwo;

var bool	bChairSlump;

function PostBeginPlay()
{
	Super.PostBeginPlay();
	bChairSlump = ( (AnimSequence == 'Chair1a') || (AnimSequence == 'Chair2a') );
}

function Trigger( actor Other, pawn EventInstigator )
{
	if ( bChairSlump )
		PlaySlump();
}
		
function Touch(Actor Other)
{
	if ( !bChairSlump )
	{
		Disable('Touch');
		Return;
	}
	if ( Other.IsA('Pawn') || Other.IsA('Projectile') )
	{
		PlaySlump();
		Disable('Touch');
	}
}

function TakeDamage( int Damage, Pawn InstigatedBy, Vector Hitlocation, 
						Vector Momentum, name DamageType)
{
	if ( bChairSlump )
		PlaySlump();
	else
		Super.TakeDamage(Damage, InstigatedBy, HitLocation, Momentum, DamageType);
}

function PlaySlump()
{
	bChairSlump = false;
	PlayAnim(AnimSequence);
}

defaultproperties
{
	  Animation=Chair1a
      PrePivot=(X=0.000000,Y=0.000000,Z=0.000000)
      CollisionHeight=+00022.000000
	  CollisionRadius=+00022.000000
}
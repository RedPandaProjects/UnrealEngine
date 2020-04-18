//=============================================================================
// QueenProjectile.
//=============================================================================
class QueenProjectile expands SkaarjProjectile;

auto state Flying
{
	function ProcessTouch (Actor Other, Vector HitLocation)
	{
		local vector momentum;
	
		if ( !Other.IsA('Queen') )
		{
			momentum = 10000.0 * Normal(Velocity);
			Other.TakeDamage(Damage, instigator, HitLocation, momentum, 'zapped');
			Destroy();
		}
	}
}

defaultproperties
{
     MaxSpeed=+02000.000000
}
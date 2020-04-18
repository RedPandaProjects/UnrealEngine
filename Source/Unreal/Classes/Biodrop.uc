//=============================================================================
// BioDrop.
//=============================================================================
class BioDrop expands BioGel;

auto state Flying
{
	function HitWall( vector HitNormal, actor Wall )
	{
		SetRotation(rotator(HitNormal));	
		Super.HitWall(HitNormal, Wall);
	}

	function BeginState()
	{
		Velocity = vect(0,0,0);
		LoopAnim('Flying',0.4);
	}
}

state OnSurface
{
	function CheckSurface()
	{
		local float DotProduct;

		DotProduct = SurfaceNormal dot vect(0,0,-1);
		if (DotProduct > -0.5)
			PlayAnim('Slide',0.2);
	}

Begin:
	FinishAnim();
	CheckSurface();
}

defaultproperties
{
     speed=+00000.000000
     MaxSpeed=+00900.000000
     Damage=+00060.000000
     ImpactSound=Unreal.GelDrip
     CollisionRadius=+00003.000000
     CollisionHeight=+00003.000000
     bProjTarget=False
     LightRadius=2
     Buoyancy=+00000.000000
     LifeSpan=+00140.000000
     RemoteRole=ROLE_DumbProxy
}

//=============================================================================
// BigBioGel.
//=============================================================================
class BigBioGel expands BioGel;

function DropDrip()
{
	local BioGel Gel;

	PlaySound(SpawnSound);		// Dripping Sound
	Gel = Spawn(class'BioDrop', Pawn(Owner),,Location-Vect(0,0,1)*10);
	Gel.DrawScale = DrawScale * 0.5;	
}


auto state Flying
{
	simulated function HitWall( vector HitNormal, actor Wall )
	{
		SetPhysics(PHYS_None);		
		MakeNoise(0.6);	
		bOnGround = True;
		PlaySound(ImpactSound);	
		SetWall(HitNormal, Wall);
		DrawScale=DrawScale*1.4;
		GoToState('OnSurface');
	}

	function BeginState()
	{	
		local Vector viewDir;
		
		viewDir = vector(Rotation);	
		Velocity = (Speed + (viewDir dot Instigator.Velocity)) * viewDir;
		Velocity.z += 120;
		RandSpin(100000);
		LoopAnim('Flying',0.4);
		bOnGround=False;
		PlaySound(SpawnSound);
		if( Region.zone.bWaterZone )
			Velocity=Velocity*0.7;
	}
}

state OnSurface
{
	function BeginState()
	{
		wallTime = DrawScale*5+2;
		
		if ( Mover(Base) != None )
		{
			BaseOffset = VSize(Location - Base.Location);
			SetTimer(0.2, true);
		}
		else 
			SetTimer(wallTime, false);
	}
}

defaultproperties
{
     speed=+00700.000000
     Damage=+00075.000000
     MomentumTransfer=30000
     CollisionRadius=+00003.000000
     CollisionHeight=+00004.000000
     LifeSpan=+00025.000000
     RemoteRole=ROLE_DumbProxy
     Class=Unreal.BigBioGel
}

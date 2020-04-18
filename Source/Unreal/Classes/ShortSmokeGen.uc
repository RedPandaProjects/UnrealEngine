//=============================================================================
// ShortSmokeGen.
//=============================================================================
class ShortSmokeGen expands SmokeGenerator;

Auto State Active
{
	Simulated function Timer()
	{
		local Effects d;
		
		d = Spawn(GenerationType);
		d.DrawScale = BasePuffSize+FRand()*SizeVariance;
		d.RemoteRole = ROLE_None;	
		if (SpriteSmokePuff(d)!=None) SpriteSmokePuff(d).RisingRate = RisingVelocity;	
		i++;
		if (i>TotalNumPuffs && TotalNumPuffs!=0) Destroy();
	}
}

simulated function PostBeginPlay()
{
	SetTimer(SmokeDelay+FRand()*SmokeDelay,True);
	Super.PostBeginPlay();
}

defaultproperties
{
     SmokeDelay=+00000.120000
     BasePuffSize=+00001.500000
     TotalNumPuffs=10
     RisingVelocity=+00040.000000
     bBlackSmoke=True
     RemoteRole=ROLE_SimulatedProxy
}

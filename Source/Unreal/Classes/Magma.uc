//=============================================================================
// Magma.
//=============================================================================
class Magma expands BigRock;

// Owner (spawner) specifies the velocity of the Magma (implicitly in
// Actor properties), the length of time it can burn for, the initial 
// brightness, as well as the damage caused.  Modified from Sparkbit
// and Rock.  MZM

var() float DelaySmoke;
var   float BurnTime;
var   float InitialBrightness;
var   float LastSmokeTime;
var   float PassedTime;

function Timer()
 {

	// The Magma should lose its brightness as it
	// burns away, but not linearly - the main brightness
	// decays in quadratic fashion but the actual brightness
	// is also randomly tweaked to give the appearance of
	// non-uniform burning.  The object also gives off
	// its own light.
	//
	local float tempBrightness;

	PassedTime += 0.15;
	// Spawn smoke if enough time has passed.
	if (PassedTime-LastSmokeTime >= DelaySmoke) 
	{
		//Spawn (class 'SmokeTrail', , '', Location+Vect(0,0,8));
		LastSmokeTime = PassedTime;
	}
	tempBrightness = InitialBrightness*(1-
				    ((PassedTime*(1-0.1+0.2*FRand()))/BurnTime) **2);
	tempBrightness = FClamp (tempBrightness, 0, 1);
		
	LightBrightness = tempBrightness * 90;
	AmbientGlow     = tempBrightness * 240;
}

auto state Flying
{
	simulated function HitWall (vector HitNormal, actor Wall)
	{
		InitialBrightness *= 1.5;
		Super.HitWall(HitNormal, Wall);
	}

Begin:
	SetTimer(0.15, true);
	if (Speed != 0) Velocity = Vector(Rotation) * Speed;
	RotationRate = RotRand();
	BurnTime = FMin(BurnTime, 0.1);
	SetPhysics (PHYS_Falling);
}

defaultproperties
{
      DrawType=DT_Mesh
      Skin=Jb1exp1
     Mesh=Unreal.TBoulder
	  bCollideActors=True
      bBlockActors=True
      bBlockPlayers=True
      Physics=PHYS_Falling
      LifeSpan=+00015.000000
	  LightType=LT_Steady
	  LightHue=120
	  LightSaturation=200
	  LightRadius=30
	  LightBrightness=130
}


//=============================================================================
// SparkBit.
//=============================================================================
class SparkBit expands Effects;

#exec MESH IMPORT MESH=bolt1 ANIVFILE=MODELS\bolt1_a.3D DATAFILE=MODELS\bolt1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=bolt1 X=0 Y=0 Z=-0 YAW=64
#exec MESH SEQUENCE MESH=bolt1 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=bolt1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jmisc1 FILE=MODELS\misc.PCX 
#exec MESHMAP SCALE MESHMAP=bolt1 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=bolt1 NUM=1 TEXTURE=Jmisc1

// Owner (spawner) specifies the velocity of the SparkBit (implicitly in
// Actor properties), the length of time it can burn for, and the initial 
// brightness of the spark.  MZM

var float BurnTime;
var float InitialBrightness;
var float PassedTime;

auto state Burning
{
	function Timer() 
	{
		// The Spark should lose its brightness as it
		// burns away, but not linearly - the main brightness
		// decays in quadratic fashion but the actual brightness
		// is also randomly tweaked to give the appearance of
		// non-uniform burning.  The object also gives off
		// its own light.
		//
		local float tempBrightness;

		PassedTime += 0.15;
		tempBrightness = InitialBrightness*(1-
 					    ((PassedTime*(1-0.1+0.2*FRand()))/BurnTime) **2);
		tempBrightness = FClamp (tempBrightness, 0, 1);
		
		LightBrightness = tempBrightness * 90;
		AmbientGlow     = tempBrightness * 240;
	}

	function HitWall (vector HitNormal, actor Wall) 
	{
		// When surface is hit, the spark appears to become
		// brighter, losing size, but still bounces off (losing half its speed).
		//
		InitialBrightness *= 1.5;
		DrawScale         /= 1.5;
		Velocity = 0.5*(-(Velocity dot HitNormal)*HitNormal*2 + Velocity);
		if ( (HitNormal.Z > 0.7) && (VSize(Velocity) < 50) )
			bBounce = false;
	}
	
	function ZoneChange (ZoneInfo NewZone) 
	{
		// Spark dies if it hits the water.
		//
		if (NewZone.bWaterZone) Destroy();
	}

	function BeginState()
	{
		SetTimer(0.15, true);
		SetPhysics (PHYS_Falling);
	}

Begin:
	BurnTime = FMax(BurnTime, 0.1);
	Sleep(BurnTime);
	Destroy();
}

defaultproperties
{
	  BurnTime=+00000.100000
      DrawType=DT_Mesh
      Texture=None
      Mesh=Chnk4
      AmbientGlow=123
      CollisionRadius=+00001.000000
      CollisionHeight=+00001.000000
      bCollideActors=True
      bCollideWorld=True
	  bBounce=True
	  bUnlit=True
      LightType=LT_Steady
      LightBrightness=130
      LightHue=120
      LightSaturation=200
      LightRadius=1
	  DrawScale=+00000.300000
}

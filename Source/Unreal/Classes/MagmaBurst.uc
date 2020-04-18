//=============================================================================
// MagmaBurst.
//=============================================================================
class MagmaBurst expands Rockslide;
    
// Spawns off a number of Magma elements, which all die out
// within a random amount of time.  This code modified from 
// Sparks.  MZM
								// Reasonable defaults
var() int      MinSpawnedAtOnce;	// 1
var() int      MaxSpawnedAtOnce;	// 3
var() float    MinSpawnSpeed;		// 200
var() float    MaxSpawnSpeed;		// 300
var() float    MinBurnTime;		// 0.4
var() float    MaxBurnTime;		// 1.0
var() float    MinBrightness;		// 0.7	(values can only go from 0.0 -> 1.0)
var() float    MaxBrightness;		// 1.0    "							   "
var() rotator  SpawnCenterDir;
var() int      AngularDeviation;	// approx. 0x2000 -> 8192

function MoreMagma () 
{
	local vector    SpawnLoc;
	local Magma 	   TempMagma;
	local FlameBall TempFlame;
	local rotator  SpawnDir;

	SpawnLoc = Location - (CubeDimensions*0.5);
	SpawnLoc.X += FRand()*CubeDimensions.X;
	SpawnLoc.Y += FRand()*CubeDimensions.Y;
	SpawnLoc.Z += FRand()*CubeDimensions.Z;

	// Time to generate another Magma Rock.
	TempMagma = Spawn (class 'Magma', , '', SpawnLoc);
	
	// Spawn a lava burst as the magma flies out
	TempFlame = Spawn (class 'FlameBall', , '', SpawnLoc);

	SpawnDir = SpawnCenterDir;
	SpawnDir.Pitch += -AngularDeviation + Rand(AngularDeviation*2);
	SpawnDir.Yaw   += -AngularDeviation + Rand(AngularDeviation*2);
	TempMagma.SetRotation(SpawnDir);
	TempMagma.RotationRate = RotRand();
	TempMagma.Speed    = (MinSpawnSpeed + 
	                       FRand()*(MaxSpawnSpeed-MinSpawnSpeed));

	TempMagma.BurnTime = MinBurnTime + FRand()*(MaxBurnTime-MinBurnTime);

	// 0=dark  1=bright
	TempMagma.InitialBrightness = MinBrightness + 
			     			     FRand()*(MaxBrightness-MinBrightness);

	TempMagma.DrawScale = (MaxScaleFactor-MinScaleFactor)*FRand()+MinScaleFactor;
}


state() Active 
{
	function MakeRock()
	{
		local int i, NumSpawnedNow;

		NumSpawnedNow = Rand(MaxSpawnedAtOnce-MinSpawnedAtOnce+1)+MinSpawnedAtOnce;
		for (i=0; i<NumSpawnedNow; i++)
			MoreMagma();
	}
}


auto state() Triggered 
{
	function Trigger (actor Other, pawn EventInstigator) 
	{
		GotoState ('Active');
	}
}

defaultproperties
{
      CubeDimensions=(X=60.000000,Y=60.000000,Z=60.000000)
      MinBetweenTime=+00000.700000
      MaxBetweenTime=+00001.700000
      MinScaleFactor=+00000.600000
      MaxScaleFactor=+00001.300000
      MinSpawnedAtOnce=1
      MaxSpawnedAtOnce=4
      MinSpawnSpeed=+00200.000000
      MaxSpawnSpeed=+01000.000000
      MinBurnTime=+00020.000000
      MaxBurnTime=+00025.000000
      MinBrightness=+00190.000000
      MaxBrightness=+00240.000000
      SpawnCenterDir=(Pitch=20000,Yaw=0,Roll=0)
      AngularDeviation=36000
      Tag=MagmaTest1
      bHidden=True
      DrawType=DT_Sprite
      Physics=PHYS_None
      LifeSpan=+00000.000000
}

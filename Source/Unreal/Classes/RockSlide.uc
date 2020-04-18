//=============================================================================
// RockSlide.
//=============================================================================
class RockSlide expands KeyPoint;

// Spawns a set of rocks within a cubical volume.  The rocks are
// produced at random intervals, and the entirety of the effect
// lasts for a set amount of time.  MZM

var() vector   CubeDimensions;
var() bool     TimeLimit;          // Is there a limit on it's lifespan?
var() float    TimeLength;
var() float    MinBetweenTime;
var() float    MaxBetweenTime;
var() float    MinScaleFactor;
var() float    MaxScaleFactor;
var() rotator  InitialDirection;
var() float    minInitialSpeed;
var() float    maxInitialSpeed;

var   float  NextRockTime;
var   float  TotalPassedTime;


function BeginPlay() 
{
	// Initialize and check the values of variables.
	MaxScaleFactor = FMin(1.0, MaxScaleFactor);
	MinScaleFactor = FMax(0.0, MinScaleFactor);
	if (MinBetweenTime >= MaxBetweenTime) 
		MaxBetweenTime=MinBetweenTime + 0.1;
	if (MinScaleFactor >= MaxScaleFactor) 
		MaxScaleFactor=MinScaleFactor;

	Super.BeginPlay();
}

function MakeRock () 
{
	// Spawns another rock somewhere within the cube,
	// of a randomized size and shape.  The rock has
	// falling physics but no initial velocity, so it 
	// needs to fall a distance before it becomes dangerous.

	local vector  SpawnLoc;
	local BigRock    TempRock;
	
	SpawnLoc = Location - (CubeDimensions*0.5);
	SpawnLoc.X += FRand()*CubeDimensions.X;
	SpawnLoc.Y += FRand()*CubeDimensions.Y;
	SpawnLoc.Z += FRand()*CubeDimensions.Z;

	TempRock = Spawn (class 'BigRock', ,'', SpawnLoc);
	if ( TempRock != None )
	{
		TempRock.SetRotation(InitialDirection);
		TempRock.Speed = (MinInitialSpeed+
		         		(MaxInitialSpeed-MinInitialSpeed)*FRand());

		TempRock.DrawScale = (MaxScaleFactor-MinScaleFactor)*FRand()+MinScaleFactor;
		TempRock.SetCollisionSize(TempRock.CollisionRadius*TempRock.DrawScale/TempRock.Default.DrawScale, 
									 TempRock.CollisionHeight*TempRock.DrawScale/TempRock.Default.DrawScale);
	}
}

auto state() Triggered 
{
	function Trigger (actor Other, pawn EventInstigator) 
	{
		MakeRock();
		GotoState ('Active');
	}
}

state Active
{
Begin:
	// A loop which lasts for the total time allowed for the
	// effect, producing rocks at randomized intervals.
	MakeRock();
	NextRockTime = FRand()*(MaxBetweenTime-MinBetweenTime)+ MinBetweenTime;
	TotalPassedTime += NextRockTime;
	sleep (NextRockTime);
	if ( !TimeLimit || (TotalPassedTime < TimeLength) ) 
		goto 'RocksFall';
	Destroy();
}

defaultproperties
{
      CubeDimensions=(X=50.000000,Y=50.000000,Z=50.000000)
      TimeLength=+00010.000000
      MinBetweenTime=+00001.000000
      MaxBetweenTime=+00003.000000
      MinScaleFactor=+00000.500000
      MaxScaleFactor=+00001.000000
      Tag=Event1
      bStatic=False
      Texture=S_bubble2
}

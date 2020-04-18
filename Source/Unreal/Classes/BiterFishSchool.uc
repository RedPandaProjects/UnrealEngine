//=============================================================================
// BiterFishSchool.
//=============================================================================
class BiterFishSchool expands FlockMasterPawn;

var() byte schoolsize;
var  byte	activeFish;
var() float schoolradius;
var() bool	bNonAggressive;
var bool	validDest;
var bool	bSawPlayer;
var vector StartLocation;
var() byte fishcolor;

function PreSetMovement()
{
	bCanSwim = true;
	bCanFly = true;
	MinHitWall = -0.6;
}

function PostBeginPlay()
{
	StartLocation = Location;
	Super.PostBeginPlay();
}

singular function ZoneChange( ZoneInfo NewZone )
{
	local biterfish aFish;
	if (!NewZone.bWaterZone)
	{
		if ( !SetLocation(OldLocation) || (!Region.Zone.bWaterZone) )
			SetLocation(StartLocation);
		Velocity = vect(0,0,0);
		Acceleration = vect(0,0,0);
		MoveTimer = -1.0;
	}
	SetPhysics(PHYS_Swimming);
}

singular function HeadZoneChange( ZoneInfo NewZone )
{
	local biterfish aFish;
	if ( (MoveTarget!=Enemy) && !NewZone.bWaterZone)
	{
		Destination.Z = Location.Z - 50;
		Velocity = vect(0,0,0);
		Acceleration = vect(0,0,0);
		MoveTimer = -1.0;
	}
}

function FishDied()
{
	activeFish--;
	if (activeFish == 0)
		destroy();
}

function RemoveFish()
{
	local biterfish aFish;
	local Pawn aPawn;

	aPawn = Level.PawnList;
	While ( aPawn != None )
	{
		aFish = biterfish(aPawn);
		if ( (aFish != None) && (aFish.School == self) && !aFish.PlayerCanSeeMe() )
			Remove(aFish);
		aPawn = aPawn.NextPawn;
	}
}	

function Remove(biterfish aFish)
{
	aFish.Destroy();
	schoolsize++;
	activeFish--;
}

function SpawnFish()
{
	if ( schoolsize > 0 )
		Timer();
}

function Timer()
{
	if ( schoolsize > 0 )
		SpawnAFish();
	if ( schoolsize > 0 )
		SpawnAFish();
	if ( schoolsize > 0 )
		SpawnAFish();
	if ( schoolsize > 0 )
		SetTimer(0.1, false);
}

function SpawnAFish()
{
	local BiterFish fish;

	fish = spawn(class 'BiterFish',self,'', Location + VRand() * CollisionRadius);
	if (fish != None)
	{
		schoolsize--;
		activeFish++;
	}
}

auto state stasis
{
ignores EncroachedBy, FootZoneChange;
	
	function SeePlayer(Actor SeenPlayer)
	{
		enemy = Pawn(SeenPlayer);
		SpawnFish();
		Gotostate('wandering');
	}

Begin:
	SetPhysics(PHYS_None);
CleanUp:
	if ( activeFish > 0 )
	{
		Sleep(1.0);
		RemoveFish();
		Goto('Cleanup');
	}
}		

state wandering
{
ignores EncroachedBy, FootZoneChange;

	function SeePlayer(Actor SeenPlayer)
	{
		bSawPlayer = true;
		Enemy = Pawn(SeenPlayer);
		Disable('SeePlayer');
		Enable('EnemyNotVisible');
	}

	function EnemyNotVisible()
	{
		Enemy = None;
		Disable('EnemyNotVisible');
		Enable('SeePlayer');
	}
	
	function PickDestination()
	{
		local actor hitactor;
		local vector hitnormal, hitlocation;
		
		Destination = Location + VRand() * 1000;
		Destination.Z = 0.5 * (Destination.Z - 250 + Location.Z);
		HitActor = Trace(HitLocation, HitNormal, Destination, Location, false);
		if ( (HitActor != None) && (VSize(HitLocation - Location) < 1.5 * CollisionRadius) )
		{
			Destination = 2 * Location - Destination;
			HitActor = Trace(HitLocation, HitNormal, Destination, Location, false);
		}
		if (HitActor != None)
			Destination = HitLocation - CollisionRadius * Normal(Destination - Location);
	}
	
Begin:
	SetPhysics(PHYS_Swimming);
	
Wander:
	if (Enemy == None)
	{
		bSawPlayer = false;
		Sleep(5.0);
		if ( !bSawPlayer )
		{
			RemoveFish();
			GotoState('Stasis');
		}
		else if ( Enemy == None )
			Goto('Wander');
	}

	validDest = false;		
	if ( !bNonAggressive && (Enemy != None) && Enemy.Region.Zone.bWaterZone && !Enemy.Region.Zone.bPainZone)
		MoveToward(Enemy);	
	else
	{
		MoveTarget = None;
		PickDestination();
		MoveTo(Destination);
	}
	validDest = true;
	if ( FRand() < 0.1 )
		Sleep(5 + 6 * FRand());
	else
		Sleep(0.5 + 2 * FRand());
	Goto('Wander');
}

defaultproperties
{
     schoolsize=12
	 fishcolor=8
     schoolradius=+00120.000000
     UnderWaterTime=-00001.000000
     PeripheralVision=-00005.000000
     AirSpeed=+00800.000000
     WaterSpeed=+00800.000000
     AccelRate=+04000.000000
     bHidden=True
     CollisionRadius=+00050.000000
     CollisionHeight=+00100.000000
     bCollideActors=False
     bBlockActors=False
     bBlockPlayers=False
     bProjTarget=False
     Mass=+00010.000000
     Buoyancy=+00010.000000
     NetPriority=+00002.000000
}

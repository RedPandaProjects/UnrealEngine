//=============================================================================
// HorseFlySwarm.
//=============================================================================
class HorseFlySwarm expands FlockMasterPawn;

var()	byte	swarmsize; //number of horseflies in swarm
var		byte	totalflies;
var()   bool	bOnlyIfEnemy;
var()	float	swarmradius;
	
function PreBeginPlay()
{
	totalflies = swarmsize;
	Super.PreBeginPlay();
}

singular function ZoneChange( ZoneInfo NewZone )
{
	if (NewZone.bWaterZone /* || NewZone.bPainZone */)
	{
		SetLocation(OldLocation);
		Velocity = vect(0,0,0);
		Acceleration = vect(0,0,0);
		MoveTimer = -1.0;
		Enemy = None;
	}
}

function SpawnFlies()
{
	while (swarmsize > 0)
	{
		swarmsize--;
		spawn(class 'horsefly',self,'', Location + VRand() * CollisionRadius);
	}
}

auto state stasis
{
ignores EncroachedBy;
	
	function SeePlayer(Actor SeenPlayer)
	{
		enemy = Pawn(SeenPlayer);
		SpawnFlies();
		Gotostate('wandering');
	}

Begin:
	SetPhysics(PHYS_None);
}		

state wandering
{
ignores EncroachedBy;

	function SeePlayer(Actor SeenPlayer)
	{
		local actor newfly;
		Enemy = Pawn(SeenPlayer);
		SpawnFlies();
		Disable('SeePlayer');
		Enable('EnemyNotVisible');
	}

	function EnemyNotVisible()
	{
		Enemy = None;
		Disable('EnemyNotVisible');
	}
	
Begin:
	SetPhysics(PHYS_Flying);

Wander:
	if (Enemy == None)
		Enable('SeePlayer');
		
	if ( (Enemy != None) && !Enemy.Region.Zone.bWaterZone  && !Enemy.Region.Zone.bPainZone )
	{
		MoveToward(Enemy);
		sleep(2 * FRand());
	}	
	else
	{
		Destination = Location + VRand() * 1000;
		Destination.Z = 0.5 * (Destination.Z + Location.Z);
		MoveTo(Destination);
	}
	Goto('Wander');
}

defaultproperties
{
     swarmsize=20
     swarmradius=+00120.000000
     SightRadius=+02000.000000
     PeripheralVision=-00005.000000
     GroundSpeed=+00200.000000
     AirSpeed=+00200.000000
     JumpZ=-00001.000000
	 bOnlyIfEnemy=true
     bHidden=True
     bCollideActors=False
     bBlockActors=False
     bBlockPlayers=False
     bProjTarget=False
}

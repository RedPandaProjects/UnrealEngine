//=============================================================================
// BiterFish.
// Do not add directly - rather add BiterFishSchools to the world
//=============================================================================
class BiterFish expands FlockPawn;

#exec MESH IMPORT MESH=AmbientFish ANIVFILE=MODELS\fish1_a.3D DATAFILE=MODELS\fish1_d.3D ZEROTEX=1
#exec MESH ORIGIN MESH=AmbientFish X=00 Y=-20 Z=00 YAW=-64

#exec MESH SEQUENCE MESH=AmbientFish SEQ=All    STARTFRAME=0   NUMFRAMES=11
#exec MESH SEQUENCE MESH=AmbientFish SEQ=Swim1  STARTFRAME=0   NUMFRAMES=8
#exec MESH SEQUENCE MESH=AmbientFish SEQ=Bite   STARTFRAME=8   NUMFRAMES=3

#exec TEXTURE IMPORT NAME=Jfish21  FILE=MODELS\fish1.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=AmbientFish X=0.06 Y=0.06 Z=0.12
#exec MESHMAP SETTEXTURE MESHMAP=AmbientFish NUM=1 TEXTURE=Jfish21

#exec TEXTURE IMPORT NAME=Jfish22  FILE=MODELS\fish2.PCX GROUP=Skins 
#exec TEXTURE IMPORT NAME=Jfish23  FILE=MODELS\fish3.PCX GROUP=Skins 
#exec TEXTURE IMPORT NAME=Jfish24  FILE=MODELS\fish4.PCX GROUP=Skins 
#exec TEXTURE IMPORT NAME=Jfish25  FILE=MODELS\fish5.PCX GROUP=Skins 
#exec TEXTURE IMPORT NAME=Jfish26  FILE=MODELS\fish6.PCX GROUP=Skins 

var() byte BiteDamage;
var float AirTime;
var vector OldSchoolDestination;
var BiterFishSchool School;
var texture FishSkins[6];

function PostBeginPlay()
{
	School = BiterfishSchool(Owner); 
	Super.PostBeginPlay();
	if ( School == None )
	{
		log("Warning - can't add biterfish independently from biterfish schools");
		destroy();
	}
	else if ( School.fishcolor > 5 )
		skin = FishSkins[Rand(6)];
	else
		skin = FishSkins[School.fishcolor];
}

function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
{
	local bool bAlreadyDead;

	bAlreadyDead = (Health <= 0);

	if (Physics == PHYS_None)
		SetMovementPhysics();
	if (Physics == PHYS_Walking)
		momentum.Z = 0.4 * Vsize(momentum);
	if ( instigatedBy == self )
		momentum *= 0.6;
	momentum = momentum/Mass;
	AddVelocity( momentum ); 
	Health -= Damage;
	if ( Health < -20 )
	{
		Spawn(class'Bloodspurt');
		Destroy();
	}
	else if ( !bAlreadyDead && (Health < 0) )
		Died(instigatedBy, damageType, hitLocation);
}

function Landed(vector HitNormal)
{
	local rotator newRotation;

	SetPhysics(PHYS_None);
	SetTimer(0.2 + FRand(), false);
	newRotation = Rotation;
	newRotation.Pitch = 0;
	newRotation.Roll = 16384;
	SetRotation(newRotation);
	GotoState('Flopping');
}

function PreSetMovement()
{
	bCanSwim = true;
	if (Region.Zone.bWaterZone)
		SetPhysics(PHYS_Swimming);
	else
		SetPhysics(PHYS_Falling);
	MinHitWall = -0.6;
}

function ZoneChange( ZoneInfo NewZone )
{
	local rotator newRotation;
	if (NewZone.bWaterZone)
	{
		if ( !Region.Zone.bWaterZone && (Physics != PHYS_Swimming) )
		{
			newRotation = Rotation;
			newRotation.Roll = 0;
			SetRotation(newRotation);
			MoveTimer = -1.0;
		}
		AirTime = 0;
		SetPhysics(PHYS_Swimming);
		if (!IsInState('Swimming'));
			GotoState('Swimming');
	}
	else if (Physics != PHYS_Falling)
	{
		MoveTimer = -1;
		SetPhysics(PHYS_Falling);
	}
}

function FootZoneChange(ZoneInfo newFootZone)
{
	if ( (Level.TimeSeconds - SplashTime > 3) && 
	 	!FootRegion.Zone.bWaterZone && newFootZone.bWaterZone )
	{
		SplashTime = Level.TimeSeconds;
		PlaySound(sound 'LSplash', SLOT_Interact, 0.4,,500);
		Spawn(class 'WaterImpact',,,Location - CollisionHeight * vect(0,0,1));
	}
	
	if ( FootRegion.Zone.bPainZone )
	{
		if ( !newFootZone.bPainZone )
			PainTime = -1.0;
	}
	else if (newFootZone.bPainZone)
		PainTime = 0.01;
}

function Died(pawn Killer, name damageType, vector HitLocation)
{
	local rotator newRot;

	newRot = Rotation;
	if ( FRand() < 0.5 )
		newRot.Roll = 16384;
	else 
		newRot.Roll = -16384;
	SetRotation(newRot);
	SetPhysics(PHYS_Falling);
	SetCollision(true,false,false);
	Buoyancy = 1.05 * mass;
	Velocity.Z = FMax(0, Velocity.Z);
	AnimRate = 0.0;

	school.FishDied();
	RemoteRole = ROLE_DumbProxy;
	GotoState('Dying');
}

Auto State Swimming
{
	function PickDestination()
	{
		if ( (School.IsInState('Stasis')) && !PlayerCanSeeMe() )
		{
			School.Remove(self);
			return;
		}
		if ( School.validDest )
			OldSchoolDestination = School.Location;
		Destination = OldSchoolDestination +
			 0.5 * School.schoolradius * ( Normal(Location - School.Location) + VRand());
	}

	function Touch(Actor Other)
	{
		if ( Pawn(Other) == School.Enemy )
			Other.TakeDamage(2, self, location, vect(0,0,0), 'bitten');
	}
		
Begin:
	if ( School.bNonAggressive )
		Disable('Touch');
	if (!Region.Zone.bWaterZone)
		GotoState('Flopping');
	SetPhysics(PHYS_Swimming);
Swim:
	Enable('HitWall');
	LoopAnim('Swim1', 0.7 + FRand());
 	if ( (!School.bNonAggressive || (FRand() < 0.5)) 
		&& (School.Enemy!=None) && (School.MoveTarget == School.Enemy) && (FRand()<0.5) )
 		MoveToward(School.Enemy);
 	else
 	{
		PickDestination();
		MoveTo(Destination);
School:
		if ( (FRand() < 0.75) && (OldSchoolDestination == School.Location) 
			&& ((School.Enemy == None) || !School.Enemy.Region.Zone.bWaterZone) )
		{
			Velocity = vect(0,0,0);
			Acceleration = vect(0,0,0);
			Sleep(3.3 * FRand());
			Goto('School');
		}
 	}
	Velocity = vect(0,0,0);
	Acceleration = vect(0,0,0);
	Sleep(0.7 * FRand());
	Goto('Swim');
}

State Flopping
{

	function Landed(vector HitNormal)
	{
		local rotator newRotation;

		SetPhysics(PHYS_None);
		SetTimer(0.3 + 0.3 * AirTime * FRand(), false);
		newRotation = Rotation;
		newRotation.Pitch = 0;
		newRotation.Roll = 16384;
		SetRotation(newRotation);
	}
		
	function Timer()
	{
		AirTime += 1;
		if (AirTime > 25 + 20 * FRand())
			GotoState('Dying');
		else
		{
			SetPhysics(PHYS_Falling);
			Velocity = 200 * VRand();
			Velocity.Z = 60 + 160 * FRand();
			DesiredRotation.Pitch = Rand(8192) - 4096;
			DesiredRotation.Yaw = Rand(65535);
		}		
	}

	function AnimEnd()
	{
		if (FRand() < 0.5)
			PlayAnim('Swim1', 0.1 * FRand());
		else
			PlayAnim('Bite', 0.1 * FRand());
	}

	function BeginState()
	{
		SetPhysics(PHYS_Falling);
	}
}

State Dying
{
	ignores zonechange, headzonechange, falling, hitwall;

	function Landed(vector HitNormal)
	{
		SetPhysics(PHYS_None);
	}	

	function Timer()
	{
		if ( !PlayerCanSeeMe() )
			Destroy();
	}

Begin:
	Sleep(12);
	SetTimer(4.0, true);
}			
		
defaultproperties
{
	 Fishskins(0)=texture'Jfish21'
	 Fishskins(1)=texture'Jfish22'
	 Fishskins(2)=texture'Jfish23'
	 Fishskins(3)=texture'Jfish24'
	 Fishskins(4)=texture'Jfish25'
	 Fishskins(5)=texture'Jfish26'
     Health=5
     ReducedDamageType=exploded
     ReducedDamagePct=+00000.900000
     UnderWaterTime=-00001.000000
     bCanStrafe=True
     SightRadius=+04100.000000
     WaterSpeed=+00180.000000
     AccelRate=+00700.000000
     DrawType=DT_Mesh
     Mesh=AmbientFish
	 Skin=JFish21
     CollisionRadius=+00008.000000
     CollisionHeight=+00006.000000
     bBlockPlayers=False
     bProjTarget=True
     Mass=+00005.000000
     Buoyancy=+00005.000000
     RotationRate=(Pitch=8192,Yaw=128000,Roll=16384)
}

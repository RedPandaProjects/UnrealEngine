//=============================================================================
// Manta.
//=============================================================================
class Manta expands ScriptedPawn;

#exec MESH IMPORT MESH=Manta1 ANIVFILE=MODELS\manta_a.3D DATAFILE=MODELS\manta_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=Manta1 X=0 Y=-50 Z=100 ROLL=-64 YAW=64

#exec MESH SEQUENCE MESH=Manta1 SEQ=All		STARTFRAME=0	NUMFRAMES=152
#exec MESH SEQUENCE MESH=Manta1 SEQ=Death	STARTFRAME=0	NUMFRAMES=26
#exec MESH SEQUENCE MESH=Manta1 SEQ=Fly		STARTFRAME=26	NUMFRAMES=20
#exec MESH SEQUENCE MESH=Manta1 SEQ=Fighter STARTFRAME=26	NUMFRAMES=1
#exec MESH SEQUENCE MESH=Manta1 SEQ=Landing STARTFRAME=46   NUMFRAMES=26
#exec MESH SEQUENCE MESH=Manta1 SEQ=Sting	STARTFRAME=72	NUMFRAMES=30  Group=Attack
#exec MESH SEQUENCE MESH=Manta1 SEQ=ThreatSting  STARTFRAME=72  NUMFRAMES=30
#exec MESH SEQUENCE MESH=Manta1 SEQ=TakeOff STARTFRAME=102  NUMFRAMES=20
#exec MESH SEQUENCE MESH=Manta1 SEQ=Whip	STARTFRAME=122	NUMFRAMES=30  Group=Attack
#exec MESH SEQUENCE MESH=Manta1 SEQ=TakeHit STARTFRAME=4	NUMFRAMES=1
#exec MESH SEQUENCE MESH=Manta1 SEQ=Waiting STARTFRAME=66   NUMFRAMES=5

#exec TEXTURE IMPORT NAME=JManta1 FILE=MODELS\manta.PCX GROUP="Skins" FLAGS=2
#exec MESHMAP SCALE MESHMAP=Manta1 X=0.07 Y=0.07 Z=0.14
#exec MESHMAP SETTEXTURE MESHMAP=Manta1 NUM=0 TEXTURE=JManta1

#exec MESH NOTIFY MESH=Manta1 SEQ=Death TIME=0.76 FUNCTION=LandThump

#exec AUDIO IMPORT FILE="Sounds\Manta\whip1a.WAV" NAME="whip1m" GROUP="Manta"
#exec AUDIO IMPORT FILE="Sounds\Manta\injur1a.WAV" NAME="injur1m" GROUP="Manta"
#exec AUDIO IMPORT FILE="Sounds\Manta\injur2a.WAV" NAME="injur2m" GROUP="Manta"
#exec AUDIO IMPORT FILE="Sounds\Manta\call1a.WAV" NAME="call1m" GROUP="Manta"
#exec AUDIO IMPORT FILE="Sounds\Manta\call2a.WAV" NAME="call2m" GROUP="Manta"
#exec AUDIO IMPORT FILE="Sounds\Manta\fly1a.WAV" NAME="fly1m" GROUP="Manta"
#exec AUDIO IMPORT FILE="Sounds\Manta\death2c.WAV" NAME="death2m" GROUP="Manta"
#exec AUDIO IMPORT FILE="Sounds\Manta\sting1a.WAV" NAME="sting1m" GROUP="Manta"
#exec AUDIO IMPORT FILE="Sounds\Manta\land1mt.WAV" NAME="land1mt" GROUP="Manta"

//-----------------------------------------------------------------------------
// Manta variables.

// Attack damage.
var() byte
	StingDamage,	// Basic damage done by Sting.
	WhipDamage;		// Basic damage done by whip.
var bool bAttackBump;
var bool bAvoidHit;
var(Sounds) sound whip;
var(Sounds) sound wingBeat;
var(Sounds) sound sting;

//-----------------------------------------------------------------------------
// Manta functions.

/* PreSetMovement()
default for walking creature.  Re-implement in subclass
for swimming/flying capability
*/

function PreBeginPlay()
{
	Super.PreBeginPlay();
	if ( skill <= 1 )
		Health = 0.6 * Health;
	if ( skill == 0 )
		AttitudeToPlayer = ATTITUDE_Ignore;
}

function PreSetMovement()
{
	MaxDesiredSpeed = 0.6 + 0.13 * skill;
	bCanJump = true;
	bCanWalk = true;
	bCanSwim = true;
	bCanFly = true;
	MinHitWall = -0.6;
	if (Intelligence > BRAINS_Reptile)
		bCanOpenDoors = true;
	if (Intelligence == BRAINS_Human)
		bCanDoSpecial = true;
}

function SetMovementPhysics()
{
	if (Region.Zone.bWaterZone)
		SetPhysics(PHYS_Swimming);
	else
		SetPhysics(PHYS_Flying); 
}

function PlayWaiting()
	{
	LoopAnim('Waiting', 0.1 + 0.5 * FRand());
	}

function PlayPatrolStop()
	{
	PlaySound(WingBeat, SLOT_Interact);
	LoopAnim('Fly', 0.4);
	}

function PlayWaitingAmbush()
	{
	PlayWaiting();
	}

function PlayChallenge()
{
	PlayAnim('Fly', 0.4, 0.1);
}

function TweenToFighter(float tweentime)
{
	TweenAnim('Fly', tweentime);
}

function TweenToRunning(float tweentime)
{
	if ( (AnimSequence != 'Fly') || !bAnimLoop )
		TweenAnim('Fly', tweentime);
}

function TweenToWalking(float tweentime)
{
	if ( (AnimSequence != 'Fly') || !bAnimLoop )
		TweenAnim('Fly', tweentime);
}

function TweenToWaiting(float tweentime)
{
	PlayAnim('Landing', 0.2 + 0.8 * FRand());
	SetPhysics(PHYS_Falling);
}

function TweenToPatrolStop(float tweentime)
{
	TweenAnim('Fly', tweentime);
}

function PlayRunning()
{
	PlaySound(WingBeat, SLOT_Interact);
	LoopAnim('Fly', -1.0/AirSpeed,, 0.4);
}

function PlayWalking()
{
	PlaySound(WingBeat, SLOT_Interact);
	LoopAnim('Fly', -1.0/AirSpeed,, 0.4);
}

function PlayThreatening()
{
	if ( FRand() < 0.8 ) 
	{
		PlaySound(WingBeat, SLOT_Interact);
		PlayAnim('Fly', 0.4);
	}
	else
		LoopAnim('ThreatSting', 0.4);
}

function PlayTurning()
{
	if (FRand() < 0.5)
		TweenAnim('Fighter', 0.2);
	else
	{
		PlaySound(WingBeat, SLOT_Interact);
		LoopAnim('Fly', -1.0/AirSpeed,, 0.4);
	}
}

function PlayDying(name DamageType, vector HitLocation)
{
	PlaySound(Die, SLOT_Talk, 4 * TransientSoundVolume);
	PlayAnim('Death', 0.7, 0.1);
}

function PlayTakeHit(float tweentime, vector HitLoc, int Damage)
{
	TweenAnim('TakeHit', tweentime);
}

function TweenToFalling()
{
	TweenAnim('Fly', 0.2);
}

function PlayInAir()
{
	LoopAnim('Fly');
}

function PlayLanded(float impactVel)
{
	PlayAnim('Landing');
}


function PlayVictoryDance()
{
	PlayAnim('Whip', 0.6, 0.1);
	PlaySound(Threaten, SLOT_Talk);		
}
	
function PlayMeleeAttack()
{
	local vector adjust;
	adjust = vect(0,0,0.8) * Target.CollisionHeight;
	Acceleration = AccelRate * Normal(Target.Location - Location + adjust);
	Enable('Bump');
	bAttackBump = false;
	//log("Start Melee Attack");
	if (FRand() < 0.5)
		{
		PlayAnim('Sting');
		PlaySound(Sting, SLOT_Interact);	 		
 		}
	else
	{
 		PlayAnim('Whip');
 		PlaySound(Whip, SLOT_Interact); 
 	}	
 }

state MeleeAttack
{
ignores SeePlayer, HearNoise;

	singular function Bump(actor Other)
	{
		Disable('Bump');
		if (AnimSequence == 'Whip')
			MeleeDamageTarget(WhipDamage, (WhipDamage * 1000.0 * Normal(Target.Location - Location)));
		else if (AnimSequence == 'Sting')
			MeleeDamageTarget(StingDamage, (StingDamage * 1000.0 * Normal(Target.Location - Location)));
		bAttackBump = true;
		Velocity *= -0.5;
		Acceleration *= -1;
		if (Acceleration.Z < 0)
			Acceleration.Z *= -1;
	}

	function KeepAttacking()
	{
		if (Target == None) 
			GotoState('Attacking');
		else if ( (Pawn(Target) != None) && (Pawn(Target).Health == 0) )
			GotoState('Attacking');
		else if (bAttackBump)
		{
			SetTimer(TimeBetweenAttacks, false);
			GotoState('TacticalMove', 'NoCharge');
		}
	}
}		

state Charging
{
ignores SeePlayer, HearNoise;

	function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
							Vector momentum, name damageType)
	{
		Global.TakeDamage(Damage, instigatedBy, hitlocation, momentum, damageType);
		if ( health <= 0 )
			return;
		if (NextState == 'TakeHit')
		{
			if (AttitudeTo(Enemy) == ATTITUDE_Fear)
			{
				NextState = 'Retreating';
				NextLabel = 'Begin';
			}
			else if ( (FRand() < 3 * Damage/Default.Health) && ((Damage > 0.5 * Health) || (VSize(Location - Enemy.Location) > 150)) )
			{
				bAvoidHit = true;
				NextState = 'TacticalMove';
				NextLabel = 'NoCharge';
			}
			else
			{
				NextState = 'Charging';
				NextLabel = 'TakeHit';
			}
			GotoState('TakeHit'); 
		}
	}
}

state TacticalMove
{
ignores SeePlayer, HearNoise;

	function PickDestination(bool bNoCharge)
	{
		local vector pick, pickdir, enemydir,Y, minDest;
		local actor HitActor;
		local vector HitLocation, HitNormal, collSpec;
		local float Aggression, enemydist, minDist, strafeSize, MaxMove;

		if ( bAvoidHit && (FRand() < 0.7) )
			MaxMove = 300;
		else 
			MaxMove = 600;
				
		bAvoidHit = false;
		enemyDist = VSize(Location - Enemy.Location);
		Aggression = 2 * FRand() - 1.0;

		if (enemyDist < CollisionRadius + Enemy.CollisionRadius + 2 * MeleeRange)
			Aggression = FMin(0.0, Aggression);	
		else if (enemyDist > FMax(VSize(OldLocation - Enemy.OldLocation), 240))
			Aggression = FMax(0.0, Aggression);
		
		enemydir = (Enemy.Location - Location)/enemyDist;
		minDist = FMin(160.0, 5*CollisionRadius);
		Y = (enemydir Cross vect(0,0,1));
		strafeSize = FMin(0.8, (2 * Abs(Aggression) * FRand() - 0.2));
		if (Aggression <= 0)
			strafeSize *= -1;
		enemydir = enemydir * strafeSize;
		if (FRand() < 0.8)
			enemydir.Z = 1.5;
		else
			enemydir.Z = FMax(0,enemydir.Z);

		strafeSize = FMax(0.0, 1 - Abs(strafeSize));
		pickdir = strafeSize * Y;
		pick = Location + (pickdir + enemydir) * (minDist + MaxMove * FRand());
		pick.Z = Location.Z + 60 + 0.65 * MaxMove * FRand();
		minDest = Location + minDist * Normal(pick - location); 
		collSpec.X = CollisionRadius;
		collSpec.Y = CollisionRadius;
		collSpec.Z = CollisionHeight;
		
		HitActor = Trace(HitLocation, HitNormal, minDest, Location, false, collSpec);
		if ( HitActor == None )
		{
			Destination = pick;
			return;	
		}
		pick = Location + (enemydir - pickdir) * (minDist + MaxMove * FRand());
		pick.Z = Location.Z + 60 + 0.5 * MaxMove * FRand();
		minDest = Location + minDist * Normal(pick - location); 
		HitActor = Trace(HitLocation, HitNormal, minDest, Location, false, collSpec);
		if ( HitActor == None )
		{
			Destination = pick;
			return;	
		}
		
		pick = Location - enemydir * (minDist + MaxMove * FRand());
		pick.Z = Location.Z + 0.5 * MaxMove * FRand();
		minDest = Location + Normal(pick - Location) * minDist;
		HitActor = Trace(HitLocation, HitNormal, minDest, Location, false, collSpec);
		if ( HitActor == None )
		{
			Destination = pick;
			return;	
		}
		
		if ( !bNoCharge && (enemyDist > 120) )
			GotoState('Charging');	
		
		pick = MaxMove * FRand() * VRand();	
		pick.Z = FMin(Location.Z, pick.Z);
		Destination = pick;	 
	}



	function Bump(Actor Other)
	{
		Disable('Bump');
		if (bAttackBump == true)
			bAttackBump = false;
		else if (Other == Enemy)
			{
			bReadyToAttack = true;
			Target = Enemy;
			GotoState('MeleeAttack');
			}
		else if (Enemy.Health <= 0)
			GotoState('Attacking');
	}
}

defaultproperties
{
     StingDamage=20
     WhipDamage=20
	 Health=78
     Whip=Unreal.whip1m
     wingBeat=Unreal.fly1m
     Sting=Unreal.sting1m
     CarcassType=Unreal.MantaCarcass
     Aggressiveness=+00000.200000
     ReFireRate=+00000.500000
     WalkingSpeed=+00000.600000
     bIsWuss=True
     Acquire=Unreal.call1m
     Fear=Unreal.injur2m
     Roam=Unreal.call2m
     Threaten=Unreal.call2m
     UnderWaterTime=-00001.000000
     SightRadius=+01500.000000
     MeleeRange=+00120.000000
     WaterSpeed=+00300.000000
     AirSpeed=+00400.000000
     AccelRate=+00800.000000
     JumpZ=+00010.000000
     HitSound1=Unreal.injur1m
     HitSound2=Unreal.injur2m
     Land=Unreal.land1mt
     Die=Unreal.death2m
     WaterStep=None
     CombatStyle=-00000.300000
     DrawType=DT_Mesh
     Mesh=Unreal.Manta1
     CollisionRadius=+00027.000000
     CollisionHeight=+00012.000000
     Mass=+00080.000000
     Buoyancy=+00080.000000
     RotationRate=(Pitch=16384,Yaw=55000,Roll=15000)
}

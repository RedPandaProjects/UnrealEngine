//=============================================================================
// Squid.
//=============================================================================
class Squid expands ScriptedPawn;

#exec MESH IMPORT MESH=Squid1 ANIVFILE=MODELS\squid_a.3D DATAFILE=MODELS\squid_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Squid1 X=0 Y=0 Z=-100 YAW=-64 ROLL=-64 

#exec MESH SEQUENCE MESH=squid1 SEQ=All		STARTFRAME=0	NUMFRAMES=142
#exec MESH SEQUENCE MESH=squid1 SEQ=Dead1	STARTFRAME=0	NUMFRAMES=21	RATE=15
#exec MESH SEQUENCE MESH=squid1 SEQ=TakeHit STARTFRAME=0	NUMFRAMES=1
#exec MESH SEQUENCE MESH=squid1 SEQ=Fighter STARTFRAME=21   NUMFRAMES=15	RATE=15
#exec MESH SEQUENCE MESH=squid1 SEQ=Grab	STARTFRAME=36   NUMFRAMES=13	RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=squid1 SEQ=Hold	STARTFRAME=49   NUMFRAMES=15	RATE=15
#exec MESH SEQUENCE MESH=squid1 SEQ=Release STARTFRAME=64   NUMFRAMES=6
#exec MESH SEQUENCE MESH=squid1 SEQ=Slap	STARTFRAME=70   NUMFRAMES=13	RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=squid1 SEQ=Spin	STARTFRAME=83   NUMFRAMES=10	RATE=15
#exec MESH SEQUENCE MESH=squid1 SEQ=Swim	STARTFRAME=93   NUMFRAMES=20	RATE=15
#exec MESH SEQUENCE MESH=squid1 SEQ=Thrust  STARTFRAME=113  NUMFRAMES=18	RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=squid1 SEQ=Turn	STARTFRAME=131  NUMFRAMES=11	RATE=15

#exec TEXTURE IMPORT NAME=JSquid1 FILE=MODELS\squid.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=Squid1 X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=squid1 NUM=1 TEXTURE=Jsquid1

#exec MESH NOTIFY MESH=Squid1 SEQ=Grab TIME=0.54 FUNCTION=GrabTarget
#exec MESH NOTIFY MESH=Squid1 SEQ=Slap TIME=0.36 FUNCTION=SlapTarget
#exec MESH NOTIFY MESH=Squid1 SEQ=Thrust TIME=0.23 FUNCTION=ThrustTarget

#exec AUDIO IMPORT FILE="Sounds\Squid\amb1sq.WAV" NAME="amb1sq" GROUP="Squid"
#exec AUDIO IMPORT FILE="Sounds\Squid\death1sq.WAV" NAME="death1sq" GROUP="Squid"
#exec AUDIO IMPORT FILE="Sounds\Squid\grab1sq.WAV" NAME="grab1sq" GROUP="Squid"
#exec AUDIO IMPORT FILE="Sounds\Squid\hit1sq.WAV" NAME="hit1sq" GROUP="Squid"
#exec AUDIO IMPORT FILE="Sounds\Squid\slap1sq.WAV" NAME="slap1sq" GROUP="Squid"
#exec AUDIO IMPORT FILE="Sounds\Squid\thrust1sq.WAV" NAME="thrust1sq" GROUP="Squid"
#exec AUDIO IMPORT FILE="Sounds\Squid\injur1sq.WAV" NAME="injur1sq" GROUP="Squid"
#exec AUDIO IMPORT FILE="Sounds\Squid\injur2sq.WAV" NAME="injur2sq" GROUP="Squid"
#exec AUDIO IMPORT FILE="Sounds\Squid\turn1sq.WAV" NAME="turn1sq" GROUP="Squid"

//-----------------------------------------------------------------------------
// Squid variables.

// Attack damage.
var() byte
	ThrustDamage,		// Basic damage done by bite.
	SlapDamage;

var(Sounds) sound thrust;
var(Sounds) sound slapgrabhit;
var(Sounds) sound thrusthit;
var(Sounds) sound slap;
var(Sounds) sound turn;
var(Sounds) sound grab;
var(Sounds) sound spin;
var(Sounds) sound flop;

//-----------------------------------------------------------------------------
// Squid functions.

function ZoneChange(ZoneInfo newZone)
{
	local vector start, checkpoint, HitNormal, HitLocation;
	local actor HitActor;
	
	if ( newZone.bWaterZone )
	{
		if (Physics != PHYS_Swimming)
			setPhysics(PHYS_Swimming);
	}
	else if (Physics == PHYS_Swimming)
	{
		SetPhysics(PHYS_Falling);
		MoveTimer = -1.0;
		GotoState('Flopping');
	}
}

function PreSetMovement()
{
	bCanJump = true;
	bCanWalk = false;
	bCanSwim = true;
	bCanFly = false;
	MinHitWall = -0.6;
	bCanOpenDoors = false;
	bCanDoSpecial = false;
}

function SetMovementPhysics()
{
	if (Region.Zone.bWaterZone)
		SetPhysics(PHYS_Swimming);
	else
	{
		SetPhysics(PHYS_Falling);
		MoveTimer = -1.0;
		GotoState('Flopping');
	} 
}

function PlayWaiting()
	{
	LoopAnim('Fighter', 0.1 + 0.3 * FRand());
	}

function PlayPatrolStop()
	{
	LoopAnim('Fighter', 0.1 + 0.3 * FRand());
	}

function PlayWaitingAmbush()
	{
	LoopAnim('Fighter', 0.1 + 0.3 * FRand());
	}

function PlayChallenge()
{
	PlayAnim('Fighter', 0.4, 0.2);
}

function TweenToFighter(float tweentime)
{
	TweenAnim('Fighter', tweentime);
}

function TweenToRunning(float tweentime)
{
	if ( (AnimSequence != 'Swim') || !bAnimLoop )
		TweenAnim('Swim', tweentime);
}

function TweenToWalking(float tweentime)
{
	if ( (AnimSequence != 'Swim') || !bAnimLoop )
		TweenAnim('Swim', tweentime);
}

function TweenToWaiting(float tweentime)
{
	PlayAnim('Fighter', 0.2 + 0.8 * FRand(), 0.3);
}

function TweenToPatrolStop(float tweentime)
{
	TweenAnim('Fighter', tweentime);
}

function PlayRunning()
{
	if ( ((AnimSequence == 'Spin') && (FRand() < 0.8)) || (FRand() < 0.06) )
		LoopAnim('Spin');
	else
		LoopAnim('Swim', -0.8/WaterSpeed,, 0.4);
}

function PlayWalking()
{
	LoopAnim('Swim', -0.8/WaterSpeed,, 0.4);
}

function PlayThreatening()
{
	if ( FRand() < 0.6 )
		PlayAnim('Swim', 0.4);
	else
	{
		PlaySound(Spin, SLOT_Interact);
		PlayAnim('Spin', 0.4);
	}
}

function PlayTurning()
{
	PlaySound(turn, SLOT_Interact);
	LoopAnim('Turn', 0.4);
}

function PlayDying(name DamageType, vector HitLocation)
{
	PlaySound(Die, SLOT_Talk, 4 * TransientSoundVolume);
	PlayAnim('Dead1', 0.7, 0.1);
}

function PlayTakeHit(float tweentime, vector HitLoc, int damage)
{
	TweenAnim('TakeHit', tweentime);
}

function TweenToFalling()
{
	DesiredRotation = Rotation;
	DesiredRotation.Pitch = 0;
	TweenAnim('Spin', 0.2);
}

function PlayInAir()
{
	LoopAnim('Fighter', 0.7);
}

function PlayLanded(float impactVel)
{
	TweenAnim('Spin', 0.2);
}

function PlayVictoryDance()
{
	PlayAnim('grab', 0.6, 0.1);
	PlaySound(Grab, SLOT_Interact);
}


function GrabTarget()
{
	if ( MeleeDamageTarget(SlapDamage, (SlapDamage * 1500.0 * Normal(Location - Target.Location))) )
		PlaySound(SlapGrabHit, SLOT_Interact);
}

function SlapTarget()
{
	if ( MeleeDamageTarget(SlapDamage, (SlapDamage * 1500.0 * Normal(Target.Location - Location))) )
		PlaySound(SlapGrabHit, SLOT_Interact);
}

function ThrustTarget()
{
	if ( MeleeDamageTarget(ThrustDamage, (ThrustDamage * 1500.0 * Normal(Target.Location - Location))) )
		PlaySound(ThrustHit, SLOT_Interact);
}
	
//FIXME - hold (turn off client's physics???
function PlayMeleeAttack()
{
	local float decision;
	decision = FRand();
	if (decision < 0.35)
	{
		PlaySound(Thrust, SLOT_Interact);
		PlayAnim('Thrust', 0.8);
	}
	if (decision < 0.7)
	{
		PlaySound(Slap, SLOT_Interact);
		PlayAnim('Slap', 0.8);
	}
	else 
	{
		PlaySound(Grab, SLOT_Interact);
		PlayAnim('Grab'); 
 	}
}


function bool MeleeDamageTarget(int hitdamage, vector pushdir)
	{
	local vector HitLocation, HitNormal, TargetPoint;
	local float TargetDist;
	local actor HitActor;
	local bool result;
	
	result = false;
	TargetDist = VSize(Target.Location - Location);
	Acceleration = AccelRate * (Target.Location - Location)/TargetDist;
	If (TargetDist <= (MeleeRange * 1.4 + Target.CollisionRadius + CollisionRadius)) //still in melee range
		{
		TargetPoint = Location - TargetDist * vector(Rotation); 
		TargetPoint.Z = FMin(TargetPoint.Z, Target.Location.Z + Target.CollisionHeight);
		TargetPoint.Z = FMax(TargetPoint.Z, Target.Location.Z - Target.CollisionHeight);
		HitActor = Trace(HitLocation, HitNormal, TargetPoint, Location, true);
		If (HitActor == Target)
			{	
			Target.TakeDamage(hitdamage, Self,HitLocation, pushdir, 'hacked');
			result = true;
			}
		}
	return result;
	}


State Flopping
{
ignores seeplayer, hearnoise, enemynotvisible, hitwall; 	
	function Timer()
	{
		SetPhysics(PHYS_Falling);
		Velocity = 200 * VRand();
		Velocity.Z = 170 + 200 * FRand();
		DesiredRotation.Pitch = Rand(8192) - 4096;
		DesiredRotation.Yaw = Rand(65535);
	}
	
	function ZoneChange( ZoneInfo NewZone )
	{
		local Rotator newRotation;
		if (NewZone.bWaterZone)
		{
			newRotation = Rotation;
			newRotation.Roll = 0;
			SetRotation(newRotation);
			SetPhysics(PHYS_Swimming);
			GotoState('Attacking');
		}
		else if (Physics != PHYS_Falling)
			SetPhysics(PHYS_Falling);
	}
	
	function Landed(vector HitNormal)
	{
		SetPhysics(PHYS_None);
		DesiredRotation.Pitch = 0;
		SetTimer(0.3 + FRand(), false);
	}
	
	function AnimEnd()
	{
		PlayAnim('Spin', 0.7);
	}

Begin:
	log("Flopping");
	SetTimer(0.3 + FRand(), false);
	TweenAnim('Flopping', 0.7);
}

state TacticalMove
{
ignores SeePlayer, HearNoise;

	function Timer()
	{
		Spawn(class'BigBlackSmoke');
	}
	
	function BeginState()
	{
		SetTimer(0.2, true);
		Super.BeginState();
	}
}

//squid has own melee attack because he faces away from his target when attacking
state MeleeAttack
{
ignores SeePlayer, HearNoise, Bump;
	function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
							Vector momentum, name damageType)
	{
		Global.TakeDamage(Damage, instigatedBy, hitlocation, momentum, damageType);
		if ( health <= 0 )
			return;
		if (NextState == 'TakeHit')
		{
			NextState = 'MeleeAttack';
			NextLabel = 'Begin';
		}
	}

	function KeepAttacking()
	{
		bReadyToAttack = true;
		if ( (Target == None) 
			|| ((Pawn(Target) != None) && (Pawn(Target).Health == 0)) )
			GotoState('Attacking');
		else if (VSize(Target.Location - Location) > (0.9 * MeleeRange + Target.CollisionRadius + CollisionRadius)) 
			GotoState('TacticalMove', 'NoCharge');
	}

	function EnemyNotVisible()
	{
		//log("enemy not visible");
		GotoState('Attacking');
	}

	function AnimEnd()
	{
		GotoState('MeleeAttack', 'DoneAttacking');
	}
	
	function BeginState()
	{
		Disable('AnimEnd');
		bCanStrafe = true; //so he can turn in place
	}

	function EndState()
	{
		bCanStrafe = false;
	}
	
Begin:
	if (Target == None)
		Target = Enemy;
	
FaceTarget:
	Acceleration = Vect(0,0,0);
	if (NeedToTurn(2 * Location - Target.Location))
	{
		PlayTurning();
		TurnTo(2 * Location - Target.Location);
		TweenToFighter(0.15);
	}
	else if ( (5 - Skill) * FRand() > 3 ) 
	{
		DesiredRotation = Rotator(Location - Target.Location);
		PlayChallenge(); 
	}
	
	FinishAnim();
		
	if (VSize(Location - Target.Location) > MeleeRange + CollisionRadius + Target.CollisionRadius)
		GotoState('Attacking'); 

ReadyToAttack:
	DesiredRotation = Rotator(Location - Target.Location);
	PlayMeleeAttack();
	Enable('AnimEnd');
Attacking:
	TurnTo(2 * Location - Target.Location);
	Goto('Attacking');
DoneAttacking:
	Disable('AnimEnd');
	KeepAttacking();
	Goto('FaceTarget');
}

	

defaultproperties
{
     ThrustDamage=35
     SlapDamage=30
     Thrust=thrust1sq
     slapgrabhit=hit1sq
     thrusthit=hit1sq
     Slap=slap1sq
     Grab=grab1sq
	 turn=turn1sq
     Aggressiveness=+00000.800000
     Health=260
     SightRadius=+02000.000000
     PeripheralVision=-00000.500000
     MeleeRange=+00070.000000
     Intelligence=BRAINS_REPTILE
     GroundSpeed=+00000.000000
     AirSpeed=+00000.000000
     WaterSpeed=+00260.000000
     MaxStepHeight=+00025.000000
     HitSound1=injur1sq
     HitSound2=injur2sq
     Die=death1sq
     CombatStyle=+00000.000000
     DrawType=DT_Mesh
     Mesh=Squid1
     AmbientSound=amb1sq
     CollisionRadius=+00040.000000
     CollisionHeight=+00060.000000
     Mass=+00200.000000
     Buoyancy=+00200.000000
     RotationRate=(Pitch=13000,Roll=13000)
}

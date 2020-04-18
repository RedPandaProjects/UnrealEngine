//=============================================================================
// Fly.
//=============================================================================
class Fly expands ScriptedPawn;

#exec MESH IMPORT MESH=FlyM ANIVFILE=MODELS\fly_a.3D DATAFILE=MODELS\fly_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=FlyM X=0 Y=-30 Z=70 YAW=64 ROLL=-62

#exec MESH SEQUENCE MESH=FlyM SEQ=All      STARTFRAME=0     NUMFRAMES=97
#exec MESH SEQUENCE MESH=FlyM SEQ=Dead     STARTFRAME=0     NUMFRAMES=16
#exec MESH SEQUENCE MESH=FlyM SEQ=TakeHit  STARTFRAME=0     NUMFRAMES=1
#exec MESH SEQUENCE MESH=FlyM SEQ=Flying   STARTFRAME=16    NUMFRAMES=10	RATE=15
#exec MESH SEQUENCE MESH=FlyM SEQ=Land     STARTFRAME=26    NUMFRAMES=11	RATE=15
#exec MESH SEQUENCE MESH=FlyM SEQ=Shoot1   STARTFRAME=37    NUMFRAMES=25 RATE=40  Group=Attack
#exec MESH SEQUENCE MESH=FlyM SEQ=Shoot2   STARTFRAME=62    NUMFRAMES=10   Group=Attack
#exec MESH SEQUENCE MESH=FlyM SEQ=Takeoff  STARTFRAME=72    NUMFRAMES=15
#exec MESH SEQUENCE MESH=FlyM SEQ=Walking  STARTFRAME=87   NUMFRAMES=10 RATE=15
#exec MESH SEQUENCE MESH=FlyM SEQ=Waiting  STARTFRAME=88   NUMFRAMES=1

#exec TEXTURE IMPORT NAME=JFly1 FILE=MODELS\fly.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=flyM X=0.06 Y=0.06 Z=0.12
#exec MESHMAP SETTEXTURE MESHMAP=flyM NUM=1 TEXTURE=Jfly1

#exec AUDIO IMPORT FILE="Sounds\Razorfly\buzz3rf.WAV" NAME="buzz3rf" GROUP="Razorfly"
#exec AUDIO IMPORT FILE="Sounds\Razorfly\injur1rf.WAV" NAME="injur1rf" GROUP="Razorfly"
#exec AUDIO IMPORT FILE="Sounds\Razorfly\injur2rf.WAV" NAME="injur2rf" GROUP="Razorfly"
#exec AUDIO IMPORT FILE="Sounds\Razorfly\death1rf.WAV" NAME="death1rf" GROUP="Razorfly"


//-----------------------------------------------------------------------------
// Fly variables.

//-----------------------------------------------------------------------------
// Fly functions.

function PreSetMovement()
{
	bCanJump = true;
	bCanWalk = true;
	bCanSwim = false;
	bCanFly = true;
	MinHitWall = -0.6;
	bCanOpenDoors = false;
	bCanDoSpecial = false;
}

function ZoneChange(ZoneInfo newZone)
{
	local vector jumpDir;

	if ( newZone.bWaterZone )
	{
		MoveTimer = -1.0;
		if ( (Enemy != None) && (Enemy.Location.Z < Location.Z) )
			GotoState('TacticalMove', 'BackOff');
		else
			Acceleration = Accelrate * vect(0,0,1);
	}

}

function SetMovementPhysics()
{
	if (Enemy != None)
		SetPhysics(PHYS_Flying); 
	else if (Physics != PHYS_Falling)
		SetPhysics(PHYS_Walking);
}

singular function Falling()
{
	SetPhysics(PHYS_Flying);
	if (bIsPlayer)
	{
		PlayInAir();
		return;
	}
		
	if (health > 0)
		SetFall();
}

function PlayWaiting()
{
	if ( Physics == PHYS_Walking )
		TweenAnim('Waiting', 10.0);
	else
		LoopAnim('Flying', 0.75);
}

function PlayPatrolStop()
{
	PlayWaiting();
}

function PlayWaitingAmbush()
{
	PlayWaiting();
}

function PlayChallenge()
{
	PlayAnim('Shoot1', 1.0, 0.1);
}

function TweenToFighter(float tweentime)
{
	TweenAnim('Flying', tweentime);
}

function TweenToRunning(float tweentime)
{
	if ( (AnimSequence != 'Flying') || !bAnimLoop )
		TweenAnim('Flying', tweentime);
}

function TweenToWalking(float tweentime)
{
	if (Physics == PHYS_Walking)
		TweenAnim('Walking', tweentime);
	else if ( (AnimSequence != 'Flying') || !bAnimLoop )
		TweenAnim('Flying', tweentime);
}

function TweenToWaiting(float tweentime)
{
	PlayAnim('Land', 0.2 + 0.5 * FRand());
	SetPhysics(PHYS_Falling);
}

function TweenToPatrolStop(float tweentime)
{
	TweenAnim('Flying', tweentime);
}

function PlayRunning()
{
	LoopAnim('Flying');
}

function PlayWalking()
{
	if (Physics == PHYS_Walking)
		LoopAnim('Walking', -1.0/GroundSpeed,, 0.4);
	else
		LoopAnim('Flying');
}

function PlayThreatening()
{
	if ( FRand() < 0.8 )
		LoopAnim('Flying');
	else
		LoopAnim('Shoot1', 0.4);
}

function PlayTurning()
{
	if (Physics == PHYS_Walking)
		LoopAnim('Walking');
	else
		LoopAnim('Flying');
}

function PlayDying(name DamageType, vector HitLocation)
{
	PlaySound(Die, SLOT_Talk, 2.5 * TransientSoundVolume);
	PlayAnim('Dead', 0.7, 0.1);
}

function PlayTakeHit(float tweentime, vector HitLoc, int damage)
{
	TweenAnim('TakeHit', tweentime);
}

function TweenToFalling()
{
	TweenAnim('Flying', 0.2);
}

function PlayInAir()
{
	LoopAnim('Flying');
}

function PlayLanded(float impactVel)
{
	PlayAnim('Land');
}

function PlayVictoryDance()
{
	if ( FRand() < 0.4 )
		TweenToWaiting(0.25);
	else
		PlayAnim('Flying',1.0, 0.05);
}
	
function PlayMeleeAttack()
{
	PlayAnim('Shoot1');
	if ( MeleeDamageTarget(15, (15 * 1000.0 * Normal(Target.Location - Location))) )
		PlaySound(Threaten, SLOT_Talk); //FIXME - stingdamage instead of projectile
	GotoState('TacticalMove', 'BackOff');
}

function PlayRangedAttack()
{
	local vector projStart;
	local vector adjust;

	PlayAnim('Shoot1');
	/*
	adjust = vect(0,0,0);
	adjust.Z = Target.CollisionHeight + 20;
	Acceleration = AccelRate * Normal(Target.Location - Location + adjust);
	projStart = Location - 0.5 * CollisionHeight * vect(0,0,1);
	spawn(RangedProjectile ,self,'',projStart,AdjustAim(ProjectileSpeed, projStart, 400, false, false));
	*/
}

function PlayMovingAttack()
{
	PlayRangedAttack();
}

state TacticalMove
{
ignores SeePlayer, HearNoise;

BackOff:
	Acceleration = AccelRate * Normal(Location - Enemy.Location);
	Acceleration.Z *= 0.5;
	Destination = Location;
	Sleep(0.5);
	SetTimer(TimeBetweenAttacks, false);
	Goto('TacticalTick');
}

state Roaming
{
	function PickDestination()
	{
		GotoState('Wandering');
	}

Begin:
	GotoState('Wandering');
}

defaultproperties
{
     CarcassType=FlyCarcass
     Aggressiveness=+00000.700000
     ReFireRate=+00000.700000
     Health=30
     bCanStrafe=True
     Visibility=100
     SightRadius=+01000.000000
     PeripheralVision=-00000.500000
     MeleeRange=+00040.000000
     WalkingSpeed=+00001.000000
     GroundSpeed=+00100.000000
     AirSpeed=+00240.000000
     AccelRate=+00600.000000
     JumpZ=+00010.000000
     Buoyancy=+00110.000000
     HitSound1=injur1rf
     HitSound2=injur2rf
     Land=None
     Die=death1rf
     CombatStyle=+00000.400000
     DrawType=DT_Mesh
     Mesh=FlyM
     bMeshCurvy=False
     AmbientSound=buzz3rf
     CollisionRadius=+00020.000000
     CollisionHeight=+00012.000000
     RotationRate=(Pitch=6000,Yaw=65000,Roll=8192)
}

//=============================================================================
// DevilFish.
//=============================================================================
class DevilFish expands ScriptedPawn;

#exec MESH IMPORT MESH=fish ANIVFILE=MODELS\fish_a.3D DATAFILE=MODELS\fish_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=fish X=0 Y=30 Z=0 YAW=64 PITCH=0 ROLL=-64 

#exec MESH SEQUENCE MESH=fish SEQ=All          STARTFRAME=0   NUMFRAMES=102
#exec MESH SEQUENCE MESH=fish SEQ=bite1        STARTFRAME=0   NUMFRAMES=10	RATE=45	Group=Attack
#exec MESH SEQUENCE MESH=fish SEQ=bite2        STARTFRAME=10  NUMFRAMES=10	RATE=45	Group=Attack
#exec MESH SEQUENCE MESH=fish SEQ=bite3        STARTFRAME=20  NUMFRAMES=10	RATE=45	Group=Attack
#exec MESH SEQUENCE MESH=fish SEQ=breathing    STARTFRAME=30  NUMFRAMES=6	RATE=6
#exec MESH SEQUENCE MESH=fish SEQ=dead1        STARTFRAME=36  NUMFRAMES=18	RATE=15
#exec MESH SEQUENCE MESH=fish SEQ=flopping     STARTFRAME=54  NUMFRAMES=20
#exec MESH SEQUENCE MESH=fish SEQ=grab1        STARTFRAME=74  NUMFRAMES=5			Group=Attack
#exec MESH SEQUENCE MESH=fish SEQ=ripper       STARTFRAME=79  NUMFRAMES=13  RATE=25	Group=Attack
#exec MESH SEQUENCE MESH=fish SEQ=Swimming     STARTFRAME=92  NUMFRAMES=10	RATE=20
#exec MESH SEQUENCE MESH=fish SEQ=fighter	   STARTFRAME=0   NUMFRAMES=10
#exec MESH SEQUENCE MESH=fish SEQ=takehit      STARTFRAME=36  NUMFRAMES=1

#exec TEXTURE IMPORT NAME=Jfish1 FILE=MODELS\shark.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=fish X=0.085 Y=0.085 Z=0.17
#exec MESHMAP SETTEXTURE MESHMAP=fish NUM=1 TEXTURE=Jfish1

#exec AUDIO IMPORT FILE="Sounds\Devilfish\ambfs.WAV" NAME="ambfs" GROUP="Razorfish"
#exec AUDIO IMPORT FILE="Sounds\Devilfish\chomp1fs.WAV" NAME="chomp1fs" GROUP="Razorfish"
#exec AUDIO IMPORT FILE="Sounds\Devilfish\death1fs.WAV" NAME="death1fs" GROUP="Razorfish"
#exec AUDIO IMPORT FILE="Sounds\Devilfish\flop1fs.WAV" NAME="flop1fs" GROUP="Razorfish"
#exec AUDIO IMPORT FILE="Sounds\Devilfish\tear1fs.WAV" NAME="tear1fs" GROUP="Razorfish"
#exec AUDIO IMPORT FILE="Sounds\Devilfish\miss1fs.WAV" NAME="miss1fs" GROUP="Razorfish"
#exec AUDIO IMPORT FILE="Sounds\Devilfish\breath1fs.WAV" NAME="breath1fs" GROUP="Razorfish"

//-----------------------------------------------------------------------------
// RazorFish variables.

// Attack damage.
var() byte
	BiteDamage,		// Basic damage done by bite.
	RipDamage;
var bool bAttackBump;
var(Sounds) sound bite;
var(Sounds) sound rip;
var float	AirTime;

//-----------------------------------------------------------------------------
// RazorFish functions.

function WhatToDoNext(name LikelyState, name LikelyLabel)
{
	bStasis = true;
	Super.WhatToDoNext(LikelyState, LikelyLabel);
}

function ZoneChange(ZoneInfo newZone)
{
	local vector start, checkpoint, HitNormal, HitLocation;
	local actor HitActor;
	
	if ( newZone.bWaterZone )
	{
		AirTime = 0;
		setPhysics(PHYS_Swimming);
	}
	else
	{
		SetPhysics(PHYS_Falling);
		MoveTimer = -1.0;
	}
}

function Landed(vector HitNormal)
{
	GotoState('Flopping');
	Landed(HitNormal);
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

function eAttitude AttitudeToCreature(Pawn Other)
{
	if ( Other.IsA('Devilfish') )
		return ATTITUDE_Friendly;
	else if ( Other.IsA('ScriptedPawn') )
		return ATTITUDE_Hate;
	else
		return ATTITUDE_Ignore;
}

function PlayWaiting()
{
	LoopAnim('Swimming', 0.1 + 0.3 * FRand());
}

function PlayPatrolStop()
{
	LoopAnim('Swimming', 0.1 + 0.3 * FRand());
}

function PlayWaitingAmbush()
{
	LoopAnim('Swimming', 0.1 + 0.3 * FRand());
}

function TweenToFighter(float tweentime)
{
	TweenAnim('Fighter', tweentime);
}

function TweenToRunning(float tweentime)
{
	if ( (AnimSequence != 'Swimming') || !bAnimLoop )
		TweenAnim('Swimming', tweentime);
}

function TweenToWalking(float tweentime)
{
	if ( (AnimSequence != 'Swimming') || !bAnimLoop )
		TweenAnim('Swimming', tweentime);
}

function TweenToWaiting(float tweentime)
{
	PlayAnim('Swimming', 0.2 + 0.8 * FRand());
}

function TweenToPatrolStop(float tweentime)
{
	TweenAnim('Swimming', tweentime);
}

function PlayRunning()
{
	LoopAnim('Swimming', -0.8/WaterSpeed,, 0.4);
}

function PlayWalking()
{
	LoopAnim('Swimming', -0.8/WaterSpeed,, 0.4);
}

function PlayThreatening()
{
	if ( FRand() < 0.5 )
		PlayAnim('Swimming', 0.4);
	else
		PlayAnim('Fighter', 0.4);
}

function PlayTurning()
{
	LoopAnim('Swimming', 0.8);
}

function PlayDying(name DamageType, vector HitLocation)
{
	if ( Region.Zone.bWaterZone )
	{
		PlaySound(Die, SLOT_Talk, 4 * TransientSoundVolume);
		PlayAnim('Dead1', 0.7, 0.1);
	}
	else
		TweenAnim('Breathing', 0.35);
}

function PlayTakeHit(float tweentime, vector HitLoc, int damage)
{
	TweenAnim('TakeHit', tweentime);
}

function TweenToFalling()
{
	TweenAnim('Flopping', 0.2);
}

function PlayInAir()
{
	LoopAnim('Flopping', 0.7);
}

function PlayLanded(float impactVel)
{
	TweenAnim('breathing', 0.2);
}


function PlayVictoryDance()
{
	PlayAnim('ripper', 0.6, 0.1);
}
	
function PlayMeleeAttack()
{
	local vector adjust;
	local float decision;
	adjust = vect(0,0,0.5) * FRand() * Target.CollisionHeight;
	Acceleration = AccelRate * Normal(Target.Location - Location + adjust);
	bAttackBump = false;
	if (AnimSequence == 'Grab1')
	{
		PlayAnim('ripper', 0.5 + 0.5 * FRand());
		PlaySound(rip,SLOT_Interact,,,500);
		MeleeDamageTarget(RipDamage, vect(0,0,0));
		Disable('Bump');
		return;
	}
	decision = FRand();
	PlaySound(bite,SLOT_Interact,,,500);
	if (decision < 0.3)
	{
		Disable('Bump');
		PlayAnim('Grab1', 0.3);
		return;
	}
	
	Enable('Bump');
	//log("Start Melee Attack");
	if (decision < 0.55)
	{
		PlayAnim('Bite1', 0.3);
	}
	else if (decision < 0.8)
	{
 		PlayAnim('Bite2', 0.3); 
 	}
 	else 
 	{
 		PlayAnim('Bite3', 0.3);
 	}
}

state Waiting
{
	function Landed(vector HitNormal)
	{
		GotoState('Flopping');
		Landed(HitNormal);
	}
}

state TakeHit 
{
ignores seeplayer, hearnoise, bump, hitwall;

	function Landed(vector HitNormal)
	{
		GotoState('Flopping');
		Landed(HitNormal);
	}
}

state FallingState 
{
ignores Bump, Hitwall, HearNoise, WarnTarget;

	function Landed(vector HitNormal)
	{
		GotoState('Flopping');
		Landed(HitNormal);
	}
}

state Ambushing
{
	function Landed(vector HitNormal)
	{
		GotoState('Flopping');
		Landed(HitNormal);
	}
}

state MeleeAttack
{
ignores SeePlayer, HearNoise;

	singular function Bump(actor Other)
	{
		Disable('Bump');
		if ( (AnimSequence == 'Bite1') || (AnimSequence == 'Bite2') || (AnimSequence == 'Bite3') )
			MeleeDamageTarget(BiteDamage, (BiteDamage * 1000.0 * Normal(Target.Location - Location)));
		else 
			return;
		bAttackBump = true;
		Velocity *= -0.5;
		Acceleration *= -1;
		if (Acceleration.Z < 0)
			Acceleration.Z *= -1;
	}

	function KeepAttacking()
	{
		if ( (Target == None) ||
			((Pawn(Target) != None) && (Pawn(Target).Health == 0)) )
			GotoState('Attacking');
		else if ( bAttackBump && (FRand() < 0.5) )
		{
			SetTimer(TimeBetweenAttacks, false);
			GotoState('TacticalMove', 'NoCharge');
		}
	}
}		


State Flopping
{
ignores seeplayer, hearnoise, enemynotvisible, hitwall; 	

	function Timer()
	{
		AirTime += 1;
		if ( AirTime > 25 + 15 * FRand() )
		{
			Health = -1;
			Died(None, 'suffocated', Location);
			return;
		}	
		SetPhysics(PHYS_Falling);
		Velocity = 200 * VRand();
		Velocity.Z = 170 + 200 * FRand();
		DesiredRotation.Pitch = Rand(8192) - 4096;
		DesiredRotation.Yaw = Rand(65535);
		TweenAnim('Flopping', 0.1);
	}
	
	function ZoneChange( ZoneInfo NewZone )
	{
		local rotator newRotation;
		if (NewZone.bWaterZone)
		{
			newRotation = Rotation;
			newRotation.Roll = 0;
			SetRotation(newRotation);
			SetPhysics(PHYS_Swimming);
			AirTime = 0;
			GotoState('Attacking');
		}
		else
			SetPhysics(PHYS_Falling);
	}
	
	function Landed(vector HitNormal)
	{
		local rotator newRotation;
		SetPhysics(PHYS_None);
		SetTimer(0.3 + 0.3 * AirTime * FRand(), false);
		newRotation = Rotation;
		newRotation.Pitch = 0;
		newRotation.Roll = Rand(16384) - 8192;
		DesiredRotation.Pitch = 0;
		SetRotation(newRotation);
		PlaySound(land,SLOT_Interact,,,400);
		TweenAnim('Breathing', 0.3);
	}
	
	function AnimEnd()
	{
		if (Physics == PHYS_None)
		{
			if (AnimSequence == 'Breathing')
			{
				PlaySound(sound'breath1fs',SLOT_Interact,,,300);
				PlayAnim('Breathing');
			}
			else 
				TweenAnim('Breathing', 0.2);
		}
		else
			PlayAnim('Flopping', 0.7);
	}
}


state TacticalMove
{
	function PickDestination(bool bNoCharge)
	{
		local vector pick, pickdir, enemydir,Y, minDest;
		local float Aggression, enemydist, minDist, strafeSize;
		local bool success;
	
		success = false;
		enemyDist = VSize(Location - Enemy.Location);
		Aggression = 2 * (CombatStyle + FRand()) - 1.0;

		if (enemyDist < CollisionRadius + Enemy.CollisionRadius + MeleeRange)
			Aggression -= 1;	
		else if (enemyDist > FMax(VSize(OldLocation - Enemy.OldLocation), 240))
			Aggression += 0.4 * FRand();
		
		enemydir = (Enemy.Location - Location)/enemyDist;
		if ( !enemy.Region.Zone.bWaterZone )
			enemydir.Z *= 0.5;
		minDist = FMin(160.0, 3*CollisionRadius);
			
		Y = (enemydir Cross vect(0,0,1));
		
		strafeSize = FMin(1.0, (2 * Abs(Aggression) * FRand() - 0.2));
		if (Aggression < 0)
			strafeSize *= -1;
		enemydir = enemydir * strafeSize;
		enemydir.Z = FMax(0,enemydir.Z);
		strafeSize = FMax(0.0, 1 - Abs(strafeSize));
		pickdir = strafeSize * Y;
		pick = Location + (pickdir + enemydir) * (minDist + 500 * FRand());
		minDest = Location + minDist * (pickdir + enemydir); 
		if (pointReachable(minDest))
		{
			success = true;
			Destination = pick;	
		}
						
		if (!success)
		{
			pick = Location + (enemydir - pickdir) * (minDist + 500 * FRand());
			pick.Z = Location.Z;
			minDest = Location + minDist * (enemydir - pickdir); 
			if (pointReachable(minDest))
			{
				success = true;
				Destination = pick;	
			}
		}
		
		if (!success)
		{
			pick = Location - enemydir * (minDist + 500 * FRand());
			pick.Z = Location.Z + 200 * FRand() - 100;
		}
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
      BiteDamage=15
      RipDamage=25
	  Health=70
      Bite=cmphitfs
      rip=tear1fs
      Land=flop1fs
      CarcassType=DevilfishCarcass
      UnderWaterTime=-00001.000000
      SightRadius=+01250.000000
      TimeBetweenAttacks=+00000.500000
      MeleeRange=+00040.000000
      Aggressiveness=+00001.000000
      Visibility=120
      Intelligence=BRAINS_NONE
      WaterSpeed=+00250.000000
      MaxStepHeight=+00025.000000
      HitSound1=chomp1fs
      HitSound2=miss1fs
      Die=death1fs
      CombatStyle=+00001.000000
      DrawType=DT_Mesh
      Mesh=fish
      AmbientSound=ambfs
      CollisionHeight=+00020.000000
	  CollisionRadius=+00035.000000
      Mass=+00060.000000
      Buoyancy=+00060.000000
      RotationRate=(Pitch=8192,Roll=8192)
}

//=============================================================================
// Pupae.
//=============================================================================
class Pupae expands ScriptedPawn;

#exec MESH IMPORT MESH=Pupae1 ANIVFILE=MODELS\pupae_a.3D DATAFILE=MODELS\pupae_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Pupae1 X=0 Y=0 Z=-90 YAW=64 PITCH=0 ROLL=-64

#exec MESH SEQUENCE MESH=pupae1 SEQ=All		 STARTFRAME=0	 NUMFRAMES=171
#exec MESH SEQUENCE MESH=pupae1 SEQ=Bite     STARTFRAME=0    NUMFRAMES=15  RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=pupae1 SEQ=Crawl    STARTFRAME=15   NUMFRAMES=20  RATE=20
#exec MESH SEQUENCE MESH=pupae1 SEQ=Dead     STARTFRAME=35   NUMFRAMES=18  RATE=15
#exec MESH SEQUENCE MESH=pupae1 SEQ=TakeHit  STARTFRAME=36   NUMFRAMES=1
#exec MESH SEQUENCE MESH=pupae1 SEQ=Fighter  STARTFRAME=53   NUMFRAMES=6   RATE=6
#exec MESH SEQUENCE MESH=pupae1 SEQ=Lunge    STARTFRAME=59   NUMFRAMES=15  RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=pupae1 SEQ=Munch    STARTFRAME=74   NUMFRAMES=8   RATE=15
#exec MESH SEQUENCE MESH=pupae1 SEQ=Pick     STARTFRAME=82   NUMFRAMES=10  RATE=15
#exec MESH SEQUENCE MESH=pupae1 SEQ=Stab     STARTFRAME=92   NUMFRAMES=10  RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=pupae1 SEQ=Tear     STARTFRAME=102  NUMFRAMES=28  RATE=15
#exec MESH SEQUENCE MESH=pupae1 SEQ=Dead2    STARTFRAME=130  NUMFRAMES=18  RATE=15
#exec MESH SEQUENCE MESH=pupae1 SEQ=Dead3    STARTFRAME=148  NUMFRAMES=23  RATE=15

#exec TEXTURE IMPORT NAME=JPupae1 FILE=MODELS\pupae.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=pupae1 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=pupae1 NUM=1 TEXTURE=Jpupae1

#exec MESH NOTIFY MESH=Pupae1 SEQ=Dead TIME=0.52 FUNCTION=LandThump

#exec AUDIO IMPORT FILE="Sounds\Pupae\scuttle1.WAV" NAME="scuttle1pp" GROUP="Pupae"
#exec AUDIO IMPORT FILE="Sounds\Pupae\injur1.WAV" NAME="injur1pp" GROUP="Pupae"
#exec AUDIO IMPORT FILE="Sounds\Pupae\injur2.WAV" NAME="injur2pp" GROUP="Pupae"
#exec AUDIO IMPORT FILE="Sounds\Pupae\roam1.WAV" NAME="roam1pp" GROUP="Pupae"
#exec AUDIO IMPORT FILE="Sounds\Pupae\hiss1.WAV" NAME="hiss1pp" GROUP="Pupae"
#exec AUDIO IMPORT FILE="Sounds\Pupae\hiss2.WAV" NAME="hiss2pp" GROUP="Pupae"
#exec AUDIO IMPORT FILE="Sounds\Pupae\hiss3.WAV" NAME="hiss3pp" GROUP="Pupae"
#exec AUDIO IMPORT FILE="Sounds\Pupae\bite1pp.WAV" NAME="bite1pp" GROUP="Pupae"
#exec AUDIO IMPORT FILE="Sounds\Pupae\tear1b.WAV" NAME="tear1pp" GROUP="Pupae"
#exec AUDIO IMPORT FILE="Sounds\Pupae\munch1pp.WAV" NAME="munch1p" GROUP="Pupae"
#exec AUDIO IMPORT FILE="Sounds\Pupae\death1b.WAV" NAME="death1pp" GROUP="Pupae"

//-----------------------------------------------------------------------------
// Pupae variables.

// Attack damage.
var() byte BiteDamage;		// Basic damage done by bite.
var() byte LungeDamage;		// Basic damage done by bite.
var(Sounds) sound bite;
var(Sounds) sound stab;
var(Sounds) sound lunge;
var(Sounds) sound chew;
var(Sounds) sound tear;
 
//-----------------------------------------------------------------------------
// Pupae functions.

function PostBeginPlay()
{
	Super.PostBeginPlay();
	MaxDesiredSpeed = 0.7 + 0.1 * skill;
}

function JumpOffPawn()
{
	Super.JumpOffPawn();
	PlayAnim('crawl', 1.0, 0.2);
}

function SetMovementPhysics()
{
	SetPhysics(PHYS_Falling); 
}

function eAttitude AttitudeToCreature(Pawn Other)
{
	if ( Other.IsA('Pupae') )
		return ATTITUDE_Friendly;
	else if ( Other.IsA('Skaarj') )
	{
		if ( Other.IsA('SkaarjBerserker') )
			return ATTITUDE_Ignore;
		else
			return ATTITUDE_Friendly;
	}
	else if ( Other.IsA('WarLord') || Other.IsA('Queen') )
		return ATTITUDE_Friendly;
	else if ( Other.IsA('ScriptedPawn') )
		return ATTITUDE_Hate;
}

function PlayWaiting()
	{
	local float decision;
	local float animspeed;
	animspeed = 0.4 + 0.6 * FRand(); 
	decision = FRand();
	if ( !bool(NextAnim) || (decision < 0.4) ) //pick first waiting animation
	{
		if ( !bQuiet )
			PlaySound(Chew, SLOT_Talk, 0.7,,800);
		NextAnim = 'Munch';
	}
	else if (decision < 0.55)
		NextAnim = 'Pick';
	else if (decision < 0.7)
	{
		if ( !bQuiet )
			PlaySound(Stab, SLOT_Talk, 0.7,,800);
		NextAnim = 'Stab';
	}
	else if (decision < 0.7)
		NextAnim = 'Bite';
	else 
		NextAnim = 'Tear';
		
	LoopAnim(NextAnim, animspeed);
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
	if ( FRand() < 0.3 )
		PlayWaiting();
	else
		PlayAnim('Fighter');
}

function TweenToFighter(float tweentime)
{
	TweenAnim('Fighter', tweentime);
}

function TweenToRunning(float tweentime)
{
	if (AnimSequence != 'Crawl' || !bAnimLoop)
		TweenAnim('Crawl', tweentime);
}

function TweenToWalking(float tweentime)
{
	TweenAnim('Crawl', tweentime);
}

function TweenToWaiting(float tweentime)
{
	TweenAnim('Munch', tweentime);
}

function TweenToPatrolStop(float tweentime)
{
	TweenAnim('Munch', tweentime);
}

function PlayRunning()
{
	PlaySound(sound'scuttle1pp', SLOT_Interact);
	LoopAnim('Crawl', -4.0/GroundSpeed,,0.4);
}

function PlayWalking()
{
	PlaySound(sound'scuttle1pp', SLOT_Interact);
	LoopAnim('Crawl', -4.0/GroundSpeed,,0.4);
}

function PlayThreatening()
{
	PlayWaiting();
}

function PlayTurning()
{
	TweenAnim('Crawl', 0.3);
}

function PlayDying(name DamageType, vector HitLocation)
{
	local carcass carc;

	PlaySound(Die, SLOT_Talk, 3.5 * TransientSoundVolume);
	if ( FRand() < 0.35 )
		PlayAnim('Dead', 0.7, 0.1);
	else if ( FRand() < 0.5 )
	{
		carc = Spawn(class 'CreatureChunks',,, Location + CollisionHeight * vect(0,0,0.8), Rotation + rot(3000,0,16384) );
		if (carc != None)
		{
			carc.Mesh = mesh'PupaeHead';
			carc.Initfor(self);
			carc.Velocity = Velocity + VSize(Velocity) * VRand();
			carc.Velocity.Z = FMax(carc.Velocity.Z, Velocity.Z);
		}
		PlayAnim('Dead2', 0.7, 0.1);
	}
	else
	{
		carc = Spawn(class 'CreatureChunks',,, Location + CollisionHeight * vect(0,0,0.8), Rotation + rot(3000,0,16384) );
		if (carc != None)
		{
			carc.Mesh = mesh'PupaeBody';
			carc.Initfor(self);
			carc.Velocity = Velocity + VSize(Velocity) * VRand();
			carc.Velocity.Z = FMax(carc.Velocity.Z, Velocity.Z);
		}
		PlayAnim('Dead3', 0.7, 0.1);
	}
}

function PlayTakeHit(float tweentime, vector HitLoc, int damage)
{
	PlayAnim('TakeHit');
}

function PlayVictoryDance()
{
	PlayAnim('Stab', 1.0, 0.1);
}

function PlayMeleeAttack()
{
	local float dist, decision;

	decision = FRand();
	dist = VSize(Target.Location - Location);
	if (dist > CollisionRadius + Target.CollisionRadius + 45)
		decision = 0.0;

	if (Physics == PHYS_Falling)
		decision = 1.0;
	if (Target == None)
		decision = 1.0;

	if (decision < 0.15)
	{
		PlaySound(Lunge, SLOT_Interact);
		Enable('Bump');
		PlayAnim('Lunge');
		Velocity = 450 * Normal(Target.Location + Target.CollisionHeight * vect(0,0,0.75) - Location);
		if (dist > CollisionRadius + Target.CollisionRadius + 35)
			Velocity.Z += 0.7 * dist;
		SetPhysics(PHYS_Falling);
	}
  	else
  	{
  		PlaySound(Stab, SLOT_Interact);
  		PlayAnim('Stab');
		MeleeRange = 50;
		MeleeDamageTarget(BiteDamage, vect(0,0,0));
		MeleeRange = Default.MeleeRange;
	}  		
}

state MeleeAttack
{
ignores SeePlayer, HearNoise;

	singular function Bump(actor Other)
	{
		Disable('Bump');
		if ( (Other == Target) && (AnimSequence == 'Lunge') )
			if (MeleeDamageTarget(LungeDamage, vect(0,0,0)))
			{
				if (FRand() < 0.5)
					PlaySound(Tear, SLOT_Interact);
				else
					PlaySound(Bite, SLOT_Interact);
			}
	}
}		

auto state StartUp
{
	function SetMovementPhysics()
	{
		SetPhysics(PHYS_None); // don't fall at start
	}
}


state Waiting
{
TurnFromWall:
	if ( NearWall(70) )
	{
		PlayTurning();
		TurnTo(Focus);
	}
Begin:
	TweenToWaiting(0.4);
	bReadyToAttack = false;
	if (Physics != PHYS_Falling) 
		SetPhysics(PHYS_None);
KeepWaiting:
	NextAnim = '';
}

defaultproperties
{
     BiteDamage=10
     LungeDamage=20
     Bite=bite1pp
     Stab=hiss1pp
     Lunge=hiss2pp
     Chew=munch1p
     Tear=tear1pp
     CarcassType=PupaeCarcass
     Aggressiveness=+00010.000000
     Acquire=hiss2pp
     Fear=hiss1pp
     Roam=roam1pp
     Threaten=hiss3pp
     Health=65
     Visibility=100
	 bCanStrafe=True
     SightRadius=+08500.000000
     PeripheralVision=-00000.400000
     MeleeRange=+00280.000000
     Intelligence=BRAINS_NONE
     GroundSpeed=+00260.000000
     WaterSpeed=+00100.000000
     JumpZ=+00340.000000
     MaxStepHeight=+00025.000000
     HitSound1=injur1pp
     HitSound2=injur2pp
     Die=death1pp
     CombatStyle=+00001.000000
     DrawType=DT_Mesh
     Mesh=Pupae1
     bMeshCurvy=False
     CollisionRadius=+00028.000000
     CollisionHeight=+00009.000000
     Mass=+00080.000000
     RotationRate=(Pitch=3072,Yaw=65000,Roll=0)
}

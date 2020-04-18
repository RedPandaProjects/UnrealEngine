//=============================================================================
// Nali.
//=============================================================================
class Nali expands ScriptedPawn;

#exec MESH IMPORT MESH=Nali1 ANIVFILE=MODELS\nali_a.3D DATAFILE=MODELS\nali_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=Nali1 X=00 Y=-130 Z=30 YAW=64 ROLL=-64

#exec MESH SEQUENCE MESH=nali1 SEQ=All      STARTFRAME=0   NUMFRAMES=397
#exec MESH SEQUENCE MESH=nali1 SEQ=Backup   STARTFRAME=0   NUMFRAMES=10  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=Bowing   STARTFRAME=10  NUMFRAMES=20  RATE=15 Group=Ducking
#exec MESH SEQUENCE MESH=nali1 SEQ=Breath   STARTFRAME=30  NUMFRAMES=8   RATE=6
#exec MESH SEQUENCE MESH=nali1 SEQ=Cough    STARTFRAME=38  NUMFRAMES=25  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=Landed   STARTFRAME=68  NUMFRAMES=1
#exec MESH SEQUENCE MESH=nali1 SEQ=Cringe   STARTFRAME=63  NUMFRAMES=15  RATE=15 Group=Ducking
#exec MESH SEQUENCE MESH=nali1 SEQ=Dead     STARTFRAME=78  NUMFRAMES=38  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=Dead2    STARTFRAME=116 NUMFRAMES=16  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=Dead3    STARTFRAME=132 NUMFRAMES=13  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=Dead4    STARTFRAME=145 NUMFRAMES=21  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=Follow   STARTFRAME=166 NUMFRAMES=23  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=GetDown  STARTFRAME=189 NUMFRAMES=5   RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=GetUp    STARTFRAME=194 NUMFRAMES=8   RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=levitate STARTFRAME=202 NUMFRAMES=6  RATE=6
#exec MESH SEQUENCE MESH=nali1 SEQ=pray     STARTFRAME=208 NUMFRAMES=8   RATE=6
#exec MESH SEQUENCE MESH=nali1 SEQ=spell    STARTFRAME=216 NUMFRAMES=28  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=Sweat    STARTFRAME=244 NUMFRAMES=18  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=walk     STARTFRAME=262 NUMFRAMES=20  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=GutHit   STARTFRAME=282 NUMFRAMES=1
#exec MESH SEQUENCE MESH=nali1 SEQ=AimDown  STARTFRAME=283 NUMFRAMES=1
#exec MESH SEQUENCE MESH=nali1 SEQ=AimUp    STARTFRAME=284 NUMFRAMES=1
#exec MESH SEQUENCE MESH=nali1 SEQ=Bow2     STARTFRAME=285 NUMFRAMES=28  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=HeadHit  STARTFRAME=313 NUMFRAMES=1
#exec MESH SEQUENCE MESH=nali1 SEQ=LeftHit  STARTFRAME=314 NUMFRAMES=1
#exec MESH SEQUENCE MESH=nali1 SEQ=RightHit STARTFRAME=315 NUMFRAMES=1
#exec MESH SEQUENCE MESH=nali1 SEQ=Run      STARTFRAME=316 NUMFRAMES=10  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=RunFire  STARTFRAME=326 NUMFRAMES=10  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=StilFire STARTFRAME=336 NUMFRAMES=1
#exec MESH SEQUENCE MESH=nali1 SEQ=WalkFire STARTFRAME=337 NUMFRAMES=20  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=WalkTool STARTFRAME=357 NUMFRAMES=20  RATE=15
#exec MESH SEQUENCE MESH=nali1 SEQ=Drowning STARTFRAME=377 NUMFRAMES=20  RATE=15

#exec MESH SEQUENCE MESH=nali1 SEQ=fighter STARTFRAME=0   NUMFRAMES=1

#exec TEXTURE IMPORT NAME=JNali1 FILE=MODELS\nali.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=nali1 X=0.069 Y=0.069 Z=0.138
#exec MESHMAP SETTEXTURE MESHMAP=nali1 NUM=0 TEXTURE=Jnali1

#exec MESH NOTIFY MESH=Nali1 SEQ=Dead TIME=0.46 FUNCTION=LandThump
#exec MESH NOTIFY MESH=Nali1 SEQ=Dead2 TIME=0.64 FUNCTION=LandThump
#exec MESH NOTIFY MESH=Nali1 SEQ=Dead3 TIME=0.84 FUNCTION=LandThump
#exec MESH NOTIFY MESH=Nali1 SEQ=Dead4 TIME=0.51 FUNCTION=LandThump
#exec MESH NOTIFY MESH=Nali1 SEQ=Backup TIME=0.25 FUNCTION=Step
#exec MESH NOTIFY MESH=Nali1 SEQ=Backup TIME=0.75 FUNCTION=Step
#exec MESH NOTIFY MESH=Nali1 SEQ=Walk   TIME=0.25 FUNCTION=Step
#exec MESH NOTIFY MESH=Nali1 SEQ=Walk   TIME=0.75 FUNCTION=Step

#exec AUDIO IMPORT FILE="Sounds\Nali\syl1.WAV" NAME="syl1n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\syl2.WAV" NAME="syl2n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\syl3.WAV" NAME="syl3n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\syl4.WAV" NAME="syl4n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\syl5.WAV" NAME="syl5n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\syl6.WAV" NAME="syl6n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\follow1c.WAV" NAME="follow1n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\injur1a.WAV" NAME="injur1n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\injur2a.WAV" NAME="injur2n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\contct1a.WAV" NAME="contct1n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\contct3a.WAV" NAME="contct3n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\fear1a.WAV" NAME="fear1n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\breath2na.WAV" NAME="breath1n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\death1na.WAV" NAME="death1n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\death2a.WAV" NAME="death2n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\bowing1a.WAV" NAME="bowing1n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\cringe2a.WAV" NAME="cringe2n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\backup2a.WAV" NAME="backup2n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\cough1na.WAV" NAME="cough1n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\sweat1na.WAV" NAME="sweat1n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Nali\levitF1.WAV" NAME="pray1n" GROUP="Nali"
#exec AUDIO IMPORT FILE="Sounds\Cow\walknc.WAV" NAME="walkC" GROUP="Cow"
#exec AUDIO IMPORT FILE="Sounds\Generic\teleprt27.WAV" NAME="Teleport1" GROUP="Generic"

//====================================================================
// Nali Variables

var() bool bNeverBow;
var bool bCringing;
var bool bGesture;
var bool bFading;
var bool bHasWandered;
var(Sounds) sound syllable1;
var(Sounds) sound syllable2;
var(Sounds) sound syllable3;
var(Sounds) sound syllable4;
var(Sounds) sound syllable5;
var(Sounds) sound syllable6;
var(Sounds) sound urgefollow;
var(Sounds) sound cringe;
var(Sounds) sound cough;
var(Sounds) sound sweat;
var(Sounds) sound bowing;
var(Sounds) sound backup;
var(Sounds) sound pray;
var(Sounds) sound breath;
var() Weapon Tool;

function PostBeginPlay()
{
	Super.PostBeginPlay();
	bCanSpeak = true;
	if ( Orders == 'Ambushing' )
		AnimSequence = 'Levitate';
}

function SpeakPrayer()
{
	PlaySound(Pray);
}


function PlayFearSound()
{
	if ( (Threaten != None) && (FRand() < 0.4) )
	{
		PlaySound(Threaten, SLOT_Talk,, true); 
		return;
	}
	if (Fear != None)
		PlaySound(Fear, SLOT_Talk,, true); 
}

function bool AdjustHitLocation(out vector HitLocation, vector TraceDir)
{
	local float adjZ, maxZ;

	TraceDir = Normal(TraceDir);
	HitLocation = HitLocation + 0.5 * CollisionRadius * TraceDir;

	if ( (GetAnimGroup(AnimSequence) == 'Ducking') && (AnimFrame > -0.03) )
	{
		if ( AnimSequence == 'Bowing' )
			maxZ = Location.Z - 0.2 * CollisionHeight;
		else
			maxZ = Location.Z + 0.25 * CollisionHeight;
		if ( HitLocation.Z > maxZ )
		{
			if ( TraceDir.Z >= 0 )
				return false;
			adjZ = (maxZ - HitLocation.Z)/TraceDir.Z;
			HitLocation.Z = maxZ;
			HitLocation.X = HitLocation.X + TraceDir.X * adjZ;
			HitLocation.Y = HitLocation.Y + TraceDir.Y * adjZ;
			if ( VSize(HitLocation - Location) > CollisionRadius )	
				return false;
		}
	}
	return true;
}

function Killed(pawn Killer, pawn Other, name damageType)
{
	if ( (Nali(Other) != None) && Killer.bIsPlayer )
		AttitudeToPlayer = ATTITUDE_Fear;
	Super.Killed(Killer, Other, damageType);
}

/* AttitudeWithFear()
may fear other, unless near home
*/

function eAttitude AttitudeWithFear()
{
	return ATTITUDE_Fear;
}

function damageAttitudeTo(pawn Other)
{
	local eAttitude OldAttitude;
	
	if ( (Other == Self) || (Other == None) || (FlockPawn(Other) != None) )
		return;
	if( Other.bIsPlayer ) //change attitude to player
		AttitudeToPlayer = ATTITUDE_Fear;
	else if ( ScriptedPawn(Other) == None )
		Hated = Other;
	SetEnemy(Other);				
}

function eAttitude AttitudeToCreature(Pawn Other)
{
	if ( Other.IsA('Nali') )
		return ATTITUDE_Friendly;
	else if ( Other.IsA('ScriptedPawn') && !Other.IsA('Cow') )
		return ATTITUDE_Fear;
	else
		return ATTITUDE_Ignore;
}

function Step()
{
	PlaySound(sound'WalkC', SLOT_Interact,0.5,,500);
}	

function PlayWaiting()
{
	local float decision;
	local float animspeed;

	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	
	animspeed = 0.4 + 0.6 * FRand(); 
	decision = FRand();
	if ( AnimSequence == 'Breath' )
	{
		if (!bQuiet && (decision < 0.12) )
		{
			PlaySound(Cough,Slot_Talk,1.0,,800);
			LoopAnim('Cough', 0.85);
			return;
		}
		else if (decision < 0.24)
		{
			PlaySound(Sweat,Slot_Talk,0.3,,300);
			LoopAnim('Sweat', animspeed);
			return;
		}
		else if (!bQuiet && (decision < 0.34) )
		{
			PlayAnim('Pray', animspeed, 0.3);
			return;
		}
	}
	else if ( AnimSequence == 'Pray' )
	{
		if (decision < 0.3)
			PlayAnim('Breath', animspeed, 0.3);
		else
		{
			SpeakPrayer();
			PlayAnim('Pray', animspeed);
		}
		return;
	}
 	
	PlaySound(Breath,SLOT_Talk,0.5,true,500,animspeed * 1.5);
 	LoopAnim('Breath', animspeed);
}

function PlayPatrolStop()
{
	PlayWaiting();
}

function PlayWaitingAmbush()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	
	LoopAnim('Levitate', 0.4 + 0.3 * FRand());
}

function PlayDive()
{
	TweenToSwimming(0.2);
}

function TweenToFighter(float tweentime)
{
	if (Region.Zone.bWaterZone)
		TweenToSwimming(tweentime);
	else if (AnimSequence == 'Bowing')
		PlayAnim('GetUp', 0.4, 0.15);
	else
		TweenAnim('Fighter', tweentime);
}

function TweenToRunning(float tweentime)
{
	if (Region.Zone.bWaterZone)
		TweenToSwimming(tweentime);
	else if ( ((AnimSequence != 'Run') && (AnimSequence != 'RunFire')) || !bAnimLoop)
	{
		if (AnimSequence == 'Bowing')
			PlayAnim('GetUp', 0.4, 0.15);
		else
			TweenAnim('Run', tweentime);
	}
}

function TweenToWalking(float tweentime)
{
	if (Region.Zone.bWaterZone)
		TweenToSwimming(tweentime);
	else if (AnimSequence == 'Bowing')
		PlayAnim('GetUp', 0.4, 0.15);
	else if ( Weapon != None )
		TweenAnim('WalkTool', tweentime);
	else
		TweenAnim('Walk', tweentime);
}

function TweenToWaiting(float tweentime)
{
	if (Region.Zone.bWaterZone)
		TweenToSwimming(tweentime);
	else if (AnimSequence == 'Bowing')
		PlayAnim('GetUp', 0.4, 0.15);
	else
		TweenAnim('Breath', tweentime);
}

function TweenToPatrolStop(float tweentime)
{
	if (Region.Zone.bWaterZone)
		TweenToSwimming(tweentime);
	else if (AnimSequence == 'Bowing')
		PlayAnim('GetUp', 0.4, 0.15);
	else if ( IsInState('Guarding')) 
		TweenAnim('Pray', tweentime);
	else
		TweenAnim('Breath', tweentime);
}

function PlayRunning()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	LoopAnim('Run', -1.0/GroundSpeed,,0.4);
}

function PlayCombatMove()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	LoopAnim('Walk', -1.3/GroundSpeed,,0.4);
}

function PlayWalking()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	if ( Weapon != None )
		LoopAnim('WalkTool', -3/GroundSpeed,,0.4);
	else
		LoopAnim('Walk', -3/GroundSpeed,,0.4);
}

function PlayThreatening()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	Acceleration = vect(0,0,0);
	if (AnimSequence == 'Backup')
	{
		PlaySound(Cringe, SLOT_Talk);
		LoopAnim('Cringe', 0.4 + 0.7 * FRand(), 0.4);
	}
	else if (AnimSequence == 'Cringe')
	{
		if ( FRand() < 0.6 )
			PlaySound(Cringe, SLOT_Talk);
		LoopAnim('Cringe', 0.4 + 0.7 * FRand());
	}
	else if (AnimSequence == 'Bowing')
	{
		PlaySound(Bowing, SLOT_Talk);
		LoopAnim('Bowing', 0.4 + 0.7 * FRand());
	}
	else if (FRand() < 0.4)
		LoopAnim('Bowing', 0.4 + 0.7 * FRand(), 0.5);		
	else 
		PlayRetreating();
}

function PlayRetreating()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	bAvoidLedges = true;
	PlaySound(Backup, SLOT_Talk);
	DesiredRotation = Rotator(Enemy.Location - Location);
	DesiredSpeed = WalkingSpeed;
	Acceleration = AccelRate * Normal(Location - Enemy.Location);
	LoopAnim('Backup');
} 

function PlayTurning()
{
	TweenAnim('Walk', 0.3);
}

function PlayDying(name DamageType, vector HitLoc)
{
	//first check for head hit
	if ( (DamageType == 'Decapitated') || (HitLoc.Z - Location.Z > 0.5 * CollisionHeight) )
	{
		PlayHeadDeath(DamageType);
		return;
	}
	Super.PlayDying(DamageType, HitLoc);
}

function PlayHeadDeath(name DamageType)
{
	local carcass carc;

	carc = Spawn(class 'CreatureChunks',,, Location + CollisionHeight * vect(0,0,0.8), Rotation + rot(3000,0,16384) );
	if (carc != None)
	{
		carc.Mesh = mesh'NaliHead';
		carc.Initfor(self);
		carc.Velocity = Velocity + VSize(Velocity) * VRand();
		carc.Velocity.Z = FMax(carc.Velocity.Z, Velocity.Z);
	}
	PlaySound(sound'Death2n', SLOT_Talk, 4 * TransientSoundVolume);
	PlayAnim('Dead3',0.5, 0.1);
}

function PlayBigDeath(name DamageType)
{
	PlaySound(Die, SLOT_Talk, 4 * TransientSoundVolume);
	PlayAnim('Dead4',0.7, 0.1);
}

function PlayLeftDeath(name DamageType)
{
	PlaySound(sound'Death2n', SLOT_Talk, 4 * TransientSoundVolume);
	PlayAnim('Dead',0.7, 0.1);
}

function PlayRightDeath(name DamageType)
{
	PlaySound(Die, SLOT_Talk, 4 * TransientSoundVolume);
	PlayAnim('Dead2',0.7, 0.1);
}

function PlayGutDeath(name DamageType)
{
	PlaySound(Die, SLOT_Talk, 4 * TransientSoundVolume);
	if ( FRand() < 0.5 )
		PlayAnim('Dead2',0.7, 0.1);
	else
		PlayAnim('Dead',0.7, 0.1);
}

function PlayLanded(float impactVel)
{
	TweenAnim('Landed', 0.1);
}

function PlayVictoryDance()
{
	PlaySound(Sweat, SLOT_Talk);
	PlayAnim('Sweat', 1.0, 0.1);
}

function PlayMeleeAttack()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	PlayThreatening();
}

function PlayRangedAttack()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	PlayThreatening();
}

function PlaySwimming()
{
	Acceleration = WaterSpeed * VRand();
	Velocity = Acceleration;
	SetPhysics(PHYS_Falling);
	LoopAnim('Drowning', 0.5 + 0.9 * FRand());
}

function TweenToSwimming(float TweenTime)
{
	TweenAnim('Drowning', TweenTime);
}

state Retreating
{
	ignores HearNoise, Bump, AnimEnd;

	function EnemyNotVisible()
	{
		bCringing = False;
		Disable('EnemyNotVisible');
		Enable('SeePlayer');
	}

	function SeePlayer(actor SeenPlayer)
	{
		MakeNoise(1.0);
		Enable('EnemyNotVisible');
		Disable('SeePlayer');
	}

	function SetFall()
	{
		NextState = 'Retreating'; 
		NextLabel = 'Moving';
		NextAnim = AnimSequence;
		GotoState('FallingState'); 
	}

	function ReachedHome()
	{
		if (LineOfSightTo(Enemy))
		{
			if (Homebase(home) != None)
			{
				MoveTarget = None;
				health = Min(default.health, health+10);
				MakeNoise(1.0);
			}
			else
				ChangeDestination();
		}
		else
		{
			health = Min(default.health, health+5);
			GotoState('FadeOut');
		}	
	}

	function Bump(actor Other)
	{
		local vector VelDir, OtherDir;
		//log(Other.class$" bumped "$class);
		if (Pawn(Other) != None)
		{
			if ( (Enemy == Other) || SetEnemy(Pawn(Other)) )
			{
				bReadyToAttack = True; //can melee right away
				LastSeenPos = Enemy.Location;
				GotoState('Attacking');
				return;
			}
		}
		setTimer(1.0, false);
		VelDir = Normal(Velocity);
		VelDir.Z = 0;
		OtherDir = Normal(Other.Location - Location);
		OtherDir.Z = 0;
		if ( (VelDir Dot OtherDir) > 0.9 )
		{
			VelDir.X = Velocity.Y;
			VelDir.Y = -1 * Velocity.X;
			VelDir.Z = Velocity.Z;
			Velocity = VelDir;
		}
		Disable('Bump');
	}
	
	function PickDestination()
	{
		//log("find retreat destination");
		if (HomeBase(Home) == None)
			Home = FindRandomDest(); //find temporary home
	}

	function ChangeDestination()
	{
		local actor oldTarget;
		local Actor path;
		
		oldTarget = Home;
		PickDestination();
		if ( (Home == oldTarget) || (Home == None) )
			MoveTarget = None;
		else
		{
			path = FindPathToward(Home);
			if (path == None)
				MoveTarget = None;
			else 
			{
				MoveTarget = path;
				Destination = path.Location;
			}
		}
	}
	
	Function BeginState()
	{
		bReadyToAttack = true;
		Disable('SeePlayer');
	}

	function EndState()
	{
		bAvoidLedges = false;
		GroundSpeed = Default.GroundSpeed;
		Super.EndState();
	}

Begin:
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(0.12);
		Goto('Drowning');
	}
	bCringing = !bNeverBow;
	Target = None;
	TweenAnim('Backup',0.2);
	FinishAnim();
	
RunAway:
	WaitForLanding();
	if ( Enemy == None )
		GotoState('Attacking');
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(0.2);
		Goto('Drowning');
	}
	if (Home == None)
		PickDestination();
	bCringing = (bCringing && !bNeverBow && (FRand() < 0.8) && (VSize(Location - Enemy.Location) < 600) );
	if (Home == None)
		MoveTarget = None;
	else
	{
		PickNextSpot();
		if ( (MoveTarget != None) && (((MoveTarget.Location - Location) Dot (Enemy.Location - Location)) > 0)
			&& LineOfSightTo(Enemy) )
		{
			MoveTarget = None;
			if ( (Home == None) || !Home.IsA('HomeBase') )
				Home = None;
		}				
	}	
Moving:
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(0.12);
		Goto('Drowning');
	}
	If (MoveTarget == None)
	{
		GroundSpeed = Default.GroundSpeed;
		TweenAnim('Backup', 0.1);
		FinishAnim();
		PlayRetreating();
		Goto('Cringe');
	}
	bAvoidLedges = false;
	if (bCringing)
	{
		TweenAnim('Backup', 0.1);
		FinishAnim();
		if ( Enemy == None )
			GotoState('Attacking');
		GroundSpeed = GroundSpeed * WalkingSpeed;
		LoopAnim('Backup', -1.0/GroundSpeed,,0.4);
		Target = MoveTarget;
		StrafeFacing(MoveTarget.Location, Enemy);
		MoveTarget = Target;
Cringe:
		FinishAnim();
		Acceleration = vect(0,0,0);
		if (FRand() < 0.4)
		{
			PlayAnim('Cringe', 0.4 + 0.6 * FRand(), 0.4);
			if ( FRand() < 0.6 )
				PlaySound(Cringe, SLOT_Talk);
KeepCringeing:
			FinishAnim();
			PlayAnim('Cringe', 0.4 + 0.6 * FRand());
			FinishAnim();
			If (FRand() < 0.15)
			{
				PlayAnim('Cringe', 0.4 + 0.6 * FRand());
				Goto('KeepCringeing');
			}
		}
		else if (FRand() < 0.6)
		{
			PlayAnim('GetDown', 0.5, 0.3);
KeepBowing:
			FinishAnim();
			PlayAnim('Bowing', 0.3 + 0.7 * FRand(), 0.1);
			FinishAnim();
			If (FRand() < 0.25)
			{
				PlayAnim('Bowing', 0.3 + 0.7 * FRand());
				Goto('KeepBowing');
			}
			PlayAnim('GetUp', 0.5);
		}
		FinishAnim(); 
		if ( AnimSequence != 'Backup' )
		{
			TweenAnim('Backup',0.3);
			FinishAnim();
		}
	}
	else
	{
		GroundSpeed = Default.GroundSpeed;
		if ( ((AnimSequence != 'Run') && (AnimSequence != 'RunFire')) || !bAnimLoop )
		{
			TweenToRunning(0.1);
			FinishAnim();
			LoopAnim('Run', -1.0/GroundSpeed,,0.5);
		}
		MoveToward(MoveTarget); 
	}

	Goto('RunAway');

TakeHit:
	Goto('Moving');

Drowning:
	FinishAnim();
	PlaySwimming();
}


state TriggerAlarm
{
	ignores HearNoise, SeePlayer;

	function Bump(actor Other)
	{
		local vector VelDir, OtherDir;
		local float speed;

		if ( (Pawn(Other) != None) && Pawn(Other).bIsPlayer 
			&& (AttitudeToPlayer == ATTITUDE_Friendly) )
			return;

		Super.Bump(Other);
	}
}

state AlarmPaused
{
	ignores HearNoise, Bump;

	function PlayWaiting()
	{
		if ( !bGesture || (FRand() < 0.3) ) //pick first waiting animation
		{
			bGesture = true;
			PlaySound(UrgeFollow, SLOT_Talk);
			NextAnim = 'Follow';
 			LoopAnim(NextAnim, 0.4 + 0.6 * FRand());
		}
		else 
			Global.PlayWaiting();
	}

	function PlayWaitAround()
	{
		if ( (AnimSequence == 'Bowing') || (AnimSequence == 'GetDown') )
			PlayAnim('Bowing', 0.75, 0.1);
		else
			PlayAnim('GetDown', 0.7, 0.25);
	}

	function BeginState()
	{
		bGesture = false;
		Super.BeginState();
	}
}

state Guarding
{
	function PlayPatrolStop()
	{
		local float decision;
		local float animspeed;
		animspeed = 0.2 + 0.6 * FRand(); 
		decision = FRand();

		if ( AnimSequence == 'Breath' )
		{
			if (!bQuiet && (decision < 0.12) )
			{
				PlaySound(Cough,Slot_Talk,1.0,,800);
				LoopAnim('Cough', 0.85);
				return;
			}
			else if (decision < 0.24)
			{
				PlaySound(Sweat,Slot_Talk,0.3,,300);
				LoopAnim('Sweat', animspeed);
				return;
			}
			else if (!bQuiet && (decision < 0.65) )
			{
				PlayAnim('Pray', animspeed, 0.3);
				return;
			}
			else if ( decision < 0.8 )
			{
				PlayAnim('GetDown', 0.4, 0.1);
				return;
			}
		}
		else if ( AnimSequence == 'Pray' )
		{
			if (decision < 0.2)
				PlayAnim('Breath', animspeed, 0.3);
			else if ( decision < 0.35 )
				PlayAnim('GetDown', 0.4, 0.1);
			else
			{
				SpeakPrayer();
				PlayAnim('Pray', animspeed);
			}
			return;
		}
		else if ( AnimSequence == 'GetDown')
		{
			PlaySound(Bowing, SLOT_Talk);
			LoopAnim('Bowing', animspeed, 0.1);
			return;
		}
		else if ( AnimSequence == 'GetUp' )
			PlayAnim('Pray', animspeed, 0.1);
		else if ( AnimSequence == 'Bowing' )
		{
			if ( decision < 0.15 )
				PlayAnim('GetUp', 0.4);
			else
			{
				PlaySound(Bowing, SLOT_Talk);
				LoopAnim('Bowing', animspeed);
			}
			return;
		}		 	
		PlaySound(Breath,SLOT_Talk,0.5,true,500,animspeed * 1.5);
 		LoopAnim('Breath', animspeed);
	}
}

state FadeOut
{
	ignores HitWall, EnemyNotVisible, HearNoise, SeePlayer;

	function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
							Vector momentum, name damageType)
	{
		Global.TakeDamage(Damage, instigatedBy, hitlocation, momentum, damageType);
		if ( health <= 0 )
			return;
		if (NextState == 'TakeHit')
		{
			NextState = 'Attacking'; 
			NextLabel = 'Begin';
			GotoState('TakeHit'); 
		}
		else if ( Enemy != None )
			GotoState('Attacking');
	}

	function Tick(float DeltaTime)
	{
		local int NewFatness; 

		if ( !bFading )
		{
			NewFatness = fatness + 50 * DeltaTime;
			bFading = ( NewFatness > 160 );
		}
		else if ( Style == STY_Translucent )
		{
			ScaleGlow -= 3 * DeltaTime;
			if ( ScaleGlow < 0.3 )
			{
				PlaySound(sound'Teleport1',, 2.0);
				Destroy();
			}
			return;
		}
		else
		{
			NewFatness = fatness - 100 * DeltaTime;
			if ( NewFatness < 80 )
			{
				bUnlit = true;
				ScaleGlow = 2.0;
				Style = STY_Translucent;
			}
		}

		fatness = Clamp(NewFatness, 0, 255);
	}

	function BeginState()
	{
		bFading = false;
		Disable('Tick');
	}

	function EndState()
	{
		bUnlit = false;
		Style = STY_Normal;
		ScaleGlow = 1.0;
		fatness = Default.fatness;
	}

Begin:
	Acceleration = Vect(0,0,0);
	if ( NearWall(100) )
	{
		PlayTurning();
		TurnTo(Focus);
	}
	PlayAnim('Levitate', 0.3, 1.0);
	FinishAnim();
	PlayAnim('Levitate', 0.3);
	FinishAnim();
	LoopAnim('Levitate', 0.3);
	Enable('Tick');
}

state Roaming
{
	ignores EnemyNotVisible;

	function PickDestination()
	{
		if ( bHasWandered && (FRand() < 0.1) )
			GotoState('FadeOut');
		else
			Super.PickDestination();
		bHasWandered = true;
	}
}

state Wandering
{
	ignores EnemyNotVisible;

	function PickDestination()
	{
		if ( bHasWandered && (FRand() < 0.1) )
			GotoState('FadeOut');
		else
			Super.PickDestination();
		bHasWandered = true;
	}
}

defaultproperties
{
      syllable1=syl1n
      syllable2=syl2n
      syllable3=syl3n
      syllable4=syl4n
      syllable5=syl5n
      syllable6=syl6n
      urgefollow=follow1n
      Cringe=cringe2n
      Cough=cough1n
      Sweat=sweat1n
      Bowing=bowing1n
      Backup=backup2n
	  Pray=pray1n
	  Breath=breath1n
      CarcassType=NaliCarcass
      Health=40
      UnderWaterTime=+00006.000000
      bHasRangedAttack=True
	  bIsWuss=True
      SightRadius=+01500.000000
      TimeBetweenAttacks=+00000.500000
      MeleeRange=+00040.000000
      Aggressiveness=-000010.000000
      AttitudeToPlayer=ATTITUDE_Friendly
      Intelligence=BRAINS_HUMAN
      ReFireRate=+00000.500000
      WalkingSpeed=+00000.400000
      GroundSpeed=+00300.000000
      WaterSpeed=+00100.000000
      AccelRate=+00900.000000
      JumpZ=-00001.000000
      MaxStepHeight=+00025.000000
      HitSound1=injur1n
      HitSound2=injur2n
      Acquire=contct1n
      Fear=fear1n
      Roam=breath1n
      Threaten=contct3n
      Die=death1n
	  Buoyancy=+00095.000000
      DrawType=DT_Mesh
      Mesh=Nali1
      CollisionRadius=+00024.000000
      CollisionHeight=+00048.000000
      RotationRate=(Pitch=2048,Yaw=40000,Roll=0)
}

//=============================================================================
// Krall.
//=============================================================================
class Krall expands ScriptedPawn;

#exec MESH IMPORT MESH=KrallM ANIVFILE=MODELS\krall_a.3D DATAFILE=MODELS\krall_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=KrallM X=70 Y=-240 Z=-120 YAW=64 ROLL=-64

#exec MESH SEQUENCE MESH=krallM SEQ=All          STARTFRAME=0   NUMFRAMES=498
#exec MESH SEQUENCE MESH=krallM SEQ=Breath       STARTFRAME=0   NUMFRAMES=6   RATE=6
#exec MESH SEQUENCE MESH=krallM SEQ=Crouch       STARTFRAME=6   NUMFRAMES=1				GROUP=Ducking
#exec MESH SEQUENCE MESH=krallM SEQ=Die1         STARTFRAME=7   NUMFRAMES=23  RATE=15
#exec MESH SEQUENCE MESH=krallM SEQ=Die2         STARTFRAME=30  NUMFRAMES=22  RATE=15
#exec MESH SEQUENCE MESH=krallM SEQ=Die3         STARTFRAME=52  NUMFRAMES=12  RATE=15
#exec MESH SEQUENCE MESH=krallM SEQ=Die4         STARTFRAME=64  NUMFRAMES=17  RATE=15
#exec MESH SEQUENCE MESH=krallM SEQ=Fighter      STARTFRAME=157 NUMFRAMES=1 
#exec MESH SEQUENCE MESH=krallM SEQ=Hit          STARTFRAME=81  NUMFRAMES=7   RATE=15
#exec MESH SEQUENCE MESH=krallM SEQ=HeadHit      STARTFRAME=88  NUMFRAMES=1
#exec MESH SEQUENCE MESH=krallM SEQ=LeftHit      STARTFRAME=89  NUMFRAMES=1
#exec MESH SEQUENCE MESH=krallM SEQ=GutHit       STARTFRAME=90  NUMFRAMES=1
#exec MESH SEQUENCE MESH=krallM SEQ=RightHit     STARTFRAME=91  NUMFRAMES=1
#exec MESH SEQUENCE MESH=krallM SEQ=Jump         STARTFRAME=92  NUMFRAMES=1
#exec MESH SEQUENCE MESH=krallM SEQ=Land		    STARTFRAME=93  NUMFRAMES=1
#exec MESH SEQUENCE MESH=krallM SEQ=Look         STARTFRAME=94  NUMFRAMES=23  RATE=15
#exec MESH SEQUENCE MESH=krallM SEQ=Run          STARTFRAME=117 NUMFRAMES=10  RATE=15
#exec MESH SEQUENCE MESH=krallM SEQ=Shoot1       STARTFRAME=127 NUMFRAMES=8   RATE=15   Group=Attack
#exec MESH SEQUENCE MESH=krallM SEQ=ThreatShoot1 STARTFRAME=127 NUMFRAMES=3   RATE=15
#exec MESH SEQUENCE MESH=krallM SEQ=Shoot2       STARTFRAME=135 NUMFRAMES=22  RATE=15	GROUP=MovingAttack
#exec MESH SEQUENCE MESH=krallM SEQ=Strike1      STARTFRAME=157 NUMFRAMES=11  RATE=15   Group=Attack
#exec MESH SEQUENCE MESH=krallM SEQ=Strike2      STARTFRAME=168 NUMFRAMES=12  RATE=15   Group=Attack
#exec MESH SEQUENCE MESH=krallM SEQ=Strike3      STARTFRAME=180 NUMFRAMES=15  RATE=15   Group=Attack
#exec MESH SEQUENCE MESH=krallM SEQ=Swim         STARTFRAME=195 NUMFRAMES=15  RATE=15
#exec MESH SEQUENCE MESH=krallM SEQ=T3           STARTFRAME=210 NUMFRAMES=7   RATE=15
#exec MESH SEQUENCE MESH=krallM SEQ=Throw        STARTFRAME=217 NUMFRAMES=25  RATE=15   Group=Attack
#exec MESH SEQUENCE MESH=krallM SEQ=Twirl        STARTFRAME=242 NUMFRAMES=35
#exec MESH SEQUENCE MESH=krallM SEQ=Walk         STARTFRAME=277 NUMFRAMES=15  RATE=15
#exec MESH SEQUENCE MESH=krallM SEQ=Breath2      STARTFRAME=292 NUMFRAMES=7   RATE=6	GROUP=Ducking
#exec MESH SEQUENCE MESH=krallM SEQ=Grasp        STARTFRAME=299 NUMFRAMES=18  RATE=15	GROUP=Ducking
#exec MESH SEQUENCE MESH=krallM SEQ=HeadRub      STARTFRAME=317 NUMFRAMES=18  RATE=15	GROUP=Ducking
#exec MESH SEQUENCE MESH=krallM SEQ=Laugh        STARTFRAME=335 NUMFRAMES=25  RATE=15	GROUP=Ducking
#exec MESH SEQUENCE MESH=krallM SEQ=Toss         STARTFRAME=360 NUMFRAMES=25  RATE=15	GROUP=Ducking
#exec MESH SEQUENCE MESH=krallM SEQ=Dead5        STARTFRAME=385 NUMFRAMES=31  RATE=15	
#exec MESH SEQUENCE MESH=krallM SEQ=Drag         STARTFRAME=416 NUMFRAMES=20  RATE=15	
#exec MESH SEQUENCE MESH=krallM SEQ=LegLoss      STARTFRAME=436 NUMFRAMES=13  RATE=15	
#exec MESH SEQUENCE MESH=krallM SEQ=Shoot3       STARTFRAME=449 NUMFRAMES=10  RATE=15	
#exec MESH SEQUENCE MESH=krallM SEQ=Sleep1       STARTFRAME=459 NUMFRAMES=8   RATE=6	
#exec MESH SEQUENCE MESH=krallM SEQ=Sleep2       STARTFRAME=467 NUMFRAMES=8   RATE=6	
#exec MESH SEQUENCE MESH=krallM SEQ=Sleep3       STARTFRAME=475 NUMFRAMES=8   RATE=6	
#exec MESH SEQUENCE MESH=krallM SEQ=SwimFire     STARTFRAME=483 NUMFRAMES=15  RATE=15	
#exec MESH SEQUENCE MESH=krallM SEQ=LeglessDeath STARTFRAME=448 NUMFRAMES=1	

#exec TEXTURE IMPORT NAME=jkrall FILE=MODELS\krall.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=krallM X=0.11 Y=0.11 Z=0.22
#exec MESHMAP SETTEXTURE MESHMAP=krallM NUM=0 TEXTURE=jkrall

#exec MESH NOTIFY MESH=krallM SEQ=Shoot1 TIME=0.25 FUNCTION=SpawnShot
#exec MESH NOTIFY MESH=krallM SEQ=Shoot2 TIME=0.48 FUNCTION=SpawnShot
#exec MESH NOTIFY MESH=krallM SEQ=Shoot2 TIME=0.8 FUNCTION=ShotMove
#exec MESH NOTIFY MESH=krallM SEQ=Shoot3 TIME=0.35 FUNCTION=SpawnShot
#exec MESH NOTIFY MESH=krallM SEQ=Strike1 TIME=0.375 FUNCTION=StrikeDamageTarget
#exec MESH NOTIFY MESH=krallM SEQ=Strike2 TIME=0.5 FUNCTION=StrikeDamageTarget
#exec MESH NOTIFY MESH=krallM SEQ=Strike3 TIME=0.26 FUNCTION=PoundDamageTarget
#exec MESH NOTIFY MESH=krallM SEQ=Throw TIME=0.19 FUNCTION=ThrowDamageTarget
#exec MESH NOTIFY MESH=krallM SEQ=Throw TIME=0.42 FUNCTION=ThrowTarget
#exec MESH NOTIFY MESH=krallM SEQ=Grasp TIME=0.58 FUNCTION=GrabDice
#exec MESH NOTIFY MESH=krallM SEQ=Toss  TIME=0.63 FUNCTION=ThrowDice
#exec MESH NOTIFY MESH=krallM SEQ=Die1  TIME=0.64 FUNCTION=LandThump
#exec MESH NOTIFY MESH=krallM SEQ=Die2  TIME=0.81 FUNCTION=LandThump
#exec MESH NOTIFY MESH=krallM SEQ=Die3  TIME=0.54 FUNCTION=LandThump
#exec MESH NOTIFY MESH=krallM SEQ=Die4  TIME=0.75 FUNCTION=LandThump
#exec MESH NOTIFY MESH=krallM SEQ=Dead5 TIME=0.83 FUNCTION=LandThump

#exec AUDIO IMPORT FILE="Sounds\Krall\amb1kr.WAV" NAME="amb1kr" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\syl13kr.WAV" NAME="syl1kr" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\syl14kr.WAV" NAME="syl2kr" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\syl15kr.WAV" NAME="syl3kr" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\syl16kr.WAV" NAME="syl4kr" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\syl17kr.WAV" NAME="syl5kr" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\syl18kr.WAV" NAME="syl6kr" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\death1a.WAV" NAME="death1k" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\death2a.WAV" NAME="death2k" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\strike1kr.WAV" NAME="strike1k" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\staflp4kr.WAV" NAME="staflp4k" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\injur1a.WAV" NAME="injur1k" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\injur2a.WAV" NAME="injur2k" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\chlng1a.WAV" NAME="chlng1k" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\chlng2a.WAV" NAME="chlng2k" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\hit2a.WAV" NAME="hit2k" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\throw1kr.WAV" NAME="throw1k" GROUP="Krall"
#exec AUDIO IMPORT FILE="Sounds\Krall\snore1kr.WAV" NAME="snore1k" GROUP="Krall"
//FIXME - also have look1a (for look anim)
//FIXME- How to use strike3kr?

var() byte StrikeDamage,
	ThrowDamage,
	PoundDamage;
var bool AttackSuccess;
var() bool bSpearToss;
var() bool bDicePlayer;
var() bool bSleeping;
var   bool bHasDice;
var	  bool bHeldDice;
var(Sounds) sound strike1;
var(Sounds) sound strike2;
var(Sounds) sound twirl;
var(Sounds) sound syllable1;
var(Sounds) sound syllable2;
var(Sounds) sound syllable3;
var(Sounds) sound syllable4;
var(Sounds) sound syllable5;
var(Sounds) sound syllable6;
var(Sounds) sound die2;
var(Sounds)	sound spearHit;
var(Sounds) sound spearThrow;
var 	name phrase;
var		byte phrasesyllable;
var		float	voicePitch;
var Dice Toy1;
var Dice Toy2;
var()	float MinDuckTime;
var		float LastDuckTime;

function PreBeginPlay()
{
	bCanSpeak = true;
	voicePitch = 0.25 + 0.5 * FRand();
	if ( CombatStyle == Default.CombatStyle)
		CombatStyle = CombatStyle + 0.4 * FRand() - 0.2;
	bCanDuck = (FRand() < 0.5);
	Super.PreBeginPlay();
	if ( bDicePlayer )
		PeripheralVision = 1.0; 
	if ( Skill == 0 )
		ProjectileSpeed *= 0.85;
	else if ( Skill > 2 )
	{
		bCanStrafe = true;
		ProjectileSpeed *= 1.1;
	}
	if ( !IsA('KrallElite') )
		bLeadTarget = false;
}

function Carcass SpawnCarcass()
{
	local carcass carc;
	
	carc = Spawn(CarcassType);
	carc.Initfor(self);
	carc.bReducedHeight = true;
	carc.PrePivot = PrePivot;
	return carc;
}

function PlayTakeHit(float tweentime, vector HitLoc, int damage)
{
	local carcass carc;
	local LeglessKrall rep;
	local pawn OtherPawn;
	local Actor A;

	if ( (Health > 30) || (damage < 24) || (HitLoc.Z > Location.Z) || (FRand() < 0.6) )
	{
		Super.PlayTakeHit(tweentime, HitLoc, damage);
		return;
	}

	carc = Spawn(class 'CreatureChunks',,, Location - CollisionHeight * vect(0,0,0.5), Rotation + rot(3000,0,16384) );
	if (carc != None)
	{
		carc.Mesh = mesh'KrallFoot';
		carc.Initfor(self);
		carc.Velocity = Velocity + VSize(Velocity) * VRand();
		carc.Velocity.Z = FMax(carc.Velocity.Z, Velocity.Z);
	}
	carc = Spawn(class 'CreatureChunks',,, Location - CollisionHeight * vect(0,0,0.5), Rotation + rot(3000,0,16384) );
	if (carc != None)
	{
		carc.Mesh = mesh'KrallFoot';
		carc.Initfor(self);
		carc.Velocity = Velocity + VSize(Velocity) * VRand();
		carc.Velocity.Z = FMax(carc.Velocity.Z, Velocity.Z);
	}

	SetCollision(false, false, false);
	OtherPawn = Level.PawnList;
	while ( OtherPawn != None )
	{
		OtherPawn.Killed(enemy, self, '');
		OtherPawn = OtherPawn.nextPawn;
	}
	if ( CarriedDecoration != None )
		DropDecoration();
	if ( Event != '' )
		foreach AllActors( class 'Actor', A, Event )
			A.Trigger( self, enemy );
	Level.Game.DiscardInventory(self);
	Velocity.Z *= 1.3;
	rep = Spawn(class'LeglessKrall');
	rep.InitFor(self);
	destroy();
}

function ZoneChange(ZoneInfo newZone)
{
	bCanSwim = newZone.bWaterZone; //only when it must
		
	if ( newZone.bWaterZone )
		CombatStyle = 1.0; //always charges when in the water
	else if (Physics == PHYS_Swimming)
		CombatStyle = Default.CombatStyle;

	Super.ZoneChange(newZone);
}

function SetMovementPhysics()
{
	if ( Region.Zone.bWaterZone )
		SetPhysics(PHYS_Swimming);
	else if (Physics != PHYS_Walking)
		SetPhysics(PHYS_Walking); 
}

function TryToDuck(vector duckDir, bool bReversed)
{
	local vector HitLocation, HitNormal, Extent;
	local actor HitActor;

	//log("duck");
	
	if ( Level.TimeSeconds - LastDuckTime < (0.25 + 0.5 * FRand()) * MinDuckTime )
		return;	
	duckDir.Z = 0;
	if ( (Skill == 0) && (FRand() < 0.5) )
		DuckDir *= -1;	

	Extent.X = CollisionRadius;
	Extent.Y = CollisionRadius;
	Extent.Z = CollisionHeight;
	HitActor = Trace(HitLocation, HitNormal, Location + 128 * duckDir, Location, false, Extent);
	if (HitActor != None)
	{
		duckDir *= -1;
		HitActor = Trace(HitLocation, HitNormal, Location + 128 * duckDir, Location, false, Extent);
	}
	if (HitActor != None)
		return;
	
	HitActor = Trace(HitLocation, HitNormal, Location + 128 * duckDir - MaxStepHeight * vect(0,0,1), Location + 128 * duckDir, false, Extent);
	if (HitActor == None)
		return;
		
	//log("good duck");

	LastDuckTime = Level.TimeSeconds;
	SetFall();
	TweenAnim('Jump', 0.3);
	Velocity = duckDir * 1.5 * GroundSpeed;
	Velocity.Z = 200;
	SetPhysics(PHYS_Falling);
	GotoState('FallingState','Ducking');
}	

//=========================================================================================
// Speech


function SpeechTimer()
{
	//last syllable expired.  Decide whether to keep the floor or quit
	if (FRand() < 0.3)
	{
		bIsSpeaking = false;
		if (TeamLeader != None)
			TeamLeader.bTeamSpeaking = false;
	}
	else
		Speak();
}

function SpeakOrderTo(ScriptedPawn TeamMember)
{
	phrase = '';
	if ( !TeamMember.bCanSpeak || (FRand() < 0.5) )
		Speak();
	else  
	{
		if (SkaarjWarrior(TeamMember) != None)
			SkaarjWarrior(TeamMember).phrase = '';
		TeamMember.Speak();
	}
}

function SpeakTo(ScriptedPawn Other)
{
	if (Other.bIsSpeaking || ((TeamLeader != None) && TeamLeader.bTeamSpeaking) )
		return;
	
	phrase = '';
	Speak();
}

function Speak()
{
	local float decision;
	
	//if (phrase != '')
	//	SpeakPhrase();
	bIsSpeaking = true;
	decision = FRand();
	if (TeamLeader != None)	
		TeamLeader.bTeamSpeaking = true;
	if (decision < 0.167)
		PlaySound(Syllable1,SLOT_Talk,0.3 + FRand(),,, FRand() + voicePitch);
	else if (decision < 0.333)
		PlaySound(Syllable2,SLOT_Talk,0.3 + FRand(),,, FRand() + voicePitch);
	else if (decision < 0.5)
		PlaySound(Syllable3,SLOT_Talk,0.3 + FRand(),,, FRand() + voicePitch);
	else if (decision < 0.667)
		PlaySound(Syllable4,SLOT_Talk,0.3 + FRand(),,, FRand() + voicePitch);
	else if (decision < 0.833)
		PlaySound(Syllable5,SLOT_Talk,0.3 + FRand(),,, FRand() + voicePitch);
	else 
		PlaySound(Syllable6,SLOT_Talk,0.3 + FRand(),,, FRand() + voicePitch);

	SpeechTime = 0.1 + 0.3 * FRand();
}
	
function PlayAcquisitionSound()
{
	if ( bCanSpeak && (TeamLeader != None) && !TeamLeader.bTeamSpeaking )
	{
		phrase = 'Acquisition';
		phrasesyllable = 0;
		Speak();
		return;
	}
	Super.PlayAcquisitionSound(); 
}

function PlayFearSound()
{
	if ( bCanSpeak && (TeamLeader != None) && !TeamLeader.bTeamSpeaking )
	{
		phrase = 'Fear';
		phrasesyllable = 0;
		Speak();
		return;
	}
	Super.PlayFearSound(); 
}

function PlayRoamingSound()
{
	if ( bCanSpeak && (TeamLeader != None) && !TeamLeader.bTeamSpeaking  && (FRand() < 0.5) )
	{
		phrase = '';
		Speak();
		return;
	} 
	Super.PlayRoamingSound();
}

function PlayThreateningSound()
{
	if ( bCanSpeak && (FRand() < 0.6) && ((TeamLeader == None) || !TeamLeader.bTeamSpeaking) )
	{
		phrase = 'Threaten';
		phrasesyllable = 0;
		Speak();
		return;
	} 
	Super.PlayThreateningSound();
}

//=========================================================================================
function eAttitude AttitudeToCreature(Pawn Other)
{
	if ( Other.IsA('Nali') )
		return ATTITUDE_Hate;
	else if ( Other.IsA('Krall') )
		return ATTITUDE_Friendly;
	else
		return ATTITUDE_Ignore;
}

function ThrowDice()
{
	local Dice d1, d2;
	local vector X,Y,Z, ThrowLoc;

	d1 = Krall(TeamLeader).Toy1;
	d2 = Krall(TeamLeader).Toy2;
	GetAxes(Rotation, X,Y,Z);
	ThrowLoc = Location + X * CollisionRadius + Y * CollisionRadius - Z * 0.6 * CollisionHeight; 
	d1.SetLocation(ThrowLoc);
	d2.SetLocation(ThrowLoc + vect(2,2,2));

	d1.instigator = self;
	d1.Throw(Y);

	d2.instigator = self;
	d2.Throw(Y);
}

function GrabDice()
{
	if ( Krall(TeamLeader).Toy1 == None )
		Krall(TeamLeader).Toy1 = Spawn(class'Dice');
	if ( Krall(TeamLeader).Toy2 == None )
		Krall(TeamLeader).Toy2 = Spawn(class'Dice');
		
	Krall(TeamLeader).Toy1.bHidden = True;
	Krall(TeamLeader).Toy2.bHidden = True;
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

	if ( bSleeping )
	{
		animspeed = 0.5 + 0.5 * FRand(); 
		SetAlertness(-1.0);
		LoopAnim('Sleep1', AnimSpeed );
		PlaySound(sound'Snore1K',SLOT_Talk);
		return;
	}

	decision = FRand();

	if ( bDicePlayer ) 
	{
		animspeed = 0.4 + 0.6 * FRand(); 
		SetAlertness(-1.0);
		
		if ( TeamLeader == None )
		{
			if ( decision < 0.9 )
				LoopAnim('Breath2', animspeed, 0.5);
			else
				LoopAnim('HeadRub', animspeed);
			return;
		}

		if ( !TeamLeader.bTeamSpeaking )
		{
			phrase = '';
			Speak();
		}
		if ( bHasDice )
		{
			if ( AnimSequence == 'Toss' )
			{
				bHasDice = false;
				Krall(TeamLeader).bHeldDice = false;
			}
			else if ( FRand() < 0.8 )
			{
				PlayAnim('Toss', animspeed);
				return;
			}
		}
		if ( Krall(TeamLeader).bHeldDice || ( FRand() < 0.65) )
		{
			if ( decision < 0.8 )
				LoopAnim('Breath2', animspeed, 0.5);
			else if ( decision < 0.9 )
				LoopAnim('Laugh', animspeed);
			else 
				LoopAnim('HeadRub', animspeed);
			return;
		}
		Krall(TeamLeader).bHeldDice = True;
		bHasDice = True;
		PlayAnim('Grasp', animspeed);
		return;
	}

	if (AnimSequence == 'Look')
	{
		SetAlertness(0.0);
		if (!bQuiet && decision < 0.3) 
		{
			LoopAnim('Twirl', 0.3 + 0.6 * FRand());
		    PlaySound(Twirl,SLOT_Interact,0.5,,500);
		}	
		else 
			LoopAnim('Breath', 0.2 + 0.7 * FRand());
		return;
	}
 	else if (AnimSequence == 'Twirl')
	{
 		SetAlertness(0.0);
 		if (decision < 0.5) 
		{
		    PlaySound(Twirl,SLOT_Interact,0.5,,500);	
			LoopAnim('Twirl', 0.3 + 0.6 * FRand());
		}
		else 
			LoopAnim( 'Breath', 0.2 + 0.7 * FRand());
		return;
	}
	
	if (decision < 0.2)
	{
		SetAlertness(0.5);
		LoopAnim('Look', 0.2 + 0.7 * FRand());
	}
	else
	{
		SetAlertness(0.0);
		LoopAnim('Breath', 0.2 + 0.7 * FRand());
	}
}

function PlayPatrolStop()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
		
	if ( (AnimSequence == 'Breath') && (FRand() < 0.4) )
	{
		SetAlertness(0.5);
		LoopAnim('Look', 0.2 + 0.7 * FRand());
		return;
	}
	else if ( !bQuiet && (AnimSequence == 'Look') && (FRand() < 0.3) )
	{
		SetAlertness(0.0);
	    PlaySound(Twirl,SLOT_Interact,0.5,,500);	
		LoopAnim('Twirl', 0.3 + 0.6 * FRand());
		return;
	}
 	else if ( (AnimSequence == 'Twirl') && (FRand() < 0.5) )
	{
 		SetAlertness(0.0);
	    PlaySound(Twirl,SLOT_Interact,0.5,,500);	
		LoopAnim('Twirl', 0.3 + 0.6 * FRand());
		return;
	}
	SetAlertness(0.0);
	LoopAnim('Breath', 0.2 + 0.7 * FRand());
}
	
function PlayWaitingAmbush()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	if (FRand() < 0.4)
		LoopAnim('Look', 0.3);
	else 
		LoopAnim('Breath', 0.3 + 0.5 * FRand());
}

function PlayChallenge()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	if ( TryToCrouch() )
	{
		TweenAnim('Crouch', 0.12);
		return;
	}	
	PlayThreateningSound();
	PlayAnim('T3', 0.7, 0.15);
}

function PlayDive()
{
	TweenToSwimming(0.2);
}

function TweenToFighter(float tweentime)
{
	if ( bDicePlayer )
	{
		PeripheralVision = Default.PeripheralVision;
		bDicePlayer = false;
	}
	bSleeping = false;
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(tweentime);
		return;
	}
	TweenAnim('Fighter', tweentime);
}

function TweenToRunning(float tweentime)
{
	if ( bDicePlayer )
	{
		PeripheralVision = Default.PeripheralVision;
		bDicePlayer = false;
	}
	bSleeping = false;
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(tweentime);
		return;
	}
	if ( (AnimSequence == 'Shoot2') && IsAnimating() )
		return;
	if ( (AnimSequence != 'Run') || !bAnimLoop )
		TweenAnim('Run', tweentime);
}

function TweenToWalking(float tweentime)
{
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(tweentime);
		return;
	}
	TweenAnim('Walk', tweentime);
}

function TweenToWaiting(float tweentime)
{
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(tweentime);
		return;
	}
	if ( bDicePlayer ) 
	{
		TweenAnim('Breath2', tweentime);
		return;
	}
	TweenAnim('Breath', tweentime);
}

function TweenToPatrolStop(float tweentime)
{
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(tweentime);
		return;
	}
	TweenAnim('Breath', tweentime);
}

function TweenToFalling()
{
	TweenAnim('Jump', 0.35);
}

function PlayInAir()
{
	TweenAnim('Jump', 0.2);
}

function PlayOutOfWater()
{
	TweenAnim('Land',0.8);
}

function PlayLanded(float impactVel)
{
	TweenAnim('Land', 0.1);
}

function PlayMovingAttack()
{
	if (Region.Zone.bWaterZone)
	{
		PlayAnim('SwimFire');
		SpawnShot();
		return;
	}
	DesiredSpeed = 0.4;
	MoveTimer += 0.2;
	PlayAnim('Shoot2');
}

function PlayRunning()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}

	DesiredSpeed = 1.0;
	if (Focus == Destination)
	{
		LoopAnim('Run', -1.0/GroundSpeed,, 0.4);
		return;
	}	
	LoopAnim('Run', StrafeAdjust(),,0.3);
}

function PlayWalking()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	LoopAnim('Walk', 0.88);
}

function TweenToSwimming(float tweentime)
{
	if ( (AnimSequence != 'Swim') || !bAnimLoop )
		TweenAnim('Swim', tweentime);
}

function PlaySwimming()
{
	LoopAnim('Swim', -1.0/WaterSpeed,,0.3);
}

function PlayThreatening()
{
	local float decision, animspeed;

	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	decision = FRand();
	animspeed = 0.4 + 0.6 * FRand(); 

	if ( decision < 0.6 )
		PlayAnim('Breath', animspeed, 0.3);
	else if ( decision < 0.7 )
	{
	    PlaySound(Twirl,SLOT_Interact,0.5,,500);	
		PlayAnim('Twirl', animspeed, 0.3);
	}
	else 
	{
		PlayThreateningSound();
		if ( decision < 0.8 )
			PlayAnim('T3', animspeed, 0.3);
		else if ( decision < 0.9 )
			PlayAnim('ThreatShoot1', 0.3, 0.3);
		else 
			TweenAnim('Fighter', 0.3);
	}
}

function PlayTurning()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	TweenAnim('Walk', 0.3);
}

function PlayBigDeath(name DamageType)
{
	PlaySound(Die2, SLOT_Talk, 4 * TransientSoundVolume);
	PlayAnim('Die2',0.7,0.1);
}

function PlayHeadDeath(name DamageType)
{
	local carcass carc;

	if ( (DamageType == 'Decapitated') || ((Health < -20) && (FRand() < 0.5)) )
	{
		carc = Spawn(class 'CreatureChunks',,, Location + CollisionHeight * vect(0,0,0.8), Rotation + rot(3000,0,16384) );
		if (carc != None)
		{
			carc.Mesh = mesh'KrallHead';
			carc.Initfor(self);
			carc.Velocity = Velocity + VSize(Velocity) * VRand();
			carc.Velocity.Z = FMax(carc.Velocity.Z, Velocity.Z);
		}
		PlayAnim('Dead5',0.7,0.1);
		if ( Velocity.Z < 120 )
		{
			Velocity = GroundSpeed * vector(Rotation);
			Velocity.Z = 150;
		}
	}
	else if ( FRand() < 0.5 )
		PlayAnim('Die4',0.7,0.1);
	else
		PlayAnim('Die3',0.7,0.1);
	PlaySound(Die, SLOT_Talk, 4 * TransientSoundVolume);
}

function PlayLeftDeath(name DamageType)
{
	PlayAnim('Die4',0.7, 0.1);
	PlaySound(Die,SLOT_Talk, 4 * TransientSoundVolume);
}

function PlayRightDeath(name DamageType)
{
	PlayAnim('Die3',0.7,0.1);
	PlaySound(Die,SLOT_Talk, 4 * TransientSoundVolume);
}

function PlayGutDeath(name DamageType)
{
	if ( Velocity.Z > 100 )
		PlayAnim('Die3',0.7,0.1);
	else
		PlayAnim('Die1',0.7,0.1);
	PlaySound(Die,SLOT_Talk, 4 * TransientSoundVolume);
}

function PlayVictoryDance()
{
    PlaySound(Twirl,SLOT_Interact,0.5,,500);	
	PlayAnim('Twirl', 0.5, 0.1);
}

function bool CanFireAtEnemy()
{
	local vector HitLocation, HitNormal,X,Y,Z, projStart, EnemyDir, EnemyUp;
	local actor HitActor;
	local float EnemyDist;
		
	EnemyDir = Enemy.Location - Location;
	EnemyDist = VSize(EnemyDir);
	EnemyUp = Enemy.CollisionHeight * vect(0,0,0.9);
	if ( EnemyDist > 300 )
	{
		EnemyDir = 300 * EnemyDir/EnemyDist;
		EnemyUp = 300 * EnemyUp/EnemyDist;
	}
	
	GetAxes(Rotation,X,Y,Z);
	projStart = Location + 0.9 * CollisionRadius * X - 0.7 * CollisionRadius * Y;
	HitActor = Trace(HitLocation, HitNormal, projStart + EnemyDir + EnemyUp, projStart, true);

	if ( (HitActor == None) || (HitActor == Enemy) 
		|| ((Pawn(HitActor) != None) && (AttitudeTo(Pawn(HitActor)) <= ATTITUDE_Ignore)) )
		return true;

	HitActor = Trace(HitLocation, HitNormal, projStart + EnemyDir, projStart , true);

	return ( (HitActor == None) || (HitActor == Enemy) 
			|| ((Pawn(HitActor) != None) && (AttitudeTo(Pawn(HitActor)) <= ATTITUDE_Ignore)) );
}
	
function SpawnShot()
{
	DesiredSpeed = 0.0; //For Kraal shoot2 (walking shoot, w/ pause)
	FireProjectile( vect(0.9, -0.5, 0), 500);
}

function ShotMove()
{
	DesiredSpeed = 1.0;
}

function StrikeDamageTarget()
{
	if (MeleeDamageTarget(StrikeDamage, StrikeDamage * 700 * Normal(Target.Location - Location)))
		PlaySound(SpearHit,SLOT_Interact);
}

function PoundDamageTarget()
{
	if (MeleeDamageTarget(PoundDamage, PoundDamage * 500 * Normal(Target.Location - Location)))
		PlaySound(SpearHit,SLOT_Interact);
}

function ThrowDamageTarget()
{
	AttackSuccess = MeleeDamageTarget(ThrowDamage, vect(0,0,0));
}

function ThrowTarget() 
{
	local rotator newRot;
	if (AttackSuccess && (Vsize(Target.Location - Location) < CollisionRadius + Target.CollisionRadius + 1.5 * MeleeRange) )
	{
		PlaySound(SpearThrow,SLOT_Interact);
		newRot = Target.Rotation;
		newRot.Pitch = 4096;
		Target.SetRotation(newRot);
		if (Pawn(Target) != None)
		{
			Pawn(Target).AddVelocity( 
				(50000.0 * (Normal(Target.Location - Location) + vect(0,0,1)))/Target.Mass);
			if (PlayerPawn(Target) != None)
				PlayerPawn(Target).ShakeView(0.2, 2000, -10);
		}
	}
}
	
function PlayMeleeAttack()
{
	local float decision;

	decision = FRand();
	if (!bSpearToss)
		decision *= 0.7;
	if (decision < 0.2)
	{
		PlayAnim('Strike1'); 
		PlaySound(Strike1,SLOT_Interact);
	}
 	else if (decision < 0.4)
 	{
   		PlayAnim('Strike2');
   		PlaySound(Strike2,SLOT_Interact);
   	}
 	else if (decision < 0.7)
 	{
 		PlayAnim('Strike3');
 		PlaySound(Strike1,SLOT_Interact);
 	}
 	else
 	{
 		PlayAnim('Throw');
 		PlaySound(Strike2,SLOT_Interact);
 	} 
}

function PlayRangedAttack()
{
	local float tweenin;
	
	if (Region.Zone.bWaterZone)
	{
		PlayAnim('SwimFire');
		SpawnShot();
		return;
	}
	if (AnimSequence == 'Shoot1')
		tweenin = 0.3 * FRand();
	else
		tweenin = 0.35;
	PlayAnim('Shoot1', 1.0, tweenin);
}

state TacticalMove
{
ignores SeePlayer, HearNoise;

	function TweenToRunning(float tweentime)
	{
		if ( bDicePlayer )
		{
			PeripheralVision = Default.PeripheralVision;
			bDicePlayer = false;
		}
		if (Region.Zone.bWaterZone)
		{
			TweenToSwimming(tweentime);
			return;
		}
		if ( bCanStrafe && (AnimSequence == 'Shoot2') && IsAnimating() )
			return;
		if ( (AnimSequence != 'Run') || !bAnimLoop )
			TweenAnim('Run', tweentime);
	}
}
	
defaultproperties
{
     StrikeDamage=20
     ThrowDamage=30
     PoundDamage=20
     RangedProjectile=KraalBolt
     bSpearToss=True
     Strike1=strike1k
     Strike2=strike1k
     Twirl=staflp4k
     syllable1=syl1kr
     syllable2=syl2kr
     syllable3=syl3kr
     syllable4=syl4kr
     syllable5=syl5kr
     syllable6=syl6kr
     Die2=death2k
     spearHit=hit2k
     spearThrow=throw1k
     Aggressiveness=+00000.500000
     MinDuckTime=+00008.000000
     ReFireRate=+00000.500000
     bHasRangedAttack=True
     bMovingRangedAttack=True
	 bLeadTarget=False
     Acquire=chlng1k
     Fear=chlng2k
     Threaten=chlng2k
     Health=180
     HearingThreshold=+00000.000000
     MeleeRange=+00050.000000
     Intelligence=BRAINS_HUMAN
     GroundSpeed=+00240.000000
     AirSpeed=+00240.000000
     AccelRate=+00500.000000
     JumpZ=+00360.000000
     MaxStepHeight=+00025.000000
     HitSound1=injur1k
     HitSound2=injur2k
     Die=death1k
     CombatStyle=+00000.800000
	 CarcassType=KrallCarcass
     DrawType=DT_Mesh
     Mesh=KrallM
     bMeshCurvy=False
	 AmbientSound=amb1kr
     CollisionRadius=+00025.000000
     CollisionHeight=+00046.000000
     Mass=+00140.000000
     Buoyancy=+00140.000000
     RotationRate=(Pitch=3072,Yaw=60000,Roll=0)
	 TransientSoundVolume=+00001.500000
}

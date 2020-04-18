//=============================================================================
// Skaarj.
//=============================================================================
class Skaarj expands ScriptedPawn
	abstract;

#exec AUDIO IMPORT FILE="Sounds\Skaarj\amb1sk.WAV" NAME="amb1sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\syl07.WAV" NAME="syl07sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\syl09.WAV" NAME="syl09sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\syl11.WAV" NAME="syl11sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\syl12.WAV" NAME="syl12sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\syl13.WAV" NAME="syl13sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\syl15.WAV" NAME="syl15sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\spin1a.WAV" NAME="spin1s" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\claw2a.WAV" NAME="claw2s" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\clawhit1a.WAV" NAME="clawhit1s" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\injur5.WAV" NAME="injur1sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\injur6.WAV" NAME="injur2sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\injur7.WAV" NAME="injur3sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\chalnge1a.WAV" NAME="chalnge1s" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\chalnge3a.WAV" NAME="chalnge3s" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\death1a.WAV" NAME="death1sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\death2a.WAV" NAME="death2sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\roam11.WAV" NAME="roam11s" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\lunge1sk.WAV" NAME="lunge1sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\squat1a.WAV" NAME="squat1sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Skaarj\hairflp2a.WAV" NAME="hairflp2sk" GROUP="Skaarj"
#exec AUDIO IMPORT FILE="Sounds\Cow\walknc.WAV" NAME="walkC" GROUP="Cow"
#exec AUDIO IMPORT FILE="Sounds\Gibs\bthump1.WAV" NAME="Thump" GROUP="Gibs"

//-----------------------------------------------------------------------------
// Skaarj variables.

// Attack damage.
var() byte
	LungeDamage,	// Basic damage done by lunge.
	SpinDamage,		// Basic damage done by spin.
	ClawDamage;		// Basic damage done by each claw.

var bool AttackSuccess;
var() bool	bButtonPusher;
var() bool  bFakeDeath;
var(Sounds) sound hitsound3;
var(Sounds) sound hitsound4;
var(Sounds) sound syllable1;
var(Sounds) sound syllable2;
var(Sounds) sound syllable3;
var(Sounds) sound syllable4;
var(Sounds) sound syllable5;
var(Sounds) sound syllable6;
var(Sounds) sound spin;
var(Sounds) sound claw;
var(Sounds) sound slice;
var(Sounds) sound lunge;
var(Sounds) sound hairflip;
var(Sounds) sound Die2;
var(Sounds) sound Footstep;
var(Sounds) sound Footstep2;

var 	name phrase;
var		byte phrasesyllable;
var		float	voicePitch;

function PreBeginPlay()
{
	Super.PreBeginPlay();
	bCanSpeak = true;
	voicePitch = Default.voicePitch + 0.6 * Default.voicePitch * FRand();
	
	if ( CombatStyle == Default.CombatStyle)
		CombatStyle = CombatStyle + 0.3 * FRand() - 0.15;

	if ( bFakeDeath )
	{
		AnimSequence = 'Death2';
		AnimFrame = 0.92;
		SimAnim.X = 9200;
	}
	if ( skill > 2 )
		ProjectileSpeed *= 1.1;
}

function RunStep()
{
	if (FRand() < 0.6)
		PlaySound(FootStep, SLOT_Interact,0.8,,900);
	else
		PlaySound(FootStep2, SLOT_Interact,0.8,,900);
}

function WalkStep()
{
	if (FRand() < 0.6)
		PlaySound(FootStep, SLOT_Interact,0.2,,500);
	else
		PlaySound(FootStep2, SLOT_Interact,0.2,,500);
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

/* PreSetMovement()
*/
function PreSetMovement()
{
	MaxDesiredSpeed = 0.7 + 0.1 * skill;
	bCanJump = true;
	bCanWalk = true;
	bCanSwim = false;
	bCanFly = false;
	MinHitWall = -0.6;
	bCanOpenDoors = true;
	if ( Intelligence > BRAINS_Mammal )
		bCanDoSpecial = true;
	bCanDuck = true;
}

function SetMovementPhysics()
{
	if ( Region.Zone.bWaterZone )
		SetPhysics(PHYS_Swimming);
	else if (Physics != PHYS_Walking)
		SetPhysics(PHYS_Walking); 
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
	local float decision, inflection, pitch;
	
	//if (phrase != '')
	//	SpeakPhrase();
	bIsSpeaking = true;
	if ( FRand() < 0.65)
	{
		inflection = 0.6 + 0.5 * FRand();
		pitch = voicePitch + 0.4 * FRand();
	}
	else
	{
		inflection = 1.3 + 0.5 * FRand();
		pitch = voicePitch + 0.8 * FRand();
	}
	decision = FRand();
	if (TeamLeader != None)	
		TeamLeader.bTeamSpeaking = true;
	if (decision < 0.167)
		PlaySound(Syllable1,SLOT_Talk,inflection,,, pitch);
	else if (decision < 0.333)
		PlaySound(Syllable2,SLOT_Talk,inflection,,, pitch);
	else if (decision < 0.5)
		PlaySound(Syllable3,SLOT_Talk,inflection,,, pitch);
	else if (decision < 0.667)
		PlaySound(Syllable4,SLOT_Talk,inflection,,, pitch);
	else if (decision < 0.833)
		PlaySound(Syllable5,SLOT_Talk,inflection,,, pitch);
	else 
		PlaySound(Syllable6,SLOT_Talk,inflection,,, pitch);

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
	if ( Other.IsA('Skaarj') )
	{
		if ( Other.IsA('SkaarjBerserker') )
			return ATTITUDE_Ignore;
		else
			return ATTITUDE_Friendly;
	}
	else if ( Other.IsA('Pupae') )
		return ATTITUDE_Friendly;
	else if ( Other.IsA('Nali') )
		return ATTITUDE_Hate;
	else if ( Other.IsA('WarLord') || Other.IsA('Queen') )
		return ATTITUDE_Friendly;
	else
		return ATTITUDE_Ignore;
}

function PushButtons()
{
	local float decision, animspeed;

	SetAlertness(-0.7);
	animspeed = 0.4 + 0.6 * FRand(); 
	decision = FRand();
	if (decision < 0.2)
		LoopAnim('Breath2', animspeed, 1.0);
	else if (decision < 0.3)
	{
		SetAlertness(0.2);
		LoopAnim('Breath', animspeed, 1.0);
	}
	else if (decision < 0.4)
		LoopAnim('MButton1', animspeed);
	else if (decision < 0.5)
		LoopAnim('MButton2', animspeed);
	else if (decision < 0.6)
		LoopAnim('MButton3', animspeed);
	else if (decision < 0.7)
		LoopAnim('MButton4', animspeed);
	else if (decision < 0.76)
		LoopAnim('Button1', animspeed);
	else if (decision < 0.82)
		LoopAnim('Button2', animspeed);
	else if (decision < 0.88)
		LoopAnim('Button3', animspeed);
	else if (decision < 0.94)
		LoopAnim('Button4', animspeed);
	else
		LoopAnim('Button5', animspeed);
	return;
}

function PlayCock()
{
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
	if ( bFakeDeath )
		return;
	if ( bButtonPusher )
	{
		PushButtons();
		return;
	}

	animspeed = 0.3 + 0.6 * FRand(); //vary speed
	decision = FRand();
	if (AnimSequence == 'Breath')
	{
		SetAlertness(0.0);
		if (decision < 0.15)
		{
			PlayAnim('gunfix', AnimSpeed, 0.7);
			if ( !bQuiet )
				PlaySound(Roam, SLOT_Talk);			
		}
		else if ( decision < 0.28 )
		{
			if ( IsA('SkaarjWarrior') )
				PlayAnim('Stretch', AnimSpeed);
			else
				PlayAnim('ShldTest', AnimSpeed);
		}
		else
			LoopAnim('Breath2', AnimSpeed);
		return;
	}
	else if ( AnimSequence == 'Breath2' )
	{
		if (decision < 0.2)
		{
			SetAlertness(0.3);
			LoopAnim('Breath', 0.2 + 0.5 * FRand());
		}
		else
			LoopAnim('Breath2', AnimSpeed);
		return;
	}
	else if ( AnimSequence == 'gunfix' )
	{
		SetAlertness(-0.3);
		if (decision < 0.25)
		{
			PlayCock();
			LoopAnim('guncheck', animspeed);
		}
		else if (decision < 0.37)
			PlayAnim('headup', animspeed);
		else
			LoopAnim('gunfix', animspeed);
		return;
 	}
	else if ( AnimSequence == 'Looking' )
	{
		if (decision < 0.7)
		{
			SetAlertness(-0.3);
			LoopAnim('gunfix', animspeed);
		}
		else if (decision < 0.85)
		{
			SetAlertness(0.0);
			PlayAnim('Breath2', AnimSpeed, 0.7);
		}
		else
		{
			SetAlertness(0.5);
			LoopAnim('Looking', AnimSpeed);
		}
		return;
	}
	else if ( AnimSequence == 'Headup' )
	{
		if (decision < 0.1)
		{
			SetAlertness(0.0);
			PlayAnim('Breath2', AnimSpeed, 0.7);
		}
		else
		{
			SetAlertness(0.6);
			LoopAnim('Looking', AnimSpeed);
			if ( !bQuiet )
				PlaySound(Roam, SLOT_Talk);
		}
		return; 	
	}
	else if ( AnimSequence == 'guncheck' )
	{
		SetAlertness(-0.4);
		if (decision < 0.87) 
			LoopAnim('gunfix', AnimSpeed);
		else
		{
			PlayCock();
			LoopAnim('guncheck', AnimSpeed);
		}
		return;
	}
	else
	{
		SetAlertness(-0.3);
		PlayAnim('gunfix', animspeed, 0.6);
		return;
	} 
}

function PlayWaitingAmbush()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	if ( bFakeDeath )
		return;
	if ( bButtonPusher )
	{
		PushButtons();
		return;
	}
	if (FRand() < 0.8)
		LoopAnim('Breath2', 0.3 + 0.6 * FRand());
	else
		LoopAnim('Breath', 0.3 + 0.6 * FRand());
}

function PlayDive()
{
	TweenToSwimming(0.2);
}

function TweenToWaiting(float tweentime)
{
	if ( bFakeDeath )
		return;
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(tweentime);
		return;
	}
	TweenAnim('gunfix', tweentime);
}

function TweenToFighter(float tweentime)
{
	bButtonPusher = false;
	bFakeDeath = false;
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(tweentime);
		return;
	}
	if ( (AnimSequence == 'Death2') && (AnimFrame > 0.8) )
	{
		SetFall();
		GotoState('FallingState', 'RiseUp');
	}
	else
		TweenAnim('Fighter', tweentime);
}

function TweenToRunning(float tweentime)
{
	bButtonPusher = false;
	bFakeDeath = false;
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(tweentime);
		return;
	}
	if ( (AnimSequence == 'Death2') && (AnimFrame > 0.8) )
	{
		SetFall();
		GotoState('FallingState', 'RiseUp');
	}
	else if ( ((AnimSequence != 'Jog') && (AnimSequence != 'JogFire')) || !bAnimLoop )
		TweenAnim('Jog', tweentime);
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

function TweenToPatrolStop(float tweentime)
{
	if ( bFakeDeath )
		return;
	if (Region.Zone.bWaterZone)
	{
		TweenToSwimming(tweentime);
		return;
	}
	TweenAnim('Breath', tweentime);
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
	LoopAnim('Swim', -1.0/WaterSpeed,, 0.5); 
}

function PlayTurning()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	if ( (AnimSequence == 'Death2') && (AnimFrame > 0.8) )
	{
		SetFall();
		GotoState('FallingState', 'RiseUp');
	}
	else
		TweenAnim('Walk', 0.3);
}

function PlayBigDeath(name DamageType)
{
	if ( FRand() < 0.35 )
		PlayAnim('Death',0.7,0.1);
	else
		PlayAnim('Death2',0.7,0.1);
	PlaySound(Die2, SLOT_Talk, 4.5 * TransientSoundVolume);
}

function PlayHeadDeath(name DamageType)
{
	local carcass carc;

	if ( (DamageType == 'Decapitated') || ((Health < -20) && (FRand() < 0.5)) )
	{
		carc = Spawn(class 'CreatureChunks',,, Location + CollisionHeight * vect(0,0,0.8), Rotation + rot(3000,0,16384) );
		if (carc != None)
		{
			carc.Mesh = mesh'SkaarjHead';
			carc.Initfor(self);
			carc.Velocity = Velocity + VSize(Velocity) * VRand();
			carc.Velocity.Z = FMax(carc.Velocity.Z, Velocity.Z);
		}
		PlayAnim('Death5',0.7,0.1);
		if ( IsA('SkaarjWarrior') && (Velocity.Z < 120) )
		{
			Velocity = GroundSpeed * vector(Rotation);
			Velocity.Z = 150;
		}
	}
	else if ( FRand() < 0.5 )
		PlayAnim('Death',0.7,0.1);
	else
		PlayAnim('Death4',0.7,0.1);
	PlaySound(Die, SLOT_Talk, 4.5 * TransientSoundVolume);
}

function PlayLeftDeath(name DamageType)
{
	if ( FRand() < 0.5 )
		PlayAnim('Death',0.7,0.1);
	else
		PlayAnim('Death4',0.7,0.1);
	PlaySound(Die, SLOT_Talk, 4.5 * TransientSoundVolume);
}

function PlayRightDeath(name DamageType)
{
	if ( FRand() < 0.3 )
		PlayAnim('Death3',0.7,0.1);
	else
		PlayAnim('Death4',0.7,0.1);
	PlaySound(Die, SLOT_Talk, 4.5 * TransientSoundVolume);
}

function PlayGutDeath(name DamageType)
{
	PlayAnim('Death3',0.7, 0.1);
	PlaySound(Die, SLOT_Talk, 4.5 * TransientSoundVolume);
}

function PlayTakeHitSound(int Damage, name damageType, int Mult)
{
	local float decision;

	if ( Level.TimeSeconds - LastPainSound < 0.25 )
		return;
	LastPainSound = Level.TimeSeconds;

	decision = FRand(); //FIXME - modify based on damage
	if (decision < 0.25)
		PlaySound(HitSound1, SLOT_Pain, 2.0 * Mult);
	else if (decision < 0.5)
		PlaySound(HitSound2, SLOT_Pain, 2.0 * Mult);
	else if (decision < 0.75)
		PlaySound(HitSound3, SLOT_Pain, 2.0 * Mult);
	else
		PlaySound(HitSound4, SLOT_Pain, 2.0 * Mult); 
}

function TweenToFalling()
{
	if ( FRand() < 0.5 )
		TweenAnim('Jog', 0.2);
	else
		PlayAnim('Jump',0.7,0.1);
}

function PlayInAir()
{
	if ( AnimSequence == 'Jog' )
		PlayAnim('Jog', 0.4);
	else if ( AnimSequence == 'JogFire' )
		PlayAnim('JogFire', 0.4);
	else
		TweenAnim('InAir',0.4);
}

function PlayOutOfWater()
{
	TweenAnim('Landed', 0.8);
}

function PlayLanded(float impactVel)
{
	if (impactVel > 1.7 * JumpZ)
		TweenAnim('Landed',0.1);
	else
		TweenAnim('Land', 0.1);
}
	
function PlayTakeHit(float tweentime, vector HitLoc, int damage)
{
	if ( (Velocity.Z > 120) && (Health < 0.4 * Default.Health) && (FRand() < 0.33) )
		PlayAnim('Death2',0.7);
	else if ( (AnimSequence != 'Spin') && (AnimSequence != 'Lunge') && (AnimSequence != 'Death2') )
		Super.PlayTakeHit(tweentime, HitLoc, damage);
}

function SpinDamageTarget()
{
	if (MeleeDamageTarget(SpinDamage, (SpinDamage * 1000 * Normal(Target.Location - Location))) )
		PlaySound(slice, SLOT_Interact);		
}

function ClawDamageTarget()
{
	if ( MeleeDamageTarget(ClawDamage, (ClawDamage * 900 * Normal(Target.Location - Location))) )
		PlaySound(slice, SLOT_Interact);			
}

	
function PlayMeleeAttack()
{
	local int hitdamage;
	local float TargetDist, decision;

	decision = FRand();
	if (AnimSequence == 'Spin')
		decision += 0.2;
	else if (AnimSequence == 'Claw')
		decision -= 0.2;
	AttackSuccess = false;
	//log("Start Melee Attack");
	if ( Region.Zone.bWaterZone || (decision < 0.5) )
	{
		Acceleration = AccelRate * Normal(Target.Location - Location);
 		PlayAnim('Spin');
		PlaySound(Spin, SLOT_Interact);	 		
 	}
	else
	{
 		PlayAnim('Claw'); 
 		PlaySound(Claw, SLOT_Interact);
 	}	
 }

function PlayRangedAttack()
{
	PlayAnim('Firing', 1.5); 
}

state TakeHit 
{
ignores seeplayer, hearnoise, bump, hitwall;

	function Landed(vector HitNormal)
	{
		local float landVol;

		if ( AnimSequence == 'Death2' )
		{
			landVol = 0.75 + Velocity.Z * 0.004;
			LandVol = Mass * landVol * landVol * 0.01;
			PlaySound(sound'thump', SLOT_Interact, landVol);
			GotoState('FallingState', 'RiseUp');
		}
		else
			Super.Landed(HitNormal);
	}

	function PlayTakeHit(float tweentime, vector HitLoc, int damage)
	{
		if ( AnimSequence != 'Death2' )
			Global.PlayTakeHit(tweentime, HitLoc, damage);
	}

	function BeginState()
	{
		Super.BeginState();
		If ( AnimSequence == 'Death2' )
			GotoState('FallingState');
	}	
}

state FallingState
{
ignores Bump, Hitwall, HearNoise, WarnTarget;

	function Landed(vector HitNormal)
	{
		local float landVol;

		if ( AnimSequence == 'Death2' )
		{
			landVol = 0.75 + Velocity.Z * 0.004;
			LandVol = Mass * landVol * landVol * 0.01;
			PlaySound(sound'Thump', SLOT_Interact, landVol);
			GotoState('FallingState', 'RiseUp');
		}
		else if ( (AnimSequence == 'LeftDodge') || (AnimSequence == 'RightDodge') )
		{
			landVol = Velocity.Z/JumpZ;
			landVol = 0.008 * Mass * landVol * landVol;
			if ( !FootRegion.Zone.bWaterZone )
				PlaySound(Land, SLOT_Interact, FMin(20, landVol));
			GotoState('FallingState', 'FinishDodge');
		}
		else
			Super.Landed(HitNormal);
	}

	function PlayTakeHit(float tweentime, vector HitLoc, int damage)
	{
		if ( AnimSequence != 'Death2' )
			Global.PlayTakeHit(tweentime, HitLoc, damage);
	}

LongFall:
	if ( AnimSequence == 'Death2' )
	{
		Sleep(1.5);
		Goto('RiseUp');
	}
	if ( bCanFly )
	{
		SetPhysics(PHYS_Flying);
		Goto('Done');
	}
	Sleep(0.7);
	TweenToFighter(0.2);
	if ( bHasRangedAttack && (Enemy != None) )
	{
		TurnToward(Enemy);
		FinishAnim();
		if ( CanFireAtEnemy() )
		{
			PlayRangedAttack();
			FinishAnim();
		}
		PlayChallenge();
		FinishAnim();
	}
	TweenToFalling();
	if ( Velocity.Z > -150 ) //stuck
	{
		SetPhysics(PHYS_Falling);
		if ( Enemy != None )
			Velocity = groundspeed * normal(Enemy.Location - Location);
		else
			Velocity = groundspeed * VRand();

		Velocity.Z = FMax(JumpZ, 250);
	}
	Goto('LongFall');
RiseUp:
	FinishAnim();
	bCanDuck = false;
	DesiredRotation = Rotation;
	Acceleration = vect(0,0,0);
	if ( !bFakeDeath )
		Sleep(1.0 + 6 * FRand());
	PlayAnim('GetUp', 0.7);
FinishDodge:
	FinishAnim();
	bCanDuck = true;
	Goto('Done');
}

state Hunting
{
ignores EnemyNotVisible; 

	function BeginState()
	{
		bCanSwim = true;
		Super.BeginState();
	}

	function EndState()
	{
		if ( !Region.Zone.bWaterZone )
			bCanSwim = false;
		Super.EndState();
	}
}

state RangedAttack
{
ignores SeePlayer, HearNoise;
	
	function Bump (Actor Other)
	{
		if ( AttackSuccess || (AnimSequence != 'Lunge') )
		{
			Disable('Bump');
			return;
		}
		else		
			LungeDamageTarget();

		if (!AttackSuccess && Pawn(Other) != None) //always add momentum
			Pawn(Other).AddVelocity((60000.0 * (Normal(Other.Location - Location)))/Other.Mass);
	}
	
	function LungeDamageTarget()
	{
		If (MeleeDamageTarget(LungeDamage, (LungeDamage * 2000 * Normal(Target.Location - Location))))
		{
			AttackSuccess = true;
			disable('Bump');
			PlaySound(Slice, SLOT_Interact);
		}
	}
	
	function PlayRangedAttack()
	{
		local float dist;
		dist = VSize(Target.Location - Location + vect(0,0,1) * (CollisionHeight - Target.CollisionHeight));
		if ( (FRand() < 0.7) && (dist < 180 + CollisionRadius + Target.CollisionRadius) && (Region.Zone.bWaterZone || !Target.Region.Zone.bWaterZone) )
		{
			PlaySound(Lunge, SLOT_Interact);
	 		Velocity = 500 * (Target.Location - Location)/dist; //instant acceleration in that direction 
	 		Velocity.Z += 1.5 * dist;
	 		if (Physics != PHYS_Swimming)
	 			SetPhysics(PHYS_Falling);
	 		Enable('Bump');
	 		PlayAnim('Lunge');
	 	}
		else
		{
			Disable('Bump');
			PlayAnim('Firing', 1.5); 
		}
	}
}

defaultproperties
{
     LungeDamage=30
     SpinDamage=20
     ClawDamage=15
     RangedProjectile=SkaarjProjectile
     syllable1=syl07sk
     syllable2=syl09sk
     syllable3=syl11sk
     syllable4=syl12sk
     syllable5=syl13sk
     syllable6=syl15sk
     Spin=spin1s
     Claw=claw2s
     slice=clawhit1s
     Lunge=lunge1sk
     HairFlip=hairflp2sk
     Footstep=walkC
     Footstep2=walkC
     CarcassType=SkaarjCarcass
     Aggressiveness=+00000.500000
     ReFireRate=+00000.500000
     voicePitch=+00000.500000
     bHasRangedAttack=True
	 bMovingRangedAttack=True
	 Acquire=chalnge1s
     Fear=breath2s
     Roam=roam11s
     Threaten=chalnge3s
     Health=210
     UnderWaterTime=-00001.000000
     bCanStrafe=True
     MeleeRange=+00040.000000
     Intelligence=BRAINS_HUMAN
     GroundSpeed=+00440.000000
     AccelRate=+01200.000000
     HitSound1=injur1sk
     HitSound2=injur2sk
     HitSound3=injur3sk
     HitSound4=injur3sk
     Die=death1sk
	 Die2=death2sk
     CombatStyle=+00000.650000
     DrawType=DT_Mesh
     bMeshCurvy=False
	 AmbientSound=amb1sk
     CollisionRadius=+00035.000000
     CollisionHeight=+00046.000000
     Mass=+00150.000000
     Buoyancy=+00150.000000
     RotationRate=(Pitch=3072,Yaw=60000,Roll=2048)
	 TransientSoundVolume=+00003.000000
}

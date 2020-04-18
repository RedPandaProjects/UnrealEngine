//=============================================================================
// Titan.
//=============================================================================
class Titan expands ScriptedPawn;

#exec MESH IMPORT MESH=Titan1 ANIVFILE=MODELS\titan_a.3D DATAFILE=MODELS\titand.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=Titan1 X=20 Y=-130 Z=-65 YAW=64 ROLL=-64

#exec MESH SEQUENCE MESH=titan1 SEQ=All       STARTFRAME=0   NUMFRAMES=300
#exec MESH SEQUENCE MESH=titan1 SEQ=TBrea001  STARTFRAME=0   NUMFRAMES=6   RATE=6
#exec MESH SEQUENCE MESH=titan1 SEQ=TChest    STARTFRAME=6   NUMFRAMES=28  RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=titan1 SEQ=TDeat001  STARTFRAME=34  NUMFRAMES=29  RATE=15
#exec MESH SEQUENCE MESH=titan1 SEQ=TDeat002  STARTFRAME=63  NUMFRAMES=18  RATE=15
#exec MESH SEQUENCE MESH=titan1 SEQ=TDeat003  STARTFRAME=81  NUMFRAMES=18  RATE=15
#exec MESH SEQUENCE MESH=titan1 SEQ=TFigh001  STARTFRAME=99  NUMFRAMES=8   RATE=6
#exec MESH SEQUENCE MESH=titan1 SEQ=TFist     STARTFRAME=107 NUMFRAMES=28  RATE=15
#exec MESH SEQUENCE MESH=titan1 SEQ=TGetUp    STARTFRAME=135 NUMFRAMES=35  RATE=15
#exec MESH SEQUENCE MESH=titan1 SEQ=TLeftUp   STARTFRAME=170 NUMFRAMES=1
#exec MESH SEQUENCE MESH=titan1 SEQ=TPunc001  STARTFRAME=171 NUMFRAMES=13  RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=titan1 SEQ=TRightUp  STARTFRAME=184 NUMFRAMES=1
#exec MESH SEQUENCE MESH=titan1 SEQ=TSit      STARTFRAME=185 NUMFRAMES=1
#exec MESH SEQUENCE MESH=titan1 SEQ=TSlap001  STARTFRAME=186 NUMFRAMES=15  RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=titan1 SEQ=TSnif001  STARTFRAME=201 NUMFRAMES=18  RATE=15
#exec MESH SEQUENCE MESH=titan1 SEQ=TStom001  STARTFRAME=219 NUMFRAMES=18  RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=titan1 SEQ=TThro001  STARTFRAME=237 NUMFRAMES=18  RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=titan1 SEQ=TWalk001  STARTFRAME=255 NUMFRAMES=30  RATE=15
#exec MESH SEQUENCE MESH=titan1 SEQ=TShuffle  STARTFRAME=285 NUMFRAMES=15  RATE=15

#exec TEXTURE IMPORT NAME=Jtitan1 FILE=MODELS\titan.PCX GROUP=Skins  
#exec MESHMAP SCALE MESHMAP=titan1 X=0.3 Y=0.3 Z=0.6
#exec MESHMAP SETTEXTURE MESHMAP=titan1 NUM=0 TEXTURE=Jtitan1

#exec MESH NOTIFY MESH=titan1 SEQ=TWalk001 TIME=0.25 FUNCTION=FootStep
#exec MESH NOTIFY MESH=titan1 SEQ=TWalk001 TIME=0.75 FUNCTION=FootStep
#exec MESH NOTIFY MESH=titan1 SEQ=TWalk001 TIME=0.4  FUNCTION=StartMoving
#exec MESH NOTIFY MESH=titan1 SEQ=TWalk001 TIME=0.9  FUNCTION=StartMoving
#exec MESH NOTIFY MESH=titan1 SEQ=TStom001 TIME=0.4  FUNCTION=Stomp
#exec MESH NOTIFY MESH=titan1 SEQ=TSlap001 TIME=0.46 FUNCTION=SlapDamageTarget
#exec MESH NOTIFY MESH=titan1 SEQ=TPunc001 TIME=0.61 FUNCTION=PunchDamageTarget
#exec MESH NOTIFY MESH=titan1 SEQ=TThro001 TIME=0.75 FUNCTION=SpawnRock
#exec MESH NOTIFY MESH=titan1 SEQ=TDeat001 TIME=0.86 FUNCTION=LandThump
#exec MESH NOTIFY MESH=titan1 SEQ=TDeat002 TIME=0.61 FUNCTION=LandThump
#exec MESH NOTIFY MESH=titan1 SEQ=TDeat003 TIME=0.69 FUNCTION=LandThump

#exec AUDIO IMPORT FILE="Sounds\Titan\throw1a.WAV" NAME="Throw1t" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\step1a.WAV" NAME="step1t" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\stomp4a.WAV" NAME="stomp4t" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\punch1Tt.WAV" NAME="punch1Ti" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\Swing1.WAV" NAME="Swing1t" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\yell1a.WAV" NAME="yell1t" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\yell2a.WAV" NAME="yell2t" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\yell3a.WAV" NAME="yell3t" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\death1a.WAV" NAME="death1t" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\injur1a.WAV" NAME="injur1t" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\injur2a.WAV" NAME="injur2t" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\amb1Ti.WAV" NAME="amb1Ti" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\sniff1Ti.WAV" NAME="sniff1Ti" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\fall1Ti.WAV" NAME="fall1Ti" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\slaphit1Ti.WAV" NAME="slaphit1Ti" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\roam1Ti.WAV" NAME="roam1Ti" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Titan\chestB2Ti.WAV" NAME="chestB2Ti" GROUP="Titan"

//FIXME - use fall1ti for titancarcass landed

//TITAN variables;
var() byte SlapDamage,
	PunchDamage;
	
var bool bStomp;
var bool bLavaTitan;
var bool bEndFootStep;
var float realSpeed;
var() name StompEvent;
var() name StepEvent;
var(Sounds) sound Step;
var(Sounds) sound StompSound;
var(Sounds) sound slap;
var(Sounds) sound swing;
var(Sounds) sound throw;
var(Sounds) sound chest;

function PlayAcquisitionSound()
{
	if (Acquire != None) 
	{
		PlaySound(Acquire, SLOT_Talk,, true); 
		PlaySound(Acquire, SLOT_Misc,, true);
	} 
}

function PlayFearSound()
{
	if (Fear != None)
	{
		PlaySound(Fear, SLOT_Talk,, true); 
		PlaySound(Fear, SLOT_Misc,, true); 
	}
}

function PlayRoamingSound()
{
	if ( (Threaten != None) && (FRand() < 0.3) )
	{
		PlaySound(Threaten, SLOT_Talk,, true);
		PlaySound(Threaten, SLOT_Misc,, true);
		return;
	}
	if ( FRand() < 0.5 )
	{
		PlaySound(Sound'roam1Ti', SLOT_Talk,, true);
		PlaySound(Sound'roam1Ti', SLOT_Misc,, true);
		return;
	}
	if (Roam != None)
	{
		PlaySound(Roam, SLOT_Talk,, true);
		PlaySound(Roam, SLOT_Misc,, true);
	}
}

function PlayThreateningSound()
{
	if (Threaten == None) return;
	if (FRand() < 0.5)
	{
		PlaySound(Threaten, SLOT_Talk,, true);
		PlaySound(Threaten, SLOT_Misc,, true);
	}
	else
	{
		PlaySound(Fear, SLOT_Talk,, true);
		PlaySound(Fear, SLOT_Misc,, true);
	}
}

singular event BaseChange()
{
	local float decorMass;
	if (Pawn(Base) != None)
	{
		Base.TakeDamage( 1000, Self,Location,0.5 * Velocity , 'stomped');
		JumpOffPawn();
	}
	else if ( (Decoration(Base) != None) && (Velocity.Z < -400) )
	{
		decorMass = FMax(Decoration(Base).Mass, 1);
		Base.TakeDamage(1000, Self, Location, 0.5 * Velocity, 'stomped');
	}
}

function eAttitude AttitudeToCreature(Pawn Other)
{
	if ( Other.IsA('Titan') )
		return ATTITUDE_Friendly;
	else if ( Other.IsA('ScriptedPawn') )
		return ATTITUDE_Hate;
	else
		return ATTITUDE_Ignore;
}

function ThrowOther(Pawn Other)
{
	local float dist, shake;
	local PlayerPawn aPlayer;
	local vector Momentum;

	if ( Other.mass > 500 )
		return;

	aPlayer = PlayerPawn(Other);
	if ( aPlayer == None )
	{	
		if ( !bStomp || (Other.Physics != PHYS_Walking) )
			return;
		dist = VSize(Location - Other.Location);
		if (dist > 500)
			return;
	}
	else
	{
		dist = VSize(Location - Other.Location);
		shake = FMax(500, 1500 - dist);
		if ( dist > 1500 )
			return;
		aPlayer.ShakeView( FMax(0, 0.35 - dist/20000), shake, 0.015 * shake);
		if ( Other.Physics != PHYS_Walking )
			return;
	}

	Momentum = -0.5 * Other.Velocity + 100 * VRand();
	Momentum.Z =  7000000.0/((0.4 * dist + 350) * Other.Mass);
	if (bStomp)
		Momentum.Z *= 5.0;		
	Other.AddVelocity(Momentum);
}

function FootStep()
{
	local actor A;
	local pawn Thrown;
	//slightly throw player if nearby ,& play footstep sound
	bStomp = false;
	bEndFootstep = false;
	if (StepEvent != '')
		foreach AllActors( class 'Actor', A, StepEvent )
			A.Trigger( Self, Instigator );

	Thrown = Level.PawnList;
	While ( Thrown != None )
	{
		ThrowOther(Thrown);
		Thrown = Thrown.nextPawn;
	}
	
	realSpeed = DesiredSpeed; //fixme - don't stop if very low friction
	DesiredSpeed = 0.0;
	PlaySound(Step, SLOT_Interact);
}

function StartMoving()
{
	DesiredSpeed = realSpeed;
}

function Stomp()
{
	local actor A;
	local pawn Thrown;

	if (StompEvent != '')
		foreach AllActors( class 'Actor', A, StompEvent )
			A.Trigger( Self, Instigator );
			
	//throw all nearby creatures, and play sound
	bStomp = true;
	Thrown = Level.PawnList;
	While ( Thrown != None )
	{
		ThrowOther(Thrown);
		Thrown = Thrown.nextPawn;
	}
	PlaySound(Step, SLOT_Interact, 24);
}

function PlayWaiting()
{
	local float decision;
	local float animspeed;

	decision = FRand();
	animspeed = 0.2 + 0.5 * FRand();

	if (bEndFootStep)
		FootStep();
				
	if ( (AnimSequence == 'TBrea001') && (decision < 0.17) )
	{
		SetAlertness(0.0);
		if (decision < 0.1)
		{
			PlaySound(sound'sniff1Ti', SLOT_Talk);
			LoopAnim('TSnif001', animspeed);
		}
		else if (decision < 0.17)
			LoopAnim('TFist', animspeed);
	}
	else
	{
		SetAlertness(0.3);
		LoopAnim('TBrea001', animspeed);
	}
}
	
//PlayPatrolStop(), and PlayWaitingAmbush() all use PlayWaiting(); 			
function PlayPatrolStop()
{
	if (bEndFootStep)
		FootStep();
	DesiredSpeed = 0.0;
	PlayWaiting();
}

function PlayWaitingAmbush()
{
	if (bEndFootStep)
		FootStep();
	DesiredSpeed = 0.0;
	PlayWaiting();
}

function PlayChallenge()
{
	local float decision;

	if (bEndFootStep)
		FootStep();
	DesiredSpeed = 0.0;
	decision = FRand();

	if ( decision < 0.2 )
		PlayAnim('TStom001', 1.0, 0.2);
	else if ( decision < 0.4 )
		PlayAnim('TFist', 1.0, 0.2);
	else if ( decision < 0.64 )
		PlayAnim('TFigh001', 1.0, 0.2);
	else if ( decision < 0.75 )
	{
		PlaySound(Chest, SLOT_Interact);
		PlaySound(Chest, SLOT_Misc);
		PlayAnim('TChest', 1.0, 0.2);
	}
	else
		PlayAnim('TShuffle',1.0, 0.2);
}

function TweenToFighter(float tweentime)
{
	bEndFootStep = ( (AnimSequence == 'TWalk001') && (AnimFrame > 0.1) );   
	TweenAnim('TFigh001', tweentime);
}

function TweenToRunning(float tweentime)
{
	if ( (AnimSequence != 'TWalk001') || !bAnimLoop )
		TweenAnim('TWalk001', tweentime);
}

function TweenToWalking(float tweentime)
{
	TweenAnim('TWalk001', tweentime);
}

function TweenToWaiting(float tweentime)
{
	TweenAnim('TBrea001', tweentime);
}

function TweenToPatrolStop(float tweentime)
{
	TweenAnim('TBrea001', tweentime);
}

function PlayRunning()
{
	LoopAnim('TWalk001', -1.0/GroundSpeed,, 0.8);
}

function PlayWalking()
{
	LoopAnim('TWalk001', -1.0/GroundSpeed,, 0.8);
	if (FRand() < 0.4)
		PlayRoamingSound();
}

function PlayThreatening()
{
	local float decision, animspeed;

	decision = FRand();
	animspeed = 0.4 + 0.6 * FRand();

	if ( decision < 0.5 )
		PlayAnim('TBrea001', animspeed, 0.4);
	else if ( decision < 0.7 )
	{
		PlaySound(StompSound, SLOT_Talk);		
		PlaySound(StompSound, SLOT_Misc);		
		PlayAnim('TStom001', animspeed, 0.4);
	}
	else
	{
		PlayThreateningSound();
		if ( decision < 0.9 )
			PlayAnim('TFist', animspeed, 0.4);
		else
			TweenAnim('TFigh001', 0.4);
	}
}

function PlayTurning()
{
	if (bEndFootStep)
		FootStep();
	DesiredSpeed = 0.0;
	LoopAnim('TShuffle',, 0.15);
}

function PlayDying(name DamageType, vector HitLocation)
{
	local float decision;
	Decision = FRand();
	if (Decision < 0.4)
		PlayAnim('TDeat001', 0.7, 0.15);
	else if (Decision < 0.7)
		PlayAnim('TDeat002', 0.7, 0.15);
	else
		PlayAnim('TDeat003', 0.7, 0.15);
	
	PlaySound(Die, SLOT_Talk);	
	PlaySound(Die, SLOT_Misc);	
}

function PlayTakeHit(float tweentime, vector HitLoc, int Damage)
{
	local float decision;
	Decision = FRand();
	if (Decision < 0.4)
		TweenAnim('TDeat001', tweentime);
	else if (Decision < 0.7)
		TweenAnim('TDeat002', tweentime);
	else
		TweenAnim('TDeat003', tweentime);
}

function SpawnRock()
{
	local Projectile Proj;
	local vector X,Y,Z, projStart;
	GetAxes(Rotation,X,Y,Z);
	
	MakeNoise(1.0);
	if (FRand() < 0.4)
	{
		projStart = Location + CollisionRadius * X + 0.4 * CollisionHeight * Z;
		Proj = spawn(class 'Boulder1' ,self,'',projStart,AdjustAim(1000, projStart, 400, false, true));
		if( Proj != None )
			Proj.SetPhysics(PHYS_Projectile);
		return;
	}
	
	projStart = Location + CollisionRadius * X + 0.4 * CollisionHeight * Z;
	Proj = spawn(class 'BigRock' ,self,'',projStart,AdjustAim(1000, projStart, 400, false, true));
	if( Proj != None )
		Proj.SetPhysics(PHYS_Projectile);

	projStart = Location + CollisionRadius * X -  40 * Y + 0.4 * CollisionHeight * Z;
	Proj = spawn(class 'BigRock' ,self,'',projStart,AdjustAim(1000, projStart, 400, true, true));
	if( Proj != None )
		Proj.SetPhysics(PHYS_Projectile);

	if (FRand() < 0.2 * skill)
	{
		projStart = Location + CollisionRadius * X + 40 * Y + 0.4 * CollisionHeight * Z;
		Proj = spawn(class 'BigRock' ,self,'',projStart,AdjustAim(1000, projStart, 2000, false, true));
		if( Proj != None )
			Proj.SetPhysics(PHYS_Projectile);
	}
}

function PlayVictoryDance()
{
	if (bEndFootStep)
		FootStep();
	DesiredSpeed = 0.0;
	PlayAnim('TStom001', 0.6, 0.2); //gib the enemy here!
	PlaySound(StompSound, SLOT_Talk);		
	PlaySound(StompSound, SLOT_Misc);		
}

function PunchDamageTarget()
{
	if ( MeleeDamageTarget(PunchDamage, (70000.0 * (Normal(Target.Location - Location)))) )
	{
		PlaySound(Slap, SLOT_Interact);
		PlaySound(Slap, SLOT_Misc);
	}
}

function SlapDamageTarget()
{
	local vector X,Y,Z;
	GetAxes(Rotation,X,Y,Z);
	
	if ( MeleeDamageTarget(SlapDamage, (70000.0 * ( Y + vect(0,0,1)))) )
	{
		PlaySound(Slap, SLOT_Interact);
		PlaySound(Slap, SLOT_Misc);
	}
}

//Titan doesn't need to face as directly
function bool NeedToTurn(vector targ)
{
	local int YawErr;

	DesiredRotation = rotator(targ - location);
	DesiredRotation.Yaw = DesiredRotation.Yaw & 65535;
	YawErr = (DesiredRotation.Yaw - (Rotation.Yaw & 65535)) & 65535;
	if ( (YawErr < 8000) || (YawErr > 57535) )
		return false;

	return true;
}
	
function PlayMeleeAttack()
{
	if (bEndFootStep)
		FootStep();
	if (FRand() < 0.45)
	{
		PlaySound(sound'Punch1Ti', SLOT_Interact);			
		PlaySound(sound'Punch1Ti', SLOT_Misc);			
  		PlayAnim('TPunc001');
	}
	else
	{ 
		PlaySound(swing, SLOT_Interact);			
		PlaySound(swing, SLOT_Misc);			
		PlayAnim('TSlap001'); 
	}
}

function PlayRangedAttack()
{
	////log("Play ranged attack");
	if ( bEndFootStep )
		FootStep();
	if ( (AnimSequence == 'TStom001') || (FRand() < 0.7) )
	{
		PlaySound(Throw, SLOT_Interact);
		PlayAnim('TThro001');
	}
	else
	{
		PlayAnim('TStom001'); 
		PlaySound(StompSound, SLOT_Talk);			
		PlaySound(StompSound, SLOT_Misc);			
	}
}

function ZoneChange(ZoneInfo newZone)
{
	if ( newZone.bPainZone && (newZone.DamageType == 'burned') )
		GotoState('LavaDeath');
	else
		Super.ZoneChange(newZone);
}

state Sitting
{
	ignores SeePlayer, HearNoise, Bump, TakeDamage;

	function Trigger( actor Other, pawn EventInstigator )
	{
		if ( EventInstigator.bIsPlayer )
		{
			AttitudeToPlayer = ATTITUDE_Hate;
			Enemy = EventInstigator;
			GotoState('Sitting', 'GetUp');
		}
		Disable('Trigger');
	}
	
	function BeginState()
	{
		bProjTarget = false;
	}

GetUp:
	bProjTarget = true;
	PlayAnim('TGetUp');
	FinishAnim();
	SetCollisionSize(0, Default.CollisionHeight);
	SetPhysics(PHYS_Walking);
	DesiredSpeed = 1.0;
	Acceleration = vector(Rotation) * AccelRate;
	PlayAnim('TWalk001');
	FinishAnim();
	SetCollisionSize(Default.CollisionRadius, Default.CollisionHeight);
	GotoState('Attacking');
	
Begin:
	TweenAnim('TSit', 0.05);
	SetPhysics(PHYS_None);
}


state WalkOut
{
	ignores SeePlayer, HearNoise, Bump, TakeDamage;

	function Trigger( actor Other, pawn EventInstigator )
	{
		if ( EventInstigator.bIsPlayer )
		{
			AttitudeToPlayer = ATTITUDE_Hate;
			Enemy = EventInstigator;
			GotoState('WalkOut', 'Walk');
		}
		Disable('Trigger');
	}
	
	function BeginState()
	{
		bProjTarget = false;
	}

Walk:
	bProjTarget = true;
	SetPhysics(PHYS_Walking);
	DesiredSpeed = 1.0;
	Acceleration = vector(Rotation) * AccelRate;
	PlayAnim('TWalk001');
	FinishAnim();
	PlayAnim('TWalk001');
	FinishAnim();
	GotoState('Attacking');
	
Begin:
	TweenAnim('TWalk001', 0.05);
	SetPhysics(PHYS_None);
}

state LavaDeath
{
	ignores SeePlayer, HearNoise, Bump, TakeDamage;
	
Begin:
	ReducedDamageType = 'Burned';
	Acceleration = vect(0,0,0);
	PlaySound(Chest, SLOT_Interact);
	PlayAnim('TChest');
	FinishAnim();
	PlayAnim('TDeat002');
	FinishAnim();
	bLavaTitan = true;
	TweenAnim('TDeat001', 2.0);
	GotoState('Attacking');
}

defaultproperties
{
     SlapDamage=85
     PunchDamage=80
     Step=step1t
     StompSound=stomp4t
     Slap=SlapHit1Ti
     swing=Swing1t
	 throw=Throw1t
	 chest=ChestB2Ti
     CarcassType=CreatureCarcass
     Aggressiveness=+00008.000000
     ReFireRate=+00000.600000
     WalkingSpeed=+00000.750000
     bHasRangedAttack=True
	 bIsBoss=True
     Acquire=yell1t
     Fear=yell2t
     Roam=yell3t
     Threaten=yell2t
	 Land=fall1Ti
     Health=1800
     ReducedDamageType=exploded
     ReducedDamagePct=+00000.300000
     Visibility=255
     MeleeRange=+00140.000000
     Intelligence=BRAINS_REPTILE
     GroundSpeed=+00400.000000
     AirSpeed=+00400.000000
     AccelRate=+01000.000000
     JumpZ=-00001.000000
     MaxStepHeight=+00025.000000
     HitSound1=injur1t
     HitSound2=injur2t
     Land=stomp4t
     Die=death1t
     CombatStyle=+00000.850000
     DrawType=DT_Mesh
     Mesh=Titan1
	 AmbientSound=Amb1Ti
     CollisionRadius=+00115.000000
     CollisionHeight=+00110.000000
     Mass=+02000.000000
     RotationRate=(Pitch=0,Yaw=30000,Roll=0)
     SoundRadius=32
	 SoundVolume=250
	 TransientSoundVolume=+00020.000000
}

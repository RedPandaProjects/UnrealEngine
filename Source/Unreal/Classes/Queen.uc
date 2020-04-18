//=============================================================================
// Queen.
//=============================================================================
class Queen expands ScriptedPawn;

#exec MESH IMPORT MESH=SkQueen ANIVFILE=MODELS\queen_a.3D DATAFILE=MODELS\queen_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SkQueen X=0 Y=-130 Z=80 YAW=64 ROLL=-64

#exec MESH SEQUENCE MESH=Skqueen SEQ=All		STARTFRAME=0	NUMFRAMES=171
#exec MESH SEQUENCE MESH=Skqueen SEQ=ThreeHit	STARTFRAME=0	NUMFRAMES=18	RATE=15  Group=Attack 
#exec MESH SEQUENCE MESH=Skqueen SEQ=Claw		STARTFRAME=18   NUMFRAMES=10	RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=Skqueen SEQ=Gouge		STARTFRAME=28   NUMFRAMES=13	RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=Skqueen SEQ=Jump		STARTFRAME=41   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Skqueen SEQ=Land		STARTFRAME=42   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Skqueen SEQ=Meditate	STARTFRAME=43   NUMFRAMES=8		RATE=6
#exec MESH SEQUENCE MESH=Skqueen SEQ=OutCold	STARTFRAME=51   NUMFRAMES=21	RATE=15
#exec MESH SEQUENCE MESH=Skqueen SEQ=TakeHit	STARTFRAME=52   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Skqueen SEQ=Run		STARTFRAME=72   NUMFRAMES=10	RATE=17
#exec MESH SEQUENCE MESH=Skqueen SEQ=Scream		STARTFRAME=82   NUMFRAMES=23	RATE=15
#exec MESH SEQUENCE MESH=Skqueen SEQ=Fighter	STARTFRAME=105  NUMFRAMES=1
#exec MESH SEQUENCE MESH=Skqueen SEQ=Shoot1		STARTFRAME=105  NUMFRAMES=23	RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=Skqueen SEQ=Stab		STARTFRAME=128  NUMFRAMES=8		RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=Skqueen SEQ=Walk		STARTFRAME=136  NUMFRAMES=15	RATE=17
#exec MESH SEQUENCE MESH=Skqueen SEQ=Shield		STARTFRAME=151  NUMFRAMES=20	RATE=25


#exec TEXTURE IMPORT NAME=JQueen1 FILE=MODELS\queen.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=Skqueen X=0.22 Y=0.22 Z=0.44
#exec MESHMAP SETTEXTURE MESHMAP=Skqueen NUM=1 TEXTURE=Jqueen1 

#exec MESH NOTIFY MESH=Skqueen SEQ=Shoot1 TIME=0.167 FUNCTION=SpawnShot
#exec MESH NOTIFY MESH=Skqueen SEQ=Shoot1 TIME=0.255 FUNCTION=SpawnShot
#exec MESH NOTIFY MESH=Skqueen SEQ=Shoot1 TIME=0.344 FUNCTION=SpawnShot
#exec MESH NOTIFY MESH=Skqueen SEQ=Shoot1 TIME=0.433 FUNCTION=SpawnShot
#exec MESH NOTIFY MESH=Skqueen SEQ=OutCold TIME=0.60 FUNCTION=LandThump
#exec MESH NOTIFY MESH=Skqueen SEQ=Claw   TIME=0.5   FUNCTION=ClawDamageTarget
#exec MESH NOTIFY MESH=Skqueen SEQ=Gouge  TIME=0.4   FUNCTION=ClawDamageTarget
#exec MESH NOTIFY MESH=Skqueen SEQ=Stab   TIME=0.33  FUNCTION=StabDamageTarget
#exec MESH NOTIFY MESH=Skqueen SEQ=Walk   TIME=0.26  FUNCTION=FootStep
#exec MESH NOTIFY MESH=Skqueen SEQ=Walk   TIME=0.76  FUNCTION=FootStep
#exec MESH NOTIFY MESH=Skqueen SEQ=Run    TIME=0.25  FUNCTION=FootStep
#exec MESH NOTIFY MESH=Skqueen SEQ=Run    TIME=0.75  FUNCTION=FootStep
#exec MESH NOTIFY MESH=Skqueen SEQ=Shield TIME=0.75  FUNCTION=SpawnShield

#exec AUDIO IMPORT FILE="Sounds\Queen\claw1Q.WAV" NAME="claw1Q" GROUP="Queen"
#exec AUDIO IMPORT FILE="Sounds\Queen\shoot1Q.WAV" NAME="shoot1Q" GROUP="Queen"
#exec AUDIO IMPORT FILE="Sounds\Queen\yell1Q.WAV" NAME="yell1Q" GROUP="Queen"
#exec AUDIO IMPORT FILE="Sounds\Queen\yell2Q.WAV" NAME="yell2Q" GROUP="Queen"
#exec AUDIO IMPORT FILE="Sounds\Queen\yell3Q.WAV" NAME="yell3Q" GROUP="Queen"
#exec AUDIO IMPORT FILE="Sounds\Queen\stab1Q.WAV" NAME="stab1Q" GROUP="Queen"
#exec AUDIO IMPORT FILE="Sounds\Queen\outcoldQ.WAV" NAME="outcoldQ" GROUP="Queen"
#exec AUDIO IMPORT FILE="Sounds\Queen\nearby2Q.WAV" NAME="nearby2Q" GROUP="Queen"
#exec AUDIO IMPORT FILE="Sounds\Queen\amb1Q.WAV" NAME="amb1Q" GROUP="Queen"
#exec AUDIO IMPORT FILE="Sounds\Queen\nearby2Q.WAV" NAME="nearby2Q" GROUP="Queen"
#exec AUDIO IMPORT FILE="Sounds\Titan\step1a.WAV" NAME="step1t" GROUP="Titan"
#exec AUDIO IMPORT FILE="Sounds\Generic\teleport1.WAV" NAME="Teleport1" GROUP="Generic"

//Queen variables;
var() int ClawDamage,
	StabDamage;
var() name ScreamEvent;

var byte row;
var(Sounds) sound footstepSound;
var(Sounds) sound ScreamSound;
var(Sounds) sound stab;
var(Sounds) sound shoot;
var(Sounds) sound claw;

var bool	bJustScreamed;
var bool	bEndFootStep;
var QueenShield Shield;
var vector TelepDest;

function PostBeginPlay()
{
	Super.PostBeginPlay();
	ProjectileSpeed = 1200 + 100 * Skill;
	GroundSpeed = GroundSpeed * (1 + 0.1 * Skill);
}

event bool EncroachingOn( actor Other )
{
	if ( (Other.Brush != None) || (Brush(Other) != None) )
		return true;
		
	return false;
}

function TryToDuck(vector duckDir, bool bReversed)
{
	if ( (Shield != None) || (AnimSequence == 'Shield') )
		return;

	PlayAnim('Shield', 1.0, 0.1);
	bCrouching = true;
	GotoState('RangedAttack', 'Challenge');
}

function SpawnShield()
{
	Shield = Spawn(class'QueenShield',,,Location + 150 * Vector(Rotation)); 
	Shield.SetBase(self);
}

function eAttitude AttitudeToCreature(Pawn Other)
{
	if ( Other.IsA('Skaarj') )
	{
		if ( Other.IsA('SkaarjBerserker') )
			return ATTITUDE_Hate;
		else
			return ATTITUDE_Friendly;
	}
	else if ( Other.IsA('Pupae') )
		return ATTITUDE_Friendly;
	else if ( Other.IsA('Nali') )
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
	if (aPlayer == None)
	{	
		if (Other.Physics != PHYS_Walking)
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
		if ( (Other.Physics != PHYS_Walking) || (dist > 1500) )
			return;
	}
	
	Momentum = -0.5 * Other.Velocity + 100 * Normal(Other.Location - Location);
	Momentum.Z =  7000000.0/((0.5 * dist + 500) * Other.Mass);
	Other.AddVelocity(Momentum);
}

function FootStep()
{
	bEndFootstep = false;
	PlaySound(FootstepSound, SLOT_Interact, 8);
}

function Scream()
{
	local actor A;
	local pawn Thrown;

	if (ScreamEvent != '')
		foreach AllActors( class 'Actor', A, ScreamEvent )
			A.Trigger( Self, Instigator );

	PlaySound(ScreamSound, SLOT_Talk, 2 * TransientSoundVolume);
	PlaySound(ScreamSound, SLOT_None, 2 * TransientSoundVolume);
	PlaySound(ScreamSound, SLOT_None, 2 * TransientSoundVolume);
	PlaySound(ScreamSound, SLOT_None, 2 * TransientSoundVolume);
	PlayAnim('Scream');
	bJustScreamed = true;
}

function PlayWaiting()
{
	local float decision;
	local float animspeed;
	
	if (bEndFootStep)
		FootStep();
	decision = FRand();
	animspeed = 0.2 + 0.5 * FRand();
	LoopAnim('Meditate', animspeed);
}

function PlayChallenge()
{
	if (bEndFootStep)
		FootStep();
	if ( IsAnimating() && (AnimSequence == 'Shield') )
		return;
	Scream();
}

function TweenToFighter(float tweentime)
{
	bEndFootStep = ( ((AnimSequence == 'Walk') || (AnimSequence == 'Run')) && (AnimFrame > 0.1) );   
	TweenAnim('Fighter', tweentime);
}

function TweenToRunning(float tweentime)
{
	if ( (AnimSequence != 'Run') || !bAnimLoop )
		TweenAnim('Run', tweentime);
}

function TweenToWalking(float tweentime)
{
	TweenAnim('Walk', tweentime);
}

function TweenToWaiting(float tweentime)
{
	TweenAnim('Meditate', tweentime);
}

function TweenToPatrolStop(float tweentime)
{
	TweenAnim('Meditate', tweentime);
}

function PlayRunning()
{
	LoopAnim('Run', -1.0/GroundSpeed,, 0.8);
}

function PlayWalking()
{
	LoopAnim('Walk', -1.0/GroundSpeed,, 0.8);
}

function PlayThreatening()
{
	DesiredSpeed = 0.0;

	if ( FRand() < 0.75)
		PlayAnim('Meditate', 0.4 + 0.6 * FRand(), 0.3);
	else 
	{
		TweenAnim('Fighter', 0.3);
		PlayThreateningSound();
	}
}

function PlayTurning()
{
	if (bEndFootStep)
		FootStep();
	DesiredSpeed = 0.0;
	TweenAnim('Run', 0.4);
}

function PlayDying(name DamageType, vector HitLocation)
{
	PlayAnim('OutCold', 0.7, 0.1);
	PlaySound(Die, SLOT_Talk);	
}

function PlayTakeHit(float tweentime, vector HitLoc, int Damage)
{
	TweenAnim('TakeHit', tweentime);
}

function SpawnShot()
{
	local vector X,Y,Z, projStart;

	GetAxes(Rotation,X,Y,Z);
	
	if (row == 0)
		MakeNoise(1.0);
	
	projStart = Location + 1 * CollisionRadius * X + ( 0.7 - 0.2 * row) * CollisionHeight * Z + 0.2 * CollisionRadius * Y;
	spawn(RangedProjectile ,self,'',projStart,AdjustAim(ProjectileSpeed, projStart, 400 * (4 - row)/(3.5-skill), false, bWarnTarget));

	projStart = Location + 1 * CollisionRadius * X + ( 0.7 - 0.2 * row) * CollisionHeight * Z - 0.2 * CollisionRadius * Y;
	spawn(RangedProjectile ,self,'',projStart,AdjustAim(ProjectileSpeed, projStart, 400 * (4 - row)/(3.5-skill), true, bWarnTarget));
	row++;
}

function PlayVictoryDance()
{
	if (bEndFootStep)
		FootStep();
	DesiredSpeed = 0.0;
	PlayAnim('ThreeHit', 0.7, 0.15); //gib the enemy here!
	PlaySound(Threaten, SLOT_Talk);		
}

function ClawDamageTarget()
{
	if ( MeleeDamageTarget(ClawDamage, (50000.0 * (Normal(Target.Location - Location)))) )
		PlaySound(Claw, SLOT_Interact);
}

function StabDamageTarget()
{
	local vector X,Y,Z;
	GetAxes(Rotation,X,Y,Z);
	
	if ( MeleeDamageTarget(StabDamage, (15000.0 * ( Y + vect(0,0,1)))) )
		PlaySound(Stab, SLOT_Interact);
}

function PlayMeleeAttack()
{
	local float decision;

	if (bEndFootStep)
		FootStep();
	decision = FRand();
	if (decision < 0.4)
	{
		PlaySound(Stab, SLOT_Interact);
 		PlayAnim('Stab');
 	}
	else if (decision < 0.7)
	{
		PlaySound(Claw, SLOT_Interact);
		PlayAnim('Claw');
	} 
	else 
	{
		PlaySound(Claw, SLOT_Interact);
		PlayAnim('Gouge');
	}
}

function TweenToFalling()
{
	TweenAnim('Jump', 0.2);
}

function PlayInAir()
{
	TweenAnim('Jump', 0.5);
}

function PlayLanded(float impactVel)
{
	local Pawn Thrown;

	TweenAnim('Land', 0.1);

	//throw all nearby creatures, and play sound
	Thrown = Level.PawnList;
	While ( Thrown != None )
	{
		ThrowOther(Thrown);
		Thrown = Thrown.nextPawn;
	}
}

function PlayRangedAttack()
{
	if (bEndFootStep)
		FootStep();

	if ( !bJustScreamed && (FRand() < 0.15) )
		Scream();
	else if ( (Shield != None) && (FRand() < 0.5)
		&& (((Enemy.Location - Location) Dot (Shield.Location - Location)) > 0) )
		Scream();
	else
	{
		if ( Shield != None )
			Shield.Destroy();
		row = 0;
		bJustScreamed = false;
		PlayAnim('Shoot1'); 
		PlaySound(Shoot, SLOT_Interact);			
	}
}

state TacticalMove
{
ignores SeePlayer, HearNoise;

	function PickDestination(bool bNoCharge)
	{
		if ( FRand() < 0.26 )
			GotoState('Teleporting');
		else
			Super.PickDestination(bNoCharge);
	}
}		
		
state Hunting
{
ignores EnemyNotVisible; 

	function PickDestination()
	{
		GotoState('Teleporting');
	}
}


State Teleporting
{
ignores TakeDamage, SeePlayer, EnemyNotVisible, HearNoise, KilledBy, Bump, HitWall, HeadZoneChange, FootZoneChange, ZoneChange, Falling, WarnTarget, Died;

	function Tick(float DeltaTime)
	{
		local int NewFatness; 
		local rotator EnemyRot;

		if ( Style == STY_Translucent )
		{
			ScaleGlow -= 3 * DeltaTime;
			if ( ScaleGlow < 0.3 )
			{
				Spawn(class'QueenTeleportEffect',,, TelepDest);
				Spawn(class'QueenTeleportLight',,, TelepDest);
				EnemyRot = rotator(Enemy.Location - Location);
				EnemyRot.Pitch = 0;
				SetLocation(TelepDest);
				setRotation(EnemyRot);
				PlaySound(sound'Teleport1', SLOT_Interface);
				GotoState('Attacking');
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

	function ChooseDestination()
	{
		local NavigationPoint N;
		local vector ViewPoint, HitLocation, HitNormal, Best;
		local actor HitActor;
		local float rating, newrating;

		N = Level.NavigationPointList;
		Best = Location;
		rating = 0;

		while ( N != None )
		{
			if ( N.IsA('QueenDest') ) // rate it
			{
				newrating = 0;
				if ( Best == Location )
					Best = N.Location;
				ViewPoint = N.Location + EyeHeight * vect(0,0,1);
				HitActor = Trace(HitLocation, HitNormal, Enemy.Location, ViewPoint, false);
				if ( HitActor == None )
					newrating = 20000;

				newrating = newrating - VSize(N.Location - Enemy.Location) + 1000 * FRand()
							+ 4 * VSize(N.Location - Location);
				if ( N.Location.Z > Enemy.Location.Z )
					newrating += 1000;
				
				if ( newrating > rating )
				{
					rating = newrating;
					Best = N.Location;
				}
			}

			N = N.nextNavigationPoint;
		}

		TelepDest = Best;
	}

	function BeginState()
	{
		Acceleration = Vect(0,0,0);
		ChooseDestination();
	}

	function EndState()
	{
		bUnlit = false;
		Style = STY_Normal;
		ScaleGlow = 1.0;
		fatness = Default.fatness;
	}
}

	
defaultproperties
{
     RangedProjectile=QueenProjectile
     ScreamSound=yell3Q
     Stab=stab1Q
     Shoot=shoot1Q
     Claw=claw1Q
	 ClawDamage=50
	 StabDamage=80
	 FootStepSound=step1t
     CarcassType=CreatureCarcass
	 TimeBetweenAttacks=+00001.000000
     Aggressiveness=+00005.000000
     ReFireRate=+00000.400000
	 ReducedDamageType='shot'
	 ReducedDamagePct=+0.5
     bHasRangedAttack=True
	 bCanDuck=True
	 bIsBoss=True
     Acquire=yell1Q
     Fear=yell2Q
     Roam=nearby2Q
     Threaten=yell2Q
     Health=1500
     Visibility=250
     SightRadius=+03000.000000
     MeleeRange=+00100.000000
     Intelligence=BRAINS_HUMAN
     GroundSpeed=+00400.00000
     AccelRate=+01500.000000
     JumpZ=+00800.000000
     MaxStepHeight=+00025.000000
     RotationRate=(Pitch=6000,Yaw=50000,Roll=3072)
     HitSound1=yell2Q
     HitSound2=yell2Q
     Die=outcoldQ
     CombatStyle=+00000.950000
     DrawType=DT_Mesh
     Mesh=SkQueen
	 AmbientSound=Amb1Q
     SoundRadius=32
     TransientSoundVolume=+00016.000000
     CollisionRadius=+00090.200000
     CollisionHeight=+00106.700000
     Mass=+01000.000000
}

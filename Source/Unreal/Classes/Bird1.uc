//=============================================================================
// Bird1.
//=============================================================================
class Bird1 expands FlockPawn;

#exec MESH IMPORT MESH=Bird ANIVFILE=MODELS\bird1_a.3D DATAFILE=MODELS\bird1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Bird X=0 Y=0 Z=0 YAW=64 ROLL=-64

#exec MESH SEQUENCE MESH=Bird SEQ=All      STARTFRAME=0   NUMFRAMES=31
#exec MESH SEQUENCE MESH=Bird SEQ=Middle	STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Bird SEQ=Dead1		STARTFRAME=1   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Bird SEQ=Dead2		STARTFRAME=2   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Bird SEQ=Hit1		STARTFRAME=3   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Bird SEQ=Hit2		STARTFRAME=4   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Bird SEQ=Ground1	STARTFRAME=5   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Bird SEQ=Ground2	STARTFRAME=6   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Bird SEQ=Flight	STARTFRAME=7   NUMFRAMES=24

#exec TEXTURE IMPORT NAME=JBird11 FILE=MODELS\Bird1.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=Bird X=0.06 Y=0.06 Z=0.12
#exec MESHMAP SETTEXTURE MESHMAP=Bird NUM=1 TEXTURE=JBird11

#exec AUDIO IMPORT FILE="Sounds\Manta\injur1a.WAV" NAME="injur1m" GROUP="Manta"
#exec AUDIO IMPORT FILE="Sounds\Manta\call1a.WAV" NAME="call1m" GROUP="Manta"
#exec AUDIO IMPORT FILE="Sounds\Bird\call2bd.WAV" NAME="call2b" GROUP="Bird"

var() name GoalTag;
var	actor GoalActor;
var() float CircleRadius;
var float Angle;
var	vector CircleCenter;
var() bool bCircle;

function PreBeginPlay()
{
	Super.PreBeginPlay();
	CircleCenter = Location;
	if ( GoalTag != '' )
	{
		AirSpeed = 2 * AirSpeed;
		ForEach AllActors(class 'Actor', GoalActor, GoalTag)
			Break;
	}
}

function PlayCall()
{
	if ( FRand() < 0.4 ) 
		PlaySound(sound'call1m',,1 + FRand(),,, 1 + 0.7 * FRand());
	else
		PlaySound(sound'call2b',,1 + FRand(),,, 0.8 + 0.4 * FRand());
}

function PlayHit(float Damage, vector HitLocation, name damageType, float MomentumZ)
{
	if ( FRand() < 0.5 )
		TweenAnim('Hit1', 0.1);
	else
		TweenAnim('Hit2', 0.1);
	PlaySound(sound'injur1m',,,,, 1.2);
	AirSpeed = 1.5 * Default.AirSpeed;	
	bCircle = false;
	SetPhysics(PHYS_Falling);
	GotoState('TakeHit');
}

function PlayDeathHit(float Damage, vector HitLocation, name damageType)
{
	PlaySound(sound'injur1m');
	if ( FRand() < 0.5 )
		TweenAnim('Dead1', 0.2);
	else
		TweenAnim('Dead2', 0.2);	
}


function Died(pawn Killer, name damageType, vector HitLocation)
{
	local Actor A;

	if( Event != '' )
		foreach AllActors( class 'Actor', A, Event )
			A.Trigger( Self, Killer );
	
	if ( Region.Zone.bDestructive && (Region.Zone.ExitActor != None) )
	{
		Spawn(Region.Zone.ExitActor);
		Destroy();
		return;
	}
	GotoState('Dying');
}

function WhatToDoNext()
{
	if ( bCircle )
		GotoState('Circle');
	else if ( GoalActor != None )
		GotoState('MoveToGoal');
	else
		GotoState('Meander');
}

auto state startup
{
	function Trigger( actor Other, pawn EventInstigator )
	{
		if ( GoalActor != None )
			GotoState('MoveToGoal');
	}

Begin:
	if ( GoalActor == None )
		WhatToDoNext();
}

state TakeHit
{
	ignores seeplayer, enemynotvisible;

Begin:
	FinishAnim();
	Sleep(0.3);
	TweenAnim('Flight', 0.1);
	WhatToDoNext();
}

state meander
{
	ignores seeplayer, enemynotvisible;

	singular function ZoneChange( ZoneInfo NewZone )
	{
		if (NewZone.bWaterZone || NewZone.bPainZone)
		{
			SetLocation(OldLocation);
			Velocity = vect(0,0,0);
			Acceleration = vect(0,0,0);
			MoveTimer = -1.0;
		}
	}
	 		
begin:
	SetPhysics(PHYS_Flying);
wander:
	if ( FRand() < 0.2 )
		PlayCall();
	Destination = CircleCenter + FRand() * CircleRadius * VRand();
	if ( Abs(Destination.Z - CircleCenter.Z) > 200 )
		Destination.Z = CircleCenter.Z; 
	if ( (Destination.Z >= Location.Z) || (FRand() < 0.5) )
		LoopAnim('Flight');
	else
		TweenAnim('Flight', 1.0);
	MoveTo(Destination);
	Goto('Wander');
}

state movetogoal
{
	ignores seeplayer, enemynotvisible;
	
	function HitWall(vector HitNormal, actor Wall)
	{
		GoalActor = None;
		GotoState('Meander');
	}
 		
begin:
	SetPhysics(PHYS_Flying);
wander:
	if ( FRand() < 0.5 )
		PlayCall();
	LoopAnim('Flight', 2.0);
	MoveTo(GoalActor.Location);
	If ( VSize(Location - GoalActor.Location) < 100 )
		Destroy();
	else
		Goto('Wander');
}

state circle
{
	ignores seeplayer, enemynotvisible;

	singular function ZoneChange( ZoneInfo NewZone )
	{
		if (NewZone.bWaterZone || NewZone.bPainZone)
		{
			SetLocation(OldLocation);
			Velocity = vect(0,0,0);
			Acceleration = vect(0,0,0);
			MoveTimer = -1.0;
		}
	}
	 		
begin:
	SetPhysics(PHYS_Flying);
wander:
	if ( FRand() < 0.2 )
	{
		LoopAnim('Flight');
		PlayCall();
	}
	else
		PlayAnim('Flight');
	Angle += 1.0484; //2*3.1415/6;	
	Destination.X = CircleCenter.X - CircleRadius * Sin(Angle);
	Destination.Y = CircleCenter.Y + CircleRadius * Cos(Angle);
	Destination.Z = CircleCenter.Z + 30 * FRand() - 15;
	MoveTo(Destination);
	Goto('Wander');
}

State Dying
{
	ignores seeplayer, enemynotvisible;

	function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
							Vector momentum, name damageType)
	{
		destroy();
	}

	function Landed(vector HitNormal)
	{
		local rotator newRot;

		newRot = Rotation;
		newRot.Pitch = 0;
		newRot.Roll = 0;
		If ( FRand() < 0.5 )
			TweenAnim('Ground1', 0.2);
		else
			TweenAnim('Ground2', 0.2);
		SetRotation(newRot);
		SetPhysics(PHYS_None);
		SetTimer(2.0, True);
	}	

	function Timer()
	{
		if ( !PlayerCanSeeMe() )
			Destroy();
	}
			
Begin:
	SetPhysics(PHYS_Falling);
	Sleep(10);
	Timer();
}			

defaultproperties
{
     AirSpeed=+00300.000000
     AccelRate=+00800.000000
     SightRadius=+02000.000000
	 CircleRadius=+00500.000000 
	 Health=17
     DrawType=DT_Mesh
	 bUnlit=true
     Mesh=Unreal.Bird
     CollisionHeight=+00006.000000
     Land=None
     RotationRate=(Pitch=12000,Yaw=20000,Roll=12000)
}

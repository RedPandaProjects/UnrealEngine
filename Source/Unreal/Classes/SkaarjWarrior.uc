//=============================================================================
// Skaarj.
//=============================================================================
class SkaarjWarrior expands Skaarj;
	
#exec MESH IMPORT MESH=Skaarjw ANIVFILE=MODELS\Skaarj_a.3D DATAFILE=MODELS\Skaarj_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=Skaarjw X=0 Y=-20 Z=-20 YAW=64 ROLL=-64

#exec MESH SEQUENCE MESH=Skaarjw SEQ=All		  STARTFRAME=0   NUMFRAMES=542
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Claw         STARTFRAME=0   NUMFRAMES=11  RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=Skaarjw SEQ=GutHit       STARTFRAME=11  NUMFRAMES=1
#exec MESH SEQUENCE MESH=Skaarjw SEQ=guncheck     STARTFRAME=12  NUMFRAMES=7   RATE=6
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Breath       STARTFRAME=19  NUMFRAMES=12  RATE=6
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Breath2      STARTFRAME=31  NUMFRAMES=8   RATE=6
#exec MESH SEQUENCE MESH=Skaarjw SEQ=MButton1     STARTFRAME=39  NUMFRAMES=33  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=MButton2     STARTFRAME=49  NUMFRAMES=33  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=MButton3     STARTFRAME=59  NUMFRAMES=36  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=MButton4     STARTFRAME=72  NUMFRAMES=23  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Button1      STARTFRAME=39  NUMFRAMES=10  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Button2      STARTFRAME=49  NUMFRAMES=10  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Button3      STARTFRAME=59  NUMFRAMES=13  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Button4      STARTFRAME=72  NUMFRAMES=10  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Button5      STARTFRAME=82  NUMFRAMES=13  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Death        STARTFRAME=95  NUMFRAMES=23  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Death2       STARTFRAME=118 NUMFRAMES=13  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Death3       STARTFRAME=131 NUMFRAMES=21  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Death4       STARTFRAME=152 NUMFRAMES=11  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Duck         STARTFRAME=163 NUMFRAMES=1            Group=Ducking
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Fighter      STARTFRAME=164 NUMFRAMES=15  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Firing       STARTFRAME=179 NUMFRAMES=11  RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Getup        STARTFRAME=190 NUMFRAMES=11  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=HairFlip     STARTFRAME=201 NUMFRAMES=20  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=HeadHit      STARTFRAME=221 NUMFRAMES=1
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Headup       STARTFRAME=222 NUMFRAMES=11  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Jog2Fight    STARTFRAME=233 NUMFRAMES=11  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Jog          STARTFRAME=244 NUMFRAMES=10  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=FullJump     STARTFRAME=254 NUMFRAMES=15  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Jump         STARTFRAME=254 NUMFRAMES=5   RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=InAir        STARTFRAME=259 NUMFRAMES=1   
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Landed       STARTFRAME=266 NUMFRAMES=1  
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Jump2        STARTFRAME=269 NUMFRAMES=1
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Land         STARTFRAME=270 NUMFRAMES=1
#exec MESH SEQUENCE MESH=Skaarjw SEQ=LeftDodge    STARTFRAME=271 NUMFRAMES=13  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=LeftHit      STARTFRAME=284 NUMFRAMES=1
#exec MESH SEQUENCE MESH=Skaarjw SEQ=StrafeLeftFr STARTFRAME=285 NUMFRAMES=14  RATE=15  Group=MovingAttack
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Looking      STARTFRAME=299 NUMFRAMES=23  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=StrafeLeft   STARTFRAME=322 NUMFRAMES=14  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Lunge        STARTFRAME=336 NUMFRAMES=13  RATE=15  Group=Attack
#exec MESH SEQUENCE MESH=Skaarjw SEQ=StrafeRightFr STARTFRAME=349 NUMFRAMES=14 RATE=15  Group=MovingAttack
#exec MESH SEQUENCE MESH=Skaarjw SEQ=RightHit     STARTFRAME=363 NUMFRAMES=1
#exec MESH SEQUENCE MESH=Skaarjw SEQ=StrafeRight  STARTFRAME=364 NUMFRAMES=14  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Spin         STARTFRAME=378 NUMFRAMES=20           Group=Attack
#exec MESH SEQUENCE MESH=Skaarjw SEQ=gunfix       STARTFRAME=398 NUMFRAMES=8   RATE=6
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Swim         STARTFRAME=406 NUMFRAMES=15  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Walk         STARTFRAME=421 NUMFRAMES=15  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=WalkFire     STARTFRAME=436 NUMFRAMES=15  RATE=15  Group=MovingAttack 
#exec MESH SEQUENCE MESH=Skaarjw SEQ=RightDodge   STARTFRAME=451 NUMFRAMES=13  RATE=15
#exec MESH SEQUENCE MESH=Skaarjw SEQ=JogFire      STARTFRAME=464 NUMFRAMES=10  RATE=15  Group=MovingAttack
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Death5       STARTFRAME=474 NUMFRAMES=28  RATE=15  Group=MovingAttack
#exec MESH SEQUENCE MESH=Skaarjw SEQ=Stretch      STARTFRAME=502 NUMFRAMES=25  RATE=15  Group=MovingAttack
#exec MESH SEQUENCE MESH=Skaarjw SEQ=SwimFire     STARTFRAME=527 NUMFRAMES=15  RATE=15  Group=MovingAttack

#exec TEXTURE IMPORT NAME=Skaarjw1 FILE=MODELS\Skaarj.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=Skaarjw X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=Skaarjw NUM=0 TEXTURE=Skaarjw1

#exec MESH NOTIFY MESH=Skaarjw SEQ=WalkFire TIME=0.5 FUNCTION=SpawnTwoShots
#exec MESH NOTIFY MESH=Skaarjw SEQ=JogFire TIME=0.5 FUNCTION=SpawnTwoShots
#exec MESH NOTIFY MESH=Skaarjw SEQ=SwimFire TIME=0.5 FUNCTION=SpawnTwoShots
#exec MESH NOTIFY MESH=Skaarjw SEQ=StrafeLeftFr TIME=0.5 FUNCTION=SpawnTwoShots
#exec MESH NOTIFY MESH=Skaarjw SEQ=StrafeRightFr TIME=0.5 FUNCTION=SpawnTwoShots
#exec MESH NOTIFY MESH=Skaarjw SEQ=Firing TIME=0.25 FUNCTION=SpawnTwoShots
#exec MESH NOTIFY MESH=Skaarjw SEQ=Spin TIME=0.48 FUNCTION=SpinDamageTarget
#exec MESH NOTIFY MESH=Skaarjw SEQ=Spin TIME=0.67 FUNCTION=SpinDamageTarget
#exec MESH NOTIFY MESH=Skaarjw SEQ=Claw TIME=0.24 FUNCTION=ClawDamageTarget
#exec MESH NOTIFY MESH=Skaarjw SEQ=Claw TIME=0.76 FUNCTION=ClawDamageTarget
#exec MESH NOTIFY MESH=Skaarjw SEQ=Death TIME=0.41 FUNCTION=LandThump
#exec MESH NOTIFY MESH=Skaarjw SEQ=Death2 TIME=0.61 FUNCTION=LandThump
#exec MESH NOTIFY MESH=Skaarjw SEQ=Death3 TIME=0.73 FUNCTION=LandThump
#exec MESH NOTIFY MESH=Skaarjw SEQ=Death4 TIME=0.62 FUNCTION=LandThump
#exec MESH NOTIFY MESH=Skaarjw SEQ=Death5 TIME=0.82 FUNCTION=LandThump
#exec MESH NOTIFY MESH=Skaarjw SEQ=Walk TIME=0.3 FUNCTION=WalkStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=Walk TIME=0.8 FUNCTION=WalkStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=WalkFire TIME=0.3 FUNCTION=WalkStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=WalkFire TIME=0.8 FUNCTION=WalkStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=Jog TIME=0.25 FUNCTION=RunStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=Jog TIME=0.75 FUNCTION=RunStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=JogFire TIME=0.25 FUNCTION=RunStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=JogFire TIME=0.75 FUNCTION=RunStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=StrafeLeft TIME=0.25 FUNCTION=RunStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=StrafeLeft TIME=0.75 FUNCTION=RunStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=StrafeLeftFr TIME=0.25 FUNCTION=RunStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=StrafeLeftFr TIME=0.75 FUNCTION=RunStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=StrafeRight TIME=0.25 FUNCTION=RunStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=StrafeRight TIME=0.75 FUNCTION=RunStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=StrafeRightFr TIME=0.25 FUNCTION=RunStep
#exec MESH NOTIFY MESH=Skaarjw SEQ=StrafeRightFr TIME=0.75 FUNCTION=RunStep

#exec AUDIO IMPORT FILE="Sounds\Skaarj\blade1a.WAV" NAME="blade1s" GROUP="Skaarj"

//-----------------------------------------------------------------------------
// SkaarjWarrior variables.

var(Sounds) sound Blade;

//=========================================================================================

function PostBeginPlay()
{
	Super.PostBeginPlay();
	if ( skill == 3 )
	{
		SpinDamage = 20;
		ClawDamage = 17;
	}
}

function TryToDuck(vector duckDir, bool bReversed)
{
	local vector HitLocation, HitNormal, Extent;
	local bool duckLeft, bSuccess;
	local actor HitActor;
	local float decision;

	//log("duck");
				
	duckDir.Z = 0;
	duckLeft = !bReversed;

	Extent.X = CollisionRadius;
	Extent.Y = CollisionRadius;
	Extent.Z = CollisionHeight;
	HitActor = Trace(HitLocation, HitNormal, Location + 200 * duckDir, Location, false, Extent);
	bSuccess = ( (HitActor == None) || (VSize(HitLocation - Location) > 150) );
	if ( !bSuccess )
	{
		duckLeft = !duckLeft;
		duckDir *= -1;
		HitActor = Trace(HitLocation, HitNormal, Location + 200 * duckDir, Location, false, Extent);
		bSuccess = ( (HitActor == None) || (VSize(HitLocation - Location) > 150) );
	}
	if ( !bSuccess )
		return;
	
	if ( HitActor == None )
		HitLocation = Location + 200 * duckDir;
	HitActor = Trace(HitLocation, HitNormal, HitLocation - MaxStepHeight * vect(0,0,1), HitLocation, false, Extent);
	if (HitActor == None)
		return;
		
	//log("good duck");

	SetFall();
	if ( duckLeft )
		PlayAnim('LeftDodge', 1.35);
	else
		PlayAnim('RightDodge', 1.35);
	Velocity = duckDir * GroundSpeed;
	Velocity.Z = 200;
	SetPhysics(PHYS_Falling);
	GotoState('FallingState','Ducking');
}	

function bool CanFireAtEnemy()
{
	local vector HitLocation, HitNormal,X,Y,Z, projStart, EnemyDir, EnemyUp;
	local actor HitActor1, HitActor2;
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
	projStart = Location + 0.9 * CollisionRadius * X + CollisionRadius * Y + 0.4 * CollisionHeight * Z;
	HitActor1 = Trace(HitLocation, HitNormal, projStart + EnemyDir + EnemyUp, projStart, true);
	if ( (HitActor1 != Enemy) && (Pawn(HitActor1) != None) 
		&& (AttitudeTo(Pawn(HitActor1)) > ATTITUDE_Ignore) )
		return false;
		 
	projStart = Location + 0.9 * CollisionRadius * X - CollisionRadius * Y + 0.4 * CollisionHeight * Z;
	HitActor2 = Trace(HitLocation, HitNormal, projStart + EnemyDir + EnemyUp, projStart, true);

	if ( (HitActor2 != Enemy) && (Pawn(HitActor2) != None) 
		&& (AttitudeTo(Pawn(HitActor2)) > ATTITUDE_Ignore) )
		return false;

	if ( (HitActor2 == None) || (HitActor2 == Enemy) || (HitActor1 == None) || (HitActor1 == Enemy) 
		|| (Pawn(HitActor2) != None) || (Pawn(HitActor1) != None) )
		return true;

	HitActor2 = Trace(HitLocation, HitNormal, projStart + EnemyDir, projStart , true);

	return ( (HitActor2 == None) || (HitActor2 == Enemy) 
			|| ((Pawn(HitActor2) != None) && (AttitudeTo(Pawn(HitActor2)) <= ATTITUDE_Ignore)) );
}

function PlayCock()
{
	PlaySound(Blade, SLOT_Interact,,,800);
}

function PlayPatrolStop()
	{
	local float decision;
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	if ( bButtonPusher )
	{
		PushButtons();
		return;
	}

	decision = FRand();
	if (decision < 0.05)
		{
		SetAlertness(-0.5);
		PlaySound(HairFlip, SLOT_Talk);
		PlayAnim('HairFlip', 0.4 + 0.3 * FRand());
		}
	else 
		{
		SetAlertness(0.2);	
		LoopAnim('Breath', 0.3 + 0.6 * FRand());
		}
	}

function PlayChallenge()
{
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}
	PlayThreateningSound();
	PlayAnim('Fighter', 0.8 + 0.5 * FRand(), 0.1);
}

function PlayRunning()
{
	local float strafeMag;
	local vector Focus2D, Loc2D, Dest2D;
	local vector lookDir, moveDir, Y;

	DesiredSpeed = MaxDesiredSpeed;
	if (Region.Zone.bWaterZone)
	{
		PlaySwimming();
		return;
	}

	if (Focus == Destination)
	{
		LoopAnim('Jog', -1.0/GroundSpeed,, 0.5);
		return;
	}	
	Focus2D = Focus;
	Focus2D.Z = 0;
	Loc2D = Location;
	Loc2D.Z = 0;
	Dest2D = Destination;
	Dest2D.Z = 0;
	lookDir = Normal(Focus2D - Loc2D);
	moveDir = Normal(Dest2D - Loc2D);
	strafeMag = lookDir dot moveDir;
	if (strafeMag > 0.8)
		LoopAnim('Jog', -1.0/GroundSpeed,, 0.5);
	else if (strafeMag < -0.8)
		LoopAnim('Jog', -1.0/GroundSpeed,, 0.5);
	else
	{
		Y = (lookDir Cross vect(0,0,1));
		if ((Y Dot (Dest2D - Loc2D)) > 0)
		{
			if ( (AnimSequence == 'StrafeRight') || (AnimSequence == 'StrafeRightFr') ) 
				LoopAnim('StrafeRight', -2.5/GroundSpeed,, 1.0);
			else 
				LoopAnim('StrafeRight', -2.5/GroundSpeed,0.1, 1.0);
		}
		else
		{
			if ( (AnimSequence == 'StrafeLeft') || (AnimSequence == 'StrafeLeftFr') ) 
				LoopAnim('StrafeLeft', -2.5/GroundSpeed,, 1.0);
			else
				LoopAnim('StrafeLeft', -2.5/GroundSpeed,0.1, 1.0);
		}
	}
}

function PlayMovingAttack()
{
	local float strafeMag;
	local vector Focus2D, Loc2D, Dest2D;
	local vector lookDir, moveDir, Y;

	if (Region.Zone.bWaterZone)
	{
		LoopAnim('SwimFire', -1.0/WaterSpeed,, 0.4); 
		return;
	}
	DesiredSpeed = MaxDesiredSpeed;

	if (Focus == Destination)
	{
		LoopAnim('JogFire', -1.0/GroundSpeed,, 0.4);
		return;
	}	
	Focus2D = Focus;
	Focus2D.Z = 0;
	Loc2D = Location;
	Loc2D.Z = 0;
	Dest2D = Destination;
	Dest2D.Z = 0;
	lookDir = Normal(Focus2D - Loc2D);
	moveDir = Normal(Dest2D - Loc2D);
	strafeMag = lookDir dot moveDir;
	if (strafeMag > 0.8)
		LoopAnim('JogFire', -1.0/GroundSpeed,, 0.4);
	else if (strafeMag < -0.8)
		LoopAnim('JogFire', -1.0/GroundSpeed,, 0.4);
	else
	{
		MoveTimer += 0.2;
		DesiredSpeed = 0.6;
		Y = (lookDir Cross vect(0,0,1));
		if ((Y Dot (Dest2D - Loc2D)) > 0) 
		{
			if ( (AnimSequence == 'StrafeRight') || (AnimSequence == 'StrafeRightFr') ) 
				LoopAnim('StrafeRightFr', -2.5/GroundSpeed,, 1.0); 
			else
				LoopAnim('StrafeRightFr', -2.5/GroundSpeed,0.1, 1.0); 
		}
		else
		{
			if ( (AnimSequence == 'StrafeLeft') || (AnimSequence == 'StrafeLeftFr') ) 
				LoopAnim('StrafeLeftFr', -2.5/GroundSpeed,, 1.0);
			else
				LoopAnim('StrafeLeftFr', -2.5/GroundSpeed,0.1, 1.0);
		}
	}
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
	
	if ( decision < 0.7 )
		PlayAnim('Breath2', animspeed, 0.3);
	else if ( decision < 0.9 )
	{
		PlayThreateningSound();
		PlayAnim('Fighter', animspeed, 0.3);
	}
	else
	{
		PlaySound(HairFlip, SLOT_Talk);
		PlayAnim('HairFlip', animspeed, 0.3);
	}	 
}

function SpawnTwoShots()
{
	local rotator FireRotation;
	local vector X,Y,Z, projStart;

	GetAxes(Rotation,X,Y,Z);
	MakeNoise(1.0);
	projStart = Location + 0.9 * CollisionRadius * X + 0.9 * CollisionRadius * Y + 0.4 * CollisionHeight * Z;
	FireRotation = AdjustAim(ProjectileSpeed, projStart, 400, bLeadTarget, bWarnTarget);  
	spawn(RangedProjectile,self,'',projStart, FireRotation);
		
	projStart = projStart - 1.8 * CollisionRadius * Y;
	FireRotation.Yaw += 400;
	spawn(RangedProjectile,self,'',projStart, FireRotation);
}

function PlayRangedAttack()
{
	if (Region.Zone.bWaterZone)
	{
		LoopAnim('SwimFire', -1.0/WaterSpeed,, 0.4); 
		return;
	}
	PlayAnim('Firing', 1.5); 
}

function PlayVictoryDance()
{
	PlaySound(HairFlip, SLOT_Talk);
	PlayAnim('HairFlip', 0.6, 0.1);
}

defaultproperties
{
     LungeDamage=30
     SpinDamage=16
     ClawDamage=14
     RangedProjectile=SkaarjProjectile
     Blade=blade1s
     CombatStyle=+00000.600000
     Mesh=Skaarjw
}

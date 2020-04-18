//=============================================================================
// NaliRabbit.
//=============================================================================
class NaliRabbit expands FlockPawn;

#exec MESH IMPORT MESH=Rabbit ANIVFILE=MODELS\NRoo_a.3D DATAFILE=MODELS\NRoo_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Rabbit X=0 Y=160 Z=00 YAW=128 ROLL=-64

#exec MESH SEQUENCE MESH=Rabbit SEQ=All     STARTFRAME=0  NUMFRAMES=98
#exec MESH SEQUENCE MESH=Rabbit SEQ=Call	STARTFRAME=0   NUMFRAMES=16
#exec MESH SEQUENCE MESH=Rabbit SEQ=Eat 	STARTFRAME=16  NUMFRAMES=40
#exec MESH SEQUENCE MESH=Rabbit SEQ=Jump    STARTFRAME=56  NUMFRAMES=14
#exec MESH SEQUENCE MESH=Rabbit SEQ=JumpUp  STARTFRAME=56  NUMFRAMES=9
#exec MESH SEQUENCE MESH=Rabbit SEQ=Land    STARTFRAME=65  NUMFRAMES=5
#exec MESH SEQUENCE MESH=Rabbit SEQ=Looking	STARTFRAME=70  NUMFRAMES=28

#exec TEXTURE IMPORT NAME=JRabbit1 FILE=MODELS\Nroo.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=Rabbit X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=Rabbit NUM=1 TEXTURE=JRabbit1

#exec AUDIO IMPORT FILE="Sounds\bunny\call1bn.WAV" NAME="CallBn" GROUP="Rabbit"

var() bool bStayClose;
var() float WanderRadius;
var vector StartLocation;

function Falling()
{
	PlayAnim('JumpUp', 1.0, 0.1);
}

function Landed(vector HitNormal)
{
	PlayAnim('Land', 1.0, 0.05);
}

function Attach( actor Other )
{
	if ( Other.Mass > 39 )
		TakeDamage( 10, Pawn(Other), Location, vect(0,0,0), 'Squashed');
}

function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
					Vector momentum, name damageType)
{
	local CreatureChunks carc;
	local Pawn aPawn;
	
	if ( Damage > 4 )
	{
		Velocity += momentum/mass;
		if ( instigator != None )
		{
			aPawn = Level.PawnList;
			While ( aPawn != None )
			{
				if ( aPawn.IsA('NaliRabbit') && (Location.Z - aPawn.Location.Z < 120) 
					&& (VSize(Location - aPawn.Location) < 500) )
				{
					aPawn.Enemy = instigator;
					aPawn.GotoState('Evade');
				}	
				aPawn = aPawn.NextPawn;
			}
		}
		Spawn(class'BloodBurst');
		carc = Spawn(class 'CreatureChunks',,, Location);
		if (carc != None)
		{
			carc.TrailSize = 3;
			carc.Mesh = mesh'CowBody1';
			carc.DrawScale = 0.65;
			carc.Initfor(self);
		}
		PlaySound(sound'gibP6',, 3.0);
		Destroy();
	}
	else
		PlaySound(sound'CallBn');
}

auto state Grazing
{
	ignores EnemyNotVisible;
	
	function SeePlayer(actor Seen)
	{
		Enemy = Pawn(Seen);
		Disable('SeePlayer');
		SetPhysics(PHYS_Walking);
		GotoState('Grazing', 'Wander');
	}
	
	function Bump(actor Other)
	{
		if ( (Normal(Destination - Location) Dot Normal(Other.Location - Location)) > 0.7 )
			MoveTimer = -1.0;
		if ( (Pawn(Other) != None) && (Pawn(Other).bIsPlayer || Other.IsA('ScriptedPawn')) )
		{
			Enemy = Pawn(Other);
			GotoState('Evade');
		}
		Disable('Bump');
	}

	function bool TestDirection(vector dir, out vector pick, bool bAlongWall)
	{	
		local vector HitLocation, HitNormal;
		local float minDist, Dist;
		local actor HitActor;

		dir.Z = 0;
		dir = Normal(dir);
		minDist = FMin(180.0, 6*CollisionRadius); 
		pick = Location + dir * (minDist + FRand() * 900);

		HitActor = Trace(HitLocation, HitNormal, pick, Location, false);
		Dist = VSize(HitLocation - Location);
		if ( (Dist < minDist) && (HitNormal.Z < 0.7) )
		{
			if ( !bAlongWall )
				return false;
			pick = HitLocation - dir + (HitNormal Cross vect(0,0,1)) * 5 * CollisionRadius;
			HitActor = Trace(HitLocation, HitNormal, pick , Location, false);
			if (HitActor != None)
				return false;
		}
		else 
			pick = HitLocation - 4 * CollisionRadius * dir;

		return true; 
	}
			
	function PickDestination()
	{
		local vector pickdir;
		local bool success;
		local float XY, dist;

		// don't wander too far
		if ( bStayClose )
		{
			pickDir = StartLocation - Location;
			dist = VSize(pickDir);
			if ( dist > WanderRadius )
			{
				pickdir = pickDir/dist;
				if ( TestDirection(pickdir, Destination, true) )
					return;
			}
		}
				
		//Favor XY alignment
		pickdir.Z = 0;
		XY = FRand();
		if (XY < 0.3)
		{
			pickdir.X = 1;
			pickdir.Y = 0;
		}
		else if (XY < 0.6)
		{
			pickdir.X = 0;
			pickdir.Y = 1;
		}
		else
		{
			pickdir.X = 2 * FRand() - 1;
			pickdir.Y = 2 * FRand() - 1;
			pickdir = Normal(pickdir);
		}
		
		success = TestDirection(pickdir, Destination, false);
		if (!success)
			success = TestDirection(-1 * pickdir, Destination, true);
		
		if ( !success )
		{
			Destination = Location;
			GotoState('Grazing', 'Turn');
		}
	}
	
	function AnimEnd()
	{
		local float decision;
		decision = FRand();
		if  ( decision < 0.6 )
			PlayAnim('Eat', 0.2 + 0.2 * FRand());
		else if ( decision < 0.8 )
		{
			PlaySound(sound'CallBn');
			PlayAnim('Call', 0.4 + 0.2 * FRand());
		}
		else
			PlayAnim('Looking', 0.2 + 0.3 * FRand());
	}

	function BeginState()
	{
		StartLocation = Location;
		bCanJump = false;
	}	
	function EndState()
	{
		bCanJump = true;
	}

Wander: 
	Disable('AnimEnd');
	WaitForLanding();
	PickDestination();
	TweenAnim('Jump', 0.2);
	FinishAnim();
	LoopAnim('Jump');
	
Moving:
	Enable('Bump');
	MoveTo(Destination, 0.4);
	Acceleration = vect(0,0,0);
Graze:
	TweenAnim('Eat', 0.2);
	Enable('AnimEnd');
	Sleep(6 + 6 * FRand());
	Disable('AnimEnd');
	FinishAnim();
	if ( !LineOfSightTo(Enemy) )
	{
		Enemy = None;
		Enable('SeePlayer');
		Goto('Begin');
	}
	Goto('Wander');

Turn:
	PlayTurning();
	TurnTo(Location + 20 * VRand());
	Goto('Graze');

Begin:
	WaitForLanding();
	SetPhysics(PHYS_None);
	TweenAnim('Eat', 0.2);
}

State Evade
{
	ignores SeePlayer;
	
	function EnemyNotVisible()
	{
		GotoState('Grazing');
	}
	
	function Bump(actor Other)
	{
		if ( (Normal(Destination - Location) Dot Normal(Other.Location - Location)) > 0.75 )
			MoveTimer = -1.0;
		if ( (Pawn(Other) != None) && (Pawn(Other).bIsPlayer || Other.IsA('ScriptedPawn')) )
			Enemy = Pawn(Other);

		Disable('Bump');
	}
	
	function bool TestDirection(vector dir, out vector pick)
	{	
		local vector HitLocation, HitNormal;
		local actor HitActor;

		HitActor = Trace(HitLocation, HitNormal, Location + dir * 150, Location, false);
		if ( (HitActor != None) && (HitNormal.Z < 0.7) )
		{
			pick = HitLocation - dir + (HitNormal Cross vect(0,0,1)) * 5 * CollisionRadius;
			HitActor = Trace(HitLocation, HitNormal, pick , Location, false);
			if (HitActor != None)
				return false;
		}
		else
			pick = Location + dir * (150 + FRand() * 450);

		return true; 
	}
	
	function PickDestination()
	{
		local vector pick, pickdir, enemyDir;
		local bool success;
		local float XY;
				
		enemyDir = Enemy.Location - Location;
		pickDir	= VRand();
		pickDir.Z = 0;
		if ( (pickDir Dot enemyDir) > 0 )
			pickDir *= -1;	
		success = TestDirection(pickdir, pick);
		if (!success)
		{
			pickDir	= VRand();
			pickDir.Z = 0;
			if ( (pickDir Dot enemyDir) > 0 )
				pickDir *= -1;	
			success = TestDirection( pickDir, pick);
		}		
		if (success)
			Destination = pick;
		else
		{
			Destination = Location;
			GotoState('Evade', 'Turn');
		}
	}

Begin:
	//log(class$" Evading");

Wander: 
	WaitForLanding();
	PickDestination();
	TurnTo(Destination);
	Falling();
	SetPhysics(PHYS_Falling);
	Velocity = GroundSpeed * Normal(Destination - Location);
	Velocity.Z = 360;
	WaitForLanding();
	FinishAnim();
	LoopAnim('Jump', 1.5);
	
Moving:
	Enable('Bump');
	MoveTo(Destination);
	Acceleration = vect(0,0,0);
Graze:
	if ( FRand() < 0.3 )
	{
		PlaySound(sound'CallBN');
		PlayAnim('Call', 0.4 + 0.2 * FRand(), 0.2);
	}
	else
		PlayAnim('Looking', 0.2 + 0.3 * FRand(), 0.2);
	FinishAnim();
	Goto('Wander');

Turn:
	PlayTurning();
	TurnTo(Location + 20 * VRand());
	Goto('Graze');
}



defaultproperties
{
	 bStayClose=true
     WanderRadius=+00350.000000
     SightRadius=+01000.000000
     PeripheralVision=-00010.000000
     UnderWaterTime=+00003.000000
     GroundSpeed=+00400.000000
     Accelrate=+01500.000000
     MinHitWall=-00000.100000
     JumpZ=+00190.000000
     DrawType=DT_Mesh
     Mesh=Unreal.Rabbit
     CollisionRadius=+00018.300000
     CollisionHeight=+00013.300000
	 Mass=+00020.000000
	 Buoyancy=+00021.000000
}

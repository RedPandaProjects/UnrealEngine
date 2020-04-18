//=============================================================================
// Eightball.
//=============================================================================
class Eightball expands Weapon;

#exec MESH IMPORT MESH=EightB ANIVFILE=MODELS\eightb_a.3D DATAFILE=MODELS\eightb_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=EightB X=0 Y=100 Z=-120 YAW=-64 ROLL=-8
#exec MESH SEQUENCE MESH=eightb SEQ=All      STARTFRAME=0   NUMFRAMES=65
#exec MESH SEQUENCE MESH=eightb SEQ=Select   STARTFRAME=0   NUMFRAMES=15 RATE=24 GROUP=Select
#exec MESH SEQUENCE MESH=eightb SEQ=Down     STARTFRAME=15  NUMFRAMES=12
#exec MESH SEQUENCE MESH=eightb SEQ=Idle     STARTFRAME=27  NUMFRAMES=2
#exec MESH SEQUENCE MESH=eightb SEQ=Idle2    STARTFRAME=29  NUMFRAMES=1
#exec MESH SEQUENCE MESH=eightb SEQ=Loading  STARTFRAME=30  NUMFRAMES=27
#exec MESH SEQUENCE MESH=eightb SEQ=Fire     STARTFRAME=57  NUMFRAMES=8
#exec TEXTURE IMPORT NAME=JEightB1 FILE=MODELS\eightbal.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=eightb X=0.005 Y=0.005 Z=0.01
#exec MESHMAP SETTEXTURE MESHMAP=eightb NUM=1 TEXTURE=Jeightb1

#exec MESH IMPORT MESH=EightPick ANIVFILE=MODELS\epick_a.3D DATAFILE=MODELS\epick_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=EightPick X=0 Y=170 Z=0 YAW=64
#exec MESH SEQUENCE MESH=eightpick SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=eightpick SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JEightB1 FILE=MODELS\eightbal.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=eightpick X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=eightpick NUM=1 TEXTURE=Jeightb1

// 3rd person perspective version
#exec MESH IMPORT MESH=8Ball3rd ANIVFILE=MODELS\8ball3_a.3D DATAFILE=MODELS\8ball3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=8Ball3rd X=0 Y=-430 Z=-45 YAW=-64 ROLL=9
#exec MESH SEQUENCE MESH=8Ball3rd SEQ=All  STARTFRAME=0  NUMFRAMES=10
#exec MESH SEQUENCE MESH=8Ball3rd SEQ=Idle  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=8Ball3rd SEQ=Fire  STARTFRAME=1  NUMFRAMES=9
#exec TEXTURE IMPORT NAME=JEightB1 FILE=MODELS\eightbal.PCX GROUP="Skins"
#exec OBJ LOAD FILE=textures\FireEffect18.utx PACKAGE=Unreal.Effect18
#exec MESHMAP SCALE MESHMAP=8Ball3rd X=0.065 Y=0.065 Z=0.13
#exec MESHMAP SETTEXTURE MESHMAP=8Ball3rd NUM=1 TEXTURE=JEightB1
#exec MESHMAP SETTEXTURE MESHMAP=8Ball3rd NUM=0 TEXTURE=Unreal.Effect18.FireEffect18

#exec AUDIO IMPORT FILE="Sounds\eightbal\8ALTF1.WAV" NAME="EightAltFire" GROUP="EightBall"
#exec AUDIO IMPORT FILE="Sounds\eightbal\Barrelm1.WAV" NAME="BarrelMove" GROUP="EightBall"
#exec AUDIO IMPORT FILE="Sounds\eightbal\Eload1.WAV" NAME="Loading" GROUP="EightBall"
#exec AUDIO IMPORT FILE="Sounds\eightbal\Lock1.WAV" NAME="SeekLock" GROUP="EightBall"
#exec AUDIO IMPORT FILE="Sounds\eightbal\SeekLost.WAV" NAME="SeekLost" GROUP="EightBall"
#exec AUDIO IMPORT FILE="Sounds\eightbal\Select.WAV" NAME="Selecting" GROUP="EightBall"

#exec MESH NOTIFY MESH=Eightb SEQ=Loading TIME=0.45 FUNCTION=BarrelTurn

var int RocketsLoaded, RocketRad;
var bool bFireLoad,bTightWad;
var Actor LockedTarget, NewTarget, OldTarget;

function PreBeginPlay()
{
	Super.PreBeginPlay();
	ProjectileClass = class'Rocket';
	AltProjectileClass = class'Grenade';
	ProjectileSpeed = class'Rocket'.Default.speed;
	AltProjectileSpeed = class'Grenade'.Default.speed;
}

function float RateSelf( out int bUseAltMode )
{
	local float EnemyDist;
	local bool bRetreating;
	local vector EnemyDir;

	if ( AmmoType.AmmoAmount <=0 )
		return -2;
	if ( Pawn(Owner).Enemy == None )
	{
		bUseAltMode = 0;
		return AIRating;
	}

	EnemyDir = Pawn(Owner).Enemy.Location - Owner.Location; 
	EnemyDist = VSize(EnemyDir);
	if ( EnemyDist < 270 )
	{
		bUseAltMode = 0;
		return -0.1;
	}

	if ( EnemyDist < -1.5 * EnemyDir.Z )
		bUseAltMode = int( FRand() < 0.5 );
	else if ( Pawn(Owner).Location.Z < Pawn(Owner).Enemy.Location.Z )
		bUseAltMode = 0;
	else
	{
		bRetreating = ( ((EnemyDir/EnemyDist) Dot Owner.Velocity) < -0.7 );
		bUseAltMode = 0;
		if ( ((EnemyDist < 600) || (bRetreating && (EnemyDist < 800)))
			&& (FRand() < 0.4) )
			bUseAltMode = 1;
	}
	return AIRating;
}

// return delta to combat style
function float SuggestAttackStyle()
{
	local float EnemyDist;

	EnemyDist = VSize(Pawn(Owner).Enemy.Location - Owner.Location);
	if ( EnemyDist < 400 )
		return -0.6;
	else
		return -0.2;
}

function BarrelTurn()
{
	Owner.PlaySound(Misc3Sound, SLOT_None, 0.1*Pawn(Owner).SoundDampening);
}

function Fire( float Value )
{
	//bFireMem = false;
	//bAltFireMem = false;
	bPointing=True;
	CheckVisibility();
	if ( AmmoType.UseAmmo(1) )
		GoToState('NormalFire');
}

function AltFire( float Value )
{
	//bFireMem = false;
	//bAltFireMem = false;
	bPointing=True;
	CheckVisibility();
	if ( AmmoType.UseAmmo(1) )
		GoToState('AltFiring');
}

function Actor CheckTarget()
{
	local Actor ETarget;
	local Vector Start, X,Y,Z;
	local float bestDist, bestAim;
	local Pawn PawnOwner;

	PawnOwner = Pawn(Owner);
	if ( !PawnOwner.bIsPlayer && (PawnOwner.Enemy == None) )
		return None; 
	GetAxes(PawnOwner.ViewRotation,X,Y,Z);
	Start = Owner.Location + CalcDrawOffset() + FireOffset.X * X + FireOffset.Y * Y + FireOffset.Z * Z; 
	bestAim = 0.93;
	ETarget = PawnOwner.PickTarget(bestAim, bestDist, X, Start);
	if ( !PawnOwner.bIsPlayer && (PawnOwner.Enemy != ETarget) )
		return None; 
	bPointing = (ETarget != None);
	Return ETarget;
}

//////////////////////////////////////////////////////
state AltFiring
{
	function Tick( float DeltaTime )
	{
		if( (pawn(Owner).bAltFire==0) || (RocketsLoaded > 5) )  // If if Fire button down, load up another
 			GoToState('FireRockets');
	}

	function BeginState()
	{
		RocketsLoaded = 1;
		bFireLoad = False;
	}

Begin:
	bLockedOn = False;
	While ( RocketsLoaded < 6 )
	{
		if (AmmoType.AmmoAmount<=0) GoToState('FireRockets');		
		Owner.PlaySound(CockingSound, SLOT_None, Pawn(Owner).SoundDampening);		
		PlayAnim('Loading', 1.1,0.0);
		FinishAnim();
		RocketsLoaded++;
		AmmoType.UseAmmo(1);		
		if ( (PlayerPawn(Owner) == None) && ((FRand() > 0.5) || (Pawn(Owner).Enemy == None)) )
			Pawn(Owner).bAltFire = 0;
	}
}

///////////////////////////////////////////////////////
state NormalFire
{
	function Tick( float DeltaTime )
	{
		if( pawn(Owner).bFire==0 || RocketsLoaded > 5)  // If if Fire button down, load up another
 			GoToState('FireRockets');
	}

	function BeginState()
	{
		bFireLoad = True;
		RocketsLoaded = 1;
		Super.BeginState();
	}

Begin:
	While ( RocketsLoaded < 6 )
	{
		if (AmmoType.AmmoAmount<=0) GoToState('FireRockets');			
		Owner.PlaySound(CockingSound, SLOT_None, Pawn(Owner).SoundDampening);	
		PlayAnim('Loading', 1.1,0.0);
		FinishAnim();
		if (pawn(Owner).bAltFire!=0) bTightWad=True;
		NewTarget = CheckTarget();
		if ( Pawn(NewTarget) != None )
			Pawn(NewTarget).WarnTarget(Pawn(Owner), ProjectileSpeed, vector(Pawn(Owner).ViewRotation));	
		If ( (LockedTarget != None) && (NewTarget != LockedTarget) ) 
		{
			LockedTarget = None;
			Owner.PlaySound(Misc2Sound, SLOT_None, Pawn(Owner).SoundDampening);
			bLockedOn=False;
		}
		else if (LockedTarget != None)
 			Owner.PlaySound(Misc1Sound, SLOT_None, Pawn(Owner).SoundDampening);
		bPointing = true;
		if ( Level.Game.Difficulty > 0 )
			MakeNoise(0.15 * Level.Game.Difficulty * Pawn(Owner).SoundDampening);		
		RocketsLoaded++;
		AmmoType.UseAmmo(1);
		if ( (PlayerPawn(Owner) == None) && ((FRand() > 0.2) || (Pawn(Owner).Enemy == None)) )
			Pawn(Owner).bFire = 0;
	}
}

///////////////////////////////////////////////////////
state Idle
{
	function Timer()
	{
		NewTarget = CheckTarget();
		if ( NewTarget == OldTarget )
		{
			LockedTarget = NewTarget;
			If (LockedTarget != None) 
			{
				bLockedOn=True;			
				Owner.MakeNoise(Pawn(Owner).SoundDampening);
				Owner.PlaySound(Misc1Sound, SLOT_None,Pawn(Owner).SoundDampening);
				if ( (Pawn(LockedTarget) != None) && (FRand() < 0.7) )
					Pawn(LockedTarget).WarnTarget(Pawn(Owner), ProjectileSpeed, vector(Pawn(Owner).ViewRotation));	
			}
		}
		else if( (OldTarget != None) && (NewTarget == None) ) 
		{
			Owner.PlaySound(Misc2Sound, SLOT_None,Pawn(Owner).SoundDampening);
			bLockedOn = False;
		}
		else 
		{
			LockedTarget = None;
			bLockedOn = False;
		}
		OldTarget = NewTarget;
	}
Begin:
	if (Pawn(Owner).bFire!=0) Fire(0.0);
	if (Pawn(Owner).bAltFire!=0) AltFire(0.0);	
	bPointing=False;
	if (AmmoType.AmmoAmount<=0) 
		Pawn(Owner).SwitchToBestWeapon();  //Goto Weapon that has Ammo
	LoopAnim('Idle', 0.01,0.4);
	OldTarget = CheckTarget();
	SetTimer(1.25,True);
	LockedTarget = None;
	bLockedOn = False;
}

///////////////////////////////////////////////////////
state FireRockets
{
	function Fire(float F) {}
	function AltFire(float F) {}

	function BeginState()
	{
		local vector FireLocation, StartLoc, X,Y,Z;
		local rotator FireRot;
		local rocket r;
		local grenade g;
		local float Angle;
		local pawn BestTarget;
		local int DupRockets;

		Angle = 0;
		DupRockets = RocketsLoaded - 1;
		if (DupRockets < 0) DupRockets = 0;
		if ( PlayerPawn(Owner) != None )
			PlayerPawn(Owner).shakeview(ShakeTime, ShakeMag*RocketsLoaded, ShakeVert); //shake player view
		else
			bTightWad = ( FRand() * 4 < Pawn(Owner).skill );

		GetAxes(Pawn(Owner).ViewRotation,X,Y,Z);
		StartLoc = Owner.Location + CalcDrawOffset(); 
		FireLocation = StartLoc + FireOffset.X * X + FireOffset.Y * Y + FireOffset.Z * Z; 
		if ( bFireLoad ) 		
			AdjustedAim = pawn(owner).AdjustAim(ProjectileSpeed, FireLocation, AimError, True, bWarnTarget);
		else 
			AdjustedAim = pawn(owner).AdjustToss(AltProjectileSpeed, FireLocation, AimError, True, bAltWarnTarget);	
			
		if ( PlayerPawn(Owner) != None )
			AdjustedAim = Pawn(Owner).ViewRotation;
				
		PlayAnim( 'Fire', 0.6, 0.05);	
		Owner.MakeNoise(Pawn(Owner).SoundDampening);
		if ( (LockedTarget!=None) || !bFireLoad )
		{
			BestTarget = Pawn(CheckTarget());
			if ( (LockedTarget!=None) && (LockedTarget != BestTarget) ) 
			{
				LockedTarget = None;
				bLockedOn=False;
			}
		}
		else 
			BestTarget = None;
		bPointing = true;
		FireRot = AdjustedAim;
		RocketRad = 4;
		if (bTightWad || !bFireLoad) RocketRad=7;
		While ( RocketsLoaded > 0 )
		{
			Firelocation = StartLoc - Sin(Angle)*Y*RocketRad + (Cos(Angle)*RocketRad - 10.78)*Z + X * (10 + 8 * FRand());
			if (bFireLoad)
			{
				if ( Angle > 0 )
				{
					if ( Angle < 3 && !bTightWad)
						FireRot.Yaw = AdjustedAim.Yaw - Angle * 600;
					else if ( Angle > 3.5 && !bTightWad)
						FireRot.Yaw = AdjustedAim.Yaw + (Angle - 3)  * 600;
					else
						FireRot.Yaw = AdjustedAim.Yaw;
				}
				if ( LockedTarget!=None )
				{
					r = Spawn( class 'SeekingRocket',, '', FireLocation,FireRot);	
					r.Seeking = LockedTarget;
					r.NumExtraRockets = DupRockets;					
				}
				else 
				{
					r = Spawn( class 'Rocket',, '', FireLocation,FireRot);
					r.NumExtraRockets = DupRockets;
					if (RocketsLoaded>5 && bTightWad) r.bRing=True;
				}
				if ( Angle > 0 )
					r.Velocity *= (0.9 + 0.2 * FRand());			
			}
			else 
			{
				g = Spawn( class 'Grenade',, '', FireLocation,AdjustedAim);
				g.WarnTarget = ScriptedPawn(BestTarget);
				g.NumExtraGrenades = DupRockets;
				Owner.PlaySound(AltFireSound, SLOT_None, 3.0*Pawn(Owner).SoundDampening);				
			}

			Angle += 1.0484; //2*3.1415/6;
			RocketsLoaded--;
		}
		bTightWad=False;
		//bFireMem = false;
		//bAltFireMem = false;		
	}

Begin:
	FinishAnim();
	if (AmmoType.AmmoAmount > 0) 
	{	
		Owner.PlaySound(CockingSound, SLOT_None,Pawn(Owner).SoundDampening);		
		PlayAnim('Loading', 1.5,0.0);	
		FinishAnim();		
		RocketsLoaded = 1;
	}
	LockedTarget = None;
	Finish();	
}

defaultproperties
{
     AmmoName=Class'Unreal.RocketCan'
     PickupAmmoCount=6
     bWarnTarget=True
     bAltWarnTarget=True
     bSplashDamage=True
     shakemag=350.000000
     shaketime=0.200000
     shakevert=7.500000
     AIRating=0.700000
     RefireRate=0.250000
     AltRefireRate=0.250000
     AltFireSound=Sound'Unreal.Eightball.EightAltFire'
     CockingSound=Sound'Unreal.Eightball.Loading'
     SelectSound=Sound'Unreal.Eightball.Selecting'
     Misc1Sound=Sound'Unreal.Eightball.SeekLock'
     Misc2Sound=Sound'Unreal.Eightball.SeekLost'
     Misc3Sound=Sound'Unreal.Eightball.BarrelMove'
     AutoSwitchPriority=5
     InventoryGroup=5
     PickupMessage="You got the Eightball gun"
     PlayerViewOffset=(X=1.900000,Z=-1.890000)
     PlayerViewMesh=Mesh'Unreal.EightB'
     BobDamping=0.985000
     PickupViewMesh=Mesh'Unreal.EightPick'
     ThirdPersonMesh=Mesh'Unreal.8Ball3rd'
     PickupSound=Sound'Unreal.Pickups.WeaponPickup'
     Mesh=Mesh'Unreal.EightPick'
     bNoSmooth=False
     bMeshCurvy=False
     CollisionHeight=10.000000
}

//=============================================================================
// GESBioRifle.
//=============================================================================
class GESBioRifle expands Weapon;

#exec MESH IMPORT MESH=BRifle ANIVFILE=MODELS\BRifle_a.3D DATAFILE=MODELS\BRifle_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=BRifle X=0 Y=0 Z=0 YAW=-64 ROLL=0 PITCH=0
#exec MESH SEQUENCE MESH=BRifle SEQ=All       STARTFRAME=0   NUMFRAMES=101
#exec MESH SEQUENCE MESH=BRifle SEQ=Select    STARTFRAME=0   NUMFRAMES=20 RATE=45 GROUP=Select
#exec MESH SEQUENCE MESH=BRifle SEQ=Still     STARTFRAME=20  NUMFRAMES=1
#exec MESH SEQUENCE MESH=BRifle SEQ=Walking   STARTFRAME=21  NUMFRAMES=20
#exec MESH SEQUENCE MESH=BRifle SEQ=Charging  STARTFRAME=41  NUMFRAMES=30
#exec MESH SEQUENCE MESH=BRifle SEQ=UnLoading STARTFRAME=71  NUMFRAMES=1
#exec MESH SEQUENCE MESH=BRifle SEQ=Fire      STARTFRAME=72  NUMFRAMES=9
#exec MESH SEQUENCE MESH=BRifle SEQ=Drip      STARTFRAME=81  NUMFRAMES=7
#exec MESH SEQUENCE MESH=BRifle SEQ=Down      STARTFRAME=88  NUMFRAMES=13
#exec TEXTURE IMPORT NAME=JBRifle1 FILE=MODELS\BRifle.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=BRifle  X=0.005 Y=0.005 Z=0.01
#exec MESHMAP SETTEXTURE MESHMAP=BRifle NUM=1 TEXTURE=JBRifle1

#exec MESH IMPORT MESH=BRiflePick ANIVFILE=MODELS\napick_a.3D DATAFILE=MODELS\napick_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=BRiflePick X=0 Y=0 Z=63 YAW=64
#exec MESH SEQUENCE MESH=BRiflepick SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=BRiflepick SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JBRifle1 FILE=MODELS\BRifle.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=BRiflepick X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=BRiflepick NUM=1 TEXTURE=JBRifle1

#exec MESH IMPORT MESH=BRifle3 ANIVFILE=MODELS\Napal3_a.3D DATAFILE=MODELS\Napal3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=BRifle3 X=0 Y=-480 Z=-80 YAW=-64 ROLL=9
#exec MESH SEQUENCE MESH=BRifle3 SEQ=All  STARTFRAME=0  NUMFRAMES=15
#exec MESH SEQUENCE MESH=BRifle3 SEQ=Still  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=BRifle3 SEQ=Charging  STARTFRAME=1  NUMFRAMES=9 RATE=40.0
#exec MESH SEQUENCE MESH=BRifle3 SEQ=Fire  STARTFRAME=9  NUMFRAMES=5 RATE=40.0
#exec TEXTURE IMPORT NAME=JBRifle1 FILE=MODELS\BRifle.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=BRifle3 X=0.035 Y=0.035 Z=0.07
#exec MESHMAP SETTEXTURE MESHMAP=BRifle3 NUM=1 TEXTURE=JBRifle1

#exec MESH NOTIFY MESH=BRifle SEQ=Drip TIME=0.4 FUNCTION=DripSound

#exec AUDIO IMPORT FILE="Sounds\BRifle\NPickup2.WAV" NAME="GelSelect" GROUP="BioRifle"
#exec AUDIO IMPORT FILE="Sounds\BRifle\Load4.WAV" NAME="GelLoad" GROUP="BioRifle"
#exec AUDIO IMPORT FILE="Sounds\BRifle\NShot1.WAV" NAME="GelShot" GROUP="BioRifle"
#exec AUDIO IMPORT FILE="Sounds\BRifle\Drip1.WAV" NAME="GelDrip" GROUP="BioRifle"
#exec AUDIO IMPORT FILE="Sounds\BRifle\GelHit1.WAV" NAME="GelHit" GROUP="BioRifle"

var float ChargeSize,Count;

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
	if ( EnemyDist > 1400 )
	{
		bUseAltMode = 0;
		return 0;
	}
	bRetreating = ( ((EnemyDir/EnemyDist) Dot Owner.Velocity) < -0.7 );
	if ( (EnemyDist > 500) && (EnemyDir.Z > -0.4 * EnemyDist) )
	{
		// only use if enemy not too far and retreating
		if ( (EnemyDist > 800) || !bRetreating )
		{
			bUseAltMode = 0;
			return 0;
		}
		return AIRating;
	}

	bUseAltMode = int( bRetreating && (FRand() < 0.3) );

	if ( bRetreating || (EnemyDir.Z < -0.7 * EnemyDist) )
		return (AIRating + 0.15);
	return AIRating;
}

// return delta to combat style
function float SuggestAttackStyle()
{
	return -0.3;
}

function float SuggestDefenseStyle()
{
	return -0.2;
}

function PreBeginPlay()
{
	Super.PreBeginPlay();
	ProjectileClass = class'BioGel';
	ProjectileSpeed = class'biogel'.default.speed;
	AltProjectileClass = class'BigBioGel';
	AltProjectileSpeed = class'BigBioGel'.default.speed;
}

function AltFire( float Value )
{
	//bFireMem = false;
	//bAltFireMem = false;
	bPointing=True;
	if ( AmmoType.UseAmmo(1) ) 
	{
		CheckVisibility();
		GoToState('AltFiring');
	}
	else
		GoToState('Idle');		
}

function Projectile ProjectileFire(class<projectile> ProjClass, float ProjSpeed, bool bWarn)
{
	local Vector Start, X,Y,Z;

	Owner.MakeNoise(Pawn(Owner).SoundDampening);
	GetAxes(Pawn(owner).ViewRotation,X,Y,Z);
	Start = Owner.Location + CalcDrawOffset() + FireOffset.X * X + FireOffset.Y * Y + FireOffset.Z * Z; 
	AdjustedAim = pawn(owner).AdjustToss(ProjSpeed, Start, 0, True, (bWarn || (FRand() < 0.4)));	
	return Spawn(ProjClass,,, Start,AdjustedAim);
}

///////////////////////////////////////////////////////
state AltFiring
{
	function Tick( float DeltaTime )
	{
		ChargeSize += DeltaTime;
		if( (pawn(Owner).bAltFire==0)) 
			GoToState('ShootLoad');
		Count += DeltaTime;
		if (Count > 1.0) 
		{
			Count = 0.0;
			if ( (PlayerPawn(Owner) == None) && (FRand() < 0.3) )
				GoToState('ShootLoad');
			else if (!AmmoType.UseAmmo(1)) 
				GoToState('ShootLoad');
		}
	}

Begin:
	ChargeSize = 0.0;
	Count = 0.0;
	Owner.PlaySound(Misc1Sound, SLOT_Misc, 1.3*Pawn(Owner).SoundDampening);	 //loading goop	
	PlayAnim('Charging',0.2,0.05);
	FinishAnim();
	GotoState('ShootLoad');
}

state ShootLoad
{
	function Fire(float F) 
	{
		//bFireMem = true;
	}
	function AltFire(float F) 
	{
		//bAltFireMem = true;
	}

	function BeginState()
	{
		Local Projectile Gel;

		Gel = ProjectileFire(AltProjectileClass, AltProjectileSpeed, bAltWarnTarget);
		Gel.DrawScale = 0.5 + ChargeSize/3.5;
		Owner.PlaySound(FireSound, SLOT_Misc, 1.7*Pawn(Owner).SoundDampening,,,fMax(0.5,1.35-ChargeSize/8.0) );	//shoot goop
		PlayAnim('Fire',0.4, 0.05);
	}

Begin:
	FinishAnim();
	Finish();
}


// Finish a firing sequence
function Finish()
{
	if ( bChangeWeapon )
		GotoState('DownWeapon');
	else if ( PlayerPawn(Owner) == None )
	{
		Pawn(Owner).bAltFire = 0;
		Super.Finish();
	}
	else if ( (AmmoType.AmmoAmount<=0) || (Pawn(Owner).Weapon != self) )
		GotoState('Idle');
	else if ( /*bFireMem ||*/ Pawn(Owner).bFire!=0 )
		Global.Fire(0);
	else 
		GotoState('Idle');
}


function PlayFiring()
{
	Owner.PlaySound(AltFireSound, SLOT_None, 1.7*Pawn(Owner).SoundDampening);	//fast fire goop
	PlayAnim('Fire',1.1, 0.05);
}
///////////////////////////////////////////////////////////
function PlayIdleAnim()
{
	if (VSize(Owner.Velocity) > 10)
		PlayAnim('Walking',0.3,0.3);
	else if (FRand() < 0.3 )
		PlayAnim('Drip', 0.1,0.3);
	else 
		TweenAnim('Still', 1.0);
	Enable('AnimEnd');
}

function DripSound()
{
	Owner.PlaySound(Misc2Sound, SLOT_None, 0.5*Pawn(Owner).SoundDampening);	// Drip
}

defaultproperties
{
     AmmoName=Class'Unreal.Sludge'
     PickupAmmoCount=25
     bAltWarnTarget=True
     FireOffset=(X=12.000000,Y=-9.000000,Z=-16.000000)
     AIRating=0.600000
     RefireRate=0.900000
     AltRefireRate=0.700000
     FireSound=Sound'Unreal.GelShot'
     AltFireSound=Sound'Unreal.GelShot'
     CockingSound=Sound'Unreal.GelLoad'
     SelectSound=Sound'Unreal.GelSelect'
     Misc1Sound=Sound'Unreal.GelLoad'
     Misc2Sound=Sound'Unreal.GelDrip'
     M_NoAmmo=""
     AutoSwitchPriority=8
     InventoryGroup=8
     PickupMessage="You got the GES BioRifle"
     PlayerViewOffset=(X=2.000000,Y=-0.700000,Z=-1.150000)
     PlayerViewMesh=Mesh'Unreal.BRifle'
     PickupViewMesh=Mesh'Unreal.BRiflePick'
     ThirdPersonMesh=Mesh'Unreal.BRifle3'
     PickupSound=Sound'Unreal.WeaponPickup'
     Mesh=Mesh'Unreal.BRiflePick'
     bNoSmooth=False
     bMeshCurvy=False
     CollisionRadius=28.000000
     CollisionHeight=15.000000
}

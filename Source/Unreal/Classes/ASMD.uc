//=============================================================================
// ASMD.
//=============================================================================
class ASMD expands Weapon;

#exec MESH IMPORT MESH=ASMDM ANIVFILE=MODELS\ASMD_a.3D DATAFILE=MODELS\ASMD_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=ASMDM X=0 Y=0 Z=0 YAW=-64 PITCH=0
#exec MESH SEQUENCE MESH=ASMDM SEQ=All       STARTFRAME=0  NUMFRAMES=80
#exec MESH SEQUENCE MESH=ASMDM SEQ=Select    STARTFRAME=0  NUMFRAMES=14 RATE=27 GROUP=Select
#exec MESH SEQUENCE MESH=ASMDM SEQ=Still     STARTFRAME=14 NUMFRAMES=2
#exec MESH SEQUENCE MESH=ASMDM SEQ=Down      STARTFRAME=16 NUMFRAMES=10
#exec MESH SEQUENCE MESH=ASMDM SEQ=Still2    STARTFRAME=26 NUMFRAMES=2
#exec MESH SEQUENCE MESH=ASMDM SEQ=Fire1     STARTFRAME=28 NUMFRAMES=9  RATE=24
#exec MESH SEQUENCE MESH=ASMDM SEQ=Fire2     STARTFRAME=37 NUMFRAMES=10
#exec MESH SEQUENCE MESH=ASMDM SEQ=Steam     STARTFRAME=47 NUMFRAMES=10
#exec MESH SEQUENCE MESH=ASMDM SEQ=Cocking   STARTFRAME=57 NUMFRAMES=19
#exec MESH SEQUENCE MESH=ASMDM SEQ=Sway      STARTFRAME=76 NUMFRAMES=4
#exec TEXTURE IMPORT NAME=ASMD1 FILE=MODELS\ASMD.PCX GROUP="Skins"
#exec OBJ LOAD FILE=Textures\SmokeEffect3.utx PACKAGE=Unreal.SEffect3
#exec MESHMAP SCALE MESHMAP=ASMDM X=0.007 Y=0.005 Z=0.01
#exec MESHMAP SETTEXTURE MESHMAP=ASMDM NUM=1 TEXTURE=ASMD1
#exec MESHMAP SETTEXTURE MESHMAP=ASMDM NUM=0 TEXTURE=Unreal.SEffect3.SmokeEffect3

#exec MESH IMPORT MESH=ASMDPick ANIVFILE=MODELS\Tazlo_a.3D DATAFILE=MODELS\Tazlo_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=ASMDPick X=0 Y=-10 Z=-13 YAW=-64 PITCH=0
#exec MESH SEQUENCE MESH=ASMDPick SEQ=All   STARTFRAME=0  NUMFRAMES=6
#exec TEXTURE IMPORT NAME=ASMD1 FILE=MODELS\ASMD.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=ASMDPick X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=ASMDPick NUM=1 TEXTURE=ASMD1

#exec MESH IMPORT MESH=ASMD3 ANIVFILE=MODELS\Tazlo_a.3D DATAFILE=MODELS\Tazlo_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=ASMD3 X=0 Y=-190 Z=-40 YAW=-64 ROLL=9
#exec MESH SEQUENCE MESH=ASMD3 SEQ=All   STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=ASMD3 SEQ=Still STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=ASMD3 SEQ=Fire1 STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=ASMD1 FILE=MODELS\ASMD.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=ASMD3 X=0.065 Y=0.065 Z=0.13
#exec MESHMAP SETTEXTURE MESHMAP=ASMD3 NUM=1 TEXTURE=ASMD1

#exec AUDIO IMPORT FILE="Sounds\Tazer\TSHOTA6.WAV" NAME="TazerFire" GROUP="ASMD"
#exec AUDIO IMPORT FILE="Sounds\Tazer\TSHOTB1.WAV" NAME="TazerAltFire" GROUP="ASMD"
#exec AUDIO IMPORT FILE="Sounds\Tazer\TPICKUP3.WAV" NAME="TazerSelect" GROUP="ASMD"
#exec AUDIO IMPORT FILE="Sounds\Tazer\Vapour1.WAV" NAME="Vapour" GROUP="ASMD"

var() int HitDamage;
var Pickup Amp;

function float RateSelf( out int bUseAltMode )
{
	local float EnemyDist;
	local float rating;

	if ( Amp != None )
		rating = 2 * AIRating;
	else 
		rating = AIRating;

	if ( AmmoType.AmmoAmount <=0 )
		return -2;
	if ( Pawn(Owner).Enemy == None )
	{
		bUseAltMode = 0;
		return rating;
	}

	EnemyDist = VSize(Pawn(Owner).Enemy.Location - Owner.Location);
	if ( EnemyDist > 800 )
	{
		bUseAltMode = 0;
		return rating;
	}
	
	bUseAltMode = int( FRand() < 0.4 );
	return rating;
}

function BecomePickup()
{
	Amp = None;
	Super.BecomePickup();
}

function PreBeginPlay()
{
	Super.PreBeginPlay();
	AltProjectileClass = class'TazerProj';
	AltProjectileSpeed = class'TazerProj'.Default.speed;
}
 

function Timer()
{
	local actor targ;
	local float bestAim, bestDist;
	local vector FireDir;

	bestAim = 0.95;
	if ( Pawn(Owner) == None )
	{
		GotoState('');
		return;
	}
	FireDir = vector(Pawn(Owner).ViewRotation);
	targ = Pawn(Owner).PickTarget(bestAim, bestDist, FireDir, Owner.Location);
	if ( Pawn(targ) != None )
	{
		bPointing = true;
		Pawn(targ).WarnTarget(Pawn(Owner), 300, FireDir);
		SetTimer(1 + 4 * FRand(), false);
	}
	else 
	{
		SetTimer(0.5 + 2 * FRand(), false);
		bPointing = false;
	}
}	

function Finish()
{
	if ( /*(bFireMem ||*/ (Pawn(Owner).bFire!=0) && (FRand() < 0.6) )
		Timer();
	Super.Finish();
}

///////////////////////////////////////////////////////
function PlayFiring()
{
	Owner.PlaySound(FireSound, SLOT_None, Pawn(Owner).SoundDampening*4.0);
	PlayAnim('Fire1', 0.5,0.05);
}

function Projectile ProjectileFire(class<projectile> ProjClass, float ProjSpeed, bool bWarn)
{
	local Vector Start, X,Y,Z;
	local float Mult;
	local Projectile t;

	if (Amp!=None) Mult = Amp.UseCharge(80);
	else Mult=1.0;

	Owner.MakeNoise(Pawn(Owner).SoundDampening);
	GetAxes(Pawn(owner).ViewRotation,X,Y,Z);
	Start = Owner.Location + CalcDrawOffset() + FireOffset.X * X + FireOffset.Y * Y + FireOffset.Z * Z; 
	AdjustedAim = pawn(owner).AdjustAim(ProjSpeed, Start, AimError, True, bWarn);	
	t = Spawn(ProjClass,,, Start,AdjustedAim);
	t.Damage = t.Damage*Mult;
}


function ProcessTraceHit(Actor Other, Vector HitLocation, Vector HitNormal, Vector X, Vector Y, Vector Z)
{
	local vector SmokeLocation,DVector;
	local rotator SmokeRotation;
	local float NumPoints,Mult;
	local int i;

	if (Other==None)
	{
		HitNormal = -X;
		HitLocation = Owner.Location + X*10000.0;
	}

	if (Amp!=None) Mult = Amp.UseCharge(100);
	else Mult=1.0;

	if ( TazerProj(Other)!=None )
	{ 
		AmmoType.UseAmmo(2);
		TazerProj(Other).SuperExplosion();
	}
	else
	{
		if (Mult>1.5) Spawn(class'RingExplosion3',,, HitLocation+HitNormal*8,rotator(HitNormal));
		else Spawn(class'RingExplosion',,, HitLocation+HitNormal*8,rotator(HitNormal));
	}

	SmokeLocation = Owner.Location + CalcDrawOffset() + FireOffset.X * X + FireOffset.Y * 3.3 * Y + FireOffset.Z * Z * 3.0;
	DVector = HitLocation - SmokeLocation;
	NumPoints = VSize(DVector)/70.0;
	SmokeLocation += DVector/NumPoints;
	SmokeRotation = rotator(HitLocation-Pawn(Owner).Location);
	if (NumPoints>15) NumPoints=15;
	if ( NumPoints>1.0 ) SpawnEffect(DVector, NumPoints, SmokeRotation, SmokeLocation);
	
	if ( (Other != self) && (Other != Owner) ) 
		Other.TakeDamage(HitDamage*Mult, Pawn(Owner), HitLocation, 50000.0*X, 'jolted');
}


simulated function SpawnEffect(Vector DVector, int NumPoints, rotator SmokeRotation, vector SmokeLocation)
{
	local RingExplosion4 Smoke;
	
	Smoke = Spawn(class'RingExplosion4',, '', SmokeLocation,SmokeRotation);
	Smoke.MoveAmount = DVector/NumPoints;
	Smoke.NumPuffs = NumPoints;
}

function PlayAltFiring()
{
	Owner.PlaySound(AltFireSound, SLOT_None,Pawn(Owner).SoundDampening*4.0);
	PlayAnim('Fire1',0.8,0.05);
}

function PlayIdleAnim()
{
	if ( AnimSequence == 'Fire1' && FRand()<0.2)
	{
		Owner.PlaySound(Misc1Sound, SLOT_None, Pawn(Owner).SoundDampening*0.5);	
		PlayAnim('Steam',0.1,0.4);
	}
	else if ( VSize(Owner.Velocity) > 20 )
	{
		if ( AnimSequence=='Still' )
			LoopAnim('Sway',0.1,0.3);
	}
	else if ( AnimSequence!='Still' ) 
	{
		if (FRand()<0.5) 
		{
			PlayAnim('Steam',0.1,0.4);
			Owner.PlaySound(Misc1Sound, SLOT_None, Pawn(Owner).SoundDampening*0.5);			
		}
		else LoopAnim('Still',0.04,0.3);
	}
	Enable('AnimEnd');
}

state Idle
{

	function BeginState()
	{
		bPointing = false;
		SetTimer(0.5 + 2 * FRand(), false);
		Super.BeginState();
		if (Pawn(Owner).bFire!=0) Fire(0.0);
		if (Pawn(Owner).bAltFire!=0) AltFire(0.0);		
	}

	function EndState()
	{
		SetTimer(0.0, false);
		Super.EndState();
	}
}

defaultproperties
{
     hitdamage=30
     AmmoName=Class'Unreal.ASMDAmmo'
     PickupAmmoCount=20
     bInstantHit=True
     bAltWarnTarget=True
     FireOffset=(X=12.000000,Y=-6.000000,Z=-7.000000)
     AIRating=0.600000
     AltRefireRate=0.700000
     FireSound=Sound'Unreal.ASMD.TazerFire'
     AltFireSound=Sound'Unreal.ASMD.TazerAltFire'
     SelectSound=Sound'Unreal.ASMD.TazerSelect'
     Misc1Sound=Sound'Unreal.ASMD.Vapour'
     AutoSwitchPriority=4
     InventoryGroup=4
     PickupMessage="You got the ASMD"
     PlayerViewOffset=(X=3.500000,Y=-1.800000,Z=-2.000000)
     PlayerViewMesh=Mesh'Unreal.ASMDM'
     PickupViewMesh=Mesh'Unreal.ASMDPick'
     ThirdPersonMesh=Mesh'Unreal.ASMD3'
     PickupSound=Sound'Unreal.Pickups.WeaponPickup'
     Mesh=Mesh'Unreal.ASMDPick'
     bNoSmooth=False
     bMeshCurvy=False
     CollisionRadius=28.000000
     CollisionHeight=8.000000
     Mass=50.000000
}

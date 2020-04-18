//=============================================================================
// DispersionPistol.
//=============================================================================
class DispersionPistol expands Weapon;

// player view version
#exec MESH IMPORT MESH=DPistol ANIVFILE=MODELS\dgun_a.3D DATAFILE=MODELS\dgun_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=DPistol X=0 Y=0 Z=0 YAW=-64 PITCH=0
#exec MESHMAP SCALE MESHMAP=DPistol X=0.005 Y=0.005 Z=0.01
#exec MESH SEQUENCE MESH=DPistol SEQ=All     STARTFRAME=0  NUMFRAMES=141
#exec MESH SEQUENCE MESH=DPistol SEQ=Select1 STARTFRAME=0  NUMFRAMES=11 GROUP=Select
#exec MESH SEQUENCE MESH=DPistol SEQ=Shoot1  STARTFRAME=11 NUMFRAMES=3
#exec MESH SEQUENCE MESH=DPistol SEQ=Idle1   STARTFRAME=14  NUMFRAMES=2
#exec MESH SEQUENCE MESH=DPistol SEQ=Down1   STARTFRAME=16  NUMFRAMES=5
#exec MESH SEQUENCE MESH=DPistol SEQ=PowerUp1 STARTFRAME=21  NUMFRAMES=4
#exec MESH SEQUENCE MESH=DPistol SEQ=Still    STARTFRAME=25  NUMFRAMES=5
#exec MESH SEQUENCE MESH=DPistol SEQ=Select2 STARTFRAME=30  NUMFRAMES=11 GROUP=Select
#exec MESH SEQUENCE MESH=DPistol SEQ=Shoot2  STARTFRAME=41 NUMFRAMES=3
#exec MESH SEQUENCE MESH=DPistol SEQ=Idle2   STARTFRAME=44  NUMFRAMES=2
#exec MESH SEQUENCE MESH=DPistol SEQ=Down2   STARTFRAME=46  NUMFRAMES=5
#exec MESH SEQUENCE MESH=DPistol SEQ=PowerUp2 STARTFRAME=51  NUMFRAMES=9
#exec MESH SEQUENCE MESH=DPistol SEQ=Select3 STARTFRAME=60  NUMFRAMES=11 GROUP=Select
#exec MESH SEQUENCE MESH=DPistol SEQ=Shoot3  STARTFRAME=71 NUMFRAMES=3
#exec MESH SEQUENCE MESH=DPistol SEQ=Idle3   STARTFRAME=74  NUMFRAMES=2
#exec MESH SEQUENCE MESH=DPistol SEQ=Down3   STARTFRAME=76  NUMFRAMES=5
#exec MESH SEQUENCE MESH=DPistol SEQ=PowerUp3 STARTFRAME=81  NUMFRAMES=9
#exec MESH SEQUENCE MESH=DPistol SEQ=Select4 STARTFRAME=90  NUMFRAMES=11 GROUP=Select
#exec MESH SEQUENCE MESH=DPistol SEQ=Shoot4  STARTFRAME=101 NUMFRAMES=3
#exec MESH SEQUENCE MESH=DPistol SEQ=Idle4   STARTFRAME=104  NUMFRAMES=2
#exec MESH SEQUENCE MESH=DPistol SEQ=Down4   STARTFRAME=106  NUMFRAMES=5
#exec MESH SEQUENCE MESH=DPistol SEQ=PowerUp4 STARTFRAME=111  NUMFRAMES=9
#exec MESH SEQUENCE MESH=DPistol SEQ=Select5 STARTFRAME=120 NUMFRAMES=11 GROUP=Select
#exec MESH SEQUENCE MESH=DPistol SEQ=Shoot5  STARTFRAME=131 NUMFRAMES=3
#exec MESH SEQUENCE MESH=DPistol SEQ=Idle5   STARTFRAME=134 NUMFRAMES=2
#exec MESH SEQUENCE MESH=DPistol SEQ=Down5   STARTFRAME=136  NUMFRAMES=5

#exec TEXTURE IMPORT NAME=DPistol1 FILE=MODELS\dgun.PCX GROUP="Skins"
#exec OBJ LOAD FILE=Textures\SmokeEffect2.utx PACKAGE=Unreal.SEffect2
#exec MESHMAP SETTEXTURE MESHMAP=DPistol NUM=1  TEXTURE=DPistol1
#exec MESHMAP SETTEXTURE MESHMAP=DPistol NUM=0  TEXTURE=Unreal.SEffect2.SmokeEffect2


// pickup version
#exec MESH IMPORT MESH=DPistolPick ANIVFILE=MODELS\dgunlo_a.3D DATAFILE=MODELS\dgunlo_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=DPistolPick X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=DPistolPick SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=DPistol1 FILE=MODELS\dgun.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=DPistolPick X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=DPistolPick NUM=1 TEXTURE=DPistol1

// 3rd person perspective version
#exec MESH IMPORT MESH=DPistol3rd ANIVFILE=MODELS\dgunlo_a.3D DATAFILE=MODELS\dgunlo_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=DPistol3rd X=0 Y=-200 Z=-110 YAW=-64 ROLL=9
#exec MESH SEQUENCE MESH=DPistol3rd SEQ=All  STARTFRAME=0  NUMFRAMES=6
#exec MESH SEQUENCE MESH=DPistol3rd SEQ=Still  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=DPistol3rd SEQ=Shoot1 STARTFRAME=0  NUMFRAMES=6
#exec TEXTURE IMPORT NAME=DPistol1 FILE=MODELS\dgun.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=DPistol3rd X=0.025 Y=0.025 Z=0.05
#exec MESHMAP SETTEXTURE MESHMAP=DPistol3rd NUM=1 TEXTURE=DPistol1

#exec AUDIO IMPORT FILE="Sounds\dispersion\Powerup3.WAV" NAME="PowerUp3" GROUP="Dispersion"
#exec AUDIO IMPORT FILE="Sounds\dispersion\DShot1.WAV" NAME="DispShot" GROUP="Dispersion"
#exec AUDIO IMPORT FILE="Sounds\dispersion\Dpickup2.WAV" NAME="DispPickup" GROUP="Dispersion"

var travel int PowerLevel;
var vector WeaponPos;
var float Count,ChargeSize;
var ChargeLight cl1,cl2;
var Pickup Amp;
var Sound PowerUpSound;

function PostBeginPlay()
{
	Super.PostBeginPlay();
	ProjectileClass = class'DispersionAmmo';
	AltProjectileClass = class'DispersionAmmo';
	ProjectileSpeed = class'DispersionAmmo'.Default.speed;
	AltProjectileSpeed = class'DispersionAmmo'.Default.speed;
}

function float RateSelf( out int bUseAltMode )
{
	local float rating;

	if ( Pawn(Owner).bShootSpecial )
		return 1000;

	if ( Amp != None )
		rating = 3.1 * AIRating;
	else 
		rating = AIRating;

	if ( AmmoType.AmmoAmount <=0 )
		return 0.05;
	if ( Pawn(Owner).Enemy == None )
	{
		bUseAltMode = 0;
		return rating * (PowerLevel + 1);
	}
	
	bUseAltMode = int( FRand() < 0.3 );
	return rating * (PowerLevel + 1);
}

// return delta to combat style
function float SuggestAttackStyle()
{
	local float EnemyDist;
	local Inventory Inv;

	if ( !Pawn(Owner).bIsPlayer || (PowerLevel > 0) )
		return 0;

	return -0.3;
}

function bool HandlePickupQuery( inventory Item )
{
	if ( Item.IsA('WeaponPowerup') )
	{ 
		AmmoType.AddAmmo(AmmoType.MaxAmmo);
		Pawn(Owner).ClientMessage(Item.PickupMessage);				
		Item.PlaySound (PickupSound);
		if ( PowerLevel<4 ) 
		{
			ShakeVert = Default.ShakeVert + PowerLevel;
			PowerUpSound = Item.ActivateSound;
			if ( Pawn(Owner).Weapon == self )
			{
				PowerLevel++;
				GotoState('PowerUp');
			}
			else if ( (Pawn(Owner).Weapon != Self) && !Pawn(Owner).bNeverSwitchOnPickup )
			{
				Pawn(Owner).Weapon.PutDown();
				Pawn(Owner).PendingWeapon = self;
				GotoState('PowerUp', 'Waiting');	
			}
			else PowerLevel++;
		}
		Item.SetRespawn();
		return true;
	}
	else
		return Super.HandlePickupQuery(Item);
}

function SetSwitchPriority(pawn Other) 
{
	local int i;
	local name MyType;

	if (PowerLevel == 0)
		MyType = 'DispersionPistol';
	else if (PowerLevel == 1)
		MyType = 'DispersionPower1';
	else if (PowerLevel == 2)
		MyType = 'DispersionPower2';
	else if (PowerLevel == 3)
		MyType = 'DispersionPower3';
	else if (PowerLevel == 4)
		MyType = 'DispersionPower4';
	else 
		MyType = 'DispersionPower5';

	if ( PlayerPawn(Other) != None )
		for ( i=0; i<20; i++)
			if ( PlayerPawn(Other).WeaponPriority[i] == MyType )
			{
				AutoSwitchPriority = i;
				return;
			}	
}

function BecomePickup()
{
	Amp = None;
	Super.BecomePickup();
}
 
function PlayFiring()
{
	AmmoType.GoToState('Idle2');
	Owner.PlaySound(AltFireSound, SLOT_None, 1.8*Pawn(Owner).SoundDampening,,,1.2);
	if ( PlayerPawn(Owner) != None )
		PlayerPawn(Owner).ShakeView(ShakeTime, ShakeMag, ShakeVert);	
	if (PowerLevel==0) 
		PlayAnim('Shoot1',0.4,0.2);
	else if (PowerLevel==1) 
		PlayAnim('Shoot2',0.3,0.2);
	else if (PowerLevel==2) 
		PlayAnim('Shoot3',0.2, 0.2);
	else if (PowerLevel==3) 
		PlayAnim('Shoot4',0.1,0.2);
	else if (PowerLevel==4) 
		PlayAnim('Shoot5',0.1,0.2);
}

function Projectile ProjectileFire(class<projectile> ProjClass, float ProjSpeed, bool bWarn)
{
	local Vector Start, X,Y,Z;
	local DispersionAmmo da;
	local float Mult;

	Owner.MakeNoise(Pawn(Owner).SoundDampening);
	
	if (Amp!=None) Mult = Amp.UseCharge(80);
	else Mult=1.0;
	
	GetAxes(Pawn(owner).ViewRotation,X,Y,Z);
	Start = Owner.Location + CalcDrawOffset() + FireOffset.X * X + FireOffset.Y * Y + FireOffset.Z * Z; 
	AdjustedAim = pawn(owner).AdjustAim(ProjSpeed, Start, AimError, True, (3.5*FRand()-1<PowerLevel));	
	if ( (PowerLevel == 0) || (AmmoType.AmmoAmount < 10) ) 
		da = DispersionAmmo(Spawn(ProjClass,,, Start,AdjustedAim));
	else
	{
		if ( (PowerLevel==1) && AmmoType.UseAmmo(2) ) 
			da = Spawn(class'DAmmo2',,, Start,AdjustedAim);
		if ( (PowerLevel==2) && AmmoType.UseAmmo(4) ) 
			da = Spawn(class'DAmmo3',,, Start,AdjustedAim);
		if ( (PowerLevel==3) && AmmoType.UseAmmo(5) ) 
			da = Spawn(class'DAmmo4',,, Start ,AdjustedAim);
		if ( (PowerLevel>=4) && AmmoType.UseAmmo(6) ) 
			da = Spawn(class'DAmmo5',,, Start,AdjustedAim);		
	}
	if ( da != None )
	{
		if (Mult>1.0) da.InitSplash(FMin(da.damage * Mult, 100));
	}
}

function AltFire( float Value )
{
	bPointing=True;
	CheckVisibility();
	GoToState('AltFiring');
}

////////////////////////////////////////////////////////
state AltFiring
{
ignores AltFire;


	function Tick( float DeltaTime )
	{
		if ( Level.NetMode == NM_StandAlone )
		{
			PlayerViewOffset.X = WeaponPos.X + FRand()*ChargeSize*7;
			PlayerViewOffset.Y = WeaponPos.Y + FRand()*ChargeSize*7;
			PlayerViewOffset.Z = WeaponPos.Z + FRand()*ChargeSize*7;
		}	
		ChargeSize += DeltaTime;
		if( (pawn(Owner).bAltFire==0)) GoToState('ShootLoad');
		Count += DeltaTime;
		if (Count > 0.3) 
		{
			Count = 0.0;
			If (!AmmoType.UseAmmo(1)) GoToState('ShootLoad');
			AmmoType.GoToState('Idle2');
		}
	}
	
	Function EndState()
	{
		PlayerviewOffset = WeaponPos;
		if (cl1!=None) cl1.Destroy();
		if (cl2!=None) cl2.Destroy();		
	}

	function BeginState()
	{
		WeaponPos = PlayerviewOffset;	
		ChargeSize=0.0;		
	}

Begin:
	if (AmmoType.UseAmmo(1))
	{
		Owner.Playsound(Misc1Sound,SLOT_Misc, Pawn(Owner).SoundDampening*4.0);
		Count = 0.0;		
		Sleep(2.0 + 0.6 * PowerLevel);
		GoToState('ShootLoad');
	}
	else GotoState('Idle');

}

///////////////////////////////////////////////////////////
state ShootLoad
{
	function Fire(float F) {}
	function AltFire(float F) {}

	function BeginState()
	{
		local DispersionAmmo d;
		local Vector Start, X,Y,Z;
		local float Mult;
		
		if (Amp!=None) Mult = Amp.UseCharge(ChargeSize*50+50);
		else Mult=1.0;
		
		Owner.MakeNoise(Pawn(Owner).SoundDampening);
		GetAxes(Pawn(owner).ViewRotation,X,Y,Z);
		InvCalcView();
		Start = Location + FireOffset.X * X + FireOffset.Y * Y + FireOffset.Z * Z; 
		AdjustedAim = pawn(owner).AdjustAim(AltProjectileSpeed, Start, AimError, True, True);
		d = DispersionAmmo(Spawn(AltProjectileClass,,, Start,AdjustedAim));
		d.bAltFire = True;
		d.DrawScale = 0.5 + ChargeSize*0.6;
		d.InitSplash(d.DrawScale * Mult * 1.1);
		Owner.PlaySound(AltFireSound, SLOT_Misc, 1.8*Pawn(Owner).SoundDampening);
		if ( PlayerPawn(Owner) != None )
			PlayerPawn(Owner).ShakeView(ShakeTime, ShakeMag*ChargeSize, ShakeVert);
		if (PowerLevel==0) PlayAnim('Shoot1',0.2, 0.05);
		else if (PowerLevel==1) PlayAnim('Shoot2',0.2, 0.05);
		else if (PowerLevel==2) PlayAnim('Shoot3',0.2, 0.05);
		else if (PowerLevel==3) PlayAnim('Shoot4',0.2, 0.05);	
		else if (PowerLevel==4) PlayAnim('Shoot5',0.2, 0.05);	
	}
Begin:
	FinishAnim();
	Finish();
}



///////////////////////////////////////////////////////////
function PlayIdleAnim()
{
	if (PowerLevel==0) LoopAnim('Idle1',0.04,0.2);
	else if (PowerLevel==1) LoopAnim('Idle2',0.04,0.2);
	else if (PowerLevel==2) LoopAnim('Idle3',0.04,0.2);
	else if (PowerLevel==3) LoopAnim('Idle4',0.04,0.2);			
	else if (PowerLevel==4) LoopAnim('Idle5',0.04,0.2);	
}

///////////////////////////////////////////////////////
state PowerUp
{
ignores fire, altfire;

	function BringUp()
	{
		bWeaponUp = false;
		PlaySelect();
		GotoState('Powerup', 'Raising');
	}

	function bool PutDown()
	{
		bChangeWeapon = true;
		return True;
	}

	function BeginState()
	{
		bChangeWeapon = false;
	}

Raising:
	FinishAnim();
	PowerLevel++;
Begin:
	if (PowerLevel<5) 
	{
		AmmoType.MaxAmmo += 10;	
		AmmoType.AddAmmo(10);
		if ( PowerLevel < 5 )
			Owner.PlaySound(PowerUpSound, SLOT_None, Pawn(Owner).SoundDampening);				
		if (PowerLevel==1)
			PlayAnim('PowerUp1',0.1, 0.05);	
		else if (PowerLevel==2) 
			PlayAnim('PowerUp2',0.1, 0.05);			
		else if (PowerLevel==3) 
			PlayAnim('PowerUp3',0.1, 0.05);					
		else if (PowerLevel==4) 
			PlayAnim('PowerUp4',0.1, 0.05);		
		FinishAnim();
		if ( bChangeWeapon )
			GotoState('DownWeapon');
		Finish();
	}
Waiting:
}


function TweenDown()
{
	if ( GetAnimGroup(AnimSequence) == 'Select' )
		TweenAnim( AnimSequence, AnimFrame * 0.4 );
	else
	{
		if (PowerLevel==0) PlayAnim('Down1', 1.0, 0.05);
		else if (PowerLevel==1) PlayAnim('Down2', 1.0, 0.05);
		else if (PowerLevel==2) PlayAnim('Down3', 1.0, 0.05);
		else if (PowerLevel==3) PlayAnim('Down4', 1.0, 0.05);	
		else if (PowerLevel==4) PlayAnim('Down5', 1.0, 0.05);	
	}
}

function TweenSelect()
{
	TweenAnim('Select1',0.001);
}

function PlaySelect()
{
	Owner.PlaySound(SelectSound, SLOT_None, Pawn(Owner).SoundDampening);
	if (PowerLevel==0) PlayAnim('Select1',0.5,0.0);
	else if (PowerLevel==1) PlayAnim('Select2',0.5,0.0);
	else if (PowerLevel==2) PlayAnim('Select3',0.5,0.0);
	else if (PowerLevel==3) PlayAnim('Select4',0.5,0.0);	
	else if (PowerLevel==4) PlayAnim('Select5',0.5,0.0);
}	

defaultproperties
{
     AmmoName=Class'Unreal.DefaultAmmo'
     PickupAmmoCount=50
     bAltWarnTarget=True
     FireOffset=(X=12.000000,Y=-8.000000,Z=-15.000000)
     shakemag=200.000000
     shaketime=0.130000
     shakevert=2.000000
     RefireRate=0.850000
     AltRefireRate=0.300000
     FireSound=Sound'Unreal.Dispersion.DispShot'
     AltFireSound=Sound'Unreal.Dispersion.DispShot'
     SelectSound=Sound'Unreal.Dispersion.DispPickup'
     Misc1Sound=Sound'Unreal.Dispersion.PowerUp3'
     PickupMessage="You got the Dispersion Pistol"
     PlayerViewOffset=(X=3.800000,Y=-2.000000,Z=-2.000000)
     PlayerViewMesh=Mesh'Unreal.DPistol'
     PickupViewMesh=Mesh'Unreal.DPistolPick'
     ThirdPersonMesh=Mesh'Unreal.DPistol3rd'
     PickupSound=Sound'Unreal.Pickups.WeaponPickup'
     Texture=None
     Mesh=Mesh'Unreal.DPistolPick'
     bNoSmooth=False
     bMeshCurvy=False
     CollisionRadius=28.000000
     CollisionHeight=8.000000
     Mass=15.000000
}

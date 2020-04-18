//=============================================================================
// Minigun.
//=============================================================================
class Minigun expands Weapon;

#exec AUDIO IMPORT FILE="Sounds\General\Bulletr2.WAV" NAME="Bulletr2"  GROUP="General"

#exec MESH IMPORT MESH=minigunM ANIVFILE=MODELS\smini_a.3D DATAFILE=MODELS\smini_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=minigunM X=0 Y=0 Z=0 YAW=64 PITCH=-7 ROLL=-62 
#exec MESH SEQUENCE MESH=minigunM SEQ=All     STARTFRAME=0  NUMFRAMES=119
#exec MESH SEQUENCE MESH=minigunM SEQ=Select  STARTFRAME=0  NUMFRAMES=23 RATE=34 GROUP=Select
#exec MESH SEQUENCE MESH=minigunM SEQ=Still   STARTFRAME=23  NUMFRAMES=1
#exec MESH SEQUENCE MESH=minigunM SEQ=Fire    STARTFRAME=24  NUMFRAMES=30
#exec MESH SEQUENCE MESH=minigunM SEQ=Wind    STARTFRAME=24  NUMFRAMES=15
#exec MESH SEQUENCE MESH=minigunM SEQ=Shoot1  STARTFRAME=39  NUMFRAMES=15
#exec MESH SEQUENCE MESH=minigunM SEQ=Shoot2  STARTFRAME=54  NUMFRAMES=20
#exec MESH SEQUENCE MESH=minigunM SEQ=Unwind  STARTFRAME=74  NUMFRAMES=15
#exec MESH SEQUENCE MESH=minigunM SEQ=Cock    STARTFRAME=89  NUMFRAMES=20
#exec MESH SEQUENCE MESH=minigunM SEQ=Down    STARTFRAME=109 NUMFRAMES=10
#exec TEXTURE IMPORT NAME=minigun1 FILE=MODELS\smini.PCX GROUP="Skins" 
#exec OBJ LOAD FILE=Textures\fireeffect13.utx PACKAGE=Unreal.Effect13
#exec MESHMAP SCALE MESHMAP=minigunM X=0.0055 Y=0.0055 Z=0.011
#exec MESHMAP SETTEXTURE MESHMAP=minigunM NUM=1 TEXTURE=minigun1
#exec MESHMAP SETTEXTURE MESHMAP=minigunM NUM=0 TEXTURE=Unreal.Effect13.FireEffect13

#exec MESH IMPORT MESH=minipick ANIVFILE=MODELS\minipi_a.3D DATAFILE=MODELS\minipi_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=minipick X=0 Y=-100 Z=-40 YAW=64 PITCH=0 ROLL=0
#exec MESH SEQUENCE MESH=minipick SEQ=All     STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=minigun1 FILE=MODELS\smini.PCX GROUP="Skins" 
#exec MESHMAP SCALE MESHMAP=minipick X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=minipick NUM=1 TEXTURE=minigun1

#exec MESH IMPORT MESH=SMini3 ANIVFILE=MODELS\Smini3_a.3D DATAFILE=MODELS\smini3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SMini3 X=0 Y=60 Z=0 YAW=64 ROLL=-7
#exec MESH SEQUENCE MESH=SMini3 SEQ=All STARTFRAME=0  NUMFRAMES=11
#exec MESH SEQUENCE MESH=SMini3 SEQ=Fire STARTFRAME=0  NUMFRAMES=10 RATE=20.0
#exec MESH SEQUENCE MESH=SMini3 SEQ=Shoot1  STARTFRAME=0  NUMFRAMES=10 RATE=20.0
#exec MESH SEQUENCE MESH=SMini3 SEQ=Shoot2  STARTFRAME=0  NUMFRAMES=10 RATE=20.0
#exec MESH SEQUENCE MESH=SMini3 SEQ=Still STARTFRAME=10  NUMFRAMES=1
#exec MESH SEQUENCE MESH=SMini3 SEQ=Select STARTFRAME=10  NUMFRAMES=1
#exec MESH SEQUENCE MESH=SMini3 SEQ=Wind STARTFRAME=10  NUMFRAMES=1
#exec MESH SEQUENCE MESH=SMini3 SEQ=UnWind STARTFRAME=10  NUMFRAMES=1
#exec MESH SEQUENCE MESH=SMini3 SEQ=Cock STARTFRAME=10  NUMFRAMES=1
#exec MESH SEQUENCE MESH=SMini3 SEQ=Down STARTFRAME=10  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=minigun1 FILE=MODELS\smini.PCX GROUP="Skins"
#exec OBJ LOAD FILE=textures\FireEffect18.utx PACKAGE=Unreal.Effect18
#exec MESHMAP SCALE MESHMAP=SMini3 X=0.30 Y=0.30 Z=0.6
#exec MESHMAP SETTEXTURE MESHMAP=SMini3 NUM=1 TEXTURE=minigun1
#exec MESHMAP SETTEXTURE MESHMAP=SMini3 NUM=0 TEXTURE=Unreal.Effect18.FireEffect18

#exec AUDIO IMPORT FILE="Sounds\minigun\MPickup2.WAV" NAME="MiniSelect" GROUP="Minigun"
#exec AUDIO IMPORT FILE="Sounds\minigun\RegF1.WAV" NAME="RegF1" GROUP="Minigun"
#exec AUDIO IMPORT FILE="Sounds\minigun\WindD2.WAV" NAME="WindD2" GROUP="Minigun"
#exec AUDIO IMPORT FILE="Sounds\minigun\AltF1.WAV" NAME="AltF1" GROUP="Minigun"

var float ShotAccuracy, Count;
var bool bOutOfAmmo;
var OverHeatLight s;

function GenerateBullet()
{
	if ( AmmoType.UseAmmo(1) ) 
		TraceFire(ShotAccuracy);
	else
		GotoState('FinishFire');
}

function ProcessTraceHit(Actor Other, Vector HitLocation, Vector HitNormal, Vector X, Vector Y, Vector Z)
{
	local int rndDam;

	if ( PlayerPawn(Owner) != None )
		PlayerPawn(Owner).ShakeView(ShakeTime, ShakeMag, ShakeVert);
	
	spawn(class'WeaponLight',self,,Owner.Location+CalcDrawOffset()+X*25+Z*15,rot(0,0,0));
	if (Other == Level) 
		Spawn(class'LightWallHitEffect',,, HitLocation+HitNormal*9, Rotator(HitNormal));
	else if (Other!=self && Other!=Owner ) 
	{
		if ( !Other.IsA('Pawn') && !Other.IsA('Carcass') )
			spawn(class'SpriteSmokePuff',,,HitLocation+HitNormal*9);
		if ( Other.IsA('ScriptedPawn') && (FRand() < 0.2) )
			Pawn(Other).WarnTarget(Pawn(Owner), 500, X);
		rndDam = 7 + Rand(7);
		if ( FRand() < 0.2 )
			X *= 2;
		Other.TakeDamage(rndDam, Pawn(Owner), HitLocation, rndDam*500.0*X, 'shot');
	}
}

function Fire( float Value )
{
	Enable('Tick');
	if ( (Count<1) && AmmoType.UseAmmo(1) )
	{
		CheckVisibility();
		if ( PlayerPawn(Owner) != None )
			PlayerPawn(Owner).ShakeView(ShakeTime, ShakeMag, ShakeVert);
		AmbientSound = FireSound;
		SoundVolume = 255*Pawn(Owner).SoundDampening;
		bPointing=True;
		ShotAccuracy = 0.1;
		PlayFiring();
		GotoState('NormalFire');
	}
	else GoToState('Idle');
}

function AltFire( float Value )
{
	Enable('Tick');
	if ( (Count<1) && AmmoType.UseAmmo(1) )
	{
		CheckVisibility();
		bPointing=True;
		ShotAccuracy = 0.8;
		AmbientSound = FireSound;
		SoundVolume = 255*Pawn(Owner).SoundDampening;		
		PlayAltFiring();	
		GoToState('AltFiring');		
	}
	else GoToState('Idle');	
}


function PlayFiring()
{	
	LoopAnim('Shoot1',0.8, 0.05);
}

function PlayAltFiring()
{
	PlayAnim('Shoot1',0.8, 0.05);
}

////////////////////////////////////////////////////////
state FinishFire
{
	function Fire(float F) {}
	function AltFire(float F) {}

Begin:
	PlayAnim('UnWind',0.8, 0.05);
	FinishAnim();
	Finish();
}

///////////////////////////////////////////////////////
state NormalFire
{
	function Tick( float DeltaTime )
	{
		if (Owner==None) 
			AmbientSound = None;			
	}

	function AnimEnd()
	{
		if (Pawn(Owner).Weapon != self) GotoState('');
		else if (Pawn(Owner).bFire!=0 && AmmoType.AmmoAmount>0)
		{
			if ( (PlayerPawn(Owner) != None) || (FRand() < ReFireRate) )
				Global.Fire(0);
			else 
				GotoState('FinishFire');
		}
		else if ( Pawn(Owner).bAltFire!=0 && AmmoType.AmmoAmount>0)
		{
			if ( (PlayerPawn(Owner) != None) || (FRand() < AltReFireRate) )
				Global.AltFire(0);
			else 
				GotoState('FinishFire');
		}	
		else 
			GotoState('FinishFire');
	}

	function EndState()
	{
		local float Damping;

		if ( Pawn(Owner) == None )
			Damping = 1;
		else
			Damping = Pawn(Owner).SoundDampening;

		AmbientSound = None;
		PlaySound(Misc1Sound, SLOT_Misc, 3.0*Damping);  //Finish firing, power down	
		Super.EndState();
	}

Begin:
	SetLocation(Owner.Location);
	Sleep(0.13);
	GenerateBullet();
	Goto('Begin');
}

////////////////////////////////////////////////////////
state AltFiring
{
	function Tick( float DeltaTime )
	{
		if (Owner==None) AmbientSound = None;			
		if ( (PlayerPawn(Owner) == None) && (FRand() > AltReFireRate) )
			Pawn(Owner).bAltFire = 0; 
		if( (pawn(Owner).bAltFire==0) || bOutOfAmmo) 
			GoToState('FinishFire');
	}

	function AnimEnd()
	{
		if ( (AnimSequence != 'Shoot2') || !bAnimLoop )
		{	
			AmbientSound = AltFireSound;
			SoundVolume = 255*Pawn(Owner).SoundDampening;
			LoopAnim('Shoot2',0.8);
		}
	}

	function EndState()
	{
		local float Damping;

		if ( Pawn(Owner) == None )
			Damping = 1;
		else
			Damping = Pawn(Owner).SoundDampening;

		AmbientSound = None;
		PlaySound(Misc1Sound, SLOT_Misc, 3.0*Damping);  //Finish firing, power down	
		Super.EndState();
	}

Begin:
	SetLocation(Owner.Location);
	Sleep(0.13);
	GenerateBullet();
	if ( AnimSequence == 'Shoot2' )
		Goto('FastShoot');
	Goto('Begin');
FastShoot:
	Sleep(0.07);
	GenerateBullet();
	Goto('FastShoot');
}



///////////////////////////////////////////////////////////
state Idle
{


Begin:
	if (Pawn(Owner).bFire!=0 && AmmoType.AmmoAmount>0) Fire(0.0);
	if (Pawn(Owner).bAltFire!=0 && AmmoType.AmmoAmount>0) AltFire(0.0);	
	PlayAnim('Still');
	bPointing=False;
	if ( (AmmoType != None) && (AmmoType.AmmoAmount<=0) ) 
		Pawn(Owner).SwitchToBestWeapon();  //Goto Weapon that has Ammo
	Disable('AnimEnd');
	PlayIdleAnim();		
}

defaultproperties
{
     AmmoName=Class'Unreal.ShellBox'
     PickupAmmoCount=50
     bInstantHit=True
     bAltInstantHit=True
     FireOffset=(Y=-5.000000,Z=-4.000000)
     shakemag=135.000000
     shakevert=8.000000
     AIRating=0.600000
     RefireRate=0.800000
     AltRefireRate=0.800000
     FireSound=Sound'Unreal.Minigun.RegF1'
     AltFireSound=Sound'Unreal.Minigun.AltF1'
     SelectSound=Sound'Unreal.Minigun.MiniSelect'
     Misc1Sound=Sound'Unreal.Minigun.WindD2'
     AutoSwitchPriority=10
     InventoryGroup=10
     PickupMessage="You got the Minigun"
     PlayerViewOffset=(X=5.600000,Y=-1.500000,Z=-1.800000)
     PlayerViewMesh=Mesh'Unreal.MinigunM'
     PickupViewMesh=Mesh'Unreal.minipick'
     ThirdPersonMesh=Mesh'Unreal.SMini3'
     PickupSound=Sound'Unreal.Pickups.WeaponPickup'
     Mesh=Mesh'Unreal.minipick'
     bNoSmooth=False
     bMeshCurvy=False
     SoundRadius=64
     SoundVolume=255
     CollisionRadius=28.000000
     CollisionHeight=8.000000
}

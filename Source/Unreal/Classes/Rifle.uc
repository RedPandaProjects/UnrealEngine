//=============================================================================
// Rifle.
//=============================================================================
class Rifle expands Weapon;

#exec MESH IMPORT MESH=RifleM ANIVFILE=MODELS\Rifle_a.3D DATAFILE=MODELS\Rifle_d.3D X=0 Y=0 Z=0 unmirror=1
#exec MESH ORIGIN MESH=RifleM X=0 Y=0 Z=0 YAW=-64 PITCH=0 ROLL=0
#exec MESH SEQUENCE MESH=RifleM SEQ=All         STARTFRAME=0   NUMFRAMES=101
#exec MESH SEQUENCE MESH=RifleM SEQ=Select      STARTFRAME=0   NUMFRAMES=20 RATE=27 GROUP=Select
#exec MESH SEQUENCE MESH=RifleM SEQ=Still       STARTFRAME=20  NUMFRAMES=1
#exec MESH SEQUENCE MESH=RifleM SEQ=Fire        STARTFRAME=21  NUMFRAMES=10 RATE=24
#exec MESH SEQUENCE MESH=RifleM SEQ=ScopeFire   STARTFRAME=31  NUMFRAMES=10
#exec MESH SEQUENCE MESH=RifleM SEQ=ScopeUp     STARTFRAME=41  NUMFRAMES=14
#exec MESH SEQUENCE MESH=RifleM SEQ=StillScope  STARTFRAME=55  NUMFRAMES=1
#exec MESH SEQUENCE MESH=RifleM SEQ=DownWScope  STARTFRAME=56  NUMFRAMES=15
#exec MESH SEQUENCE MESH=RifleM SEQ=Down        STARTFRAME=71  NUMFRAMES=15
#exec MESH SEQUENCE MESH=RifleM SEQ=ScopeDown   STARTFRAME=86  NUMFRAMES=15
#exec TEXTURE IMPORT NAME=JRifle1 FILE=MODELS\Rifle.PCX GROUP=Skins
#exec OBJ LOAD FILE=textures\FireEffect18.utx PACKAGE=Unreal.Effect18
#exec MESHMAP SCALE MESHMAP=RifleM X=0.005 Y=0.004 Z=0.01
#exec MESHMAP SETTEXTURE MESHMAP=RifleM NUM=1 TEXTURE=JRifle1
#exec MESHMAP SETTEXTURE MESHMAP=RifleM NUM=0 TEXTURE=Unreal.Effect18.FireEffect18

#exec MESH IMPORT MESH=RiPick ANIVFILE=MODELS\RiPick_a.3D DATAFILE=MODELS\RiPick_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=RiPick X=0 Y=0 Z=0 YAW=-64 
#exec MESH SEQUENCE MESH=RiPick SEQ=All         STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=RiPick SEQ=Still       STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JRifle1 FILE=MODELS\Rifle.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=RiPick X=0.06 Y=0.06 Z=0.12
#exec MESHMAP SETTEXTURE MESHMAP=RiPick NUM=1 TEXTURE=JRifle1

#exec MESH IMPORT MESH=Rifle3rd ANIVFILE=MODELS\Rifle3_a.3D DATAFILE=MODELS\Rifle3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Rifle3rd X=0 Y=-550 Z=-45 YAW=-64 ROLL=9
#exec MESH SEQUENCE MESH=Rifle3rd SEQ=All  STARTFRAME=0  NUMFRAMES=10
#exec MESH SEQUENCE MESH=Rifle3rd SEQ=Still  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=Rifle3rd SEQ=Fire  STARTFRAME=1  NUMFRAMES=9
#exec MESHMAP SCALE MESHMAP=Rifle3rd X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=Rifle3rd NUM=1 TEXTURE=JRifle1
#exec MESHMAP SETTEXTURE MESHMAP=Rifle3rd NUM=0 TEXTURE=Unreal.Effect18.FireEffect18

#exec AUDIO IMPORT FILE="Sounds\rifle\RPICKUP1.WAV" NAME="RiflePickup" GROUP="Rifle"
#exec AUDIO IMPORT FILE="Sounds\rifle\RSHOT9.WAV" NAME="RifleShot" GROUP="Rifle"

var int NumFire;
var bool bZoomOn;
var float CurFOV, TempFOV;

function float RateSelf( out int bUseAltMode )
{
	if ( AmmoType.AmmoAmount <=0 )
		return -2;

	bUseAltMode = 0;
	return AIRating;

}

function AltFire( float Value )
{
	GoToState('AltFiring');

}


///////////////////////////////////////////////////////
state NormalFire
{
	function Fire(float F) 
	{
		//bFireMem = true;
	}
	function AltFire(float F) 
	{
		//bAltFireMem = true;
	}

Begin:

	if ( Owner.IsA('PlayerPawn') && (TempFOV>1.0) ) 
	{
		CurFOV = TempFOV;
		TempFOV = 0.0;
		PlayerPawn(Owner).SetDesiredFOV(CurFOV);	
		bZoomOn = True;
	}
	FinishAnim();
	Finish();
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
		SetTimer(1 + 4 * FRand(), false);
		bPointing = true;
		Pawn(targ).WarnTarget(Pawn(Owner), 200, FireDir);
	}
	else 
	{
		SetTimer(0.4 + 1.6 * FRand(), false);
		bPointing = false;
	}
}	

function PlayAltFiring()
{
	if ( PlayerPawn(Owner) != None )
		PlayerPawn(Owner).shakeview(ShakeTime, ShakeMag, ShakeVert);
	Owner.PlaySound(FireSound, SLOT_None,Pawn(Owner).SoundDampening*3.0,,,0.9 + 0.2 * FRand());
	PlayAnim('Fire', 2.0,0.05);
}

function PlayFiring()
{
	Owner.PlaySound(FireSound, SLOT_None, Pawn(Owner).SoundDampening*3.0);
	PlayAnim('Fire', 0.7,0.05);
}

function ProcessTraceHit(Actor Other, Vector HitLocation, Vector HitNormal, Vector X, Vector Y, Vector Z)
{
	local shellcase s;

	s = Spawn(class'ShellCase',Pawn(Owner), '', Owner.Location + CalcDrawOffset() + 30 * X + (2.8 * FireOffset.Y+5.0) * Y - Z * 1);
	if ( s != None ) 
	{
		s.DrawScale = 2.0;
		s.Eject(((FRand()*0.3+0.4)*X + (FRand()*0.2+0.2)*Y + (FRand()*0.3+1.0) * Z)*160);              
	}
	spawn(class'WeaponLight', self, '', Owner.Location + CalcDrawOffset() + 30.0 * X,rot(0,0,0));
	if (Other == Level) 
		Spawn(class'HeavyWallHitEffect',,, HitLocation+HitNormal*9, Rotator(HitNormal));
	else if ((Other != self) && (Other != Owner)) 
	{
		if ( Other.IsA('Pawn') && (HitLocation.Z - Other.Location.Z > 0.62 * Other.CollisionHeight) 
			&& (instigator.IsA('PlayerPawn') || (instigator.skill > 1)) 
			&& (!Other.IsA('ScriptedPawn') || !ScriptedPawn(Other).bIsBoss) )
			Other.TakeDamage(100, Pawn(Owner), HitLocation, 35000 * X, 'decapitated');
		else
			Other.TakeDamage(45,  Pawn(Owner), HitLocation, 30000.0*X, 'shot');	
		if ( !Other.IsA('Pawn') && !Other.IsA('Carcass') )
			spawn(class'SpriteSmokePuff',,,HitLocation+HitNormal*9);	
	}
}

function Finish()
{
	if ( ((Pawn(Owner).bFire!=0) || (Pawn(Owner).bAltFire!=0)) && (FRand() < 0.6) )
		Timer();
	Super.Finish();
}

state Idle
{

	function AltFire( float Value )
	{
		TempFOV = CurFOV;
		GoToState('AltFiring');
	}

	function Fire( float Value )
	{
		if (AmmoType.UseAmmo(1))
		{
			TempFOV = CurFOV;		
			GotoState('NormalFire');
			if ( PlayerPawn(Owner) != None )
				PlayerPawn(Owner).ShakeView(ShakeTime, ShakeMag, ShakeVert);
			bPointing=True;
			TraceFire(0.0);
			PlayFiring();
			CheckVisibility();
		}
	}


	function BeginState()
	{
		if (Pawn(Owner).bFire!=0) Fire(0.0);		
		bPointing = false;
		SetTimer(0.4 + 1.6 * FRand(), false);
		Super.BeginState();
	}

	function EndState()
	{	
		if ( (Owner != None) && Owner.IsA('PlayerPawn') )
		{	
			PlayerPawn(Owner).SetDesiredFOV(PlayerPawn(Owner).Default.DesiredFOV);	
			CurFOV = PlayerPawn(Owner).DesiredFOV;
		}	
		bZoomOn = False;	
		SetTimer(0.0, false);
		Super.EndState();
	}
	
Begin:
	bPointing=False;
	if ( (AmmoType != None) && (AmmoType.AmmoAmount<=0) ) 
		Pawn(Owner).SwitchToBestWeapon();  //Goto Weapon that has Ammo
	if ( Pawn(Owner).bFire!=0 ) Fire(0.0);
	Disable('AnimEnd');
	PlayIdleAnim();
}
///////////////////////////////////////////////////////
state AltFiring
{

function Timer()
{
	if (Pawn(Owner).bAltFire==0 || CurFOV<=10) 
	{
		SetTimer(0.0,False);
		GoToState('Idle');
	}
	else if ( Owner.IsA('PlayerPawn') )
	{
		CurFOV -= CurFOV*0.09;
		PlayerPawn(Owner).SetDesiredFOV(CurFOV);
	}

}

Begin:
	if ( Owner.IsA('PlayerPawn') )
	{
		if (TempFOV!=0.0 && TempFOV != PlayerPawn(Owner).Default.FOVAngle) bZoomOn = True;

		if (!bZoomOn)
		{	
			SetTimer(0.075,True);
			CurFOV = PlayerPawn(Owner).DesiredFOV;	
			bZoomOn = True;
		}
		else 
		{
			PlayerPawn(Owner).SetDesiredFOV(PlayerPawn(Owner).Default.FOVAngle);		
			SetTimer(0.0,False);
			CurFOV = PlayerPawn(Owner).DesiredFOV;
			bZoomOn = False;
			GoToState('Idle');
		}
	}
	else
	{
		Pawn(Owner).bFire = 1;
		Pawn(Owner).bAltFire = 0;
		Global.Fire(0);
	}
}

///////////////////////////////////////////////////////////
function PlayIdleAnim()
{
	PlayAnim('Still',1.0, 0.05);
}

defaultproperties
{
     AmmoName=Class'Unreal.RifleAmmo'
     PickupAmmoCount=8
     bInstantHit=True
     bAltInstantHit=True
     FireOffset=(Y=-5.000000,Z=-2.000000)
     shakemag=400.000000
     shaketime=0.150000
     shakevert=8.000000
     AIRating=0.700000
     RefireRate=0.600000
     AltRefireRate=0.300000
     FireSound=Sound'Unreal.Rifle.RifleShot'
     SelectSound=Sound'Unreal.Rifle.RiflePickup'
     AutoSwitchPriority=9
     InventoryGroup=9
     PickupMessage="You got the Rifle"
     PlayerViewOffset=(X=3.200000,Y=-1.200000,Z=-1.700000)
     PlayerViewMesh=Mesh'Unreal.RifleM'
     PickupViewMesh=Mesh'Unreal.RiPick'
     ThirdPersonMesh=Mesh'Unreal.Rifle3rd'
     PickupSound=Sound'Unreal.Pickups.WeaponPickup'
     Mesh=Mesh'Unreal.RiPick'
     bNoSmooth=False
     bMeshCurvy=False
     CollisionRadius=28.000000
     CollisionHeight=8.000000
}

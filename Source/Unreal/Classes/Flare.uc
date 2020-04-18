//=============================================================================
// Flare.
//=============================================================================
class Flare expands Pickup;

#exec AUDIO IMPORT FILE="Sounds\Pickups\GENPICK3.WAV" NAME="GenPickSnd"    GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\Pickups\flarel1.WAV" NAME="flarel1"    GROUP="Pickups"
#exec AUDIO IMPORT FILE="Sounds\Pickups\flares1.WAV" NAME="flares1"    GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_Flare FILE=TEXTURES\HUD\i_flare.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=FlareM ANIVFILE=MODELS\Flare_a.3D DATAFILE=MODELS\Flare_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=FlareM X=0 Y=0 Z=-150 YAW=-64 PITCH=64
#exec MESH SEQUENCE MESH=FlareM SEQ=All STARTFRAME=0  NUMFRAMES=2
#exec MESH SEQUENCE MESH=FlareM SEQ=In  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=FlareM SEQ=Out STARTFRAME=1  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JMisc1 FILE=MODELS\misc.PCX GROUP="Skins"
#exec OBJ LOAD FILE=Textures\fireeffect8.utx  PACKAGE=Unreal.Effect8
#exec MESHMAP SCALE MESHMAP=FlareM X=0.02 Y=0.02 Z=0.04
#exec MESHMAP SETTEXTURE MESHMAP=FlareM NUM=1 TEXTURE=JMisc1
#exec MESHMAP SETTEXTURE MESHMAP=FlareM NUM=0 TEXTURE=Unreal.Effect8.FireEffect8

var vector X,Y,Z;
var Flare f;
var bool bFirstTick;
var bool bDamaged;

state Activated  // Delete from inventory and toss in front of player.
{
	function Timer()
	{
		if( bFirstTick )
		{
			bFirstTick=False;
			PlayAnim('out',0.1);
			PlaySound(ActivateSound);
			LightType = LT_Steady;		
			LightBrightness = 250;
			LightRadius = 33;
			LightSaturation = 89;
			AmbientGlow = 200;
			SetTimer(1.0,True);
			AmbientSound = sound'flarel1';
		}
		Charge--;
		if (Charge<=0) TakeDamage(10,None, Vect(0,0,0), Vect(0,0,0), 'Detonated');
	}
	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
					Vector momentum, name damageType)
	{
		if (bDamaged) Return;
		bDamaged = True;
		Spawn(Class 'SpriteBallExplosion',,,Location+Vect(0,0,9));
		HurtRadius(50, 50, 'exploded', 0, Location);
		Destroy();
	}
	simulated function HitWall( vector HitNormal, actor Wall )
	{
		Velocity = 0.6*(( Velocity dot HitNormal ) * HitNormal * (-2.0) + Velocity);   // Reflect off Wall w/damping
		bRotatetoDesired=True;
		bFixedRotationDir=False;
		DesiredRotation.Pitch=0;	
		DesiredRotation.Yaw=FRand()*65536;
		DesiredRotation.Roll=0;		
		RotationRate.Yaw = RotationRate.Yaw*0.75;
		RotationRate.Roll = RotationRate.Roll*0.75;
		RotationRate.Pitch = RotationRate.Pitch*0.75;	
		If (VSize(Velocity) < 5)
		{
			bBounce = False;
			SetPhysics(PHYS_None);
		}
	}

Begin:
	if (NumCopies>0)
	{
		NumCopies--;
		GetAxes(Pawn(Owner).ViewRotation,X,Y,Z);
		f=Spawn(class'Flare', Owner, '', Pawn(Owner).Location +10*Y - 20*Z );
		f.NumCopies=-10;
		f.GoToState('Activated');
		GoToState('');
	}
	else
	{
		Disable('Touch');
		GetAxes(Pawn(Owner).ViewRotation,X,Y,Z);
		SetPhysics(PHYS_Falling);
		Velocity = Owner.Velocity + Vector(Pawn(Owner).ViewRotation) * 450.0;
		Velocity.z += 100;
		SetTimer(0.25,True);
		DesiredRotation = RotRand();
		RotationRate.Yaw = 200000*FRand() - 100000;
		RotationRate.Pitch = 200000*FRand() - 100000;
		RotationRate.Roll = 200000*FRand() - 100000;
		bFixedRotationDir=True;
		SetLocation(Owner.Location+Y*10-Z*20);
		if (NumCopies>-5) {
			Pawn(Owner).NextItem();
			if (Pawn(Owner).SelectedItem == Self) Pawn(Owner).SelectedItem=None;	
			Pawn(Owner).DeleteInventory(Self);
		}
		bFirstTick=True;
		BecomePickup();		
		bStasis = false;
		bBounce=True;
		bCollideWorld=True;		
	}
}

defaultproperties
{
     bCanHaveMultipleCopies=True
     bActivatable=True
     bDisplayableInv=True
     PickupMessage="You got a flare"
     RespawnTime=30.000000
     PickupViewMesh=Mesh'Unreal.FlareM'
     Charge=10
     PickupSound=Sound'Unreal.Pickups.GenPickSnd'
     ActivateSound=Sound'Unreal.Pickups.flares1'
     Icon=Texture'Unreal.Icons.I_Flare'
     RemoteRole=ROLE_DumbProxy
     Mesh=Mesh'Unreal.FlareM'
     bUnlit=True
     bMeshCurvy=False
     CollisionRadius=13.000000
     CollisionHeight=8.000000
     bCollideWorld=True
     bProjTarget=True
     LightBrightness=199
     LightHue=25
     LightSaturation=89
     LightRadius=33
}

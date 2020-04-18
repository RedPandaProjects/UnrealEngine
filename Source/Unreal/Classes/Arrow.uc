//=============================================================================
// Arrow.
//=============================================================================
class Arrow expands Projectile;

#exec MESH IMPORT MESH=ArrowM ANIVFILE=MODELS\arrow_a.3D DATAFILE=MODELS\arrow_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=ArrowM X=0 Y=0 Z=0 YAW=-64
#exec MESH SEQUENCE MESH=ArrowM SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=ArrowM SEQ=Still  STARTFRAME=0   NUMFRAMES=1 
#exec TEXTURE IMPORT NAME=JArrow1 FILE=MODELS\arrow.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=ArrowM X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=ArrowM NUM=1 TEXTURE=JArrow1

#exec MESH IMPORT MESH=burst ANIVFILE=MODELS\burst_a.3D DATAFILE=MODELS\burst_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=burst X=0 Y=0 Z=0 YAW=-64
#exec MESH SEQUENCE MESH=burst SEQ=All       STARTFRAME=0   NUMFRAMES=6
#exec MESH SEQUENCE MESH=burst SEQ=Explo     STARTFRAME=0   NUMFRAMES=6
#exec TEXTURE IMPORT NAME=Jburst1 FILE=MODELS\burst.PCX GROUP=Skin
#exec MESHMAP SCALE MESHMAP=burst X=0.2 Y=0.2 Z=0.4 YAW=128
#exec MESHMAP SETTEXTURE MESHMAP=burst NUM=0 TEXTURE=Jburst1

#exec AUDIO IMPORT FILE="Sounds\general\ArrowSpawn.wav" NAME="ArrowSpawn" GROUP="General"
#exec AUDIO IMPORT FILE="Sounds\Razor\bladehit.wav" NAME="BladeHit" GROUP="RazorJack"

	simulated function PostBeginPlay()
	{
		local rotator RandRot;

		Super.PostBeginPlay();
		Velocity = Vector(Rotation) * Speed;      // velocity
		RandRot.Pitch = FRand() * 200 - 100;
		RandRot.Yaw = FRand() * 200 - 100;
		RandRot.Roll = FRand() * 200 - 100;
		Velocity = Velocity >> RandRot;
		PlaySound(SpawnSound, SLOT_Misc, 2.0);		
	}

	function ProcessTouch( Actor Other, Vector HitLocation )
	{
		local int hitdamage;

		if (Arrow(Other) == none)
		{
			Other.TakeDamage(damage, instigator,HitLocation,
				(MomentumTransfer * Normal(Velocity)), 'shot');
			Destroy();
		}
	}

	function HitWall( vector HitNormal, actor Wall )
	{
		Super.HitWall(HitNormal, Wall);	
		PlaySound(ImpactSound, SLOT_Misc, 0.5);
  		mesh = mesh'Burst';
  		Skin = Texture'JArrow1';
		SetPhysics(PHYS_None); 
		SetCollision(false,false,false);
		MakeNoise(0.3);
		PlayAnim   ( 'Explo', 0.9 );
		GotoState('Exploding');
	}

	function Explode(vector HitLocation, vector HitNormal)
	{
	}


state Exploding
{
Begin:
	FinishAnim();
	Destroy();
}

defaultproperties
{
     speed=+00700.000000
     Damage=+00020.000000
     MomentumTransfer=2000
     SpawnSound=Unreal.ArrowSpawn
     ImpactSound=Unreal.BladeHit
     Mesh=Unreal.ArrowM
     bMeshCurvy=False
	 bUnlit=true
	 RemoteRole=ROLE_SimulatedProxy
}

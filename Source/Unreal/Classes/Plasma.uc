//=============================================================================
// Plasma.
//=============================================================================
class Plasma expands Projectile;


#alwaysexec MESH IMPORT MESH=plasmaM ANIVFILE=MODELS\cros_t_a.3D DATAFILE=MODELS\cros_t_d.3D X=0 Y=0 Z=0 
#alwaysexec MESH ORIGIN MESH=plasmaM X=0 Y=-420 Z=0 YAW=-64
#alwaysexec MESH SEQUENCE MESH=plasmaM SEQ=All STARTFRAME=0  NUMFRAMES=1
#alwaysexec MESH SEQUENCE MESH=plasmaM SEQ=Still  STARTFRAME=0 NUMFRAMES=1
#alwaysexec MESHMAP SCALE MESHMAP=plasmaM X=0.09 Y=0.15 Z=0.08
#alwaysexec  OBJ LOAD FILE=Textures\fireeffect1.utx PACKAGE=Unreal.Effect1
#alwaysexec MESHMAP SETTEXTURE MESHMAP=plasmaM NUM=0 TEXTURE=Unreal.Effect1.FireEffect1u
#alwaysexec MESHMAP SETTEXTURE MESHMAP=plasmaM NUM=1 TEXTURE=Unreal.Effect1.FireEffect1t
  
simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	Velocity = Vector(Rotation) * speed;
	RandSpin(50000);
	PlaySound(SpawnSound);
}

function Explode(vector HitLocation, vector HitNormal)
{
	HurtRadius(Damage,150.0, 'exploded', MomentumTransfer, HitLocation );	
	Destroy();
}

function ProcessTouch (Actor Other, vector HitLocation)
{
	If ( (Other!=Instigator) && Other.IsA('DispersionAmmo') )
		Explode(HitLocation, HitLocation);
}

defaultproperties
{
     speed=1300.000000
     Mesh=Mesh'Unreal.plasmaM'
     bUnlit=True
     bMeshCurvy=False
}

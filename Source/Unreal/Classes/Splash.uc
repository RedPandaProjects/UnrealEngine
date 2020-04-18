//=============================================================================
// Splash.
//=============================================================================
class Splash expands Effects;


#exec MESH IMPORT MESH=WaterImpactM ANIVFILE=MODELS\splash_a.3D DATAFILE=MODELS\splash_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=WaterImpactM X=0 Y=0 Z=0 YAW=0 PITCH=0 ROLL=0
#exec MESH SEQUENCE MESH=WaterImpactM SEQ=All  STARTFRAME=0   NUMFRAMES=26
#exec MESH SEQUENCE MESH=WaterImpactM SEQ=Still STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=WaterImpactM SEQ=Burst STARTFRAME=1   NUMFRAMES=25
#exec TEXTURE IMPORT NAME=Asplash1 FILE=MODELS\splash.PCX GROUP="Skins" FLAGS=2
#exec MESHMAP SCALE MESHMAP=WaterImpactM X=0.08 Y=0.08 Z=0.16 
#exec MESHMAP SETTEXTURE MESHMAP=WaterImpactM NUM=1 TEXTURE=Asplash1

function PostBeginPlay()
{
	local rotator NewRot;

	PlayAnim  ( 'Burst', 2.0 );
	NewRot.Yaw = FRand()*65536;
	NewRot.Pitch = 0;
	NewRot.Roll = 0;
	SetRotation(NewRot);
	Super.PostBeginPlay();
} 
 
function AnimEnd()
{
	Destroy();
}

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Unreal.WaterImpactM
     bMeshCurvy=False
     Physics=PHYS_None
     RemoteRole=ROLE_SimulatedProxy
     Class=Unreal.splash
	 bNetOptional=true
}

//=============================================================================
// Flag1.
//=============================================================================
class Flag1 expands Decoration;

#exec MESH IMPORT MESH=Flag1M ANIVFILE=MODELS\flag_a.3D DATAFILE=MODELS\flag_d.3D X=0 Y=0 Z=0 ZeroTex=1
#exec MESH ORIGIN MESH=Flag1M X=0 Y=100 Z=0 YAW=128 PITCH=0 ROLL=-64
#exec MESH SEQUENCE MESH=flag1M SEQ=All    STARTFRAME=0  NUMFRAMES=14
#exec MESH SEQUENCE MESH=flag1M SEQ=Wave  STARTFRAME=1  NUMFRAMES=13
#exec TEXTURE IMPORT NAME=JFlag11 FILE=MODELS\flag.PCX GROUP=Skins
#exec TEXTURE IMPORT NAME=JFlag12 FILE=MODELS\flagb.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=flag1M X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=flag1M NUM=0 TEXTURE=Jflag11

function PostBeginPlay()
{
	Super.PostBeginPlay();
	LoopAnim('Wave',0.7,0.1);
}

defaultproperties
{
     bStatic=False
     DrawType=DT_Mesh
     Texture=Unreal.JFlag11
     Mesh=Unreal.Flag1M
     Physics=PHYS_Walking
     Class=Unreal.Flag1
     RemoteRole=ROLE_SimulatedProxy
}

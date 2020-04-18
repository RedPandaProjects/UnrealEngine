//=============================================================================
// Arm1.
//=============================================================================
class Arm1 expands PlayerChunks;


#exec MESH IMPORT MESH=Arm1M ANIVFILE=MODELS\g_Arm1_a.3D DATAFILE=MODELS\g_Arm1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Arm1M X=0 Y=0 Z=-160 YAW=64
#exec MESH SEQUENCE MESH=Arm1M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Arm1M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JArm11  FILE=MODELS\g_Arm.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Arm1M X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=Arm1M NUM=1 TEXTURE=JArm11
defaultproperties
{
     Mesh=Arm1M
     CollisionRadius=+00018.000000
}

//=============================================================================
// Pottery2.
//=============================================================================
class Pottery2 expands Vase;

#exec MESH IMPORT MESH=Pottery2M ANIVFILE=MODELS\pot2_a.3D DATAFILE=MODELS\pot2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Pottery2M X=0 Y=0 Z=110 YAW=64
#exec MESH SEQUENCE MESH=Pottery2M SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=Pottery2M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JPottery21 FILE=MODELS\pot2.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=Pottery2M X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=Pottery2M NUM=1 TEXTURE=JPottery21

defaultproperties
{
     Mesh=Pottery2M
     bMeshCurvy=False
     CollisionRadius=+00014.000000
     CollisionHeight=+00022.000000
}

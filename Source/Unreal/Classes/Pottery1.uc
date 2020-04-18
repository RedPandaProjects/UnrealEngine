//=============================================================================
// Pottery1.
//=============================================================================
class Pottery1 expands Vase;

#exec MESH IMPORT MESH=Pottery1M ANIVFILE=MODELS\pot1_a.3D DATAFILE=MODELS\pot1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Pottery1M X=0 Y=0 Z=98 YAW=64
#exec MESH SEQUENCE MESH=Pottery1M SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=Pottery1M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JPottery11 FILE=MODELS\pot1.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=Pottery1M X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=Pottery1M NUM=1 TEXTURE=JPottery11

defaultproperties
{
     Mesh=Pottery1M
     bMeshCurvy=False
     CollisionRadius=+00014.000000
     CollisionHeight=+00019.500000
}

//=============================================================================
// Tree10.
//=============================================================================
class Tree10 expands Tree;


#exec MESH IMPORT MESH=Tree10M ANIVFILE=MODELS\Tree16_a.3D DATAFILE=MODELS\Tree16_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Tree10M X=0 Y=320 Z=0 YAW=64 ROLL=-64
#exec MESH SEQUENCE MESH=Tree10M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Tree10M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JTree101 FILE=MODELS\Tree16.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Tree10M X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=Tree10M NUM=1 TEXTURE=JTree101

defaultproperties
{
     Mesh=Unreal.Tree10M
     CollisionRadius=+00010.000000
     CollisionHeight=+00032.000000
}

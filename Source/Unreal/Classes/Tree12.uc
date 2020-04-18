//=============================================================================
// Tree12.
//=============================================================================
class Tree12 expands Tree;


#exec MESH IMPORT MESH=Tree12M ANIVFILE=MODELS\Tree18_a.3D DATAFILE=MODELS\Tree18_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Tree12M X=0 Y=320 Z=0 YAW=64 ROLL=-64
#exec MESH SEQUENCE MESH=Tree12M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Tree12M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JTree121 FILE=MODELS\Tree18.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Tree12M X=0.2 Y=0.2 Z=0.4
#exec MESHMAP SETTEXTURE MESHMAP=Tree12M NUM=1 TEXTURE=JTree121

defaultproperties
{
     Mesh=Unreal.Tree12M
     CollisionRadius=+00016.000000
     CollisionHeight=+00063.000000
}

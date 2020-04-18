//=============================================================================
// Tree9.
//=============================================================================
class Tree9 expands Tree;


#exec MESH IMPORT MESH=Tree9M ANIVFILE=MODELS\Tree15_a.3D DATAFILE=MODELS\Tree15_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Tree9M X=0 Y=320 Z=0 YAW=64 ROLL=-64
#exec MESH SEQUENCE MESH=Tree9M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Tree9M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JTree91 FILE=MODELS\Tree15.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Tree9M X=0.12 Y=0.12 Z=0.24
#exec MESHMAP SETTEXTURE MESHMAP=Tree9M NUM=1 TEXTURE=JTree91

defaultproperties
{
     Mesh=Unreal.Tree9M
     CollisionRadius=+00015.000000
     CollisionHeight=+00039.000000
}

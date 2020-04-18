//=============================================================================
// Tree4.
//=============================================================================
class Tree4 expands Tree;

#exec MESH IMPORT MESH=Tree4M ANIVFILE=MODELS\Tree4_a.3D DATAFILE=MODELS\Tree4_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Tree4M X=50 Y=0 Z=50 YAW=64 ROLL=-64
#exec MESH SEQUENCE MESH=Tree4M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Tree4M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JTree41 FILE=MODELS\Tree4.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Tree4M X=0.25 Y=0.25 Z=0.5
#exec MESHMAP SETTEXTURE MESHMAP=Tree4M NUM=1 TEXTURE=JTree41

defaultproperties
{
     Mesh=Unreal.Tree4M
}

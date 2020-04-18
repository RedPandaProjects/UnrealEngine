//=============================================================================
// Tree1.
//=============================================================================
class Tree1 expands Tree;

#exec MESH IMPORT MESH=Tree1M ANIVFILE=MODELS\Tree1_a.3D DATAFILE=MODELS\Tree1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Tree1M X=0 Y=0 Z=0 YAW=64 ROLL=-64
#exec MESH SEQUENCE MESH=Tree1M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Tree1M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JTree11 FILE=MODELS\Tree1.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Tree1M X=0.2 Y=0.2 Z=0.4
#exec MESHMAP SETTEXTURE MESHMAP=Tree1M NUM=1 TEXTURE=JTree11

defaultproperties
{
     Mesh=Tree1M
}

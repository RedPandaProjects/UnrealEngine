//=============================================================================
// Tree2.
//=============================================================================
class Tree2 expands Tree;

#exec MESH IMPORT MESH=Tree2M ANIVFILE=MODELS\Tree2_a.3D DATAFILE=MODELS\Tree2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Tree2M X=50 Y=0 Z=50 YAW=64 ROLL=-64
#exec MESH SEQUENCE MESH=Tree2M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Tree2M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JTree21 FILE=MODELS\Tree2.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Tree2M X=0.25 Y=0.25 Z=0.5
#exec MESHMAP SETTEXTURE MESHMAP=Tree2M NUM=1 TEXTURE=JTree21

defaultproperties
{
     Mesh=Tree2M
}

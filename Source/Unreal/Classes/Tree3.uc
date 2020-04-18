//=============================================================================
// Tree3.
//=============================================================================
class Tree3 expands Tree;

#exec MESH IMPORT MESH=Tree3M ANIVFILE=MODELS\Tree3_a.3D DATAFILE=MODELS\Tree3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Tree3M X=50 Y=0 Z=50 YAW=64 ROLL=-64
#exec MESH SEQUENCE MESH=Tree3M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Tree3M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JTree31 FILE=MODELS\Tree3.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Tree3M X=0.25 Y=0.25 Z=0.5
#exec MESHMAP SETTEXTURE MESHMAP=Tree3M NUM=1 TEXTURE=JTree31

defaultproperties
{
     Mesh=Tree3M
}

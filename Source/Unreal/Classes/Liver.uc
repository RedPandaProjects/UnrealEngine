//=============================================================================
// Liver.
//=============================================================================
class Liver expands PlayerChunks;

#exec MESH IMPORT MESH=LiverM ANIVFILE=MODELS\g_gut1_a.3D DATAFILE=MODELS\g_gut1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=LiverM X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=LiverM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=LiverM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jparts1  FILE=MODELS\g_parts.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=LiverM X=0.02 Y=0.02 Z=0.04
#exec MESHMAP SETTEXTURE MESHMAP=LiverM NUM=1 TEXTURE=Jparts1
defaultproperties
{
     Mesh=LiverM
     CollisionRadius=+00005.000000
     CollisionHeight=+00003.000000
}

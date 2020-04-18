//=============================================================================
// Stomach.
//=============================================================================
class Stomach expands PlayerChunks;

#exec MESH IMPORT MESH=stomachM ANIVFILE=MODELS\g_stm_a.3D DATAFILE=MODELS\g_stm_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=stomachM X=0 Y=0 Z=0 YAW=64 PITCH=128
#exec MESH SEQUENCE MESH=stomachM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=stomachM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jparts1  FILE=MODELS\g_parts.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=stomachM X=0.03 Y=0.03 Z=0.06
#exec MESHMAP SETTEXTURE MESHMAP=stomachM NUM=1 TEXTURE=Jparts1
defaultproperties
{
     bPlayerCarcass=True
     Mesh=StomachM
     CollisionRadius=+00007.000000
     CollisionHeight=+00003.000000
}

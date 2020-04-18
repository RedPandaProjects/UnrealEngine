//=============================================================================
// Flag2.
//=============================================================================
class Flag2 expands Decoration;

#exec MESH IMPORT MESH=Flag2M ANIVFILE=MODELS\flag2_a.3D DATAFILE=MODELS\flag2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Flag2M X=0 Y=100 Z=-120 YAW=64

#exec MESH SEQUENCE MESH=flag2M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=flag2M SEQ=Still  STARTFRAME=0   NUMFRAMES=1

#exec TEXTURE IMPORT NAME=JFlag21 FILE=MODELS\flag2.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=flag2M X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=flag2 NUM=1 TEXTURE=Jflag21

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Flag2M
}

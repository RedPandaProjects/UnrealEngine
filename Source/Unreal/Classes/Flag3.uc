//=============================================================================
// Flag3.
//=============================================================================
class Flag3 expands Decoration;

#exec MESH IMPORT MESH=Flag3M ANIVFILE=MODELS\flag3_a.3D DATAFILE=MODELS\flag3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Flag3M X=0 Y=100 Z=-120 YAW=64

#exec MESH SEQUENCE MESH=flag3M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=flag3M SEQ=Still  STARTFRAME=0   NUMFRAMES=1

#exec TEXTURE IMPORT NAME=JFlag31 FILE=MODELS\flag3.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=flag3M X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=flag3M NUM=1 TEXTURE=Jflag31

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Flag3M
     Physics=PHYS_Walking
}

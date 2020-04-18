//=============================================================================
// Lantern.
//=============================================================================
class Lantern expands Decoration;


#exec MESH IMPORT MESH=LanternM ANIVFILE=MODELS\Lantrn_a.3D DATAFILE=MODELS\Lantrn_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=LanternM X=0 Y=0 Z=0 ROLL=-64
#exec MESH SEQUENCE MESH=LanternM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=LanternM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JLantern1 FILE=MODELS\LNTR.pcx GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=LanternM X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=LanternM NUM=1 TEXTURE=JLantern1

defaultproperties
{
     bDirectional=True
     DrawType=DT_Mesh
     Mesh=LanternM
     bMeshCurvy=False
     LightType=LT_Steady
     LightBrightness=203
     LightHue=35
     LightSaturation=59
     LightRadius=32
}

//=============================================================================
// Lantern2.
//=============================================================================
class Lantern2 expands Decoration;

#exec MESH IMPORT MESH=Lantern2M ANIVFILE=MODELS\Lantr2_a.3D DATAFILE=MODELS\Lantr2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Lantern2M X=0 Y=0 Z=0 ROLL=-64
#exec MESH SEQUENCE MESH=Lantern2M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Lantern2M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JLantern1 FILE=MODELS\LNTR.pcx GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Lantern2M X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=Lantern2M NUM=1 TEXTURE=JLantern1

defaultproperties
{
     bDirectional=True
     DrawType=DT_Mesh
     Mesh=Lantern2M
     bMeshCurvy=False
     LightBrightness=201
     LightHue=36
     LightSaturation=60
     LightRadius=33
}

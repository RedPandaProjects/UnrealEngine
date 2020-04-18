//=============================================================================
// Plant7.
//=============================================================================
class Plant7 expands Decoration;


#exec MESH IMPORT MESH=Plant7M ANIVFILE=MODELS\Plant7_a.3D DATAFILE=MODELS\Plant7_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Plant7M X=0 Y=0 Z=0 ROLL=-64
#exec MESH SEQUENCE MESH=Plant7M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Plant7M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JPlant61 FILE=MODELS\Plnt2m.pcx GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Plant7M X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=Plant7M NUM=1 TEXTURE=JPlant61

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Unreal.Plant7M
     bMeshCurvy=False
}

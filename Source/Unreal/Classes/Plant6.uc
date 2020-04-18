//=============================================================================
// Plant6.
//=============================================================================
class Plant6 expands Decoration;


#exec MESH IMPORT MESH=Plant6M ANIVFILE=MODELS\Plant6_a.3D DATAFILE=MODELS\Plant6_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Plant6M X=0 Y=0 Z=0 ROLL=-64
#exec MESH SEQUENCE MESH=Plant6M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Plant6M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JPlant61 FILE=MODELS\Plnt2m.pcx GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Plant6M X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=Plant6M NUM=1 TEXTURE=JPlant61

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Unreal.Plant6M
     bMeshCurvy=False
}

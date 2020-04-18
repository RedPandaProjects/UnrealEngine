//=============================================================================
// Plant1.
//=============================================================================
class Plant1 expands Decoration;

#exec MESH IMPORT MESH=Plant1M ANIVFILE=MODELS\plant1_a.3D DATAFILE=MODELS\plant1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Plant1M X=0 Y=100 Z=-10 YAW=64

#exec MESH SEQUENCE MESH=plant1M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=plant1M SEQ=Still  STARTFRAME=0   NUMFRAMES=1

#exec TEXTURE IMPORT NAME=JPlant11 FILE=MODELS\plant1.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=plant1M X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=plant1M NUM=1 TEXTURE=Jplant11

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Plant1M
     CollisionRadius=+00006.000000
     CollisionHeight=+00013.000000
     bCollideWorld=True
}

//=============================================================================
// Plant2.
//=============================================================================
class Plant2 expands Decoration;

#exec MESH IMPORT MESH=Plant2M ANIVFILE=MODELS\plant2_a.3D DATAFILE=MODELS\plant2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Plant2M X=0 Y=100 Z=100 YAW=64

#exec MESH SEQUENCE MESH=plant2M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=plant2M SEQ=Still  STARTFRAME=0   NUMFRAMES=1

#exec TEXTURE IMPORT NAME=JPlant21 FILE=MODELS\plant2.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=plant2M X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=plant2M NUM=1 TEXTURE=Jplant21
defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Plant2M
     CollisionRadius=+00006.000000
     CollisionHeight=+00009.000000
     bCollideWorld=True
}

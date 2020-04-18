//=============================================================================
// SeaWeed.
//=============================================================================
class SeaWeed expands Decoration;

#exec MESH IMPORT MESH=SeaWeedM ANIVFILE=MODELS\seaw_a.3D DATAFILE=MODELS\seaw_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SeaWeedM X=-70 Y=-400 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SeaWeedM SEQ=All  STARTFRAME=0  NUMFRAMES=4
#exec MESH SEQUENCE MESH=SeaWeedM SEQ=Type1Waver  STARTFRAME=0   NUMFRAMES=2
#exec MESH SEQUENCE MESH=SeaWeedM SEQ=Type2Waver  STARTFRAME=2   NUMFRAMES=2
#exec TEXTURE IMPORT NAME=JSeaWeed1 FILE=MODELS\seaweed.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=SeaWeedM X=0.25 Y=0.25 Z=0.5
#exec MESHMAP SETTEXTURE MESHMAP=SeaWeedM NUM=1 TEXTURE=JSeaWeed1

var() enum ESeaWeedType
{
	Waver1,
	Waver2,
} WeedType;


Auto State Waver
{
 
 Begin:
	if (WeedType == Waver1) LoopAnim('Type1Waver',0.01);
	else LoopAnim('Type2Waver',0.01);
}

defaultproperties
{
     WeedType=Waver2
     bStatic=False
     DrawType=DT_Mesh
     Mesh=SeaWeedM
}

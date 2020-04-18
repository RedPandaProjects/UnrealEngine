//=============================================================================
// Knife.
//=============================================================================
class Knife expands Decoration;


#exec MESH IMPORT MESH=KnifeM ANIVFILE=MODELS\Knife_a.3D DATAFILE=MODELS\Knife_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=KnifeM X=0 Y=0 Z=0 PITCH=-64
#exec MESH SEQUENCE MESH=KnifeM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=KnifeM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JLantern1 FILE=MODELS\LNTR.pcx GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=KnifeM X=0.03 Y=0.03 Z=0.06
#exec MESHMAP SETTEXTURE MESHMAP=KnifeM NUM=1 TEXTURE=JLantern1

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=KnifeM
     bMeshCurvy=False
}

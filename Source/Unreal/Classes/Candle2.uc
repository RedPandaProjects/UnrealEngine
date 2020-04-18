//=============================================================================
// Candle2
//=============================================================================
class Candle2 expands Decoration;

#exec MESH IMPORT MESH=Candl2 ANIVFILE=MODELS\candl2_a.3D DATAFILE=MODELS\candl2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Candl2 X=0 Y=-120 Z=-40 YAW=64
#exec MESH SEQUENCE MESH=candl2 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=candl2 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JCandl21 FILE=MODELS\candl2.PCX GROUP=Skins FLAGS=2
#exec OBJ LOAD FILE=textures\cflame.utx PACKAGE=Unreal.CFLAM
#exec MESHMAP SCALE MESHMAP=candl2 X=0.03 Y=0.03 Z=0.06
#exec MESHMAP SETTEXTURE MESHMAP=candl2 NUM=1 TEXTURE=Jcandl21
#exec MESHMAP SETTEXTURE MESHMAP=candl2 NUM=0 TEXTURE=Unreal.CFLAM.cflame

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Unreal.Candl2
     bMeshCurvy=False
     CollisionRadius=+00003.000000
     CollisionHeight=+00012.500000
     bCollideActors=True
     bCollideWorld=True
}

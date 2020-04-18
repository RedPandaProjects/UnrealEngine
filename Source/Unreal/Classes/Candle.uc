//=============================================================================
// Candle.
//=============================================================================
class Candle expands Decoration;

#exec MESH IMPORT MESH=CandleM ANIVFILE=MODELS\candle_a.3D DATAFILE=MODELS\candle_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=CandleM X=0 Y=0 Z=-50 YAW=64
#exec MESH SEQUENCE MESH=candleM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=candleM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JCandle1 FILE=MODELS\candle.PCX GROUP=Skins 
#exec OBJ LOAD FILE=textures\cflame.utx PACKAGE=Unreal.CFLAM
#exec MESHMAP SCALE MESHMAP=candleM X=0.03 Y=0.03 Z=0.06
#exec MESHMAP SETTEXTURE MESHMAP=candleM NUM=1 TEXTURE=Jcandle1
#exec MESHMAP SETTEXTURE MESHMAP=candleM NUM=0 TEXTURE=Unreal.CFLAM.cflame

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Mesh'Unreal.CandleM'
     bMeshCurvy=False
     CollisionRadius=2.000000
     CollisionHeight=14.000000
     bCollideActors=True
     bCollideWorld=True
}

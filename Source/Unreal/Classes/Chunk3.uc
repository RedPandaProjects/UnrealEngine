//=============================================================================
// Chunk3.
//=============================================================================
class Chunk3 expands Chunk;

#exec MESH IMPORT MESH=Chnk3 ANIVFILE=MODELS\Chunk3_a.3D DATAFILE=MODELS\Chunk3_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Chnk3 X=0 Y=0 Z=-0 YAW=64
#exec MESH SEQUENCE MESH=Chnk3 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Chnk3 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jflakshel1 FILE=MODELS\FlakShel.PCX
#exec MESHMAP SCALE MESHMAP=Chnk3 X=0.025 Y=0.025 Z=0.05
#exec MESHMAP SETTEXTURE MESHMAP=Chnk3 NUM=1 TEXTURE=Jflakshel1

defaultproperties
{
     Damage=+00016.000000
     Mesh=Unreal.Chnk3
     AmbientGlow=63
     LifeSpan=+00002.900000
     Class=Unreal.Chunk3
}

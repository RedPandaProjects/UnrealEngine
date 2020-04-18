//=============================================================================
// Chunk4.
//=============================================================================
class Chunk4 expands Chunk;

#exec MESH IMPORT MESH=Chnk4 ANIVFILE=MODELS\Chunk4_a.3D DATAFILE=MODELS\Chunk4_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Chnk4 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=Chnk4 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Chnk4 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jflakshel1 FILE=MODELS\FlakShel.PCX
#exec MESHMAP SCALE MESHMAP=Chnk4 X=0.025 Y=0.025 Z=0.05
#exec MESHMAP SETTEXTURE MESHMAP=Chnk4 NUM=1 TEXTURE=Jflakshel1

defaultproperties
{
     Damage=+00016.000000
     Mesh=Unreal.Chnk4
     AmbientGlow=41
     LifeSpan=+00002.800000
     Class=Unreal.Chunk4
}

//=============================================================================
// Chunk2.
//=============================================================================
class Chunk2 expands Chunk;

#exec MESH IMPORT MESH=Chnk2 ANIVFILE=MODELS\Chunk2_a.3D DATAFILE=MODELS\Chunk2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Chnk2 X=0 Y=0 Z=-0 YAW=64
#exec MESH SEQUENCE MESH=Chnk2 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Chnk2 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jflakshel1 FILE=MODELS\FlakShel.PCX
#exec MESHMAP SCALE MESHMAP=Chnk2 X=0.025 Y=0.025 Z=0.05
#exec MESHMAP SETTEXTURE MESHMAP=Chnk2 NUM=1 TEXTURE=Jflakshel1

defaultproperties
{
     Damage=+00016.000000
     Mesh=Unreal.Chnk2
     AmbientGlow=38
     LifeSpan=+00003.100000
     Class=Unreal.Chunk2
}

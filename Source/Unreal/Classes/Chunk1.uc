//=============================================================================
// Chunk1.
//=============================================================================
class Chunk1 expands Chunk;

#exec MESH IMPORT MESH=Chnk1 ANIVFILE=MODELS\Chunk1_a.3D DATAFILE=MODELS\Chunk1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Chnk1 X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=Chnk1 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Chnk1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jflakshel1 FILE=MODELS\FlakShel.PCX
#exec MESHMAP SCALE MESHMAP=Chnk1 X=0.025 Y=0.025 Z=0.05
#exec MESHMAP SETTEXTURE MESHMAP=Chnk1 NUM=1 TEXTURE=Jflakshel1

defaultproperties
{
     Damage=+00016.000000
     Mesh=Unreal.Chnk1
     AmbientGlow=43
     Class=Unreal.Chunk1
}

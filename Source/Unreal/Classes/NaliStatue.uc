//=============================================================================
// NaliStatue.
//=============================================================================
class NaliStatue expands MonkStatue;

#exec  MESH IMPORT MESH=NaliStatueM ANIVFILE=MODELS\statue_a.3D DATAFILE=MODELS\statue_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=NaliStatueM X=0 Y=0 Z=0 YAW=0
#exec MESH SEQUENCE MESH=NaliStatueM SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=NaliStatueM SEQ=Still STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JNaliStatue1 FILE=MODELS\nstatue.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=NaliStatueM X=0.09 Y=0.09 Z=0.18
#exec MESHMAP SETTEXTURE MESHMAP=NaliStatueM NUM=1 TEXTURE=JNaliStatue1

defaultproperties
{
     Mesh=Mesh'Unreal.NaliStatueM'
}

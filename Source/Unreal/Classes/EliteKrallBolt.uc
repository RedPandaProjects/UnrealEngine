//=============================================================================
// EliteKrallBolt.
//=============================================================================
class EliteKrallBolt expands KraalBolt;

#exec MESH IMPORT MESH=eplasma ANIVFILE=MODELS\cros_t_a.3D DATAFILE=MODELS\cros_t_d.3D X=0 Y=0 Z=0 
#exec MESH ORIGIN MESH=eplasma X=0 Y=0 Z=0 YAW=-64
#exec MESH SEQUENCE MESH=eplasma SEQ=All STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=eplasma SEQ=Still  STARTFRAME=0 NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=eplasma X=0.04 Y=0.04 Z=0.08
#exec OBJ LOAD FILE=Textures\fireeffect1.utx PACKAGE=Unreal.Effect1
#exec MESHMAP SETTEXTURE MESHMAP=eplasma NUM=0 TEXTURE=Unreal.Effect1.FireEffect1e
#exec MESHMAP SETTEXTURE MESHMAP=eplasma NUM=1 TEXTURE=Unreal.Effect1.FireEffect1d

defaultproperties
{
     Speed=+00880.000000
     Damage=+00019.000000
     Mesh=Mesh'Unreal.eplasma'
}

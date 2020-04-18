//=============================================================================
// Flame.
//=============================================================================
class Flame expands Effects;

#exec MESH IMPORT MESH=FlameM ANIVFILE=MODELS\Flame_a.3D DATAFILE=MODELS\Flame_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=FlameM X=0 Y=0 Z=0 YAW=-64
#exec MESH SEQUENCE MESH=FlameM SEQ=All       STARTFRAME=0   NUMFRAMES=1

#exec MESHMAP SCALE MESHMAP=FlameM X=0.3 Y=0.3 Z=0.6 YAW=128
#exec OBJ LOAD FILE=Textures\fireeffect28.utx PACKAGE=Unreal.Effect28
#exec MESHMAP SETTEXTURE MESHMAP=FlameM NUM=0 TEXTURE=Unreal.Effect28.FireEffect28
#exec MESHMAP SETTEXTURE MESHMAP=FlameM NUM=1 TEXTURE=Unreal.Effect28.FireEffect28a

defaultproperties
{
     DrawType=DT_Mesh
     bUnlit=True
     bMeshCurvy=False
}

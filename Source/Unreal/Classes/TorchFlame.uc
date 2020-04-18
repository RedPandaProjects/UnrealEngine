//=============================================================================
// Torchflame.
//=============================================================================
class TorchFlame expands Light;

#exec MESH IMPORT MESH=FlameM ANIVFILE=MODELS\flame_a.3D DATAFILE=MODELS\flame_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=FlameM X=0 Y=100 Z=350 YAW=0
#exec MESH SEQUENCE MESH=FlameM SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec OBJ LOAD FILE=Textures\fireeffect28.utx PACKAGE=Unreal.Effect28
#exec MESHMAP SCALE MESHMAP=FlameM X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=FlameM NUM=0 TEXTURE=Unreal.Effect28.FireEffect28
#exec MESHMAP SETTEXTURE MESHMAP=FlameM NUM=1 TEXTURE=Unreal.Effect28.FireEffect28a

defaultproperties
{
     bStatic=False
     bHidden=False
	 bMovable=False
     DrawType=DT_Mesh
     Mesh=Unreal.FlameM
     bUnlit=True
     LightEffect=LE_FireWaver
     LightBrightness=40
     LightRadius=32
     AnimRate=+00001.000000
}

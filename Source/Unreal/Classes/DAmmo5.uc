//=============================================================================
// DAmmo5.
//=============================================================================
class DAmmo5 expands DispersionAmmo;

#exec MESH IMPORT MESH=DispM4 ANIVFILE=MODELS\cros_t_a.3D DATAFILE=MODELS\cros_t_d.3D X=0 Y=0 Z=0 
#exec MESH ORIGIN MESH=DispM4 X=0 Y=-500 Z=0 YAW=-64
#exec MESH SEQUENCE MESH=DispM4 SEQ=All STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=DispM4 SEQ=Still  STARTFRAME=0 NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=DispM4 X=0.09 Y=0.15 Z=0.08
#exec  OBJ LOAD FILE=Textures\fireeffect1.utx PACKAGE=Unreal.Effect1
#exec MESHMAP SETTEXTURE MESHMAP=DispM4 NUM=0 TEXTURE=Unreal.Effect1.FireEffect1p
#exec MESHMAP SETTEXTURE MESHMAP=DispM4 NUM=1 TEXTURE=Unreal.Effect1.FireEffect1ob

defaultproperties
{
     ParticleType=Class'Unreal.Spark35'
     SparkModifier=3.000000
     ExpType=Class'Unreal.SpriteRedE'
     Damage=75.000000
     Mesh=Mesh'Unreal.DispM4'
     LightBrightness=190
     LightHue=5
     LightSaturation=63
}

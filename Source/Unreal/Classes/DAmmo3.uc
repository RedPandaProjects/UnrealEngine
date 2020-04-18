//=============================================================================
// DAmmo3.
//=============================================================================
class DAmmo3 expands DispersionAmmo;

#exec MESH IMPORT MESH=DispM2 ANIVFILE=MODELS\cros_t_a.3D DATAFILE=MODELS\cros_t_d.3D X=0 Y=0 Z=0 
#exec MESH ORIGIN MESH=DispM2 X=0 Y=-500 Z=0 YAW=-64
#exec MESH SEQUENCE MESH=DispM2 SEQ=All STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=DispM2 SEQ=Still  STARTFRAME=0 NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=DispM2 X=0.09 Y=0.15 Z=0.08
#exec  OBJ LOAD FILE=Textures\fireeffect1.utx PACKAGE=Unreal.Effect1
#exec MESHMAP SETTEXTURE MESHMAP=DispM2 NUM=0 TEXTURE=Unreal.Effect1.FireEffect1a
#exec MESHMAP SETTEXTURE MESHMAP=DispM2 NUM=1 TEXTURE=Unreal.Effect1.FireEffect1

defaultproperties
{
     ParticleType=Class'Unreal.Spark33'
     SparkModifier=2.000000
     ExpType=Class'Unreal.SpriteGreenE'
     Damage=40.000000
     Mesh=Mesh'Unreal.DispM2'
     LightBrightness=150
     LightHue=83
     LightSaturation=50
}

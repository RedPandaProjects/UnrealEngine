//=============================================================================
// Moon.
//=============================================================================
class Moon expands Decoration;


#exec MESH IMPORT MESH=Moon1 ANIVFILE=MODELS\moon_a.3D DATAFILE=MODELS\moon_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=Moon1 X=0 Y=0 Z=0 YAW=0 ROLL=7
#exec MESH SEQUENCE MESH=Moon1 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Moon1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JMoon1 FILE=MODELS\moon.pcx GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=Moon1 X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=Moon1 NUM=0 TEXTURE=JMoon1

defaultproperties
{
     bStatic=False
     DrawType=DT_Mesh
     Skin=Unreal.JMoon1
     Mesh=Unreal.Moon1
     DrawScale=+00002.000000
     Physics=PHYS_Rotating
     bFixedRotationDir=True
     RotationRate=(Yaw=500)
     DesiredRotation=(Yaw=500)
}

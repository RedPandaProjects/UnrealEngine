//=============================================================================
// Moon3.
//=============================================================================
class Moon3 expands Moon;
 
#exec MESH IMPORT MESH=Moon3M ANIVFILE=MODELS\moon_a.3D DATAFILE=MODELS\moon_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=Moon3M X=0 Y=10000 Z=0 YAW=0 ROLL=7
#exec MESH SEQUENCE MESH=Moon3M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Moon3M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JMoon3 FILE=MODELS\moon3.pcx GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=Moon3M X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=Moon3M NUM=0 TEXTURE=JMoon3

defaultproperties
{
     Skin=None
     Mesh=Unreal.Moon3M
     DrawScale=+00000.150000
     RotationRate=(Yaw=400,Roll=100)
     DesiredRotation=(Yaw=400,Roll=100)
}

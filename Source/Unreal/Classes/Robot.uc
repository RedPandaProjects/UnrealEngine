//=============================================================================
// Robot.
//=============================================================================
class Robot expands Decoration;


#exec MESH IMPORT MESH=RobotM ANIVFILE=MODELS\Robot_a.3D DATAFILE=MODELS\Robot_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=RobotM X=0 Y=0 Z=0 ROLL=-64
#exec MESH SEQUENCE MESH=RobotM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=RobotM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JRobot1 FILE=MODELS\Robot.pcx GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=RobotM X=0.3 Y=0.3 Z=0.6
#exec MESHMAP SETTEXTURE MESHMAP=RobotM NUM=1 TEXTURE=JRobot1

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Unreal.RobotM
     bMeshCurvy=False
     CollisionRadius=+00050.000000
     CollisionHeight=+00114.000000
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
}

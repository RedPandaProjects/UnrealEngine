//=============================================================================
// IntroShip.
//=============================================================================
class IntroShip expands Decoration;

#exec MESH IMPORT MESH=IntroShipM ANIVFILE=MODELS\ship_a.3D DATAFILE=MODELS\ship_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=IntroShipM X=0 Y=100 Z=-120 YAW=64
#exec MESH SEQUENCE MESH=IntroShipM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=IntroShipM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JIntroShip1 FILE=MODELS\Ship.PCX GROUP=Skins 
#exec MESHMAP SCALE MESHMAP=IntroShipM X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=IntroShipM NUM=1 TEXTURE=JIntroShip1

var() float Speed;

function Trigger( actor Other, pawn EventInstigator )
{
	SetPHysics(PHYS_Projectile);
	Velocity = Vector(Rotation) * Speed;
}

defaultproperties
{
     speed=100.000000
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Mesh'Unreal.IntroShipM'
     bMeshCurvy=False
}

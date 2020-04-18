//=============================================================================
// Boulder1.
//=============================================================================
class Boulder1 expands BigRock;

#exec MESH IMPORT MESH=boulderM ANIVFILE=MODELS\rock_a.3D DATAFILE=MODELS\rock_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=boulderM X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=boulderM SEQ=All  STARTFRAME=0  NUMFRAMES=4
#exec MESH SEQUENCE MESH=boulderM SEQ=Pos1  STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=boulderM SEQ=Pos2  STARTFRAME=1   NUMFRAMES=1
#exec MESH SEQUENCE MESH=boulderM SEQ=Pos3  STARTFRAME=2   NUMFRAMES=1
#exec MESH SEQUENCE MESH=boulderM SEQ=Pos4  STARTFRAME=3   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jboulder1 FILE=MODELS\rock.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=boulderM X=0.005 Y=0.005 Z=0.01
#exec MESHMAP SETTEXTURE MESHMAP=boulderM NUM=1 TEXTURE=Jboulder1

auto state Flying
{
	function HitWall (vector HitNormal, actor Wall)
	{
		Velocity = 0.75 * (Velocity - 2 * HitNormal * (Velocity Dot HitNormal));
		SetRotation(rotator(HitNormal));
		DrawScale *= 0.7;
		SpawnChunks(8);
		Destroy();
	}
}

defaultproperties
{
     speed=+01300.000000
     MaxSpeed=+01300.000000
     Mesh=Unreal.BoulderM
     DrawScale=+00001.700000
     bMeshCurvy=False
     CollisionRadius=+00060.000000
}

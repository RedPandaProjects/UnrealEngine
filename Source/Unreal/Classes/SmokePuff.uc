//=============================================================================
// SmokePuff.
//=============================================================================
class SmokePuff expands Effects;

#exec MESH IMPORT MESH=SmokePuffM ANIVFILE=MODELS\puff_a.3D DATAFILE=MODELS\puff_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SmokePuffM X=0 Y=0 Z=0 YAW=0
#exec MESH SEQUENCE MESH=SmokePuffM SEQ=All     STARTFRAME=0   NUMFRAMES=2
#exec MESH SEQUENCE MESH=SmokePuffM SEQ=Puff    STARTFRAME=0   NUMFRAMES=2
#exec OBJ LOAD FILE=Textures\SmokeEffect1.utx PACKAGE=Unreal.SEffect1
#exec MESHMAP SCALE MESHMAP=SmokePuffM X=0.03 Y=0.03 Z=0.06
#exec MESHMAP SETTEXTURE MESHMAP=SmokePuffM NUM=1 TEXTURE=Unreal.SEffect1.Smoke1


function PostBeginPlay()
{
	PlayAnim( 'Puff', 0.3);	
	Super.PostBeginPlay();
}

function AnimEnd()
{
	Destroy();
}

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Unreal.SmokePuffM
     Physics=PHYS_None
}

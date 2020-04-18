//=============================================================================
// Blood2.
//=============================================================================
class Blood2 expands Effects;

#exec MESH IMPORT MESH=blood2M ANIVFILE=MODELS\blood2_a.3D DATAFILE=MODELS\blood2_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=blood2M X=0 Y=0 Z=0 YAW=128
#exec MESH SEQUENCE MESH=blood2M SEQ=All       STARTFRAME=0   NUMFRAMES=45
#exec MESH SEQUENCE MESH=blood2M SEQ=Spray     STARTFRAME=0   NUMFRAMES=6
#exec MESH SEQUENCE MESH=blood2M SEQ=Still     STARTFRAME=6   NUMFRAMES=1
#exec MESH SEQUENCE MESH=blood2M SEQ=GravSpray STARTFRAME=7   NUMFRAMES=5
#exec MESH SEQUENCE MESH=blood2M SEQ=Stream    STARTFRAME=12  NUMFRAMES=11
#exec MESH SEQUENCE MESH=blood2M SEQ=Trail     STARTFRAME=23  NUMFRAMES=11
#exec MESH SEQUENCE MESH=blood2M SEQ=Burst     STARTFRAME=34  NUMFRAMES=2
#exec MESH SEQUENCE MESH=blood2M SEQ=GravSpray2 STARTFRAME=36 NUMFRAMES=7

#exec TEXTURE IMPORT NAME=BloodSpot FILE=MODELS\bloods2.PCX GROUP=Skins FLAGS=2
#exec TEXTURE IMPORT NAME=BloodSGrn FILE=MODELS\bloodg2.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=blood2M X=0.055 Y=0.055 Z=0.11 YAW=128
#exec MESHMAP SETTEXTURE MESHMAP=blood2M NUM=0  TEXTURE=BloodSpot


function GreenBlood()
{
	Texture = texture'BloodSGrn';
}

auto state Explode
{
Begin:
	// note: spawner should play appropriate anim
	FinishAnim();
  	Destroy   ();
}

defaultproperties
{
     RemoteRole=ROLE_SimulatedProxy
     DrawType=DT_Mesh
     Style=STY_Masked
     Texture=Texture'Unreal.BloodSpot'
     Mesh=Mesh'Unreal.Blood2M'
     DrawScale=0.250000
     AmbientGlow=56
     bUnlit=True
     bParticles=True
}

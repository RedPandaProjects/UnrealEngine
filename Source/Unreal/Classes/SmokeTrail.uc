//=============================================================================
// SmokeTrail.
//=============================================================================
class SmokeTrail expands Effects;

#exec MESH IMPORT MESH=SmoketrailM ANIVFILE=MODELS\Smoke1_a.3D DATAFILE=MODELS\Smoke1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SmoketrailM X=0 Y=0 Z=0 YAW=-64
#exec MESH SEQUENCE MESH=SmoketrailM SEQ=All     STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=SmoketrailM X=0.2 Y=0.2 Z=0.4 YAW=128
#exec MESHMAP SETTEXTURE MESHMAP=SmoketrailM NUM=0 TEXTURE=Default

defaultproperties
{
     DrawType=DT_Mesh
     Mesh=Unreal.SmokeTrailM
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     RemoteRole=ROLE_SimulatedProxy
}

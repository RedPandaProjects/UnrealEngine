//=============================================================================
// BulletHit.
//=============================================================================
class BulletHit expands Effects;

#exec MESH IMPORT MESH=BulletHitM ANIVFILE=MODELS\Burst4_a.3D DATAFILE=MODELS\Burst4_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=BulletHitM X=0 Y=0 Z=0 YAW=64 ROLL=64 
#exec MESH SEQUENCE MESH=BulletHitM SEQ=All      STARTFRAME=0   NUMFRAMES=32
#exec MESH SEQUENCE MESH=BulletHitM SEQ=Bounce   STARTFRAME=0   NUMFRAMES=11
#exec MESH SEQUENCE MESH=BulletHitM SEQ=Fall     STARTFRAME=11  NUMFRAMES=7
#exec MESH SEQUENCE MESH=BulletHitM SEQ=Explo    STARTFRAME=18  NUMFRAMES=5
#exec MESH SEQUENCE MESH=BulletHitM SEQ=Still    STARTFRAME=23  NUMFRAMES=2
#exec MESH SEQUENCE MESH=BulletHitM SEQ=Bubble   STARTFRAME=25  NUMFRAMES=7
#exec TEXTURE IMPORT NAME=Jmisc1 FILE=MODELS\misc.PCX GROUP=Skins
#exec TEXTURE IMPORT NAME=Jmisc2 FILE=MODELS\misc2.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=BulletHitM X=0.06 Y=0.06 Z=0.12 YAW=128
#exec MESHMAP SETTEXTURE MESHMAP=BulletHitM NUM=0 TEXTURE=Jmisc1

var name AnimType ;
var rotator MyRotation;

auto state Explode
{
Begin:
	if (AnimType=='Bounce') SetPhysics(PHYS_None);
	if (AnimType=='Bounce' || AnimType=='Explo') {
		MyRotation = Rotation;
		MyRotation.Yaw = Frand()*65535;
		MyRotation.Pitch += -32768;
		SetRotation(MyRotation);
	}
	if (AnimType=='Fall') {
		MyRotation = Rotation;
		MyRotation.Yaw += Frand()*8000-4000;
		SetRotation(MyRotation);
	}
	PlaySound (EffectSound1);	
	PlayAnim   ( AnimType, 0.5 );
	FinishAnim ();
  	Destroy();
}
defaultproperties
{
     DrawType=DT_Mesh
     Mesh=BulletHitM
     AmbientGlow=64
     CollisionRadius=+00000.000000
     CollisionHeight=+00000.000000
     RemoteRole=ROLE_SimulatedProxy
}

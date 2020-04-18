//=============================================================================
// TazerExplosion.
//=============================================================================
class TazerExplosion expands Effects;

#exec MESH IMPORT MESH=TazerExpl ANIVFILE=MODELS\tex_a.3D DATAFILE=MODELS\tex_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TazerExpl X=0 Y=0 Z=0 YAW=0
#exec MESH SEQUENCE MESH=TazerExpl SEQ=All       STARTFRAME=0   NUMFRAMES=6
#exec MESH SEQUENCE MESH=TazerExpl SEQ=Explosion STARTFRAME=0   NUMFRAMES=6
#exec OBJ LOAD FILE=Textures\fireeffect3.utx PACKAGE=Unreal.Effect3
#exec MESHMAP SCALE MESHMAP=TazerExpl X=.4 Y=0.4 Z=0.8 YAW=128
#exec MESHMAP SETTEXTURE MESHMAP=TazerExpl NUM=1 TEXTURE=Unreal.Effect3.FireEffect3a 

var rotator NormUp;
var() float Damage;
var() float radius;
var() float MomentumTransfer;

auto state Explode
{
Begin:
	PlayAnim( 'Explosion', 1 );
	PlaySound (EffectSound1);
	MakeNoise(1.0);				
	FinishAnim();
	Destroy();	
}

defaultproperties
{
     Damage=+00040.000000
     Radius=+00120.000000
     MomentumTransfer=+01400.000000
     EffectSound1=Unreal.Explode1
     DrawType=DT_Mesh
     Mesh=Unreal.TazerExpl
     RemoteRole=ROLE_SimulatedProxy
}

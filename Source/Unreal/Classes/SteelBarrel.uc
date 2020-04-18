//=============================================================================
// SteelBarrel.
//=============================================================================
class SteelBarrel expands Decoration;

#exec AUDIO IMPORT FILE="sounds\general\bPush1.wav" NAME="ObjectPush" GROUP="General"
#exec AUDIO IMPORT FILE="sounds\general\EndPush.wav" NAME="Endpush" GROUP="General"

#exec MESH IMPORT MESH=steelbarrelM ANIVFILE=MODELS\steelb_a.3D DATAFILE=MODELS\steelb_d.3D X=0 Y=0 Z=0 ZEROTEX=1
#exec MESH ORIGIN MESH=steelbarrelM X=0 Y=0 Z=-20 YAW=0 ROLL=128
#exec MESH SEQUENCE MESH=steelbarrelM SEQ=All    STARTFRAME=0  NUMFRAMES=2
#exec MESH SEQUENCE MESH=steelbarrelM SEQ=Normal STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=steelbarrelM SEQ=Crush  STARTFRAME=1  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jsteelbarrel1 FILE=MODELS\steelb.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=steelbarrelM X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=steelbarrelM NUM=0 TEXTURE=Jsteelbarrel1

var() int Health;

Auto State Animate
{

	function HitWall (vector HitNormal, actor Wall)
	{
		if (VSize(Velocity)>200) PlayAnim('Crush');
		Velocity = 0.8*(( Velocity dot HitNormal ) * HitNormal * (-1.8 + FRand()*0.8) + Velocity);   // Reflect off Wall w/damping
		Velocity.Z = Velocity.Z*0.6;
		If (VSize(Velocity) < 5) {
			SetPhysics(PHYS_None);	
			bBounce = False;
		}
	}

	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
			SetPhysics(PHYS_Falling);
			bBounce = True;
			Momentum.Z = 1000;
			Velocity=Momentum*0.01;
	}


Begin:
	PlayAnim('normal');
}

defaultproperties
{
     Health=100
     bPushable=True
     PushSound=Sound'Unreal.General.ObjectPush'
     EndPushSound=Sound'Unreal.General.Endpush'
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Mesh'Unreal.SteelBarrelM'
     bMeshCurvy=False
     CollisionRadius=14.000000
     CollisionHeight=23.500000
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     bProjTarget=True
     Mass=100.000000
     Buoyancy=1.000000
}

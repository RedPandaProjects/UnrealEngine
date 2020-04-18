//=============================================================================
// SteelBox.
//=============================================================================
class SteelBox expands Decoration;

#exec AUDIO IMPORT FILE="sounds\general\bPush1.wav" NAME="ObjectPush" GROUP="General"
#exec AUDIO IMPORT FILE="sounds\general\EndPush.wav" NAME="Endpush" GROUP="General"

#exec MESH IMPORT MESH=SteelBoxM ANIVFILE=MODELS\Box_a.3D DATAFILE=MODELS\Box_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=SteelBoxM X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=SteelBoxM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=SteelBoxM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JSteelBox1 FILE=MODELS\steelBox.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=SteelBoxM X=0.08 Y=0.08 Z=0.16
#exec MESHMAP SETTEXTURE MESHMAP=SteelBoxM NUM=1 TEXTURE=JSteelBox1


Auto State Animate
{

	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		Instigator = InstigatedBy;
		SetPhysics(PHYS_Falling);
		Momentum.Z = 1000;
		Velocity=Momentum*0.016;
	}
}

defaultproperties
{
     bPushable=True
     PushSound=Sound'Unreal.General.ObjectPush'
     EndPushSound=Sound'Unreal.General.Endpush'
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Mesh'Unreal.SteelBoxM'
     bMeshCurvy=False
     CollisionRadius=29.000000
     CollisionHeight=26.000000
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     Mass=60.000000
}

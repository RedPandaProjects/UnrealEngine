//=============================================================================
// Vase.
//=============================================================================
class Vase expands Decoration;

#exec AUDIO IMPORT FILE="sounds\general\bPush1.wav" NAME="ObjectPush" GROUP="General"
#exec AUDIO IMPORT FILE="sounds\general\EndPush.wav" NAME="Endpush" GROUP="General"

#exec MESH IMPORT MESH=vaseM ANIVFILE=MODELS\vase_a.3D DATAFILE=MODELS\vase_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=vaseM X=0 Y=0 Z=-320 YAW=64
#exec MESH SEQUENCE MESH=vaseM SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=vaseM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jvase1 FILE=MODELS\vase.PCX GROUP=Skins

#exec MESHMAP SCALE MESHMAP=vaseM X=0.1 Y=0.1 Z=0.2

#exec MESHMAP SETTEXTURE MESHMAP=vaseM NUM=1 TEXTURE=Jvase1



auto state active
{
	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		skinnedFrag(class'Fragment1',texture'JVase1', Momentum,0.7,7);
		Instigator = InstigatedBy;
		if ( Instigator != None )
			MakeNoise(1.0);
	}

Begin:
}

defaultproperties
{
     bPushable=True
     PushSound=Sound'Unreal.General.ObjectPush'
     EndPushSound=Sound'Unreal.General.Endpush'
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Mesh'Unreal.VaseM'
     CollisionHeight=28.000000
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     Mass=100.000000
}

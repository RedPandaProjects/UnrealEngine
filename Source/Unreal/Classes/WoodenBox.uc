//=============================================================================
// WoodenBox.
//=============================================================================
class WoodenBox expands Decoration;

#exec AUDIO IMPORT FILE="sounds\general\bPush1.wav" NAME="ObjectPush" GROUP="General"
#exec AUDIO IMPORT FILE="sounds\general\EndPush.wav" NAME="Endpush" GROUP="General"

#exec MESH IMPORT MESH=WoodenBoxM ANIVFILE=MODELS\Box_a.3D DATAFILE=MODELS\Box_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=WoodenBoxM X=0 Y=0 Z=0 YAW=64
#exec MESH SEQUENCE MESH=WoodenBoxM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=WoodenBoxM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JWoodenBox1 FILE=MODELS\Box.PCX GROUP=Skins
#exec MESHMAP SCALE MESHMAP=WoodenBoxM X=0.08 Y=0.08 Z=0.16
#exec MESHMAP SETTEXTURE MESHMAP=WoodenBoxM NUM=1 TEXTURE=JWoodenBox1

var() int Health;
var() int FragChunks;
var() Float Fragsize;

function PreBeginPlay()
{
	// some boxes will float (randomly)
	if ( Buoyancy == Default.Buoyancy)
		Buoyancy = Mass * (0.9 + 0.6 * FRand());

	Super.PreBeginPlay();
}

Auto State Animate
{

	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		Instigator = InstigatedBy;
		if (Health<0) Return;
		if ( Instigator != None )
			MakeNoise(1.0);
		bBobbing = false;
		Health -= NDamage;
		if (Health <0) 	
			Frag(Class'WoodFragments',Momentum,FragSize,FragChunks);		
		else 
		{
			SetPhysics(PHYS_Falling);
			Momentum.Z = 1000;
			Velocity=Momentum*0.016;
		}
	}
}

defaultproperties
{
     Health=20
     FragChunks=12
     Fragsize=1.750000
     bPushable=True
     PushSound=Sound'Unreal.General.ObjectPush'
     EndPushSound=Sound'Unreal.General.Endpush'
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Mesh'Unreal.WoodenBoxM'
     bMeshCurvy=False
     CollisionRadius=29.000000
     CollisionHeight=26.000000
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     Mass=50.000000
}

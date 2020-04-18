//=============================================================================
// MonkStatue.
//=============================================================================
class MonkStatue expands Decoration;

#exec  MESH IMPORT MESH=MonkStatueM ANIVFILE=MODELS\monk_a.3D DATAFILE=MODELS\monk_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=MonkStatueM X=0 Y=0 Z=0 YAW=0
#exec MESH SEQUENCE MESH=MonkStatueM SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=MonkStatueM SEQ=Still STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JMonkStatue1 FILE=MODELS\monk.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=MonkStatueM X=0.12 Y=0.12 Z=0.24
#exec MESHMAP SETTEXTURE MESHMAP=MonkStatueM NUM=1 TEXTURE=JMonkStatue1

#exec AUDIO IMPORT FILE="sounds\general\bPush1.wav" NAME="ObjectPush" GROUP="General"
#exec AUDIO IMPORT FILE="sounds\general\EndPush.wav" NAME="Endpush" GROUP="General"

Auto State Animate
{
	function HitWall (vector HitNormal, actor Wall)
	{
		if ( (Velocity.z<-200) && (HitNormal.Z > 0.5)  
			|| (Rotation.Pitch>4000) && (Rotation.Pitch<61000) )
			skinnedFrag(class'Fragment1',texture, VRand() * 40000,DrawScale*2.0,20);
		Velocity = 0.8*(( Velocity dot HitNormal ) * HitNormal * (-1.8 + FRand()*0.8) + Velocity);   // Reflect off Wall w/damping
		Velocity.Z = Velocity.Z*0.6;
		If ( (HitNormal.Z > 0.7) && (VSize(Velocity) < 60) )
		{
			SetPhysics(PHYS_None);	
			bBounce = False;
		}
	}

	function Timer()
	{
		if (Velocity.z<-80) 
		{
			RotationRate.Yaw = 15000;
			RotationRate.Pitch = 15000;
			RotationRate.Roll = 15000;	
			bRotatetoDesired=True;
			DesiredRotation.Pitch=16000;	
			DesiredRotation.Yaw=0;
			DesiredRotation.Roll=0;
		}		
	}

	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		Instigator = InstigatedBy;
		if ( Instigator != None )
			MakeNoise(1.0);
		SetPhysics(PHYS_Falling);
		bBounce = True;
		Momentum.Z = 1000;
		Velocity=Momentum*0.01;
		SetTimer(0.4,False);			
	}
}

function Bump( Actor Other )
{
	bBounce = ( bPushable && (Pawn(Other)!=None) );
	if ( bBounce )
		Super.Bump(Other);
}

defaultproperties
{
     bPushable=True
     PushSound=Sound'Unreal.General.ObjectPush'
     EndPushSound=Sound'Unreal.General.Endpush'
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Mesh'Unreal.MonkStatueM'
     bMeshCurvy=False
     CollisionHeight=88.000000
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     bBounce=True
     Mass=100.000000
     Buoyancy=1.000000
}

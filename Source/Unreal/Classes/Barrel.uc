//=============================================================================
// Barrel.
//=============================================================================
class Barrel expands Decoration;

#exec AUDIO IMPORT FILE="sounds\general\bPush1.wav" NAME="ObjectPush" GROUP="General"
#exec AUDIO IMPORT FILE="sounds\general\EndPush.wav" NAME="Endpush" GROUP="General"

#exec MESH IMPORT MESH=BarrelM ANIVFILE=MODELS\Barrel_a.3D DATAFILE=MODELS\Barrel_d.3D ZEROTEX=1
#exec MESH ORIGIN MESH=BarrelM X=320 Y=160 Z=95 YAW=64
#exec MESH SEQUENCE MESH=BarrelM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=BarrelM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JBarrel1 FILE=MODELS\Barrel.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=BarrelM X=0.15 Y=0.15 Z=0.3
#exec MESHMAP SETTEXTURE MESHMAP=BarrelM NUM=0 TEXTURE=JBarrel1


var() int Health;

Auto State Animate
{
	function HitWall (vector HitNormal, actor Wall)
	{
		if (Velocity.Z<-200) TakeDamage(100,Pawn(Owner),HitNormal,HitNormal*10000,'shattered');	
		bBounce = False;
		Velocity = vect(0,0,0);
	}


	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		Instigator = InstigatedBy;
		bBobbing = false;
		if (Health<0) Return;
		if ( Instigator != None )
			MakeNoise(1.0);
		Health -= NDamage;
		if (Health <0) 	
			Frag(class'WoodFragments',Momentum,1.75,12);		
		else 
		{
			SetPhysics(PHYS_Falling);
			bBounce = True;
			Momentum.Z = 1000;
			Velocity=Momentum*0.01;
		}
	}

Begin:
}

defaultproperties
{
     Health=10
     bPushable=True
     PushSound=Sound'Unreal.General.ObjectPush'
     EndPushSound=Sound'Unreal.General.Endpush'
     bStatic=False
     DrawType=DT_Mesh
     Skin=Texture'Unreal.Skins.JBarrel1'
     Mesh=Mesh'Unreal.BarrelM'
     CollisionRadius=24.000000
     CollisionHeight=29.000000
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     Mass=50.000000
     Buoyancy=60.000000
}

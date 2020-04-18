//=============================================================================
// Chair.
//=============================================================================
class Chair expands Decoration;


#exec MESH IMPORT MESH=Chair1 ANIVFILE=MODELS\chair_a.3D DATAFILE=MODELS\chair_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Chair1 X=0 Y=0 Z=0 ROLL=-64 
#exec MESH SEQUENCE MESH=Chair1 SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JChair11 FILE=MODELS\chair.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=Chair1 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=Chair1 NUM=1 TEXTURE=JChair11


var() int Health;
var() int FragChunks;
var() Float Fragsize;

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
     Health=15
     FragChunks=9
     Fragsize=+00001.200000
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Unreal.Chair1
     bMeshCurvy=False
     CollisionRadius=+00017.000000
     CollisionHeight=+00015.000000
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     Class=Unreal.Chair
}

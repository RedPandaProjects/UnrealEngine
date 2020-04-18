//=============================================================================
// Table.
//=============================================================================
class Table expands Decoration;

#exec MESH IMPORT MESH=Table1 ANIVFILE=MODELS\table_a.3D DATAFILE=MODELS\table_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Table1 X=0 Y=0 Z=0 ROLL=-64
#exec MESH SEQUENCE MESH=Table1 SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JTable11 FILE=MODELS\table.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=Table1 X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=Table1 NUM=1 TEXTURE=JTable11

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
     Health=25
     FragChunks=17
     Fragsize=+00001.000000
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Unreal.Table1
     bMeshCurvy=False
     CollisionRadius=+00045.000000
     CollisionHeight=+00019.000000
     bCollideActors=True
     bCollideWorld=True
     bBlockActors=True
     bBlockPlayers=True
     Class=Unreal.Table
}

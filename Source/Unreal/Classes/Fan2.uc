//=============================================================================
// Fan2.
//=============================================================================
class Fan2 expands Decoration;

#exec MESH IMPORT MESH=Fan2M ANIVFILE=MODELS\fan2_a.3D DATAFILE=MODELS\fan2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Fan2M X=0 Y=-300 Z=0 YAW=64

#exec MESH SEQUENCE MESH=fan2M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=fan2M SEQ=Still  STARTFRAME=0   NUMFRAMES=1


#exec TEXTURE IMPORT NAME=JFan21 FILE=MODELS\fan2.PCX GROUP=Skins FLAGS=2

#exec MESHMAP SCALE MESHMAP=Fan2M X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=fan2M NUM=1 TEXTURE=Jfan21


auto state active
{
	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		skinnedFrag(class'Fragment1',texture'JFan21', Momentum,2.0, 9);
		Instigator = InstigatedBy;
		if ( Instigator != None )
			MakeNoise(1.0);
	}

Begin:
}
defaultproperties
{
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Fan2M
     bMeshCurvy=False
     CollisionRadius=+00044.000000
     CollisionHeight=+00044.000000
     bCollideActors=True
     bCollideWorld=True
     bProjTarget=True
     Physics=PHYS_Rotating
     bFixedRotationDir=True
     RotationRate=(Roll=20000)
     DesiredRotation=(Roll=1)
}

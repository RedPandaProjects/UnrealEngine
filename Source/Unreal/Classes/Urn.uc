//=============================================================================
// Urn
//=============================================================================
class Urn expands Decoration;

#exec MESH IMPORT MESH=UrnM ANIVFILE=MODELS\urn_a.3D DATAFILE=MODELS\urn_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=UrnM X=0 Y=0 Z=-120 YAW=64
#exec MESH SEQUENCE MESH=urnM SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=urnM SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JUrn1 FILE=MODELS\urn.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=urnM X=0.025 Y=0.025 Z=0.05
#exec MESHMAP SETTEXTURE MESHMAP=urnM NUM=1 TEXTURE=Jurn1


auto state active
{
	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		skinnedFrag(class'Fragment1', texture'JUrn1', Momentum,0.5, 5);
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
     Mesh=UrnM
     CollisionRadius=+00019.000000
     CollisionHeight=+00011.000000
     bCollideActors=True
     bCollideWorld=True
     bProjTarget=True
     Mass=+00100.000000
}

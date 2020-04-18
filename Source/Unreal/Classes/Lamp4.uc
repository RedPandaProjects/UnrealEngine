//=============================================================================
// Lamp4.
//=============================================================================
class Lamp4 expands Decoration;

#exec MESH IMPORT MESH=Lamp4M ANIVFILE=MODELS\lamp4_a.3D DATAFILE=MODELS\lamp4_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Lamp4M X=0 Y=0 Z=0 YAW=64

#exec MESH SEQUENCE MESH=Lamp4M SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Lamp4M SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JLamp41 FILE=MODELS\lamp4.PCX GROUP=Skins FLAGS=2

#exec MESHMAP SCALE MESHMAP=Lamp4M X=0.05 Y=0.05 Z=0.1
#exec MESHMAP SETTEXTURE MESHMAP=Lamp4M NUM=1 TEXTURE=Jlamp41


auto state active
{
	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		skinnedFrag(class'Fragment1',texture'JLamp41', Momentum, 0.3, 5);
		if ( Instigator != None )
			MakeNoise(1.0);
	}

Begin:
}
defaultproperties
{
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Lamp4M
     CollisionHeight=+00032.000000
     bCollideActors=True
     bCollideWorld=True
     Physics=PHYS_None
}

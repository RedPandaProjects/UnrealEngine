//=============================================================================
// FemaleTwoCarcass.
// DO NOT USE THESE AS DECORATIONS
//=============================================================================
class FemaleTwoCarcass expands Female2Body;

function ForceMeshToExist()
{
	//never called
	Spawn(class 'FemaleTwo');
}

defaultproperties
{
     Mesh=Mesh'Unreal.Female2'
     AnimSequence=Dead1
	 Physics=PHYS_Falling
	 bBlockActors=true
	 bBlockPlayers=true
}
//=============================================================================
// FemaleOneCarcass.
// DO NOT USE THESE AS DECORATIONS
//=============================================================================
class FemaleOneCarcass expands FemaleBody;

function ForceMeshToExist()
{
	//never called
	Spawn(class 'FemaleOne');
}

defaultproperties
{
     Mesh=Mesh'Unreal.Female1'
     AnimSequence=Dead1
	 Physics=PHYS_Falling
     bBlockActors=true
     bBlockPlayers=true
}
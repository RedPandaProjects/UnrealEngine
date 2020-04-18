//=============================================================================
// FemaleOneBot.
//=============================================================================
class FemaleOneBot expands FemaleBot;

function ForceMeshToExist()
{
	Spawn(class'FemaleOne');
}

defaultproperties
{
     Mesh=Female1
	 CarcassType=FemaleOneCarcass
}

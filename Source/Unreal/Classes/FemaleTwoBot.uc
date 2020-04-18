//=============================================================================
// FemaleTwoBot.
//=============================================================================
class FemaleTwoBot expands FemaleBot;

function ForceMeshToExist()
{
	Spawn(class'FemaleTwo');
}


defaultproperties
{
    Mesh=Female2
	CarcassType=FemaleTwoCarcass
}
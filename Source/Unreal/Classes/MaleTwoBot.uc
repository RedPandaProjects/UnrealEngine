//=============================================================================
// MaleTwoBot.
//=============================================================================
class MaleTwoBot expands MaleBot;

#exec AUDIO IMPORT FILE="Sounds\male\jump10.WAV" NAME="MJump2" GROUP="Male"
#exec AUDIO IMPORT FILE="Sounds\male\land10.WAV" NAME="MLand2" GROUP="Male"

function ForceMeshToExist()
{
	Spawn(class'MaleTwo');
}


defaultproperties
{
     Mesh=Male2
	 JumpSound=MJump2
	 LandGrunt=MLand2
	 CarcassType=MaleTwoCarcass
}

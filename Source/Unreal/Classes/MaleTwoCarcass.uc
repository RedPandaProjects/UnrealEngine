//=============================================================================
// MaleTwoCarcass.
// DO NOT USE THESE AS DECORATIONS
//=============================================================================
class MaleTwoCarcass expands MaleBodyTwo;

function ForceMeshToExist()
{
	//never called
	Spawn(class 'MaleTwo');
}

defaultproperties
{
     Mesh=Mesh'Unreal.Male2'
     AnimSequence=Dead1
	 Physics=PHYS_Falling
	 bBlockActors=true
	 bBlockPlayers=true
}
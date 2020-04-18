//=============================================================================
// MaleThreeCarcass.
// DO NOT USE THESE AS DECORATIONS
//=============================================================================
class MaleThreeCarcass expands MaleBodyThree;

function ForceMeshToExist()
{
	//never called
	Spawn(class 'MaleThree');
}

defaultproperties
{
     Mesh=Mesh'Unreal.Male3'
     AnimSequence=Dead1
	 Physics=PHYS_Falling
	 bBlockActors=true
	 bBlockPlayers=true
}
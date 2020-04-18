//=============================================================================
// MaleOneCarcass.
// DO NOT USE THESE AS DECORATIONS
//=============================================================================
class MaleOneCarcass expands MaleBody;

function ForceMeshToExist()
{
	//never called
	Spawn(class 'MaleOne');
}

defaultproperties
{
     Mesh=Mesh'Unreal.Male1'
     AnimSequence=Dead1
	 Physics=PHYS_Falling
	 bBlockActors=true
	 bBlockPlayers=true
}
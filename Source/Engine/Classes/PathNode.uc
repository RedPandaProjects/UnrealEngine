//=============================================================================
// PathNode.
//=============================================================================
class PathNode expands NavigationPoint
	intrinsic;

/*
function PreBeginPlay()
{
	local int j;
	log("Describe paths for pathnode at "$Location);
	for (j=0; j<16; j++)
	{
		if (Paths[j] != -1)
			describeSpec(Paths[j]);
	}
}
*/

defaultproperties
{
     Texture=S_Pickup
     SoundVolume=128
}

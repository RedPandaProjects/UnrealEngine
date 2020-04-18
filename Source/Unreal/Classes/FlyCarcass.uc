//=============================================================================
// FlyCarcass.
//=============================================================================
class FlyCarcass expands CreatureCarcass;

function CreateReplacement()
{
	if (Bugs != None)
		Bugs.Destroy();
}

function ClientExtraChunks()
{
	Spawn(class 'BloodBurst');
}

function ForceMeshToExist()
{
	//never called
	Spawn(class 'Fly');
}

defaultproperties
{
     Mesh=FlyM
     Mass=+00100.000000
	 Buoyancy=+00099.000000
}

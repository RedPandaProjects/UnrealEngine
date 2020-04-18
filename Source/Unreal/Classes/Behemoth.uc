//=============================================================================
// Behemoth.
//=============================================================================
class Behemoth expands Brute;

#exec TEXTURE IMPORT NAME=Brute2 FILE=Models\Brute2H.PCX GROUP="Skins"

function GoBerserk()
{
	bLongBerserk = false;
	bBerserk = false;
}

defaultproperties
{
     WhipDamage=35
	 bLeadTarget=True
     Health=500
     SightRadius=+02000.000000
     ReFireRate=+00000.500000
	 DrawScale=+00001.300000
	 Skin=Brute2
     CollisionRadius=+00068.000000
     CollisionHeight=+00068.000000
     Mass=+00500.000000
	 TransientSoundVolume=+00006.000000
}

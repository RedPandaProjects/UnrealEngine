//=============================================================================
// SkaarjOfficer.
//=============================================================================
class SkaarjOfficer expands SkaarjTrooper;

#exec TEXTURE IMPORT NAME=sktrooper3 FILE=MODELS\zaarj3.PCX GROUP=Skins 

defaultproperties
{
	 WeaponType=Unreal.Razorjack
	 Health=200
	 Fatness=140
	 DrawScale=+00001.100000
     CollisionRadius=+00035.000000
     CollisionHeight=+00046.000000
     Mass=+00150.000000
     Buoyancy=+00150.000000
	 Skin=sktrooper3
}
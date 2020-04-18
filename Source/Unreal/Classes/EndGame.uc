//=============================================================================
// EndGame.
//=============================================================================
class EndGame expands UnrealGameInfo;

event AcceptInventory(pawn PlayerPawn)
{
	local inventory Inv;

	// accept no inventory
	for ( Inv=PlayerPawn.Inventory; Inv!=None; Inv=Inv.Inventory )
		Inv.Destroy(); 
}


function PlayTeleportEffect( actor Incoming, bool bOut, bool bSound)
{
}

defaultproperties
{
     DefaultWeapon=None
     HUDType=Class'Unreal.EndgameHud'
}

//=============================================================================
// VRikersGame.
//
// no default weapon
//
//=============================================================================
class VRikersGame expands UnrealGameInfo;

event playerpawn Login
(
	string[32] Portal,
	string[120] Options,
	out string[80] Error,
	class<playerpawn> SpawnClass
)
{
	local PlayerPawn NewPlayer;
	NewPlayer = Super.Login(Portal, Options, Error, SpawnClass);
	if ( (NewPlayer != None)
		&& (NewPlayer.Health == NewPlayer.Default.Health)  )
	{
		NewPlayer.ViewRotation.Pitch = 16384;
		NewPlayer.Health = 12;
	}
	return NewPlayer;
}

function bool RestartPlayer(pawn aPlayer)	
{
	local bool result;

	result = Super.RestartPlayer(aPlayer);
	if ( result )
	{
		aPlayer.Health = 12;
		aPlayer.ViewRotation.Pitch = 16384;
	}
	return result;
}

function int ReduceDamage(int Damage, name DamageType, pawn injured, pawn instigatedBy)
{
	// no friendly fire always in coop
	if ( (instigatedBy != None) 
		&& instigatedBy.bIsPlayer && injured.bIsPlayer && (instigatedBy != injured) )
		return 0;

	return Super.ReduceDamage(Damage, DamageType, injured, instigatedBy);
}

function AddDefaultInventory(pawn PlayerPawn)
{
	return;
}

function bool PickupQuery(Pawn Other, Inventory item)
{
	if ( item.IsA('DispersionPistol') )
		DefaultWeapon = class'DispersionPistol';
	return Super.PickupQuery(Other, item);
}

defaultproperties
{
	 bHumansOnly=true
     DefaultWeapon=None
}
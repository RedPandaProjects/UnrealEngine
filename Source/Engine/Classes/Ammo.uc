//=============================================================================
// Ammo.
//=============================================================================
class Ammo expands Pickup
	abstract;

#exec Texture Import File=Textures\Ammo.pcx Name=S_Ammo Mips=Off Flags=2

var() travel int AmmoAmount;
var() travel int MaxAmmo;
var() class<ammo> ParentAmmo;    // Class of ammo to be represented in inventory
var() byte UsedInWeaponSlot[10];
var   ammo  PAmmo;



// Network replication
//
replication
{
	// Things the server should send to the client.
	reliable if( Role==ROLE_Authority && bNetOwner )
		AmmoAmount;
}

event float BotDesireability(Pawn Bot)
{
	local Ammo AlreadyHas;

	if ( ParentAmmo != None )
		AlreadyHas = Ammo(Bot.FindInventoryType(ParentAmmo));
	else
		AlreadyHas = Ammo(Bot.FindInventoryType(Class));
	if ( AlreadyHas == None )
		return (0.4 * MaxDesireability);
	if ( AlreadyHas.AmmoAmount == 0 )
		return MaxDesireability;
	if (AlreadyHas.AmmoAmount >= AlreadyHas.MaxAmmo) 
		return -1;

	return ( MaxDesireability * FMin(1, 0.15 * MaxAmmo/AlreadyHas.AmmoAmount) );
}

function bool HandlePickupQuery( inventory Item )
{
	if ( (class == item.class) || 
		(ClassIsChildOf(item.class, class'Ammo') && (class == Ammo(item).parentammo)) ) 
	{
		if (AmmoAmount==MaxAmmo) return true;
		Pawn(Owner).ClientMessage(item.PickupMessage);
		item.PlaySound( item.PickupSound );
		AddAmmo(Ammo(item).AmmoAmount);
		item.SetRespawn();
		return true;				
	}
	if ( Inventory == None )
		return false;

	return Inventory.HandlePickupQuery(Item);
}

// This function is called by an actor that wants to use ammo.  
// Return true if ammo exists 
//
function bool UseAmmo(int AmountNeeded)
{
	if (AmmoAmount < AmountNeeded) return False;   // Can't do it
	AmmoAmount -= AmountNeeded;
	return True;
}

// If we can, add ammo and return true.  
// We we are at max ammo, return false
//
function bool AddAmmo(int AmmoToAdd)
{
	If (AmmoAmount >= MaxAmmo) return false;
	AmmoAmount += AmmoToAdd;
	if (AmmoAmount > MaxAmmo) AmmoAmount = MaxAmmo;
	return true;
}

function inventory SpawnCopy( Pawn Other )
{
	local Inventory Copy;

	if ( parentammo != None )
	{
		Copy = spawn(parentammo,Other);
		Copy.Tag           = Tag;
		Copy.Event         = Event;
		Copy.Instigator    = Other;
		Ammo(Copy).AmmoAmount = AmmoAmount;
		Copy.BecomeItem();
		Other.AddInventory( Copy );
		Copy.GotoState('');
		if ( Level.Game.ShouldRespawn(self) )
			GotoState('Sleeping');
		else
			Destroy();
		return Copy;
	}
	Copy = Super.SpawnCopy(Other);
	Ammo(Copy).AmmoAmount = AmmoAmount; 
	return Copy;
}

defaultproperties
{
     PickupMessage="You picked up some ammo."
     RespawnTime=+00030.000000
     MaxDesireability=+00000.200000
     Texture=Engine.S_Ammo
     bCollideActors=False
}

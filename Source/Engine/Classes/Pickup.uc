//=============================================================================
// Pickup items.
//=============================================================================
class Pickup expands Inventory
	localized
	abstract;

var inventory Inv;
var travel int NumCopies;
var() bool bCanHaveMultipleCopies;  // if player can possess more than one of this
var() bool bCanActivate;			// Item can be selected and activated
var() localized String[64] ExpireMessage; // Messages shown when pickup charge runs out
var() bool bAutoActivate;

replication
{
	// Things the server should send to the client.
	reliable if( Role==ROLE_Authority && bNetOwner )
		NumCopies;
}

//
// Advanced function which lets existing items in a pawn's inventory
// prevent the pawn from picking something up. Return true to abort pickup
// or if item handles the pickup
function bool HandlePickupQuery( inventory Item )
{
	if (item.class == class) 
	{
		if (bCanHaveMultipleCopies) 
		{   // for items like Artifact
			NumCopies++;
			Pawn(Owner).ClientMessage(PickupMessage);				
			Item.PlaySound (Item.PickupSound,,2.0);
			Item.SetRespawn();
		}
		else if ( bDisplayableInv ) 
		{		
			if ( Charge<Item.Charge )	
				Charge= Item.Charge;
			Pawn(Owner).ClientMessage(PickupMessage);						
			Item.PlaySound (PickupSound,,2.0);
			Item.SetReSpawn();
		}
		return true;				
	}
	if ( Inventory == None )
		return false;

	return Inventory.HandlePickupQuery(Item);
}

function float UseCharge(float Amount);

function inventory SpawnCopy( pawn Other )
{
	local inventory Copy;

	Copy = Super.SpawnCopy(Other);
	Copy.Charge = Charge;
	return Copy;
}

auto state Pickup
{	
	function Touch( actor Other )
	{
		local Inventory Copy;
		if ( ValidTouch(Other) ) 
		{
			Copy = SpawnCopy(Pawn(Other));
			if (bActivatable && Pawn(Other).SelectedItem==None) 
				Pawn(Other).SelectedItem=Copy;
			if (bActivatable && bAutoActivate && Pawn(Other).bAutoActivate) Copy.Activate();
			Pawn(Other).ClientMessage(PickupMessage);	// add to inventory and run pickupfunction	
			PlaySound (PickupSound,,2.0);	
			PickupFunction(Pawn(Other));
		}
	}

	function BeginState()
	{
		Super.BeginState();
		NumCopies = 0;
	}
}

function PickupFunction(Pawn Other)
{
}

//
// This is called when a usable inventory item has used up it's charge.
//
function UsedUp()
{
	if ( Pawn(Owner) != None )
	{
		Pawn(Owner).NextItem();
		if (Pawn(Owner).SelectedItem == Self) {
			Pawn(Owner).NextItem();	
			if (Pawn(Owner).SelectedItem == Self) Pawn(Owner).SelectedItem=None;
		}
		Pawn(Owner).ClientMessage(ExpireMessage);	
	}
	Owner.PlaySound(DeactivateSound);
	Destroy();
}


state Activated
{
	function Activate()
	{
		if ( (Pawn(Owner) != None) && Pawn(Owner).bAutoActivate 
			&& bAutoActivate && (Charge>0) )
				return;

		Super.Activate();	
	}
}

defaultproperties
{
     bRotatingPickup=False
}

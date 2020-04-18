//=============================================================================
// PowerShield.
//=============================================================================
class PowerShield expands ShieldBelt;

function Timer()
{
	Charge-=1;
	if (Charge<0) Destroy();
}

defaultproperties
{
     PickupMessage="You got the PowerShield"
     RespawnTime=+00100.000000
     Charge=200
     CollisionRadius=+00030.000000
     CollisionHeight=+00030.000000
}

//=============================================================================
// KingOfTheHill.
//=============================================================================
class KingOfTheHill expands DeathMatchGame
	localized;

var pawn king;
var localized string[128] KingMessage;

function Killed(pawn killer, pawn Other, name damageType)
{
	Other.AmbientGlow = 0;
	Other.LightType = LT_None;
	Other.DamageScaling = 1.0;
	Other.bUnLit = false;

	if ( (killer == None) || (killer == Other) )
	{
		if ( King == Other )
			King = None; 
	}
	else if ( (killer != king) 
			&& ((king == None) || (king == Other)) )
		CrownNewKing(killer);
		
	Super.Killed(killer, Other, damageType);
}

function CrownNewKing(pawn newKing)
{
	local Inventory Inv;
	
	King = newKing;
	BroadcastMessage(newKing.PlayerName$KingMessage, true);
	NewKing.health = Max(NewKing.Health, 100);
	NewKing.bUnLit = true; 
	NewKing.Score += 5.0;
	NewKing.DamageScaling = 2.0;
	NewKing.LightEffect=LE_NonIncidence;
	NewKing.LightBrightness=255;
	NewKing.LightHue=0;
	NewKing.LightRadius=10;
	NewKing.LightSaturation=0;
	NewKing.AmbientGlow = 200;
	NewKing.bUnlit = true;
	NewKing.LightType=LT_Steady;
}

defaultproperties
{
	BeaconName="King"
	KingMessage=" is the new king of the hill!"
}

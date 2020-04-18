//=============================================================================
// DamageType, the base class of all damagetypes.
// this and its subclasses are never spawned, just used as information holders
//=============================================================================
class DamageType expands Actor
	localized
	abstract;

// Description of a type of damage.
var() localized string[32] Name;         // Description of damage.
var() localized string[32] AltName;      // Alternative description.
var() float                ViewFlash;    // View flash to play.
var() vector               ViewFog;      // View fog to play.
var() class<effects>       DamageEffect; // Special effect.

static function string[64] DeathMessage()
{
	if( FRand() < 0.5 )
		return Default.Name;
	else 
		return Default.AltName;
}

defaultproperties
{
	 name="killed"
	 altname="killed"
}

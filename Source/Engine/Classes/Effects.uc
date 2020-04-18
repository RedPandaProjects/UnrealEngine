//=============================================================================
// Effects, the base class of all gratuitous special effects.
//=============================================================================
class Effects expands Actor;

var() sound 	EffectSound1;
var() sound 	EffectSound2;
var() bool bOnlyTriggerable;

defaultproperties
{
     DrawType=DT_None
     Physics=PHYS_None
	 bAlwaysRelevant=true
	 CollisionRadius=+0.00000
	 CollisionHeight=+0.00000
}

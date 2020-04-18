//=============================================================================
// SmokeHoseDest.
//=============================================================================
class SmokeHoseDest expands Effects;

// The position of this actor implies the direction the smokepuffs will
// be shot out from the SmokeHose.
// The Tag of this actor must be the same as the DestTag of the SmokeHose Actor.

defaultproperties
{
     bHidden=True
     DrawType=DT_Sprite
     Physics=PHYS_None
}

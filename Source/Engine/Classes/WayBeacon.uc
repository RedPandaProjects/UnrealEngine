//=============================================================================
// WayBeacon.
//=============================================================================
class WayBeacon expands Keypoint;

//temporary beacon for serverfind navigation

function touch(actor other)
{
	if (other == owner)
		PlayerPawn(owner).ShowPath();
}

defaultproperties
{
     bStatic=False
     bHidden=False
     DrawType=DT_Mesh
     Mesh=Lamp4
     DrawScale=+00000.500000
     AmbientGlow=40
     bOnlyOwnerSee=True
     bCollideActors=True
     LightType=LT_Steady
     LightBrightness=125
     LightSaturation=125
     LifeSpan=+00006.000000
}

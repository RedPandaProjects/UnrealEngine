//=============================================================================
// FlashLightBeam.
//=============================================================================
class FlashLightBeam expands Light;

function BeginPlay()
{
	SetTimer(1.0,True);
}

function Timer()
{
	MakeNoise(0.3);
}

defaultproperties
{
     bStatic=False
     bNoDelete=False
     bMovable=True
     bMeshCurvy=False
     LightEffect=LE_NonIncidence
     LightBrightness=250
     LightHue=32
     LightSaturation=142
     LightRadius=7
     LightPeriod=0
}

//=============================================================================
// SeekingRocket.
//=============================================================================
class SeekingRocket expands rocket;

simulated function PostBeginPlay()
{
	Count = -0.1;
	if (Level.bHighDetailMode) SmokeRate = 0.035;
	else SmokeRate = 0.15;
}

function Timer()
{
	local vector SeekingDir;

	If (Seeking != None  && Seeking != Instigator) 
	{
		SeekingDir = Normal(Seeking.Location - Location);
		if ( (SeekingDir Dot InitialDir) > 0 )
		{
			MagnitudeVel = VSize(Velocity);
			Velocity =  MagnitudeVel * Normal(SeekingDir * 0.47 * MagnitudeVel + Velocity);		
			SetRotation(rotator(Velocity));
		}
	}
}

auto state Flying
{
	function BeginState()
	{	
		SetTimer(0.15,True);
		Super.BeginState();
	}
}	

defaultproperties
{
     RemoteRole=ROLE_DumbProxy
     LifeSpan=10.000000
     LightBrightness=182
     LightHue=27
     LightSaturation=75
     LightRadius=8
     bCorona=False
}

//=============================================================================
// RazorBladeAlt.
//=============================================================================
class RazorBladeAlt expands RazorBlade;


auto state Flying
{

	simulated function SetRoll(vector NewVelocity) 
	{
		local rotator newRot;

		newRot = rotator(NewVelocity);
		newRot.Roll += 12768;		
		SetRotation(newRot);				
	}

	function Timer()
	{
		local rotator newRot;

		newRot = instigator.ViewRotation;
		newRot.Roll += 12768;
		SetRotation(newRot);	
		Velocity = VSize(Velocity)*Vector(Rotation);
	}

Begin:
	SetTimer(0.1,true);		
	maxspeed = speed;
	PitchStart = Instigator.ViewRotation.Pitch;
	YawStart = Instigator.ViewRotation.Yaw;	
	Sleep(0.2);
	bCanHitInstigator = true;
}

defaultproperties
{
     RemoteRole=ROLE_DumbProxy
}

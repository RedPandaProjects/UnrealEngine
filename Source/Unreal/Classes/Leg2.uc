//=============================================================================
// Leg2.
//=============================================================================
class Leg2 expands Leg1;

function Initfor(actor Other)
{
	bDecorative = false;
	Mass = 0.2 * Other.Mass;
	Buoyancy = 0.9 * Mass;
}

function Landed(vector HitNormal)
{
	SetPhysics(PHYS_Rotating);
	Velocity = vect(0,0,0);
	PlaySound(sound 'gibP1');
	Spawn(class 'Bloodspurt');
	Acceleration = vect(0,0,0);
	SetCollision(true, false, false);
}

auto state fallover
{

	function BeginState()
	{
		local rotator newRot;
		newRot = Rotation;
		newRot.Pitch = 16384;
		newRot.Yaw += 32768;
		setRotation(newRot);
	}
			
Begin:
	Sleep(3.0);
	bRotateToDesired = true;
	DesiredRotation = Rotation;
	DesiredRotation.Pitch = 0;
	if (Physics != PHYS_Falling)
	{
		SetPhysics(PHYS_Falling);
		Velocity = vect(0,0,150);
	}
	GotoState('Dead');
}

defaultproperties
{
      bBounce=False
      bFixedRotationDir=False
      RotationRate=(Pitch=75000,Yaw=0,Roll=0)
      LifeSpan=+00090.000000
}
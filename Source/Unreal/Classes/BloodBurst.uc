//=============================================================================
// BloodBurst.
//=============================================================================
class BloodBurst expands Blood2;


auto state Explode
{
Begin:
	PlayAnim  ( 'Burst', 0.2 );
	SetRotation( RotRand() );
	FinishAnim();
  	Destroy   ();
}

defaultproperties
{
     DrawScale=0.400000
     AmbientGlow=80
}

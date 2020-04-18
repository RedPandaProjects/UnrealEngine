//=============================================================================
// BloodSpurt.
//=============================================================================
class BloodSpurt expands Blood2;


auto state Explode
{
Begin:
	PlayAnim  ( 'GravSpray2', 0.9 );
	FinishAnim();
  	Destroy   ();
}

defaultproperties
{
     DrawScale=0.200000
     ScaleGlow=1.300000
     AmbientGlow=0
}

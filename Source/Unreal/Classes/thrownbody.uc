//=============================================================================
// ThrownBody.
//=============================================================================
class ThrownBody expands MaleBody;

#exec AUDIO IMPORT FILE="Sounds\Male\injurL04.WAV" NAME="MInjur2" GROUP="Male"

	function TakeDamage( int Damage, Pawn InstigatedBy, Vector Hitlocation, 
							Vector Momentum, name DamageType)
	{	
		if ( DamageType == 'exploded' )
			ChunkUp(Damage);
	}

	function HitWall(vector HitNormal, actor Wall)
	{
	
		DesiredRotation.Pitch = 0;
		DesiredRotation.Roll = 0;
		bRotateToDesired = true;

		Spawn(class 'Bloodspurt',,,,Rotator(HitNormal));
		PlaySound(sound'MInjur2', SLOT_None, 8);
		TweenAnim('Slump2', 0.25);
		Velocity = 0.7 * (Velocity - 2 * HitNormal * (Velocity Dot HitNormal));
		Velocity.Z *= 0.9;
		bBounce = false;
		Disable('HitWall');
	}
	
	function Landed(vector HitNormal)
	{
		SetPhysics(PHYS_Rotating);
	 	SetCollision(bCollideActors, false, false);
		if ( HitNormal.Z < 0.99 )
			ReducedHeightFactor = 0.1;
		if ( HitNormal.Z < 0.93 )
			ReducedHeightFactor = 0.0;
		if ( !IsAnimating() )
			LieStill();
	}

defaultproperties
{
     AnimSequence=Hang2
	 bPushable=true
	 Rotation=(Pitch=8000)
     RotationRate=(Pitch=80000, Roll=80000)
}
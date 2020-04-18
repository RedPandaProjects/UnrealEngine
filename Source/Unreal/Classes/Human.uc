//=============================================================================
// Human.
//=============================================================================
class Human expands UnrealPlayer
	abstract;

#exec AUDIO IMPORT FILE="Sounds\female\stone02.WAV" NAME="stwalk1" GROUP="Female"
#exec AUDIO IMPORT FILE="Sounds\female\stone04.WAV" NAME="stwalk2" GROUP="Female"
#exec AUDIO IMPORT FILE="Sounds\female\stone05.WAV" NAME="stwalk3" GROUP="Female"

//-----------------------------------------------------------------------------
// Animation functions

function PlayTurning()
{
	BaseEyeHeight = Default.BaseEyeHeight;
	if ( (Weapon == None) || (Weapon.Mass < 20) )
		PlayAnim('TurnSM', 0.3, 0.3);
	else
		PlayAnim('TurnLG', 0.3, 0.3);
}

function TweenToWalking(float tweentime)
{
	BaseEyeHeight = Default.BaseEyeHeight;
	if (Weapon == None)
		TweenAnim('Walk', tweentime);
	else if ( Weapon.bPointing || (CarriedDecoration != None) ) 
	{
		if (Weapon.Mass < 20)
			TweenAnim('WalkSMFR', tweentime);
		else
			TweenAnim('WalkLGFR', tweentime);
	}
	else
	{
		if (Weapon.Mass < 20)
			TweenAnim('WalkSM', tweentime);
		else
			TweenAnim('WalkLG', tweentime);
	} 
}

function TweenToRunning(float tweentime)
	{
		BaseEyeHeight = Default.BaseEyeHeight;
		if (bIsWalking)
		{
			TweenToWalking(0.1);
			return;
		}

		if (Weapon == None)
			PlayAnim('RunSM', 0.9, tweentime);
		else if ( Weapon.bPointing ) 
		{
			if (Weapon.Mass < 20)
				PlayAnim('RunSMFR', 0.9, tweentime);
			else
				PlayAnim('RunLGFR', 0.9, tweentime);
		}
		else
		{
			if (Weapon.Mass < 20)
				PlayAnim('RunSM', 0.9, tweentime);
			else
				PlayAnim('RunLG', 0.9, tweentime);
		} 
	}

function PlayWalking()
	{
		BaseEyeHeight = Default.BaseEyeHeight;
		if (Weapon == None)
			LoopAnim('Walk');
		else if ( Weapon.bPointing || (CarriedDecoration != None) ) 
		{
			if (Weapon.Mass < 20)
				LoopAnim('WalkSMFR');
			else
				LoopAnim('WalkLGFR');
		}
		else
		{
			if (Weapon.Mass < 20)
				LoopAnim('WalkSM');
			else
				LoopAnim('WalkLG');
		}
	}

function PlayRunning()
	{
		BaseEyeHeight = Default.BaseEyeHeight;
		if (Weapon == None)
			LoopAnim('RunSM');
		else if ( Weapon.bPointing ) 
		{
			if (Weapon.Mass < 20)
				LoopAnim('RunSMFR');
			else
				LoopAnim('RunLGFR');
		}
		else
		{
			if (Weapon.Mass < 20)
				LoopAnim('RunSM');
			else
				LoopAnim('RunLG');
		}
	}

function PlayRising()
{
	BaseEyeHeight = 0.4 * Default.BaseEyeHeight;
	TweenAnim('DuckWlkS', 0.7);
}

function PlayFeignDeath()
{
	local float decision;

	BaseEyeHeight = 0;
	if ( decision < 0.33 )
		TweenAnim('DeathEnd', 0.5);
	else if ( decision < 0.67 )
		TweenAnim('DeathEnd2', 0.5);
	else 
		TweenAnim('DeathEnd3', 0.5);
}

function PlayDying(name DamageType, vector HitLoc)
{
	local vector X,Y,Z, HitVec, HitVec2D;
	local float dotp;
	local carcass carc;

	BaseEyeHeight = Default.BaseEyeHeight;
	PlayDyingSound();
			
	if ( FRand() < 0.15 )
	{
		PlayAnim('Dead3',0.7,0.1);
		return;
	}

	// check for big hit
	if ( (Velocity.Z > 250) && (FRand() < 0.7) )
	{
		PlayAnim('Dead2', 0.7, 0.1);
		return;
	}

	// check for head hit
	if ( (DamageType == 'Decapitated') || (HitLoc.Z - Location.Z > 0.6 * CollisionHeight) )
	{
		DamageType = 'Decapitated';
		if ( Level.NetMode != NM_Client )
		{
			carc = Spawn(class 'FemaleHead',,, Location + CollisionHeight * vect(0,0,0.8), Rotation + rot(3000,0,16384) );
			if (carc != None)
			{
				carc.Initfor(self);
				carc.Velocity = Velocity + VSize(Velocity) * VRand();
				carc.Velocity.Z = FMax(carc.Velocity.Z, Velocity.Z);
			}
		}
		PlayAnim('Dead6', 0.7, 0.1);
		return;
	}

	
	if ( FRand() < 0.15)
	{
		PlayAnim('Dead1', 0.7, 0.1);
		return;
	}

	GetAxes(Rotation,X,Y,Z);
	X.Z = 0;
	HitVec = Normal(HitLoc - Location);
	HitVec2D= HitVec;
	HitVec2D.Z = 0;
	dotp = HitVec2D dot X;
	
	if (Abs(dotp) > 0.71) //then hit in front or back
		PlayAnim('Dead4', 0.7, 0.1);
	else
	{
		dotp = HitVec dot Y;
		if (dotp > 0.0)
		{
			PlayAnim('Dead7', 0.7, 0.1);
			carc = Spawn(class 'Arm1');
			if (carc != None)
			{
				carc.Initfor(self);
				carc.Velocity = Velocity + VSize(Velocity) * VRand();
				carc.Velocity.Z = FMax(carc.Velocity.Z, Velocity.Z);
			}
		}
		else
			PlayAnim('Dead5', 0.7, 0.1);
	}
}

function PlayGutHit(float tweentime)
{
	if ( (AnimSequence == 'GutHit') || (AnimSequence == 'Dead2') )
	{
		if (FRand() < 0.5)
			TweenAnim('LeftHit', tweentime);
		else
			TweenAnim('RightHit', tweentime);
	}
	else if ( FRand() < 0.6 )
		TweenAnim('GutHit', tweentime);
	else
		TweenAnim('Dead2', tweentime);

}

function PlayHeadHit(float tweentime)
{
	if ( (AnimSequence == 'HeadHit') || (AnimSequence == 'Dead4') )
		TweenAnim('GutHit', tweentime);
	else if ( FRand() < 0.6 )
		TweenAnim('HeadHit', tweentime);
	else
		TweenAnim('Dead4', tweentime);
}

function PlayLeftHit(float tweentime)
{
	if ( (AnimSequence == 'LeftHit') || (AnimSequence == 'Dead3') )
		TweenAnim('GutHit', tweentime);
	else if ( FRand() < 0.6 )
		TweenAnim('LeftHit', tweentime);
	else 
		TweenAnim('Dead3', tweentime);
}

function PlayRightHit(float tweentime)
{
	if ( (AnimSequence == 'RightHit') || (AnimSequence == 'Dead5') )
		TweenAnim('GutHit', tweentime);
	else if ( FRand() < 0.6 )
		TweenAnim('RightHit', tweentime);
	else
		TweenAnim('Dead5', tweentime);
}
	
function PlayLanded(float impactVel)
{	
	impactVel = impactVel/JumpZ;
	impactVel = 0.1 * impactVel * impactVel;
	BaseEyeHeight = Default.BaseEyeHeight;

	if ( Role == ROLE_Authority )
	{
		if ( impactVel > 0.17 )
			PlaySound(LandGrunt, SLOT_Talk, FMin(5, 5 * impactVel),false,1200,FRand()*0.4+0.8);
		if ( !FootRegion.Zone.bWaterZone && (impactVel > 0.01) )
			PlaySound(Land, SLOT_Interact, FClamp(4 * impactVel,0.5,5), false,1000, 1.0);
	}
	if ( (impactVel > 0.06) || (GetAnimGroup(AnimSequence) == 'Jumping') )
	{
		if ( (Weapon == None) || (Weapon.Mass < 20) )
			TweenAnim('LandSMFR', 0.12);
		else
			TweenAnim('LandLGFR', 0.12);
	}
	else if ( !IsAnimating() )
	{
		if ( GetAnimGroup(AnimSequence) == 'TakeHit' )
		{
			SetPhysics(PHYS_Walking);
			AnimEnd();
		}
		else 
		{
			if ( (Weapon == None) || (Weapon.Mass < 20) )
				TweenAnim('LandSMFR', 0.12);
			else
				TweenAnim('LandLGFR', 0.12);
		}
	}
}
	
function PlayInAir()
{
	BaseEyeHeight =  0.7 * Default.BaseEyeHeight;
	if ( (Weapon == None) || (Weapon.Mass < 20) )
		TweenAnim('JumpSMFR', 0.8);
	else
		TweenAnim('JumpLGFR', 0.8); 
}

function PlayDuck()
{
	BaseEyeHeight = 0;
	if ( (Weapon == None) || (Weapon.Mass < 20) )
		TweenAnim('DuckWlkS', 0.25);
	else
		TweenAnim('DuckWlkL', 0.25);
}

function PlayCrawling()
{
	//log("Play duck");
	BaseEyeHeight = 0;
	if ( (Weapon == None) || (Weapon.Mass < 20) )
		LoopAnim('DuckWlkS');
	else
		LoopAnim('DuckWlkL');
}

function TweenToWaiting(float tweentime)
{
	if ( (IsInState('PlayerSwimming')) || (Physics == PHYS_Swimming) )
	{
		BaseEyeHeight = 0.7 * Default.BaseEyeHeight;
		if ( (Weapon == None) || (Weapon.Mass < 20) )
			TweenAnim('TreadSM', tweentime);
		else
			TweenAnim('TreadLG', tweentime);
	}
	else
	{
		BaseEyeHeight = Default.BaseEyeHeight;
		if ( (Weapon == None) || (Weapon.Mass < 20) )
			TweenAnim('StillSMFR', tweentime);
		else
			TweenAnim('StillFRRP', tweentime);
	}
}
	
function PlayWaiting()
{
	local name newAnim;

	if ( (IsInState('PlayerSwimming')) || (Physics == PHYS_Swimming) )
	{
		BaseEyeHeight = 0.7 * Default.BaseEyeHeight;
		if ( (Weapon == None) || (Weapon.Mass < 20) )
			LoopAnim('TreadSM');
		else
			LoopAnim('TreadLG');
	}
	else
	{	
		BaseEyeHeight = Default.BaseEyeHeight;
		ViewRotation.Pitch = ViewRotation.Pitch & 65535;
		If ( (ViewRotation.Pitch > RotationRate.Pitch) 
			&& (ViewRotation.Pitch < 65536 - RotationRate.Pitch) )
		{
			If (ViewRotation.Pitch < 32768) 
			{
				if ( (Weapon == None) || (Weapon.Mass < 20) )
					TweenAnim('AimUpSm', 0.3);
				else
					TweenAnim('AimUpLg', 0.3);
			}
			else
			{
				if ( (Weapon == None) || (Weapon.Mass < 20) )
					TweenAnim('AimDnSm', 0.3);
				else
					TweenAnim('AimDnLg', 0.3);
			}
		}
		else if ( (Weapon != None) && Weapon.bPointing )
		{
			if ( Weapon.Mass < 20 )
				TweenAnim('StillSMFR', 0.3);
			else
				TweenAnim('StillFRRP', 0.3);
		}
		else
		{
			if ( FRand() < 0.1 )
			{
				if ( (Weapon == None) || (Weapon.Mass < 20) )
					PlayAnim('CockGun', 0.5 + 0.5 * FRand(), 0.3);
				else
					PlayAnim('CockGunL', 0.5 + 0.5 * FRand(), 0.3);
			}
			else
			{
				if ( (Weapon == None) || (Weapon.Mass < 20) )
				{
					if ( Health > 50 )
						newAnim = 'Breath1';
					else
						newAnim = 'Breath2';
				}
				else
				{
					if ( Health > 50 )
						newAnim = 'Breath1L';
					else
						newAnim = 'Breath2L';
				}
								
				if ( AnimSequence == newAnim )
					LoopAnim(newAnim, 0.3 + 0.7 * FRand());
				else
					PlayAnim(newAnim, 0.3 + 0.7 * FRand(), 0.25);
			}
		}
	}
}	

function PlayFiring()
{
	// switch animation sequence mid-stream if needed
	if (AnimSequence == 'RunLG')
		AnimSequence = 'RunLGFR';
	else if (AnimSequence == 'RunSM')
		AnimSequence = 'RunSMFR';
	else if (AnimSequence == 'WalkLG')
		AnimSequence = 'WalkLGFR';
	else if (AnimSequence == 'WalkSM')
		AnimSequence = 'WalkSMFR';
	else if ( AnimSequence == 'JumpSMFR' )
		TweenAnim('JumpSMFR', 0.03);
	else if ( AnimSequence == 'JumpLGFR' )
		TweenAnim('JumpLGFR', 0.03);
	else if ( (GetAnimGroup(AnimSequence) == 'Waiting') || (GetAnimGroup(AnimSequence) == 'Gesture')
		&& (AnimSequence != 'TreadLG') && (AnimSequence != 'TreadSM') )
	{
		if ( Weapon.Mass < 20 )
			TweenAnim('StillSMFR', 0.02);
		else
			TweenAnim('StillFRRP', 0.02);
	}
}

function PlayWeaponSwitch(Weapon NewWeapon)
{
	if ( (Weapon == None) || (Weapon.Mass < 20) )
	{
		if ( (NewWeapon != None) && (NewWeapon.Mass > 20) )
		{
			if ( (AnimSequence == 'RunSM') || (AnimSequence == 'RunSMFR') )
				AnimSequence = 'RunLG';
			else if ( (AnimSequence == 'WalkSM') || (AnimSequence == 'WalkSMFR') )
				AnimSequence = 'WalkLG';	
		 	else if ( AnimSequence == 'JumpSMFR' )
		 		AnimSequence = 'JumpLGFR';
			else if ( AnimSequence == 'DuckWlkL' )
				AnimSequence = 'DuckWlkS';
		 	else if ( AnimSequence == 'StillSMFR' )
		 		AnimSequence = 'StillFRRP';
			else if ( AnimSequence == 'AimDnSm' )
				AnimSequence = 'AimDnLg';
			else if ( AnimSequence == 'AimUpSm' )
				AnimSequence = 'AimUpLg';
		 }	
	}
	else if ( (NewWeapon == None) || (NewWeapon.Mass < 20) )
	{		
		if ( (AnimSequence == 'RunLG') || (AnimSequence == 'RunLGFR') )
			AnimSequence = 'RunSM';
		else if ( (AnimSequence == 'WalkLG') || (AnimSequence == 'WalkLGFR') )
			AnimSequence = 'WalkSM';
	 	else if ( AnimSequence == 'JumpLGFR' )
	 		AnimSequence = 'JumpSMFR';
		else if ( AnimSequence == 'DuckWlkS' )
			AnimSequence = 'DuckWlkL';
	 	else if (AnimSequence == 'StillFRRP')
	 		AnimSequence = 'StillSMFR';
		else if ( AnimSequence == 'AimDnLg' )
			AnimSequence = 'AimDnSm';
		else if ( AnimSequence == 'AimUpLg' )
			AnimSequence = 'AimUpSm';
	}
}

function PlaySwimming()
{
	BaseEyeHeight = 0.7 * Default.BaseEyeHeight;
	if ((Weapon == None) || (Weapon.Mass < 20) )
		LoopAnim('SwimSM');
	else
		LoopAnim('SwimLG');
}

function TweenToSwimming(float tweentime)
{
	BaseEyeHeight = 0.7 * Default.BaseEyeHeight;
	if ((Weapon == None) || (Weapon.Mass < 20) )
		TweenAnim('SwimSM',tweentime);
	else
		TweenAnim('SwimLG',tweentime);
}

defaultproperties
{
     Footstep1=stwalk1
     Footstep2=stwalk2
     Footstep3=stwalk3
     BaseEyeHeight=+00023.000000
     CollisionRadius=+00017.000000
     CollisionHeight=+00039.000000
     Buoyancy=+00099.000000
}

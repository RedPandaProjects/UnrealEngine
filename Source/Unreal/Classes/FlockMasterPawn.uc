//=============================================================================
// FlockMasterPawn.
//=============================================================================
class FlockMasterPawn expands Pawn;

//==============
// Encroachment
function bool EncroachingOn( actor Other )
{
	if ( (Other.Brush != None) || (Brush(Other) != None) )
		return true;
		
	return false;
}

event FootZoneChange(ZoneInfo newFootZone)
{
}

function EncroachedBy( actor Other )
{
}

function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
{
}

function BaseChange()
{
}

defaultproperties
{
     Mass=+00005.000000
     Buoyancy=+00005.000000
     bCollideActors=False
     bBlockActors=False
     bBlockPlayers=False
     bProjTarget=False
	 bForceStasis=True
}

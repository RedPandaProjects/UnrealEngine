//=============================================================================
// HumanCarcass.
//=============================================================================
class HumanCarcass expands CreatureCarcass
	abstract;

function SpawnHead()
{
	local carcass carc;

	carc = Spawn(class'FemaleHead');
	if ( carc != None )
		carc.Initfor(self);
}

function CreateReplacement()
{
	if ( Level.NetMode == NM_StandAlone )
		Super.CreateReplacement();
}

function Initfor(actor Other)
{
	bReducedHeight = false;
	PrePivot = vect(0,0,0);	
	Super.InitFor(Other);
}

function ReduceCylinder()
{
	Super.ReduceCylinder();
	PrePivot = PrePivot - vect(0,0,2);
}

defaultproperties
{
	  bReducedHeight=true	
      PrePivot=(X=0.000000,Y=0.000000,Z=26.000000)
      CollisionHeight=+00013.000000
	  CollisionRadius=+00027.000000
	  bBlockActors=false
	  bBlockPlayers=false
      flies=0
}

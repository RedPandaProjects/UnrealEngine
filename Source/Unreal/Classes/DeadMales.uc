//=============================================================================
// DeadMales.
//=============================================================================
class DeadMales expands HumanCarcass
	abstract;


function SpawnHead()
{
	local carcass carc;

	carc = Spawn(class'MaleHead');
	if ( carc != None )
		carc.Initfor(self);
}

function ClientExtraChunks()
{
	local carcass carc;

	if ( (AnimSequence != 'Dead4') && (AnimSequence != 'Dead5') )
		SpawnHead();

	if ( Level.NetMode == NM_Client )
	{
		carc = Spawn(class 'CreatureChunks');
		if (carc != None)
		{
			carc.Mesh = mesh 'CowBody1';
			carc.Initfor(self);
		}
	}

	if ( AnimSequence != 'Dead5' )
	{
		if ( Level.bHighDetailMode )
		{
			if ( FRand() < 0.3 )
			{
				carc = Spawn(class 'Liver');
				if (carc != None)
					carc.Initfor(self);
			}
			else if ( FRand() < 0.5 )
			{
				carc = Spawn(class 'Stomach');
				if (carc != None)
					carc.Initfor(self);
			}
			else
			{
				carc = Spawn(class 'PHeart');
				if (carc != None)
					carc.Initfor(self);
			}
			if ( FRand() < 0.5 )
			{
				carc = Spawn(class 'Leg1');
				if (carc != None)
					carc.Initfor(self);
			}
			carc = Spawn(class 'Thigh');
			if (carc != None)
				carc.Initfor(self);
		}
		carc = Spawn(class 'CreatureChunks');
		if (carc != None)
		{
			carc.Mesh = mesh 'CowBody1';
			carc.Initfor(self);
		}
		carc = Spawn(class 'Arm1');
		if (carc != None)
		{
			carc.Initfor(self);
			carc.fatness = 140;
		}
	}
	carc = Spawn(class 'Leg1');
	if (carc != None)
	{
		carc.Initfor(self);
		carc.fatness = 140;
	}

	carc = Spawn(class 'Thigh');
	if (carc != None)
		carc.Initfor(self);
	Spawn(class 'Bloodspurt');
}

defaultproperties
{
	Physics=PHYS_None
}
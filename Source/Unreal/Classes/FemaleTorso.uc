//=============================================================================
// FemaleTorso.
//=============================================================================
class FemaleTorso expands HumanCarcass;

#exec MESH IMPORT MESH=Body1 ANIVFILE=MODELS\g_Bod1_a.3D DATAFILE=MODELS\g_Bod1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Body1 X=0 Y=0 Z=-160 YAW=-64
#exec MESH SEQUENCE MESH=Body1 SEQ=All    STARTFRAME=0   NUMFRAMES=1
#exec MESH SEQUENCE MESH=Body1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JBody11  FILE=MODELS\g_body1.PCX GROUP=Skins FLAGS=2
#exec MESHMAP SCALE MESHMAP=Body1 X=0.06 Y=0.06 Z=0.12
#exec MESHMAP SETTEXTURE MESHMAP=Body1 NUM=1 TEXTURE=JBody11

function Initfor(actor Other)
{
	Velocity = other.Velocity;
	Mass = 0.7 * Other.Mass;
	if (FRand() < 0.5)
		Buoyancy = 1.06 * Mass; // float corpse
	else
		Buoyancy = 0.9 * Mass;
}

function ClientExtraChunks()
{
	local carcass carc;
	
	// arm, leg and thigh
	carc = Spawn(class 'Liver');
	if (carc != None)
		carc.Initfor(self);
	carc = Spawn(class 'Thigh');
	if (carc != None)
		carc.Initfor(self);
	carc = Spawn(class 'Liver');
	if (carc != None)
		carc.Initfor(self);
	carc = Spawn(class 'Stomach');
	if (carc != None)
		carc.Initfor(self);
	carc = Spawn(class 'PHeart');
	if (carc != None)
		carc.Initfor(self);
	carc = Spawn(class 'CreatureChunks');
	if (carc != None)
	{
		carc.Mesh = mesh 'CowBody1';
		carc.Initfor(self);
	}
}
function CreateReplacement()
{
	local carcass carc;

	if (bHidden)
		return;	
	carc = Spawn(class 'PHeart');
	if (carc != None)
	{
		carc.Bugs = Bugs;
		Bugs = None;
		carc.Initfor(self);
		carc.Velocity = velocity; //no rand
	}
}

function ReduceCylinder()
{
	bReducedHeight = true;
}

defaultproperties
{
     flies=4
     bReducedHeight=True
     Mesh=Unreal.Body1
     CollisionRadius=+00027.000000
     CollisionHeight=+00006.000000
}

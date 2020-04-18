//=============================================================================
// MaleOneBot.
//=============================================================================
class MaleOneBot expands MaleBot;

var texture TimSkins[8];

#exec AUDIO IMPORT FILE="Sounds\male\metal01.WAV" NAME="metwalk1" GROUP="Male"
#exec AUDIO IMPORT FILE="Sounds\male\metal02.WAV" NAME="metwalk2" GROUP="Male"
#exec AUDIO IMPORT FILE="Sounds\male\metal03.WAV" NAME="metwalk3" GROUP="Male"

function ForceMeshToExist()
{
	Spawn(class'MaleOne');
}

function BeginPlay()
{
	Super.BeginPlay();
	Skin=TimSkins[rand(ArrayCount(TimSkins))];
}

simulated function PlayMetalStep()
{
	local sound step;
	local float decision;

	if ( Role < ROLE_Authority )
		return;
	if ( !bIsWalking && (Level.Game.Difficulty > 1) && ((Weapon == None) || !Weapon.bPointing) )
		MakeNoise(0.05 * Level.Game.Difficulty);
	if ( FootRegion.Zone.bWaterZone )
	{
		PlaySound(sound 'LSplash', SLOT_Interact, 0.5, false, 1500.0, 1.0);
		return;
	}

	decision = FRand();
	if ( decision < 0.34 )
		step = sound'MetWalk1';
	else if (decision < 0.67 )
		step = sound'MetWalk2';
	else
		step = sound'MetWalk3';

	if ( bIsWalking )
		PlaySound(step, SLOT_Interact, 0.5, false, 400.0, 1.0);
	else 
		PlaySound(step, SLOT_Interact, 1, false, 1000.0, 1.0);
}

defaultproperties
{
     TimSkins(0)=JMale1
     TimSkins(1)=JMale2
     TimSkins(2)=JMale3
     TimSkins(3)=JMale4
     TimSkins(4)=JMale5
     TimSkins(5)=JMale6
     TimSkins(6)=JMale7
     TimSkins(7)=JMale8
     Mesh=Male1
	 CarcassType=MaleOneCarcass
}

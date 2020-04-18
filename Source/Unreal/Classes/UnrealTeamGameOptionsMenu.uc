//=============================================================================
// UnrealTeamGameOptionsMenu
//=============================================================================
class UnrealTeamGameOptionsMenu expands UnrealDMGameOptionsMenu
	localized;


function bool ProcessLeft()
{
	if ( Selection == 10 )
		TeamGame(GameType).FriendlyFireScale = FMax(0, TeamGame(GameType).FriendlyFireScale - 0.1);
	else 
		return Super.ProcessLeft();

	return true;
}

function bool ProcessRight()
{
	if ( Selection == 10 )
		TeamGame(GameType).FriendlyFireScale = FMin(1, TeamGame(GameType).FriendlyFireScale + 0.1);
	else 
		return Super.ProcessRight();

	return true;
}

function DrawOptions(canvas Canvas, int StartX, int StartY, int Spacing)
{
	MenuList[10] = Default.MenuList[10];

	Super.DrawOptions(Canvas, StartX, StartY, Spacing);
}

function DrawValues(canvas Canvas, int StartX, int StartY, int Spacing)
{
	MenuList[10] = string(TeamGame(GameType).FriendlyFireScale);

	Super.DrawValues(Canvas, StartX, StartY, Spacing);
}

defaultproperties
{
	MenuLength=10
	MenuList(10)="Friendly Fire Scale"
	HelpMessage(10)="Percentage of damage taken when hit by friendly fire.  Note that you always take full damage from your own weapons."
    GameClass=TeamGame
}
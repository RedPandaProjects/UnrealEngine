//=============================================================================
// UnrealDMGameOptionsMenu
//=============================================================================
class UnrealDMGameOptionsMenu expands UnrealGameOptionsMenu
	localized;

function bool ProcessYes()
{
	if ( Selection == 6 )
		DeathMatchGame(GameType).bCoopWeaponMode = True;		
	else if ( Selection == 9 )
		DeathMatchGame(GameType).bMultiPlayerBots = True;		
	else 
		return Super.ProcessYes();

	return true;
}

function bool ProcessNo()
{
	if ( Selection == 6 )
		DeathMatchGame(GameType).bCoopWeaponMode = False;		
	else if ( Selection == 9 )
		DeathMatchGame(GameType).bMultiPlayerBots = false;		
	else 
		return Super.ProcessNo();

	return true;
}

function bool ProcessLeft()
{
	if ( Selection == 3 )
		DeathMatchGame(GameType).FragLimit = FMax(0, DeathMatchGame(GameType).FragLimit - 5);
	else if ( Selection == 4 )
		DeathMatchGame(GameType).TimeLimit = FMax(0, DeathMatchGame(GameType).TimeLimit - 5);
	else if ( Selection == 5 )
		DeathMatchGame(GameType).MaxPlayers = Max(1, DeathMatchGame(GameType).MaxPlayers - 1);
	else if ( Selection == 6 )
		DeathMatchGame(GameType).bCoopWeaponMode = !DeathMatchGame(GameType).bCoopWeaponMode;		
	else if ( Selection == 7 )
		DeathMatchGame(GameType).InitialBots = Max(0, DeathMatchGame(GameType).InitialBots - 1);
	else if ( Selection == 9 )
		DeathMatchGame(GameType).bMultiPlayerBots = !DeathMatchGame(GameType).bMultiPlayerBots;		
	else 
		return Super.ProcessLeft();

	return true;
}

function bool ProcessRight()
{
	if ( Selection == 3 )
		DeathMatchGame(GameType).FragLimit += 5;
	else if ( Selection == 4 )
		DeathMatchGame(GameType).TimeLimit += 5;
	else if ( Selection == 5 )
		DeathMatchGame(GameType).MaxPlayers = Min(16, DeathMatchGame(GameType).MaxPlayers + 1);
	else if ( Selection == 6 )
		DeathMatchGame(GameType).bCoopWeaponMode = !DeathMatchGame(GameType).bCoopWeaponMode;		
	else if ( Selection == 7 )
		DeathMatchGame(GameType).InitialBots = Min(15, DeathMatchGame(GameType).InitialBots + 1);
	else if ( Selection == 9 )
		DeathMatchGame(GameType).bMultiPlayerBots = !DeathMatchGame(GameType).bMultiPlayerBots;		
	else 
		return Super.ProcessRight();

	return true;
}

function bool ProcessSelection()
{
	local Menu ChildMenu;

	if ( Selection == 6 )
		DeathMatchGame(GameType).bCoopWeaponMode = !DeathMatchGame(GameType).bCoopWeaponMode;		
	else if ( Selection == 8 )
		ChildMenu = spawn(class'UnrealBotConfigMenu', owner);
	else if ( Selection == 9 )
		DeathMatchGame(GameType).bMultiPlayerBots = !DeathMatchGame(GameType).bMultiPlayerBots;		
	else
		return Super.ProcessSelection();

	if ( ChildMenu != None )
	{
		HUD(Owner).MainMenu = ChildMenu;
		ChildMenu.ParentMenu = self;
		ChildMenu.PlayerOwner = PlayerOwner;
	}
	return true;
}

function DrawOptions(canvas Canvas, int StartX, int StartY, int Spacing)
{
	local int i;

	for ( i=3; i<MenuLength+1; i++ )
		MenuList[i] = Default.MenuList[i];

	Super.DrawOptions(Canvas, StartX, StartY, Spacing);
}

function DrawValues(canvas Canvas, int StartX, int StartY, int Spacing)
{
	local DeathMatchGame DMGame;

	DMGame = DeathMatchGame(GameType);

	// draw text
	MenuList[3] = string(DMGame.FragLimit);
	MenuList[4] = string(DMGame.TimeLimit);
	MenuList[5] = string(DMGame.MaxPlayers);
	MenuList[6] = string(DMGame.bCoopWeaponMode);
	MenuList[7] = string(DMGame.InitialBots);
	MenuList[8] = "";
	MenuList[9] = string(DMGame.bMultiPlayerBots);
	Super.DrawValues(Canvas, StartX, StartY, Spacing);
}

defaultproperties
{
	MenuLength=9
	MenuList(3)="Frag limit"
	MenuList(4)="Time Limit"
	MenuList(5)="Max Players"
	MenuList(6)="Coop Weapon Mode"
	MenuList(7)="Number of bots"
	MenuList(8)="Configure Bots"
	menuList(9)="Bots in Multiplayer"
	HelpMessage(3)="Number of frags scored by leading player to end game.  If 0, there is no limit."
	HelpMessage(4)="Time limit (in minutes) to end game.  If 0, there is no limit."
	HelpMessage(5)="Maximum number of players allowed in the game."
	HelpMessage(6)="If Coop Weapon Mode is enabled, weapons respawn instantly, but can only be picked up once by a given player."
	HelpMessage(7)="Number of bots to start play (max 15)."
	HelpMessage(8)="Configure bot game and individual parameters."
	HelpMessage(9)="Use bots when playing with other people."
    GameClass=DeathMatchGame
}
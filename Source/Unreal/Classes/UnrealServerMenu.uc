//=============================================================================
// UnrealServerMenu
//=============================================================================
class UnrealServerMenu expands UnrealMenu
	config
	localized;

var config string[64] Map;
var config string[32] GameType;
var config string[32] Games[16];
var config int MaxGames;
var int CurrentGame;
var class<GameInfo> GameClass;
var bool bStandalone;
var localized string[32] BotTitle;

function PostBeginPlay()
{
	Super.PostBeginPlay();

	if ( class'GameInfo'.Default.bShareware )
		MaxGames = 2;

	CurrentGame = 0;
	SetGameClass();
}

function SetGameClass()
{
	GameType = Games[CurrentGame];
	GameClass = class<gameinfo>(DynamicLoadObject(GameType, class'Class'));
	Map = GetMapName(GameClass.Default.MapPrefix, Map,0);
}

function bool ProcessLeft()
{
	if ( Selection == 1 )
	{
		CurrentGame--;
		if ( CurrentGame < 0 )
			CurrentGame = MaxGames;
		SetGameClass();
		if ( GameClass == None )
		{
			MaxGames--;
			if ( MaxGames > 0 )
				ProcessLeft();
		}
	}
	else if ( Selection == 2 )
		Map = GetMapName(GameClass.Default.MapPrefix, Map, -1);
	else 
		return false;

	return true;
}

function bool ProcessRight()
{
	if ( Selection == 1 )
	{
		CurrentGame++;
		if ( CurrentGame > MaxGames )
			CurrentGame = 0;
		SetGameClass();
		if ( GameClass == None )
		{
			MaxGames--;
			if ( MaxGames > 0 )
				ProcessRight();
		}

	}
	else if ( Selection == 2 )
		Map = GetMapName(GameClass.Default.MapPrefix, Map, 1);
	else
		return false;

	return true;
}

function bool ProcessSelection()
{
	local Menu ChildMenu;
	local string[240] URL;
	if( Selection == 3 )
	{
		ChildMenu = spawn( GameClass.Default.GameMenuType, owner );
		HUD(Owner).MainMenu = ChildMenu;
		ChildMenu.ParentMenu = self;
		ChildMenu.PlayerOwner = PlayerOwner;
		return true;
	}
	else if ( Selection == 4 )
	{
		GameClass = class<gameinfo>(DynamicLoadObject(GameType, class'Class'));
		URL = Map $ "?Game=" $ GameType;
		if( !bStandAlone && Level.Netmode!=NM_ListenServer )
			URL = URL $ "?Listen";
		SaveConfigs();
		ChildMenu = spawn(class'UnrealMeshMenu', owner);
		if ( ChildMenu != None )
		{
			UnrealMeshMenu(ChildMenu).StartMap = URL;
			HUD(Owner).MainMenu = ChildMenu;
			ChildMenu.ParentMenu = self;
			ChildMenu.PlayerOwner = PlayerOwner;
		}
		log( "URL: '" $ URL $ "'" );
		return true;
	}
	else if ( Selection == 5 )
	{
		GameClass = class<gameinfo>(DynamicLoadObject(GameType, class'Class'));
		URL = Map $ "?Game=" $ GameType;
		SaveConfigs();
		PlayerOwner.ConsoleCommand("RELAUNCH "$URL$" -server");
		return true;
	}
	else return false;
}

function SaveConfigs()
{
	SaveConfig();
	PlayerOwner.SaveConfig();
}

function DrawMenu(canvas Canvas)
{
	local int i, StartX, StartY, Spacing;
	local string[32] MapName;

	DrawBackGround(Canvas, false);

	// Draw Title
	if ( bStandAlone )
	{
		MenuLength = 4;
		MenuTitle = BotTitle;
	}
	DrawTitle(Canvas);
		
	Spacing = Clamp(0.07 * Canvas.ClipY, 12, 48);
	StartX = Max(40, 0.5 * Canvas.ClipX - 120);
	StartY = Max(40, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));
	Canvas.Font = Canvas.MedFont;

	// draw text
	for( i=1; i<MenuLength+1; i++ )
		MenuList[i] = Default.MenuList[i];

	DrawList(Canvas, false, Spacing, StartX, StartY);  

	// draw values
	MenuList[1] = GameType;
	i = InStr(MenuList[1], ".");
	if( i != -1 )
		MenuList[1] =  Right(MenuList[1], Len(MenuList[1]) - i - 1);
	MapName = Left(Map, Len(Map) - 4 );	
	MenuList[2] = MapName;
	MenuList[3] = "";
	MenuList[4] = "";
	MenuList[5] = "";
	DrawList(Canvas, false, Spacing, StartX + 130, StartY);  

	// Draw help panel
	DrawHelpPanel(Canvas, StartY + MenuLength * Spacing + 8, 228);
}

defaultproperties
{
	 BotTitle="BOTMATCH"
	 MenuTitle="MULTIPLAYER"
	 MenuList(1)="Select Game"
	 MenuList(2)="Select Map"
	 MenuList(3)="Configure Game"
	 MenuList(4)="Start Game"
	 MenuList(5)="Launch Dedicated Server"
     Games(0)="Unreal.DeathMatchGame"
     Games(1)="Unreal.TeamGame"
     Games(2)="Unreal.CoopGame"
     Games(3)="Unreal.KingOfTheHill"
     Games(4)="Unreal.DarkMatch"
     MaxGames=4
     HelpMessage(1)="Choose Game Type."
     HelpMessage(2)="Choose Map."
     HelpMessage(3)="Modify Game Options."
     HelpMessage(4)="Start Game."
     HelpMessage(4)="Start a dedicated server on this machine."
     MenuLength=5
     Class=Unreal.UnrealServerMenu
}

//=============================================================================
// UnrealJoinGameMenu
//=============================================================================
class UnrealJoinGameMenu expands UnrealMenu
	localized
	config;

var string[128] LastServer;
var string[128] OldLastServer;
var localized string[128] InternetOption;
var localized string[128] FastInternetOption;
var localized string[128] LANOption;
var int netspeed;

function PostBeginPlay()
{
	Super.PostBeginPlay();
	OldLastServer = LastServer;
} 

function ProcessMenuInput( coerce string[64] InputString )
{
	local UnrealMeshMenu ChildMenu;

	if ( selection == 3 )
	{
		LastServer = InputString;
		SaveConfigs();
		ChildMenu = spawn(class'UnrealMeshMenu', owner);
		if ( ChildMenu != None )
		{
			ChildMenu.StartMap = LastServer;
			HUD(Owner).MainMenu = ChildMenu;
			ChildMenu.ParentMenu = self;
			ChildMenu.PlayerOwner = PlayerOwner;
		}
	}
}

function ProcessMenuEscape()
{
	if ( selection == 3 )
		LastServer = OldLastServer;
}

function ProcessMenuUpdate( coerce string[64] InputString )
{
	if ( selection == 3 )
		LastServer = (InputString$"_");
}

function bool ProcessLeft()
{
	if ( Selection == 4 )
	{
		netspeed--;
		if ( netspeed < 0 )
			netspeed = 2;
		PlayerOwner.ChangeNetSpeed(netspeed);
	}
	else
		ProcessRight();
}

function bool ProcessRight()
{
	if ( Selection == 3 )
	{
		LastServer = "_";
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else if ( Selection == 4 )
	{
		netspeed++;
		if ( netspeed > 2 )
			netspeed = 0;
		PlayerOwner.ChangeNetSpeed(netspeed);
	}
}

function bool ProcessSelection()
{
	local Menu ChildMenu;
	local class<Menu> ListenMenuClass;

	if ( Selection == 3 )
	{
		LastServer = "_";
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else if ( Selection == 1 )
	{
		if ( PlayerOwner.NetSpeed < 12500 )
			PlayerOwner.ChangeNetSpeed(2);
		SaveConfigs();
		ListenMenuClass = class<Menu>(DynamicLoadObject("Unreal.UnrealListenMenu", class'Class'));
		ChildMenu = spawn(ListenMenuClass, owner);
	}
	else if ( Selection == 2 )
	{
		SaveConfigs();
		ChildMenu = spawn(class'UnrealFavoritesMenu', owner);
	}
	else if ( Selection == 5 )
		PlayerOwner.ConsoleCommand("start http://www.unreal.com/serverlist");
	if ( ChildMenu != None )
	{
		HUD(Owner).MainMenu = ChildMenu;
		ChildMenu.ParentMenu = self;
		ChildMenu.PlayerOwner = PlayerOwner;
	}
	return true;
}

function SaveConfigs()
{
	PlayerOwner.SaveConfig();
	SaveConfig();
}

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing;
	
	DrawBackGround(Canvas, false);	
	DrawTitle(Canvas);

	Spacing = Clamp(0.06 * Canvas.ClipY, 11, 32);
	StartX = Max(12, 0.5 * Canvas.ClipX - 120);
	StartY = Max(32, 0.5 * (Canvas.ClipY - 3 * Spacing - 128));

	DrawList(Canvas, false, Spacing, StartX, StartY); 

	Canvas.Font = Canvas.MedFont;
	SetFontBrightness( Canvas, (Selection == 3) );
	Canvas.SetPos(StartX + 100, StartY + 2 * Spacing );	
	Canvas.DrawText(LastServer, false);
	Canvas.DrawColor = Canvas.Default.DrawColor;

	SetFontBrightness( Canvas, (Selection == 4) );
	Canvas.SetPos(StartX + 100, StartY + 3 * Spacing );	
	if ( PlayerOwner.NetSpeed <= 3000 )
	{
		netspeed = 0;
		Canvas.DrawText(InternetOption, false);
	}
	else if ( PlayerOwner.NetSpeed < 12500 )
	{
		netspeed = 1;
		Canvas.DrawText(FastInternetOption, false);
	}
	else
	{
		netspeed = 2;
		Canvas.DrawText(LANOption, false);
	}
	Canvas.DrawColor = Canvas.Default.DrawColor;

	DrawHelpPanel(Canvas, StartY + MenuLength * Spacing + 8, 228);
}

defaultproperties
{
	 MenuTitle="JOIN GAME"
	 MenuLength=5
	 InternetOption="Internet (28.8)"
	 FastInternetOption="Fast Internet (56K)"
	 LANOption="LAN"
	 MenuList(1)="Find Local Servers"
	 MenuList(2)="Choose From Favorites"
	 MenuList(3)="Open"
	 MenuList(4)="Optimized for"
	 MenuList(5)="Go to the Epic Unreal server list"
     HelpMessage(1)="Listen for local servers"
     HelpMessage(2)="Choose a server from a list of favorites"
     HelpMessage(3)="Hit enter to type in a server address.  Hit enter again to go to this server."
	 HelpMessage(4)="Set networking optimization to LAN, Fast Internet (56K modem or ISDN) or Internet (28.8 to 33.6 modem speed)"
	 HelpMessage(5)="Open the Epic Unreal server list WWW page"
}

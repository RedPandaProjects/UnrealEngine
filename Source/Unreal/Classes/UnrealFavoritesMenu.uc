//=============================================================================
// UnrealFavoritesMenu
//=============================================================================
class UnrealFavoritesMenu expands UnrealMenu
	config
	localized;

var config string[128] Favorites[12]; //Menu List has aliases
var config string[128] Aliases[12]; //Menu List has aliases
var localized string[32] EditList[2];
var bool	bEditMode;
var bool	bEditAlias;
var bool	bEditFavorite;
var string[128] OldFavorite;
var string[128] OldAlias;
var int EditSelection;

function SaveConfigs()
{
	SaveConfig();
}	

function ProcessMenuInput( coerce string[64] InputString )
{
	if ( bEditAlias )
	{
		Aliases[EditSelection] = InputString;
		bEditAlias = false;
		Favorites[EditSelection] = "_";
		bEditFavorite = true;
	}
	else
	{
		bEditFavorite = false;
		bEditMode = false;
		Selection = EditSelection + 1;
		Favorites[EditSelection] = InputString;
	}
}

function ProcessMenuEscape()
{
	if ( bEditAlias )
		Aliases[EditSelection] = OldAlias;
	else
		Favorites[EditSelection] = OldFavorite; 
}

function ProcessMenuUpdate( coerce string[64] InputString )
{
	if ( bEditAlias )
		Aliases[EditSelection] = (InputString$"_");
	else
		Favorites[EditSelection] = (InputString$"_");
}

function bool ProcessSelection()
{
	local Menu ChildMenu;

	if ( Aliases[Selection-1] != "....." )
	{
		SaveConfigs();
		ChildMenu = spawn(class'UnrealMeshMenu', owner);
		if ( ChildMenu != None )
		{
			UnrealMeshMenu(ChildMenu).StartMap = Favorites[Selection - 1];
			HUD(Owner).MainMenu = ChildMenu;
			ChildMenu.ParentMenu = self;
			ChildMenu.PlayerOwner = PlayerOwner;
		}
	}
	return true;
}

function bool ProcessLeft()
{
	bEditMode = true;
	bEditAlias = true;
	OldFavorite = Favorites[Selection-1];
	OldAlias = Aliases[Selection-1];
	Favorites[Selection-1] = "";
	Aliases[Selection-1] = "_";	
	EditSelection = Selection - 1;	
	PlayerOwner.Player.Console.GotoState('MenuTyping');
}

function bool ProcessRight()
{
	ProcessLeft();
}

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing, i;

	DrawBackGround(Canvas, (Canvas.ClipY < 250));

	// Draw Title
	StartX = Max(16, 0.5 * Canvas.ClipX - 80);
	Canvas.Font = Canvas.LargeFont;
	Canvas.SetPos(StartX, 4 );
	Canvas.DrawText(MenuTitle, False);

	if ( !bEditMode )
	{		
		// List Aliases
		Spacing = Clamp(Canvas.ClipY/20, 10, 32);
		StartX = Max(48, 0.5 * Canvas.ClipX - 64);
		StartY = Max(40, 0.5 * (Canvas.ClipY - MenuLength * Spacing));

		Canvas.Font = Canvas.MedFont;
		for ( i=0; i<12; i++ )
		{
			if ( Aliases[i] != "" )
			{
				SetFontBrightness( Canvas, (Selection == i + 1) );
				Canvas.SetPos(StartX, StartY + i * Spacing );
				Canvas.DrawText(Aliases[i], false);
			}
		}
		Canvas.DrawColor = Canvas.Default.DrawColor;

		DrawHelpPanel(Canvas, StartY + MenuLength * Spacing + 8, 228);
	}
	else
	{
		if ( bEditFavorite )
		{
			bEditFavorite = false;
			PlayerOwner.Player.Console.GotoState('MenuTyping');
		}

		Spacing = Clamp(Canvas.ClipY/20, 16, 32);
		StartX = Max(48, 0.5 * Canvas.ClipX - 120);
		StartY = Max(40, 0.5 * (Canvas.ClipY - 2 * Spacing));

		Canvas.Font = Canvas.MedFont;
		SetFontBrightness( Canvas, bEditAlias );
		Canvas.SetPos(StartX, StartY );
		Canvas.DrawText(EditList[0], false);
		Canvas.SetPos(StartX + 128, StartY );
		Canvas.DrawText(Aliases[EditSelection], false);

		SetFontBrightness( Canvas, !bEditAlias );
		Canvas.SetPos(StartX, StartY + Spacing );
		Canvas.DrawText(EditList[1], false);
		Canvas.SetPos(StartX + 104, StartY + Spacing );
		Canvas.DrawText(Favorites[EditSelection], false);
		Canvas.DrawColor = Canvas.Default.DrawColor;
	}
}

defaultproperties
{
	 MenuLength=12
	 MenuTitle="FAVORITES"
	 EditList(0)="Name for Server:"
	 EditList(1)="Address:"
	 HelpMessage(1)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 HelpMessage(2)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 HelpMessage(3)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 HelpMessage(4)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 HelpMessage(5)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 HelpMessage(6)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 HelpMessage(7)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 HelpMessage(8)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 HelpMessage(9)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 HelpMessage(10)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 HelpMessage(11)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 HelpMessage(12)="Hit enter to go to this server.  Hit the right arrow key to edit this entry."
	 Aliases(0)="MPlayer.com"
	 Aliases(1)="Heat.net"
	 Aliases(2)="World Opponent Network"
	 Aliases(3)="GameSpy"
	 Aliases(4)="AT&T Worldnet"
	 Aliases(5)="VRGN Game Network"
	 Aliases(6)="Now OnLine"
	 Aliases(7)="The Unreal Org"
	 Aliases(8)="Epic MegaGames"
	 Aliases(9)="..Empty.."
	 Aliases(10)="..Empty.."
	 Aliases(11)="..Empty.."
	 Favorites(0)="unreal://unreal.mplayer.com"
	 Favorites(1)="unreal://unreal.heat.net"
	 Favorites(2)="unreal://unreal.won.net"
	 Favorites(3)="unreal://unreal.gamespy.com"
	 Favorites(4)="unreal://unreal.gamehub.net"
	 Favorites(5)="unreal://unreal.vrgn.com"
	 Favorites(6)="unreal://nali.unrealserver.net"
	 Favorites(7)="unreal://krall.unreal.org"
	 Favorites(8)="unreal://server.unreal.com"
}

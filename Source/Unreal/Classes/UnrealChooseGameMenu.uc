//=============================================================================
// UnrealChooseGameMenu
//=============================================================================
class UnrealChooseGameMenu expands UnrealMenu
	localized;

var() config string[64] StartMaps[20];
var() config string[64] GameNames[20];

function bool ProcessSelection()
{
	local Menu ChildMenu;

	ChildMenu = spawn(class'UnrealNewGameMenu', owner);
	HUD(Owner).MainMenu = ChildMenu;
	ChildMenu.PlayerOwner = PlayerOwner;
	PlayerOwner.UpdateURL("Game=");
	UnrealNewGameMenu(ChildMenu).StartMap = StartMaps[Selection];

	if ( MenuLength == 1 )
	{
		ChildMenu.ParentMenu = ParentMenu;
		Destroy();
	}
	else
		ChildMenu.ParentMenu = self;
}

function DrawMenu(canvas Canvas)
{
	local int i, StartX, StartY, Spacing;

	if ( MenuLength == 1 )
	{
		DrawBackGround(Canvas, false);
		Selection = 1;
		ProcessSelection();
		return;
	}

	DrawBackGround(Canvas, false);
	DrawTitle(Canvas);

	Canvas.Style = 3;
	Spacing = Clamp(0.04 * Canvas.ClipY, 11, 32);
	StartX = Max(40, 0.5 * Canvas.ClipX - 120);
	StartY = Max(36, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));

	// draw text
	for ( i=0; i<20; i++ )
		MenuList[i] = GameNames[i];
	DrawList(Canvas, false, Spacing, StartX, StartY); 

	// Draw help panel
	DrawHelpPanel(Canvas, StartY + MenuLength * Spacing + 8, 228);
}

defaultproperties
{
	MenuTitle="CHOOSE GAME"
	MenuLength=1
	GameNames(1)="Unreal"
	StartMaps(1)="..\\maps\\Vortex2.unr"
	HelpMessage(1)="Choose which game to play."
	HelpMessage(2)="Choose which game to play."
	HelpMessage(3)="Choose which game to play."
	HelpMessage(4)="Choose which game to play."
	HelpMessage(5)="Choose which game to play."
	HelpMessage(6)="Choose which game to play."
	HelpMessage(7)="Choose which game to play."
	HelpMessage(8)="Choose which game to play."
	HelpMessage(9)="Choose which game to play."
	HelpMessage(10)="Choose which game to play."
	HelpMessage(11)="Choose which game to play."
	HelpMessage(12)="Choose which game to play."
	HelpMessage(13)="Choose which game to play."
	HelpMessage(14)="Choose which game to play."
	HelpMessage(15)="Choose which game to play."
	HelpMessage(16)="Choose which game to play."
	HelpMessage(17)="Choose which game to play."
	HelpMessage(18)="Choose which game to play."
	HelpMessage(19)="Choose which game to play."
}
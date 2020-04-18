//=============================================================================
// Menu: An in-game menu.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Menu expands Actor
	localized
	intrinsic;

var Menu	ParentMenu;
var int		Selection;
var() int	MenuLength;
var bool	bConfigChanged;
var bool    bExitAllMenus;
var PlayerPawn PlayerOwner;
var() localized string[255] HelpMessage[24];
var() localized string[128] MenuList[24];
var() localized string[32] LeftString;
var() localized string[32] RightString;
var() localized string[32] CenterString;
var() localized string[32] EnabledString;
var() localized string[32] DisabledString;
var() localized string[32] MenuTitle;

function bool ProcessSelection();
function bool ProcessLeft();
function bool ProcessRight();
function bool ProcessYes();
function bool ProcessNo();
function SaveConfigs();
function PlaySelectSound();
function PlayModifySound();
function PlayEnterSound();
function ProcessMenuInput( coerce string[64] InputString );
function ProcessMenuUpdate( coerce string[64] InputString );
function ProcessMenuEscape();
function ProcessMenuKey( int KeyNo, string[32] KeyName );
function MenuTick( float DeltaTime );

function ExitAllMenus()
{
	while ( Hud(Owner).MainMenu != None )
		Hud(Owner).MainMenu.ExitMenu();
}

function Menu ExitMenu()
{
	Hud(Owner).MainMenu = ParentMenu;
	if ( bConfigChanged )
		SaveConfigs();
	if ( ParentMenu == None )
	{
		PlayerOwner.bShowMenu = false;
		PlayerOwner.Player.Console.GotoState('');
		if( Level.Netmode == NM_Standalone )
			PlayerOwner.SetPause(False);
	}

	Destroy();
}

function SetFontBrightness(canvas Canvas, bool bBright)
{
	if ( bBright )
	{
		Canvas.DrawColor.R = 255;
		Canvas.DrawColor.G = 255;
		Canvas.DrawColor.B = 255;
	}
	else 
		Canvas.DrawColor = Canvas.Default.DrawColor;
}

function DrawList(canvas Canvas, bool bLargeFont, int Spacing, int StartX, int StartY)
{
	local int i;

	if ( bLargeFont )
	{
		if ( Spacing < 30 )
		{
			StartX += 0.5 * ( 0.5 * Canvas.ClipX - StartX);
			Canvas.Font = Canvas.BigFont;
		}
		else
			Canvas.Font = Canvas.LargeFont;
	}
	else
		Canvas.Font = Canvas.MedFont;

	for (i=0; i< (MenuLength); i++ )
	{
		SetFontBrightness(Canvas, (i == Selection - 1) );
		Canvas.SetPos(StartX, StartY + Spacing * i);
		Canvas.DrawText(MenuList[i + 1], false);
	}
	Canvas.DrawColor = Canvas.Default.DrawColor;
}

function DrawHelpPanel(canvas Canvas, int StartY, int XClip)
{
	local int StartX;

	if ( Canvas.ClipY < 92 + StartY )
		return;
	else
	{
		StartX = 0.5 * Canvas.ClipX - 128;
		StartY = Canvas.ClipY - 92;
	}

	Canvas.bCenter = false;
	Canvas.Font = Canvas.MedFont;
	Canvas.SetOrigin(StartX + 18, StartY);
	Canvas.SetClip(XClip,128);
	Canvas.SetPos(0,0);
	Canvas.Style = 1;
	SetFontBrightness(Canvas, true);	
	if ( Selection < 20 )
		Canvas.DrawText(HelpMessage[Selection], False);	
	SetFontBrightness(Canvas, false);
}

function MenuProcessInput( byte KeyNum, byte ActionNum )
{
	if ( KeyNum == EInputKey.IK_Escape )
	{
		PlayEnterSound();
		ExitMenu();
		return;
	}	

	if ( KeyNum == EInputKey.IK_Up )
	{
		PlaySelectSound();
		Selection--;
		if ( Selection < 1 )
			Selection = MenuLength;
	}
	else if ( KeyNum == EInputKey.IK_Down )
	{
		PlaySelectSound();
		Selection++;
		if ( Selection > MenuLength )
			Selection = 1;
	}
	else if ( KeyNum == EInputKey.IK_Enter )
	{
		bConfigChanged=true;
		if ( ProcessSelection() )
			PlayEnterSound();
	}
	else if ( KeyNum == EInputKey.IK_Left )
	{
		bConfigChanged=true;
		if ( ProcessLeft() )
			PlayModifySound();
	}
	else if ( KeyNum == EInputKey.IK_Right )
	{
		bConfigChanged=true;
		if ( ProcessRight() )
			PlayModifySound();
	}
	else if ( KeyNum == EInputKey.IK_Y )
	{
		bConfigChanged=true;
		if ( ProcessYes() )
			PlayModifySound();
	}
	else if ( KeyNum == EInputKey.IK_N )
	{
		bConfigChanged=true;
		if ( ProcessNo() )
			PlayModifySound();
	}

	if ( bExitAllMenus )
		ExitAllMenus(); 
	
}

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing;
	
	Spacing = Clamp(0.1 * Canvas.ClipY, 32, 48);
	StartX = Max(8, 0.5 * Canvas.ClipX - 160);
	StartY = Max(4, 0.5 * (Canvas.ClipY - 5 * Spacing - 128));
	Canvas.Font = Canvas.LargeFont;

	// draw text
	Canvas.SetPos(StartX, StartY );
	Canvas.DrawText("NOT YET IMPLEMENTED", False);

	// Draw help panel
	DrawHelpPanel(Canvas, StartY + 5 * Spacing, 228);
}

function DrawTitle(canvas Canvas)
{
	if ( Canvas.ClipY < 300 )
	{
		Canvas.Font = Canvas.BigFont;
		Canvas.SetPos(Max(8, 0.5 * Canvas.ClipX - 4 * Len(MenuTitle)), 4 );
	}
	else
	{
		Canvas.Font = Canvas.LargeFont;
		Canvas.SetPos(Max(8, 0.5 * Canvas.ClipX - 8 * Len(MenuTitle)), 4 );
	}
	Canvas.DrawText(MenuTitle, False);
}

defaultproperties
{
    HelpMessage(1)="This menu has not yet been implemented."
	bHidden=true
	Selection=1
	LeftString="Left"
	RightString="Right"
	CenterString="Center"
	EnabledString="Enabled"
	DisabledString="Disabled"
}

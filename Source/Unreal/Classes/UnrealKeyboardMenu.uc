//=============================================================================
// UnrealKeyboardMenu
//=============================================================================
class UnrealKeyboardMenu expands UnrealMenu
	localized;

var string[64] MenuValues1[24];
var string[64] MenuValues2[24];
var string[64] AliasNames[24];
var string[255] PendingCommands[30];
var int Pending;
var localized string[16] OrString;
var bool bSetUp;

function SaveConfigs()
{
	ProcessPending();
}

function ProcessPending()
{
	local int i;

	for ( i=0; i<Pending; i++ )
		PlayerOwner.ConsoleCommand(PendingCommands[i]);
		
	Pending = 0;
}

function AddPending(string[255] newCommand)
{
	PendingCommands[Pending] = newCommand;
	Pending++;
	if ( Pending == 30 )
		ProcessPending();
}
	
function SetUpMenu()
{
	local int i, j, pos;
	local string[32] KeyName;
	local string[32] Alias;

	bSetup = true;

	for ( i=0; i<255; i++ )
	{
		KeyName = PlayerOwner.ConsoleCommandResult ( "KEYNAME "$i );
		if ( KeyName != "" )
		{	
			Alias = PlayerOwner.ConsoleCommandResult( "KEYBINDING "$KeyName );
			if ( Alias != "" )
			{
				pos = InStr(Alias, " " );
				if ( pos != -1 )
					Alias = Left(Alias, pos);
				for ( j=1; j<20; j++ )
				{
					if ( AliasNames[j] == Alias )
					{
						if ( MenuValues1[j] == "" )
							MenuValues1[j] = KeyName;
						else if ( MenuValues2[j] == "" )
							MenuValues2[j] = KeyName;
					}
				}
			}
		}
	}
}

function ProcessMenuKey( int KeyNo, string[32] KeyName )
{
	local int i;

	if ( (KeyName == "") || (KeyName == "Escape")  
		|| ((KeyNo >= 0x70 ) && (KeyNo <= 0x79)) ) //function keys
		return;

	// make sure no overlapping
	for ( i=1; i<20; i++ )
	{
		if ( MenuValues2[i] == KeyName )
			MenuValues2[i] = "";
		if ( MenuValues1[i] == KeyName )
		{
			MenuValues1[i] = MenuValues2[i];
			MenuValues2[i] = "";
		}
	}
	if ( MenuValues1[Selection] != "_" )
		MenuValues2[Selection] = MenuValues1[Selection];
	else if ( MenuValues2[Selection] == "_" )
		MenuValues2[Selection] = "";

	MenuValues1[Selection] = KeyName;
	AddPending("SET Input "$KeyName$" "$AliasNames[Selection]);
}

function ProcessMenuEscape();
function ProcessMenuUpdate( coerce string[64] InputString );

function bool ProcessSelection()
{
	local int i;

	if ( Selection == MenuLength )
	{
		Pending = 0;
		PlayerOwner.ResetKeyboard();
		for ( i=0; i<24; i++ )
		{
			MenuValues1[i] = "";
			MenuValues2[i] = "";
		}
		SetupMenu();
		return true;
	}
	if ( MenuValues2[Selection] != "" )
	{
		AddPending( "SET Input "$MenuValues2[Selection]$" ");
		AddPending( "SET Input "$MenuValues1[Selection]$" ");
		MenuValues1[Selection] = "_";
		MenuValues2[Selection] = "";
	}
	else
		MenuValues2[Selection] = "_";
		
	PlayerOwner.Player.Console.GotoState('KeyMenuing');
	return true;
}

function DrawValues(canvas Canvas, Font RegFont, int Spacing, int StartX, int StartY)
{
	local int i;

	Canvas.Font = RegFont;

	for (i=0; i< MenuLength; i++ )
		if ( MenuValues1[i+1] != "" )
		{
			SetFontBrightness( Canvas, (i == Selection - 1) );
			Canvas.SetPos(StartX, StartY + Spacing * i);
			if ( MenuValues2[i+1] == "" )
				Canvas.DrawText(MenuValues1[i + 1], false);
			else
				Canvas.DrawText(MenuValues1[i + 1]$OrString$MenuValues2[i+1], false);
		}
		Canvas.DrawColor = Canvas.Default.DrawColor;
}

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing;
	
	DrawBackGround(Canvas, (Canvas.ClipY < 250));

	Spacing = Clamp(0.04 * Canvas.ClipY, 9, 32);
	StartX = Max(8, 0.5 * Canvas.ClipX - 120);

	if ( Canvas.ClipY > 280 )
	{	
		DrawTitle(Canvas);
		StartY = Max(36, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));
	}
	else
		StartY = Max(4, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));

	if ( !bSetup )
		SetupMenu();

	DrawList(Canvas, false, Spacing, StartX, StartY); 
	DrawValues(Canvas, Canvas.MedFont, Spacing, StartX+112, StartY);

}

defaultproperties
{
	 OrString=" or "
	 MenuTitle="CONTROLS"
	 MenuLength=21
	 MenuList(1)="Fire"
	 MenuList(2)="Alternate Fire"
	 MenuList(3)="Move Forward"
	 MenuList(4)="Move Backward"
	 MenuList(5)="Turn Left"
	 MenuList(6)="Turn Right"
	 MenuList(7)="Strafe Left"
	 MenuList(8)="Strafe Right"
	 MenuList(9)="Jump/Up"
	 MenuList(10)="Crouch/Down"
	 MenuList(11)="Mouse Look"
	 MenuList(12)="Activate Item"
	 MenuList(13)="Next Item"
	 MenuList(14)="Previous Item"
	 MenuList(15)="Look Up"
	 MenuList(16)="Look Down"
	 MenuList(17)="Center View"
	 MenuList(18)="Walk"
	 MenuList(19)="Strafe"
	 MenuList(20)="Next Weapon"
	 MenuList(21)="RESET TO DEFAULTS"
	 AliasNames(1)="Fire"
	 AliasNames(2)="AltFire"
	 AliasNames(3)="MoveForward"
	 AliasNames(4)="MoveBackward"
	 AliasNames(5)="TurnLeft"
	 AliasNames(6)="TurnRight"
	 AliasNames(7)="StrafeLeft"
	 AliasNames(8)="StrafeRight"
	 AliasNames(9)="Jump"
	 AliasNames(10)="Duck"
	 AliasNames(11)="Look"
	 AliasNames(12)="InventoryActivate"
	 AliasNames(13)="InventoryNext"
	 AliasNames(14)="InventoryPrevious"
	 AliasNames(15)="LookUp"
	 AliasNames(16)="LookDown"
	 AliasNames(17)="CenterView"
	 AliasNames(18)="Walking"
	 AliasNames(19)="Strafe"
	 AliasNames(20)="NextWeapon"
	 HelpMessage(1)=""
}
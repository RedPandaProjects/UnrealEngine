//=============================================================================
// UnrealPlayerMenu
//=============================================================================
class UnrealPlayerMenu expands UnrealMenu
	localized;

var actor RealOwner;
var bool bSetup;
var string[21] PlayerName, TeamName;
var string[64] PreferredSkin;

function ProcessMenuInput( coerce string[64] InputString )
{
	InputString = Left(InputString, 20);

	if ( selection == 1 )
	{
		PlayerOwner.ChangeName(InputString);
		PlayerName = PlayerOwner.PlayerName;
		PlayerOwner.UpdateURL("Name="$InputString);
	}
	else if ( selection == 2 )
	{
		PlayerOwner.ChangeTeam(InputString);
		TeamName = PlayerOwner.TeamName;
		PlayerOwner.UpdateURL("Team="$InputString);
	}
}

function ProcessMenuEscape()
{
	PlayerName = PlayerOwner.PlayerName;
	TeamName = PlayerOwner.TeamName;
}

function ProcessMenuUpdate( coerce string[64] InputString )
{
	InputString = Left(InputString, 20);

	if ( selection == 1 )
		PlayerName = (InputString$"_");
	else if ( selection == 2 )
		TeamName = (InputString$"_");
}

function Menu ExitMenu()
{
	SetOwner(RealOwner);
	Super.ExitMenu();
}

function bool ProcessLeft()
{
	local string[64] SkinName;
	local texture NewSkin;

	if ( Selection == 1 )
	{
		PlayerName = "_";
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else if ( Selection == 2 )
	{
		TeamName = "_";
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else if ( Selection == 3 )
	{
		SkinName = GetNextSkin(string(Mesh), string(Mesh)$"Skins."$string(Skin), -1);
		if ( SkinName != "" )
		{
			PlayerOwner.ServerChangeSkin(SkinName);
			PreferredSkin = SkinName;
			PlayerOwner.UpdateURL("Skin="$SkinName);
		}
	}

	return true;
}

function bool ProcessRight()
{
	local string[64] SkinName;
	local texture NewSkin;

	if ( Selection == 1 )
	{
		PlayerName = "_";
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else if ( Selection == 2 )
	{
		TeamName = "_";
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else if ( Selection == 3 )
	{
		SkinName = GetNextSkin(string(Mesh), string(Mesh)$"Skins."$string(Skin), 1);
		if ( SkinName != "" )
		{
			PlayerOwner.ServerChangeSkin(SkinName);
			PreferredSkin = SkinName;
			PlayerOwner.UpdateURL("Skin="$SkinName);
		}
	}

	return true;
}

function bool ProcessSelection()
{
	if ( Selection == 1 )
	{
		PlayerName = "_";
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else if ( Selection == 2 )
	{
		TeamName = "_";
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	return true;
}

function SaveConfigs()
{
	PlayerOwner.SaveConfig();
}

function SetUpDisplay()
{
	bSetup = true;
	RealOwner = Owner;
	SetOwner(PlayerOwner);
	PlayerName = PlayerOwner.PlayerName;
	TeamName = PlayerOwner.TeamName;
	PlayerOwner.bBehindView = false;
	Mesh = PlayerOwner.Mesh;
	Skin = PlayerOwner.Skin;
	LoopAnim(AnimSequence);
}

function DrawMenu(canvas Canvas)
{
	local int i, StartX, StartY, Spacing;
	local vector DrawOffset;
	local rotator NewRot;

	//DrawBackGround(Canvas, false);

	// Draw Title
	DrawTitle(Canvas);

	Spacing = Clamp(0.04 * Canvas.ClipY, 12, 32);
	StartX = Max(10, 0.25 * Canvas.ClipX - 115);
	StartY = Max(40, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));

	if ( !bSetup )
		SetUpDisplay();

	for ( i=1; i<6; i++ )
		MenuList[i] = Default.MenuList[i];
	DrawList(Canvas, false, Spacing, StartX, StartY);  

	if ( !PlayerOwner.Player.Console.IsInState('MenuTyping') )
	{
		PlayerName = PlayerOwner.PlayerName;
		TeamName = PlayerOwner.TeamName;
	}
	MenuList[1] = PlayerName;
	MenuList[2] = TeamName;
	if ( Mesh == PlayerOwner.Mesh )
	{
		Skin = PlayerOwner.Skin;
	}

	if ( Mesh != None )
	{
		MenuList[3] = string(Skin);
		MenuList[4] = string(Mesh);
	}
	else
	{
		MenuList[3] = "";
		MenuList[4] = "Spectator";
	}
	MenuList[5] = "";
	DrawList(Canvas, false, Spacing, StartX + 80, StartY);  

	// Draw help panel
	DrawHelpPanel(Canvas, Canvas.ClipY - 64, 228);

	PlayerOwner.ViewRotation.Pitch = 0;
	PlayerOwner.ViewRotation.Roll = 0;
	DrawOffset = ((vect(4.0,0.0,0.0)) >> PlayerOwner.ViewRotation);
	DrawOffset += (PlayerOwner.EyeHeight * vect(0,0,1));
	SetLocation(PlayerOwner.Location + DrawOffset);
	NewRot = PlayerOwner.ViewRotation;
	NewRot.Yaw = Rotation.Yaw;
	SetRotation(NewRot);
}

function DrawHelpPanel(canvas Canvas, int StartY, int XClip)
{
	local int StartX;

	StartX = 0.5 * Canvas.ClipX - 128;

	Canvas.bCenter = false;
	Canvas.Font = Canvas.MedFont;
	Canvas.SetOrigin(StartX + 18, StartY + 16);
	Canvas.SetClip(XClip,64);
	Canvas.SetPos(0,0);
	Canvas.Style = 1;	
	if ( Selection < 20 )
		Canvas.DrawText(HelpMessage[Selection], False);	
	Canvas.SetPos(0,32);
}

function MenuTick( float DeltaTime )
{
	local rotator newRot;
	local float RemainingTime;

	if ( Level.Pauser == "" )
		return;

	// explicit rotation, since game is paused
	newRot = Rotation;
	newRot.Yaw = newRot.Yaw + RotationRate.Yaw * DeltaTime;
	SetRotation(newRot);

	//explicit animation
	RemainingTime = DeltaTime * 0.5;
	while ( RemainingTime > 0 )
	{
		if ( AnimFrame < 0 )
		{
			AnimFrame += TweenRate * RemainingTime;
			if ( AnimFrame > 0 )
				RemainingTime = AnimFrame/TweenRate;
			else
				RemainingTime = 0;
		}
		else
		{
			AnimFrame += AnimRate * RemainingTime;
			if ( AnimFrame > 1 )
			{
				RemainingTime = (AnimFrame - 1)/AnimRate;
				AnimFrame = 0;
			}
			else
				RemainingTime = 0;
		}
	}
}

defaultproperties
{
	MenuLength=3
	MenuTitle="PLAYER"
	MenuList(1)="Name: "
	MenuList(2)="Team Name:"
	MenuList(3)="Skin:"
	HelpMessage(1)="Hit enter to type in your name. Be sure to do this before joining a multiplayer game."
	HelpMessage(2)="Hit enter to type in your team (Red, Blue, Green, or Yellow)."
	HelpMessage(3)="Change your skin using the left and right arrow keys."
	bHidden=false
	bOnlyOwnerSee=true
	bUnlit=true
	DrawScale=+00000.040000
    bFixedRotationDir=True
    RotationRate=(Yaw=8000)
    DesiredRotation=(Yaw=30000)
    DrawType=DT_Mesh
	AnimSequence=Walk
	Physics=PHYS_Rotating
}
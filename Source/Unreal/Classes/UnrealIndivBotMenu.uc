//=============================================================================
// UnrealndivBotMenu
//=============================================================================
class UnrealndivBotMenu expands UnrealMenu
	config
	localized;

var actor RealOwner;
var bool bSetup;
var int Num;
var config string[32] PlayerClasses[16];
var int NumPlayerClasses;
var int PlayerClassNum;
var string[32] RealName, RealTeam;
var byte SkinNum;
var   string[64] MenuValues[20];
var	BotInfo BotConfig;

function PostBeginPlay()
{
	if ( class'GameInfo'.Default.bShareWare )
		NumPlayerClasses = 1;
	if ( Level.Game.IsA('DeathMatchGame') )
		BotConfig = DeathMatchGame(Level.Game).BotConfig;
	else
		BotConfig = Spawn(class'BotInfo');
	Super.PostBeginPlay();
}

function Destroyed()
{
	Super.Destroyed();
	if ( !Level.Game.IsA('DeathMatchGame') || (BotConfig != DeathMatchGame(Level.Game).BotConfig) )
		BotConfig.Destroy();
}

function ProcessMenuInput( coerce string[64] InputString )
{
	InputString = Left(InputString, 20);
	if ( selection == 2 )
		BotConfig.SetBotName(InputString, Num);
	else if ( selection == 6 )
		BotConfig.SetBotTeam(InputString, Num);
}

function ProcessMenuEscape()
{
	if ( selection == 2 )
		BotConfig.SetBotName(RealName, Num);
	else if ( selection == 6 )
		BotConfig.SetBotTeam(RealTeam, Num);
}

function ProcessMenuUpdate( coerce string[64] InputString )
{
	InputString = Left(InputString, 19);
	if ( selection == 2 )
		BotConfig.SetBotName(InputString$"_", Num);
	else if ( selection == 6 )
		BotConfig.SetBotTeam(InputString$"_", Num);
}

function Menu ExitMenu()
{
	SetOwner(RealOwner);
	Super.ExitMenu();
}

function bool ProcessLeft()
{
	local int i;
	local string[64] SkinName;
	local texture NewSkin;

	if ( Selection == 1 )
	{
		Num--;
		if ( Num < 0 )
			Num = 15;

		for ( i=0; i<NumPlayerClasses; i++ )
			if( (PlayerClasses[i]$"Bot") ~= BotConfig.GetBotClassName(Num) )
			{
				PlayerClassNum = i;
				break;
			}
		SkinName = BotConfig.GetBotSkin(Num);
		ChangeMesh();
		if ( SkinName != "" )
		{
			NewSkin = texture(DynamicLoadObject(SkinName, class'Texture'));
			if ( NewSkin != None )
			{
				Skin = NewSkin;
				BotConfig.SetBotSkin(SkinName, Num);
			}
		}
	}
	else if ( Selection == 2 )
	{
		RealName = BotConfig.GetBotName(Num);
		BotConfig.SetBotName("_", Num);
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else if ( selection == 3 )
	{
		PlayerClassNum++;
		if ( PlayerClassNum == NumPlayerClasses )
			PlayerClassNum = 0;
		ChangeMesh();
	}	
	else if ( Selection == 4 )
	{
		SkinName = GetNextSkin(string(Mesh), string(Mesh)$"Skins."$string(Skin), -1);
		if ( SkinName != "" )
		{
			NewSkin = texture(DynamicLoadObject(SkinName, class'Texture'));
			if ( NewSkin != None )
			{
				Skin = NewSkin;
				BotConfig.SetBotSkin(SkinName, Num);
			}
		}
	}
	else if ( selection == 5 )
		BotConfig.BotSkills[Num] = FMax(0, BotConfig.BotSkills[Num] - 0.2);
	else if ( Selection == 6 )
	{
		RealName = BotConfig.GetBotTeam(Num);
		BotConfig.SetBotTeam("_", Num);
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else
		return false;

	return true;
}

function bool ProcessRight()
{
	local int i;
	local string[64] SkinName;
	local texture NewSkin;

	if ( Selection == 1 )
	{
		Num++;
		if ( Num > 15 )
			Num = 0;

		for ( i=0; i<NumPlayerClasses; i++ )
			if ( (PlayerClasses[i]$"Bot") ~= BotConfig.GetBotClassName(Num) )
			{
				PlayerClassNum = i;
				break;
			}
		SkinName = BotConfig.GetBotSkin(Num);
		ChangeMesh();
		if ( SkinName != "" )
		{
			NewSkin = texture(DynamicLoadObject(SkinName, class'Texture'));
			if ( NewSkin != None )
			{
				Skin = NewSkin;
				BotConfig.SetBotSkin(SkinName, Num);
			}
		}
	}
	else if ( Selection == 2 )
	{
		RealName = BotConfig.GetBotName(Num);
		BotConfig.SetBotName("_", Num);
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else if ( selection == 3 )
	{
		PlayerClassNum--;
		if ( PlayerClassNum < 0 )
			PlayerClassNum = NumPlayerClasses - 1;
		ChangeMesh();
	}
	else if ( Selection == 4 )
	{
		SkinName = GetNextSkin(string(Mesh), string(Mesh)$"Skins."$string(Skin), 1);
		if ( SkinName != "" )
		{
			NewSkin = texture(DynamicLoadObject(SkinName, class'Texture'));
			if ( NewSkin != None )
			{
				Skin = NewSkin;
				BotConfig.SetBotSkin(SkinName, Num);
			}
		}
	}
	else if ( selection == 5 )
		BotConfig.BotSkills[Num] = FMin(3.0, BotConfig.BotSkills[Num] + 0.2);
	else if ( Selection == 6 )
	{
		RealName = BotConfig.GetBotTeam(Num);
		BotConfig.SetBotTeam("_", Num);
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else
		return false;

	return true;
}

function bool ProcessSelection()
{
	local Menu ChildMenu;

	if ( Selection == 2 )
	{
		RealName = BotConfig.GetBotName(Num);
		BotConfig.SetBotName("_", Num);
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else if ( Selection == 6 )
	{
		RealName = BotConfig.GetBotTeam(Num);
		BotConfig.SetBotTeam("_", Num);
		PlayerOwner.Player.Console.GotoState('MenuTyping');
	}
	else
		return false;

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
	BotConfig.SaveConfig();
}

function SetUpDisplay()
{
	local int i;
	local string[64] SkinName;
	local texture NewSkin;

	for ( i=0; i<NumPlayerClasses; i++ )
		if ( (PlayerClasses[i]$"Bot") ~= BotConfig.GetBotClassName(Num) )
		{
			PlayerClassNum = i;
			break;
		}

	bSetup = true;
	RealOwner = Owner;
	SetOwner(PlayerOwner);
	SkinName = BotConfig.GetBotSkin(Num);
	ChangeMesh();
	if ( SkinName != "" )
	{
		NewSkin = texture(DynamicLoadObject(SkinName, class'Texture'));
		if ( NewSkin != None )
		{
			Skin = NewSkin;
			BotConfig.SetBotSkin(SkinName, Num);
		}
	}
	LoopAnim(AnimSequence);
}

function DrawValues(canvas Canvas, Font RegFont, int Spacing, int StartX, int StartY)
{
	local int i;

	Canvas.Font = RegFont;
	for (i=0; i< MenuLength; i++ )
	{
		SetFontBrightness( Canvas, (i == Selection - 1) );
		Canvas.SetPos(StartX, StartY + Spacing * i);
		Canvas.DrawText(MenuValues[i + 1], false);
	}
	Canvas.DrawColor = Canvas.Default.DrawColor;
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

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing, i;
	local bool bFoundValue;
	local vector DrawOffset;
	local rotator NewRot;

	//DrawBackGround(Canvas, false);

	// Draw Title
	DrawTitle(Canvas);
		
	Spacing = Clamp(0.04 * Canvas.ClipY, 12, 32);
	StartX = Max(10, 0.25 * Canvas.ClipX - 115);
	StartY = Max(40, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));

	// draw text
	DrawList(Canvas, false, Spacing, StartX, StartY);  

	MenuValues[1] = string(Num);
	MenuValues[2] = BotConfig.GetBotName(Num);
	MenuValues[3] = string(Mesh);
	MenuValues[4] = string(Skin);
	MenuValues[5] = string(BotConfig.BotSkills[Num]);
	MenuValues[6] = BotConfig.GetBotTeam(Num);
	DrawValues(Canvas, Canvas.MedFont, Spacing, StartX+120, StartY);

	if ( !bSetup )
		SetupDisplay();

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


function MenuTick( float DeltaTime )
{
	local rotator newRot;
	local float RemainingTime;

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

function ChangeMesh()
{ 
	local class<playerpawn> NewPlayerClass;

	BotConfig.SetBotClass(PlayerClasses[PlayerClassNum]$"Bot", Num);
 	NewPlayerClass = class<playerpawn>(DynamicLoadObject(PlayerClasses[PlayerClassNum], class'Class'));
	BotConfig.SetBotSkin(string(NewPlayerClass.Default.Skin), Num); 
	mesh = NewPlayerClass.Default.mesh;
	skin = NewPlayerClass.Default.skin;
}	

defaultproperties
{
	 MenuTitle="BOT CONFIG"
	 MenuList(1)="Configuration"
	 MenuList(2)="Name"
	 MenuList(3)="Class"
	 MenuList(4)="Skin"
	 MenuList(5)="Skill Adjust"
	 MenuList(6)="Team"
     HelpMessage(1)="Which Bot Configuration is being edited. Use left and right arrows to change."
     HelpMessage(2)="Hit enter to edit the name of this bot."
     HelpMessage(3)="Use the left and right arrow keys to change the class of this bot."
     HelpMessage(4)="Use the left and right arrow keys to change the skin of this bot."
	 HelpMessage(5)="Adjust the overall skill of this bot by this amount (relative to the base skill for bots)."
	 HelpMessage(6)="Type in which team this bot plays on (Red, Blue, Green, or Yellow)."
     MenuLength=6
	 PlayerClasses(0)="Unreal.FemaleOne"
	 PlayerClasses(1)="Unreal.MaleThree"
	 PlayerClasses(2)="Unreal.MaleOne"
	 PlayerClasses(3)="Unreal.MaleTwo"
	 PlayerClasses(4)="Unreal.FemaleTwo"
	 PlayerClasses(5)="Unreal.SkaarjPlayer"
	 NumPlayerClasses=6
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

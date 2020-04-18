//=============================================================================
// UnrealBotConfigMenu
//=============================================================================
class UnrealBotConfigMenu expands UnrealMenu
	localized;

var   string[64] MenuValues[20];
var		bool bAdjustSkill;
var		bool bRandomOrder;

function PostBeginPlay()
{
	local BotInfo BotConfig;

	Super.PostBeginPlay();

	if ( Level.Game.IsA('DeathMatchGame') )
		BotConfig = DeathMatchGame(Level.Game).BotConfig;
	else
		BotConfig = Spawn(class'BotInfo');

	bAdjustSkill = BotConfig.bAdjustSkill;
	bRandomOrder = BotConfig.bRandomOrder;

	if ( !Level.Game.IsA('DeathMatchGame') )
		BotConfig.Destroy();
}

function bool ProcessYes()
{
	if ( Selection == 1 )
		bAdjustSkill = true;
	else if ( Selection == 3 )
		bRandomOrder = true;
	else
		return false;

	return true;
}

function bool ProcessNo()
{
	if ( Selection == 1 )
		bAdjustSkill = false;
	else if ( Selection == 3 )
		bRandomOrder = false;
	else
		return false;

	return true;
}

function bool ProcessLeft()
{
	if ( Selection == 1 )
		bAdjustSkill = !bAdjustSkill;
	else if ( Selection == 2 )
		Level.Game.Difficulty = Max( 0, Level.Game.Difficulty - 1 );
	else if ( Selection == 3 )
		bRandomOrder = !bRandomOrder;
	else
		return false;


	return true;
}

function bool ProcessRight()
{
	if ( Selection == 1 )
		bAdjustSkill = !bAdjustSkill;
	else if ( Selection == 2 )
		Level.Game.Difficulty = Min( 3, Level.Game.Difficulty + 1 );
	else if ( Selection == 3 )
		bRandomOrder = !bRandomOrder;
	else
		return false;

	return true;
}

function bool ProcessSelection()
{
	local Menu ChildMenu;

	if ( Selection == 1 )
		bAdjustSkill = !bAdjustSkill;
	else if ( Selection == 3 )
		bRandomOrder = !bRandomOrder;
	else if ( Selection == 4 )
		ChildMenu = spawn(class'UnrealndivBotMenu', owner);
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
	local BotInfo BotConfig;

	if ( Level.Game.IsA('DeathMatchGame') )
	{
		DeathMatchGame(Level.Game).BotConfig.bAdjustSkill = bAdjustSkill;
		DeathMatchGame(Level.Game).BotConfig.bRandomOrder = bRandomOrder;
	}
	BotConfig = Spawn(class'BotInfo');
	BotConfig.bAdjustSkill = bAdjustSkill;
	BotConfig.bRandomOrder = bRandomOrder;
	Level.Game.SaveConfig();
	BotConfig.SaveConfig();

	if ( !Level.Game.IsA('DeathMatchGame') )
		BotConfig.Destroy();
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

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing, i;
	local bool bFoundValue;

	DrawBackGround(Canvas, (Canvas.ClipY < 250) );

	// Draw Title
	DrawTitle(Canvas);
		
	Spacing = Clamp(0.04 * Canvas.ClipY, 11, 32);
	StartX = Max(40, 0.5 * Canvas.ClipX - 120);
	StartY = Max(36, 0.5 * (Canvas.ClipY - MenuLength * Spacing - 128));

	// draw text
	DrawList(Canvas, false, Spacing, StartX, StartY);  

	MenuValues[1] = string(bAdjustSkill);
	MenuValues[2] = string(Level.Game.Difficulty);
	MenuValues[3] = string(bRandomOrder);
	
	DrawValues(Canvas, Canvas.MedFont, Spacing, StartX+160, StartY);

	// Draw help panel
	DrawHelpPanel(Canvas, StartY + MenuLength * Spacing, 228);
}

defaultproperties
{
	 MenuTitle="BOTS"
	 MenuList(1)="Auto-Adjust Skills"
	 MenuList(2)="Base Skill"
	 MenuList(3)="Random Order"
	 MenuList(4)="Configure Individual Bots"
     HelpMessage(1)="If true, bots adjust their skill level based on how they are doing against players."
     HelpMessage(2)="Base skill level of bots (between 0 and 3)."
     HelpMessage(3)="If true, bots enter the game in random order. If false, they enter in their configuration order."
     HelpMessage(4)="Change the configuration of individual bots."
     MenuLength=4
}

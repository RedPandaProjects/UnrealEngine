//=============================================================================
// BotInfo.
//=============================================================================
class BotInfo expands Info
	config;

var() config bool	bAdjustSkill;
var() config bool	bRandomOrder;

var() config string[20] BotNames[32];
var() config string[20] BotTeams[32];
var() config float BotSkills[32];
var	  byte ConfigUsed[32];
var() config string[32] BotClasses[32];
var() config string[64] BotSkins[32];

function PreBeginPlay()
{
	//DON'T Call parent prebeginplay
}

function int ChooseBotInfo()
{
	local int n, start;

	if ( bRandomOrder )
		n = Rand(16);
	else 
		n = 0;

	start = n;
	while ( (n < 32) && (ConfigUsed[n] == 1) )
		n++;

	if ( (n == 32) && bRandomOrder )
	{
		n = 0;
		while ( (n < start) && (ConfigUsed[n] == 1) )
			n++;
	}

	if ( n > 31 )
		n = 31;

	return n;
}

function class<bots> GetBotClass(int n)
{
	return class<bots>( DynamicLoadObject(GetBotClassName(n), class'Class') );
}

function Individualize(bots NewBot, int n, int NumBots)
{
	local texture NewSkin;
	local string[64] NewSkinName;

	// Set bot's skin
	NewSkinName = string(NewBot.skin);
	if ( (n >= 0) && (n < 32) && (BotSkins[n] != "") )
		NewSkinName = BotSkins[n];

	NewSkin = texture(DynamicLoadObject(string(NewBot.Mesh)$"Skins."$NewSkinName, class'Texture'));
	if ( NewSkin != None )
		NewBot.Skin = NewSkin;

	// Set bot's name.
	if ( (BotNames[n] == "") || (ConfigUsed[n] == 1) )
	{
		if ( NewBot.skin != None )
			BotNames[n] = string(NewBot.skin);
		else
			BotNames[n] = "Bot";
	}

	Level.Game.ChangeName( NewBot, BotNames[n], false );
	if ( BotNames[n] != NewBot.PlayerName )
		Level.Game.ChangeName( NewBot, ("Bot"$NumBots), false);

	ConfigUsed[n] = 1;

	// adjust bot skill
	NewBot.Skill = FClamp(NewBot.Skill + BotSkills[n], 0, 3);
	NewBot.ReSetSkill();
}

function SetBotClass(string[32] ClassName, int n)
{
	BotClasses[n] = ClassName;
}

function SetBotName(coerce string[20] NewName, int n)
{
	BotNames[n] = NewName;
}

function String[32] GetBotName(int n)
{
	return BotNames[n];
}

function string[20] GetBotTeam(int num)
{
	return BotTeams[Num];
}

function SetBotTeam(coerce string[20] NewTeam, int n)
{
	BotTeams[n] = NewTeam;
}

function string[64] GetBotSkin(int num)
{
	return BotSkins[Num];
}

function SetBotSkin(coerce string[64] NewSkin, int n)
{
	BotSkins[n] = NewSkin;
}

function String[32] GetBotClassName(int n)
{
	local float decision;

	if ( (n < 0) || (n > 31) || (BotClasses[n] == "") )
	{
		decision = FRand();
		if ( decision < 0.16 )
			BotClasses[n] = "Unreal.FemaleOneBot";
		else if ( decision < 0.33 )
			BotClasses[n] = "Unreal.FemaleTwoBot";
		else if ( decision < 0.5 )
			BotClasses[n] = "Unreal.MaleOneBot";
		else if ( decision < 0.67 )
			BotClasses[n] = "Unreal.MaleTwoBot";
		else if ( decision < 0.83 )
			BotClasses[n] = "Unreal.MaleThreeBot";	
		else
			BotClasses[n] = "Unreal.SkaarjPlayerBot";
	}

	if ( Level.Game.bShareware )
	{
		if ( BotClasses[n] == "Unreal.FemaleTwoBot" )
			BotClasses[n] = "Unreal.FemaleOneBot";
		else
			BotClasses[n] = "Unreal.MaleThreeBot";
	}

	return BotClasses[n];
}

defaultproperties
{
	BotNames(0)="Dante"
	BotNames(1)="Ash"
	BotNames(2)="Rhiannon"
	BotNames(3)="Kurgan"
	BotNames(4)="Sonja"
	BotNames(5)="Avatar"
	BotNames(6)="Dominator"
	BotNames(7)="Cholerae"
	BotNames(8)="Apocalypse"
	BotNames(9)="Bane"
	BotNames(10)="Hippolyta"
	BotNames(11)="Eradicator"
	BotNames(12)="Nikita"
	BotNames(13)="Arcturus"
	BotNames(14)="Shiva"
	BotNames(15)="Vindicator"

	BotClasses(0)="Unreal.MaleThreeBot"
	BotClasses(1)="Unreal.MaleTwoBot"
	BotClasses(2)="Unreal.FemaleOneBot"
	BotClasses(3)="Unreal.MaleOneBot"
	BotClasses(4)="Unreal.FemaleTwoBot"
	BotClasses(5)="Unreal.MaleThreeBot"
	BotClasses(6)="Unreal.SkaarjPlayerBot"
	BotClasses(7)="Unreal.FemaleOneBot"
	BotClasses(8)="Unreal.MaleThreeBot"
	BotClasses(9)="Unreal.MaleTwoBot"
	BotClasses(10)="Unreal.FemaleTwoBot"
	BotClasses(11)="Unreal.SkaarjPlayerBot"
	BotClasses(12)="Unreal.FemaleOneBot"
	BotClasses(13)="Unreal.MaleOneBot"
	BotClasses(14)="Unreal.MaleTwoBot"
	BotClasses(15)="Unreal.SkaarjPlayerBot"

	BotTeams(0)="Blue"
	BotTeams(1)="Red"
	BotTeams(2)="Blue"
	BotTeams(3)="Red"
	BotTeams(4)="Blue"
	BotTeams(5)="Red"
	BotTeams(6)="Blue"
	BotTeams(7)="Red"
	BotTeams(8)="Blue"
	BotTeams(9)="Red"
	BotTeams(10)="Blue"
	BotTeams(11)="Red"
	BotTeams(12)="Blue"
	BotTeams(13)="Red"
	BotTeams(14)="Blue"
	BotTeams(15)="Red"
}

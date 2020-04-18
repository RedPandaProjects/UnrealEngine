//=============================================================================
// UnrealMeshMenu
//=============================================================================
class UnrealMeshMenu expands UnrealPlayerMenu
	config
	localized;

var config string[32] PlayerClasses[16];
var config int NumPlayerClasses;
var int PlayerClassNum;
var string[128] StartMap;
var config byte SinglePlayerMesh[16];
var bool SinglePlayerOnly;
var string[64] ClassString;

function PostBeginPlay()
{
	Super.PostBeginPlay();

	if ( class'GameInfo'.Default.bShareware )
		NumPlayerClasses = 2;
}
	
function SaveConfigs()
{
	PlayerOwner.SaveConfig();
	SaveConfig();
}

function bool ProcessSelection()
{
	local int i, p;

	if( selection == 5 )
	{
		SetOwner(RealOwner);
		bExitAllMenus = true;
		if ( ClassString == "" )
			ClassString = string(PlayerOwner.Class.Name);

		while ( i<NumPlayerClasses )
		{
			p = InStr(PlayerClasses[i],".");
			if ( (p != -1) 
				&& (Right(PlayerClasses[i], Len(PlayerClasses[i]) - p - 1) ~= ClassString) )
			{
				ClassString = PlayerClasses[i];
				break;
			}
			i++;
		}
		
		StartMap = StartMap
					$"?Class="$ClassString
					$"?Skin="$PreferredSkin
					$"?Name="$PlayerOwner.PlayerName
					$"?Team="$PlayerOwner.TeamName
					$"?Rate="$PlayerOwner.NetSpeed;

		SaveConfigs();
		PlayerOwner.ClientTravel(StartMap, TRAVEL_Absolute, false);
	}
	else
		Super.ProcessSelection();
	return true;
}

function bool ProcessLeft()
{
	local string[64] SkinName;
	local texture NewSkin;

	if ( selection == 4 )
	{
		PlayerClassNum++;
		if ( PlayerClassNum == NumPlayerClasses )
			PlayerClassNum = 0;
		if ( SinglePlayerOnly && (SinglePlayerMesh[PlayerClassNum] == 0) )
		{
			ProcessLeft();
			return true;
		}
		ChangeMesh();
	}
	else if ( Selection == 3 )
	{
		SkinName = GetNextSkin(string(Mesh), string(Mesh)$"Skins."$string(Skin), -1);
		if ( SkinName != "" )
		{
			if ( Mesh == PlayerOwner.Mesh )
			{
				PlayerOwner.ServerChangeSkin(SkinName);
				PreferredSkin = SkinName;
			}
			else
			{
				NewSkin = texture(DynamicLoadObject(string(Mesh)$"Skins."$SkinName, class'Texture'));
				if ( NewSkin != None )
				{
					Skin = NewSkin;
					PreferredSkin = SkinName;
				}
			}
		}
	}
	else
		Super.ProcessLeft();
	return true;
}

function bool ProcessRight()
{
	local string[64] SkinName;
	local texture NewSkin;

	if ( selection == 4 )
	{
		PlayerClassNum--;
		if ( PlayerClassNum < 0 )
			PlayerClassNum = NumPlayerClasses - 1;
		if ( SinglePlayerOnly && (SinglePlayerMesh[PlayerClassNum] == 0) )
		{
			ProcessRight();
			return true;
		}
		ChangeMesh();
	}
	else if ( Selection == 3 )
	{
		SkinName = GetNextSkin(string(Mesh), string(Mesh)$"Skins."$string(Skin), 1);
		if ( SkinName != "" )
		{
			if ( Mesh == PlayerOwner.Mesh )
			{
				PlayerOwner.ServerChangeSkin(SkinName);
				PreferredSkin = SkinName;
			}
			else
			{
				NewSkin = texture(DynamicLoadObject(string(Mesh)$"Skins."$SkinName, class'Texture'));
				if ( NewSkin != None )
				{
					Skin = NewSkin;
					PreferredSkin = SkinName;
				}
			}
		}
	}
	else 
		Super.ProcessRight();
	return true;
}

function bool ChangeMesh()
{ 
	local class<playerpawn> NewPlayerClass;

	NewPlayerClass = class<playerpawn>(DynamicLoadObject(PlayerClasses[PlayerClassNum], class'Class'));

	if ( NewPlayerClass != None )
	{
		ClassString = PlayerClasses[PlayerClassNum];
		mesh = NewPlayerClass.Default.mesh;
		skin = NewPlayerClass.Default.skin;
		PreferredSkin="";
		return true;
	}
	return false;
}	

function LoadAllMeshes()
{
	local int i;

	for ( i=0; i<NumPlayerClasses; i++ )
		DynamicLoadObject(PlayerClasses[i], class'Class');
}

function SetUpDisplay()
{
	local int i;

	Super.SetUpDisplay();

	for ( i=0; i<NumPlayerClasses; i++ )
		if ( PlayerClasses[i] ~= ("Unreal."$PlayerOwner.Class) )
		{
			PlayerClassNum = i;
			break;
		}
}

defaultproperties
{
	Selection=5
	MenuLength=5
	MenuList(4)="Class:"
	MenuList(5)="Start Game"
	HelpMessage(4)="Change your class using the left and right arrow keys."
	HelpMessage(5)="Press enter to start game."
	PlayerClasses(0)="Unreal.FemaleOne"
	PlayerClasses(1)="Unreal.FemaleTwo"
	PlayerClasses(2)="Unreal.MaleOne"
	PlayerClasses(3)="Unreal.MaleTwo"
	PlayerClasses(4)="Unreal.MaleThree"
	PlayerClasses(5)="Unreal.SkaarjPlayer"
	PlayerClasses(6)="Unreal.UnrealSpectator"
	SinglePlayerMesh(0)=1
	SinglePlayerMesh(1)=1
	SinglePlayerMesh(2)=1
	SinglePlayerMesh(3)=1
	SinglePlayerMesh(4)=1
	SinglePlayerMesh(5)=0
	SinglePlayerMesh(6)=0
	NumPlayerClasses=7
}
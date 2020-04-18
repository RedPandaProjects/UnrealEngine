//=============================================================================
// UnrealSaveMenu
//=============================================================================
class UnrealSaveMenu expands UnrealSlotMenu
	localized
	config;

var localized string[128] CantSave;

function BeginPlay()
{
	local int i;

	Super.BeginPlay();
	For (i=0; i<9; i++ )
		if (SlotNames[i] ~= "..Empty.." )
		{
			Selection = i + 1;
			break;
		}
}

function bool ProcessSelection()
{
	if ( PlayerOwner.Health <= 0 )
		return true;

	if ( Level.Minute < 10 )
		SlotNames[Selection - 1] = (Level.Title$" "$Level.Hour$"\:0"$Level.Minute$" "$MonthNames[Level.Month - 1]$" "$Level.Day);
	else
		SlotNames[Selection - 1] = (Level.Title$" "$Level.Hour$"\:"$Level.Minute$" "$MonthNames[Level.Month - 1]$" "$Level.Day);

	if ( Level.NetMode != NM_Standalone )
		SlotNames[Selection - 1] = "Net:"$SlotNames[Selection - 1];
	SaveConfig();
	bExitAllMenus = true;
	PlayerOwner.ClientMessage(" ");
	PlayerOwner.bDelayedCommand = true;
	PlayerOwner.DelayedCommand = "SaveGame "$(Selection - 1);
	return true;
}

function DrawMenu(canvas Canvas)
{

	if ( PlayerOwner.Health <= 0 )
	{
		MenuTitle = CantSave;
		DrawTitle(Canvas);
		return;
	}

	DrawBackGround(Canvas, (Canvas.ClipY < 320));
	DrawTitle(Canvas);
	DrawSlots(Canvas);	
}

defaultproperties
{
     CantSave="CAN'T SAVE WHEN DEAD"
     MenuTitle="SAVE GAME"
}

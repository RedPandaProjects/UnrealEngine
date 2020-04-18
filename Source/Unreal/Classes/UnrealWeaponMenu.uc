//=============================================================================
// UnrealWeaponMenu
//=============================================================================
class UnrealWeaponMenu expands UnrealMenu
	localized;

var int Slot[21];
var bool bInitLength;

function bool ProcessLeft()
{
	local name temp;

	if ( Selection > 1 )
	{
		temp = PlayerOwner.WeaponPriority[Slot[Selection]];
		PlayerOwner.WeaponPriority[Slot[Selection]] = PlayerOwner.WeaponPriority[Slot[Selection - 1]];
		PlayerOwner.WeaponPriority[Slot[Selection - 1]] = temp;
		Selection--;
	}
	else 
		return false;

	return true;
}

function bool ProcessRight()
{
	local name temp;

	if ( Selection < MenuLength )
	{
		temp = PlayerOwner.WeaponPriority[Slot[Selection]];
		PlayerOwner.WeaponPriority[Slot[Selection]] = PlayerOwner.WeaponPriority[Slot[Selection + 1]];
		PlayerOwner.WeaponPriority[Slot[Selection + 1]] = temp;
		Selection++;
	}
	else
		return false;

	return true;
}

function SaveConfigs()
{
	PlayerOwner.SaveConfig();
	PlayerOwner.ServerUpdateWeapons();
}

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing, i, j;

	DrawBackGround(Canvas, (Canvas.ClipY < 250));

	// Draw Title
	StartX = Max(16, 0.5 * Canvas.ClipX - 80);
	Canvas.Font = Canvas.LargeFont;
	Canvas.SetPos(StartX, 4 );
	Canvas.DrawText(MenuTitle, False);
		
	// Draw Weapon Priorities
		
	Spacing = Clamp(Canvas.ClipY/20, 10, 32);
	StartX = Max(48, 0.5 * Canvas.ClipX - 64);
	StartY = Max(40, 0.5 * (Canvas.ClipY - MenuLength * Spacing));

	j = 1;
	MenuLength = 0;
	Canvas.Font = Canvas.MedFont;
	for ( i=19; i>=0; i-- )
	{
		if ( PlayerOwner.WeaponPriority[i] != '' )
		{
			SetFontBrightness( Canvas, (Selection==j) );
			MenuLength++;
			Slot[j] = i;
			Canvas.SetPos(StartX, StartY + (j-1) * Spacing );
			j++;
			Canvas.DrawText(PlayerOwner.WeaponPriority[i], false);
			if ( MenuLength > Canvas.ClipY/Spacing - 1 )
				break;
		}
	}
	Canvas.DrawColor = Canvas.Default.DrawColor;
}

defaultproperties
{
	 MenuTitle="PRIORITIES"
     Class=Unreal.UnrealWeaponMenu
}

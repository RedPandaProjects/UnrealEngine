//=============================================================================
// UnrealSlotMenu
//=============================================================================
class UnrealSlotMenu expands UnrealMenu
	config
	localized;

var globalconfig string[100] SlotNames[9];
var localized string[16] MonthNames[12];

function DrawSlots(canvas Canvas)
{
	local int StartX, StartY, Spacing, i;
			
	Spacing = Clamp(0.05 * Canvas.ClipY, 12, 32);
	StartX = Max(20, 0.5 * (Canvas.ClipX - 206));
	StartY = Max(40, 0.5 * (Canvas.ClipY - MenuLength * Spacing-40));
	Canvas.Font = Canvas.MedFont;

	For ( i=1; i<10; i++ )
	{
		Canvas.SetPos(StartX, StartY + i * Spacing );
		Canvas.DrawText(SlotNames[i-1], False);
	}

	// show selection
	Canvas.SetPos( StartX - 20, StartY + Spacing * Selection);
	Canvas.DrawText("[]", false);	
}

defaultproperties
{
     SlotNames(0)="..Empty.."
     SlotNames(1)="..Empty.."
     SlotNames(2)="..Empty.."
     SlotNames(3)="..Empty.."
     SlotNames(4)="..Empty.."
     SlotNames(5)="..Empty.."
     SlotNames(6)="..Empty.."
     SlotNames(7)="..Empty.."
     SlotNames(8)="..Empty.."
     MonthNames(0)="January"
     MonthNames(1)="February"
     MonthNames(2)="March"
     MonthNames(3)="April"
     MonthNames(4)="May"
     MonthNames(5)="June"
     MonthNames(6)="July"
     MonthNames(7)="August"
     MonthNames(8)="September"
     MonthNames(9)="October"
     MonthNames(10)="November"
     MonthNames(11)="December"
     MenuLength=9
}

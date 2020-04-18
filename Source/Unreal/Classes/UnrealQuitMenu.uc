//=============================================================================
// UnrealQuitMenu
//=============================================================================
class UnrealQuitMenu expands UnrealMenu
	localized;

var bool bResponse;
var localized string[32] YesSelString;
var localized string[32] NoSelString;

function bool ProcessYes()
{
	bResponse = true;
	return true;
}

function bool ProcessNo()
{
	bResponse = false;
	return true;
}

function bool ProcessLeft()
{
	bResponse = !bResponse;
	return true;
}

function bool ProcessRight()
{
	bResponse = !bResponse;
	return true;
}

function bool ProcessSelection()
{
	local Menu ChildMenu;

	ChildMenu = None;

	if ( bResponse )
	{
		PlayerOwner.SaveConfig();
		if ( Level.Game != None )
			Level.Game.SaveConfig();
		PlayerOwner.ConsoleCommand("Exit");
	}
	else 
		ExitMenu();
}


function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing, SecSpace;
	
	DrawBackGround(Canvas, (Canvas.ClipY < 320));
	
	StartX = 0.5 * Canvas.ClipX - 120;
	StartY = 2;
	Spacing = 9;
	Canvas.Font = Canvas.MedFont;
	
	Canvas.SetPos(StartX, StartY );
	Canvas.DrawText(MenuList[0], False);	
	Canvas.SetPos(StartX+72, StartY+10 );	
	Canvas.DrawText(MenuList[1], False);	
	StartX = Max(8, 0.5 * Canvas.ClipX - 116);	
	Spacing = Clamp(0.04 * Canvas.ClipY, 7, 40);
	StartY = 16 + Spacing;
	SecSpace = 2 + Spacing/6;
	Canvas.Font = Canvas.SmallFont;
	
	Canvas.DrawColor.R = 30;
	Canvas.DrawColor.G = 90;
	Canvas.DrawColor.B = 30;
		
	Canvas.SetPos(StartX, StartY);
	Canvas.DrawText(MenuList[2], false);
	Canvas.SetPos(StartX+8, StartY+Spacing);
	Canvas.DrawText(MenuList[3], false);
	Canvas.SetPos(StartX, StartY+Spacing*2+SecSpace);
	Canvas.DrawText(MenuList[4], false);	
	Canvas.SetPos(StartX+8, StartY+Spacing*3+SecSpace);
	Canvas.DrawText(MenuList[5],  false);	
	Canvas.SetPos(StartX+8, StartY+Spacing*4+SecSpace);
	Canvas.DrawText(MenuList[6],  false);		
	Canvas.SetPos(StartX+8, StartY+Spacing*5+SecSpace);
	Canvas.DrawText(MenuList[7],  false);
	
	Canvas.SetPos(StartX, StartY+Spacing*6+SecSpace*2);
	Canvas.DrawText(MenuList[8],  false);
	
	Canvas.SetPos(StartX, StartY+Spacing*7+SecSpace*3);
	Canvas.DrawText(MenuList[9],  false);
	Canvas.SetPos(StartX+8, StartY+Spacing*8+SecSpace*3);
	Canvas.DrawText(MenuList[10],  false);	

	Canvas.SetPos(StartX, StartY+Spacing*9+SecSpace*4);
	Canvas.DrawText(MenuList[11],  false);
	Canvas.SetPos(StartX+8, StartY+Spacing*10+SecSpace*4);
	Canvas.DrawText(MenuList[12],  false);
	Canvas.SetPos(StartX+8, StartY+Spacing*11+SecSpace*4);
	Canvas.DrawText(MenuList[13],  false);		

	Canvas.SetPos(StartX, StartY+Spacing*12+SecSpace*5);
	Canvas.DrawText(MenuList[14],  false);	
	Canvas.SetPos(StartX+8, StartY+Spacing*13+SecSpace*5);
	Canvas.DrawText(MenuList[15],  false);
	
	Canvas.SetPos(StartX, StartY+Spacing*14+SecSpace*6);
	Canvas.DrawText(MenuList[16],  false);	

	Canvas.DrawColor.R = 40;
	Canvas.DrawColor.G = 60;
	Canvas.DrawColor.B = 20;
	
	Canvas.SetPos(StartX, StartY+Spacing*15+SecSpace*7);
	Canvas.DrawText(MenuList[17],  false);	
	
	Canvas.SetPos(StartX, StartY+Spacing*16+SecSpace*8);
	Canvas.DrawText(MenuList[18],  false);
			
	// draw text
	Canvas.Font = Canvas.MedFont;	
	SetFontBrightness(Canvas, true);
	StartY = Clamp(StartY+Spacing*17+SecSpace*9, Canvas.ClipY - 66, Canvas.ClipY - 12);
	StartX = Canvas.ClipX*0.5 - 59;
	Canvas.SetPos(StartX, StartY );
	Canvas.DrawText(MenuTitle, False);
	Canvas.SetPos(StartX + 48, StartY);
	if ( bResponse )
		Canvas.DrawText(YesSelString, False);
	else
		Canvas.DrawText(NoSelString, False);
	Canvas.DrawColor = Canvas.Default.DrawColor;

	// Draw help panel
//	DrawHelpPanel(Canvas, 0.5 * Canvas.ClipY + 16, 228);
}

defaultproperties
{
     YesSelString="[YES]  No"
     NoSelString=" Yes  [NO]"
     HelpMessage(1)="Select yes and hit enter to return to your puny, miserable, useless real life, if you can't handle Unrealty."
     MenuList(0)="A Digital Extremes/Epic Megagames"
     MenuList(1)="Collaboration"
     MenuList(2)="Game Design: James Schmalz"
     MenuList(3)="Cliff Bleszinski"
     MenuList(4)="Level Design: Cliff Bleszinski"
     MenuList(5)="T. Elliot Cannon  Pancho Eekels"
     MenuList(6)="Jeremy War  Cedric Fiorentino"
     MenuList(7)="Shane Caudle"
     MenuList(8)="Animator: Dave Carter"
     MenuList(9)="Art: James Schmalz "
     MenuList(10)="Mike Leatham  Artur Bialas"
     MenuList(11)="Programming: Tim Sweeney  Steven Polge"
     MenuList(12)="Erik de Neve  James Schmalz"
     MenuList(13)="Carlo Vogelsang  Nick Michon"
     MenuList(14)="Music: Alexander Brandon"
     MenuList(15)="Michiel van den Bos"
     MenuList(16)="Sound Effects: Dave Ewing"
     MenuList(17)="Producer for GT: Jason Schreiber"
     MenuList(18)="Biz:Mark Rein Nigel Kent Craig Lafferty"
     MenuTitle="Quit?"
}

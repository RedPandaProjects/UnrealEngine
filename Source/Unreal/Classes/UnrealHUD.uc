//=============================================================================
// UnrealHUD
// Parent class of heads up display
//=============================================================================
class UnrealHUD expands HUD
	config;

#exec TEXTURE IMPORT NAME=HalfHud FILE=TEXTURES\HUD\HalfHud.PCX GROUP="Icons" MIPS=OFF
#exec TEXTURE IMPORT NAME=HudLine FILE=TEXTURES\HUD\Line.PCX GROUP="Icons" MIPS=OFF
#exec TEXTURE IMPORT NAME=HudGreenAmmo FILE=TEXTURES\HUD\greenammo.PCX GROUP="Icons" MIPS=OFF
#exec TEXTURE IMPORT NAME=IconHealth FILE=TEXTURES\HUD\i_health.PCX GROUP="Icons" MIPS=OFF
#exec TEXTURE IMPORT NAME=IconSelection FILE=TEXTURES\HUD\i_rim.PCX GROUP="Icons" FLAGS=2 MIPS=OFF
#exec TEXTURE IMPORT NAME=IconSkull FILE=TEXTURES\HUD\i_skull.PCX GROUP="Icons" MIPS=OFF

#exec TEXTURE IMPORT NAME=TranslatorHUD3 FILE=models\TRANHUD3.PCX GROUP="Icons" FLAGS=2 MIPS=OFF

#exec TEXTURE IMPORT NAME=Crosshair1 FILE=Textures\Hud\chair1.PCX GROUP="Icons" FLAGS=2 MIPS=OFF
#exec TEXTURE IMPORT NAME=Crosshair2 FILE=Textures\Hud\chair2.PCX GROUP="Icons" FLAGS=2 MIPS=OFF
#exec TEXTURE IMPORT NAME=Crosshair3 FILE=Textures\Hud\chair3.PCX GROUP="Icons" FLAGS=2 MIPS=OFF
#exec TEXTURE IMPORT NAME=Crosshair4 FILE=Textures\Hud\chair4.PCX GROUP="Icons" FLAGS=2 MIPS=OFF
#exec TEXTURE IMPORT NAME=Crosshair5 FILE=Textures\Hud\chair5.PCX GROUP="Icons" FLAGS=2 MIPS=OFF
#exec TEXTURE IMPORT NAME=Crosshair6 FILE=Textures\Hud\chair6.PCX GROUP="Icons" FLAGS=2 MIPS=OFF
#exec TEXTURE IMPORT NAME=Crosshair7 FILE=Textures\Hud\chair7.PCX GROUP="Icons" FLAGS=2 MIPS=OFF

#exec Font Import File=Textures\Lrgred.pcx Name=LargeRedFont
#exec Font Import File=Textures\TinyFont.pcx Name=TinyFont
#exec Font Import File=Textures\TinyFon3.pcx Name=TinyWhiteFont
#exec Font Import File=Textures\TinyFon2.pcx Name=TinyRedFont

var int TranslatorTimer;
var() int TranslatorY,CurTranY,SizeY,Count;
var string[255] CurrentMessage;
var bool bDisplayTran,bFlashTranslator;

simulated function ChangeHud(int d)
{
	HudMode = HudMode + d;
	if ( HudMode>5 ) HudMode = 0;
	else if ( HudMode < 0 ) HudMode = 5;
}

simulated function ChangeCrosshair(int d)
{
	Crosshair = Crosshair + d;
	if ( Crosshair>6 ) Crosshair=0;
	else if ( Crosshair < 0 ) Crosshair = 6;
}

simulated function CreateMenu()
{
	if ( PlayerPawn(Owner).bSpecialMenu && (PlayerPawn(Owner).SpecialMenu != None) )
	{
		MainMenu = Spawn(PlayerPawn(Owner).SpecialMenu, self);
		PlayerPawn(Owner).bSpecialMenu = false;
	}
	
	if ( MainMenu == None )
		MainMenu = Spawn(MainMenuType, self);
		
	if ( MainMenu == None )
	{
		PlayerPawn(Owner).bShowMenu = false;
		Level.bPlayersOnly = false;
		return;
	}
	else
	{
		MainMenu.PlayerOwner = PlayerPawn(Owner);
		MainMenu.PlayEnterSound();
	}
}

simulated function HUDSetup(canvas canvas)
{
	// Setup the way we want to draw all HUD elements
	Canvas.Reset();
	Canvas.SpaceX=0;
	Canvas.bNoSmooth = True;
	Canvas.DrawColor.r = 255;
	Canvas.DrawColor.g = 255;
	Canvas.DrawColor.b = 255;	
	Canvas.Font = Canvas.LargeFont;
}

simulated function DrawCrossHair( canvas Canvas, int StartX, int StartY )
{
	local PlayerPawn P;

	if (Crosshair>5) Return;
	Canvas.SetPos(StartX, StartY );
	Canvas.Style = 2;
	P = PlayerPawn(Owner);	
	if ( (P.Weapon != None) && !P.bShowMenu 
		&& P.Weapon.bLockedOn && P.Weapon.bPointing) 
		Canvas.DrawIcon(Texture'Crosshair6', 1.0);
	else if (Crosshair==0) 	Canvas.DrawIcon(Texture'Crosshair1', 1.0);
	else if (Crosshair==1) 	Canvas.DrawIcon(Texture'Crosshair2', 1.0);	
	else if (Crosshair==2) 	Canvas.DrawIcon(Texture'Crosshair3', 1.0);
	else if (Crosshair==3) 	Canvas.DrawIcon(Texture'Crosshair4', 1.0);
	else if (Crosshair==4) 	Canvas.DrawIcon(Texture'Crosshair5', 1.0);	
	else if (Crosshair==5) 	Canvas.DrawIcon(Texture'Crosshair7', 1.0);		
	Canvas.Style = 1;	
}

simulated function DisplayProgressMessage( canvas Canvas )
{
	Canvas.DrawColor.R = 255;
	Canvas.DrawColor.G = 255;
	Canvas.DrawColor.B = 255;
	Canvas.SetPos(0, 0.25 * Canvas.ClipY);
	Canvas.bCenter = true;
	Canvas.Font = Canvas.MedFont;
	Canvas.DrawText(PlayerPawn(Owner).ProgressMessage, false);
	Canvas.SetPos(0, 0.25 * Canvas.ClipY + 12);
	Canvas.DrawText(PlayerPawn(Owner).ProgressMessageTwo, false);
	Canvas.bCenter = false;
}

simulated function PostRender( canvas Canvas )
{
	HUDSetup(canvas);

	if ( PlayerPawn(Owner) != None )
	{
		if ( PlayerPawn(Owner).bShowMenu )
		{
			if ( MainMenu == None )
				CreateMenu();
			if ( MainMenu != None )
				MainMenu.DrawMenu(Canvas);
			return;
		}
		if ( PlayerPawn(Owner).bShowScores )
		{
			if ( (PlayerPawn(Owner).Scoring == None) && (PlayerPawn(Owner).ScoringType != None) )
				PlayerPawn(Owner).Scoring = Spawn(PlayerPawn(Owner).ScoringType, PlayerPawn(Owner));
			if ( PlayerPawn(Owner).Scoring != None )
			{ 
				PlayerPawn(Owner).Scoring.ShowScores(Canvas);
				return;
			}
		}
		else if ( (PlayerPawn(Owner).Weapon != None) && (Level.LevelAction == LEVACT_None) )
			DrawCrossHair(Canvas, 0.5 * Canvas.ClipX - 8, 0.5 * Canvas.ClipY - 8);

		if ( PlayerPawn(Owner).ProgressTimeOut > Level.TimeSeconds )
			DisplayProgressMessage(Canvas);

	}

	if (HudMode==5) 
	{
		DrawInventory(Canvas, Canvas.ClipX-96, 0,False);		
		Return;
	}
	if (Canvas.ClipX<320) HudMode = 4;

	// Draw Armor
	if (HudMode<2) DrawArmor(Canvas, 0, 0,False);
	else if (HudMode==3 || HudMode==2) DrawArmor(Canvas, 0, Canvas.ClipY-32,False);
	else if (HudMode==4) DrawArmor(Canvas, Canvas.ClipX-64, Canvas.ClipY-64,True);
	
	// Draw Ammo
	if (HudMode!=4) DrawAmmo(Canvas, Canvas.ClipX-48-64, Canvas.ClipY-32);
	else DrawAmmo(Canvas, Canvas.ClipX-48, Canvas.ClipY-32);
	
	// Draw Health
	if (HudMode<2) DrawHealth(Canvas, 0, Canvas.ClipY-32);
	else if (HudMode==3||HudMode==2) DrawHealth(Canvas, Canvas.ClipX-128, Canvas.ClipY-32);
	else if (HudMode==4) DrawHealth(Canvas, Canvas.ClipX-64, Canvas.ClipY-32);
		
	// Display Inventory
	if (HudMode<2) DrawInventory(Canvas, Canvas.ClipX-96, 0,False);
	else if (HudMode==3) DrawInventory(Canvas, Canvas.ClipX-96, Canvas.ClipY-64,False);
	else if (HudMode==4) DrawInventory(Canvas, Canvas.ClipX-64, Canvas.ClipY-64,True);
	else if (HudMode==2) DrawInventory(Canvas, Canvas.ClipX/2-64, Canvas.ClipY-32,False);	

	// Display Frag count
	if ( (Level.Game == None) || Level.Game.IsA('DeathMatchGame') ) 
	{
		if (HudMode<3) DrawFragCount(Canvas, Canvas.ClipX-32,Canvas.ClipY-64);
		else if (HudMode==3) DrawFragCount(Canvas, 0,Canvas.ClipY-64);
		else if (HudMode==4) DrawFragCount(Canvas, 0,Canvas.ClipY-32);
	}
}

simulated function DrawFragCount(Canvas Canvas, int X, int Y)
{
	Canvas.SetPos(X,Y);
	Canvas.DrawIcon(Texture'IconSkull', 1.0);	
	Canvas.CurX -= 19;
	Canvas.CurY += 23;
	Canvas.Font = Font'TinyWhiteFont';	
	if (Pawn(Owner).Score<100) Canvas.CurX+=6;
	if (Pawn(Owner).Score<10) Canvas.CurX+=6;	
	if (Pawn(Owner).Score<0) Canvas.CurX-=6;
	Canvas.DrawText(int(Pawn(Owner).Score),False);
				
}

simulated function DrawInventory(Canvas Canvas, int X, int Y, bool bDrawOne)
{	
	local bool bGotNext, bGotPrev, bGotSelected;
	local inventory Inv,Prev, Next, SelectedItem;
	local translator Translator;
	local int TempX,TempY, HalfHUDX, HalfHUDY, AmmoIconSize, i;

	if ( HudMode < 4 ) //then draw HalfHUD
	{
		Canvas.Font = Font'TinyFont';
		HalfHUDX = Canvas.ClipX-64;
		HalfHUDY = Canvas.ClipY-32;
		Canvas.CurX = HalfHudX;
		Canvas.CurY = HalfHudY;
		Canvas.DrawIcon(Texture'HalfHud', 1.0);	
	}

	if ( Owner.Inventory==None) Return;
	bGotSelected = False;
	bGotNext = false;
	bGotPrev = false;
	Prev = None;
	Next = None;
	SelectedItem = Pawn(Owner).SelectedItem;

	for ( Inv=Owner.Inventory; Inv!=None; Inv=Inv.Inventory )
	{
		if ( !bDrawOne ) // if drawing more than one inventory, find next and previous items
		{
			if ( Inv == SelectedItem )
				bGotSelected = True;
			else if ( Inv.bActivatable )
			{
				if ( bGotSelected )
				{
					if ( !bGotNext )
					{
						Next = Inv;
						bGotNext = true;
					}
					else if ( !bGotPrev )
						Prev = Inv;
				}
				else
				{
					if ( Next == None )
						Next = Prev;
					Prev = Inv;
					bGotPrev = True;
				}
			}
		}
		
		if ( Translator(Inv) != None )
			Translator = Translator(Inv);

		if ( (HudMode < 4) && (Inv.InventoryGroup>0) && (Weapon(Inv)!=None) ) 
		{
			if (Pawn(Owner).Weapon == Inv) Canvas.Font = Font'TinyWhiteFont';
			else Canvas.Font = Font'TinyFont';
			Canvas.CurX = HalfHudX-3+Inv.InventoryGroup*6;
			Canvas.CurY = HalfHudY+4;
			if (Inv.InventoryGroup<10) Canvas.DrawText(Inv.InventoryGroup,False);
			else Canvas.DrawText("0",False);
		}
		
		
		if ( (HudMode < 4) && (Ammo(Inv)!=None) ) 
		{
			for (i=0; i<10; i++)
			{
				if (Ammo(Inv).UsedInWeaponSlot[i]==1)
				{
					Canvas.CurX = HalfHudX+3+i*6;
					if (i==0) Canvas.CurX += 60;
					Canvas.CurY = HalfHudY+11;
					AmmoIconSize = 16.0*FMin(1.0,(float(Ammo(Inv).AmmoAmount)/float(Ammo(Inv).MaxAmmo)));
					if (AmmoIconSize<8 && Ammo(Inv).AmmoAmount<10 && Ammo(Inv).AmmoAmount>0) 
					{
						Canvas.CurX -= 6;			
						Canvas.CurY += 5;
						Canvas.Font = Font'TinyRedFont';
						Canvas.DrawText(Ammo(Inv).AmmoAmount,False);				
						Canvas.CurY -= 12;
					}
					Canvas.CurY += 19-AmmoIconSize;
					Canvas.CurX -= 6;
					Canvas.DrawColor.g = 255;
					Canvas.DrawColor.r = 0;		
					Canvas.DrawColor.b = 0;					
					if (AmmoIconSize<8) 
					{
						Canvas.DrawColor.r = 255-AmmoIconSize*30;
						Canvas.DrawColor.g = AmmoIconSize*30+40;				
					}
					if (Ammo(Inv).AmmoAmount >0) 
					{
						Canvas.DrawTile(Texture'HudGreenAmmo',4.0,AmmoIconSize,0,0,4.0,AmmoIconSize);		
					}
					Canvas.DrawColor.g = 255;
					Canvas.DrawColor.r = 255;		
					Canvas.DrawColor.b = 255;	
				}
			}
		}


		
	}

	// List Translator messages if activated
	if ( Translator!=None )
	{
		if( Translator.IsInState('Activated') )
		{
			Canvas.bCenter = false;
			Canvas.Font = Canvas.MedFont;
			TempX = Canvas.ClipX;
			TempY = Canvas.ClipY;
			CurrentMessage = Translator.NewMessage;
			Canvas.Style = 2;	
			Canvas.SetPos(Canvas.ClipX/2-128, Canvas.ClipY/2-68);
			Canvas.DrawIcon(texture'TranslatorHUD3', 1.0);
			Canvas.SetOrigin(Canvas.ClipX/2-110,Canvas.ClipY/2-52);
			Canvas.SetClip(225,110);
			Canvas.SetPos(0,0);
			Canvas.Style = 1;	
			Canvas.DrawText(CurrentMessage, False);	
			HUDSetup(canvas);	
			Canvas.ClipX = TempX;
			Canvas.ClipY = TempY;
		}
		else 
			bFlashTranslator = ( Translator.bNewMessage || Translator.bNotNewMessage );
	}

	if ( HUDMode == 5 )
		return;

	if ( SelectedItem != None )
	{	
		Count++;
		if (Count>20) Count=0;
		
		if (Prev!=None) 
		{
			if ( Prev.bActive || (bFlashTranslator && (Translator == Prev) && (Count>15)) )
			{
				Canvas.DrawColor.b = 0;		
				Canvas.DrawColor.g = 0;		
			}
			DrawHudIcon(Canvas, X, Y, Prev);				
			if ( (Pickup(Prev) != None) && Pickup(Prev).bCanHaveMultipleCopies )
				DrawNumberOf(Canvas,Pickup(Prev).NumCopies,X,Y);
			Canvas.DrawColor.b = 255;
			Canvas.DrawColor.g = 255;		
		}
		if ( SelectedItem.Icon != None )	
		{
			if ( SelectedItem.bActive || (bFlashTranslator && (Translator == SelectedItem) && (Count>15)) )
			{
				Canvas.DrawColor.b = 0;		
				Canvas.DrawColor.g = 0;		
			}
			if ( (Next==None) && (Prev==None) && !bDrawOne) DrawHudIcon(Canvas, X+64, Y, SelectedItem);
			else DrawHudIcon(Canvas, X+32, Y, SelectedItem);		
			Canvas.Style = 2;
			Canvas.CurX = X+32;
			if ( (Next==None) && (Prev==None) && !bDrawOne ) Canvas.CurX = X+64;
			Canvas.CurY = Y;
			Canvas.DrawIcon(texture'IconSelection', 1.0);
			if ( (Pickup(SelectedItem) != None) 
				&& Pickup(SelectedItem).bCanHaveMultipleCopies )
				DrawNumberOf(Canvas,Pickup(SelectedItem).NumCopies,Canvas.CurX-32,Y);
			Canvas.Style = 1;
			Canvas.DrawColor.b = 255;
			Canvas.DrawColor.g = 255;		
		}
		if (Next!=None) {
			if ( Next.bActive || (bFlashTranslator && (Translator == Next) && (Count>15)) )
			{
				Canvas.DrawColor.b = 0;		
				Canvas.DrawColor.g = 0;		
			}
			DrawHudIcon(Canvas, X+64, Y, Next);
			if ( (Pickup(Next) != None) && Pickup(Next).bCanHaveMultipleCopies )
				DrawNumberOf(Canvas,Pickup(Next).NumCopies,Canvas.CurX-32,Y);
			Canvas.DrawColor.b = 255;
			Canvas.DrawColor.g = 255;
		}
	}
}

simulated function DrawNumberOf(Canvas Canvas, int NumberOf, int X, int Y)
{
	local int TempX,TempY;
	
	if (NumberOf<=0) Return;
	
	Canvas.CurX = X + 14;
	Canvas.CurY = Y + 20;
	NumberOf++;
	if (NumberOf<100) Canvas.CurX+=6;
	if (NumberOf<10) Canvas.CurX+=6;	
	Canvas.Font = Font'TinyRedFont';						
	Canvas.DrawText(NumberOf,False);			
}

simulated function DrawArmor(Canvas Canvas, int X, int Y, bool bDrawOne)
{
	Local int ArmorAmount,CurAbs;
	Local inventory Inv,BestArmor;

	ArmorAmount = 0;
	Canvas.CurX = X;
	Canvas.CurY = Y;
	CurAbs=0;
	BestArmor=None;
	for( Inv=Owner.Inventory; Inv!=None; Inv=Inv.Inventory ) 
	{
		if (Inv.bIsAnArmor) 
		{
			ArmorAmount += Inv.Charge;				
			if (Inv.Charge>0 && Inv.Icon!=None) 
			{
				if (!bDrawOne) 
				{
					DrawHudIcon(Canvas, Canvas.CurX, Y, Inv);
					DrawIconValue(Canvas, Inv.Charge);						
				}
				else if (Inv.ArmorAbsorption>CurAbs) 
				{
					CurAbs = Inv.ArmorAbsorption;
					BestArmor = Inv;
				}
			}
		}
	}
	if (bDrawOne && BestArmor!=None) 
	{
		DrawHudIcon(Canvas, Canvas.CurX, Y, BestArmor);
		DrawIconValue(Canvas, BestArmor.Charge);		
	}
	Canvas.CurY = Y;
	if (ArmorAmount>0 && HudMode==0) Canvas.DrawText(ArmorAmount,False);	
}

// Draw the icons value in text on the icon
//
simulated function DrawIconValue(Canvas Canvas, int Amount)
{
	local int TempX,TempY;

	if (HudMode==0 || HudMode==3) Return;

	TempX = Canvas.CurX;
	TempY = Canvas.CurY;
	Canvas.CurX -= 20;
	Canvas.CurY -= 5;
	if (Amount<100) Canvas.CurX+=6;
	if (Amount<10) Canvas.CurX+=6;	
	Canvas.Font = Font'TinyFont';						
	Canvas.DrawText(Amount,False);
	Canvas.Font = Canvas.LargeFont;
	Canvas.CurX = TempX;
	Canvas.CurY = TempY;						
}

simulated function DrawAmmo(Canvas Canvas, int X, int Y)
{
	if ( (Pawn(Owner).Weapon == None) || (Pawn(Owner).Weapon.AmmoType == None) )
		return;
	Canvas.CurY = Y;
	Canvas.CurX = X;
	Canvas.Font = Canvas.LargeFont;
	if (Pawn(Owner).Weapon.AmmoType.AmmoAmount<10) Canvas.Font = Font'LargeRedFont';	
	if (HudMode==0) {
		if (Pawn(Owner).Weapon.AmmoType.AmmoAmount>=100) Canvas.CurX -= 16;
		if (Pawn(Owner).Weapon.AmmoType.AmmoAmount>=10) Canvas.CurX -= 16;
		Canvas.DrawText(Pawn(Owner).Weapon.AmmoType.AmmoAmount,False);
		Canvas.CurY = Canvas.ClipY-32;
	}
	else Canvas.CurX+=16;
	if (Pawn(Owner).Weapon.AmmoType.Icon!=None) Canvas.DrawIcon(Pawn(Owner).Weapon.AmmoType.Icon, 1.0);
	Canvas.CurY += 29;
	DrawIconValue(Canvas, Pawn(Owner).Weapon.AmmoType.AmmoAmount);
	Canvas.CurX = X+19;
	Canvas.CurY = Y+29;
	if (HudMode!=1 && HudMode!=2 && HudMode!=4)  Canvas.DrawTile(Texture'HudLine',
		FMin(27.0*(float(Pawn(Owner).Weapon.AmmoType.AmmoAmount)/float(Pawn(Owner).Weapon.AmmoType.MaxAmmo)),27),2.0,0,0,32.0,2.0);
}

simulated function DrawHealth(Canvas Canvas, int X, int Y)
{
	Canvas.CurY = Y;
	Canvas.CurX = X;	
	Canvas.Font = Canvas.LargeFont;
	if (Pawn(Owner).Health<25) Canvas.Font = Font'LargeRedFont';
	Canvas.DrawIcon(Texture'IconHealth', 1.0);
	Canvas.CurY += 29;	
	DrawIconValue(Canvas, Max(0,Pawn(Owner).Health));
	Canvas.CurY -= 29;		
	if (HudMode==0) Canvas.DrawText(Max(0,Pawn(Owner).Health),False);	
	Canvas.CurY = Y+29;		
	Canvas.CurX = X+2;
	if (HudMode!=1 && HudMode!=2 && HudMode!=4) 
		Canvas.DrawTile(Texture'HudLine',FMin(27.0*(float(Pawn(Owner).Health)/float(Pawn(Owner).Default.Health)),27),2.0,0,0,32.0,2.0);	
}

simulated function DrawHudIcon(Canvas Canvas, int X, int Y, Inventory Item)
{
	Local int Width;
	if (Item.Icon==None) Return;
	Width = Canvas.CurX;
	Canvas.CurX = X;
	Canvas.CurY = Y;
	Canvas.DrawIcon(Item.Icon, 1.0);
	Canvas.CurX -= 30;
	Canvas.CurY += 28;
	if ((HudMode!=2 && HudMode!=4 && HudMode!=1) || !Item.bIsAnArmor)
		Canvas.DrawTile(Texture'HudLine',fMin(27.0,27.0*(float(Item.Charge)/float(Item.Default.Charge))),2.0,0,0,32.0,2.0);
	Canvas.CurX = Width + 32;
}

defaultproperties
{
     TranslatorY=-128
     CurTranY=-128
     HudMode=0
     MainMenuType=Class'Unreal.UnrealMainMenu'
}

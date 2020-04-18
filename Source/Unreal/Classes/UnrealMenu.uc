//=============================================================================
// UnrealMenu
//=============================================================================
class UnrealMenu expands Menu;

#exec TEXTURE IMPORT NAME=IconSkull FILE=TEXTURES\HUD\i_skull.PCX GROUP="Icons" MIPS=OFF
#exec TEXTURE IMPORT NAME=TranslatorHUD3 FILE=models\TRANHUD3.PCX GROUP="Icons" FLAGS=2 MIPS=OFF

#exec Font Import File=Textures\TinyFont.pcx Name=TinyFont
#exec Font Import File=Textures\TinyFon3.pcx Name=TinyWhiteFont
#exec Font Import File=Textures\TinyFon2.pcx Name=TinyRedFont

#exec Texture Import File=Textures\dot.pcx Name=Dot MIPS=OFF
#exec Texture Import File=Textures\Slide1.pcx Name=Slide1 MIPS=OFF
#exec Texture Import File=Textures\Slide2.pcx Name=Slide2 MIPS=OFF
#exec Texture Import File=Textures\Slide3.pcx Name=Slide3 MIPS=OFF
#exec Texture Import File=Textures\Slide4.pcx Name=Slide4 MIPS=OFF
#exec Texture Import File=Textures\ex.pcx Name=ex MIPS=OFF
#exec Texture Import File=Textures\check.pcx Name=Check MIPS=OFF

#exec OBJ LOAD FILE=textures\menugr.utx PACKAGE=Unreal.MenuGfx

#exec AUDIO IMPORT FILE="Sounds\Menu\Select4.WAV" NAME="Select4" GROUP="Menu"
#exec AUDIO IMPORT FILE="Sounds\Menu\updown3.WAV" NAME="Updown3" GROUP="Menu"
#exec AUDIO IMPORT FILE="Sounds\Menu\side1b.WAV" NAME="side1b" GROUP="Menu"

simulated function PlaySelectSound()
{
	PlayerOwner.PlaySound(sound'updown3');
}

simulated function PlayModifySound()
{
	PlayerOwner.PlaySound(sound'Select4',,2.0);
}

simulated function PlayEnterSound() 
{
	PlayerOwner.PlaySound(sound'Select4',,2.0);
}

function DrawBackGround(canvas Canvas, bool bNoLogo)
{
	local int StartX;

	Canvas.DrawColor.r = 255;
	Canvas.DrawColor.g = 255;
	Canvas.DrawColor.b = 255;	
	Canvas.bNoSmooth = True;	

	StartX = 0.5 * Canvas.ClipX - 128;
	Canvas.Style = 1;
	Canvas.SetPos(StartX,0);
	Canvas.DrawIcon(texture'Menu2', 1.0);
	
	if (Canvas.ClipY>256)
	{
		StartX = 0.5 * Canvas.ClipX - 128;
		Canvas.SetPos(StartX,256);
		Canvas.DrawIcon(texture'Menu2', 1.0);
		
		if (Canvas.ClipY>512)	
		{
			StartX = 0.5 * Canvas.ClipX - 128;
			Canvas.SetPos(StartX,512);
			Canvas.DrawIcon(texture'Menu2', 1.0);
		}
	}
	
	if ( bNoLogo )
		Return;
	
	Canvas.Style = 3;	
	StartX = 0.5 * Canvas.ClipX - 128;	
	Canvas.SetPos(StartX,Canvas.ClipY-58);	
	Canvas.DrawTile( Texture'MenuBarrier', 256, 64, 0, 0, 256, 64 );
	StartX = 0.5 * Canvas.ClipX - 128;
	Canvas.Style = 2;	
	Canvas.SetPos(StartX,Canvas.ClipY-52);
	Canvas.DrawIcon(texture'Logo2', 1.0);	
	Canvas.Style = 1;
}

function DrawSlider( canvas Canvas, int StartX, int StartY, int Value, int sMin, int StepSize )
{
	local bool bFoundValue;
	local int i;

	Canvas.SetPos( StartX, StartY );
	Canvas.DrawIcon(Texture'Slide1',1.0);	
	Canvas.Style = 2;
	bFoundValue = false;
	For ( i=1; i<8; i++ )
	{
		if ( !bFoundValue && ( StepSize * i + sMin > Value) )
		{
			bFoundValue = true; 
			Canvas.DrawIcon(Texture'Slide2',1.0);
		}
		else
			Canvas.DrawIcon(Texture'Slide4',1.0);
	}
	if ( bFoundValue )
		Canvas.DrawIcon(Texture'Slide4',1.0);
	else
		Canvas.DrawIcon(Texture'Slide2',1.0);

	Canvas.DrawIcon(Texture'Slide3',1.0);							
	Canvas.Style = 1;	
}

defaultproperties
{
}

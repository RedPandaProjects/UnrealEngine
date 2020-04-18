//=============================================================================
// UnrealListenMenu
//=============================================================================
class UnrealListenMenu expands UnrealMenu
	localized;

var string[128] LastServer;
var ClientBeaconReceiver receiver;
var float ListenTimer;

function PostBeginPlay()
{
	local class<ClientBeaconReceiver> C;

	Super.PostBeginPlay();
	C = class<ClientBeaconReceiver>(DynamicLoadObject("IpDrv.ClientBeaconReceiver", class'Class'));
	receiver = spawn(C);
}

function Destroyed()
{
	Super.Destroyed();
	if ( receiver != None )
		receiver.Destroy();
}

function bool ProcessSelection()
{
	local Menu ChildMenu;

	if ( MenuLength == 0 )
		return false;

	ChildMenu = spawn(class'UnrealMeshMenu', owner);
	UnrealMeshMenu(ChildMenu).StartMap = Receiver.GetBeaconAddress(Selection - 1);

	if ( ChildMenu != None )
	{
		HUD(Owner).MainMenu = ChildMenu;
		ChildMenu.ParentMenu = self;
		ChildMenu.PlayerOwner = PlayerOwner;
	}
	return true;
}

function DrawMenu(canvas Canvas)
{
	local int StartX, StartY, Spacing, i;
	
	DrawBackGround(Canvas, false);	
	DrawTitle(Canvas);

	MenuLength = 0;
	for ( i=0; i<16; i++ )
	{
		if ( Receiver.GetBeaconAddress(i) != "" )
		{
			MenuLength++;
			MenuList[i+1] =  Receiver.GetBeaconText(i);
		}
	}

	if ( MenuLength == 0 )	
		return;
	else if ( Selection == 0 )
		Selection = 1;

	Spacing = Clamp(0.08 * Canvas.ClipY, 12, 32);
	StartX = Max(20, 0.5 * Canvas.ClipX - 124);
	StartY = Max(40, 0.5 * (Canvas.ClipY - 3 * Spacing - 128));

	DrawList(Canvas, false, Spacing, StartX, StartY); 

	DrawHelpPanel(Canvas, StartY + MenuLength * Spacing + 8, 228);
}

defaultproperties
{
	 MenuTitle="LOCAL SERVERS"
	 MenuLength=0
	 HelpMessage(1)="Hit Enter to select this server."
	 HelpMessage(2)="Hit Enter to select this server."
	 HelpMessage(3)="Hit Enter to select this server."
	 HelpMessage(4)="Hit Enter to select this server."
	 HelpMessage(5)="Hit Enter to select this server."
	 HelpMessage(6)="Hit Enter to select this server."
	 HelpMessage(7)="Hit Enter to select this server."
	 HelpMessage(8)="Hit Enter to select this server."
	 HelpMessage(9)="Hit Enter to select this server."
	 HelpMessage(10)="Hit Enter to select this server."
	 HelpMessage(11)="Hit Enter to select this server."
	 HelpMessage(12)="Hit Enter to select this server."
	 HelpMessage(13)="Hit Enter to select this server."
	 HelpMessage(14)="Hit Enter to select this server."
	 HelpMessage(15)="Hit Enter to select this server."
	 HelpMessage(16)="Hit Enter to select this server."
	 HelpMessage(17)="Hit Enter to select this server."
	 HelpMessage(18)="Hit Enter to select this server."
	 HelpMessage(19)="Hit Enter to select this server."
}

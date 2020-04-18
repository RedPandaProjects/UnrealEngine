//=============================================================================
// EditorEngine: The UnrealEd subsystem.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class EditorEngine expands Engine
	intrinsic
	transient
	config;

#exec Texture Import File=Textures\B_MenuDn.pcx Mips=Off
#exec Texture Import File=Textures\B_MenuUp.pcx Mips=Off
#exec Texture Import File=Textures\B_CollOn.pcx Mips=Off
#exec Texture Import File=Textures\B_CollOf.pcx Mips=Off
#exec Texture Import File=Textures\B_PlyrOn.pcx Mips=Off
#exec Texture Import File=Textures\B_PlyrOf.pcx Mips=Off
#exec Texture Import File=Textures\B_LiteOn.pcx Mips=Off
#exec Texture Import File=Textures\B_LiteOf.pcx Mips=Off

#exec Texture Import File=Textures\Bad.pcx
#exec Texture Import File=Textures\Bkgnd.pcx
#exec Texture Import File=Textures\BkgndHi.pcx

// Objects.
var const int         NotifyVtbl;
var const level       Level;
var const model       TempModel;
var const texture     CurrentTexture;
var const class       CurrentClass;
var const transbuffer Trans;
var const textbuffer  Results;
var const int         Pad[8];

// Icons.
var const texture MenuUp, MenuDn;
var const texture CollOn, CollOff;
var const texture PlyrOn, PlyrOff;
var const texture LiteOn, LiteOff;

// Textures.
var const texture Bad, Bkgnd, BkgndHi;

// Toggles.
var const bool bShow2DGrid;
var const bool bShow3DGrid;
var const bool bBootstrapping;

// Other variables.
var const config int AutoSaveIndex;
var const int AutoSaveCount, Mode, ClickFlags;
var const float MovementSpeed;
var const package PackageContext;
var const vector AddLocation;
var const plane AddPlane;

// Misc.
var const dynamicarray Tools;
var const class BrowseClass;

// Grid.
var const int ConstraintsVtbl;
var(Grid) config bool GridEnabled;
var(Grid) config bool SnapVertices;
var(Grid) config float SnapDistance;
var(Grid) config vector GridSize;

// Rotation grid.
var(RotationGrid) config bool RotGridEnabled;
var(RotationGrid) config rotator RotGridSize;

// Advanced.
var(Advanced) config float FovAngleDegrees;
var(Advanced) config bool GodMode;
var(Advanced) config bool AutoSave;
var(Advanced) config byte AutosaveTimeMinutes;
var(Advanced) config string[240] GameCommandLine;
var(Advanced) config string[96] EditPackages[32];

// Color preferences.
var(Colors) config color
	C_WorldBox,
	C_GroundPlane,
	C_GroundHighlight,
	C_BrushWire,
	C_Pivot,
	C_Select,
	C_Current,
	C_AddWire,
	C_SubtractWire,
	C_GreyWire,
	C_BrushVertex,
	C_BrushSnap,
	C_Invalid,
	C_ActorWire,
	C_ActorHiWire,
	C_Black,
	C_White,
	C_Mask,
	C_SemiSolidWire,
	C_NonSolidWire,
	C_WireBackground,
	C_WireGridAxis,
	C_ActorArrow,
	C_ScaleBox,
	C_ScaleBoxHi,
	C_ZoneWire,
	C_Mover,
	C_OrthoBackground;

defaultproperties
{
     MenuUp=B_MenuUp
     MenuDn=B_MenuDn
     CollOn=B_CollOn
     CollOff=B_CollOf
     PlyrOn=B_PlyrOn
     PlyrOff=B_PlyrOf
     LiteOn=B_LiteOn
     Bad=Bad
     Bkgnd=Bkgnd
     BkgndHi=BkgndHi
}

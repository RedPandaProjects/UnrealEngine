//=============================================================================
// The brush class.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Brush expands Actor
	intrinsic;

//-----------------------------------------------------------------------------
// Variables.

// CSG operation performed in editor.
var() enum ECsgOper
{
	CSG_Active,			// Active brush.
	CSG_Add,			// Add to world.
	CSG_Subtract,		// Subtract from world.
	CSG_Intersect,		// Form from intersection with world.
	CSG_Deintersect,	// Form from negative intersection with world.
} CsgOper;

// Outdated.
var const object UnusedLightMesh;
var vector  PostPivot;

// Scaling.
var() scale MainScale;
var() scale PostScale;
var scale   TempScale;

// Information.
var() color BrushColor;
var() int	PolyFlags;
var() bool  bColored;

defaultproperties
{
     MainScale=(Scale=(X=1,Y=1,Z=1),SheerRate=0,SheerAxis=SHEER_None)
     PostScale=(Scale=(X=1,Y=1,Z=1),SheerRate=0,SheerAxis=SHEER_None)
     TempScale=(Scale=(X=1,Y=1,Z=1),SheerRate=0,SheerAxis=SHEER_None)
     bStatic=True
     bNoDelete=True
     bEdShouldSnap=True
     DrawType=DT_Brush
     bFixedRotationDir=True
}

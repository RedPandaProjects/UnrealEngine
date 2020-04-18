///=============================================================================
// Contains a configurable list of favorite destinations (as an internet starting point)
//=============================================================================
class FavoritesTeleporter expands Teleporter;

var() byte FavoriteNumber;

function PostBeginPlay()
{
	local class<menu> MenuClass;
	local UnrealFavoritesMenu TempM;

	MenuClass = class<menu>(DynamicLoadObject("Unreal.UnrealFavoritesMenu", class'Class'));
	TempM = UnrealFavoritesMenu(spawn(MenuClass));
	// FIXME (Help TIm?)
	//if ( FavoriteNumber < 12 )
	//	URL = TempM.Favorites[FavoriteNumber];
	Super.PostBeginPlay();
}

defaultproperties
{
}

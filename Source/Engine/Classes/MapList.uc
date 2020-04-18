//=============================================================================
// MapList.
//
// contains a list of maps to cycle through
//
//=============================================================================
class MapList expands Info
	config;

var config string[32] Maps[32];
var config int MapNum;

function string[32] GetNextMap()
{
	MapNum++;
	if ( MapNum > ArrayCount(Maps) - 1 )
		MapNum = 0;
	if ( Maps[MapNum] == "" )
		MapNum = 0;
	log("Map List return MapNum "$MapNum);

	SaveConfig();
	return Maps[MapNum];
}
//=============================================================================
// SavedMove is used during network play to buffer recent client moves,
// for use when the server modifies the clients actual position, etc.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class SavedMove expands Info;

// also stores info in Acceleration attribute
var SavedMove NextMove;
var float TimeStamp;
var float Delta;
var byte MoveFlags;
var EDodgeDir DodgeMove;
var bool	bSent;

final function Clear()
{
	TimeStamp = 0;
	Delta = 0;
	MoveFlags = 0;
	bSent = false;
	DodgeMove = DODGE_None;
	Acceleration = vect(0,0,0);
}

defaultproperties
{
     bHidden=True
}

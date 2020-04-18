//=============================================================================
// For internal testing.
//=============================================================================
class UnrealTestInfo expands TestInfo;

function Tick( float DeltaTime )
{
}

state QQ
{
	function f();
}
state XXAA expands AA
{
	function f();
}
state XXCCAA expands CCAA
{
	function f();
}
state XXEEDDAA expands EEDDAA
{
	function f();
}

/*=============================================================================
	UnCorSc.cpp: UnrealScript execution and support code.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Description:
	UnrealScript execution and support code.

Revision history:
	* Created by Tim Sweeney 

=============================================================================*/

#include "CorePrivate.h"

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

CORE_API void (UObject::*GIntrinsics[EX_Max])( FFrame &Stack, BYTE *&Result );
CORE_API int GIntrinsicDuplicate=0;

#if DO_SLOW_GUARD
	static int Runaway=0;
	#define CHECK_RUNAWAY {if( ++Runaway > 1000000 ) Stack.ScriptWarn( 1, "Runaway loop detected (over 1000000 iterations)" );}
	CORE_API void GInitRunaway() {Runaway=0;}
#else
	#define CHECK_RUNAWAY
	CORE_API void GInitRunaway() {}
#endif

/*-----------------------------------------------------------------------------
	FFrame implementation.
-----------------------------------------------------------------------------*/

void CDECL FFrame::ScriptWarn( UBOOL Critical, char* Fmt, ... )
{
	char TempStr[4096];
	GET_VARARGS( TempStr, Fmt );
	guard(ScriptWarn);
	if( Critical || GIsStrict ) appErrorf
	(
		"%s (%s:%04X) %s",
		Object->GetFullName(),
		Node->GetFullName(),
		Code - &Node->Script(0),
		TempStr
	);
	else debugf
	(
		NAME_ScriptWarning,
		"%s (%s:%04X) %s",
		Object->GetFullName(),
		Node->GetFullName(),
		Code - &Node->Script(0),
		TempStr
	);
	unguard;
}

/*-----------------------------------------------------------------------------
	Global script execution functions.
-----------------------------------------------------------------------------*/

//
// Have an object go to a named state, and idle at no label.
// If state is NAME_None or was not found, goes to no state.
// Returns 1 if we went to a state, 0 if went to no state.
//
EGotoState UObject::GotoState( FName NewState )
{
	guard(UObject::GotoState);
	if( !MainFrame )
		return GOTOSTATE_NotFound;

	MainFrame->LatentAction = 0;
	UState* StateNode = NULL;
	FName OldStateName = MainFrame->StateNode!=Class ? MainFrame->StateNode->GetFName() : NAME_None;
	if( NewState != NAME_Auto )
	{
		// Find regular state.
		StateNode = FindState( NewState );
	}
	else
	{
		// Find auto state.
		for( TFieldIterator<UState> It(GetClass()); It && !StateNode; ++It )
			if( It->StateFlags & STATE_Auto )
				StateNode = *It;
	}

	if( !StateNode )
	{
		// Going nowhere.
		NewState  = NAME_None;
		StateNode = GetClass();
	}
	else if( NewState == NAME_Auto )
	{
		// Going to auto state.
		NewState = StateNode->GetFName();
	}

	// Send EndState notification.
	if
	(	OldStateName!=NAME_None
	&&	NewState!=OldStateName
	&&	IsProbing(NAME_EndState) 
	&&	!(GetFlags() & RF_InEndState) )
	{
		ClearFlags( RF_StateChanged );
		SetFlags( RF_InEndState );
		EndState();
		ClearFlags( RF_InEndState );
		if( GetFlags() & RF_StateChanged )
			return GOTOSTATE_Preempted;
	}

	// Go there.
	MainFrame->Node		  = StateNode;
	MainFrame->StateNode  = StateNode;
	MainFrame->Code	      = NULL;
	MainFrame->ProbeMask  = (StateNode->ProbeMask | GetClass()->ProbeMask) & StateNode->IgnoreMask;

	// Send BeginState notification.
	if( NewState!=NAME_None && NewState!=OldStateName && IsProbing(NAME_BeginState) )
	{
		ClearFlags( RF_StateChanged );
		BeginState();
		if( GetFlags() & RF_StateChanged )
			return GOTOSTATE_Preempted;
	}

	// Return result.
	if( NewState != NAME_None )
	{
		SetFlags( RF_StateChanged );
		return GOTOSTATE_Success;
	}
	else return GOTOSTATE_NotFound;

	unguard;
}

//
// Goto a label in the current state.
// Returns 1 if went, 0 if not found.
//
UBOOL UObject::GotoLabel( FName FindLabel )
{
	guard(UObject::GotoLabel);
	if( MainFrame )
	{
		MainFrame->LatentAction = 0;
		if( FindLabel != NAME_None )
		{
			for( UState* SourceState=MainFrame->StateNode; SourceState; SourceState=SourceState->GetSuperState() )
			{
				if( SourceState->LabelTableOffset != MAXWORD )
				{
					for( FLabelEntry* Label = (FLabelEntry *)&SourceState->Script(SourceState->LabelTableOffset); Label->Name!=NAME_None; Label++ )
					{
						if( Label->Name==FindLabel )
						{
							MainFrame->Node = SourceState;
							MainFrame->Code = &SourceState->Script(Label->iCode);
							return 1;
						}
					}
				}
			}
		}
		MainFrame->Code = NULL;
	}
	return 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	Intrinsics.
-----------------------------------------------------------------------------*/

/////////////////////////////////
// Undefined intrinsic handler //
/////////////////////////////////

void UObject::execUndefined( FFrame& Stack, BYTE*& Result  )
{
	guardSlow(UObject::execUndefined);

	Stack.ScriptWarn( 1, "Unknown code token %02X", Stack.Code[-1] );

	unguardexecSlow;
}

///////////////
// Variables //
///////////////

void UObject::execLocalVariable( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLocalVariable);

	debug(Stack.Object==this);
	debug(Stack.Locals!=NULL);
	GProperty = ((UProperty*)Stack.ReadInt());
	Result = Stack.Locals + GProperty->Offset;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_LocalVariable, execLocalVariable );

void UObject::execInstanceVariable( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execInstanceVariable);

	GProperty = (UProperty*)Stack.ReadInt();
	Result = (BYTE*)this + GProperty->Offset;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_InstanceVariable, execInstanceVariable );

void UObject::execDefaultVariable( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execDefaultVariable);

	GProperty = (UProperty*)Stack.ReadInt();
	Result = &GetClass()->Defaults(GProperty->Offset);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_DefaultVariable, execDefaultVariable );

void UObject::execClassContext( FFrame& Stack, BYTE*& Result )
{
	guard(UObject::execClassContext);

	// Get class expression.
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );

	// Execute expression in class context.
	UClass* ClassContext = *(UClass**)Addr;
	if( ClassContext )
	{
		Stack.Code += 3;
		Stack.Step( ClassContext->GetDefaultObject(), Result );
	}
	else
	{
		Stack.ScriptWarn( 0, "Accessed null class context" );
		INT wSkip = Stack.ReadWord();
		BYTE bSize = *Stack.Code++;
		Stack.Code += wSkip;
		if( Result )
			appMemset( Result, 0, bSize );
		GBoolAddr = NULL;
	}

	unguardexec;
}
AUTOREGISTER_INTRINSIC( UObject, EX_ClassContext, execClassContext );

void UObject::execArrayElement( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execArrayElement);

	// Get array index expression.
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );

	// Get base element.
	Stack.Step( this, Result );

	// Bounds check.
	if( *(INT*)Addr>=GProperty->ArrayDim || *(INT*)Addr<0 )
	{
		// Display out-of-bounds warning and continue on with index clamped to valid range.
		Stack.ScriptWarn( 0, "Accessed array out of bounds (%i/%i)", *(INT*)Addr, GProperty->ArrayDim );
		*(INT*)Addr = Clamp( *(INT*)Addr, 0, GProperty->ArrayDim - 1 );
	}

	// Add scaled offset to base pointer.
	if( Result )
		Result += *(INT*)Addr * GProperty->GetElementSize();

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_ArrayElement, execArrayElement );

void UObject::execBoolVariable( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execBoolVariable);

	// Get bool variable.
	GBoolAddr = NULL;
	BYTE B = *Stack.Code++;
	UBoolProperty* Property = *(UBoolProperty**)Stack.Code;
	(this->*GIntrinsics[B])( Stack, *(BYTE**)&GBoolAddr );
	GProperty = Property;

	// Note that we're not returning an in-place pointer to to the bool, so EX_Let 
	// must take special precautions with bools.does
	if( Result )
		*(DWORD*)Result = (*GBoolAddr & ((UBoolProperty*)GProperty)->BitMask) ? 1 : 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_BoolVariable, execBoolVariable );

/////////////
// Nothing //
/////////////

void UObject::execNothing( FFrame& Stack, BYTE*& Result )
{
	// Do nothing.
}
AUTOREGISTER_INTRINSIC( UObject, EX_Nothing, execNothing );

void UObject::execIntrinsicParm( FFrame& Stack, BYTE*& Result )
{
	Result = Stack.Locals + ((UProperty*)Stack.ReadInt())->Offset;
}
AUTOREGISTER_INTRINSIC( UObject, EX_IntrinsicParm, execIntrinsicParm );

void UObject::execEndFunctionParms( FFrame& Stack, BYTE*& Result )
{
	// For skipping over optional function parms without values specified.
	Stack.Code--;
}
AUTOREGISTER_INTRINSIC( UObject, EX_EndFunctionParms, execEndFunctionParms );

//////////////
// Commands //
//////////////

void UObject::execStop( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execStop);
	Stack.Code = NULL;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_Stop, execStop );

void UObject::execEndCode( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execEndCode);
	Stack.Code = NULL;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_EndCode, execEndCode );

void UObject::execSwitch( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSwitch);

	// Get switch size.
	BYTE bSize = *Stack.Code++;

	// Get switch expression.
	BYTE SwitchBuffer[MAX_CONST_SIZE], *SwitchVal=SwitchBuffer;
	Stack.Step( Stack.Object, SwitchVal );

	// Check each case clause till we find a match.
	for( ; ; )
	{
		// Skip over case token.
		debug(*Stack.Code==EX_Case);
		Stack.Code++;

		// Get address of next handler.
		INT wNext = Stack.ReadWord();
		if( wNext == MAXWORD ) // Default case or end of cases.
			break;

		// Get case expression.
		BYTE Buffer[MAX_STRING_CONST_SIZE], *Val=Buffer;
		Stack.Step( Stack.Object, Val );

		// Compare.
		if( (bSize ? appMemcmp(SwitchVal,Val,bSize) : appStrcmp((char*)SwitchVal,(char*)Val) )==0 )
			break;

		// Jump to next handler.
		Stack.Code = &Stack.Node->Script(wNext);
	}
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_Switch, execSwitch );

void UObject::execCase( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execCase);

	// Get address of next handler.
	INT wNext = Stack.ReadWord();
	if( wNext != MAXWORD )
	{
		BYTE Buffer[MAX_STRING_CONST_SIZE], *Val=Buffer;
		Stack.Step( Stack.Object, Val );
	}
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_Case, execCase );

void UObject::execJump( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execJump);
	CHECK_RUNAWAY;

	// Jump immediate.
	Stack.Code = &Stack.Node->Script(Stack.ReadWord() );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_Jump, execJump );

void UObject::execJumpIfNot( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execJumpIfNot);
	CHECK_RUNAWAY;

	// Get code offset.
	INT wOffset = Stack.ReadWord();

	// Get boolean test value.
	BYTE Buffer[MAX_CONST_SIZE], *Val=Buffer;
	Stack.Step( Stack.Object, Val );

	// Jump if false.
	if( !*(DWORD*)Val )
		Stack.Code = &Stack.Node->Script( wOffset );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_JumpIfNot, execJumpIfNot );

void UObject::execAssert( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAssert);

	// Get line number.
	INT wLine = Stack.ReadWord();

	// Get boolean assert value.
	BYTE Buffer[MAX_CONST_SIZE], *Val=Buffer;
	Stack.Step( Stack.Object, Val );

	// Check it.
	if( !*(DWORD*)Val )
		Stack.ScriptWarn( 1, "Assertion failed, line %i", wLine );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_Assert, execAssert );

void UObject::execGotoLabel( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGotoLabel);

	P_GET_NAME(N);
	if( !GotoLabel( N ) )
		Stack.ScriptWarn( 0, "GotoLabel (%s): Label not found", N );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_GotoLabel, execGotoLabel );

////////////////
// Assignment //
////////////////

void UObject::execLet( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLet);

	// Get variable address.
	BYTE* Var=NULL;
	Stack.Step( Stack.Object, Var );
	GProperty->ExecLet( Var, Stack );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_Let, execLet );

/////////////////////////
// Context expressions //
/////////////////////////

void UObject::execSelf( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSelf);

	// Get Self actor for this context.
	*(UObject**)Result = this;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_Self, execSelf );

void UObject::execContext( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execContext);

	// Get actor variable.
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( this, Addr );

	// Execute or skip the following expression in the actor's context.
	UObject* NewContext = *(UObject**)Addr;
	if( NewContext != NULL )
	{
		Stack.Code += 3;
		Stack.Step( NewContext, Result );
	}
	else
	{
		Stack.ScriptWarn( 0, "Accessed None" );
		INT wSkip = Stack.ReadWord();
		BYTE bSize = *Stack.Code++;
		Stack.Code += wSkip;
		if( Result )
			appMemset( Result, 0, bSize );
		GBoolAddr = NULL;
	}
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_Context, execContext );

////////////////////
// Function calls //
////////////////////

void UObject::execVirtualFunction( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execVirtualFunction);

	// Call the virtual function.
	CallFunction( Stack, Result, FindFunctionChecked(Stack.ReadName()) );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_VirtualFunction, execVirtualFunction );

void UObject::execFinalFunction( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execFinalFunction);

	// Call the final function.
	CallFunction( Stack, Result, (UFunction*)Stack.ReadInt() );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_FinalFunction, execFinalFunction );

void UObject::execGlobalFunction( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGlobalFunction);

	// Call global version of virtual function.
	CallFunction( Stack, Result, FindFunctionChecked(Stack.ReadName(),1) );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_GlobalFunction, execGlobalFunction );

///////////////////////
// Struct comparison //
///////////////////////

void UObject::execStructCmpEq( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execStructCmpEq);

	// Get struct.
	UStruct* Struct = *(UStruct**)Stack.Code;
	Stack.Code += sizeof(UStruct*);

	// Get first expression.
	BYTE Buffer1[255], *Addr1=Buffer1;
	Stack.Step( this, Addr1 );

	// Get second expression.
	BYTE Buffer2[255], *Addr2=Buffer2;
	Stack.Step( this, Addr2 );

	// Compare.
	*(DWORD*)Result = Struct->StructCompare(Addr1,Addr2);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_StructCmpEq, execStructCmpEq );

void UObject::execStructCmpNe( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execStructCmpNe);

	// Get struct.
	UStruct* Struct = *(UStruct**)Stack.Code;
	Stack.Code += sizeof(UStruct*);

	// Get first expression.
	BYTE Buffer1[MAX_STRING_CONST_SIZE], *Addr1=Buffer1;
	Stack.Step( this, Addr1 );

	// Get second expression.
	BYTE Buffer2[MAX_STRING_CONST_SIZE], *Addr2=Buffer2;
	Stack.Step( this, Addr2 );

	// Compare.
	*(DWORD*)Result = !Struct->StructCompare(Addr1,Addr2);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_StructCmpNe, execStructCmpNe );

void UObject::execStructMember( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execStructMember);

	// Get structure element.
	UProperty* Property = (UProperty*)Stack.ReadInt();

	// Get struct expression.
	BYTE Buffer[255], *Addr = Result ? Buffer : NULL;
	Stack.Step( this, Addr );

	// Set result.
	Result = Addr ? Addr+Property->Offset : NULL;
	GProperty = Property;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_StructMember, execStructMember );

///////////////
// Constants //
///////////////

void UObject::execIntConst( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execIntConst);
	*(INT*)Result = Stack.ReadInt();
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_IntConst, execIntConst );

void UObject::execFloatConst( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execFloatConst);
	*(FLOAT*)Result = Stack.ReadFloat();
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_FloatConst, execFloatConst );

void UObject::execStringConst( FFrame& Stack, BYTE*& Result )
{
	// Safe because the caller won't overwrite Result (==Stack.Code).
	guardSlow(UObject::execStringConst);
	Result      = Stack.Code;
	Stack.Code += appStrlen((char*)Stack.Code)+1;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_StringConst, execStringConst );

void UObject::execObjectConst( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execObjectConst);
	*(UObject**)Result = (UObject*)Stack.ReadInt();
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_ObjectConst, execObjectConst );

void UObject::execNameConst( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execNameConst);
	*(FName*)Result = Stack.ReadName();
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_NameConst, execNameConst );

void UObject::execByteConst( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execByteConst);
	*(BYTE*)Result = *Stack.Code++;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_ByteConst, execByteConst );

void UObject::execIntZero( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execIntZero);
	*(INT*)Result = 0;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_IntZero, execIntZero );

void UObject::execIntOne( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execIntOne);
	*(INT*)Result = 1;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_IntOne, execIntOne );

void UObject::execTrue( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execTrue);
	*(INT*)Result = 1;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_True, execTrue );

void UObject::execFalse( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execFalse);
	*(DWORD*)Result = 0;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_False, execFalse );

void UObject::execNoObject( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execNoObject);
	*(UObject**)Result = NULL;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_NoObject, execNoObject );

void UObject::execIntConstByte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execIntConstByte);
	*(INT*)Result = *Stack.Code++;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_IntConstByte, execIntConstByte );

/////////////////
// Conversions //
/////////////////

void UObject::execResizeString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execResizeString);

	// Get new length.
	BYTE NewLength = *Stack.Code++;
	debug(NewLength>0);

	// Get copy of string expression to convert.
	BYTE* Addr = Result;
	Stack.Step( Stack.Object, Addr );
	if( Addr != Result )
		appStrncpy( (char*)Result, (char*)Addr, NewLength );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_ResizeString, execResizeString );

void UObject::execDynamicCast( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execDynamicCast);

	// Get destination class of dynamic actor class.
	UClass* Class = (UClass *)Stack.ReadInt();

	// Compile actor expression.
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );

	UObject* Castee = *(UObject**)Addr;
	*(UObject**)Result = (Castee && Castee->IsA(Class)) ? Castee : NULL;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_DynamicCast, execDynamicCast );

void UObject::execMetaCast( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execMetaCast);

	// Get destination class of dynamic actor class.
	UClass* MetaClass = (UClass*)Stack.ReadInt();

	// Compile actor expression.
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );

	UObject* Castee = *(UObject**)Addr;
	*(UObject**)Result = (Castee && Castee->IsA(UClass::StaticClass) && ((UClass*)Castee)->IsChildOf(MetaClass)) ? Castee : NULL;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_MetaCast, execMetaCast );

void UObject::execByteToInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execByteToInt);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(INT*)Result = *(BYTE*)Addr;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_ByteToInt, execByteToInt );

void UObject::execByteToBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execByteToBool);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(DWORD*)Result = *(BYTE*)Addr ? 1 : 0;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_ByteToBool, execByteToBool );

void UObject::execByteToFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execByteToFloat);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(FLOAT*)Result = *(BYTE*)Addr;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_ByteToFloat, execByteToFloat );

void UObject::execByteToString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execByteToString);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	appSprintf( (char*)Result, "%i", *(BYTE*)Addr );
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_ByteToString, execByteToString );

void UObject::execIntToByte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execIntToByte);
	BYTE Buffer[MAX_CONST_SIZE];
	Result = Buffer;
	Stack.Step( Stack.Object, Result );
	*(BYTE*)Result = *(INT*)Result;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_IntToByte, execIntToByte );

void UObject::execIntToBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execIntToBool);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(INT*)Result = *(INT*)Addr ? 1 : 0;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_IntToBool, execIntToBool );

void UObject::execIntToFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execIntToFloat);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(FLOAT*)Result = *(INT*)Addr;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_IntToFloat, execIntToFloat );

void UObject::execIntToString( FFrame& Stack, BYTE*& Result )
{
	// Safe because integers can't overflow maximum size of 16 characters.
	guardSlow(UObject::execIntToString);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	appSprintf( (char*)Result, "%i", *(INT*)Addr );
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_IntToString, execIntToString );

void UObject::execBoolToByte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execBoolToByte);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(BYTE*)Result = *(DWORD*)Addr & 1;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_BoolToByte, execBoolToByte );

void UObject::execBoolToInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execBoolToInt);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(INT*)Result = *(DWORD*)Addr & 1;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_BoolToInt, execBoolToInt );

void UObject::execBoolToFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execBoolToFloat);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(FLOAT*)Result = *(DWORD*)Addr & 1;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_BoolToFloat, execBoolToFloat );

void UObject::execBoolToString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execBoolToString);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	appStrcpy( (char*)Result, (*(DWORD*)Addr&1) ? "True" : "False" );
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_BoolToString, execBoolToString );

void UObject::execFloatToByte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execFloatToByte);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(BYTE*)Result = *(FLOAT*)Addr;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_FloatToByte, execFloatToByte );

void UObject::execFloatToInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execFloatToInt);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(INT*)Result = *(FLOAT*)Addr;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_FloatToInt, execFloatToInt );

void UObject::execFloatToBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execFloatToBool);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(DWORD*)Result = *(FLOAT*)Addr!=0.0 ? 1 : 0;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_FloatToBool, execFloatToBool );

void UObject::execFloatToString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execFloatToString);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	appSprintf((char*)Result,"%.2f",*(FLOAT*)Addr);
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_FloatToString, execFloatToString );

void UObject::execObjectToBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execObjectToBool);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(DWORD*)Result = *(UObject**)Addr!=NULL;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_ObjectToBool, execObjectToBool );

void UObject::execObjectToString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execObjectToString);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	appStrcpy((char*)Result, *(UObject**)Addr ? (*(UObject**)Addr)->GetName() : "None");
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_ObjectToString, execObjectToString );

void UObject::execNameToBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execNameToBool);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(DWORD*)Result = *(FName*)Addr!=NAME_None ? 1 : 0;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_NameToBool, execNameToBool );

void UObject::execNameToString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execNameToString);
	BYTE Buffer[MAX_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	appStrcpy( (char*)Result, **(FName*)Addr );
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_NameToString, execNameToString );

void UObject::execStringToByte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execStringToByte);
	BYTE Buffer[MAX_STRING_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(BYTE*)Result = appAtoi((char*)Addr);
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_StringToByte, execStringToByte );

void UObject::execStringToInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execStringToInt);
	BYTE Buffer[MAX_STRING_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(INT*)Result = appAtoi((char*)Addr);
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_StringToInt, execStringToInt );

void UObject::execStringToBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execStringToBool);
	BYTE Buffer[MAX_STRING_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	if     ( appStricmp((char*)Addr,"True" ) == 0 ) *(INT*)Result = 1;
	else if( appStricmp((char*)Addr,"False") == 0 ) *(INT*)Result = 0;
	else                                            *(INT*)Result = appAtoi((char*)Addr) ? 1 : 0;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_StringToBool, execStringToBool );

void UObject::execStringToFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execStringToFloat);
	BYTE Buffer[MAX_STRING_CONST_SIZE], *Addr=Buffer;
	Stack.Step( Stack.Object, Addr );
	*(FLOAT*)Result = appAtof((char*)Addr);
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, EX_StringToFloat, execStringToFloat );

////////////////////////////////////////////
// Intrinsic bool operators and functions //
////////////////////////////////////////////

void UObject::execNot_PreBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execNot_PreBool);

	P_GET_UBOOL(A);
	P_FINISH;

	*(DWORD*)Result = !A;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 129, execNot_PreBool );

void UObject::execEqualEqual_BoolBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execEqualEqual_BoolBool);

	P_GET_UBOOL(A);
	P_GET_UBOOL(B);
	P_FINISH;

	*(DWORD*)Result = ((!A) == (!B));
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 242, execEqualEqual_BoolBool );

void UObject::execNotEqual_BoolBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execNotEqual_BoolBool);

	P_GET_UBOOL(A);
	P_GET_UBOOL(B);
	P_FINISH;

	*(DWORD*)Result = ((!A) != (!B));

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 243, execNotEqual_BoolBool );

void UObject::execAndAnd_BoolBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAndAnd_BoolBool);

	P_GET_UBOOL(A);
	P_GET_SKIP_OFFSET(W);

	if( A )
	{
		P_GET_UBOOL(B);
		*(DWORD*)Result = A && B;
		P_FINISH;
	}
	else
	{
		*(DWORD*)Result = 0;
		Stack.Code += W;
	}
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 130, execAndAnd_BoolBool );

void UObject::execXorXor_BoolBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execXorXor_BoolBool);

	P_GET_UBOOL(A);
	P_GET_UBOOL(B);
	P_FINISH;

	*(DWORD*)Result = !A ^ !B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 131, execXorXor_BoolBool );

void UObject::execOrOr_BoolBool( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execOrOr_BoolBool);
	P_GET_UBOOL(A);
	P_GET_SKIP_OFFSET(W);
	if( !A )
	{
		P_GET_UBOOL(B);
		*(DWORD*)Result = A || B;
		P_FINISH;
	}
	else
	{
		*(DWORD*)Result = 1;
		Stack.Code += W;
	}
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 132, execOrOr_BoolBool );

////////////////////////////////////////////
// Intrinsic byte operators and functions //
////////////////////////////////////////////

void UObject::execMultiplyEqual_ByteByte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execMultiplyEqual_ByteByte);

	P_GET_BYTE_REF(A);
	P_GET_BYTE(B);
	P_FINISH;

	*(BYTE*)Result = (*A *= B);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 133, execMultiplyEqual_ByteByte );

void UObject::execDivideEqual_ByteByte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execDivideEqual_ByteByte);

	P_GET_BYTE_REF(A);
	P_GET_BYTE(B);
	P_FINISH;

	*(BYTE*)Result = B ? (*A /= B) : 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 134, execDivideEqual_ByteByte );

void UObject::execAddEqual_ByteByte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAddEqual_ByteByte);

	P_GET_BYTE_REF(A);
	P_GET_BYTE(B);
	P_FINISH;

	*(BYTE*)Result = (*A += B);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 135, execAddEqual_ByteByte );

void UObject::execSubtractEqual_ByteByte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSubtractEqual_ByteByte);

	P_GET_BYTE_REF(A);
	P_GET_BYTE(B);
	P_FINISH;

	*(BYTE*)Result = (*A -= B);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 136, execSubtractEqual_ByteByte );

void UObject::execAddAdd_PreByte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAddAdd_PreByte);

	P_GET_BYTE_REF(A);
	P_FINISH;

	*(BYTE*)Result = ++(*A);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 137, execAddAdd_PreByte );

void UObject::execSubtractSubtract_PreByte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSubtractSubtract_PreByte);

	P_GET_BYTE_REF(A);
	P_FINISH;

	*(BYTE*)Result = --(*A);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 138, execSubtractSubtract_PreByte );

void UObject::execAddAdd_Byte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAddAdd_Byte);

	P_GET_BYTE_REF(A);
	P_FINISH;

	*(BYTE*)Result = (*A)++;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 139, execAddAdd_Byte );

void UObject::execSubtractSubtract_Byte( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSubtractSubtract_Byte);

	P_GET_BYTE_REF(A);
	P_FINISH;

	*(BYTE*)Result = (*A)--;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 140, execSubtractSubtract_Byte );

/////////////////////////////////
// Int operators and functions //
/////////////////////////////////

void UObject::execComplement_PreInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execComplement_PreInt);

	P_GET_INT(A);
	P_FINISH;

	*(INT*)Result = ~A;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 141, execComplement_PreInt );

void UObject::execSubtract_PreInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSubtract_PreInt);

	P_GET_INT(A);
	P_FINISH;

	*(INT*)Result = -A;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 143, execSubtract_PreInt );

void UObject::execMultiply_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execMultiply_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A * B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 144, execMultiply_IntInt );

void UObject::execDivide_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execDivide_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = B ? A / B : 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 145, execDivide_IntInt );

void UObject::execAdd_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAdd_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A + B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 146, execAdd_IntInt );

void UObject::execSubtract_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSubtract_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A - B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 147, execSubtract_IntInt );

void UObject::execLessLess_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLessLess_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A << B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 148, execLessLess_IntInt );

void UObject::execGreaterGreater_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGreaterGreater_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A >> B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 149, execGreaterGreater_IntInt );

void UObject::execLess_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLess_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A < B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 150, execLess_IntInt );

void UObject::execGreater_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGreater_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A > B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 151, execGreater_IntInt );

void UObject::execLessEqual_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLessEqual_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A <= B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 152, execLessEqual_IntInt );

void UObject::execGreaterEqual_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGreaterEqual_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A >= B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 153, execGreaterEqual_IntInt );

void UObject::execEqualEqual_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execEqualEqual_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A == B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 154, execEqualEqual_IntInt );

void UObject::execNotEqual_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execNotEqual_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(DWORD*)Result = A != B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 155, execNotEqual_IntInt );

void UObject::execAnd_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAnd_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A & B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 156, execAnd_IntInt );

void UObject::execXor_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execXor_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A ^ B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 157, execXor_IntInt );

void UObject::execOr_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execOr_IntInt);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = A | B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 158, execOr_IntInt );

void UObject::execMultiplyEqual_IntFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execMultiplyEqual_IntInt);

	P_GET_INT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(INT*)Result = (*A *= B);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 159, execMultiplyEqual_IntFloat );

void UObject::execDivideEqual_IntFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execDivideEqual_IntInt);

	P_GET_INT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(INT*)Result = (B ? *A /= B : 0);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 160, execDivideEqual_IntFloat );

void UObject::execAddEqual_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAddEqual_IntInt);

	P_GET_INT_REF(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = (*A += B);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 161, execAddEqual_IntInt );

void UObject::execSubtractEqual_IntInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSubtractEqual_IntInt);

	P_GET_INT_REF(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = (*A -= B);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 162, execSubtractEqual_IntInt );

void UObject::execAddAdd_PreInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAddAdd_PreInt);

	P_GET_INT_REF(A);
	P_FINISH;

	*(INT*)Result = ++(*A);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 163, execAddAdd_PreInt );

void UObject::execSubtractSubtract_PreInt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSubtractSubtract_PreInt);

	P_GET_INT_REF(A);
	P_FINISH;

	*(INT*)Result = --(*A);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC(UObject,  164, execSubtractSubtract_PreInt );

void UObject::execAddAdd_Int( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAddAdd_Int);

	P_GET_INT_REF(A);
	P_FINISH;

	*(INT*)Result = (*A)++;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 165, execAddAdd_Int );

void UObject::execSubtractSubtract_Int( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSubtractSubtract_Int);

	P_GET_INT_REF(A);
	P_FINISH;

	*(INT*)Result = (*A)--;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 166, execSubtractSubtract_Int );

void UObject::execRand( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execRand);

	P_GET_INT(A);
	P_FINISH;

	*(INT*)Result = A>0 ? (appRand() % A) : 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 167, execRand );

void UObject::execMin( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execMin);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = Min(A,B);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 249, execMin );

void UObject::execMax( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execMax);

	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = Max(A,B);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 250, execMax );

void UObject::execClamp( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execClamp);

	P_GET_INT(V);
	P_GET_INT(A);
	P_GET_INT(B);
	P_FINISH;

	*(INT*)Result = Clamp(V,A,B);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 251, execClamp );

///////////////////////////////////
// Float operators and functions //
///////////////////////////////////

void UObject::execSubtract_PreFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSubtract_PreFloat);

	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = -A;

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 169, execSubtract_PreFloat );

void UObject::execMultiplyMultiply_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execMultiplyMultiply_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = appPow(A,B);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 170, execMultiplyMultiply_FloatFloat );

void UObject::execMultiply_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execMultiply_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = A * B;

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 171, execMultiply_FloatFloat );

void UObject::execDivide_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execDivide_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = A / B;

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 172, execDivide_FloatFloat );

void UObject::execPercent_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execPercent_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = appFmod(A,B);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 173, execPercent_FloatFloat );

void UObject::execAdd_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAdd_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = A + B;

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 174, execAdd_FloatFloat );

void UObject::execSubtract_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSubtract_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = A - B;

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 175, execSubtract_FloatFloat );

void UObject::execLess_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLess_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A < B;

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 176, execLess_FloatFloat );

void UObject::execGreater_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGreater_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A > B;

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 177, execGreater_FloatFloat );

void UObject::execLessEqual_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLessEqual_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A <= B;

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 178, execLessEqual_FloatFloat );

void UObject::execGreaterEqual_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGreaterEqual_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A >= B;

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 179, execGreaterEqual_FloatFloat );

void UObject::execEqualEqual_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execEqualEqual_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A == B;

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 180, execEqualEqual_FloatFloat );

void UObject::execNotEqual_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execNotEqual_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = A != B;

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 181, execNotEqual_FloatFloat );

void UObject::execComplementEqual_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execComplementEqual_FloatFloat);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(DWORD*)Result = Abs(A - B) < (1.e-4);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 210, execComplementEqual_FloatFloat );

void UObject::execMultiplyEqual_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execMultiplyEqual_FloatFloat);

	P_GET_FLOAT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = (*A *= B);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 182, execMultiplyEqual_FloatFloat );

void UObject::execDivideEqual_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execDivideEqual_FloatFloat);

	P_GET_FLOAT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = (*A /= B);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 183, execDivideEqual_FloatFloat );

void UObject::execAddEqual_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAddEqual_FloatFloat);

	P_GET_FLOAT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = (*A += B);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 184, execAddEqual_FloatFloat );

void UObject::execSubtractEqual_FloatFloat( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSubtractEqual_FloatFloat);

	P_GET_FLOAT_REF(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = (*A -= B);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 185, execSubtractEqual_FloatFloat );

void UObject::execAbs( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAbs);

	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = Abs(A);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 186, execAbs );

void UObject::execSin( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSin);

	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appSin(A);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 187, execSin );

void UObject::execCos( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execCos);

	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appCos(A);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 188, execCos );

void UObject::execTan( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execTan);

	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appTan(A);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 189, execTan );

void UObject::execAtan( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAtan);

	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appAtan(A);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 190, execAtan );

void UObject::execExp( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execExp);

	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appExp(A);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 191, execExp );

void UObject::execLoge( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLoge);

	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appLoge(A);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 192, execLoge );

void UObject::execSqrt( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSqrt);

	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = appSqrt(A);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 193, execSqrt );

void UObject::execSquare( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSquare);

	P_GET_FLOAT(A);
	P_FINISH;

	*(FLOAT*)Result = Square(A);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 194, execSquare );

void UObject::execFRand( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execFRand);

	P_FINISH;

	*(FLOAT*)Result = appFrand();

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 195, execFRand );

void UObject::execFMin( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execFMin);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = Min(A,B);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 244, execFMin );

void UObject::execFMax( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execFMax);

	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = Max(A,B);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 245, execFMax );

void UObject::execFClamp( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execFClamp);

	P_GET_FLOAT(V);
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = Clamp(V,A,B);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 246, execFClamp );

void UObject::execLerp( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLerp);

	P_GET_FLOAT(V);
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = A + V*(B-A);

	unguardexecSlow;
}	
AUTOREGISTER_INTRINSIC( UObject, 247, execLerp );

void UObject::execSmerp( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSmerp);

	P_GET_FLOAT(V);
	P_GET_FLOAT(A);
	P_GET_FLOAT(B);
	P_FINISH;

	*(FLOAT*)Result = A + (3.0*V*V - 2.0*V*V*V)*(B-A);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 248, execSmerp );

////////////////////////////////////
// String operators and functions //
////////////////////////////////////

void UObject::execConcat_StringString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execConcat_StringString);

	P_GET_STRING(A);
	P_GET_STRING(B);
	P_FINISH;

	INT Size = appStrlen(A);
	appStrcpy((char*)Result,A);
	appStrncpy((char*)Result + Size, B, MAX_STRING_CONST_SIZE - Size);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 228, execConcat_StringString );

void UObject::execLess_StringString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLess_StringString);

	P_GET_STRING(A);
	P_GET_STRING(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(A,B) < 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 197, execLess_StringString );

void UObject::execGreater_StringString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGreater_StringString);

	P_GET_STRING(A);
	P_GET_STRING(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(A,B) > 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 198, execGreater_StringString );

void UObject::execLessEqual_StringString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLessEqual_StringString);

	P_GET_STRING(A);
	P_GET_STRING(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(A,B) <= 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 199, execLessEqual_StringString );

void UObject::execGreaterEqual_StringString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGreaterEqual_StringString);

	P_GET_STRING(A);
	P_GET_STRING(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(A,B) >= 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 200, execGreaterEqual_StringString );

void UObject::execEqualEqual_StringString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execEqualEqual_StringString);

	P_GET_STRING(A);
	P_GET_STRING(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(A,B) == 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 201, execEqualEqual_StringString );

void UObject::execNotEqual_StringString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execNotEqual_StringString);

	P_GET_STRING(A);
	P_GET_STRING(B);
	P_FINISH;

	*(DWORD*)Result = appStrcmp(A,B) != 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 202, execNotEqual_StringString );

void UObject::execComplementEqual_StringString( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execComplementEqual_StringString);

	P_GET_STRING(A);
	P_GET_STRING(B);
	P_FINISH;

	*(DWORD*)Result = appStricmp(A,B) == 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 168, execComplementEqual_StringString );

void UObject::execLen( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLen);

	P_GET_STRING(S);
	P_FINISH;

	*(INT*)Result = appStrlen(S);

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 204, execLen );

void UObject::execInStr( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execInStr);

	P_GET_STRING(S);
	P_GET_STRING(A);
	P_FINISH;

	CHAR *Ptr = appStrstr(S,A);
	*(INT*)Result = Ptr ? Ptr - S : -1;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 205, execInStr );

void UObject::execMid( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execMid);

	P_GET_STRING(A);
	P_GET_INT(I);
	P_GET_INT_OPT(C,65535);
	P_FINISH;

	if( I < 0 ) C += I;
	I = Clamp( I, 0, (int)appStrlen(A) );
	C = Clamp( C, 0, (int)appStrlen(A)-I );
	appStrncpy( (char*)Result, A + I, C+1 );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 206, execMid );

void UObject::execLeft( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLeft);

	P_GET_STRING(A);
	P_GET_INT(N);
	P_FINISH;

	N = Clamp( N, 0, appStrlen(A) );
	appStrncpy( (char*)Result, A, N+1 );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 207, execLeft );

void UObject::execRight( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execRight);

	P_GET_STRING(A);
	P_GET_INT(N);
	P_FINISH;

	N = Clamp( appStrlen(A) - N, 0, appStrlen(A) );
	appStrcpy( (char*)Result, A + N );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 208, execRight );

void UObject::execCaps( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execCaps);

	P_GET_STRING(A);
	P_FINISH;

	for( int i=0; A[i]; i++ )
		Result[i] = appToUpper(A[i]);
	Result[i]=0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 209, execCaps );

void UObject::execChr( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execChr);

	P_GET_INT(i);
	P_FINISH;

	((char*)Result)[0] = i;
	((char*)Result)[1] = 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, INDEX_NONE, execChr );

void UObject::execAsc( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execAsc);

	P_GET_STRING(S);
	P_FINISH;

	*(INT*)Result = S[0];	

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, INDEX_NONE, execAsc );

////////////////////////////////////////////
// Intrinsic name operators and functions //
////////////////////////////////////////////

void UObject::execEqualEqual_NameName( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execEqualEqual_NameName);

	P_GET_NAME(A);
	P_GET_NAME(B);
	P_FINISH;

	*(DWORD*)Result = A == B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 254, execEqualEqual_NameName );

void UObject::execNotEqual_NameName( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execNotEqual_NameName);

	P_GET_NAME(A);
	P_GET_NAME(B);
	P_FINISH;

	*(DWORD*)Result = A != B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 255, execNotEqual_NameName );

////////////////////////////////////
// Object operators and functions //
////////////////////////////////////

void UObject::execEqualEqual_ObjectObject( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execEqualEqual_ObjectObject);

	P_GET_OBJECT(UObject,A);
	P_GET_OBJECT(UObject,B);
	P_FINISH;

	*(DWORD*)Result = A == B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 114, execEqualEqual_ObjectObject );

void UObject::execNotEqual_ObjectObject( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execNotEqual_ObjectObject);

	P_GET_OBJECT(UObject,A);
	P_GET_OBJECT(UObject,B);
	P_FINISH;

	*(DWORD*)Result = A != B;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 119, execNotEqual_ObjectObject );

/////////////////////////////
// Log and error functions //
/////////////////////////////

void UObject::execLog( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLog);

	P_GET_STRING(S);
	P_GET_NAME_OPT(N,NAME_ScriptLog);
	P_FINISH;

	debugf( (EName)N.GetIndex(), "%s", S );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 231, execLog );

void UObject::execWarn( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execWarn);

	P_GET_STRING(S);
	P_FINISH;

	Stack.ScriptWarn( 0, "%s", S );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 232, execWarn );

void UObject::execLocalize( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execLocalize);

	P_GET_NAME(SectionName);
	P_GET_NAME(KeyName);
	P_GET_NAME(PackageName);
	P_FINISH;

	appStrcpy( (char*)Result, Localize( *SectionName, *KeyName, *PackageName ) );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, INDEX_NONE, execLocalize );

/////////////////////
// High intrinsics //
/////////////////////

#define HIGH_INTRINSIC(n) \
	void UObject::execHighIntrinsic##n( FFrame& Stack, BYTE*& Result ) \
	{ \
		guardSlow(UObject::execHighIntrinsic##n); \
		BYTE B = *Stack.Code++; \
		(this->*GIntrinsics[ n*0x100 + B ])( Stack, Result ); \
		unguardexecSlow; \
	} \
	AUTOREGISTER_INTRINSIC( UObject, 0x60 + n, execHighIntrinsic##n );

HIGH_INTRINSIC(0);
HIGH_INTRINSIC(1);
HIGH_INTRINSIC(2);
HIGH_INTRINSIC(3);
HIGH_INTRINSIC(4);
HIGH_INTRINSIC(5);
HIGH_INTRINSIC(6);
HIGH_INTRINSIC(7);
HIGH_INTRINSIC(8);
HIGH_INTRINSIC(9);
HIGH_INTRINSIC(10);
HIGH_INTRINSIC(11);
HIGH_INTRINSIC(12);
HIGH_INTRINSIC(13);
HIGH_INTRINSIC(14);
HIGH_INTRINSIC(15);

/////////////////////////////
// Class related functions //
/////////////////////////////

void UObject::execClassIsChildOf( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execClassIsChildOf);

	P_GET_OBJECT(UClass,K);
	P_GET_OBJECT(UClass,C);
	P_FINISH;

	*(DWORD*)Result = (C && K) ? K->IsChildOf(C) : 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 258, execClassIsChildOf );

///////////////////////////////
// State and label functions //
///////////////////////////////

void UObject::execGotoState( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGotoState);

	// Get parameters.
	FName CurrentStateName = MainFrame->StateNode!=Class ? MainFrame->StateNode->GetFName() : NAME_None;
	P_GET_NAME_OPT( S, CurrentStateName );
	P_GET_NAME_OPT( L, NAME_None );
	P_FINISH;

	// Go to the state.
	EGotoState Result = GOTOSTATE_Success;
	if( S!=CurrentStateName )
		Result = GotoState( S );

	// Handle success.
	if( Result==GOTOSTATE_Success )
	{
		// Now go to the label.
		if( !GotoLabel( L==NAME_None ? NAME_Begin : L ) && L!=NAME_None )
			Stack.ScriptWarn( 0, "GotoState (%s %s): Label not found", *S, *L );
	}
	else if( Result==GOTOSTATE_NotFound )
	{
		// Warning.
		if( S!=NAME_None && S!=NAME_Auto )
			Stack.ScriptWarn( 0, "GotoState (%s %s): State not found", *S, *L );
	}
	else
	{
		// Safely preempted by another GotoState.
	}
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 113, execGotoState );

void UObject::execEnable( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execEnable);

	P_GET_NAME(N);
	if( N.GetIndex()>=NAME_PROBEMIN && N.GetIndex()<NAME_PROBEMAX && GetMainFrame() )
	{
		QWORD BaseProbeMask = (GetMainFrame()->StateNode->ProbeMask | GetClass()->ProbeMask) & GetMainFrame()->StateNode->IgnoreMask;
		GetMainFrame()->ProbeMask |= (BaseProbeMask & ((QWORD)1<<(N.GetIndex()-NAME_PROBEMIN)));
	}
	else Stack.ScriptWarn( 0, "Enable: '%s' is not a probe function", *N );
	P_FINISH;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 117, execEnable );

void UObject::execDisable( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execDiable);

	P_GET_NAME(N);
	P_FINISH;

	if( N.GetIndex()>=NAME_PROBEMIN && N.GetIndex()<NAME_PROBEMAX && GetMainFrame() )
		GetMainFrame()->ProbeMask &= ~((QWORD)1<<(N.GetIndex()-NAME_PROBEMIN));
	else
		Stack.ScriptWarn( 0, "Enable: '%s' is not a probe function", *N );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 118, execDisable );

///////////////////
// Property text //
///////////////////

void UObject::execGetPropertyText( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGetPropertyText);

	P_GET_STRING(PropName);
	P_FINISH;

	*(char*)Result = 0;
	for( UField* Field=GetClass()->Children; Field; Field=Field->Next )
		if( appStricmp( Field->GetName(), PropName )==0 )
			break;
	UProperty* Property = Cast<UProperty>( Field );
	if( Property && (Property->GetFlags() & RF_Public) )
		Property->ExportText( 0, (char*)Result, (BYTE*)this, (BYTE*)this, 1 );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, INDEX_NONE, execGetPropertyText );

void UObject::execSetPropertyText( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execSetPropertyText);

	P_GET_STRING(PropName);
	P_GET_STRING(PropValue);
	P_FINISH;
	for( UField* Field=GetClass()->Children; Field; Field=Field->Next )
		if( appStricmp( Field->GetName(), PropName )==0 )
			break;
	UProperty* Property = Cast<UProperty>( Field );
	if
	(	(Property)
	&&	(Property->GetFlags() & RF_Public)
	&&	!(Property->PropertyFlags & CPF_Const) )
		Property->ImportText( PropValue, (BYTE*)this + Property->Offset, 1 );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, INDEX_NONE, execSetPropertyText );

void UObject::execSaveConfig( FFrame& Stack, BYTE*& Result )
{
	guard(UObject::execSaveConfig);
	P_FINISH;
	SaveConfig();
	unguard;
}
AUTOREGISTER_INTRINSIC( UObject, 536, execSaveConfig);

void UObject::execResetConfig( FFrame& Stack, BYTE*& Result )
{
	guard(UObject::execResetConfig);
	P_FINISH;
	GObj.ResetConfig(GetClass());
	unguard;
}
AUTOREGISTER_INTRINSIC( UObject, 543, execResetConfig);

void UObject::execGetEnum( FFrame& Stack, BYTE*& Result )
{
	guard(UObject::execGetEnum);

	P_GET_OBJECT(UObject,E);
	P_GET_INT(i);
	P_FINISH;

	*(FName*)Result = NAME_None;
	if( Cast<UEnum>(E) && i>=0 && i<Cast<UEnum>(E)->Names.Num() )
		*(FName*)Result = Cast<UEnum>(E)->Names(i);

	unguard;
}
AUTOREGISTER_INTRINSIC( UObject, INDEX_NONE, execGetEnum);

void UObject::execDynamicLoadObject( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execDynamicLoadObject);

	P_GET_STRING(Name);
	P_GET_OBJECT(UClass,Class);
	P_FINISH;

	*(UObject**)Result = GObj.LoadObject( Class, NULL, Name, NULL, LOAD_NoWarn | LOAD_KeepImports, NULL );

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, INDEX_NONE, execDynamicLoadObject );

void UObject::execIsInState( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execIsInState);

	P_GET_NAME(StateName);
	P_FINISH;

	if( MainFrame )
		for( UState* Test=MainFrame->StateNode; Test; Test=Test->GetSuperState() )
			if( Test->GetFName()==StateName )
				{*(DWORD*)Result=1; return;}
	*(DWORD*)Result = 0;

	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 281, execIsInState );

void UObject::execGetStateName( FFrame& Stack, BYTE*& Result )
{
	guardSlow(UObject::execGetStateName);
	P_FINISH;
	*(FName*)Result = (MainFrame && MainFrame->StateNode) ? MainFrame->StateNode->GetFName() : NAME_None;
	unguardexecSlow;
}
AUTOREGISTER_INTRINSIC( UObject, 284, execGetStateName );

/*-----------------------------------------------------------------------------
	Intrinsic iterator functions.
-----------------------------------------------------------------------------*/

void UObject::execIterator( FFrame& Stack, BYTE*& Result )
{}
AUTOREGISTER_INTRINSIC( UObject, EX_Iterator, execIterator );

/*-----------------------------------------------------------------------------
	Intrinsic registry.
-----------------------------------------------------------------------------*/

//
// Register an intrinsic function.
//warning: Called at startup time, before engine initialization.
//
BYTE CORE_API GRegisterIntrinsic( int iIntrinsic, void* Func )
{
	static int Initialized = 0;
	if( !Initialized )
	{
		Initialized = 1;
		for( int i=0; i<ARRAY_COUNT(GIntrinsics); i++ )
			GIntrinsics[i] = &UObject::execUndefined;
	}
	if( iIntrinsic != INDEX_NONE )
	{
		if( iIntrinsic<0 || iIntrinsic>ARRAY_COUNT(GIntrinsics) || GIntrinsics[iIntrinsic]!=&UObject::execUndefined) 
			GIntrinsicDuplicate = iIntrinsic;
		*(void**)&GIntrinsics[iIntrinsic] = Func;
	}
	return 0;
}

/*-----------------------------------------------------------------------------
	Script processing function.
-----------------------------------------------------------------------------*/

//
// Call a function.
//
void UObject::CallFunction( FFrame& Stack, BYTE*& Result, UFunction* Function )
{
	guardSlow(UObject::CallFunction);
#if DO_SLOW_GUARD
	DWORD Cycles=0; clock(Cycles);
#endif

	// Found it.
	if( Function->iIntrinsic )
	{
		// Call intrinsic final function.
		(this->*Function->Func)( Stack, Result );
	}
	else if( Function->FunctionFlags & FUNC_Intrinsic )
	{
		// Call intrinsic networkable function.
		BYTE Buffer[1024], *Addr;
		if( !ProcessRemoteFunction( Function, Buffer, &Stack ) )
		{
			// Call regular intrinsic.
			(this->*Function->Func)( Stack, Result );
		}
		else
		{
			// Eat up the remaining parameters in the stream.
			while( *Stack.Code != EX_EndFunctionParms )
				Stack.Step( Stack.Object, Addr=Buffer );
			Stack.Code++;
		}
	}
	else
	{
		// Make new stack frame in the current context.
		FMemMark Mark(GMem);
		FFrame NewStack( this, Function, 0, NewZeroed<BYTE>(GMem,Function->GetPropertiesSize()) );
		debug(*NewStack.Code==EX_BeginFunction);
		NewStack.Code++;
		BYTE* Dest = NewStack.Locals;
		FOutParmRec Outs[MAX_FUNC_PARMS], *Out = Outs;
		while( (Out->Size = *NewStack.Code++) != 0 )
		{
			debug(*NewStack.Code==0 || *NewStack.Code==1);
			Out->Src = Out->Dest = Dest;
			Stack.Step( Stack.Object, Out->Dest );
			if( Out->Dest != Dest )
				appMemcpy( Dest, Out->Dest, Out->Size );
			Dest += Out->Size;
			Out  += *NewStack.Code++;
		}
		debug(*Stack.Code==EX_EndFunctionParms);
		Stack.Code++;

		// Execute the code.
		ProcessInternal( NewStack );

		// Copy back outparms.
		while( --Out >= Outs )
			appMemcpy( Out->Dest, Out->Src, Out->Size );

		// Snag return offset and finish.
		Result = &NewStack.Locals[Function->ReturnValueOffset];

		// Release temp memory.
		Mark.Pop();
	}
#if DO_SLOW_GUARD
	unclock(Cycles);
	Function->Cycles += Cycles;
	Function->Calls++;
#endif
	unguardSlow;
}

//
// Internal function call processing.
//
void UObject::ProcessInternal( FFrame& Stack )
{
	guardSlow(UObject::ProcessInternal);
	if
	(	!ProcessRemoteFunction( (UFunction*)Stack.Node, Stack.Locals, NULL )
	&&	IsProbing( Stack.Node->GetFName() ) )
	{
		// Local call.
		if( !(((UFunction*)Stack.Node)->FunctionFlags & FUNC_Singular) )
		{
			BYTE Buffer[MAX_STRING_CONST_SIZE], *Addr;
			while( *Stack.Code != EX_Return )
				Stack.Step( Stack.Object, Addr=Buffer );
		}
		else if( !(GetFlags() & RF_InSingularFunc) )
		{
			SetFlags( RF_InSingularFunc );
			BYTE Buffer[MAX_STRING_CONST_SIZE], *Addr;
			while( *Stack.Code != EX_Return )
				Stack.Step( Stack.Object, Addr=Buffer );
			ClearFlags( RF_InSingularFunc );
		}
	}
	unguardSlow;
}

//
// Script processing function.
//
void UObject::ProcessEvent( UFunction* Function, void* Parms )
{
	guard(UObject::ProcessEvent);

	// Reject.
	if
	(	GIsEditor
	||	!IsProbing( Function->GetFName() )
	||	IsPendingKill()
	||	Function->iIntrinsic )
		return;

	// Checks.
	debug(Function->ParmsSize==0 || Parms!=NULL);
	if( ++GScriptEntryTag == 1 )
		clock(GScriptCycles);

	// Call the function.
	if
	(	!ProcessRemoteFunction( Function, Parms, NULL )
	&&	(Function->FunctionFlags & (FUNC_Defined|FUNC_Intrinsic)) )
	{
		// Create a new local execution stack.
		FMemMark Mark(GMem);
		FFrame NewStack( this, Function, 0, new(GMem,MEM_Zeroed,Function->GetPropertiesSize())BYTE );
		appMemcpy( NewStack.Locals, Parms, Function->ParmsSize );
		if( !(Function->FunctionFlags & FUNC_Intrinsic) )
		{
			// Skip the parm info in the script code.
			debug(*NewStack.Code==EX_BeginFunction);
			NewStack.Code++;
			while( *NewStack.Code++ != 0 )
				NewStack.Code++;
			ProcessInternal( NewStack );
		}
		else
		{
			// Call intrinsic function and copy the return value back.
			BYTE* Result = NewStack.Locals+Function->ReturnValueOffset, *Dest=Result;
			(this->*Function->Func)( NewStack, Result );
			if( Result != Dest )
				appMemcpy( Dest, Result, Function->ParmsSize - Function->ReturnValueOffset );
		}

		// Copy everything back.
		appMemcpy( Parms, NewStack.Locals, Function->ParmsSize );

		// Restore locals bin.
		Mark.Pop();
	}
	if( --GScriptEntryTag == 0 )
		unclock(GScriptCycles);
	unguardf(( "(%s, %s)", GetFullName(), Function->GetFullName() ));
}

//
// Execute the state code of the object.
//
void UObject::ProcessState( FLOAT DeltaSeconds )
{
}

//
// Process a remote function; returns 1 if remote, 0 if local.
//
UBOOL UObject::ProcessRemoteFunction( UFunction* Function, void* Parms, FFrame* Stack )
{
	return 0;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

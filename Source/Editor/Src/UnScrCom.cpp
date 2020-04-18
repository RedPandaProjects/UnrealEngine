/*=============================================================================
	UnScrCom.cpp: UnrealScript compiler.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"
#include "UnScrCom.h"

/*-----------------------------------------------------------------------------
	Constants & declarations.
-----------------------------------------------------------------------------*/

enum {MAX_VARIABLE_DATA_SIZE=4096};
enum {MAX_ARRAY_SIZE=255};

static int GCheckIntrinsics=1;

/*-----------------------------------------------------------------------------
	Transaction tracking.
-----------------------------------------------------------------------------*/

class FTransaction
{
public:
	class FObjectRecord
	{
	public:
		TArray<BYTE> Data, Defaults;
		UBOOL Restored;
		FObjectRecord( FTransaction& InOwner, UObject* InObject )
		:	Object		( InObject )
		,	Restored	( 0 )
		,	Owner		( InOwner )
		{
			guard(FObjectRecord::FObjectRecord);
			FWriter Writer( InOwner, Data );
			Object->Serialize( Writer );
			unguardf(( "(%s)", Object->GetFullName() ));
		}
		void Restore()
		{
			guard(FObjectRecord::Restore);
			if( !Restored )
			{
				Restored = 1;
				FReader Reader( Owner, Data );
				Object->Serialize( Reader );
			}
			unguardf(( "(%s)", Object->GetFullName() ));
		}
		class FReader : public FArchive
		{
		public:
			FReader( FTransaction& InOwner, TArray<BYTE>& InBytes )
			:	Bytes	( InBytes )
			,	Offset	( 0 )
			,	Owner	( InOwner )
			{
				ArIsLoading = ArIsTrans = 1;
			}
			FArchive& Serialize( void* Data, int Num )
			{
				appMemcpy( Data, &Bytes(Offset), Num );
				Offset += Num;
				return *this;
			}
			FArchive& operator<<( class FName& N )
			{
				N = *(FName*)&Bytes(Offset);
				Offset += sizeof(FName);
				return *this;
			}
			FArchive& operator<<( class UObject*& Res )
			{
				Res = *(UObject**)&Bytes(Offset);
				Offset += sizeof(UObject*);
				return *this;
			}
			void Preload( UObject* Object )
			{
				guard(FReader::Preload);
				for( INT i=0; i<Owner.Records.Num(); i++ )
					if( Owner.Records(i).Object==Object )
						Owner.Records(i).Restore();
				unguard;
			}
			FTransaction& Owner;
			TArray<BYTE>& Bytes;
			INT Offset;
		};
		class FWriter : public FArchive
		{
		public:
			FWriter( FTransaction& InOwner, TArray<BYTE>& InBytes )
			:	Bytes	( InBytes )
			,	Owner	( InOwner )
			{
				ArIsSaving = ArIsTrans = 1;
			}
			FArchive& Serialize( void* Data, INT Num )
			{
				INT Index = Bytes.Add(Num);
				appMemcpy( &Bytes(Index), Data, Num );
				return *this;
			}
			FArchive& operator<<( class FName& N )
			{			
				INT Index = Bytes.Add(sizeof(FName));
				*(FName*)&Bytes(Index) = N;
				return *this;
			}
			FArchive& operator<<( class UObject*& Res )
			{
				INT Index = Bytes.Add(sizeof(UObject*));
				*(UObject**)&Bytes(Index) = Res;
				return *this;
			}
			FTransaction& Owner;
			TArray<BYTE>& Bytes;
		};
		FTransaction& Owner;
		UObject* Object;
	};
	TArray<FObjectRecord> Records;
	FTransaction()
	{}
	void Restore()
	{
		guard(FTransaction::Restore);
		for( INT i=0; i<Records.Num(); i++ )
			Records(i).Restore();
		for( i=0; i<Records.Num(); i++ )
			Records(i).Object->PostLoad();
		unguard;
	}
};

static void RecursiveSaveField( FTransaction& Transaction, UField* Field )
{
	guard(RecursiveSaveField);
	new( Transaction.Records )FTransaction::FObjectRecord( Transaction, Field );
	UStruct* ThisStruct = Cast<UStruct>( Field );
	if( ThisStruct )
		for( TFieldIterator<UField> It(ThisStruct); It && It.GetStruct()==ThisStruct; ++It )
			RecursiveSaveField( Transaction, *It );
	unguard;
}

/*-----------------------------------------------------------------------------
	Utility functions.
-----------------------------------------------------------------------------*/

//
// Get conversion token between two types.
// Converting a type to itself has no conversion function.
// EX_Max indicates that a conversion isn't possible.
// Conversions to type CPT_String must not be automatic.
//
DWORD GetConversion( const FPropertyBase& Dest, const FPropertyBase& Src )
{
#define AUTOCONVERT 0x100 /* Compiler performs the conversion automatically */
#define TRUNCATE    0x200 /* Conversion requires truncation */
#define CONVERT_MASK ~(AUTOCONVERT | TRUNCATE)
#define AC  AUTOCONVERT
#define TAC TRUNCATE|AUTOCONVERT
static DWORD GConversions[CPT_MAX][CPT_MAX] =
{
			/*   None      Byte                Int                 Bool              Float                Object             Name             String               EnumDef          StructDef        Struct           Vector               Rotator             
			/*   --------  ------------------  ------------------  ----------------  -------------------  -----------------  ---------------  -------------------  ---------------  ---------------  ---------------- -------------------  -------------------- */
/* None     */ { EX_Max,   EX_Max,             EX_Max,             EX_Max,           EX_Max,              EX_Max,            EX_Max,          EX_Max,              EX_Max,          EX_Max,          EX_Max,          EX_Max,              EX_Max,              },
/* Byte     */ { EX_Max,   EX_Max,             EX_IntToByte|TAC,   EX_BoolToByte,	 EX_FloatToByte|TAC,  EX_Max,            EX_Max,          EX_StringToByte,     EX_Max,          EX_Max,          EX_Max,          EX_Max,              EX_Max,              },
/* Int      */ { EX_Max,   EX_ByteToInt|AC,    EX_Max,             EX_BoolToInt,     EX_FloatToInt|TAC,   EX_Max,            EX_Max,          EX_StringToInt,      EX_Max,          EX_Max,          EX_Max,          EX_Max,              EX_Max,              },
/* Bool     */ { EX_Max,   EX_ByteToBool,      EX_IntToBool,       EX_Max,           EX_FloatToBool,      EX_ObjectToBool,   EX_NameToBool,   EX_StringToBool,     EX_Max,          EX_Max,          EX_Max,          EX_VectorToBool,     EX_RotationToBool,   },
/* Float    */ { EX_Max,   EX_ByteToFloat|AC,  EX_IntToFloat|AC,   EX_BoolToFloat,   EX_Max,              EX_Max,            EX_Max,          EX_StringToFloat,    EX_Max,          EX_Max,          EX_Max,          EX_Max,              EX_Max,              },
/* Object   */ { EX_Max,   EX_Max,             EX_Max,             EX_Max,           EX_Max,              EX_Max,            EX_Max,          EX_Max,              EX_Max,          EX_Max,          EX_Max,          EX_Max,              EX_Max,              },
/* Name     */ { EX_Max,   EX_Max,             EX_Max,             EX_Max,           EX_Max,              EX_Max,            EX_Max,          EX_Max,              EX_Max,          EX_Max,          EX_Max,          EX_Max,              EX_Max,              },
/* String   */ { EX_Max,   EX_ByteToString,    EX_IntToString,     EX_BoolToString,  EX_FloatToString,    EX_ObjectToString, EX_NameToString, EX_Max,              EX_Max,          EX_Max,          EX_Max,          EX_VectorToString,   EX_RotationToString, },
/* EnumDef  */ { EX_Max,   EX_Max,             EX_Max,             EX_Max,           EX_Max,              EX_Max,            EX_Max,          EX_Max,              EX_Max,          EX_Max,          EX_Max,          EX_Max,              EX_Max,              },
/* StructDef*/ { EX_Max,   EX_Max,             EX_Max,             EX_Max,           EX_Max,              EX_Max,            EX_Max,          EX_Max,              EX_Max,          EX_Max,          EX_Max,          EX_Max,              EX_Max,              },
/* Struct   */ { EX_Max,   EX_Max,             EX_Max,             EX_Max,           EX_Max,              EX_Max,            EX_Max,          EX_Max,              EX_Max,          EX_Max,          EX_Max,          EX_Max,              EX_Max,              },
/* Vector   */ { EX_Max,   EX_Max,             EX_Max,             EX_Max,           EX_Max,              EX_Max,            EX_Max,          EX_StringToVector,   EX_Max,          EX_Max,          EX_Max,          EX_Max,              EX_RotationToVector, },
/* Rotator  */ { EX_Max,   EX_Max,             EX_Max,             EX_Max,           EX_Max,              EX_Max,            EX_Max,          EX_StringToRotation, EX_Max,          EX_Max,          EX_Max,          EX_VectorToRotation, EX_Max,              },
};
#undef AC
#undef TAC
	INT DestType = Dest.IsVector() ? CPT_Vector : Dest.IsRotator() ? CPT_Rotation : Dest.Type;
	INT SrcType  = Src .IsVector() ? CPT_Vector : Src .IsRotator() ? CPT_Rotation : Src.Type;
	return GConversions[DestType][SrcType];
}

/*-----------------------------------------------------------------------------
	Single-character processing.
-----------------------------------------------------------------------------*/

//
// Get a single character from the input stream and return it, or 0=end.
//
char inline FScriptCompiler::GetChar( UBOOL Literal )
{
	guardSlow(FScriptCompiler::GetChar);
	int CommentCount=0;

	PrevPos  = InputPos;
	PrevLine = InputLine;

	Loop:
	char c = Input[InputPos++];
	if( c==0x0a )
	{
		InputLine++;
	}
	else if( !Literal && c=='/' && Input[InputPos]=='*' )
	{
		CommentCount++;
		InputPos++;
		goto Loop;
	}
	else if( !Literal && c=='*' && Input[InputPos]=='/' )
	{
		if( --CommentCount < 0 )
			appThrowf( "Unexpected '*/' outside of comment" );
		InputPos++;
		goto Loop;
	}
	if( CommentCount > 0 )
	{
		if( c==0 )
			appThrowf( "End of script encountered inside comment" );
		goto Loop;
	}
	return c;
	unguardSlow;
}

//
// Unget the previous character retrieved with GetChar().
//
void inline FScriptCompiler::UngetChar()
{
	guardSlow(FScriptCompiler::UngetChar);

	InputPos  = PrevPos;
	InputLine = PrevLine;

	unguardSlow;
}

//
// Look at a single character from the input stream and return it, or 0=end.
// Has no effect on the input stream.
//
char inline FScriptCompiler::PeekChar()
{
	guardSlow(FScriptCompiler::PeekChar);
	return Input[InputPos];
	unguardSlow;
}

//
// Skip past all spaces and tabs in the input stream.
//
char inline FScriptCompiler::GetLeadingChar()
{
	guardSlow(FScriptCompiler::GetLeadingChar);

	// Skip blanks.
	char c;
	Skip1: do c=GetChar(); while( c==0x20 || c==0x09 || c==0x0d || c==0x0a );
	if( c=='/' && Input[InputPos]=='/' )
	{
		// Comment, so skip to start of next line.
		do c=GetChar(1); while( c!=0x0d && c!=0x0a && c!=0x00 );
		goto Skip1;
	}
	return c;
	unguardSlow;
}

//
// Return 1 if input as a valid end-of-line character, or 0 if not.
// EOL characters are: Comment, CR, linefeed, 0 (end-of-file mark)
//
int inline FScriptCompiler::IsEOL( char c )
{
	guardSlow(FScriptCompiler::IsEOL);
	return c==0x0d || c==0x0a || c==0;
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	Code emitting.
-----------------------------------------------------------------------------*/

//
// Emit a constant expression.
//
void FScriptCompiler::EmitConstant( FToken& Token )
{
	guard(FScriptCompiler::EmitConstant);
	check(Token.TokenType==TOKEN_Const);

	switch( Token.Type )
	{
		case CPT_Int:
		{
			if( Token.Int == 0 )
			{
				Writer << EX_IntZero;
			}
			else if( Token.Int == 1 )
			{
				Writer << EX_IntOne;
			}
			else if( Token.Int>=0 && Token.Int<=255 )
			{
				BYTE B = Token.Int;
				Writer << EX_IntConstByte;
				Writer << B;
			}
			else
			{
				Writer << EX_IntConst;
				Writer << Token.Int;
			}
			break;
		}
		case CPT_Byte:
		{
			Writer << EX_ByteConst;
			Writer << Token.Byte;
			break;
		}
		case CPT_Bool:
		{
			if( Token.Bool ) Writer << EX_True;
			else Writer << EX_False;
			break;
		}
		case CPT_Float:
		{
			Writer << EX_FloatConst;
			Writer << Token.Float;
			break;
		}
		case CPT_String:
		{
			Writer << EX_StringConst;
			Writer.String(Token.String,MAX_STRING_CONST_SIZE);
			break;
		}
		case CPT_ObjectReference:
		{
			if( Token.Object==NULL )
			{
				Writer << EX_NoObject;
			}
			else
			{
				if( Token.PropertyClass->IsChildOf( AActor::StaticClass ) )
					appThrowf( "Illegal actor constant" );
				Writer << EX_ObjectConst;
				Writer << Token.Object;
			}
			break;
		}
		case CPT_Name:
		{
			FName N;
			Token.GetConstName(N);
			Writer << EX_NameConst;
			Writer << N;
			break;
		}
		case CPT_Struct:
		{
			if( Token.IsVector() )
			{
				FVector V;
				Token.GetConstVector(V);
				Writer << EX_VectorConst;
				Writer << V;
			}
			else if( Token.IsRotator() )
			{
				FRotator R;
				Token.GetConstRotation(R);
				Writer << EX_RotationConst;
				Writer << R;
			}
			else
			{
				//caveat: Constant structs aren't supported.
				appThrowf("Not yet implemented");
			}
			break;
		}
		default:
		{
			appThrowf( "Internal EmitConstant token type error %i", Token.Type );
		}
	}
	unguard;
}

//
// Emit the function corresponding to a stack node link.
//
void FScriptCompiler::EmitStackNodeLinkFunction( UFunction* Node, UBOOL ForceFinal, UBOOL Global )
{
	guard(FScriptCompiler::EmitStackNodeFunction);
	UBOOL IsFinal = (Node->FunctionFlags & FUNC_Final) || ForceFinal;

	// Emit it according to function type.
	if( IsFinal && Node->iIntrinsic && Node->iIntrinsic<256 )
	{
		// One-byte call intrinsic final call.
		check( Node->iIntrinsic >= EX_FirstIntrinsic );
		BYTE B = Node->iIntrinsic;
		Writer << B;
	}
	else if( IsFinal && Node->iIntrinsic )
	{
		// Two-byte intrinsic final call.
		BYTE B = EX_ExtendedIntrinsic + (Node->iIntrinsic/256);
		check( B < EX_FirstIntrinsic );
		BYTE C = (Node->iIntrinsic) % 256;
		Writer << B;
		Writer << C;
	}
	else if( IsFinal )
	{
		// Prebound, non-overridable function.
		Writer << EX_FinalFunction;
		Writer << Node;
	}
	else if( Global )
	{
		// Non-state function.
		Writer << EX_GlobalFunction;
		FName N(Node->GetFName());
		Writer << N;
	}
	else
	{
		// Virtual function.
		Writer << EX_VirtualFunction;
		FName N(Node->GetFName());
		Writer << N;
	}
	unguard;
}

//
// Emit a code offset which the script compiler will fix up in the
// proper PopNest call.
//
void FScriptCompiler::EmitAddressToFixupLater( FNestInfo* Nest, EFixupType Type, FName Name )
{
	guard(FScriptCompiler::EmitAddressToFixupLater);

	// Add current code address to the nest level's fixup list.
	Nest->FixupList = new(GMem)FNestFixupRequest( Type, TopNode->Script.Num(), Name, Nest->FixupList );

	// Emit a dummy code offset as a placeholder.
	_WORD Temp=0;
	Writer << Temp;

	unguard;
}

//
// Emit a code offset which should be chained to later.
//
void FScriptCompiler::EmitAddressToChainLater( FNestInfo* Nest )
{
	guard(FScriptCompiler::EmitAddressToChainLater);

	// Note chain address in nest info.
	Nest->iCodeChain = TopNode->Script.Num();

	// Emit a dummy code offset as a placeholder.
	_WORD Temp=0;
	Writer << Temp;

	unguard;
}

//
// Update and reset the nest info's chain address.
//
void FScriptCompiler::EmitChainUpdate( FNestInfo* Nest )
{
	guard(FScriptCompiler::EmitChainUpdate);

	// If there's a chain address, plug in the current script offset.
	if( Nest->iCodeChain != INDEX_NONE )
	{
		check((INT)Nest->iCodeChain+(INT)sizeof(_WORD)<=TopNode->Script.Num())
		*(_WORD*)&TopNode->Script( Nest->iCodeChain ) = TopNode->Script.Num();
		Nest->iCodeChain = INDEX_NONE;
	}
	unguard;
}

//
// Emit a variable size, making sure it's within reason.
//
void FScriptCompiler::EmitSize( INT Size, const char* Tag )
{
	guard(FScriptCompiler::EmitSize);

	BYTE B = Size;
	if( B != Size )
		appThrowf( "%s: Variable is too large (%i bytes, 255 max)", Tag, Size );
	Writer << B;

	unguard;
}

//
// Emit an assignment.
//
void FScriptCompiler::EmitLet( const FPropertyBase& Type, const char *Tag )
{
	guard(FScriptCompiler::EmitLet);

	// Validate the required type.
	if( Type.PropertyFlags & CPF_Const )
		appThrowf( "Can't assign Const variables" );
	if( Type.ArrayDim != 1 )
		appThrowf( "Can only assign individual booleans, not boolean arrays" );
	Writer << EX_Let;

	unguard;
}

/*-----------------------------------------------------------------------------
	Signatures.
-----------------------------------------------------------------------------*/

static const char* CppTags[96] =
{	"Spc", "Not", "DoubleQuote", "Pound", "Concat", "Percent", "And", "SingleQuote"
,	"OpenParen", "CloseParen", "Multiply", "Add", "Comma", "Subtract", "Dot", "Divide"
,	"0", "1", "2", "3", "4", "5", "6", "7"
,	"8", "9", "Colon", "Semicolon", "Less", "Equal", "Greater", "Question"
,	"At", "A", "B", "C", "D", "E", "F", "G"
,	"H", "I", "J", "K", "L", "M", "N", "O"
,	"P", "Q", "R", "S", "T", "U", "V", "W"
,	"X", "Y", "Z", "OpenBracket", "Backslash", "CloseBracket", "Xor", "_"
,	"Not", "a", "b", "c", "d", "e", "f", "g"
,	"h", "i", "j", "k", "l", "m", "n", "o"
,	"p", "q", "r", "s", "t", "u", "v", "w"
,	"x", "y", "z", "OpenBrace", "Or", "CloseBrace", "Complement", "Or" };

/*-----------------------------------------------------------------------------
	Tokenizing.
-----------------------------------------------------------------------------*/

//
// Get the next token from the input stream, set *Token to it.
// Returns 1 if processed a token, 0 if end of line.
//
// If you initialize the token's Type info prior to calling this
// function, we perform special logic that tries to evaluate Token in the 
// context of that type. This is how we distinguish enum tags.
//
INT FScriptCompiler::GetToken( FToken& Token, const FPropertyBase* Hint, INT NoConsts )
{
	guard(FScriptCompiler::GetToken);
	Token.TokenName	= NAME_None;
	char c = GetLeadingChar();
	char p = PeekChar();
	if( c == 0 )
	{
		UngetChar();
		return 0;
	}
	Token.StartPos		= PrevPos;
	Token.StartLine		= PrevLine;
	if( (c>='A' && c<='Z') || (c>='a' && c<='z') )
	{
		// Alphanumeric token.
		INT Length=0;
		do
		{
			Token.Identifier[Length++] = c;
			if( Length > NAME_SIZE )
				appThrowf( "Identifer length exceeds maximum of %i", NAME_SIZE );
			c = GetChar();
		} while( ((c>='A')&&(c<='Z')) || ((c>='a')&&(c<='z')) || ((c>='0')&&(c<='9')) || (c=='_') );
		UngetChar();
		Token.Identifier[Length]=0;

		// Assume this is an identifier unless we find otherwise.
		Token.TokenType = TOKEN_Identifier;

		// Lookup the token's global name.
		Token.TokenName = FName( Token.Identifier, FNAME_Find );

		// See if the idenfitifier is part of a vector, rotation, or object constant.
		if( Token.TokenName==NAME_Vect && !NoConsts && MatchSymbol("(") )
		{
			// This is a vector constant.
			FVector V;
			if(!GetConstFloat(V.X))  appThrowf( "Missing X component of vector" );
			if(!MatchSymbol(","))    appThrowf( "Missing ',' in vector"         );
			if(!GetConstFloat(V.Y))  appThrowf( "Missing Y component of vector" );
			if(!MatchSymbol(","))    appThrowf( "Missing ',' in vector"         );
			if(!GetConstFloat(V.Z))  appThrowf( "Missing Z component of vector" );
			if(!MatchSymbol(")"))    appThrowf( "Missing ')' in vector"         );

			Token.SetConstVector(V);
			return 1;
		}
		if( Token.TokenName==NAME_Rot && !NoConsts && MatchSymbol("(") )
		{
			// This is a rotation constant.
			FRotator R;
			if(!GetConstInt(R.Pitch))             appThrowf( "Missing Pitch component of rotation" );
			if(!MatchSymbol(","))                 appThrowf( "Missing ',' in rotation"             );
			if(!GetConstInt(R.Yaw))               appThrowf( "Missing Yaw component of rotation"   );
			if(!MatchSymbol(","))                 appThrowf( "Missing ',' in vector"               );
			if(!GetConstInt(R.Roll))              appThrowf( "Missing Roll component of rotation"  );
			if(!MatchSymbol(")"))                 appThrowf( "Missing ')' in vector"               );

			Token.SetConstRotation(R);
			return 1;
		}
		if( Token.TokenName==NAME_True && !NoConsts )
		{
			Token.SetConstBool(1);
			return 1;
		}
		if( Token.TokenName==NAME_False && !NoConsts )
		{
			Token.SetConstBool(0);
			return 1;
		}
		if( Token.TokenName==NAME_ArrayCount && !NoConsts )
		{
			FToken TypeToken;
			RequireSizeOfParm( TypeToken, "'ArrayCount'" );
			if( TypeToken.ArrayDim==1 )
				appThrowf( "ArrayCount argument is not an array" );
			Token.SetConstInt( TypeToken.ArrayDim );
			return 1;
		}
		if( Token.Matches("None") && !NoConsts )
		{
			Token.SetConstObject( NULL );
			return 1;
		}

		// See if this is an enum, which we can only evaluate with knowledge
		// about the specified type.
		if( Hint && Hint->Type==CPT_Byte && Hint->Enum && Token.TokenName!=NAME_None && !NoConsts )
		{
			// Find index into the enumeration.
			INT EnumIndex=INDEX_NONE;
			if( Hint->Enum->Names.FindItem( Token.TokenName, EnumIndex ) )
			{
				Token.SetConstByte(Hint->Enum,EnumIndex);
				return 1;
			}
		}

		// See if this is a general object constant.
		if( !NoConsts && PeekSymbol("'") )
		{
			UClass* Type = FindObject<UClass>( ANY_PACKAGE, Token.Identifier );
			if
			(	Type
			&&	!Type->IsChildOf( AActor::StaticClass )
			&&	!NoConsts
			&&	MatchSymbol("'") )
			{
				// This is an object constant.
				CheckInScope( Type );
				FToken NameToken;
				if( !GetIdentifier(NameToken,1) )
					appThrowf( "Missing %s name", Type->GetName() );

				// Find object.
				UObject* Ob = GObj.FindObject( Type, ANY_PACKAGE, NameToken.Identifier );
				if( Ob == NULL )
					appThrowf( "Can't find %s '%s'", Type->GetName(), NameToken.Identifier );
				if( !MatchSymbol("'") )
					appThrowf( "Missing single quote after %s name", Type->GetName() );
				CheckInScope( Ob );

				// Got a constant object.
				Token.SetConstObject( Ob );
				return 1;
			}
		}
		return 1;
	}
	else if( (c>='0' && c<='9') || ((c=='+' || c=='-') && (p>='0' && p<='9')) && !NoConsts )
	{
		// Integer or floating point constant.
		int  IsFloat = 0;
		int  Length  = 0;
		int  IsHex   = 0;
		do 
		{
			if( c=='.' ) IsFloat = 1;
			if( c=='X' ) IsHex   = 1;

			Token.Identifier[Length++] = c;
			if( Length >= NAME_SIZE )
				appThrowf( "Number length exceeds maximum of %i ", NAME_SIZE );
			c = appToUpper(GetChar());
		} while( (c>='0' && c<='9') || c=='.' || c=='X' || (c>='A' && c<='F') );

		Token.Identifier[Length]=0;
		UngetChar();

		if( IsFloat )
		{
			Token.SetConstFloat( appAtof(Token.Identifier) );
		}
		else if( IsHex )
		{
			char* End = Token.Identifier + appStrlen(Token.Identifier);
			Token.SetConstInt( appStrtoi(Token.Identifier,&End,0) );
		}
		else
		{
			Token.SetConstInt( appAtoi(Token.Identifier) );
		}
		return 1;
	}
	else if( c=='\'' && !NoConsts)
	{
		// Name constant.
		int Length=0;
		c = GetChar();
		while( (c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') || (c=='_') )
		{
			Token.Identifier[Length++] = c;
			if( Length > NAME_SIZE )
				appThrowf( "Name length exceeds maximum of %i", NAME_SIZE );
			c = GetChar();
		}
		if( c != '\'' )
			appThrowf( "Illegal character in name" );
		Token.Identifier[Length]=0;

		// Make constant name.
		Token.SetConstName( FName(Token.Identifier) );
		return 1;
	}
	else if( c=='"' )
	{
		// String constant.
		char Temp[MAX_STRING_CONST_SIZE];
		int Length=0;
		c = GetChar(1);
		while( (c!='"') && !IsEOL(c) )
		{
			if(c=='\\')
			{
				c = GetChar(1);
				if( IsEOL(c) )
					break;
			}
			Temp[Length++] = c;
			if( Length >= MAX_STRING_CONST_SIZE )
				appThrowf( "String constant exceeds maximum of %i characters", NAME_SIZE );
			c = GetChar(1);
		}
		if( c!='"' ) 
			appThrowf( "Unterminated string constant" );

		Temp[Length]=0;

		Token.SetConstString(Temp);
		return 1;
	}
	else
	{
		// Symbol.
		int Length=0;
		Token.Identifier[Length++] = c;

		// Handle special 2-character symbols.
		#define PAIR(cc,dd) ((c==cc)&&(d==dd)) /* Comparison macro for convenience */
		char d=GetChar();
		if
		(	PAIR('<','<')
		||	PAIR('>','>')
		||	PAIR('!','=')
		||	PAIR('<','=')
		||	PAIR('>','=')
		||	PAIR('+','+')
		||	PAIR('-','-')
		||	PAIR('+','=')
		||	PAIR('-','=')
		||	PAIR('*','=')
		||	PAIR('/','=')
		||	PAIR('&','&')
		||	PAIR('|','|')
		||	PAIR('^','^')
		||	PAIR('=','=')
		||	PAIR('*','*')
		||	PAIR('~','=')
		)
		{
			Token.Identifier[Length++] = d;
		}
		else UngetChar();
		#undef PAIR

		Token.Identifier[Length]=0;
		Token.TokenType = TOKEN_Symbol;

		// Lookup the token's global name.
		Token.TokenName = FName( Token.Identifier, FNAME_Find );

		return 1;
	}
	return 0;
	unguard;
}

//
// Get a raw token until we reach end of line.
//
INT FScriptCompiler::GetRawToken( FToken& Token )
{
	guard(FScriptCompiler::GetRawToken);

	// Get token after whitespace.
	char Temp[MAX_STRING_CONST_SIZE];
	int  Length=0;
	char c=GetLeadingChar();
	while( !IsEOL(c) )
	{
		if( (c=='/' && PeekChar()=='/') || (c=='/' && PeekChar()=='*') )
			break;
		Temp[Length++] = c;
		if( Length >= MAX_STRING_CONST_SIZE )
			appThrowf( "Identifier exceeds maximum of %i characters", NAME_SIZE );
		c = GetChar(1);
	}
	UngetChar();

	// Get rid of trailing whitespace.
	while( Length>0 && (Temp[Length-1]==' ' || Temp[Length-1]==9 ) )
		Length--;
	Temp[Length]=0;

	Token.SetConstString(Temp);

	return Length>0;
	unguard;
}

//
// Get an identifier token, return 1 if gotten, 0 if not.
//
INT FScriptCompiler::GetIdentifier( FToken& Token, INT NoConsts )
{
	guard(FScriptCompiler::GetIdentifier);

	if( !GetToken( Token, NULL, NoConsts ) )
		return 0;

	if( Token.TokenType == TOKEN_Identifier )
		return 1;

	UngetToken(Token);
	return 0;
	unguard;
}

//
// Get a symbol token, return 1 if gotten, 0 if not.
//
INT FScriptCompiler::GetSymbol( FToken& Token )
{
	guard(FScriptCompiler::GetSymbol);

	if( !GetToken(Token) )
		return 0;

	if( Token.TokenType == TOKEN_Symbol ) 
		return 1;

	UngetToken(Token);
	return 0;
	unguard;
}

//
// Get an integer constant, return 1 if gotten, 0 if not.
//
INT FScriptCompiler::GetConstInt( INT& Result, const char* Tag )
{
	guard(FScriptCompiler::GetConstInt);

	FToken Token;
	if( GetToken(Token) ) 
	{
		if( Token.GetConstInt(Result) ) return 1;
		else                            UngetToken(Token);
	}

	if( Tag ) appThrowf( "%s: Missing constant integer" );
	return 0;

	unguard;
}

//
// Get a real number, return 1 if gotten, 0 if not.
//
INT FScriptCompiler::GetConstFloat( FLOAT& Result, const char* Tag )
{
	guard(FScriptCompiler::GetConstFloat);

	FToken Token;
	if( GetToken(Token) ) 
	{
		if( Token.GetConstFloat(Result) ) return 1;
		else                              UngetToken(Token);
	}

	if( Tag ) appThrowf( "%s: Missing constant integer" );
	return 0;

	unguard;
}

//
// Get a specific identifier and return 1 if gotten, 0 if not.
// This is used primarily for checking for required symbols during compilation.
//
INT	FScriptCompiler::MatchIdentifier( const char* Match )
{
	guard(FScriptCompiler::MatchIdentifier);
	FToken Token;

	if( !GetToken(Token) )
		return 0;

	if( (Token.TokenType==TOKEN_Identifier) && !appStricmp(Token.Identifier,Match) )
		return 1;

	UngetToken(Token);
	return 0;
	unguard;
}

//
// Get a specific symbol and return 1 if gotten, 0 if not.
//
INT	FScriptCompiler::MatchSymbol( const char* Match )
{
	guard(FScriptCompiler::MatchSymbol);
	FToken Token;

	if( !GetToken(Token,NULL,1) )
		return 0;

	if( Token.TokenType==TOKEN_Symbol && !appStricmp(Token.Identifier,Match) )
		return 1;

	UngetToken(Token);
	return 0;
	unguard;
}

//
// Peek ahead and see if a symbol follows in the stream.
//
INT FScriptCompiler::PeekSymbol( const char* Match )
{
	guard(FScriptCompiler::PeekSymbol);

	FToken Token;
	if( !GetToken(Token,NULL,1) )
		return 0;
	UngetToken(Token);

	return Token.TokenType==TOKEN_Symbol && appStricmp(Token.Identifier,Match)==0;
	unguard;
}

//
// Peek ahead and see if an identifier follows in the stream.
//
INT FScriptCompiler::PeekIdentifier( const char* Match )
{
	guard(FScriptCompiler::PeekSymbol);

	FToken Token;
	if( !GetToken(Token,NULL,1) )
		return 0;
	UngetToken(Token);

	return Token.TokenType==TOKEN_Identifier && appStricmp(Token.Identifier,Match)==0;
	unguard;
}

//
// Unget the most recently gotten token.
//
void inline FScriptCompiler::UngetToken( FToken& Token )
{
	guardSlow(FScriptCompiler::UngetToken);
	InputPos	= Token.StartPos;
	InputLine	= Token.StartLine;
	unguardSlow;
}

//
// Require a symbol.
//
void FScriptCompiler::RequireSymbol( const char* Match, const char* Tag )
{
	guardSlow(FScriptCompiler::RequireSymbol);
	if( !MatchSymbol(Match) )
		appThrowf( "Missing '%s' in %s", Match, Tag );
	unguardSlow;
}

//
// Require an identifier.
//
void FScriptCompiler::RequireIdentifier( const char *Match, const char *Tag )
{
	guardSlow(FScriptCompiler::RequireSymbol);
	if( !MatchIdentifier(Match) )
		appThrowf( "Missing '%s' in %s", Match, Tag );
	unguardSlow;
}

//
// Require a SizeOf-style parenthesis-enclosed type.
//
void FScriptCompiler::RequireSizeOfParm( FToken &TypeToken, const char *Tag )
{
	guard(FScriptCompiler::RequireSizeOfParm);

	// Setup a retry point.
	FRetryPoint Retry;
	InitRetry( Retry );

	// Get leading paren.
	RequireSymbol( "(", Tag );

	// Get an expression.
	if( !CompileExpr( FPropertyBase(CPT_None), Tag, &TypeToken ) )
		appThrowf( "Bad or missing expression in %s", Tag );

	// Get trailing paren.
	RequireSymbol( ")", Tag );

	// Return binary code pointer (not script text) to where it was.
	PerformRetry( Retry, 1, 0 );

	unguard;
}

//
// Get a qualified class.
//
UClass* FScriptCompiler::GetQualifiedClass( const char* Thing )
{
	guard(FScriptCompiler::GetQualifiedClass);
	UClass* Result = NULL;
	char ClassName[256]="";
	while( 1 )
	{
		FToken Token;
		if( !GetIdentifier(Token) )
			break;
		appStrncat( ClassName, Token.Identifier, ARRAY_COUNT(ClassName) );
		if( !MatchSymbol(".") )
			break;
		appStrncat( ClassName, ".", ARRAY_COUNT(ClassName) );
	}
	if( ClassName[0] )
	{
		Result = FindObject<UClass>( ANY_PACKAGE, ClassName );
		if( !Result )
			appThrowf( "Class '%s' not found", ClassName );
	}
	else if( Thing )
	{
		appThrowf( "%s: Missing class name", Thing );
	}
	return Result;
	unguard;
}

/*-----------------------------------------------------------------------------
	Fields.
-----------------------------------------------------------------------------*/

UField* FScriptCompiler::FindField
(
	UStruct*	Scope,
	const char*	InIdentifier,
	UClass*		FieldClass,
	const char* Thing
)
{
	guard(FScriptCompiler::FindField);
	check(InIdentifier);
	FName InName( InIdentifier, FNAME_Find );
	if( InName!=NAME_None )
	{
		for( Scope; Scope; Scope=Cast<UStruct>( Scope->GetParent()) )
		{
			for( TFieldIterator<UField> It(Scope); It; ++It )
			{
				if( It->GetFName()==InName )
				{
					if( !It->IsA(FieldClass) )
					{
						if( Thing )
							appThrowf( "%s: expecting %s, got %s", Thing, FieldClass->GetName(), It->GetClassName() );
						return NULL;
					}
					return *It;
				}
			}
		}
	}
	return NULL;
	unguard;
}

/*-----------------------------------------------------------------------------
	Variables.
-----------------------------------------------------------------------------*/

//
// Compile an enumeration definition.
//
UEnum* FScriptCompiler::CompileEnum( UStruct* Scope )
{
	guard(FScriptCompiler::CompileEnum);
	check(Scope);

	// Make sure enums can be declared here.
	if( TopNest->NestType!=NEST_Class )
		appThrowf( "Enums can only be declared in class or struct scope" );

	// Get enumeration name.
	FToken EnumToken;
	if( !GetIdentifier(EnumToken) )
		appThrowf( "Missing enumeration name" );

	// Verify that the enumeration definition is unique within this scope.
	UField* Existing = FindField( Scope, EnumToken.Identifier );
	if( Existing && Existing->GetParent()==Scope )
		appThrowf( "enum: '%s' already defined here", *EnumToken.TokenName );

	// Get opening brace.
	RequireSymbol( "{", "'Enum'" );

	// Create enum definition.
	UEnum* Enum = new( Scope, EnumToken.Identifier, RF_Public )UEnum( NULL );
	Enum->Next = Scope->Children;
	Scope->Children = Enum;

	// Parse all enums tags.
	FToken TagToken;
	while( GetIdentifier(TagToken) )
	{
		INT iFound;
		FName NewTag(TagToken.Identifier);
		if( Enum->Names.FindItem(NewTag,iFound) )
			appThrowf( "Duplicate enumeration tag %s", TagToken.Identifier );
		Enum->Names.AddItem( NewTag );
		if( Enum->Names.Num() > 255 )
			appThrowf( "Exceeded maximum of 255 enumerators" );
		if( !MatchSymbol(",") )
			break;
	}
	if( !Enum->Names.Num() )
		appThrowf( "Enumeration must contain at least one enumerator" );

	// Trailing brace.
	RequireSymbol( "}", "'Enum'" );

	return Enum;
	unguard;
}

//
// Compile a struct definition.
//
UStruct* FScriptCompiler::CompileStruct( UStruct* Scope )
{
	guard(FScriptCompiler::CompileStruct);
	check(Scope);

	// Make sure enums can be declared here.
	if( TopNest->NestType!=NEST_Class )
		appThrowf( "Enums can only be declared in class or struct scope" );

	// Get struct name.
	FToken StructToken;
	if( !GetIdentifier(StructToken) )
		appThrowf( "Missing struct name" );

	// Verify uniqueness.
	UField* Existing = FindField( Scope, StructToken.Identifier );
	if( Existing && Existing->GetParent()==Scope )
		appThrowf( "struct: '%s' already defined here", *StructToken.TokenName );

	// Get optional superstruct.
	UStruct* BaseStruct = NULL;
	if( MatchIdentifier( "Expands" ) )
	{
		FToken ParentName;
		if( !GetIdentifier( ParentName ) )
			appThrowf( "'struct': Missing parent struct after 'Expands'" );
		BaseStruct = Cast<UStruct>( FindField( Scope, ParentName.Identifier, UStruct::StaticClass, "'expands'" ) );
		if( !BaseStruct )
			appThrowf( "'expands': Can't find base struct '%s'", ParentName.Identifier );
	}

	// Create.
	UStruct* Struct = new( Scope, StructToken.Identifier, RF_Public )UStruct(BaseStruct);
	Struct->Next = Scope->Children;
	Scope->Children = Struct;

	// Get opening brace.
	RequireSymbol( "{", "'struct'" );

	// Parse all struct variables.
	INT NumElements=0;
	FToken Token;
	do
	{
		GetToken( Token );
		if( Token.Matches(NAME_Struct) )
		{
			guard(Struct);
			CompileStruct( Struct );
			RequireSymbol( ";", "'struct'" );
			unguard;
		}
		else if
		(	Token.Matches(NAME_Const) )
		{
			guard(Const);
			CompileConst( Struct );
			RequireSymbol( ";", "'const'" );
			unguard;
		}
		else if
		(	Token.Matches( NAME_Var ) )
		{
			guard(Var);

			// Get editability.
			DWORD EdFlags = 0;
			if( MatchSymbol("(") )
			{
				EdFlags |= CPF_Edit;
				RequireSymbol( ")", "Editable 'struct' member variable" );
			}

			// Get variable type.
			DWORD ObjectFlags=0;
			FPropertyBase OriginalProperty(CPT_None);
			GetVarType( Struct, OriginalProperty, ObjectFlags, CPF_ParmFlags, "'struct' member variable" );
			OriginalProperty.PropertyFlags |= EdFlags;
			FName Category = (EdFlags & CPF_Edit) ? FName( StructToken.Identifier, FNAME_Find ) : NAME_None;

			// Validate.
			if( OriginalProperty.PropertyFlags & CPF_ParmFlags )
				appThrowf( "Illegal type modifiers in variable" );

			// Process all variables of this type.
			do
			{
				FPropertyBase Property = OriginalProperty;
				GetVarNameAndDim( Struct, Property, ObjectFlags, 0, 0, NULL, "Variable declaration", Category, 0 );
			} while( MatchSymbol(",") );

			// Expect a semicolon.
			RequireSymbol( ";", "'struct'" );
			NumElements++;
			unguard;
		}
		else if( !Token.Matches( "}" ) )
		{
			appThrowf( "'struct': Expecting 'Var', got '%s'", Token.Identifier );
		}
	} while( !Token.Matches( "}" ) );
	FArchive DummyAr;
	Struct->LinkOffsets( DummyAr );
	return Struct;
	unguard;
}

//
// Compile a constant definition.
//
void FScriptCompiler::CompileConst( UStruct* Scope )
{
	guard(FScriptCompiler::CompileConst);
	check(Scope);

	// Get varible name.
	FToken ConstToken;
	if( !GetIdentifier(ConstToken) ) 
		appThrowf( "Missing constant name" );

	// Verify uniqueness.
	FName ConstName = FName( ConstToken.Identifier );
	UField* Existing = FindField( Scope, ConstToken.Identifier );
	if( Existing && Existing->GetParent()==Scope )
		appThrowf( "const: '%s' already defined", *ConstToken.Identifier );

	// Get equals and value.
	RequireSymbol( "=", "'const'" );
	const char* Start = Input+InputPos;
	FToken ValueToken;
	if( !GetToken(ValueToken) )
		appThrowf( "const %s: Missing value", *ConstName );
	if( ValueToken.TokenType != TOKEN_Const )
		appThrowf( "const %s: Value is not constant", *ConstName );

	// Format constant.
	char Value[1024];
	appStrncpy( Value, Start, Min(1024,Input+InputPos-Start+1) );

	// Create constant.
	UConst* NewConst = new(Scope,ConstName)UConst(NULL,Value);
	NewConst->Next = Scope->Children;
	Scope->Children = NewConst;

	unguard;
}

/*-----------------------------------------------------------------------------
	Retry management.
-----------------------------------------------------------------------------*/

//
// Remember the current compilation points, both in the source being
// compiled and the object code being emitted.  Required because
// UnrealScript grammar isn't quite LALR-1.
//
void FScriptCompiler::InitRetry( FRetryPoint& Retry )
{
	guardSlow(FScriptCompiler::InitRetry);

	Retry.Input     = Input;
	Retry.InputPos	= InputPos;
	Retry.InputLine	= InputLine;
	Retry.CodeTop	= TopNode->Script.Num();

	unguardSlow;
}

//
// Return to a previously-saved retry point.
//
void FScriptCompiler::PerformRetry( FRetryPoint& Retry, UBOOL Binary, UBOOL Text )
{
	guardSlow(FScriptCompiler::PerformRetry);
	if( Text )
	{
		Input     = Retry.Input;
		InputPos  = Retry.InputPos;
		InputLine = Retry.InputLine;
	}
	if( Binary )
	{
		check(Retry.CodeTop <= TopNode->Script.Num());
		TopNode->Script.Remove( Retry.CodeTop, TopNode->Script.Num() - Retry.CodeTop );
		check(TopNode->Script.Num()==Retry.CodeTop);
	}
	unguardSlow;
}

//
// Insert the code in the interval from Retry2-End into the code stream
// beginning at Retry1.
//
void FScriptCompiler::CodeSwitcheroo( FRetryPoint& LowRetry, FRetryPoint& HighRetry )
{
	guardSlow(FScriptCompiler::CodeSwitcheroo);
	FMemMark Mark(GMem);
	INT HighSize = TopNode->Script.Num() - HighRetry.CodeTop;
	INT LowSize  = HighRetry.CodeTop   - LowRetry.CodeTop;

	BYTE *Temp = new(GMem,HighSize)BYTE;
	appMemcpy ( Temp,                                          &TopNode->Script(HighRetry.CodeTop),HighSize);
	appMemmove( &TopNode->Script(LowRetry.CodeTop + HighSize), &TopNode->Script(LowRetry.CodeTop), LowSize );
	appMemcpy ( &TopNode->Script(LowRetry.CodeTop           ), Temp,                             HighSize);

	Mark.Pop();
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	Functions.
-----------------------------------------------------------------------------*/

//
// Try to compile a complete function call or field expression with a field name 
// matching Token.  Returns 1 if a function call was successfully parsed, or 0 if no matching function
// was found.  Handles the error condition where the function was called but the
// specified parameters didn't match, or there was an error in a parameter 
// expression.
//
// The function to call must be accessible within the current scope.
//
// This also handles unary operators identically to functions, but not binary
// operators.
//
// Sets ResultType to the function's return type.
//
UBOOL FScriptCompiler::CompileFieldExpr
(
	UStruct*		Scope,
	FPropertyBase&	RequiredType,
	FToken			Token,
	FToken&			ResultType,
	UBOOL			IsSelf,
	UBOOL			IsConcrete
)
{
	guard(FScriptCompiler::CompileFieldExpr);
	FRetryPoint Retry; InitRetry(Retry);
	UBOOL    ForceFinal = 0;
	UBOOL    Global     = 0;
	UBOOL    Super      = 0;
	UBOOL    Default    = 0;
	UBOOL    Static     = 0;
	UClass*  FieldClass = NULL;

	// Handle specifiers.
	if( Token.Matches(NAME_Default) )
	{
		// Default value of variable.
		Default    = 1;
		FieldClass = UProperty::StaticClass;
		RequireSymbol( ".", "'default'" );
		GetToken(Token);
	}
	else if( Token.Matches(NAME_Static) )
	{
		// Default value of variable.
		Static    = 1;
		FieldClass = UFunction::StaticClass;
		RequireSymbol( ".", "'static'" );
		GetToken(Token);
	}
	else if( Token.Matches(NAME_Global) )
	{
		// Call the highest global, final (non-state) version of a function.
		if( !IsSelf )
			appThrowf( "Can only use 'global' with self" );
		if( !IsConcrete )
			appThrowf( "Can only use 'global' with concrete objects" );
		Scope      = Scope->GetOwnerClass();
		FieldClass = UFunction::StaticClass;
		Global     = 1;
		IsSelf     = 0;
		RequireSymbol( ".", "'global'" );
		GetToken(Token);
	}
	else if( Token.Matches(NAME_Super) && !PeekSymbol("(") )
	{
		// Call the superclass version of the function.
		if( !IsSelf )
			appThrowf( "Can only use 'super' with self" );

		while( Scope && !Scope->GetInheritanceSuper() )
			Scope = CastChecked<UStruct>( Scope->GetParent() );
		if( !Scope )
			appThrowf( "Can't use 'super': no superclass" );
		Scope = Scope->GetSuperStruct();

		FieldClass = UFunction::StaticClass;
		ForceFinal = 1;
		IsSelf     = 0;
		Super      = 1;
		RequireSymbol( ".", "'super'" );
		GetToken(Token);
	}
	else if( Token.Matches(NAME_Super) )
	{
		// Call the final version of a function residing at or below a certain class.
		if( !IsSelf )
			appThrowf( "Can only use 'super(classname)' with self" );
		RequireSymbol( "(", "'super'" );
		FToken ClassToken;
		if( !GetIdentifier(ClassToken) )
			appThrowf( "Missing class name" );
		Scope = FindObject<UClass>( ANY_PACKAGE, ClassToken.Identifier );//!!
		if( !Scope )
			appThrowf( "Bad class name '%s'", ClassToken.Identifier );
		if( !Scope->IsChildOf( Scope->GetOwnerClass() ) )
			appThrowf( "'Super(classname)': class '%s' does not expand '%s'", ClassToken.Identifier, Scope->GetName(), Scope->GetName() );
		RequireSymbol( ")", "'super(classname)'" );
		RequireSymbol( ".", "'super(classname)'" );
		FieldClass = UFunction::StaticClass;
		ForceFinal = 1;
		IsSelf     = 0;
		GetToken(Token);
	}

	// Handle field type.
	UField* Field = FindField( Scope, Token.Identifier, FieldClass );
	if( Field && Field->GetClass()==UEnum::StaticClass && MatchSymbol(".") )
	{
		// Enum constant.
		UEnum* Enum = CastChecked<UEnum>( Field );
		INT EnumIndex = INDEX_NONE;
		if( !GetToken(Token) )
			appThrowf( "Missing enum tag after '%s'", Enum->GetName() );
		else if( Token.TokenName==NAME_EnumCount )
			EnumIndex = Enum->Names.Num();
		else if( Token.TokenName==NAME_None || !Enum->Names.FindItem( Token.TokenName, EnumIndex ) )
			appThrowf( "Missing enum tag after '%s'", Enum->GetName() );
		ResultType.SetConstByte( Enum, EnumIndex );
		ResultType.PropertyFlags &= ~CPF_OutParm;
		EmitConstant( ResultType );
		return 1;
	}
	else if( Field && Field->IsA(UProperty::StaticClass) )
	{
		// Check validity.
		UProperty* Property = CastChecked<UProperty>( Field );
		if( !(Property->GetFlags() & RF_Public) && (Property->GetOwnerClass()!=Class) )
			appThrowf( "Can't access private variable '%s' in '%s'", Property->GetName(), Property->GetOwnerClass()->GetName() );
		if( Default && Property->GetParent()->GetClass()!=UClass::StaticClass )
			appThrowf( "You can't access the default value of static and local variables" );
		if( !IsConcrete && !Default && Property->GetParent()->GetClass()!=UFunction::StaticClass )
			appThrowf( "You can only access default values of variables here" );

		// Process the variable we found.
		if( Property->IsA(UBoolProperty::StaticClass) )
			Writer << EX_BoolVariable;
		EExprToken ExprToken = Default ? EX_DefaultVariable : Property->GetParent()->GetClass()==UFunction::StaticClass ? EX_LocalVariable : EX_InstanceVariable;
		Writer << ExprToken;
		Writer << Property;

		// Return the type.
		ResultType = FPropertyBase( Property );
		if( !Default )
			ResultType.PropertyFlags |= CPF_OutParm;
		if( Property->GetFName()==NAME_Class && Property->GetParent()==UObject::StaticClass )
			ResultType.MetaClass = Scope->GetOwnerClass();
		return 1;
	}
	else if( Field && Field->GetClass()==UFunction::StaticClass && MatchSymbol("(") )
	{
		// Function.
		UFunction* Function = CastChecked<UFunction>( Field );
		GotAffector = 1;

		// Verify that the function is callable here.
		if( Function->FunctionFlags & FUNC_Latent )
			CheckAllow( Function->GetName(), ALLOW_StateCmd );
		if( !(Function->FunctionFlags & FUNC_Static) && !IsConcrete )
			appThrowf( "Can't call instance functions from within static functions" );
		if( Static && !(Function->FunctionFlags & FUNC_Static) )
			appThrowf( "Function '%s' is not static", Function->GetName() );
		if( (Function->FunctionFlags & FUNC_Iterator) && !AllowIterator )
			appThrowf( "Can't call iterator functions here" );
		if( Function->FunctionFlags & FUNC_Iterator )
			GotIterator = 1;

		// Emit the function call.
		EmitStackNodeLinkFunction( Function, ForceFinal, Global );

		// See if this is an iterator with automatic casting of parm 2 object to the parm 1 class.
		UBOOL IsIteratorCast = 0;
		if
		(	(Function->FunctionFlags & FUNC_Iterator)
		&&	(Function->NumParms>=2) )
		{
			TFieldIterator<UProperty> It(Function);
			UObjectProperty* A=Cast<UObjectProperty>(*It); ++It;
			UObjectProperty* B=Cast<UObjectProperty>(*It);
			if( A && B && A->PropertyClass==UClass::StaticClass )
				IsIteratorCast=1;
		}
		UClass* IteratorClass = NULL;

		// Parse the parameters.
		FToken ParmToken[MAX_FUNC_PARMS];
		INT Count=0;
		for( TFieldIterator<UProperty> It(Function); It && (It->PropertyFlags&(CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It,++Count )
		{
			// Get parameter.
			FPropertyBase Parm = FPropertyBase( *It );
			if( Parm.PropertyFlags & CPF_ReturnParm )
				break;

			// If this is an iterator, automatically adjust the second parameter's type.
			if( Count==1 && IteratorClass )
				Parm.PropertyClass = IteratorClass;

			// Get comma parameter delimiter.
			if( Count!=0 && !MatchSymbol(",") )
			{
				// Failed to get a comma.
				if( !(Parm.PropertyFlags & CPF_OptionalParm) )
					appThrowf( "Call to '%s': missing or bad parameter %i", Function->GetName(), Count+1 );

				// Ok, it was optional.
				break;
			}

			INT Result = CompileExpr( Parm, NULL, &ParmToken[Count] );
			if( Result == -1 )
			{
				// Type mismatch.
				appThrowf( "Call to '%s': type mismatch in parameter %i", Token.Identifier, Count+1 );
			}
			else if( Result == 0 )
			{
				// Failed to get an expression.
				if( !(Parm.PropertyFlags & CPF_OptionalParm) ) 
					appThrowf( "Call to '%s': bad or missing parameter %i", Token.Identifier, Count+1 );
				if( PeekSymbol(")") )
					break;
				else
					Writer << EX_Nothing;
			}
			else if( IsIteratorCast && Count==0 )
				ParmToken[Count].GetConstObject( UClass::StaticClass, *(UObject**)&IteratorClass );
		}
		for( It; It && (It->PropertyFlags&(CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It )
			check(It->PropertyFlags & CPF_OptionalParm);

		// Get closing paren.
		FToken Temp;
		if( !GetToken(Temp) || Temp.TokenType!=TOKEN_Symbol )
			appThrowf( "Call to '%s': Bad expression or missing ')'", Token.Identifier );
		if( !Temp.Matches(")") )
			appThrowf( "Call to '%s': Bad '%s' or missing ')'", Token.Identifier, Temp.Identifier );

		// Emit end-of-function-parms tag.
		Writer << EX_EndFunctionParms;

		// Check return value.
		if( It && (It->PropertyFlags & CPF_ReturnParm) )
		{
			// Has a return value.
			ResultType = FPropertyBase( *It );
			ResultType.PropertyFlags &= ~CPF_OutParm;

			// Spawn special case: Make return type the same as a constant class passed to it.
			if( Token.Matches(NAME_Spawn) )
				ResultType.PropertyClass = ParmToken[0].MetaClass;
		}
		else
		{
			// No return value.
			ResultType = FToken(FPropertyBase(CPT_None));
		}

		// Returned value is an r-value.
		ResultType.PropertyFlags &= ~CPF_OutParm;

		return 1;
	}
	else if( Field && Field->GetClass()==UConst::StaticClass )
	{
		// Named constant.
		UConst* Const = CastChecked<UConst>( Field );
		FRetryPoint Retry; InitRetry(Retry);
		Input = *Const->Value;
		InputPos = 0;
		if( !GetToken( ResultType, &RequiredType ) || ResultType.TokenType!=TOKEN_Const )
			appThrowf( "Error in constant" );
		PerformRetry(Retry,0,1);
		ResultType.AttemptToConvertConstant( RequiredType );
		ResultType.PropertyFlags &= ~CPF_OutParm;
		EmitConstant( ResultType );
		return 1;
	}
	else
	{
		// Nothing.
		if( FieldClass )
			appThrowf( "Unknown %s '%s' in '%s'", FieldClass->GetName(), Token.Identifier, Scope->GetFullName() );
		PerformRetry( Retry );
		return 0;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Type conversion.
-----------------------------------------------------------------------------*/

//
// Return the cost of converting a type from Source to Dest:
//    0 if types are identical.
//    1 if dest is more precise than source.
//    2 if converting integral to float.
//    3 if dest is less precise than source, or a generalization of source.
//    MAXINT if the types are incompatible.
//
int FScriptCompiler::ConversionCost
(
	const FPropertyBase& Dest,
	const FPropertyBase& Source
)
{
	guard(FScriptCompiler::ConversionCost);
	DWORD Conversion = GetConversion( Dest, Source );

	if( Dest.MatchesType(Source,1) )
	{
		// Identical match.
		//AddResultText("Identical\r\n");
		return 0;
	}
	else if( Dest.PropertyFlags & CPF_OutParm )
	{
		// If converting to l-value, conversions aren't allowed.
		//AddResultText("IllegalOut\r\n");
		return MAXINT;
	}
	else if( Dest.MatchesType(Source,0) )
	{
		// Generalization.
		//AddResultText("Generalization\r\n");
		int Result = 1;
		if( Source.Type==CPT_ObjectReference && Source.PropertyClass!=NULL )
		{
			// The fewer classes traversed in this conversion, the better the quality.
			check(Dest.Type==CPT_ObjectReference);
			check(Dest.PropertyClass!=NULL);
			for( UClass* Test=Source.PropertyClass; Test && Test!=Dest.PropertyClass; Test=Test->GetSuperClass() )
				Result++;
			check(Test!=NULL);
		}
		return Result;
	}
	else if( Dest.ArrayDim!=1 || Source.ArrayDim!=1 )
	{
		// Can't cast arrays.
		//AddResultText("NoCastArrays\r\n");
		return MAXINT;
	}
	else if( Dest.Type==CPT_Byte && Dest.Enum!=NULL )
	{
		// Illegal enum cast.
		//AddResultText("IllegalEnumCast\r\n");
		return MAXINT;
	}
	else if( Dest.Type==CPT_ObjectReference && Dest.PropertyClass!=NULL )
	{
		// Illegal object cast.
		//AddResultText("IllegalObjectCast\r\n");
		return MAXINT;
	}
	else if( (Dest.PropertyFlags & CPF_CoerceParm) ? (Conversion==EX_Max) : !(Conversion & AUTOCONVERT) )
	{
		// No conversion at all.
		//AddResultText("NoConversion\r\n");
		return MAXINT;
	}
	else if( GetConversion( Dest, Source ) & TRUNCATE )
	{
		// Truncation.
		//AddResultText("Truncation\r\n");
		return 103;
	}
	else if( (Source.Type==CPT_Int || Source.Type==CPT_Byte) && Dest.Type==CPT_Float )
	{
		// Conversion to float.
		//AddResultText("ConvertToFloat\r\n");
		return 102;
	}
	else
	{
		// Expansion.
		//AddResultText("Expansion\r\n");
		return 101;
	}
	unguard;
}

//
// Compile a dynamic object upcast expression.
//
UBOOL FScriptCompiler::CompileDynamicCast( FToken Token, FToken& ResultType )
{
	guard(FScriptCompiler::CompileDynamicCast);
	FRetryPoint LowRetry; InitRetry(LowRetry);
	if( !MatchSymbol("(") && !PeekSymbol("<") )
		return 0;
	UClass* DestClass = FindObject<UClass>( ANY_PACKAGE, Token.Identifier );
	if( !DestClass )
	{
		PerformRetry( LowRetry );
		return 0;
	}
	UClass* MetaClass = UObject::StaticClass;
	if( DestClass==UClass::StaticClass )
	{
		if( MatchSymbol("<") )
		{
			FToken Token;
			if( !GetIdentifier(Token) )
				return 0;
			MetaClass = FindObject<UClass>( ANY_PACKAGE, Token.Identifier );
			if( !MetaClass || !MatchSymbol(">") || !MatchSymbol("(") )
			{
				PerformRetry( LowRetry );
				return 0;
			}
		}
	}

	// Get expression to cast, and ending paren.
	FToken TempType;
	FPropertyBase RequiredType( UObject::StaticClass );
	if
	(	CompileExpr(RequiredType, NULL, &TempType)!=1
	||	!MatchSymbol(")") )
	{
		// No ending paren, therefore it is probably a function call.
		PerformRetry( LowRetry );
		return 0;
	}
	CheckInScope( DestClass );

	// See what kind of conversion this is.
	if
	(	(!TempType.PropertyClass || TempType.PropertyClass->IsChildOf(DestClass))
	&&	(TempType.PropertyClass!=UClass::StaticClass || DestClass!=UClass::StaticClass || TempType.MetaClass->IsChildOf(MetaClass) ) )
	{
		// Redundent conversion.
		appThrowf
		(
			"Cast from '%s' to '%s' is unnecessary",
			TempType.PropertyClass ? TempType.PropertyClass->GetName() : "None",
			DestClass->GetName()
		);
	}
	else if
	(	(DestClass->IsChildOf(TempType.PropertyClass))
	&&	(DestClass!=UClass::StaticClass || TempType.MetaClass==NULL || MetaClass->IsChildOf(TempType.MetaClass) ) )
	{
		// Dynamic cast, must be handled at runtime.
		FRetryPoint HighRetry; InitRetry(HighRetry);
		if( DestClass==UClass::StaticClass )
		{
			Writer << EX_MetaCast;
			Writer << MetaClass;
		}
		else
		{
			Writer << EX_DynamicCast;
			Writer << DestClass;
		}
		CodeSwitcheroo(LowRetry,HighRetry);
	}
	else
	{
		// The cast will always fail.
		appThrowf( "Cast from '%s' to '%s' will always fail", TempType.PropertyClass->GetName(), DestClass->GetName() );
	}

	// A cast is no longer an l-value.
	ResultType = FToken( FPropertyBase( DestClass ) );
	if( ResultType.PropertyClass == UClass::StaticClass )
		ResultType.MetaClass = MetaClass;
	return 1;
	unguard;
}

/*-----------------------------------------------------------------------------
	Expressions.
-----------------------------------------------------------------------------*/

//
// Compile a top-level expression.
//
// Returns:
//		 0	if no expression was parsed.
//		 1	if an expression matching RequiredType (or any type if CPT_None) was parsed.
//		-1	if there was a type mismatch.
//
UBOOL FScriptCompiler::CompileExpr
(
	FPropertyBase	RequiredType,
	const char*		ErrorTag,
	FToken*			ResultToken,
	INT				MaxPrecedence,
	FPropertyBase*	HintType
)
{
	guard(FScriptCompiler::CompileExpr);
	FRetryPoint LowRetry; InitRetry(LowRetry);

	FPropertyBase P(CPT_None);
	FToken Token(P);
	UStruct* NewContext=NULL;
	if( !GetToken( Token, HintType ? HintType : &RequiredType ) )
	{
		// This is an empty expression.
		(FPropertyBase&)Token = FPropertyBase( CPT_None );
	}
	else if( Token.TokenType == TOKEN_Const )
	{
		// This is some kind of constant.
		Token.AttemptToConvertConstant( RequiredType );
		Token.PropertyFlags &= ~CPF_OutParm;
		EmitConstant( Token );
	}
	else if( Token.Matches("(") )
	{
		// Parenthesis. Recursion will handle all error checking.
		if( !CompileExpr( RequiredType, NULL, &Token ) )
			appThrowf( "Bad or missing expression in parenthesis" );
		RequireSymbol(")","expression");
		if( Token.Type == CPT_None )
			appThrowf( "Bad or missing expression in parenthesis" );
	}
	else if
	((	Token.TokenName==NAME_Byte
	||	Token.TokenName==NAME_Int
	||	Token.TokenName==NAME_Bool
	||	Token.TokenName==NAME_Float
	||	Token.TokenName==NAME_Name
	||	Token.TokenName==NAME_String
	||	Token.TokenName==NAME_Struct
	||	Token.TokenName==NAME_Vector
	||	Token.TokenName==NAME_Rotator )
	&&	MatchSymbol("(") )
	{
		// An explicit type conversion, so get source type.
		FPropertyBase ToType(CPT_None);
		if( Token.TokenName==NAME_Vector )
		{
			ToType = FPropertyBase( FindObjectChecked<UStruct>( ANY_PACKAGE, "Vector"  ));
		}
		else if( Token.TokenName==NAME_Rotator )
		{
			ToType = FPropertyBase( FindObjectChecked<UStruct>( ANY_PACKAGE, "Rotator" ));
		}
		else
		{
			EPropertyType T
			=	Token.TokenName==NAME_Byte				? CPT_Byte
			:	Token.TokenName==NAME_Int				? CPT_Int
			:	Token.TokenName==NAME_Bool				? CPT_Bool
			:	Token.TokenName==NAME_Float				? CPT_Float
			:	Token.TokenName==NAME_Name				? CPT_Name
			:	Token.TokenName==NAME_String			? CPT_String
			:	Token.TokenName==NAME_Struct			? CPT_Struct
			:   CPT_None;
			check(T!=CPT_None);
			ToType = FPropertyBase( T );
		}
		//caveat: General struct constants aren't supported.!!

		// Get destination type.
		FToken FromType;
		CompileExpr( FPropertyBase(CPT_None), *Token.TokenName, &FromType );
		if( FromType.Type == CPT_None )
			appThrowf("'%s' conversion: Bad or missing expression",*Token.TokenName);

		// Can we perform this explicit conversion?
		DWORD Conversion = GetConversion( ToType, FromType );
		if( Conversion != EX_Max )
		{
			// Perform conversion.
			FRetryPoint HighRetry; InitRetry(HighRetry);
			Writer << (EExprToken)(Conversion & CONVERT_MASK);
			CodeSwitcheroo(LowRetry,HighRetry);
			Token = FToken(ToType);
		}
		else if( ToType.Type == FromType.Type )
			appThrowf( "No need to cast '%s' to itself", *FName((EName)ToType.Type) );
		else
			appThrowf( "Can't convert '%s' to '%s'", *FName((EName)FromType.Type), *FName((EName)ToType.Type) );

		// The cast is no longer an l-value.
		Token.PropertyFlags &= ~CPF_OutParm;
		if( !MatchSymbol(")") )
			appThrowf( "Missing ')' in type conversion" );
	}
	else if( Token.TokenName==NAME_Self )
	{
		// Special Self context expression.
		CheckAllow("'self'",ALLOW_Instance);		
		Writer << EX_Self;
		Token = FToken(FPropertyBase( Class ));
	}
	else if( CompileDynamicCast( FToken(Token), Token) )
	{
		// Successfully compiled a dynamic object cast.
	}
	else if( CompileFieldExpr( TopNode, RequiredType, Token, Token, 1, (TopNest->Allow&ALLOW_Instance)!=0 ) )
	{
		// We successfully parsed a variable or function expression.
	}
	else
	{
		// This doesn't match an expression, so put it back.
		UngetToken( Token );
		(FPropertyBase&)Token = FPropertyBase( CPT_None );
	}

	// Intercept member selection operator for objects.
	for( ; ; )
	{
		if( Token.ArrayDim!=1 )
		{
			// If no array handler, we're done.
			if( !MatchSymbol("[") )
				break;

			// Emit array token.
			FRetryPoint HighRetry; InitRetry(HighRetry);
			Writer << EX_ArrayElement;

			// Emit index expression.
			Token.ArrayDim = 1;
			CompileExpr( FPropertyBase(CPT_Int), "array index" );
			if( !MatchSymbol("]") )
				appThrowf( "%s is an array; expecting ']'", Token.Identifier );

			// Emit element size and array dimension.
			CodeSwitcheroo( LowRetry, HighRetry );
		}
		else if( Token.Type==CPT_Struct && MatchSymbol(".") )
		{
			// Get member.
			check(Token.Struct!=NULL);
			FToken Tag; GetToken(Tag);
			UProperty* Member=NULL;
			for( TFieldIterator<UProperty> It(Token.Struct); It && !Member; ++It )
				if( It->GetFName() == Tag.TokenName )
					Member = *It;
			if( !Member )
				appThrowf( "Unknown member '%s' in struct '%s'", Tag.Identifier, Token.Struct->GetName() );
			DWORD OriginalFlags      = Token.PropertyFlags;
			(FPropertyBase&)Token    = FPropertyBase( Member );
			Token.PropertyFlags      = OriginalFlags | (Token.PropertyFlags & CPF_PropagateFromStruct);

			// Write offset.
			FRetryPoint HighRetry; InitRetry(HighRetry);
			Writer << EX_StructMember;
			Writer << Member;
			CodeSwitcheroo( LowRetry, HighRetry );
		}
		else if( Token.Type==CPT_ObjectReference && MatchSymbol(".") )
		{
			// Compile an object context expression.
			check(Token.PropertyClass!=NULL);
			FToken OriginalToken = Token;

			// Add a shallow dependency.
			Class->AddDependency( OriginalToken.PropertyClass, 0 );

			// Handle special class default and static members.
			if( Token.PropertyClass==UClass::StaticClass )
			{
				check(Token.MetaClass);
				Class->AddDependency( Token.MetaClass, 0 );
				if( PeekIdentifier("default") || PeekIdentifier("static") )
				{
					// Write context token.
					FRetryPoint HighRetry; InitRetry(HighRetry);
					Writer << EX_ClassContext;
					CodeSwitcheroo( LowRetry, HighRetry );

					// Compile class default context expression.
					GetToken(Token);
					FRetryPoint ContextStart; InitRetry(ContextStart);
					if( !CompileFieldExpr( Token.MetaClass, RequiredType, FToken(Token), Token, 0, 0 ) )
						appThrowf( "'%s': Bad context expression", Token.MetaClass->GetName() );

					// Insert skipover info for handling null contexts.
					FRetryPoint ContextEnd; InitRetry(ContextEnd);
					_WORD wSkip = TopNode->Script.Num() - ContextStart.CodeTop; Writer << wSkip;
					EmitSize( Token.GetSize(), "Context expression" );
					CodeSwitcheroo( ContextStart, ContextEnd );
					continue;
				}
			}

			// Emit object context override token.
			FRetryPoint HighRetry; InitRetry(HighRetry);
			Writer << EX_Context;
			CodeSwitcheroo( LowRetry, HighRetry );

			// Get the context variable or expression.
			FToken TempToken;
			GetToken(TempToken);

			// Compile a variable or function expression.
			FRetryPoint ContextStart; InitRetry(ContextStart);
			if(	!CompileFieldExpr( OriginalToken.PropertyClass, RequiredType, FToken(TempToken), Token, 0, 1 ) )
				appThrowf( "Unrecognized member '%s' in class '%s'", TempToken.Identifier, OriginalToken.PropertyClass->GetName() );

			// Insert skipover info for handling null contexts.
			FRetryPoint ContextEnd; InitRetry(ContextEnd);
			_WORD wSkip = TopNode->Script.Num() - ContextStart.CodeTop; Writer << wSkip;
			EmitSize( Token.GetSize(), "Context expression" );
			CodeSwitcheroo( ContextStart, ContextEnd );
		}
		else break;
	}

	// See if the following character is an operator.
	Test:
	FToken OperToken;
	INT Precedence=0, BestMatch=0, Matches=0, NumParms=3;
	TArray<UFunction*>OperLinks;
	UFunction* BestOperLink;
	UBOOL IsPreOperator = Token.Type==CPT_None;
	DWORD RequiredFunctionFlags = (IsPreOperator ? FUNC_PreOperator : 0) | FUNC_Operator;
	if( GetToken(OperToken,NULL,1) )
	{
		if( OperToken.TokenName != NAME_None )
		{
			// Build a list of matching operators.
			for( INT i=NestLevel-1; i>=1; i-- )
			{
				for( TFieldIterator<UFunction> It(Nest[i].Node); It; ++It )
				{
					if
					(	It->FriendlyName==OperToken.TokenName
					&&	RequiredFunctionFlags==(It->FunctionFlags & (FUNC_PreOperator|FUNC_Operator)) )
					{
						// Add this operator to the list.
						OperLinks.AddItem( *It );
						Precedence = It->OperPrecedence;
						NumParms   = Min(NumParms,(INT)It->NumParms);
					}
				}
			}

			// See if we got a valid operator, and if we want to handle it at the current precedence level.
			if( OperLinks.Num()>0 && Precedence<MaxPrecedence )
			{
				// Compile the second expression.
				FRetryPoint MidRetry; InitRetry(MidRetry);
				FPropertyBase NewRequiredType(CPT_None);
				FToken NewResultType;
				if( NumParms==3 || IsPreOperator )
				{
					char NewErrorTag[80];
					appSprintf( NewErrorTag, "Following '%s'", *OperToken.TokenName );
					CompileExpr( NewRequiredType, NewErrorTag, &NewResultType, Precedence, &Token );
					if( NewResultType.Type == CPT_None )
						appThrowf( "Bad or missing expression after '%s'", *OperToken.TokenName );
				}

				// Figure out which operator overload is best.
				BestOperLink = NULL;
				//AddResultText("Oper %s:\r\n",OperLinks[0]->Node().Name());
				UBOOL AnyLeftValid=0, AnyRightValid=0;
				for( INT i=0; i<OperLinks.Num(); i++ )
				{
					// See how good a match the first parm is.
					UFunction*  Node      = OperLinks(i);
					INT			ThisMatch = 0;
					TFieldIterator<UProperty> It(Node);

					if( Node->NumParms==3 || !IsPreOperator )
					{
						// Check match of first parm.
						UProperty* Parm1 = *It; ++It;
						//AddResultText("Left  (%s->%s): ",FName(Token.Type)(),FName(Parm1.Type)());
						INT Cost         = ConversionCost(FPropertyBase(Parm1),Token);
						ThisMatch        = Cost;
						AnyLeftValid     = AnyLeftValid || Cost!=MAXINT;
					}

					if( Node->NumParms == 3 || IsPreOperator )
					{
						// Check match of second parm.
						UProperty* Parm2 = *It; ++It;
						//AddResultText("Right (%s->%s): ",FName(NewResultType.Type)(),FName(Parm2.Type)());
						INT Cost         = ConversionCost(FPropertyBase(Parm2),NewResultType);
						ThisMatch        = Max(ThisMatch,Cost);
						AnyRightValid    = AnyRightValid || Cost!=MAXINT;
					}

					if( (!BestOperLink || ThisMatch<BestMatch) && (Node->NumParms==NumParms) )
					{
						// This is the best match.
						BestOperLink = OperLinks(i);
						BestMatch    = ThisMatch;
						Matches      = 1;
					}
					else if( ThisMatch == BestMatch ) Matches++;
				}
				if( BestMatch == MAXINT )
				{
					if
					(	(appStrcmp(*OperToken.TokenName,"==")==0 || appStrcmp(*OperToken.TokenName,"!=")==0)
					&&	Token.Type==CPT_Struct
					&&	NewResultType.Type==CPT_Struct
					&&	Token.Struct==NewResultType.Struct )
					{
						// Special-case struct binary comparison operators.
						FRetryPoint HighRetry; InitRetry(HighRetry);
						if( appStrcmp(*OperToken.TokenName,"==")==0 )
							Writer << EX_StructCmpEq;
						else
							Writer << EX_StructCmpNe;
						Writer << Token.Struct;
						CodeSwitcheroo( LowRetry, HighRetry );
						Token = FToken(FPropertyBase(CPT_Bool));
						goto Test;
					}
					else if( AnyLeftValid && !AnyRightValid )
						appThrowf( "Right type is incompatible with '%s'", *OperToken.TokenName );
					else if( AnyRightValid && !AnyLeftValid )
						appThrowf( "Left type is incompatible with '%s'", *OperToken.TokenName );
					else
						appThrowf( "Types are incompatible with '%s'", *OperToken.TokenName );
				}
				else if( Matches > 1 )
				{
					appThrowf( "Operator '%s': Can't resolve overload (%i matches of quality %i)", *OperToken.TokenName, Matches, BestMatch );
				}
				else
				{
					// Now BestOperLink points to the operator we want to use, and the code stream
					// looks like:
					//
					//       |LowRetry| Expr1 |MidRetry| Expr2
					//
					// Here we carefully stick any needed expression conversion operators into the
					// code stream, and swap everything until we end up with:
					//
					// |LowRetry| Oper [Conv1] Expr1 |MidRetry| [Conv2] Expr2 EX_EndFunctionParms

					// Get operator parameter pointers.
					check(BestOperLink);
					TFieldIterator<UProperty> It(BestOperLink);
					UProperty* OperParm1 = *It;
					if( !IsPreOperator )
						++It;
					UProperty* OperParm2 = *It;
					UProperty* OperReturn = BestOperLink->GetReturnProperty();
					check(OperReturn);

					// Convert Expr2 if necessary.
					if( BestOperLink->NumParms==3 || IsPreOperator )
					{
						if( OperParm2->PropertyFlags & CPF_OutParm )
						{
							// Note that this expression has a side-effect.
							GotAffector = 1;
						}
						if( NewResultType.Type != FPropertyBase(OperParm2).Type )
						{
							// Emit conversion.
							FRetryPoint HighRetry; InitRetry(HighRetry);
							Writer << (EExprToken)(GetConversion(FPropertyBase(OperParm2),NewResultType) & CONVERT_MASK);
							CodeSwitcheroo(MidRetry,HighRetry);
						}
						if( OperParm2->PropertyFlags & CPF_SkipParm )
						{
							// Emit skip expression for short-circuit operators.
							FRetryPoint HighRetry; InitRetry(HighRetry);
							_WORD wOffset = 1 + HighRetry.CodeTop - MidRetry.CodeTop;
							Writer << EX_Skip;
							Writer << wOffset;
							CodeSwitcheroo(MidRetry,HighRetry);
						}
					}

					// Convert Expr1 if necessary.
					if( !IsPreOperator )
					{
						if( OperParm1->PropertyFlags & CPF_OutParm )
						{
							// Note that this expression has a side-effect.
							GotAffector = 1;
						}
						if( Token.Type != FPropertyBase(OperParm1).Type  )
						{
							// Emit conversion.
							FRetryPoint HighRetry; InitRetry(HighRetry);
							Writer << (EExprToken)(GetConversion(FPropertyBase(OperParm1),Token) & CONVERT_MASK);
							CodeSwitcheroo(LowRetry,HighRetry);
						}
					}

					// Emit the operator function call.			
					FRetryPoint HighRetry; InitRetry(HighRetry);
					EmitStackNodeLinkFunction( BestOperLink, 1, 0 );
					CodeSwitcheroo(LowRetry,HighRetry);

					// End of call.
					Writer << EX_EndFunctionParms;

					// Update the type with the operator's return type.
					Token = FPropertyBase(OperReturn);
					Token.PropertyFlags &= ~CPF_OutParm;
					goto Test;
				}
			}
		}
	}
	UngetToken(OperToken);

	// Verify that we got an expression.
	if( Token.Type==CPT_None && RequiredType.Type!=CPT_None )
	{
		// Got nothing.
		if( ErrorTag )
			appThrowf( "Bad or missing expression in %s", ErrorTag );
		if( ResultToken )
			*ResultToken = Token;
		return 0;
	}

	// Make sure the type is correct.
	if( !RequiredType.MatchesType(Token,0) )
	{
		// Can we perform an automatic conversion?
		DWORD Conversion = GetConversion( RequiredType, Token );
		if( RequiredType.PropertyFlags & CPF_OutParm )
		{
			// If the caller wants an l-value, we can't do any conversion.
			if( ErrorTag )
			{
				if( Token.TokenType == TOKEN_Const )
					appThrowf( "Expecting a variable, not a constant" );
				else if( Token.PropertyFlags & CPF_Const )
					appThrowf( "Const mismatch in Out variable %s", ErrorTag );
				else
					appThrowf( "Type mismatch in Out variable %s", ErrorTag );
			}
			if( ResultToken )
				*ResultToken = Token;
			return -1;
		}
		else if( RequiredType.ArrayDim!=1 || Token.ArrayDim!=1 )
		{
			// Type mismatch, and we can't autoconvert arrays.
			if( ErrorTag )
				appThrowf( "Array mismatch in %s", ErrorTag );
			if( ResultToken )
				*ResultToken = Token;
			return -1;
		}
		else if( RequiredType.Type==CPT_String && Token.Type==CPT_String )
		{
			// Perform an automatic string conversion.
			FRetryPoint HighRetry; InitRetry(HighRetry);
			Writer << EX_ResizeString;
			BYTE b = RequiredType.StringSize; Writer << b;
			CodeSwitcheroo(LowRetry,HighRetry);
			Token.StringSize = RequiredType.StringSize;
		}
		else if
		(	( (RequiredType.PropertyFlags & CPF_CoerceParm) ? (Conversion!=EX_Max) : (Conversion & AUTOCONVERT) )
		&&	(RequiredType.Type!=CPT_Byte || RequiredType.Enum==NULL) )
		{
			// Perform automatic conversion or coercion.
			FRetryPoint HighRetry; InitRetry(HighRetry);
			Writer << (EExprToken)(GetConversion( RequiredType, Token ) & CONVERT_MASK);
			CodeSwitcheroo(LowRetry,HighRetry);
			Token.PropertyFlags &= ~CPF_OutParm;
			Token = FToken(FPropertyBase((EPropertyType)RequiredType.Type));
		}
		else
		{
			// Type mismatch.
			if( ErrorTag )
				appThrowf( "Type mismatch in %s", ErrorTag );
			if( ResultToken )
				*ResultToken = Token;
			return -1;
		}
	}

	if( ResultToken )
		*ResultToken = Token;
	return Token.Type != CPT_None;
	unguard;
}

/*-----------------------------------------------------------------------------
	Nest information.
-----------------------------------------------------------------------------*/

//
// Return the name for a nest type.
//
const char *FScriptCompiler::NestTypeName( ENestType NestType )
{
	guard(FScriptCompiler::NestTypeName);
	switch( NestType )
	{
		case NEST_None:		return "Global Scope";
		case NEST_Class:	return "Class";
		case NEST_State:	return "State";
		case NEST_Function:	return "Function";
		case NEST_If:		return "If";
		case NEST_Loop:		return "Loop";
		case NEST_Switch:	return "Switch";
		case NEST_For:		return "For";
		case NEST_ForEach:	return "ForEach";
		default:			return "Unknown";
	}
	unguard;
}

//
// Make sure that a particular kind of command is allowed on this nesting level.
// If it's not, issues a compiler error referring to the token and the current
// nesting level.
//
void FScriptCompiler::CheckAllow( const char *Thing, int AllowFlags )
{
	guard(FScriptCompiler::CheckAllow);
	if( (TopNest->Allow & AllowFlags) != AllowFlags )
	{
		if( TopNest->NestType==NEST_None )
		{
			appThrowf( "%s is not allowed before the Class definition", Thing );
		}
		else
		{
			appThrowf( "%s is not allowed here", Thing );
		}
	}
	if( AllowFlags & ALLOW_Cmd )
	{
		// Don't allow variable declarations after commands.
		TopNest->Allow &= ~(ALLOW_VarDecl | ALLOW_Function | ALLOW_Ignores);
	}
	unguard;
}

//
// Check that a specified object is accessible from
// this object's scope.
//
void FScriptCompiler::CheckInScope( UObject* Obj )
{
	guard(FScriptCompiler::CheckInScope);
	check(Obj);
	//INT i;
	//if( !Class->PackageImports.FindItem( Obj->GetParent()->GetFName(), i ) )
	//	appThrowf(" '%s' is not accessible to this script", Obj->GetFullName() );
	unguard;
}

/*-----------------------------------------------------------------------------
	Nest management.
-----------------------------------------------------------------------------*/

//
// Increase the nesting level, setting the new top nesting level to
// the one specified.  If pushing a function or state and it overrides a similar
// thing declared on a lower nesting level, verifies that the override is legal.
//
void FScriptCompiler::PushNest( ENestType NestType, FName ThisName, UStruct* InNode )
{
	guard(FScriptCompiler::PushNest);

	// Defaults.
	UStruct* PrevTopNode = TopNode;
	UStruct* PrevNode = NULL;
	DWORD PrevAllow = 0;

	if( Pass==0 && (NestType==NEST_State || NestType==NEST_Function) )
		for( TFieldIterator<UField> It(TopNode); It && It->GetParent()==TopNode; ++It )
			if( It->GetFName()==ThisName )
				appThrowf( "'%s' conflicts with '%s'", *ThisName, It->GetFullName() );

	// Update pointer to top nesting level.
	TopNest					= &Nest[NestLevel++];
	TopNode					= NULL;
	TopNest->Node			= InNode;
	TopNest->NestType		= NestType;
	TopNest->iCodeChain		= INDEX_NONE;
	TopNest->SwitchType		= FPropertyBase(CPT_None);
	TopNest->FixupList		= NULL;
	TopNest->LabelList		= NULL;

	// Init fixups.
	for( INT i=0; i<FIXUP_MAX; i++ )
		TopNest->Fixups[i] = MAXWORD;

	// Prevent overnesting.
	if( NestLevel >= MAX_NEST_LEVELS )
		appThrowf( "Maximum nesting limit exceeded" );

	// Inherit info from stack node above us.
	INT IsNewNode = NestType==NEST_Class || NestType==NEST_State || NestType==NEST_Function;
	if( NestLevel > 1 )
	{
		if( Pass == 1 )
		{
			if( !IsNewNode )
				TopNest->Node = TopNest[-1].Node;
			TopNode = TopNest[0].Node;
		}
		else
		{
			if( IsNewNode )
			{
				// Create a new stack node.
				if( NestType==NEST_Class )
				{
					TopNest->Node = TopNode = Class;
					Class->ProbeMask		= 0;
					Class->IgnoreMask		= ~(QWORD)0;
					Class->VfHash           = NULL;
					Class->LabelTableOffset = MAXWORD;
				}
				else if( NestType==NEST_State )
				{
					UState* State;
					TopNest->Node = TopNode = State = new(PrevTopNode ? (UObject*)PrevTopNode : (UObject*)Class, ThisName, RF_Public)UState( NULL );
					State->ProbeMask		= 0;
					State->IgnoreMask		= ~(QWORD)0;
					State->VfHash           = NULL;
					State->LabelTableOffset = MAXWORD;
				}
				else if( NestType==NEST_Function )
				{
					UFunction* Function;
					TopNest->Node = TopNode = Function = new(PrevTopNode ? (UObject*)PrevTopNode : (UObject*)Class, ThisName, RF_Public)UFunction( NULL );
					Function->RepOffset         = MAXWORD;
					Function->ReturnValueOffset = MAXWORD;
				}

				// Init general info.
				TopNode->FriendlyName		= ThisName;
				TopNode->TextPos			= INDEX_NONE;
				TopNode->Line				= MAXWORD;
			}
			else
			{
				// Use the existing stack node.
				TopNest->Node = TopNest[-1].Node;
				TopNode = TopNest->Node;
			}
		}
		check(TopNode!=NULL);
		PrevNode  = TopNest[-1].Node;
		PrevAllow = TopNest[-1].Allow;
	}

	// NestType specific logic.
	switch( NestType )
	{
		case NEST_None:
			check(PrevNode==NULL);
			TopNest->Allow = ALLOW_Class;
			break;

		case NEST_Class:
			check(ThisName!=NAME_None);
			check(PrevNode==NULL);
			TopNest->Allow = ALLOW_VarDecl | ALLOW_Function | ALLOW_State | ALLOW_Ignores | ALLOW_Instance;
			break;

		case NEST_State:
			check(ThisName!=NAME_None);
			check(PrevNode!=NULL);
			TopNest->Allow = ALLOW_Function | ALLOW_Label | ALLOW_StateCmd | ALLOW_Ignores | ALLOW_Instance;
			if( Pass==0 )
			{
				TopNode->Next = PrevNode->Children;
				PrevNode->Children = TopNode;
			}
			break;

		case NEST_Function:
			check(ThisName!=NAME_None);
			check(PrevNode!=NULL);
			TopNest->Allow = ALLOW_VarDecl | ALLOW_Return | ALLOW_Cmd | ALLOW_Label;
			if( !(Cast<UFunction>(TopNode)->FunctionFlags & FUNC_Static) )
				TopNest->Allow |= ALLOW_Instance;
			if( Pass==0 )
			{
				TopNode->Next = PrevNode->Children;
				PrevNode->Children = TopNode;
			}
			break;

		case NEST_If:
			check(ThisName==NAME_None);
			check(PrevNode!=NULL);
			TopNest->Allow = ALLOW_ElseIf | (PrevAllow & (ALLOW_Cmd|ALLOW_Label|ALLOW_Break|ALLOW_StateCmd|ALLOW_Return|ALLOW_Instance));
			break;

		case NEST_Loop:
			check(ThisName==NAME_None);
			check(PrevNode!=NULL);
			TopNest->Allow = ALLOW_Break | (PrevAllow & (ALLOW_Cmd|ALLOW_Label|ALLOW_Break|ALLOW_StateCmd|ALLOW_Return|ALLOW_Instance));
			break;

		case NEST_Switch:
			check(ThisName==NAME_None);
			check(PrevNode!=NULL);
			TopNest->Allow = ALLOW_Case | ALLOW_Default | (PrevAllow & (ALLOW_StateCmd|ALLOW_Return|ALLOW_Instance));
			break;

		case NEST_For:
			check(ThisName==NAME_None);
			check(PrevNode!=NULL);
			TopNest->Allow = ALLOW_Break | (PrevAllow & (ALLOW_Cmd|ALLOW_Label|ALLOW_Break|ALLOW_StateCmd|ALLOW_Return|ALLOW_Instance));
			break;

		case NEST_ForEach:
			check(ThisName==NAME_None);
			check(PrevNode!=NULL);
			TopNest->Allow = ALLOW_Break | (PrevAllow & (ALLOW_Cmd|ALLOW_Label|ALLOW_Break|ALLOW_Return|ALLOW_Instance));
			break;

		default:
			appThrowf( "Internal error in PushNest, type %i", NestType );
			break;
	}
	unguard;
}

//
// Decrease the nesting level and handle any errors that result.
//
void FScriptCompiler::PopNest( ENestType NestType, const char* Descr )
{
	guard(FScriptCompiler::PopNest);

	if( TopNode )
		if( TopNode->Script.Num() > 65534 )
			appThrowf( "Code space for %s overflowed by %i bytes", TopNode->GetName(), TopNode->Script.Num() - 65534 );

	// Validate the nesting state.
	if( NestLevel <= 0 )
		appThrowf( "Unexpected '%s' at global scope", Descr, NestTypeName(NestType) );
	else if( NestType==NEST_None )
		NestType = TopNest->NestType;
	else if( TopNest->NestType!=NestType )
		appThrowf( "Unexpected end of %s in '%s' block", Descr, NestTypeName(TopNest->NestType) );

	if( Pass == 0 )
	{
		// Remember code position.
		guard(CleanupParse);
		if( NestType==NEST_State || NestType==NEST_Function )
		{
			TopNode->TextPos    = InputPos;
			TopNode->Line       = InputLine;
		}
		else if( NestType!=NEST_Class )
		{
			appErrorf( "Bad first pass NestType %i", NestType );
		}
		FArchive DummyAr;
		TopNode->LinkOffsets( DummyAr );
		unguard;
	}
	else
	{
		// If ending a state, process labels.
		guard(CleanupCompile);
		if( NestType==NEST_State )
		{
			// Emit stop command.
			Writer << EX_EndCode;

			// Write all labels to code.
			if( TopNest->LabelList )
			{
				// Make sure the label table entries are aligned.
				while( (TopNode->Script.Num() & 3) != 3 )
					Writer << EX_Nothing;
				Writer << EX_LabelTable;

				// Remember code offset.
				UState* State = CastChecked<UState>( TopNode );
				State->LabelTableOffset = TopNode->Script.Num();

				// Write out all label entries.
				for( FLabelRecord* LabelRecord = TopNest->LabelList; LabelRecord; LabelRecord=LabelRecord->Next )
					Writer << *LabelRecord;

				// Write out empty record.
				FLabelEntry Entry(NAME_None,MAXWORD);
				Writer << Entry;
			}
		}
		else if( NestType==NEST_Function )
		{
			// Emit return.
			UFunction* TopFunction = CastChecked<UFunction>( TopNode );
			if( !(TopFunction->FunctionFlags & FUNC_Intrinsic) )
			{
				Writer << EX_Return;
				Writer << EX_EndCode;
			}
		}
		else if( NestType==NEST_Switch )
		{
			if( TopNest->Allow & ALLOW_Case )
			{
				// No default was specified, so emit end-of-case marker.
				EmitChainUpdate(TopNest);
				Writer << EX_Case;
				_WORD W=MAXWORD; Writer << W;
			}

			// Here's the big end.
			TopNest->SetFixup(FIXUP_SwitchEnd,TopNode->Script.Num());
		}
		else if( NestType==NEST_If )
		{
			if( MatchIdentifier("Else") )
			{
				// Send current code to the end of the if block.
				Writer << EX_Jump;
				EmitAddressToFixupLater( TopNest, FIXUP_IfEnd, NAME_None );

				// Update previous If's next-address.
				EmitChainUpdate( TopNest );
				if( MatchIdentifier("If") )
				{
					// ElseIf.
					CheckAllow( "'Else If'", ALLOW_ElseIf );

					// Jump to next evaluator if expression is false.
					Writer << EX_JumpIfNot;
					EmitAddressToChainLater( TopNest );

					// Compile boolean expr.
					RequireSymbol( "(", "'Else If'" );
					CompileExpr( FPropertyBase(CPT_Bool), "'Else If'" );
					RequireSymbol( ")", "'Else If'" );

					// Handle statements.
					if( !MatchSymbol("{") )
					{
						CompileStatement();
						PopNest( NEST_If, "'ElseIf'" );
					}
				}
				else
				{
					// Else.
					CheckAllow( "'Else'", ALLOW_ElseIf );

					// Prevent further ElseIfs.
					TopNest->Allow &= ~(ALLOW_ElseIf);

					// Handle statements.
					if( !MatchSymbol("{") )
					{
						CompileStatement();
						PopNest( NEST_If, "'Else'" );
					}
				}
				return;
			}
			else
			{
				// Update last link, if any:
				EmitChainUpdate( TopNest );

				// Here's the big end.
				TopNest->SetFixup( FIXUP_IfEnd, TopNode->Script.Num() );
			}
		}
		else if( NestType==NEST_For )
		{
			// Compile the incrementor expression here.
			FRetryPoint Retry; InitRetry(Retry);
			PerformRetry(TopNest->ForRetry,0,1);		
				CompileAffector();
			PerformRetry(Retry,0,1);

			// Jump back to start of loop.
			Writer << EX_Jump;
			EmitAddressToFixupLater(TopNest,FIXUP_ForStart,NAME_None);

			// Here's the end of the loop.
			TopNest->SetFixup(FIXUP_ForEnd,TopNode->Script.Num());
		}
		else if( NestType==NEST_ForEach )
		{
			// Perform next iteration.
			Writer << EX_IteratorNext;

			// Here's the end of the loop.
			TopNest->SetFixup( FIXUP_IteratorEnd, TopNode->Script.Num() );
			Writer << EX_IteratorPop;
		}
		else if( NestType==NEST_Loop )
		{
			if( MatchIdentifier("Until") )
			{
				// Jump back to start of loop.
				Writer << EX_JumpIfNot;
				EmitAddressToFixupLater( TopNest, FIXUP_LoopStart, NAME_None );

				// Compile boolean expression.
				RequireSymbol( "(", "'Until'" );
				CompileExpr( FPropertyBase(CPT_Bool), "'Until'" );
				RequireSymbol( ")", "'Until'" );

				// Here's the end of the loop.
				TopNest->SetFixup( FIXUP_LoopEnd, TopNode->Script.Num() );
			}
			else if( MatchIdentifier("While") )
			{
				// Not allowed here.
				appThrowf( "The loop syntax is Do...Until, not Do...While" );
			}
			else
			{
				// Jump back to start of loop.
				Writer << EX_Jump;
				EmitAddressToFixupLater( TopNest, FIXUP_LoopStart, NAME_None );

				// Here's the end of the loop.
				TopNest->SetFixup( FIXUP_LoopEnd, TopNode->Script.Num() );
			}
		}
		unguard;

		// Perform all code fixups.
		guard(Fixup);
		for( FNestFixupRequest* Fixup = TopNest->FixupList; Fixup!=NULL; Fixup=Fixup->Next )
		{
			if( Fixup->Type == FIXUP_Label )
			{
				// Fixup a local label.
				for( FLabelRecord* LabelRecord = TopNest->LabelList; LabelRecord; LabelRecord=LabelRecord->Next )
				{
					if( LabelRecord->Name == Fixup->Name )
					{
						*(_WORD*)&TopNode->Script(Fixup->iCode) = LabelRecord->iCode;
						break;
					}
				}
				if( LabelRecord == NULL )
					appThrowf( "Label '%s' not found in this block of code", *Fixup->Name );
			}
			else
			{
				// Fixup a code structure address.
				if( TopNest->Fixups[Fixup->Type] == MAXWORD )
					appThrowf( "Internal fixup error %i", Fixup->Type );
				*(_WORD*)&TopNode->Script(Fixup->iCode) = TopNest->Fixups[Fixup->Type];
			}
		}
		unguard;
	}

	// Make sure there's no dangling chain.
	check(TopNest->iCodeChain==INDEX_NONE);

	// Pop the nesting level.
	NestType = TopNest->NestType;
	NestLevel--;
	TopNest--;
	TopNode	= TopNest->Node;

	// Update allow-flags.
	if( NestType==NEST_Function )
	{
		// Don't allow variable declarations after functions.
		TopNest->Allow &= ~(ALLOW_VarDecl);
	}
	else if( NestType == NEST_State )
	{
		// Don't allow variable declarations after states.
		TopNest->Allow &= ~(ALLOW_VarDecl);
	}
	unguard;
}

//
// Find the highest-up nest info of a certain type.
// Used (for example) for associating Break statements with their Loops.
//
INT FScriptCompiler::FindNest( ENestType NestType )
{
	guard(FScriptCompiler::FindNest);
	for( int i=NestLevel-1; i>0; i-- )
		if( Nest[i].NestType == NestType )
			return i;
	return -1;
	unguard;
}

/*-----------------------------------------------------------------------------
	Compiler directives.
-----------------------------------------------------------------------------*/

//
// Process a compiler directive.
//
void FScriptCompiler::CompileDirective()
{
	guard(FScriptCompiler::ProcessCompilerDirective);
	FToken Directive;

	if( !GetIdentifier(Directive) )
	{
		appThrowf( "Missing compiler directive after '#'" );
	}
	else if( Directive.Matches("Error") )
	{
		appThrowf( "#Error directive encountered" );
	}
	else if( Directive.Matches("Call") || Directive.Matches("AlwaysCall") )
	{
		FToken Identifier;
		if( !GetRawToken(Identifier) )
			appThrowf("'Call': missing command line");
		if( Booting || Directive.Matches("AlwaysCall") )
		{
			UTextBuffer* Text = ImportObjectFromFile<UTextBuffer>( GObj.GetTransientPackage(), NAME_None, Identifier.String, GSystem );
			if( Text )
			{
				debugf( NAME_Log, "Calling %s", Identifier.String );
				char Temp[256]; const char* Data = *Text->Text;
				while( ParseLine( &Data, Temp, ARRAY_COUNT(Temp) ) )
				{
					GEditor->Bootstrapping++;
					GEditor->ParentContext = Class->GetParent();
					GEditor->SafeExec( Temp );
					GEditor->ParentContext = NULL;
					GEditor->Bootstrapping--;
				}
				delete Text;
			}
			else debugf( NAME_ExecWarning, "Macro file %s not found", Identifier.String );
		}
	}
	else if( Directive.Matches("Exec") || Directive.Matches("AlwaysExec") )
	{
		FToken Identifier;
		if( !GetRawToken(Identifier) )
			appThrowf("'#Exec': missing command line");

		if( Booting || Directive.Matches("AlwaysExec") )
		{
			GEditor->Bootstrapping++;
			GEditor->ParentContext = Class->GetParent();
			GEditor->SafeExec( Identifier.String );
			GEditor->ParentContext = NULL;
			GEditor->Bootstrapping--;
		}
	}
	else
	{
		appThrowf( "Unrecognized compiler directive %s", Directive.Identifier );
	}

	// Skip to end of line.
	char c;
	while( !IsEOL( c=GetChar() ) );
	if( c==0 ) UngetChar();
	unguard;
}

/*-----------------------------------------------------------------------------
	Variable declaration parser.
-----------------------------------------------------------------------------*/

//
// Parse one variable declaration and set up its properties in VarProperty.
// Returns pointer to the class property if success, or NULL if there was no variable 
// declaration to parse. Called during variable 'Dim' declaration and function 
// parameter declaration.
//
// If you specify a hard-coded name, that name will be used as the variable name (this
// is used for function return values, which are automatically called "ReturnValue"), and
// a default value is not allowed.
//
UBOOL FScriptCompiler::GetVarType
(
	UStruct*		Scope,
	FPropertyBase&	VarProperty,
	DWORD&			ObjectFlags,
	DWORD			Disallow,
	const char*		Thing
)
{
	guard(FScriptCompiler::GetVarType);
	check(Scope);
	FPropertyBase TypeDefType(CPT_None);
	UClass* TempClass;
	UBOOL IsVariable = 0;

	// Get flags.
	ObjectFlags=RF_Public;
	DWORD Flags=0;
	for( ; ; )
	{
		FToken Specifier;
		GetToken(Specifier);
		if( Specifier.Matches(NAME_Const) )
		{
			Flags      |= CPF_Const;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_Config) )
		{
			Flags      |= CPF_Config;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_GlobalConfig) )
		{
			Flags      |= CPF_GlobalConfig | CPF_Config;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_Localized) )
		{
			Flags      |= CPF_Localized;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_Travel) )
		{
			Flags      |= CPF_Travel;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_Private) )
		{
			ObjectFlags &= ~RF_Public;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_EditConst) )
		{
			Flags      |= CPF_EditConst;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_Input) )
		{
			Flags      |= CPF_Input;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_Transient) )
		{
			Flags      |= CPF_Transient;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_Intrinsic) )
		{
			Flags      |= CPF_Intrinsic;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_Out) )
		{
			Flags      |= CPF_OutParm;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_Export) )
		{
			Flags      |= CPF_ExportObject;
			IsVariable  = 1;
		}
		else if( Specifier.Matches(NAME_Skip) )
		{
			Flags     |= CPF_SkipParm;
			IsVariable = 1;
		}
		else if( Specifier.Matches(NAME_Coerce) )
		{
			Flags     |= CPF_CoerceParm;
			IsVariable = 1;
		}
		else if( Specifier.Matches(NAME_Optional) )
		{
			Flags     |= CPF_OptionalParm;
			IsVariable = 1;
		}
		else
		{
			UngetToken(Specifier);
			break;
		}
	}

	// Get variable type.
	FToken VarType;
	UField* Field;
	if( !GetIdentifier(VarType,1) )
	{
		if( !Thing )
			return 0;
		appThrowf( "%s: Missing variable type", Thing );
	}
	else if( VarType.Matches(NAME_Enum) )
	{
		// Compile an Enum definition and variable declaration here.
		VarProperty = FPropertyBase( CompileEnum(Scope) );
	}
	else if( VarType.Matches(NAME_Struct) )
	{
		// Compile a Struct definition and variable declaration here.
		VarProperty = FPropertyBase( CompileStruct(Scope) );
	}
	else if( VarType.Matches(NAME_Byte) )
	{
		// Intrinsic Byte type.
		VarProperty = FPropertyBase(CPT_Byte);
	}
	else if( VarType.Matches(NAME_Int) )
	{
		// Intrinsic Int type.
		VarProperty = FPropertyBase(CPT_Int);
	}
	else if( VarType.Matches(NAME_Bool) )
	{
		// Intrinsic Bool type.
		VarProperty = FPropertyBase(CPT_Bool);
	}
	else if( VarType.Matches(NAME_Float) )
	{
		// Intrinsic Real type
		VarProperty = FPropertyBase(CPT_Float);
	}
	else if( VarType.Matches(NAME_Name) )
	{
		// Intrinsic Name type.
		VarProperty = FPropertyBase(CPT_Name);
	}
	else if( VarType.Matches(NAME_String) )
	{
		// Intrinsic String type.
		VarProperty = FPropertyBase(CPT_String);

		if( !MatchSymbol("[") )
			appThrowf( "Missing string dimension, i.e. String[16]" );

		if( !VarType.Matches(NAME_String) )
			appThrowf( "Arrays dimension belongs after name, not type" );
		
		if( !GetConstInt(VarProperty.StringSize) )
			appThrowf( "%s: Missing string size",Thing?Thing:"Declaration" );

		if( VarProperty.StringSize<=0 || VarProperty.StringSize>MAX_STRING_CONST_SIZE )
			appThrowf( "%s: Illegal string size %i",Thing?Thing:"Declaration",VarProperty.StringSize );
		
		if( !MatchSymbol("]") )
			appThrowf( "%s: Missing ']'", Thing?Thing:"Declaration" );
	}
	else if( (TempClass = FindObject<UClass>( ANY_PACKAGE, VarType.Identifier ) ) != NULL )
	{
		// An object reference.
		CheckInScope( TempClass );
		VarProperty = FPropertyBase( TempClass );
		if( TempClass==UClass::StaticClass )
		{
			if( MatchSymbol("<") )
			{
				FToken Limitor;
				if( !GetIdentifier(Limitor) )
					appThrowf( "'class': Missing class limitor" );
				VarProperty.MetaClass = FindObject<UClass>( ANY_PACKAGE, Limitor.Identifier );
				if( !VarProperty.MetaClass )
					appThrowf( "'class': Limitor '%s' is not a class name", Limitor.Identifier );
				RequireSymbol( ">", "'class limitor'" );
			}
			else VarProperty.MetaClass = UObject::StaticClass;
		}
	}
	else if( (Field=FindField( Scope, VarType.Identifier ))!=NULL )
	{
		// In-scope enumeration or struct.
		if( Field->GetClass()==UEnum::StaticClass )
			VarProperty = FPropertyBase( CastChecked<UEnum>(Field) );
		else if( Field->GetClass()==UStruct::StaticClass )
			VarProperty = FPropertyBase( CastChecked<UStruct>(Field) );
		else
		{
			if( !Thing )
			{
				// Not recognized.
				UngetToken( VarType );
				return 0;
			}
			else appThrowf( "Unrecognized type '%s'", VarType.Identifier );
		}
	}
	else if( !Thing )
	{
		// Not recognized.
		UngetToken( VarType );
		return 0;
	}
	else appThrowf( "Unrecognized type '%s'", VarType.Identifier );

	// Set FPropertyBase info.
	VarProperty.PropertyFlags |= Flags;

	// Make sure the overrides are allowed here.
	if( VarProperty.PropertyFlags & Disallow )
		appThrowf( "Specified type modifiers not allowed here" );

	return 1;
	unguard;
}

UProperty* FScriptCompiler::GetVarNameAndDim
(
	UStruct*		Scope,
	FPropertyBase&	VarProperty,
	DWORD			ObjectFlags,
	UBOOL			NoArrays,
	UBOOL			IsFunction,
	const char*		HardcodedName,
	const char*		Thing,
	FName			Category,
	UBOOL			Skip
)
{
	guard(FScriptCompiler::GetVarNameAndDim);
	check(Scope);

	// Get varible name.
	FToken VarToken;
	if( HardcodedName )
	{
		// Hard-coded variable name, such as with return value.
		VarToken.TokenType = TOKEN_Identifier;
		appStrcpy( VarToken.Identifier, HardcodedName );
	}
	else if( !GetIdentifier(VarToken) ) 
		appThrowf( "Missing variable name" );

	// Make sure it doesn't conflict.
	if( !Skip )
	{
		UField* Existing = FindField( Scope, VarToken.Identifier );
		if( Existing && Existing->GetParent()==Scope )
			appThrowf( "%s: '%s' already defined", Thing, VarToken.Identifier );
	}

	// Get optional dimension immediately after name.
	if( MatchSymbol("[") )
	{
		if( NoArrays ) 
			appThrowf( "Arrays aren't allowed in this context" );

		if( VarProperty.Type == CPT_Bool )
			appThrowf( "Bool arrays are not allowed" );

		if( !GetConstInt(VarProperty.ArrayDim) )
			appThrowf( "%s %s: Bad or missing array size", Thing, VarToken.Identifier );

		if( VarProperty.ArrayDim<=1 && VarProperty.ArrayDim>MAX_ARRAY_SIZE )
			appThrowf( "%s %s: Illegal array size %i", Thing, VarToken.Identifier, VarProperty.ArrayDim );

		if( !MatchSymbol("]") )
			appThrowf( "%s %s: Missing ']'", Thing, VarToken.Identifier );
	}
	else if( MatchSymbol("(") )
	{
		appThrowf( "Use [] for arrays, not ()" );
	}

	// Add property.
	UProperty* NewProperty=NULL;
	if( !Skip )
	{
		UProperty* Prev=NULL;
		for( TFieldIterator<UProperty> It(Scope); It && It.GetStruct()==Scope; ++It )
			Prev = *It;
		if( VarProperty.Type==CPT_Byte )
		{
			NewProperty = new(Scope,VarToken.Identifier,ObjectFlags)UByteProperty;
			Cast<UByteProperty>(NewProperty)->Enum = VarProperty.Enum;
		}
		else if( VarProperty.Type==CPT_Int )
		{
			NewProperty = new(Scope,VarToken.Identifier,ObjectFlags)UIntProperty;
		}
		else if( VarProperty.Type==CPT_Bool )
		{
			NewProperty = new(Scope,VarToken.Identifier,ObjectFlags)UBoolProperty;
		}
		else if( VarProperty.Type==CPT_Float )
		{
			NewProperty = new(Scope,VarToken.Identifier,ObjectFlags)UFloatProperty;
		}
		else if( VarProperty.Type==CPT_ObjectReference )
		{
			check(VarProperty.PropertyClass);
			if( VarProperty.PropertyClass==UClass::StaticClass )
			{
				NewProperty = new(Scope,VarToken.Identifier,ObjectFlags)UClassProperty;
				Cast<UClassProperty>(NewProperty)->MetaClass = VarProperty.MetaClass;
			}
			else
			{
				NewProperty = new(Scope,VarToken.Identifier,ObjectFlags)UObjectProperty;
			}
			Cast<UObjectProperty>(NewProperty)->PropertyClass = VarProperty.PropertyClass;
		}
		else if( VarProperty.Type==CPT_Name )
		{
			NewProperty = new(Scope,VarToken.Identifier,ObjectFlags)UNameProperty;
		}
		else if( VarProperty.Type==CPT_String )
		{
			NewProperty = new(Scope,VarToken.Identifier,ObjectFlags)UStringProperty;
			Cast<UStringProperty>(NewProperty)->StringSize = VarProperty.StringSize;
		}
		else if( VarProperty.Type==CPT_Struct )
		{
			NewProperty = new(Scope,VarToken.Identifier,ObjectFlags)UStructProperty;
			Cast<UStructProperty>(NewProperty)->Struct = VarProperty.Struct;
		}
		else appErrorf( "Unknown property type %i", VarProperty.Type );
		NewProperty->ArrayDim      = VarProperty.ArrayDim;
		NewProperty->PropertyFlags = VarProperty.PropertyFlags;
		NewProperty->Category      = Category;
		if( Prev )
		{
			NewProperty->Next = Prev->Next;
			Prev->Next = NewProperty;
		}
		else
		{
			NewProperty->Next = Scope->Children;
			Scope->Children = NewProperty;
		}
	}
	return NewProperty;
	unguard;
}

//
// Compile a variable assignment statement.
//
void FScriptCompiler::CompileAssignment( const char* Tag )
{
	guard(FScriptCompiler::CompileAssignment);

	// Set up.
	FRetryPoint LowRetry; InitRetry(LowRetry);
	FToken RequiredType, VarToken;

	// Compile l-value expression.
	CompileExpr( FPropertyBase(CPT_None), "Assignment", &RequiredType );
	if( RequiredType.Type == CPT_None )
		appThrowf( "%s assignment: Missing left value", Tag );
	else if( !(RequiredType.PropertyFlags & CPF_OutParm) )
		appThrowf( "%s assignment: Left value is not a variable", Tag );
	else if( !MatchSymbol("=") )
		appThrowf( "%s: Missing '=' after %s", Tag );

	// Emit let.
	FRetryPoint HighRetry; InitRetry(HighRetry);
	EmitLet( RequiredType, Tag );

	// Switch around.
	CodeSwitcheroo(LowRetry,HighRetry);

	// Compile right value.
	RequiredType.PropertyFlags &= ~CPF_OutParm;
	CompileExpr( RequiredType, Tag );

	unguard;
}

//
// Try to compile an affector expression or assignment.
//
void FScriptCompiler::CompileAffector()
{
	guard(FScriptCompiler::CompileAffector);

	// Try to compile an affector expression or assignment.
	FRetryPoint LowRetry; InitRetry(LowRetry);
	GotAffector = 0;

	// Try to compile an expression here.
	FPropertyBase RequiredType(CPT_None);
	FToken ResultType;
	if( CompileExpr( RequiredType, NULL, &ResultType ) < 0 )
	{
		FToken Token;
		GetToken(Token);
		appThrowf( "'%s': Bad command or expression", Token.Identifier );
	}

	// Did we get a function call or a varible assignment?
	if( MatchSymbol("=") )
	{
		// Variable assignment.
		if( !(ResultType.PropertyFlags & CPF_OutParm) )
			appThrowf( "'=': Left value is not a variable" );

		// Compile right value.
		RequiredType = ResultType;
		RequiredType.PropertyFlags &= ~CPF_OutParm;
		CompileExpr( RequiredType, "'='" );

		// Emit the let.
		FRetryPoint HighRetry; InitRetry(HighRetry);
		EmitLet( ResultType, "'='" );
		CodeSwitcheroo(LowRetry,HighRetry);
	}
	else if( GotAffector )
	{
		// Function call or operators with outparms.
	}
	else if( ResultType.Type != CPT_None )
	{
		// Whatever expression we parsed had no effect.
		FToken Token;
		GetToken(Token);
		appThrowf( "'%s': Expression has no effect", Token.Identifier );
	}
	else
	{
		// Didn't get anything, so throw an error at the appropriate place.
		FToken Token;
		GetToken(Token);
		appThrowf( "'%s': Bad command or expression", Token.Identifier );
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Statement compiler.
-----------------------------------------------------------------------------*/

//
// Compile a declaration in Token. Returns 1 if compiled, 0 if not.
//
INT FScriptCompiler::CompileDeclaration( FToken& Token, UBOOL& NeedSemicolon )
{
	guard(FScriptCompiler::CompileDeclaration);
	if( Token.Matches(NAME_Class) && (TopNest->Allow & ALLOW_Class) )
	{
		// Start of a class block.
		guard(Class);
		CheckAllow("'Class'",ALLOW_Class);

		// Class name.
		if( !GetToken(Token) )
			appThrowf( "Missing class name" );
		if( !Token.Matches(Class->GetName()) )
			appThrowf( "Class must be named %s, not %s", Class->GetName(), Token.Identifier );

		// Get parent class.
		if( MatchIdentifier("Expands") )
		{
			// Set the superclass.
			UClass* TempClass = GetQualifiedClass( "'expands'" );
			if( Class->GetSuperClass() == NULL )
				Class->SuperField = TempClass;
			else if( Class->GetSuperClass() != TempClass )
				appThrowf( "%s's superclass must be %s, not %s", Class->GetPathName(), Class->GetSuperClass()->GetPathName(), TempClass->GetPathName() );

			// Copy from parent if expanding a database.
			Class->ClassRecordSize = Class->GetSuperClass()->ClassRecordSize;
		}
		else if( Class->GetSuperClass() )
			appThrowf( "Class: missing 'Expands %s'", Class->GetSuperClass()->GetName() );

		// Class attributes.
		FToken Token;
		for( ; ; )
		{
			GetToken(Token);
			if( Token.Matches(NAME_Intrinsic) )
			{
				// Note that this class has C++ code dependencies.
				if( Class->GetSuperClass() && !(Class->GetSuperClass()->GetFlags() & RF_Intrinsic) )
					appThrowf( "Intrinsic classes cannot expand non-intrinsic classes" );
				Class->SetFlags(RF_Intrinsic);
			}
			else if( Token.Matches(NAME_Abstract) )
			{
				// Hide all editable properties.
				Class->ClassFlags |= CLASS_Abstract;
			}
			else if( Token.Matches(NAME_Guid) )
			{
				// Get the class's GUID.
				RequireSymbol( "(", "'Guid'" );
					GetConstInt(*(INT*)&Class->ClassGuid.A);
				RequireSymbol( ",", "'Guid'" );
					GetConstInt(*(INT*)&Class->ClassGuid.B);
				RequireSymbol( ",", "'Guid'" );
					GetConstInt(*(INT*)&Class->ClassGuid.C);
				RequireSymbol( ",", "'Guid'" );
					GetConstInt(*(INT*)&Class->ClassGuid.D);
				RequireSymbol( ")", "'Guid'" );
			}
			else if( Token.Matches(NAME_Transient) )
			{
				// Transient class.
				Class->ClassFlags |= CLASS_Transient;
			}
			else if( Token.Matches(NAME_Localized) )
			{
				// Transient class.
				Class->ClassFlags |= CLASS_Localized;
			}
			else if( Token.Matches(NAME_Config) )
			{
				// Transient class.
				Class->ClassFlags |= CLASS_Config;
			}
			else if( Token.Matches(NAME_SafeReplace) )
			{
				// Safely replaceable.
				Class->ClassFlags |= CLASS_SafeReplace;
			}
			else
			{
				UngetToken(Token);
				break;
			}
		}

		// Get semicolon.
		RequireSymbol( ";", "'Class'" );
		NeedSemicolon=0;

		// Init variables.
		Class->Script.Empty();
		Class->Children			= NULL;
		Class->Next				= NULL;
		Class->HashNext			= NULL;
		Class->TextPos			= 0;
		Class->Line				= 0;
		Class->ProbeMask        = 0;
		Class->IgnoreMask       = 0;
		Class->VfHash           = NULL;
		Class->StateFlags       = 0;
		Class->LabelTableOffset = 0;

		// Make visible outside the package.
		Class->ClearFlags( RF_Transient );
		Class->SetFlags( RF_Public|RF_Standalone );

		// Setup initial package imports to include the packages and the package imports
		// of all base classes.
		Class->PackageImports.Empty();
		for( UClass* C = Class; C; C=C->GetSuperClass() )
		{
			Class->PackageImports.AddUniqueItem( C->GetParent()->GetFName() );
			for( int i=0; i<C->PackageImports.Num(); i++ )
				Class->PackageImports.AddUniqueItem( C->PackageImports( i ) );
		}

		// Setup initial dependencies.
		Class->Dependencies.Empty();
		Class->AddDependency( Class, 1 );
		if( Class->GetSuperClass() )
			Class->AddDependency( Class->GetSuperClass(), 1 );

		// Copy properties from parent class.
		Class->Defaults.Empty();
		if( Class->GetSuperClass() )
		{
			Class->SetPropertiesSize( Class->GetSuperClass()->GetPropertiesSize() );
			Class->Defaults = Class->GetSuperClass()->Defaults;
		}

		// Push the class nesting.
		PushNest( NEST_Class, Class->GetName(), NULL );

		unguard;
	}
	else if( Token.Matches(NAME_Import) )
	{
		guard(Uses);
		CheckAllow("'Uses'",ALLOW_VarDecl);
		if( TopNest->NestType != NEST_Class )
			appThrowf( "'Uses' is are only allowed at class scope" );

		// Get thing to import.
		FToken ImportThing;
		GetToken( ImportThing );
		if( !ImportThing.Matches(NAME_Enum) && !ImportThing.Matches(NAME_Package) )
			appThrowf( "'import': Missing 'enum', 'struct', or 'package'" );

		// Get name to import.
		FToken ImportName;
		if( !GetIdentifier(ImportName) )
			appThrowf( "'import': Missing package, enum, or struct name to import" );

		// Handle package, enum, or struct.
		if( ImportThing.Matches(NAME_Package) )
		{
			// Import a package.
			Class->PackageImports.AddUniqueItem( FName(ImportName.Identifier) );
		}
		else if( ImportThing.Matches(NAME_Enum) )
		{
			// From package.
			UPackage* Pkg = CastChecked<UPackage>(Class->GetParent());
			if( MatchIdentifier("From") )
			{
				FToken ImportPackage;
				if(	GetIdentifier( ImportPackage ) )
				{
					Pkg = FindObject<UPackage>( NULL, ImportPackage.Identifier );
					if( !Pkg )
						appThrowf( "'Uses': Unrecognized package '%s'", ImportPackage.Identifier );
				}
			}
			else appThrowf( "'Uses': Unrecognized '%s'", ImportThing.Identifier );
			new( Pkg, ImportName.Identifier )UEnum( NULL );
		}

		unguard;
	}
	else if
	(	Token.Matches(NAME_Function)
	||	Token.Matches(NAME_Operator)
	||	Token.Matches(NAME_PreOperator) 
	||	Token.Matches(NAME_PostOperator) 
	||	Token.Matches(NAME_Intrinsic) 
	||	Token.Matches(NAME_Final) 
	||	Token.Matches(NAME_Private) 
	||	Token.Matches(NAME_Latent)
	||	Token.Matches(NAME_Iterator)
	||	Token.Matches(NAME_Singular)
	||	Token.Matches(NAME_Static)
	||	Token.Matches(NAME_Exec)
	|| (Token.Matches(NAME_Event) && (TopNest->Allow & ALLOW_Function) )
	|| (Token.Matches(NAME_Simulated) && !PeekIdentifier("State")) )
	{
		// Function or operator.
		guard(Function/Operator);
		const char* NestName = NULL;
		FRetryPoint FuncNameRetry;
		FFuncInfo FuncInfo;

		// Process all specifiers.
		for( ;; )
		{
			InitRetry(FuncNameRetry);
			if( Token.Matches(NAME_Function) )
			{
				// Get function name.
				CheckAllow( "'Function'",ALLOW_Function);
				NestName = "function";
			}
			else if( Token.Matches(NAME_Operator) )
			{
				// Get operator name or symbol.
				CheckAllow("'Operator'",ALLOW_Function);
				NestName = "operator";
				FuncInfo.FunctionFlags |= FUNC_Operator;
				FuncInfo.ExpectParms = 3;

				if( !MatchSymbol("(") )
					appThrowf( "Missing '(' and precedence after 'Operator'" );
				else if( !GetConstInt(FuncInfo.Precedence) )
					appThrowf( "Missing precedence value" );
				else if( FuncInfo.Precedence<0 || FuncInfo.Precedence>255 )
					appThrowf( "Bad precedence value" );
				else if( !MatchSymbol(")") )
					appThrowf( "Missing ')' after operator precedence" );
			}
			else if( Token.Matches(NAME_PreOperator) )
			{
				// Get operator name or symbol.
				CheckAllow("'PreOperator'",ALLOW_Function);
				NestName = "preoperator";
				FuncInfo.ExpectParms = 2;
				FuncInfo.FunctionFlags |= FUNC_Operator | FUNC_PreOperator;
			}
			else if( Token.Matches(NAME_PostOperator) )
			{
				// Get operator name or symbol.
				CheckAllow("'PostOperator'",ALLOW_Function);
				NestName = "postoperator";
				FuncInfo.FunctionFlags |= FUNC_Operator;
				FuncInfo.ExpectParms = 2;
			}
			else if( Token.Matches(NAME_Intrinsic) )
			{
				// Get internal id.
				FuncInfo.FunctionFlags |= FUNC_Intrinsic;
				if( MatchSymbol("(") )
				{
					if( !GetConstInt(FuncInfo.iIntrinsic) )
						appThrowf( "Missing intrinsic id" );
					else if( !MatchSymbol(")") )
						appThrowf( "Missing ')' after internal id" );
				}
			}
			else if( Token.Matches(NAME_Event) )
			{
				CheckAllow( "'Function'",ALLOW_Function);
				NestName = "event";
				FuncInfo.FunctionFlags |= FUNC_Event;
			}
			else if( Token.Matches(NAME_Static) )
			{
				FuncInfo.FunctionFlags |= FUNC_Static;
				if( TopNode->GetClass()==UState::StaticClass )
					appThrowf( "Static functions cannot exist in a state" );
			}
			else if( Token.Matches(NAME_Simulated) )
			{
				FuncInfo.FunctionFlags |= FUNC_Simulated;
			}
			else if( Token.Matches(NAME_Iterator) )
			{
				FuncInfo.FunctionFlags |= FUNC_Iterator;
			}
			else if( Token.Matches(NAME_Singular) )
			{
				FuncInfo.FunctionFlags |= FUNC_Singular;
			}
			else if( Token.Matches(NAME_Latent) )
			{
				FuncInfo.FunctionFlags |= FUNC_Latent;
			}
			else if( Token.Matches(NAME_Exec) )
			{
				FuncInfo.FunctionFlags |= FUNC_Exec;
			}
			else if( Token.Matches(NAME_Final) )
			{
				// This is a final (prebinding, non-overridable) function or operator.
				FuncInfo.FunctionFlags |= FUNC_Final;
			}
			else break;
			GetToken(Token);
		}
		UngetToken(Token);

		// Make sure we got a function.
		if( !NestName )
			appThrowf( "Missing 'function'" );

		// Warn if intrinsic doesn't actually exist.
#if CHECK_INTRINSIC_MATCH
		if( FuncInfo.iIntrinsic!=0 )
			if( FuncInfo.iIntrinsic<EX_FirstIntrinsic || FuncInfo.iIntrinsic>EX_Max || GIntrinsics[FuncInfo.iIntrinsic]==execUndefined )
				appThrowf( "Warning: Bad intrinsic id %i\r\n",FuncInfo.iIntrinsic);
#endif

		// Get return type.
		FRetryPoint Start; InitRetry(Start);
		DWORD ObjectFlags;
		FPropertyBase ReturnType( CPT_None );
		FToken TestToken;
		UBOOL HasReturnValue = 0;
		if( GetIdentifier(TestToken,1) )
		{
			if( !PeekSymbol("(") )
			{
				PerformRetry( Start );
				HasReturnValue = GetVarType( TopNode, ReturnType, ObjectFlags, ~0, NULL );
			}
			else PerformRetry( Start );
		}

		// Get function or operator name.
		if( !GetIdentifier(FuncInfo.Function) && (!(FuncInfo.FunctionFlags&FUNC_Operator) || !GetSymbol(FuncInfo.Function)) )
			appThrowf( "Missing %s name", NestName );
		if( !MatchSymbol("(") )
			appThrowf( "Bad %s definition", NestName );

		// Validate flag combinations.
		if( FuncInfo.FunctionFlags & FUNC_Intrinsic )
		{
			if( FuncInfo.iIntrinsic && !(FuncInfo.FunctionFlags & FUNC_Final) )
				appThrowf( "Numbered intrinsics must be final" );
		}
		else
		{
			if( FuncInfo.FunctionFlags & FUNC_Latent )
				appThrowf( "Only intrinsic functions may use 'Latent'" );
			if( FuncInfo.FunctionFlags & FUNC_Iterator )
				appThrowf( "Only intrinsic functions may use 'Iterator'" );
		}

		// If operator, figure out the function signature.
		char Signature[NAME_SIZE]="";
		if( FuncInfo.FunctionFlags & FUNC_Operator )
		{
			// Name.
			const char* In = FuncInfo.Function.Identifier;
			while( *In )
				appStrncat( Signature, CppTags[*In++-32], NAME_SIZE );

			// Parameter signature.
			FRetryPoint Retry;
			InitRetry( Retry );
			if( !MatchSymbol(")") )
			{
				// Parameter types.
				appStrncat( Signature, "_", NAME_SIZE );
				if( FuncInfo.FunctionFlags & FUNC_PreOperator )
					appStrncat( Signature, "Pre", NAME_SIZE );
				do
				{
					// Get parameter type.
					FPropertyBase Property(CPT_None);
					DWORD ObjectFlags;
					GetVarType( TopNode, Property, ObjectFlags, ~CPF_ParmFlags, "Function parameter" );
					GetVarNameAndDim( TopNode, Property, ObjectFlags, 0, 1, NULL, "Function parameter", NAME_None, 1 );

					// Add to signature.
					if
					(	Property.Type==CPT_ObjectReference
					||	Property.Type==CPT_Struct )
					{
						appStrncat( Signature, Property.PropertyClass->GetName(), NAME_SIZE );
					}
					else
					{
						char Temp[NAME_SIZE];
						appStrcpy( Temp, *FName((EName)Property.Type) );
						if( appStrstr( Temp, "Property" ) )
							*appStrstr( Temp, "Property" ) = 0;
						appStrncat( Signature, Temp, NAME_SIZE );
					}
				} while( MatchSymbol(",") );
				RequireSymbol( ")", "parameter list" );
			}
			PerformRetry( Retry, 1, 1 );
		}
		else
		{
			appStrcpy( Signature, FuncInfo.Function.Identifier );
		}

		// Allocate local property frame, push nesting level and verify 
		// uniqueness at this scope level.
		PushNest( NEST_Function, Signature, NULL );
		UFunction* TopFunction = CastChecked<UFunction>(TopNode);
		TopFunction->FunctionFlags  |= FuncInfo.FunctionFlags;
		TopFunction->OperPrecedence  = FuncInfo.Precedence;
		TopFunction->iIntrinsic      = FuncInfo.iIntrinsic;
		TopFunction->FriendlyName    = FName( FuncInfo.Function.Identifier, FNAME_Add );

		// Get parameter list.
		if( !MatchSymbol(")") )
		{
			UBOOL Optional=0;
			do
			{
				// Get parameter type.
				FPropertyBase Property(CPT_None);
				DWORD ObjectFlags;
				GetVarType( TopNode, Property, ObjectFlags, ~CPF_ParmFlags, "Function parameter" );
				Property.PropertyFlags |= CPF_Parm;
				GetVarNameAndDim( TopNode, Property, ObjectFlags, 0, 1, NULL, "Function parameter", NAME_None, 0 );
				TopFunction->NumParms++;

				// Check parameters.
				if( (FuncInfo.FunctionFlags & FUNC_Operator) && (Property.PropertyFlags & ~CPF_ParmFlags) )
					appThrowf( "Operator parameters may not have modifiers" );
				else if( Property.Type==CPT_Bool && (Property.PropertyFlags & CPF_OutParm) )
					appThrowf( "Booleans may not be out parameters" );
				else if
				(	(Property.PropertyFlags & CPF_SkipParm)
				&&	(!(TopFunction->FunctionFlags&FUNC_Intrinsic) || !(TopFunction->FunctionFlags&FUNC_Operator) || TopFunction->NumParms!=2) )
					appThrowf( "Only parameter 2 of intrinsic operators may be 'Skip'" );
				if( Property.PropertyFlags & CPF_OptionalParm )
				{
					if( Property.PropertyFlags & CPF_OutParm )
						appThrowf( "Out parameters may not be optional" );
					Optional = 1;
				}
				else if( Optional ) appThrowf( "After an optional parameters, all other parmeters must be optional" );
			} while( MatchSymbol(",") );
			RequireSymbol( ")", "parameter list" );
		}

		// Get return type, if any.
		if( HasReturnValue )
		{
			ReturnType.PropertyFlags |= CPF_Parm | CPF_OutParm | CPF_ReturnParm;
			UProperty* ReturnProperty = GetVarNameAndDim( TopNode, ReturnType, ObjectFlags, 1, 1, "ReturnValue", "Function return type", NAME_None, 0 );
			TopFunction->NumParms++;
		}

		// Check overflow.
		if( TopFunction->NumParms > MAX_FUNC_PARMS )
			appThrowf( "'%s': too many parameters", TopNode->GetName() );

		// For operators, verify that: the operator is either binary or unary, there is
		// a return value, and all parameters have the same type as the return value.
		if( FuncInfo.FunctionFlags & FUNC_Operator )
		{
			INT n = TopFunction->NumParms;
			if( n != FuncInfo.ExpectParms )
				appThrowf( "%s must have %i parameters", NestName, FuncInfo.ExpectParms-1 );

			if( !TopFunction->GetReturnProperty() )
				appThrowf( "Operator must have a return value" );

			if( !(FuncInfo.FunctionFlags & FUNC_Final) )
				appThrowf( "Operators must be declared as 'Final'" );
		}

		// Detect whether the function is being defined or declared.
		if( PeekSymbol(";") )
		{
			// Function is just being declared, not defined.
			check( (TopFunction->FunctionFlags & FUNC_Defined)==0 );
		}
		else
		{
			// Function is being defined.
			TopFunction->FunctionFlags |= FUNC_Defined;
			if( TopFunction->FunctionFlags & FUNC_Intrinsic )
				appThrowf( "Intrinsic functions may only be declared, not defined" );

			// Require bracket.
			RequireSymbol( "{", NestName );
			NeedSemicolon=0;
		}

		// Verify parameter list and return type compatibility within the 
		// function, if any, that it overrides.
		//!!tfieldit
		for( INT i=NestLevel-2; i>=1; i-- )
		{
			for( TFieldIterator<UFunction> Function(Nest[i].Node); Function; ++Function )
			{
				// If the other function's name matches this one's, process it.
				if
				(	Function->GetFName()==TopNode->GetFName()
				&&	*Function!=TopNode
				&&	((Function->FunctionFlags ^ TopFunction->FunctionFlags) & (FUNC_Operator | FUNC_PreOperator))==0 )
				{
					// Check precedence.
					if( Function->OperPrecedence!=TopFunction->OperPrecedence && Function->NumParms==TopFunction->NumParms )
						appThrowf( "Overloaded operator differs in precedence" );

					// See if all parameters match.
					if
					(	TopFunction->NumParms!=Function->NumParms
					||	(!TopFunction->GetReturnProperty())!=(!Function->GetReturnProperty()) )
						appThrowf( "Redefinition of '%s %s' differs from original", NestName, FuncInfo.Function.Identifier );

					// Check all individual parameters.
					INT Count=0;
					for( TFieldIterator<UProperty> It1(TopFunction),It2(*Function); Count<Function->NumParms; ++It1,++It2,++Count )
					{
						if( !FPropertyBase(*It1).MatchesType(FPropertyBase(*It2), 1) )
						{
							if( It1->PropertyFlags & CPF_ReturnParm )
								appThrowf( "Redefinition of %s %s differs only by return type", NestName, FuncInfo.Function.Identifier );
							else if( !(FuncInfo.FunctionFlags & FUNC_Operator) )
								appThrowf( "Redefinition of '%s %s' differs from original", NestName, FuncInfo.Function.Identifier );
							break;
						}
					}
					if( Count<TopFunction->NumParms )
						continue;

					// Function flags to copy from parent.
					FuncInfo.FunctionFlags |= (Function->FunctionFlags & FUNC_FuncInherit);

					// Balk if required specifiers differ.
					if( (Function->FunctionFlags&FUNC_FuncOverrideMatch) != (FuncInfo.FunctionFlags&FUNC_FuncOverrideMatch) )
						appThrowf( "Function '%s' specifiers differ from original", Function->GetName() );

					// Are we overriding a function?
					if( TopNode==Function->GetParent() )
					{
						// Duplicate.
						PerformRetry( FuncNameRetry );
						appThrowf( "Duplicate function '%s'", Function->GetName() );
					}
					else
					{
						// Overriding an existing function.
						if( Function->FunctionFlags & FUNC_Final )
						{
							PerformRetry(FuncNameRetry);
							appThrowf( "%s: Can't override a 'final' function", Function->GetName() );
						}
					}

					// Here we have found the original.
					TopNode->SuperField = *Function;
					goto Found;
				}
			}
		}
		Found:

		// Bind the function.
		TopFunction->Bind();

		// If declaring a function, end the nesting.
		if( !(TopFunction->FunctionFlags & FUNC_Defined) )
			PopNest( NEST_Function, NestName );

		unguard;
	}
	else if( Token.Matches(NAME_Const) )
	{
		guard(Const);
		CompileConst( Class );
		unguard;
	}
	else if( Token.Matches(NAME_Var) || Token.Matches(NAME_Local) )
	{
		// Variable definition.
		guard(Var);

		DWORD Disallow;
		if( Token.Matches(NAME_Var) )
		{
			// Declaring per-object variables.
			CheckAllow("'Var'",ALLOW_VarDecl);
			if( TopNest->NestType != NEST_Class )
				appThrowf( "Instance variables are only allowed at class scope (use 'local'?)" );
			Disallow = CPF_ParmFlags;
		}
		else
		{
			// Declaring local variables.
			CheckAllow("'Local'",ALLOW_VarDecl);
			if( TopNest->NestType == NEST_Class )
				appThrowf( "Local variables are only allowed in functions" );
			Disallow	= ~0;
		}

		// Get category, if any.
		FName EdCategory = NAME_None;
		DWORD EdFlags    = 0;
		if( MatchSymbol("(") )
		{
			// Get optional property editing category.
			EdFlags |= CPF_Edit;
			FToken Category;
			if( GetIdentifier( Category, 1 ) )	EdCategory = FName( Category.Identifier );
			else								EdCategory = Class->GetFName();
			
			if( !MatchSymbol(")") )
				appThrowf( "Missing ')' after editable category" );
		}

		// Compile the variable type.		
		FPropertyBase OriginalProperty(CPT_None);
		DWORD ObjectFlags=0;
		GetVarType( TopNode, OriginalProperty, ObjectFlags, Disallow, "Variable declaration" );
		OriginalProperty.PropertyFlags |= EdFlags;

		// If editable but no category was specified, the category name is our class name.
		if( (OriginalProperty.PropertyFlags & CPF_Edit) && (EdCategory==NAME_None) )
			EdCategory = Class->GetFName();

		// Validate combinations.
		if( (OriginalProperty.PropertyFlags & (CPF_Transient|CPF_Intrinsic)) && TopNest->NestType!=NEST_Class )
			appThrowf( "Static and local variables may not be transient or intrinsic" );
		if( OriginalProperty.PropertyFlags & CPF_ParmFlags )
			appThrowf( "Illegal type modifiers in variable" );

		// Process all variables of this type.
		do
		{
			FPropertyBase Property = OriginalProperty;
			GetVarNameAndDim( TopNode, Property, ObjectFlags, 0, 0, NULL, "Variable declaration", EdCategory, 0 );
		} while( MatchSymbol(",") );
		unguard;
	}
	else if( Token.Matches(NAME_Enum) )
	{
		// Enumeration definition.
		guard(Enum);
		CheckAllow("'Enum'",ALLOW_VarDecl);

		// Compile enumeration.
		CompileEnum( Class );

		unguard;
	}
	else if( Token.Matches(NAME_Struct) )
	{
		// Struct definition.
		guard(Struct);
		CheckAllow( "'struct'", ALLOW_VarDecl );

		// Compile struct.
		CompileStruct( Class );

		unguard;
	}
	else if
	(	Token.Matches(NAME_State)
	||	Token.Matches(NAME_Auto) 
	||	Token.Matches(NAME_Simulated) )
	{
		// State block.
		guard(State);
		check(TopNode!=NULL);
		CheckAllow("'State'",ALLOW_State);
		DWORD StateFlags=0, GotState=0;

		// Process all specifiers.
		for( ;; )
		{
			if( Token.Matches(NAME_State) )
			{
				GotState=1;
				if( MatchSymbol("(") )
				{
					RequireSymbol( ")", "'State'" );
					StateFlags |= STATE_Editable;
				}
			}
			else if( Token.Matches(NAME_Simulated) )
			{
				StateFlags |= STATE_Simulated;
			}
			else if( Token.Matches(NAME_Auto) )
			{
				StateFlags |= STATE_Auto;
			}
			else
			{
				UngetToken(Token);
				break;
			}
			GetToken(Token);
		}
		if( !GotState )
			appThrowf( "Missing 'State'" );

		// Get name and default parent state.
		FToken NameToken;
		if( !GetIdentifier(NameToken) )
			appThrowf( "Missing state name" );
		UState* ParentState = Cast<UState>(FindField( TopNode, NameToken.Identifier, UState::StaticClass, "'state'" ));
		if( ParentState && ParentState->GetOwnerClass()==Class )
			appThrowf( "Duplicate state '%s'", NameToken.Identifier );

		// Check for 'expands' keyword.
		if( MatchIdentifier("Expands") )
		{
			FToken ParentToken;
			if( ParentState )
				appThrowf( "'Expands' not allowed here: state '%s' overrides version in parent class", NameToken.Identifier );
			if( !GetIdentifier(ParentToken) )
				appThrowf( "Missing parent state name" );
			ParentState = Cast<UState>(FindField( TopNode, ParentToken.Identifier, UState::StaticClass, "'state'" ));
			if( !ParentState )
				appThrowf( "'expands': Parent state '%s' not found", ParentToken.Identifier );
		}

		// Begin the state block.
		PushNest( NEST_State, NameToken.Identifier, NULL );
		UState* State = CastChecked<UState>( TopNode );
		State->StateFlags |= StateFlags;
		State->SuperField = ParentState;
		RequireSymbol( "{", "'State'" );
		NeedSemicolon=0;

		unguard;
	}
	else if( Token.Matches(NAME_Ignores) )
	{
		// Probes to ignore in this state.
		guard(Ignores);
		CheckAllow("'Ignores'",ALLOW_Ignores);
		for( ; ; )
		{
			FToken IgnoreFunction;
			if( !GetToken(IgnoreFunction) )
				appThrowf( "'Ignores': Missing probe function name" );
			if
			(	IgnoreFunction.TokenName==NAME_None
			||	IgnoreFunction.TokenName.GetIndex() <  NAME_PROBEMIN
			||	IgnoreFunction.TokenName.GetIndex() >= NAME_PROBEMAX )
			{
				for( INT i=NestLevel-2; i>=1; i-- )
				{
					for( TFieldIterator<UFunction> Function(Nest[i].Node); Function; ++Function )
					{
						if( Function->GetFName()==IgnoreFunction.TokenName )
						{
							// Verify that function is ignoreable.
							if( Function->FunctionFlags & FUNC_Final )
								appThrowf( "'%s': Cannot ignore final functions", Function->GetName() );

							// Insert empty function definition to intercept the call.
							PushNest( NEST_Function, Function->GetName(), NULL );
							UFunction* TopFunction = CastChecked<UFunction>( TopNode );
							TopFunction->FunctionFlags    |= (Function->FunctionFlags & FUNC_FuncOverrideMatch);
							TopFunction->NumParms          = Function->NumParms;
							TopFunction->SuperField        = *Function;

							// Copy parameters.
							UField** PrevLink = &TopFunction->Children;
							check(*PrevLink==NULL);
							for( TFieldIterator<UProperty> It(*Function); It && (It->PropertyFlags & CPF_Parm); ++It )
							{
								UProperty* NewProperty=NULL;
								if( It->IsA(UByteProperty::StaticClass) )
								{
									NewProperty = new(TopFunction,It->GetName(),RF_Public)UByteProperty;
									Cast<UByteProperty>(NewProperty)->Enum = Cast<UByteProperty>(*It)->Enum;
								}
								else if( It->IsA(UIntProperty::StaticClass) )
								{
									NewProperty = new(TopFunction,It->GetName(),RF_Public)UIntProperty;
								}
								else if( It->IsA(UBoolProperty::StaticClass) )
								{
									NewProperty = new(TopFunction,It->GetName(),RF_Public)UBoolProperty;
								}
								else if( It->IsA(UFloatProperty::StaticClass) )
								{
									NewProperty = new(TopFunction,It->GetName(),RF_Public)UFloatProperty;
								}
								else if( It->IsA(UClassProperty::StaticClass) )
								{
									NewProperty = new(TopFunction,It->GetName(),RF_Public)UClassProperty;
									Cast<UObjectProperty>(NewProperty)->PropertyClass = Cast<UObjectProperty>(*It)->PropertyClass;
									Cast<UClassProperty>(NewProperty)->MetaClass = Cast<UClassProperty>(*It)->MetaClass;
								}
								else if( It->IsA(UObjectProperty::StaticClass) )
								{
									NewProperty = new(TopFunction,It->GetName(),RF_Public)UObjectProperty;
									Cast<UObjectProperty>(NewProperty)->PropertyClass = Cast<UObjectProperty>(*It)->PropertyClass;
								}
								else if( It->IsA(UNameProperty::StaticClass) )
								{
									NewProperty = new(TopFunction,It->GetName(),RF_Public)UNameProperty;
								}
								else if( It->IsA(UStringProperty::StaticClass) )
								{
									NewProperty = new(TopFunction,It->GetName(),RF_Public)UStringProperty;
									Cast<UStringProperty>(NewProperty)->StringSize = Cast<UStringProperty>(*It)->StringSize;
								}
								else if( It->IsA(UStructProperty::StaticClass) )
								{
									NewProperty = new(TopFunction,It->GetName(),RF_Public)UStructProperty;
									Cast<UStructProperty>(NewProperty)->Struct = Cast<UStructProperty>(*It)->Struct;
								}
								else appErrorf( "Unknown property type %s", It->GetClassName() );
								NewProperty->ArrayDim = It->ArrayDim;
								NewProperty->PropertyFlags = It->PropertyFlags;
								*PrevLink              = NewProperty;
								PrevLink = &(*PrevLink)->Next;
							}

							// Finish up.
							PopNest( NEST_Function, "Ignores" );
							goto FoundIgnore;
						}
					}
				}
				appThrowf( "'Ignores': '%s' is not a function", IgnoreFunction.Identifier );
				FoundIgnore:;
			}
			else
			{
				// Ignore a probe function.
				UState* State = CastChecked<UState>( TopNode );
				State->IgnoreMask &= ~((QWORD)1 << (IgnoreFunction.TokenName.GetIndex() - NAME_PROBEMIN));
			}

			// More?
			if( !MatchSymbol(",") )
				break;
		}
		unguard;
	}
	else if( Token.Matches(NAME_Replication) )
	{
		// Network replication definition.
		guard(Replication);
		if( TopNest->NestType != NEST_Class )
			appThrowf( "'Replication' is not allowed here" );
		RequireSymbol( "{", "'Replication'" );
		TopNode->TextPos = InputPos;
		TopNode->Line    = InputLine;
		SkipStatements( 1, "'Replication'" );

		NeedSemicolon=0;
		unguard;
	}
	else if( Token.Matches("#") )
	{
		// Compiler directive.
		guard(Directive);
		CompileDirective();
		NeedSemicolon=0;
		unguard;
	}
	else
	{
		// Not a declaration.
		return 0;
	}
	return 1;
	unguard;
}

//
// Compile a command in Token. Handles any errors that may occur.
//
void FScriptCompiler::CompileCommand( FToken& Token, UBOOL& NeedSemicolon )
{
	guard(FScriptCompiler::CompileCommand);
	check(Pass==1);

	if( Token.Matches(NAME_Switch) )
	{
		// Switch.
		guard(Switch);
		CheckAllow("'Switch'",ALLOW_Cmd);
		PushNest(NEST_Switch,"",NULL);

		// Compile the select-expression.
		Writer << EX_Switch;
		FRetryPoint LowRetry; InitRetry(LowRetry);
		CompileExpr( FPropertyBase(CPT_None), "'Switch'", &TopNest->SwitchType );
		if( TopNest->SwitchType.ArrayDim != 1 )
			appThrowf( "Can't switch on arrays" );
		FRetryPoint HighRetry; InitRetry(HighRetry);
		if( TopNest->SwitchType.Type == CPT_String )
		{
			BYTE B=0;
			Writer << B;
		}
		else
		{
			EmitSize( TopNest->SwitchType.GetSize(), "'Switch'" );
		}
		CodeSwitcheroo(LowRetry,HighRetry);
		TopNest->SwitchType.PropertyFlags &= ~(CPF_OutParm);

		// Get bracket.
		RequireSymbol( "{", "'Switch'" );
		NeedSemicolon=0;

		unguard;
	}
	else if( Token.Matches(NAME_Case) )
	{
		guard(Case);
		CheckAllow("'Class'",ALLOW_Case);

		// Update previous Case's chain address.
		EmitChainUpdate(TopNest);

		// Insert this case statement and prepare to chain it to the next one.
		Writer << EX_Case;
		EmitAddressToChainLater(TopNest);
		CompileExpr( TopNest->SwitchType,"'Case'");
		RequireSymbol(":","'Case'");
		NeedSemicolon=0;

		TopNest->Allow |= ALLOW_Cmd | ALLOW_Label | ALLOW_Break;
		unguard;
	}
	else if( Token.Matches(NAME_Default) )
	{
		// Default case.
		guard(Default);
		CheckAllow("'Default'",ALLOW_Case);

		// Update previous Case's chain address.
		EmitChainUpdate(TopNest);

		// Emit end-of-case marker.
		Writer << EX_Case;
		_WORD W=MAXWORD; Writer << W;
		RequireSymbol(":","'Default'");
		NeedSemicolon=0;

		// Don't allow additional Cases after Default.
		TopNest->Allow &= ~ALLOW_Case;
		TopNest->Allow |=  ALLOW_Cmd | ALLOW_Label | ALLOW_Break;
		unguard;
	}
	else if( Token.Matches(NAME_Return) )
	{
		// Only valid from within a function or operator.
		guard(Return);
		CheckAllow( "'Return'", ALLOW_Return );
		for( INT i=NestLevel-1; i>0; i-- )
		{
			if( Nest[i].NestType==NEST_Function )
				break;
			else if( Nest[i].NestType==NEST_ForEach )
				Writer << EX_IteratorPop;
		}
		if( i <= 0 )
			appThrowf( "Internal consistency error on 'Return'" );
		UFunction* Function = CastChecked<UFunction>(Nest[i].Node);
		UProperty* Return = Function->GetReturnProperty();
		if( Return )
		{
			// Emit an assignment to the variable.
			EmitLet( FPropertyBase(Return), "'Return'" );

			// Emit variable info.
			if( Return->IsA(UBoolProperty::StaticClass) )
				Writer << EX_BoolVariable;
			Writer << EX_LocalVariable;
			Writer << Return;

			// Return expression.
			FPropertyBase Property = FPropertyBase(Return);
			Property.PropertyFlags &= ~CPF_OutParm;
			CompileExpr( Property, "'Return'" );
		}

		// Emit the return.
		Writer << EX_Return;
		unguard;
	}
	else if( Token.Matches(NAME_If) )
	{
		// If.
		guard(If);
		CheckAllow( "'If'", ALLOW_Cmd );
		PushNest( NEST_If, "", NULL );

		// Jump to next evaluator if expression is false.
		Writer << EX_JumpIfNot;
		EmitAddressToChainLater(TopNest);

		// Compile boolean expr.
		RequireSymbol( "(", "'If'" );
		CompileExpr( FPropertyBase(CPT_Bool), "'If'" );
		RequireSymbol( ")", "'If'" );

		// Handle statements.
		NeedSemicolon = 0;
		if( !MatchSymbol( "{" ) )
		{
			CompileStatements();
			PopNest( NEST_If, "'If'" );
		}
		unguard;
	}
	else if( Token.Matches(NAME_While) )
	{
		guard(While);
		CheckAllow( "'While'", ALLOW_Cmd );
		PushNest( NEST_Loop, "", NULL );

		// Here is the start of the loop.
		TopNest->SetFixup(FIXUP_LoopStart,TopNode->Script.Num());

		// Evaluate expr and jump to end of loop if false.
		Writer << EX_JumpIfNot; 
		EmitAddressToFixupLater(TopNest,FIXUP_LoopEnd,NAME_None);

		// Compile boolean expr.
		RequireSymbol("(","'While'");
		CompileExpr( FPropertyBase(CPT_Bool), "'While'" );
		RequireSymbol(")","'While'");

		// Handle statements.
		NeedSemicolon=0;
		if( !MatchSymbol("{") )
		{
			CompileStatements();
			PopNest( NEST_Loop, "'While'" );
		}

		unguard;
	}
	else if(Token.Matches(NAME_Do))
	{
		guard(Do);
		CheckAllow( "'Do'", ALLOW_Cmd );
		PushNest( NEST_Loop, "", NULL );

		TopNest->SetFixup(FIXUP_LoopStart,TopNode->Script.Num());

		// Handle statements.
		NeedSemicolon=0;
		if( !MatchSymbol("{") )
		{
			CompileStatements();
			PopNest( NEST_Loop, "'Do'" );
		}

		unguard;
	}
	else if( Token.Matches(NAME_Break) )
	{
		guard(Break);
		CheckAllow( "'Break'", ALLOW_Break );
		
		// Find the nearest For or Loop.
		INT iNest = FindNest(NEST_Loop);
		iNest     = Max(iNest,FindNest(NEST_For    ));
		iNest     = Max(iNest,FindNest(NEST_ForEach));
		iNest     = Max(iNest,FindNest(NEST_Switch ));
		check(iNest>0);

		// Jump to the loop's end.
		Writer << EX_Jump;
		if     ( Nest[iNest].NestType == NEST_Loop    ) EmitAddressToFixupLater( &Nest[iNest], FIXUP_LoopEnd,     NAME_None );
		else if( Nest[iNest].NestType == NEST_For     ) EmitAddressToFixupLater( &Nest[iNest], FIXUP_ForEnd,      NAME_None );
		else if( Nest[iNest].NestType == NEST_ForEach ) EmitAddressToFixupLater( &Nest[iNest], FIXUP_IteratorEnd, NAME_None );
		else                                            EmitAddressToFixupLater(TopNest,FIXUP_SwitchEnd,NAME_None);

		unguard;
	}
	else if(Token.Matches(NAME_For))
	{
		guard(For);
		CheckAllow( "'For'", ALLOW_Cmd );
		PushNest( NEST_For, "", NULL );

		// Compile for parms.
		RequireSymbol("(","'For'");
			CompileAssignment("'For'");
		RequireSymbol(";","'For'");
			TopNest->SetFixup(FIXUP_ForStart,TopNode->Script.Num());
			Writer << EX_JumpIfNot; 
			EmitAddressToFixupLater(TopNest,FIXUP_ForEnd,NAME_None);
			CompileExpr( FPropertyBase(CPT_Bool), "'For'" );
		RequireSymbol(";","'For'");
			// Skip the increment expression text but not code.
			InitRetry(TopNest->ForRetry);
			CompileAffector();
			PerformRetry(TopNest->ForRetry,1,0);
		RequireSymbol(")","'For'");

		// Handle statements.
		NeedSemicolon=0;
		if( !MatchSymbol("{") )
		{
			CompileStatements();
			PopNest( NEST_For, "'If'" );
		}
		unguard;
	}
	else if( Token.Matches(NAME_ForEach) )
	{
		guard(ForEach);
		CheckAllow( "'ForEach'", ALLOW_Cmd );
		PushNest( NEST_ForEach, "", NULL );

		// Compile iterator function call.
		AllowIterator = 1;
		GotIterator   = 0;

		// Emit iterator token.
		Writer << EX_Iterator;

		// Compile the iterator expression.
		FToken TypeToken;
		CompileExpr( FPropertyBase(CPT_None), "'ForEach'" );
		if( !GotIterator )
			appThrowf( "'ForEach': An iterator expression is required" );
		AllowIterator = 0;

		// Emit end offset.
		EmitAddressToFixupLater( TopNest, FIXUP_IteratorEnd, NAME_None );

		// Handle statements.
		NeedSemicolon = 0;
		if( !MatchSymbol("{") )
		{
			CompileStatements();
			PopNest( NEST_ForEach, "'ForEach'" );
		}
		unguard;
	}
	else if( Token.Matches(NAME_Assert) )
	{
		guard(Assert);
		CheckAllow( "'Assert'", ALLOW_Cmd );
		_WORD wLine = InputLine;
		Writer << EX_Assert;
		Writer << wLine;
		CompileExpr( FPropertyBase(CPT_Bool), "'Assert'" );
		unguard;
	}
	else if( Token.Matches(NAME_Goto) )
	{
		guard(Goto);
		CheckAllow( "'Goto'", ALLOW_Label );
		if( TopNest->Allow & ALLOW_StateCmd )
		{
			// Emit virtual state goto.
			Writer << EX_GotoLabel;
			CompileExpr( FPropertyBase(CPT_Name), "'Goto'" );
		}
		else
		{
			// Get label list for this nest level.
			for( INT iNest=NestLevel-1; iNest>=2; iNest-- )
				if( Nest[iNest].NestType==NEST_State || Nest[iNest].NestType==NEST_Function || Nest[iNest].NestType==NEST_ForEach )
					break;
			if( iNest < 2 )
				appThrowf( "Goto is not allowed here" );
			FNestInfo* LabelNest = &Nest[iNest];

			// Get label.
			FToken Label;
			if( !GetToken(Label) )
				appThrowf( "Goto: Missing label" );
			if( Label.TokenName == NAME_None )
				Label.TokenName = FName( Label.Identifier );
			if( Label.TokenName == NAME_None )
				appThrowf( "Invalid label '%s'", Label.Identifier );

			// Emit final goto.
			Writer << EX_Jump;
			EmitAddressToFixupLater( LabelNest, FIXUP_Label, Label.TokenName );
		}
		unguard;
	}
	else if( Token.Matches(NAME_Stop) )
	{
		guard(Stop);
		CheckAllow( "'Stop'", ALLOW_StateCmd );
		Writer << EX_Stop;
		unguard;
	}
	else if( Token.Matches("}") )
	{
		// End of block.
		guard(EndBracket);
		if( TopNest->NestType==NEST_Class )
			appThrowf( "Unexpected '}' at class scope" );
		else if( TopNest->NestType==NEST_None )
			appThrowf( "Unexpected '}' at global scope" );
		PopNest( NEST_None, "'}'" );
		NeedSemicolon=0;
		unguard;
	}
	else if( Token.Matches(";") )
	{
		// Extra semicolon.
		guard(Semicolon);
		NeedSemicolon=0;
		unguard;
	}
	else if( MatchSymbol(":") )
	{
		// A label.
		guard(Label);
		CheckAllow( "Label", ALLOW_Label );

		// Validate label name.
		if( Token.TokenName == NAME_None )
			Token.TokenName = FName( Token.Identifier );
		if( Token.TokenName == NAME_None )
			appThrowf( "Invalid label name '%s'", Token.Identifier );

		// Handle first label in a state.
		if( !(TopNest->Allow & ALLOW_Cmd ) )
		{
			// This is the first label in a state, so set the code start and enable commands.
			check(TopNest->NestType==NEST_State);
			TopNest->Allow     |= ALLOW_Cmd;
			TopNest->Allow     &= ~(ALLOW_Function | ALLOW_VarDecl);
		}

		// Get label list for this nest level.
		for( INT iNest=NestLevel-1; iNest>=2; iNest-- )
			if( Nest[iNest].NestType==NEST_State || Nest[iNest].NestType==NEST_Function || Nest[iNest].NestType==NEST_ForEach )
				break;
		if( iNest < 2 )
			appThrowf( "Labels are not allowed here" );
		FNestInfo *LabelNest = &Nest[iNest];

		// Make sure the label is unique here.
		for( FLabelRecord *LabelRec = LabelNest->LabelList; LabelRec; LabelRec=LabelRec->Next )
			if( LabelRec->Name == Token.TokenName )
				appThrowf( "Duplicate label '%s'", *Token.TokenName );

		// Add label.
		LabelNest->LabelList = new(GMem)FLabelRecord( Token.TokenName, TopNode->Script.Num(), LabelNest->LabelList );
		NeedSemicolon=0;

		unguard;
	}
	else
	{
		guard(Unknown);
		CheckAllow( "Expression", ALLOW_Cmd );
		UngetToken(Token);

		// Try to compile an affector expression or assignment.
		CompileAffector();

		unguard;
	}
	unguard;
}

//
// Compile a statement: Either a declaration or a command.
// Returns 1 if success, 0 if end of file.
//
int FScriptCompiler::CompileStatement()
{
	guard(FScriptCompiler::CompileStatement);
	UBOOL NeedSemicolon = 1;

	// Get a token and compile it.
	FToken Token;
	if( !GetToken(Token,NULL,1) )
	{
		// End of file.
		return 0;
	}
	else if( !CompileDeclaration( Token, NeedSemicolon ) )
	{
		if( Pass == 0 )
		{
			// Skip this and subsequent commands so we can hit them on next pass.
			if( NestLevel < 3 )
				appThrowf("Unexpected '%s'",Token.Identifier);
			UngetToken(Token);
			PopNest( TopNest->NestType, NestTypeName(TopNest->NestType) );
			SkipStatements( 1, NestTypeName(TopNest->NestType) );
			NeedSemicolon = 0;
		}
		else
		{
			// Compile the command.
			CompileCommand( Token, NeedSemicolon );
		}
	}

	// Make sure no junk is left over.
	if( NeedSemicolon )
	{
		if( !MatchSymbol(";") )
		{
			if( GetToken(Token) )	appThrowf( "Missing ';' before '%s'", Token.Identifier );
			else					appThrowf( "Missing ';'" );
		}
	}
	return 1;
	unguard;
}

//
// Compile multiple statements.
//
void FScriptCompiler::CompileStatements()
{
	guard(FScriptCompiler::CompileStatements);

	INT OriginalNestLevel = NestLevel;
	do CompileStatement();
	while( NestLevel > OriginalNestLevel );

	unguard;
}

/*-----------------------------------------------------------------------------
	Probe mask building.
-----------------------------------------------------------------------------*/

//
// Generate probe bitmask for a script.
//
void FScriptCompiler::PostParse( UStruct* Node )
{
	guard(FScriptCompiler::PostParse);

	// Allocate defaults.
	UClass* ThisClass = Cast<UClass>( Node );
	if( ThisClass )
		ThisClass->Defaults.AddZeroed( ThisClass->GetPropertiesSize() - ThisClass->Defaults.Num() );

	// Handle function.
	UFunction* ThisFunction = Cast<UFunction>( Node );
	if( ThisFunction )
	{
		ThisFunction->ParmsSize=0;
		for( TFieldIterator<UProperty> It(ThisFunction); It; ++It )
		{
			if( It->PropertyFlags & CPF_Parm )
				ThisFunction->ParmsSize = It->Offset + It->GetSize();
			if( It->PropertyFlags & CPF_ReturnParm )
				ThisFunction->ReturnValueOffset = It->Offset;
		}
	}

	// Accumulate probe masks based on all functions in this state.
	UState* ThisState = Cast<UState>( Node );
	if( ThisState )
	{
		for( TFieldIterator<UFunction> Function(ThisState); Function; ++Function )
		{
			if
			(	(Function->GetFName().GetIndex() >= NAME_PROBEMIN)
			&&	(Function->GetFName().GetIndex() <  NAME_PROBEMAX)
			&&  (Function->FunctionFlags & FUNC_Defined) )
				ThisState->ProbeMask |= (QWORD)1 << (Function->GetFName().GetIndex() - NAME_PROBEMIN);
		}
	}

	// Recurse with all child states in this class.
	for( TFieldIterator<UStruct> It(Node); It && It.GetStruct()==Node; ++It )
		PostParse( *It );

	unguard;
}

/*-----------------------------------------------------------------------------
	Code skipping.
-----------------------------------------------------------------------------*/

//
// Skip over code, honoring { and } pairs.
//
void FScriptCompiler::SkipStatements( int SubCount, const char* ErrorTag  )
{
	guard(FScriptCompiler::SkipStatements);
	FToken Token;
	while( SubCount>0 && GetToken( Token, NULL, 1 ) )
	{
		if		( Token.Matches("{") ) SubCount++;
		else if	( Token.Matches("}") ) SubCount--;
	}
	if( SubCount > 0 )
		appThrowf( "Unexpected end of file at end of %s", ErrorTag );
	unguard;
}

/*-----------------------------------------------------------------------------
	Main script compiling routine.
-----------------------------------------------------------------------------*/

//
// Perform a second-pass compile on the current class.
//
void FScriptCompiler::CompileSecondPass( UStruct* Node )
{
	guard(FScriptCompiler::CompileSecondPass);

	// Restore code pointer to where it was saved in the parsing pass.
	INT StartNestLevel = NestLevel;

	// Push this new nesting level.
	ENestType NewNest=NEST_None;
	if( Node->IsA(UFunction::StaticClass) )
		NewNest = NEST_Function;
	else if( Node == Class )
		NewNest = NEST_Class;
	else if( Node->IsA(UState::StaticClass) )
		NewNest = NEST_State;
	check(NewNest!=NEST_None);
	PushNest( NewNest, Node->GetName(), Node );
	check(TopNode==Node);

	// Propagate function replication flags down, since they aren't known until the second pass.
	UFunction* TopFunction = Cast<UFunction>( TopNode );
	if( TopFunction && TopFunction->GetSuperFunction() )
	{
		TopFunction->FunctionFlags &= ~FUNC_NetFuncFlags;
		TopFunction->FunctionFlags |= (TopFunction->GetSuperFunction()->FunctionFlags & FUNC_NetFuncFlags);
	}

	// If compiling the class node and an input line is specified, it's the replication defs.
	if( Node==Class && Node->Line!=MAXWORD )
	{
		// Remember input positions.
		InputPos  = PrevPos  = Node->TextPos;
		InputLine = PrevLine = Node->Line;

		// Compile all replication defs.
		while( 1 )
		{
			// Get Reliable or Unreliable.
			DWORD PropertyFlags = CPF_Net;
			DWORD FunctionFlags = FUNC_Net;
			FToken Token;
			GetToken( Token );
			if( Token.Matches( "}" ) )
			{
				break;
			}
			else if( Token.Matches(NAME_Reliable) )
			{
				PropertyFlags |= CPF_NetReliable;
				FunctionFlags |= FUNC_NetReliable;
			}
			else if( !Token.Matches(NAME_Unreliable) )
			{
				appThrowf( "Missing 'Reliable' or 'Unreliable'" );
			}
			if( MatchIdentifier("Always") )
			{
				PropertyFlags |= CPF_NetAlways;
			}

			// Compile optional conditional expression.
			_WORD RepOffset = MAXWORD;
			RequireIdentifier( "if", "Replication statement" );
			RequireSymbol( "(", "Replication condition" );
			RepOffset = TopNode->Script.Num();
			CompileExpr( FPropertyBase(CPT_Bool), "Replication condition" );
			RequireSymbol( ")", "Replication condition" );

			// Compile list of variables defined in this class and hook them into the
			// replication conditions.
			do
			{
				// Get variable name.
				FToken VarToken;
				if( !GetIdentifier(VarToken) )
					appThrowf( "Missing variable name in replication definition" );
				FName VarName = FName( VarToken.Identifier, FNAME_Find );
				if( VarName == NAME_None )
					appThrowf( "Unrecognized variable '%s' name in replication definition", VarToken.Identifier );

				// Find variable.
				UBOOL Found=0;
				for( TFieldIterator<UProperty> It(Class); It && It.GetStruct()==Class; ++It )
				{
					if( It->GetFName()==VarName )
					{
						// Found it, so make sure it's replicatable.
						if( It->PropertyFlags & CPF_Net )
							appThrowf( "Variable '%s' already has a replication definition", *VarName );

						// Set its properties.
						It->PropertyFlags |= PropertyFlags;
						It->RepOffset = RepOffset;
						Found = 1;
						break;
					}
				}
				if( !Found )
				{
					// Find function.
					for( TFieldIterator<UFunction> Function(Class); Function && Function.GetStruct()==Class; ++Function )
					{
						if( Function->GetFName()==VarName )
						{
							// Found it, so make sure it's replicable.
							if( Function->GetSuperFunction() )
								appThrowf( "Function '%s' is defined in base class '%s'", *VarName, Function->GetSuperFunction()->GetOwnerClass()->GetName() );
							if( Function->FunctionFlags & FUNC_Net )
								appThrowf( "Function '%s' already has a replication definition", *VarName );
							if( (Function->FunctionFlags&FUNC_Intrinsic) && (Function->FunctionFlags&FUNC_Final) )
								appThrowf( "Intrinsic final functions may not be replicated" );

							// Set its properties.
							Function->FunctionFlags  |= FunctionFlags;
							Function->RepOffset       = RepOffset;
							Found                     = 1;
							break;
						}
					}
				}
				if( !Found )
					appThrowf( "Bad variable or function '%s' in replication definition", *VarName );
			} while( MatchSymbol( "," ) );

			// Semicolon.
			RequireSymbol( ";", "Replication definition" );
		}
	}

	// Compile all child functions in this class or state.
	for( TFieldIterator<UStruct> Child(Node); Child && Child.GetStruct()==Node; ++Child )
		if( Child->GetClass() != UStruct::StaticClass )
			CompileSecondPass( *Child );
	check(TopNode==Node);

	// Prepare for function or state compilation.
	UBOOL DoCompile=0;
	if( Node->Line!=MAXWORD && Node!=Class )
	{
		// Remember input positions.
		InputPos  = PrevPos  = Node->TextPos;
		InputLine = PrevLine = Node->Line;

		// Emit function parms info into code stream.
		UFunction* TopFunction = Cast<UFunction>( TopNode );
		UState*    TopState    = Cast<UState   >( TopNode );
		if( TopFunction && !(TopFunction->FunctionFlags & FUNC_Intrinsic) )
		{
			Writer << EX_BeginFunction;
			for( TFieldIterator<UProperty> It(TopFunction); It && (It->PropertyFlags&(CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It )
			{
				// Emit parm size into code stream.
				UProperty* Property = *It;
				INT Size = Property->GetSize();
				TFieldIterator<UProperty> Next(It);
				++Next;
				if( Next && (Next->PropertyFlags&(CPF_Parm|CPF_ReturnParm))==CPF_Parm )
					Size += Next->Offset - (Property->Offset + Property->GetSize());
				BYTE bSize = Size;
				if( bSize != Size )
					appThrowf( "Parameter '%s' is too large", Property->GetName() );
				check(bSize!=0);
				Writer << bSize;

				// Emit outparm flag into code stream.
				BYTE bOutParm = (Property->PropertyFlags & CPF_OutParm) ? 1 : 0;
				Writer << bOutParm;
			}
			BYTE bEnd = 0;
			Writer << bEnd;

			// Should we compile any code?
			if( TopFunction )
				DoCompile = (TopFunction->FunctionFlags & FUNC_Defined);
		}
		else if( TopFunction )
		{
			// Inject intrinsic function parameter offsets.
			for( TFieldIterator<UProperty> It(TopFunction); It && (It->PropertyFlags&(CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It )
			{
				UProperty* Property = *It;
				Writer << EX_IntrinsicParm;
				Writer << Property;
			}
		}
		else if( TopState )
		{
			// Compile the code.
			DoCompile = 1;
		}
	}

	if( DoCompile )
	{
		// Compile statements until we get back to our original nest level.
		LinesCompiled -= InputLine;
		while( NestLevel > StartNestLevel )
			if( !CompileStatement() )
				appThrowf( "Unexpected end of code in %s", Node->GetClassName() );
		LinesCompiled += InputLine;
	}
	else if( Node!=Class )
	{
		// Pop the nesting.
		PopNest( NewNest, Node->GetClassName() );
	}
	unguardf(( "(%s)", Node->GetFullName() ));
}

void InitReplication( UClass* Class, UStruct* Node )
{
	guard(InitReplication);
	UFunction* Function = Cast<UFunction>( Node );
	if( Function )
	{
		Function->FunctionFlags &= ~(FUNC_NetFuncFlags);
		Function->RepOffset = MAXWORD;
	}
	for( TFieldIterator<UStruct> Child(Node); Child && Child.GetStruct()==Node; ++Child )
	{
		InitReplication( Class, *Child );
	}
	unguard;
}

//
// Compile the script associated with the specified class.
// Returns 1 if compilation was a success, 0 if any errors occured.
//
UBOOL FScriptCompiler::CompileScript
(
	UClass*		InClass,
	FMemStack*	InMem,
	UBOOL		InBooting,
	INT			InPass
)
{
	guard(FScriptCompiler::CompileScript);
	Booting       = InBooting;
	Class         = InClass;
	Pass	      = InPass;
	Mem           = InMem;
	UBOOL Success  = 0;
	FMemMark Mark(*Mem);

	// Save directory and switch into package directory.
	char SavedDir[256], NewDir[256];
	appSprintf( NewDir, "%s\\Source\\%s",appBaseDir(), InClass->GetParent()->GetName() );
	appGetcwd( SavedDir, ARRAY_COUNT(SavedDir) );
	appChdir( NewDir );

	// Message.
	guard(InitMessage);
	GSystem->StatusUpdatef( 0, 0, "%s %s...", Pass ? "Compiling" : "Parsing", Class->GetName() );
	debugf( NAME_Log, "%s %s...", Pass ? "Compiling" : "Parsing", Class->GetName() );
	unguard;

	// Make sure our parent classes is parsed.
	guard(CheckParsed);
	for( UClass *Temp = Class->GetSuperClass(); Temp; Temp=Temp->GetSuperClass() )
		if( !(Temp->ClassFlags & CLASS_Parsed) )
			appThrowf( "'%s' can't be compiled: Parent class '%s' has errors", Class->GetName(), Temp->GetName() );
	unguard;

	// Init class.
	check(!(Class->ClassFlags & CLASS_Compiled));
	if( Pass == 0 )
	{
		// First pass.
		guard(InitFirstPass);
		Class->Script.Empty();
		Class->Defaults.Empty();
		Class->Dependencies.Empty();
		Class->PropertiesSize=0;

		// Set class flags.
		Class->ClassFlags &= ~CLASS_Inherit;
		if( Class->GetSuperClass() )
			Class->ClassFlags |= (Class->GetSuperClass()->ClassFlags) & CLASS_Inherit;

		unguard;
	}
	else
	{
		// Second pass.
		guard(InitSecondPass);

		// Replace the script.
		Class->Script.Empty();

		// Init the replication defs.
		for( TFieldIterator<UProperty> It(Class); It && It.GetStruct()==Class; ++It )
		{
			It->PropertyFlags &= ~(CPF_NetFlags);
			It->RepOffset = MAXWORD;
		}
		InitReplication( Class, Class );
		unguard;
	}

	// Init compiler variables.
	guard(InitCompiler);
	OriginalPropertiesSize = Class->GetPropertiesSize();
	AllowIterator = 0;
	Input		  = *Class->ScriptText->Text;
	InputPos	  = 0;
	InputLine	  = 1;
	PrevPos		  = 0;
	PrevLine	  = 1;
	unguard;

	// Init nesting.
	guard(InitNesting);
	NestLevel	= 0;
	TopNest		= &Nest[-1];
	PushNest( NEST_None, "", NULL );
	unguard;

	// Try to compile it, and catch any errors.
	guard(TryCompile);
	try
	{
		// Compile until we get an error.
		if( Pass == 0 )
		{
			// Parse entire program.
			guard(FirstPass);
			while( CompileStatement() )
				StatementsCompiled++;

			// Precompute info for runtime optimization.
			LinesCompiled += InputLine;

			// Stub out the script.
			Class->Script.Empty();
			Class->ClassFlags |= CLASS_Parsed;
			unguard;
		}
		else
		{
			// Compile all uncompiled sections of parsed code.
			guard(SecondPass);
			CompileSecondPass( Class );

			// Mark as compiled.
			Class->ClassFlags |= CLASS_Compiled;
			unguard;
		}

		// Make sure the compilation ended with valid nesting.
		if     ( NestLevel==0 )	appThrowf ( "Internal nest inconsistency" );
		else if( NestLevel==1 )	appThrowf ( "Missing 'Class' definition" );
		else if( NestLevel==2 ) PopNest( NEST_Class, "'Class'" );
		else if( NestLevel >2 ) appThrowf ( "Unexpected end of script in '%s' block", NestTypeName(TopNest->NestType) );

		// Cleanup after first pass.
		if( Pass == 0 )
		{
			// Finish parse.
			PostParse( Class );
			Class->Dependencies.Shrink();
			Class->PackageImports.Shrink();
			Class->Defaults.Shrink();
			check(Class->GetPropertiesSize()==Class->Defaults.Num());

			// Check intrinsic size.
			guard(FirstPassCleanup);
			if
			(  (Class->GetFlags() & RF_Intrinsic)
			&&	OriginalPropertiesSize
			&&	Align(Class->Defaults.Num(),4)!=OriginalPropertiesSize
			&&	GCheckIntrinsics )
			{
				GSystem->Warnf( "Intrinsic class %s size mismatch (script %i, C++ %i)", Class->GetName(), Class->Defaults.Num(), OriginalPropertiesSize );
				GCheckIntrinsics = 0;
			}

			// First-pass success.
			Class->GetDefaultObject()->SetClass( Class );
			unguard;
		}
		if( Pass==0 || GEditor->Bootstrapping )
		{
			// Update the default object.
			guard(RestoreProperties);
			if( Class->DefaultPropText )
				ImportProperties( Class, &Class->Defaults(0), (ULevel*)NULL, *Class->DefaultPropText->Text, Class->GetParent() );
			unguard;
		}
		Success = 1;
	}
	catch( char* ErrorMsg )
	{
		// All errors are critical when booting.
		guard(CompileError);
		if( GEditor->Bootstrapping )
			appErrorf( "Error in %s, Line %i: %s", Class->GetName(), InputLine, ErrorMsg );

		// Handle compiler error.
		AddResultText( "Error in %s, Line %i: %s\r\n", Class->GetName(), InputLine, ErrorMsg );

		// Invalidate this class and scrap its dependencies.
		Class->ClassFlags &= ~(CLASS_Parsed | CLASS_Compiled);
		Class->Script.Empty();
		Class->Dependencies.Empty();

		unguard;
	}
	unguard;

	// Clean up and exit.
	guard(Cleanup);
	Class->Bind();
	Mark.Pop();
	appChdir( SavedDir );
	unguard;

	return Success;
	unguardf(( "(%s, Pass %i, Line %i)", Class->GetFullName(), InPass, InputLine ));
}

/*-----------------------------------------------------------------------------
	FScriptCompiler error handling.
-----------------------------------------------------------------------------*/

//
// Print a formatted debugging message (same format as printf).
//
void VARARGS FScriptCompiler::AddResultText( char* Fmt, ... )
{
	char TempStr[4096];
	GET_VARARGS(TempStr,Fmt);
	debugf( NAME_Log, TempStr );
	if( ErrorText )
		ErrorText->Log( TempStr );
};

/*-----------------------------------------------------------------------------
	Global functions.
-----------------------------------------------------------------------------*/

//
// Guarantee that Class and all its child classes are CLASS_Compiled and return 1, 
// or 0 if error.
//
static UBOOL ParseScripts( TArray<UClass*>& AllClasses, FScriptCompiler& Compiler, UClass* Class, UBOOL MakeAll, UBOOL Booting )
{
	guard(MakeScript);
	check(Class!=NULL);
	if( !Class->ScriptText )
		return 1;

	// First-pass compile this class if needed.
	if( MakeAll )
		Class->ClassFlags &= ~CLASS_Parsed;
	if( !(Class->ClassFlags & CLASS_Parsed) )
		if( !Compiler.CompileScript( Class, &GMem, Booting, 0 ) )
			return 0;
	check(Class->ClassFlags & CLASS_Parsed);

	// First-pass compile subclasses.
	for( TPtrIterator<UClass> It(AllClasses); It; ++It )
		if( It->GetSuperClass()==Class && !ParseScripts( AllClasses, Compiler, *It, MakeAll, Booting ) )
			return 0;

	// Success.
	return 1;
	unguard;
}

//
// Hierarchically recompile all scripts.
//
static UBOOL CompileScripts( TArray<UClass*>& AllClasses, FScriptCompiler& Compiler, UClass* Class )
{
	guard(CompileScripts);

	// Compile it.
	if( Class->ScriptText )
	{
		if( !(Class->ClassFlags & CLASS_Compiled) )
			if( !Compiler.CompileScript( Class, &GMem, 0, 1 ) )
				return 0;
		check(Class->ClassFlags & CLASS_Compiled);
	}

	// Compile subclasses.
	for( TPtrIterator<UClass> It(AllClasses); It; ++It )
		if( It->GetSuperClass()==Class && !CompileScripts( AllClasses, Compiler, *It ) )
			return 0;

	// Success.
	return 1;
	unguard;
}

//
// Make all scripts.
// Returns 1 if success, 0 if errors.
// Not recursive.
//
UBOOL UEditorEngine::MakeScripts( UBOOL MakeAll, UBOOL Booting )
{
	guard(UEditorEngine::MakeScripts);
	FMemMark Mark(GMem);
	FTransaction Transaction;
	FScriptCompiler	Compiler;

	// Make list of all classes.
	TArray<UClass*> AllClasses;
	for( TObjectIterator<UClass> ObjIt; ObjIt; ++ObjIt )
		AllClasses.AddItem( *ObjIt );

	// Find any classes whose scripts have changed and mark them uncompiled and unlinked.
	guard(DowngradeClasses);
	if( Compiler.ShowDep )
		debugf( NAME_DevCompile, "DowngradeClasses:" );
	for( TPtrIterator<UClass> It(AllClasses); It; ++It )
	{
		if
		(	It->ScriptText
		&&	(It->Dependencies.Num()==0 || It->GetScriptTextCRC()!=It->Dependencies(0).ScriptTextCRC || MakeAll) )
		{
			if( Compiler.ShowDep )
			{
				if( MakeAll ) debugf( NAME_DevCompile, "   MakeAll: %s", It->GetName() );
				else          debugf( NAME_DevCompile, "   Uncompiled: %s", It->GetName() );
			}
			It->ClassFlags &= ~(CLASS_Compiled | CLASS_Parsed);
		}
	}
	unguard;

	// Find any classes with unparsed parents and mark them unparsed.
	guard(DowngradeParents);
	for( TPtrIterator<UClass> It(AllClasses); It; ++It )
	{
		if( It->ScriptText )
		{
			for( UClass* Parent=It->GetSuperClass(); Parent; Parent=Parent->GetSuperClass() )
			{
				if( Parent->ScriptText && !(Parent->ClassFlags & CLASS_Parsed) )
				{
					if( Compiler.ShowDep )
						debugf( NAME_DevCompile, "   Parent unparsed: %s", It->GetName() );
					It->ClassFlags &= ~(CLASS_Parsed|CLASS_Compiled);
				}
			}
		}
	}
	unguard;

	// Find any classes with unparsed dependencies and mark them as uncompiled.
	guard(DowngradeDependencies);
	for( TPtrIterator<UClass> It(AllClasses); It; ++It )
	{
		// If this class has unparsed dependencies, mark it uncompiled.
		if( It->ScriptText )
		{
			for( INT i=1; i<It->Dependencies.Num(); i++ )
			{
				if( !(It->Dependencies(i).Class->ClassFlags & CLASS_Parsed) )
				{
					if( Compiler.ShowDep )
						debugf( NAME_DevCompile, "   Uncompiled due to dependency on %s: %s", It->Dependencies(i).Class->GetName(), It->GetName() );
					It->ClassFlags &= ~CLASS_Compiled;
				}
			}
		}
	}
	unguard;

	// Export properties of objects we're going to reparse.
	guard(ExportObjects);
	for( TPtrIterator<UClass> It(AllClasses); It; ++It )
	{
		// Export properties.
		if
		(	It->ScriptText
		&&	!(It->ClassFlags & CLASS_Parsed)
		&&	!It->DefaultPropText
		&&	It->Defaults.Num() )
		{
			It->DefaultPropText = new UTextBuffer;
			GObj.ExportProperties( *It, &It->Defaults(0), It->DefaultPropText, 0, It->GetSuperClass(), It->GetSuperClass() ? (BYTE*)It->GetSuperClass()->GetDefaultObject() : NULL );
		}

		// Save the class.
		if
		(	It->ScriptText
		&&	(!(It->ClassFlags & CLASS_Parsed) || !(It->ClassFlags & CLASS_Compiled)) )
			RecursiveSaveField( Transaction, *It );
	}
	unguard;

	// Do compiling.
	Compiler.ShowDep            = ParseParam( appCmdLine(), "SHOWDEP" );
	Compiler.StatementsCompiled	= 0;
	Compiler.LinesCompiled		= 0;
	Compiler.ErrorText			= Results;
	if( Compiler.ErrorText )
		Compiler.ErrorText->Text.Empty();

	// Hierarchically parse and compile all classes.
	UBOOL Success=0;
	guard(DoScripts);
	Success
	=	ParseScripts( AllClasses, Compiler, UObject::StaticClass, MakeAll, Booting )
	&&	CompileScripts( AllClasses, Compiler, UObject::StaticClass );
	unguard;

	// Done with make.
	if( Success )
	{
		// Success.
		if( Compiler.LinesCompiled )
			Compiler.AddResultText( "Success: Compiled %i line(s), %i statement(s).\r\n", Compiler.LinesCompiled, Compiler.StatementsCompiled );
		else
			Compiler.AddResultText( "Success: Everything is up to date" );
	}
	else
	{
		// Restore all classes after compile fails.
		Transaction.Restore();
	}
	guard(CleanupPropText);
	for( TPtrIterator<UClass> It(AllClasses); It; ++It )
	{
		// Cleanup all exported property text.
		if( It->DefaultPropText )
			delete It->DefaultPropText;
		It->DefaultPropText = NULL;
	}
	unguard;
	Mark.Pop();
	return Success;
	unguard;
}

//
// Verify that all scripts are up to date.
// Returns 1 if so, 0 if not.
//
int UEditorEngine::CheckScripts( UClass *Class, FOutputDevice &Out )
{
	guard(UEditorEngine::CheckScripts);
	check(Class!=NULL);

	// Skip if not a scripted class.
	if( Class->Dependencies.Num()==0 )
		return 1;
	check(Class->Dependencies(0).Class==Class);

	// Make sure this class is parsed.
	if( !(Class->ClassFlags & CLASS_Parsed) )
	{
		Out.Logf( "Class %s is unparsed",Class->GetName() );
		return 0;
	}

	// Make sure this class is compiled.
	if( !(Class->ClassFlags & CLASS_Compiled) )
	{
		Out.Logf( "Class %s is uncompiled", Class->GetName() );
		return 0;
	}

	// Check all dependencies.
	for( INT i=0; i<Class->Dependencies.Num(); i++ )
	{
		if( !Class->Dependencies(i).IsUpToDate() )
		{
			if( i==0 )
				Out.Logf( "Class %s is out of date", Class->GetName() );
			else if( i==1 && Class->GetSuperClass() )
				Out.Logf( "Class %s's parent is out of date", Class->GetName() );
			else
				Out.Logf( "Class %s's dependency %s is out of date", Class->GetName(), Class->Dependencies(i).Class->GetName() );
			return 0;
		}
	}

	// Check all child class scripts.
	for( TObjectIterator<UClass> It; It; ++It )
		if( It->GetSuperClass()==Class && !CheckScripts(*It,Out) )
			return 0;

	// Everything here is up to date.
	return 1;
	unguardf(( "(%s)", Class->GetName() ));
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

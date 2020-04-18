/*=============================================================================
	UnClsPrp.cpp: FProperty implementation
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "CorePrivate.h"

/*-----------------------------------------------------------------------------
	Helpers.
-----------------------------------------------------------------------------*/

//
// Parse a hex digit.
//
static int HexDigit( char c )
{
	if( c>='0' && c<='9' )
		return c - '0';
	else if( c>='a' && c<='f' )
		return c + 10 - 'a';
	else if( c>='A' && c<='F' )
		return c + 10 - 'A';
	else
		return 0;
}

//
// Parse a token.
//
const char* ReadToken( const char* Buffer, char* String, UBOOL DottedNames=0 )
{
	int Count=0;
	if( *Buffer == 0x22 )
	{
		// Get quoted string.
		Buffer++;
		while( *Buffer && *Buffer!=0x22 && *Buffer!=13 && *Buffer!=10 && Count<255 )
		{
			if( *Buffer != '\\' )
			{
				String[Count++] = *Buffer++;
			}
			else
			{
				String[Count++] = HexDigit(Buffer[0])*16 + HexDigit(Buffer[1]);
				Buffer += 3;
			}
		}
		if( Count==255 )
		{
			debugf( NAME_Warning, "ReadToken: Quoted string too long" );
			return NULL;
		}
		if( *Buffer++!=0x22 )
		{
			debugf( NAME_Warning, "ReadToken: Bad quoted string" );
			return NULL;
		}
	}
	else if( appIsAlnum( *Buffer ) )
	{
		// Get identifier.
		while
		(	(appIsAlnum(*Buffer) || *Buffer=='_' || *Buffer=='-' || (DottedNames && *Buffer=='.' ))
		&&	Count<255 )
			String[Count++] = *Buffer++;
		if( Count==255 )
		{
			debugf( NAME_Warning, "ReadToken: Alphanumeric overflow" );
			return NULL;
		}
			
	}
	else
	{
		// Get single character.
		String[Count++] = *Buffer;
	}
	String[Count] = 0;
	return Buffer;
}

/*-----------------------------------------------------------------------------
	UProperty implementation.
-----------------------------------------------------------------------------*/

//
// Constructors.
//
UProperty::UProperty()
:	UField( NULL )
,	ArrayDim( 1 )
{}

//
// Serializer.
//
void UProperty::Serialize( FArchive& Ar )
{
	guard(UProperty::Serialize);
	UField::Serialize( Ar );

	// Archive the basic info.
	Ar << ArrayDim << PropertyFlags << Category;
	if( PropertyFlags & CPF_Net )
		Ar << RepOffset;
	if( Ar.IsLoading() )
		Offset = 0;

	unguard;
}

//
// Export this class property to an output
// device as a C++ header file.
//
void UProperty::ExportCPP( FOutputDevice& Out, UBOOL IsLocal, UBOOL IsParm )
{
	guard(UProperty::ExportCPP)
	char ArrayStr[80]="";
	if( IsParm && IsA(UStringProperty::StaticClass) && !(PropertyFlags & CPF_OutParm) )
		Out.Log( "const " );
	ExportCPPItem( Out );
	if( ArrayDim != 1 )
		appSprintf( ArrayStr, "[%i]", ArrayDim );
	if( IsParm )
	{
		if( IsA(UStringProperty::StaticClass) )
			Out.Log( "*" );
		else if( PropertyFlags & CPF_OutParm )
			Out.Log( "&" );
	}
	if( IsA(UBoolProperty::StaticClass) )
	{
		if( ArrayDim==1 && !IsLocal && !IsParm )
			Out.Logf( " %s%s:1", GetName(), ArrayStr );
		else
			Out.Logf( " %s%s", GetName(), ArrayStr );
	}
	else if( IsA(UStringProperty::StaticClass) )
	{
		if( IsParm )
			Out.Logf( " %s%s", GetName(), ArrayStr );
		else
			Out.Logf( " %s[%i]%s", GetName(), Cast<UStringProperty>(this)->StringSize, ArrayStr );
	}
	else
	{
		Out.Logf( " %s%s", GetName(), ArrayStr );
	}
	unguard;
}

//
// Export the contents of a property.
//
UBOOL UProperty::ExportText
(
	INT		Index,
	char*	ValueStr,
	BYTE*	Data,
	BYTE*	Delta,
	UBOOL	HumanReadable
)
{
	guard(UProperty::ExportText);
	ValueStr[0]=0;
	if( Data==Delta || !Matches(Data,Delta,Index) )
	{
		ExportTextItem
		(
			ValueStr,
			Data + Offset + Index * GetElementSize(),
			Delta ? Delta + Offset + Index * GetElementSize() : NULL,
			HumanReadable
		);
		return 1;
	}
	else return 0;
	unguard;
}

//
// UnrealScript assignment.
//
void UProperty::ExecLet( void* Var, FFrame& Stack )
{
	guardSlow(UProperty::ExecLet);

	// Assign value.
	BYTE Buffer[MAX_CONST_SIZE], *Val=Buffer;
	Stack.Step( Stack.Object, Val );
	if( Var )
		appMemcpy( Var, Val, GetElementSize() );

	unguardSlow;
}

IMPLEMENT_CLASS(UProperty);

/*-----------------------------------------------------------------------------
	UByteProperty.
-----------------------------------------------------------------------------*/

void UByteProperty::Serialize( FArchive& Ar )
{
	guard(UByteProperty::Serialize);
	UProperty::Serialize( Ar );
	Ar << Enum;
	unguard;
}
void UByteProperty::ExportCPPItem( FOutputDevice& Out ) const
{
	guard(UByteProperty::ExportCPPItem);
	Out.Log( "BYTE" );
	unguard;
}
void UByteProperty::ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable )
{
	guard(UByteProperty::ExportTextItem);
	if( Enum )
		appSprintf( ValueStr, "%s", *Enum->Names(*(BYTE*)PropertyValue) );
	else
		appSprintf( ValueStr, "%i", *(BYTE*)PropertyValue );
	unguard;
}
const char* UByteProperty::ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const
{
	guard(UByteProperty::ImportText);
	char Temp[256];
	if( Enum )
	{
		Buffer = ReadToken( Buffer, Temp );
		if( !Buffer )
			return NULL;
		FName EnumName = FName( Temp, FNAME_Find );
		if( EnumName != NAME_None )
		{
			INT EnumIndex=0;
			if( Enum->Names.FindItem( EnumName, EnumIndex ) )
			{
				*(BYTE*)Data = EnumIndex;
				return Buffer;
			}
		}
	}
	if( appIsDigit(*Buffer) )
	{
		*(BYTE*)Data = appAtoi( Buffer );
		while( *Buffer>='0' && *Buffer<='9' )
			Buffer++;
	}
	else
	{
		//debugf( "Import: Missing byte" );
		return NULL;
	}
	return Buffer;
	unguard;
}
IMPLEMENT_CLASS(UByteProperty);

/*-----------------------------------------------------------------------------
	UIntProperty.
-----------------------------------------------------------------------------*/

void UIntProperty::ExportCPPItem( FOutputDevice& Out ) const
{
	guard(UIntProperty::ExportCPPItem);
	Out.Log( "INT" );
	unguard;
}
void UIntProperty::ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable )
{
	guard(UIntProperty::ExportTextItem);
	appSprintf( ValueStr, "%i", *(INT *)PropertyValue );
	unguard;
}
const char* UIntProperty::ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const
{
	guard(UIntProperty::ImportText);
	if( *Buffer=='-' || (*Buffer>='0' && *Buffer<='9') )
		*(INT*)Data = appAtoi( Buffer );
	while( *Buffer=='-' || (*Buffer>='0' && *Buffer<='9') )
		Buffer++;
	return Buffer;
	unguard;
}
IMPLEMENT_CLASS(UIntProperty);

/*-----------------------------------------------------------------------------
	UBoolProperty.
-----------------------------------------------------------------------------*/

void UBoolProperty::Serialize( FArchive& Ar )
{
	guard(UBoolProperty::Serialize);
	UProperty::Serialize( Ar );
	if( !Ar.IsLoading() && !Ar.IsSaving() )
		Ar << BitMask;
	unguard;
}
void UBoolProperty::ExportCPPItem( FOutputDevice& Out ) const
{
	guard(UBoolProperty::ExportCPPItem);
	Out.Log( "DWORD" );
	unguard;
}
void UBoolProperty::ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable )
{
	guard(UBoolProperty::ExportTextItem);
	char *Temp = ((*(DWORD *)PropertyValue) & BitMask) ? "True" : "False";
	appSprintf( ValueStr, "%s", Temp );
	unguard;
}
const char* UBoolProperty::ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const
{
	guard(UBoolProperty::ImportText);
	char Temp[256];
	Buffer = ReadToken( Buffer, Temp );
	if( !Buffer )
		return NULL;
	if( appStricmp(Temp,"1")==0 || appStricmp(Temp,"True")==0 )
	{
		*(DWORD*)Data |= BitMask;
	}
	else if( appStricmp(Temp,"0")==0 || appStricmp(Temp,"False")==0 )
	{
		*(DWORD*)Data &= ~BitMask;
	}
	else
	{
		//debugf( "Import: Failed to get bool" );
		return NULL;
	}
	return Buffer;
	unguard;
}
void UBoolProperty::ExecLet( void* Var, FFrame& Stack )
{
	guardSlow(UBoolProperty::ExecLet);

	// Safely set the bool.
	DWORD* BoolAddr = GBoolAddr;
	BYTE Buffer[MAX_CONST_SIZE], *Val=Buffer;
	Stack.Step( Stack.Object, Val );
	if( BoolAddr )
	{
		if( *(DWORD*)Val ) *BoolAddr |=  BitMask;
		else               *BoolAddr &= ~BitMask;
	}
	unguardSlow;
}
IMPLEMENT_CLASS(UBoolProperty);

/*-----------------------------------------------------------------------------
	UFloatProperty.
-----------------------------------------------------------------------------*/

void UFloatProperty::ExportCPPItem( FOutputDevice& Out ) const
{
	guard(UFloatProperty::ExportCPPItem);
	Out.Log( "FLOAT" );
	unguard;
}
void UFloatProperty::ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable )
{
	guard(UFloatProperty::ExportTextItem);
	appSprintf( ValueStr, "%f", *(FLOAT*)PropertyValue );
	unguard;
}
const char* UFloatProperty::ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const
{
	guard(UFloatProperty::ImportText);
	*(FLOAT*)Data = appAtof(Buffer);
	while( *Buffer && *Buffer!=',' && *Buffer!=')' && *Buffer!=13 && *Buffer!=10 )
		Buffer++;
	return Buffer;
	unguard;
}
IMPLEMENT_CLASS(UFloatProperty);

/*-----------------------------------------------------------------------------
	UObjectProperty.
-----------------------------------------------------------------------------*/

void UObjectProperty::Serialize( FArchive& Ar )
{
	guard(UObjectProperty::Serialize);
	UProperty::Serialize( Ar );
	Ar << PropertyClass;
	unguard;
}
void UObjectProperty::ExportCPPItem( FOutputDevice& Out ) const
{
	guard(UObjectProperty::ExportCPPItem);
	Out.Logf( "class %s*", PropertyClass->GetNameCPP() );
	unguard;
}
void UObjectProperty::ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable )
{
	guard(UObjectProperty::ExportTextItem);
	UObject* Temp = *(UObject **)PropertyValue;
	if( Temp != NULL )
		appSprintf( ValueStr, "%s'%s'", Temp->GetClassName(), Temp->GetPathName() );
	else
		appStrcpy( ValueStr, "None" );
	unguard;
}
const char* UObjectProperty::ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const
{
	guard(UObjectProperty::ImportText);
	char Temp[256], Other[256];
	Buffer = ReadToken( Buffer, Temp, 1 );
	if( !Buffer )
	{
		return NULL;
	}
	if( appStricmp( Temp, "None" )==0 )
	{
		*(UObject**)Data = NULL;
	}
	else
	{
		while( *Buffer == ' ' )
			Buffer++;
		if( *Buffer++ != '\'' )
		{
			*(UObject**)Data = GObj.FindObject( PropertyClass, ANY_PACKAGE, Temp );
			if( !*(UObject**)Data )
				return NULL;
		}
		else
		{
			Buffer = ReadToken( Buffer, Other, 1 );
			if( !Buffer )
				return NULL;
			if( *Buffer++ != '\'' )
				return NULL;
			UClass* ObjectClass = FindObject<UClass>( ANY_PACKAGE, Temp );
			if( !ObjectClass )
				return NULL;
			*(UObject**)Data = GObj.FindObject( ObjectClass, ANY_PACKAGE, Other );
			if( !*(UObject**)Data )
				return NULL;
		}
	}
	return Buffer;
	unguard;
}
IMPLEMENT_CLASS(UObjectProperty);

/*-----------------------------------------------------------------------------
	UClassProperty.
-----------------------------------------------------------------------------*/

void UClassProperty::Serialize( FArchive& Ar )
{
	guard(UClassProperty::Serialize);
	UObjectProperty::Serialize( Ar );
	Ar << MetaClass;
	check(MetaClass);
	unguard;
}
const char* UClassProperty::ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const
{
	const char* Result = UObjectProperty::ImportText( Buffer, Data, HumanReadable );
	/*if( Result )
	{
		// Validate.
		UClass*& C = *(UClass**)Data;
		if( C )
		{
			if( C->GetClass()!=UClass::StaticClass || !C->IsChildOf(MetaClass) )
				C = NULL;
		}
	}*/
	return Result;
}
IMPLEMENT_CLASS(UClassProperty);

/*-----------------------------------------------------------------------------
	UNameProperty.
-----------------------------------------------------------------------------*/

void UNameProperty::ExportCPPItem( FOutputDevice& Out ) const
{
	guard(UNameProperty::ExportCPPItem);
	Out.Log( "FName" );
	unguard;
}
void UNameProperty::ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable )
{
	guard(UNameProperty::ExportTextItem);
	FName Temp = *(FName *)PropertyValue;
	appStrcpy( ValueStr, *Temp );
	unguard;
}
const char* UNameProperty::ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const
{
	guard(UNameProperty::ImportText);
	char Temp[256];
	Buffer = ReadToken( Buffer, Temp );
	if( !Buffer )
		return NULL;
	*(FName*)Data = FName(Temp);
	return Buffer;
	unguard;
}
IMPLEMENT_CLASS(UNameProperty);

/*-----------------------------------------------------------------------------
	UStringProperty.
-----------------------------------------------------------------------------*/

void UStringProperty::Serialize( FArchive& Ar )
{
	guard(UStringProperty::Serialize);
	UProperty::Serialize( Ar );
	Ar << StringSize;
	unguard;
}
void UStringProperty::ExportCPPItem( FOutputDevice& Out ) const
{
	guard(UStringProperty::ExportCPPItem);
	Out.Log( "CHAR" );
	unguard;
}
void UStringProperty::ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable )
{
	guard(UStringProperty::ExportTextItem);
	if( HumanReadable )
		appStrcpy( ValueStr, (char*)PropertyValue );
	else
		appSprintf( ValueStr, "\"%s\"", (char*)PropertyValue );
	unguard;
}
const char* UStringProperty::ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const
{
	guard(UStringProperty::ImportText);
	char Temp[256];
	if( HumanReadable )
	{
		appStrncpy( (char*)Data, Buffer, StringSize );
	}
	else
	{
		Buffer = ReadToken( Buffer, Temp );
		if( !Buffer )
			return NULL;
		appStrncpy( (char*)Data, Temp, StringSize );
	}
	return Buffer;
	unguard;
}
void UStringProperty::ExecLet( void* Var, FFrame& Stack )
{
	guardSlow(UStringProperty::ExecLet);

	// Assign value.
	BYTE Buffer[MAX_STRING_CONST_SIZE], *Val=Buffer;
	Stack.Step( Stack.Object, Val );
	if( Var )
		appStrcpy( (char*)Var, (char*)Val );

	unguardSlow;
}
IMPLEMENT_CLASS(UStringProperty);

/*-----------------------------------------------------------------------------
	UStructProperty.
-----------------------------------------------------------------------------*/

void UStructProperty::Serialize( FArchive& Ar )
{
	guard(UStructProperty::Serialize);
	UProperty::Serialize( Ar );
	Ar << Struct;
	unguard;
}
void UStructProperty::ExportCPPItem( FOutputDevice& Out ) const
{
	guard(UStructProperty::ExportCPPItem);
	Out.Logf( "%s", Struct->GetNameCPP() );
	unguard;
}
void UStructProperty::ExportTextItem( char* ValueStr, BYTE* PropertyValue, BYTE* DefaultValue, UBOOL HumanReadable )
{
	guard(UStructProperty::ExportTextItem);
	if( Struct->GetFName()==NAME_DynamicString )
	{
		PropertyValue = (BYTE*)**(FString*)PropertyValue;
		if( !PropertyValue )
			PropertyValue = (BYTE*)"";
		if( !HumanReadable )
			*ValueStr++ = 0x22;
		for( PropertyValue; *PropertyValue!=0; PropertyValue++ )
		{
			if( *PropertyValue>=32 && *PropertyValue<127 )
				*ValueStr++ = *PropertyValue;
			else ValueStr += appSprintf( ValueStr, "\\02X", *PropertyValue );
		}
		if( !HumanReadable )
			*ValueStr++ = 0x22;
		*ValueStr = 0;
	}
	else
	{
		INT Count=0;
		for( TFieldIterator<UProperty> It(Struct); It; ++It )
		{
			if( It->Port() )
			{
				for( INT Index=0; Index<It->ArrayDim; Index++ )
				{
					char Value[1024];
					if( It->ExportText(Index,Value,PropertyValue,DefaultValue,0) )
					{
						Count++;
						if( Count == 1 )
							*ValueStr++ = '(';
						else
							*ValueStr++ = ',';
						if( It->ArrayDim == 1 )
							ValueStr += appSprintf( ValueStr, "%s=", It->GetName() );
						else
							ValueStr += appSprintf( ValueStr, "%s[%i]=", It->GetName(), Index );
						ValueStr += appSprintf( ValueStr, "%s", Value );
					}
				}
			}
		}
		if( Count > 0 )
		{
			*ValueStr++ = ')';
			*ValueStr = 0;
		}
	}
	unguard;
}
const char* UStructProperty::ImportText( const char* Buffer, BYTE* Data, UBOOL HumanReadable ) const
{
	guard(UStructProperty::ImportText);
	if( Struct->GetFName()==NAME_DynamicString )
	{
		char Temp[256]="";
		if( HumanReadable )
		{
			appStrncpy( Temp, Buffer, ARRAY_COUNT(Temp) );
		}
		else
		{
			Buffer = ReadToken( Buffer, Temp );
			if( !Buffer )
				return NULL;
		}
		*(FString*)Data = Temp;
		return Buffer;
	}
	else
	{
		// Import the struct.
		if( *Buffer++ == '(' )
		{
			// Parse all properties.
			while( *Buffer != ')' )
			{
				// Get key name.
				char Name[NAME_SIZE];
				int Count=0;
				while( Count<NAME_SIZE-1 && *Buffer && *Buffer!='=' && *Buffer!='[' )
					Name[Count++] = *Buffer++;
				Name[Count++] = 0;

				// Get optional array element.
				INT Element=0;
				if( *Buffer=='[' )
				{
					const char* Start=Buffer;
					while( *Buffer>='0' && *Buffer<='9' )
						Buffer++;
					if( *Buffer++ != ']' )
					{
						debugf( NAME_Warning, "ImportText: Illegal array element" );
						return NULL;
					}
					Element = appAtoi( Start );
				}

				// Verify format.
				if( *Buffer++ != '=' )
				{
					debugf( NAME_Warning, "ImportText: Illegal or missing key name" );
					return NULL;
				}

				// See if the property exists in the struct.
				FName GotName( Name, FNAME_Find );
				UBOOL Parsed = 0;
				if( GotName!=NAME_None )
				{
					for( TFieldIterator<UProperty> It(Struct); It; ++It )
					{
						UProperty* Property = *It;
						if
						(	Property->GetFName()==GotName
						&&	Element>=0
						&&	Element<Property->ArrayDim
						&&	Property->GetSize()!=0
						&&	Property->Port() )
						{
							// Import this property.
							Buffer = Property->ImportText( Buffer, Data + Property->Offset + Element*Property->GetElementSize(),0 );
							if( Buffer == NULL )
								return NULL;

							// Done with this property.
							Parsed = 1;
						}
					}
				}

				// If not parsed, skip this property in the stream.
				if( !Parsed )
				{
					INT SubCount=0;
					while
					(	*Buffer
					&&	*Buffer!=10
					&&	*Buffer!=13 
					&&	(SubCount>0 || *Buffer!=')')
					&&	(SubCount>0 || *Buffer!=',') )
					{
						if( *Buffer == 0x22 )
						{
							while( *Buffer && *Buffer!=0x22 && *Buffer!=10 && *Buffer!=13 )
								Buffer++;
							if( *Buffer != 0x22 )
							{
								debugf( NAME_Warning, "ImportText: Bad quoted string" );
								return NULL;
							}
						}
						else if( *Buffer == '(' )
						{
							SubCount++;
						}
						else if( *Buffer == ')' )
						{
							SubCount--;
							if( SubCount < 0 )
							{
								debugf( NAME_Warning, "ImportText: Bad parenthesised struct" );
								return NULL;
							}
						}
						Buffer++;
					}
					if( SubCount > 0 )
					{
						debugf( NAME_Warning, "ImportText: Incomplete parenthesised struct" );
						return NULL;
					}
				}

				// Skip comma.
				if( *Buffer==',' )
				{
					// Skip comma.
					Buffer++;
				}
				else if( *Buffer!=')' )
				{
					debugf( NAME_Warning, "ImportText: Bad termination" );
					return NULL;
				}
			}

			// Skip trailing ')'.
			Buffer++;
		}
		else
		{
			debugf( NAME_Warning, "ImportText: Struct missing '('" );
			return NULL;
		}
		return Buffer;
	}
	unguard;
}
IMPLEMENT_CLASS(UStructProperty);

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

/*=============================================================================
	UnSyntax.cpp: Unreal script syntax highlighting functions.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"

/*---------------------------------------------------------------------------------------
	Globals.
---------------------------------------------------------------------------------------*/

// Rich text format codes for syntax highlighting.
#define RTF_FILE_PREPEND	"{\\rtf1\\ansi\\deff0\\deftab720{\\fonttbl{\\f0\\fswiss MS Sans Serif;}{\\f1\\froman\\fcharset2 Symbol;}{\\f2\\fmodern Courier New;}{\\f3\\fmodern Courier New;}}\r\n" \
							"{\\colortbl" \
							"\\red0\\green0\\blue0;" \
							"\\red0\\green255\\blue0;" \
							"\\red0\\green255\\blue255;" \
							"\\red255\\green255\\blue0;" \
							"\\red255\\green0\\blue0;" \
							"\\red0\\green0\\blue255;" \
							"\\red255\\green0\\blue255;" \
							"\\red255\\green255\\blue255;" \
							"\\red128\\green128\\blue128;" \
							"}\r\n" \
							"\\deflang1033\\pard\\tx0\\tx420\\tx840\\tx1260\\tx1680\\tx2100\\tx2520\\tx2940\\tx3360\\tx3780\\tx4200\\tx4620\\tx5040\\tx5460\\tx5880\\tx6300\\plain\\f2\\fs17\\cf1 "

#define RTF_FILE_APPEND		"\\plain\\f3\\fs17\\cf1 \\par }"
#define RTF_LINE_APPEND		"\r\n\\par "

#define RTF_GREEN			"\\plain\\f3\\fs17\\cf1 "
#define RTF_CYAN			"\\plain\\f3\\fs17\\cf2 "
#define RTF_YELLOW			"\\plain\\f3\\fs17\\cf3 "
#define RTF_RED				"\\plain\\f3\\fs17\\cf4 "
#define RTF_BLUE			"\\plain\\f3\\fs17\\cf5 "
#define RTF_MAGENTA			"\\plain\\f3\\fs17\\cf6 "
#define RTF_WHITE			"\\plain\\f3\\fs17\\cf7 "
#define RTF_GREY			"\\plain\\f3\\fs17\\cf8 "

// UnrealScript keywords to highlight.
static inline BYTE CalcHash( const char *c )
{
	return appToUpper(c[0]) + appToUpper(c[1]) * 13;
}
struct FKeyHash
{
	FKeyHash *Next;
	char	 *Key;
	FKeyHash( FKeyHash *InNext, char *InKey )
	:	Next(InNext)
	,	Key(InKey)
	{}
};

/*---------------------------------------------------------------------------------------
   Syntax highlighting functions.
---------------------------------------------------------------------------------------*/

AUTOREGISTER_TOPIC("RTF",RtfTopicHandler);
void RtfTopicHandler::Get( ULevel *Level, const char *Item, FOutputDevice &Out )
{
	guard(RtfTopicHandler::Get);

	// Hash the keywords.
	static int Inited=0;
	static FKeyHash *Hash[256];

	if( !Inited )
	{
		// Init hash table.
		Inited = 1;
		appMemset( Hash, 0, sizeof(Hash) );

		// Hash the hardcoded names which are tagged for syntax highlighting.
		for( int i=0; i<FName::GetMaxNames(); i++ )
		{
			FNameEntry *Entry = FName::GetEntry(i);
			if( Entry && Entry->Flags & RF_HighlightedName )
				Hash[CalcHash(Entry->Name)] = new FKeyHash( Hash[CalcHash(Entry->Name)], Entry->Name );
		}
	}

	// Look up text buffer.
	UClass* Class = FindObject<UClass>( ANY_PACKAGE, Item );
	UTextBuffer* Text = Class ? Class->ScriptText : NULL;
	if( Text && Text->Text.Length() )
	{
		Out.Log(RTF_FILE_PREPEND);

		// Convert all lines to rtf.
		int iLine=1;
		const char* Stream = *Text->Text;
		char Line[2048];
		int CommentCount=0;
		while( *Stream != 0 )
		{
			// Get line.
			int LineComment = 0;
			int IsLiteral   = 0;
			int IsQuote     = 0;
			int FirstWord   = 1;
			char *End = Line, *Word = NULL;
			if( *Stream != '#' )
			{
				End = Line + appSprintf( Line, "%s", CommentCount ? RTF_GREEN : RTF_WHITE );
			}
			else
			{
				IsLiteral = 1;
				End = Line + appSprintf( Line, "%s", CommentCount ? RTF_GREEN : RTF_GREY );
			}
			for( ; ; )
			{
				// Detect keywords.
				if( CommentCount==0 && !IsQuote && !IsLiteral )
				{
					if( Word==NULL )
					{
						if( appIsAlpha(*Stream) || *Stream=='_' )
						{
							// Got start of word.
							Word = End;
						}
					}
					else
					{
						if( !appIsAlnum(*Stream) && *Stream!='_')
						{
							// Got end of word.
							*End = 0;
							if( FirstWord && *Stream==':' && appStricmp(Word,"default")!=0 )
							{
								// Label.
								char Temp[256];
								appStrcpy( Temp, Word );
								End = Word + appSprintf(Word,"%s%s%s", RTF_YELLOW, Temp, RTF_WHITE );
							}
							else for( FKeyHash *Item=Hash[CalcHash(Word)]; Item; Item=Item->Next )
							{
								if( appStricmp(Word,Item->Key)==0 )
								{
									// Found a keyword to syntax highlight.
									char Temp[256];
									appStrcpy( Temp, Word );
									End = Word + appSprintf(Word,"%s%s%s", RTF_CYAN, Temp, RTF_WHITE );
									break;
								}
							}
							Word      = NULL;
							FirstWord = 0;
						}
					}
				}

				// Detect end of line.
				if( *Stream==0 || *Stream==13 )
					break;

				// Handle quotes and comments.
				if( *Stream==34 && CommentCount==0 && !IsLiteral )
				{
					if( IsQuote )	End += appSprintf(End,"\"" RTF_WHITE);
					else			End += appSprintf(End,RTF_GREEN "\"");
					IsQuote = !IsQuote;
					Stream++;
				}
				else if( *Stream=='\\' || *Stream=='{' || *Stream=='}' )
				{
					*End++ = '\\';
					*End++ = *Stream++;
				}
				else if( *Stream=='/' && Stream[1]=='/' && CommentCount==0 && !IsQuote )
				{
					End += appSprintf(End,RTF_GREEN);
					*End++ = *Stream++;
					*End++ = *Stream++;
					CommentCount++;
					LineComment=1;
				}
				else if( *Stream=='/' && Stream[1]=='*' )
				{
					End += appSprintf(End,RTF_GREEN);
					*End++ = *Stream++;
					*End++ = *Stream++;
					CommentCount++;
				}
				else if( *Stream=='*' && Stream[1]=='/' )
				{
					*End++ = *Stream++;
					*End++ = *Stream++;
					End += appSprintf(End,RTF_WHITE);
					CommentCount = Max(CommentCount-1,0);
				}
				else
				{
					*End++ = *Stream++;
				}
			}
			if( *Stream==13 ) Stream++;
			if( *Stream==10 ) Stream++;

			// Finish up.
			*End++ = 0;
			CommentCount -= LineComment;

			// Output it.
			Out.Logf("%s%s", Line, RTF_LINE_APPEND );
			iLine++;
		}
		Out.Log(RTF_FILE_APPEND);
	}
	unguard;
}
void RtfTopicHandler::Set( ULevel *Level, const char *Item, const char *Data )
{
	guard(RtfTopicHandler::Set);
	unguard;
}

/*---------------------------------------------------------------------------------------
   The End.
---------------------------------------------------------------------------------------*/

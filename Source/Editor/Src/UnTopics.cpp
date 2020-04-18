/*=============================================================================
	UnTopics.cpp: FTopicTable implementation
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	What's happening: This is how pieces of information are exchanged between
	the editor app and the server.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"

/*-------------------------------------------------------------------------------
	FTopicTable init/exit/register.
-------------------------------------------------------------------------------*/

//
// Initialize the topic table by clearing out its linked list.
// Only initializes on the very first call to it, because the topic
// table init function may be called several times during startup.
//
void FGlobalTopicTable::Init()
{
	guard(FGlobalTopicTable::Init);

	static int Started=0;
	if( !Started )
	{
		Started			= 1;
		FirstHandler	= NULL;
	}
	unguard;
}

//
// Shut down the topic table by clearing freeing its linked list.
//
void FGlobalTopicTable::Exit()
{
	guard(FGlobalTopicTable::Exit);
	FirstHandler = NULL;
	unguard;
}

//
// Register a topic.
//
//warning: This routine is called in the object type constructors before
// anything has been initialized and before the code has actually begun
// executing.  Anything done here must be 100% safe independent of startup order.
//
void FGlobalTopicTable::Register(const char *TopicName,FTopicHandler *Handler)
{
	static int Started=0;
	if( !Started )
	{
		Init();
		Started = 1;
	}
	if( TopicName && *TopicName && appStrlen(TopicName)<NAME_SIZE )
	{
		// Create a new TopicInfo entry and insert it in the linked list of topics.
		appStrcpy( Handler->TopicName, TopicName );
		Handler->Next = FirstHandler;
		FirstHandler  = Handler;
	}
}

/*-------------------------------------------------------------------------------
	FTopicTable Find/Get/Set.
-------------------------------------------------------------------------------*/

//
// Find a named topic in the topic table, and return a pointer
// to its handler, or NULL if not found.
//
FTopicHandler* FGlobalTopicTable::Find(const char *TopicName)
{
	guard(FGlobalTopicTable::Find);

	FTopicHandler *Handler = FirstHandler;
	while( Handler )
	{
		if( appStricmp( TopicName, Handler->TopicName )==0 )
			return Handler;
		Handler = Handler->Next;
	}
	return NULL;

	unguard;
}

//
// Get the value of a topic's item.
//
void FGlobalTopicTable::Get( ULevel *Level, const char *Topic, const char *Item, FOutputDevice &Out )
{
	guard(FGlobalTopicTable::Get);
	check(Level!=NULL);

	FTopicHandler *Handler = Find(Topic);
	if( Handler )
		Handler->Get(Level,Item,Out);

	unguard;
}

//
// Set the value of a topic's item.
//
void FGlobalTopicTable::Set( ULevel *Level, const char *Topic, const char *Item, const char *Value )
{
	guard(FGlobalTopicTable::Set);
	check(Level!=NULL);

	FTopicHandler *Handler = Find(Topic);
	if( Handler )
		Handler->Set(Level,Item,Value);

	unguard;
}

/*-------------------------------------------------------------------------------
	The End.
-------------------------------------------------------------------------------*/

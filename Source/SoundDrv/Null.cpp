#pragma warning (disable:4201)
#include <windows.h>
#include <mmsystem.h>
#include "Engine.h"
#include "UnRender.h"
class DLL_EXPORT UNullAudioSubsystem : public UAudioSubsystem
{
	DECLARE_CLASS(UNullAudioSubsystem, UAudioSubsystem, CLASS_Config)
public:
	virtual UBOOL Init();
	virtual void SetViewport(UViewport* Viewport);
	virtual UBOOL Exec(const char* Cmd, FOutputDevice* Out = GSystem);
	virtual void Update(FPointRegion Region, FCoords& Listener);
	virtual void RegisterMusic(UMusic* Music);;
	virtual void RegisterSound(USound* Music);
	virtual void UnregisterSound(USound* Sound);
	virtual void UnregisterMusic(UMusic* Music);
	virtual UBOOL PlaySound(AActor* Actor, INT Id, USound* Sound, FVector Location, FLOAT Volume, FLOAT Radius, FLOAT Pitch);
	virtual void NoteDestroy(AActor* Actor);
	virtual UBOOL GetLowQualitySetting();
};




UBOOL UNullAudioSubsystem::Init()
{
	return UBOOL();
}

void UNullAudioSubsystem::SetViewport(UViewport * Viewport)
{
}

UBOOL UNullAudioSubsystem::Exec(const char * Cmd, FOutputDevice * Out)
{
	return UBOOL();
}

void UNullAudioSubsystem::Update(FPointRegion Region, FCoords & Listener)
{
}

void UNullAudioSubsystem::RegisterMusic(UMusic * Music)
{
}

void UNullAudioSubsystem::RegisterSound(USound * Music)
{
}

void UNullAudioSubsystem::UnregisterSound(USound * Sound)
{
}

void UNullAudioSubsystem::UnregisterMusic(UMusic * Music)
{
}

UBOOL UNullAudioSubsystem::PlaySound(AActor * Actor, INT Id, USound * Sound, FVector Location, FLOAT Volume, FLOAT Radius, FLOAT Pitch)
{
	return UBOOL();
}

void UNullAudioSubsystem::NoteDestroy(AActor * Actor)
{
}

UBOOL UNullAudioSubsystem::GetLowQualitySetting()
{
	return UBOOL();
}

IMPLEMENT_CLASS(UNullAudioSubsystem);
IMPLEMENT_PACKAGE(SoundDrv);
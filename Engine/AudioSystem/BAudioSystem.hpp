#pragma once

//-------------------------------------------------------------------------------------------------
// Link in the fmodex_vc.lib static library
#pragma comment( lib, "ThirdParty/fmod/fmodex_vc" )


//-------------------------------------------------------------------------------------------------
#include "Engine/MemorySystem/UntrackedAllocator.hpp"
#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>


//-------------------------------------------------------------------------------------------------
class BAudioSystem;


//-------------------------------------------------------------------------------------------------
typedef unsigned int SoundID;
typedef void * AudioChannelHandle;


//-------------------------------------------------------------------------------------------------
class BAudioSystem
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static int const MAX_AUDIO_DEVICE_NAME_LEN = 256;
	static unsigned int const MISSING_SOUND_ID = 0xffffffff;

private:
	static BAudioSystem * s_AudioSystem;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	FMOD::System * m_fmodSystem;
	std::map<size_t, SoundID> m_registeredSoundIDs;
	std::vector<FMOD::Sound*> m_registeredSounds;

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:
	static void Startup();
	static void Shutdown();
	static void Update(); // Must be called at regular intervals (e.g. every frame)
	static BAudioSystem * GetSystem();
	static void ValidateResult(FMOD_RESULT result);
	static SoundID CreateOrGetSound(std::string const & soundFileName);
	static AudioChannelHandle PlaySound(SoundID soundID, float volumeLevel = 1.f, bool looping = false);
	static void StopSound(AudioChannelHandle channel);

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
private:
	BAudioSystem();
	~BAudioSystem();
	SoundID SystemCreateOrGetSound(std::string const & soundFileName);
	AudioChannelHandle SystemPlaySound(SoundID soundID, float volumeLevel = 1.f, bool looping = false);
	void SystemStopSound(AudioChannelHandle channel);
};
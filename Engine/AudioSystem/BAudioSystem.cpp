#include "Engine/AudioSystem/BAudioSystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"


//-------------------------------------------------------------------------------------------------
STATIC bool BAudioSystem::s_initialized = false;
STATIC FMOD::System * BAudioSystem::s_fmodSystem = nullptr;
STATIC std::map<size_t, SoundID> BAudioSystem::s_registeredSoundIDs;
STATIC std::vector<FMOD::Sound*> BAudioSystem::s_registeredSounds;


//---------------------------------------------------------------------------
// FMOD startup code based on "GETTING STARTED With FMOD Ex Programmer’s API for Windows" document
//	from the FMOD programming API at http://www.fmod.org/download/
STATIC void BAudioSystem::Startup()
{
	if(s_initialized)
	{
		return;
	}

	// Create a System object and initialize.
	FMOD_RESULT result = FMOD::System_Create(&s_fmodSystem);
	ValidateResult(result);

	if(!s_fmodSystem)
	{
		return;
	}

	unsigned int fmodVersion;
	result = s_fmodSystem->getVersion(&fmodVersion);
	ValidateResult(result);

	if(fmodVersion < FMOD_VERSION)
	{
		DebuggerPrintf("AUDIO SYSTEM ERROR!  Your FMOD .dll is of an older version (0x%08x == %d) than that the .lib used to compile this code (0x%08x == %d).\n", fmodVersion, fmodVersion, FMOD_VERSION, FMOD_VERSION);
	}

	int numDrivers;
	result = s_fmodSystem->getNumDrivers(&numDrivers);
	ValidateResult(result);

	if(numDrivers == 0)
	{
		result = s_fmodSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
		ValidateResult(result);
	}
	else
	{
		FMOD_CAPS deviceCapabilities;
		FMOD_SPEAKERMODE speakerMode;
		result = s_fmodSystem->getDriverCaps(0, &deviceCapabilities, 0, &speakerMode);
		ValidateResult(result);

		// Set the user selected speaker mode.
		result = s_fmodSystem->setSpeakerMode(speakerMode);
		ValidateResult(result);

		if(deviceCapabilities & FMOD_CAPS_HARDWARE_EMULATED)
		{
			// The user has the 'Acceleration' slider set to off! This is really bad
			// for latency! You might want to warn the user about this.
			result = s_fmodSystem->setDSPBufferSize(1024, 10);
			ValidateResult(result);
		}

		char audioDeviceName[MAX_AUDIO_DEVICE_NAME_LEN];
		result = s_fmodSystem->getDriverInfo(0, audioDeviceName, MAX_AUDIO_DEVICE_NAME_LEN, 0);
		ValidateResult(result);

		if(strstr(audioDeviceName, "SigmaTel"))
		{
			// Sigmatel sound devices crackle for some reason if the format is PCM 16bit.
			// PCM floating point output seems to solve it.
			result = s_fmodSystem->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);
			ValidateResult(result);
		}
	}

	result = s_fmodSystem->init(100, FMOD_INIT_NORMAL, 0);
	if(result == FMOD_ERR_OUTPUT_CREATEBUFFER)
	{
		// Ok, the speaker mode selected isn't supported by this sound card. Switch it
		// back to stereo...
		result = s_fmodSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
		ValidateResult(result);

		// ... and re-init.
		result = s_fmodSystem->init(100, FMOD_INIT_NORMAL, 0);
		ValidateResult(result);
	}

	s_initialized = true;
}


//-------------------------------------------------------------------------------------------------
STATIC void BAudioSystem::Shutdown()
{
	if(s_initialized && s_fmodSystem)
	{
		//delete s_fmodSystem;
		s_fmodSystem = nullptr;
	}
}


//---------------------------------------------------------------------------
STATIC void BAudioSystem::Update()
{
	if(s_initialized && s_fmodSystem)
	{
		FMOD_RESULT result = s_fmodSystem->update();
		ValidateResult(result);
	}
}


//---------------------------------------------------------------------------
STATIC SoundID BAudioSystem::CreateOrGetSound(std::string const & soundFileName)
{
	if(!s_initialized || !s_fmodSystem)
	{
		return MISSING_SOUND_ID;
	}

	size_t soundFileNameHash = std::hash<std::string>{}(soundFileName);
	std::map<size_t, SoundID>::iterator found = s_registeredSoundIDs.find(soundFileNameHash);
	if(found != s_registeredSoundIDs.end())
	{
		return found->second;
	}
	else
	{
		FMOD::Sound* newSound = nullptr;
		s_fmodSystem->createSound(soundFileName.c_str(), FMOD_DEFAULT, nullptr, &newSound);
		if(newSound)
		{
			SoundID newSoundID = s_registeredSounds.size();
			s_registeredSoundIDs[soundFileNameHash] = newSoundID;
			s_registeredSounds.push_back(newSound);
			return newSoundID;
		}
	}

	return MISSING_SOUND_ID;
}


//---------------------------------------------------------------------------
STATIC AudioChannelHandle BAudioSystem::PlaySound(SoundID soundID, float volumeLevel /*= 1.f*/, bool looping /*= false*/)
{
	if(!s_initialized || !s_fmodSystem)
	{
		return nullptr;
	}

	unsigned int numSounds = s_registeredSounds.size();
	if(soundID < 0 || soundID >= numSounds)
		return nullptr;

	FMOD::Sound * sound = s_registeredSounds[soundID];
	if(!sound)
		return nullptr;

	FMOD::Channel * channelAssignedToSound = nullptr;
	s_fmodSystem->playSound(FMOD_CHANNEL_FREE, sound, false, &channelAssignedToSound);
	if(channelAssignedToSound)
	{
		channelAssignedToSound->setVolume(volumeLevel);
		if(looping)
		{
			channelAssignedToSound->setMode(FMOD_LOOP_NORMAL);
		}
	}

	return static_cast<AudioChannelHandle>(channelAssignedToSound);
}


//---------------------------------------------------------------------------
STATIC void BAudioSystem::StopSound(AudioChannelHandle channel)
{
	FMOD::Channel* fmodChannel = static_cast<FMOD::Channel*>(channel);
	if(fmodChannel)
	{
		fmodChannel->stop();
	}
}


//---------------------------------------------------------------------------
STATIC void BAudioSystem::ValidateResult(FMOD_RESULT result)
{
	if(result != FMOD_OK)
	{
		DebuggerPrintf("AUDIO SYSTEM ERROR: Got error result code %d.\n", result);
		__debugbreak();
	}
}
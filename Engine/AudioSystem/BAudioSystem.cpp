#include "Engine/AudioSystem/BAudioSystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/EventSystem/BEventSystem.hpp"


//-------------------------------------------------------------------------------------------------
STATIC BAudioSystem * BAudioSystem::s_System = nullptr;


//-------------------------------------------------------------------------------------------------
// FMOD startup code based on "GETTING STARTED With FMOD Ex Programmer’s API for Windows" document
// from the FMOD programming API at http://www.fmod.org/download/
STATIC void BAudioSystem::Startup()
{
	if(!s_System)
	{
		s_System = new BAudioSystem();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BAudioSystem::Shutdown()
{
	if(s_System)
	{
		delete s_System;
		s_System = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC BAudioSystem * BAudioSystem::CreateOrGetSystem()
{
	if(!s_System)
	{
		Startup();
	}
	return s_System;
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


//-------------------------------------------------------------------------------------------------
STATIC SoundID BAudioSystem::CreateOrGetSound(std::string const & soundFileName)
{
	if(s_System)
	{
		return s_System->SystemCreateOrGetSound(soundFileName);
	}
	return MISSING_SOUND_ID;
}


//-------------------------------------------------------------------------------------------------
STATIC AudioChannelHandle BAudioSystem::PlaySound(SoundID soundID, float volumeLevel /*= 1.f*/, bool looping /*= false*/)
{
	if(s_System)
	{
		return s_System->SystemPlaySound(soundID, volumeLevel, looping);
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
STATIC void BAudioSystem::StopSound(AudioChannelHandle channel)
{
	if(s_System)
	{
		s_System->SystemStopSound(channel);
	}
}


//-------------------------------------------------------------------------------------------------
BAudioSystem::BAudioSystem()
	: m_fmodSystem(nullptr)
{
	// Create a System object and initialize.
	FMOD_RESULT result = FMOD::System_Create(&m_fmodSystem);
	ValidateResult(result);

	if(!m_fmodSystem)
	{
		ERROR_AND_DIE("FMOD System was not created.");
	}

	unsigned int fmodVersion;
	result = m_fmodSystem->getVersion(&fmodVersion);
	ValidateResult(result);

	if(fmodVersion < FMOD_VERSION)
	{
		DebuggerPrintf("AUDIO SYSTEM ERROR!  Your FMOD .dll is of an older version (0x%08x == %d) than that the .lib used to compile this code (0x%08x == %d).\n", fmodVersion, fmodVersion, FMOD_VERSION, FMOD_VERSION);
	}

	int numDrivers;
	result = m_fmodSystem->getNumDrivers(&numDrivers);
	ValidateResult(result);

	if(numDrivers == 0)
	{
		result = m_fmodSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
		ValidateResult(result);
	}
	else
	{
		FMOD_CAPS deviceCapabilities;
		FMOD_SPEAKERMODE speakerMode;
		result = m_fmodSystem->getDriverCaps(0, &deviceCapabilities, 0, &speakerMode);
		ValidateResult(result);

		// Set the user selected speaker mode.
		result = m_fmodSystem->setSpeakerMode(speakerMode);
		ValidateResult(result);

		if(deviceCapabilities & FMOD_CAPS_HARDWARE_EMULATED)
		{
			// The user has the 'Acceleration' slider set to off! This is really bad
			// for latency! You might want to warn the user about this.
			result = m_fmodSystem->setDSPBufferSize(1024, 10);
			ValidateResult(result);
		}

		char audioDeviceName[MAX_AUDIO_DEVICE_NAME_LEN];
		result = m_fmodSystem->getDriverInfo(0, audioDeviceName, MAX_AUDIO_DEVICE_NAME_LEN, 0);
		ValidateResult(result);

		if(strstr(audioDeviceName, "SigmaTel"))
		{
			// Sigmatel sound devices crackle for some reason if the format is PCM 16bit.
			// PCM floating point output seems to solve it.
			result = m_fmodSystem->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);
			ValidateResult(result);
		}
	}

	result = m_fmodSystem->init(100, FMOD_INIT_NORMAL, 0);
	if(result == FMOD_ERR_OUTPUT_CREATEBUFFER)
	{
		// Ok, the speaker mode selected isn't supported by this sound card. Switch it
		// back to stereo...
		result = m_fmodSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
		ValidateResult(result);

		// ... and re-init.
		result = m_fmodSystem->init(100, FMOD_INIT_NORMAL, 0);
		ValidateResult(result);
	}

	BEventSystem::RegisterEvent(EVENT_ENGINE_UPDATE, this, &BAudioSystem::OnUpdate);
}


//-------------------------------------------------------------------------------------------------
BAudioSystem::~BAudioSystem()
{
	BEventSystem::Unregister(this);
}


//-------------------------------------------------------------------------------------------------
void BAudioSystem::OnUpdate(NamedProperties &)
{
	if(m_fmodSystem)
	{
		// Must be called at regular intervals (e.g. every frame)
		FMOD_RESULT result = m_fmodSystem->update();
		ValidateResult(result);
	}
}


//---------------------------------------------------------------------------
SoundID BAudioSystem::SystemCreateOrGetSound(std::string const & soundFileName)
{
	if(!s_System || !m_fmodSystem)
	{
		return MISSING_SOUND_ID;
	}

	size_t soundFileNameHash = std::hash<std::string>{}(soundFileName);
	std::map<size_t, SoundID>::iterator found = m_registeredSoundIDs.find(soundFileNameHash);
	if(found != m_registeredSoundIDs.end())
	{
		return found->second;
	}
	else
	{
		FMOD::Sound* newSound = nullptr;
		m_fmodSystem->createSound(soundFileName.c_str(), FMOD_DEFAULT, nullptr, &newSound);
		if(newSound)
		{
			SoundID newSoundID = m_registeredSounds.size();
			m_registeredSoundIDs[soundFileNameHash] = newSoundID;
			m_registeredSounds.push_back(newSound);
			return newSoundID;
		}
	}

	return MISSING_SOUND_ID;
}


//---------------------------------------------------------------------------
AudioChannelHandle BAudioSystem::SystemPlaySound(SoundID soundID, float volumeLevel /*= 1.f*/, bool looping /*= false*/)
{
	if(!s_System || !m_fmodSystem)
	{
		return nullptr;
	}

	unsigned int numSounds = m_registeredSounds.size();
	if(soundID < 0 || soundID >= numSounds)
		return nullptr;

	FMOD::Sound * sound = m_registeredSounds[soundID];
	if(!sound)
		return nullptr;

	FMOD::Channel * channelAssignedToSound = nullptr;
	m_fmodSystem->playSound(FMOD_CHANNEL_FREE, sound, false, &channelAssignedToSound);
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
void BAudioSystem::SystemStopSound(AudioChannelHandle channel)
{
	FMOD::Channel* fmodChannel = static_cast<FMOD::Channel*>(channel);
	if(fmodChannel)
	{
		fmodChannel->stop();
	}
}
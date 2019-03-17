#include "Engine/DebugSystem/BProfilerScopedSample.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/BConsoleSystem.hpp"


//-------------------------------------------------------------------------------------------------
BScopedSample::BScopedSample(char const * sampleTag)
	: tag(sampleTag)
	, startSampleTime(Time::GetCurrentOpCount())
	, stopSampleTime(0)
{
	// Nothing
}


//-------------------------------------------------------------------------------------------------
BScopedSample::~BScopedSample()
{
	stopSampleTime = Time::GetCurrentOpCount();
	double sampleTime = Time::GetTimeFromOpCount(stopSampleTime - startSampleTime);
	BConsoleSystem::AddLog(Stringf("%s: %.5fseconds", tag, sampleTime), BConsoleSystem::DEFAULT);
}
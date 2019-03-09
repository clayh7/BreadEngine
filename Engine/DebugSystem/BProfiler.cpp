#include "Engine/DebugSystem/BProfiler.hpp"

#include <set>
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/BProfilerReport.hpp"
#include "Engine/DebugSystem/BProfilerSample.hpp"
#include "Engine/DebugSystem/Console.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/InputSystem/Input.hpp"
#include "Engine/RenderSystem/BitmapFont.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * BProfiler::ROOT_SAMPLE = "FRAME";
STATIC bool BProfiler::s_initialized = false;
STATIC bool BProfiler::s_toBeEnabled = false;
STATIC bool BProfiler::s_toBePinged = false;
STATIC bool BProfiler::s_enabled = false;
STATIC bool BProfiler::s_reportList = false;
STATIC bool BProfiler::s_show = true;
STATIC BitmapFont * BProfiler::s_profilerFont = nullptr;
STATIC uint64_t BProfiler::s_previousFrameTotalTime = 0;
STATIC BProfilerSample * BProfiler::s_currentSample = nullptr;
STATIC BProfilerSample * BProfiler::s_currentSampleSet = nullptr;
STATIC BProfilerSample * BProfiler::s_previousSampleSet = nullptr;
STATIC ObjectPool<BProfilerSample> BProfiler::s_samplePool = ObjectPool<BProfilerSample>(POOL_SIZE);
STATIC std::vector<TextRenderer*, UntrackedAllocator<TextRenderer*>> BProfiler::s_profilerLines;


//-------------------------------------------------------------------------------------------------
void PingProfilerCommand(Command const &)
{
	g_ConsoleSystem->AddLog("Profiler pinged.", Console::GOOD);
	BProfiler::SetPing();
	BProfiler::SetProfilerVisible(true);
}


//-------------------------------------------------------------------------------------------------
void EnableProfilerCommand(Command const &)
{
	g_ConsoleSystem->AddLog("Profiler enabled.", Console::GOOD);
	BProfiler::SetEnabled(true);
	BProfiler::SetProfilerVisible(true);
}


//-------------------------------------------------------------------------------------------------
void DisableProfilerCommand(Command const &)
{
	g_ConsoleSystem->AddLog("Profiler disabled.", Console::GOOD);
	BProfiler::SetEnabled(false);
	BProfiler::SetProfilerVisible(false);
}


//-------------------------------------------------------------------------------------------------
void ToggleReportLayoutCommand(Command const &)
{
	g_ConsoleSystem->AddLog("Profiler layout changed.", Console::GOOD);
	BProfiler::ToggleLayout();
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Startup()
{
#if DEBUG_PROFILER
	if(s_initialized)
	{
		return;
	}
	s_profilerFont = BitmapFont::CreateOrGetFont("Data/Fonts/ClayFont.png");
	g_ConsoleSystem->RegisterCommand("profiler_enable", EnableProfilerCommand, " : Tracks samples injected in code and reports on screen.");
	g_ConsoleSystem->RegisterCommand("profiler_disable", DisableProfilerCommand, " : Stops tracking samples injected in code and reports on screen.");
	g_ConsoleSystem->RegisterCommand("profiler_ping", PingProfilerCommand, " : Tracks one sample.");
	g_ConsoleSystem->RegisterCommand("profiler_layout", ToggleReportLayoutCommand, " : Change the profiler's report layout. List/Flat");
	s_profilerLines.reserve(LINE_COUNT);
	for(int lineIndex = 0; lineIndex < LINE_COUNT; ++lineIndex)
	{
		TextRenderer * profilerLine = new TextRenderer("", Vector2f(50.f, 850.f - 20.f * lineIndex), 13, s_profilerFont);
		s_profilerLines.push_back(profilerLine);
	}
	s_initialized = true;
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Shutdown()
{
#if DEBUG_PROFILER
	if(s_initialized)
	{
		s_currentSample = nullptr;
		s_samplePool.Destroy();
		for(int lineIndex = 0; lineIndex < LINE_COUNT; ++lineIndex)
		{
			delete s_profilerLines[lineIndex];
			s_profilerLines[lineIndex] = nullptr;
		}
		s_profilerLines.clear();
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Update()
{
	if(!s_initialized)
	{
		return;
	}

#if DEBUG_PROFILER
	//Frame Mark
	if(s_enabled)
	{
		ASSERT_RECOVERABLE(s_currentSample == s_currentSampleSet, "Current Sample is not the root sample");

		//Clear last Sample Set
		if(s_previousSampleSet)
		{
			Delete(s_previousSampleSet);
		}
		if(s_currentSample)
		{
			StopSample();
		}
		//Swap current sample set to previous
		s_previousSampleSet = s_currentSampleSet;
		if(s_previousSampleSet)
		{
			s_previousFrameTotalTime = s_previousSampleSet->opCountEnd - s_previousSampleSet->opCountStart;
		}
	}
	s_enabled = s_toBeEnabled;
	if(s_enabled)
	{
		StartSample(ROOT_SAMPLE);
		s_currentSampleSet = s_currentSample;
	}
	//Ping only one frame
	if(s_toBePinged)
	{
		s_toBeEnabled = false;
		s_toBePinged = false;
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
void GenerateListReport(BProfilerSample const & sample, int & index, int depth, std::multimap<int, BProfilerReport> & report, uint64_t previousFrameTotalTime)
{
#if DEBUG_PROFILER
	BProfilerReport reportNode;
	reportNode.index = index;
	reportNode.tag = sample.tag;
	reportNode.depth = depth;
	reportNode.totalTime = sample.opCountEnd - sample.opCountStart;
	reportNode.childrenTime = 0;
	reportNode.newCount = sample.newCount;
	reportNode.deleteCount = sample.deleteCount;
	BProfilerSample * child = sample.children;
	while(child)
	{
		index += 1;
		reportNode.childrenTime += (child->opCountEnd - child->opCountStart);
		GenerateListReport(*child, index, depth + 1, report, previousFrameTotalTime);
		child = child->next;
	}
	reportNode.selfTime = reportNode.totalTime - reportNode.childrenTime;
	reportNode.percent = ((float)((reportNode.totalTime * 1000) / previousFrameTotalTime)) / 10.f;
	report.insert(std::pair<int, BProfilerReport>(reportNode.index, reportNode));
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
void PopulateTagList(BProfilerSample const & sample, std::set<char const *> & tags)
{
#if DEBUG_PROFILER
	tags.insert(sample.tag);
	BProfilerSample * child = sample.children;
	while(child)
	{
		PopulateTagList(*child, tags);
		child = child->next;
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
void PopulateReportFromSample(BProfilerReport & reportData, BProfilerSample const & sample)
{
#if DEBUG_PROFILER
	//Add contents
	if(reportData.tag == sample.tag)
	{
		reportData.newCount += sample.newCount;
		reportData.deleteCount += sample.deleteCount;
		reportData.totalTime += sample.opCountEnd - sample.opCountStart;
		BProfilerSample * checkChild = sample.children;
		while(checkChild)
		{
			reportData.childrenTime += (checkChild->opCountEnd - checkChild->opCountStart);
			checkChild = checkChild->next;
		}
		reportData.selfTime = reportData.totalTime - reportData.childrenTime;
	}

	//Continue down tree
	BProfilerSample * child = sample.children;
	while(child)
	{
		PopulateReportFromSample(reportData, *child);
		child = child->next;
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
void GenerateFlatReport(BProfilerSample const & sample, std::multimap<int, BProfilerReport> & report, uint64_t previousFrameTotalTime)
{
#if DEBUG_PROFILER
	std::set<char const *> tagNames;
	PopulateTagList(sample, tagNames);
	for(auto tagName : tagNames)
	{
		BProfilerReport currentReport;
		currentReport.tag = tagName;
		PopulateReportFromSample(currentReport, sample);
		currentReport.percent = ((float)((currentReport.selfTime * 1000) / previousFrameTotalTime)) / 10.f;
		//This is to sort it by percent, lowest number is sorted to the top
		report.insert(std::pair<int, BProfilerReport>(-(int)(currentReport.percent * 100.f), currentReport));
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Render()
{
#if DEBUG_PROFILER
	if(!s_initialized)
	{
		return;
	}

	if(!s_show)
	{
		return;
	}

	if(!s_previousSampleSet)
	{
		return;
	}

	//Render previous frame's sample set
	if(s_reportList)
	{
		int line = 0;
		std::multimap<int, BProfilerReport> listReport;
		GenerateListReport(*s_previousSampleSet, line, 0, listReport, s_previousFrameTotalTime);
		RenderReport(listReport);
	}
	else
	{
		std::multimap<int, BProfilerReport> flatReport;
		GenerateFlatReport(*s_previousSampleSet, flatReport, s_previousFrameTotalTime);
		RenderReport(flatReport);
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::StartSample(char const * sampleTag)
{
#if DEBUG_PROFILER
	if(!s_initialized || !s_enabled)
	{
		return;
	}

	BProfilerSample * previousSample = s_currentSample;
	s_currentSample = s_samplePool.Alloc();
	if(s_currentSample)
	{
		s_currentSample->SetTag(sampleTag);
		s_currentSample->parent = previousSample;
	}

	if(previousSample)
	{
		previousSample->AddChild(s_currentSample);
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::StopSample()
{
#if DEBUG_PROFILER
	if(!s_initialized || !s_enabled)
	{
		return;
	}

	if(s_currentSample)
	{
		s_currentSample->opCountEnd = Time::GetCurrentOpCount();
		s_currentSample = s_currentSample->parent;
	}
	else
	{
		ASSERT_RECOVERABLE(s_currentSample, "No current sample in profiler, too many StopSample()'s");
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::SetEnabled(bool isEnabled)
{
	s_toBeEnabled = isEnabled;
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::SetPing()
{
	s_toBePinged = true;
	s_toBeEnabled = true;
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::ToggleLayout()
{
	s_reportList = !s_reportList;
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::SetProfilerVisible(bool show)
{
	s_show = show;
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::IncrementNews()
{
	if(s_initialized && s_currentSample)
	{
		s_currentSample->newCount++;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::IncrementDeletes()
{
	if(s_initialized && s_currentSample)
	{
		s_currentSample->deleteCount++;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Delete(BProfilerSample * sample)
{
	if(s_initialized)
	{
		s_samplePool.Delete(sample);
	}
}


//-------------------------------------------------------------------------------------------------
STATIC bool BProfiler::IsEnabled()
{
	return (s_initialized && s_enabled);
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::RenderReport(std::multimap<int, BProfilerReport> const & report)
{
#if DEBUG_PROFILER
	s_profilerLines[0]->SetText(Stringf("TAG                  TIME      SELF TIME  PERCENT  NEW   DELETE"));
	s_profilerLines[0]->Update();
	s_profilerLines[0]->Render();
	int count = 1;
	for(auto reportData : report)
	{
		BProfilerReport const & profilerReport = reportData.second;
		double timeSeconds = Time::GetTimeFromOpCount(profilerReport.totalTime);
		double selfTimeSeconds = Time::GetTimeFromOpCount(profilerReport.selfTime);
		std::string tag = Stringf("%.*s%s", profilerReport.depth, "--------------------", profilerReport.tag);
		s_profilerLines[count]->SetText(Stringf("%-20s %06.2fms  %06.2fms   %5.1f%%   %-6d%-6d", tag.c_str(), timeSeconds*1000.0, selfTimeSeconds*1000.0, profilerReport.percent, profilerReport.newCount, profilerReport.deleteCount));
		s_profilerLines[count]->Update();
		s_profilerLines[count]->Render();
		count += 1;
		if(count >= LINE_COUNT)
		{
			return;
		}
	}
#endif // DEBUG_PROFILER
}
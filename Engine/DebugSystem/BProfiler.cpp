#include "Engine/DebugSystem/BProfiler.hpp"

#include <set>
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/BProfilerReport.hpp"
#include "Engine/DebugSystem/BProfilerSample.hpp"
#include "Engine/DebugSystem/BConsoleSystem.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/InputSystem/BInputSystem.hpp"
#include "Engine/RenderSystem/BitmapFont.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * BProfiler::ROOT_SAMPLE = "FRAME";
STATIC BProfiler * BProfiler::s_Instance = nullptr;


//-------------------------------------------------------------------------------------------------
void PingProfilerCommand(Command const &)
{
	BConsoleSystem::AddLog("Profiler pinged.", BConsoleSystem::GOOD);
	if(BProfiler::s_Instance)
	{
		BProfiler::s_Instance->SetPing();
		BProfiler::s_Instance->SetProfilerVisible(true);
	}
}


//-------------------------------------------------------------------------------------------------
void EnableProfilerCommand(Command const &)
{
	BConsoleSystem::AddLog("Profiler enabled.", BConsoleSystem::GOOD);
	if(BProfiler::s_Instance)
	{
		BProfiler::s_Instance->SetEnabled(true);
		BProfiler::s_Instance->SetProfilerVisible(true);
	}
}


//-------------------------------------------------------------------------------------------------
void DisableProfilerCommand(Command const &)
{
	BConsoleSystem::AddLog("Profiler disabled.", BConsoleSystem::GOOD);
	if(BProfiler::s_Instance)
	{
		BProfiler::s_Instance->SetEnabled(false);
		BProfiler::s_Instance->SetProfilerVisible(false);
	}
}


//-------------------------------------------------------------------------------------------------
void ToggleReportLayoutCommand(Command const &)
{
	BConsoleSystem::AddLog("Profiler layout changed.", BConsoleSystem::GOOD);
	if(BProfiler::s_Instance)
	{
		BProfiler::s_Instance->ToggleLayout();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Startup()
{
#if DEBUG_PROFILER
	if(!s_Instance)
	{
		s_Instance = new BProfiler();
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Shutdown()
{
#if DEBUG_PROFILER
	if(s_Instance)
	{
		delete s_Instance;
		s_Instance = nullptr;
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::Update()
{
#if DEBUG_PROFILER
	if(s_Instance)
	{
		s_Instance->InstanceUpdate();
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
	if(s_Instance)
	{
		s_Instance->InstanceRender();
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC BProfiler * BProfiler::CreateOrGetInstance()
{
	if(!s_Instance)
	{
		Startup();
	}

	return s_Instance;
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::StartSample(char const * sampleTag)
{
#if DEBUG_PROFILER
	if(s_Instance)
	{
		s_Instance->InstanceStartSample(sampleTag);
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
STATIC void BProfiler::StopSample()
{
#if DEBUG_PROFILER
	if(s_Instance)
	{
		s_Instance->InstanceStopSample();
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
BProfiler::BProfiler()
	: m_toBeEnabled(false)
	, m_toBePinged(false)
	, m_enabled(false)
	, m_reportList(false)
	, m_show(true)
	, m_profilerFont(nullptr)
	, m_previousFrameTotalTime(0)
	, m_currentSample(nullptr)
	, m_currentSampleSet(nullptr)
	, m_previousSampleSet(nullptr)
	, m_samplePool(POOL_SIZE)
	, m_profilerLines()
{
	m_profilerFont = BitmapFont::CreateOrGetFont("Data/Fonts/ClayFont.png");
	BConsoleSystem::Register("profiler_enable", EnableProfilerCommand, " : Tracks samples injected in code and reports on screen.");
	BConsoleSystem::Register("profiler_disable", DisableProfilerCommand, " : Stops tracking samples injected in code and reports on screen.");
	BConsoleSystem::Register("profiler_ping", PingProfilerCommand, " : Tracks one sample.");
	BConsoleSystem::Register("profiler_layout", ToggleReportLayoutCommand, " : Change the profiler's report layout. List/Flat");
	m_profilerLines.reserve(LINE_COUNT);
	for(int lineIndex = 0; lineIndex < LINE_COUNT; ++lineIndex)
	{
		TextRenderer * profilerLine = new TextRenderer("", Vector2f(50.f, 850.f - 20.f * lineIndex), 13, m_profilerFont);
		m_profilerLines.push_back(profilerLine);
	}
}


//-------------------------------------------------------------------------------------------------
BProfiler::~BProfiler()
{
	m_currentSample = nullptr;
	m_samplePool.Destroy();
	for(int lineIndex = 0; lineIndex < LINE_COUNT; ++lineIndex)
	{
		delete m_profilerLines[lineIndex];
		m_profilerLines[lineIndex] = nullptr;
	}
	m_profilerLines.clear();
}


//-------------------------------------------------------------------------------------------------
void BProfiler::InstanceUpdate()
{
	//Frame Mark
	if(m_enabled)
	{
		ASSERT_RECOVERABLE(m_currentSample == m_currentSampleSet, "Current Sample is not the root sample");

		//Clear last Sample Set
		if(m_previousSampleSet)
		{
			Delete(m_previousSampleSet);
		}
		if(m_currentSample)
		{
			StopSample();
		}
		//Swap current sample set to previous
		m_previousSampleSet = m_currentSampleSet;
		if(m_previousSampleSet)
		{
			m_previousFrameTotalTime = m_previousSampleSet->opCountEnd - m_previousSampleSet->opCountStart;
		}
	}
	m_enabled = m_toBeEnabled;
	if(m_enabled)
	{
		StartSample(ROOT_SAMPLE);
		m_currentSampleSet = m_currentSample;
	}
	//Ping only one frame
	if(m_toBePinged)
	{
		m_toBeEnabled = false;
		m_toBePinged = false;
	}
}


//-------------------------------------------------------------------------------------------------
void BProfiler::InstanceRender()
{
	if(!m_show)
	{
		return;
	}

	if(!m_previousSampleSet)
	{
		return;
	}

	//Render previous frame's sample set
	if(m_reportList)
	{
		int line = 0;
		std::multimap<int, BProfilerReport> listReport;
		GenerateListReport(*m_previousSampleSet, line, 0, listReport, m_previousFrameTotalTime);
		RenderReport(listReport);
	}
	else
	{
		std::multimap<int, BProfilerReport> flatReport;
		GenerateFlatReport(*m_previousSampleSet, flatReport, m_previousFrameTotalTime);
		RenderReport(flatReport);
	}
}


//-------------------------------------------------------------------------------------------------
void BProfiler::RenderReport(std::multimap<int, BProfilerReport> const & report)
{
#if DEBUG_PROFILER
	if(m_profilerLines[0])
	{
		m_profilerLines[0]->SetText(Stringf("TAG                  TIME      SELF TIME  PERCENT  NEW   DELETE"));
		m_profilerLines[0]->Update();
		m_profilerLines[0]->Render();
	}
	int count = 1;
	for(auto reportData : report)
	{
		BProfilerReport const & profilerReport = reportData.second;
		double timeSeconds = Time::GetTimeFromOpCount(profilerReport.totalTime);
		double selfTimeSeconds = Time::GetTimeFromOpCount(profilerReport.selfTime);
		std::string tag = Stringf("%.*s%s", profilerReport.depth, "--------------------", profilerReport.tag);
		if(m_profilerLines[count])
		{
			m_profilerLines[count]->SetText(Stringf("%-20s %06.2fms  %06.2fms   %5.1f%%   %-6d%-6d", tag.c_str(), timeSeconds*1000.0, selfTimeSeconds*1000.0, profilerReport.percent, profilerReport.newCount, profilerReport.deleteCount));
			m_profilerLines[count]->Update();
			m_profilerLines[count]->Render();
		}
		count += 1;
		if(count >= LINE_COUNT)
		{
			return;
		}
	}
#endif // DEBUG_PROFILER
}


//-------------------------------------------------------------------------------------------------
void BProfiler::InstanceStartSample(char const * sampleTag)
{
	if(!m_enabled)
	{
		return;
	}

	BProfilerSample * previousSample = m_currentSample;
	m_currentSample = m_samplePool.Alloc();
	if(m_currentSample)
	{
		m_currentSample->SetTag(sampleTag);
		m_currentSample->parent = previousSample;
	}

	if(previousSample)
	{
		previousSample->AddChild(m_currentSample);
	}
}


//-------------------------------------------------------------------------------------------------
void BProfiler::InstanceStopSample()
{
	if(!m_enabled)
	{
		return;
	}

	if(m_currentSample)
	{
		m_currentSample->opCountEnd = Time::GetCurrentOpCount();
		m_currentSample = m_currentSample->parent;
	}
	else
	{
		ASSERT_RECOVERABLE(m_currentSample, "No current sample in profiler, too many StopSample()'s");
	}
}


//-------------------------------------------------------------------------------------------------
void BProfiler::SetEnabled(bool isEnabled)
{
	m_toBeEnabled = isEnabled;
}


//-------------------------------------------------------------------------------------------------
void BProfiler::SetPing()
{
	m_toBePinged = true;
	m_toBeEnabled = true;
}


//-------------------------------------------------------------------------------------------------
void BProfiler::ToggleLayout()
{
	m_reportList = !m_reportList;
}


//-------------------------------------------------------------------------------------------------
void BProfiler::SetProfilerVisible(bool show)
{
	m_show = show;
}


//-------------------------------------------------------------------------------------------------
void BProfiler::IncrementNews()
{
	if(m_currentSample)
	{
		m_currentSample->newCount++;
	}
}


//-------------------------------------------------------------------------------------------------
void BProfiler::IncrementDeletes()
{
	if(m_currentSample)
	{
		m_currentSample->deleteCount++;
	}
}


//-------------------------------------------------------------------------------------------------
void BProfiler::Delete(BProfilerSample * sample)
{
	m_samplePool.Delete(sample);
}


//-------------------------------------------------------------------------------------------------
bool BProfiler::IsEnabled()
{
	return m_enabled;
}
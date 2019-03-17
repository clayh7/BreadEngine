#include "Engine/DebugSystem/BConsoleSystem.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/Logger.hpp"
#include "Engine/InputSystem/BMouseKeyboard.hpp"
#include "Engine/Math/AABB2f.hpp"
#include "Engine/NetworkSystem/RCS/RemoteCommandServer.hpp"
#include "Engine/RenderSystem/BRenderSystem.hpp"
#include "Engine/RenderSystem/RenderState.hpp"
#include "Engine/RenderSystem/Mesh.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/TextRenderer.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
STATIC float BConsoleSystem::OPEN_SPEED = 3.f;
STATIC float BConsoleSystem::BLINK_SPEED = 2.f;
STATIC Vector2f BConsoleSystem::CONSOLE_BORDER(5.f, 5.f);
STATIC float BConsoleSystem::CONSOLE_LINE_BOTTOM = 20.f;
STATIC float BConsoleSystem::CONSOLE_LINE_TO_BOX_GAP_HEIGHT = 35.f;
STATIC float BConsoleSystem::CONSOLE_HEIGHT = 450.f;
STATIC float BConsoleSystem::CONSOLE_LINE_HEIGHT = 25.f;
STATIC float BConsoleSystem::CONSOLE_LEFT_PADDING = 5.f;
STATIC int BConsoleSystem::NUM_LOGS_TO_VIEW = 16;


//-------------------------------------------------------------------------------------------------
STATIC Color BConsoleSystem::GOOD;
STATIC Color BConsoleSystem::BAD;
STATIC Color BConsoleSystem::DEFAULT;
STATIC Color BConsoleSystem::INFO;
STATIC Color BConsoleSystem::REMOTE;
STATIC BConsoleSystem * BConsoleSystem::s_System = nullptr;


//-------------------------------------------------------------------------------------------------
void HelpCommand(Command const &)
{
	if(BConsoleSystem::s_System)
	{
		BConsoleSystem::s_System->ShowHelp();
	}
}


//-------------------------------------------------------------------------------------------------
void ClearCommand(Command const &)
{
	if(BConsoleSystem::s_System)
	{
		BConsoleSystem::s_System->ClearConsoleLogs();
	}
}


//-------------------------------------------------------------------------------------------------
void QuitCommand(Command const &)
{
	BConsoleSystem::AddLog("Quiting...", Color::GREEN);
	g_isQuitting = true;
}


//-------------------------------------------------------------------------------------------------
void LogCommand(Command const & command)
{
	BConsoleSystem * system = BConsoleSystem::s_System;
	if(system)
	{
		std::string defaultArg = "log.txt";
		std::string fileName = command.GetArg(0, defaultArg);
		std::string const logFilePath = Stringf("Data/Logs/%s", &fileName[0]);
		std::string const logFileBuffer = system->BuildLogFile();
		SaveBufferToBinaryFile(logFilePath, logFileBuffer);
		BConsoleSystem::AddLog(Stringf("Printed (%d) logs to file: %s", system->GetLogSize(), &logFilePath[0]), Color::GREEN);
	}
}


//-------------------------------------------------------------------------------------------------
void ShadowCommand(Command const & command)
{
	int arg0 = command.GetArg(0, 0);
	unsigned char amount = (unsigned char)Clamp(arg0, 0, 255);

	if(BConsoleSystem::s_System)
	{
		BConsoleSystem::s_System->SetShadow(amount);
	}
}


//-------------------------------------------------------------------------------------------------
void ServerEchoCommand(Command const &)
{
	BConsoleSystem * system = BConsoleSystem::s_System;
	if(system)
	{
		system->m_serverEchoEnabled = !system->m_serverEchoEnabled;
		system->AddLog((system->m_serverEchoEnabled) ? "Echo enabled." : "Echo disabled.", BConsoleSystem::GOOD);
	}
}


//-------------------------------------------------------------------------------------------------
ConsoleLog::ConsoleLog(std::string const &log, Color const &color)
	: m_log(log)
	, m_color(color)
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
ConsoleLog::~ConsoleLog()
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void SetupColor()
{
	//Static initialization didn't always work because Color names are set at static initialization as well
	BConsoleSystem::GOOD	= Color::GREEN;
	BConsoleSystem::BAD		= Color::RED;
	BConsoleSystem::DEFAULT = Color::WHITE;
	BConsoleSystem::INFO	= Color::BLUE;
	BConsoleSystem::REMOTE	= Color::ORANGE;
}


//-------------------------------------------------------------------------------------------------
STATIC void BConsoleSystem::Startup()
{
	if(!s_System)
	{
		s_System = new BConsoleSystem();

		//Register initial commands
		Register("help", HelpCommand, " : Show all available commands.");
		Register("clear", ClearCommand, " : Clears console and all previous logs.");
		Register("quit", QuitCommand, " : Closes current application.");
		Register("log", LogCommand, " [filename] : Print all console logs to Data/Logs/[filename]. Default = log.txt");
		Register("shadow", ShadowCommand, " [0-255] : Change the drop shadow alpha to [0-255]. Default = 0");
		Register("server_echo", ServerEchoCommand, " : Toggles sending all console lines to all network connections.");

		//Startup text
		s_System->AddLog("Engine: Bread v1.0.0", BConsoleSystem::INFO);
		s_System->AddLog("Author: Clay Howell", BConsoleSystem::INFO);

		//Register Events
		BEventSystem::RegisterEvent(BMouseKeyboard::EVENT_TYPED_CHAR, s_System, &BConsoleSystem::OnTypedChar);
		BEventSystem::RegisterEvent(BMouseKeyboard::EVENT_KEY_DOWN, s_System, &BConsoleSystem::OnKeyDown);
		BEventSystem::RegisterEvent(BMouseKeyboard::EVENT_MOUSE_WHEEL, s_System, &BConsoleSystem::OnMouseWheel);
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BConsoleSystem::Shutdown()
{
	if(s_System)
	{
		delete s_System;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BConsoleSystem::Update()
{
	if(s_System)
	{
		s_System->SystemUpdate();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BConsoleSystem::Render()
{
	if(s_System)
	{
		s_System->SystemRender();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC BConsoleSystem * BConsoleSystem::CreateOrGetSystem()
{
	if(!s_System)
	{
		Startup();
	}
	return s_System;
}


//-------------------------------------------------------------------------------------------------
STATIC void BConsoleSystem::Register(std::string const & commandName, CommandCallback * callback, std::string const & commandDescription)
{
	BConsoleSystem * system = BConsoleSystem::CreateOrGetSystem();
	system->RegisterCommand(commandName, callback, commandDescription);
}


//-------------------------------------------------------------------------------------------------
STATIC void BConsoleSystem::Register(std::string const & commandName, EventCallback * callback, std::string const & commandDescription)
{
	BConsoleSystem * system = BConsoleSystem::CreateOrGetSystem();
	system->RegisterEvent(commandName, callback, commandDescription);
}


//-------------------------------------------------------------------------------------------------
STATIC void BConsoleSystem::AddLog(std::string const & log, Color const & color /*= DEFAULT*/, bool remote /*= false*/, bool debug /*= true*/)
{
	if(s_System)
	{
		s_System->SystemAddLog(log, color, remote, debug);
	}
}


//-------------------------------------------------------------------------------------------------
BConsoleSystem::BConsoleSystem()
	: m_open(false)
	, m_openAmount(0.f)
	, m_currentLog(-1)
	, m_logCount(0)
	, m_blinkTimer(0.f)
	, m_showCursor(false)
	, m_shiftActive(false)
	, m_shadowAmount(255)
	, m_consoleLine("")
	, m_consoleLineTextRenderer(nullptr)
	, m_consoleBox(nullptr)
	, m_consoleBoxBottom(nullptr)
{
	//Setup Colors
	SetupColor();

	//Setup Basic MeshRenderers
	m_consoleLineTextRenderer = new TextRenderer("Console Text", Vector2f::ZERO);
	m_consoleLineTextRenderer->SetColor(BConsoleSystem::DEFAULT);
	m_consoleBox = new MeshRenderer(eMeshShape_QUAD, Transform(Vector3f(0.f, -0.5f, 0.f), Matrix4f::IDENTITY, Vector3f(2.f, 1.f, 1.f)), RenderState::BASIC_2D);
	m_consoleBox->SetUniform("uColor", Color(0, 0, 0, 50));
	m_consoleBoxBottom = new MeshRenderer(eMeshShape_QUAD, Transform(Vector3f(0.f, -0.5f, 0.f), Matrix4f::IDENTITY, Vector3f(2.f, 1.f, 1.f)), RenderState::BASIC_2D);
	m_consoleBoxBottom->SetUniform("uColor", Color(0, 0, 0, 50));
}


//-------------------------------------------------------------------------------------------------
BConsoleSystem::~BConsoleSystem()
{
	BEventSystem::Unregister(this);

	delete m_consoleBoxBottom;
	m_consoleBoxBottom = nullptr;

	delete m_consoleBox;
	m_consoleBox = nullptr;

	delete m_consoleLineTextRenderer;
	m_consoleLineTextRenderer = nullptr;

	//Delete Statics
	for(size_t i = 0; i < m_consoleLogs.size(); ++i)
	{
		ConsoleLog * log = m_consoleLogs[i];
		delete log;
		log = nullptr;
	}
	m_consoleLogs.clear();

	for(size_t i = 0; i < m_consoleTextRenderers.size(); ++i)
	{
		TextRenderer * textRenderer = m_consoleTextRenderers[i];
		delete textRenderer;
		textRenderer = nullptr;
	}
	m_consoleTextRenderers.clear();

	for(auto command : m_commands)
	{
		delete command.second;
		command.second = nullptr;
	}
	m_commands.clear();
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::SystemUpdate()
{
	//Opening Effect Timer
	if(m_open)
	{
		m_openAmount += Time::DELTA_SECONDS * OPEN_SPEED;
		m_blinkTimer += Time::DELTA_SECONDS * BLINK_SPEED;
		if(m_blinkTimer > 1.f)
		{
			m_blinkTimer = 0.f;
			m_showCursor = !m_showCursor;
		}
	}
	else
	{
		m_openAmount -= Time::DELTA_SECONDS * OPEN_SPEED;
	}

	m_openAmount = Clamp(m_openAmount, 0.f, 1.f);

	if(m_openAmount <= 0.f && !m_open)
	{
		return;
	}

	//Update current line mesh
	if(m_showCursor)
	{
		m_consoleLineTextRenderer->SetText(m_consoleLine + "|");
	}
	else
	{
		m_consoleLineTextRenderer->SetText(m_consoleLine);
	}

	//Update console box
	float boxPosition = Lerp(-1.5f, -.5f, m_openAmount);
	m_consoleBox->SetPosition(Vector3f(0.f, boxPosition, 0.f));
	m_consoleBox->Update(true);

	m_consoleBoxBottom->SetPosition(Vector3f(0.f, boxPosition - 0.915f, 0.f));
	m_consoleBoxBottom->Update(true);

	//Update console line
	float linePosition = Lerp(CONSOLE_LINE_BOTTOM - CONSOLE_HEIGHT, CONSOLE_LINE_BOTTOM, m_openAmount);
	m_consoleLineTextRenderer->SetPosition(Vector2f(CONSOLE_LEFT_PADDING, linePosition));
	m_consoleLineTextRenderer->Update();

	//Update logs
	for(unsigned int logIndex = 0; logIndex < m_consoleTextRenderers.size(); ++logIndex)
	{
		float logPositionYOffset = CONSOLE_LINE_HEIGHT * (m_currentLog - logIndex);
		Vector2f logPosition = Vector2f(CONSOLE_LEFT_PADDING, linePosition + CONSOLE_LINE_TO_BOX_GAP_HEIGHT + logPositionYOffset);

		m_consoleTextRenderers[logIndex]->SetColor(m_consoleLogs[logIndex]->m_color);
		m_consoleTextRenderers[logIndex]->SetPosition(logPosition);
		m_consoleTextRenderers[logIndex]->Update();
	}
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::SystemRender() const
{
	if(m_openAmount <= 0.f)
	{
		return;
	}

	BRenderSystem * RSystem = BRenderSystem::s_System;
	if(!RSystem)
	{
		return;
	}

	//Render console box
	RSystem->MeshRender(m_consoleBox);
	RSystem->MeshRender(m_consoleBoxBottom);
	m_consoleLineTextRenderer->Render();

	//Render the lines in the box
	for(int logIndex = 0; logIndex < (int)m_consoleTextRenderers.size(); ++logIndex)
	{
		if(logIndex <= m_currentLog && logIndex > m_currentLog - NUM_LOGS_TO_VIEW)
		{
			m_consoleTextRenderers[logIndex]->Render();
		}
	}
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::RegisterCommand(std::string const & commandName, CommandCallback * callback, std::string const & commandDescription)
{
	//if it's a new command
	size_t commandHash = std::hash<std::string>{}(commandName);
	auto foundCommand = m_commands.find(commandHash);
	if(foundCommand == m_commands.end())
	{
		//Add command description
		std::string fullDescription = Stringf("%s%s", commandName.c_str(), commandDescription.c_str());
		char * descriptionString = CreateNewCString(fullDescription);

		//Make new command
		CommandStaticFunction * registerCommand = new CommandStaticFunction();
		registerCommand->m_description = descriptionString;
		registerCommand->m_function = callback;

		m_commands.insert(std::pair<size_t, CommandBase*>(commandHash, registerCommand));
	}
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::RegisterEvent(std::string const & commandName, EventCallback * callback, std::string const & commandDescription)
{
	//if it's a new command
	size_t commandHash = std::hash<std::string>{}(commandName);
	auto foundCommand = m_commands.find(commandHash);
	if(foundCommand == m_commands.end())
	{
		//Add command description
		std::string fullDescription = Stringf("%s%s", commandName.c_str(), commandDescription.c_str());
		char * descriptionString = CreateNewCString(fullDescription);

		//Make new command
		EventStaticFunction * registerCommand = new EventStaticFunction();
		registerCommand->m_description = descriptionString;
		registerCommand->m_event = callback;

		m_commands.insert(std::pair<size_t, CommandBase*>(commandHash, registerCommand));
	}
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::RunCommand(std::string const & commandString, bool remote /*= false*/)
{
	//Remote commands are commands send over the network
	if(remote)
	{
		std::string remoteCommandString = Stringf("REMOTE COMMAND: %s", commandString.c_str());
		AddLog(remoteCommandString, REMOTE, remote, false);
	}
	else
	{
		AddLog(commandString, DEFAULT, remote, false);
	}

	if(LoggingSystem)
	{
		LoggingSystem->LogPrintf("Input: %s", commandString.c_str());
	}

	Command command(commandString);
	size_t commandHash = std::hash<std::string>{}(command.GetName());
	auto foundCommandIter = m_commands.find(commandHash);
	if(foundCommandIter != m_commands.end())
	{
		//Execute command if registered
		foundCommandIter->second->Execute(command);
	}
	else
	{
		AddLog("Invalid Command. Type 'help' for a list of commands.", BAD, remote);
	}
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::SystemAddLog(std::string const & log, Color const & color /*= DEFAULT*/, bool remote /*= false*/, bool debug /*= true*/)
{
	//Add the new log
	m_consoleLogs.push_back(new ConsoleLog(log, color));
	TextRenderer * logTextRenderer = new TextRenderer(log, Vector2f::ZERO);
	m_consoleTextRenderers.push_back(logTextRenderer);

	//Send to other network connections
	if(!remote)
	{
		if(m_serverEchoEnabled)
		{
			RemoteCommandServer::Send(eRCSMessageType_ECHO, log);
		}
	}

	//Add Console log to logging system
	if(debug)
	{
		if(LoggingSystem)
		{
			LoggingSystem->LogPrintf("%s", log.c_str());
		}
	}

	//If you're looking at the most recent log, have the logs shift up
	if(m_currentLog == m_logCount - 1)
	{
		++m_currentLog;
	}
	++m_logCount;
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::ClearConsoleLogs()
{
	//Clear Statics
	for(ConsoleLog * log : m_consoleLogs)
	{
		delete log;
		log = nullptr;
	}
	m_consoleLogs.clear();

	for(TextRenderer * textRenderer : m_consoleTextRenderers)
	{
		delete textRenderer;
		textRenderer = nullptr;
	}
	m_consoleTextRenderers.clear();

	m_currentLog = -1;
	m_logCount = 0;
}


//-------------------------------------------------------------------------------------------------
std::string const BConsoleSystem::BuildLogFile()
{
	std::string logFile = "";
	std::string newLine = "\r\n";
	for(int logIndex = 0; logIndex < m_logCount; ++logIndex)
	{
		logFile.append(m_consoleLogs[logIndex]->m_log);
		logFile.append(newLine);
	}
	return logFile;
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::ShowHelp()
{
	AddLog("Showing Registered Commands", Color::GREEN);

	// Get list of command descriptions
	std::vector<char*> descriptionList;
	for(auto commandIter = m_commands.begin(); commandIter != m_commands.end(); ++commandIter)
	{
		if(commandIter->second)
		{
			descriptionList.push_back(commandIter->second->m_description);
		}
	}

	// Sort them
	SortStrings(descriptionList);

	// Print them
	for(size_t index = 0; index < descriptionList.size(); ++index)
	{
		AddLog(descriptionList[index]);
	}
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::SetShadow(unsigned char shadowAmount)
{
	m_shadowAmount = shadowAmount;
	AddLog(Stringf("Setting Drop Shadow to: %d", (int)shadowAmount), Color::GREEN);
}


//-------------------------------------------------------------------------------------------------
int BConsoleSystem::GetLogSize() const
{
	return m_logCount;
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::ToggleOpen()
{
	m_open = !m_open;
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::AddChar(unsigned char newChar)
{
	bool success = false;
	if(newChar >= 'a' && newChar <= 'z')
	{
		success = true;
	}

	else if(newChar >= 'A' && newChar <= 'Z')
	{
		success = true;
	}

	else if(newChar >= '0' && newChar <= '9')
	{
		success = true;
	}

	else if(newChar == '-'
		|| newChar == '_'
		|| newChar == '+'
		|| newChar == '='
		|| newChar == '|'
		|| newChar == '/'
		|| newChar == '.'
		|| newChar == ':'
		|| newChar == ';'
		|| newChar == ' ')
	{
		success = true;
	}

	if(success)
	{
		m_consoleLine.push_back(newChar);
	}
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::RemoveLastChar()
{
	if(m_consoleLine.size() > 0)
	{
		m_consoleLine.pop_back();
	}
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::ClearConsoleLine()
{
	m_consoleLine = "";
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::OnTypedChar(NamedProperties & params)
{
	if(m_open)
	{
		KeyCode button;
		params.Get(BMouseKeyboard::PARAM_KEY, button);
		AddChar((unsigned char)button);
	}
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::OnKeyDown(NamedProperties & params)
{
	KeyCode button;
	params.Get(BMouseKeyboard::PARAM_KEY, button);

	//Open/Close Console
	if(button == eKeyboardButton_TILDE)
	{
		ToggleOpen();
	}
	else if(button == eKeyboardButton_BACKSPACE)
	{
		RemoveLastChar();
	}
	else if(button == eKeyboardButton_ESCAPE)
	{
		if(m_consoleLine.length() > 0)
		{
			ClearConsoleLine();
		}
		else
		{
			if(IsOpen())
			{
				ToggleOpen();
			}
		}
	}
	else if(button == eKeyboardButton_ENTER)
	{
		if(m_consoleLine.length() > 0)
		{
			RunCommand(m_consoleLine);
			ClearConsoleLine();
		}
	}
}


//-------------------------------------------------------------------------------------------------
void BConsoleSystem::OnMouseWheel(NamedProperties & params)
{
	int wheelValue;
	params.Get(BMouseKeyboard::PARAM_MOUSE_WHEEL, wheelValue);

	if(wheelValue > 0)
	{
		//Moving up
		m_currentLog--;
	}
	else
	{
		//Moving down
		m_currentLog++;
	}
	m_currentLog = Clamp(m_currentLog, 0, m_logCount - 1);
}
#pragma once

#include <map>
#include <string>
#include <vector>
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/DebugSystem/Command.hpp"
#include "Engine/EventSystem/BEventSystem.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/MemorySystem/UntrackedAllocator.hpp"
#include "Engine/RenderSystem/Color.hpp"
#include "Engine/Utils/StringUtils.hpp"


//-------------------------------------------------------------------------------------------------
typedef void (EventCallback)(NamedProperties &);
class Command;
class MeshRenderer;
class NamedProperties;
class TextRenderer;


//-------------------------------------------------------------------------------------------------
typedef void (CommandCallback)(Command const &);
void HelpCommand(Command const &);
void ClearCommand(Command const &);
void QuitCommand(Command const &);
void LogCommand(Command const &);
void ServerEchoCommand(Command const &);


//-----------------------------------------------------------------------------------------------
class CommandBase
{
public:
	char * m_description;

public:
	virtual void Execute(Command &) const = 0;

public:
	~CommandBase()
	{
		delete m_description;
		m_description = nullptr;
	}
};


//-------------------------------------------------------------------------------------------------
template<typename T_ObjectType, typename T_FunctionType>
class CommandObjectFunction : public CommandBase
{
public:
	T_ObjectType * m_object;
	T_FunctionType m_function;

public:
	virtual void Execute(Command & command) const override
	{
		(m_object->*m_function)(command);
	}
};


//-------------------------------------------------------------------------------------------------
class CommandStaticFunction : public CommandBase
{
public:
	CommandCallback * m_function;

public:
	virtual void Execute(Command & command) const override
	{
		(m_function)(command);
	}
};


//-------------------------------------------------------------------------------------------------
template<typename T_ObjectType, typename T_FunctionType>
class EventObjectFunction : public CommandBase
{
public:
	T_ObjectType * m_object;
	T_FunctionType m_function;

public:
	virtual void Execute(Command & command) const override
	{
		NamedProperties commandEvent;
		commandEvent.Set("Command", command);
		(m_object->*m_function)(commandEvent);
	}
};


//-------------------------------------------------------------------------------------------------
class EventStaticFunction : public CommandBase
{
public:
	EventCallback * m_event;

public:
	virtual void Execute(Command & command) const override
	{
		NamedProperties commandEvent;
		commandEvent.Set("Command", command);
		(m_event)(commandEvent);
	}
};


//-------------------------------------------------------------------------------------------------
class ConsoleLog
{
	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
public:
	std::string m_log;
	Color m_color;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	ConsoleLog(std::string const & log, Color const & color);
	~ConsoleLog();
};


//-------------------------------------------------------------------------------------------------
class BConsoleSystem
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
private:
	static float OPEN_SPEED;
	static float BLINK_SPEED;
	static Vector2f CONSOLE_BORDER;
	static float CONSOLE_LINE_BOTTOM;
	static float CONSOLE_LINE_TO_BOX_GAP_HEIGHT;
	static float CONSOLE_HEIGHT;
	static float CONSOLE_LINE_HEIGHT;
	static float CONSOLE_LEFT_PADDING;
	static int NUM_LOGS_TO_VIEW;

public:
	static Color GOOD;
	static Color BAD;
	static Color DEFAULT;
	static Color INFO;
	static Color REMOTE;
	static BConsoleSystem * s_System;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
public:
	bool m_serverEchoEnabled;

private:
	bool m_open;
	bool m_showCursor;
	bool m_shiftActive;
	int m_currentLog;
	int m_logCount;
	float m_openAmount;
	float m_blinkTimer;
	unsigned char m_shadowAmount;
	std::string m_consoleLine;
	TextRenderer * m_consoleLineTextRenderer;
	MeshRenderer * m_consoleBox;
	MeshRenderer * m_consoleBoxBottom;

	std::vector<ConsoleLog*> m_consoleLogs;
	std::vector<TextRenderer*> m_consoleTextRenderers;
	std::map<size_t, CommandBase*> m_commands;

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:
	static void Startup();
	static void Shutdown();
	static void Update();
	static void Render();
	static BConsoleSystem * CreateOrGetSystem();
	static void Register(std::string const & commandName, CommandCallback * callback, std::string const & commandDescription);
	static void Register(std::string const & commandName, EventCallback * callback, std::string const & commandDescription);
	static void AddLog(std::string const & log, Color const & color = DEFAULT, bool remote = false, bool debug = true);

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	BConsoleSystem();
	~BConsoleSystem();
	void SystemUpdate();
	void SystemRender() const;
	void RegisterCommand(std::string const & commandName, CommandCallback * callback, std::string const & commandDescription);
	void RegisterEvent(std::string const & commandName, EventCallback * callback, std::string const & commandDescription);
	void RunCommand(std::string const & commandString, bool remote = false);
	void SystemAddLog(std::string const & log, Color const & color = DEFAULT, bool remote = false, bool debug = true);
	void ClearConsoleLogs();
	std::string const BuildLogFile();
	void ShowHelp();
	void SetShadow(unsigned char shadowAmount);
	int GetLogSize() const;
	bool IsOpen() const { return m_open; }

private:
	void ToggleOpen();
	void AddChar(unsigned char newChar);
	void RemoveLastChar();
	void ClearConsoleLine();
	void OnTypedChar(NamedProperties & params);
	void OnKeyDown(NamedProperties & params);
	void OnMouseWheel(NamedProperties & params);

	//-------------------------------------------------------------------------------------------------
	// Function Templates
	//-------------------------------------------------------------------------------------------------
public:
	template<typename T_ObjectType, typename T_FunctionType>
	void Register(std::string const & commandName, T_ObjectType * object, T_FunctionType function, std::string const & commandDescription)
	{
		//Make new command
		CommandObjectFunction<T_ObjectType, T_FunctionType> * registerCommand = new CommandObjectFunction<T_ObjectType, T_FunctionType>();
		registerCommand->m_object = object;
		registerCommand->m_function = function;

		//Add new command
		size_t commandHash = std::hash<std::string>{}(commandName);
		m_commands.insert(std::pair<size_t, CommandBase*>(commandHash, registerCommand));

		//Add command description
		std::string fullDescription = Stringf("%s%s", commandName.c_str(), commandDescription.c_str());
		char * descriptionString = CreateNewCString(fullDescription);
		m_commandDescriptions.insert(std::pair<std::string, char*>(commandName, descriptionString));
	}

	template<typename T_ObjectType, typename T_FunctionType>
	void RegisterEvent(std::string const & commandName, T_ObjectType * object, T_FunctionType function, std::string const & commandDescription)
	{
		//Make new command
		EventObjectFunction<T_ObjectType, T_FunctionType> * registerCommand = new EventObjectFunction<T_ObjectType, T_FunctionType>();
		registerCommand->m_object = object;
		registerCommand->m_function = function;

		//Add new command
		size_t commandHash = std::hash<std::string>{}(commandName);
		m_commands.insert(std::pair<size_t, CommandBase*>(commandHash, registerCommand));

		//Add command description
		std::string fullDescription = Stringf("%s%s", commandName.c_str(), commandDescription.c_str());
		char * descriptionString = CreateNewCString(fullDescription);
		m_commandDescriptions.insert(std::pair<std::string, char*>(commandName, descriptionString));
	}
};
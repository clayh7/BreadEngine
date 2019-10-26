#include "Game/Game.hpp"

#include <vector>
#include "Engine/Utils/StreamReader.hpp"
#include "Engine/Utils/StreamWriter.hpp"


//-------------------------------------------------------------------------------------------------
Game * g_GameSystem = nullptr;


//-------------------------------------------------------------------------------------------------
// Example
StreamWriter g_writer;
StreamReader g_reader;
class Player
{
public:
	Player(char const * name = "None")
		: m_health(10)
		, m_level(3)
		, m_friend(nullptr)
		, m_sequence()
	{
		memcpy(m_name, name, strnlen_s(name, 128));
	}
	void Write(StreamWriter & inStream) const
	{
		inStream.Write(m_health);
		inStream.Write(m_level);
		inStream.Write(m_name, 128);
	}
	void Read(StreamReader & inStream)
	{
		inStream.Read(m_health);
		inStream.Read(m_level);
		inStream.Read(m_name, 128);
	}
private:
	size_t m_health;
	size_t m_level;
	Player * m_friend;
	char m_name[128];
	std::vector<size_t> m_sequence;
};


//-------------------------------------------------------------------------------------------------
Game::Game()
{
	Player testPlayer1 = Player("Isabel");
	testPlayer1.Write(g_writer);
	g_reader.SetBuffer(g_writer);
	Player testPlayer2;
	testPlayer2.Read(g_reader);
}


//-------------------------------------------------------------------------------------------------
Game::~Game()
{

}


//-------------------------------------------------------------------------------------------------
void Game::Update()
{
	g_writer.Write(3);
}


//-------------------------------------------------------------------------------------------------
void Game::Render() const
{

}
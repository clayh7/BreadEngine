#pragma once


//-------------------------------------------------------------------------------------------------
class Euler
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const Euler ZERO;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	float m_pitchDegreesAboutX;
	float m_yawDegreesAboutY;
	float m_rollDegreesAboutZ;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Euler( float pitchX, float yawY, float rollZ );
	~Euler( );
};